/*! \file
	Description: Object handler and instance for Counting Report

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#ifdef _DEBUG
  // activate the following define to log calculating report window size handling
  // #define CALCRPT_WNDSIZE_LOGGING
#endif



#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_WCOUNT           // word count functions
#define INCL_EQF_TP               // public translation processor functions

#include "eqf.h"                  // general TranslationManager include file
#include "eqfhlog.h"              // defines and structures of history log file
#include "eqfdde.h"               // batch mode definitions
#include <eqffol00.h>             //subfolder stuff


#include "eqfdde.h"

#include "OTMFUNC.H"            // function call interface public defines
#include "eqffunci.h"           // function call interface private defines

#include "eqfwcnti.h"             // private include file for wordcount
#include "eqfcnt01.id"



#include "eqfrpt.h"               // public include file for report
#include "eqfrpt.id"              // private id's for report
#include "eqfrpt00.h"             // private include file for report

// #include "eqfrpt03.h"             // private include file for report

// function prototypes
BOOL RPTXMLREPORT( BOOL bDde, HWND hwnd, PRPT pRpt);

// MRESULT RptListCallBack (PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM);

#define MAX2_OPTIONS4                       73  // # of currencies

// local currencies

typedef CHAR LOCAL_CURRENCY[5];

LOCAL_CURRENCY szRptCurrency2[MAX2_OPTIONS4] = {"ARP",
                                               "AUD",
                                               "ATS",
                                               "BSD",
                                               "BBD",
                                               "BEF",
                                               "BMD",
                                               "BRR",
                                               "BGL",
                                               "CAD",
                                               "CHF",
                                               "CLP",
                                               "CNY",
                                               "CYP",
                                               "CSK",
                                               "DEM",
                                               "DKK",
                                               "DZD",
                                               "EEK",  // XJR:KBT1011;Estonia
                                               "EGP",
                                               "EUR",
                                               "FJD",
                                               "FIM",
                                               "FRF",
                                               "GBP",
                                               "GRD",
                                               "HKD",
                                               "HUF",
                                               "ISK",
                                               "INR",
                                               "IDR",
                                               "IEP",
                                               "ILS",
                                               "ITL",
                                               "JMD",
                                               "JOD",
                                               "JPY",
                                               "LBP",
                                               "LTL",  //XJR:KBT1011:Lithuania
                                               "LUF",
                                               "LVL",  //XJR: KBT1011: Latvia
                                               "MYR",
                                               "MXP",
                                               "NLG",
                                               "NZD",
                                               "NOK",
                                               "PKR",
                                               "PHP",
                                               "PLZ",
                                               "PTE",
                                               "ROL",
                                               "SUR",
                                               "SAR",
                                               "SGD",
                                               "SKK",
                                               "ZAR",
                                               "KRW",
                                               "ESP",
                                               "SDD",
                                               "SEK",
                                               "TWD",
                                               "THB",
                                               "TTD",
                                               "TRL",
                                               "USD",
                                               "VEB",
                                               "ZMK",
                                               "XEC",
                                               "XEU",
                                               "XDR",
                                               "XAG",
                                               "XAU",
                                               "XPT" };


USHORT RptBatchCount
(
HWND             hwndParent,         // handle of count handler window
PDDECNTRPT       pCntRpt             // wordcount data structure
);

static MRESULT
RptListCallBack_HandleWMCommand
(
	HWND             hwnd,
    WPARAM           mp1
);

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:    ReportListHandlerCallBack
//------------------------------------------------------------------------------
// Function call:    ReportListHandlerCallBack (pCommArea, hwnd, msg, mp1, mp2)
//------------------------------------------------------------------------------
// Description:      Object handler and instance window procedure for
//                   Counting Report
//------------------------------------------------------------------------------
// Parameters:       PHANDLERCOMMAREA pCommArea
//                   HWND             hwnd
//                   WINMSG           msg
//                   WPARAM           mp1
//                   LPARAM           mp2
//------------------------------------------------------------------------------
// Returncode type:  MRESULT
//------------------------------------------------------------------------------
// Function flow:    See Generic Handler documentation
//------------------------------------------------------------------------------

MRESULT ReportListHandlerCallBack
(
PHANDLERCOMMAREA pCommArea,
HWND             hwnd,
WINMSG           msg,
WPARAM           mp1,
LPARAM           mp2
)
{
    MRESULT mResult = MRFROMSHORT(FALSE);  // return code of function
    USHORT  usReturn;                      // return code of dialog
    BOOL    fOk;                           // error indicator
    PRPT    pRpt = NULL;                   // pointer to RPT structure

    switch (msg)
    {
    // WM_CREATE: fill variables of communication area
    case WM_CREATE :
        pCommArea->pfnCallBack          = RptListCallBack;  // pointer to callback function
        strcpy (pCommArea->szHandlerName, REPORTHANDLER);   // name of handler
        pCommArea->sBaseClass           = clsREPORT;         // base class

        pCommArea->sListWindowID        = ID_CNT_HISTREPORT_LW;
        pCommArea->sListboxID           = DID_HISTRPT_OUT_LB;

        // Define object classes to be notified for EQFN messages
        pCommArea->asNotifyClassList[0] = clsREPORT;
        pCommArea->asNotifyClassList[1] = 0;       // end of list

        // Define additional messages processed by the callback function
        pCommArea->asMsgsWanted[0]      = 0;       // end of list
        break;


    case WM_EQF_CREATE:
        // mp1: SHORT1 = open flag  SHORT2 = NULL
        // mp2: object name

        // allocate storage for RPT structure passed to report dialog
        // and report instance, if fails display error message
        pRpt = NULL;
        usReturn = 0;
        fOk = UtlAlloc ((PVOID*)&pRpt, 0L, (ULONG)sizeof (RPT), ERROR_STORAGE);
        if ( fOk )
        {
          fOk = UtlAlloc( (PVOID *)&(pRpt->pszActualDocument), 0L, (ULONG)MAX_FILELIST, ERROR_STORAGE );
        } /* endif */

        if (fOk)   // processing ok so far
        {
            // save pRpt pointer in communication area
            pCommArea->pUserIDA = pRpt;

            // save folder object name to RPT structure
            strcpy (pRpt->szFolderObjName, (PSZ) mp2);
            strcpy (pRpt->szParentObjName, (PSZ) mp2);

            if ( FolIsSubFolderObject( pRpt->szFolderObjName ) )
            {
               // cut off subfolder name and property directory to form folder object name
               UtlSplitFnameFromPath( pRpt->szFolderObjName );
               UtlSplitFnameFromPath( pRpt->szFolderObjName );
            } /* endif */

            // save folder name to RPT structure
            strcpy (pRpt->szFolderName, UtlGetFnameFromPath (pRpt->szFolderObjName));

            // save folder name without extension to RPT structure
            Utlstrccpy( pRpt->szFolder, pRpt->szFolderName, DOT );

            // get folder long name and store it in RPT structure
            SubFolObjectNameToName( pRpt->szParentObjName, pRpt->szLongFolderName );

            pRpt->usRptStatus = 1; // set report state

            // save report instance object name to RPT structure, that is :
            // COUNTHANDLER (#define) concatenated with folder objectname
            strcpy( pRpt->szRptInstanceObjName, REPORTHANDLER );
            strcat( pRpt->szRptInstanceObjName, pRpt->szFolderObjName);

            // if object with this object name and class report exists
            {
                HWND hFrame = EqfQueryObject (pRpt->szRptInstanceObjName, clsREPORT, 0);

                if ( hFrame )
                {
                    // issue error message "count window already open"
                    UtlError( ERROR_CALC_WINDOW_OPEN, MB_CANCEL, 0, NULL, EQF_WARNING );

                    ActivateMDIChild( hFrame );
                    fOk = FALSE;                        // set fOk to FALSE to stop processing
                } // end if
            }
        } // end if

        if (fOk)
        {
            // check, if histlog.dat already exists
            // build absolut path for histlog record
            UtlMakeEQFPath( pRpt->szWorkString, pRpt->szFolderObjName[0],
                            PROPERTY_PATH,
                            UtlGetFnameFromPath(pRpt->szFolderObjName) );
            strcat( pRpt->szWorkString, BACKSLASH_STR );
            strcat( pRpt->szWorkString, HISTLOGFILE);

            fOk = UtlFileExist( pRpt->szWorkString );

            if ( !fOk )
            {
                PSZ pszTmp = pRpt->szFolder;
                UtlErrorHwnd (MESSAGE_RPT_NO_HISTLOG_AVAIL, MB_OK, 1,
                              &pszTmp, EQF_INFO, hwnd);
            } // end if
        } // end if

        if (fOk)
        {
            HWND hwndLB = NULLHANDLE;           // handle of invisible LB

            // create invisible LB for selected documents
            hwndLB = WinCreateWindow (hwnd, WC_LISTBOX, "",  // parent handle, ClassName, name
                                      LBS_MULTIPLESEL,       // style
                                      0, 0, 0, 0,            // x-, y-coordinate, width, heigth
                                      hwnd, HWND_TOP,        // owner, behind
                                      DID_RPT_INVISIBLE_LB,  // ID
                                      NULL, NULL);

            // update handler of invisible listbox in RPT structure
            pRpt->hwndRptHandlerLB = hwndLB;
        } // end if

        if (fOk && !pRpt->fBatch)   // processing ok so far
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

          // start Counting Report dialog
          BOOL fReturn = 0;
          DIALOGBOX( EqfQueryTwbClient(),COUNTINGPROPDLGPROC, hResMod, ID_RPTPROP_DLG, pRpt, fReturn  );
          usReturn = (USHORT)fReturn;
        } // end if

        if ( fOk && usReturn && !pRpt->fBatch)
        {
            // -----------------------------------
            // start list window for report output
            // -----------------------------------
            fOk = CreateListWindow (pRpt->szRptInstanceObjName,
                                    RptListCallBack, (PVOID)pRpt, FALSE);
        }

        break;


    case  WM_EQF_DDE_REQUEST:
        /************************************************************/
        /*     mp1:  (DDETASK) Task                                 */
        /*     mp2:  (PVOID) pTaskIda                               */
        /************************************************************/
        switch ( SHORT1FROMMP1( mp1 ) )
        {
        case  TASK_CNTRPT:
            {
                PDDECNTRPT pCntRpt = (PDDECNTRPT)PVOIDFROMMP2(mp2);
                RptBatchCount( hwnd, pCntRpt );
            }
            break;
        default :
            break;
        } /* endswitch */
        break;


    case WM_DESTROY:
        // do nothing
        break;


    default:
        break;
    } // end switch

    return(mResult);
} // end of ReportListHandlerCallBack


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:    RptListCallBack
//------------------------------------------------------------------------------
// Function call:    RptListCallBack (pCommArea, hwnd, msg, mp1, mp2)
//------------------------------------------------------------------------------
// Description:      Callback function for Counting Report
//------------------------------------------------------------------------------
// Parameters:       PLISTCOMMAREA pCommArea
//                   HWND          hwnd
//                   WINMSG        msg
//                   WPARAM        mp1
//                   LPARAM        mp2
//------------------------------------------------------------------------------
// Returncode type:  MRESULT
//------------------------------------------------------------------------------
// Function flow:    See Generic Handler documentation
//------------------------------------------------------------------------------
MRESULT RptListCallBack
(
PLISTCOMMAREA    pCommArea,
HWND             hwnd,
WINMSG           msg,
WPARAM           mp1,
LPARAM           mp2
)
{
    MRESULT mResult = MRFROMSHORT(FALSE);  // return code of function
    BOOL    fOk = TRUE;                    // error indicator
    PRPT    pRpt = 0;                      // pointer to RPT structure
    SHORT   sSize = 0;                     // width of list window
	BOOL	bDde = TRUE;

	PPROPSYSTEM  pPropSys= NULL;           // buffer for system properties

    switch (msg)
    {
    case WM_CREATE :
        // mp2 contains pointer to RPT structure
        pRpt = (PRPT)mp2;
        pRpt->hwndErrMsg = hwnd;

        // set column listbox title string
        sprintf (szListWindowTitle, "%s - %s", pRpt->szReport, pRpt->szOption);

        // supply all information required to create a report list
        if (fOk)
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
#ifdef CALCRPT_WNDSIZE_LOGGING
            FILE *hfLog = NULL;
            CHAR szLogFile[MAX_EQF_PATH];
#endif
            pCommArea->sListObjClass      = clsREPORT;         // class of list window
            LOADSTRING (NULLHANDLE, hResMod, SID_RPT_LW_TITLE, pCommArea->szTitle); // title of list window
            pCommArea->hIcon              = (HPOINTER) UtlQueryULong(QL_COUNTICON); //hiconCOUNT;        // icon
            pCommArea->fNoClose           = FALSE;             // close in system menue
            pCommArea->sObjNameIndex      = 0;
            pCommArea->sNameIndex         = 0;

            switch ( pRpt->usReport )
            {
            case HISTORY_REPORT :
                pCommArea->sListWindowID      = ID_CNT_HISTREPORT_LW;
                pCommArea->sListboxID         = DID_HISTRPT_OUT_LB;
                break;
            case SUMMARY_COUNTING_REPORT :
                pCommArea->sListWindowID      = ID_CNT_SUMREPORT_LW;
                pCommArea->sListboxID         = DID_SUMRPT_OUT_LB;
                break;
            case CALCULATION_REPORT :
            default :
                pCommArea->sListWindowID      = ID_CNT_CALCREPORT_LW;
                pCommArea->sListboxID         = DID_CALCRPT_OUT_LB;
                break;

            } /* endswitch */

            sSize = (SHORT)WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);  // width

            pPropSys = GetSystemPropPtr();
            pCommArea->swpSizePos.x    = (SHORT) pPropSys->sCalcReportWinX;
            pCommArea->swpSizePos.y    = (SHORT) pPropSys->sCalcReportWinY;

            pCommArea->swpSizePos.cx    = (SHORT) pPropSys->sCalcReportWinCX;
            pCommArea->swpSizePos.cy    = (SHORT) pPropSys->sCalcReportWinCY;

#ifdef CALCRPT_WNDSIZE_LOGGING
            UtlMakeEQFPath( szLogFile, NULC, SYSTEM_PATH, NULL );
            strcat( szLogFile, "\\LOGS" );
            UtlMkDir( szLogFile, 0L, FALSE );
            strcat( szLogFile, "\\CLCRPTWND.LOG" );
            hfLog = fopen( szLogFile, "a" );
            if ( hfLog )
            {
              SHORT x,y;
              fprintf( hfLog, "***********************\n" );
              x = (SHORT)WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);  // width
              y = (SHORT)WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);
              fprintf( hfLog, "Reported screen resolution is %dx%d\n", x, y );
              fprintf( hfLog, "Saved CalcWindowSize is (x=%d,y=%d,cx=%d,cy=%d)\n", pCommArea->swpSizePos.x,
                       pCommArea->swpSizePos.y, pCommArea->swpSizePos.cx, pCommArea->swpSizePos.cy );
            } /* endif */

#endif

            if (pCommArea->swpSizePos.x == 0) pCommArea->swpSizePos.x = 20;
            if (pCommArea->swpSizePos.y == 0) pCommArea->swpSizePos.y = 10;
			      if (pCommArea->swpSizePos.cx == 0) pCommArea->swpSizePos.cx = (sSize < MAX_O_LENGTH*10) ? (sSize-20) : (MAX_O_LENGTH*10);
		        if (pCommArea->swpSizePos.cy == 0) pCommArea->swpSizePos.cy = 310;

#ifdef CALCRPT_WNDSIZE_LOGGING
            if ( hfLog )
            {
              fprintf( hfLog, "Size after applying defaults is (x=%d,y=%d,cx=%d,cy=%d)\n", pCommArea->swpSizePos.x,
                       pCommArea->swpSizePos.y, pCommArea->swpSizePos.cx, pCommArea->swpSizePos.cy );
            } /* endif */

#endif


            // ensure that window is visible within our workbench
            {
              // caution: we work here with OS/2 screen coordinates; i.e. the lower left corner is ( 0, 0 )
              SWP   swpTWB;             // size+pos of translation workbench
              PSWP  pSwp = &(pCommArea->swpSizePos);   // short cut for window position and size

              WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );

              if ( pSwp->x < 0L )                     pSwp->x = 0L;
              if ( (pSwp->x + pSwp->cx) > swpTWB.cx ) pSwp->x = max( (swpTWB.cx - pSwp->cx), 0 );
              if ( (pSwp->x + pSwp->cx) > swpTWB.cx ) pSwp->cx = swpTWB.cx;

              if ( pSwp->y < 0L    )                  pSwp->y = 0;
              if ( (pSwp->y + pSwp->cy) > swpTWB.cy ) pSwp->y = max( (swpTWB.cy - pSwp->cy), 0 );
              if ( (pSwp->y + pSwp->cy) > swpTWB.cy ) pSwp->cy = swpTWB.cy;
            }

#ifdef CALCRPT_WNDSIZE_LOGGING
            if ( hfLog )
            {
              fprintf( hfLog, "Size after KeepInTWB is (x=%d,y=%d,cx=%d,cy=%d)\n", pCommArea->swpSizePos.x,
                       pCommArea->swpSizePos.y, pCommArea->swpSizePos.cx, pCommArea->swpSizePos.cy );
              fclose( hfLog );
            } /* endif */
#endif

            pCommArea->sPopupMenuID       = -1;
            pCommArea->sGreyedPopupMenuID = -1;
            pCommArea->sNoSelPopupMenuID  = -1;
            pCommArea->pColData           = &RptCLBCTLData;    // column LB control area
            pCommArea->fMultipleSel       = FALSE;
            pCommArea->sDefaultAction     = PID_FILE_MI_OPEN;
            pCommArea->sItemClass         = clsREPORT;         // object class
            pCommArea->sItemPropClass     = 0;                 // no property class
            pCommArea->pUserIDA           = pRpt;              // pointer to IDA of callback function
            pCommArea->fUserFlag          = FALSE;             // no user flag
            pCommArea->asMsgsWanted[0]    = 0;                 // ends list of messages } // end if

        } /* end if */

        // In case of errors set error return code
        if (!fOk )
        {
            mResult = MRFROMSHORT(DO_NOT_CREATE);
        } // end if

        break;

    case WM_CLOSE :
    case WM_EQF_TERMINATE :
        pRpt = (PRPT) pCommArea->pUserIDA;  // set pointer to RPT

        if (pRpt)
        {
            if ( (pRpt->usRptStatus == RPT_ACTIVE) &&
                 (msg == WM_EQF_TERMINATE) &&
                 !(SHORT1FROMMP1(mp1) & TWBFORCE) )
            {
                // building report is active
                pRpt->usRptStatus = RPT_KILL;  // set state = kill
                mResult = MRFROMSHORT (TRUE);  // do not close right now
            }
            else
            {
                mResult = MRFROMSHORT (FALSE);  // close right now
            } // end if

            // save current window position in win.ini
            {
       			pPropSys = GetSystemPropPtr();
            pPropSys->sCalcReportWinX    = (SHORT)pCommArea->swpSizePos.x ;
            pPropSys->sCalcReportWinY    = (SHORT)pCommArea->swpSizePos.y;

            pPropSys->sCalcReportWinCX   = (SHORT) pCommArea->swpSizePos.cx ;
            pPropSys->sCalcReportWinCY    = (SHORT)pCommArea->swpSizePos.cy ;
#ifdef CALCRPT_WNDSIZE_LOGGING
            {
              FILE *hfLog = NULL;
              CHAR szLogFile[MAX_EQF_PATH];
              UtlMakeEQFPath( szLogFile, NULC, SYSTEM_PATH, NULL );
              strcat( szLogFile, "\\LOGS" );
              UtlMkDir( szLogFile, 0L, FALSE );
              strcat( szLogFile, "\\CLCRPTWND.LOG" );
              hfLog = fopen( szLogFile, "a" );
              if ( hfLog )
              {
                fprintf( hfLog, "Saving CalcWindowSize (x=%d,y=%d,cx=%d,cy=%d)\n", pCommArea->swpSizePos.x,
                        pCommArea->swpSizePos.y, pCommArea->swpSizePos.cx, pCommArea->swpSizePos.cy );
                fclose( hfLog );
              } /* endif */
            }
#endif

            }
        }
        else
        {
            mResult = MRFROMSHORT (FALSE);  // close right now
        } // end if

        break;

    case WM_DESTROY:
        {
          // free memory areas
          pRpt = (PRPT) pCommArea->pUserIDA;  // set pointer to RPT
          if ( pRpt )
          {
            if ( pRpt->pszActualDocument ) UtlAlloc( (PVOID *)&(pRpt->pszActualDocument), 0L, 0L, NOMSG );
            UtlAlloc ((PVOID*)&pRpt, 0L, 0L, NOMSG );;
            pCommArea->pUserIDA = NULL;
          } /* endif */
        }
        break;

    case WM_EQF_INITIALIZE:
        pRpt = (PRPT) pCommArea->pUserIDA;      // set pointer to RPT
        pRpt->usRptStatus = RPT_ACTIVE;  // set report state to active

        // start report
//        mResult = (MRESULT)RptMain (bDde, pCommArea->hwndLB, pCommArea->pUserIDA);
        mResult = (MRESULT)RPTXMLREPORT(bDde, pCommArea->hwndLB, (PRPT) pCommArea->pUserIDA);

        break;

// invoke File->open if this is the command
    case WM_COMMAND:
    case WM_EQF_COMMAND:
        mResult = RptListCallBack_HandleWMCommand( hwnd, mp1);
        break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
      {
		UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
        UtlMenuEnableItem( PID_FILE_MI_OPEN );
      }
      break;

    default:
        break;
    } // end switch

    return(mResult);
} // end of TagListCallBack



static MRESULT
RptListCallBack_HandleWMCommand
(
	HWND             hwnd,
    WPARAM           mp1
)
{   MRESULT mResult = MRFROMSHORT(FALSE);  // return code of function

	POPENANDPOS pOpen;
	CHAR        szWorkString[500];
	CHAR        szShort[MAX_FILESPEC];
	BOOL        fIsNew = FALSE;

	if (SHORT1FROMMP1(mp1) == PID_FILE_MI_OPEN  )  //os2
	{
		/***********************************************************************/
		/* Setup open-and-pos structure and pass request to document handler   */
		/***********************************************************************/

		if ( UtlAlloc( (PVOID *)&pOpen, 0L, (LONG)sizeof(OPENANDPOS), ERROR_STORAGE ) )
		{
      BOOL fValid = TRUE;              // TRUE = line is a valid folder/documentname
      PSZ  pFolder = NULL;             // position of folder
      PSZ  pSegNum = NULL;             // position of segment number
      PSZ  pDocName = NULL;            // position of document name
      int iItem;

      // get selected line
      iItem = SendDlgItemMessage( hwnd, DID_CALCRPT_OUT_LB, LB_GETCURSEL, 0, 0 );
			QUERYITEMTEXT( hwnd,DID_CALCRPT_OUT_LB , iItem, szWorkString );

      //
      // GQ:
      // the layout of a valid line is as follows (Blanks displayed as underscore, d=drive, n=number):
      // "[n]_d:\EQF\folshortname.F00\longdocname_:_#nnn"
      //

      // split line into components..

      // check/cut off number prefix
      fValid = ((szWorkString[0] == '[') && isdigit(szWorkString[1]) );
      if ( fValid )
      {
        pFolder = szWorkString + 1;
        while ( isdigit(*pFolder) ) pFolder++;
        fValid = (*pFolder++ == ']' );
        if ( fValid ) fValid = (*pFolder++ == ' ' );
      } /* endif */

      // isolate segment number
      if ( fValid ) fValid = ((pSegNum = strrchr( pFolder, '#' )) != NULL );
      if ( fValid )
      {
        *pSegNum = EOS;
        pSegNum++;
      } /* endif */

      // check/cut off trailer at end of document name
      if ( fValid )
      {
        int iLen = strlen(pFolder);
        if ( (pFolder[iLen-1] == ' ') && (pFolder[iLen-2] == ':') && (pFolder[iLen-3] == ' ')  )
        {
          pFolder[iLen-3] = EOS;
        }
        else
        {
          fValid = FALSE;
        } /* endif */
      } /* endif */

      // split folder/document name
      if ( fValid )
      {
        pDocName = strchr ( pFolder, BACKSLASH );                       // position to EQF directory
        if ( pDocName ) pDocName = strchr ( pDocName + 1, BACKSLASH );  // position to folder directory
        if ( pDocName ) pDocName = strchr ( pDocName + 1, BACKSLASH );  // position to document name
        if ( pDocName )
        {
          *pDocName = EOS;
          pDocName++;
        } /* endif */
        if ( !pDocName ) fValid = FALSE;
      } /* endif */

      if ( fValid )
      {
				FolLongToShortDocName( pFolder, pDocName, szShort, &fIsNew );

				strcpy(pOpen->szDocName, pFolder);
				strcat(pOpen->szDocName, BACKSLASH_STR);
        strcat(pOpen->szDocName, szShort[0] ? szShort : pDocName );
				pOpen->ulSeg = (ULONG)atol(pSegNum);
				pOpen->usOffs= 0;
				pOpen->usLen = 0;
				pOpen->chFind[0] = EOS;

				EqfPost2Handler( DOCUMENTHANDLER, WM_EQF_PROCESSTASK,
								 MP1FROMSHORT(OPEN_AND_POSITION_TASK),
								 MP2FROMP(pOpen) );
  			} /* endif */
		} /* endif */
	} /* endif */
  return (mResult);
}


//------------------------------------------------------------------------------
// Function name: RptBatchCount
//------------------------------------------------------------------------------
// Function call:
//
//------------------------------------------------------------------------------
// Description:
//
//------------------------------------------------------------------------------
// Input parameter:
//
//------------------------------------------------------------------------------
// Output parameter:
//
//------------------------------------------------------------------------------
// Returncode type:
//------------------------------------------------------------------------------
// Returncodes:
//
//------------------------------------------------------------------------------
// Prerequesits:
//
//------------------------------------------------------------------------------
// Side effects:
//
//------------------------------------------------------------------------------
// Samples:
//
//------------------------------------------------------------------------------
// Function flow:
//
//
//
//------------------------------------------------------------------------------

USHORT RptBatchCount
(
HWND             hwndParent,         // handle of count handler window
PDDECNTRPT       pCntRpt             // wordcount data structure
)
{
    HWND           hwndLB = NULLHANDLE; // handle of invisible listbox
    PSZ            pszParm;             // pointer for error parameters
    BOOL           fOK = TRUE;          // internal O.K. flag
    PRPT           pRpt;                // pointer to count structure
    OBJNAME        szFolObjName;        // buffer for folder object name
    SHORT  sIndex;                      // index of listbox items
    SHORT  sItemCount = 0;                  // number of listbox items
    SHORT  sRc;                         //rc from WM_EQF_QUERYSYMBOL
    SHORT sItem;                        // index of listbox items
    USHORT   i;                         // loop index
    USHORT   initial,j;                 // loop index
    PPROPFOLDER  ppropFolder;           // pointer to folder properties
    HPROP        hpropFolder;           // folder properties handle
    PPROPFOLDER  p1propFolder;          // pointer to folder properties
    HPROP        h1propFolder;          // folder properties handle
    ULONG        ulErrorInfo;           // error indicator from property handler
    OBJNAME      szFolObject;           // folder object name
    USHORT usRc;                        // retrun code from Utl... functions
    USHORT usAction;                    // action performed by UtlOpen
    PSZ    pszReplace;                  // error message parameter pointer
    HFILE  hfFile;                      // file handle
    PSZ    pszTemp;
    CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    /********************************************************************/
    /* Create invisible listbox for names of folder/documents/...       */
    /********************************************************************/
    hwndLB = WinCreateWindow( hwndParent, WC_LISTBOX, "",
                              LBS_MULTIPLESEL,
                              0, 0, 0, 0,
                              hwndParent, HWND_TOP,
                              DID_CNT00_COUNTHANDLER_LB,
                              NULL, NULL );

    /*******************************************************************/
    /* Allocate wordcount data area                                    */
    /*******************************************************************/
    fOK = UtlAllocHwnd( (PVOID *)&pRpt, 0L, (ULONG)sizeof(RPT), ERROR_STORAGE,
                        pCntRpt->hwndErrMsg );
    if ( fOK )
    {
      fOK = UtlAllocHwnd( (PVOID *)&(pRpt->pszActualDocument), 0L, (ULONG)MAX_FILELIST, ERROR_STORAGE,
                        pCntRpt->hwndErrMsg );
    } /* endif */

    /*******************************************************************/
    /* Check if folder exists                                          */
    /*******************************************************************/
    if ( fOK )
    {
        BOOL fIsNew = FALSE;

        ObjLongToShortName( pCntRpt->szFolder, szShortName, FOLDER_OBJECT, &fIsNew );
        if ( fIsNew )
        {
            fOK = FALSE;
            pszParm = pCntRpt->szFolder;
            pCntRpt->DDEReturn.usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
            UtlErrorHwnd( pCntRpt->DDEReturn.usRc, MB_CANCEL, 1,
                          &pszParm, EQF_ERROR, pCntRpt->hwndErrMsg );
        } /* endif */
    } /* endif */

    /*******************************************************************/
    /* Check if documents exist                                        */
    /*******************************************************************/
    if ( fOK )
    {
        /*****************************************************************/
        /* Build folder object name (access to folder properties is      */
        /* required to correct folder drive letter)                      */
        /*****************************************************************/
        {
            UtlMakeEQFPath( szFolObjName, NULC, SYSTEM_PATH, NULL );
            strcat( szFolObjName, BACKSLASH_STR );
            strcat( szFolObjName, szShortName );
            strcat( szFolObjName, EXT_FOLDER_MAIN );
            hpropFolder = OpenProperties( szFolObjName, NULL,
                                          PROP_ACCESS_READ, &ulErrorInfo);
            if ( hpropFolder )
            {
                ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
                if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
                {
                    szFolObjName[0] = ppropFolder->chDrive;
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
                         MP1FROMHWND( hwndLB ), MP2FROMP(szFolObjName)  );

        /******************************************************************/
        /* Search document names in listbox                               */
        /******************************************************************/
        i = 0;
        while ( fOK && (i < pCntRpt->usFileNums) )
        {
            CHAR szDocShortName[MAX_FILESPEC];        // buffer for short doc name
            BOOL fIsNew = FALSE;

            FolLongToShortDocName( szFolObjName, pCntRpt->ppFileArray[i],
                                   szDocShortName, &fIsNew );

            if ( fIsNew )
            {
                sItem = -1;                   // trigger not-found condition
            }
            else
            {
                sItem = 1;                    // a dummy value ...
            } /* endif */

            if ( sItem < 0 )
            {
                fOK = FALSE;
                pszParm = pCntRpt->ppFileArray[i];
                pCntRpt->DDEReturn.usRc = ERROR_TA_SOURCEFILE;
                UtlErrorHwnd( pCntRpt->DDEReturn.usRc, MB_CANCEL, 1,
                              &pszParm, EQF_ERROR, pCntRpt->hwndErrMsg );
            } /* endif */
            i++;
        } /* endwhile */
    } /* endif */

    /*******************************************************************/
    /* Fill count area data fields                                     */
    /*******************************************************************/
    if ( fOK )
    {
        char szProfiles[MAX_PATH144+MAX_FILESPEC];

        INT     i_begin_option;

        pRpt->fBatch = TRUE;
        pRpt->hwndErrMsg = pCntRpt->hwndErrMsg;
        pRpt->pDDECntRpt = pCntRpt;

        pRpt->usAllDocuments = QUERYITEMCOUNTHWND( hwndLB );

        strcpy( pRpt->szFolderObjName, szFolObjName );   // folder object name
        strcpy( pRpt->szFolderName, szShortName ); // folder name

        strcat( pRpt->szFolderName, EXT_FOLDER_MAIN );
        sprintf( pRpt->szRptInstanceObjName, "%s%s",   // count instance name
                 COUNTHANDLER, pRpt->szFolderObjName );
        pRpt->hwndRptHandlerLB = hwndLB;
//@     strcpy( pRpt->szTitle, COUNTHANDLER );
        pRpt->fRptFile = TRUE;
        strcpy(pRpt->szLongFolderName, pCntRpt->szFolder);
        strcpy(pRpt->szFolder, szShortName);

        // fill report/counting options
        pRpt->usReport  = pCntRpt->usReport;
        pRpt->usOptions = pCntRpt->usOptions;


        LOADSTRING (hab, hResMod, SID_RPT_REPORT_1 + pRpt->usReport,
                    pRpt->szReport);

        // fill OPTIONS for reports
        // as function of report
        // to be changed from Spinbutton
        if (pRpt->usReport == HISTORY_REPORT)
        {
            i_begin_option = 0;
        }
        else if (pRpt->usReport == CALCULATION_REPORT)
        {
            i_begin_option =  REPORT_2_OFFSET;
        }
        else
        {
            i_begin_option =  REPORT_3_OFFSET;
        }                              // end if

        LOADSTRING (hab, hResMod, SID_RPT_OPTION_1 +
                    pRpt->usOptions + i_begin_option,
                    pRpt->szOption);


        //------------------------------
        // fill report/counting options
        // from profile
        //------------------------------

        // load profile

        if (pCntRpt->szProfile[0])
        {

            UtlMakeEQFPath( szProfiles, NULC,
                            PROPERTY_PATH,NULL );
            pszTemp = UtlSplitFnameFromPath(szProfiles);
            strcat(szProfiles , BACKSLASH_STR );
            strcat(szProfiles , pCntRpt->szProfile );

            ulErrorInfo = 0;
            if ((h1propFolder = OpenProperties(szProfiles, NULL,PROP_ACCESS_READ,&ulErrorInfo )) != NULL)
            {
                p1propFolder = (PPROPFOLDER) MakePropPtrFromHnd(h1propFolder);
                /***********************************************************/
                /* copy profile into folder properties                     */
                /***********************************************************/

                pRpt->usFormat      =  p1propFolder->usFormat      ;
                pRpt->usOption[0]   =  p1propFolder->usOption[0]   ;
                pRpt->usOption[1]   =  p1propFolder->usOption[1]   ;
                pRpt->usOption[2]   =  p1propFolder->usOption[2]   ;
                pRpt->usOption[3]   =  p1propFolder->usOption[3]   ;
                pRpt->usOption[4]   =  p1propFolder->usOption[4]   ;
                pRpt->usOption[5]   =  p1propFolder->usOption[5]   ;
                pRpt->usOption1     =  p1propFolder->usOption1     ;
                pRpt->usOption2     =  p1propFolder->usOption2     ;
                pRpt->usOption3     =  p1propFolder->usOption3     ;
                pRpt->usOption4     =  p1propFolder->usOption4     ;
                pRpt->usOption21    =  p1propFolder->usOption21    ;
                pRpt->usOption22    =  p1propFolder->usOption22    ;
                pRpt->usColumns[0]  =  p1propFolder->usColumns[0]  ;
                pRpt->usColumns[1]  =  p1propFolder->usColumns[1]  ;
                pRpt->usColumns[2]  =  p1propFolder->usColumns[2]  ;
                pRpt->usColumns[3]  =  p1propFolder->usColumns[3]  ;
                pRpt->usColumns4[0]  =  p1propFolder->usColumns4[0]  ;
                pRpt->usColumns4[1]  =  p1propFolder->usColumns4[1]  ;
                pRpt->usColumns4[2]  =  p1propFolder->usColumns4[2]  ;
                pRpt->usColumns4[3]  =  p1propFolder->usColumns4[3]  ;
                pRpt->usColumns4[4]  =  p1propFolder->usColumns4[4]  ;
                pRpt->usStandard    =  p1propFolder->usStandard    ;
                pRpt->Pay_per_Standard = p1propFolder->Pay_per_Standard;
                pRpt->usCurrency    =  p1propFolder->usCurrency    ;
                strcpy(pRpt->szCurrency, szRptCurrency2[pRpt->usCurrency]);
                strcpy(pRpt->szShipment, p1propFolder->szShipment);
                pRpt->usShipmentChk = p1propFolder->usShipmentChk;
                strcpy( pRpt->szTargetLang, p1propFolder->szTargetLang );


                // set shipment string if not specified in profile
                // (in the GUI the shipment string is set...)
                if ( pRpt->szShipmentChk[0] == EOS )
                {
                  strcpy(pRpt->szShipmentChk, "All Shipments");
                } /* endif */

                // Set complexity and pay factor for final fact sheet
                // according properties
                for (initial=0;initial<MAX_PAYREPORT_COLUMNS;initial++)
                {
                    for (j=0;j<3;j++)
                    {
                        pRpt->Complexity_Factor[initial][j] =
                        p1propFolder->Complexity_Factor[initial][j];
                        pRpt->Pay_Factor[initial][j] =
                        p1propFolder->Pay_Factor[initial][j];
                    }  //end for
                } //end for

                if (h1propFolder) CloseProperties(h1propFolder,PROP_FILE,&ulErrorInfo);

                strcpy( pRpt->szProfile, pCntRpt->szProfile );

            }
            else
            {
                // error opening profile
                fOK = FALSE;
                pszParm = szProfiles;
                pCntRpt->DDEReturn.usRc =  ERROR_PROFILE_LOAD;
                UtlErrorHwnd( pCntRpt->DDEReturn.usRc, MB_CANCEL, 1,
                              &pszParm, EQF_ERROR, pCntRpt->hwndErrMsg );
            } /* end if */

        }
        else
        {
            pRpt->usFormat      =  0 ;
            pRpt->usOption[0]   =  0 ;
            pRpt->usOption[1]   =  0 ;
            pRpt->usOption[2]   =  0 ;
            pRpt->usOption[3]   =  0 ;
            pRpt->usOption[4]   =  0 ;
            pRpt->usOption[5]   =  0 ;
            pRpt->usOption1     =  0 ;
            pRpt->usOption2     =  0 ;
            pRpt->usOption3     =  0 ;
            pRpt->usOption4     =  0 ;
            pRpt->usOption21    =  0 ;
            pRpt->usOption22    =  0 ;
            pRpt->usColumns[0]  =  0 ;
            pRpt->usColumns[1]  =  0 ;
            pRpt->usColumns[2]  =  0 ;
            pRpt->usColumns[3]  =  0 ;
            pRpt->usColumns4[0]  =  0 ;
            pRpt->usColumns4[1]  =  0 ;
            pRpt->usColumns4[2]  =  0 ;
            pRpt->usColumns4[3]  =  0 ;
            pRpt->usColumns4[4]  =  0 ;
            pRpt->usStandard    =  0 ;
            pRpt->Pay_per_Standard = 1.0 ;
            pRpt->usCurrency    =  1 ;

            // Set complexity and pay factor for final fact sheet
            // according properties
            for (initial=0;initial<MAX_PAYREPORT_COLUMNS;initial++)
            {
                for (j=0;j<3;j++)
                {
                    pRpt->Complexity_Factor[initial][j] = 1.;
                    pRpt->Pay_Factor[initial][j] = 1.;
                }  //end for
            } //end for

        } /* end if */

        // handle any output format specified by the user
        if ( pCntRpt->szFormat[0] )
        {
			    strcpy(pRpt->szFormat, pCntRpt->szFormat );
		      if (!strcmp(pCntRpt->szFormat, "ASCII"))
		      {
			      pRpt->usFormat = ASCII;
		      }
		      else if (!strcmp(pCntRpt->szFormat, "HTML"))
		      {
			      pRpt->usFormat = HTML;
		      }
		      else if (!strcmp(pCntRpt->szFormat, "XML"))
		      {
			      pRpt->usFormat = XMLFILEFORMAT;
		      }
		      else
          {
			      pRpt->usFormat = ASCII;
          } /* endif */
        } /* endif */
    } /* endif */

    // check if there is a histlog file
    if ( fOK )
    {
        // check, if histlog.dat already exists
        // build absolut path for histlog record
        UtlMakeEQFPath( pRpt->szWorkString, pRpt->szFolderObjName[0],
                        PROPERTY_PATH,
                        UtlGetFnameFromPath(pRpt->szFolderObjName) );
        strcat( pRpt->szWorkString, BACKSLASH_STR );
        strcat( pRpt->szWorkString, HISTLOGFILE);

        fOK = UtlFileExist( pRpt->szWorkString );

        if ( !fOK )
        {
            PSZ pszTmp = pCntRpt->szFolder;
            pCntRpt->DDEReturn.usRc = MESSAGE_RPT_NO_HISTLOG_AVAIL;
            UtlErrorHwnd( pCntRpt->DDEReturn.usRc, MB_OK, 1,
                          &pszTmp, EQF_INFO, pCntRpt->hwndErrMsg );
        } // end if
    } /* endif */

    /*******************************************************************/
    /* Retrieve values from folder properties                          */
    /*******************************************************************/
    if ( fOK )
    {

        UtlMakeEQFPath( szFolObject, NULC, SYSTEM_PATH, NULL );
        strcat( szFolObject, BACKSLASH_STR );
        strcat( szFolObject, szShortName ); // folder name
        strcat( szFolObject, EXT_FOLDER_MAIN );

//@   fOK = FolQueryInfoHwnd( szFolObject, pCnt->szMemory, pCnt->szFormat, NULL,
//@                    NULL, TRUE, pCntRpt->hwndErrMsg ) == NO_ERROR;
    } /* endif */

    /*******************************************************************/
    /* Insert documents into listbox                                   */
    /*******************************************************************/
    if ( fOK )
    {
        DELETEALLHWND( hwndLB );
        if ( pCntRpt->usFileNums )
        {
            /***************************************************************/
            /* insert specified documents                                  */
            /***************************************************************/

            for ( i = 0; i < pCntRpt->usFileNums; i++ )
            {
                CHAR szDocShortName[MAX_FILESPEC];        // buffer for short doc name
                BOOL fIsNew = FALSE;

                FolLongToShortDocName( szFolObjName, pCntRpt->ppFileArray[i],
                                       szDocShortName, &fIsNew );

                INSERTITEMHWND( hwndLB, szDocShortName );

            } /* endwhile */
        }
        else
        {
            /***************************************************************/
            /* insert all documents of folder                              */
            /***************************************************************/
            EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                             MP1FROMHWND( hwndLB ), MP2FROMP(pRpt->szFolderObjName) );
        } /* endif */
    } /* endif */

    /*******************************************************************/
    /* Check if count files are in use                                 */
    /* Note: Files which are in use are skipped without further        */
    /*       notice                                                    */
    /*******************************************************************/
    if ( fOK )
    {

        sIndex = 0;
        sItemCount = QUERYITEMCOUNTHWND( hwndLB );
        pRpt->usSelectedDocuments = sItemCount;

        while ( sIndex < sItemCount )
        {
            /***************************************************************/
            /* Get current item from listbox                               */
            /***************************************************************/
            QUERYITEMTEXTHWND( hwndLB, sIndex, pRpt->szFileName );

            /*****************************************************************/
            /* build object name (folder name concatenated with filename )   */
            /*****************************************************************/
            sprintf( pRpt->szFileObjName, "%s\\%s",
                     pRpt->szFolderObjName, pRpt->szFileName );

            /*****************************************************************/
            /* check if symbol for this file already exists                  */
            /* i.e. file is in use                                           */
            /*****************************************************************/
            sRc = QUERYSYMBOL( pRpt->szFileObjName );
            if ( sRc != -1 )
            {
                /*************************************************************/
                /* flag file as locked in count handler listbox.             */
                /*************************************************************/
                SETITEMHANDLEHWND( hwndLB, sIndex, MP2FROMLONG(ITEM_LOCKED) );
            }
            else
            {
                /***************************************************************/
                /* file is not in use, lock it !                               */
                /***************************************************************/
                SETSYMBOL( pRpt->szFileObjName );

                /***************************************************************/
                /* flag file as unlocked in count handler listbox              */
                /***************************************************************/
                SETITEMHANDLEHWND( hwndLB, sIndex, NULL );
            } /* endif */

            sIndex++;                       // continue with next item
        } /* endwhile */
    } /* endif */

    /*******************************************************************/
    /* Check output file name                                          */
    /*******************************************************************/
    if ( fOK && (UtlGetFnameFromPath( pCntRpt->szOutFile ) == NULL) )
    {
        fOK = UtlInsertCurDir( pCntRpt->szOutFile,
                               &(pCntRpt->ppCurDirArray),pRpt->szRptOutputFile);
        if ( !fOK )
        {
            PSZ pszFileName = pCntRpt->szOutFile;

            fOK = FALSE;                               // name is not valid
            UtlErrorHwnd( UTL_PARAM_TOO_LONG, MB_CANCEL, 1,
                          &pszFileName, EQF_ERROR, pCntRpt->hwndErrMsg );
        } /* endif */
    }
    else
    {
        strcpy( pRpt->szRptOutputFile, pCntRpt->szOutFile );
    } /* endif */
    strupr( pRpt->szRptOutputFile );

    if ( fOK )
    {
        CHAR szSysPath[MAX_EQF_PATH];

        //--- get system path
        UtlMakeEQFPath( szSysPath, NULC, SYSTEM_PATH, NULL );
        strcat( szSysPath, "\\" );

        //--- compare if target path contains as first directory the
        //--- system path
        if ( (strnicmp( pRpt->szRptOutputFile + 2, szSysPath + 2,
                        strlen(szSysPath) - 2 ) == 0) ||
             (strnicmp( pRpt->szRptOutputFile + 3, szSysPath + 3,
                        strlen(szSysPath) - 2 ) == 0) )
        {
            //--- display error message that this is not allowed
            PSZ pszReplace = szSysPath;
            UtlErrorHwnd( ERROR_EQF_PATH_INVALID, MB_CANCEL,
                          1, &pszReplace, EQF_ERROR, pCntRpt->hwndErrMsg );
            //--- stop further processing
            fOK = FALSE;
        } /* endif */

        if ( fOK )
        {


            //--- check filename
            usRc = UtlOpen( pRpt->szRptOutputFile,         //filename
                            &hfFile,                       //file handle
                            &usAction,                     //action taken by Open
                            0L,                            //file size
                            FILE_NORMAL,                   //attribute  read/write
                            OPEN_ACTION_FAIL_IF_EXISTS |   //fail if exist
                            OPEN_ACTION_CREATE_IF_NEW,
                            OPEN_ACCESS_READONLY |         //open for read only
                            OPEN_SHARE_DENYREADWRITE,      //deny any other access
                            0L,                            //reserved, must be 0
                            FALSE );                       //do no error handling
            switch ( usRc )   //--- rc from UtlOpen
            {
            //-----------------------------------------------------------
            case ( ERROR_FILENAME_EXCED_RANGE ) :   //--- no valid filename
                //--- display error message that filename is no valid
                pszReplace = pRpt->szRptOutputFile;
                UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1,
                              &pszReplace, EQF_WARNING, pCntRpt->hwndErrMsg );

                fOK = FALSE;
                break;
                //-----------------------------------------------------------
            case ( ERROR_PATH_NOT_FOUND ) :   //--- path does not exist
                //--- display error message path not exist
                UtlErrorHwnd( ERROR_PATH_NOT_EXIST, MB_CANCEL, 0, (PSZ *) NULP,
                              EQF_WARNING, pCntRpt->hwndErrMsg );
                //--- stop further processing
                fOK = FALSE;
                break;
                //-----------------------------------------------------------
            case ( ERROR_NOT_READY ) :   //--- disk not ready
                //--- display error message disk not ready
                pszReplace = pRpt->szRptDriveLetter;
                UtlErrorHwnd( ERROR_NOT_READY_MSG, MB_CANCEL, 1,
                              &pszReplace, EQF_ERROR, pCntRpt->hwndErrMsg );
                //--- stop further processing
                fOK = FALSE;
                break;
                //-----------------------------------------------------------
            case ( NO_ERROR ) :   //--- filename ok
                //--- close file, do not handle  error
                UtlClose( hfFile, FALSE );
                //--- delete file, do not handle  error
                UtlDelete( pRpt->szRptOutputFile, 0L, FALSE );
                break;
                //-----------------------------------------------------------
            case ( ERROR_FILE_EXISTS ) :   //--- file exists
            case ( ERROR_OPEN_FAILED ) :
                //--- display error message that file exists
                pszReplace = pRpt->szRptOutputFile;
                if ( pCntRpt->fOverWrite )
                {
                    if ( UtlDeleteHwnd( pRpt->szRptOutputFile, 0L, TRUE, pCntRpt->hwndErrMsg ) )
                    {
                        fOK = FALSE;
                    } /* endif */
                }
                else
                {
                    UtlErrorHwnd( ERROR_FILE_EXISTS_ALREADY, MB_CANCEL,
                                  1, &pszReplace, EQF_QUERY, pCntRpt->hwndErrMsg );

                    sIndex=0;
                    while ( sIndex < sItemCount )
                    {
                        /*******************************************************/
                        /* Get current item from listbox                       */
                        /*******************************************************/
                        QUERYITEMTEXTHWND( hwndLB, sIndex, pRpt->szFileName );

                        /*******************************************************/
                        /* build object name                                   */
                        /*******************************************************/
                        sprintf( pRpt->szFileObjName, "%s\\%s",
                                  pRpt->szFolderObjName, pRpt->szFileName );

                        /*******************************************************/
                        /* unlock file                                         */
                        /*******************************************************/

                        if (!(SHORT)QUERYITEMHANDLEHWND(hwndLB,sIndex))
                        {
                            REMOVESYMBOL( pRpt->szFileObjName );
                        }

                        sIndex++;
                    }/* end while */
                    fOK = FALSE;
                } /* endif */
                break;
            } /* endswitch */
        } /* endif */
    } /* endif */


    /*******************************************************************/
    /* Start count process                                             */
    /*******************************************************************/
    if ( fOK )
    {
        fOK = CreateListWindow( pRpt->szRptInstanceObjName, RptListCallBack,
                                (PVOID)pRpt, FALSE );
    } /* endif */



    // unlock files
    // ------------

    if ( fOK )
    {

        sIndex=0;
        while ( sIndex < sItemCount )
        {
            /*******************************************************/
            /* Get current item from listbox                       */
            /*******************************************************/
            QUERYITEMTEXTHWND( hwndLB, sIndex, pRpt->szFileName );

            /*******************************************************/
            /* build object name                                   */
            /*******************************************************/
            sprintf( pRpt->szFileObjName, "%s\\%s",
                     pRpt->szFolderObjName, pRpt->szFileName );

            /*******************************************************/
            /* unlock file                                         */
            /*******************************************************/

            if (!(SHORT)QUERYITEMHANDLEHWND(hwndLB,sIndex))
            {
                REMOVESYMBOL( pRpt->szFileObjName );
            }

            sIndex++;
        }/* end while */

    }/* end if */

    /*******************************************************************/
    /* Cleanup                                                         */
    /*******************************************************************/
    if ( !fOK )
    {
        if ( pRpt   != NULL ) UtlAlloc( (PVOID *) &pRpt, 0L, 0L, NOMSG) ;

        /****************************************************************/
        /* report end of task to DDE handler                            */
        /****************************************************************/
        WinPostMsg( pCntRpt->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                    &pCntRpt->DDEReturn );
    } /* endif */

    if ( !fOK ) pCntRpt->DDEReturn.usRc = UtlGetDDEErrorCode( pCntRpt->hwndErrMsg );

    return( pCntRpt->DDEReturn.usRc );

} /* end of function RptBatchCount */



USHORT MemFuncCreateCntReport(HSESSION hSession,
							  CHAR chDriveLetter,
							  PSZ pszFolderName,
							  PSZ pszDocuments,
							  PREPORTTYPE pReportType,
							  PSZ pszOutfileName,
							  PSZ pszFormat,
							  PSZ pszProfile,
							  PREPORTSETTINGS pRepSettings,
							  PFACTSHEET pFactSheetArg,
							  USHORT usColumn,
							  USHORT usCategory,
							  PFINALFACTORS pFinalFactors,
							  LONG lOptSecurity,
							  BOOL bSingleShipment,
                PSZ pszShipment )
{
	USHORT		usRC = NO_ERROR;
	PRPT 		pRpt = NULL;
	USHORT		usCnti = 0;
	USHORT		usCntj = 0;
	CHAR		szBuf[40];
    PCHAR		pszParm = NULL;

    BOOL fIsNew;
    CHAR szTargetShortFolName[10];
    CHAR szTargetLongFolName[MAX_LONGFILESPEC];
    CHAR szProfile[MAX_FILESPEC];

    BOOL fSubFolder = FALSE;
    USHORT usSubFolderLevel = 0;
    CHAR szObjName[60];
    CHAR szObjSubFolderName[60];
    CHAR szSubFolderObjName[60];
    PSZ pszObjName = NULL;
    BOOL fFolderLocked = FALSE;

	#define COLUMN 		10
	#define CATEGORY 	3

  hSession;                                    // avoid compiler warning

  if ( UtlAlloc ((PVOID*)&pRpt, 0L, (ULONG)sizeof (RPT), ERROR_STORAGE) )
  {
    if ( !UtlAlloc( (PVOID *)&(pRpt->pszActualDocument), 0L, (ULONG)MAX_FILELIST, ERROR_STORAGE ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
  }
  else
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  if ( pRpt )
  {
    pRpt->fBatch = TRUE;
    pRpt->hwndErrMsg = HWND_FUNCIF;
  } /* endif */


	// check for valid foldername
	if (!usRC)
	{
	  if ((pszFolderName == NULL) || (*pszFolderName == EOS))
	  {
		  usRC = ERROR_INVALID_FOLDER_NAME;
		  pszParm = pszFolderName;
	  }

	  // check if the specified name is a valid one
	  if (!usRC)
	  {
        USHORT usI = 0;
        USHORT usLength = (USHORT)strlen(pszFolderName);

        // determine if it's a subfolder
        for (usI = 0; usI <= usLength; usI++)
        {
            if (pszFolderName[usI] == BACKSLASH)
            {
                fSubFolder = TRUE;
                usSubFolderLevel++;
            }
        }

/*
		if (!UtlCheckLongName(pszFolderName))
		{
		  usRC = ERROR_INV_LONGNAME;
		  pszParm = pszFolderName;
		}
*/
	  } /* endif */


	  // Check if folder target drive is valid
	  if (!usRC && chDriveLetter != EOS)
	  {
		CHAR szEqfDrives[MAX_DRIVELIST];   // buffer for EQF drive letters

		/* Get valid EQF drives                                           */
		UtlGetCheckedEqfDrives(szEqfDrives);

		/* Check if specified target drive is in list of valid drives     */
		if (strchr( szEqfDrives, toupper(chDriveLetter)) == NULL)
		{
		  szBuf[0] = chDriveLetter;
		  szBuf[1] = ':';
		  szBuf[2] = EOS;

		  usRC = ERROR_EQF_DRIVE_NOT_VALID;
		  pszParm = szBuf;
		}
	  } /* endif */

    // get folder drive letter if not specified
	  if ( !usRC && (chDriveLetter == EOS) )
    {
      OBJNAME ObjName;
      // GQ 2016/05/24 copy folder name to a temporary buffer as SubFolNameToObjectName tends to overwrite the folder long name and the caller's long name buffer should not be touched
      strcpy( szTargetLongFolName, pszFolderName ); 
      if ( SubFolNameToObjectName( szTargetLongFolName, ObjName ) )
      {
        chDriveLetter = ObjName[0];
      } /* endif */
    }

   	  // fill Rpt structure
	    if (!usRC && fSubFolder == FALSE)
      {

        strcpy(szTargetLongFolName, pszFolderName);

		    ObjLongToShortName( szTargetLongFolName,
							szTargetShortFolName,
							FOLDER_OBJECT,
							&fIsNew );

		    // assemble full qualified name
		    UtlMakeEQFPath(pRpt->szFolderObjName, chDriveLetter, SYSTEM_PATH, szTargetShortFolName);
        strcat(pRpt->szFolderObjName, EXT_FOLDER_MAIN);

        strcpy(pRpt->szFolderName, szTargetShortFolName);
        strcat(pRpt->szFolderName, EXT_FOLDER_MAIN);

        strcpy(pRpt->szFolder, szTargetShortFolName);

        UtlUpper( pRpt->szFolderObjName );

        strcpy(pRpt->szLongFolderName, pszFolderName );

		// check if the folder (directory) exists with the short name
        if ( !UtlDirExist(pRpt->szFolderObjName) )
        {
			usRC = FILE_NOT_EXISTS;
			pszParm = szTargetLongFolName;
        }
	  } // end if
	} // end if


    if (!usRC && fSubFolder == TRUE)
    {
      BOOL fIsNew;
      USHORT usI = 0;
      USHORT usLength = (USHORT)strlen(pszFolderName);

      // needed for output in calculating report
      strcpy(pRpt->szLongFolderName, pszFolderName);

      // now get the folder ...

      // get long name of root folder
      for (usI = 0; usI <= usLength; usI++)
      {
         if (pszFolderName[usI] == BACKSLASH)
         {
             strncpy( szTargetLongFolName, pszFolderName, usI);
             break;
         }
      }

      // strcpy(szTargetLongFolName, pszFolderName);

      ObjLongToShortName( szTargetLongFolName,
	 					  szTargetShortFolName,
						  FOLDER_OBJECT,
						  &fIsNew );

	  // assemble full qualified name
	  UtlMakeEQFPath(pRpt->szFolderObjName, chDriveLetter, SYSTEM_PATH, szTargetShortFolName);
      strcat(pRpt->szFolderObjName, EXT_FOLDER_MAIN);

      strcpy(pRpt->szFolderName, szTargetShortFolName);
      strcat(pRpt->szFolderName, EXT_FOLDER_MAIN);

      strcpy(pRpt->szFolder, szTargetShortFolName);

      UtlUpper(pRpt->szFolderObjName);
      strcpy(pRpt->szParentObjName, pRpt->szFolderObjName);

      // now get the subfolder ...

      strcpy(szObjSubFolderName, pszFolderName);

      fIsNew = !SubFolNameToObjectName( szObjSubFolderName, szObjName );

      if (!fIsNew)
      {
        // for keeps
        // strcpy(szObjSubFolderName, szObjName);
        strcpy( szSubFolderObjName, szObjName );
      } /* endif */

      if (fIsNew)
      {
        pszParm = pszFolderName;
        usRC = FILE_NOT_EXISTS;
      } /* endif */
    }

  // check if folder/subfolder is locked
  if ( !usRC )
  {
    SHORT sRC = 0;
    pszObjName = ( fSubFolder ) ? szSubFolderObjName : pRpt->szFolderObjName;
    sRC = QUERYSYMBOL( pszObjName );
    if ( sRC != -1 )
    {
      pszParm = pszFolderName;
      usRC = ERROR_FOLDER_LOCKED;
    }
    else
    {
      SETSYMBOL( pszObjName );
      fFolderLocked = TRUE;
    } /* endif */
  } /* endif */

	// retrieve documents to be analysed
	if (!usRC)
	{
		if ( (pszDocuments == NULL) || (*pszDocuments == EOS) )				// use all documents
		{
			PSZ 	pszDocuments = NULL;
			//USHORT 	usCnti = 0;
			//USHORT 	usCntj = 0;

      // the buffer is allocated and maintained by LoadDocumentNames!
      if (fSubFolder == FALSE)
			  // load all short file names in property folder. Delimiter is ascii null.
			  usRC = LoadDocumentNames(pRpt->szFolderObjName, HWND_FUNCIF, LOADDOCNAMES_INCLSUBFOLDERS, (PSZ)&pszDocuments);
      else
   			usRC = LoadDocumentNames( szSubFolderObjName, HWND_FUNCIF, LOADDOCNAMES_INCLSUBFOLDERS, (PSZ)&pszDocuments);

			// needed for output header
			pRpt->usAllDocuments = usRC;
      pRpt->usSelectedDocuments = usRC;

			if (usRC == 0)
			{
				usRC = ERROR_NO_FILE_LOADED;
				pszParm = pRpt->szFolderObjName;
			}
			else
			{
        // reformat string. change ascii null to comma
        PSZ pszTemp = pszDocuments;
        while ( *pszTemp != EOS )
        {
          // skip document name
          while ( *pszTemp != EOS ) pszTemp++;

          // replace  EOS with comma if not the last document in the list
          if ( pszTemp[1] != EOS )
          {
            *pszTemp = ',';
            pszTemp++;
          } /* endif */
        } /*endwhile */

				// use document name buffer allocated by LoadDocumentNames
       if ( pRpt->pszActualDocument ) UtlAlloc( (PVOID *)&(pRpt->pszActualDocument), 0L, 0L, NOMSG );
			 pRpt->pszActualDocument = pszDocuments;
			}
			pRpt->fFolderSelected = TRUE;

      // reset usRC as it held count of documents loaded
			if (usRC > 0) usRC = 0;
		}
		else // comma separated documents
		{
      BOOL bFirst = TRUE;
			int iLen;
      CHAR szDocBuffer[200];
			PSZ 	pszPropDocuments = NULL;
			int 	iCnti = 0;
			int  	iCntj = 0;
      USHORT usNumProps = 0;
      int  iSetoff = 0;
      int iNumOfDocuments = 0;
      BOOL bFound = FALSE;

      // reject empty string
      if (*pszDocuments == EOS)
			{
				usRC = NO_FILE_SELECTED;
				pszParm = pszDocuments;
			}

			// read file names in property folder
      if (fSubFolder == FALSE)
			  // load all short file names in property folder. Delimiter is ascii null.
			  usNumProps = LoadDocumentNames(pRpt->szFolderObjName, HWND_FUNCIF, LOADDOCNAMES_NAME, (PSZ)&pszPropDocuments);
      else
   			usNumProps = LoadDocumentNames( szSubFolderObjName, HWND_FUNCIF, LOADDOCNAMES_INCLSUBFOLDERS, (PSZ)&pszPropDocuments);

			// needed for output header
			pRpt->usAllDocuments = usNumProps;

			if (usNumProps == 0)
			{
				usRC = ERROR_NO_FILE_LOADED;
				pszParm = pRpt->szFolderObjName;
			}

      // get comma separated property document names
			if (!usRC)
			{
        ULONG ulBufferSize = MAX_FILELIST; // initial size of list

				iLen = strlen(pszDocuments);

				iCnti = 0;
				iCntj = 0;
				// scan document list as defined by user
				while (iCntj < iLen)
				{
					// find comma
          if ( pszDocuments[iCntj] == DOUBLEQUOTE )
          {
            iCnti++; iCntj++;
					  while (pszDocuments[iCntj] != DOUBLEQUOTE && pszDocuments[iCntj] != EOS)
						  iCntj++;

            if (pszDocuments[iCntj] == EOS)
					      strncpy(szDocBuffer, pszDocuments + iCnti, iCntj-iCnti);
            else
            {
					    strncpy(szDocBuffer, pszDocuments + iCnti, iCntj-iCnti-1);
              iCntj++; // skip closing double quote
            } /* endif */
            iNumOfDocuments++;


          }
          else
          {
            int t = 0;
					  while ( (pszDocuments[iCntj] != ',') && (pszDocuments[iCntj] != EOS) )
            {
              szDocBuffer[t++] = pszDocuments[iCntj++]; 
            }
            szDocBuffer[t] = EOS;
            iNumOfDocuments++;
          } /* endif */

					 // strip leading and trailing blanks
 					UtlStripBlanks(szDocBuffer);

					// get document short name
          {
            BOOL fIsNew = FALSE;
            CHAR szShortName[MAX_FILESPEC];

            FolLongToShortDocName( pRpt->szFolderObjName, szDocBuffer, szShortName, &fIsNew );

            if ( !fIsNew )
            {
              // add short name to our list
              int iUsedLen = strlen(pRpt->pszActualDocument);

              // enlarge buffer when allocated size is exceeded
              if ( (iUsedLen + MAX_FILESPEC + 1) >= ulBufferSize  )
              {
                if ( UtlAlloc( (PVOID *)&(pRpt->pszActualDocument), ulBufferSize, ulBufferSize + MAX_FILELIST, ERROR_STORAGE ) )
                {
                  ulBufferSize = ulBufferSize + MAX_FILELIST;
                } /* endif */                       
              } /* endif */                     

              if (bFirst == TRUE)
              {
                bFirst = FALSE;
                pRpt->pszActualDocument[0] = EOS;
              }
              else
              {
                strcat( pRpt->pszActualDocument, ",");
              }
              strcat( pRpt->pszActualDocument, szShortName );
            }
            else
            {
			        usRC = FILE_NOT_EXISTS;
			        pszParm = szDocBuffer;
              break;   // leave while loop and return error code
            } /* endif */               
			    }

          if ( pszDocuments[iCntj] != EOS ) iCntj++;
          iCnti = iCntj;
				} /* endwhile */
			} // end if

			pRpt->fFolderSelected = FALSE;
			if ( pszPropDocuments ) UtlAlloc( (PVOID *)&pszPropDocuments, 0L, 0L, NOMSG );
		} // end else
	} // end if


	if ( !usRC )
	{
		// retrieve the specified report
		if ( (pReportType->pszReport == NULL) && (pszProfile == NULL) )
		{
			usRC = TQM_REPORT_NO_NAME;
			pszParm = pReportType->pszReport;
		}
		else if (!strcmp(pReportType->pszReport, "Calculating Report"))
		{
	      pRpt->usReport = SUMMARY_COUNTING_REPORT;
		  strcpy(pRpt->szReport, pReportType->pszReport);
		}
		else if (!strcmp(pReportType->pszReport, "Pre-Analysis Report"))
   	    {
	      pRpt->usReport = PRE_ANALYSIS_REPORT;
		  strcpy(pRpt->szReport, pReportType->pszReport);
		}
	  else if (!strcmp(pReportType->pszReport, "Redundancy Report"))
	  {
          FILE *hInput = NULL;
          CHAR szInFile[256];

          strcpy(szInFile,pRpt->szFolderObjName);
          strcat(szInFile,"\\property\\redund.log");

          if ((hInput = fopen( szInFile, "rb" )) == NULL)
          {
      			usRC = MESSAGE_RPT_NO_REDUND_AVAIL;
		      	pszParm = pszFolderName;
   		  }
          else
            fclose(hInput);

  	      pRpt->usReport = REDUNDANCY_REPORT;
		  strcpy(pRpt->szReport, pReportType->pszReport);
		}
		else if (!strcmp(pReportType->pszReport, "Redundant Segment List"))
		{
          FILE *hInput = NULL;
          CHAR szInFile[256];

          strcpy(szInFile,pRpt->szFolderObjName);
          strcat(szInFile,"\\property\\redund.log");

          if ((hInput = fopen( szInFile, "rb" )) == NULL)
          {
            usRC = MESSAGE_RPT_NO_REDUND_AVAIL;
            pszParm = pszFolderName;
          }
          else
            fclose(hInput);

	      pRpt->usReport = COMBINED_REPORT;
		  strcpy(pRpt->szReport, pReportType->pszReport);
		  pReportType->lRepType = 1;
		}
		else if ( pszProfile == NULL )
		{
			usRC = TQM_REPORT_NAME_INVALID;
			pszParm = pReportType->pszReport;
		}
	} // end if


	if ( !usRC )
	{
        // retrieve the report information and profile name
        HPROP        hpropFolder;
        PPROPFOLDER  ppropFolder;
        ULONG        ulErrorInfo;
        CHAR         szFolder[MAX_PATH144+MAX_FILESPEC]; // profile names
        PSZ          pszTemp;                     // tmp pointer

        UtlMakeEQFPath( szFolder, NULC, PROPERTY_PATH,NULL );
        pszTemp = UtlSplitFnameFromPath( szFolder );
        strcat(szFolder , BACKSLASH_STR );
        strcat(szFolder , pRpt->szFolderName);
        // strcat(szFolder , ".F00" );

        if ((hpropFolder = OpenProperties( szFolder, NULL,PROP_ACCESS_READ,&ulErrorInfo )) != NULL)
        {
           ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd(hpropFolder);

           strcpy( pRpt->szTargetLang, ppropFolder->szTargetLang );

           // use folder report only if none has been specified
           if ( pReportType->pszReport == NULL )
           {
             switch (ppropFolder->usReport)
             {
                case SUMMARY_COUNTING_REPORT:
                {
	                pRpt->usReport = SUMMARY_COUNTING_REPORT;
                  strcpy(pRpt->szReport, "Calculating Report");
                  break;
                }
                case PRE_ANALYSIS_REPORT:
                {
	                pRpt->usReport = PRE_ANALYSIS_REPORT;
		              strcpy(pRpt->szReport, "Pre-Analysis Report");
                  break;
                }
                case REDUNDANCY_REPORT:
                {
                  FILE *hInput = NULL;
                  CHAR szInFile[256];

                  strcpy(szInFile,pRpt->szFolderObjName);
                  strcat(szInFile,"\\property\\redund.log");

                  if ((hInput = fopen( szInFile, "rb" )) == NULL)
                  {
                      usRC = MESSAGE_RPT_NO_REDUND_AVAIL;
                      pszParm = pRpt->szLongFolderName;
                  }
                  else
                      fclose( hInput );

	                pRpt->usReport = REDUNDANCY_REPORT;
		              strcpy(pRpt->szReport, "Redundancy Report");
                  break;
                }
                case COMBINED_REPORT:
                {
                    FILE *hInput = NULL;
                    CHAR szInFile[256];

                    strcpy(szInFile,pRpt->szFolderObjName);
                    strcat(szInFile,"\\property\\redund.log");

                    if ((hInput = fopen( szInFile, "rb" )) == NULL)
                    {
                      usRC = MESSAGE_RPT_NO_REDUND_AVAIL;
                      pszParm = pRpt->szLongFolderName;
                    }
                    else
                      fclose( hInput );

	                pRpt->usReport = COMBINED_REPORT;
    		          strcpy(pRpt->szReport, "Redundant Segment List");
                  break;
                }
            }
           } /* endi f*/

           // get default profile if none specified
           if ( (pszProfile == NULL) || (*pszProfile == EOS) )
           {
             if ( ppropFolder->szProfile[0] != EOS )
             {
               CHAR szProfilePath[MAX_EQF_PATH];

               UtlMakeEQFPath( szProfilePath, NULC, PROPERTY_PATH, NULL );
               strcat( szProfilePath, BACKSLASH_STR );
               strcat( szProfilePath, ppropFolder->szProfile );

               if ( UtlFileExist(szProfilePath)  )
               {
                 Utlstrccpy( szProfile, ppropFolder->szProfile, DOT );
                 pszProfile = szProfile;
               } /* endif */
             }
             else
             {
               pszProfile = NULL;
             } /* endif */
           } /* endif */

           // update profile name in folder properties if a profile has been specified
           if ( (pszProfile != NULL) && (*pszProfile != EOS) )
           {
              CHAR szProfilePath[MAX_EQF_PATH];
              CHAR szProfileName[MAX_EQF_PATH];

              strcpy( szProfileName, pszProfile );
              UtlUpper( szProfileName );
              strcat( szProfileName, ".R00" );

              UtlMakeEQFPath( szProfilePath, NULC, PROPERTY_PATH, NULL );
              strcat( szProfilePath, BACKSLASH_STR );
              strcat( szProfilePath, szProfileName );

              if ( UtlFileExist(szProfilePath)  )
              {
                if ( stricmp( ppropFolder->szProfile, szProfileName ) != 0 )
                {
                  if ( SetPropAccess( hpropFolder, PROP_ACCESS_WRITE) )
                  {
                  strcpy( ppropFolder->szProfile, szProfileName );
                  SaveProperties( hpropFolder, &ulErrorInfo );
                  ResetPropAccess( hpropFolder, PROP_ACCESS_WRITE );
                  } /* endif */                  
                } /* endif */                  
              } /* endif */                  
           } /* endif */              

           CloseProperties(hpropFolder,PROP_FILE,&ulErrorInfo);
        }
        else
        {
           CloseProperties(hpropFolder,PROP_FILE,&ulErrorInfo);
           usRC = ERROR_FOLDER_READ_FAILED;
           pszParm = pRpt->szFolder;
        } // endif
    } // end if


	if (!usRC)
	{
		switch (pReportType->lRepType)
		{
			case 1 :   // only BASE_TYP
			{
				strcpy(pRpt->szOption,"Base");
				pRpt->usOptions = 0;
				break;
			}
			case 2 :   // only FACT_TYP
			{
				strcpy(pRpt->szOption,"Fact Sheet");
				pRpt->usOptions = 4;
				break;
			}
			case 3 :   // not acceptable: BASE_TYP and FACT_TYP
			{
				usRC = ERROR_UNACCEPTABLE_REP_TYPE;
				pszParm = pRpt->szOption;
				break;
			}
			case 4 :   // only SUM_TYP
			{
				strcpy(pRpt->szOption,"Summary");
				pRpt->usOptions = 5;
				break;
			}
			case 5 :   // BASE_TYP and SUM_TYP
			{
				strcpy(pRpt->szOption,"Base & Summary");
				pRpt->usOptions = 1;
				break;
			}
			case 6 :   // FACT_TYP and SUM_TYP
			{
				strcpy(pRpt->szOption,"Summary & Fact Sheet");
				pRpt->usOptions = 3;
				break;
			}
			case 7 :   // BASE_TYP and FACT_TYP and SUM_TYP
			{
				strcpy(pRpt->szOption,"Base,Summary & Fact Sheet");
				pRpt->usOptions = 2;
				break;
			}
      case BRIEF_SORTBYDATE_TYP:
			{
				strcpy(pRpt->szOption,"Brief, sort by Date");
				pRpt->usOptions = BRIEF_SORT_BY_DATE;
				break;
			}
      case BRIEF_SORTBYDOC_TYP:
			{
				strcpy(pRpt->szOption,"Brief, sort by Date");
				pRpt->usOptions = BRIEF_SORT_BY_DOCUMENT;
				break;
			}
      case DETAIL_TYP:
			{
				strcpy(pRpt->szOption,"Detail");
				pRpt->usOptions = DETAIL;
				break;
			}
			default:
			{
				usRC = ERROR_NO_REP_TYPE;
				pszParm = pRpt->szOption;
			}
		} // end switch
	} // end if


	if (!usRC)
	{
		// report description
		if (pReportType->pszDescription != NULL)
			strcpy(pRpt->szRptDescription, pReportType->pszDescription);
		else  // valid
			pReportType->pszDescription = NULL;
	} // end if


	if (!usRC)
	{
		// report to be stored
		if (pszOutfileName == NULL)
		{
			usRC = TQM_REPORT_NAME_INVALID;
			pszParm = pszOutfileName;
		}
		else
		{
			strcpy(pRpt->szRptOutputFile, pszOutfileName);
			pRpt->fRptFile = TRUE;
            pRpt->usOutputFileProcess = TRUE;
		}
	} // end if


  // check any specified shipment value
  if ( !usRC )
  {
    if ( (pszShipment != NULL) && (*pszShipment != EOS) )
    {
      if ( (stricmp( pszShipment, "Single Shipments" ) != 0) && (stricmp( pszShipment, "All Shipments" ) != 0) )
      {
        long lShipment = atol( pszShipment );
        if ( (lShipment < 1) || (lShipment > 99) )
        {
          usRC = ERROR_INVALID_SHIPMENT_NUMBER;
          pszParm = pszShipment;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

	if (!usRC)
	{
		// retrieve the output file format
		if (pszFormat == NULL)
		{
			usRC = ERROR_NO_OUTPUT_FILE_FORMAT;
			pszParm = pszFormat;
		}
		else if (!strcmp(pszFormat, "ASCII"))
		{
			strcpy(pRpt->szFormat, pszFormat);
			pRpt->usFormat = ASCII;
		}
		else if (!strcmp(pszFormat, "HTML"))
		{
			strcpy(pRpt->szFormat, pszFormat);
			pRpt->usFormat = HTML;
		}
		else if (!strcmp(pszFormat, "RTF"))
		{
			strcpy(pRpt->szFormat, pszFormat);
			pRpt->usFormat = RTF;
		}
		else if (!strcmp(pszFormat, "XML"))
		{
			strcpy(pRpt->szFormat, pszFormat);
			pRpt->usFormat = XMLFILEFORMAT;
		}
		else
		{
			usRC = ERROR_WRONG_OUTPUT_FILE_FORMAT;
			pszParm = pszFormat;
		}
	} // end if


	if (!usRC)
	{
		if ( pszProfile == NULL )
    {
			strcpy( pRpt->szProfile, "" );
    }
		else if ( stricmp( pszProfile, "NONE" ) == 0 )
    {
			strcpy( pRpt->szProfile, "" );
    }
		else
    {
           // retrieve the profile
            HPROP        hpropFolder;
            PPROPFOLDER  ppropFolder;
            ULONG        ulErrorInfo;
            CHAR         szProfiles[MAX_PATH144+MAX_FILESPEC]; // profile names
            PSZ          pszTemp;                     // tmp pointer
            SHORT        i,j ;                        // counter

            strcpy( pRpt->szProfile, pszProfile);
            strcat( pRpt->szProfile, ".R00" );
			      strcpy( szProfiles, pRpt->szProfile );

            UtlMakeEQFPath( szProfiles, NULC, PROPERTY_PATH,NULL );
            pszTemp = UtlSplitFnameFromPath(szProfiles);
            strcat(szProfiles , BACKSLASH_STR );
            strcat(szProfiles , pRpt->szProfile );

            if ((hpropFolder = OpenProperties(szProfiles, NULL,PROP_ACCESS_READ,&ulErrorInfo )) != NULL )
            {
              ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd(hpropFolder);

              pRpt->usOption[0]   =  ppropFolder->usOption[0]   ;
              pRpt->usOption[1]   =  ppropFolder->usOption[1]   ;
              pRpt->usOption[2]   =  ppropFolder->usOption[2]   ;
              pRpt->usOption[3]   =  ppropFolder->usOption[3]   ;
              pRpt->usOption[4]   =  ppropFolder->usOption[4]   ;
              pRpt->usOption[5]   =  ppropFolder->usOption[5]   ;
              pRpt->usOption1     =  ppropFolder->usOption1     ;
              pRpt->usOption2     =  ppropFolder->usOption2     ;
              pRpt->usOption3     =  ppropFolder->usOption3     ;
              pRpt->usOption4     =  ppropFolder->usOption4     ;
              pRpt->usOption5     =  ppropFolder->usOption5     ;
              pRpt->usOption21    =  ppropFolder->usOption21    ;
              pRpt->usOption22    =  ppropFolder->usOption22    ;
              pRpt->usColumns[0]  =  ppropFolder->usColumns[0]  ;
              pRpt->usColumns[1]  =  ppropFolder->usColumns[1]  ;
              pRpt->usColumns[2]  =  ppropFolder->usColumns[2]  ;
              pRpt->usColumns[3]  =  ppropFolder->usColumns[3]  ;
              pRpt->usColumns4[0]  =  ppropFolder->usColumns4[0] ;
              pRpt->usColumns4[1]  =  ppropFolder->usColumns4[1]  ;
              pRpt->usColumns4[2]  =  ppropFolder->usColumns4[2]  ;
              pRpt->usColumns4[3]  =  ppropFolder->usColumns4[3]  ;
              pRpt->usColumns4[4]  =  ppropFolder->usColumns4[4]  ;
              pRpt->usStandard    =  ppropFolder->usStandard    ;
              pRpt->Pay_per_Standard = ppropFolder->Pay_per_Standard;
              pRpt->usCurrency    =  ppropFolder->usCurrency ;
              strcpy(pRpt->szShipment, ppropFolder->szShipment);
              pRpt->usShipmentChk = ppropFolder->usShipmentChk;

              for (i=0;i<MAX_PAYREPORT_COLUMNS;i++)
              {
                for (j=0;j<3;j++)
                {
                  pRpt->Complexity_Factor[i][j] = ppropFolder->Complexity_Factor[i][j];
                  pRpt->Pay_Factor[i][j] = ppropFolder->Pay_Factor[i][j];

                }  //end for
              } //end for

              CloseProperties(hpropFolder,PROP_FILE,&ulErrorInfo);
            }
            else
            {
                CloseProperties(hpropFolder,PROP_FILE,&ulErrorInfo);
                usRC = ERROR_PROFILE_LOAD;
                pszParm = pszProfile;
            } /* endif */
        } // end if
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		// retrieve the specified count number
		if (pRepSettings->pszCountType == NULL)
		{
			usRC = ERROR_NO_COUNTING_TYPE;
			pszParm = pRepSettings->pszCountType;
		}
		else if (!strcmp(pRepSettings->pszCountType, "Source Words"))
			pRpt->usOption1 = 0;
		else if (!strcmp(pRepSettings->pszCountType, "Target Words"))
			pRpt->usOption1 = 1;
		else if (!strcmp(pRepSettings->pszCountType, "Segments"))
			pRpt->usOption1 = 2;
		else if (!strcmp(pRepSettings->pszCountType, "Modified Words"))
			pRpt->usOption1 = 3;
		else
		{
			usRC = ERROR_WRONG_COUNTING_TYPE;
			pszParm = pRepSettings->pszCountType;
		}
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		// show categories
		if (pRepSettings->bShow == TRUE)
		{
			pRpt->usOption2 = 0;
		}
		else if (pRepSettings->bShow == FALSE)
		{
			pRpt->usOption2 = 1;
			pRpt->usColumns[0] = 0;         // switch off summary
		}
		else
		{
			usRC = ERROR_NO_BOOLEAN_VALUE;
			strcpy(szBuf, "pRepSettings->bShow");
			pszParm = szBuf;
		}
	} // end if


	if (!usRC && pszProfile == NULL)
	{
 		// show summary
		if (pRepSettings->bSummary == FALSE)
			pRpt->usColumns[0] = 0;
		else if (pRepSettings->bSummary == TRUE)
			pRpt->usColumns[0] = 1;
		else
		{
			usRC = ERROR_NO_BOOLEAN_VALUE;
			strcpy(szBuf, "pRepSettings->bSummary");
			pszParm = szBuf;
		}
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		// retrieve the specified layout
		if (pRepSettings->pszRepLayout == NULL)
		{
			pRpt->usOption3 = 0;
		}
		else if (!strcmp(pRepSettings->pszRepLayout, "Standard"))
			pRpt->usOption3 = 0;
		else if (!strcmp(pRepSettings->pszRepLayout, "Standard and Group-Summary"))
			pRpt->usOption3 = 1;
		else if (!strcmp(pRepSettings->pszRepLayout, "Shrunk to Groups"))
			pRpt->usOption3 = 2;
		else
		{
			pRpt->usOption3 = 0;
		}
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		// automatic shrink
		if (pRepSettings->bShrink == FALSE)
			pRpt->usColumns[1] = 0;
		else if (pRepSettings->bShrink == TRUE)
			pRpt->usColumns[1] = 1;
		else
		{
			pRpt->usColumns[1] = 1;
		}
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		if (pRepSettings->pszStatisticType == NULL)   // valid
		{
			pRpt->usColumns[2] = 0;
			pRpt->usColumns[3] = 0;
		}

		// check the statistics type against the report
		else if (!strcmp(pReportType->pszReport, "Calculating Report"))
		{
			if (!strcmp(pRepSettings->pszStatisticType, "Standard"))
			{
				pRpt->usOption4 = 0;
				pRpt->usColumns[2] = 1;
			}
			else if (!strcmp(pRepSettings->pszStatisticType, "Advanced"))
			{
				pRpt->usOption4 = 1;
				pRpt->usColumns[2] = 1;
			}
			else
			{
				pRpt->usOption4 = 0;
				pRpt->usColumns[2] = 1;
			}
		} // end if
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		// use existing proposals
		if (pRepSettings->bExProposal == FALSE)
			pRpt->usColumns[3] = 0;
		else if (pRepSettings->bExProposal == TRUE)
			pRpt->usColumns[3] = 1;
		else
		{
			pRpt->usColumns[3] = 1;
		}
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		// initial set of factors for final factsheet
		for (usCnti=0; usCnti<COLUMN; usCnti++)
		{
		  for (usCntj=0; usCntj<CATEGORY; usCntj++)
		  {
			pRpt->Complexity_Factor[usCnti][usCntj] = 1.0;
			pRpt->Pay_Factor[usCnti][usCntj] = 1.0;
		  }  //end for
		} //end for
	}


	if (!usRC)
	{
		if (usColumn > COLUMN || usCategory > CATEGORY)
		{
			usRC = ERROR_EXCEEDING_VALUE;
            if (usColumn > COLUMN)     _itoa( usColumn, szBuf, 10 );
            if (usCategory > CATEGORY) _itoa( usCategory, szBuf, 10 );
			pszParm = szBuf;
		}

		if (!usRC && pszProfile == NULL)
		{
			// copy values by means of uColumn and uCategory to the internal factsheet
      if ( pFactSheetArg )
      {
			  for (usCnti=0; usCnti<usColumn; usCnti++)
			  {
			    for (usCntj=0; usCntj<usCategory; usCntj++)
			    {
				  pRpt->Complexity_Factor[usCnti][usCntj] = (float)pFactSheetArg->lComplexity;
				  pRpt->Pay_Factor[usCnti][usCntj] = (float)pFactSheetArg->lPayFactor;
				  pFactSheetArg++;
			    }  // end for
			  } // end for
      } /* endif */
		} // end if
	}


	if (!usRC && pszProfile == NULL)
	{
		// The first array index represents the column number
		pRpt->usOption21 = 30;

		// The second array index represents the category number
		pRpt->usOption22 = 10;
	}


	if (!usRC && pszProfile == NULL)
	{
    if ( pFinalFactors == NULL  )
    {
  	  pRpt->usStandard = RPT_UNITY;
    }
    else
    {
		  switch (pFinalFactors->lUnit)
		  {
			  case 1   :		// by word
			  {
				  pRpt->usStandard = RPT_UNITY;
				  break;
			  }
			  case 10  :		// by line
			  {
				  pRpt->usStandard = RPT_LINE;
				  break;
			  }
			  case 250 :		// by page
			  {
				  pRpt->usStandard = RPT_PAGE;
				  break;
			  }
			  default:
			  {
				  usRC = ERROR_WRONG_PAY_STANDARD;
				  _itoa( pRpt->usStandard, szBuf, 10 );
				  pszParm = szBuf;
			  }
		  } // end switch
    } /* endif */
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		// Local Currency Factor.
    if ( pFinalFactors == NULL )
    {
	  	pRpt->Pay_per_Standard = 1.0;
    }
    else
    {
  		// Local Currency Factor.
	  	pRpt->Pay_per_Standard = (float)pFinalFactors->lCurrFactor;
    }
	}


	if (!usRC)
	{
        BOOL fFound = FALSE;
        USHORT usI = 0;

        if (pszProfile != NULL)
        {
            strcpy(pRpt->szCurrency, szRptCurrency2[pRpt->usCurrency]);
        }
        else if ( pFinalFactors == NULL )
        {
          strcpy(pRpt->szCurrency, "" );
        }
        else
        {
            // Local Currency.
    		    if (pFinalFactors->pszLocalCurrency)
            {
                for (usI = 0; usI <= MAX2_OPTIONS4; usI++)
                {
                    if (!strcmp(pFinalFactors->pszLocalCurrency, szRptCurrency2[usI]))
                    {
                        fFound = TRUE;
                        break;
                    }
                }
            }

            if (fFound == TRUE)
            {
                strcpy(pRpt->szCurrency, pFinalFactors->pszLocalCurrency);
                pRpt->usCurrency = usI;
            }
            else
            {
			    usRC = ERROR_NO_VALID_CURRENCY;
                strcpy(szBuf, pFinalFactors->pszLocalCurrency);
		        pszParm = szBuf;
		    }
        } // end else
	} // end if


	if (!usRC && pszProfile == NULL)
	{
        pRpt->usColumns4[0] = 0;
		pRpt->usColumns4[1] = 0;
		pRpt->usColumns4[2] = 0;

		switch (lOptSecurity)
		{
			case 0 :   // use initial values
				break;
			case 1 :   // only PLAUS_OPT     -- (Plausibilty check)
			{
				pRpt->usColumns4[0] = 1;
				break;
			}
			case 2 :   // only LOST_OPT		 -- (Lost Data: Force new shipment)
			{
				pRpt->usColumns4[2] = 1;
				break;
			}
			case 3 :   // PLAUS_OPT and LOST_OPT
			{
				pRpt->usColumns4[0] = 1;
				pRpt->usColumns4[2] = 1;
				break;
			}
			case 4 :   // only LIST_OPT      -- (List of Documents)
			{
				pRpt->usColumns4[1] = 1;
				break;
			}
			case 5 :   // PLAUS_OPT and LIST_OPT
			{
				pRpt->usColumns4[0] = 1;
				pRpt->usColumns4[1] = 1;
				break;
			}
			case 6 :   // LOST_OPT and LIST_OPT
			{
				pRpt->usColumns4[2] = 1;
				pRpt->usColumns4[1] = 1;
				break;
			}
			case 7 :   // PLAUS_OPT and LOST_OPT and LIST_OPT
			{
				pRpt->usColumns4[0] = 1;
				pRpt->usColumns4[1] = 1;
				pRpt->usColumns4[2] = 1;
				break;
			}
			default:
			{
				usRC = ERROR_WRONG_SECURITY_OPTION;
				pszParm = szBuf;
			}
		} // end switch
	} // end if


	if (!usRC )
	{
		// folder shipment allways turned on
		pRpt->usShipmentChk = 1;

	    if ( (pszShipment == NULL) || (*pszShipment == EOS) )
	    {
		    // shipment type
		    if (bSingleShipment == TRUE)
		    {
			    strcpy(pRpt->szShipmentChk, "Single Shipments");
		    }
		    else if (bSingleShipment == FALSE)
		    {
			    strcpy(pRpt->szShipmentChk, "All Shipments");
		    }
		    else
		    {
			    usRC = ERROR_NO_BOOLEAN_VALUE;
			    strcpy(szBuf, "pRepSettings->szShipmentChk");
			    pszParm = szBuf;
		    }
      }
      else
      {
        strcpy( pRpt->szShipmentChk, pszShipment );
      } /* endif */
	} // end if


	if (!usRC && pszProfile == NULL)
	{
		pRpt->usOption5 = 1;
		pRpt->usOption21 = 0; // 30;
		pRpt->usOption22 = 0; // 10;
    }


    // do it anyway
    {
		pRpt->usRptStatus = RPT_ACTIVE;
		pRpt->fBatch = TRUE;
    pRpt->iShipmentsUsed = 0;
		pRpt->hHTMLControl = NULL;
	}


	if (!usRC)
    {
//		usRC = (USHORT)RptMain (FALSE, NULL, pRpt);
		usRC = (USHORT)RPTXMLREPORT(FALSE, 0, pRpt);
        // RptMain returns fOk == 1 when all is ok.
        if(usRC == 1)
            usRC = 0;
    }
	else
		UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );

  if ( fFolderLocked ) REMOVESYMBOL( pszObjName );

  if ( pRpt )
  {
    if ( pRpt->pszActualDocument ) UtlAlloc( (PVOID *)&(pRpt->pszActualDocument), 0L, 0L, NOMSG );
    UtlAlloc ((PVOID*)&pRpt, 0L, 0L, NOMSG );
  }


    return usRC;
} // end of function MemFuncCreateCntReport


