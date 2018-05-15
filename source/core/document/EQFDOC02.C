/*! \file
	Description: Document related functions and dialogs
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TAGTABLE         // tagtable defines and functions
  #define INCL_EQF_FOLDER           // folder list and document list functions
  #define INCL_EQF_DLGUTILS         // dialog utilities
  #define INCL_EQF_TP               // public translation processor functions
  #include <eqf.h>                  // General Translation Manager include file

  #include "eqfutpck.h"             // packaging utilities
  #include "eqfdoc01.h"             // internal header file
  #include "eqfdoc01.id"            // internal ID file
  #include <time.h>
  #include "EQFHLOG.H"            // defines for history log processing
  #include "eqfdde.h"             // ... required for EQFFOL00.H
  #include "EQFFOL00.H"           // defines for folder history log functions

  #include "EQFSETUP.H"           // defines for property directory

  #include "eqfstart.h"           // help processing

  #include "SHLOBJ.H"           // folder browse function

#undef _WPTMIF                         // we don't care about WP I/F
#include "eqfhelp.id"                  // help resource IDs
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                          // message help table

extern HELPSUBTABLE hlpsubtblDocPropDlg[];
extern HELPSUBTABLE hlpsubtblNewDocDlg[];

// buffer for OEM->Ansi conversion for display operations
static CHAR szConvBuffer[1024];


static VOID EnableImportControls (HWND hwnd, BOOL fEnable);
static MRESULT DocImpCommand( HWND, SHORT, SHORT );
static MRESULT DocImpInit( HWND, PDOCIMPIDA );
static MRESULT DocImpChar( HWND, WPARAM, LPARAM );

static VOID PrepExtFormDlg( PDOCIMPIDA pIda, HWND hwnd, SHORT sId );
static VOID PrepIntFormDlg( PDOCIMPIDA pIda, HWND hwnd, SHORT sId );
static VOID PrepImpPathFormDlg( PDOCIMPIDA pIda, HWND hwnd, SHORT sId );

extern HELPSUBTABLE hlpsubtblPropSettingsDlg[];

// name of last used values file
#define DOCIMPSTARTPATH "DOCIMPSTARTPATH"

BOOL EQFDoImportDoc ( PDOCIMPIDA, PEXTRAPAGES );
MRESULT UtlDMGETDEFID1( HWND, WPARAM, LPARAM, PCONTROLSIDA, USHORT );

//f1////////////////////////////////////////////////////////////////////////////
// dialog procedure long-to-short document name                               //
////////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK LONGTOSHORTDLGPROC( HWND   hwnd,
                            WINMSG msg,
                            WPARAM mp1,
                            LPARAM mp2 )

{
  PDOCEXPIDA  pIda;                      //instance area for EQFFEXPO
  MRESULT     mResult = FALSE;           //function return value
  BOOL        fOK;

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_DOCSHORT_DLG, mp2 ); break;

      //------------------------------------------------------------------------
    case ( WM_INITDLG ) :
      pIda = (PDOCEXPIDA) mp2;
      ANCHORDLGIDA( hwnd, pIda );

      // set long name static field
      SETTEXT( hwnd, ID_DOCSHORT_NAME_TEXT, pIda->szLongName );

      // build default short name
      {
        CHAR szDefaultName[MAX_FILESPEC];     // buffer for default name
        int i;                                // counter
        PSZ  pszSource, pszTarget;            // ptr for character copying
        PSZ  pszExt;

        pszExt = strrchr( pIda->szLongName, DOT );
        if ( pszExt != NULL )
        {
          *pszExt = EOS;
        } /* endif */
        pszSource = pIda->szLongName;
        pszTarget = szDefaultName;
        i = 0;
        while ( (i < 8) && (*pszSource != EOS))
        {
          if ( isalnum(*pszSource) || (*pszSource == '@') ||
               (*pszSource == '$') || (*pszSource == '_') ||
               (*pszSource == '-') )
          {
            CHAR c = *pszSource++;
            *pszTarget++ = (CHAR)toupper(c);
            i++;
          }
          else
          {
            pszSource++;            // try next character
          } /* endif */
        } /* endwhile */

        if ( pszExt != NULL )
        {
          *pszExt = DOT;
          pszSource = pszExt + 1;
          *pszTarget++ = DOT;

          i = 0;
          while ( (i < 3) && (*pszSource != EOS))
          {
            if ( isalnum(*pszSource) || (*pszSource == '@') ||
                 (*pszSource == '$') || (*pszSource == '_') ||
                 (*pszSource == '-') )
            {
              CHAR c = *pszSource++;
              *pszTarget++ = (CHAR)toupper(c);
              i++;
            }
            else
            {
              pszSource++;            // try next character
            } /* endif */
          } /* endwhile */
        } /* endif */
        *pszTarget = EOS;
        SETTEXT( hwnd, ID_DOCSHORT_SHORTNAME_EF, szDefaultName );
      }

      mResult = DIALOGINITRETURN(FALSE);
      break;
      //------------------------------------------------------------------------

      //------------------------------------------------------------------------
    case ( WM_COMMAND ) :
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      mResult = MRFROMSHORT( TRUE);

      switch (WMCOMMANDID( mp1, mp2 ))
      {
        case ID_DOCSHORT_HELP_PB:
          UtlInvokeHelp();
          break;

        case ID_DOCSHORT_SET_PB:
          pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

          // get short file name
          QUERYTEXT( hwnd, ID_DOCSHORT_SHORTNAME_EF, pIda->szBuffer );

          // strip blanks from name
          UtlStripBlanks( pIda->szBuffer );

          // check file name
          {
            PSZ  pszSource;         // ptr for character checking
            PSZ  pszExt;
            BOOL fValid = TRUE;

            pszExt = strrchr( pIda->szBuffer, DOT );
            if ( pszExt != NULL )
            {
              *pszExt = EOS;
              fValid = (strlen(pszExt+1) <= 3);
            } /* endif */

            if ( fValid )
            {
              fValid = (strlen(pIda->szBuffer) <= 8);
            } /* endif */

            pszSource = pIda->szBuffer;
            while ( fValid && (*pszSource != EOS) )
            {
              if ( isalnum(*pszSource) || (*pszSource == '@') ||
                   (*pszSource == '$') || (*pszSource == '_') ||
                   (*pszSource == '-') )
              {
                pszSource++;
              }
              else
              {
                fValid = FALSE;         // name contains invalid characters
              } /* endif */
            } /* endwhile */

            if ( fValid && (pszExt != NULL) )
            {
              *pszExt = DOT;
              pszSource = pszExt + 1;
              while ( fValid && (*pszSource != EOS) )
              {
                if ( isalnum(*pszSource) || (*pszSource == '@') ||
                     (*pszSource == '$') || (*pszSource == '_') ||
                     (*pszSource == '-') )
                {
                  pszSource++;
                }
                else
                {
                  fValid = FALSE;         // name contains invalid characters
                } /* endif */
              } /* endwhile */
            } /* endif */
            fOK = fValid;
          }

          // leave dialog if O.K.
          if ( fOK )
          {
            UtlUpper( pIda->szBuffer );
            strcpy( pIda->szExpName, pIda->szBuffer );
            WinDismissDlg( hwnd, TRUE );
          }
          else
          {
            UtlError( INVALID_OS2_FILENAME, MB_CANCEL, 0, NULL, EQF_ERROR);
            SETFOCUS( hwnd, ID_DOCSHORT_SHORTNAME_EF );
          } /* endif */
          break;

        case DID_CANCEL:
        case ID_DOCSHORT_CANCEL_PB:
          WinDismissDlg( hwnd, FALSE );
          break;
      } /* endswitch */
      break;
      //------------------------------------------------------------------------
    case ( WM_DESTROY ) :
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      if ( pIda != NULL )
      {
      } /* endif */
      break;
      //------------------------------------------------------------------------
    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /*endswitch */

  return( mResult );
}/*end LONGTOSHORTDLGPROC*/

//f1////////////////////////////////////////////////////////////////////////////
// function CheckExistence                                                    //
////////////////////////////////////////////////////////////////////////////////
BOOL CheckExistence( PSZ     pszName,       //full qualifiled file name
                     BOOL    fMsg,          //display error message
                     USHORT  usAttr,        //file attribute(s)
                     PUSHORT pusDosRc       //set by UtlFindFirst
                   )
//------------------------------------------------------------------------------
//check existence of passed full qualified filename and display error message --
//in dependency of passed flag                                                --
//------------------------------------------------------------------------------
{
  //declare variables
  LONGFILEFIND *pstResultBuf;              //DosFindFirst result buffer
  HDIR        hDirHandle;                  //DOS dir handle
  USHORT      usCount;                     //UtlFindFirst parameter
  BOOL        fOK;                         //error flag
  USHORT      usMBCode = 0;                //return code from msg box /*KIT1154A*/

  //allocate storage for result buffer
  fOK = UtlAlloc( (PVOID *)&pstResultBuf, 0L, (ULONG)sizeof( LONGFILEFIND ), ERROR_STORAGE );
  if ( fOK )
  {
    do
    {
      //set DosFindFirst parameters
      hDirHandle = HDIR_CREATE;
      usCount    = 1;

      //check if file exists
      *pusDosRc = UtlFindFirstLong
                  ( pszName,                         //full specified file to find
                    &hDirHandle,                     //directory handle
                    usAttr,                          //search attribute
                    pstResultBuf,                    //return buffer
                    fMsg
                  );
      if ( *pusDosRc == NO_ERROR )
      {
        UtlFindCloseLong( hDirHandle, FALSE );
      } /* endif */

      if ( *pusDosRc == ERROR_DISK_CHANGE )
      {
        /* ask for permission to search on new drive */
        usMBCode = UtlError (*pusDosRc, 0, 1, &pszName, DOS_ERROR);
        if ( (usMBCode == MBID_RETRY) && (*pusDosRc == ERROR_DISK_CHANGE) )
        {
          UtlSetDrive( *pszName );
        } /* endif */
      }
      else
      {
        if ( *pusDosRc != NO_ERROR )            //file not exits
        {
          fOK = FALSE;                       //set fOK to FALSE
        }/*end if*/
      } /* endif */
    } while ( (*pusDosRc == ERROR_DISK_CHANGE) &&
              (usMBCode == MBID_RETRY) ); /* enddo */
  }/*end if fOK */

  //free storage of pstResultBuf
  UtlAlloc( (PVOID *)&pstResultBuf, 0L, 0L, NOMSG );

  if ( *pusDosRc == ERROR_DISK_CHANGE )
  {
    /* set flag that file wasn't found to avoid file existence warning */
    fOK = FALSE;
  } /* endif */

  return fOK;
}/*end CheckExistence*/

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocPropertyDlg                                           |
//+----------------------------------------------------------------------------+
//|Description:       Pops-up document property dialog or new document dialog  |
//|                   depending on fPropDlg flag                               |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndParent  handle of dialog parent window          |
//|                   PSZ  pszDocName  object name of document                 |
//|                   BOOL fPropDlg    TRUE  = popup document property dlg     |
//|                                    FALSE = popup document new dlg          |
//|                   PDOCINFO pDocInfo ptr to document info structure         |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   dialog ended normally                             |
//|                   FALSE  dialog has been cancelled                         |
//+----------------------------------------------------------------------------+
//|Side effects:      Document properties my be changed                        |
//+----------------------------------------------------------------------------+
BOOL DocPropertyDlg
(
  HWND             hwndParent,         // handle of dialog parent window
  PSZ              pszDocName,         // object name of document
  BOOL             fPropDlg,           // TRUE  = popup document property dlg
                                       // FALSE = popup document new dlg
  PDOCUMENTINFO    pDocInfo,           // ptr to document info structure
  PSZ              pszObjNameList      // ptr to document object name list when more
                                       // than one document is selected or NULL
)
{
  BOOL             fOK;                // function return code
  PDOCPROPIDA      pIda = NULL;        // pointer to document property dlg IDA
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /********************************************************************/
  /* Allocate document property dialog IDA                            */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(DOCPROPIDA), ERROR_STORAGE );

  /********************************************************************/
  /* Fill-in initial values                                           */
  /********************************************************************/
  if ( fOK )
  {
    pIda->fPropDlg = fPropDlg;
    strcpy( pIda->szDocObjName, pszDocName );
    pIda->pDocInfo = pDocInfo;
    pIda->pszObjNameList = pszObjNameList;
  } /* endif */

  /********************************************************************/
  /* Popup document property dialog                                   */
  /********************************************************************/
  if ( pIda->fPropDlg )  // Document Properties   //gs new document
  {
    DIALOGBOX( hwndParent, DocPropSheetsDlgProc, hResMod, ID_DOC_PROP_DLG, pIda, fOK ); //dialog with property sheets for WINDOWS
  }
  else // Document new dialog
  {
    DIALOGBOX( hwndParent, DocPropSheetsDlgProc, hResMod, ID_DOCNEW_PROP_DLG, pIda, fOK );
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function DocPropertyDlg */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocPropSheetsDlgProc                                     |
//+----------------------------------------------------------------------------+
//|Description:       Dialog procedure for document properties dialog          |
//|                   with property sheets                                     |
//+----------------------------------------------------------------------------+
//|Parameters:        standard window procedure parameters                     |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK DocPropSheetsDlgProc
(
HWND             hwnd,               // dialog window handle
WINMSG           msg,                // message
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  MRESULT          mResult = FALSE;    // function return value
  BOOL             fOK = TRUE;         // internal OK flag
  PDOCPROPIDA      pIda;               // ptr to document property dlg IDA
  EQFINFO          ErrorInfo;          // error info returned by prop functions
  PSZ              pszTemp;            // working pointer
  PPROPDOCUMENT    pProp;              // pointer to document properties
  INT              nItem;


  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwnd, PDOCPROPIDA );
      if ( pIda->fPropDlg )
      {
        HANDLEQUERYID( ID_DOC_PROP_DLG, mp2 );
      }
      else
      {
        HANDLEQUERYID( ID_DOCNEW_PROP_DLG, mp2 );
      } /* endif */
      break;


    case WM_INITDLG :
      fOK = TRUE;

      /**************************************************************/
      /* Get and anchor document property dialog IDA pointer        */
      /**************************************************************/
      pIda = (PDOCPROPIDA)mp2;
      ANCHORDLGIDA( hwnd, pIda );

      /**************************************************************/
      /* Set ID for 'new document' or 'document properties'         */
      /**************************************************************/

      if ( pIda->fPropDlg )
      {
        SETWINDOWID( hwnd, ID_DOC_PROP_DLG );
      }
      else
      {
        SETWINDOWID( hwnd, ID_DOCNEW_PROP_DLG );
      } /* endif */

      // handling for properties of more than one document
      if ( pIda->pszObjNameList )
      {
        // load document properties and check if the documents use different
        // settings
        BOOL fFirstDocument = TRUE;
        PSZ pszCurrent = pIda->pszObjNameList;
        PSZ pszCurName = pIda->szAllNameBuffer;
        int iNameBufferLeft = sizeof(pIda->szAllNameBuffer)-1;
        while ( fOK && *pszCurrent )
        {
          HPROP hDocProp = OpenProperties( pszCurrent, NULL, PROP_ACCESS_READ, &ErrorInfo );
          if ( hDocProp == NULL )
          {
            UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszCurrent, EQF_ERROR );
            fOK = FALSE;
          }
          else
          {
            pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hDocProp );

            // for first document copy its setting to IDA, for all others compare
            // settings with the settings in the IDA
            if ( fFirstDocument )
            {
              strcpy( pIda->szDocEditor, pProp->szEditor );
              strcpy( pIda->szDocFormat, pProp->szFormat );
              if ( pProp->szLongMemory[0] != EOS )
              {
                strcpy( pIda->szDocMemory, pProp->szLongMemory );
              }
              else
              {
                strcpy( pIda->szDocMemory, pProp->szMemory );
              } /* endif */
              strcpy( pIda->szDocConversion, pProp->szConversion );
              strcpy( pIda->szDocVendorName, pProp->szVendor );
              strcpy( pIda->szDocVendorEMail, pProp->szVendorEMail );
              strcpy( pIda->szDocSourceLang, pProp->szSourceLang );
              strcpy( pIda->szDocTargetLang, pProp->szTargetLang );
              strcpy( pIda->szDocShipment, pProp->szShipment );

              // set mode flags
              pIda->modeEditor      = SAMEVALUE;
              pIda->modeFormat      = SAMEVALUE;
              pIda->modeMemory      = SAMEVALUE;
              pIda->modeConversion  = SAMEVALUE;
              pIda->modeVendorName  = SAMEVALUE;
              pIda->modeVendorEMail = SAMEVALUE;
              pIda->modeSourceLang  = SAMEVALUE;
              pIda->modeTargetLang  = SAMEVALUE;
              pIda->modeShipment    = SAMEVALUE;

              // remember parent of documents
              if ( pProp->ulParentFolder == 0 )
              {
                // parent is the main folder
                strcpy( pIda->szParentObject, pProp->PropHead.szPath );
                pIda->fParentIsSubfolder = FALSE;
              }
              else
              {
                // parent is a subfolder
                SubFolIdToObjectName( pProp->PropHead.szPath, pProp->ulParentFolder, pIda->szParentObject );
                pIda->fParentIsSubfolder = TRUE;
              } /* endif */

              fFirstDocument = FALSE;
            }
            else
            {
              PSZ pszNewTM;

              if ( strcmp( pIda->szDocEditor, pProp->szEditor ) != 0 )
                pIda->modeEditor = DIFFERENTVALUE;
              if ( strcmp( pIda->szDocFormat, pProp->szFormat ) != 0 )
                pIda->modeFormat      = DIFFERENTVALUE;
              pszNewTM = pProp->szLongMemory;
              if ( !*pszNewTM ) pszNewTM = pProp->szMemory;
              if ( strcmp( pIda->szDocMemory, pszNewTM ) != 0 )
                pIda->modeMemory      = DIFFERENTVALUE;
              if ( strcmp( pIda->szDocConversion, pProp->szConversion ) != 0 )
                pIda->modeConversion  = DIFFERENTVALUE;
              if ( strcmp( pIda->szDocVendorName, pProp->szVendor ) != 0 )
                pIda->modeVendorName  = DIFFERENTVALUE;
              if ( strcmp( pIda->szDocVendorEMail, pProp->szVendorEMail ) != 0 )
                pIda->modeVendorEMail = DIFFERENTVALUE;
              if ( strcmp( pIda->szDocSourceLang, pProp->szSourceLang ) != 0 )
                pIda->modeSourceLang  = DIFFERENTVALUE;
              if ( strcmp( pIda->szDocTargetLang, pProp->szTargetLang ) != 0 )
                pIda->modeTargetLang  = DIFFERENTVALUE;
              if ( strcmp( pIda->szDocShipment, pProp->szShipment ) != 0 )
                pIda->modeShipment    = DIFFERENTVALUE;
            } /* endif */

            // add document long name to concatenated name buffer
            {
              int iLen;
              PSZ pszName = pProp->szLongName;
              if ( *pszName == EOS ) pszName = pProp->PropHead.szName;
              iLen = strlen(pszName);
              if ( (iLen + 3) < iNameBufferLeft )
              {
                // add delimiter if not the first name in buffer
                if ( pszCurName != pIda->szAllNameBuffer )
                {
                  *pszCurName++ = ',';
                  *pszCurName++ = ' ';
                  iNameBufferLeft -= 2;
                } /* endif*/

                // add document name to name buffer
                strcpy( pszCurName, pszName );
                pszCurName += iLen;
                iNameBufferLeft -= iLen;
              } /* endif */
            }

            CloseProperties( hDocProp, 0, &ErrorInfo );
          } /* endif */

          // continue with next document
          pszCurrent = pszCurrent + strlen(pszCurrent) + 1;
        } /* endwhile */
      }
      else
      {
        if ( (pIda->pDocInfo != NULL) && (pIda->pDocInfo->pDocProps != NULL) )
        {
          // use supplied document properties
        }
        else
        {
          // load document properties
          pIda->hDocProp = OpenProperties( pIda->szDocObjName, NULL,
                                          PROP_ACCESS_READ, &ErrorInfo );

          if ( pIda->hDocProp == NULL )
          {
            /************************************************************/
            /* Error accessing properties of document                   */
            /************************************************************/
            pszTemp = pIda->szDocObjName;
            UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
            fOK = FALSE;
          }/* endif */
        } /* endif */
      }
      mResult = DocPropPropertySheetLoad( hwnd, pIda );
      break;

    case WM_NOTIFY:

      mResult = DocPropPropertySheetNotification( hwnd, mp1, mp2 );
      break;

    case WM_EQF_CLOSE :


      DISMISSDLG( hwnd, mp1 );
      break;

    case WM_DESTROY :
      /**************************************************************/
      /* Close any open properties                                  */
      /**************************************************************/
      pIda = ACCESSDLGIDA( hwnd, PDOCPROPIDA );
      if ( pIda->hDocProp )
      {
        CloseProperties( pIda->hDocProp, 0, &ErrorInfo );
      } /* endif */
      break;

    case WM_COMMAND :
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_DOCPROP_HELP_PB:
          UtlInvokeHelp();
          break;

        case ID_DOCPROP_CANCEL_PB :  // CANCEL button selected
        case DID_CANCEL :            // ESC key pressed
          /******************************************************/
          /* Leave dialog and discard changes                   */
          /******************************************************/

          WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
          break;

        case ID_DOCPROP_CHANGE_PB :
          /**********************************************************/
          /* Get access to IDA                                      */
          /**********************************************************/
          pIda = ACCESSDLGIDA( hwnd, PDOCPROPIDA );
          fOK = TRUE;


          // issue command to all active dialog pages
          nItem = 0;
          while ( pIda->hwndPages[nItem] && fOK )
          {
            PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                                 DWL_DLGPROC );

            switch ( nItem )
            {
              // settings
              case 0:
                fOK =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                              ID_DOCPROP_CHANGE_PB, 0L);
                break;

                // admin
              case 1:
                fOK =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                              ID_DOCPROP_CHANGE_PB, 0L);
                break;

                // statistics
              case 2:
                pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                       ID_DOCPROP_CHANGE_PB, 0L);
                break;

            } /* endswitch */
            nItem++;
          } /* endwhile */


          /************************************************************/
          /* Check source and target language                         */
          /************************************************************/
          if ( fOK )
          {
            strcpy( pIda->szBuffer, pIda->szDocSourceLang );
            UtlStripBlanks( pIda->szBuffer );
            if ( pIda->szBuffer[0] == EOS )
            {
              strcpy( pIda->szBuffer, pIda->szSourceLangFolBuffer );
            } /* endif */

            strcpy( pIda->szBuffer2, pIda->szDocTargetLang );
            UtlStripBlanks( pIda->szBuffer2 );
            if ( pIda->szBuffer2[0] == EOS )
            {
              strcpy( pIda->szBuffer2, pIda->szTargetLangFolBuffer );
            } /* endif */


            if ( strcmp( pIda->szBuffer, pIda->szBuffer2 ) == 0 )
            {
              if ( UtlErrorHwnd( ERROR_MEM_SAME_LANGUAGES,
                                 MB_YESNO | MB_DEFBUTTON2,
                                 0, NULL, EQF_QUERY, hwnd ) == MBID_NO )
              {
                fOK = FALSE;
              } /* endif */
            } /* endif */
    		  } /* endif */

          /************************************************************/
          /* Check memory, call memory handler to create new memories */
          /************************************************************/
          if ( fOK )
          {
            strcpy( pIda->szBuffer, pIda->szDocMemory );
            ANSITOOEM( pIda->szBuffer );
            strcpy( pIda->szBuffer2, pIda->szTransMemFolBuffer );
            ANSITOOEM( pIda->szBuffer2 );
            UtlStripBlanks( pIda->szBuffer );
            if ( pIda->szBuffer[0] == EOS )
            {
              /********************************************************/
              /* ignore empty document memory                         */
              /********************************************************/
            }
            else if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
            {
              /********************************************************/
              /* Document memory is different from folder memory      */
              /********************************************************/
              BOOL fIsNew = FALSE;
              CHAR szShortName[MAX_FNAME];
              strcpy( pIda->szNewMemory, pIda->szBuffer );
              ObjLongToShortName( pIda->szNewMemory, szShortName,
                                  TM_OBJECT, &fIsNew );
              if ( fIsNew )
              {
                /*****************************************************/
                /* This is the name of a new memory ...              */
                /*****************************************************/
                //--- set 'dialog is disabled' flag
                // Note: using WinEnableWindow to disable the dialog
                //       does not work correctly (probably the dialog
                //       owner/parentship is not set correctly within
                //       the memory handler)
                pIda->fDisabled = TRUE;

                /*****************************************************/
                /* call the memory handler to create a new TM        */
                /*****************************************************/
                strcat( pIda->szBuffer, X15_STR );
                strcpy(pIda->szBuffer, pIda->szDocFormat);
                if ( pIda->szBuffer2[0] == EOS )
                {
                  strcpy(pIda->szBuffer2, pIda->szMarkupFolBuffer);
                } /* endif */
                strcat( pIda->szBuffer, pIda->szBuffer2 );
                strcat( pIda->szBuffer, X15_STR );


                strcpy( pIda->szBuffer2, pIda->szDocSourceLang);
                strcpy( pIda->szBuffer2, pIda->szSourceLangFolBuffer);
                strcat( pIda->szBuffer, pIda->szBuffer2 );
                strcat( pIda->szBuffer, X15_STR );

                strcpy( pIda->szBuffer2, pIda->szDocTargetLang);
                if ( pIda->szBuffer2[0] == EOS )
                {
                  strcpy( pIda->szBuffer2, pIda->szTargetLangFolBuffer);
                } /* endif */
                strcat( pIda->szBuffer, pIda->szBuffer2 );
                strcat( pIda->szBuffer, X15_STR );

                fOK = (BOOL) EqfSend2Handler( MEMORYHANDLER,
                                                  WM_EQF_CREATE,
                                                  MP1FROMSHORT(0),
                                                  MP2FROMP( pIda->szBuffer ) );

                //--- reset 'dialog is disabled' flag, dialog is active
                pIda->fDisabled = FALSE;
                SETFOCUSHWND( hwnd );

                if ( fOK )
                {
                  //--- update list of memory dbs
                  CBDELETEALL( hwnd, ID_DOCPROP_DOCTM_CB );
                  EqfSend2Handler( MEMORYHANDLER,
                                   WM_EQF_INSERTNAMES,
                                   MP1FROMHWND( WinWindowFromID( hwnd,
                                                                 ID_DOCPROP_DOCTM_CB) ),
                                   MP2FROMP( MEMORY_ALL ) );

                  // set name to uppercase
                  OEMTOANSI( pIda->szNewMemory );
                  SETTEXT( hwnd, ID_DOCPROP_DOCTM_CB, pIda->szNewMemory );
                  ANSITOOEM( pIda->szNewMemory );
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */

          /************************************************************/
          /* Check editor                                             */
          /************************************************************/
	        if ( fOK )
	        {
			      strcpy( pIda->szBuffer, pIda->szDocEditor );
			      UtlStripBlanks( pIda->szBuffer );
			      if ( pIda->szBuffer[0] == EOS )
			      {
			        strcpy( pIda->szBuffer, pIda->szEditorFolBuffer );
			      } /* endif */
	        } /* endif */


          // clear fields which have the same value as the parent (sub)folder
          if ( fOK )
          {
              // fix for S613005: leave document format as-is

              //if ( stricmp( pIda->szDocFormat, pIda->szMarkupFolBuffer ) == 0 )
              //{
              //  pIda->szDocFormat[0] = EOS;
              //} /* endif */
              if ( pIda->szDocFormat[0] == EOS )
              {
                strcpy( pIda->szDocFormat, pIda->szMarkupFolBuffer );
              } /* endif */

              if ( stricmp( pIda->szDocMemory, pIda->szTransMemFolBuffer ) == 0 )
              {
                pIda->szDocMemory[0] = EOS;
              } /* endif */

              if ( stricmp( pIda->szDocSourceLang, pIda->szSourceLangFolBuffer ) == 0 )
              {
                pIda->szDocSourceLang[0] = EOS;
              } /* endif */

              if ( stricmp( pIda->szDocTargetLang, pIda->szTargetLangFolBuffer ) == 0 )
              {
                pIda->szDocTargetLang[0] = EOS;
              } /* endif */

              if ( stricmp( pIda->szDocEditor, pIda->szEditorFolBuffer ) == 0 )
              {
                pIda->szDocEditor[0] = EOS;
              } /* endif */

              if ( stricmp( pIda->szDocConversion, pIda->szConvFolBuffer ) == 0 )
              {
                pIda->szDocConversion[0] = EOS;
              } /* endif */
          } /* endif */

          if ( fOK )
          {
            if ( pIda->pszObjNameList )
            {
              // process all documents in pszObjNamelist
              PSZ pszCurrentDoc = pIda->pszObjNameList;
              while ( fOK && (*pszCurrentDoc != EOS)  )
              {
                BOOL fPropsChanged = FALSE;

                // open document properties
                HPROP hDocProp = OpenProperties( pszCurrentDoc, NULL, PROP_ACCESS_READ, &ErrorInfo );
                if ( hDocProp == NULL )
                {
                  UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszCurrentDoc, EQF_ERROR );
                  fOK = FALSE;
                } /* endif */

                // get write access to properties
                if ( fOK )
                {
                  if ( !SetPropAccess( hDocProp, PROP_ACCESS_WRITE) )
                  {
                    fOK = FALSE;
                    UtlErrorHwnd( 0, MB_CANCEL, 1, &pszCurrentDoc, PROP_ERROR, hwnd );
                  } /* endif */
                } /* endif */

                // update document properties
                if ( fOK )
                {
                  pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hDocProp );

                  // update changed fields only

                  if ( pIda->modeEditor == VALUECHANGED )
                  {
                    strcpy( pProp->szEditor, pIda->szDocEditor );
                    fPropsChanged = TRUE;
                  } /* endif */

                  if ( pIda->modeFormat == VALUECHANGED )
                  {
                    strcpy( pProp->szFormat, pIda->szDocFormat );
                    fPropsChanged = TRUE;
                  } /* endif */

                  if ( pIda->modeMemory == VALUECHANGED )
                  {
                    ANSITOOEM( pIda->szDocMemory );
                    if ( pIda->szDocMemory[0] == EOS )
                    {
                      pProp->szMemory[0] = EOS;
                      pProp->szLongMemory[0] = EOS;
                    }
                    else
                    {
                      BOOL fIsNew = FALSE;
                      ObjLongToShortName( pIda->szDocMemory, pProp->szMemory, TM_OBJECT, &fIsNew );
                      strcpy( pProp->szLongMemory, pIda->szDocMemory );
                    } /* endif */
                    OEMTOANSI( pIda->szDocMemory );
                    fPropsChanged = TRUE;
                  } /* endif */

                  if ( pIda->modeConversion == VALUECHANGED )
                  {
                    UtlStripBlanks( pIda->szDocConversion );
                    strcpy( pProp->szConversion, pIda->szDocConversion );
                    fPropsChanged = TRUE;
                  } /* endif */

                  if ( pIda->modeVendorName == VALUECHANGED )
                  {
                    strcpy( pProp->szVendor, pIda->szDocVendorName );
                    fPropsChanged = TRUE;
                  } /* endif */

                  if ( pIda->modeVendorEMail == VALUECHANGED )
                  {
                    strcpy( pProp->szVendorEMail, pIda->szDocVendorEMail );
                    fPropsChanged = TRUE;
                  } /* endif */

                  if ( pIda->modeSourceLang == VALUECHANGED )
                  {
                    strcpy( pProp->szSourceLang, pIda->szDocSourceLang );
                    fPropsChanged = TRUE;
                  } /* endif */

                  if ( pIda->modeTargetLang == VALUECHANGED )
                  {
                    strcpy( pProp->szTargetLang, pIda->szDocTargetLang );
                    fPropsChanged = TRUE;
                  } /* endif */

                  if ( pIda->modeShipment == VALUECHANGED )
                  {
                    strcpy( pProp->szShipment, pIda->szDocShipment );
                    fPropsChanged = TRUE;
                  } /* endif */

                  // save long name for histlog record
                  strcpy( pIda->szBuffer, pProp->szLongName );
                } /* endif */

                // save properties
                if ( fOK && fPropsChanged )
                {
                  if ( SaveProperties( hDocProp, &ErrorInfo) )
                  {
                    UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszCurrentDoc, EQF_ERROR );
                    fOK = FALSE;
                  } /* endif */
                  ResetPropAccess( hDocProp, PROP_ACCESS_WRITE );
                  CloseProperties( hDocProp, PROP_FILE, &ErrorInfo );
                } /* endif */

                // add properties changed record to history log
                if ( fOK && fPropsChanged )
                {
                  DOCPROPHIST DocPropHist;    // history record for document props
                  OBJNAME     szFolder;       // buffer for folder object name
                  PSZ         pszDocName;     // ptr to document name

                  strcpy( DocPropHist.szMarkup,     pIda->szDocFormat );
                  strcpy( DocPropHist.szMemory,     pIda->szDocMemory );
                  strcpy( DocPropHist.szSourceLang, pIda->szDocSourceLang );
                  strcpy( DocPropHist.szTargetLang, pIda->szDocTargetLang );

                  // split document name from document object name
                  strcpy( szFolder, pszCurrentDoc );
                  pszDocName = UtlSplitFnameFromPath( szFolder );

                  EQFBWriteHistLog2( szFolder, pszDocName,
                                    DOCPROP_LOGTASK,
                                    sizeof(DOCPROPHIST), (PVOID)&DocPropHist,
                                    TRUE, NULLHANDLE, pIda->szBuffer );
                } /* endif */

                pszCurrentDoc += strlen(pszCurrentDoc) + 1;
              } /* endwhile */
            }
            else
            {
              // update supplied document property area or document property file
              pProp = NULL;
              if ( pIda->pDocInfo && pIda->pDocInfo->pDocProps )
              {
                // use supplied document properties area
                pProp = pIda->pDocInfo->pDocProps;
              }
              else
              {
                // update document property file
                if ( !SetPropAccess( pIda->hDocProp, PROP_ACCESS_WRITE) )
                {
                  fOK = FALSE;
                  pszTemp = pIda->szDocObjName;
                  UtlErrorHwnd( 0, MB_CANCEL, 1, &pszTemp, PROP_ERROR, hwnd );
                } /* endif */
                if ( fOK ) pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->hDocProp );
              } /* endif */

              if ( fOK )
              {
                if ( !pProp->fMarkupLocked ) strcpy( pProp->szFormat, pIda->szDocFormat );

                ANSITOOEM( pIda->szDocMemory );
                if ( pIda->szDocMemory[0] == EOS )
                {
                  pProp->szMemory[0] = EOS;
                  pProp->szLongMemory[0] = EOS;
                }
                else
                {
                  BOOL fIsNew = FALSE;
                  ObjLongToShortName( pIda->szDocMemory, pProp->szMemory, TM_OBJECT, &fIsNew );
                  strcpy( pProp->szLongMemory, pIda->szDocMemory );
                } /* endif */

                if ( !pProp->fSourceLangLocked ) strcpy( pProp->szSourceLang, pIda->szDocSourceLang );
                if ( !pProp->fTargetLangLocked ) strcpy( pProp->szTargetLang, pIda->szDocTargetLang );
                strcpy( pProp->szEditor, pIda->szDocEditor );
                UtlStripBlanks( pIda->szDocConversion );
                strcpy( pProp->szConversion, pIda->szDocConversion );

                // Update document info structure                         
                if ( fOK )
                {
                  if ( pIda->pDocInfo )
                  {
                    strcpy( pIda->pDocInfo->szMemory, pProp->szMemory );
                    strcpy( pIda->pDocInfo->szLongMemory, pProp->szLongMemory );
                    strcpy( pIda->pDocInfo->szFormat, pProp->szFormat );
                    strcpy( pIda->pDocInfo->szSourceLang, pProp->szSourceLang );
                    strcpy( pIda->pDocInfo->szTargetLang, pProp->szTargetLang );
                    strcpy( pIda->pDocInfo->szEditor, pProp->szEditor );
                    strcpy( pIda->pDocInfo->szTranslator, pProp->szVendor );
                    strcpy( pIda->pDocInfo->szTranslatorEmail, pProp->szVendorEMail );
                    strcpy( pIda->pDocInfo->szConversion, pProp->szConversion );
                    pIda->pDocInfo->fUseForAll = QUERYCHECK( hwnd, ID_DOCPROP_USEFORALL_CHK );
                  } /* endif */

                  // update document alias name
                  strcpy( pProp->szAlias, pIda->szChangedAliasBuffer);

                  ANSITOOEM( pProp->szAlias );

                  /************************************************************/
                  /* Add properties changed record to history log             */
                  /************************************************************/
                  if ( pIda->fPropDlg )
                  {
                    DOCPROPHIST DocPropHist;    // history record for document props
                    OBJNAME     szFolder;       // buffer for folder object name
                    PSZ         pszDocName;     // ptr to document name

                    strcpy( DocPropHist.szMarkup, pProp->szFormat );
                    strcpy( DocPropHist.szMemory, pProp->szMemory );
                    strcpy( DocPropHist.szSourceLang, pProp->szSourceLang );
                    strcpy( DocPropHist.szTargetLang, pProp->szTargetLang );

                    // split document name from document object name
                    strcpy( szFolder, pIda->szDocObjName );
                    pszDocName = UtlSplitFnameFromPath( szFolder );

                    EQFBWriteHistLog2( szFolder, pszDocName,
                                      DOCPROP_LOGTASK,
                                      sizeof(DOCPROPHIST), (PVOID)&DocPropHist,
                                      TRUE, NULLHANDLE, pProp->szLongName );
                  } /* endif */

                  if ( pIda->pDocInfo && pIda->pDocInfo->pDocProps )
                  {
                    // nothing to do, document property file will be written by caller
                  }
                  else
                  {
                    if ( SaveProperties( pIda->hDocProp, &ErrorInfo) )
                    {
                      pszTemp = pIda->szDocObjName;
                      UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
                      fOK = FALSE;
                    } /* endif */

                    ResetPropAccess( pIda->hDocProp, PROP_ACCESS_WRITE );
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */

          /************************************************************/
          /* Close dialog                                             */
          /************************************************************/
          if ( fOK )
          {
            WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(TRUE), NULL );
          } /* endif */
          break;

        case ID_DOCPROP_NAME_TEXT:
        case ID_DOCPROP_ALIAS_EF:
          if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
          {
            ClearIME( hwnd );
          } /* endif */
          break;
      } /* endswitch */
      break;


    case WM_HELP:
      {

        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/
        pIda = ACCESSDLGIDA( hwnd, PDOCPROPIDA );
        if ( pIda->fPropDlg ) //existing document
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblDocPropDlg[0] );
        }
        else   // new document
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblNewDocDlg[0] );
        }

        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP




    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /*endswitch */
  return( mResult );
} /* end of function DocPropDlgProc */




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocPropPropertySheetNotification                         |
//+----------------------------------------------------------------------------+
//|Function call:     DocPropPropertySheetNotifiaction( hwnd, mp1, mp2);       |
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwnd   handle of the dialog                         |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch ( pNMHdr->code )                                  |
//|                     case TCN_SELCHANGE:                                    |
//|                       activate new page                                    |
//|                     case TCN_SELCHANGING                                   |
//|                       hide the dialog                                      |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
MRESULT DocPropPropertySheetNotification
(
HWND   hwnd,
WPARAM  mp1,
LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  ULONG      ulTabCtrl;
  MRESULT      mResult = FALSE;
  PDOCPROPIDA     pIda;
  pNMHdr = (LPNMHDR)mp2;

  mp1;
  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwnd, PDOCPROPIDA);
      if ( pIda )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwnd, ID_DOC_PROP_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = (USHORT)Item.lParam;
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA( hwnd, PDOCPROPIDA );
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        TC_ITEM Item;
        PFNWP pfnWp;
        HWND hwndTabCtrl = GetDlgItem( hwnd, ID_DOC_PROP_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = (USHORT)Item.lParam;
        pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ ulTabCtrl ], DWL_DLGPROC );

        mResult = pfnWp( pIda->hwndPages[ulTabCtrl], WM_COMMAND,
                         PID_PB_OK, 1L);
        if ( mResult )
        {
          /************************************************************/
          /* stick on the side                                        */
          /* we have to post the request again since one of the system*/
          /* routines thinks that we still want to change the page    */
          /************************************************************/
          WinPostMsg( hwnd, TCM_SETCURSEL, ulTabCtrl, 0L );
        } /* endif */
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_HIDE );
      } /* endif */
      break;
    case TTN_NEEDTEXT:
      {
        TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) mp2;
        if ( pToolTipText )
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          TC_ITEM Item;
          HWND hwndTabCtrl = GetDlgItem( hwnd, ID_DOC_PROP_TABCTRL );
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
          switch ( (SHORT)Item.lParam )
          {
            case 0:      // first page: settings
              LOADSTRING( hab, hResMod, SID_DOC_PROP_TAB_SETTINGS,
                          pToolTipText->szText );
              break;

            case 1:      // second page: admin
              LOADSTRING( hab, hResMod, SID_DOC_PROP_TAB_ADMIN,
                          pToolTipText->szText );
              break;

            case 2:      // third page: statistics
              LOADSTRING( hab, hResMod, SID_DOC_PROP_TAB_STATISTICS,
                          pToolTipText->szText );
              break;

          } /* endswitch */
        } /* endif */
      }
      break;
    default:
      break;
  } /* endswitch */
  return mResult;
} /* end of function DocPropPropertySheetNotification */






//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocPropPropertySheetLoad                                 |
//+----------------------------------------------------------------------------+
//|Function call:     DocPropertySheetLoad( hwnd, mp2 );                       |
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwnd   handle of the dialog                         |
//|                   PDOCPROPIDA pIda pointer to ID structure                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     create any pages,                                        |
//|                   load the tabctrl text                                    |
//|                   load the (modeless) dialog, register it and position into|
//|                     tab area                                               |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
BOOL DocPropPropertySheetLoad
(
HWND hwnd,
PDOCPROPIDA     pIda
)
{
  BOOL      fOk = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  CHAR      szBuffer[80];
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( fOk )
  {
    RECT rect;
    // remember adress of user area
    hInst = GETINSTANCE( hwnd );
    hwndTabCtrl = GetDlgItem( hwnd, ID_DOC_PROP_TABCTRL );
    pIda->hwndTabCtrl = hwndTabCtrl;
    GetClientRect( hwndTabCtrl, &rect );
    TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

    // leave some additional space at top
    rect.top += 20;
    MapWindowPoints( hwndTabCtrl, hwnd, (POINT *) &rect, 2 );


    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // -----------------------------------------------------------------
    //
    // create the appropriate TAB control and load the associated dialog
    //
    // -----------------------------------------------------------------

    //
    // SID_DOC_PROP_TAB_SETTINGS
    //

    LOADSTRING( hab, hResMod, SID_DOC_PROP_TAB_SETTINGS , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_DOC_PROP_SETTINGS_DLG ),
                       hwnd,
                       DocPropSettingsDlgProc,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    //
    // SID_DOC_PROP_TAB_ADMIN
    //

    LOADSTRING( hab, hResMod, SID_DOC_PROP_TAB_ADMIN , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_DOC_PROP_ADMIN_DLG ),
                       hwnd,
                       DocPropAdminDlgProc,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    //
    // SID_DOC_PROP_TAB_STATISTICS
    //
    // statistics only for single document properties
    if ( pIda->fPropDlg && (pIda->pszObjNameList == NULL) )
    {

      LOADSTRING( hab, hResMod, SID_DOC_PROP_TAB_STATISTICS , szBuffer );
      TabCtrlItem.pszText = szBuffer;
      TabCtrlItem.lParam = nItem;
      SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
      pIda->hwndPages[nItem] =
      CreateDialogParam( hInst,
                         MAKEINTRESOURCE( ID_DOC_PROP_STATISTICS_DLG ),
                         hwnd,
                         DocPropStatisticsDlgProc,
                         (LPARAM)pIda );

      SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                    rect.left, rect.top,
                    rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
      SetFocus( pIda->hwndPages[nItem] );
      UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
      nItem++;

    } // end if

  } /* endif */


  // -----------------------------------------------------------------
  //
  // hide all dialog pages but the first one
  //
  // -----------------------------------------------------------------

  if ( fOk )
  {
    int i = 1;
    while ( pIda->hwndPages[i] )
    {
      ShowWindow( pIda->hwndPages[i], SW_HIDE );
      i++;
    } /* endwhile */
  } /* endif */

  if ( !fOk )
  {
    POSTEQFCLOSE( hwnd, FALSE );
  } /* endif */

  return fOk;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocPropSettingsDlgProc                                   |
//+----------------------------------------------------------------------------+
//|Function call:     DocPropSettingsDlgProc( hwnd, winmsg, mp1, mp2 );        |
//+----------------------------------------------------------------------------+
//|Description:       handles 1st property page: folder and document settings  |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND    hwnd   handle of the dialog                      |
//|                   WINMSG  msg    windows message                           |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch message                                           |
//|                      init dialog                                           |
//|                      command                                               |
//|                      close dialog                                          |
//|                   return                                                   |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK DocPropSettingsDlgProc
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{ PDOCPROPIDA     pIda;               // ptr to instance data area
  MRESULT         mResult = FALSE;    // dialog procedure return value
  BOOL            fOK = TRUE;         // internal ok flag
  PSZ             pszTemp;            // working pointer
  PPROPDOCUMENT    pProp = NULL;       // pointer to document properties
  SHORT            sItem;              // item index for listboxes and comboboxes

  switch ( msg )
  {
    case WM_INITDLG :
      fOK = TRUE;

      /**************************************************************/
      /* Get and anchor document property dialog IDA pointer        */
      /**************************************************************/
      pIda = (PDOCPROPIDA)mp2;

      ANCHORDLGIDA( hwnd, pIda );

      /**************************************************************/
      /* Access document properties                                 */
      /**************************************************************/
      if ( (pIda->hDocProp == NULL) && (pIda->pszObjNameList == NULL) &&
           (((pIda->pDocInfo != NULL) && (pIda->pDocInfo->pDocProps == NULL) ) ) )

      {
        /************************************************************/
        /* Error accessing properties of document                   */
        /************************************************************/
        pszTemp = pIda->szDocObjName;
        UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
        fOK = FALSE;
      } /* endif */

      /**************************************************************/
      /* Fill dialog fields with available data                     */
      /**************************************************************/
      if ( fOK )
      {
        if ( pIda->pszObjNameList )
        {
          // no prop file to be used
        }
        else if ( (pIda->pDocInfo != NULL) && (pIda->pDocInfo->pDocProps != NULL) )
        {
          // use supplied document properties
          pProp = pIda->pDocInfo->pDocProps;
        }
        else
        {
          // access loaded properties
          pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->hDocProp );
        } /* endif */

        /**************************************************************/
        /* Set color of output text statics to SYSCLR_WINDOWTEXT      */
        /**************************************************************/
        SETCOLOR( hwnd, ID_DOCPROP_NAME_TEXT, SYSCLR_WINDOWTEXT );
        SETCOLOR( hwnd, ID_DOCPROP_FOLFORMAT_TEXT, SYSCLR_WINDOWTEXT );
        SETCOLOR( hwnd, ID_DOCPROP_FOLTM_TEXT, SYSCLR_WINDOWTEXT );
        SETCOLOR( hwnd, ID_DOCPROP_FOLSRCLNG_TEXT, SYSCLR_WINDOWTEXT );
        SETCOLOR( hwnd, ID_DOCPROP_FOLTRGLNG_TEXT, SYSCLR_WINDOWTEXT );
        SETCOLOR( hwnd, ID_DOCPROP_FOLEDITOR_TEXT, SYSCLR_WINDOWTEXT );
        SETCOLOR( hwnd, ID_DOCPROP_FOLCONV_TEXT, SYSCLR_WINDOWTEXT );
        /**************************************************************/
        /* Fill document name text static                             */
        /**************************************************************/
        if ( pIda->pszObjNameList )
        {
          OEMTOANSI( pIda->szAllNameBuffer );
          SETTEXT( hwnd, ID_DOCPROP_NAME_TEXT, pIda->szAllNameBuffer );
          ANSITOOEM( pIda->szAllNameBuffer );
          HIDECONTROL( hwnd, ID_DOCPROP_RELPATH_TEXT );
          HIDECONTROL( hwnd, ID_DOCPROP_RELPATH_LABEL );
          HIDECONTROL( hwnd, ID_DOCPROP_SHORTNAME_TEXT );
          HIDECONTROL( hwnd, ID_DOCPROP_SHORTNAME_LABEL );
        }
        else if ( pProp->szLongName[0] != EOS )
        {
          PSZ  pszPathEnd;

          OEMTOANSI( pProp->szLongName );
          pszPathEnd = strrchr( pProp->szLongName, BACKSLASH );
          if ( pszPathEnd != NULL )
          {
            *pszPathEnd = EOS;
            SETTEXT( hwnd, ID_DOCPROP_NAME_TEXT, pszPathEnd + 1 );
            SETTEXT( hwnd, ID_DOCPROP_RELPATH_TEXT, pProp->szLongName );
            *pszPathEnd = BACKSLASH;
          }
          else
          {
            SETTEXT( hwnd, ID_DOCPROP_NAME_TEXT, pProp->szLongName );
            HIDECONTROL( hwnd, ID_DOCPROP_RELPATH_TEXT );
            HIDECONTROL( hwnd, ID_DOCPROP_RELPATH_LABEL );
          } /* endif */
          ANSITOOEM( pProp->szLongName );
        }
        else
        {
          SETTEXT( hwnd, ID_DOCPROP_NAME_TEXT, pProp->PropHead.szName );
          HIDECONTROL( hwnd, ID_DOCPROP_RELPATH_TEXT );
          HIDECONTROL( hwnd, ID_DOCPROP_RELPATH_LABEL );
        } /* endif */
        SETTEXT( hwnd, ID_DOCPROP_SHORTNAME_TEXT, pProp->PropHead.szName );

        /**************************************************************/
        /* Fill document alias name entry field                       */
        /**************************************************************/
        if ( pIda->pszObjNameList )
        {
          HIDECONTROL( hwnd, ID_DOCPROP_ALIAS_EF );
        }
        else
        {
          SETTEXTLIMIT( hwnd, ID_DOCPROP_ALIAS_EF, sizeof(pProp->szAlias) - 1 );
          OEMTOANSI( pProp->szAlias );
          SETTEXT( hwnd, ID_DOCPROP_ALIAS_EF, pProp->szAlias );
          ANSITOOEM( pProp->szAlias );
        }

        SetCtrlFnt (hwnd, GetCharSet(),ID_DOCPROP_NAME_TEXT, 0);
        SetCtrlFnt (hwnd, GetCharSet(),ID_DOCPROP_ALIAS_EF, 0);

        /**************************************************************/
        /* Fill listbox part of comboboxes                            */
        /**************************************************************/
        EqfSend2Handler( TAGTABLEHANDLER, WM_EQF_INSERTNAMES,
                         MP1FROMHWND( WinWindowFromID( hwnd, ID_DOCPROP_DOCFORMAT_CB) ),
                         0L );
        CBINSERTITEM( hwnd, ID_DOCPROP_DOCFORMAT_CB, EMPTY_STRING );


        UtlFillTableLB( WinWindowFromID( hwnd, ID_DOCPROP_DOCSRCLNG_CB ),
                        SOURCE_LANGUAGES );
        CBINSERTITEM( hwnd, ID_DOCPROP_DOCSRCLNG_CB, EMPTY_STRING );

        UtlFillTableLB( WinWindowFromID( hwnd, ID_DOCPROP_DOCTRGLNG_CB ),
                        ALL_TARGET_LANGUAGES );
        CBINSERTITEM( hwnd, ID_DOCPROP_DOCTRGLNG_CB, EMPTY_STRING );

        EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES,
                         MP1FROMHWND( WinWindowFromID( hwnd,
                                                       ID_DOCPROP_DOCTM_CB )),
                         MP2FROMP( MEMORY_ALL ) );
        CBINSERTITEM( hwnd, ID_DOCPROP_DOCTM_CB, EMPTY_STRING );

        CBSETTEXTLIMIT( hwnd, ID_DOCPROP_DOCTM_CB, MAX_LONGFILESPEC - 1 );
        // fill editor combobox
        UtlMakeEQFPath( pIda->szBuffer, NULC, PROPERTY_PATH, NULL );
        UtlMakeFullPath( pIda->szBuffer2, NULL, pIda->szBuffer,
                         DEFAULT_PATTERN_NAME, EXT_OF_EDITOR );
        UtlLoadFileNames( pIda->szBuffer2, FILE_NORMAL,
                          WinWindowFromID( hwnd, ID_DOCPROP_DOCEDITOR_CB),
                          NAMFMT_NOEXT );
        CBINSERTITEM( hwnd, ID_DOCPROP_DOCEDITOR_CB, EMPTY_STRING );

        // fill conversion combobox
        //UtlHandleConversionStrings( CONVLOAD_MODE,
        //                            WinWindowFromID( hwnd, ID_DOCPROP_DOCCONV_CB ),
        //                            NULL, NULL, NULL  );
        //CBINSERTITEM( hwnd, ID_DOCPROP_DOCCONV_CB, EMPTY_STRING );

        /**************************************************************/
        /* For multsel props:        get settings from IDA, set state */
        /*                           of controls depending on mode    */
        /*                           flags                            */
        /* For property dialogs:     fill statistics part of dialog,  */
        /*                           select active document memory,   */
        /*                           format and languages             */
        /* For new document dialogs: use settings from document info  */
        /*                           structure,                       */
        /*                           change dialog ID                 */
        /**************************************************************/
        if ( pIda->pszObjNameList )
        {
          if ( pIda->modeFormat == SAMEVALUE )
          {
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCFORMAT_CB, pIda->szDocFormat );
            if ( sItem == LIT_NONE )
            {
              CHAR szFormat[MAX_FORMAT];
              PSZ pszErrParm = szFormat;
              pszErrParm = pIda->szDocFormat;
              if ( pIda->szDocFormat[0] ) UtlError( ERROR_FORMAT_NOTFOUND, MB_OK, 1, &pszErrParm, EQF_ERROR );
              SendDlgItemMessage( hwnd, ID_DOCPROP_DOCFORMAT_CB, CB_SETCURSEL, 0, 0L );
            } /* endif */
          } /* endif */

          if ( (pIda->modeMemory == SAMEVALUE) && (pIda->szDocMemory[0] != EOS) )
          {
            OEMTOANSI( pIda->szDocMemory );
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCTM_CB, pIda->szDocMemory );
            if ( sItem == LIT_NONE)
            {
              PSZ pszErrParm = pIda->szDocMemory;
              UtlError( ERROR_MEMORY_NOTFOUND, MB_OK, 1, &pszErrParm, EQF_ERROR );
              SETTEXT( hwnd, ID_DOCPROP_DOCTM_CB, pIda->szDocMemory );
              SETFOCUS( hwnd, ID_DOCPROP_DOCTM_CB );
            } /* endif */
            ANSITOOEM( pIda->szDocMemory );
          } /* endif */

          if ( pIda->modeSourceLang == SAMEVALUE )
          {
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCSRCLNG_CB, pIda->szDocSourceLang );
          } /* endif */

          if ( pIda->modeTargetLang == SAMEVALUE )
          {
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCTRGLNG_CB, pIda->szDocTargetLang );
          } /* endif */

          if ( pIda->modeEditor == SAMEVALUE )
          {
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCEDITOR_CB, pIda->szDocEditor );
          } /* endif */

          //if ( pIda->modeConversion == SAMEVALUE )
          //{
          //  CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCCONV_CB, pIda->szDocConversion );
          //} /* endif */
        }
        else
        if ( pIda->fPropDlg )
        {
          /************************************************************/
          /* Select currently active values in comboboxes             */
          /************************************************************/
          if ( pProp->szFormat[0] != EOS )
          {
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCFORMAT_CB, pProp->szFormat );
            if ( sItem == LIT_NONE )
            {
              PSZ pszErrParm = pProp->szFormat;
              UtlError( ERROR_FORMAT_NOTFOUND, MB_OK, 1, &pszErrParm, EQF_ERROR );
              CBSELECTITEM( hwnd, ID_DOCPROP_DOCFORMAT_CB, 0 );
            } /* endif */
          } /* endif */

          if ( pProp->szLongMemory[0] != EOS )
          {
            strcpy( szConvBuffer, pProp->szLongMemory );
            OEMTOANSI( szConvBuffer );
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCTM_CB, szConvBuffer );
            if ( sItem == LIT_NONE)
            {
              PSZ pszErrParm = szConvBuffer;
              UtlError( ERROR_MEMORY_NOTFOUND, MB_OK, 1, &pszErrParm, EQF_ERROR );
              SETTEXT( hwnd, ID_DOCPROP_DOCTM_CB, szConvBuffer );
              SETFOCUS( hwnd, ID_DOCPROP_DOCTM_CB );
            } /* endif */
          }
          else if ( pProp->szMemory[0] != EOS )
          {
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCTM_CB, pProp->szMemory );
            if ( sItem == LIT_NONE)
            {
              PSZ pszErrParm = pProp->szMemory;
              UtlError( ERROR_MEMORY_NOTFOUND, MB_OK, 1, &pszErrParm, EQF_ERROR );
              SETTEXT( hwnd, ID_DOCPROP_DOCTM_CB, pProp->szMemory );
              SETFOCUS( hwnd, ID_DOCPROP_DOCTM_CB );

            } /* endif */
          } /* endif */
          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCSRCLNG_CB, pProp->szSourceLang );
          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCTRGLNG_CB, pProp->szTargetLang );
          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCEDITOR_CB, pProp->szEditor );
//          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCCONV_CB, pProp->szConversion );

          if ( pProp->fMarkupLocked ) ENABLECTRL( hwnd, ID_DOCPROP_DOCFORMAT_CB, FALSE );
          if ( pProp->fSourceLangLocked ) ENABLECTRL( hwnd, ID_DOCPROP_DOCSRCLNG_CB, FALSE );
          if ( pProp->fTargetLangLocked ) ENABLECTRL( hwnd, ID_DOCPROP_DOCTRGLNG_CB, FALSE );
        }
        else
        {
          /************************************************************/
          /* Use document info structure for current combobox settings*/
          /************************************************************/
          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCFORMAT_CB,
                          pIda->pDocInfo->szFormat );
          if ( sItem == LIT_NONE ) {
             CBSELECTITEM( hwnd, ID_DOCPROP_DOCFORMAT_CB, 0 );
          }
          if ( pIda->pDocInfo->szLongMemory[0] )
          {
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCTM_CB, pIda->pDocInfo->szLongMemory );
          }
          else
          {
            CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCTM_CB, pIda->pDocInfo->szMemory );
          } /* endif */
          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCSRCLNG_CB,
                          pIda->pDocInfo->szSourceLang );
          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCTRGLNG_CB,
                          pIda->pDocInfo->szTargetLang );
          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCEDITOR_CB,
                          pIda->pDocInfo->szEditor );
//          CBSEARCHSELECT( sItem, hwnd, ID_DOCPROP_DOCCONV_CB, pIda->pDocInfo->szConversion );

          if ( pIda->pDocInfo->fMarkupLocked ) ENABLECTRL( hwnd, ID_DOCPROP_DOCFORMAT_CB, FALSE );
          if ( pIda->pDocInfo->fSourceLangLocked ) ENABLECTRL( hwnd, ID_DOCPROP_DOCSRCLNG_CB, FALSE );
          if ( pIda->pDocInfo->fTargetLangLocked ) ENABLECTRL( hwnd, ID_DOCPROP_DOCTRGLNG_CB, FALSE );
        } /* endif */

        // get parent object name
        if ( pIda->pszObjNameList )
        {
          // parent name has already been set ...
        }
        else
        {
          if ( pProp->ulParentFolder == 0 )
          {
            // parent is the main folder
            strcpy( pIda->szParentObject, pProp->PropHead.szPath );
            pIda->fParentIsSubfolder = FALSE;
          }
          else
          {
            // parent is a subfolder
            SubFolIdToObjectName( pProp->PropHead.szPath, pProp->ulParentFolder, pIda->szParentObject );
            pIda->fParentIsSubfolder = TRUE;
          } /* endif */
        } /* endif */

        FolQueryInfoEx( pIda->szParentObject,    // parent object name
                        pIda->szMemory,
                        pIda->szFormat,
                        pIda->szSourceLang,
                        pIda->szTargetLang,
                        pIda->szEditor,
                        pIda->szConversion,
                         NULL, NULL, NULL, NULL, NULL, TRUE, hwnd );


        // for subfolder change heading of folder groupbox
        if ( pIda->fParentIsSubfolder )
        {
          SETTEXT( hwnd, ID_DOCPROP_FOLSETTINGS_TEXT, "Subfolder settings" );
        } /* endif */

        SETTEXT( hwnd, ID_DOCPROP_FOLFORMAT_TEXT, pIda->szFormat );
        strcpy( szConvBuffer, pIda->szMemory );
        OEMTOANSI( szConvBuffer );
        SETTEXT( hwnd, ID_DOCPROP_FOLTM_TEXT, szConvBuffer  );
        SETTEXT( hwnd, ID_DOCPROP_FOLSRCLNG_TEXT, pIda->szSourceLang );
        SETTEXT( hwnd, ID_DOCPROP_FOLTRGLNG_TEXT, pIda->szTargetLang );
        SETTEXT( hwnd, ID_DOCPROP_FOLEDITOR_TEXT, pIda->szEditor );
        SETTEXT( hwnd, ID_DOCPROP_FOLCONV_TEXT, pIda->szConversion );
      } /* endif */

      /**************************************************************/
      /* End dialog in case of failures or set focus to first       */
      /* control of dialog                                          */
      /**************************************************************/


      if ( fOK )
      {
//      UtlCheckDlgPos( hwnd, FALSE );
//      SETFOCUS( hwnd, ID_DOCPROP_ALIAS_EF );
      }
      else
      {
        WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
      } /* endif */

      mResult = fOK;

      break;

    case WM_COMMAND:
      {
        SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );

        pIda = ACCESSDLGIDA( hwnd, PDOCPROPIDA );

        switch ( WMCOMMANDID( mp1, mp2 ) )
        {

	        case ID_DOCPROP_DOCTM_CB :
            if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
            {
              pIda->modeMemory = VALUECHANGED;
            } /* endif */
		        break;

          case ID_DOCPROP_DOCSRCLNG_CB:
            if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
            {
              pIda->modeSourceLang = VALUECHANGED;
            } /* endif */
		        break;

          case ID_DOCPROP_DOCTRGLNG_CB:
            if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
            {
              pIda->modeTargetLang = VALUECHANGED;
            } /* endif */
		        break;

          case ID_DOCPROP_DOCEDITOR_CB:
            if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
            {
              pIda->modeEditor = VALUECHANGED;
            } /* endif */
		        break;

          //case ID_DOCPROP_DOCCONV_CB:
          //  if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
          //  {
          //    pIda->modeConversion = VALUECHANGED;
          //  } /* endif */
		        //break;

          case ID_DOCPROP_DOCFORMAT_CB:
            if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
            {
              pIda->modeFormat = VALUECHANGED;
            } /* endif */
		        break;


          case ID_DOCPROP_CHANGE_PB :
            // get document values
            QUERYTEXT( hwnd, ID_DOCPROP_DOCTM_CB,     pIda->szDocMemory );
            QUERYTEXT( hwnd, ID_DOCPROP_DOCSRCLNG_CB, pIda->szDocSourceLang );
            QUERYTEXT( hwnd, ID_DOCPROP_DOCTRGLNG_CB, pIda->szDocTargetLang );
            QUERYTEXT( hwnd, ID_DOCPROP_DOCEDITOR_CB, pIda->szDocEditor );
            QUERYTEXT( hwnd, ID_DOCPROP_DOCCONV_CB,   pIda->szDocConversion );
            QUERYTEXT( hwnd, ID_DOCPROP_ALIAS_EF, pIda->szChangedAliasBuffer );
            QUERYTEXT( hwnd, ID_DOCPROP_DOCFORMAT_CB, pIda->szDocFormat );

            // get folder settings
            QUERYTEXT( hwnd, ID_DOCPROP_FOLCONV_TEXT, pIda->szConvFolBuffer );
            QUERYTEXT( hwnd, ID_DOCPROP_FOLTM_TEXT, pIda->szTransMemFolBuffer );
            QUERYTEXT( hwnd, ID_DOCPROP_FOLSRCLNG_TEXT, pIda->szSourceLangFolBuffer );
            QUERYTEXT( hwnd, ID_DOCPROP_FOLTRGLNG_TEXT, pIda->szTargetLangFolBuffer );
            QUERYTEXT( hwnd, ID_DOCPROP_FOLEDITOR_TEXT, pIda->szEditorFolBuffer );
            QUERYTEXT( hwnd, ID_DOCPROP_FOLFORMAT_TEXT, pIda->szMarkupFolBuffer );

   		    mResult = MRFROMSHORT( TRUE ); // TRUE is the default return value

            // Determine if selected source/target language is matching our selected editor
            if ( pIda->szDocSourceLang[0] || pIda->szDocTargetLang[0] || pIda->szDocEditor[0] )
		        {
			        PSZ pEditor = (pIda->szDocEditor[0]) ? pIda->szDocEditor : pIda->szEditorFolBuffer;
			        PSZ pTgtLang = (pIda->szDocTargetLang[0]) ? pIda->szDocTargetLang : pIda->szTargetLangFolBuffer;
			        PSZ pSrcLang = (pIda->szDocSourceLang[0]) ? pIda->szDocSourceLang : pIda->szSourceLangFolBuffer;
              PSZ pFail = UtlEditorLangSupport( pEditor, pSrcLang, pTgtLang );
                if ( pFail )
                {
                  PSZ pszErrParm[2];
                  pszErrParm[0] = pFail;
                  UtlErrorHwnd( TB_LANG_NOT_SUPPORTED_BY_EDITOR,
                                MB_CANCEL,
                                1, pszErrParm, EQF_ERROR, hwnd );

        				  SETFOCUS( hwnd, ID_DOCPROP_DOCEDITOR_CB );

		          mResult = MRFROMSHORT( FALSE ); // TRUE is the default return value
                } /* endif */
            } /* endif */
            break;

        }//end switch
      }
      break;

    case WM_CLOSE:
      //-----------------------------------/

//    DelCtrlFont( hwnd, ID_DOCPROP_NAME_TEXT );
//    DelCtrlFont( hwnd, ID_DOCPROP_ALIAS_EF );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  }
  return mResult;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:      UPDATE_DOCPROP_DOCTRANSL_NAME                           |
//+----------------------------------------------------------------------------+
//|Function call:      UPDATE_DOCPROP_DOCTRANSL_NAME (pIda, hwndDlg);          +
//+----------------------------------------------------------------------------+
//|Description:        updates comboboxes for translator's  name               |
//+----------------------------------------------------------------------------+
//|Parameters:         PDOCPROPIDA    pIda    pointer to ID structure          |
//|                    HWND           hwnd    handle of dialog                 |
//+----------------------------------------------------------------------------+
//|Returncode type:    fOK                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:        TRUE or FALSE                                           |
//+----------------------------------------------------------------------------+
//|Function flow:      init name field                                         |
//|                    select last used values                                 |
//|                    insert new value                                        |
//+----------------------------------------------------------------------------+


MRESULT APIENTRY UPDATE_DOCPROP_DOCTRANSL_NAME
(
PDOCPROPIDA   pIda,
HWND hwndDlg                                // handle of dialog window
)

{
  BOOL            fOK = TRUE;
  PPROPDOCUMENT   pDocProp = NULL;        // AH pointer to document properties

  if ( pIda->pszObjNameList )
  {
    //init project translator's name field of document administration --------------------
    ENABLECTRL( hwndDlg, ID_DOCPROP_DOCTRANSL_NAME_CB, TRUE );
    CBDELETEALL(hwndDlg, ID_DOCPROP_DOCTRANSL_NAME_CB);
    OEMTOANSI( pIda->szDocVendorName );
    CBINSERTITEMEND( hwndDlg, ID_DOCPROP_DOCTRANSL_NAME_CB, pIda->szDocVendorName );
    ANSITOOEM( pIda->szDocVendorName );
  }
  else
  {
    if ( (pIda->pDocInfo != NULL) && (pIda->pDocInfo->pDocProps != NULL) )
    {
      // use supplied document properties
      pDocProp = pIda->pDocInfo->pDocProps;
    }
    else
    {
      // access loaded properties
      pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->hDocProp );
    } /* endif */


    //init project translator's name field of document administration --------------------

    ENABLECTRL( hwndDlg, ID_DOCPROP_DOCTRANSL_NAME_CB, TRUE );

    CBDELETEALL(hwndDlg, ID_DOCPROP_DOCTRANSL_NAME_CB);


    OEMTOANSI(pDocProp->szVendor);
    CBINSERTITEMEND( hwndDlg,ID_DOCPROP_DOCTRANSL_NAME_CB, pDocProp->szVendor);
    ANSITOOEM(pDocProp->szVendor);
  }

  {

    HPROP           hFLLProp;   // folder list properties handler
    PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
    EQFINFO         ErrorInfo;  // error returned by property handler
    int             i;

    /************************************************************/
    /* Select last-used values and insert new value             */
    /************************************************************/
    UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szBuffer1, BACKSLASH_STR );
    strcat( pIda->szBuffer1, DEFAULT_FOLDERLIST_NAME );
    hFLLProp = OpenProperties( pIda->szBuffer1, NULL,
                               PROP_ACCESS_READ, &ErrorInfo);
    if ( hFLLProp )
    {
      pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );

      i=0;
      while (i < 5 && pFLLProp->szTranslatorList[i][0] != EOS)
      {
        SHORT sItem = CBSEARCHITEM(hwndDlg,ID_DOCPROP_DOCTRANSL_NAME_CB ,
                                   pFLLProp->szTranslatorList[i]);

        if (sItem == LIT_NONE)
        {
          OEMTOANSI(pFLLProp->szTranslatorList[i]);
          CBINSERTITEMEND( hwndDlg,ID_DOCPROP_DOCTRANSL_NAME_CB , pFLLProp->szTranslatorList[i]);
          ANSITOOEM(pFLLProp->szTranslatorList[i]);
        }
        i++;

      }// end while

      CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);
    } /* endif */

  }

  if ( pIda->pszObjNameList )
  {
    SHORT sItem = CBSEARCHITEM(hwndDlg,ID_DOCPROP_DOCTRANSL_NAME_CB ,
                               pIda->szDocVendorName );
    SendDlgItemMessage( hwndDlg, ID_DOCPROP_DOCTRANSL_NAME_CB, CB_SETCURSEL, max(sItem,0), 0L );
  }
  else
  {
    SHORT sItem = CBSEARCHITEM(hwndDlg,ID_DOCPROP_DOCTRANSL_NAME_CB ,
                               pDocProp->szVendor);
    CBSELECTITEM (hwndDlg,ID_DOCPROP_DOCTRANSL_NAME_CB ,max(sItem,0) );
  }

  return fOK;
} // end of function UPDATE_DOCPROP_DOCTRANSL_NAME

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocPropAdminDlgProc                                      |
//+----------------------------------------------------------------------------+
//|Function call:     DocPropAdminDlgProc( hwnd, winmsg, mp1, mp2 );           |
//+----------------------------------------------------------------------------+
//|Description:       handles 2nd property page: administration of documents   |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND    hwnd   handle of the dialog                      +
//|                   WINMSG  msg    windows message                           |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       +
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch message                                           |
//|                      init dialog                                           |
//|                      command                                               |
//|                      close dialog                                          |
//|                   return                                                   |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK DocPropAdminDlgProc
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  PDOCPROPIDA     pIda;               // ptr to instance data area
  MRESULT         mResult = FALSE;    // dialog procedure return value
  PPROPDOCUMENT   pDocProp;           // AH pointer to document properties
  EQFINFO         ErrorInfo;          // error code of property handler calls
  BOOL            fOK = TRUE;         // internal ok flag
  PPROPFOLDER     pFolProp;           // ptr to folder properties
  PVOID           hFolProp;           // handle of folder properties
  PSZ             pszReplace;

  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );

  switch ( msg )
  {
    case WM_INITDLG :

      {
        fOK = TRUE;

        /**************************************************************/
        /* Get and anchor document property dialog IDA pointer        */
        /**************************************************************/
        pIda = (PDOCPROPIDA)mp2;
        ANCHORDLGIDA( hwnd, pIda );

        /**************************************************************/
        /* Set color of output text statics to SYSCLR_WINDOWTEXT      */
        /**************************************************************/
        SETCOLOR( hwnd, ID_DOCPROP_FOLTRANSL_NAME_EF, SYSCLR_WINDOWTEXT );
        SETCOLOR( hwnd, ID_DOCPROP_FOLTRANSL_MAIL_EF, SYSCLR_WINDOWTEXT );
        SETCOLOR( hwnd, ID_DOCPROP_FOLPROD_SHIPMNT_EF, SYSCLR_WINDOWTEXT );

        /******************************************************************/
        /* Open folder properties                                         */
        /******************************************************************/
        UtlQueryString( QST_PRIMARYDRIVE, pIda->szBuffer, sizeof(pIda->szBuffer) );

        if ( pIda->pszObjNameList )
        {
          // create folder object name (= primary drive + document object name without document name)
          pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->hDocProp );
          strcpy( pIda->szBuffer+1, pIda->pszObjNameList +1 );
          UtlSplitFnameFromPath( pIda->szBuffer );
        }
        else
        {
          // create folder object name (= primary drive + document path)
          if ( (pIda->pDocInfo != NULL) && (pIda->pDocInfo->pDocProps != NULL) )
          {
            // use supplied document properties
            pDocProp = pIda->pDocInfo->pDocProps;
          }
          else
          {
            // access loaded properties
            pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->hDocProp );
          } /* endif */
          strcpy( pIda->szBuffer+1, pDocProp->PropHead.szPath+1 );
        } /* endif */

        {
          hFolProp = OpenProperties( pIda->szBuffer,
                                     NULL,
                                     PROP_ACCESS_READ,
                                     &ErrorInfo);
        }

        /******************************************************************/
        /* Get translator's name and email of folder, get shipment number */
        /******************************************************************/

        if ( hFolProp )
        {
          pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );

          strcpy(pIda->szFolVendorName,pFolProp->szVendor);
          strcpy(pIda->szFolVendorEMail,pFolProp->szVendorEMail);
          strcpy(pIda->szFolShipment,pFolProp->szShipment);

          CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
        }
        else
        {
          pszReplace = pIda->szBuffer;
          UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL,
                    1, &pszReplace, EQF_ERROR);

        }//end if

        /********************************************************************/
        /* Write translator's name and email of folder, get shipment number */
        /********************************************************************/

        OEMTOANSI(pIda->szFolVendorName);
        SETTEXT( hwnd, ID_DOCPROP_FOLTRANSL_NAME_EF, pIda->szFolVendorName );
        ANSITOOEM(pIda->szFolVendorName);

        OEMTOANSI(pIda->szFolVendorEMail);
        SETTEXT( hwnd, ID_DOCPROP_FOLTRANSL_MAIL_EF, pIda->szFolVendorEMail );
        ANSITOOEM(pIda->szFolVendorEMail);

        OEMTOANSI(pIda->szFolShipment);
        SETTEXT( hwnd, ID_DOCPROP_FOLPROD_SHIPMNT_EF, pIda->szFolShipment );
        ANSITOOEM(pIda->szFolShipment);


        SetCtrlFnt (hwnd, GetCharSet(),ID_DOCPROP_FOLTRANSL_NAME_EF, 0);
        SetCtrlFnt (hwnd, GetCharSet(),ID_DOCPROP_FOLTRANSL_MAIL_EF, 0);
        SetCtrlFnt (hwnd, GetCharSet(),ID_DOCPROP_FOLPROD_SHIPMNT_EF, 0);

        /*********************************************/
        /* Handle user name and email  of document   */
        /*********************************************/


        /*Init project translator's name field for documents*/

        UPDATE_DOCPROP_DOCTRANSL_NAME(pIda, hwnd);

        /*Init project translator's email field for documents*/

        ENABLECTRL( hwnd, ID_DOCPROP_DOCTRANSL_MAIL_CB, TRUE );
        if ( pIda->pszObjNameList )
        {
          if ( pIda->szDocVendorEMail[0] != EOS )
          {
            OEMTOANSI(pIda->szDocVendorEMail);
            SETTEXT( hwnd, ID_DOCPROP_DOCTRANSL_MAIL_CB, pIda->szDocVendorEMail );
            ANSITOOEM(pIda->szDocVendorEMail);
          }
        }
        else
        {
          if (pDocProp->szVendorEMail[0] != EOS )
          {
            OEMTOANSI(pDocProp->szVendorEMail);
            SETTEXT( hwnd, ID_DOCPROP_DOCTRANSL_MAIL_CB, pDocProp->szVendorEMail );
            ANSITOOEM(pDocProp->szVendorEMail);
          }
        }

        /*Init shipment number field for documents*/

        ENABLECTRL( hwnd, ID_DOCPROP_DOCPROD_SHIPMNT_CB, TRUE );
        if ( pIda->pszObjNameList )
        {
          if ( pIda->szDocShipment[0] != EOS )
          {
            OEMTOANSI(pIda->szDocShipment);
            SETTEXT( hwnd, ID_DOCPROP_DOCPROD_SHIPMNT_CB, pIda->szDocShipment );
            ANSITOOEM(pIda->szDocShipment);
          }
        }
        else
        {
          if ( pDocProp->szShipment[0] != EOS )
          {
            OEMTOANSI(pDocProp->szShipment);
            SETTEXT( hwnd, ID_DOCPROP_DOCPROD_SHIPMNT_CB, pDocProp->szShipment );
            ANSITOOEM(pDocProp->szShipment);
          }
        }


        /**************************************************************/
        /* End dialog in case of failures or set focus to first       */
        /* control of dialog                                          */
        /**************************************************************/
        if ( fOK )
        {
          UtlCheckDlgPos( hwnd, FALSE );
          SETFOCUS( hwnd, ID_DOCPROP_DOCTRANSL_NAME_CB );
        }
        else
        {
          WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
        } /* endif */

        mResult = TRUE;

      }

      break;  //INIT

    case WM_COMMAND:
      {
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_DOCPROP_CHANGE_PB :
            {
              VENDORNAME szVendor;
              VENDORNAME szVendorMail;
              USHORT          usErrMsg = 0;           // message number to display in UtlError

              pIda = ACCESSDLGIDA(hwnd, PDOCPROPIDA );

              if ( !usErrMsg )
              {
                OEMQUERYTEXT( hwnd, ID_DOCPROP_DOCTRANSL_NAME_CB, szVendor );
                OEMQUERYTEXT( hwnd, ID_DOCPROP_DOCTRANSL_MAIL_CB, szVendorMail );
              } /* endif */

              if ( pIda->pszObjNameList )
              {
                strcpy( pIda->szDocVendorName, szVendor);
                strcpy( pIda->szDocVendorEMail, szVendorMail );
              }
              else
              {
                if ( (pIda->pDocInfo != NULL) && (pIda->pDocInfo->pDocProps != NULL) )
                {
                  // use supplied document properties
                  pDocProp = pIda->pDocInfo->pDocProps;
                }
                else
                {
                  // access loaded properties
                  pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->hDocProp );
                } /* endif */
                strcpy( pDocProp->szVendor, szVendor);
                strcpy( pDocProp->szVendorEMail, szVendorMail );
              } /* endif */

              /* Get translator's name, email and shipment number of document*/



              /* Update last used translators list*/

              if ( szVendor[0] != EOS )
              {

                HPROP           hFLLProp;   // folder list properties handler
                PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
                EQFINFO         ErrorInfo;  // error returned by property handler

                /************************************************************/
                /* Set last-used values                                     */
                /************************************************************/
                UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
                strcat( pIda->szBuffer1, BACKSLASH_STR );
                strcat( pIda->szBuffer1, DEFAULT_FOLDERLIST_NAME );
                hFLLProp = OpenProperties( pIda->szBuffer1, NULL,
                                           PROP_ACCESS_READ, &ErrorInfo);
                if ( hFLLProp )
                {
                  int             i;
                  int             iFound=-1;

                  SetPropAccess( hFLLProp, PROP_ACCESS_WRITE);
                  pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );


                  // is insertion necessary ????
                  for (i=0;i<5;i++)
                  {
                    if (!strcmp(szVendor,pFLLProp->szTranslatorList[i] )) iFound = i;
                  }// end for


                  if (iFound>=0)
                  {
                    for (i=iFound;i<4;i++)
                    {
                      strcpy(pFLLProp->szTranslatorList[i] , pFLLProp->szTranslatorList[i+1]);
                      strcpy(pFLLProp->szTranslatorMailList[i] , pFLLProp->szTranslatorMailList[i+1]);
                    }// end for

                  } // end iFound

                  for (i=4;i>=1;i--)
                  {
                    strcpy(pFLLProp->szTranslatorList[i] , pFLLProp->szTranslatorList[i-1]);
                    strcpy(pFLLProp->szTranslatorMailList[i] , pFLLProp->szTranslatorMailList[i-1]);
                  }// end for

                  strcpy(pFLLProp->szTranslatorList[0] , szVendor );
                  strcpy(pFLLProp->szTranslatorMailList[0] , szVendorMail );

                  SaveProperties( hFLLProp, &ErrorInfo );
                  CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);
                } /* endif */

              }

              if (!usErrMsg )
              {
                mResult = MRFROMSHORT( TRUE );  // result value of procedure
              }
              else
              {
                mResult = FALSE;
              }

              mResult = TRUE;

            }    // end case
            break;       //ID_DOCPROP_CHANGE_PB

          case ID_DOCPROP_DOCTRANSL_MAIL_CB:
            {
              if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
              {
                pIda = ACCESSDLGIDA(hwnd, PDOCPROPIDA );
                pIda->modeVendorEMail = VALUECHANGED;
              } /* endif */
              mResult = MRFROMSHORT(TRUE);
            }
            break;


          case ID_DOCPROP_FOLPROD_SHIPMNT_EF:
            {
              if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
              {
                pIda = ACCESSDLGIDA(hwnd, PDOCPROPIDA );
                pIda->modeShipment = VALUECHANGED;
              } /* endif */
              mResult = MRFROMSHORT(TRUE);
            }
            break;


          case ID_DOCPROP_DOCTRANSL_NAME_CB:
            if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
            {
              pIda = ACCESSDLGIDA(hwnd, PDOCPROPIDA );
              pIda->modeVendorName = VALUECHANGED;
            } /* endif */

            if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
            {
              int iFound = -1;
              int i;
              PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
              HPROP           hFLLProp;   // folder list properties handler
              EQFINFO         ErrorInfo;  // error returned by property handler
              PSZ             pszVendor;
              PSZ             pszVendorMail;

              pIda = ACCESSDLGIDA(hwnd, PDOCPROPIDA );

              if ( pIda->pszObjNameList )
              {
                pszVendor = pIda->szDocVendorName;
                pszVendorMail = pIda->szDocVendorEMail;
              }
              else
              {
                if ( (pIda->pDocInfo != NULL) && (pIda->pDocInfo->pDocProps != NULL) )
                {
                  // use supplied document properties
                  pDocProp = pIda->pDocInfo->pDocProps;
                }
                else
                {
                  // access loaded properties
                  pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->hDocProp );
                } /* endif */
                pszVendor = pDocProp->szVendor;
                pszVendorMail = pDocProp->szVendorEMail;
              } /* endif */

              {
                UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
                strcat( pIda->szBuffer1, BACKSLASH_STR );
                strcat( pIda->szBuffer1, DEFAULT_FOLDERLIST_NAME );
                hFLLProp = OpenProperties( pIda->szBuffer1, NULL,
                                           PROP_ACCESS_READ, &ErrorInfo);
                if ( hFLLProp )
                {

                  pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );

                  if ( sNotification == CBN_SELCHANGE )
                  {
                    int ipos;
                    CBQUERYSELECTEDITEMTEXT (ipos ,hwnd,
                                             ID_DOCPROP_DOCTRANSL_NAME_CB , pszVendor );
                  }
                  else
                  {
                    GetDlgItemText( hwnd, ID_DOCPROP_DOCTRANSL_NAME_CB, pszVendor,
                                    sizeof( pIda->szDocVendorName ));
                  } /* endif */

                  if (*pszVendor != EOS)
                  {
                    for (i=0;i<5;i++)
                    {
                      if (!strcmp(pszVendor,pFLLProp->szTranslatorList[i] )) iFound = i;
                    }// end for


                    // perhaps iFound does not exist

                    if (0 <= iFound && iFound < 5)
                    {
                      if (pFLLProp->szTranslatorMailList[iFound][0] != EOS)
                      {
                        strcpy(pszVendorMail, pFLLProp->szTranslatorMailList[iFound]);

                        OEMTOANSI(pszVendorMail);
                        SETTEXT(hwnd, ID_DOCPROP_DOCTRANSL_MAIL_CB, pszVendorMail);
                        ANSITOOEM(pszVendorMail);
                      } // end if
                    } // end if
                    if ( sNotification == CBN_SELCHANGE )
                    {
                      SETFOCUS( hwnd, ID_DOCPROP_DOCTRANSL_NAME_CB );
                    } // end if


                  } // end if vendor
                  CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);

                } // endif
              } // end notification
            } // end case
            mResult = MRFROMSHORT(TRUE);

            break;


        } // end switch WM_COMMAND
        break;  // case WM_COMMAND
      }


    case WM_CLOSE:
      //-----------------------------------/

      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );

      break;


    case WM_HELP:
      {

        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/
        pIda = ACCESSDLGIDA( hwnd, PDOCPROPIDA );
        if ( pIda->fPropDlg ) //existing document
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblDocPropDlg[0] );
        }
        else   // new document
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblNewDocDlg[0] );
        }

        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP






    default:

      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );

      break;





  } //end switch


  return mResult;


} // end of DocPropAdminDlgProc


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocPropStatisticsDlgProc                                 |
//+----------------------------------------------------------------------------+
//|Function call:     DocPropStatisticsDlgProc( hwnd, winmsg, mp1, mp2 );      |
//+----------------------------------------------------------------------------+
//|Description:       handles 3nd property page: statistics of documents       |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND    hwnd   handle of the dialog                      +
//|                   WINMSG  msg    windows message                           |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       +
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch message                                           |
//|                      init dialog                                           |
//|                      command                                               |
//|                      close dialog                                          |
//|                   return                                                   |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK DocPropStatisticsDlgProc
(
HWND hwnd,                       /* handle of dialog window */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{ PDOCPROPIDA     pIda;                   // ptr to instance data area
  MRESULT         mResult = FALSE;        // dialog procedure return value
  BOOL            fOK = TRUE;             // internal ok flag
  PPROPDOCUMENT   pProp;                  // pointer to document properties

  mp1;
  switch ( msg )
  {
    case WM_INITDLG :
      fOK = TRUE;
      /**************************************************************/
      /* Get and anchor document property dialog IDA pointer        */
      /**************************************************************/
      pIda = (PDOCPROPIDA)mp2;
      ANCHORDLGIDA( hwnd, pIda );
      /**************************************************************/
      /* Fill dialog fields with available data                     */
      /**************************************************************/
      if ( fOK )
      {
        if ( (pIda->pDocInfo != NULL) && (pIda->pDocInfo->pDocProps != NULL) )
        {
          // use supplied document properties
          pProp = pIda->pDocInfo->pDocProps;
        }
        else
        {
          // access loaded properties
          pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->hDocProp );
        } /* endif */

        /**************************************************************/
        /* Set color of output text statics to SYSCLR_WINDOWTEXT      */
        /**************************************************************/
        if ( pIda->fPropDlg )
        {
          SETCOLOR( hwnd, ID_DOCPROP_TRANSLATED_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_ANALYZED_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_IMPORTED_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_EXPORTED_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_SOURCE_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_UPDATED_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_SIZE_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_COMPLETED_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_CHANGEDSEGS_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_NEWSEGS_TEXT, SYSCLR_WINDOWTEXT );
          SETCOLOR( hwnd, ID_DOCPROP_COPIEDSEGS_TEXT, SYSCLR_WINDOWTEXT );
        } /* endif */

        /**************************************************************/
        /* For property dialogs:     fill statistics part of dialog,  */
        /*                           select active document memory,   */
        /*                           format and languages             */
        /* For new document dialogs: use setings from document info   */
        /*                           structure,                       */
        /*                           change dialog ID                 */
        /**************************************************************/
        if ( pIda->fPropDlg )
        {
          FILEFINDBUF stFile;        // Output buffer of UtlFindFirst
          USHORT      usCount = 1;   // For UtlFindFirst
          HDIR        hSearch = HDIR_CREATE; // Dir handle for UtlFindFirst

          /**********************************************************/
          /* Build document path                                    */
          /**********************************************************/
          strcpy( pIda->szBuffer, pProp->PropHead.szPath );
          strcat( pIda->szBuffer, BACKSLASH_STR );
          UtlQueryString( QST_SOURCEDIR,
                          pIda->szBuffer + strlen(pIda->szBuffer),
                          MAX_FILESPEC );
          strcat( pIda->szBuffer, BACKSLASH_STR );
          strcat( pIda->szBuffer, pProp->PropHead.szName );

          /**********************************************************/
          /* Get document size                                      */
          /**********************************************************/
          memset( &stFile, 0, sizeof(stFile) );
          UtlFindFirst( pIda->szBuffer, &hSearch, 0, &stFile, sizeof(stFile), &usCount,
                        0L, FALSE );
          UtlFindClose( hSearch, FALSE );

          /**********************************************************/
          /* Fill static text controls                              */
          /**********************************************************/
          ltoa( RESBUFSIZE(stFile), pIda->szBuffer, 10 );
          SETTEXT( hwnd, ID_DOCPROP_SIZE_TEXT, pIda->szBuffer );
          LONGTODATETIME( pProp->ulXLated, pIda->szBuffer );
          SETTEXT( hwnd, ID_DOCPROP_TRANSLATED_TEXT, pIda->szBuffer );
          LONGTODATETIME( pProp->ulSeg, pIda->szBuffer );
          SETTEXT( hwnd, ID_DOCPROP_ANALYZED_TEXT, pIda->szBuffer );
          LONGTODATETIME( pProp->ulExp, pIda->szBuffer );
          SETTEXT( hwnd, ID_DOCPROP_EXPORTED_TEXT, pIda->szBuffer );
          LONGTODATETIME( pProp->ulImp, pIda->szBuffer );
          SETTEXT( hwnd, ID_DOCPROP_IMPORTED_TEXT, pIda->szBuffer );
          pIda->szBuffer[0] = 0x00;
          if (pProp->ulSrc)
          {
            LONGTODATETIME( pProp->ulSrc, pIda->szBuffer );
          }
          SETTEXT( hwnd, ID_DOCPROP_SOURCE_TEXT, pIda->szBuffer );
          LONGTODATETIME( pProp->ulTouched, pIda->szBuffer );
          SETTEXT( hwnd, ID_DOCPROP_UPDATED_TEXT, pIda->szBuffer );
          itoa( pProp->usComplete, pIda->szBuffer, 10 );
          SETTEXT( hwnd, ID_DOCPROP_COMPLETED_TEXT, pIda->szBuffer );
          itoa( pProp->usModified, pIda->szBuffer, 10 );
          SETTEXT( hwnd, ID_DOCPROP_CHANGEDSEGS_TEXT, pIda->szBuffer );
          itoa( pProp->usScratch, pIda->szBuffer, 10 );
          SETTEXT( hwnd, ID_DOCPROP_NEWSEGS_TEXT, pIda->szBuffer );
          itoa( pProp->usCopied, pIda->szBuffer, 10 );
          SETTEXT( hwnd, ID_DOCPROP_COPIEDSEGS_TEXT, pIda->szBuffer );
	    } /* endif */
      } /* endif */
      mResult = TRUE;
      break;

    case WM_CLOSE:
      //-----------------------------------/
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    case WM_HELP:
      {
        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/
        pIda = ACCESSDLGIDA( hwnd, PDOCPROPIDA );
        if ( pIda->fPropDlg ) //existing document
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblDocPropDlg[0] );
        }
        else   // new document
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblNewDocDlg[0] );
        }
        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP

    default:
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  }
  return mResult;
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     usGetNumberOfDocsInFolder                                |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Get the number of documents already in the folder        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       number of documents                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
USHORT usGetNumberOfDocsInFolder (PSZ pszFolderObjectName)
{
  FILEFINDBUF stResultBuf;                     // DOS file find struct
  USHORT      usCount = 1;                     // number of files requested
  HDIR        hDirHandle = HDIR_CREATE;        // DosFind routine handle
  PSZ         pszDocPropPath;                  // path to document properties
  PSZ         pszSearchArg;                    // search argument for FindFirst
  USHORT      usDocuments = 0;

  SETCURSOR( SPTR_WAIT );

  UtlAlloc ((PVOID *)&pszDocPropPath, 0L, (LONG) MAX_EQF_PATH, NOMSG);
  UtlAlloc ((PVOID *)&pszSearchArg, 0L, (LONG) MAX_PATH144, NOMSG);

  if ( pszDocPropPath && pszSearchArg )
  {
    UtlQueryString (QST_PROPDIR, pszDocPropPath, MAX_EQF_PATH);
    sprintf( pszSearchArg, "%s\\%s\\*%s", pszFolderObjectName, pszDocPropPath,
             EXT_DOCUMENT);

    UtlFindFirst( pszSearchArg, &hDirHandle, FILE_NORMAL,
                  &stResultBuf, sizeof(stResultBuf), &usCount, 0L, 0);
    while ( usCount )
    {
      usDocuments++;
      UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf), &usCount, 0);
    } /* endwhile */
  } /* endif */
  // close search file handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

  UtlAlloc ((PVOID *)&pszDocPropPath, 0L, 0L, NOMSG);
  UtlAlloc ((PVOID *)&pszSearchArg, 0L, 0L, NOMSG);

  SETCURSOR( SPTR_ARROW );

  return(usDocuments);
} /* end of function usGetNumberOfDocsInFolder */

//f2////////////////////////////////////////////////////////////////////////////
// function DocumentDelete                                                    //     //
////////////////////////////////////////////////////////////////////////////////
BOOL DocumentDelete( PSZ  pszObjName, BOOL fMsg, PUSHORT pusMBReturn )
{
  return( DocumentDeleteEx( pszObjName, fMsg, pusMBReturn, 0 ) );
}

BOOL DocumentDeleteEx( PSZ  pszObjName, BOOL fMsg, PUSHORT pusMBReturn, int iOptions )
{
  //---------------------------------------------------------------------------
  // delete a document in a folder                                           --
  //---------------------------------------------------------------------------
  //declare variables

  BOOL        fOK = TRUE;                      //error flag
  USHORT      usResponse;                      //return from Utlx
  CHAR        szFolder[MAX_PATH144];           //folder objname
  PSZ         pszFolderName;                   //pointer to folder name
  CHAR        szDocumentName[MAX_FILESPEC];    //document name
  PSZ         pszDocumentName = szDocumentName;
  CHAR        szFileSpec[MAX_PATH144           //file specification
                         +MAX_FILESPEC];
  CHAR        szDocPath[MAX_PATH144            //document path
                        +MAX_FILESPEC];
  static CHAR szLongName[MAX_LONGFILESPEC];
  PSZ pszLongName;

  // get document long name
  szLongName[0] = EOS;
  DocQueryInfo2( pszObjName, NULL, NULL, NULL, NULL, szLongName, NULL, NULL, FALSE );
  if ( szLongName[0] == EOS )
  {
    pszLongName = UtlGetFnameFromPath( pszObjName );
  }
  else
  {
    OEMTOANSI( szLongName );
    pszLongName = szLongName;
  } /* endif */

  //split folder and document name from passed document object name
  strcpy( szFolder, pszObjName );
  pszDocumentName = UtlSplitFnameFromPath( szFolder );
  pszFolderName = UtlSplitFnameFromPath( szFolder );

  if ( fMsg )                              //display confirmation message
  {
    if ( *pusMBReturn != MBID_EQF_YESTOALL )
    {
      PSZ pszParm = pszLongName;
      usResponse = UtlError( WARNING_DELETE_DOCUMENT,
                             MB_EQF_YESTOALL,
                             1, &pszParm, EQF_QUERY );
      *pusMBReturn = usResponse;
      if ( (usResponse != MBID_YES) &&
           (usResponse != MBID_EQF_YESTOALL) ) //file should not be deleted
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    // delete property file of the document
    UtlMakeEQFPath( szDocPath, pszObjName[0],
                    PROPERTY_PATH, pszFolderName );
    sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
    if ( UtlFileExist(szFileSpec) )
    {
      usResponse = UtlDelete( szFileSpec, 0L, fMsg );
    } /* endif */
  }/*end if Ok*/

  if ( fOK )
  {
    //call UtlDelete to delete source file of the document
    UtlMakeEQFPath( szDocPath, pszObjName[0],
                    DIRSOURCEDOC_PATH, pszFolderName );

    sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
    if ( UtlFileExist(szFileSpec) )
    {
      usResponse = UtlDelete( szFileSpec, 0L, fMsg );
    } /* endif */

    if ( fOK )
    {
      //call UtlDelete to delete segmented source file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0],
                      DIRSEGSOURCEDOC_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist(szFileSpec) )
      {
        UtlDelete( szFileSpec, 0L, fMsg );
      } /* endif */

      //call UtlDelete to delete target file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0],
                      DIRTARGETDOC_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist(szFileSpec) )
      {
        UtlDelete( szFileSpec, 0L, fMsg );
      } /* endif */

      //call UtlDelete to delete segmented target file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0],
                      DIRSEGTARGETDOC_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist(szFileSpec) )
      {
        UtlDelete( szFileSpec, 0L, fMsg );
      } /* endif */

      //call UtlDelete to delete SNOMATCH file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0],
                      DIRSEGNOMATCH_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist(szFileSpec) )
      {
        UtlDelete( szFileSpec, 0L, fMsg );
      } /* endif */

      // delete any METADATA file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0], METADATA_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist( szFileSpec) ) UtlDelete( szFileSpec, 0L, fMsg );

      // delete any RTF file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0], DIRSEGRTF_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist(szFileSpec) ) UtlDelete( szFileSpec, 0L, fMsg );

      if ( !(iOptions & DOCDELETE_NOMTLOGDELETE) )
      {
        // delete any MTLOG file of the document
        UtlMakeEQFPath( szDocPath, pszObjName[0], MTLOG_PATH, pszFolderName );
        sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
        if ( UtlFileExist(szFileSpec) ) UtlDelete( szFileSpec, 0L, fMsg );
      } /* endif */         


      // delete any XLIFF file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0], XLIFF_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist(szFileSpec) ) UtlDelete( szFileSpec, 0L, fMsg );

      // delete any MISC file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0], MISC_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist(szFileSpec) ) UtlDelete( szFileSpec, 0L, fMsg );

      // delete any Entity file of the document
      UtlMakeEQFPath( szDocPath, pszObjName[0], ENTITY_PATH, pszFolderName );
      sprintf( szFileSpec, "%s\\%s", szDocPath, pszDocumentName );
      if ( UtlFileExist(szFileSpec) ) UtlDelete( szFileSpec, 0L, fMsg );

      //send message to all handlers, that document is deleted
      EqfSend2AllHandlers( WM_EQFN_DELETED,
                           MP1FROMSHORT( clsDOCUMENT ),
                           MP2FROMP( pszObjName ));

      // write a delete record to folder history log
      {
        //split document name from passed document object name
        strcpy( szFolder, pszObjName );
        pszDocumentName = UtlSplitFnameFromPath( szFolder );

        if ( szLongName[0] ) ANSITOOEM( szLongName );
        EQFBWriteHistLog2( szFolder, pszDocumentName, DOCDELETE_LOGTASK,
                           0, NULL, fMsg, NULLHANDLE, szLongName );
      }
    }/*endif fOK*/
  }/*endif fOK*/

  return fOK;
}/*end DocumentDelete*/

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocImportDlg   Dialog procedure for the list import dlg  |
//+----------------------------------------------------------------------------+
//|Function call:     DocImportDlg ( HWND hwnd, USHORT msg, MPARAM mp1,        |
//|                                  MPARAM mp2 )                              |
//+----------------------------------------------------------------------------+
//|Description:      Dialog procedure for the import of documents dialog.      |
//+----------------------------------------------------------------------------+
//|Input parameter:  HWND    hwnd     handle of window                         |
//|                  USHORT  msg      type of message                          |
//|                  MPARAM  mp1      first message parameter                  |
//|                  MPARAM  mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:      depends on message type                                   |
//|                  normal return codes are:                                  |
//|                  TRUE  = message has been processed                        |
//|                  FALSE = message has not been processed                    |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK FIMPODLGPROPPROC
(
HWND   hwnd,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT     mResult = FALSE;
  PDOCIMPIDA  pIda = NULL;                       // pointer to document import IDA
  PFNWP pfnWp;
  CHAR      szBuffer[80];
  TC_ITEM   TabCtrlItem;
  //UCD_ONE_PROPERTY
  SHORT       sNotification = WMCOMMANDCMD( mp1, mp2 );               // notification
  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FILIMP_DLG, mp2 ); break;

    case WM_INITDLG :             //initialize and display dialogbox
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        //CHAR      szBuffer[80];
        //TC_ITEM   TabCtrlItem;
        RECT      rect;
        HINSTANCE hInst       = GETINSTANCE( hwnd );
        HWND      hwndTabCtrl = GetDlgItem( hwnd, ID_DOCIMP_PROP_TABCTRL );
        HWND      hwndDlg;
        USHORT    nItem = 0;
        SHORT     sItem = 0;

        GetClientRect( hwndTabCtrl, &rect );
        TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

        // leave some additional space at top
        rect.top += 20;
        MapWindowPoints( hwndTabCtrl, hwnd, (POINT *) &rect, 2 );


        TabCtrlItem.mask = TCIF_TEXT;
        /******************************************************************/
        /* create the appropriate TAB control and load the associated     */
        /* dialog                                                         */
        /******************************************************************/
        LOADSTRING( hab, hResMod, SID_DOCIMP_EXTFORMAT, szBuffer );
        TabCtrlItem.pszText = szBuffer;
        SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
        nItem++;
        LOADSTRING( hab, hResMod, SID_DOCIMP_IMPPATHFORMAT, szBuffer );
        TabCtrlItem.pszText = szBuffer;
        SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
        nItem++;
        LOADSTRING( hab, hResMod, SID_DOCIMP_INTFORMAT, szBuffer );
        TabCtrlItem.pszText = szBuffer;
        SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
        nItem++;

        hwndDlg = CreateDialogParam( hInst, MAKEINTRESOURCE( ID_DOCIMP_DLG ), hwnd, FIMPODLGPROC, (LPARAM)pIda );
        pIda = ACCESSDLGIDA( hwndDlg, PDOCIMPIDA );
        ANCHORDLGIDA( hwnd, pIda );
        pIda->hwndPropertySheet = hwnd;  // anchor property sheet handle

        SetWindowPos( hwndDlg, HWND_TOP, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );

        UtlRegisterModelessDlg( hwndDlg );


        //Fill Import Format CB

        CBDELETEALL(hwnd, ID_DOCIMP_FORMAT_CB);

        LOADSTRING (hab, hResMod, SID_DOCEXP_EXTFORMAT, pIda->szBuffer);
        sItem = CBINSERTITEMEND( hwnd, ID_DOCIMP_FORMAT_CB,pIda->szBuffer);
        CBSETITEMHANDLE( hwnd, ID_DOCIMP_FORMAT_CB, sItem, ID_DOCIMP_EXTERNAL_RB );
        CBSELECTITEM (hwnd, ID_DOCIMP_FORMAT_CB, sItem ); // select first item as default

        LOADSTRING (hab, hResMod, SID_DOCEXP_IMPPATHFORMAT, pIda->szBuffer);
        sItem = CBINSERTITEMEND( hwnd, ID_DOCIMP_FORMAT_CB,pIda->szBuffer);
        CBSETITEMHANDLE( hwnd, ID_DOCIMP_FORMAT_CB, sItem, ID_DOCIMP_IMPPATH_RB );
        if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB ) CBSELECTITEM (hwnd, ID_DOCIMP_FORMAT_CB, sItem );

        LOADSTRING (hab, hResMod, SID_DOCEXP_INTFORMAT, pIda->szBuffer);
        sItem = CBINSERTITEMEND( hwnd, ID_DOCIMP_FORMAT_CB,pIda->szBuffer);
        CBSETITEMHANDLE( hwnd, ID_DOCIMP_FORMAT_CB, sItem, ID_DOCIMP_INTERNAL_RB );
        if ( pIda->usFormatID == ID_DOCIMP_INTERNAL_RB ) CBSELECTITEM (hwnd, ID_DOCIMP_FORMAT_CB, sItem );

        //
        // Select active page, suppress all other pages
        //
        //UCD_ONE_PROPERTY

        TabCtrl_DeleteAllItems(hwndTabCtrl );
      }
      break;

    case WM_COMMAND :  //UCD_ONE_PROPERTY
      pIda = ACCESSDLGIDA(hwnd, PDOCIMPIDA );
      if (WMCOMMANDID( mp1, mp2) == ID_DOCIMP_FORMAT_CB)
      {
        HWND      hwndTabCtrl = GetDlgItem( hwnd, ID_DOCIMP_PROP_TABCTRL );
        RECT      rect;
        GetClientRect( hwndTabCtrl, &rect );

        if (sNotification == CBN_SELCHANGE)
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          SHORT  sItem = CBQUERYSELECTION (hwnd, ID_DOCIMP_FORMAT_CB);
          USHORT usFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, ID_DOCIMP_FORMAT_CB, sItem );

          // stop thread only if non-path formats are selected
          if (usFormat != ID_DOCIMP_IMPPATH_RB)
          {
            DocImpStopThread( pIda );           //gs stop any running thread
          } /* endif */
          TabCtrl_DeleteAllItems(hwndTabCtrl );
          // set the tab to full width
          {
            RECT rect;

            GetClientRect( hwndTabCtrl, &rect );
            TabCtrl_SetMinTabWidth(  hwndTabCtrl, rect.right - rect.left -4);
          }


          TabCtrlItem.mask = TCIF_TEXT;

          if (usFormat == ID_DOCIMP_EXTERNAL_RB)
          {

            LOADSTRING( hab, hResMod, SID_DOCEXP_EXTFORMAT, szBuffer );
            TabCtrlItem.pszText = szBuffer;
            SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&TabCtrlItem);
            DocImpControl( pIda->hwndDlg, ID_DOCIMP_EXTERNAL_RB, BN_CLICKED );
            pIda->usFormatID = ID_DOCIMP_EXTERNAL_RB;
            ShowWindow( pIda->hwndDlg, SW_SHOW );
            ShowWindow( pIda->ExtraPages[0].hwndDlg, SW_HIDE );



          }
          else if (usFormat == ID_DOCIMP_IMPPATH_RB)
          {

            LOADSTRING( hab, hResMod, SID_DOCEXP_IMPPATHFORMAT, szBuffer );
            TabCtrlItem.pszText = szBuffer;
            SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&TabCtrlItem);
            DocImpControl( pIda->hwndDlg, ID_DOCIMP_IMPPATH_RB, BN_CLICKED );
            pIda->usFormatID = ID_DOCIMP_IMPPATH_RB;
            ShowWindow( pIda->hwndDlg, SW_SHOW );
            ShowWindow( pIda->ExtraPages[0].hwndDlg, SW_HIDE );

          }
          else if (usFormat == ID_DOCIMP_INTERNAL_RB)
          {
            LOADSTRING( hab, hResMod, SID_DOCEXP_INTFORMAT, szBuffer );
            TabCtrlItem.pszText = szBuffer;
            SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&TabCtrlItem);
            DocImpControl( pIda->hwndDlg, ID_DOCIMP_INTERNAL_RB, BN_CLICKED );
            pIda->usFormatID = ID_DOCIMP_INTERNAL_RB;
            ShowWindow( pIda->hwndDlg, SW_SHOW );
            ShowWindow( pIda->ExtraPages[0].hwndDlg, SW_HIDE );
          } // end if
          TabCtrl_SetCurSel( hwndTabCtrl, 0 );
          //    TabCtrl_HighlightItem(hwndTabCtrl, 0,2);
        }// end if
      }
      else
      {
        //ulTabCtrl = TabCtrl_GetCurSel(GetDlgItem( hwnd, ID_DOCIMP_PROP_TABCTRL ));
        //UCD_ONE_PROPERTY
        switch ( pIda->usFormatID )
        {
          case ID_DOCIMP_INTERNAL_RB:  // default ...
          case ID_DOCIMP_IMPPATH_RB:
          case ID_DOCIMP_EXTERNAL_RB:
            mResult = DocImpCommand( pIda->hwndDlg, WMCOMMANDID( mp1, mp2 ),
                                     WMCOMMANDCMD( mp1, mp2 ) );
            break;
          default:
            switch ( WMCOMMANDID( mp1, mp2) )
            {
			  case PID_PB_HELP:
				  UtlInvokeHelp();
				  break;
              case PID_PB_CANCEL :             // CANCEL button selected
              case DID_CANCEL :                 // ESCape key pressed

                DocImpStopThread( pIda );           //gs stop any running thread
                if ( pIda->fImporting )
                {
                  if ( UtlError( ERROR_DOCIMP_CANCEL,
                                 MB_YESNO | MB_DEFBUTTON2, 0,
                                 NULL, EQF_QUERY ) == MBID_YES )
                  {
                    pIda->fImporting = FALSE;
                  } /* endif */
                }
                else
                {
                  WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
                } /* endif */
                break;

              default:   //gs
                break;

            } /* endswitch */
            break;
        } /* endswitch */
      } // end if  UCD_ONE_PROPERTY

      break;

    case DM_GETDEFID:
      pIda = ACCESSDLGIDA(hwnd, PDOCIMPIDA );
      /***************************************************************/
      /* we could not pass on the message because MS/MFC framework   */
      /* destroys our return value -- so call sequence directly ...  */
      /***************************************************************/
      if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
      {
        mResult = DocImpDMGETDEFID( pIda->hwndDlg, mp1, mp2 );
      }
      else
      {
        mResult = UtlDMGETDEFID1( pIda->hwndDlg, mp1, mp2, &pIda->Controls,pIda->usFormatID );
      } /* endif */
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblPropSettingsDlg[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_EQF_CLOSE:     //UCD_ONE_PROPERTY todo
      //--- get rid off dialog ---
      pIda = ACCESSDLGIDA(hwnd, PDOCIMPIDA );

      if ( pIda )
      {
        USHORT nItem = EXTRAPAGE_START;

        DocImpStopThread( pIda );           //gs stop any running thread
        while ( pIda->ExtraPages[ nItem - EXTRAPAGE_START ].hwndDlg )
        {
          UtlUnregisterModelessDlg(pIda->ExtraPages[ nItem - EXTRAPAGE_START ].hwndDlg );
          DestroyWindow( pIda->ExtraPages[ nItem - EXTRAPAGE_START ].hwndDlg );
          FreeLibrary( pIda->ExtraPages[ nItem - EXTRAPAGE_START ].hmodSegmentDll );
          nItem++;
        } /* endwhile */
        UtlUnregisterModelessDlg(pIda->hwndDlg );
      } /* endif */
      DISMISSDLG( hwnd, mp1 );
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

} /* end of function FIMPODLGPROPPROC */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocImportDlg   Dialog procedure for the list import dlg  |
//+----------------------------------------------------------------------------+
//|Function call:     DocImportDlg ( HWND hwnd, USHORT msg, MPARAM mp1,        |
//|                                  MPARAM mp2 )                              |
//+----------------------------------------------------------------------------+
//|Description:      Dialog procedure for the import of documents dialog.      |
//+----------------------------------------------------------------------------+
//|Input parameter:  HWND    hwnd     handle of window                         |
//|                  USHORT  msg      type of message                          |
//|                  MPARAM  mp1      first message parameter                  |
//|                  MPARAM  mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:      depends on message type                                   |
//|                  normal return codes are:                                  |
//|                  TRUE  = message has been processed                        |
//|                  FALSE = message has not been processed                    |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK FIMPODLGPROC
(
HWND   hwnd,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT     mResult = FALSE;
  PDOCIMPIDA  pIda;                    // pointer to document import IDA
  EQFINFO     ErrorInfo;               // error info returned by property functions

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FILIMP_DLG, mp2 ); break;

    case WM_INITDLG :             //initialize and display dialogbox
      mResult = DocImpInit( hwnd, (PDOCIMPIDA) mp2 );
      break;

    case WM_COMMAND :
      mResult = DocImpCommand( hwnd, WMCOMMANDID( mp1, mp2 ),
                               WMCOMMANDCMD( mp1, mp2 ) );
      break;

    case WM_EQF_CLOSE :
      pIda = ACCESSDLGIDA( hwnd, PDOCIMPIDA );
      if ( pIda != NULL )
      {
        if ( pIda->fImporting )
        {
          if ( UtlError( ERROR_DOCIMP_CANCEL,
                         MB_YESNO | MB_DEFBUTTON2, 0,
                         NULL, EQF_QUERY ) == MBID_YES )
          {
            pIda->fImporting = FALSE;
            pIda->fBeingClosed = TRUE;
          } /* endif */
        }
        else
        {
          DelCtrlFont(hwnd, ID_DOCIMP_PATH_EF );
          DelCtrlFont(hwnd, ID_DOCIMP_DIR_TEXT );
          DelCtrlFont(hwnd, ID_DOCIMP_DIR_LB );
          if ( pIda->hwndPropertySheet )
          {
            POSTEQFCLOSE( pIda->hwndPropertySheet, SHORT1FROMMP1( mp1 ));
          } /* endif */
          DISMISSDLG( hwnd, SHORT1FROMMP1( mp1 ) );
        } /* endif */
      }
      else
      {
        if ( pIda->hwndPropertySheet )
        {
          POSTEQFCLOSE( pIda->hwndPropertySheet, SHORT1FROMMP1( mp1 ));
        } /* endif */
        DISMISSDLG( hwnd, SHORT1FROMMP1( mp1 ) );
      } /* endif */
      break;

    case DM_GETDEFID:
      pIda = ACCESSDLGIDA( hwnd, PDOCIMPIDA );
      if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
      {
        mResult = DocImpDMGETDEFID( hwnd, mp1, mp2 );
      }
      else
      {
        mResult = UtlDMGETDEFID1( hwnd, mp1, mp2, &pIda->Controls, pIda->usFormatID );
      } /* endif */
      break;

    case WM_DESTROY :
      pIda = ACCESSDLGIDA( hwnd, PDOCIMPIDA );
      if ( pIda != NULL )
      {
        // destroy listbox for short document names
        if ( pIda->hwndShortNameLB != NULLHANDLE )
        {
          WinDestroyWindow( pIda->hwndShortNameLB );
        } /* endif */
        if ( pIda->hwndFolderListBox != NULLHANDLE )
        {
          WinDestroyWindow( pIda->hwndFolderListBox );
        } /* endif */
        // close import/export properties and cleanup storage
        CloseProperties( pIda->hProp, 0, &ErrorInfo);
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
      } /* endif */
      break;

    default :
      //return to default dialog procedure
      mResult = UTLDEFDIALOGPROC( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */
  return( mResult );

} /* end of function FIMPODLGPROC */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocImpInit                                               |
//+----------------------------------------------------------------------------+
//|Function call:     DocImpInit( HWND hwnd );                                 |
//+----------------------------------------------------------------------------+
//|Description:       Initialization code for the list import dialog.          |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd      Dialog window handle                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE  (always)                                          |
//+----------------------------------------------------------------------------+
//|Side effects:      The dialog is removed using WInDismissDlg if severe      |
//|                   errors occur during initialization                       |
//+----------------------------------------------------------------------------+
static MRESULT DocImpInit
(
HWND  hwnd,                          // handle of dialog window
PDOCIMPIDA pIda                      // pointer to list import IDA
)
{
  EQFINFO    ErrorInfo;                // return code of property handler
  PPROPIMEX  pProp = NULL;             // pointer to import/export properties
  PSZ        pszError;                 // pointer to error message parameter
  BOOL       fOK = TRUE;               // internal OK flag

  /*******************************************************************/
  /* Allocate and anchor IDA                                         */
  /*******************************************************************/
  if ( !pIda )
  {
    fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(DOCIMPIDA), ERROR_STORAGE );
  } /* endif */

  if ( fOK )
  {
    pIda->fBeingClosed = FALSE;
    fOK = ANCHORDLGIDA( hwnd, pIda );
    if ( !fOK )
    {
      // anchor of IDA failed ==> display error message
      UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Setup path to active folder                                     */
  /*******************************************************************/
  if ( fOK )
  {
    EqfQueryObjectName( EqfQueryActiveFolderHwnd(), pIda->szTargetFolder );
    strcpy( pIda->szParentObjName, pIda->szTargetFolder );
    if ( FolIsSubFolderObject( pIda->szTargetFolder ) )
    {
      // get main folder object name to allow the remaining code to work
      // without changes
      pIda->ulParentID = FolGetSubFolderIdFromObjName( pIda->szTargetFolder );
      UtlSplitFnameFromPath( pIda->szTargetFolder ); // cut off subfolder name
      UtlSplitFnameFromPath( pIda->szTargetFolder ); // cut off property directory
    }
    else
    {
      pIda->ulParentID = 0L;
    } /* endif */
    strcpy( pIda->szToFolder, UtlGetFnameFromPath( pIda->szTargetFolder ) );
    pIda->usDocsInFolder = usGetNumberOfDocsInFolder(pIda->szTargetFolder); /*KIT0925A*/
    pIda->hwndDlg = hwnd;
  } /* endif */

  /********************************************************************/
  /* Open imex properties                                             */
  /********************************************************************/
  if ( fOK )
  {
    UtlMakeEQFPath( pIda->szDummy, NULC, SYSTEM_PATH, NULL );
    sprintf(pIda->szObjName,"%s\\%s", pIda->szDummy, IMEX_PROPERTIES_NAME);

    pIda->hProp = OpenProperties( pIda->szObjName, NULL,
                                  PROP_ACCESS_READ, &ErrorInfo);
    if ( !pIda->hProp )
    {                                     //error from propery handler
      pszError   = pIda->szObjName;
      UtlError( ERROR_PROPERTY_ACCESS,
                MB_CANCEL,
                1,
                &pszError,
                EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Setup controls IDA needed for dialog control utility             */
  /********************************************************************/
  if ( fOK )
  {
    pIda->Controls.idFilesLB            = ID_DOCIMP_FILES_LB;
    pIda->Controls.idDirLB              = ID_DOCIMP_DIR_LB;
    pIda->Controls.idPathEF             = ID_DOCIMP_PATH_EF;
//gs    pIda->Controls.idCurrentDirectoryEF = ID_DOCIMP_NEWCURDIR_TEXT;
    pIda->Controls.idCurrentDirectoryEF = ID_DOCIMP_STARTPATH_CB;
//gq    pIda->Controls.idToLB               = ID_DOCIMP_FOLDER_TEXT;
    pIda->Controls.idInternalRB         = ID_DOCIMP_INTERNAL_RB;
    pIda->Controls.idExternalRB         = ID_DOCIMP_EXTERNAL_RB;
//WL    pIda->Controls.idDriveBTN           = ID_DOCIMP_DUMMY_PB;
    pIda->Controls.idControlsGB         = ID_DOCIMP_IMPORT_GB;
    pIda->Controls.idOkPB               = PID_PB_OK;
    pIda->Controls.fMultiSelectionLB    = TRUE;
    pIda->Controls.fLongFileNames       = TRUE;
    strcpy( pIda->Controls.szHandler, EMPTY_STRING );
    pIda->Controls.fImport = TRUE;
  } /* endif */

  /********************************************************************/
  /* Load last used values from properties into controls IDA          */
  /********************************************************************/
  if ( fOK )
  {
    pProp = (PPROPIMEX)MakePropPtrFromHnd( pIda->hProp);

    // use last used values from folder properties if available
    {
      PPROPFOLDER pFolProp;            // ptr to folder properties
      PVOID       hFolProp;            // handle of folder properties
      OBJNAME     szFolObjName;        // buffer for folder object name
      CHAR        szTmgrDrive[3];      // buffer for Tmgr primary drive
      ULONG       ErrorInfo = 0;

      // setup folder object name
      UtlQueryString( QST_PRIMARYDRIVE, szTmgrDrive, sizeof(szTmgrDrive) );
      strcpy( szFolObjName, pIda->szTargetFolder );
      szFolObjName[0] = szTmgrDrive[0];

      hFolProp = OpenProperties( szFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
      if ( hFolProp )
      {
        pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
        if ( pFolProp->fDocImpLastUsed  )
        {
          strcpy( pProp->szSavedDlgLoadPath, pFolProp->szSavedDlgLoadPath  );
          strcpy( pProp->szSavedDlgLoadDrive, pFolProp->szSavedDlgLoadDrive );
          strcpy( pProp->szSavedStartPath, pFolProp->szSavedStartPath );
          pProp->sSavedDocImpDlgMode  = pFolProp->sSavedDocImpDlgMode;
        } /* endif */
        CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
    }

    // avoid traps if saved path overflowed our last used value fields (which were a bit too small...)
    if ( strlen( pProp->szSavedDlgLoadPath ) >= sizeof(pProp->szSavedDlgLoadPath) )
    {
      // reset path and drive to default values
      pProp->szSavedDlgLoadPath[0] = EOS;
      UtlQueryString( QST_PRIMARYDRIVE, pProp->szSavedDlgLoadDrive, sizeof(pProp->szSavedDlgLoadDrive) );
    } /* endif */

    // check that a szSavedDlgLoadDrive really contains a drive letter, if not use system drive as default
    if ( !isalpha(pProp->szSavedDlgLoadDrive[0]) || 
         (pProp->szSavedDlgLoadDrive[1] != ':')  || 
         (pProp->szSavedDlgLoadDrive[2] != EOS) )
    {
      // reset path and drive to default values
      pProp->szSavedDlgLoadPath[0] = EOS;
      UtlQueryString( QST_PRIMARYDRIVE, pProp->szSavedDlgLoadDrive, sizeof(pProp->szSavedDlgLoadDrive) );
    } /* endif */

    pIda->Controls.chSavedDrive = pProp->szSavedDlgLoadDrive[0];
    if ( ( pProp->szSavedDlgLoadPath[0] != BACKSLASH ) &&
         ( pProp->szSavedDlgLoadPath[0] != EOS ) )
    {
      strcpy( pIda->Controls.szSavedPath, pProp->szSavedDlgLoadPath );
    }
    else
    {
      sprintf( pIda->Controls.szSavedPath, "%c:\\",
               pProp->szSavedDlgLoadDrive[0] );
    } /* endif */
    pIda->Controls.usSavedFormat = 0;
    SubFolObjectNameToName( pIda->szParentObjName, pIda->Controls.szSelectedName );
    SETTEXT( hwnd, ID_DOCIMP_FOLDER_TEXT, pIda->Controls.szSelectedName );

    UtlLoadLastUsedStrings( hwnd, ID_DOCIMP_STARTPATH_CB, DOCIMPSTARTPATH );

    strcpy( pIda->szStartPath, pProp->szSavedStartPath );
    {
      //UCD_ONE_PROPERTY
      //
      // get Import format
      //
      char szBuffer[1000];
      pIda->usFormatID  = pProp->sSavedDocImpDlgMode;
      //gs

      if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB)
      {
        strcpy(szBuffer,  pProp->szSavedDlgLoadDrive);
        strcat(szBuffer, pIda->szStartPath  );
      }
      else if (pIda->usFormatID == ID_DOCIMP_EXTERNAL_RB)
      {
        strcpy(szBuffer,  pProp->szSavedDlgLoadDrive);
        strcat(szBuffer, pIda->szStartPath  );
      }
      else if (pIda->usFormatID == ID_DOCIMP_INTERNAL_RB)
      {
        strcpy(szBuffer,  pProp->szSavedDlgLoadDrive);
      } // end if
      SETTEXT( hwnd, ID_DOCIMP_STARTPATH_CB, szBuffer );
    }
  } /* endif */

  /********************************************************************/
  /* Create listbox for short document names when importing           */
  /* in internal format                                               */
  /********************************************************************/
  if ( fOK )
  {
    pIda->hwndShortNameLB = WinCreateWindow( hwnd, WC_LISTBOX, "", 0,
                                             0, 0, 0, 0,
                                             hwnd, HWND_TOP, 1, NULL, NULL );
    pIda->hwndFolderListBox = WinCreateWindow( hwnd, WC_LISTBOX, "", 0,
                                             0, 0, 0, 0,
                                             hwnd, HWND_TOP, 1, NULL, NULL );
  } /* endif */

  /********************************************************************/
  /* Call up utility to initialize dialog controls                    */
  /********************************************************************/
  if ( fOK )
  {
    SetCtrlFnt(hwnd, GetCharSet(),ID_DOCIMP_STARTPATH_CB, 0 );
    SetCtrlFnt(hwnd, GetCharSet(),ID_DOCIMP_PATH_EF, ID_DOCIMP_NEWCURDIR_TEXT );
    SetCtrlFnt(hwnd, GetCharSet(),ID_DOCIMP_DIR_TEXT, ID_DOCIMP_FILES_TEXT );
    SetCtrlFnt(hwnd, GetCharSet(),ID_DOCIMP_DIR_TEXT, ID_DOCIMP_FILESIMPPATH_TEXT );
    SetCtrlFnt(hwnd, GetCharSet(),ID_DOCIMP_DIR_LB, ID_DOCIMP_FILES_LB );
    UtlControlsInit( hwnd, &pIda->Controls );
    SETTEXTLIMIT( hwnd, ID_DOCIMP_STARTPATH_CB, MAX_LONGPATH - 1 );
  } /* endif */

  /*******************************************************************/
  /* Remove dialog if initialization failed or set focus to          */
  /* currently active format radio button and set radiobutton        */
  /* selection state                                                  */
  /*******************************************************************/
  if ( !fOK )
  {
    DISMISSDLG( hwnd, FALSE );
  }
  else
  {
    UtlCheckDlgPos( hwnd, FALSE );
    pIda->fInitInProgress = TRUE;

    // handling for older LUs, where fSavedDlgFImpoInternal
    // (now called sSavedDocImpDlgMode) was only TRUE or FALSE
    if ( pProp->sSavedDocImpDlgMode == TRUE )
    {
      pProp->sSavedDocImpDlgMode = ID_DOCIMP_INTERNAL_RB;
    }
    else if ( pProp->sSavedDocImpDlgMode == FALSE )
    {
      pProp->sSavedDocImpDlgMode = ID_DOCIMP_EXTERNAL_RB;
    } /* endif */

    switch ( pProp->sSavedDocImpDlgMode )
    {
      case ID_DOCIMP_EXTERNAL_RB:
        PrepExtFormDlg( pIda, hwnd, ID_DOCIMP_EXTERNAL_RB );
        break;

     case ID_DOCIMP_IMPPATH_RB:
        PrepImpPathFormDlg( pIda, hwnd, ID_DOCIMP_IMPPATH_RB );
        break;

      case ID_DOCIMP_INTERNAL_RB:
      default:
         PrepIntFormDlg( pIda, hwnd, ID_DOCIMP_INTERNAL_RB );
         break;
    } /* endswitch */
    pIda->fInitInProgress = FALSE;
  } /* endif */

  return( MRFROMSHORT(TRUE) );
} /* end of function DocImpInit */

//+----------------------------------------------------------------------------+
//|Function name:     DocImpCommand                                            |
//+----------------------------------------------------------------------------+
//|Function call:     DocImpCommand( HWND hwnd, MPARAM mp2, MPARAM mp2 );      |
//+----------------------------------------------------------------------------+
//|Description:       Process the WM_COMMAND message of the doc import dialog  |
//|                   and do the actual importing of the document.             |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd      Dialog window handle                    |
//|                   MPARAM mp1       First message parameter of WM_INITDLG   |
//|                                    message: dialog window handle           |
//|                   MPARAM mp2       Second message parameter of WM_INITDLG  |
//|                                    message: ptr to object name of list     |
//|                                    being imported                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE  (always)                                          |
//+----------------------------------------------------------------------------+
//|Side effects:      The list is imported if 'import' has been pressed. The   |
//|                   dialog is ended on Cancel or successful import.          |
//+----------------------------------------------------------------------------+
//|Function flow:     switch control ID                                        |
//|                     case CANCEL button:                                    |
//|                     case escape key:                                       |
//|                       post a WM_EQF_CLOSE message to the dialog                |
//|                     case import button:                                    |
//|                       address IDA                                          |
//|                       if not in MAT format import then                     |
//|                         get and check specified file name                  |
//|                       endif                                                |
//|                       if ok and not in MAT format import                   |
//|                         build and check name of import file                |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         check if system drive has enough free space to     |
//|                          receive the imported list                         |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         call DocImportList to do the actual import         |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         close the dialog and save last used values         |
//|                       endif                                                |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
MRESULT DocImpCommand
(
HWND  hwnd,                          // handle of dialog window
SHORT sId,                           // id of button
SHORT sNotification                  // notification type
)
{
  PDOCIMPIDA    pIda = NULL;          // Load dialog IDA
  USHORT        usRC;                 // Return code from Dos functions
  SHORT         sIndexItem;           // index of selected item in listb.
  MRESULT       mResult = (MRESULT)FALSE; // function return value
  BOOL          fOK = TRUE;           // success indicator
  PPROPIMEX     pProp = NULL;         // ptr to properties
  EQFINFO       ErrorInfo;            // error info from property handler
  BOOL          fSaveRc;              // save BOOL return code of funct.
  PPROPDOCUMENT ppropDoc;             // pointer to document properties
  HPROP         hpropDoc;             // handle of document properties
  PSZ           pszReplace;           // pointer to replace string UtlError
  SHORT         sRc;                  // rc for QuerySymbol... calls /*KIT1173A*/
  SHORT         sFocusID;             // ID of control receiving the focus
  pIda = ACCESSDLGIDA( hwnd, PDOCIMPIDA );

  switch ( sId )
  {
    case PID_PB_HELP:
      UtlInvokeHelp();
      break;

    case ID_DOCIMP_SELALL_PB:
      DocImpStopThread( pIda );           // stop any running thread
      if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
      {
        SELECTALL( hwnd, ID_DOCIMP_IMPPATH_LB );
      }
      else
      {
        SELECTALL( hwnd, ID_DOCIMP_FILES_LB );
      }
      break;

    case ID_DOCIMP_DESALL_PB:
      DocImpStopThread( pIda );           // stop any running thread
      if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
      {
        DESELECTALL( hwnd, ID_DOCIMP_IMPPATH_LB );
      }
      else
      {
        DESELECTALL( hwnd, ID_DOCIMP_FILES_LB );
      }
      break;

    case PID_PB_CANCEL :             // CANCEL button selected
    case DID_CANCEL :                 // ESCape key pressed
      DocImpStopThread( pIda );           // stop any running thread
      if ( pIda->fImporting )
      {
        if ( UtlError( ERROR_DOCIMP_CANCEL,
                       MB_YESNO | MB_DEFBUTTON2, 0,
                       NULL, EQF_QUERY ) == MBID_YES )
        {
          pIda->fImporting = FALSE;
        } /* endif */
      }
      else
      {
        WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
      } /* endif */
      break;

    case PID_PB_OK:
      sFocusID = 0;                   // no focus control yet
      DocImpStopThread( pIda );           // stop any running thread
      // disable controls
      EnableImportControls (hwnd, FALSE);

      if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
      {
        //get selected item count
        pIda->usSelDocs = 0;
        sIndexItem = QUERYNEXTSELECTION( hwnd, ID_DOCIMP_IMPPATH_LB, LIT_FIRST );
        while ( sIndexItem >= 0 )
        {
          pIda->usSelDocs++;
          sIndexItem = QUERYNEXTSELECTION( hwnd, ID_DOCIMP_IMPPATH_LB,
                                           sIndexItem );
        } /* endwhile */

        //query if a item in Files listbox is selected
        pIda->fYesToAll = FALSE;
        sIndexItem = QUERYNEXTSELECTION( hwnd, ID_DOCIMP_IMPPATH_LB, LIT_FIRST );
      }
      else
      {
        //get selected item count
        pIda->usSelDocs = 0;
        sIndexItem = QUERYNEXTSELECTION( hwnd, ID_DOCIMP_FILES_LB, LIT_FIRST );
        while ( sIndexItem >= 0 )
        {
          pIda->usSelDocs++;
          sIndexItem = QUERYNEXTSELECTION( hwnd, ID_DOCIMP_FILES_LB,
                                           sIndexItem );
        } /* endwhile */

        //query if a item in Files listbox is selected
        pIda->fYesToAll = FALSE;
        sIndexItem = QUERYNEXTSELECTION( hwnd, ID_DOCIMP_FILES_LB, LIT_FIRST );
      }

      if ( sIndexItem != LIT_NONE )       // an item selected?
      {
        //initialize fSaveRc and usRC
        fSaveRc = TRUE;
        usRC = LOAD_OK;
        pIda->fImporting = TRUE;     // we are importing files ...

        while (( sIndexItem != LIT_NONE ) && ( usRC != LOAD_CANCEL )
               && (usRC != LOAD_CLOSED) )
        {
          // Ensure that document currently processed is visible in listbox and
          // get name of selected file
          if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
          {
            SETTOPINDEX( hwnd, ID_DOCIMP_IMPPATH_LB, sIndexItem );
            QUERYITEMTEXT(hwnd, ID_DOCIMP_IMPPATH_LB,sIndexItem,pIda->szString);
          }
          else
          {
            SETTOPINDEX( hwnd, ID_DOCIMP_FILES_LB, sIndexItem );
            QUERYITEMTEXT(hwnd, ID_DOCIMP_FILES_LB,sIndexItem,pIda->szString);
          }
          // check if selected name is a long file name
          pIda->stFs.fIsLongName = UtlIsLongFileName( pIda->szString );

          // save long file name
          strcpy( pIda->stFs.szLongName, pIda->szString );

          if ( (pIda->usFormatID == ID_DOCIMP_EXTERNAL_RB) ||
               (pIda->usFormatID == ID_DOCIMP_IMPPATH_RB) )
          {
            BOOL fIsNew;

            FolLongToShortDocName( pIda->szTargetFolder,
                                   pIda->stFs.szLongName,
                                   pIda->szString,
                                   &fIsNew );

            // handle long or short name
            if ( !pIda->stFs.fIsLongName )
            {
              if ( fIsNew )
              {
                // restore original short name
                strcpy( pIda->szString, pIda->stFs.szLongName );
              }
              else if (stricmp( pIda->stFs.szLongName, pIda->szString ) != 0 )
              {
                // existing name is from a long file name ...
                pIda->stFs.fIsLongName = TRUE;
              } /* endif */
            } /* endif */
          }
          else
          {
            // get document short name from our invisible short name listbox
            SHORT sShortIndex = (SHORT)QUERYITEMHANDLE( hwnd,
                                                        ID_DOCIMP_FILES_LB,
                                                        sIndexItem );
            QUERYITEMTEXTHWND( pIda->hwndShortNameLB, sShortIndex,
                               pIda->szString);
          } /* endif */

          // save last used values
          UtlSaveLastUsedString( hwnd, ID_DOCIMP_STARTPATH_CB, DOCIMPSTARTPATH, 10 );

          //save filename and extension to IDA
          if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
          {
            // build complete document path
            strcpy( pIda->Controls.szPath, pIda->Controls.szDrive );
            if ( pIda->szStartPath[0] != BACKSLASH )
            {
              strcat( pIda->Controls.szPath, BACKSLASH_STR );
            } /* endif */
            strcat( pIda->Controls.szPath, pIda->szStartPath );
          }  /* endif */

          _splitpath( pIda->szString,
                      pIda->szDummy,
                      pIda->szDummy,
                      pIda->stFs.szName,
                      pIda->stFs.szExt );

		  strcpy( pIda->stFs.szDrive, pIda->Controls.szDrive );
		  if (pIda->stFs.szDrive[0] == '\\')  // we are dealing with a LAN drive
		  {
            strcpy( pIda->stFs.szPath, pIda->Controls.szPath );
			pIda->stFs.szDrive[0] = EOS;
		  }
		  else
		  {
            strcpy( pIda->stFs.szPath, pIda->Controls.szPath + 3 );
		  }

          /*********************************************************/
          /* build document object name                            */
          /*********************************************************/
          strcpy( pIda->szDocObjName, pIda->szTargetFolder);
          strcat( pIda->szDocObjName, BACKSLASH_STR );
          strcat( pIda->szDocObjName, pIda->szString );

          // check if object already exists, i.e. document is in use
          sRc = QUERYSYMBOL( pIda->szDocObjName );
          if ( sRc != -1 )
          {
            pszReplace = pIda->stFs.szLongName;
            UtlError( ERROR_DOC_LOCKED, MB_CANCEL,
                      1, &pszReplace, EQF_ERROR );
            usRC = LOAD_NOTOK;
            fOK = FALSE;
          }
          else
          {
            SETSYMBOL( pIda->szDocObjName );

            SETCURSOR( SPTR_WAIT );
            if ( (pIda->usFormatID == ID_DOCIMP_EXTERNAL_RB) ||
                 (pIda->usFormatID == ID_DOCIMP_IMPPATH_RB) )
            {
              usRC = DocumentLoad( pIda, TRUE );     // load file
              strcpy( pIda->szToDoc, pIda->szString );
            }
            else                               // import in internal format
            {
              usRC = DocImpInternal( pIda );  // load file
            }/*end if*/
            SETCURSOR( SPTR_ARROW );
            REMOVESYMBOL( pIda->szDocObjName );
          } /* endif */

          fSaveRc &= (usRC == LOAD_OK) ;    // set save rc

          switch ( usRC )
          {
            case LOAD_OK:
              // increase number of documents in folder
              pIda->usDocsInFolder++;

              //deselect item
              if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
              {
                DESELECTITEM( hwnd, ID_DOCIMP_IMPPATH_LB, sIndexItem );
              }
              else
              {
                DESELECTITEM( hwnd, ID_DOCIMP_FILES_LB, sIndexItem );
              }

              //create object name to update documentlist
              UtlMakeFullPath(pIda->szString, pIda->szTargetFolder,
                              BACKSLASH_STR, pIda->szToDoc, NULL );

              //inform folderhandler to update documentlist
              EqfSend2AllHandlers ( WM_EQFN_CREATED,
                                    MP1FROMSHORT( clsDOCUMENT ),
                                    MP2FROMP( pIda->szString ) );

              // build property file name
              UtlMakeFullPath (pIda->szProperties, (PSZ)NULP,
                               pIda->szTargetFolder,
                               pIda->szToDoc, NULL );

              //open document properties
              if ( ( hpropDoc = OpenProperties
                      ( pIda->szProperties, NULL,
                        PROP_ACCESS_READ, &ErrorInfo)) == NULL)
              {
                fOK = FALSE;
                pszReplace = pIda->szProperties;
                UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL,
                          1, &pszReplace, EQF_ERROR);
              }
              else
              {
                // set import timestamp
                usRC = (USHORT)SetPropAccess( hpropDoc, PROP_ACCESS_WRITE);
                ppropDoc = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDoc );
                UtlTime( (PLONG)&ppropDoc->ulImp );

                // set source document timestamp for external documents
                if ( (pIda->usFormatID == ID_DOCIMP_EXTERNAL_RB) ||
                     (pIda->usFormatID == ID_DOCIMP_IMPPATH_RB) )
                {
                  LONG lTime = 0L;
                  CHAR szFullFileName[MAX_PATH+1];

                  strcpy(szFullFileName, pIda->Controls.szPath);
                  if ( szFullFileName[strlen(szFullFileName)-1] != BACKSLASH )
                  {
                    strcat( szFullFileName, BACKSLASH_STR );
                  } /* endif */
                  strcat(szFullFileName, pIda->stFs.szLongName);

                  lTime = UtlFileTimeToLong(szFullFileName);

                  if (lTime)
                  {
                     ppropDoc->ulSrc = lTime;
                  }
                  else
                  {
                    ppropDoc->ulSrc = 0;
                  }
                } /* endif */

                // save and close document properties
                usRC = SaveProperties( hpropDoc, &ErrorInfo );
                ResetPropAccess( hpropDoc, PROP_ACCESS_WRITE);
                CloseProperties( hpropDoc, PROP_FILE, &ErrorInfo);
              }/*endif*/

              //query if a item in Files listbox is selected
              if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
              {
                sIndexItem = QUERYNEXTSELECTION( hwnd,
                                                 ID_DOCIMP_IMPPATH_LB, sIndexItem );
              }
              else
              {
                sIndexItem = QUERYNEXTSELECTION( hwnd,
                                                 ID_DOCIMP_FILES_LB, sIndexItem );
              }
              break;
            case LOAD_NOTOK:
              if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
              {
                sIndexItem = QUERYNEXTSELECTION( hwnd,
                                                 ID_DOCIMP_IMPPATH_LB, sIndexItem );
              }
              else
              {
                sIndexItem = QUERYNEXTSELECTION( hwnd,
                                                 ID_DOCIMP_FILES_LB, sIndexItem );
              }
              break;
            default:
              break;
          } /* endswitch */

          /*********************************************************/
          /* Dispatch messages before continuing with next file    */
          /*********************************************************/
          if ( (sIndexItem != LIT_NONE) && (usRC != LOAD_CANCEL) )
          {
            UtlDispatch();
            pIda = ACCESSDLGIDA( hwnd, PDOCIMPIDA );
            /*******************************************************/
            /* Check if dialog has been canceled                   */
            /*******************************************************/
            if ( (pIda == NULL) || !pIda->fImporting )
            {
              usRC = LOAD_CLOSED;
            } /* endif */
          } /* endif */
        } /*endwhile*/

        if ( pIda != NULL )
        {
          pIda->fImporting = FALSE;    // importing files has been stopped
        } /* endif */

        if ( (pIda != NULL) && fOK )
        {
          //if error from DocumentImport or -Load display message
          // display only if not canc.
          if ( usRC != LOAD_CANCEL)
          {
            if ( (!fSaveRc) || (usRC == LOAD_CLOSED) )
            {
              //display message that not all files successfull imported
              UtlError( NOT_ALL_IMPORTED, MB_OK, 0, NULL, EQF_WARNING );
            }
            else
            {
              //display message that all files successfull imported
              UtlError( ALL_IMPORTED, MB_OK, 0, NULL, EQF_INFO );

              //get write access to import/export properties
              if ( SetPropAccess( pIda->hProp, PROP_ACCESS_WRITE) )
              {
                //get pointer
                pProp = (PPROPIMEX)MakePropPtrFromHnd( pIda->hProp);

                //copy drive, properties, pattern name and extension
                strcpy( pProp->szSavedDlgLoadPath, pIda->Controls.szPath );
                strcpy( pProp->szSavedDlgLoadDrive, pIda->Controls.szDrive );
                strcpy( pProp->szSavedStartPath, pIda->szStartPath );

                if ( pIda->usFormatID == ID_DOCIMP_EXTERNAL_RB )
                {
                  pProp->sSavedDocImpDlgMode = ID_DOCIMP_EXTERNAL_RB;
                }
                else if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
                {
                  pProp->sSavedDocImpDlgMode = ID_DOCIMP_IMPPATH_RB;
                }
                else
                {
                  pProp->sSavedDocImpDlgMode = ID_DOCIMP_INTERNAL_RB;
                } /* endif */

                //save properties
                SaveProperties( pIda->hProp, &ErrorInfo );
                //reset access mode to save import/export properties
                ResetPropAccess( pIda->hProp, PROP_ACCESS_WRITE);
              }
              else
              {
                UtlError( ERROR_PROPERTY_ACCESS,
                          MB_CANCEL,
                          0, NULL,
                          EQF_ERROR );
              } /* endif */

              // save last used values in folder properties as well
              {
                PPROPFOLDER pFolProp;            // ptr to folder properties
                PVOID       hFolProp;            // handle of folder properties
                OBJNAME     szFolObjName;        // buffer for folder object name
                CHAR        szTmgrDrive[3];      // buffer for Tmgr primary drive
                ULONG       ErrorInfo = 0;

                // setup folder object name
                UtlQueryString( QST_PRIMARYDRIVE, szTmgrDrive, sizeof(szTmgrDrive) );
                strcpy( szFolObjName, pIda->szTargetFolder );
                szFolObjName[0] = szTmgrDrive[0];

                hFolProp = OpenProperties( szFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
                if ( hFolProp )
                {
                  if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE) )
                  {
                    pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
                    pFolProp->fDocImpLastUsed = TRUE;
                    strcpy( pFolProp->szSavedDlgLoadPath, pIda->Controls.szPath );
                    strcpy( pFolProp->szSavedDlgLoadDrive, pIda->Controls.szDrive );
                    strcpy( pFolProp->szSavedStartPath, pIda->szStartPath );
                    pFolProp->sSavedDocImpDlgMode  = pProp->sSavedDocImpDlgMode;
                    SaveProperties( hFolProp, &ErrorInfo );
                    ResetPropAccess( hFolProp, PROP_ACCESS_WRITE);
                  } /* endif */
                  CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
                } /* endif */
              }

              //close dialog
              WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
            } /* endif */
          } /* endif */

          if ( (usRC == LOAD_CLOSED) && pIda->fBeingClosed )
          {
            //close dialog
            WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
          } /* endif */
        } /* endif fOK */
      }
      else                          //no item is selected in Files listbox
      {
        //display error message that no file is selected
        UtlError( NO_FILE_SELECTED, MB_OK, 0, NULL, EQF_WARNING );
        sFocusID = ID_DOCIMP_FILES_LB;
      }/*end if*/

      if ( pIda != NULL )
      {
        pIda->fImporting = FALSE;  // importing files has been stopped
        // enable controls
        EnableImportControls (hwnd, TRUE);

        if ( sFocusID )
        {
          SETFOCUS( hwnd, sFocusID );
        } /* endif */
      } /* endif */
      break;

    case ID_DOCIMP_BROWSE_PB:
      {
        if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB)
        {
          DocImpStopThread( pIda );           // stop any running thread
        }//end if
        {
          BROWSEINFO bi;
          LPITEMIDLIST pidlBrowse;    // PIDL selected by user

          // Fill in the BROWSEINFO structure.
          bi.hwndOwner = hwnd;
          bi.pidlRoot = NULL;
          QUERYTEXT( hwnd, ID_DOCIMP_STARTPATH_CB, pIda->szString );
          bi.pszDisplayName = pIda->szString;
          if (pIda->usFormatID == ID_DOCIMP_EXTERNAL_RB)
          {
            bi.lpszTitle = "Select a path";
          }
          else if (pIda->usFormatID == ID_DOCIMP_IMPPATH_RB)
          {
            bi.lpszTitle = "Select a start path";
          }
          else if (pIda->usFormatID == ID_DOCIMP_INTERNAL_RB)
          {
            bi.lpszTitle = "Select drive";
          }

          bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

          // if UtlBrowseForFolderCallbackProc is specified as callback the lParam
          // parameter must contain the initial folder directory!!!
          bi.lpfn = UtlBrowseForFolderCallbackProc;
          bi.lParam = (LPARAM)pIda->szString;

          // Browse for a folder and return its PIDL.
          pidlBrowse = SHBrowseForFolder(&bi);
          if (pidlBrowse != NULL)
          {
            // get the selected directory path
            SHGetPathFromIDList( pidlBrowse, pIda->szString );
            if (pIda->szString[strlen(pIda->szString)-1] != '\\' )
            {
              strcat(pIda->szString,"\\");
            }
            // activate selected path
            if (pIda->usFormatID == ID_DOCIMP_INTERNAL_RB)
            {
              pIda->szString[2] = EOS;
            }

            SETTEXT( hwnd, ID_DOCIMP_STARTPATH_CB, pIda->szString );
            //gs
            if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB)
            {
              DocImpHandlePathInput( hwnd );
            }
            else if (pIda->usFormatID == ID_DOCIMP_EXTERNAL_RB)
            {
              DocExpHandlePathInput(hwnd);
              UtlEFValidityTest( &pIda->Controls, pIda->hwndDlg );
            }
            else if (pIda->usFormatID == ID_DOCIMP_INTERNAL_RB)
            {
              USHORT usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                            pIda->szString[0]);
              DocImpControl (hwnd,usDrive,BN_CLICKED);
            } // end if

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
        }
      }
      break;

      /**************************************************************/
      /* Pass notication messages to WM_CONTROL message handler     */
      /**************************************************************/
    case ID_DOCIMP_EXTERNAL_RB :
    case ID_DOCIMP_INTERNAL_RB :
    case ID_DOCIMP_DIR_LB:
    case ID_DOCIMP_FILES_LB:
    case ID_DOCIMP_IMPPATH_RB:
       mResult = DocImpControl( hwnd, sId, sNotification );
      break;

    case ID_DOCIMP_PATH_EF:
      if ( sNotification == EN_KILLFOCUS )
      {
        ClearIME( hwnd );
      } /* endif */
      break;

    case ID_DOCIMP_STARTPATH_CB:
      if ( sNotification == EN_KILLFOCUS )
      {
        ClearIME( hwnd );
      } /* endif */
      else if ( sNotification == CBN_SELCHANGE )
      { 
        int ipos;
        CBQUERYSELECTEDITEMTEXT ( ipos, hwnd, ID_DOCIMP_STARTPATH_CB, pIda->szStartPath  );
        SETTEXT( hwnd, ID_DOCIMP_STARTPATH_CB, pIda->szStartPath );
        DocExpHandlePathInput(hwnd);
        UtlEFValidityTest( &pIda->Controls, pIda->hwndDlg );
      } /* endif */
      break;
    default :
// WL
      if ( (sId >= PID_DRIVEBUTTON_A) && (sId <= PID_DRIVEBUTTON_Z) )
      {
        mResult = DocImpControl( hwnd, sId, sNotification );
      } /* endif */
      break;
  } /* endswitch */
  return mResult;
} /* end of function DocImpCommand */

/**********************************************************************/
/* Helper functions for dialog preparations                           */
/**********************************************************************/
static VOID
PrepExtFormDlg( PDOCIMPIDA pIda, HWND hwnd, SHORT sId )
{
  if ( (pIda->usFormatID != (USHORT)sId) || pIda->fInitInProgress )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DocImpStopThread( pIda );           // stop any running thread
    pIda->usFormatID = (USHORT)sId;
    SETTEXTFROMRES( hwnd, ID_DOCIMP_IMPORT_GB, pIda->szString,
                    hResMod, SID_DOCIMP_EXTIMPORT );

    SHOWCONTROL( hwnd, ID_DOCIMP_FILENAME_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_PATH_EF );
    SHOWCONTROL( hwnd, ID_DOCIMP_NEWCURDIR_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_CURDIR_TEXT );

    SHOWCONTROL( hwnd, ID_DOCIMP_FILES_LB );
    HIDECONTROL( hwnd, ID_DOCIMP_DIR_LB );
    HIDECONTROL( hwnd, ID_DOCIMP_DIR_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_FILES_TEXT );

    SHOWCONTROL( hwnd, ID_DOCIMP_BROWSE_PB );
    HIDECONTROL( hwnd, ID_DOCIMP_IMPPATH_LB );
    HIDECONTROL( hwnd, ID_DOCIMP_FILESIMPPATH_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_STARTPATH_CB );

    SETTEXTFROMRES( hwnd, ID_DOCIMP_FILES_TEXT, pIda->szString,
                    hResMod, SID_DOCIMP_INTFILE );
    SETTEXTFROMRES( hwnd, ID_DOCIMP_CURDIR_TEXT, pIda->szString,
                    hResMod, SID_DOCIMP_INTIMPORT );

    /********************************************************/
    /* Force a refresh of the listboxes                     */
    /********************************************************/
    {
      char szBuffer[1000];
      strcpy(szBuffer,  pIda->Controls.szDrive);
      //gs
      strcat(szBuffer, pIda->szStartPath  );
      SETTEXT( hwnd, ID_DOCIMP_STARTPATH_CB, szBuffer );
//      DocExpHandlePathInput(hwnd);
      UtlEFValidityTest( &pIda->Controls, pIda->hwndDlg );
    }// end of construction
  } /* endif */
}// end of function


static VOID
PrepImpPathFormDlg( PDOCIMPIDA pIda, HWND hwnd, SHORT sId )
{
  if ( (pIda->usFormatID != (USHORT)sId) || pIda->fInitInProgress )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    pIda->usFormatID = (USHORT)sId;
    DocImpStopThread( pIda );           // stop any running thread
    SETTEXTFROMRES( hwnd, ID_DOCIMP_IMPORT_GB, pIda->szString,
                    hResMod, SID_DOCIMP_EXTIMPORT );

    SHOWCONTROL( hwnd, ID_DOCIMP_FILENAME_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_PATH_EF );
    SHOWCONTROL( hwnd, ID_DOCIMP_CURDIR_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_STARTPATH_CB );
    SHOWCONTROL( hwnd, ID_DOCIMP_BROWSE_PB );
    SHOWCONTROL( hwnd, ID_DOCIMP_IMPPATH_LB );

    HIDECONTROL( hwnd, ID_DOCIMP_NEWCURDIR_TEXT );
    HIDECONTROL( hwnd, ID_DOCIMP_FILES_LB );
    HIDECONTROL( hwnd, ID_DOCIMP_DIR_LB );
    HIDECONTROL( hwnd, ID_DOCIMP_FILES_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_FILESIMPPATH_TEXT );
    HIDECONTROL( hwnd, ID_DOCIMP_DIR_TEXT );

    SETTEXTFROMRES( hwnd, ID_DOCIMP_FILESIMPPATH_TEXT, pIda->szString,
                    hResMod, SID_DOCIMP_EXTFILE );
    SETTEXTFROMRES( hwnd, ID_DOCIMP_CURDIR_TEXT, pIda->szString,
                    hResMod, SID_DOCIMP_STARTPATH );
    /********************************************************/
    /* Force a refresh of the listboxes                     */
    /********************************************************/
    {
      char szBuffer[1000];
      strcpy(szBuffer,  pIda->Controls.szDrive);
      strcat(szBuffer, pIda->szStartPath  );
      SETTEXT( hwnd, ID_DOCIMP_STARTPATH_CB, szBuffer );
      strcpy(pIda->szStartPath, szBuffer);
      DocImpHandlePathInput( hwnd );
    }// end of construction
  } /* endif */
}


static VOID PrepIntFormDlg( PDOCIMPIDA pIda, HWND hwnd, SHORT sId )
{
  if ( (pIda->usFormatID != (USHORT)sId) || pIda->fInitInProgress )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DocImpStopThread( pIda );           // stop any running thread
    pIda->usFormatID = (USHORT)sId;

    /********************************************************/
    /* Hide name, current directory, directory and file     */
    /* control                                              */
    /********************************************************/
    SETTEXTFROMRES( hwnd, ID_DOCIMP_IMPORT_GB, pIda->szString,
                    hResMod, SID_DOCIMP_INTIMPORT );
    HIDECONTROL( hwnd, ID_DOCIMP_FILENAME_TEXT );

    HIDECONTROL( hwnd, ID_DOCIMP_PATH_EF );
    HIDECONTROL( hwnd, ID_DOCIMP_NEWCURDIR_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_CURDIR_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_BROWSE_PB );
    HIDECONTROL( hwnd, ID_DOCIMP_IMPPATH_LB );
    SHOWCONTROL( hwnd, ID_DOCIMP_STARTPATH_CB );
    SHOWCONTROL( hwnd, ID_DOCIMP_FILES_LB );
    SHOWCONTROL( hwnd, ID_DOCIMP_DIR_LB );
    SHOWCONTROL( hwnd, ID_DOCIMP_FILES_TEXT );
    HIDECONTROL( hwnd, ID_DOCIMP_FILESIMPPATH_TEXT );
    SHOWCONTROL( hwnd, ID_DOCIMP_DIR_TEXT );

//    SETTEXTFROMRES( hwnd, ID_DOCIMP_CURDIR_TEXT, pIda->szString,
//                    hResMod, SID_DOCIMP_STARTPATH );

    SETTEXTFROMRES( hwnd, ID_DOCIMP_DIR_TEXT, pIda->szString,
                    hResMod, SID_DOCIMP_INTDIRECTORY );
    SETTEXTFROMRES( hwnd, ID_DOCIMP_CURDIR_TEXT, pIda->szString,
                    hResMod, SID_DOCIMP_INTIMPORT );
    SETTEXTFROMRES( hwnd, ID_DOCIMP_FILES_TEXT, pIda->szString,
                    hResMod, SID_DOCIMP_INTFILE );

    /********************************************************/
    /* Force a refresh of the listboxes                     */
    /********************************************************/
    {
      char szBuffer[1000];
      USHORT usDrive;
      strcpy(szBuffer,  pIda->Controls.szDrive);
      SETTEXT( hwnd, ID_DOCIMP_STARTPATH_CB, szBuffer );

      usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                             szBuffer[0]);
      DocImpControl (hwnd,usDrive,BN_CLICKED);
    }// end of construction
  } /* endif */
}
//+----------------------------------------------------------------------------+
//| DocImpControl           - process WM_CONTROL msg of Document Import Dialog |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Handle all WM_CONTROL messages of the document import dialog            |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HWND      hwnd                IN      dialog window handle              |
//|    MPARAM    mp1                 IN      first message parameter           |
//|    MPARAM    mp2                 IN      second message parameter          |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    MRESULT                                  result of message processing   |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    IDA must have been allocated and anchored using WinSetWindowULong.      |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    IDA fiels may be changed to reflect the new dialog status.              |
//+----------------------------------------------------------------------------+
//static
MRESULT DocImpControl
(
HWND   hwnd,                        // dialog window handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PDOCIMPIDA  pIda;                   // instance area for EQFFIMPO
  SHORT       sIndexItem;             // index for list box items
  USHORT      usOldDrive;             // buffer for old drive
  FILEFINDBUF ResultBuf;              // DOS file find struct
  USHORT      usCount = 1;            // # of files for DosFindFirst
  HDIR        hDirHandle = HDIR_CREATE; // DosFind routine handle
  USHORT      usDosRC;                // return code of Dos... alias Utl...
  CHAR        szDrive[MAX_DRIVE];     // buffer for drive string
  PSZ         pszErrParm;             // pointer to erraneous parameter
  USHORT      usMBID;                 // message box return code

  /****************************************************************/
  /* Get access to ida                                            */
  /****************************************************************/
  pIda = ACCESSDLGIDA( hwnd, PDOCIMPIDA );

  /****************************************************************/
  /* Filter messages from format radio buttons as they are        */
  /* currently not handled correctly in the dialog utility        */
  /****************************************************************/
  switch ( sId )
  {
    //UCD_ONE_PROPERTY
    case ID_DOCIMP_EXTERNAL_RB :
//      if ( QUERYCHECK( hwnd, sId ) )
      {
        PrepExtFormDlg( pIda, hwnd, sId );
      } /* endif */
      break;

    case ID_DOCIMP_IMPPATH_RB :
//      if ( QUERYCHECK( hwnd, sId ) )
      {
        PrepImpPathFormDlg( pIda, hwnd, sId );
      } /* endif */
      break;

    case ID_DOCIMP_INTERNAL_RB :
//      if ( QUERYCHECK( hwnd, sId ) )
      {
        PrepIntFormDlg( pIda, hwnd, sId );
      } /* endif */
      break;

    case ID_DOCIMP_DIR_LB:    // only for the internal Import (TranslationManager)
      if ( pIda->usFormatID == ID_DOCIMP_INTERNAL_RB )
      {
        if ( sNotification == LN_SELECT )
        {
          DELETEALL( hwnd, ID_DOCIMP_FILES_LB );
          sIndexItem = QUERYSELECTION( hwnd,  ID_DOCIMP_DIR_LB );
          if ( sIndexItem != LIT_NONE )
          {
            // get selected folder long/short name
            int iFolderIndex = QUERYITEMHANDLE( hwnd, ID_DOCIMP_DIR_LB, sIndexItem );

            // get folder short name from folder short name listbox
            QUERYITEMTEXTHWND( pIda->hwndFolderListBox, iFolderIndex, pIda->szFromFolder );
            strcat( pIda->szFromFolder, EXT_FOLDER_MAIN );

            // get package short names
            DELETEALLHWND( pIda->hwndShortNameLB );
            UtlPackListPackages( pIda->hwndShortNameLB,
                                 pIda->Controls.szDrive[0],
                                 pIda->szFromFolder, NULL,
                                 DEFAULT_PATTERN, PACKFLAG_WITHEXT );

            // get the long names for documents from the document package
            {
              SHORT sItem = QUERYITEMCOUNTHWND( pIda->hwndShortNameLB );
              while ( sItem-- )
              {
                SHORT sInsertItem;           // index of inserted item

                // get the document name
                QUERYITEMTEXTHWND( pIda->hwndShortNameLB, sItem,
                                   pIda->szString );

                // open the package
                pIda->pPackCtrl = UtlPackOpenPackage( pIda->Controls.szDrive[0],
                                                      pIda->szFromFolder,
                                                      NULL,
                                                      pIda->szString, NULLHANDLE );
                if ( pIda->pPackCtrl )
                {
                  ULONG ulRC;

                  // get package header
                  memset( &(pIda->Header), 0, sizeof(pIda->Header) );
                  ulRC = UtlPackReadHeader( pIda->pPackCtrl,
                                            (PBYTE)&(pIda->Header),
                                            (ULONG)sizeof(pIda->Header) );
                  if ( pIda->Header.szLongName[0] != EOS )
                  {
                    strcpy( pIda->szString, pIda->Header.szLongName );
                    OEMTOANSI( pIda->szString );
                  } /* endif */

                  // close package
                  UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
                } /* endif */

                // add long name or short document name to listbox
                sInsertItem = INSERTITEM( hwnd, ID_DOCIMP_FILES_LB,
                                          pIda->szString );

                // set item handle to index of document in invisible document lb
                if ( sInsertItem >= 0 )
                {
                  SETITEMHANDLE( hwnd, ID_DOCIMP_FILES_LB, sInsertItem, sItem );
                } /* endif */
              } /* endwhile */
              {
                HWND hwndLB = GetDlgItem(hwnd, ID_DOCIMP_FILES_LB);
                UtlSetHorzScrollingForLB(hwndLB);
              }
            }
          } /* endif */
        } /* endif */
      }
//WL // RJ @@@@ ??? (03/02/15)
      else if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
      {
      }
      else
      {
        UtlWMControls( hwnd, WM_CONTROL, sId, sNotification, &pIda->Controls );
      } /* endif */
      break;

    case ID_DOCIMP_FILES_LB:
      if ( pIda->usFormatID == ID_DOCIMP_EXTERNAL_RB )
      {
        if ( sNotification == LN_SELECT )
        {
          UtlWMControls( hwnd, WM_CONTROL, sId, sNotification, &pIda->Controls );
        } /* endif */
      } /* endif */
      break;

    default :
//WL
      if ( pIda->usFormatID == ID_DOCIMP_INTERNAL_RB )
      {
        /**********************************************************/
        /* Handle control message within dialog as dialog utility */
        /* can not handle internal formats                        */
        /**********************************************************/
        if ( ( sNotification == BN_CLICKED )          &&
             ( sId >= PID_DRIVEBUTTON_A ) && ( sId <= PID_DRIVEBUTTON_Z ) )
        {
          DocImpStopThread( pIda );           // stop any running thread
          if ( pIda->Controls.szDrive[0] != EOS )
          {
            usOldDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                      pIda->Controls.szDrive[0]);
            SETDRIVE( hwnd, usOldDrive, FALSE );
          }
          else
          {
            usOldDrive = 0;
          } /* endif */

          /**********************************************************/
          /* Save newly selected drive character in IDA and select  */
          /* it                                                     */
          /**********************************************************/
          pIda->Controls.szDrive[0] = DRIVEFROMID( PID_DRIVEBUTTON_A, sId );
          SETDRIVE( hwnd, sId, TRUE );

          /**********************************************************/
          /* Update the folder and files listbox                    */
          /**********************************************************/
          DELETEALL( hwnd, ID_DOCIMP_DIR_LB );
          DELETEALL( hwnd, ID_DOCIMP_FILES_LB );
          DELETEALLHWND( pIda->hwndFolderListBox );
          UtlMakeEQFPath( pIda->szString, pIda->Controls.szDrive[0], EXPORT_PATH, NULL );

          do
          {
            usDosRC = UtlFindFirst( pIda->szString, &hDirHandle,
                                    FILE_DIRECTORY,
                                    &ResultBuf, sizeof( ResultBuf),
                                    &usCount, 0L, FALSE );
            UtlFindClose( hDirHandle, FALSE );

            switch ( usDosRC )
            {
              case NO_ERROR :
                usMBID = MBID_OK;
                break;
              case ERROR_DISK_CHANGE :
              case ERROR_NOT_READY :
                szDrive[0] = pIda->szString[0];
                szDrive[0] = COLON;
                szDrive[0] = EOS;
                pszErrParm = szDrive;
                usMBID = UtlError( usDosRC, MB_RETRYCANCEL, 1,
                                   &pszErrParm, DOS_ERROR );
                break;
              default :
                usMBID = MBID_CANCEL;
                break;
            } /* endswitch */
          } while ( usMBID == MBID_RETRY ); /* enddo */

          if ( usDosRC == NO_ERROR )
          {
            // get folder short names
            strcat( pIda->szString, BACKSLASH_STR );
            strcat( pIda->szString, DEFAULT_PATTERN_NAME );
            strcat( pIda->szString, EXT_FOLDER_MAIN );
            UtlLoadFileNames( pIda->szString,
                              FILE_DIRECTORY, pIda->hwndFolderListBox,
                              NAMFMT_NODRV | NAMFMT_NOEXT |
                              NAMFMT_TOPSEL);

            // get folder long names for all folders
            {
              int iFolderCount = QUERYITEMCOUNTHWND( pIda->hwndFolderListBox );
              int iFolder = 0;
              while ( iFolder < iFolderCount )
              {
                CHAR szFolShortName[MAX_FILESPEC];
                int iInsertedItem;

                // get folder short name
                QUERYITEMTEXTHWND( pIda->hwndFolderListBox, iFolder, szFolShortName );

                // setup name of dummy file containg folder long name
                UtlMakeEQFPath( pIda->szString, pIda->Controls.szDrive[0], EXPORT_PATH, NULL );
                strcat( pIda->szString, BACKSLASH_STR );
                strcat( pIda->szString, szFolShortName );
                strcat( pIda->szString, EXT_FOLDER_MAIN );
                strcat( pIda->szString, BACKSLASH_STR );
                strcat( pIda->szString, szFolShortName );
                strcat( pIda->szString, EXT_FOLDER_MAIN );

                // load dummy file and extract folder long name (if any)
                strcpy( pIda->szFolLongName, szFolShortName );   // use short name as default
                {
                  FILE *hFile = fopen( pIda->szString, "r" );
                  if ( hFile )
                  {
                    memset( pIda->szString, 0, sizeof(pIda->szString) );
                    fread( pIda->szString, 1, sizeof(pIda->szString), hFile );
                    if ( strnicmp( pIda->szString, "FolName=", 8 ) == 0 )
                    {
                      strcpy( pIda->szFolLongName, pIda->szString+8 );
                    } /* endif */
                    fclose( hFile );
                  } /* endif */
                }

                // add folder name to folder listbox
                iInsertedItem = INSERTITEM( hwnd, ID_DOCIMP_DIR_LB, pIda->szFolLongName );

                // set handle of inserted item to item in folder short name listbox
                if ( iInsertedItem >= 0 )
                {
                  SETITEMHANDLE( hwnd, ID_DOCIMP_DIR_LB, iInsertedItem, iFolder );
                } /* endif */

                // next item
                iFolder++;
              } /* endwhile */
            }

            // update documents listbox
            DocImpControl( hwnd, ID_DOCIMP_DIR_LB, LN_SELECT );
           } /* endif */
        } /* endif */
      }
      else if ( pIda->usFormatID == ID_DOCIMP_IMPPATH_RB )
      {
        // handle control message within dialog as dialog utility
        // can not handle relative path information
        if ( ( sNotification == BN_CLICKED )          &&
             ( sId >= PID_DRIVEBUTTON_A ) && ( sId <= PID_DRIVEBUTTON_Z ) )
        {
          if ( pIda->Controls.szDrive[0] != EOS )
          {
            usOldDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                      pIda->Controls.szDrive[0]);
            SETDRIVE( hwnd, usOldDrive, FALSE );
          }
          else
          {
            usOldDrive = 0;
          } /* endif */

          // get current search pattern
          QUERYTEXT( hwnd, ID_DOCIMP_PATH_EF, pIda->Controls.szPathContent );
          UtlMakeFNameAndPath( pIda->Controls.szPathContent,
                               pIda->Controls.szDummy,
                               pIda->Controls.szPatternName);
          if ( pIda->Controls.szPatternName[0] == EOS )
          {
            strcpy( pIda->Controls.szPatternName, DEFAULT_PATTERN );
          } /* endif */
          // Save newly selected drive character in IDA and select it
          pIda->Controls.szDrive[0] = DRIVEFROMID( PID_DRIVEBUTTON_A, sId );
          SETDRIVE( hwnd, sId, TRUE );

          // use current start path or clear start path field if path does not exist
          sprintf( pIda->szString, "%c:%s", pIda->Controls.szDrive[0], pIda->szStartPath );

          // remove traling backslash to allow usage of UtlDirExist
          if ( pIda->szString[strlen(pIda->szString)-1] == BACKSLASH )
          {
            pIda->szString[strlen(pIda->szString)-1] = EOS;
          } /* endif */
          if ( !UtlDirExist( pIda->szString ) )
          {
            pIda->szStartPath[0] = EOS;
          } /* endif */
          SETTEXT( hwnd, ID_DOCIMP_STARTPATH_CB, pIda->szStartPath );

          // Update the files listbox
          DELETEALL( hwnd, ID_DOCIMP_IMPPATH_LB );
          sprintf( pIda->szString, "%c:%s", pIda->Controls.szDrive[0],
                   pIda->szStartPath );
          if ( pIda->szString[strlen(pIda->szString)-1] != BACKSLASH )
          {
            strcat( pIda->szString, BACKSLASH_STR );
          } /* endif */
          DocScanDirStartThread( pIda->szString,
                                 pIda->Controls.szPatternName,
                                 pIda->Controls.szDummy,
                                 (SHORT)strlen(pIda->szString),
                                 WinWindowFromID( hwnd, ID_DOCIMP_IMPPATH_LB ),
                                 &pIda->sThreadStatus );
        } /* endif */
      }
      else
      {
        UtlWMControls( hwnd, WM_CONTROL, sId, sNotification, &pIda->Controls );
      } /* endif */
      break;
  } /* endswitch */

  return( MRFROMSHORT(FALSE ) );

} /* end of DocImpControl */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EnableImportControls                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EnableImportControls (hwnd, TRUE);                       |
//+----------------------------------------------------------------------------+
//|Description:       Enable/disable all controls of the import dialog         |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND hwnd - window handle of import dialog               |
//|                   BOOL fEnable - enable or disable controls                |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       None                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+

static VOID EnableImportControls (HWND hwnd, BOOL fEnable)
{
  ENABLECTRL( hwnd, PID_PB_OK, fEnable);
  ENABLECTRL( hwnd, PID_PB_HELP, fEnable);

  ENABLECTRL( hwnd, ID_DOCIMP_FORMAT_GB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_INTERNAL_RB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_EXTERNAL_RB, fEnable);

  ENABLECTRL( hwnd, ID_DOCIMP_IMPORT_GB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_FILENAME_TEXT, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_PATH_EF, fEnable);
//  ENABLECTRL( hwnd, ID_DOCIMP_CURDIR_TEXT, fEnable);
//  ENABLECTRL( hwnd, ID_DOCIMP_NEWCURDIR_TEXT, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_DIR_TEXT, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_FILES_TEXT, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_FILESIMPPATH_TEXT, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_DIR_LB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_FILES_LB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_SELALL_PB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_DESALL_PB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_TO_TEXT, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_FOLDER_TEXT, fEnable);

  ENABLECTRL( hwnd, ID_DOCIMP_IMPPATH_RB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_IMPPATH_LB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_STARTPATH_CB, fEnable);
  ENABLECTRL( hwnd, ID_DOCIMP_BROWSE_PB, fEnable);
} /* end of function EnableImportControls */

//+----------------------------------------------------------------------------+
// Function name:  UtlDMGETDEFID                                                
//+----------------------------------------------------------------------------+
// Description:  processes DM_GETDEFID message of export and import dialogs     
//+----------------------------------------------------------------------------+
// Parameters:                                                                  
//     HWND           hwndDlg             IN      dialog window handle          
//     MPARAM         mp1                 IN      first message parameter       
//     MPARAM         mp2                 IN      second message parameter      
//     PCONTROLSIDA   pControlsIda        IN      ida with dialog control info  
//+----------------------------------------------------------------------------+
// Returncode type:  MRESULT                                                    
//+----------------------------------------------------------------------------+
// Returncodes: -                                                               
//+----------------------------------------------------------------------------+
// Prerequesites:                                                               
//     The following fields in CONTROLSIDA need to be filled before the         
//     function may be called:                                                  
//     idPathEF              with the id of path entry field                    
//     idDirLB               with the id of directories listbox                 
//     idFilesLB             with the id of files listbox for import dialogs    
//     idOkPB                with the id of btn that triggers process           
//+----------------------------------------------------------------------------+
// Side effects:                                                                
//     IDA fields may be changed to reflect the new dialog status.              
//     szPathContent         contains the fully qualified path name             
//     szPath                contains drive and directory                       
//     szPatternName         contains file name plus extension                  
//     szDrive               contains drive with colon                          
//     The rest of the fields in the ida are used for control processing        
//+---------------------------------------------------------------------------- 
// Function call: UtlDMGETDEFID( hwndDlg, mp1, mp2, &pIda->ControlsIda );       
//+----------------------------------------------------------------------------+
MRESULT UtlDMGETDEFID1
(
HWND             hwndDlg,            // dialog window handle
WPARAM           mp1,                // first message parameter
LPARAM           mp2,                // second message parameter
PCONTROLSIDA pControlsIda,           // pointer to ida
USHORT usFormatID                    // pointer to document import IDA
)
{
  MRESULT     mResult = (MRESULT)FALSE; //return code of this function
  HWND        hwndFocus;                //handle for input focus
  mp1; mp2;
  /************************************************************/
  /* check if user pressed the ENTER key, but wants only to   */
  /* select/deselect an item of the listbox via a simulated   */
  /* (keystroke) double click or if the user is in the        */
  /* file name entry field and tries to activate a new        */
  /* search pattern                                           */
  /************************************************************/
  if ( GetKeyState(VK_RETURN) & 0x8000  )
  {
    HWND hwndParent;

    hwndFocus = GetFocus();
    hwndParent = GetParent( hwndFocus );

    //UCD_ONE_PROPERTY

    if ( (hwndFocus == GetDlgItem( hwndDlg, pControlsIda->idPathEF) )  ||
         (hwndFocus == GetDlgItem( hwndDlg, pControlsIda->idCurrentDirectoryEF )  ) ||
         (hwndParent == GetDlgItem( hwndDlg, pControlsIda->idPathEF ) )  ||
         (hwndParent == GetDlgItem( hwndDlg, pControlsIda->idCurrentDirectoryEF )  ) )
    {

      if ( usFormatID == ID_DOCIMP_IMPPATH_RB)
      {
        // nothing to do
      }
      else if (usFormatID == ID_DOCIMP_EXTERNAL_RB)
      {
#ifndef _TQM
        DocExpHandlePathInput(hwndDlg);
#endif
        UtlEFValidityTest( pControlsIda, hwndDlg );
      }
      else if (usFormatID == ID_DOCIMP_INTERNAL_RB)
      {
        USHORT usDrive;

        //QUERYTEXT( hwndDlg, pControlsIda->idPathEF, pControlsIda->szPathContent );

        //load content of current dir entryfield into szPath
        QUERYTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF, pControlsIda->szDrive );

        pControlsIda->szDrive[2] = EOS;

        SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF, pControlsIda->szDrive );


        usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                               pControlsIda->szDrive[0]);

  #ifndef _TQM
        DocImpControl (hwndDlg,usDrive,BN_CLICKED);
  #endif


      }
      else
      {
        UtlEFValidityTest( pControlsIda, hwndDlg );

      } // end if




      mResult = TRUE;                  // do not process as default pushbutton
    }
    else if ( hwndFocus == GetDlgItem( hwndDlg, pControlsIda->idDirLB ) )
    {
      PostMessage( hwndDlg, WM_COMMAND,
                   MP1FROMSHORT(pControlsIda->idDirLB),
                   MP2FROM2SHORT( 0, LN_ENTER ) );
      mResult = TRUE;                  // do not process as default pushbutton
    } /* endif */
  } /* endif */
  return( mResult );
} /* end of function UtlDMGETDEFID1 */

