
//+----------------------------------------------------------------------------+
//|EQFCNT01.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Description: Word Count dialog procedure to get input for word count        |
//+----------------------------------------------------------------------------+

#define INCL_EQF_DLGUTILS         // set output file dialog
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_WCOUNT           // word count functions
#define INCL_EQF_DICTPRINT        // dictionary print functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdde.h"               // batch mode defintions
#ifdef FUNCCALLIF
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines
#endif
#include "eqfwcnti.h"             // Private include file for wordcount
#include <eqfcnt01.id>            // word count dialog id file

#undef _WPTMIF                         // we don't care about WP I/F
#include "eqfhelp.id"                  // help resource IDs
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                          // message help table

#include "core\pluginmanager\OtmMorph.h"

extern HELPSUBTABLE hlpsubtblWCountDlg[];

//--- function prototypes
static VOID InitWordCountDlg( HWND, LPARAM );
static VOID WordCountWM_COMMAND( HWND, WPARAM, LPARAM );
static VOID HandleCountButton( HWND );
static MRESULT WordCountWM_CONTROL( HWND, SHORT, SHORT );
void CntFormatFillCB( HWND hwnd, int id );
USHORT CntFormatGetSelected( HWND hwnd, int id );
void CntFormatSelect( HWND hwnd, int id, USHORT usFormat );
BOOL CntAdjustReportFileExtension( PSZ pszFileName, USHORT usFormat );

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     WORDCOUNTDLGPROC                                         |
//+----------------------------------------------------------------------------+
//|Function call:     MRESULT APIENTRY WORDCOUNTDLGPROC( HWND   hwnd,          |
//|                                                      USHORT msg,           |
//|                                                      MPARAM mp1,           |
//|                                                      MPARAM mp2 )          |
//+----------------------------------------------------------------------------+
//|Description:       Word Count dialog proedure                               |
//+----------------------------------------------------------------------------+
//|Parameters:        hwnd  dialog handle                                      |
//|                   msg   message type                                       |
//|                   mp1   message parameter 1                                |
//|                   mp2   message parameter                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       depending on messages                                    |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|mResult = FALSE                                                             |
//|                                                                            |
//|switch ( message )                                                          |
//|   case WM_INITDLG :                                                        |
//|      call function InitWordCountDlg                                        |
//|      break                                                                 |
//|   case WM_COMMAND :                                                        |
//|      call function WordCountWM_COMMAND                                     |
//|      break                                                                 |
//|   case WM_CONTROL :                                                        |
//|      call function WordCountWM_CONTROL                                     |
//|      break                                                                 |
//|   case WM_CLOSE :                                                          |
//|      if ( access to IDA is ok )                                            |
//|         if ( mp1 ) (==start count instance)                                |
//|            save Properties  and do error handling                          |
//|         else                                                               |
//|            set mp1 = FALSE (==do not start count instance)                 |
//|         endif                                                              |
//|      else                                                                  |
//|         free IDA                                                           |
//|      endif                                                                 |
//|      distroy dialog ( mp1 is passed to tell the count hanldler to start    |
//|                       or not start the count instance )                    |
//|      break                                                                 |
//|   default :                                                                |
//|      mResult = UtlDefDialogProc                                            |
//|      break                                                                 |
//|endswitch                                                                   |
//|                                                                            |
//|return mResult                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK  WORDCOUNTDLGPROC( HWND   hwnd,
                                   WINMSG msg,
                                   WPARAM mp1,
                                   LPARAM mp2 )
{
  PIDA         pIda;                       //pointer to dialog instance area
  ULONG        ulErrorInfo;                //error indicator from PRHA
  MRESULT      mResult=FALSE;              //function return value

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_WORDCNT_DLG, mp2 ); break;
      //-----------------------------------------------------------------------
    case ( WM_INITDLG ) :
      //--- call function InitWordCountDlg to initialize dialog
      //--- mp2 contains pointer to count datastructure passed from
      //--- count Handler CountHandler_WP
      InitWordCountDlg( hwnd, mp2 );
      mResult = (MRESULT)TRUE;
      break;
      //-----------------------------------------------------------------------
    case ( WM_COMMAND ) :
      //--- call function to handle WM_COMMAND message
      WordCountWM_COMMAND( hwnd, mp1, mp2 );
      break;

    case ( WM_EQF_CLOSE ) :
      //--- mp1 : TRUE -> start start word count window
      //--- mp2 : not used

      pIda = ACCESSDLGIDA( hwnd, PIDA );
      if ( pIda )
      {
        if ( mp1 )   //--- start count window
        {
          //--- SAVE and close folder properties
          //--- return of SaveProperties !0 ==> error
          //--- SaveProperties displayes a error message
          if ( !SaveProperties( pIda->hpropFolder, &ulErrorInfo ) )
          {
            //--- SaveProperties returns no error => close properties
            if ( pIda->hpropFolder ) CloseProperties
              ( pIda->hpropFolder,
                PROP_FILE, &ulErrorInfo );
          }
          else //--- SaveProp. returns error => do not start count instance
          {
            //--- do not start count window
            mp1 = FALSE;
          } /* endif */
        }
        else   //--- !mp1 => do not start count instance
        {
          //--- QUIT and close folder properties
          if ( pIda->hpropFolder ) CloseProperties
            ( pIda->hpropFolder,
              PROP_QUIT,  &ulErrorInfo );
        } /* endif */
        //--- free IDA
        UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
      }/*endif*/

      DelCtrlFont(hwnd, DID_CNT_LB);
      //--- destroy dialog pass flag in mp1, to tell the count handler if
      //--- count instance should be started or not
      //--- mp1 == FALSE : do not start count instance
      //--- mp1 == TRUE  : start count instance
      WinDismissDlg( hwnd, SHORT1FROMMP1(mp1) );
      break;
      //-----------------------------------------------------------------------

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblWCountDlg[0] );
      mResult = TRUE;  // message processed
      break;


    default :
      //--- call default dialog procedure
      mResult = UTLDEFDIALOGPROC( hwnd, msg, mp1, mp2 );
      break;
  }/*end switch (msg)*/
  return( mResult );
}/*end WORDCOUNTDLGPROC*/

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     InitWordCountDlg                                         |
//+----------------------------------------------------------------------------+
//|Function call:     VOID InitWordCountDlg ( HWND hwnd, LPARAM mp2 )          |
//+----------------------------------------------------------------------------+
//|Description:       Initializes the Word Count dialog                        |
//|                   handles the WM_INITDLG message of procedure              |
//|                   WORDCOUNTDLGPROC                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        hwnd  dialog handle                                      |
//|                   mp2   message parameter to                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|fOk = TRUE                                                                  |
//|                                                                            |
//|if ( allocation of IDA is done with no error )                              |
//|   if ( connection of IDA with dialog fails )                               |
//|      display error message, close dialog and set fOk = FALSE               |
//|   endif                                                                    |
//|else                                                                        |
//|   close dialog and set fOk = FALSE                                         |
//|endif                                                                       |
//|if ( fOk )                                                                  |
//|   do initialization of variables                                           |
//|   if ( error opening properties )                                          |
//|      display error message, close dialog and set fOk = FALSE               |
//|   endif                                                                    |
//|endif                                                                       |
//|                                                                            |
//|if ( fOk )                                                                  |
//|   get write access to properties and fill files to be counted into countLB |
//|   if ( no files to be counted )                                            |
//|      display error message, close dialog and set fOk = FALSE               |
//|   endif                                                                    |
//|endif                                                                       |
//|if ( fOk )                                                                  |
//|   select all files in countlistbox                                         |
//|   select radio buttons in depency of last used values                      |
//|   fill document type listbox                                               |
//|   set file checkbox and file entryfiled in dependency of last used values  |
//|endif                                                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
static
VOID InitWordCountDlg ( HWND hwnd, LPARAM mp2)
{
  PIDA        pIda;               //instance data area
  SHORT       sNum;               //number of LB items returned by CopyListBox
  PSZ         pszReplace;         //pointer to replace string UtlError
  ULONG       ulErrorInfo;        //error indicator from PRHA
  BOOL        fOk = TRUE;         //error indicator

  if ( UtlAlloc( (PVOID *) &pIda, 0L, (ULONG)sizeof( IDA ), ERROR_STORAGE ) ) //no alloc
  {
    //error
    if ( !ANCHORDLGIDA( hwnd, pIda ) )
    {                                                        //IDA fails
      //--- set fOk to FALSE to stop dialog
      fOk = FALSE;
      //--- display error message system error
      UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
      //--- post message to close dialog, do not start count instance
      WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
    } /* endif */
  }
  else   //--- allocation error
  {
    //--- set fOk to FALSE to stop dialog
    fOk = FALSE;
    //--- post message to destroy dialog, do not start count instance
    WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
  }/*end if*/

  if ( fOk )
  {
    //--- select display on screen CB by default,
    SETCHECK_TRUE ( hwnd, DID_CNT_SCREEN_CHK  );

    //--- disable screen CB
    WinEnableWindow( WinWindowFromID(hwnd, DID_CNT_SCREEN_CHK ),
                     FALSE);

    //--- deselect all items in count listbox
    DESELECTITEM( hwnd, DID_CNT_LB, LIT_NONE );

    //--- set file entry field length limit
    SETTEXTLIMIT( hwnd, DID_CNT_FILE_EF, MAX_PATH144 - 1 );
    //--- save pointer to count structure passed in mp2 to IDA
    pIda->pCnt = (PCNT) mp2;

    //--- build name of folder properties
    //--- get system path
    UtlMakeEQFPath( pIda->szFolderPropName, NULC,
                    SYSTEM_PATH,(PSZ) NULP );
    //--- append folder name to systempath
    strcat( pIda->szFolderPropName, "\\" );
    strcat( pIda->szFolderPropName, pIda->pCnt->szFolderName );

    if ( ( pIda->hpropFolder =                                //open folder
            OpenProperties( pIda->szFolderPropName, NULL,      //properties fails
                            PROP_ACCESS_READ, &ulErrorInfo)) == NULL)   //foldername set
    //from handler
    {
      //--- set fOk to FALSE to stop dialog
      fOk = FALSE;
      //--- display error message
      pszReplace = pIda->szFolderPropName;
      UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszReplace, EQF_ERROR);

      //--- post message to close dialog, do not start count instance
      WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
    }/*endif*/
  }/*endif fOk*/

  if ( fOk )
  {
    HWND hwndDocLB = NULLHANDLE;

    // create a temporary listbox for the document names
    hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "", 0L, 0, 0, 0, 0,
                                 hwnd, HWND_TOP, 1, NULL, NULL );


    //--- set write access to properties
    SetPropAccess( pIda->hpropFolder, PROP_ACCESS_WRITE);
    //--- get pointer to folder properties and save it to ida
    pIda->ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( pIda->hpropFolder );

    //--- documents selected in folder
    if ( EqfQueryObjectStatus( EqfQueryActiveFolderHwnd()) & OBJ_FOCUS )
    {
      pIda->pCnt->usFillMode = WM_EQF_QUERYSELECTEDNAMES;
      //--- insert selected documents in listbox
      EqfSend2Handler ( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                        MP1FROMHWND(hwndDocLB),
                        (LPARAM) pIda->pCnt->szParentObjName );
    }
    else   //--- folder selected in folder list
    {
      pIda->pCnt->usFillMode = WM_EQF_INSERTNAMES;
      //--- insert all documents in listbox
      EqfSend2Handler ( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                        MP1FROMHWND(hwndDocLB),
                        (LPARAM) pIda->pCnt->szParentObjName );
    }/*endif*/


    // get document long names and add them to the visible doc listbox
    {
      SHORT sIndex;                  // listbox item buffer
      SHORT sNumOfDocs;              // number of documents in listbox
      HPROP         hPropDocument;   // handle to document properties
      PPROPDOCUMENT pPropDocument;   // pointer to document properties
      ULONG         ulErrorInfo;     // error indicator from PRHA

      sIndex = 0;
      sNumOfDocs =  QUERYITEMCOUNTHWND( hwndDocLB );
      while ( sIndex < sNumOfDocs )
      {
        CHAR  szDocument[MAX_FILESPEC];

        QUERYITEMTEXTHWND( hwndDocLB, sIndex, szDocument );
        hPropDocument = OpenProperties( szDocument,
                                        pIda->pCnt->szFolderObjName,
                                        PROP_ACCESS_READ, &ulErrorInfo );
        if ( hPropDocument )
        {
          pPropDocument = (PPROPDOCUMENT) MakePropPtrFromHnd( hPropDocument );

          if ( pPropDocument->szLongName[0] != EOS )
          {
            strcpy( pIda->szWorkString, pPropDocument->szLongName );
            OEMTOANSI( pIda->szWorkString );
            INSERTITEM( hwnd, DID_CNT_LB, pIda->szWorkString );
          }
          else
          {
            INSERTITEM( hwnd, DID_CNT_LB, szDocument );
          } /* endif */
          CloseProperties( hPropDocument, PROP_QUIT, &ulErrorInfo );
        } /* endif */
        sIndex ++;
      } /* endwhile */


      {

        HWND hwndLB = GetDlgItem(hwnd, DID_CNT_LB);
        UtlSetHorzScrollingForLB(hwndLB);

      }
    }

    if ( (sNum = QUERYITEMCOUNT( hwnd, DID_CNT_LB )) == 0 )   //--- no files in listbox
    {
      //--- dislay warning message
      UtlError( NO_FILE_SELECTED, MB_CANCEL, 0, NULL, EQF_ERROR );
      //--- set fOk to FALSE to stop dialog
      fOk = FALSE;
      //--- post message to close dialog, do not start count instance
      WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
    }
    else
    {
      //--- select all files in count listbox
      while ( sNum-- )
      {
        SELECTITEM( hwnd, DID_CNT_LB, sNum );
      } /* endwhile */
    }/*endif*/

    if ( hwndDocLB != NULLHANDLE )
    {
      WinDestroyWindow( hwndDocLB );
    } /* endif */
  }/*endif fOk*/

  if ( fOk )
  {
    //--- set entry filed length limit
    SETTEXTLIMIT( hwnd, DID_CNT_FILE_EF, MAX_PATH144 - 1 );
    //--- select radio buttons in dependency of last used values from folder prop.
    switch ( pIda->ppropFolder->fOrg )
    {
      case FUZZYMATCH_STATE :
        SETCHECK_TRUE( hwnd, DID_CNT_FUZZYMATCH_RB );
        CLICK( hwnd, DID_CNT_FUZZYMATCH_RB );
        ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, FALSE);
        break;
      case DUPLICATE_STATE :
        SETCHECK_TRUE( hwnd, DID_CNT_DUPLICATES_RB );
        CLICK( hwnd, DID_CNT_DUPLICATES_RB );
        ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, FALSE);
        break;
      case TMMATCHES_STATE :
        SETCHECK_TRUE( hwnd, DID_CNT_TMMATCHES_RB );
        CLICK( hwnd, DID_CNT_TMMATCHES_RB );
        ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, TRUE );
        break;
      case FALSE           :
        CLICK( hwnd, DID_CNT_TRANSLATION_RB );
        ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, FALSE );
        break;
      case TRUE            :
        CLICK( hwnd, DID_CNT_ORIGINAL_RB );
        ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, FALSE );
        break;
      default              :
        CLICK( hwnd, DID_CNT_ORIGINAL_RB );
        ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, FALSE );
        break;
    } /* endswitch */

    if ( pIda->ppropFolder->fFile )                //last used value of file
    {
      //checkbox is selected
      SETCHECK_TRUE( hwnd, DID_CNT_FILE_CHK );
    }/*endif*/

    // fill format combobox and select last used value
    CntFormatFillCB( hwnd, DID_CNT_FORMAT_CB );
    CntFormatSelect( hwnd, DID_CNT_FORMAT_CB, pIda->ppropFolder->usCntFormat );

    //--- disable file selection dialog
    ENABLECTRL( hwnd, DID_CNT_FILE_SET_PB, pIda->ppropFolder->fFile );

    if ( pIda->ppropFolder->szOutputFile[0] != EOS )  //a last used value for
    {
      // entry filed exists
      //display last used value in entry field
      SETTEXT( hwnd, DID_CNT_FILE_EF, pIda->ppropFolder->szOutputFile );
    }
    else if ( pIda->ppropFolder->szOldOutputFile[0] != EOS )  //a last used value for
    {
      // entry filed exists
      //display last used value in entry field
      SETTEXT( hwnd, DID_CNT_FILE_EF, pIda->ppropFolder->szOldOutputFile );
    }
    else                                           //no last used value for
    {
      // entry filed exists
      //--- build last used value from active folder object name concatenated
      //--- with the default extension and dislplay it in entry field
      strncpy( pIda->ppropFolder->szOutputFile, pIda->pCnt->szFolderObjName, 3 );

      //--- append end of string
      pIda->ppropFolder->szOutputFile[3] = EOS;
      OEMTOANSI( pIda->pCnt->szLongFolderName );
      strcat( pIda->ppropFolder->szOutputFile, pIda->pCnt->szLongFolderName );
      ANSITOOEM( pIda->pCnt->szLongFolderName );
      strcat( pIda->ppropFolder->szOutputFile, "." );

      strcat( pIda->ppropFolder->szOutputFile, CNT_OUTPUT_EXTENSION );
      //display last used value in entry field
      SETTEXT( hwnd, DID_CNT_FILE_EF, pIda->ppropFolder->szOutputFile );
    }/*endif*/


    /****************************************************************/
    /* get markup table used                                        */
    /****************************************************************/
    strcpy( pIda->pCnt->szFormat, pIda->ppropFolder->szFormat );
    /****************************************************************/
    /* get memory used                                              */
    /****************************************************************/
    strcpy( pIda->pCnt->szMemory, pIda->ppropFolder->szMemory );
    SetCtrlFnt(hwnd, GetCharSet(), DID_CNT_LB, DID_CNT_FILE_EF );
  }/*endif fOk*/
}/*end InitWordCountDlg*/

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     WordCountWM_COMMAND                                      |
//+----------------------------------------------------------------------------+
//|Function call:     WordCountWM_COMMAND ( HWND hwnd, WPARAM mp1, LPARAM mp2 )|
//+----------------------------------------------------------------------------+
//|Description:       handles the WM_COMMAND message of dialog procedure       |
//|                   WORDCOUNTDLGPROC                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        hwnd  dialog handle                                      |
//|                   mp1 message parameter 1                                  |
//|                   mp2 message parameter 2                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|switch ( control ID )                                                       |
//|   case ( CANCEL button ) :                                                 |
//|   case ( ESC key )     ) :                                                 |
//|      close dialog do not start count instance                              |
//|      break                                                                 |
//|   case ( COUNT button ) :                                                  |
//|      call interna function HandleCountButton                               |
//|      break                                                                 |
//|endswitch                                                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
static
VOID WordCountWM_COMMAND ( HWND hwnd, WPARAM mp1, LPARAM mp2 )
{
  PIDA  pIda;                         //instance area

  mp2;
  switch ( WMCOMMANDID( mp1, mp2 ) )      //--- switch on control ID
  {
    case PID_PB_HELP:
      UtlInvokeHelp();
      break;
    //-----------------------------------------------------------------------
    case ( PID_PB_CANCEL ) :   //--- CANCEL button selected
    case ( DID_CANCEL )    :   //--- ESC key pressed
      //--- post message to close dialog, do start count instance
      WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
      break;
      //-----------------------------------------------------------------------
    case ( PID_PB_COUNT ) :    //--- Count button selected
      //--- call functiom HandleCountButton to handle OK button
      HandleCountButton( hwnd );
      break;
      //-----------------------------------------------------------------------
    case ( DID_CNT_FILE_SET_PB ) :   //--- select button selected


      {
        OPENFILENAME ofn;
        BOOL fOk = TRUE;

        TCHAR szTitle[64];
        TCHAR szT[256];
        PSZ   pszExtension;
        BOOL  fExtensionStripped = FALSE;
        USHORT usFormat = 0;

        pIda = ACCESSDLGIDA (hwnd, PIDA);

        usFormat = CntFormatGetSelected( hwnd, DID_CNT_FORMAT_CB );

        ofn.lStructSize                 = sizeof(ofn);
        ofn.hInstance                   = (HINSTANCE) UtlQueryULong(QL_HAB);
        ofn.lpstrFilter                 = "ASCII (*.CNT)\0*.CNT\0HTML (*.HTM)\0*.HTM\0XML (*.XML)\0*.XML\0\0";
        switch ( usFormat )
        {
          case ASCII_FORMAT : ofn.nFilterIndex = 1; break;
          case HTML_FORMAT  : ofn.nFilterIndex = 2; break;
          case XML_FORMAT   : ofn.nFilterIndex = 3; break;
          default           : ofn.nFilterIndex = 1; break;
        } /*endswitch */
        ofn.lpstrCustomFilter           = NULL;
        ofn.nMaxCustFilter              = 0;
        ofn.lpstrFileTitle              = szTitle;               // output
        ofn.nMaxFileTitle               = sizeof(szTitle);
        ofn.lpstrInitialDir             = NULL;
        ofn.lpstrTitle                  = "Select File" ;
        ofn.nFileOffset                 = 0;
        ofn.nFileExtension              = 0;
        ofn.lCustData                   = 0L;
        ofn.lpfnHook                    = NULL;
        ofn.lpTemplateName              = NULL;
        ofn.hwndOwner                   = hwnd;
        ofn.lpstrFile                   = szT;
        ofn.nMaxFile                    = sizeof(szT);
        ofn.Flags                       = OFN_HIDEREADONLY |
                                          OFN_NONETWORKBUTTON ;

        ofn.lpstrDefExt                 = NULL;

        szTitle[0] = TEXT('\0');  // output
        szT[0] = TEXT('\0');      // input
        QUERYTEXT (hwnd, DID_CNT_FILE_EF, szT);

        // Scan Extension of Input File Name
        // in case of standard extension, eliminate the extension

        pszExtension = strrchr(szT,'.');
        if (pszExtension )
        {
          if (!strcmp(pszExtension,".CNT"))
          {
            *(pszExtension) = EOS;
            fExtensionStripped = TRUE;
          } /* endif */
        } /* endif */

        fOk = GetSaveFileName(&ofn);

        if (fOk )
        {
          // update format
          if ( ofn.nFilterIndex == 3 )
          {
            CntFormatSelect( hwnd, DID_CNT_FORMAT_CB, XML_FORMAT );
          }
          else if ( ofn.nFilterIndex == 2 )
          {
            CntFormatSelect( hwnd, DID_CNT_FORMAT_CB, HTML_FORMAT );
          }
          else
          {
            CntFormatSelect( hwnd, DID_CNT_FORMAT_CB, ASCII_FORMAT );
          } /* endif */

          // if no extension use standard one
          // and standard Extension was stripped off
          if (fExtensionStripped )
          {
            pszExtension = strrchr(szT,'.');
            if (!pszExtension )
            {
              strcat(szT,".CNT");
            } /* endif */
          } /* endif */

          SETTEXT (hwnd, DID_CNT_FILE_EF, szT);
        } /* endif */
      }
      break;

    case DID_CNT_LB :   //-- selection in count LB
    case DID_CNT_FILE_CHK:
    case DID_CNT_TMMATCHES_RB:
    case DID_CNT_DUPLICATES_RB:
    case DID_CNT_TMREPLMATCH_CHK:
    case DID_CNT_TRANSLATION_RB:
    case DID_CNT_ORIGINAL_RB:
    case DID_CNT_FUZZYMATCH_RB:

      WordCountWM_CONTROL( hwnd,
                           WMCOMMANDID( mp1, mp2 ),
                           WMCOMMANDCMD( mp1, mp2 ) );
      break;
  }/*end switch*/
}/* end WordCountWM_COMMAND */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HandleCountButton                                        |
//+----------------------------------------------------------------------------+
//|Function call:     static VOID HandleCountButton ( HWND hwnd )              |
//+----------------------------------------------------------------------------+
//|Description:       function to handles the Ok button of WordCountDlgProc    |
//+----------------------------------------------------------------------------+
//|Parameters:        hwnd   dialog handle                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|fOk = TRUE                                                                  |
//|                                                                            |
//|get access to IDA                                                           |
//|get status of original and tranlation RB and save it                        |
//|get selected document type and do error handling, if error fOk = FALSE      |
//|if ( fOk )                                                                  |
//|   set document type in dependency of selected one                          |
//|   if ( fOk )                                                               |
//|      if ( output to file is selected )                                     |
//|         get filename from entry field and do error handling                |
//|         if ( fOK )                                                         |
//|            check filename and do error handling                            |
//|         endif                                                              |
//|      endif                                                                 |
//|   endif                                                                    |
//|endif                                                                       |
//|                                                                            |
//|if ( fOk )                                                                  |
//|   close dialog and start count instance                                    |
//|endif                                                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
static
VOID HandleCountButton ( HWND hwnd )
{
  PIDA        pIda;                        //instance area
  CHAR        szReplace[20];               //replace string for error message
  PSZ         pszReplace = szReplace;      //pointer to replace string
  HFILE       hfFileHandle;                //DOS file handle
  USHORT      usAction;                    //UtlOpen output
  USHORT      usRc;                        //USHORT return code
  LONG        lRc1;                       //USHORT return code from strnicmp
  LONG        lRc2;                       //USHORT return code from strnicmp
  USHORT      usResponse;                  //return from UtlError
  BOOL        fOk = TRUE;                  //error indicator
  USHORT      usIndex;                     //index of current drive in drivelist


  //--- get access to ida
  pIda = ACCESSDLGIDA( hwnd, PIDA );

//   //--- check if files are locked and lock files
//   fOk = LockCountFiles( hwnd, pIda->pCnt );

  if ( fOk )
  {
    //--- get status of original radio button and save it to folder properties
    //--- and  count structure
    if ( QUERYCHECK( hwnd, DID_CNT_TRANSLATION_RB ) )
    {
      pIda->ppropFolder->fOrg = FALSE;
      pIda->ppropFolder->fTran = TRUE;

    }
    else if ( QUERYCHECK( hwnd, DID_CNT_TMMATCHES_RB ) )
    {
      pIda->ppropFolder->fOrg = TMMATCHES_STATE;
      pIda->ppropFolder->fTran = FALSE;
    }
    else if ( QUERYCHECK( hwnd, DID_CNT_DUPLICATES_RB ) )
    {
      pIda->ppropFolder->fOrg = DUPLICATE_STATE;
      pIda->ppropFolder->fTran = FALSE;
    }
    else if ( QUERYCHECK( hwnd, DID_CNT_FUZZYMATCH_RB ) )
    {
      pIda->ppropFolder->fOrg = FUZZYMATCH_STATE;
      pIda->ppropFolder->fTran = FALSE;
    }
    else
    {
      pIda->ppropFolder->fOrg = TRUE;
      pIda->ppropFolder->fTran = FALSE;
    } /* endif */
    pIda->pCnt->fOrg = pIda->ppropFolder->fOrg;
    pIda->pCnt->fTran = pIda->ppropFolder->fTran;
    pIda->pCnt->fReplMatch = pIda->ppropFolder->fReplMatch = QUERYCHECK( hwnd, DID_CNT_TMREPLMATCH_CHK );
    pIda->pCnt->fInclMemMatch = pIda->ppropFolder->fInclMemMatch = QUERYCHECK( hwnd, DID_CNT_MEMMATCH_CHK );
  } /* endif */

  if ( fOk )
  {
    //--- get status of file checkbox and save it to folder properties
    //--- and count structure
    pIda->ppropFolder->fFile = QUERYCHECK( hwnd, DID_CNT_FILE_CHK );
    pIda->pCnt->fFile = pIda->ppropFolder->fFile;

    if ( fOk )
    {
      if ( pIda->pCnt->fFile )   //--- file checkbox is selected
      {
        // get report format
        pIda->ppropFolder->usCntFormat = pIda->pCnt->usFormat = CntFormatGetSelected( hwnd, DID_CNT_FORMAT_CB );
        pIda->pCnt->fXMLOutput = (pIda->pCnt->usFormat == HTML_FORMAT) || (pIda->pCnt->usFormat == XML_FORMAT);

        //--- get content of file entry field
        QUERYTEXT( hwnd, DID_CNT_FILE_EF, pIda->pCnt->szOutputFile );
        strupr( pIda->pCnt->szOutputFile );
        //--- isolate drive letter for error messages
        if ( pIda->pCnt->szOutputFile[1] == COLON )   //filespec contains
        {
          //a drive
          //--- save drive to count structured
          pIda->pCnt->szDriveLetter[0] = pIda->pCnt->szOutputFile[0];
          pIda->pCnt->szDriveLetter[1] = EOS;
        }
        else   //--- filespec contains
        {
          //--- no drive
          //--- get current drive
          usIndex = UtlGetDriveList( (BYTE *) pIda->szDrives );

          //--- save drive to count structured
          pIda->pCnt->szDriveLetter[0] = pIda->szDrives[usIndex];
          pIda->pCnt->szDriveLetter[1] = EOS;

          //--- prepend drive letter and colon to output file name
          memmove( &pIda->pCnt->szOutputFile[2],
                   pIda->pCnt->szOutputFile,
                   ( strlen( pIda->pCnt->szOutputFile ) +1 ) );
          pIda->pCnt->szOutputFile[0] = pIda->pCnt->szDriveLetter[0];
          pIda->pCnt->szOutputFile[1] = COLON;
        }/*endif*/

        //--- convert drive letter to uppercase for error message
        pIda->pCnt->szDriveLetter[0] =
        (CHAR)toupper( pIda->pCnt->szDriveLetter[0] );

        //--- get system path
        UtlMakeEQFPath( pIda->szSysPath, NULC, SYSTEM_PATH, NULL );

        //--- append backslash to system path
        strcat( pIda->szSysPath, "\\" );

        //--- compare if target path contains as first directory the
        //--- system path
        lRc1 = strnicmp( pIda->pCnt->szOutputFile + 2, pIda->szSysPath + 2,
                          strlen(pIda->szSysPath) - 2 );
        lRc2 = strnicmp( pIda->pCnt->szOutputFile + 3, pIda->szSysPath + 3,
                          strlen(pIda->szSysPath) - 2 );
        //--- if target path contains as first directory the system path
        if ( lRc1 == 0 || lRc2 == 0 )
        {
          //--- display error message that this is not allowed
          strcpy( pszReplace, pIda->szSysPath );
          UtlError( ERROR_EQF_PATH_INVALID, MB_CANCEL,
                    1, &pszReplace, EQF_ERROR );
          //--- stop further processing
          fOk = FALSE;
        } /* endif */

        if ( fOk )
        {
          //--- check filename
          usRc = UtlOpen( pIda->pCnt->szOutputFile,      //filename
                          &hfFileHandle,                 //file handle
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
              //--- dispaly error message that filename is no valid
              pszReplace = pIda->pCnt->szOutputFile;
              UtlError( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszReplace,
                        EQF_WARNING );

              //--- set cursor to entryfiled
              WinSetFocus( HWND_DESKTOP, WinWindowFromID( hwnd, DID_CNT_FILE_EF));

              //--- stop further processing
              fOk = FALSE;
              break;
              //-----------------------------------------------------------
            case ERROR_NETWORK_ACCESS_DENIED:
            case ( ERROR_PATH_NOT_FOUND ) :   //--- path does not exist
              //--- display error message path not exist
              UtlError( ERROR_PATH_NOT_EXIST, MB_CANCEL, 0, (PSZ *) NULP, EQF_WARNING);

              //--- set cursor to entryfiled
              WinSetFocus( HWND_DESKTOP, WinWindowFromID( hwnd, DID_CNT_FILE_EF));

              //--- stop further processing
              fOk = FALSE;
              break;
              //-----------------------------------------------------------
            case ( ERROR_NOT_READY ) :   //--- disk not ready
              //--- display error message disk not ready
              pszReplace = pIda->pCnt->szDriveLetter;
              usResponse = UtlError( ERROR_NOT_READY_MSG, MB_RETRYCANCEL, 1,
                                     &pszReplace, EQF_ERROR );
              if ( usResponse == MBID_RETRY )
              {
                WinPostMsg( hwnd, WM_COMMAND, MP1FROMSHORT(PID_PB_COUNT ), NULL);
              }/*endif*/

              //--- stop further processing
              fOk = FALSE;
              break;
              //-----------------------------------------------------------
            case ( NO_ERROR ) :   //--- filename ok
              //--- save filename to folder properties
              strcpy( pIda->ppropFolder->szOutputFile, pIda->pCnt->szOutputFile );

              //--- close file, do not handle  error
              UtlClose( hfFileHandle, FALSE );

              //--- delete file, do not handle  error
              UtlDelete( pIda->pCnt->szOutputFile, 0L, FALSE );
              break;
              //-----------------------------------------------------------
            case ( ERROR_FILE_EXISTS ) :   //--- file exists
            case ( ERROR_OPEN_FAILED ) :   //--- file exists
              //--- display error message that file exists
              pszReplace = pIda->pCnt->szOutputFile;
              usResponse = UtlError( ERROR_FILE_EXISTS_ALREADY,
                                     MB_YESNO | MB_DEFBUTTON2,
                                     1, &pszReplace, EQF_QUERY );
              switch ( usResponse )
              {
                //------------------------------------------------------
                case MBID_YES :   //--- delete existing file
                  //--- delete output file
                  usRc = UtlDelete( pIda->pCnt->szOutputFile, 0L, TRUE );
                  if ( usRc )
                  {
                    fOk = FALSE;
                  }/*endif*/
                  break;
                  //--------------------------------------------------------------------
                case MBID_NO :   //--- do not delete existing file
                  //--- stop further processing
                  fOk = FALSE;
                  break;
              }/*endswitch*/
              break;
            default :
              /**************************************************/
              /* display standard error message for all cases   */
              /* not checked for explicitly                     */
              /**************************************************/
              {
                CHAR chNum[ 10 ];
                pszReplace = chNum;
                itoa( usRc, chNum, 10 );
                usResponse = UtlError( ERROR_GENERAL_DOS_ERROR_MSG,
                                       MB_CANCEL,
                                       1, &pszReplace, EQF_ERROR );

                fOk = FALSE;
              }
              break;
          }/*endswitch*/
        }/*endif*/
      }/*endif*/
    }/*endif fOk */
  }/*endif fOk */

  if ( fOk )
  {
    //--- send WM_CLOSE message to destroy the dialog and start the
    //--- count instance
    //--- if fOk == FALSE the dialog is not removed
    WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(TRUE), NULL );
  }/*endif*/
}/* end HandleCountButton */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     WordCountWM_CONTROL                                      |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                   MRESULT WordCountWM_CONTROL( HWND   hwnd,                |
//|                                                SHORT  sID,                 |
//|                                                SHORT  sNotification )      |
//+----------------------------------------------------------------------------+
//|Description:       handles WM_CONTROL message of WORDCOUNTDLGPROC dialog    |
//|                   procedure                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   hwnd  dialog handle                                      |
//|                   sID   ID of button                                       |
//|                   sNot  Notification message                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       dependend on the message                                 |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|mResult = FALSE                                                             |
//|                                                                            |
//|switch ( control ID )                                                       |
//|   case ( selection in count listbox ) :                                    |
//|      if ( an item is selected ) deselect it                                |
//|      break                                                                 |
//|   default :                                                                |
//|      mResult = UtlDefDialogProc                                            |
//|      break                                                                 |
//|endswitch                                                                   |
//|return ( mResult )                                                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
static
MRESULT WordCountWM_CONTROL( HWND   hwnd,
                             SHORT  sId,
                             SHORT  sNotification )
{
  MRESULT     mResult = FALSE;             //function return value

  switch ( sId )                           //--- control id
  {
    //--------------------------------------------------------------------------
    case DID_CNT_LB :   //-- selection in count LB
      //--- item double clicked, enter pressed or selected
      if ( (sNotification == LN_ENTER) || (sNotification == LN_SELECT) )
      {
        //--- deselect all items in count listbox
        DESELECTITEM( hwnd, DID_CNT_LB, LIT_NONE );
      }/*endif*/
      break;
      //--------------------------------------------------------------------------
    case DID_CNT_FILE_CHK:
      if ( sNotification == BN_CLICKED  )
      {
        ENABLECTRL( hwnd, DID_CNT_FILE_SET_PB, (BOOL) QUERYCHECK( hwnd, DID_CNT_FILE_CHK ));
        ENABLECTRL( hwnd, DID_CNT_FORMAT_CB, (BOOL) QUERYCHECK( hwnd, DID_CNT_FILE_CHK ));
      } /* endif */
      break;
      //--------------------------------------------------------------------------
    case DID_CNT_TRANSLATION_RB:
    case DID_CNT_ORIGINAL_RB:
    case DID_CNT_DUPLICATES_RB:
    case DID_CNT_FUZZYMATCH_RB:
      if ( sNotification == BN_CLICKED  )
      {
        PIDA pIda = ACCESSDLGIDA( hwnd, PIDA );   // dialog instance area ptr
        BOOL fEnable = (BOOL)QUERYCHECK( hwnd, DID_CNT_TMMATCHES_RB );

        ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, fEnable );
        if ( !fEnable )
        {
          SETCHECK_FALSE( hwnd, DID_CNT_TMREPLMATCH_CHK  );
        } /* endif */

        fEnable = (BOOL)QUERYCHECK( hwnd, DID_CNT_DUPLICATES_RB );
        ENABLECTRL( hwnd, DID_CNT_MEMMATCH_CHK, fEnable );
        if ( fEnable ) 
        {
          SETCHECK( hwnd, DID_CNT_MEMMATCH_CHK, pIda->ppropFolder->fInclMemMatch );
        }
        else
        {
          pIda->ppropFolder->fReplMatch  = QUERYCHECK( hwnd, DID_CNT_TMREPLMATCH_CHK );
          SETCHECK_FALSE( hwnd, DID_CNT_MEMMATCH_CHK  );
        } /* endif */
      } /* endif */
      break;

    case DID_CNT_TMMATCHES_RB:
      {
        PIDA pIda = ACCESSDLGIDA( hwnd, PIDA );   // dialog instance area ptr

        if ( QUERYCHECK( hwnd, DID_CNT_TMMATCHES_RB ) )
        {
          ENABLECTRL( hwnd, DID_CNT_MEMMATCH_CHK, FALSE );
          ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, TRUE );
          SETCHECK( hwnd, DID_CNT_TMREPLMATCH_CHK, pIda->ppropFolder->fReplMatch );
          pIda->ppropFolder->fReplMatch  = QUERYCHECK( hwnd, DID_CNT_TMREPLMATCH_CHK );
          SETCHECK_FALSE( hwnd, DID_CNT_MEMMATCH_CHK  );
        }
        else
        {
          ENABLECTRL( hwnd, DID_CNT_TMREPLMATCH_CHK, FALSE );
          SETCHECK_FALSE( hwnd, DID_CNT_TMREPLMATCH_CHK  );
        } /* endif */
      }
      break;

      //--------------------------------------------------------------------------
    case DID_CNT_TMREPLMATCH_CHK:
      if ( sNotification == BN_CLICKED  )
      {
        PIDA pIda = ACCESSDLGIDA( hwnd, PIDA );   // dialog instance area ptr
        pIda->ppropFolder->fReplMatch = QUERYCHECK( hwnd, DID_CNT_TMREPLMATCH_CHK );
      } /* endif */
      break;
      //--------------------------------------------------------------------------
    case DID_CNT_MEMMATCH_CHK:
      if ( sNotification == BN_CLICKED  )
      {
        PIDA pIda = ACCESSDLGIDA( hwnd, PIDA );   // dialog instance area ptr
        pIda->ppropFolder->fInclMemMatch = QUERYCHECK( hwnd, DID_CNT_MEMMATCH_CHK );
      } /* endif */
      break;
      //--------------------------------------------------------------------------

    default :
      break;
  }/*end switch*/
  return( mResult );
}/* end WordCountWM_CONTROL */


void CntFormatFillCB( HWND hwnd, int id )
{
  SHORT sItem = 0;

  CBDELETEALL( hwnd, id );
    
  sItem = CBINSERTITEMEND( hwnd, id, "ASCII" );
  CBSETITEMHANDLE( hwnd, id, sItem, ASCII_FORMAT ); 

  sItem = CBINSERTITEMEND( hwnd, id, "HTML" );
  CBSETITEMHANDLE( hwnd, id, sItem, HTML_FORMAT ); 

  sItem = CBINSERTITEMEND( hwnd, id, "XML" );
  CBSETITEMHANDLE( hwnd, id, sItem, XML_FORMAT ); 
}

// get selected format
USHORT CntFormatGetSelected( HWND hwnd, int id )
{
  USHORT usFormat = 0;
  SHORT sItem = 0;

  sItem = CBQUERYSELECTION( hwnd, id ); 
  if ( sItem >= 0 )
  {
    usFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, id, sItem );
  } /* endif */
  return( usFormat );
}

// select given format
void CntFormatSelect( HWND hwnd, int id, USHORT usFormat )
{
  int iNumOfItems = CBQUERYITEMCOUNT( hwnd, id );
  int iSelItem = 0;
  int iItem = 0;

  for( iItem = 0; iItem < iNumOfItems; iItem++ )
  {
    USHORT usCurFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, id, iItem );
    if ( usCurFormat == usFormat )
    {
      iSelItem = iItem;
    } /* endif */
  } /* endfor */
  CBSELECTITEM( hwnd, id, iSelItem );
}

// adjust the file extension of the report file depending on active format
BOOL CntAdjustReportFileExtension( PSZ pszFileName, USHORT usFormat )
{
  PSZ pszExtension;
  PSZ pszName;
  BOOL fChanged = FALSE;
  
  // find name part of file name
  pszName = strrchr( pszFileName, '\\' );
  if ( pszName == NULL ) pszName =  pszFileName;

  // find any file extension
  pszExtension = strrchr( pszFileName, '.' );

  // correct the extension
  switch ( usFormat)
  {
    case HTML_FORMAT :
      if ( pszExtension )
      { 
        if ( (stricmp( pszExtension, ".XML" ) == 0) ||
            (stricmp( pszExtension, ".TXT" ) == 0) ||
            (stricmp( pszExtension, ".RPT" ) == 0) ||
            (stricmp( pszExtension, ".CNT" ) == 0) )
        {
          *pszExtension = EOS;
          pszExtension = NULL;
          fChanged = TRUE;
        } /* endif */
      } /* endif */

      if ( pszExtension )
      { 
        if ( (stricmp( pszExtension, ".HTML" ) != 0) &&
             (stricmp( pszExtension, ".HTM" ) != 0) )
        {
          strcat( pszName, ".HTM" );
          fChanged = TRUE;
        } /* endif */
      }
      else
      {
        strcat( pszName, ".HTM" );
        fChanged = TRUE;
      } /* endif */
      break;

    case XML_FORMAT :
      if ( pszExtension )
      { 
        if ( (stricmp( pszExtension, ".HTM" ) == 0) ||
            (stricmp( pszExtension, ".HTML" ) == 0) ||
            (stricmp( pszExtension, ".TXT" ) == 0) ||
            (stricmp( pszExtension, ".RPT" ) == 0) ||
            (stricmp( pszExtension, ".CNT" ) == 0) )
        {
          *pszExtension = EOS;
          pszExtension = NULL;
          fChanged = TRUE;
        } /* endif */
      } /* endif */

      if ( pszExtension )
      { 
        if ( stricmp( pszExtension, ".XML" ) != 0 )
        {
          strcat( pszName, ".XML" );
          fChanged = TRUE;
        } /* endif */
      }
      else
      {
        strcat( pszName, ".XML" );
        fChanged = TRUE;
      } /* endif */
      break;

    case ASCII_FORMAT :
    default:
      if ( pszExtension )
      { 
        if ( (stricmp( pszExtension, ".HTM" ) == 0) ||
            (stricmp( pszExtension, ".HTML" ) == 0) ||
            (stricmp( pszExtension, ".XML" ) == 0) )
        {
          *pszExtension = EOS;
          pszExtension = NULL;
          fChanged = TRUE;
        } /* endif */
      } /* endif */

      if ( !pszExtension )
      { 
        strcat( pszName, ".TXT" );
        fChanged = TRUE;
      } /* endif */
      break;
  } /*endswitch */

  return ( fChanged );
}

/**
* \brief Count the number of words in a given string (API mode)
*
*	\param pszMarkup name of the markup table to be used for the recognition of in-line tags,
*  if this parameter is NULL no in-line tag recognition will be performed
*	\param pszLanguage OpenTM2 name for the language of the given text
*	\param pszText null-terminated string containing the text to be counted, the encoding is UTF-16
* \param pulWords points to an unsigned long value receiving the number of words in the text
* \param pulInlineTags points to an unsigned long value receiving the number of inline tags in the text
*	\returns 0 if successful or an error code
*	
**/
unsigned short CntFuncCountWordsInstring
(
  char        *pszMarkup,              // name of the markup table to be used for the recognition of in-line tags
  char        *pszLanguage,            // OpenTM2 name for the language of the given text
  wchar_t     *pszText,                // null-terminated string containing the text to be counted, the encoding is UTF-16
  unsigned long *pulWords,             // points to an unsigned long value receiving the number of words in the text
  unsigned long *pulInlineTags         // points to an unsigned long value receiving the number of inline tags in the text
)
{
  unsigned short usRC = 0;
  PSTARTSTOP   pStartStop = NULL;
  ULONG ulOemCP = 0;
  SHORT sLangID = -1;
  unsigned long ulWords = 0;
  unsigned long ulTags = 0;

  if ( pulWords != NULL ) *pulWords = 0;
  if ( pulInlineTags != NULL ) *pulInlineTags = 0;

  // get language ID and ASCII CP for language
  usRC = MorphGetLanguageID( pszLanguage, &(sLangID) );
  ulOemCP = GetLangOEMCP( pszLanguage );

  // perform inline tag recognition when requested
  if ( usRC == 0 )
  {
    if ( pszMarkup != NULL )
    {
      // load tag table for the markup
      PLOADEDTABLE  pLoadedTable = NULL;
      usRC = TALoadTagTableExHwnd( pszMarkup, &pLoadedTable, FALSE, TALOADUSEREXIT | TALOADPROTTABLEFUNC, TRUE, HWND_FUNCIF );

      // detect tags in input data
      if ( usRC == 0 )
      {
        int iIterations = 0;
        int          iAddEntries = 0;
        int          iIncrement = max( (wcslen(pszText) / 4), 20 );
        PTOKENENTRY  pTokenList = NULL;

        usRC = EQFRS_AREA_TOO_SMALL;  // force at least one interation inside the loop
        while ((iIterations < 10) && (usRC == EQFRS_AREA_TOO_SMALL))
        {
          // (re)allocate token buffer
          LONG lOldSize = iAddEntries * sizeof(TOKENENTRY);
          LONG lNewSize = (iAddEntries + iIncrement) * sizeof(TOKENENTRY);

          if (UtlAlloc((PVOID *) &pTokenList, lOldSize, lNewSize, NOMSG) )
          {
            iAddEntries += iIncrement;
            iIterations++;
          }
          else
          {
            iIterations = 10;    // force end of loop
          } /* endif */

          // retry tokenization
          if (iIterations < 10 )
          {
            usRC = TACreateProtectTableW( pszText, pLoadedTable, 1, (PTOKENENTRY)pTokenList, (USHORT)lNewSize, &pStartStop,  
                                          pLoadedTable->pfnProtTable, pLoadedTable->pfnProtTableW, ulOemCP );
          } /* endif */
        } /* endwhile */
    
        if ( pLoadedTable) TAFreeTagTable( pLoadedTable );
      } /* endif */
    }
    else
    {
      // generate a start-stop entry for the text string
      UtlAlloc( (PVOID *)&pStartStop, 0, sizeof(STARTSTOP)*2, NOMSG );
      if ( pStartStop != NULL )
      {
        // entry for the text string
        pStartStop[0].usType = UNPROTECTED_CHAR;
        pStartStop[0].usStart = 0;
        pStartStop[0].usStop = (USHORT)(wcslen(pszText) - 1);

        // end-of-list entry 
        pStartStop[1].usType = 0;
        pStartStop[1].usStart = 0;
        pStartStop[1].usStop = 0;
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } 
    }
  }

  // recognize words in the translatable parts of the start-stop list
  if ( usRC == 0 )
  {
    PSTARTSTOP pstCurrent = (PSTARTSTOP) pStartStop;

    while ( (pstCurrent->usType != 0) && (usRC == 0) )
    {
      if ( pstCurrent->usType == UNPROTECTED_CHAR )
      {
        PFLAGOFFSLIST pTermList = NULL;      // pt to created term list
        PFLAGOFFSLIST pActTerm;              // actual term
        CHAR_W chTemp = pszText[ pstCurrent->usStop+1 ];
        PSZ_W pszStart = pszText + pstCurrent->usStart;
        USHORT usListSize = 0;

        pszText[ pstCurrent->usStop+1 ] = EOS;
        usRC = MorphTokenizeW( sLangID, pszStart, &usListSize, (PVOID *)&pTermList, MORPH_FLAG_OFFSLIST, ulOemCP );
        pszText[ pstCurrent->usStop+1 ] = chTemp;

        if ( pTermList )
        {
          pActTerm = pTermList;
          while ( pActTerm->usLen )
          {
            if ( !(pActTerm->lFlags & OtmMorph::TERMTYPE_NEWSENTENCE ) && !(pActTerm->lFlags & OtmMorph::TERMTYPE_NOCOUNT) )
            {
              ulWords++;
            } /* endif */
            pActTerm++;
          } /* endwhile */
        } /* endif */
        UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
      }
      else
      {
        // count tags and attributes as one word
        ulTags++;
      } /* endif */
      pstCurrent++;
    } /*  endwhile */

    if ( pulWords != NULL ) *pulWords = ulWords;
    if ( pulInlineTags != NULL ) *pulInlineTags = ulTags;
  }

  // cleanup
  if ( pStartStop != NULL )   UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
  if ( sLangID != -1 ) MorphFreeLanguageID( sLangID );

  return( usRC );
}