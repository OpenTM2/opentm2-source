//+----------------------------------------------------------------------------+
//|EQFMEMLD.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:  Markus Conrad, Stefan Doersam                                      |
//+----------------------------------------------------------------------------+
//|Description: Dialog to load (import) a Translation Memory                   |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|T  MEMLOADDLG   -TM import dialog procedure                                 |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//| UtlControlsInit                                                            |
//| UtlWMControls                                                              |
//| UtlWMChar                                                                  |
//| UtlDefDialogProc                                                           |
//| UtlError                                                                   |
//| UtlMakeFullPath                                                            |
//| UtlOpen                                                                    |
//| TmOpen                                                                     |
//| TmClose                                                                    |
//| MemOpenProp                                                                |
//| CloseProperties                                                            |
//| SaveProperties                                                             |
//| QUERYSELECTION                                                             |
//| QUERYITEMTEXT                                                              |
//| DELETEALL                                                                  |
//| SEARCHSELECT                                                               |
//| EnableOKAndCANCELButton                                                    |
//| EqfSend2Handler                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|   MemLoadCommand  -handles WM_COMMAND messages of MEMLOADDLG dialog proc.  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "MemoryFactory.h"

#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMEM.ID>              // Translation Memory IDs
#include "SHLOBJ.H"             // shell functions
extern HELPSUBTABLE pid_memimp_subtable[];

//--- prototypes for internal functions
static MRESULT MemLoadCommand( HWND, SHORT );

// hook procedure for standard file open dialog
UINT_PTR CALLBACK MemOpenFileHook( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MEMLOADDLG                                               |
//+----------------------------------------------------------------------------+
//|Function call:     MRESULT EXPENTRY MEMLOADDLG( HWND hwnd,                  |
//|                                                USHORT msg,                 |
//|                                                MPARAM mp1,                 |
//|                                                MPARAM mp2 )                |
//+----------------------------------------------------------------------------+
//|Description:       Dialog procedure to import a translation memory          |
//|                   The function will only be called by the TM handler       |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND    hwnd    handle of dialog                         |
//|                   USHORT  msg     message number                           |
//|                   MPARAM  mp1     message parameter 1                      |
//|                   MPARAM  mp2     message parameter 2                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       depends on message type                                  |
//|                   normal return codes are:                                 |
//|                   TRUE  = message has been processed                       |
//|                   FALSE = message has not been processed                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//| - mp2 contains the pointer to MEM_LOAD_DLG_IDA                             |
//|                                                                            |
//| the following variables in MEM_IMPORT_IDA need to be filled before         |
//| function call :                                                            |
//|   szMemName    with the name of the TM to import to or NULL                |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  the following variables of MEM_LOAD_DLG_IDA are changed                   |
//|    hFile      contains the file handle if a file has been selected to be   |
//|               loaded into a memory database.                               |
//|               NULL if no file has been selected or the dialog has been     |
//|               canceled                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  switch ( message )                                                        |
//|  {                                                                         |
//|     case WM_INITDLG :                                                      |
//|        initialize variables ,IDA and file handle                           |
//|                                                                            |
//|        if ( connection dialog to IDA fails )                               |
//|        {                                                                   |
//|           display error message                                            |
//|           fOk = FALSE                                                      |
//|        }                                                                   |
//|        if ( fOK )                                                          |
//|        {                                                                   |
//|           open TM last used properties to get last used dialog values      |
//|           set fOk = FALSE when error open properties                       |
//|        }                                                                   |
//|        if ( fOK )                                                          |
//|        {                                                                   |
//|           initialize input parameters for UtlControlsInit utility          |
//|           post message WM_EQF_INITIALIZE                                   |
//|        }                                                                   |
//|        if ( !fOK )                                                         |
//|        {                                                                   |
//|           post WM_CLOSE to the dialog                                      |
//|        }                                                                   |
//|        break;                                                              |
//|     case WM_EQF_INITIALIZE:                                                |
//|           call dialog utility UtlControlsInit to initialize the dialog     |
//|        break;                                                              |
//|     case WM_ADJUSTWINDOWPOS:                                               |
//|        get access to IDA                                                   |
//|        if ( disabled flag is set )                                         |
//|          update position and size of window                                |
//|        else                                                                |
//|           call UtlDefDialogProc                                            |
//|        break;                                                              |
//|     case WM_HITTEST :                                                      |
//|        if ( disabled flag is set )                                         |
//|          return HT_ERROR                                                   |
//|        else                                                                |
//|           call UtlDefDialogProc                                            |
//|        break;                                                              |
//|     case WM_CONTROL :                                                      |
//|        get access to IDA and call dialog utility UtlWMControls to handle   |
//|        the WM_CONTROL messages                                             |
//|        break;                                                              |
//|     case WM_COMMAND :                                                      |
//|        call internal function MemLoadCommand to handle WM_COMAMND message  |
//|        break;                                                              |
//|     case WM_CLOSE   :                                                      |
//|        close properties and destroy dialog                                 |
//|        break;                                                              |
//|     case WM_CHAR    :                                                      |
//|        if ( no keyup message )                                             |
//|        {                                                                   |
//|           switch( virtual key )                                            |
//|           {                                                                |
//|              case ( virtual key NEWLINE is pressed ) :                     |
//|              case ( virtual key ENTER is pressed )   :                     |
//|                 call dialog utility UtlWMChar to handle WM_CHAR messages   |
//|                 break;                                                     |
//|              default :                                                     |
//|                 call default dialog procedure                              |
//|                 break;                                                     |
//|           }                                                                |
//|        }                                                                   |
//|        break;                                                              |
//|     default :                                                              |
//|        call default dialog procedure                                       |
//|        break;                                                              |
//|  }                                                                         |
//|  if ( !fOK )                                                               |
//|     close the dialog                                                       |
//|                                                                            |
//|  return;                                                                   |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK MEMLOADDLG( HWND hwnd,
                             WINMSG msg,
                             WPARAM mp1,
                             LPARAM mp2 )  //--- pointer to import IDA
{
   PMEM_LOAD_DLG_IDA   pLIDA;               // Load dialog IDA
   BOOL        fOk = TRUE;                  // error flag
   MRESULT     mResult = (MRESULT)FALSE;    // rc of this function
   EQFINFO     ErrorInfo;                   // returned from property handler
   CHAR        szCurPath[256];

   switch ( msg )
   {
      //-----------------------------------------------------------------------
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_MEMLOAD_DLG, mp2 ); break;

      case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
       EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &pid_memimp_subtable[0] );
       mResult = TRUE;  // message processed
       break;


      case ( WM_INITDLG ) :
         SETWINDOWID( hwnd, ID_MEMLOAD_DLG );
         //--- Address the load/import IDA
         pLIDA = (PMEM_LOAD_DLG_IDA)PVOIDFROMMP2( mp2 );

         //--- Set file handle to NULL
         pLIDA->hFile = NULLHANDLE;

         pLIDA->usImpMode = MEM_SGMLFORMAT_ASCII;    // set default value
         //--- connect the dialog with the IDA
         if ( ANCHORDLGIDA( hwnd, pLIDA ) == FALSE )
         {
           //--- Issue the system error message
           UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
           fOk = FALSE;
         } /* endif */

         //--- access last used values by opening TM last used properties
         //--- issue an error message in case of error
         if ( fOk )
         {
           //--- get system path
           UtlMakeEQFPath( pLIDA->szDummy, NULC, SYSTEM_PATH, NULL );
           fOk = MemOpenProp(&(pLIDA->hPropLast),      // handle of properties
                             (PVOID *)&(pLIDA->pPropLast),// pointer to properties
                             MEMORYLU_PROPERTIES_NAME, // properties name
                             pLIDA->szDummy,           // system path
                             PROP_ACCESS_READ,
                             TRUE );                   // display error messages
         } /* endif */

         if ( fOk )
         {
           //--- tell control utility the dialog IDs
           pLIDA->ControlsIda.idPathEF             = DID_LD_PATH;
           pLIDA->ControlsIda.idDirLB              = DID_LD_DIR;
           pLIDA->ControlsIda.idFilesLB            = DID_LD_FILES;
           pLIDA->ControlsIda.idCurrentDirectoryEF = DID_LD_CURDIR_EF;
           pLIDA->ControlsIda.idDriveBTN           = DID_LD_DUMMY;
           pLIDA->ControlsIda.idControlsGB         = DID_LD_IMPORTGROUP;
           pLIDA->ControlsIda.idInternalRB         = DID_LD_INTERNALFORMAT;
           pLIDA->ControlsIda.idExternalRB         = DID_LD_ASCIIFORMAT;
           pLIDA->ControlsIda.idToLB               = DID_LD_TO;
           pLIDA->ControlsIda.idOkPB               = PID_PB_OK;
           pLIDA->ControlsIda.fLongFileNames       = TRUE;
           pLIDA->ControlsIda.fMultiSelectionLB = TRUE;

           //--- tell controls utility the name of the object handler
           //--- to fill the To TM listbox
           strcpy( pLIDA->ControlsIda.szHandler, MEMORYHANDLER );

           //--- Tell the controls utility the name to be displayed in the
           //--- entry field of the To TM combo box. This is the TM name
           //--- to import to
           strcpy( pLIDA->ControlsIda.szSelectedName, pLIDA->szMemName );

           //--- tell control utility the RB to be selected
           pLIDA->ControlsIda.usSavedFormat = EXTERNAL;

           //--- tell control utility that an import dialog is calling
           pLIDA->ControlsIda.fImport = TRUE;

           //--- tell control utility last used path x:\xxx\xxx
           if ( pLIDA->pPropLast->szImpPathLastUsed2[0] != EOS )
           {
             strncpy( pLIDA->ControlsIda.szSavedPath,
                      pLIDA->pPropLast->szImpPathLastUsed2,
                      sizeof(pLIDA->ControlsIda.szSavedPath)-1 );
             pLIDA->ControlsIda.szSavedPath[sizeof(pLIDA->ControlsIda.szSavedPath)-1] = EOS;
           }
           else
           {
             strncpy( pLIDA->ControlsIda.szSavedPath,
                      pLIDA->pPropLast->szImpPathLastUsed,
                      sizeof(pLIDA->ControlsIda.szSavedPath)-1 );
             pLIDA->ControlsIda.szSavedPath[sizeof(pLIDA->ControlsIda.szSavedPath)-1] = EOS;
           } /* endif */

           //--- tell controls utility last used drive without colon
           pLIDA->ControlsIda.chSavedDrive =
                                      pLIDA->ControlsIda.szSavedPath[0];

           if (  (pLIDA->pPropLast->usImpMode == MEM_FORMAT_TMX_TRADOS) ||
                 (pLIDA->pPropLast->usImpMode == MEM_FORMAT_TMX) )
           {
            strcpy( pLIDA->ControlsIda.szDefPattern, "*.TMX" );
           } else
           if ( pLIDA->pPropLast->usImpMode == MEM_FORMAT_XLIFF_MT ) 
           {
            strcpy( pLIDA->ControlsIda.szDefPattern, "*.XLF" );
           }
           else
           {
            strcpy( pLIDA->ControlsIda.szDefPattern, "*.EXP" );
           } /* endif */

           if ( pLIDA->pPropLast->usImpMode ) pLIDA->usImpMode = pLIDA->pPropLast->usImpMode;
         } /*end if*/

         //--- close last used properties if open                    /* @A8A */
         if ( pLIDA->hPropLast )                                     /* @A8A */
         {                                                           /* @A8A */
           CloseProperties( pLIDA->hPropLast, PROP_QUIT, &ErrorInfo);/* @A8A */
           pLIDA->hPropLast = NULL;                                  /* @A8A */
         } /*end if*/

         // fill format combobox and select last used format
         {
           PSZ pszFormatList = MEM_FORMAT_FILTERS;
           USHORT usIndex = 1;
           while ( pszFormatList[0] != EOS )
           {
             SHORT sItem = CBINSERTITEM( hwnd, DID_LD_FORMAT_CB, pszFormatList );
             if ( sItem != LIT_NONE )
             {
               CBSETITEMHANDLE( hwnd, DID_LD_FORMAT_CB, sItem, usIndex );
               if ( usIndex == pLIDA->usImpMode )
               {
                 CBSELECTITEM( hwnd, DID_LD_FORMAT_CB, sItem );
               } /* endif */
             } /* endif */
             pszFormatList += strlen(pszFormatList) + 1;   // skip filter name
             pszFormatList += strlen(pszFormatList) + 1;   // skip filter extention
             usIndex++;
           } /* endwhile */
         }

         /**********************************************************/
         /* hide the dummy button to avoid flickering              */
         /**********************************************************/
         HIDECONTROL( hwnd, DID_LD_DUMMY );

         //--- post message to call utility UtlControlsInit
         //--- this is done because of the following reason:
         //--- if as last used drive a diskette drive is stored and this drive
         //--- is not ready, the message drive not ready is displayed before
         //--- the dialog is up
         WinPostMsg( hwnd, WM_EQF_INITIALIZE, NULL, mp2 );
         if ( !fOk )   //--- error occured
         {
           //--- close the dialog
           DISMISSDLG( hwnd, TRUE );
         } /* endif */

         mResult = DIALOGINITRETURN( FALSE );
         break;
      //-----------------------------------------------------------------------

      case WM_EQF_INITIALIZE:
         //--- Address the load/import IDA
         pLIDA = (PMEM_LOAD_DLG_IDA)PVOIDFROMMP2( mp2 );

         //--- call controls utility to initialize controls
         UtlControlsInit( hwnd, &pLIDA->ControlsIda );
         // cv
         // display path
         if (strlen(pLIDA->ControlsIda.szPath) > 30)
         {
            CHAR szPath[256];
            CHAR *psz;
            int len;
            strcpy(szPath,pLIDA->ControlsIda.szPath);
            strcpy( szCurPath, pLIDA->ControlsIda.szDrive );
            strcat( szCurPath, BACKSLASH_STR );
            strcat( szCurPath, "...");
            //get last directory entry
            len = strlen(szPath);
            szPath[len-1] = EOS;
            psz = strrchr(szPath,BACKSLASH);
            strcat(szCurPath, psz);
            strcat( szCurPath, BACKSLASH_STR );

            SETTEXT( hwnd, pLIDA->ControlsIda.idCurrentDirectoryEF+1,
                     szCurPath );
         }

         //--- disable internal format RB
         ENABLECTRL( hwnd, DID_LD_INTERNALFORMAT, FALSE );
         SETCHECK_TRUE( hwnd, DID_LD_ASCIIFORMAT);
         break;

      //-----------------------------------------------------------------------
      case ( WM_COMMAND ) :
        switch ( WMCOMMANDID( mp1, mp2 ))
        {
		      case PID_PB_HELP:
            mResult = UtlInvokeHelp();
            break;
          case PID_PB_CANCEL:
          case DID_CANCEL:
          case PID_PB_OK:
            mResult = MemLoadCommand( hwnd, WMCOMMANDID( mp1, mp2 ) );
            break;

          case DID_LD_FORMAT_CB:
            if ( HIWORD(mp1) == CBN_SELCHANGE )
            {
              // CHAR szPath[256];

              // check if default pattern is still active...
              // GQ: check removed, always switch pattern when changing format
              //QUERYTEXT( hwnd, DID_LD_PATH, szPath );
              //if ( strncmp( szPath, "*.", 2 ) == 0 )
              {
                // get new selection and set search pattern accordingly
                SHORT sItem = CBQUERYSELECTION( hwnd, DID_LD_FORMAT_CB );
                if ( sItem >= 0 )
                {
                  USHORT usImpMode = (USHORT)CBQUERYITEMHANDLE( hwnd, DID_LD_FORMAT_CB, sItem );
                  switch ( usImpMode )
                  {
                    case MEM_FORMAT_TMX:
                    case MEM_FORMAT_TMX_TRADOS:
                        SETTEXT( hwnd, DID_LD_PATH, "*.TMX" );
                        break;
                    case MEM_FORMAT_XLIFF_MT:
                        SETTEXT( hwnd, DID_LD_PATH, "*.XLF" );
                        break;
                    default:
                        SETTEXT( hwnd, DID_LD_PATH, "*.EXP" );
                        break;
                  } /*endswitch */

                  // force a refresh of the listboxes
                  pLIDA = ACCESSDLGIDA(hwnd, PMEM_LOAD_DLG_IDA);
                  UtlEFValidityTest( &pLIDA->ControlsIda , hwnd  );
                } /* endif */
              } /* endif */

            } /* endif */
            break;

          default:
            //--- get acces to instance area
            pLIDA = ACCESSDLGIDA(hwnd, PMEM_LOAD_DLG_IDA);

            //--- call utility to handle WM_CONTROL message
            UtlWMControls( hwnd, msg, WMCOMMANDID( mp1, mp2 ),
                           WMCOMMANDCMD( mp1, mp2 ),
                           &pLIDA->ControlsIda );
        } /* endswitch */
        break;
      case WM_EQF_CLOSE:
         //--- get acces to IDA
        pLIDA = ACCESSDLGIDA(hwnd, PMEM_LOAD_DLG_IDA);

        if ( pLIDA )  //--- access to IDA OK
        {
           //--- close last used properties if open
           if ( pLIDA->hPropLast )
             CloseProperties( pLIDA->hPropLast, PROP_QUIT, &ErrorInfo);
        } /* endif */

        //distroy dialog
        DISMISSDLG( hwnd, FALSE );
        break;

      case DM_GETDEFID:
         pLIDA = ACCESSDLGIDA(hwnd, PMEM_LOAD_DLG_IDA);
         mResult = UtlDMGETDEFID( hwnd, mp1, mp2, &pLIDA->ControlsIda );
         break;

      //-----------------------------------------------------------------------
      default :
         //--- return to default dialog procedure
         mResult = UTLDEFDIALOGPROC( hwnd, msg, mp1, mp2 );
        break;
   }/*end switch*/
   return ( mResult );
}/* end of function MEMLOADDLG */

//
// Check if specified import method matches the memory being imported
//
BOOL MemCheckFileType
( 
  PMEM_LOAD_DLG_IDA pLIDA, 
  PSZ              pszImportFile,
  USHORT           usImpMode
)
{
  BOOL             fOK = TRUE;
  BOOL             fFileAvail = TRUE;
  PSZ              pBuffer = NULL;
  int              iBufSize = 2048;
  BOOL             fUTF16 = FALSE;
  BOOL             fAsciiOrAnsi = FALSE;
  BOOL             fTMX = FALSE;
  BOOL             fXLIFF = FALSE;
  USHORT           usNewImpMode = 0;

  // allocate buffer
  fOK = UtlAlloc( (PVOID *)&pBuffer, 0, iBufSize, ERROR_STORAGE );

  // fill buffer
  {
    FILE *hfFile = fopen( pszImportFile, "rb" );
    if ( hfFile )
    {
      iBufSize = fread( pBuffer, 1, iBufSize-2, hfFile );
      if ( usImpMode == MEM_FORMAT_XLIFF_MT ) {
         for( int i=0 ; i<10 && iBufSize ; ++i ) { 
            if ( strstr( (PSZ)pBuffer, "<mda:meta type=\"OpenTM2\">" ) ) { 
               fXLIFF = TRUE ;
               break;
            }
            iBufSize = fread( pBuffer, 1, iBufSize-2, hfFile );
         }
      }
      fclose( hfFile );
    }
    else
    {
      fFileAvail = FALSE;
    } /* endif */
  }

  // try to evaluate file type
  if ( fFileAvail )
  {
    if ( wcsstr( (PSZ_W)pBuffer, L"<NTMMemoryDb>" ) != NULL )
    {
      fUTF16 = TRUE;
    }
    else if ( wcsstr( (PSZ_W)pBuffer, L"<NTMMEMORYDB>" ) != NULL )
    {
      fUTF16 = TRUE;
    }
    else if ( wcsstr( (PSZ_W)pBuffer, L"<ntmmemorydb>" ) != NULL )
    {
      fUTF16 = TRUE;
    }
    else if ( strstr( pBuffer, "<NTMMemoryDb>" ) != NULL )
    {
      fAsciiOrAnsi = TRUE;
    }
    else if ( strstr( pBuffer, "<NTMMEMORYDB>" ) != NULL )
    {
      fAsciiOrAnsi = TRUE;
    }
    else if ( strstr( pBuffer, "<ntmmemorydb>") != NULL )
    {
      fAsciiOrAnsi = TRUE;
    }
    else if ( strstr( pBuffer, "<tmx" ) != NULL )
    {
      fTMX = TRUE;
    }
    else if ( strstr( pBuffer, "<Tmx" ) != NULL )
    {
      fTMX = TRUE;
    }
    else if ( strstr( pBuffer, "<TMX" ) != NULL )
    {
      fTMX = TRUE;
    } /* endif */
  } /* endif */

  // compare evaluated import type (if any) with specified import type
  if ( fAsciiOrAnsi || fTMX || fXLIFF || fUTF16 )
  {
    PSZ pszSpecifiedType;
    PSZ pszEvaluatedType;

    // get text for specified import type
    switch( usImpMode )
    {
      case  MEM_SGMLFORMAT_ANSI:
            pszSpecifiedType = "SGML ANSI";
            break;
      case  MEM_SGMLFORMAT_UNICODE:
            pszSpecifiedType = "SGML UTF-16";
            break;
      case  MEM_FORMAT_TMX:
      case  MEM_FORMAT_TMX_TRADOS:
            pszSpecifiedType = "TMX";
            break;
      case  MEM_FORMAT_XLIFF_MT:
            pszSpecifiedType = "XLIFF";
            break;
      case  MEM_SGMLFORMAT_ASCII:
      default:
          pszSpecifiedType = "SGML ASCII";
          break;
    } /*endswitch */

    // get text for evaluated import type
    if ( fAsciiOrAnsi )
    {
      pszEvaluatedType = "SGML ASCII or SGML ANSI";
    }
    else if ( fUTF16 )
    {
      pszEvaluatedType = "SGML UTF-16";
      usNewImpMode = MEM_SGMLFORMAT_UNICODE;
    }
    else if ( fXLIFF) 
    {
      pszEvaluatedType = "XLIFF";
      usNewImpMode = MEM_FORMAT_XLIFF_MT;
    } 
    else     {
      pszEvaluatedType = "TMX";
      usNewImpMode = MEM_FORMAT_TMX;
    } /* endif */

    // compare import types
    if ( fAsciiOrAnsi )
    {
      PSZ pszParm[2];
      pszParm[0] = pszEvaluatedType;
      pszParm[1] = pszSpecifiedType;

      if ( (usImpMode != MEM_SGMLFORMAT_ANSI) && (usImpMode != MEM_SGMLFORMAT_ASCII) )
      {
        USHORT usResponse = UtlError( WARNING_MEMIMP_ASCIIORANSI,MB_OKCANCEL,
                                      2, pszParm, EQF_QUERY );
        if ( usResponse == MBID_CANCEL )
        {
          fOK = FALSE;
        } /* endif */

      } /* endif */
    }
    else if ( ( usImpMode != usNewImpMode ) &&
              ( ( usImpMode != MEM_FORMAT_TMX_TRADOS ) ||
                ( usNewImpMode != MEM_FORMAT_TMX     ) ) ) 
    {
      PSZ pszParm[3];
      USHORT usResponse = 0;

      pszParm[0] = pszEvaluatedType;
      pszParm[1] = pszSpecifiedType;
      pszParm[2] = pszEvaluatedType;

      usResponse = UtlError( WARNING_MEMIMP_WRONGFORMAT, MB_YESNOCANCEL,
                             3, pszParm, EQF_QUERY );

      if ( usResponse == MBID_YES )
      {
        pLIDA->usImpMode = usNewImpMode;
        pLIDA->fImpModeSet = TRUE;
      } 
      else if ( usResponse == MBID_CANCEL )
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } else 
  if ( ( usImpMode == MEM_FORMAT_XLIFF_MT ) &&
       ( usNewImpMode != MEM_FORMAT_TMX   ) ) 
  {
    UtlError( ERROR_MEMORY_IMPORT_XLIFF, MB_CANCEL, 0, NULL, EQF_ERROR );
    fOK = FALSE;
  } /* endif */



  // free buffer
  UtlAlloc( (PVOID *)&pBuffer, 0, 0, NOMSG );


  return( fOK );
} /* MemCheckFileType */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemLoadCommand                                           |
//+----------------------------------------------------------------------------+
//|Function call:     static MRESULT MemLoadCommand( HWND   hwnd, WPARAM mp1 ) |
//+----------------------------------------------------------------------------+
//|Description:       Function to handle the WM_COMMAND messages of the        |
//|                   dialog procedure MemLoadDlg                              |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND   hwnd  handle of dialog                            |
//|                   WPARAM mp1   message parameter 1                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       depends on message type                                  |
//|                   normal return codes are                                  |
//|                   TRUE  = message has been processed                       |
//|                   FALSE = message has not been processed                   |
//+----------------------------------------------------------------------------+
//|Side effects:      the following in MEM_IMPORT_IDA is changed               |
//|  szName       -filename of selected file (without extension)               |
//|  szExt        -extension of selected file                                  |
//|  szFilePath   -full qualified file to be imported                          |
//|  hFile        -file handle of file to be impotred or NULL if open fails    |
//|  szMemName    -name of TM to import to (from To TM combo box entry field)  |
//|  szMemPath    -full qualified TM to import to                              |
//|  hMem         -handle of TM to import to or NULL if TmOpen failed          |
//|  fDisabled    -dialog diabled flag                                         |
//+----------------------------------------------------------------------------+
//|Samples:           case ( WM_COMMAND ) :                                    |
//|                     mResult = MemLoadCommand( hwnd, mp1 );                 |
//|                     mbreak                                                 |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  switch( id of dialog control )                                            |
//|  {                                                                         |
//|     case CANCEL button   :                                                 |
//|     case ESC key pressed :                                                 |
//|        close dialog                                                        |
//|        break;                                                              |
//|     case IMPORT button :                                                   |
//|        get access to import ida                                            |
//|                                                                            |
//|        if ( IMPORT button is enabled and ida is accessed )                 |
//|        {                                                                   |
//|           disable IMPORT button and switch mouse pointer to hour glass     |
//|           call utility to check contet of Name entry field for correctness |
//|           set fOk in dependency of rc from utility                         |
//|                                                                            |
//|           if ( fOk )                                                       |
//|           {                                                                |
//|              get filename from content  of Name entryfield, search and     |
//|              select it in files listbox                                    |
//|                                                                            |
//|              if ( an item is selected in files listbox )                   |
//|              {                                                             |
//|                 get state of external format radio button                  |
//|                                                                            |
//|                 if ( external format radio button is selected )            |
//|                 {                                                          |
//|                    if ( no TM name in To TM combo box entry field )        |
//|                    {                                                       |
//|                       reset mouse pointer to arrow                         |
//|                       display error message                                |
//|                       set cursor to combo box entry field                  |
//|                    }                                                       |
//|                    else                                                    |
//|                    {                                                       |
//|                       create full qualified TM name and check if exist     |
//|                                                                            |
//|                       if ( TM does not exist )                             |
//|                       {                                                    |
//|                          set dialog disabled flag                          |
//|                          reset mouse pointer to arrow                      |
//|                          call TM handler to create new TM                  |
//|                          set mouse pointer to hour glass                   |
//|                          reset dialog disabled flag                        |
//|                                                                            |
//|                          if ( TM created without error )                   |
//|                          {                                                 |
//|                             display TM name in comb box entry field        |
//|                             update combo box                               |
//|                             get full path to created TM neede for TM open  |
//|                          }                                                 |
//|                          else                                              |
//|                             set cursor to combo box entry field            |
//|                       }                                                    |
//|                    }                                                       |
//|                    if ( TM created without error )                         |
//|                       open the TM                                          |
//|                    if ( TM successfully opened )                           |
//|                    {                                                       |
//|                       build full qualified file name of file to be imported|
//|                       open file to be imported                             |
//|                                                                            |
//|                       if ( error from open )                               |
//|                       {                                                    |
//|                          set file handle to NULL                           |
//|                          set error indicator                               |
//|                       }                                                    |
//|                    }                                                       |
//|                 }                                                          |
//|                 if ( error indicator is set )                              |
//|                 {                                                          |
//|                   save last used values and close properties               |
//|                   close dialog                                             |
//|                 }                                                          |
//|              }                                                             |
//|              else ( no item selected in files LB )                         |
//|              {                                                             |
//|                reset mouse pointer to arrow                                |
//|                display error message                                       |
//|              }                                                             |
//|                                                                            |
//|              if ( error indicator is set and TM was opened )               |
//|              {                                                             |
//|                close TM and set TM handle to NULL                          |
//|              }                                                             |
//|           }                                                                |
//|           reset mouse pointer to arrow                                     |
//|           enable buttons                                                   |
//|        }                                                                   |
//|        break;                                                              |
//|  }                                                                         |
//|  return;                                                                   |
//+----------------------------------------------------------------------------+
static MRESULT MemLoadCommand( HWND   hwnd, SHORT sID )
{
   PMEM_LOAD_DLG_IDA   pLIDA = NULL;     // Load dialog IDA
   BOOL        fOk = TRUE;               // process return code
   USHORT      usDosRc;                  // Return code from Dos functions
   USHORT      usAction;                 // Action for DosOpen
   PSZ         pszReplace[2];            // pointer to replace string UtlError /*@K0C*/
   USHORT      usResponse;               // return from UtlError               /*@K0A*/
   MRESULT     mResult = (MRESULT)FALSE; // function return value
   EQFINFO     ErrorInfo;                // returned from property handler

   switch ( sID )                   //switch on control ID
   {
     //------------------------------------------------------------------------
     case ( PID_PB_CANCEL ) :       //CANCEL button selected
     case ( DID_CANCEL ) :          //ESC key pressed
       POSTEQFCLOSE( hwnd, FALSE );
       break;
     //------------------------------------------------------------------------
     case ( PID_PB_OK ) :
       //--- Get access to IDA
       pLIDA = ACCESSDLGIDA(hwnd, PMEM_LOAD_DLG_IDA);
       MemoryFactory *pFactory = MemoryFactory::getInstance();

       //--- The IDA must be present and the Import button enabled
       //--- otherwise do not perform this function
       if ( pLIDA && ISCTRLENABLED( hwnd, PID_PB_OK ) )
       {
          //--- Disable the Import and Cancel button
          EnableOKAndCANCELButton( hwnd, FALSE );

          //--- Switch to hourglass pointer while checking the input
          SETCURSOR( SPTR_WAIT );

         // if multiple import files build list of import names as UtlEFValiditytest
         // will re-fill the files listbox thus removing any selections...
         {
           int iSelItems = SendDlgItemMessage( hwnd, DID_LD_FILES, LB_GETSELCOUNT, 0L, 0L );

           // free any previously allocated file list buffer
           if ( pLIDA->pszList )
           {
             UtlAlloc( (PVOID *)&(pLIDA->pszList), 0, 0, NOMSG );
             pLIDA->pszList = NULL;
           } /* endif */

           if ( iSelItems > 1 )
           {
             PSZ pszList = NULL;

             // allocate buffer for the selected names list
             BOOL fOK = UtlAlloc( (PVOID *)&pszList, 0, (MAX_LONGPATH*iSelItems)+10, ERROR_STORAGE );

             // fill-in names of selected import files
             if ( fOK )
              {
                PSZ pszCurPos = pszList;
                int iItems = iSelItems;
                SHORT sItem = LIT_FIRST;

                while ( iItems )
                {
                  sItem = QUERYNEXTSELECTION( hwnd, DID_LD_FILES, sItem );
                  if ( sItem >= 0 )
                  {
                    QUERYITEMTEXT( hwnd, DID_LD_FILES, sItem, pLIDA->szString );
                    strcpy( pszCurPos, pLIDA->szString );
                    pszCurPos += strlen( pszCurPos ) + 1;
                  } /* endif */
                  iItems--;
                } /* endwhile */
                *pszCurPos = EOS;          // terminate list
              } /* endif */
              pLIDA->pszList = pszList;
            } /* endif */
         }
          //--- call utility to check path entered im Name entry field
          // only for single selection!!!
          if ( !pLIDA->pszList )
          {
            pLIDA->ControlsIda.fCommand = TRUE;
            fOk = UtlEFValidityTest( &pLIDA->ControlsIda, hwnd );
            pLIDA->ControlsIda.fCommand = FALSE;
          } /* endif */

          if ( fOk )
          {
                                                                     /*10@L6D*/
             //--- Get state of ASCII format radio button


             pLIDA->fAscii = TRUE; // fAscii means import in external format ...
             {
               SHORT sItem = CBQUERYSELECTION( hwnd, DID_LD_FORMAT_CB );
               if ( sItem >= 0 )
               {
                 pLIDA->usImpMode = (USHORT)CBQUERYITEMHANDLE( hwnd, DID_LD_FORMAT_CB, sItem );
               }
               else
               {
                 // use default mode
                 pLIDA->usImpMode = MEM_SGMLFORMAT_ASCII;
               } /* endif */
             }

              // if multiple import files were selected use name of first selected file as TM name
              if ( pLIDA->pszList )
              {
                Utlstrccpy( pLIDA->szMemName, pLIDA->pszList, DOT );
                SETTEXT( hwnd, DID_LD_TO, pLIDA->szMemName );
                UtlSplitFnameFromPath( pLIDA->ControlsIda.szPathContent );
                strcat( pLIDA->ControlsIda.szPathContent, BACKSLASH_STR );
                strcat( pLIDA->ControlsIda.szPathContent, pLIDA->pszList );
              } /* endif */

              if ( fOk )
              {
                //--- if no TM name is specified in the To TM entry filed
                if( !QUERYTEXT( hwnd, DID_LD_TO, pLIDA->szMemName ))
                {
                   //--- Reset hour glass arrow pointer
                   SETCURSOR( SPTR_ARROW );

                   //--- Display error message that no translation memory name
                   //--- has been specified
                   UtlError( ERROR_NO_MEM_NAME, MB_CANCEL, 0, NULL, EQF_WARNING );

                   //--- Set cursor to the incorrect field
                   SETFOCUS( hwnd, DID_LD_TO );
                   fOk = FALSE;
                }
                else
                {
                  ANSITOOEM( pLIDA->szMemName );

                  if ( !UtlCheckLongName( pLIDA->szMemName ) )
                  {
                     //--- Reset hour glass arrow pointer
                     SETCURSOR( SPTR_ARROW );

                     //--- Display error message that incorrect translation
                     //--- memory name has been specified
                     OEMTOANSI( pLIDA->szMemName );
                     pszReplace[0] = pLIDA->szMemName;
                     UtlError(  ERROR_INV_LONGNAME, MB_CANCEL, 1,
                                pszReplace, EQF_ERROR );

                     //--- Set cursor to the incorrect field
                     SETFOCUS( hwnd, DID_LD_TO );
                     fOk = FALSE;
                  }
                  else
                  {
                    if ( !pFactory->exists( NULL, pLIDA->szMemName ) )
                    {
                        pLIDA->fDisabled = TRUE;
                        SETCURSOR( SPTR_ARROW );

                        //--- call the memory handler to create a new TM
                        fOk = (BOOL)EqfSend2Handler( MEMORYHANDLER, WM_EQF_CREATE, MP1FROMSHORT( 0 ),  MP2FROMP( pLIDA->szMemName ));

                        SETCURSOR( SPTR_WAIT );
                        pLIDA->fDisabled = FALSE;

                        if ( fOk )   //--- TM created without error
                        {
                           //--- Assign TM name which was returned by the TM create dialog to the TM name field
                           OEMTOANSI( pLIDA->szMemName );
                           SETTEXT( hwnd, DID_LD_TO, pLIDA->szMemName );
                           ANSITOOEM( pLIDA->szMemName );

                           //--- Update To TM combo box
                           DELETEALL( hwnd, DID_LD_TO );
                           EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND( WinWindowFromID( hwnd, DID_LD_TO) ), MP2FROMP( MEMORY_ALL ) );
                        }
                        else  //--- error creating TM
                        {
                          //--- Set cursor to TM field
                          SETFOCUS( hwnd, DID_LD_TO );
                        } /* endif */
                    }
                    else
                    {
                       //--- display merge confirmation message
                       OEMTOANSI( pLIDA->szMemName );
                       pszReplace[0] = pLIDA->szMemName;
                       pszReplace[1] = pLIDA->ControlsIda.szPathContent;
                       usResponse = UtlError( MEM_LOAD_MERGE_CONFIRMATION, MB_YESNO, 2, &pszReplace[0], EQF_QUERY );
                       ANSITOOEM( pLIDA->szMemName );

                       //--- if file should not merged into existing TM
                       if ( usResponse == MBID_NO )
                       {
                         //--- clear to TM name entry filed and set focus to it
                         SETTEXT( hwnd, DID_LD_TO, EMPTY_STRING );
                         SETFOCUS( hwnd, DID_LD_TO );
                         //--- stop further processing
                         fOk = FALSE;
                       } /* endif */
                    } /* endif */
                  } /* endif */
                }

                if ( fOk )   //--- TM created without error
                {
                  int iRC = 0;
                  pFactory->openMemory( NULL, pLIDA->szMemName, EXCLUSIVE, &iRC );

                  fOk = iRC == 0;

                  /* if user kicks on organize we have to get rid of our dialog - let him invoke the dialog again*/
                  if ( iRC == TM_WAS_CORRUPTED_IS_ORGANIZED )
                  {
                    //--- close dialog
                    //--- the handler which called this dialog invokes the export
                    //--- process in dependend on the file handle in the IDA
                    POSTEQFCLOSE( hwnd, FALSE );
                  } /* endif */
                } /* endif */

                if ( fOk )  //--- TM opened without error
                {
                   //--- copy complete file spec to IDA
                   strcpy( pLIDA->szFilePath, pLIDA->ControlsIda.szPathContent );

                   //--- Open the file to be imported and store file handle.
                   usDosRc = UtlOpen ( pLIDA->ControlsIda.szPathContent, //filename
                                     &(pLIDA->hFile),    //file handle (out)
                                     &usAction,    // action taken (out)
                                     0L,           // Create file size
                                     FILE_NORMAL,  // Normal attributes
                                     FILE_OPEN,    // Open if exists else fail
                                     OPEN_SHARE_DENYWRITE,// Deny Write access
                                     0L,           // Reserved but handle errors
                                     TRUE );       // display error message
                   if ( usDosRc )  //--- error from open
                   {
                      //--- Reset file handle to NULL and set fOk to FALSE
                      pLIDA->hFile = NULLHANDLE;
                      fOk = FALSE;
                   }/* endif */
                } /* endif */
             }

             if ( fOk )                                           /* @A8A */
             {                                                    /* @A8A */
               fOk = MemOpenProp(&(pLIDA->hPropLast),             /* @A8A */
                                 (PVOID *)&(pLIDA->pPropLast),    /* @A8A */
                                 MEMORYLU_PROPERTIES_NAME,        /* @A8A */
                                 pLIDA->szDummy,                  /* @A8A */
                                 PROP_ACCESS_WRITE,               /* @A8A */
                                 TRUE );                          /* @A8A */
             } /* endif */                                        /* @A8A */

             //--- if no error until now save last used values
             if ( fOk )
             {
               //--- save last used path x:\xxx\xxx?
               if ( strlen(pLIDA->ControlsIda.szPath) >= sizeof(pLIDA->pPropLast->szImpPathLastUsed) )
               {
                 strncpy( pLIDA->pPropLast->szImpPathLastUsed2,
                          pLIDA->ControlsIda.szPath,
                          sizeof(pLIDA->pPropLast->szImpPathLastUsed)-1 );
                 pLIDA->pPropLast->szImpPathLastUsed2[sizeof(pLIDA->pPropLast->szImpPathLastUsed)-1] = EOS;
               }
               else
               {
                 strcpy( pLIDA->pPropLast->szImpPathLastUsed,
                         pLIDA->ControlsIda.szPath );
               } /* endif */
               pLIDA->pPropLast->usImpMode = pLIDA->usImpMode;
               SaveProperties( pLIDA->hPropLast, &ErrorInfo);

               //--- close dialog
               //--- the handler which called this dialog invokes the export
               //--- process in dependend on the file handle in the IDA
               POSTEQFCLOSE( hwnd, FALSE );
             } /* endif */
                                                                      /*11@L6D*/
             if ( !fOk && (pLIDA->pMem != NULL) )  //--- if an error occured and the TM
             {                           //--- was opened
               //--- close the TM
               pFactory->closeMemory( pLIDA->pMem );
               pLIDA->pMem = NULL;
             } /* endif */

          } /* endif */
          SETCURSOR( SPTR_ARROW );
          EnableOKAndCANCELButton( hwnd, TRUE );
       } /* endif */
       break;
   } /*end switch*/
   return mResult;
} /* end of function MemLoadCommand */

// memory import using standard file open dialog
BOOL MemImportFileOpenDialog
(
  PMEM_LOAD_DLG_IDA   pLIDA
)
{
  BOOL fOK = TRUE;
  EQFINFO ErrorInfo;
  OPENFILENAME OpenStruct;
  PSZ pszFileNameBuf = NULL;
  int iFileBufSize = 0xFFF0;                     // size to use for file name buffer

  // initial processing
  pLIDA->hFile = NULLHANDLE;                     // set file handle to NULL
  pLIDA->usImpMode = MEM_SGMLFORMAT_ASCII;       // set default value
  memset( &OpenStruct, 0, sizeof(OpenStruct) );
  OpenStruct.lStructSize = sizeof(OpenStruct);
  OpenStruct.hwndOwner = QUERYACTIVEWINDOW();

  // access last used values
  if ( fOK )
  {
    //--- get system path
    UtlMakeEQFPath( pLIDA->szDummy, NULC, SYSTEM_PATH, NULL );
    fOK = MemOpenProp( &(pLIDA->hPropLast),(PVOID *)&(pLIDA->pPropLast), MEMORYLU_PROPERTIES_NAME, 
                       pLIDA->szDummy, PROP_ACCESS_READ, TRUE ); 
  } /* endif */

  // apply last used values
  if ( fOK )
  {
    // set last used path
    if ( pLIDA->pPropLast->szImpPathLastUsed2[0] != EOS )
    {
      strncpy( pLIDA->ControlsIda.szSavedPath, pLIDA->pPropLast->szImpPathLastUsed2, sizeof(pLIDA->ControlsIda.szSavedPath)-1 );
    }
    else
    {
      strncpy( pLIDA->ControlsIda.szSavedPath, pLIDA->pPropLast->szImpPathLastUsed,sizeof(pLIDA->ControlsIda.szSavedPath)-1 );
    } /* endif */
    pLIDA->ControlsIda.szSavedPath[sizeof(pLIDA->ControlsIda.szSavedPath)-1] = EOS;
    OpenStruct.lpstrInitialDir = pLIDA->ControlsIda.szSavedPath;

    pLIDA->usImpMode = pLIDA->pPropLast->usImpMode;
    
    // fill import formats select last used format
    OpenStruct.lpstrFilter = "SGML ANSI (*.EXP)\0*.EXP\0SGML ASCII (*.EXP)\0*.EXP\0SGML UTF16 (*.EXP)\0*.EXP\0TMX (*.TMX)\0*.TMX\0TMX (Trados) (*.TMX)\0*.TMX\0XLIFF (MT) (*.XLF)\0*.XLF\0\0";
    OpenStruct.lpstrCustomFilter = NULL;
    switch ( pLIDA->usImpMode )
    {
      case MEM_SGMLFORMAT_ANSI:    OpenStruct.nFilterIndex = 1; break;
      case MEM_SGMLFORMAT_ASCII:   OpenStruct.nFilterIndex = 2; break;
      case MEM_SGMLFORMAT_UNICODE: OpenStruct.nFilterIndex = 3; break;
      case MEM_FORMAT_TMX:         OpenStruct.nFilterIndex = 4; break;
      case MEM_FORMAT_TMX_TRADOS:  OpenStruct.nFilterIndex = 5; break;
      case MEM_FORMAT_XLIFF_MT:    OpenStruct.nFilterIndex = 6; break;
      default:                     OpenStruct.nFilterIndex = 1; break;
    } /*endswitch */
    pLIDA->fImpModeSet = FALSE;
  } /*end if*/

  // close last used properties if open
  if ( pLIDA->hPropLast )                                     
  {                                                           
    CloseProperties( pLIDA->hPropLast, PROP_QUIT, &ErrorInfo);
    pLIDA->hPropLast = NULL;                                  
  } /*end if*/

  // prepare and show standard file open dialog
  OpenStruct.lpstrFile = pLIDA->ControlsIda.szSelectedName;
  OpenStruct.nMaxFile = sizeof(pLIDA->ControlsIda.szSelectedName)-1;
  OpenStruct.lpstrFileTitle = NULL;
  OpenStruct.nMaxFileTitle = 0;
  OpenStruct.lpstrTitle = "Memory Import";
  OpenStruct.lpfnHook = MemOpenFileHook;
  OpenStruct.lCustData = (LONG)pLIDA;
  OpenStruct.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_ALLOWMULTISELECT |
                     OFN_ENABLEHOOK | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;

  // allocate buffer for file names
  fOK = UtlAlloc( (PVOID *)&pszFileNameBuf, 0, iFileBufSize, ERROR_STORAGE );

  if ( fOK )
  {
    OpenStruct.lpstrFile = pszFileNameBuf;
    OpenStruct.nMaxFile = iFileBufSize-1;

    if ( GetOpenFileName( &OpenStruct ) )
    {
      fOK = TRUE;

      // get import format 
      pLIDA->fAscii = TRUE; // fAscii means import in external format ...
      if ( !pLIDA->fImpModeSet )
      {
        switch ( OpenStruct.nFilterIndex )
        {
          case 1:  pLIDA->usImpMode = MEM_SGMLFORMAT_ANSI; break;
          case 2:  pLIDA->usImpMode = MEM_SGMLFORMAT_ASCII; break;
          case 3:  pLIDA->usImpMode = MEM_SGMLFORMAT_UNICODE; break;
          case 4:  pLIDA->usImpMode = MEM_FORMAT_TMX; break;
          case 5:  pLIDA->usImpMode = MEM_FORMAT_TMX_TRADOS; break;
          case 6:  pLIDA->usImpMode = MEM_FORMAT_XLIFF_MT; break;
          default: pLIDA->usImpMode = MEM_SGMLFORMAT_ANSI; break;
        } /*endswitch */
      } /* endif */

      // open last used values property file
      if ( fOK )
      {                                        
        fOK = MemOpenProp( &(pLIDA->hPropLast), (PVOID *)&(pLIDA->pPropLast), MEMORYLU_PROPERTIES_NAME, 
                          pLIDA->szDummy, PROP_ACCESS_WRITE, TRUE );  
      } /* endif */ 

      // if no error until now save last used values
      if ( fOK )
      {
        //--- save last used path x:\xxx\xxx?
        if ( strlen(pLIDA->ControlsIda.szPath) >= sizeof(pLIDA->pPropLast->szImpPathLastUsed) )
        {
          strncpy( pLIDA->pPropLast->szImpPathLastUsed2,
                  pLIDA->ControlsIda.szPath,
                  sizeof(pLIDA->pPropLast->szImpPathLastUsed)-1 );
          pLIDA->pPropLast->szImpPathLastUsed2[sizeof(pLIDA->pPropLast->szImpPathLastUsed)-1] = EOS;
        }
        else
        {
          strcpy( pLIDA->pPropLast->szImpPathLastUsed,
                  pLIDA->ControlsIda.szPath );
        } /* endif */
        pLIDA->pPropLast->usImpMode = pLIDA->usImpMode;
        SaveProperties( pLIDA->hPropLast, &ErrorInfo );
      } /* endif */
    }
    else
    {
      fOK = FALSE;
    } /* endif */

    if ( pszFileNameBuf ) UtlAlloc( (PVOID *)&pszFileNameBuf, 0, 0, NOMSG );

  } /* endif */

  if ( pLIDA->hPropLast ) CloseProperties( pLIDA->hPropLast, PROP_QUIT, &ErrorInfo);
  return( fOK );

} /* end of function */

//
// hook procedure for standard file open dialog
//
UINT_PTR CALLBACK MemOpenFileHook
(
  HWND hdlg,      // handle to child dialog box
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
)
{
  UINT uiResult = 0;
  wParam;

  switch ( uiMsg )
  {
    case WM_INITDIALOG:
      {
        BOOL fOK = TRUE;
        LPOPENFILENAME pOf = (LPOPENFILENAME)lParam;
        HWND hwndDialog = NULLHANDLE;
        HWND hwndFormatStatic = NULLHANDLE;
        HWND hwndReadOnlyCheck = NULLHANDLE;
        HWND hwndToStatic  = NULLHANDLE;
        HWND hwndFormatCombo = NULLHANDLE;
        HWND hwndToCombo = NULLHANDLE;
        RECT rctFormatStatic, rctReadOnlyCheck, rctFormatCombo;


        // get LOAD IDA
        PMEM_LOAD_DLG_IDA pLIDA = (PMEM_LOAD_DLG_IDA)pOf->lCustData;

        // intialize RECT structures
        memset( &rctReadOnlyCheck, 0, sizeof(rctReadOnlyCheck) );
        memset( &rctFormatStatic, 0, sizeof(rctFormatStatic) );
        memset( &rctFormatCombo, 0, sizeof(rctFormatCombo) );

        // get dialog handle
        hwndDialog = GetParent( hdlg );
        fOK = (hwndDialog != NULLHANDLE);

        // change to English labelled controls
        SetDlgItemText( hwndDialog, 1091, "Look &in:" );
        SetDlgItemText( hwndDialog, 1090, "File &name:" );
        SetDlgItemText( hwndDialog, 1089, "&Format:" );
        SetDlgItemText( hwndDialog, IDCANCEL, "Cancel" );
        SetDlgItemText( hwndDialog, IDOK, "&Import" );

        // get handle of dialog controls
        if ( fOK )
        {
          hwndReadOnlyCheck = GetDlgItem( hwndDialog, 1040 );
          hwndFormatStatic  = GetDlgItem( hwndDialog, 1089 );
          hwndFormatCombo   = GetDlgItem( hwndDialog, 1136 );
          fOK = (hwndReadOnlyCheck != NULLHANDLE) && 
                (hwndFormatStatic != NULLHANDLE) && 
                (hwndFormatCombo != NULLHANDLE) ; 
        } /* endif */

        // get size and position of dialog controls and map to window coordinates
        if ( fOK )
        {
          GetWindowRect( hwndReadOnlyCheck, &rctReadOnlyCheck );
          MapWindowPoints( HWND_DESKTOP, hwndDialog, (LPPOINT)&rctReadOnlyCheck, 2 );
          GetWindowRect( hwndFormatStatic,  &rctFormatStatic );
          MapWindowPoints( HWND_DESKTOP, hwndDialog, (LPPOINT)&rctFormatStatic, 2 );
          GetWindowRect( hwndFormatCombo,   &rctFormatCombo );
          MapWindowPoints( HWND_DESKTOP, hwndDialog, (LPPOINT)&rctFormatCombo, 2 );
        } /* endif */

        // create "To" static
        hwndToStatic = CreateWindow( "STATIC", "&To memory:", WS_CHILD | WS_VISIBLE | SS_LEFT, 
                                     rctFormatStatic.left, rctReadOnlyCheck.top, 
                                     rctFormatStatic.right - rctFormatStatic.left,
                                     rctFormatStatic.bottom - rctFormatStatic.top,
                                     hwndDialog, 0, (HINSTANCE)UtlQueryULong( QL_HAB ),
                                     0 );
         if ( hwndToStatic ) 
         {
           SetWindowLong( hwndToStatic, GWL_ID, DID_LD_TO_TEXT );
           SetCtrlFnt( hwndDialog, GetCharSet(), DID_LD_TO_TEXT, 0 );
         } /* endif */

        // create "To" combo
        hwndToCombo = CreateWindow( "COMBOBOX", "", 
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_SORT | CBS_AUTOHSCROLL, 
                                     rctFormatCombo.left, rctReadOnlyCheck.top - 2, 
                                     rctFormatCombo.right - rctFormatCombo.left,
                                     120,
                                     hwndDialog, 0, (HINSTANCE)UtlQueryULong( QL_HAB ),
                                     0 );
         if ( hwndToCombo ) 
         {
           SetWindowLong( hwndToCombo, GWL_ID,  DID_LD_TO );
           SetCtrlFnt( hwndDialog, GetCharSet(),  DID_LD_TO, 0 );
         } /* endif */

        // hide read-only checkbox
        ShowWindow( hwndReadOnlyCheck, SW_HIDE );

        // correct Z_order of controls
        SetWindowPos( hwndToCombo, hwndFormatCombo, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOSENDCHANGING );

        // fill to combo box
        EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES, (WPARAM)hwndToCombo, 0L );

        // remember handle of combo box
        pLIDA->hwndToCombo = hwndToCombo;

        // select the memory
        {
          SHORT sItem = 0;
          CBSEARCHSELECTHWND( sItem, hwndToCombo, pLIDA->szMemName );
        }
      }
      break;

    case WM_NOTIFY:
      {
        LPOFNOTIFY pOfNotify = (LPOFNOTIFY)lParam;
        if ( pOfNotify)
        {
          BOOL fOk = TRUE;
          HWND hwndDialog = GetParent( hdlg );
          LPOPENFILENAME pOf = pOfNotify->lpOFN;
          PMEM_LOAD_DLG_IDA pLIDA = (PMEM_LOAD_DLG_IDA)pOf->lCustData;
          int iItems = 0;

          switch ( pOfNotify->hdr.code )
          {
            case CDN_SELCHANGE :
              // selection change in file list, use first selected file as memory name
                           
              // get name of first selected file 
              pLIDA->ControlsIda.szSelectedName[0] = EOS;
              SendMessage( hwndDialog, CDM_GETSPEC, (WPARAM)sizeof(pLIDA->ControlsIda.szSelectedName),
                           (LPARAM)pLIDA->ControlsIda.szSelectedName );

              if ( pLIDA->ControlsIda.szSelectedName[0] )
              {
                // use file name up to DOT as memory name
                if ( pLIDA->ControlsIda.szSelectedName[0] == '\"' )
                {
                  PSZ pszSource = pLIDA->ControlsIda.szSelectedName + 1;
                  PSZ pszTarget = pLIDA->szMemName;
                  while ( *pszSource && (*pszSource != DOT) && (*pszSource != '\"') ) *pszTarget++ = *pszSource++;
                  *pszTarget = EOS;

                  // check if there are more names to follow
                  while ( *pszSource && (*pszSource != '\"') ) pszSource++;
                  while ( *pszSource && (*pszSource != '\"') ) pszSource++;
                  if ( *pszSource == '\"' )
                  {
                    iItems = 2; //  2 or more files selected
                  }
                  else
                  {
                    iItems = 1; //  only one file selected
                  } /* endif */

                }
                else
                {
                  Utlstrccpy( pLIDA->szMemName, pLIDA->ControlsIda.szSelectedName, DOT );
                  iItems = 1;
                } /* endif */
              } /* endif */
              SETTEXTHWND( pLIDA->hwndToCombo, pLIDA->szMemName );

              // hide or show combo depending on selected items
              if ( iItems > 1 )
              {
                ShowWindow( pLIDA->hwndToCombo, SW_HIDE );
              }
              else
              {
                ShowWindow( pLIDA->hwndToCombo, SW_SHOW );
              } /* endif */
              break;

            case CDN_FILEOK :
              // user pressed OK button

              // get currently selected folder
              pLIDA->ControlsIda.szSavedPath[0] = EOS;
              SendMessage( hwndDialog, CDM_GETFOLDERPATH, (WPARAM)sizeof(pLIDA->ControlsIda.szSavedPath),
                          (LPARAM)pLIDA->ControlsIda.szSavedPath );
              strcpy( pLIDA->ControlsIda.szPath, pLIDA->ControlsIda.szSavedPath );
              strcpy( pLIDA->ControlsIda.szPathContent, pLIDA->ControlsIda.szSavedPath );

              // free any previously allocated file list buffer
              if ( pLIDA->pszList )
              {
                UtlAlloc( (PVOID *)&(pLIDA->pszList), 0, 0, NOMSG );
                pLIDA->pszList = NULL;
              } /* endif */

              // get selected files
              {
                // allocate a rather large file list buffer as we do not know how much files are selected
                int iAlloc = 0xFFF0;
                int iStrLen = 0;
                int iItems = 0;
                UtlAlloc( (PVOID *)&(pLIDA->pszList), 0, iAlloc, ERROR_STORAGE );

                // get currently selected file(s)
                iStrLen = SendMessage( hwndDialog, CDM_GETSPEC, (WPARAM)iAlloc, (LPARAM)pLIDA->pszList );

                // count the number of files
                if ( iStrLen > 0 )
                {
                  // if the list starts with a double-quote look we have files enclosed in
                  // double quotes, convert them to a 0 terminated list
                  if ( pLIDA->pszList[0] == '\"' )
                  {
                    PSZ pszSource = pLIDA->pszList + 1; 
                    PSZ pszTarget = pLIDA->pszList; 
                    while ( *pszSource )
                    {
                      // copy up to end of current file
                      while ( *pszSource && (*pszSource != '\"') ) *pszTarget++ = *pszSource++;

                      // skip delimiters up to next file
                      if ( *pszSource == '\"' ) pszSource++;
                      while ( *pszSource && (*pszSource != '\"') ) pszSource++;
                      if ( *pszSource == '\"' ) pszSource++;

                      *pszTarget++ = EOS;          // terminate current file
                      iItems++;
                    } /*endwhile */
                    *pszTarget++ = EOS;            // terminate list
                  }
                  else
                  {
                   iItems = 1;
                  } /* endif */

                  // setup fully qualified path to first file
                  strcpy( pLIDA->ControlsIda.szPathContent, pLIDA->ControlsIda.szSavedPath );
                  strcat( pLIDA->ControlsIda.szPathContent, "\\" );
                  strcat( pLIDA->ControlsIda.szPathContent, pLIDA->pszList );
                  strcpy( pLIDA->szFilePath, pLIDA->ControlsIda.szPathContent );

                  // for a single file free pszList  
                  if ( iItems == 1 )
                  {
                    UtlAlloc( (PVOID *)&(pLIDA->pszList), 0, 0, NOMSG );
                    pLIDA->pszList = NULL;
                  } /* endif */
                } /* endif */
              }

              // if multiple import files were selected use name of first selected file as TM name
              if ( pLIDA->pszList )
              {
                Utlstrccpy( pLIDA->szMemName, pLIDA->pszList, DOT );
                SETTEXTHWND( pLIDA->hwndToCombo, pLIDA->szMemName );
                UtlSplitFnameFromPath( pLIDA->ControlsIda.szPathContent );
                strcat( pLIDA->ControlsIda.szPathContent, BACKSLASH_STR );
                strcat( pLIDA->ControlsIda.szPathContent, pLIDA->pszList );
              } /* endif */

              // check if import type matches our data
              if ( fOk )
              {
                switch ( pOf->nFilterIndex )
                {
                  case 1:  pLIDA->usImpMode = MEM_SGMLFORMAT_ANSI; break;
                  case 2:  pLIDA->usImpMode = MEM_SGMLFORMAT_ASCII; break;
                  case 3:  pLIDA->usImpMode = MEM_SGMLFORMAT_UNICODE; break;
                  case 4:  pLIDA->usImpMode = MEM_FORMAT_TMX; break;
                  case 5:  pLIDA->usImpMode = MEM_FORMAT_TMX_TRADOS; break;
                  case 6:  pLIDA->usImpMode = MEM_FORMAT_XLIFF_MT; break;
                  default: pLIDA->usImpMode = MEM_SGMLFORMAT_ANSI; break;
                } /*endswitch */

                fOk = MemCheckFileType( pLIDA, pLIDA->ControlsIda.szPathContent, pLIDA->usImpMode );
                if ( !fOk )
                {
                  SETFOCUSHWND( pLIDA->hwndToCombo );
                  uiResult = 1;
                  SetWindowLong( hdlg, DWL_MSGRESULT, WARNING_MEMIMP_WRONGFORMAT ); 
                } /* endif */
              } /* endif */

              if ( fOk )
              {
                // check selected to memory
                if( !QUERYTEXTHWND( pLIDA->hwndToCombo, pLIDA->szMemName ) )
                {
                  UtlErrorHwnd( ERROR_NO_MEM_NAME, MB_CANCEL, 0, NULL, EQF_WARNING, hdlg );
                  SETFOCUSHWND( pLIDA->hwndToCombo );
                  uiResult = 1;
                  SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_NO_MEM_NAME ); 
                }
                else
                {
                  ANSITOOEM( pLIDA->szMemName );

                  if ( !UtlCheckLongName( pLIDA->szMemName ) )
                  {
                    PSZ pszReplace[2];
                    // display error message that incorrect translation memory name has been specified
                    OEMTOANSI( pLIDA->szMemName );
                    pszReplace[0] = pLIDA->szMemName;
                    UtlErrorHwnd(  ERROR_INV_LONGNAME, MB_CANCEL, 1, pszReplace, EQF_ERROR, hdlg );
                    SETFOCUSHWND( pLIDA->hwndToCombo );
                    uiResult = 1;
                    SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_INV_LONGNAME ); 
                  }
                  else
                  {
                    MemoryFactory *pFactory = MemoryFactory::getInstance();

                    if ( !pFactory->exists( NULL, pLIDA->szMemName ) )
                    {
                      // call the memory handler to create a new TM
                      fOk = (BOOL)EqfSend2Handler( MEMORYHANDLER, WM_EQF_CREATE, 0, MP2FROMP( pLIDA->szMemName ) );

                      if ( fOk )   //--- TM created without error
                      {
                        // assign TM name which was returned by the TM create dialog to the TM name field
                        OEMTOANSI( pLIDA->szMemName );
                        SETTEXTHWND( pLIDA->hwndToCombo, pLIDA->szMemName );
                        ANSITOOEM( pLIDA->szMemName );

                        // update To TM combo box
                        DELETEALLHWND( pLIDA->hwndToCombo );
                        EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND( pLIDA->hwndToCombo  ), MP2FROMP( MEMORY_ALL ) );
                      }
                      else  //--- error creating TM
                      {
                        SETFOCUSHWND( pLIDA->hwndToCombo  );
                        uiResult = 1;
                        SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_INV_LONGNAME ); 
                      } /* endif */
                    }
                    else
                    {
                      PSZ pszReplace[2];
                      USHORT usResponse;

                      strcpy( pLIDA->szMemPath, pLIDA->szShortMemName );
                      pLIDA->fMerge = TRUE;

                      //  display merge confirmation message
                      pszReplace[0] = pLIDA->szMemName;
                      pszReplace[1] = pLIDA->ControlsIda.szPathContent;
                      OEMTOANSI( pLIDA->szMemName );
                      if ( pLIDA->pszList )
                      {
                        usResponse = UtlErrorHwnd( MEM_LOAD_MERGE_CONFIRMATION, MB_EQF_YESTOALL, 2, &pszReplace[0], EQF_QUERY, hdlg );
                      }
                      else
                      {
                        usResponse = UtlErrorHwnd( MEM_LOAD_MERGE_CONFIRMATION, MB_YESNO, 2, &pszReplace[0], EQF_QUERY, hdlg );
                      } /* endif */
                      ANSITOOEM( pLIDA->szMemName );

                      if ( usResponse == MBID_EQF_YESTOALL )
                      {
                        pLIDA->fYesToAll = TRUE;
                        usResponse = MBID_YES;
                      } /* endif */

                      // if file should not merged into existing TM
                      if ( (usResponse == MBID_NO) || (usResponse == MBID_CANCEL)  )
                      {
                        //--- clear to TM name entry filed and set focus to it
                        SETTEXTHWND( pLIDA->hwndToCombo, EMPTY_STRING );
                        SETFOCUSHWND( pLIDA->hwndToCombo );
                        uiResult = 1;
                        SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_INV_LONGNAME ); 
                        fOk = FALSE;
                      } /* endif */
                    } /* endif */
                  } /* endif */
                }
              } /* endif */

              if ( fOk )   //--- TM created without error
              {
                // open TM
                MemoryFactory *pFactory = MemoryFactory::getInstance();
                int iRC = 0;
                pLIDA->pMem = pFactory->openMemory( NULL, pLIDA->szMemName, EXCLUSIVE, &iRC );
                fOk = iRC == 0;

                // if user kicks on organize we have to get rid of our dialog - let him invoke the dialog again
                if ( iRC == TM_WAS_CORRUPTED_IS_ORGANIZED )
                {
                  // close dialog
                  PostMessage( hwndDialog, WM_COMMAND, MP1FROMSHORT(IDCANCEL), 0 );
                } /* endif */
              } /* endif */

              if ( fOk )  // tM opened without error
              {
                USHORT usDosRc, usAction;

                // copy complete file spec to IDA
                strcpy( pLIDA->szFilePath, pLIDA->ControlsIda.szPathContent );
                // open the file to be imported and store file handle.
                usDosRc = UtlOpenHwnd( pLIDA->ControlsIda.szPathContent, &(pLIDA->hFile), &usAction, 0L,
                                       FILE_NORMAL, FILE_OPEN, OPEN_SHARE_DENYWRITE, 0L, TRUE, hdlg ); 
                if ( usDosRc )  //--- error from open
                {
                  //--- Reset file handle to NULL and set fOk to FALSE
                  pLIDA->hFile = NULLHANDLE;
                  uiResult = 1;
                  SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_INV_LONGNAME ); 
                  fOk = FALSE;
                }/* endif */
              } /* endif */
            }

            if ( !fOk && pLIDA->pMem != NULL )  // if an error occured and the TM is open close the TM 
            {                                         
              MemoryFactory *pFactory = MemoryFactory::getInstance();
              pFactory->closeMemory( pLIDA->pMem );
              pLIDA->pMem = NULL;
            } /* endif */
        } /* endif */
      }

      break;
    case WM_DESTROY:
      {
        HWND hwndDialog = GetParent( hdlg );
        DelCtrlFont( hwndDialog, DID_LD_TO_TEXT );
        DelCtrlFont( hwndDialog, DID_LD_TO );
      }
      break;
    default:
        break;
  } /*endswitch */
  return( uiResult );
} /* end of function MemOpenFileHook */

