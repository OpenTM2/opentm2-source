/*! \file
	Description: This file contains all functions concerned with some kind of initialisation

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // public EDITOR API functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFB.ID"                // Translation Processor IDs

#include <eqfdoc00.h>             // for document handler defines
#include "ReImportDoc.h"          // for re-imort document function

SHORT EQFBFindMatch( PTBDOCUMENT, PFINDDATA);

  /********************************************************************/
  /* support IME (32-bit) for double byte                             */
  /********************************************************************/
  #include <IMM.H>


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncStart
//------------------------------------------------------------------------------
// Function call:     EQFBFuncStart( PSZ,PSZ,PSTEQFGEN,BOOL, PTBDOCUMENT )
//------------------------------------------------------------------------------
// Description:       start the translation browser
//------------------------------------------------------------------------------
// Parameters:        PSZ           pSegSource  ,   //segmented Source file:
//                    PSZ           pSegTarget  ,   // segmented Target file:
//                    PSTEQFGEN     pstEQFGen   ,   // ptr to gen. edit struct
//                    BOOL          fReflow     ,   // reflow allowed ??
//                    PTBDOCUMENT   pDocAnchor      // document to anchor with
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       fOK   success indicator (True/FAlse)
//------------------------------------------------------------------------------
// Function flow:     - build load structure for loading source + target file
//                      (use the values passed with the generic structure
//                            for setting the window positions )
//                    - issue a load call to the editor ( EQFBDocLoad )
//                    - set pointer to generic strucuture in doc structure
//                    - do actionbar settings for SSource and target window
//                    - inform Troja that load was successful
//                    - select first segment and activate Services
//                              ( EQFBTSeg )
//                    - activate the target window
//                             ( special "SOTEC" necessity )
//
//------------------------------------------------------------------------------

BOOL
EQFBFuncStart
(
   PSZ           pSegSource  ,   //segmented Source file:
   PSZ           pSegTarget  ,   // segmented Target file:
   PSTEQFGEN     pstEQFGen   ,   // pointer to generic edit structure
   BOOL          fReflow     ,   // reflow allowed ??
   PTBDOCUMENT   pDocAnchor      // document to anchor with
)

{
   BOOL    fOk = TRUE;                 //error flag
   LOADSTRUCT LoadStruct;              // load structure
   PTBDOCUMENT  pDocument = NULL;      // pointer to document structure
   SHORT   sIDM;                       // for IDM_PROTECTED e.g.
   PTBDOCUMENT  pTgtDoc = NULL;
   EQFBBLOCK* pEQFBBlockMark = get_EQFBBlockMark();
   BOOL            fStartSpellCheck = FALSE;


   memset(&LoadStruct, 0, sizeof(LOADSTRUCT));
   EQFBLoadResource ();                 // load all resources
   #if defined(MEASURETIME)
      GETTIME( ulLoadResource );
   #endif

   TPLOG();

   if ( fOk )
   {
  // build loadstructure to load ssource and starget
       if ( pstEQFGen->flEditTgtStyle )
       {
         LoadStruct.flFrameStyle = (pstEQFGen->flEditTgtStyle & AVAILSTYLES) |
                                      FCF_SIZEBORDER | FCF_MAXBUTTON |
                                      FCF_DBE_APPSTAT;
       }
       else
       {
         LoadStruct.flFrameStyle = FCF_TITLEBAR   | FCF_MENU      | FCF_SYSMENU |
                                   FCF_SIZEBORDER | FCF_MAXBUTTON |
                                   FCF_VERTSCROLL | FCF_HORZSCROLL |
                                   FCF_DBE_APPSTAT; // allow DBCS status line
       } /* endif */
       LoadStruct.fsFlagStyle = EQF_SWP_MOVE | EQF_SWP_SIZE |
                                EQF_SWP_SHOW | EQF_SWP_ACTIVATE;
       if ( pDocAnchor )
       {
         // set size an position to same values as active window
         SWP swpFrame;
         WinQueryWindowPos( pDocAnchor->hwndFrame, &swpFrame );
         RECTL_XLEFT(LoadStruct.rclPos)   =  swpFrame.x;
         RECTL_XRIGHT(LoadStruct.rclPos)  = (swpFrame.cx + swpFrame.x);
         RECTL_YTOP(LoadStruct.rclPos)    = (swpFrame.cy + swpFrame.y);
         RECTL_YBOTTOM(LoadStruct.rclPos) = swpFrame.y;

         // move window one titlebar height down
         if ( !pstEQFGen->fLoadedBySpellcheck )
         {
           RECTL_YTOP(LoadStruct.rclPos)    -= WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
           RECTL_YBOTTOM(LoadStruct.rclPos) -= WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
           EQFBValidatePositions( &LoadStruct.rclPos, STARGET_DOC );
         } /* endif */
       }
       else
       {
         LoadStruct.rclPos =  pstEQFGen->rclEditorTgt;
         EQFBValidatePositions( &LoadStruct.rclPos, STARGET_DOC );
       } /* endif */

       LoadStruct.fReadOnly = TRUE;
       LoadStruct.fReflow   = fReflow; // not used any more: RJ P018279, 03/10/04

       LoadStruct.pFileName = pSegTarget;
       LoadStruct.docType = STARGET_DOC;         // set document type
       LoadStruct.pszTagTable = (PSZ)pstEQFGen->szTagTable;
       LoadStruct.pszEQFTagTable = (PSZ)pstEQFGen->szEQFTagTable;
       LoadStruct.pDoc  = pDocAnchor;            // document to anchor with
       LoadStruct.hwndParent = pstEQFGen->hwndTWBS;
       LoadStruct.pstEQFGen = pstEQFGen;
       LoadStruct.usEditor  = STANDARD_EDITOR;
       TPLOG();

       fOk = ( EQFBDocLoad ( &LoadStruct)  == 0 );

       TPLOG();

       if ( fOk )                            // load document and pass LoadStruct
       {
          pDocument = LoadStruct.pDoc;       // get pointer to loaded doc
          pDocument->pstEQFGen = pstEQFGen;  // store address to generic struct.
          {
            PDOCUMENT_IDA pIdaDoc = (PDOCUMENT_IDA)pstEQFGen->pDoc;
            if ( pIdaDoc )
            {
              if ( !pIdaDoc->ulSrcOemCP)
              {
                pIdaDoc->ulSrcOemCP = GetLangOEMCP(pIdaDoc->szDocSourceLang);
              }
              //fOk = ( EQFBCntAllSrcWords( LoadStruct.pDoc,
              //                            (SHORT)pIdaDoc->usSrcLang,
              //                            pIdaDoc->ulSrcOemCP) == 0 );
            } /* endif */
          }
          switch ( pDocument->DispStyle)
          {
            case  DISP_PROTECTED:
              sIDM = IDM_PROTECTED;
              break;
            case  DISP_UNPROTECTED:
              sIDM = IDM_UNPROTECTED;
              break;
            case  DISP_HIDE:
              sIDM = IDM_HIDE;
              break;
            case  DISP_SHRINK:
              sIDM = IDM_SHRINK;
              break;
            case  DISP_COMPACT:
              sIDM = IDM_COMPACT;
              break;
            default :
              break;
          } /* endswitch */

          // set fForceEqualWhiteSpace flag if the file EQFANAWS
          // exists in the EQF\PROPERTY directory
          {
            CHAR szCheckMe[MAX_EQF_PATH];
            UtlMakeEQFPath( szCheckMe, NULC, PROPERTY_PATH, NULL );
            strcat( szCheckMe, "\\EQFANAWS" );
            pDocument->fForceEqualWhiteSpace = (EQF_BOOL) UtlFileExist( szCheckMe );
          }

          /************************************************************/
          /* check if SpellPullDown should be enabled                 */
          /************************************************************/
          pDocument->fSpellCheck = EQFBUtlSpell( pstEQFGen );
          pTgtDoc = pDocument;

          EnableMenuItem( GetMenu(pDocument->hwndFrame), IDM_SPELL_MENU,
                          pDocument->fSpellCheck ? (MF_BYPOSITION | MF_ENABLED) :
                                              (MF_BYPOSITION | MF_GRAYED) );

          pstEQFGen->hwndEditorTgt = pDocument->hwndClient; // save wnd handle

          TPLOG();

//          UtlDispatch();    // dispatch all incoming messages       /* @9CC */

          {
            PTBDOCUMENT  pTempDoc;             // pointer to document structure

            pTempDoc = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT);
            if ( pTempDoc != NULL )
            {
              pDocument = pTempDoc;
            } /* endif */
          }


          /********************************************************/
          /* allow for DBCS input methods ...                     */
          /********************************************************/
          pDocument->hlfIME = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                                                (LONG)sizeof(LOGFONT));

          TPLOG();

          if ( pDocument )
          {
            LoadStruct.hwndParent = pstEQFGen->hwndTWBS;
            LoadStruct.fReadOnly = TRUE;    // mark every thing readonly
            LoadStruct.fReflow   = fReflow;  // not used any more: RJ P018279, 03/10/04

            LoadStruct.rclPos =  pstEQFGen->rclEditorSrc;
            EQFBValidatePositions( &LoadStruct.rclPos, SSOURCE_DOC );
            LoadStruct.fsFlagStyle = EQF_SWP_MOVE | EQF_SWP_SIZE | EQF_SWP_HIDE ;
            if ( pstEQFGen->flEditSrcStyle )
            {
              LoadStruct.flFrameStyle = (pstEQFGen->flEditSrcStyle & AVAILSTYLES) |
                                         FCF_SIZEBORDER | FCF_MAXBUTTON;
            }
            else
            {
              LoadStruct.flFrameStyle = FCF_TITLEBAR | FCF_MENU | FCF_SYSMENU |
                                        FCF_SIZEBORDER | FCF_MAXBUTTON |
                                        FCF_VERTSCROLL | FCF_HORZSCROLL;
            } /* endif */
            LoadStruct.fsFlagStyle = EQF_SWP_MOVE | EQF_SWP_SIZE;

            LoadStruct.pFileName = pSegSource;
            LoadStruct.docType = SSOURCE_DOC;
            LoadStruct.pstEQFGen = pstEQFGen;

            TPLOG();

            fOk = (EQFBDocLoad( &LoadStruct) == 0 ); // load target document
            TPLOG();

            if ( fOk )
            {
              PDOCUMENT_IDA pIdaDoc = (PDOCUMENT_IDA)pstEQFGen->pDoc;
              if ( pIdaDoc )
              {
                if ( !pIdaDoc->ulSrcOemCP)
                {
                  pIdaDoc->ulSrcOemCP = GetLangOEMCP(pIdaDoc->szDocSourceLang);
                }  
                fOk = ( EQFBCntAllSrcWords( LoadStruct.pDoc, pTgtDoc, 
                                            (SHORT)pIdaDoc->usSrcLang,
                                            pIdaDoc->ulSrcOemCP) == 0 );
              }
            }
          }
          else
          {
             fOk = FALSE;
          } /* endif */
       } /* endif */
       if ( fOk )
       {

          ActivateMTLog( pSegSource, LoadStruct.pszTagTable );
                                              // use last used values later on
          LoadStruct.pDoc->DispStyle = DISP_PROTECTED;   // protect SGML tags
          LoadStruct.pDoc->pstEQFGen = pstEQFGen;  // store generic struct.

                                                // save window handle
          pstEQFGen->hwndEditorSrc = LoadStruct.pDoc->hwndClient;

     // do actionbar settings for STarget
          LoadStruct.pDoc->twin = pDocument;   // store twin
          pDocument->twin =  LoadStruct.pDoc;

     // inform Troja that load was successful
          WinPostMsg( LoadStruct.hwndParent, EQFM_DOC_IS_LOADED, NULL, NULL);

          TPLOG();

//        UtlDispatch();                   // dispatch all incoming messages
          TPLOG();


   #if defined(MEASURETIME)
      GETTIME( ulLoadFile );
   #endif



          if ( WinIsWindow( (HAB) UtlQueryULong( QL_HAB ),
                              pDocument->hwndClient ))

          {
            PTBDOCUMENT pTempDoc;
            POPENANDPOS pOpenAndPos = (POPENANDPOS) pstEQFGen->pOpenAndPos;
            TPLOG();
            pTempDoc = pDocument;
            do
            {
              pTempDoc->fTransEnvAct = FALSE;
              pTempDoc = pTempDoc->next;
            } while ( pTempDoc != pDocument ); /* enddo */
            pDocument->fTransEnvAct = TRUE;
            /*********************************************************/
            /* position at a specific segment or at the current one  */
            /*********************************************************/
            if ( pOpenAndPos != NULL )
            {
              if ( pOpenAndPos->fSpellcheck )
              {
                PTBSEGMENT pTBSeg;                  //ptr to segment struct
                ULONG ulStartSeg = 1;
                ULONG ulSeg = 1;


                fStartSpellCheck = TRUE;
                pstEQFGen->pszCurSpellCheckDoc = pOpenAndPos->pszDocumentList;
                pstEQFGen->pszSpellCheckDocList = pOpenAndPos->pszDocumentList;
                pstEQFGen->fLoadedBySpellcheck = FALSE;

                // start at default screen position
                pstEQFGen->xSpellChecklDlg = 0;
                pstEQFGen->ySpellChecklDlg = 0;

                // position to first translated segment
                pTBSeg = EQFBGetVisSeg( pDocument, &ulSeg );  
                while ( pTBSeg && (pTBSeg->qStatus != QF_XLATED) && (pTBSeg->ulSegNum != ulStartSeg))
                {
                  ulSeg++;
                  pTBSeg = EQFBGetVisSeg( pDocument, &ulSeg);
                } /* endwhile */

                fOk = EQFBSendNextSource( pDocument, &ulSeg, FALSE, POS_CURSOR ); 
                if ( fOk )
                {
                  EQFBActivateSegm( pDocument, ulSeg );
                } /* endif */
              }
              else if ( pOpenAndPos->ulSeg )
              {
                fOk = EQFBSendNextSource( pDocument,   // pointer to document
                                &(pOpenAndPos->ulSeg),   // ptr to new segment
                                          TRUE,        // foreground mode
                                          POS_CURSOR); // position at cursor

                if ( fOk )
                {
                  EQFBActivateSegm( pDocument, pOpenAndPos->ulSeg );
                  EQFBGotoSeg( pDocument,
                                pOpenAndPos->ulSeg,
                                pOpenAndPos->usOffs);
                  EQFBSetFindData( pOpenAndPos->chFind );
                } /* endif */
              }
              else if ( pOpenAndPos->ulLine )
              {
                ULONG ulSeg;
                EQFBFindLine( pDocument, pOpenAndPos->ulLine );
                ulSeg = pDocument->TBCursor.ulSegNum;
                EQFBSendNextSource( pDocument, &ulSeg, TRUE, POS_CURSOR);
                EQFBActivateSegm( pDocument, ulSeg);
              }
              else if ( pOpenAndPos->chFind[0] != 0 )
              {
                PFINDDATA pFindData = NULL;

                if ( UtlAlloc( (PVOID *)&pFindData, 0, sizeof(FINDDATA), ERROR_STORAGE ) )
                {
                  pFindData->ulSegNumBegin = 1;
                  pFindData->usSegOffsetBegin = 0;
                  pFindData->fChange = FALSE;
                  wcscpy( pFindData->chFind, pOpenAndPos->chFind );
                  EQFBFindMatch( pDocument, pFindData );
                  UtlAlloc( (PVOID *)&pFindData, 0, 0, NOMSG );
                } /* endif */
              }
              else
              {
                EQFBTSeg ( pDocument );          // select first segment
              } /* endif */
              UtlAlloc(&pstEQFGen->pOpenAndPos, 0L, 0L, NOMSG );
            }
            else if ( pstEQFGen->fLoadedBySpellcheck )
            {
              PTBSEGMENT pTBSeg;                  //ptr to segment struct
              ULONG ulStartSeg = 1;
              ULONG ulSeg = 1;

              // position to first translated segment
              pTBSeg = EQFBGetVisSeg( pDocument, &ulSeg );  
              while ( pTBSeg && (pTBSeg->qStatus != QF_XLATED) && (pTBSeg->ulSegNum != ulStartSeg))
              {
                ulSeg++;
                pTBSeg = EQFBGetVisSeg( pDocument, &ulSeg);
              } /* endwhile */

              fOk = EQFBSendNextSource( pDocument, &ulSeg, FALSE, POS_CURSOR ); 
              if ( fOk )
              {
                EQFBActivateSegm( pDocument, ulSeg );
              } /* endif */
            }
            else
            {
              EQFBTSeg ( pDocument );          // select first segment
            } /* endif */
            TPLOG();
            pDocument = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT);

            if ( pDocument )
            {
              PTBDOCUMENT *ppDoc = (PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0]);
              *ppDoc = pDocument;
              EQFBScreenData( pDocument );     // force screen update
              EQFBScreenCursor( pDocument );   // position cursor and slider
              TPLOG();

              SendMessage( pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                             0, MP2FROMHWND( pDocument->hwndFrame ));
              TPLOG();

//@@              UtlDispatch();              // let PM process incoming requests
              TPLOG();
     #if defined(MEASURETIME)
        GETTIME( ulGetSeg );
     #endif

              memset( pEQFBBlockMark, 0, sizeof( EQFBBLOCK )); // init block


            }
            else
            {
              fOk = FALSE;
            } /* endif */
          }
          else
          {
            fOk = FALSE;
          } /* endif */


          TPLOG();

       } /* endif */
   } /* endif */


  if ( ! fOk )           // something went wrong -- get rid of TB environment
  {
     PTBDOCUMENT *ppDoc = (PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0]);
     *ppDoc = NULL;
     WinPostMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_SAVED, NULL, NULL );
  }
  else if (pDocAnchor)
  {
    /******************************************************************/
    /* we have already a document loaded -- activate the new one      */
    /******************************************************************/
     pDocAnchor->Redraw |= REDRAW_ALL;
  } /* endif */

  if (fOk && pTgtDoc && pstEQFGen->fLoadedBySpellcheck )
  {
    pTgtDoc->pvSpellData = pstEQFGen->pvSpellData;
  }

  if (fOk && pTgtDoc )
  {
    USEROPT* pEQFBUserOpt = get_EQFBUserOpt();


    if (pEQFBUserOpt->UserOptFlags.bAutoSpellCheck &&
        (pTgtDoc->docType==STARGET_DOC)
         && pTgtDoc->fSpellCheck   )
    {
      pTgtDoc->fAutoSpellCheck = FALSE;  // force set to TRUE
      EQFBFuncSpellAuto ( pTgtDoc );
    } /* endif */
  } /* endif */

  // start spellchecking when called with special open-and-pos spellcheck request or document has been opened by spellchecker
  if ( fOk && pTgtDoc )
  {
    if ( fStartSpellCheck )
    {
      SendMessage( pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS, 0, MP2FROMHWND( pDocument->hwndClient ));
      PostMessage( pDocument->hwndClient, WM_COMMAND, MAKELONG( IDM_PROOFALL, 0), 0L );
    }
    else if ( pstEQFGen->fLoadedBySpellcheck )
    {
      SendMessage( pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS, 0, MP2FROMHWND( pDocument->hwndClient ));
      pstEQFGen->fLoadedBySpellcheck = FALSE; 
      pstEQFGen->pNewSpellCheckDoc = pDocument;
      //PostMessage( pDocument->hwndClient, WM_COMMAND, MAKELONG( IDM_PROOFALL, 0), 0L );
    } /* endif */
  } /* endif */

  return ( fOk );
}/* endif */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncClose
//------------------------------------------------------------------------------
// Function call:     EQFBFuncClose(PSTEQFGEN)
//------------------------------------------------------------------------------
// Description:       close all files (save doc and stop editor)
//------------------------------------------------------------------------------
// Parameters:        PSTEQFGEN pstEQFGen   : pointer to generic editor struct
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - if saving is requested do the following:
//                       - save last source window and target window
//                           positions in generic structure
//                       - issue a file command for the target document
//                            ( EQFBFuncFile)
//                       - post a message to services that document
//                           is saved  (EQFM_DOC_IS_SAVED)
//------------------------------------------------------------------------------
VOID
EQFBFuncClose
(
   PSTEQFGEN pstEQFGen                          // generic editor structure                        //
)
{
   PTBDOCUMENT pDoc;                            // pointer to doc
   HWND     hwnd;                               // window handle
   SWP      swp;                                 // window position structure
   BOOL     fIconic = FALSE;


   /****************************************************************/
   /* get special settings for our TENV window                     */
   /****************************************************************/
   fIconic = IsIconic( pstEQFGen->hwndTWBS );
   /*******************************************************************/
   /* get last positions used for target window                       */
   /* Attention: Under Windows frame handle is equal to target handle */
   /*******************************************************************/
   hwnd = pstEQFGen->hwndEditorTgt;

   if (!fIconic && hwnd &&
        WinQueryWindowPos ( hwnd, &swp) && WinIsWindowVisible(hwnd) )
   {
     /*****************************************************************/
     /* if window maximized -- restore it...                          */
     /*****************************************************************/
     if ( SWP_FLAG(swp) & EQF_SWP_MAXIMIZE )
     {
       ShowWindow( hwnd, SW_RESTORE );
       WinQueryWindowPos ( hwnd, &swp );
     } /* endif */
      if ( ! (SWP_FLAG(swp) & ( EQF_SWP_MINIMIZE | EQF_SWP_MAXIMIZE )))
      {
         RECTL_XLEFT(pstEQFGen->rclEditorTgt)      = swp.x;
         RECTL_XRIGHT(pstEQFGen->rclEditorTgt)     = (swp.x + swp.cx);
         RECTL_YBOTTOM(pstEQFGen->rclEditorTgt)    = swp.y;
         RECTL_YTOP(pstEQFGen->rclEditorTgt)       = (swp.y + swp.cy);
      } /* endif */
   } /* endif */

   pDoc = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT );
   if ( pDoc )
   {

      /********************************************************/
      /* free memory allocated for DBCS input methods ...     */
      /********************************************************/
      if ( pDoc->hIMEDll )
      {
        DosFreeModule( pDoc->hIMEDll );
        pDoc->hIMEDll = 0;
      } /* endif */
      if ( pDoc->hIME )
      {
        GlobalFree(pDoc->hIME);
        pDoc->hIME = 0;
      } /* endif */
      if ( pDoc->hlfIME )
      {
        #ifdef WIN32BIT
          ImeMoveConvertWin( pDoc, pDoc->hwndClient, -1, -1);
        #endif
        GlobalFree(pDoc->hlfIME);
        pDoc->hlfIME = 0;
      } /* endif */

      /****************************************************************/
      /* close all open target documents                              */
      /****************************************************************/
      {
        PTBDOCUMENT pDocStart;

        while ( pDoc )
        {
          /************************************************************/
          /* find next target doc..                                   */
          /* Assumption: pDoc points to STARGET_DOC                   */
          /************************************************************/
          pDocStart = pDoc->next;
          while ( (pDocStart->docType != STARGET_DOC) && (pDocStart != pDoc) )
          {
            pDocStart = pDocStart->next;
          } /* endwhile */
          EQFBFuncFile( pDoc );                            // file starget document
          pDoc = ( pDocStart != pDoc ) ? pDocStart : NULL ;

        } /* endwhile */
      }
   } /* endif */

   /*******************************************************************/
   /* close segsource if save was okay, otherwise stay ....           */
   /* this is done via checking if target window still available ...  */
   /*******************************************************************/
   if ( !WinIsWindow((HAB) UtlQueryULong(QL_HAB), hwnd) )
   {
     /*****************************************************************/
     /* get frame handle of ssource doc and quit it if openend...     */
     /* -- under Windows the frame handle is equal to client handle   */
     /*****************************************************************/
     hwnd = pstEQFGen->hwndEditorSrc;

     if (!fIconic && hwnd &&
         WinIsWindowVisible( hwnd ) && WinQueryWindowPos ( hwnd, &swp))
     {
        /*****************************************************************/
        /* if window maximized -- restore it...                          */
        /*****************************************************************/
        if ( SWP_FLAG(swp) & EQF_SWP_MAXIMIZE )
        {
          ShowWindow( hwnd, SW_RESTORE );
          WinQueryWindowPos ( hwnd, &swp );
        } /* endif */
        if ( ! (SWP_FLAG(swp) & ( EQF_SWP_MINIMIZE | EQF_SWP_MAXIMIZE )) )
        {
           RECTL_XLEFT(pstEQFGen->rclEditorSrc)      = (LONG)swp.x;
           RECTL_XRIGHT(pstEQFGen->rclEditorSrc)     = (LONG)(swp.x + swp.cx);
           RECTL_YBOTTOM(pstEQFGen->rclEditorSrc)    = (LONG)swp.y;
           RECTL_YTOP(pstEQFGen->rclEditorSrc)       = (LONG)(swp.y + swp.cy);
        } /* endif */
     } /* endif */

     pDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
     if ( pDoc )
     {
       while ( pDoc )
       {
         pDoc = EQFBFuncQuit( pDoc );              // quit document
       } /* endwhile */
     } /* endif */
                                                  // inform caller about success
     pstEQFGen->hwndEditorSrc = (HWND) NULL;   // reset handles
     pstEQFGen->hwndEditorTgt = (HWND) NULL;
     WinPostMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_SAVED, NULL, NULL );
     *((PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0])) = NULL;

// EQFBFreeTables();                          // free space allocated for tbl

   #if defined(MEASURETIME)
   {
      FILE  * hTimeLog;

      hTimeLog = fopen( "\\TIME.LOG", "a" );
      fprintf( hTimeLog, "==========================================\n");
      fprintf( hTimeLog, "Init DBCS: %10ld ms\n", ulInitDBCS);
      fprintf( hTimeLog, "Load Resource: %10ld ms\n", ulLoadResource);
      fprintf( hTimeLog, "Load Files: %10ld ms\n", ulLoadFile);
      fprintf( hTimeLog, "Get first Segment: %10ld ms\n", ulGetSeg);

      fclose( hTimeLog );
   }
   #endif

#if defined(TPLOGGING)
   if ( hTPLog )
   {
     fclose( hTPLog );
     hTPLog = NULL;
   } /* endif */
#endif

   {
     FILE* hMTLog = get_hMTLog();
     if ( hMTLog )
     {
       fclose( hMTLog );
     } /* endif */
   }
   
   } /* endif */

   return;
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncNextDoc
//------------------------------------------------------------------------------
// Function call:     EQFBFuncNextDoc(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       switch to next document
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc          pointer to doc instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     If there is more than 1 doc, toggle to next doc in ring
//------------------------------------------------------------------------------

 VOID EQFBFuncNextDoc
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
    pDoc;
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncPrevDoc
//------------------------------------------------------------------------------
// Function call:     EQFBFuncPrevDoc(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       switch too previous document
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     If there is more than 1 document in the ring,
//                    toggle to previous document
//------------------------------------------------------------------------------

 VOID EQFBFuncPrevDoc
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {
   /* No need to bother if there is only one document */
   if (pDoc->prev != pDoc)
   {
      pDoc = pDoc->prev;
      pDoc->Redraw |= REDRAW_ALL;
                                          // activate document window
      WinShowWindow( pDoc->hwndFrame, TRUE );
      SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
                     0, MP2FROMHWND( pDoc->hwndFrame ));
   }
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDoQuit
//------------------------------------------------------------------------------
// Function call:     EQFBFUncDoQuit(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       quit the current doc
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     switch (doctype)
//                     case source doc:
//                       make window invisible
//                     case target doc:
//                       beep; quit is not allowed
//                     case other document:
//                       call EQFBFuncQuit (quit doc)
//------------------------------------------------------------------------------

 VOID EQFBFuncDoQuit
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   switch ( pDoc->docType )
   {
      case SSOURCE_DOC:
         WinShowWindow( pDoc->hwndFrame , FALSE );
         SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
                        0, MP2FROMHWND( pDoc->twin->hwndFrame ));
         break;
      case STARGET_DOC:
         EQFBFuncClose( (PSTEQFGEN)pDoc->pstEQFGen );          // same as IDM_QUIT
//         EQFBFuncNothing( pDoc );                 // not allowed for target docs
         break;
      case OTHER_DOC:
         pDoc = EQFBFuncQuit( pDoc );             // quit document
         break;
      case TRNOTE_DOC:
         pDoc = EQFBFuncQuit(pDoc);              // quit document
         break;
   } /* endswitch */
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncQuit
//------------------------------------------------------------------------------
// Function call:     EQFBFuncQuit(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       quit the current document
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    pointer to doc instance
//------------------------------------------------------------------------------
// Returncode type:   PTBDOCUMENT
//------------------------------------------------------------------------------
// Function flow:     ensure current line changes are noted
//                    issure a warning if document has changed
//                    quit current document
//                    if there are more docs in ring, toggle to next
//------------------------------------------------------------------------------

 PTBDOCUMENT EQFBFuncQuit
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
    USHORT  usResult;                     // return value from function
    PSZ     pData;                        // pointer to error data

   /* Ensure current line changes are noted */
   EQFBWorkSegOut( pDoc );

   /* Issue a warning message if the document has changed */
   if (pDoc->flags.changed)
   {
    pData = pDoc->szDocName; // get document name

      usResult = UtlError( TB_QUIT, MB_YESNO, 1, &pData, EQF_QUERY);
      if ( usResult == MBID_NO )
      {
        /* get segment in again */
         EQFBWorkSegIn( pDoc );

         return( pDoc );
      } /* endif */
   }

   /* Throw away the current document */
   pDoc = EQFBDocDelete( pDoc );
   /* If there are still documents left */
   if (pDoc != NULL)
   {
      pDoc->Redraw |= REDRAW_ALL;
   }
   return( pDoc );
 }

 //------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncReImportDoc
//------------------------------------------------------------------------------
// Function call:     EQFBFuncReImportDoc(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       quit the current document
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    pointer to doc instance
//------------------------------------------------------------------------------
// Returncode type:   PTBDOCUMENT
//------------------------------------------------------------------------------
// Function flow:     ensure current line changes are noted
//                    issure a warning if document has changed
//                    quit current document
//                    if there are more docs in ring, toggle to next
//------------------------------------------------------------------------------

 void EQFBFuncReImportDoc
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
    USHORT  usResult;                     // return value from function
    PSZ     pData;                        // pointer to error data
    PVOID   pvReImportHandle = NULL;      // handle of re-import document function

   /* Ensure current line changes are noted */
   EQFBWorkSegOut( pDoc );

   /* Issue a warning message if the document has changed */
   if (pDoc->flags.changed)
   {
    pData = pDoc->szDocName; // get document name

      usResult = UtlError( TB_QUIT, MB_YESNO, 1, &pData, EQF_QUERY);
      if ( usResult == MBID_NO )
      {
        /* get segment in again */
         EQFBWorkSegIn( pDoc );

         return;
      } /* endif */
   }

   // prepare re-import of document
   ReImport_Prepare( pDoc->szDocName, pDoc->ulWorkSeg, &pvReImportHandle );

   // shut down editor
   EQFBFuncClose( (PSTEQFGEN)pDoc->pstEQFGen );

   // start re-import of document
   ReImport_Start( pvReImportHandle );

   return;
 }



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncSave
//------------------------------------------------------------------------------
// Function call:     EQFBFuncSave(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       save current document
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    pointer to doc instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFBDocSave with name of current document
//------------------------------------------------------------------------------

 VOID EQFBFuncSave
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   EQFBDocSave( pDoc, pDoc->szDocName, FALSE );

   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDoFile
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDoFile(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       save current doc through shut down of editor
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    pointer to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if current doc is target
//                      issue a shut down request
//                    else
//                      beep
//                    endif
//------------------------------------------------------------------------------

 VOID EQFBFuncDoFile
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   /*******************************************************************/
   /* if in other-doc or source doc, disable function (beep)          */
   /*******************************************************************/
   if ( pDoc && pDoc->docType == STARGET_DOC )
   {
      EQFBFuncClose( (PSTEQFGEN)pDoc->pstEQFGen );     // shut down editor

   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if nothing deleted
   } /* endif */
    return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncFile
//------------------------------------------------------------------------------
// Function call:     EQFBFuncFile(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       file the current document
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   pointer to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     write out worksegment
//                    save current document
//                    if save ok
//                       quit current document
//                    endif
//                    if there is still a document
//                      read in worksegment
//------------------------------------------------------------------------------

 VOID EQFBFuncFile
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   EQFBWorkLineOut( pDoc );
   if (EQFBDocSave(pDoc, pDoc->szDocName, TRUE) >= 0)
   {
      pDoc->flags.changed = FALSE;        // no changes in doc
      pDoc->EQFBFlags.workchng = FALSE;   //    and work segment

      if (pDoc->hwndRichEdit)
      {
        SendMessage( pDoc->hwndRichEdit, EM_SETMODIFY, 0, 0L );
      }
      pDoc = EQFBFuncQuit( pDoc );
   } /* endif */
   if (pDoc)
       EQFBWorkLineIn( pDoc );
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncTopDoc - move to the top of the document
//------------------------------------------------------------------------------
// Function call:     EQFBFuncTopDoc ( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move to the top of the document
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set TBCursor to 1st segment, offset 0
//                    set Cursorrow to  line 0
//                    reset TBRow table
//                    call EQFBDBCS2ND
//                    call EQFBWorkSegCheck
//
//------------------------------------------------------------------------------

 VOID EQFBFuncTopDoc
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {

   (pDoc->TBCursor).ulSegNum = 1;            // cursor row
   (pDoc->TBCursor).usSegOffset = 0;         // cursor offset
   pDoc->lCursorRow = 0;                    // set cursor row on top of screen
   pDoc->ulVScroll = 1;
   if ( !pDoc->hwndRichEdit )
   {
    EQFBScrnLinesFromSeg ( pDoc,              // pointer to doc ida
                          pDoc->lCursorRow,  // starting row
                          pDoc->lScrnRows,   // number of rows
                          &(pDoc->TBCursor));// starting segment
    pDoc->lDBCSCursorCol = pDoc->lCursorCol;
    EQFBDBCS2ND(pDoc,FALSE);                  //correct to the left

    pDoc->Redraw |= REDRAW_ALL;
   }
   EQFBWorkSegCheck( pDoc );                 //check if segment changed
   return;
 }
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncBottomDoc  - move to the end of the document
//------------------------------------------------------------------------------
// Function call:     EQFBFuncBottomDoc(PTBDOCUMENT);
//------------------------------------------------------------------------------
// Description:       move cursor to the end of the document
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set Cursorrow
//                    set TBCursor
//                    reset TBRow table
//                    scroll up if we moved too far duw to orientation on
//                     last screen row
//                    adjust if cursor is on 2nd DBCS byte
//                    call EQFBWorkSegCheck
//------------------------------------------------------------------------------

 VOID EQFBFuncBottomDoc
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {
    ULONG       ulRow;                         // current row
    PTBSEGMENT  pSeg;                     // pointer to segment
    ULONG  ulSeg;                         // segment number
    ULONG       ulActLine;                // active cursor line

   pDoc->lCursorRow = pDoc->lScrnRows-1;
   ulRow = pDoc->lScrnRows;

   // check if requested segment is visible (i.e. not part of a joined one
   // if so pSeg will be Null and you have to scan backward.
   ulSeg = pDoc->ulMaxSeg - 1;
   pSeg = EQFBGetVisSeg( pDoc, &ulSeg );    // check if segment is visible
   if ( !pSeg )
   {
      ulSeg = pDoc->ulMaxSeg - 1;
      pSeg = EQFBGetPrevVisSeg( pDoc, &ulSeg ); // find last visible segment
   } /* endif */

   (pDoc->TBCursor).ulSegNum = ulSeg;                      // cursor row
   (pDoc->TBCursor).usSegOffset = (pSeg->usLength>1) ? (pSeg->usLength-2) : 0;
                                                   // init TBRow
   memset( pDoc->TBRowOffset, 0 , sizeof(TBROWOFFSET) * MAX_SCREENLINES );
                                                   // set last screen TBRowOff

   memcpy((pDoc->TBRowOffset + ulRow),&(pDoc->TBCursor),
             sizeof( TBROWOFFSET));

   EQFBFillPrevTBRow ( pDoc,                             // pointer to doc ida
                       (ulRow - 1));             // starting row

   // scroll up as long as first row contains nothing
   while (( pDoc->TBRowOffset+1)->ulSegNum == 0 && pDoc->lCursorRow > 0)
   {
      memcpy( (pDoc->TBRowOffset+1), (pDoc->TBRowOffset+2),
              pDoc->lScrnRows * sizeof( TBROWOFFSET ));
      pDoc->lCursorRow --;
   } /* endwhile */

   pDoc->lDBCSCursorCol = pDoc->lCursorCol;
   EQFBDBCS2ND(pDoc,FALSE);                      //correct to the left

   ulActLine = EQFBQueryActLine(pDoc,
                                (pDoc->TBCursor).ulSegNum,
                                (pDoc->TBCursor).usSegOffset );
   pDoc->ulVScroll = ulActLine - (ULONG)(pDoc->lCursorRow);

   pDoc->Redraw |= REDRAW_ALL;
   EQFBWorkSegCheck( pDoc );                 //check if segment changed
   return;
 }

/*******************************************************************************
*
*       function        EQFBValidatePositions
*
* -----------------------------------------------------------------------------
*       check for valid size of window - if invalid use defines as
*       defined in Setup
*******************************************************************************/
VOID EQFBValidatePositions
(
   PRECTL  prcl,                    // rectangle to validate
   USHORT  usType                   // type of rectangle
)
{

   USHORT usCx;                     // horizontal size
   USHORT usCy;                     // vertiacal size
   USHORT usBd;                     // border

   usCx = (USHORT) WinQuerySysValue( HWND_DESKTOP, SV_CXFULLSCREEN );
   usCy = (USHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYFULLSCREEN );
   usBd = 2 * (USHORT) WinQuerySysValue( HWND_DESKTOP, SV_CXSIZEBORDER );

   if (!( ((PRECTL_XLEFT(prcl)+usBd)>= 0) && PRECTL_YBOTTOM(prcl) >= 0
        && PRECTL_XRIGHT(prcl) <= (SHORT)(usCx + usBd)
        && PRECTL_YTOP(prcl) <= (SHORT) usCy
        && PRECTL_XRIGHT(prcl) > PRECTL_XLEFT(prcl)
        && PRECTL_YTOP(prcl) > PRECTL_YBOTTOM(prcl)  ))
   {
      switch ( usType )
      {
        case SERVDICT_DOC:
           PRECTL_XLEFT(prcl)   = 0L;
           PRECTL_XRIGHT(prcl)  = (LONG)usCx;
           PRECTL_YBOTTOM(prcl) = 0L;
           PRECTL_YTOP(prcl)    = 100L;
           break;
        case SERVPROP_DOC:
           PRECTL_XLEFT(prcl)   = 0L;
           PRECTL_XRIGHT(prcl)  = (LONG)usCx;
           PRECTL_YBOTTOM(prcl) = 110L;
           PRECTL_YTOP(prcl)    = 220L;
           break;
        case SERVSOURCE_DOC:
           PRECTL_XLEFT(prcl)   = 50L;
           PRECTL_XRIGHT(prcl)  = (LONG)usCx;
           PRECTL_YBOTTOM(prcl) = 100L;
           PRECTL_YTOP(prcl)    = 200L;
           break;
        case STARGET_DOC:
           PRECTL_XLEFT(prcl)   = 0L;
           PRECTL_XRIGHT(prcl)  = (LONG)usCx;
           PRECTL_YBOTTOM(prcl) = 230L;
           PRECTL_YTOP(prcl)    = (LONG) (usCy);
           break;
        case SSOURCE_DOC:
           PRECTL_XLEFT(prcl)  = 0L;
           PRECTL_XRIGHT(prcl)  = (LONG)usCx;
           PRECTL_YBOTTOM(prcl) = 200L;
           PRECTL_YTOP(prcl)    = 380L;
        case OTHER_DOC:
        case TRNOTE_DOC:
        default:
           PRECTL_XLEFT(prcl)   = 0L;
           PRECTL_XRIGHT(prcl)  = (LONG) usCx;
           PRECTL_YBOTTOM(prcl) = 190L;
           PRECTL_YTOP(prcl)    = 370L;
           break;
      } /* endswitch */
   } /* endif */

}

/**********************************************************************/
/* enable for hot-spot conversion under DBCS                          */
/**********************************************************************/
  VOID ImeMoveConvertWin
  (
    PTBDOCUMENT pDoc,
    HWND    hwnd,
    SHORT   x,
    SHORT   y
  )
  {
    COMPOSITIONFORM CompForm;
    HIMC  hImc;

    pDoc;
    if ((x == -1) && (y == -1))
    {
      /*****************************************************************/
      /* get rid of all currently entered stuff in the ime             */
      /*****************************************************************/
      ClearIME( hwnd );
      CompForm.dwStyle = CFS_DEFAULT;
    }
    else
    {
      CompForm.dwStyle = CFS_POINT;
    }

    GetCaretPos( (LPPOINT) &CompForm.ptCurrentPos );
    hImc = ImmGetContext( hwnd );
    if ( hImc  )
    {
      ImmSetCompositionWindow( hImc, (COMPOSITIONFORM FAR*)&CompForm );
      ImmReleaseContext( hwnd, hImc );
    } /* endif */
    return;
  }


/**********************************************************************/
/* set the font of the IME control window to be our currently use     */
/* font                                                               */
/**********************************************************************/
  VOID ImeSetFont
  (
    PTBDOCUMENT pDoc,
    HWND       hwnd,
    PLOGFONT   pLogFont
  )
  {
    HIMC        hImc;
    if ( (  hImc = ImmGetContext( hwnd ) ) != NULL)
    {
      /******************************************************************/
      /* set the conversion font to be our usually used font...         */
      /******************************************************************/
      PLOGFONT plfIME = (PLOGFONT) GlobalLock( pDoc->hlfIME );
      memcpy( plfIME, pLogFont, sizeof( LOGFONT ));
      GlobalUnlock(pDoc->hlfIME);

      ImmSetCompositionFont( hImc, plfIME );
      ImmReleaseContext( hwnd, hImc );
    }
    return;
  }

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncActDoc
//------------------------------------------------------------------------------
// Function call:     EQFBFuncActDoc( pDoc );
//------------------------------------------------------------------------------
// Description:       activate the tranlation environment for the passed doc
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  -- ptr to doc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     select correct pDoc struct and call ActTransEnv
//------------------------------------------------------------------------------
VOID
EQFBFuncActDoc
(
  PTBDOCUMENT pDoc
)
{
  /********************************************************************/
  /* find next target doc in chain and activate it...                 */
  /********************************************************************/
  PTBDOCUMENT pDocStart = pDoc;

  while ( pDoc->docType != STARGET_DOC && pDoc->next != pDocStart )
  {
    pDoc = pDoc->next;
  } /* endwhile */
  if ( pDoc->docType == STARGET_DOC )
  {
    if ( !ActTransEnv( pDoc ))
    {
      EQFBFuncClose( (PSTEQFGEN)pDoc->pstEQFGen );
    }
    else
    {
      /****************************************************************/
      /* new document activated                                       */
      /****************************************************************/
    } /* endif */
  }
  else
  {
    EQFBFuncClose( (PSTEQFGEN)pDoc->pstEQFGen );
  } /* endif */
} /* end of function EQFBFuncActDoc */



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncRTFStart
//------------------------------------------------------------------------------
// Function call:     EQFBFuncRTFStart( PSZ,PSZ,PSTEQFGEN,BOOL, PTBDOCUMENT )
//------------------------------------------------------------------------------
// Description:       start the translation browser
//------------------------------------------------------------------------------
// Parameters:        PSZ           pSegSource  ,   //segmented Source file:
//                    PSZ           pSegTarget  ,   // segmented Target file:
//                    PSTEQFGEN     pstEQFGen   ,   // ptr to gen. edit struct
//                    BOOL          fReflow     ,   // reflow allowed ??
//                    PTBDOCUMENT   pDocAnchor      // document to anchor with
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       fOK   success indicator (True/FAlse)
//------------------------------------------------------------------------------
// Function flow:     - build load structure for loading source + target file
//                      (use the values passed with the generic structure
//                            for setting the window positions )
//                    - issue a load call to the editor ( EQFBDocLoad )
//                    - set pointer to generic strucuture in doc structure
//                    - do actionbar settings for SSource and target window
//                    - inform Troja that load was successful
//                    - select first segment and activate Services
//                              ( EQFBTSeg )
//                    - activate the target window
//                             ( special "SOTEC" necessity )
//
//------------------------------------------------------------------------------

BOOL
EQFBFuncRTFStart
(
   PSZ           pSegSource  ,   //segmented Source file:
   PSZ           pSegTarget  ,   // segmented Target file:
   PSTEQFGEN     pstEQFGen   ,   // pointer to generic edit structure
   BOOL          fReflow     ,   // reflow allowed ??
   PTBDOCUMENT   pDocAnchor      // document to anchor with
)
{
   BOOL        fOk = TRUE;               //error flag
   LOADSTRUCT  LoadStruct;               // load structure
   PTBDOCUMENT pDocument = NULL;         // pointer to document structure
   SHORT       sIDM;                     // for IDM_PROTECTED e.g.
   PTBDOCUMENT pTgtDoc = NULL;


   EQFBLoadResource ();                 // load all resources

   if ( fOk )
   {
  // build loadstructure to load ssource and starget
     if ( pstEQFGen->flEditTgtStyle )
     {
       LoadStruct.flFrameStyle = (pstEQFGen->flEditTgtStyle & AVAILSTYLESRTF) |
                                    FCF_SIZEBORDER | FCF_MAXBUTTON |
                                    FCF_DBE_APPSTAT;
     }
     else
     {
       LoadStruct.flFrameStyle = FCF_TITLEBAR   | FCF_MENU      | FCF_SYSMENU |
                                 FCF_SIZEBORDER | FCF_MAXBUTTON |
                                 FCF_DBE_APPSTAT; // allow DBCS status line
     } /* endif */
     LoadStruct.fsFlagStyle = EQF_SWP_MOVE | EQF_SWP_SIZE |
                              EQF_SWP_SHOW | EQF_SWP_ACTIVATE;
     if ( pDocAnchor )
     {
       // set size an position to same values as active window
       SWP swpFrame;
       WinQueryWindowPos( pDocAnchor->hwndFrame, &swpFrame );
       RECTL_XLEFT(LoadStruct.rclPos)   = (LONG) swpFrame.x;
       RECTL_XRIGHT(LoadStruct.rclPos)  = (LONG) (swpFrame.cx + swpFrame.x);
       RECTL_YTOP(LoadStruct.rclPos)    = (LONG) (swpFrame.cy + swpFrame.y);
       RECTL_YBOTTOM(LoadStruct.rclPos) = (LONG) swpFrame.y;

       // move window one titlebar height down
       RECTL_YTOP(LoadStruct.rclPos)    -= WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
       RECTL_YBOTTOM(LoadStruct.rclPos) -= WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
       EQFBValidatePositions( &LoadStruct.rclPos, STARGET_DOC );
     }
     else
     {
       LoadStruct.rclPos =  pstEQFGen->rclEditorTgt;
       EQFBValidatePositions( &LoadStruct.rclPos, STARGET_DOC );
     } /* endif */

     LoadStruct.fReadOnly = TRUE;
     LoadStruct.fReflow   = fReflow;
     LoadStruct.pFileName = pSegTarget;
     LoadStruct.docType = STARGET_DOC;         // set document type
     LoadStruct.pszTagTable = (PSZ)pstEQFGen->szTagTable;
     LoadStruct.pszEQFTagTable = (PSZ)pstEQFGen->szEQFTagTable;
     LoadStruct.pDoc  = pDocAnchor;            // document to anchor with
     LoadStruct.hwndParent = pstEQFGen->hwndTWBS;
     LoadStruct.pstEQFGen = pstEQFGen;
     LoadStruct.usEditor  = RTFEDIT_EDITOR;

     fOk = ( EQFBDocLoad ( &LoadStruct)  == 0 );

     if ( fOk )                            // load document and pass LoadStruct
     {
       pDocument = LoadStruct.pDoc;       // get pointer to loaded doc
       pDocument->pstEQFGen = pstEQFGen;         // store address to generic struct.

       EQFBSetWYSIWYGType( pDocument, LoadStruct.pszTagTable );

       {
         PDOCUMENT_IDA pIdaDoc = (PDOCUMENT_IDA)pstEQFGen->pDoc;
         if ( pIdaDoc )
         {
           if (!pIdaDoc->ulSrcOemCP)
           {
             pIdaDoc->ulSrcOemCP = GetLangOEMCP(pIdaDoc->szDocSourceLang);
           }
           //fOk = ( EQFBCntAllSrcWords( LoadStruct.pDoc,
           //                            (SHORT)pIdaDoc->usSrcLang,
           //                            pIdaDoc->ulSrcOemCP) == 0 );
         } /* endif */
       }
       switch ( pDocument->DispStyle)
       {
         case  DISP_PROTECTED:
           sIDM = IDM_PROTECTED;
           break;
         case  DISP_UNPROTECTED:
           sIDM = IDM_UNPROTECTED;
           break;
         case  DISP_HIDE:
           sIDM = IDM_HIDE;
           break;
         case  DISP_SHRINK:
           sIDM = IDM_SHRINK;
           break;
         case  DISP_COMPACT:
           sIDM = IDM_COMPACT;
           break;
         default :
           break;
       } /* endswitch */

       // set fForceEqualWhiteSpace flag if the file EQFANAWS
       // exists in the EQF\PROPERTY directory
       {
         CHAR szCheckMe[MAX_EQF_PATH];
         UtlMakeEQFPath( szCheckMe, NULC, PROPERTY_PATH, NULL );
         strcat( szCheckMe, "\\EQFANAWS" );
         pDocument->fForceEqualWhiteSpace = (EQF_BOOL)UtlFileExist( szCheckMe );
       }

       /************************************************************/
       /* check if SpellPullDown should be enabled                 */
       /************************************************************/
       pDocument->fSpellCheck = EQFBUtlSpell( pstEQFGen );
       pTgtDoc = pDocument;

       EnableMenuItem( GetMenu(pDocument->hwndFrame), IDM_SPELL_MENU,
                       pDocument->fSpellCheck ? (MF_BYPOSITION | MF_ENABLED) :
                                           (MF_BYPOSITION | MF_GRAYED) );
       pstEQFGen->hwndEditorTgt = pDocument->hwndClient; // save wnd handle
       {
         PTBDOCUMENT  pTempDoc;             // pointer to document structure
         pTempDoc = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT);
         if ( pTempDoc != NULL )
         {
           pDocument = pTempDoc;
         } /* endif */
       }

       /********************************************************/
       /* allow for DBCS input methods ...                     */
       /********************************************************/
       pDocument->hlfIME = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                                       (LONG)sizeof(LOGFONT));
       if ( pDocument )
       {
         LoadStruct.hwndParent = pstEQFGen->hwndTWBS;
         LoadStruct.fReadOnly = TRUE;    // mark every thing readonly
         LoadStruct.fReflow   = fReflow;
         LoadStruct.rclPos =  pstEQFGen->rclEditorSrc;
         EQFBValidatePositions( &LoadStruct.rclPos, SSOURCE_DOC );
         LoadStruct.fsFlagStyle = EQF_SWP_MOVE | EQF_SWP_SIZE | EQF_SWP_HIDE ;
         if ( pstEQFGen->flEditSrcStyle )
         {
           LoadStruct.flFrameStyle = (pstEQFGen->flEditSrcStyle & AVAILSTYLESRTF) |
                                      FCF_SIZEBORDER | FCF_MAXBUTTON;
         }
         else
         {
           LoadStruct.flFrameStyle = FCF_TITLEBAR | FCF_MENU | FCF_SYSMENU |
                                     FCF_SIZEBORDER | FCF_MAXBUTTON ;
         } /* endif */
         LoadStruct.fsFlagStyle = EQF_SWP_MOVE | EQF_SWP_SIZE;
         LoadStruct.pFileName = pSegSource;
         LoadStruct.docType = SSOURCE_DOC;
         LoadStruct.pstEQFGen = pstEQFGen;

         fOk = (EQFBDocLoad( &LoadStruct) == 0 ); // load target document
         if ( fOk )
         {
            if ( fOk )
            {
              PDOCUMENT_IDA pIdaDoc = (PDOCUMENT_IDA)pstEQFGen->pDoc;
              if ( pIdaDoc )
              {
                if ( !pIdaDoc->ulSrcOemCP)
                {
                  pIdaDoc->ulSrcOemCP = GetLangOEMCP(pIdaDoc->szDocSourceLang);
                }  
                fOk = ( EQFBCntAllSrcWords( LoadStruct.pDoc, pTgtDoc, 
                                            (SHORT)pIdaDoc->usSrcLang,
                                            pIdaDoc->ulSrcOemCP) == 0 );
              }
            }
         }
       }
       else
       {
          fOk = FALSE;
       } /* endif */
     } /* endif */
     if ( fOk )
     {
       ActivateMTLog( pSegSource, LoadStruct.pszTagTable );
                                           // use last used values later on
       LoadStruct.pDoc->DispStyle = DISP_PROTECTED;   // protect SGML tags
       LoadStruct.pDoc->pstEQFGen = pstEQFGen;  // store generic struct.
       pstEQFGen->hwndEditorSrc = LoadStruct.pDoc->hwndClient;

       // do actionbar settings for STarget
       LoadStruct.pDoc->twin = pDocument;   // store twin
       pDocument->twin =  LoadStruct.pDoc;

       // inform Troja that load was successful
       WinPostMsg( LoadStruct.hwndParent, EQFM_DOC_IS_LOADED, NULL, NULL);

       if ( pDocument )
       {
         EQFBDispFileRTF( pDocument );
       } /* endif */
       /***************************************************************/
       /* Delayed loading of original document                        */
       /***************************************************************/
//     if ( LoadStruct.pDoc->hwndRichEdit )
//     {
//       EQFBDispFileRTF( LoadStruct.pDoc );
//     }
       if ( WinIsWindow( (HAB) UtlQueryULong( QL_HAB ),
                           pDocument->hwndClient ))

       {
         PTBDOCUMENT pTempDoc;
         POPENANDPOS pOpenAndPos = (POPENANDPOS) pstEQFGen->pOpenAndPos;
         pTempDoc = pDocument;
         do
         {
           pTempDoc->fTransEnvAct = FALSE;
           pTempDoc = pTempDoc->next;
         } while ( pTempDoc != pDocument ); /* enddo */
         pDocument->fTransEnvAct = TRUE;
         /*********************************************************/
         /* position at a specific segment or at the current one  */
         /*********************************************************/
         if ( pOpenAndPos != NULL )
         {
           if ( pOpenAndPos->ulSeg )
           {
            fOk = EQFBSendNextSource( pDocument,   // pointer to document
                              &(pOpenAndPos->ulSeg), // ptr to new segment
                                      TRUE,        // foreground mode
                                      POS_CURSOR); // position at cursor

            if ( fOk )
            {
                EQFBActivateSegm( pDocument, pOpenAndPos->ulSeg );
                EQFBGotoSegRTF( pDocument,
                                pOpenAndPos->ulSeg,
                                pOpenAndPos->usOffs);
                EQFBSetFindData( pOpenAndPos->chFind );
            } /* endif */
           }
           else if ( pOpenAndPos->ulLine )
           {
             EQFBFindLine( pDocument, pOpenAndPos->ulLine );
           }
           else
           {
             EQFBTransRTF( pDocument, POS_TOBEORDONE ); // position at untranslated ones
           } /* endif */
           UtlAlloc(&pstEQFGen->pOpenAndPos, 0L, 0L, NOMSG );
         }
         else
         {
           EQFBTransRTF( pDocument, POS_TOBEORDONE ); // position at untranslated ones
         } /* endif */
         pDocument = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT);

         if ( pDocument )
         {
           EQFBBLOCK* pEQFBBlockMark = get_EQFBBlockMark();
           PTBDOCUMENT *ppDoc = (PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0]);
           *ppDoc = pDocument;

           SendMessage( pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                        0, MP2FROMHWND( pDocument->hwndFrame ));

           memset( pEQFBBlockMark, 0, sizeof( EQFBBLOCK )); // init block
         }
         else
         {
           fOk = FALSE;
         } /* endif */
       }
       else
       {
         fOk = FALSE;
       } /* endif */
     } /* endif */
   } /* endif */

   if ( ! fOk )           // something went wrong -- get rid of TB environment
   {
      PTBDOCUMENT *ppDoc = (PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0]);
      *ppDoc = NULL;
      WinPostMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_SAVED, NULL, NULL );
   } /* endif */

   if (fOk && pTgtDoc )
   {
     USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
     if (pEQFBUserOpt->UserOptFlags.bAutoSpellCheck &&
         (pTgtDoc->docType==STARGET_DOC)
          && pTgtDoc->fSpellCheck   )
     {
       pTgtDoc->fAutoSpellCheck = FALSE;  // force set to TRUE
       EQFBFuncSpellAuto ( pTgtDoc );
     } /* endif */
   } /* endif */
  return ( fOk );
}/* endif */
