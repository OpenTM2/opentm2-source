//+----------------------------------------------------------------------------+
//|EQFITMD.C                                                                   |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|  Regine Jornitz                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  contains the window procedure for the ITM dialog                          |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_MORPH
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_EDITORAPI        // editor API
#include <eqf.h>                  // General Translation Manager include file
#include "SHLOBJ.H"               // folder browse function

#include "core\memory\MemoryFactory.h"
#include "EQFITMD.ID"             // EQFITM ids
#include "EQFITM.H"

MRESULT EQFBITMCommand( HWND, WPARAM, LPARAM );
static MRESULT EQFBITMInit ( HWND ,WPARAM,LPARAM );
VOID EQFBITMFillListboxes( HWND, PITMDLGIDA,PSPECFILE );
MRESULT EQFITMChar( HWND, WPARAM, LPARAM );
MRESULT EQFBITMControl( HWND, SHORT, SHORT );
static BOOL  EQFBAdjToDir( HWND, PITMDLGIDA, PSPECFILE, SHORT );
static BOOL EQFBFillFileEF( HWND, PSPECFILE, SHORT );
static BOOL FillStringPool(HWND, PITMIDA ,PSZ **);
static BOOL ITMGetFileName(HWND, PITMDLGIDA , PSPECFILE );
static BOOL EQFBDelInList(HWND, PITMDLGIDA);
static USHORT EQFBQueryLineHeight( VOID );
static BOOL  ITMCheckPropDir (PITMDLGIDA, PITMIDA, HWND );
static BOOL  EQFBDelAll(HWND, PITMDLGIDA);
BOOL CreateTMIfNotExist ( HWND, PITMIDA, PSZ );
static void  AbbrevFileName(PSZ, LONG, HPS, PSZ);
static BOOL  EQFITMPrepForFillLB(PITMDLGIDA, PSPECFILE, PSZ);
static BOOL  EQFITMStartpathForFillLB(PITMDLGIDA, PSPECFILE, PSZ);
static MRESULT ITMDlgHandleResize( HWND hwndDlg, WPARAM mp1, LPARAM mp2 );

static USHORT usEQFBLineHeight = 0;    // static value for line heigth
static ULONG  ulEQFBLineWidth  = 0;    // static value for line width


// helper functions for resizing
HDWP ITMDlgSetNewSizePos( HWND hwnd, HDWP hdwp, int ID, int x, int y, int cx, int cy, PRECT pRectNew  );
HDWP ITMDlgAlignButtons( HWND hwnd, HDWP hdwp, int *piButtons, int x, int cx, int y  );
int ITMDlgGetYOffset( HWND hwnd, int iControl1, int iControl2 );
int ITMDlgGetXOffset( HWND hwnd, int iControl1, int iControl2 );
int ITMDlgGetVertDistance( HWND hwnd, int iControl1, int iControl2 );
int ITMDlgGetWidth( HWND hwnd, int iControl );
int ItmAddAllMatchingFiles( HWND hwndDlg, PITMDLGIDA pIda );
BOOL ItmDlgBrowse( HWND hwnd, PSZ pszBuffer, PSZ pszTitle );

#define ALIGN_LEFT_MODE     0x01
#define ALIGN_RIGHT_MODE    0x02
#define ALIGN_TOP_MODE      0x04
#define ALIGN_BOTTOM_MODE   0x08
HDWP ITMDlgPosAligned( HWND hwnd, HDWP hdwp, int iControlID, int iAlignID, int mode, int dx, int dy, PRECT pRectNew    );

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBITMDLGPROC                                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBITMDLGPROC via WinLoadDlg                            |
//+----------------------------------------------------------------------------+
//|Description:       allow init ITM with a dialog                             |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg    dialog handle                            |
//|                   USHORT  msg     message id                               |
//|                   MPARAM  mp1     message parameter 1                      |
//|                   MPARAM  mp2     message parameter 2                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     handle the following messages                            |
//|                     WM_INITDLG:                                            |
//|                       call the initial functions                           |
//|                     WM_COMMAND:                                            |
//|                       call the handle Command function                     |
//|                     WM_CONTROL:                                            |
//|                       call the control function                            |
//|                     WM_DRAWITEM:                                           |
//|                       draw filepair list                                   |
//|                     WM_CLOSE                                               |
//|                       Dismiss the dialog                                   |
//|                     default:                                               |
//|                       call the default dialog proc..                       |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK EQFBITMDLGPROC
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
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_ITM_MAIN_DLG, mp2 ); break;

      case WM_INITDLG:
          mResult = DIALOGINITRETURN( EQFBITMInit( hwndDlg, mp1, mp2 ));
          break;

    case WM_GETMINMAXINFO:
      {
        MINMAXINFO FAR *lpMinMax;      // ptr to min/max info structure
        PITMDLGIDA       pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);

        lpMinMax = (MINMAXINFO*) PVOIDFROMMP2(mp2);
        lpMinMax->ptMinTrackSize.x = pIda->iMinWidth;
        lpMinMax->ptMinTrackSize.y = pIda->iMinWidth;
      }
      break;

      case WM_SIZE:
        mResult = ITMDlgHandleResize( hwndDlg, mp1, mp2 );
        break;

      case WM_COMMAND:
         mResult = EQFBITMCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_MEASUREITEM:
         mResult = MRFROM2SHORT( EQFBQueryLineHeight(), 0 );
         break;

      case WM_DRAWITEM:
       {
         PITMDLGIDA       pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);
         PSZ              pszTgtFile;
         LPDRAWITEMSTRUCT lpDisp = (LPDRAWITEMSTRUCT)mp2;
         CHAR             szCurFile[ MAX_LONGPATH ];
         LONG             lMax;
         BOOL             fHighContrast = FALSE;
         DWORD   dwRGB_HIGHLIGHT = GetSysColor(COLOR_HIGHLIGHT);
		 DWORD   dwRGB_HIGHLIGHTTEXT = CLR_RED;
		 DWORD   dwRGB_WINDOW = CLR_WHITE;
         DWORD   dwRGB_WINDOWTEXT = CLR_BLACK;

         if ( (lpDisp->itemID != -1) )
         {
           if ( lpDisp->itemAction & (ODA_DRAWENTIRE | ODA_SELECT) )
           {
             SendMessage( lpDisp->hwndItem, LB_GETTEXT, lpDisp->itemID,
                          (LPARAM) (LPCSTR) pIda->szFile1 );
             pszTgtFile = strchr( pIda->szFile1, '\t' );
             *pszTgtFile++ = EOS;

             fHighContrast = UtlIsHighContrast();
             if (!fHighContrast)
			 {
                 FILLRECT( lpDisp->hDC, lpDisp->rcItem, (HBRUSH) (COLOR_WINDOW+1) );

                 ulEQFBLineWidth = (ULONG)(lpDisp->rcItem.right / 2);
				 lpDisp->rcItem.right -= (LOUSHORT(ulEQFBLineWidth)+ 10);
				 lMax = (lpDisp->rcItem.right - lpDisp->rcItem.left);
				 strcpy(szCurFile,pIda->szFile1);
				 AbbrevFileName(szCurFile, lMax,lpDisp->hDC,pIda->szFile1);
				 DRAWTEXT(lpDisp->hDC, szCurFile, lpDisp->rcItem, CLR_BLACK,
							CLR_WHITE, DT_LEFT);

			     lpDisp->rcItem.right += LOUSHORT(ulEQFBLineWidth) + 10;
				 lpDisp->rcItem.left += LOUSHORT(ulEQFBLineWidth) + 10;
				 lMax = (lpDisp->rcItem.right - lpDisp->rcItem.left);
				 strcpy(szCurFile,pszTgtFile);
				 AbbrevFileName(szCurFile, lMax,lpDisp->hDC,pszTgtFile);
		         DRAWTEXT(lpDisp->hDC, szCurFile, lpDisp->rcItem, CLR_BLACK,
							CLR_WHITE, DT_LEFT);
                 lpDisp->rcItem.left -= (LOUSHORT(ulEQFBLineWidth)+10);
                 if (  lpDisp->itemState & ODS_SELECTED  )
				 { // draw selected item
				    InvertRect( lpDisp->hDC, &lpDisp->rcItem );
		         }
 			 }
 			 else
             {
				 dwRGB_HIGHLIGHTTEXT = GetSysColor(COLOR_HIGHLIGHTTEXT);
				 dwRGB_HIGHLIGHT = GetSysColor(COLOR_HIGHLIGHT);
				 dwRGB_WINDOW = GetSysColor(COLOR_WINDOW);
				 dwRGB_WINDOWTEXT = GetSysColor(COLOR_WINDOWTEXT);

				 if ( !( lpDisp->itemState & ODS_SELECTED  ))
				 {
					 ulEQFBLineWidth = (ULONG)(lpDisp->rcItem.right / 2);
					 lpDisp->rcItem.right -= (LOUSHORT(ulEQFBLineWidth)+ 10);
					 lMax = (lpDisp->rcItem.right - lpDisp->rcItem.left);
					 strcpy(szCurFile,pIda->szFile1);
					 AbbrevFileName(szCurFile, lMax,lpDisp->hDC,pIda->szFile1);
					 DRAWTEXT(lpDisp->hDC, szCurFile, lpDisp->rcItem,
								 dwRGB_WINDOWTEXT ,
								 dwRGB_WINDOW, DT_LEFT);

					 lpDisp->rcItem.right += LOUSHORT(ulEQFBLineWidth) + 10;
					 lpDisp->rcItem.left += LOUSHORT(ulEQFBLineWidth) + 10;
					 lMax = (lpDisp->rcItem.right - lpDisp->rcItem.left);
					 strcpy(szCurFile,pszTgtFile);

					 AbbrevFileName(szCurFile, lMax,lpDisp->hDC,pszTgtFile);
					 DRAWTEXT(lpDisp->hDC, szCurFile, lpDisp->rcItem,
								 dwRGB_WINDOWTEXT ,
								 dwRGB_WINDOW, DT_LEFT);

					 lpDisp->rcItem.left -= (LOUSHORT(ulEQFBLineWidth)+10);
				 }
				 else /*if (  lpDisp->itemState & ODS_SELECTED  ) */
				 { // draw selected item
				     ulEQFBLineWidth = (ULONG)(lpDisp->rcItem.right / 2);
					 lpDisp->rcItem.right -= (LOUSHORT(ulEQFBLineWidth)+ 10);
					 lMax = (lpDisp->rcItem.right - lpDisp->rcItem.left);
					 strcpy(szCurFile,pIda->szFile1);
					 AbbrevFileName(szCurFile, lMax,lpDisp->hDC,pIda->szFile1);
					 DRAWTEXT(lpDisp->hDC, szCurFile, lpDisp->rcItem,
								 dwRGB_HIGHLIGHTTEXT ,
								 dwRGB_HIGHLIGHT, DT_LEFT);

					 lpDisp->rcItem.right += LOUSHORT(ulEQFBLineWidth) + 10;
					 lpDisp->rcItem.left += LOUSHORT(ulEQFBLineWidth) + 10;
					 lMax = (lpDisp->rcItem.right - lpDisp->rcItem.left);
					 strcpy(szCurFile,pszTgtFile);

					 AbbrevFileName(szCurFile, lMax,lpDisp->hDC,pszTgtFile);
					 DRAWTEXT(lpDisp->hDC, szCurFile, lpDisp->rcItem,
								 dwRGB_HIGHLIGHTTEXT ,
								 dwRGB_HIGHLIGHT, DT_LEFT);

					 lpDisp->rcItem.left -= (LOUSHORT(ulEQFBLineWidth)+10);
				 } /* endif */
		      } /* endif */
           } /* endif */
           /**********************************************************/
           /* item gets or loses the focus ...                       */
           /**********************************************************/
           if (  lpDisp->itemAction & ODA_FOCUS  )
           {
             DrawFocusRect( lpDisp->hDC, &lpDisp->rcItem );
           } /* endif */
         } /* endif */
         mResult = MRFROMSHORT( TRUE );
      }
         break;

       case WM_VKEYTOITEM:
         if ( LOWORD( mp1 ) == VK_DELETE )
         {
           if ( GETFOCUS() == GetDlgItem( hwndDlg, ID_ITM_FILELIST_LB ))
           {
             PITMDLGIDA  pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);
             //del selected entry in filelist
             EQFBDelInList(hwndDlg, pIda);
           } /* endif */
         } /* endif */
         mResult = -1;
         break;

       case DM_GETDEFID:
         mResult = EQFITMChar( hwndDlg, mp1, mp2 );
         break;

      case WM_EQF_CLOSE:
         //--- get rid off dialog ---
         // mp1 contains success indicator
         //     TRUE: user selected align button
         //     FALSE: user pressed ESC or Cancel button...
         {
           PITMDLGIDA pIda;

           pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);
           if ( pIda )
           {
              SWP   swp;
              CHAR szNumber[10];
              WinQueryWindowPos( hwndDlg, &swp );

              itoa( swp.x, szNumber, 10 );
              WriteProfileString( APPL_Name, "VISITM_x", szNumber );
              itoa( swp.y, szNumber, 10 );
              WriteProfileString( APPL_Name, "VISITM_y", szNumber  );
              itoa( swp.cx, szNumber, 10 );
              WriteProfileString( APPL_Name, "VISITM_cx", szNumber );
              itoa( swp.cy, szNumber, 10 );
              WriteProfileString( APPL_Name, "VISITM_cy", szNumber );

             UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
           } /* endif */
           DISMISSDLG( hwndDlg, SHORT1FROMMP1(mp1) );
         }
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */
   return (mResult);
} /* end of EQFBITMDLGPROC */

static VOID
 AbbrevFileName
 (
    PSZ      pszAbbrFile,
    LONG     lMax,
    HPS      hpsLB,
    PSZ      pszInFile
 )
{
    LONG             lCx = 0L;
    LONG             lCy = 0L;
    LONG             lMaxLen;
    CHAR             chStop;
    PSZ              pszEnd;
    LONG             lLen;
    LONG             lIndex = 0;
    LONG             lBack = 0;

    TEXTSIZE(hpsLB, pszInFile, lCx, lCy);

    if (lCx > lMax )
    {
      lMaxLen = strlen(pszInFile);
      while (lIndex<12 && (lBack < 2))
      {
        if (*(pszInFile+lIndex ) == BACKSLASH )
        {
          lBack ++;
        } /* endif */
        lIndex++;
      } /* endwhile */
      chStop = pszInFile[lIndex];
      pszInFile[lIndex] = EOS;
      lLen = 17;
      do
      {
        lLen +=3;
        pszEnd = pszInFile + lLen;
        sprintf( pszAbbrFile, "%s...%s", pszInFile,pszEnd);

        TEXTSIZE(hpsLB, pszAbbrFile, lCx, lCy);
      } while ((lCx > lMax) && (lLen < lMaxLen));

      pszInFile[lIndex] = chStop;

    } /* endif */
   return ;
} /* end of AbbrevFileName */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBITMCommand                                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBITMCommand( hwndDlg, mp1, mp2);                      |
//+----------------------------------------------------------------------------+
//|Description:       get the settings and write it down, or ignore changes.   |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg   handle of the dialog                      |
//|                   MPARAM  mp1    message parameter 1                       |
//|                   MPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:      switch ( kind of pushbutton)                            |
//|                     case Add:get src + tgt filenames                       |
//|                              fill filepair in filepairlist                 |
//|                     case DelInList: delete selected entry in filelist      |
//|                     case Visual:                                           |
//|                     case Align:                                            |
//|                     case Prepare:                                          |
//|                         set indicator what the user selected               |
//|                         fill string pool with all filenames in filepairlist|
//|                         check & delete duplicate filepairs                 |
//|                         fill ITMIda with all other entry fields            |
//|                         fill propertyfile structure                        |
//|                     case Cancel: write propertyfile                        |
//+----------------------------------------------------------------------------+

MRESULT EQFBITMCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT       mResult = MRFROMSHORT(TRUE);     // TRUE = cmd is processed
   BOOL          fOK = TRUE;
   PITMDLGIDA    pIda;
   SHORT         sItem;
   HWND          hwndLB;                                   // listbox handle
   PSPECFILE     pActFile;
   static CHAR   chText[MAX_LONGFILESPEC];
   USHORT        usI;
   PITMIDA       pITMIda;
   SHORT         sId = WMCOMMANDID( mp1, mp2 );
   LONG          lLen = 0;
   BOOL          fSrcStartPath = FALSE;
   BOOL          fTgtStartPath = FALSE;

   mp2;
   pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);
   pITMIda = pIda->pITMIda;

   switch ( sId )
   {  case ID_ITM_HELP_PB:
        mResult = UtlInvokeHelp();
        break;
      case ID_ITM_ADDALLTOLIST_PB:           // add all matching files
        ItmAddAllMatchingFiles( hwndDlg, pIda );
        break;
      case ID_ITM_ADDTOLIST_PB:           // add to filelist

       /***************************************************************/
       /* check whether EF is filled with or without fully qualified  */
       /* filename                                                    */
       /***************************************************************/
       pActFile= &(pIda->SrcFile);
       fOK = ITMGetFileName(hwndDlg, pIda, &(pIda->SrcFile));
       if ( fOK )
       {
          fOK = ITMGetFileName(hwndDlg, pIda, &(pIda->TgtFile));
       } /* endif */
       if ( fOK )
       {
         sprintf( pIda->szFile1, "  %s\t%s", pIda->SrcFile.szFileName,
                                     pIda->TgtFile.szFileName );
         // add to filelist
         hwndLB = GETHANDLEFROMID( hwndDlg, ID_ITM_FILELIST_LB );
         ENABLEUPDATEHWND_FALSE( hwndLB );
         sItem = INSERTITEMENDHWND( hwndLB, pIda->szFile1 );
         ENABLEUPDATEHWND_TRUE( hwndLB );
         ENABLECTRL( hwndDlg, ID_ITM_ALIGN_PB,  TRUE );
         ENABLECTRL( hwndDlg, ID_ITM_VISUAL_PB, TRUE );
         ENABLECTRL( hwndDlg, ID_ITM_DELALL_PB, TRUE );
         ENABLECTRL( hwndDlg, ID_ITM_PREPARE_PB, TRUE );
       } /* endif */
        break;
      case ID_ITM_DELINLIST_PB:           // delete from filelist
        fOK = EQFBDelInList(hwndDlg, pIda);
        break;
      case ID_ITM_DELALL_PB:             // delete all in filelist
        fOK = EQFBDelAll(hwndDlg, pIda);
        break;
      case ID_ITM_VISUAL_PB:               // add to filelist
      case ID_ITM_ALIGN_PB:               // add to filelist
      case ID_ITM_PREPARE_PB:               // add to filelist

        switch ( sId )
        {
          case  ID_ITM_VISUAL_PB:
            pITMIda->fVisual = TRUE;
            pITMIda->fPrepare = FALSE;
            break;
          case  ID_ITM_PREPARE_PB:
            pITMIda->fVisual = FALSE;
            pITMIda->fPrepare = TRUE;
            break;
          default :
            pITMIda->fVisual = FALSE;
            pITMIda->fPrepare = FALSE;
            break;
        } /* endswitch */
        if (fOK )
        {
          QUERYTEXT( hwndDlg, ID_ITM_SRCSTARTPATH_EF, pITMIda->szSrcStartPath);
          UtlStripBlanks(pITMIda->szSrcStartPath);
          lLen = strlen(pITMIda->szSrcStartPath);
          if ( lLen )
          {
            /**********************************************************/
            /* assure that startpath ends with a backslash            */
            /**********************************************************/
            if (pITMIda->szSrcStartPath[lLen-1] != BACKSLASH )
            {
              strcat(pITMIda->szSrcStartPath, BACKSLASH_STR);
            } /* endif */
            fSrcStartPath = TRUE;
          } /* endif */
        } /* endif */
        if (fOK )
        {
          QUERYTEXT( hwndDlg, ID_ITM_TGTSTARTPATH_EF, pITMIda->szTgtStartPath);
          UtlStripBlanks(pITMIda->szTgtStartPath);
          lLen = strlen(pITMIda->szTgtStartPath);
          if ( lLen )
          {
            /**********************************************************/
            /* assure that startpath ends with a backslash            */
            /**********************************************************/
            fTgtStartPath = TRUE;
            if (pITMIda->szTgtStartPath[lLen-1] != BACKSLASH )
            {
              strcat(pITMIda->szTgtStartPath, BACKSLASH_STR);
            } /* endif */
          } /* endif */
        } /* endif */
        /**************************************************************/
        /* check that either both or NO startpath is specified        */
        /**************************************************************/
        if (fTgtStartPath != fSrcStartPath )
        {
          ITMUtlError( pITMIda, ITM_STARTPATHNOTEQUAL, MB_CANCEL,
                               0, NULL, EQF_ERROR );
          fOK = FALSE;
        } /* endif */
        if (fOK )
        {
          fOK = FillStringPool(hwndDlg,pITMIda, &pIda->ppListIndex);
        } /* endif */
        if (fOK )
        {
          CheckDoubleFilePairs (&pIda->ppListIndex);
          pITMIda->fSGMLITM = FALSE;
        } /* endif */
        //call itm with listpairs

        if ( fOK )
        {
          QUERYTEXT( hwndDlg, ID_ITM_MARKUP_CB, chText );
          fOK = UtlCopyParameter(pITMIda->chTagTableName,
                                 chText,
                                 MAX_FNAME,
                                 TRUE);
        } /* endif */
        /**************************************************************/
        /* get selected source and target language                    */
        /**************************************************************/
        QUERYTEXT( hwndDlg, ID_ITM_SRCLANG_CB, pITMIda->szSourceLang );

        QUERYTEXT( hwndDlg, ID_ITM_TGTLANG_CB, pITMIda->szTargetLang );
        QUERYTEXT( hwndDlg, ID_ITM_TGTLANG_CB, pITMIda->szTargetInputLang );


        if ( fOK )
        {
          QUERYTEXT( hwndDlg, ID_ITM_TRANSMEM_CB, chText );
          fOK = CreateTMIfNotExist( hwndDlg, pITMIda, chText );
        } /* endif */
        if ( fOK )
        {
          strcpy( pITMIda->chLongTranslMemory, chText );
          fOK = UtlCopyParameter(pITMIda->chTranslMemory,
                                 chText,
                                 MAX_LONGFILESPEC,
                                 TRUE);
        } /* endif */
        if ( fOK )
        {
          usI = 0;
          while ( pIda->ppListIndex[usI] )
          {
            usI++;
          } /* endwhile */
          pIda->pITMIda->ppArgv = pIda->ppListIndex;
          pIda->pITMIda->usArgc = usI;
          fOK = FillPropFilePairs(pITMIda);
          if (fOK )
          {
            fOK = EQFITMPropFill ( pITMIda );
            if (fOK )
            {
              if ( pIda->SrcFile.szDirectory[0] == EOS  )
              {
               sprintf( pITMIda->pstPropItm->szSrcDirectory,  "%c:\\",
                        pIda->SrcFile.chDrive);
              }
              else
              {
                sprintf( pITMIda->pstPropItm->szSrcDirectory, "%c:\\%s",
                        pIda->SrcFile.chDrive, pIda->SrcFile.szDirectory);
              } /* endif */
              if ( pIda->TgtFile.szDirectory[0] == EOS  )
              {
               sprintf( pITMIda->pstPropItm->szTgtDirectory,  "%c:\\",
                        pIda->TgtFile.chDrive);
              }
              else
              {
                sprintf( pITMIda->pstPropItm->szTgtDirectory, "%c:\\%s",
                        pIda->TgtFile.chDrive, pIda->TgtFile.szDirectory);
              } /* endif */

            } /* endif */
          } /* endif */

          /************************************************************/
          /* check if language of source file same as of TM....       */
          /************************************************************/
          if ( fOK )
          {
            PSZ pTemp = pITMIda->szBuffer;
            MemoryFactory *pFactory = MemoryFactory::getInstance();
            OtmMemoryPlugin::PMEMORYINFO pInfo = (OtmMemoryPlugin::PMEMORYINFO)malloc( sizeof(OtmMemoryPlugin::MEMORYINFO) );

            if ( pFactory->getMemoryInfo( NULL, pITMIda->chTranslMemory, pInfo ) != 0 )
            {
              fOK = FALSE;
              ITMUtlError( pITMIda, ERROR_MEMORY_NOTFOUND, MB_CANCEL,
                           1, &pTemp, EQF_ERROR);
              SETFOCUS( hwndDlg, ID_ITM_TRANSMEM_CB );
            }
            else if ( stricmp( pInfo->szSourceLanguage, pITMIda->szSourceLang ) )
            {
              // get source language of the memory
              pFactory->getMemoryInfo( NULL, pITMIda->chTranslMemory, pInfo );

              /****************************************************************/
              /* selected source language does not match TM Source language   */
              /****************************************************************/
              PSZ pPtr[2];
              pPtr[0]= pInfo->szSourceLanguage;
              pPtr[1]= pITMIda->szSourceLang;
              fOK = FALSE;
              ITMUtlError( pITMIda, ERROR_TM_FILE_LANG_DIFFERENT, MB_CANCEL, 2, &pPtr[0], EQF_ERROR);
              SETFOCUS( hwndDlg, ID_ITM_TRANSMEM_CB );
            } /* endif */
            free( pInfo );
          } /* endif */

          if ( fOK )
          {
             if ( !isValidLanguage( pITMIda->szTargetLang, FALSE ) )
             {
               USHORT usMBID;
               PSZ    pTemp;
               pTemp = pITMIda->szTargetLang;
               usMBID = ITMUtlError( pITMIda, ITM_LING_SUPPORT_MISSING,
                                     MB_YESNO | MB_DEFBUTTON2,
                                     1, &pTemp, EQF_ERROR );
               if ( usMBID == MBID_YES )
               {
                 /*************************************************************/
                 /* if user wants to go on we use the source language         */
                 /*************************************************************/
                 strcpy( pITMIda->szTargetLang, pITMIda->szSourceLang );
               }
               else
               {
                 fOK = FALSE;
                 SETFOCUS( hwndDlg, ID_ITM_TGTLANG_CB );
               } /* endif */
             } /* endif */
          } /* endif */
        } /* endif */
        if ( fOK )
        {
          pITMIda->sMPerCent = -1;      // not yet avail. on dialog
          POSTEQFCLOSE( hwndDlg, TRUE );
        } /* endif */
        break;

      case ID_ITM_CANCEL_PB:
      case DID_CANCEL:
        /**************************************************************/
        /* write propfile is nec if alifile was deleted via del pb    */
        /**************************************************************/
        EQFITMPropWrite(pIda->pITMIda);
        POSTEQFCLOSE( hwndDlg, FALSE );
        break;
      case ID_ITM_SRCFILE_LB:
      case ID_ITM_TGTFILE_LB:
      case ID_ITM_SRCDIR_LB:
      case ID_ITM_TGTDIR_LB:
      case ID_ITM_FILELIST_LB:
          mResult = EQFBITMControl( hwndDlg, sId, WMCOMMANDCMD( mp1, mp2 ) );
          break;
      case ID_ITM_CURDIRSRC_BROWSE_PB:
      case ID_ITM_CURDIRTGT_BROWSE_PB:
        {
          PSPECFILE pActFile = NULL;
          PSZ pszTitle = NULL;
          BOOL fOK = TRUE;

          if ( sId == ID_ITM_CURDIRSRC_BROWSE_PB )
          {
            pActFile = &(pIda->SrcFile);
            pszTitle = "Select source directory";
          }
          else
          {
            pActFile = &(pIda->TgtFile);
            pszTitle = "Select target directory";
          } /* endif */

          QUERYTEXT( hwndDlg, pActFile->sIdCurDir, pIda->szCurDir );

          fOK = ItmDlgBrowse( hwndDlg, pIda->szCurDir, pszTitle );

          if ( fOK )
          {
            SETTEXT( hwndDlg, pActFile->sIdCurDir, pIda->szCurDir );
            EQFBAdjToDir( hwndDlg, pIda, pActFile, LN_ENTER );
          } /* endif */
        }
        break;
    } /* endswitch */

   return( mResult );
} /* end of EQFBITMCommand */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBITMInit                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBITMInit   ( hwndDlg );                               |
//+----------------------------------------------------------------------------+
//|Description:       do the initial settings                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND  hwndDlg     dialog handle                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     create and anchor dialog ida                             |
//|                   init fields of dialog ida                                |
//|                   if property file exists, fill fields with last-used      |
//|                   fill target and source directories &files listboxes      |
//|                   fill translation memory listbox                          |
//|                   fill markup language listbox                             |
//|                   if not ok post WM_CLOSE                                  |
//+----------------------------------------------------------------------------+
static MRESULT
EQFBITMInit
(
  HWND  hwndDlg,
  WPARAM mp1,
  LPARAM mp2
)
{
   BOOL        fOK = TRUE;                       // internal OK flag
   PITMDLGIDA  pIda;
   HWND        hwndCombo;
   SHORT       sItem;

   mp1 = mp1;                          // suppress 'unreferenced parameter' msg
   mp2 = mp2;

   /*******************************************************************/
   /* create and anchor IDA                                           */
   /*******************************************************************/
   fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(ITMDLGIDA), ERROR_STORAGE );
   if ( fOK )
   {
      fOK = ANCHORDLGIDA( hwndDlg, pIda );
      pIda->pITMIda = (PITMIDA) mp2;
   } /* endif */
   if ( fOK )
   {
     BOOL fFilled;    // indicator for property file used
      // no file name given; set default values
      pIda->SrcFile.chDrive = 'C';
      pIda->SrcFile.szDirectory[0] = EOS;
      pIda->SrcFile.szFileName[0] = EOS;
      pIda->SrcFile.szPath[0] = EOS;
      pIda->SrcFile.sIdDirLB = ID_ITM_SRCDIR_LB;
      pIda->SrcFile.sIdFileLB = ID_ITM_SRCFILE_LB;
      pIda->SrcFile.sIdFileEF = ID_ITM_SOURCE_EF;
      pIda->SrcFile.sIdCurDir = ID_ITM_CURDIRSRC_EF;
      strcpy( pIda->SrcFile.szFilePattern, "*.*" );
      pIda->TgtFile.chDrive = 'C';
      pIda->TgtFile.szDirectory[0] = EOS;
      pIda->TgtFile.szFileName[0] = EOS;
      pIda->TgtFile.szPath[0] = EOS;
      pIda->TgtFile.sIdDirLB = ID_ITM_TGTDIR_LB;
      pIda->TgtFile.sIdFileLB = ID_ITM_TGTFILE_LB;
      pIda->TgtFile.sIdFileEF = ID_ITM_TARGET_EF;
      pIda->TgtFile.sIdCurDir = ID_ITM_CURDIRTGT_EF;
      strcpy( pIda->TgtFile.szFilePattern, "*.*" );
      fFilled = ITMCheckPropDir (pIda, pIda->pITMIda,
                                WinWindowFromID(hwndDlg,ID_ITM_FILELIST_LB));
//    WinEnableWindowUpdate( hwndLB, TRUE );
      ENABLECTRL( hwndDlg, ID_ITM_DELINLIST_PB, FALSE );
      ENABLECTRL( hwndDlg, ID_ITM_DELALL_PB,  fFilled);
      ENABLECTRL( hwndDlg, ID_ITM_ALIGN_PB,  fFilled);
      ENABLECTRL( hwndDlg, ID_ITM_VISUAL_PB,  fFilled);
      ENABLECTRL( hwndDlg, ID_ITM_PREPARE_PB,  fFilled);
      ENABLECTRL( hwndDlg, ID_ITM_HELP_PB, TRUE);
      /****************************************************************/
      /* fill source and target directories and files listboxes       */
      /****************************************************************/
      EQFBITMFillListboxes(hwndDlg,pIda, &(pIda->SrcFile));
      EQFBITMFillListboxes(hwndDlg, pIda, &(pIda->TgtFile));
   //--- set text limits for entry fields and combo boxes ---
      SETTEXTLIMIT (hwndDlg, ID_ITM_SOURCE_EF, MAX_LONGPATH);
      SETTEXTLIMIT (hwndDlg, ID_ITM_TARGET_EF, MAX_LONGPATH);
      SETTEXTLIMIT (hwndDlg, ID_ITM_CURDIRSRC_EF, MAX_LONGPATH);
      SETTEXTLIMIT (hwndDlg, ID_ITM_CURDIRTGT_EF, MAX_LONGPATH);

   //--- set source and target start path
      SETTEXTLIMIT( hwndDlg, ID_ITM_SRCSTARTPATH_EF, MAX_LONGFILESPEC );
      if (pIda->pITMIda->szSrcStartPath[0] != EOS )
      {
        SETTEXT( hwndDlg, ID_ITM_SRCSTARTPATH_EF, pIda->pITMIda->szSrcStartPath );
        SETEFSEL( hwndDlg, ID_ITM_SRCSTARTPATH_EF, 0, MAX_LONGFILESPEC);
      }
      else if (pIda->pITMIda->pstPropItm &&
               (pIda->pITMIda->pstPropItm->szSrcStartPath[0] != EOS) )
      {
        SETTEXT( hwndDlg, ID_ITM_SRCSTARTPATH_EF, pIda->pITMIda->pstPropItm->szSrcStartPath );
        SETEFSEL( hwndDlg, ID_ITM_SRCSTARTPATH_EF, 0, MAX_LONGFILESPEC);
      } /* endif */
      SETTEXTLIMIT( hwndDlg, ID_ITM_TGTSTARTPATH_EF, MAX_LONGFILESPEC );
      if (pIda->pITMIda->szTgtStartPath[0] != EOS )
      {
        SETTEXT( hwndDlg, ID_ITM_TGTSTARTPATH_EF, pIda->pITMIda->szTgtStartPath );
        SETEFSEL( hwndDlg, ID_ITM_TGTSTARTPATH_EF, 0, MAX_LONGFILESPEC);
      }
      else if (pIda->pITMIda->pstPropItm &&
               (pIda->pITMIda->pstPropItm->szTgtStartPath[0] != EOS) )
      {
        SETTEXT( hwndDlg, ID_ITM_TGTSTARTPATH_EF, pIda->pITMIda->pstPropItm->szTgtStartPath );
        SETEFSEL( hwndDlg, ID_ITM_TGTSTARTPATH_EF, 0, MAX_LONGFILESPEC);
      } /* endif */

   // fill translation memory listbox
      hwndCombo = GETHANDLEFROMID( hwndDlg, ID_ITM_TRANSMEM_CB);

      MemFillCBNames( hwndCombo, pIda->pITMIda->szBuffer, "" );
      // add to memory drobdown combobox
      if ( (pIda->pITMIda->chTranslMemory[0])  )
      {
        //select the memory given in cmdline
        sItem = CBSEARCHITEMHWND( hwndCombo, pIda->pITMIda->chTranslMemory );
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else if (pIda->pITMIda->pstPropItm && pIda->pITMIda->pstPropItm->chLongTranslMemory[0] )
      {
        //select the memory given in propertyfile
        sItem = CBSEARCHITEMHWND( hwndCombo,
                                  pIda->pITMIda->pstPropItm->chLongTranslMemory );
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else if (pIda->pITMIda->pstPropItm && pIda->pITMIda->pstPropItm->chTranslMemory[0] )
      {
        //select the memory given in propertyfile
        sItem = CBSEARCHITEMHWND( hwndCombo,
                                  pIda->pITMIda->pstPropItm->chTranslMemory );
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else
      {
        CBSELECTITEMHWND( hwndCombo, 0 );
      } /* endif */
      CBSETTEXTLIMIT( hwndDlg, ID_ITM_TRANSMEM_CB, MAX_FNAME - 1 );

   // fill markup language listbox
      hwndCombo = GETHANDLEFROMID( hwndDlg, ID_ITM_MARKUP_CB);
      ENABLEUPDATEHWND_FALSE( hwndCombo );
      EqfSend2Handler( TAGTABLEHANDLER, WM_EQF_INSERTNAMES,
                       MP1FROMHWND(hwndCombo), NULLHANDLE );
      ENABLEUPDATEHWND_TRUE( hwndCombo );
      if ( (pIda->pITMIda->chTagTableName[0])  )
      {
        //select the markup given in cmdline
        sItem = CBSEARCHITEMHWND( hwndCombo, pIda->pITMIda->chTagTableName );
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else if ( pIda->pITMIda->pstPropItm && pIda->pITMIda->pstPropItm->chTagTable[0])
      {
        /**************************************************************/
        /* select markup given in property file                       */
        /**************************************************************/
        sItem = CBSEARCHITEMHWND( hwndCombo, pIda->pITMIda->pstPropItm->chTagTable);
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else
      {
         CBSELECTITEMHWND( hwndCombo, 0 );
      } /* endif */

      /****************************************************************/
      /* fill source and target languages ...                         */
      /****************************************************************/
      hwndCombo = GETHANDLEFROMID( hwndDlg, ID_ITM_SRCLANG_CB);
      ENABLEUPDATEHWND_FALSE( hwndCombo );
      UtlFillTableLB( hwndCombo, SOURCE_LANGUAGES );
      ENABLEUPDATEHWND_TRUE( hwndCombo );
      if (pIda->pITMIda->szSourceLang[0]  )
      {
        //select the source language given in cmdline
        sItem = CBSEARCHITEMHWND( hwndCombo, pIda->pITMIda->szSourceLang );
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else if (pIda->pITMIda->pstPropItm && pIda->pITMIda->pstPropItm->szSourceLang[0])
      {
        /**************************************************************/
        /* select srclng given in property file                       */
        /**************************************************************/
        sItem = CBSEARCHITEMHWND( hwndCombo,
                  pIda->pITMIda->pstPropItm->szSourceLang);
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else
      {
         CBSELECTITEMHWND( hwndCombo, 0 );
      } /* endif */

      hwndCombo = GETHANDLEFROMID( hwndDlg, ID_ITM_TGTLANG_CB);
      ENABLEUPDATEHWND_FALSE( hwndCombo );
      UtlFillTableLB( hwndCombo, ALL_TARGET_LANGUAGES );
      ENABLEUPDATEHWND_TRUE( hwndCombo );
      if ( pIda->pITMIda->szTargetLang[0]  )
      {
        //select the tgtlng given in cmdline
        sItem = CBSEARCHITEMHWND( hwndCombo, pIda->pITMIda->szTargetLang );
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else if (pIda->pITMIda->pstPropItm && pIda->pITMIda->pstPropItm->szTargetLang[0])
      {
        /**************************************************************/
        /* select tgtlng given in property file                       */
        /**************************************************************/
        sItem = CBSEARCHITEMHWND( hwndCombo,
                  pIda->pITMIda->pstPropItm->szTargetLang);
        CBSELECTITEMHWND( hwndCombo, max(sItem,0) );
      }
      else
      {
        CBSELECTITEMHWND( hwndCombo, 0 );
      } /* endif */

   } /* endif */


   // remember some dialog and dialog control positions and sizes for  later
   // resize request 
   if ( fOK )
   {
     RECT rectDlg; 

     GetWindowRect( hwndDlg, &rectDlg );

     // use original window size as minimum value
     pIda->iMinHeight = rectDlg.bottom - rectDlg.top;
     pIda->iMinWidth  = rectDlg.right - rectDlg.left;

     // get offsets and distance values for some of the controls
     pIda->iMarkupCBXOffset = ITMDlgGetXOffset( hwndDlg, ID_ITM_MARKUP_STATIC, ID_ITM_MARKUP_CB );
     pIda->iSrcStartpathXOffset = ITMDlgGetXOffset( hwndDlg, ID_ITM_SRCSTARTPATH_STATIC, ID_ITM_SRCSTARTPATH_EF );
     pIda->iTgtStartpathXOffset = ITMDlgGetXOffset( hwndDlg, ID_ITM_TGTSTARTPATH_STATIC, ID_ITM_TGTSTARTPATH_EF );

     pIda->iButtonArea1Space = ITMDlgGetVertDistance( hwndDlg, ID_ITM_SRCFILE_LB, ID_ITM_FILELIST_LB );
     pIda->iButtonArea1Offs = ITMDlgGetVertDistance( hwndDlg, ID_ITM_SRCFILE_LB, ID_ITM_ADDTOLIST_PB );

     pIda->iButtonArea2Offs = ITMDlgGetVertDistance( hwndDlg, ID_ITM_FILELIST_LB, ID_ITM_VISUAL_PB );
     pIda->iFileListOffs = ITMDlgGetYOffset( hwndDlg, ID_ITM_FILELIST_STATIC, ID_ITM_FILELIST_LB );
     pIda->iBrowsePBWidth = ITMDlgGetWidth( hwndDlg, ID_ITM_CURDIRSRC_BROWSE_PB );

     // get total heigth of fixed heigth dialog controls 
     {
       RECT rectFileList, rectSrcFiles;

       GetWindowRect( GetDlgItem( hwndDlg, ID_ITM_FILELIST_LB), &rectFileList );
       MapWindowPoints( HWND_DESKTOP, hwndDlg, (LPPOINT)&rectFileList, 2 );
       GetWindowRect( GetDlgItem( hwndDlg, ID_ITM_SRCFILE_LB), &rectSrcFiles );
       MapWindowPoints( HWND_DESKTOP, hwndDlg, (LPPOINT)&rectSrcFiles, 2 );
       pIda->iFixedAreaHeight = (rectDlg.bottom - rectDlg.top) - (rectFileList.bottom - rectFileList.top) -
                                (rectSrcFiles.bottom - rectSrcFiles.top);
       pIda->iButtonArea2Space = (rectDlg.bottom - rectDlg.top) - rectFileList.bottom ;
     }
   } /* endif */

   // restore original window size and position
   if ( fOK )
   {
     int x = GetProfileInt( APPL_Name, "VISITM_x", -1 );
     int y = GetProfileInt( APPL_Name, "VISITM_y", -1 );
     int cx = GetProfileInt( APPL_Name, "VISITM_cx", -1 );
     int cy = GetProfileInt( APPL_Name, "VISITM_cy", -1 );

     if ( (x != -1) && (y != -1) && (cx != -1) && (cy != -1) )
     {
       WinSetWindowPos( hwndDlg, HWND_TOP, x, y, cx, cy, EQF_SWP_MOVE | EQF_SWP_SIZE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
     } /* endif */
   } /* endif */

   if ( !fOK )
   {
     POSTEQFCLOSE( hwndDlg, FALSE );
   } /* endif */

   return ((MRESULT) !fOK);
} /* end of function EQFBITMInit */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMCheckPropDir                                          |
//+----------------------------------------------------------------------------+
//|Function call:     ITMCheckPropDir(pIda, pITMIda, hwndLB)                   |
//+----------------------------------------------------------------------------+
//|Description:       fill last-used values from propertyfile into dialog      |
//+----------------------------------------------------------------------------+
//|Parameters:        PITMDLGIDA     pIda,                                     |
//|                   PITMIDA        pITMIda                                   |
//|                   HWND           hwndLB                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE if lastused values exist for filepair list          |
//|                   FALSE   if filepairlist empty                            |
//+----------------------------------------------------------------------------+
//|Function flow:     if srcdirectory stored in propertyfile exists            |
//|                      set directory & path to lastused values               |
//|                   if tgt path stored in propertyfile exists                |
//|                      set directory & path to lastused values               |
//|                   add filepairs to filepairlistbox                         |
//+----------------------------------------------------------------------------+
static BOOL
ITMCheckPropDir
(
   PITMDLGIDA     pIda,
   PITMIDA        pITMIda,
   HWND           hwndLB
)
{
  PFILEPAIR       pFilePair;
  LONG            lIndex;
  BOOL            fFileListfilled = FALSE;
  SHORT           sItem;
  PFILEPOOL       pPool;
  PSZ             pszTemp = pITMIda->szBuffer;
  BOOL            fStartPath = FALSE;

/**********************************************************************/
/* check first whether directory exists currently                     */
/**********************************************************************/
  if (pITMIda->pstPropItm )
  {
    strcpy(pszTemp, pITMIda->pstPropItm->szSrcDirectory);
    lIndex = strlen(pszTemp);
    if ( lIndex )
    {
      pszTemp[lIndex - 1] = EOS;
    }
    else
    {
      pszTemp[0] = EOS;
    } /* endif */

    if ( UtlDirExist(pszTemp ))
    {
      if ( pITMIda->pstPropItm->szSrcDirectory[0] )
      {
        pIda->SrcFile.chDrive = pITMIda->pstPropItm->szSrcDirectory[0];
        strcpy(pIda->SrcFile.szDirectory, &(pITMIda->pstPropItm->szSrcDirectory[3]));
      } /* endif */
    } /* endif */

    strcpy(pszTemp, pITMIda->pstPropItm->szTgtDirectory);
    lIndex = strlen(pszTemp);
    if ( lIndex )
    {
      pszTemp[lIndex - 1] = EOS;
    }
    else
    {
      pszTemp[0] = EOS;
    } /* endif */
    if ( UtlDirExist(pszTemp) )
    {
      if ( pITMIda->pstPropItm->szTgtDirectory[0] )
      {
        pIda->TgtFile.chDrive = pITMIda->pstPropItm->szTgtDirectory[0];
        strcpy(pIda->TgtFile.szDirectory, &(pITMIda->pstPropItm->szTgtDirectory[3]));
      } /* endif */
    } /* endif */

    strcpy(pszTemp, pITMIda->pstPropItm->szSrcStartPath);
    lIndex = strlen(pszTemp);
    if ( lIndex )
    {
      if ( pszTemp[lIndex - 1] == '\\' )
      {
        pszTemp[lIndex - 1] = EOS;
      } /* endif */
    }
    else
    {
      pszTemp[0] = EOS;
    } /* endif */
    if ( UtlDirExist(pszTemp) )
    {
      if ( pITMIda->pstPropItm->szSrcStartPath[0] )
      {
        strcpy(pIda->pITMIda->szSrcStartPath, pITMIda->pstPropItm->szSrcStartPath);
        fStartPath = TRUE;
      } /* endif */
    } /* endif */
    if (fStartPath )
    {
      strcpy(pszTemp, pITMIda->pstPropItm->szTgtStartPath);
      lIndex = strlen(pszTemp);
      if ( lIndex )
      {
        if ( pszTemp[lIndex - 1] == '\\' )
        {
          pszTemp[lIndex - 1] = EOS;
        } /* endif */
      }
      else
      {
        pszTemp[0] = EOS;
      } /* endif */
      if ( UtlDirExist(pszTemp) )
      {
        if ( pITMIda->pstPropItm->szTgtStartPath[0] )
        {
          strcpy(pIda->pITMIda->szTgtStartPath, pITMIda->pstPropItm->szTgtStartPath);
        }
        else
        {
          fStartPath = FALSE;
        } /* endif */
      } /* endif */
    } /* endif */

    // add to filelist from propertyfile
    pFilePair = pITMIda->pstPropItm->pListFiles;
    pPool = pITMIda->pstPropItm->pFileNamePool;
    if (fStartPath )
    {
      if (pITMIda->szSrcStartPath[strlen(pITMIda->szSrcStartPath)-1]
                           != BACKSLASH )
      {
        strcat(pITMIda->szSrcStartPath, BACKSLASH_STR);
      } /* endif */
      if (pITMIda->szTgtStartPath[strlen(pITMIda->szTgtStartPath)-1]
                           != BACKSLASH )
      {
        strcat(pITMIda->szTgtStartPath, BACKSLASH_STR);
      } /* endif */
    } /* endif */
    for ( lIndex=0; lIndex<pITMIda->pstPropItm->usNumFiles ;lIndex++ )
    {
      if ( pFilePair[lIndex].usAliNum )
      {
        if (!fStartPath )
        {
          sprintf( pszTemp, "p %s\t%s", (PBYTE)pPool + pFilePair[lIndex].ulSrcOffset,
                                     (PBYTE)pPool + pFilePair[lIndex].ulTgtOffset );
        }
        else
        {
          strcpy(pIda->szFile1, pITMIda->szSrcStartPath);
          strcpy(pIda->szFile2, pITMIda->szTgtStartPath);
          strcat(pIda->szFile1, (PSZ)((PBYTE)pPool + pFilePair[lIndex].ulSrcOffset ));
          strcat(pIda->szFile2, (PSZ)((PBYTE)pPool + pFilePair[lIndex].ulTgtOffset ));
          sprintf(pszTemp, "p %s\t%s", pIda->szFile1, pIda->szFile2);
        } /* endif */
      }
      else
      {
        if (!fStartPath )
        {
          sprintf( pszTemp, "  %s\t%s", (PBYTE)pPool + pFilePair[lIndex].ulSrcOffset,
                                     (PBYTE)pPool + pFilePair[lIndex].ulTgtOffset );
        }
        else
        {
          strcpy(pIda->szFile1, pITMIda->szSrcStartPath);
          strcpy(pIda->szFile2, pITMIda->szTgtStartPath);
          strcat(pIda->szFile1, (PSZ)((PBYTE)pPool + pFilePair[lIndex].ulSrcOffset ));
          strcat(pIda->szFile2, (PSZ)((PBYTE)pPool + pFilePair[lIndex].ulTgtOffset ));
          sprintf(pszTemp, "  %s\t%s", pIda->szFile1, pIda->szFile2);
        } /* endif */
      } /* endif */
      sItem = (SHORT) WinSendMsg( hwndLB, LM_INSERTITEM,
                                     MP1FROMSHORT(LIT_END),
                                     MP2FROMP(pszTemp) );
      fFileListfilled = TRUE;
    } /* endfor */
  } /* endif */


  return(fFileListfilled);
} /* end of function ITMCheckPropDir  */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBITMFillListboxes                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBITMFillListboxes(hwndDlg,PSPECFILE )                 |
//+----------------------------------------------------------------------------+
//|Description:       fill directory & files listboxes & filename entryfield   |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND  hwndDlg     dialog handle                          |
//|                   PITMDLGIDA  pIda                                         |
//|                   PSPECFILE   pSpecFile                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     fill directory & files listboxes & filename entryfield   |
//+----------------------------------------------------------------------------+
VOID EQFBITMFillListboxes
(
 HWND        hwndDlg,
 PITMDLGIDA  pIda,
 PSPECFILE   pSpecFile
)
{
   HWND  hwndLB;                       // handle of currently processed listbox
   USHORT usNoOfItems;                   // no of items in a listbox
//   USHORT usI;
//   SHORT sItem;
//   CHAR  szTempDrive[5];
   BOOL   fLongFileNames = TRUE;         // use-long-file-name-functions flag

   // under Windows we can use long file name functions for all
   // Windows Version higher than or equal to 3.95
   DWORD dwVersion = GetVersion();
   BYTE  bWinMajVersion = LOBYTE( LOWORD( dwVersion ) );
   BYTE  bWinMinVersion = HIBYTE( LOWORD( dwVersion ) );
   if ( bWinMajVersion <= 2 )
   {
     fLongFileNames = FALSE;
   }
   else if ( (bWinMajVersion == 3) && (bWinMinVersion < 95) )
   {
     fLongFileNames = FALSE;
   } /* endif */

   //--- build directory path ---
   if ( pSpecFile->szDirectory[0] == EOS  )
   {
      sprintf( pSpecFile->szPath, "%c:\\%s", pSpecFile->chDrive,
                DEFAULT_PATTERN );
   }
   else
   {
      sprintf( pSpecFile->szPath, "%c:\\%s%s", pSpecFile->chDrive,
               pSpecFile->szDirectory, DEFAULT_PATTERN );
   } /* endif */

   //--- update file name field ---
   SETTEXT( hwndDlg, pSpecFile->sIdFileEF, pSpecFile->szFilePattern );

   //--- refresh directory listbox ---
   // GQ: no directory listbox anymore
   //hwndLB = GETHANDLEFROMID( hwndDlg, pSpecFile->sIdDirLB );
   //ENABLEUPDATEHWND_FALSE( hwndLB );
   //DELETEALL( hwndDlg, pSpecFile->sIdDirLB );
   //if (fLongFileNames )
   //{
   //  usNoOfItems = UtlLoadLongFileNames( pSpecFile->szPath, FILE_DIRECTORY,
   //                                      hwndLB, 0 );
   //}
   //else
   //{
   //  usNoOfItems = UtlLoadFileNames( pSpecFile->szPath, FILE_DIRECTORY,
   //                                 hwndLB, 0 );
   //} /* endif */
   //if ( usNoOfItems == UTLERROR )
   //{
   //  //retrieve contents of Current directory field
   //  hwndAct = GETHANDLEFROMID( hwndDlg, pSpecFile->sIdCurDir );
   //  WinQueryWindowText( hwndAct, sizeof( pIda->szFile1 ), pIda->szFile1) ;
   //  /*****************************************************************/
   //  /* reset chDrive and szDirectory to these values                 */
   //  /*****************************************************************/
   //  pSpecFile->chDrive = pIda->szFile1[0];
   //  if (pIda->szFile1[3])
   //  {
   //    strcpy (pSpecFile->szDirectory, &(pIda->szFile1[3]));
   //    sprintf( pSpecFile->szPath, "%c:\\%s%s", pSpecFile->chDrive,
   //         pSpecFile->szDirectory, DEFAULT_PATTERN );
   //  }
   //  else
   //  {
   //    pSpecFile->szDirectory[0] = EOS;
   //    sprintf( pSpecFile->szPath, "%c:\\%s", pSpecFile->chDrive,
   //             DEFAULT_PATTERN );
   //  } /* endif */
   //  if (fLongFileNames )
   //  {
   //    usNoOfItems = UtlLoadLongFileNames( pSpecFile->szPath, FILE_DIRECTORY,
   //                                        hwndLB, 0 );
   //  }
   //  else
   //  {
   //    usNoOfItems = UtlLoadFileNames( pSpecFile->szPath, FILE_DIRECTORY,
   //                                 hwndLB, 0 );
   //  } /* endif */
   //} /* endif */

   ////add drives to directories listbox
   UtlGetDriveList( (PBYTE)pIda->szDrives );
   //usI = 0;
   //while ( (c = (pIda->szDrives[usI])) != EOS )
   //{
   //  if ( c != pSpecFile->chDrive )
   //  {
   //    sprintf(szTempDrive, "[%c:]", c);
   //    sItem = INSERTITEMENDHWND( hwndLB, szTempDrive );
   //  } /* endif */
   //  usI++;
   //} /* endwhile */
   //ENABLEUPDATEHWND_TRUE( hwndLB );

   ////select first element in listbox
   //SELECTITEM(hwndDlg,pSpecFile->sIdDirLB, 0);

   // prepare refresh of file listbox
   hwndLB = GETHANDLEFROMID( hwndDlg, pSpecFile->sIdFileLB );
   ENABLEUPDATEHWND_FALSE( hwndLB );
   DELETEALL( hwndDlg, pSpecFile->sIdFileLB );

   sprintf( pSpecFile->szPath, "%c:\\%s%s",
               pSpecFile->chDrive,
               pSpecFile->szDirectory,
               pSpecFile->szFilePattern );
   if (fLongFileNames )
   {
     usNoOfItems = UtlLoadLongFileNames( pSpecFile->szPath,
                                         FILE_NORMAL,
                                         hwndLB,
                                         0 );
   }
   else
   {
     usNoOfItems = UtlLoadFileNames( pSpecFile->szPath,
                                     FILE_NORMAL,
                                     hwndLB,
                                     0 );
   } /* endif */
   // select document if only one is displayed in listbox
   if ( usNoOfItems == 1 )
   {
      SELECTITEM( hwndDlg, pSpecFile->sIdFileLB, 0 );
   } /* endif */
   ENABLEUPDATEHWND_TRUE( hwndLB );

   // set current directory
   sprintf( pSpecFile->szPath, "%c:\\%s", pSpecFile->chDrive,
            pSpecFile->szDirectory );
   //update current directory entry field with new drive
   SETTEXT( hwndDlg, pSpecFile->sIdCurDir, pSpecFile->szPath );

} /* end of EQFBITMFillListboxes */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBITMChar                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBITMChar(hwndDlg, mp1,mp2)                            |
//+----------------------------------------------------------------------------+
//|Description:       Handle WM_CHAR messages in ITM dialog panel              |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND  hwndDlg     dialog handle                          |
//|                   MPARAM mp1                                               |
//|                   MPARAM mp2                                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     work on WM_CHAR message                                  |
//+----------------------------------------------------------------------------+
MRESULT EQFITMChar
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = FALSE;            // result of message processing

   PITMDLGIDA  pIda;                      // ptr to dialog IDA
   BOOL        fOK = TRUE;             // internal OK flag
   HWND        hwndFocus;              // window having the focus
   PSPECFILE   pActFile = NULL;
   USHORT      usKey = SHORT2FROMMP2(mp2);
   CHAR        szPath[_MAX_PATH];     // active pattern (file mode)

   mp1;
   if ( (GetKeyState(VK_RETURN) & 0x8000) || (GetKeyState(VK_DELETE) & 0x8000) )
   {
      pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);
      hwndFocus = GETFOCUS();

      if ( usKey == VK_DELETE )
      {
        if ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_FILELIST_LB ))
        {
          //del selected entry in filelist
          fOK = EQFBDelInList(hwndDlg, pIda);
        }
        else
        {
          fOK = FALSE;
        } /* endif */
      }
      else
      {
        if ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_SOURCE_EF ))
        {
          pActFile = &(pIda->SrcFile);
          QUERYTEXT( hwndDlg, pActFile->sIdFileEF, szPath );
        }
        else if ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_TARGET_EF ))
        {
          pActFile = &(pIda->TgtFile);
          QUERYTEXT( hwndDlg, pActFile->sIdFileEF, szPath );
        }
        else if ( ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_SRCDIR_LB )) )
        {
          /**************************************************************/
          /* force update of files listbox; don't reset search pattern. */
          /**************************************************************/
          pActFile = &(pIda->SrcFile);

          // force a refresh of files listbox
          EQFBITMControl( hwndDlg, ID_ITM_SRCDIR_LB, LN_ENTER );
          fOK = FALSE;                     // we've already updated the listbox
          mResult = TRUE;                  // do not process as default pushbutton
        }
        else if ( ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_TGTDIR_LB )) )
        {
          /**************************************************************/
          /* force update of files listbox, don't reset search pattern. */
          /**************************************************************/
          pActFile = &(pIda->TgtFile);

          // force a refresh of files listbox
          EQFBITMControl( hwndDlg, ID_ITM_TGTDIR_LB, LN_ENTER );
          fOK = FALSE;                     // we've already updated the listbox
          mResult = TRUE;                  // do not process as default pushbutton
        }
        else if ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_FILELIST_LB ))
        {
          ENABLECTRL( hwndDlg, ID_ITM_DELINLIST_PB, TRUE );
          fOK = FALSE;                     // we've already updated the listbox
        }
        else if ( ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_SRCFILE_LB)) )
        {
          /**************************************************************/
          /* force update of files listbox, don't reset search pattern. */
          /**************************************************************/
          pActFile = &(pIda->SrcFile);

          // fill files entryfield
          EQFBITMControl( hwndDlg, ID_ITM_SRCFILE_LB, LN_ENTER );
          fOK = FALSE;                     // we've already updated the listbox
          mResult = TRUE;                  // do not process as default pushbutton
        }
        else if ( ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_TGTFILE_LB)) )
        {
          /**************************************************************/
          /* force update of files listbox, don't reset search pattern. */
          /**************************************************************/
          pActFile = &(pIda->TgtFile);

          // fill files entryfield
          EQFBITMControl( hwndDlg, ID_ITM_TGTFILE_LB, LN_ENTER );
          fOK = FALSE;                     // we've already updated the listbox
          mResult = TRUE;                  // do not process as default pushbutton
        }
        else if ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_SRCSTARTPATH_EF ))
          {
          QUERYTEXT( hwndDlg, ID_ITM_SRCSTARTPATH_EF, pIda->pITMIda->szSrcStartPath);
          UtlStripBlanks(pIda->pITMIda->szSrcStartPath);    // tk 2001-07-19
          strcpy ( szPath, pIda->pITMIda->szSrcStartPath);
          pActFile = &(pIda->SrcFile);
          fOK = EQFITMStartpathForFillLB(pIda, pActFile, &(szPath[0]));
          if (fOK )
          {
            EQFBITMFillListboxes(hwndDlg,pIda, pActFile);
          }
          else
          {
            mResult = WinDefDlgProc( hwndDlg, WM_CHAR, mp1, mp2 );
          } /* endif */
          fOK = FALSE;               // we've already updated the listbox
        }
        else if ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_TGTSTARTPATH_EF ))
        {
          QUERYTEXT( hwndDlg, ID_ITM_TGTSTARTPATH_EF, pIda->pITMIda->szTgtStartPath);
          UtlStripBlanks(pIda->pITMIda->szTgtStartPath);    // tk 2001-07-19
          strcpy ( szPath, pIda->pITMIda->szTgtStartPath);
          pActFile = &(pIda->TgtFile);
          fOK = EQFITMStartpathForFillLB(pIda, pActFile, &(szPath[0]));
          if (fOK )
          {
            EQFBITMFillListboxes(hwndDlg,pIda, pActFile);
          }
          else
          {
            mResult = WinDefDlgProc( hwndDlg, WM_CHAR, mp1, mp2 );
          } /* endif */
          fOK = FALSE;               // we've already updated the listbox
        }
        else if ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_CURDIRSRC_EF ))
        {
          EQFBAdjToDir( hwndDlg, pIda, &(pIda->SrcFile), LN_ENTER );
          fOK = FALSE;               // we've already updated the listbox
        }
        else if ( hwndFocus == GetDlgItem( hwndDlg, ID_ITM_CURDIRTGT_EF ))
        {
          EQFBAdjToDir( hwndDlg, pIda, &(pIda->TgtFile), LN_ENTER );
          fOK = FALSE;               // we've already updated the listbox
        }
        else
        {
          fOK = FALSE;
        } /* endif */
        if ( fOK )
        {
          fOK = EQFITMPrepForFillLB(pIda, pActFile, &(szPath[0]));
        } /* endif */

        // if ok use specified values
        if ( fOK )
        {
           EQFBITMFillListboxes(hwndDlg,pIda, pActFile);
        }
        else
        {
          mResult = WinDefDlgProc( hwndDlg, WM_CHAR, mp1, mp2 );
        } /* endif */
      } /* endif */
   }
   else
   {
      mResult = WinDefDlgProc( hwndDlg, WM_CHAR, mp1, mp2 );
   } /* endif */

   return( mResult );

} /* end of EQFBITMChar */

static BOOL
EQFITMPrepForFillLB
(
   PITMDLGIDA      pIda,
   PSPECFILE       pActFile,
   PSZ             pszInputPath
)
{
   PSZ         pszPath;
   PSZ         pszDrive;
   PSZ         pszName;
   CHAR        chDrive = EOS;                // selected drive
   BOOL        fOK = TRUE;

   // split drive from path
   pszPath = strchr( pszInputPath, COLON );
   if ( pszPath )
   {
      pszDrive = pszInputPath;
      *pszPath = EOS;
      pszPath ++;
   }
   else
   {
      pszDrive = NULL;
      pszPath = pszInputPath;
   } /* endif */

   // skip first backslash in path or file name
   if ( *pszPath == BACKSLASH )
   {
      pszPath++;
   } /* endif */

   // split filename from path
   pszName = strrchr( pszPath, BACKSLASH );
   if ( pszName )
   {
      *pszName = EOS;
      pszName++;
   }
   else
   {
      pszName = pszPath;
      pszPath = NULL;
   } /* endif */

   // check specified drive
   if ( pszDrive )
   {
      strupr( pszDrive );
      if ( ( strlen( pszDrive ) != 1 ) ||
           !strchr( pIda->szDrives, *pszDrive ) )
      {
         fOK = FALSE;         // drive is invalid
      }
      else
      {
         chDrive = *pszDrive; // set new drive
      } /* endif */
   }
   else
   {
      chDrive = pActFile->chDrive;          // use current drive
   } /* endif */

   // check directory information -- update it if necessary
   if ( fOK )
   {
      if ( pszPath )
      {
         sprintf( pActFile->szPath, "%c:\\%s", chDrive, pszPath );
         if ( EQFBFileExists( pActFile->szPath ) )
         {
            sprintf( pActFile->szDirectory, "%s\\", pszPath );
         }
         else
         {
            fOK = FALSE;
         } /* endif */
      }
      else
      {
         sprintf( pActFile->szPath, "%c:\\%s",
                  chDrive, pActFile->szDirectory);
         pActFile->szPath[ strlen(pActFile->szPath) -1 ] = EOS;
         if ( !EQFBFileExists( pActFile->szPath ) )
         {
           pActFile->szDirectory[0] = EOS;
         } /* endif */
      } /* endif */
   } /* endif */
   if (fOK )
   {
     pActFile->chDrive = chDrive;
     //--- refresh contents of listboxes ---
     if ( pszName && *pszName )
     {
        strcpy( pActFile->szFilePattern, pszName );
     }
     else
     {
        strcpy( pActFile->szFilePattern, "*.*" );
     } /* endif */
   } /* endif */

   return( fOK);

} /* end of EQFBITMPrepForFillLB */

static BOOL
EQFITMStartpathForFillLB
(
   PITMDLGIDA      pIda,
   PSPECFILE       pActFile,
   PSZ             pszInputPath
)
{
   PSZ         pszPath;
   PSZ         pszDrive;
   CHAR        chDrive = EOS;                // selected drive
   BOOL        fOK = TRUE;
   LONG        lLen = 0;

   // split drive from path
   pszPath = strchr( pszInputPath, COLON );
   if ( pszPath )
   {
      pszDrive = pszInputPath;
      *pszPath = EOS;
      pszPath ++;
   }
   else
   {
      pszDrive = NULL;
      pszPath = pszInputPath;
   } /* endif */

   // skip first backslash in path or file name
   if ( *pszPath == BACKSLASH )
   {
      pszPath++;
   } /* endif */

   // check specified drive
   if ( pszDrive )
   {
      strupr( pszDrive );
      if ( ( strlen( pszDrive ) != 1 ) ||
           !strchr( pIda->szDrives, *pszDrive ) )
      {
         fOK = FALSE;         // drive is invalid
      }
      else
      {
         chDrive = *pszDrive; // set new drive
      } /* endif */
   }
   else
   {
      chDrive = pActFile->chDrive;          // use current drive
   } /* endif */

   // check directory information -- update it if necessary
   if ( fOK )
   {
      if ( pszPath )
      {
         sprintf( pActFile->szPath, "%c:\\%s", chDrive, pszPath );
         if ( EQFBFileExists( pActFile->szPath ) )
         {
            lLen = strlen(pszPath);
            if ( lLen )
            {
              if ( pszPath[lLen - 1] != '\\' )
              {
                sprintf( pActFile->szDirectory, "%s\\", pszPath );
              }
              else
              {
                sprintf( pActFile->szDirectory, "%s", pszPath );
              } /* endif */
            }
            else
            {
              pActFile->szDirectory[0] = EOS;
              sprintf( pActFile->szPath, "%c:\\", chDrive );
            } /* endif */
         }
         else
         {
            fOK = FALSE;
         } /* endif */
      }
      else
      {
         pActFile->szDirectory[0] = EOS;
         sprintf( pActFile->szPath, "%c:\\", chDrive );
      } /* endif */
   } /* endif */
   if (fOK )
   {
     pActFile->chDrive = chDrive;
     //--- refresh contents of listboxes ---
     strcpy( pActFile->szFilePattern, "*.*" );
   } /* endif */

   return( fOK);

} /* end of EQFBITMStartpathForFillLB */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBITMControl                                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBITMControl(hwndDlg, mp1,mp2)                         |
//+----------------------------------------------------------------------------+
//|Description:       Handle WM_Control msg   in ITM dialog panel              |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND  hwndDlg     dialog handle                          |
//|                   SHORT sId                                                |
//|                   SHORT sNotification                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     switch ( listbox)                                        |
//|                     case srcfile lb: fill srcfile entryfield               |
//|                     case tgtfile lb: fill tgtfile entryfield               |
//|                     case srcdirectory lb: adjust files lb to sel.dir       |
//|                     case tgtdirectory lb: adjust files lb to sel.dir       |
//|                     case filepair lb: if double-clicked, delete entry      |
//+----------------------------------------------------------------------------+
MRESULT EQFBITMControl
(
   HWND hwndDlg,
   SHORT  sId,                         // id of control
   SHORT  sNotification                // notification message
)
{
   MRESULT     mResult = FALSE;        // result of message processing
   PITMDLGIDA  pIda;
   BOOL        fOK;

   pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);

   switch ( sId )
   {
      case ID_ITM_SRCFILE_LB:
         fOK = EQFBFillFileEF( hwndDlg, &(pIda->SrcFile), sNotification );
         break;
      case  ID_ITM_TGTFILE_LB:
         fOK = EQFBFillFileEF( hwndDlg, &(pIda->TgtFile), sNotification );
         break;

      case ID_ITM_SRCDIR_LB:
        SETCURSOR(SPTR_WAIT);
        fOK = EQFBAdjToDir( hwndDlg, pIda, &(pIda->SrcFile), sNotification );
        SETCURSOR(SPTR_ARROW);
        break;
      case  ID_ITM_TGTDIR_LB:
        SETCURSOR(SPTR_WAIT);
        fOK = EQFBAdjToDir( hwndDlg, pIda, &(pIda->TgtFile), sNotification );
        SETCURSOR(SPTR_ARROW);
        break;
      case  ID_ITM_FILELIST_LB:
        //if item is double-clicked, delete it
        if ( sNotification == LN_ENTER )
        {
          fOK = EQFBDelInList(hwndDlg, pIda);
        }
        else if ( sNotification == LN_SELECT )
        {
          if ( QUERYSELECTION( hwndDlg, ID_ITM_FILELIST_LB ) != LIT_NONE )
          {
            ENABLECTRL( hwndDlg, ID_ITM_DELINLIST_PB, TRUE );
          } /* endif */
        } /* endif */
        break;
   } /* endswitch */

   return( mResult );
} /* end of EQFBITMControl */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFillFileEF                                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFillFileEF(HWND,PSPECFILE,MPARAM)                    |
//+----------------------------------------------------------------------------+
//|Description:       fill file entryfield with selected filename              |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND       hwndDlg,                                      |
//|                   PSPECFILE  pActFile          file specs of src or tgt    |
//|                   MPARAM     mp1               parameter to spec WM_CONTROL|
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    if ok                                            |
//|                   FALSE   if something went wrong                          |
//+----------------------------------------------------------------------------+
//|Function flow:     fill file entryfield with selected filename from listbox |
//+----------------------------------------------------------------------------+
static
BOOL EQFBFillFileEF
(
  HWND       hwndDlg,
  PSPECFILE  pActFile,
  SHORT      sNotification
)
{
  SHORT      sItem;
  BOOL       fOK = TRUE;

   switch ( sNotification )
   {
     //file is selected via doubleclick
      case LN_ENTER:
        // activate directory
        sItem = QUERYSELECTION( hwndDlg, pActFile->sIdFileLB );
        if ( sItem != LIT_NONE )
        {
          QUERYITEMTEXT( hwndDlg, pActFile->sIdFileLB, sItem,
                          pActFile->szPath );
          if ( pActFile->szDirectory[0] == EOS )
          {
            sprintf(pActFile->szFileName, "%c:\\%s", pActFile->chDrive,
                     pActFile->szPath);
          }
          else
          {
            sprintf( pActFile->szFileName, "%c:\\%s%s", pActFile->chDrive,
                   pActFile->szDirectory, pActFile->szPath );
          } /* endif */

          //--- update file name field ---
          SETTEXT( hwndDlg, pActFile->sIdFileEF, pActFile->szPath );
        }
        else
        {
          fOK = FALSE;
        } /* endif */
       break;
  } /* endswitch */

  return ( fOK );
} /* end of function EQFBFillFileEF */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBAdjToDir                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBAdjToDir(HWND,PSPECFILE,MPARAM)                      |
//+----------------------------------------------------------------------------+
//|Description:       adjust files listbox & curdir &fileentryfield to         |
//|                   selected directory                                       |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND       hwndDlg,                                      |
//|                   PSPECFILE  pActFile          file specs of src or tgt    |
//|                   SHORT      sNotification     parameter to spec WM_CONTROL|
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    if ok                                            |
//|                   FALSE   if something went wrong                          |
//+----------------------------------------------------------------------------+
//|Function flow:     adjust files listbox & curdir &fileentryfield to         |
//|                   selected directory                                       |
//+----------------------------------------------------------------------------+
static
BOOL  EQFBAdjToDir
(
  HWND        hwndDlg,
  PITMDLGIDA  pIda,
  PSPECFILE   pActFile,
  SHORT       sNotification
)
{
//  SHORT      sItem;
  BOOL       fOK = TRUE;
//  PSZ        pszPointer;

  switch ( sNotification )
  {
     case LN_ENTER:

        // get directory selection
        //sItem = QUERYSELECTION( hwndDlg, pActFile->sIdDirLB );
        //if ( sItem != LIT_NONE )
        //{
        //   QUERYITEMTEXT( hwndDlg, pActFile->sIdDirLB, sItem,
        //                  pActFile->szPath );
        //}
        //else
        //{
        //   fOK = FALSE;
        //} /* endif */
        QUERYTEXT( hwndDlg, pActFile->sIdCurDir, pActFile->szPath );

        // update chDrive and szDirectory
        if ( fOK )
        {
          PSZ pszDir = pActFile->szPath;

          // split in path and directory part
          if ( (pszDir[1] == COLON) && isalpha(pszDir[0]) )
          {
            pActFile->chDrive = pszDir[0];
            pszDir += 2;
          } /* endif */

          if ( pszDir[0] == BACKSLASH )
          {
            pszDir++;
          } /* endif */

          strcpy( pActFile->szDirectory, pszDir );

          // add trailing backslash if there is none
          if ( pActFile->szDirectory[strlen(pActFile->szDirectory)-1] != BACKSLASH )
          {
            strcat( pActFile->szDirectory, BACKSLASH_STR );
          } /* endif */

          // refresh files LB
          EQFBITMFillListboxes(hwndDlg,pIda, pActFile);
        } /* endif */
     break;
  } /* endswitch */

  return ( fOK );
} /* end of function EQFBAdjToDir   */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FillStringPool                                           |
//+----------------------------------------------------------------------------+
//|Function call:     FillStringPool(HWND,PPSZ)                                |
//+----------------------------------------------------------------------------+
//|Description:       fill filelist contents to allocated memory               |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND       hwndLB                                        |
//|                   PPSZ       ppString          returns ptr to filled mem   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    if ok                                            |
//|                   FALSE   if something went wrong                          |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate space for at most 1000 filepairs                |
//|                   while not thru listbox                                   |
//|                     get next item with filepair                            |
//|                     fill both filenames into new filelist                  |
//+----------------------------------------------------------------------------+
static
BOOL FillStringPool
(
 HWND     hwndDlg,
 PITMIDA  pITMIda,
 PSZ  ** pppListTokens
)
{
  BOOL       fOK = TRUE;
  LONG       lSize;
  LONG       lUsed;
  PSZ        pString;
  SHORT      sItemCnt;
  HWND       hwndLB;
  SHORT      sI;
  LONG       lItemLen;
  USHORT     usNoTokens;
  PSZ        *ppTokens;
  LONG       lSrcStartPathLen = 0;
  LONG       lTgtStartPathLen = 0;
  PSZ        pszTgtFile = NULL;


  /******************************************************************/
  /* allocate memory for 2000 tokens -- should be enough ...        */
  /* if pppListTokens exists already, delete it                       */
  /******************************************************************/

  if ( pppListTokens )                     // if it exists, free it
  {
    UtlAlloc((PVOID *)pppListTokens, 0L, 0L, ERROR_STORAGE);
  } /* endif */

  /******************************************************************/
  /* allocate memory for 2000 tokens -- should be enough ...        */
  /******************************************************************/
  fOK = UtlAlloc( (PVOID *)pppListTokens, 0L,
                   2000L * sizeof(PSZ) , ERROR_STORAGE );
  if ( fOK )
  {
    PITMDLGIDA  pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);
    ppTokens = *pppListTokens;
    lUsed = 0;
    usNoTokens = 0;

    hwndLB = GETHANDLEFROMID( hwndDlg, ID_ITM_FILELIST_LB );
    ENABLEUPDATEHWND_FALSE( hwndLB );
    sItemCnt = QUERYITEMCOUNTHWND( hwndLB );
    lSize = 0;
    lSrcStartPathLen = strlen(pITMIda->szSrcStartPath);
    fOK = UtlAlloc( (PVOID *)&pString, 0L,
                    (LONG)(MAX_LONGPATH * 2), ERROR_STORAGE);

    for ( sI = 0;sI < sItemCnt ;sI++ )
    {
       lItemLen = QUERYITEMTEXTHWND( hwndLB, sI, pIda->szFile1 );
       /*****************************************************************/
       /* QUERYITEMTEXTHWND will return LB_ERR if sItem is not a valid  */
       /* index                                                         */
       /*****************************************************************/
       if ( lItemLen != LB_ERR )
       {
//         lSize += lItemLen + 1 - (2*sStartPathLen);
         lSize += lItemLen + 1;

         //copy source filename in temp buffer
         // if it starts with sStartPath, copy rel. path into pString
         // else issue error msg!
         //copy temporarily to compare without regard of Upper/Lower
         strcpy(pString, &(pIda->szFile1[2]));
         UtlUpper (pString);
         strcpy(&(pIda->szFile2[0]), pITMIda->szSrcStartPath);
         UtlUpper (&(pIda->szFile2[0]));

         pszTgtFile = strchr( pString, '\t' );
         if (pszTgtFile )
         {
           *pszTgtFile = EOS;
           pszTgtFile++;

           if (strncmp(&(pIda->szFile2[0]), pString, lSrcStartPathLen) )
           {
              ITMUtlError( pITMIda, ITM_STARTPATHINVALID, MB_CANCEL,
                                   1, &pString, EQF_ERROR );
              fOK = FALSE;
           }
           else
           {
             strcpy(&(pIda->szFile2[0]), pITMIda->szTgtStartPath);
             UtlUpper (&(pIda->szFile2[0]));
             lTgtStartPathLen = strlen(pITMIda->szTgtStartPath);
             if (strncmp(&(pIda->szFile2[0]), pszTgtFile, lTgtStartPathLen) )
             {
                ITMUtlError( pITMIda, ITM_STARTPATHINVALID, MB_CANCEL,
                                     1, &pszTgtFile, EQF_ERROR );
                fOK = FALSE;
             } /* endif */
           } /* endif */
         } /* endif */

       } /* endif */
    } /* endfor */
    if ( fOK  )
    {
      if (pString ) UtlAlloc( (PVOID *)&pString, 0L, 0L, ERROR_STORAGE);

      lSize += 2;
      fOK = UtlAlloc( (PVOID *)&pString, 0L, (LONG)lSize, ERROR_STORAGE);
    } /* endif */
    if ( fOK )
    {
      for ( sI=0 ;sI < sItemCnt ;sI++ )
      {
         lItemLen = QUERYITEMTEXTHWND( hwndLB, sI, pIda->szFile1 );
         /*****************************************************************/
         /* QUERYITEMTEXTHWND will return LB_ERR if sItem is not a valid  */
         /* index                                                         */
         /*****************************************************************/
         if ( lItemLen != LB_ERR )
         {
           pString[lUsed] = EOS;
           lUsed++;
           /*****************************************************************/
           /* skip 1st two chars because they are 2 blanks or P and a blank */
           /*****************************************************************/
           if (lSrcStartPathLen )
           {
             pszTgtFile = strchr( &(pIda->szFile1[2]), '\t' );
             *pszTgtFile = EOS;
             pszTgtFile++;

             strcpy(pString + lUsed, &(pIda->szFile1[2+lSrcStartPathLen]));
             strcat(pString + lUsed, "\t" );
             strcat(pString + lUsed, pszTgtFile+lTgtStartPathLen);
             strcat(pString + lUsed, "\t" );
           }
           else
           {
             strcpy (pString + lUsed, &(pIda->szFile1[2]));
           } /* endif */
           ppTokens[usNoTokens++] = strtok( pString + lUsed, "\t" );
           ppTokens[usNoTokens++] = strtok( NULL, "\t" );
           lUsed += lItemLen;
         } /* endif */

      } /* endfor */
    } /* endif */
    ENABLEUPDATEHWND_TRUE( hwndLB );
    if ( lUsed < lSize )
    {
      pString[lUsed] = EOS;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
    /********************************************************************/
    /* allocated storage freed at end of program                        */
    /********************************************************************/
  } /* endif */
  return ( fOK );
} /* end of function FillStringPool */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMGetFileName                                           |
//+----------------------------------------------------------------------------+
//|Function call:     ITMGetFileName(HWND, PSPECFILE )                         |
//+----------------------------------------------------------------------------+
//|Description:       build fully qualified filename from EF and curdir        |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND       hwndLB                                        |
//|                   PSPECFILE  pSpecFile                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    if ok                                            |
//|                   FALSE   if something went wrong                          |
//+----------------------------------------------------------------------------+
//|Function flow:     build fully qualified filename from EF and curdir        |
//|                   issue error if file does not exist                       |
//+----------------------------------------------------------------------------+
static BOOL
ITMGetFileName
(
  HWND      hwndDlg,
  PITMDLGIDA pIda,
  PSPECFILE pActFile
)
{
  HWND      hwndAct;
  PSZ       pchDirBuffer = pIda->szFile1;
  PSZ       pchBuffer    = pIda->szFile2;
  BOOL      fOK = TRUE;
  PSZ       pszPath;                      //pt to drive in EF
  PSZ       pszDrive;                     //pt to path in EF
  PSZ       pszName;                      //pt to name in EF
  CHAR      chDrive = EOS;                      //contains drive char

  //retrieve contents of Current directory field
  hwndAct = GETHANDLEFROMID( hwndDlg, pActFile->sIdCurDir );
  WinQueryWindowText( hwndAct, MAX_LONGPATH, pchDirBuffer) ;

  //retrieve contents of Filename entryfield
  hwndAct = GETHANDLEFROMID( hwndDlg, pActFile->sIdFileEF );
  WinQueryWindowText( hwndAct, MAX_LONGPATH, pchBuffer) ;
  // split drive from EF contents
  pszPath = strchr( pchBuffer, COLON );

  if ( pszPath )
  {
     pszDrive = pchBuffer;
     *pszPath = EOS;
     pszPath ++;
  }
  else
  {
     pszDrive = NULL;
     pszPath = pchBuffer;
  } /* endif */

  // skip first backslash in path or file name
  if ( *pszPath == BACKSLASH )
  {
     pszPath++;
  } /* endif */

  // split filename from path
  pszName = strrchr( pszPath, BACKSLASH );
  if ( pszName )
  {
     *pszName = EOS;
     pszName++;
  }
  else
  {
     pszName = pszPath;
     pszPath = NULL;
  } /* endif */

  // check specified drive
  if ( pszDrive )
  {
     strupr( pszDrive );
     if ( ( strlen( pszDrive ) != 1 ) ||
          !strchr( pIda->szDrives, *pszDrive ) )
     {
        fOK = FALSE;         // drive is invalid
     }
     else
     {
        chDrive = *pszDrive; // set new drive
     } /* endif */
  }
  else
  {
     chDrive = pActFile->chDrive;          // use current drive
  } /* endif */

  // build temporairy name in pActFile->szPath
  if ( fOK )
  {
    if ( !pszDrive && !pszPath )
    {
      sprintf(pActFile->szPath, "%s%s", pchDirBuffer, pchBuffer);
    }
    else if (!pszPath)
    {
       sprintf( pActFile->szPath, "%c:\\%s", *pszDrive, pszName );
    }
    else
    {
       sprintf( pActFile->szPath, "%c:\\%s\\%s", chDrive, pszPath, pszName );
    } /* endif */
    if ( EQFBFileExists( pActFile->szPath ) )
    {
        strcpy(pActFile->szFileName, pActFile->szPath);
    }
    else
    {
        /**************************************************************/
        /* if file is selected in listbox, use this file              */
        /**************************************************************/
        // activate directory
        SHORT sItem = QUERYSELECTION( hwndDlg, pActFile->sIdFileLB );
        if ( sItem != LIT_NONE )
        {
          QUERYITEMTEXT( hwndDlg, pActFile->sIdFileLB, sItem,
                          pActFile->szPath );
          if ( pActFile->szDirectory[0] == EOS )
          {
            sprintf(pActFile->szFileName, "%c:\\%s", pActFile->chDrive,
                     pActFile->szPath);
          }
          else
          {
            sprintf( pActFile->szFileName, "%c:\\%s%s", pActFile->chDrive,
                   pActFile->szDirectory, pActFile->szPath );
          } /* endif */

          //--- update file name field ---
          SETTEXT( hwndDlg, pActFile->sIdFileEF, pActFile->szPath );
        }
        else
        {
          fOK = FALSE;
        } /* endif */
    } /* endif */
  } /* endif */
  if ( !fOK )
  {
     WinQueryWindowText( hwndAct, MAX_LONGPATH, pchBuffer) ;
     pszPath = pchBuffer;
     ITMUtlError( pIda->pITMIda, ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszPath,
                      EQF_ERROR );
     SETFOCUS( hwndDlg, pActFile->sIdFileEF );
  } /* endif */

  return ( fOK );
} /* end of function ITMGetFileName */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBDelInList                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBDelInList (HWND, PITMDLGIDA )                        |
//+----------------------------------------------------------------------------+
//|Description:       delete selected item in filelist                         |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND       hwndDlg                                       |
//|                   PITMDLGIDA pIda                                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    if ok                                            |
//|                   FALSE   if something went wrong                          |
//+----------------------------------------------------------------------------+
//|Function flow:     get selected filepair                                    |
//|                   if filepair is already prepared, ask user whether to goon|
//|                   if user really wants to delete,                          |
//|                     delete entry in filelist                               |
//|                     delete continuation file if it exists                  |
//+----------------------------------------------------------------------------+
static BOOL
EQFBDelInList
(
 HWND       hwndDlg,
 PITMDLGIDA pIda
)
{
  HWND      hwndLB;
  SHORT     sItem;
  BOOL      fOK = TRUE;
  USHORT    usMBID = MBID_YES;
  LONG      lItemLen;
  PSZ       pSrcTok, pTgtTok;
  PITMIDA   pITMIda;

  pITMIda= pIda->pITMIda;
  sItem =  QUERYSELECTION( hwndDlg, ID_ITM_FILELIST_LB );
  if ( sItem != LIT_NONE )
  {
    hwndLB = GETHANDLEFROMID( hwndDlg, ID_ITM_FILELIST_LB );
    ENABLEUPDATEHWND_FALSE( hwndLB );
    /******************************************************************/
    /* if filepair is already prepared, issue warning that alifile    */
    /* will be deleted also                                           */
    /******************************************************************/
    lItemLen = (SHORT)QUERYITEMTEXTHWND( hwndLB, sItem, pIda->szFile1 );
    if ( pIda->szFile1[0] == 'p')
    {
      usMBID = ITMUtlError( pITMIda, ITM_DELETEALI, MB_YESNO,
                            0, NULL, EQF_QUERY );
    } /* endif */
    if ( usMBID == MBID_YES )
    {
      sItem = (SHORT) DELETEITEMHWND( hwndLB, sItem );
      if ( sItem == 0 )
      {
        ENABLECTRL( hwndDlg, ID_ITM_ALIGN_PB, FALSE );
        ENABLECTRL( hwndDlg, ID_ITM_VISUAL_PB, FALSE );
        ENABLECTRL( hwndDlg, ID_ITM_PREPARE_PB, FALSE );
        ENABLECTRL( hwndDlg, ID_ITM_DELALL_PB, FALSE );
      } /* endif */
      /****************************************************************/
      /* delete disabled since nothing selected                       */
      /****************************************************************/
      ENABLECTRL( hwndDlg, ID_ITM_DELINLIST_PB, FALSE );

      /****************************************************************/
      /* if alifile exists, delete it                                 */
      /****************************************************************/
      if ( lItemLen )
      {
        pSrcTok = strtok(&(pIda->szFile1[2]), "\t" );
        pTgtTok = strtok( NULL, "\t" );
        strcpy(pITMIda->chSourceFile, pSrcTok);
        strcpy(pITMIda->chTargetFile, pTgtTok);
      } /* endif */
      /****************************************************************/
      /* call IsPrepared( additional param srcfilename/tgtfilename)   */
      /* be carefull, filepair may be twice in filelist,              */
      /* once prepared, the other time unprepared....                 */
      /****************************************************************/
      if ( pIda->szFile1[0] == 'p')
      {
        USHORT  usNumPrepared;
        IsPrepared(pITMIda, &(pITMIda->usNumPrepared)) ;
        usNumPrepared = pITMIda->usNumPrepared;
        EQFITMDelAli(pITMIda);
        EQFITMDelFilePairList(pITMIda, usNumPrepared);
        pITMIda->chSourceFile[0] = EOS;
        pITMIda->chTargetFile[0] = EOS;
      } /* endif */
    } /* endif */
    ENABLEUPDATEHWND_TRUE( hwndLB );
    SETFOCUSHWND( hwndLB );
  }
  else
  {
    ENABLECTRL( hwndDlg, ID_ITM_DELINLIST_PB, FALSE );
    WinAlarm( HWND_DESKTOP, WA_WARNING );
    fOK = FALSE;
  } /* endif */
  return ( fOK );
} /* end of function EQFBDelInList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBDelAll                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBDelAll    (HWND, PITMDLGIDA )                        |
//+----------------------------------------------------------------------------+
//|Description:       delete all     items in filelist                         |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND       hwndDlg                                       |
//|                   PITMDLGIDA pIda                                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    if ok                                            |
//|                   FALSE   if something went wrong                          |
//+----------------------------------------------------------------------------+
//|Function flow:     get selected filepair                                    |
//|                   if filepair is already prepared, ask user whether to goon|
//|                   if user really wants to delete,                          |
//|                     delete entry in filelist                               |
//|                     delete continuation file if it exists                  |
//+----------------------------------------------------------------------------+
static BOOL
EQFBDelAll
(
 HWND       hwndDlg,
 PITMDLGIDA pIda
)
{
  HWND      hwndLB;
  LONG      lItem = 0;
  BOOL      fOK = TRUE;
  USHORT    usMBID = MBID_YES;
  LONG      lItemLen;
  PSZ       pSrcTok, pTgtTok;
  PITMIDA   pITMIda;
  LONG      lUsed;
  USHORT    usNoTokens;
  LONG      lItemCnt;
  SHORT     sI;

  pITMIda= pIda->pITMIda;
  lUsed = 0;
  usNoTokens = 0;

  hwndLB = GETHANDLEFROMID( hwndDlg, ID_ITM_FILELIST_LB );
  ENABLEUPDATEHWND_FALSE( hwndLB );
  lItemCnt = QUERYITEMCOUNTHWND( hwndLB );
  /********************************************************************/
  /* if one filepair is prepared, ask user whether to delete          */
  /* if more than one filepair is prepared, ask user only once !!     */
  /********************************************************************/
  usMBID = MBID_YES;
  sI = 0;
  while ( (pIda->szFile1[0] != 'p') && (sI < lItemCnt)  )
  {
     lItemLen = QUERYITEMTEXTHWND( hwndLB, sI, pIda->szFile1 );
     if ( pIda->szFile1[0] == 'p')
     {
       usMBID = ITMUtlError( pITMIda, ITM_DELETEALI, MB_YESNO,
                             0, NULL, EQF_QUERY );
     } /* endif */
     sI++;
  } /* endwhile */
  if ( usMBID == MBID_YES )
  {
    for ( sI = 0;sI < lItemCnt ;sI++ )
    {
      lItemLen = QUERYITEMTEXTHWND( hwndLB, 0, pIda->szFile1 );
      /*****************************************************************/
      /* QUERYITEMTEXTHWND will return LB_ERR if lItem is not a valid  */
      /* index                                                         */
      /*****************************************************************/
      if ( lItemLen == LB_ERR )
      {
        lItemLen = 0;
      } /* endif */
      lItem =  DELETEITEMHWND( hwndLB, 0 );
      /****************************************************************/
      /* if alifile exists, delete it                                 */
      /****************************************************************/
      if ( pIda->szFile1[0] == 'p' && lItemLen)
      {
        pSrcTok = strtok(&(pIda->szFile1[2]), "\t" );
        pTgtTok = strtok( NULL, "\t" );
        strcpy(pITMIda->chSourceFile, pSrcTok);
        strcpy(pITMIda->chTargetFile, pTgtTok);
        IsPrepared(pITMIda, &(pITMIda->usNumPrepared)) ;
        EQFITMDelAli(pITMIda);
      } /* endif */
    } /* endfor */
  } /* endif */

  if ( (usMBID== MBID_YES) && (lItem == 0) )
  {
    ENABLECTRL( hwndDlg, ID_ITM_DELINLIST_PB, FALSE );
    ENABLECTRL( hwndDlg, ID_ITM_DELALL_PB, FALSE );
    ENABLECTRL( hwndDlg, ID_ITM_ALIGN_PB, FALSE );
    ENABLECTRL( hwndDlg, ID_ITM_VISUAL_PB, FALSE );
    ENABLECTRL( hwndDlg, ID_ITM_PREPARE_PB, FALSE );
  } /* endif */

  ENABLEUPDATEHWND_TRUE( hwndLB );
  SETFOCUSHWND( hwndLB );

  return ( fOK );
} /* end of function EQFBDelAll */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBQueryLineHeight
*/
// Description:
//    Returns the line height of the default font
//
//   Flow:
//       if static variable usEQFBLineHeight is not set
//         query font metrics of default font;
//         set static variable usEQFBLineHeight to lExternalLeading value of
//         default font;
//       endif;
//       return static variable usEQFBLineHeight
//
// Arguments:
//  None
//
// Returns:
//  USHORT usLineHeight             - height of default font
//
// Prereqs:
//   None
//
// SideEffects:
//   if not already done, the static variable usEQFBLineHeight is set
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
static USHORT EQFBQueryLineHeight( VOID )
{
   if ( !usEQFBLineHeight )            // if no line height computed yet ...
   {
     GETLINEHEIGHT( HWND_DESKTOP, usEQFBLineHeight );
   } /* endif */
   return( usEQFBLineHeight );
} /* endof EQFBQueryLineHeight */

/**********************************************************************/
/* create the translation memory if it does not exist....             */
/**********************************************************************/
BOOL CreateTMIfNotExist
(
  HWND  hwndDlg,
  PITMIDA pITMIda,
  PSZ pchName
)
{
  BOOL fExist = TRUE;
  SHORT lItem;
  lItem = CBSEARCHITEM( hwndDlg, ID_ITM_TRANSMEM_CB, pchName );
  if( lItem == LIT_NONE)
  {
     //--- call the memory handler to create a new TM ---
     strcpy( pITMIda->szBuffer, pchName );
     strcat( pITMIda->szBuffer, X15_STR );
     strcat( pITMIda->szBuffer, pITMIda->chTagTableName );
     strcat( pITMIda->szBuffer, X15_STR );
     strcat( pITMIda->szBuffer, pITMIda->szSourceLang);
     strcat( pITMIda->szBuffer, X15_STR );
     strcat( pITMIda->szBuffer, pITMIda->szTargetLang);
     strcat( pITMIda->szBuffer, X15_STR );
     fExist = (BOOL) EqfSend2Handler( MEMORYHANDLER,
                           WM_EQF_CREATE,
                           MP1FROMSHORT(0),
                           MP2FROMP( pITMIda->szBuffer ) );
     if ( fExist )
     {
       strcpy( pchName, pITMIda->szBuffer );
     } /* endif */
  };
  return fExist;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     ITMFileExist           Check if a file exists            |
//+----------------------------------------------------------------------------+
//|Description:       Checks if a file exists.                                 |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL ITMFileExist    ( PSZ pszFileName )                 |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFileName is the fully qualified file name           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   file exists                                       |
//|                   FALSE  file does not exist                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirstLong to check if file exists            |
//|                   close file search handle                                 |
//|                   return result                                            |
//+----------------------------------------------------------------------------+
BOOL ITMFileExist( PSZ pszFile )
{
    LONGFILEFIND    ResultBuf;           // DOS file find struct
    HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle
    USHORT  usDosRC;                     // return code of Dos... alias Utl...

    usDosRC = UtlFindFirstLong( pszFile, &hDirHandle,
                                FILE_NORMAL,
                                &ResultBuf, FALSE );
    UtlFindCloseLong( hDirHandle, FALSE );

    if ( usDosRC == NO_ERROR )
    {
      PSZ pFileName = UtlGetFnameFromPath( pszFile );
      if (pFileName)
      {
        strcpy( pFileName, ResultBuf.achName );
      } /* endif */
    } /* endif */
    return( usDosRC == NO_ERROR );
} /* endof ITMFileExistLong */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     EQFITMPropFill     fill pstPropItm structure             |
//+----------------------------------------------------------------------------+
//|Description:       fills pstPropItm structure                               |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL EQFITMPropFill                                      |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                   - pITMIda itm ida                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   copy was fine                                     |
//|                   FALSE  one or more copy cannot be done, probably         |
//|                          due to size problems                              |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//+----------------------------------------------------------------------------+
BOOL
EQFITMPropFill
(
      PITMIDA       pITMIda
)
{
  BOOL   fOK = TRUE;

  if ( fOK && pITMIda->pstPropItm )
  {
    fOK = UtlCopyParameter(pITMIda->pstPropItm->chTagTable,
                           pITMIda->chTagTableName,
                           MAX_FNAME,
                           TRUE);
  } /* endif */

  if ( fOK && pITMIda->pstPropItm )
  {
    fOK = UtlCopyParameter(pITMIda->pstPropItm->szSourceLang,
                           pITMIda->szSourceLang,
                           sizeof(pITMIda->szSourceLang),
                           TRUE);
  } /* endif */
  if ( fOK && pITMIda->pstPropItm )
  {
    fOK = UtlCopyParameter(pITMIda->pstPropItm->szTargetLang,
                           pITMIda->szTargetLang,
                           sizeof(pITMIda->szTargetLang),
                           TRUE);
  } /* endif */
  if ( fOK && pITMIda->pstPropItm )
  {
    strcpy( pITMIda->pstPropItm->chLongTranslMemory, pITMIda->chLongTranslMemory );
    fOK = UtlCopyParameter(pITMIda->pstPropItm->chLongTranslMemory,
                           pITMIda->chTranslMemory,
                           MAX_LONGFILESPEC,
                           TRUE);
  } /* endif */

  if ( fOK && pITMIda->pstPropItm )
  {
    fOK = UtlCopyParameter(pITMIda->pstPropItm->chVersion,
                           ITM_VERSION4,
                           sizeof(pITMIda->pstPropItm->chVersion),
                           TRUE);

    fOK = UtlCopyParameter(pITMIda->pstPropItm->szSrcStartPath,
                           pITMIda->szSrcStartPath,
                           sizeof(pITMIda->szSrcStartPath),
                           TRUE);
    fOK = UtlCopyParameter(pITMIda->pstPropItm->szTgtStartPath,
                           pITMIda->szTgtStartPath,
                           sizeof(pITMIda->szTgtStartPath),
                           TRUE);
  } /* endif */


  return ( fOK );
}


//
// handle WM_SIZE message
//
MRESULT ITMDlgHandleResize( HWND hwndDlg, WPARAM mp1, LPARAM mp2 )
{
  MRESULT       mResult = MRFROMSHORT(TRUE);     // TRUE = cmd is processed
  PITMDLGIDA    pIda = ACCESSDLGIDA(hwndDlg, PITMDLGIDA);

  if ( (pIda != NULL) && ((mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN)) )
  {
    SHORT   sWidth  = LOWORD( mp2 );      // new width of dialog
    SHORT   sHeight = HIWORD( mp2 );      // new height of dialog
    RECT    rcDlg;                        // dialog rectangle 
    LONG   lBorderSize  = WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
    int iVarHeight = (sHeight - pIda->iFixedAreaHeight) / 2;

    LONG   cxAvail = sWidth - (2 * lBorderSize);
    LONG   xMiddle = (cxAvail / 2) + lBorderSize;


    GetWindowRect( hwndDlg, &rcDlg );

    // re-arrange controls
    {
      {
        RECT rect;
        RECT rectNew;

        int aiButtonGroup1[] = { ID_ITM_ADDTOLIST_PB, ID_ITM_ADDALLTOLIST_PB, ID_ITM_DELINLIST_PB, ID_ITM_DELALL_PB, - 1};
        int aiButtonGroup2[] = { ID_ITM_VISUAL_PB, ID_ITM_ALIGN_PB, ID_ITM_PREPARE_PB, ID_ITM_CANCEL_PB, ID_ITM_HELP_PB, - 1};

        // we re-position/re-size all dialog controls ...
        HDWP hdwp = BeginDeferWindowPos( 35 );

        int iComboboxWidth = xMiddle - pIda->iMarkupCBXOffset - 15;
        int iComboboxPos = xMiddle + pIda->iMarkupCBXOffset - 10;
        int iFieldsWidth = xMiddle - 20;
        int iFieldsOffs =  10;
        int curYPos = 0;
        int iSelectionGBTop = 0;

        memset( &rectNew, 0, sizeof(rectNew) );

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TRANSMEM_CB, -1, -1, iComboboxWidth, -1, &rectNew );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_MARKUP_STATIC, xMiddle + 10, -1, -1, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_MARKUP_CB, iComboboxPos, -1, iComboboxWidth, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_SRCLANG_CB, -1, -1, iComboboxWidth, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TGTLANG_STATIC, xMiddle + 10, -1, -1, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TGTLANG_CB, iComboboxPos, -1, iComboboxWidth, -1, &rectNew  );

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_SELECTION_GB, -1, rectNew.bottom, cxAvail - 5, -1, &rectNew   );
        iSelectionGBTop = rectNew.top;

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_SRCSTARTPATH_EF, -1, -1, iComboboxWidth, -1, &rectNew  );


        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_CURDIRSRC_EF, -1, -1, iFieldsWidth, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_CURDIRSRC_BROWSE_PB, 
                                                rectNew.right - pIda->iBrowsePBWidth, -1, -1, -1, &rectNew  );

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TGTSTARTPATH_STATIC, xMiddle + 10, -1, -1, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TGTSTARTPATH_EF, iComboboxPos, -1, iComboboxWidth, -1, &rectNew  );

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_SOURCE_EF, -1, -1, iComboboxWidth, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TARGET_STATIC, xMiddle + 10, -1, -1, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TARGET_EF, iComboboxPos, -1, iComboboxWidth, -1, &rectNew  );

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_SRCFILE_LB, -1, -1, iFieldsWidth, iVarHeight, &rectNew  );

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_SELECTION_GB, -1, -1, cxAvail - 5, rectNew.bottom - iSelectionGBTop +10, &rectNew   );


        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_CURDIRTGT_STATIC, xMiddle + iFieldsOffs, -1, -1, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_CURDIRTGT_EF, xMiddle + iFieldsOffs, -1, iFieldsWidth, -1, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_CURDIRTGT_BROWSE_PB, 
                                                rectNew.right - pIda->iBrowsePBWidth, -1, -1, -1, &rectNew  );

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TGTFILE_LB, xMiddle + iFieldsOffs, -1, iFieldsWidth, iVarHeight, &rectNew  );
        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_TGTFILE_STATIC, rectNew.left, -1, -1, -1, &rectNew  );

        // compute Y position of file list
        GetWindowRect( GetDlgItem( hwndDlg, ID_ITM_TGTFILE_LB ), &rect );
        MapWindowPoints( HWND_DESKTOP, hwndDlg, (LPPOINT)&rect, 2 );

        curYPos = rect.top + iVarHeight;

        if ( hdwp ) hdwp = ITMDlgAlignButtons( hwndDlg, hdwp, aiButtonGroup1, 1, cxAvail, curYPos + pIda->iButtonArea1Offs );

        curYPos += pIda->iButtonArea1Space;

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_FILELIST_STATIC, -1, curYPos, -1, -1, &rectNew  );

        curYPos += pIda->iFileListOffs;

        if ( hdwp ) hdwp = ITMDlgSetNewSizePos( hwndDlg, hdwp, ID_ITM_FILELIST_LB, -1, curYPos, (iFieldsWidth + iFieldsOffs) * 2, iVarHeight, &rectNew  );

        curYPos += iVarHeight;

        if ( hdwp ) hdwp = ITMDlgAlignButtons( hwndDlg, hdwp, aiButtonGroup2, 1, cxAvail, curYPos + pIda->iButtonArea2Offs );

#ifdef xxxxxxxxxxxxxxxxxx
        // re-arrange pushbuttons
        for ( i = 0; i < 5; i++ )
        {
          lTotSize += pIda->sButtonWidth[i];
        } /* endfor */

        lTotGaps = (cxAvail > lTotSize) ? (cxAvail - lTotSize) : 0;
        lGap = lTotGaps / 5;
        lCorrect = (cxAvail > lTotSize) ? ((cxAvail - (lGap * 5) - lTotSize) / 2) : 0;
        lXPos    = pIda->sBorderSize;
        for ( i = 0; (i < 5) && (hdwp != NULL); i++ )
        {
          lXPos += i ? (pIda->sButtonWidth[i-1] + lGap) : ((lGap / 2) + lCorrect);
          hdwp = DeferWindowPos( hdwp, pIda->hwndButton[i], HWND_TOP, lXPos,
                                sHeight - pIda->sBorderSize - pIda->sButtonHeight,
                                pIda->sButtonWidth[i], pIda->sButtonHeight,
                                SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
        } /* endfor */

        // resize find text and replace text entry fields
        {
          RECT rect;

          GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_FIND_EF ), &rect );
          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_FIND_EF ),
                                HWND_TOP, 0, 0,
                                (cxAvail > (rect.left + 3)) ? (cxAvail - rect.left - 3) : 0,
                                rect.bottom - rect.top,
                                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

          GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_CHANGE_EF ), &rect );
          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_CHANGE_EF ),
                                HWND_TOP, 0, 0,
                                (cxAvail > (rect.left + 3)) ? (cxAvail - rect.left - 3) : 0,
                                rect.bottom - rect.top,
                                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
        }

        // resize documents listbox and groupbox
        {
          RECT rect;

          GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_DOCS_GB ), &rect );
          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_DOCS_GB ),
                                HWND_TOP, 0, 0,
                                (cxAvail > (rect.left + 3)) ? (cxAvail - rect.left - 3) : 0,
                                rect.bottom - rect.top,
                                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
          yTextStartPos = rect.bottom + 2;

          GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_DOCS_LB ), &rect );
          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_DOCS_LB ),
                                HWND_TOP, 0, 0,
                                (cxAvail > (rect.left + 5)) ? (cxAvail - rect.left - 5) : 0,
                                rect.bottom - rect.top,
                                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
        }

        // set y position for text MLE
        {
          RECT rect;

          GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_WILDCARDSINGLE_TEXT ), &rect );
          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
          yTextStartPos = rect.bottom + 2;
        }

        // resize text entryfield and text groupbox
        {
          RECT rect;
          LONG lYDistance;

          // get vertical displacement of text MLE to text groupbox
          GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_TEXT_MLE ), &rect );
          lYDistance = rect.top;
          GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_TEXT_GB ), &rect );
          lYDistance -= rect.top;

          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );

          // resize and reposition text groupbox
          rect.top = yTextStartPos;
          rect.bottom = sHeight - pIda->sBorderSize - pIda->sButtonHeight;
          rect.left = pIda->sBorderSize;
          rect.right = cxAvail - (2 * pIda->sBorderSize);

          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_TEXT_GB ),
                                HWND_TOP,
                                rect.left, rect.top,
                                rect.right - rect.left,
                                rect.bottom - rect.top,
                                SWP_NOACTIVATE | SWP_NOZORDER );

          // resize and reposition text entryfield
          rect.top    += lYDistance;
          rect.bottom -= 4;
          rect.left   += 4;
          rect.right  -= 4;
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_TEXT_MLE ),
                                HWND_TOP,
                                rect.left, rect.top,
                                rect.right - rect.left,
                                rect.bottom - rect.top,
                                SWP_NOACTIVATE | SWP_NOZORDER );
        }
#endif

        // do actual dialog control re-positioning
        if ( hdwp != NULL ) EndDeferWindowPos( hdwp );
      }
    }
  } /* endif */
  return( mResult );
} /* end of function ITMDlgHandleResize */


//
// get y offset of control 2 relative to control 1
//
int ITMDlgGetYOffset( HWND hwnd, int iControl1, int iControl2 )
{
  RECT rect1, rect2;

  memset( &rect1, 0, sizeof(rect1) );
  memset( &rect2, 0, sizeof(rect2) );
  GetWindowRect( GetDlgItem( hwnd, iControl1 ), &rect1 );
  GetWindowRect( GetDlgItem( hwnd, iControl2 ), &rect2 );
  return( rect2.top - rect1.top );
} /* end of function ITMDlgSetNewXPos */

//
// get x offset of control 2 relative to control 1
//
int ITMDlgGetXOffset( HWND hwnd, int iControl1, int iControl2 )
{
  RECT rect1, rect2;

  memset( &rect1, 0, sizeof(rect1) );
  memset( &rect2, 0, sizeof(rect2) );
  GetWindowRect( GetDlgItem( hwnd, iControl1 ), &rect1 );
  GetWindowRect( GetDlgItem( hwnd, iControl2 ), &rect2 );
  return( rect2.left - rect1.left );
} /* end of function ITMDlgSetNewXPos */

//
// get heigth of free area between control 1 and control 2
//
int ITMDlgGetVertDistance( HWND hwnd, int iControl1, int iControl2 )
{
  RECT rect1, rect2;

  memset( &rect1, 0, sizeof(rect1) );
  memset( &rect2, 0, sizeof(rect2) );
  GetWindowRect( GetDlgItem( hwnd, iControl1 ), &rect1 );
  GetWindowRect( GetDlgItem( hwnd, iControl2 ), &rect2 );
  return( rect2.top - rect1.bottom );
} /* end of function ITMDlgSetNewXPos */

//
// get width of a control
//
int ITMDlgGetWidth( HWND hwnd, int iControl )
{
  RECT rect;

  memset( &rect, 0, sizeof(rect) );
  GetWindowRect( GetDlgItem( hwnd, iControl ), &rect );
  return( rect.right - rect.left );
} /* end of function ITMDlgGetWidth */


//
// set new size and pos for a control (-1 in one of the size fields = do not change value)
//
HDWP ITMDlgSetNewSizePos( HWND hwnd, HDWP hdwp, int ID, int x, int y, int cx, int cy, PRECT pRectNew    )
{
  RECT rect;
  HWND hwndControl = GetDlgItem( hwnd, ID );
  UINT uiFlags = SWP_NOACTIVATE | SWP_NOZORDER;

  if ( (x == -1) && (y == -1 ) )    uiFlags |= SWP_NOMOVE; 
  if ( (cx == -1) && (cy == -1 ) )  uiFlags |= SWP_NOSIZE; 

  GetWindowRect( hwndControl, &rect );
  MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
  if ( x == -1 )  x = rect.left;
  if ( y == -1 )  y = rect.top;
  if ( cx == -1 ) cx = rect.right - rect.left;
  if ( cy == -1 ) cy = rect.bottom - rect.top;

  pRectNew->top  = y;
  pRectNew->left = x;
  pRectNew->bottom = y + cy;
  pRectNew->right = x + cx;

  hdwp = DeferWindowPos( hdwp, hwndControl, HWND_TOP, x, y, cx, cy, uiFlags );
  return( hdwp );
} /* end of function ITMDlgSetNewSizePos */

//
// posiion control relative to another control
//
HDWP ITMDlgPosAligned( HWND hwnd, HDWP hdwp, int iControlID, int iAlignID, int mode, int dx, int dy, PRECT pRectNew    )
{
  RECT rect, rectAlign;
  HWND hwndControl = GetDlgItem( hwnd, iControlID );

  int x = -1;
  int y = -1;

  GetWindowRect( hwndControl, &rect );
  MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
  GetWindowRect( GetDlgItem( hwnd, iAlignID ), &rectAlign );
  MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rectAlign, 2 );

  if ( mode & ALIGN_LEFT_MODE )
  {
    x = rectAlign.left;
  }
  else if ( mode & ALIGN_RIGHT_MODE )
  {
    x = rectAlign.right - (rect.right - rect.left);
  } /* endf */

  if ( mode & ALIGN_TOP_MODE )
  {
    y = rectAlign.top;
  }
  else if ( mode & ALIGN_BOTTOM_MODE )
  {
    y = rectAlign.bottom - (rect.bottom - rect.top);
  } /* endif */

  if ( x == -1 )  x = rect.left;
  if ( y == -1 )  y = rect.top;

  pRectNew->top  = y;
  pRectNew->left = x;
  pRectNew->bottom = y + (rect.bottom - rect.top);
  pRectNew->right = x + (rect.right - rect.left);

  hdwp = DeferWindowPos( hdwp, hwndControl, HWND_TOP, x + dx, y + dy, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE );  
;
  return( hdwp );
} /* end of function ITMDlgPosAligned */


//
// align buttons in given space
//
HDWP ITMDlgAlignButtons( HWND hwnd, HDWP hdwp, int *piButtons, int x, int cx, int y  )
{
  int iButtonNum = 0;                  // number of buttons being aligned
  int iTotalWidth = 0;                 // total width of button
  int i = 0;
  int iTotalGaps = 0;                  // total room available for gaps
  int iGap = 0;                        // gap between two buttons
  int iCorrect = 0;                    // correction value for gaps


  // compute width of buttons
  i = 0;
  while( piButtons[i] != -1 )
  {
    RECT rect;
    HWND hwndControl = GetDlgItem( hwnd, piButtons[i] );

    GetWindowRect( hwndControl, &rect );

    iButtonNum++;
    iTotalWidth += rect.right - rect.left;
    i++;
  } /*endwhile */

  iTotalGaps = (cx > iTotalWidth) ? (cx - iTotalWidth) : 0;
  iGap = iTotalGaps / iButtonNum;
  iCorrect = (cx > iTotalWidth) ? ((cx - (iGap * iButtonNum) - iTotalWidth) / 2) : 0;

  for ( i = 0; (i < iButtonNum) && (hdwp != NULL); i++ )
  {
    RECT rect;
    HWND hwndControl = GetDlgItem( hwnd, piButtons[i] );
    int iButtonWidth = 0;

    GetWindowRect( hwndControl, &rect );
    iButtonWidth = rect.right - rect.left;

    x += (i != 0) ? iGap : ((iGap / 2) + iCorrect);
    hdwp = DeferWindowPos( hdwp, hwndControl, HWND_TOP, x, y, rect.right - rect.left,
                           rect.bottom - rect.top, 
                           SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
    x += iButtonWidth;
  } /* endfor */

  return( hdwp );
} /* end of function ITMDlgAlignButtons */

//The call back is used to set the initial directory when using SHBrowseForFolder
int CALLBACK ItmBrowseForFolderCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
  lp;

  switch(uMsg)
  {
    case BFFM_INITIALIZED:
        //Initial directory is set here
        {
            // WParam is TRUE since you are passing a path.
            // It would be FALSE if you were passing a pidl.
            SendMessage(hwnd,BFFM_SETSELECTION,TRUE,pData);
        }
        break;
    default:
        break;
  }
  return 0;
}



BOOL ItmDlgBrowse( HWND hwnd, PSZ pszBuffer, PSZ pszTitle )
{
  BOOL fOK = FALSE;
  BROWSEINFO bi;
  LPITEMIDLIST pidlBrowse;    // PIDL selected by user

  bi.hwndOwner = hwnd;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = pszBuffer;
  bi.lpszTitle = pszTitle;
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

  // if ItmBrowseForFolderCallbackProc is specified as callback the lParam
  // parameter must contain the initial folder directory!!!
  bi.lpfn = ItmBrowseForFolderCallbackProc;
  bi.lParam = (LPARAM)pszBuffer;

  // Browse for a folder and return its PIDL.
  pidlBrowse = SHBrowseForFolder(&bi);

  if (pidlBrowse != NULL)
  {
    // get the selected directory path
    SHGetPathFromIDList( pidlBrowse, pszBuffer );

    fOK = TRUE;

    // Free the PIDL returned by SHBrowseForFolder.
    {
      LPMALLOC pMalloc;
      SHGetMalloc( &pMalloc );
#ifdef __cplusplus
      pMalloc->Free(pidlBrowse );
#else
      pMalloc->lpVtbl->Free(pMalloc, pidlBrowse );
#endif
    }
  } /* endif */
  return( fOK );
} /* end of function DocExpBrowse */

//
// add all matching file pairs
//
int ItmAddAllMatchingFiles( HWND hwndDlg, PITMDLGIDA pIda )
{
  SHORT sItem = 0;
  BOOL  fOK = TRUE;
  int iAddedPairs = 0;
  SHORT sItemCount = 0;
  HWND hwndLB = GETHANDLEFROMID( hwndDlg, ID_ITM_FILELIST_LB );

  ENABLEUPDATEHWND_FALSE( hwndLB );

  // get number of items in source listbox
  sItemCount = QUERYITEMCOUNT( hwndDlg, ID_ITM_SRCFILE_LB );

  // get current source and target dir
  QUERYTEXT( hwndDlg, ID_ITM_CURDIRSRC_EF, pIda->SrcFile.szPath );
  QUERYTEXT( hwndDlg, ID_ITM_CURDIRTGT_EF, pIda->TgtFile.szPath );

  // loop through files in source listbox and find matching entry in target listbox
  while ( fOK && (sItem < sItemCount) )
  {
    CHAR szName[MAX_LONGFILESPEC];

    // get current file name
    QUERYITEMTEXT( hwndDlg, ID_ITM_SRCFILE_LB, sItem, szName );

    // add to file pairs if same file is in target listbox
    if ( SEARCHITEM( hwndDlg, ID_ITM_TGTFILE_LB, szName ) >= 0 )
    {
      BOOL fExists = FALSE;
      SHORT sPairs = 0;
      SHORT i = 0;

      strcpy( pIda->SrcFile.szFileName, pIda->SrcFile.szPath );
      strcat( pIda->SrcFile.szFileName, szName );
      strcpy( pIda->TgtFile.szFileName, pIda->TgtFile.szPath );
      strcat( pIda->TgtFile.szFileName, szName );

      sprintf( pIda->szFile1, "  %s\t%s", pIda->SrcFile.szFileName, pIda->TgtFile.szFileName );

      // run through listbox and check if this entry exists already
      sPairs = QUERYITEMCOUNTHWND( hwndLB );
      i = 0;
      while ( !fExists && (i < sPairs) )
      {
        QUERYITEMTEXTHWND( hwndLB, i, pIda->szFile2 );
        if ( strcmp( pIda->szFile1, pIda->szFile2  ) == 0 )
        {
          fExists = TRUE;
        } /* endif */
        i++;
      } /*endwhile */

      if ( !fExists )
      {
         INSERTITEMENDHWND( hwndLB, pIda->szFile1 );
         iAddedPairs++;
      } /* endif */
    } /* endif */

    // next item
    sItem++;
  } /*endwhile */


  ENABLEUPDATEHWND_TRUE( hwndLB );

  if ( iAddedPairs != 0 )
  {
    ENABLECTRL( hwndDlg, ID_ITM_ALIGN_PB,  TRUE );
    ENABLECTRL( hwndDlg, ID_ITM_VISUAL_PB, TRUE );
    ENABLECTRL( hwndDlg, ID_ITM_DELALL_PB, TRUE );
    ENABLECTRL( hwndDlg, ID_ITM_PREPARE_PB, TRUE );
  } /* endif */

  return( iAddedPairs );
}
