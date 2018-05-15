//+----------------------------------------------------------------------------+
//|EQFMEMLD.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:  Markus Conrad, Stefan Doersam                                      |
//+----------------------------------------------------------------------------+
//|Description: Dialog to load (import) a Translation Memory                   |
//+----------------------------------------------------------------------------+
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
                // use file name up to last DOT as memory name
                if ( pLIDA->ControlsIda.szSelectedName[0] == '\"' )
                {
                  // copy first name to memory name field and remove file extension
                  Utlstrccpy( pLIDA->szMemName , pLIDA->ControlsIda.szSelectedName + 1, '\"' );
                  PSZ pszExt = strrchr( pLIDA->szMemName, DOT );
                  if ( pszExt != NULL ) *pszExt = EOS;

                  // check if there are more names to follow
                  PSZ pszNextName = pLIDA->ControlsIda.szSelectedName + 1; 
                  while ( *pszNextName && (*pszNextName != '\"') ) pszNextName++;
                  if ( *pszNextName == '\"' ) pszNextName++;
                  while ( *pszNextName && (*pszNextName != '\"') ) pszNextName++;
                  if ( *pszNextName == '\"' )
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
                  strcpy( pLIDA->szMemName, pLIDA->ControlsIda.szSelectedName );
                  PSZ pszExt = strrchr( pLIDA->szMemName, DOT );
                  if ( pszExt != NULL ) *pszExt = EOS;
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
                strcpy( pLIDA->szMemName, pLIDA->pszList );
                PSZ pszExt = strrchr( pLIDA->szMemName, DOT );
                if ( pszExt != NULL ) *pszExt = EOS;
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

