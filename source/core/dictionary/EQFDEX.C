//
/// *************************** Prolog *********************************
//
//               Copyright (C) 1990-2015, International Business Machines      
//               Corporation and others. All rights reserved         
//
//  Short description: Dialog to export a dictionary.
//
//  Author:
//
//  Description:       This program provides the end user dialog
//                     to provide the name of the file to be loaded
//                     to create a SGML-based file.
//
//  Calling Hierarchy:
//
//  Entry point:  DictExpDlg()
//
//  Changes:
//
// *********************** End Prolog *********************************

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_ANALYSIS         // analysis functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqfdtag.h>              // include tag definitions
#include <eqfdde.h>
#include "OtmDictionaryIF.H"
#include "eqfdicti.h"             // Private include file of dictionary handler
#include "eqfrdics.h"
#include "eqfdex.id"
#include "eqfdimp.id"
#include "eqfdic00.id"
#ifdef FUNCCALLIF
  #include "OTMFUNC.H"            // public defines for function call interface
  #include "EQFFUNCI.H"           // private defines for function call interface
#endif
  #include "SHLOBJ.H"           // folder browse function

#define M_SIZE     0xFF00          // maximum size
#define MAX_USER     20            // max number of user-defined entry fields

MRESULT DicExportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

static BOOL DexpInit ( PDEXIDA, HWND );
static VOID PrepareCode( PDEXIDA );
static VOID DexCleanUp( PDEXIDA );
static PCODETAGSW DicGetUserName( PDEXIDA, PCODETAGSW, PPROFENTRY );
static BOOL FillDexBuffer( PDEXIDA, PSZ_W, ULONG );
static BOOL WriteMapTable ( PDEXIDA );
static BOOL WriteHeader ( PDEXIDA );
static VOID GetLevelName( PDEXIDA, PCODETAGSW, SHORT );
static BOOL GetNextEntry( PDEXIDA );
static VOID AddCrLf( PDEXIDA, PSZ_W, PUSHORT );
static BOOL ExportEntry( PDEXIDA );
// static SHORT FindCodeTags( PCODETAGSW, PUCHAR);
static SHORT FindCurrentIndex( PDEXIDA );
static BOOL DexClose( PDEXIDA );
static BOOL DexProcess( HWND, PDEXIDA );
static VOID DexEscapeXMLChars( PSZ_W, USHORT* );

static PFNWP pfnwpDimpFrameProc;        // address of old frame proc

#ifdef FUNCCALLIF
USHORT DicFuncPrepExport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszDictName,             // name of dictionary
  LONG        lOptions,                // dictionary export options or 0L
  PSZ         pszOutFile               // fully qualified name of output file
);
USHORT DicFuncExportProcess
(
  PFCTDATA    pData                    // function I/F session data
);
#endif

// check entered data and prepare export of dictionary
BOOL DicCheckAndPrepareExport
(
  PDEXIDA     pDexIda,                 // export IDA
  BOOL        fMsg,                    // TRUE = show error messages in case of errors
  PBOOL       pfCancelled              // callers cancelled flag
);


//This is the dictionary export main function which first calls up the dialog
//and initializes the actual export routine. It is called up with the handle
//of the 'dictionary list' window and a pointer to the selected dictionary
//in the 'dictionary list' window.

VOID DictionaryExport( HWND hwnd,
                       PSZ pSelDictName,
                       PVOID pDictList )
{
   PDEXIDA         pDexIda;                     // Dic export dialog IDA
   BOOL            fOK;                         // return value

   // create DexIda
   fOK = (UtlAlloc( (PVOID *) &pDexIda, 0L, (LONG) sizeof(DEXIDA), ERROR_STORAGE ));
   if ( fOK )
   {
     if ( pSelDictName != NULL )
     {
       strcpy( pDexIda->szName, pSelDictName );
     } /* endif */
   } /* endif */

   if ( fOK )
   {
     EQFINFO  ErrorInfo;
     pDexIda->ulOemCP = GetLangOEMCP(NULL);      // retrieve CP from system preferences lang
     pDexIda->ulAnsiCP = GetLangAnsiCP(NULL);
     pDexIda->fAnsiConv = TRUE;
     if (pDexIda->ulOemCP == 874 )                  // is it THAI?
     {
        pDexIda->fAnsiConv = FALSE;
     }

     //
     // initial processing (borrowed from WM_INITDLG of old dialog)
     //
     pDexIda->pDictList = (PSELDICTINFO)pDictList;
     pDexIda->pActiveDict = pDexIda->pDictList;
     UtlMakeEQFPath( pDexIda->szDummy, NULC, SYSTEM_PATH,(PSZ) NULP );
     pDexIda->hDictListProp = OpenProperties( DICT_PROPERTIES_NAME,
                                           pDexIda->szDummy,
                                           PROP_ACCESS_READ, &ErrorInfo );
     if( !pDexIda->hDictListProp )
     {
        PSZ pszError = DICT_PROPERTIES_NAME;
        UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszError, EQF_ERROR);
        fOK = FALSE;
     } /* endif */

     if ( fOK )
     {
        PPROPDICTLIST pProp = (PPROPDICTLIST) MakePropPtrFromHnd( pDexIda->hDictListProp );
        pDexIda->ControlsIda.chSavedDrive = pProp->chDexDrive;
        if ( pProp->chDexPath[0] == EOS  )
        {
          strcpy( pProp->chDexPath, pProp->chOldDexPath );
        } /* endif */
        strcpy( pDexIda->ControlsIda.szSavedPath, pProp->chDexPath );
        pDexIda->usExpFormat = ( pProp->usLastExpFormat) ? pProp->usLastExpFormat :
                                                           SGMLFORMAT_ASCII;

        strcpy( pDexIda->ControlsIda.szExt, SGML_EXT );
        strcpy( pDexIda->ControlsIda.szSelectedName, pDexIda->szName );
        strcpy( pDexIda->szSgmlDictName, pDexIda->szName );
     } /* endif */

     // call standard save as dialog
     if ( fOK )
     {
  #ifdef USE_BROWSE_FOR_FOLDER_DLG
       if ( pDexIda->pDictList )
       {
         // for multiple dictionaries use standard browse dialog
          BROWSEINFO bi;
          LPITEMIDLIST pidlBrowse;    // PIDL selected by user

          // Fill in the BROWSEINFO structure.
          bi.hwndOwner      = EqfQueryTwbClient();
          bi.pidlRoot       = NULL;
          bi.pszDisplayName = pDexIda->szString;
          bi.lpszTitle      = "Select directory for exported dictionaries";
          bi.ulFlags        = BIF_RETURNONLYFSDIRS  | BIF_USENEWUI;

          // if UtlBrowseForFolderCallbackProc is specified as callback the lParam
          // parameter must contain the initial folder directory!!!
          bi.lpfn = UtlBrowseForFolderCallbackProc;
          bi.lParam = (LPARAM)pDexIda->szString;

          bi.lpfn           = NULL;
          bi.lParam         = 0;
          pidlBrowse = SHBrowseForFolder( &bi );
          if (pidlBrowse != NULL)
          {
            // get the selected directory path and build full name of first dict
            SHGetPathFromIDList( pidlBrowse, pDexIda->szString );
            strcat( pDexIda->szString, BACKSLASH_STR );
            strcat( pDexIda->szString, pDexIda->szSgmlDictName );
            strcpy( pDexIda->szSgmlDictName, pDexIda->szString );
            strcat( pDexIda->szSgmlDictName, SGML_EXT );
          }
          else
          {
            fOK = FALSE;
          } /* endif */

       }
       else
  #endif
       {
         BOOL fDone = TRUE;
         BOOL fExtAdded = FALSE;

         OPENFILENAME OpenFileName;
         
         memset( &OpenFileName, 0, sizeof(OpenFileName) );
         OpenFileName.lStructSize        = sizeof(OpenFileName);
         OpenFileName.hwndOwner          = EqfQueryTwbClient();
         OpenFileName.hInstance          = NULLHANDLE;
         OpenFileName.lpstrFilter        = DICT_FORMAT_FILTERS;
         OpenFileName.lpstrCustomFilter  = NULL;
         OpenFileName.nMaxCustFilter     = 0;
         OpenFileName.nFilterIndex       = pDexIda->usExpFormat ;
         OEMTOANSI( pDexIda->szSgmlDictName );
         OpenFileName.lpstrFile          = pDexIda->szSgmlDictName;
         OpenFileName.nMaxFile           = sizeof(pDexIda->szSgmlDictName);
         OpenFileName.lpstrFileTitle     = NULL;
         OpenFileName.nMaxFileTitle      = 0;
         OpenFileName.lpstrInitialDir    = pDexIda->ControlsIda.szSavedPath;
         if ( pDexIda->pDictList )
         {
           sprintf( pDexIda->szTitle, "Export the selected dictionaries" );
         }
         else
         {
           sprintf( pDexIda->szTitle, "Export dictionary %s", pDexIda->szSgmlDictName );
         } /* endif */
         OpenFileName.lpstrTitle         = pDexIda->szTitle;
         OpenFileName.Flags              = OFN_ENABLESIZING | OFN_EXPLORER |
                                           OFN_LONGNAMES | OFN_NODEREFERENCELINKS |
                                           OFN_NOTESTFILECREATE | 
                                           OFN_PATHMUSTEXIST;
         OpenFileName.nFileOffset        = 0;
         OpenFileName.nFileExtension     = 0;
//          OpenFileName.lpstrDefExt        = "SGM";
         OpenFileName.lpstrDefExt        = NULL;   // no default extension, we have to set the extension depending of the format
         OpenFileName.lCustData          = 0L;
         OpenFileName.lpfnHook           = NULL;
         OpenFileName.lpTemplateName     = NULL;
         
         do
         {
           fOK = (GetSaveFileName( &OpenFileName ) != 0 );
           if ( fOK )
           {
             pDexIda->usExpFormat = (USHORT)OpenFileName.nFilterIndex;
             fExtAdded = FALSE;

             // set export file extension if none specified by user
             {
               // find file name within full path
               PSZ pszFileName = strrchr( pDexIda->szSgmlDictName, '\\' );
               if ( pszFileName != NULL )
               {
                 pszFileName++;               
               }
               else
               {
                 pszFileName = pDexIda->ControlsIda.szPathContent;
               } /* endif */

               // set default extension if none specified
               if ( strchr( pszFileName, '.' ) == NULL )
               {
                 // append file extension depending on export format
                 switch ( pDexIda->usExpFormat )
                 {
                   case DICT_FORMAT_XML_UTF8:
                     strcat( pDexIda->szSgmlDictName, ".DXT" );
                       break;
                   default:
                     strcat( pDexIda->szSgmlDictName, ".SGM" );
                       break;
                 } /*endswitch */
                 fExtAdded = TRUE;
               } /* endif */
             }
           } /* endif */

           fDone = TRUE;
         } while( !fDone ) ;

       } /* endif */

       if ( fOK )
       {
         // save last used values
         if( SetPropAccess( pDexIda->hDictListProp, PROP_ACCESS_WRITE))
         {
            PPROPDICTLIST pProp = (PPROPDICTLIST) MakePropPtrFromHnd( pDexIda->hDictListProp );
            strcpy( pProp->chDexPath, pDexIda->szSgmlDictName );
            UtlSplitFnameFromPath( pProp->chDexPath );
            pProp->chDexDrive = pDexIda->szSgmlDictName[0];
            pProp->usLastExpFormat = pDexIda->usExpFormat;
            SaveProperties( pDexIda->hDictListProp, &ErrorInfo);
            CloseProperties( pDexIda->hDictListProp, PROP_QUIT, &ErrorInfo );
         } /* endif */

         // set fields in DEXIDA and controls IDA
         strcpy( pDexIda->ControlsIda.szPathContent, pDexIda->szSgmlDictName );
         strncpy( pDexIda->ControlsIda.szDrive , pDexIda->szSgmlDictName, 2 );
         strcpy( pDexIda->ControlsIda.szPath, pDexIda->szSgmlDictName );
         UtlSplitFnameFromPath( pDexIda->ControlsIda.szPath );
       }
       else
       {
          CloseProperties( pDexIda->hDictListProp, PROP_QUIT, &ErrorInfo );
          EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_PROCESSTASK,
                           MP1FROMSHORT( COMPLETED_TASK ),
                           0L );
       } /* endif */
     } /* endif */

     // check dictionary and prepare the export
     if ( fOK )
     {
       BOOL fCancelled = FALSE;
       strcpy( pDexIda->szPath, pDexIda->ControlsIda.szPath );
       strcpy( pDexIda->szSgmlDictName, pDexIda->ControlsIda.szPathContent );
       strcpy( pDexIda->szDrive, pDexIda->ControlsIda.szDrive );

       do
       {
         fOK = DicCheckAndPrepareExport( pDexIda, TRUE, &fCancelled );

         // try next dictionary if not ok and not not cancelled
         if ( !fOK )
         {
           if ( pDexIda->pDictList )
           {
             // mark as not completed
             pDexIda->pActiveDict->szName[0] = X15;

             pDexIda->pActiveDict++;
             if ( pDexIda->pActiveDict->szName[0] == EOS )
             {
               fCancelled = TRUE;
             }
             else
             {
                strcpy( pDexIda->szDictName, pDexIda->pActiveDict->szObjName );
                Utlstrccpy( pDexIda->szShortName, UtlGetFnameFromPath( pDexIda->szDictName ), DOT );
                UtlSplitFnameFromPath( pDexIda->szSgmlDictName );
                strcat( pDexIda->szSgmlDictName, BACKSLASH_STR );
                strcat( pDexIda->szSgmlDictName, pDexIda->pActiveDict->szName );
                strcat( pDexIda->szSgmlDictName, SGML_EXT );
                strcpy( pDexIda->szName, pDexIda->pActiveDict->szName );
                pSelDictName = pDexIda->szShortName;
             } /* endif */
           }
           else
           { 
             fCancelled = TRUE;
           } /* endif */
         } /* endif */
        } while ( !fOK && !fCancelled );
     } /* endif */

     if ( !fOK )
     {
        EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_PROCESSTASK,
                         MP1FROMSHORT( COMPLETED_TASK ), 0L );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check if dictionary is in use                                   */
   /*******************************************************************/
   if ( fOK )
   {
      SHORT sRC;

      PROPNAME( pDexIda->szString, pSelDictName );
      sRC = QUERYSYMBOL( pDexIda->szString );

      if ( sRC != -1 )
      {
         UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pSelDictName, EQF_ERROR );
         fOK = FALSE;
      } /* endif */
   } /* endif */

   if ( fOK && WinIsWindow( (HAB)NULL, hwnd))   // WM_DESTROYed ?
   {
//      strcpy( pDexIda->IdaHead.szObjName, UtlGetFnameFromPath( pDexIda->szSgmlDictName ) );
      // GQ: use dictionary short name to build object name of process window
      strcpy( pDexIda->IdaHead.szObjName, pDexIda->szShortName );
      pDexIda->IdaHead.pszObjName = pDexIda->IdaHead.szObjName;
      pDexIda->hwndError = NULLHANDLE;
      pDexIda->pDicExp = NULL;

      /*******************************************************************/
      /* Start dictionary export by creating the export process window   */
      /*******************************************************************/
      if ( fOK )
      {
        fOK = CreateProcessWindow( pDexIda->IdaHead.pszObjName,
                                   DicExportCallBack, pDexIda );
        if ( !fOK )
        {
           // Issue the system error message
           UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR);
        } /* endif */
      } /* endif */
      else
      {
         EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_PROCESSTASK,
                          MP1FROMSHORT( COMPLETED_TASK ),
                          0L );
         UtlAlloc( (PVOID *) &(pDexIda), 0L, 0L, NOMSG) ;   //release ida
      } /* endif */
   }
   else
   {
      UtlAlloc( (PVOID *) &(pDexIda), 0L, 0L, NOMSG) ;  //release ida
   } /* endif */
}

/**********************************************************************/
/* The following function ist the callback function for the dictionary*/
/* process window. The functions is called by the generic process     */
/* window function to perform specific tasks for dictionary export    */
/**********************************************************************/
MRESULT DicExportCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  PDEXIDA         pDexIda;             // pointer to instance area
  MRESULT         mResult = FALSE;     // return code for handler proc
   PSZ             pszMsgTable[1];
  ULONG            ulLength;

  switch( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
	{
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      /**************************************************************/
      /* Anchor IDA                                                 */
      /**************************************************************/
      pDexIda             = (PDEXIDA) PVOIDFROMMP2(mp2);
      pDexIda->hwnd       = hwnd;
      pCommArea->pUserIDA = pDexIda;

      /****************************************************************/
      /* supply all information required to create the process        */
      /* window                                                       */
      /****************************************************************/
      pCommArea->sProcessWindowID = ID_DICTEXP_WINDOW;
      pCommArea->sProcessObjClass = clsDICTEXP;
      pCommArea->Style            = PROCWIN_TEXTSLIDERENTRY;
      pCommArea->sSliderID        = ID_DEXSLIDER;
      pCommArea->sTextID          = ID_EXPORTDICT_TEXT;
      pCommArea->sEntryGBID       = ID_DEX_ENTRY_TEXT;

      pszMsgTable[0] = pDexIda->szName;
      LOADSTRING( NULLHANDLE, hResMod, IDS_DEX_TITLEBAR, pDexIda->szString );
      DosInsMessage( pszMsgTable, 1, pDexIda->szString, strlen ( pDexIda->szString ),
                     pCommArea->szTitle, (sizeof (pCommArea->szTitle ) - 1),
                     &ulLength );
      pCommArea->szTitle[ulLength] = EOS;

      pszMsgTable[0] =  pDexIda->szSgmlDictName;
      LOADSTRING( NULLHANDLE, hResMod, IDS_DEX_EXPORT_DICTNAME, pDexIda->szString );
      DosInsMessage( pszMsgTable, 1, pDexIda->szString, strlen(pDexIda->szString),
                     pDexIda->szToExternalFileText,
                     (sizeof(pDexIda->szToExternalFileText) - 1), &ulLength );
      pDexIda->szToExternalFileText[ulLength] = EOS;
      strcpy( pCommArea->szText, pDexIda->szToExternalFileText );

      LOADSTRING( NULLHANDLE, hResMod, IDS_DEX_ENTRY_STATICTEXT,
                  pCommArea->szGroupBoxTitle );

      pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_DICTEXPICON); //hiconDICTEXP;
      pCommArea->fNoClose         = FALSE;
      pCommArea->swpSizePos.x     = 100;
      pCommArea->swpSizePos.y     = 100;
      pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 50;
      pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 16;
      pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
      pCommArea->asMsgsWanted[1]  = WM_EQF_SHUTDOWN;
      pCommArea->asMsgsWanted[2]  = WM_TIMER;
      pCommArea->asMsgsWanted[3]  = 0;
      pCommArea->usComplete       = 0;
	  }
      break;


      /****************************************************************/
      /* Start processing                                             */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      pDexIda = (PDEXIDA) pCommArea->pUserIDA;
      WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
      break;

    /******************************************************************/
    /* WM_CLOSE:                                                      */
    /*                                                                */
    /* Prepare/initialize shutdown of process                         */
    /******************************************************************/
    case WM_CLOSE:
      pDexIda = (PDEXIDA) pCommArea->pUserIDA;
      if ( pDexIda )
      {
         pDexIda->fKill = TRUE;
         pDexIda->fHalfCompleted = TRUE;
         mResult = MRFROMSHORT( TRUE );   // = do not close right now
      }
      else
      {
         pDexIda->fKill = TRUE; //set before terminate msg to clear window
         mResult = MRFROMSHORT( FALSE );  // = continue with close
      } /* endif */
      break;

    /******************************************************************/
    /* WM_DESTROY:                                                    */
    /*                                                                */
    /* Cleanup all resources used by the process                      */
    /******************************************************************/
    case WM_DESTROY:
      pDexIda = (PDEXIDA) pCommArea->pUserIDA;
      WinStopTimer( (HAB)UtlQueryULong( QL_HAB ), hwnd, TIMER );
      DexCleanUp( pDexIda );
      UtlAlloc( (PVOID *) &(pDexIda), 0L, 0L, NOMSG) ;
      pCommArea->pUserIDA = NULL;
      break;


    /******************************************************************/
    /* WM_EQF_TERMINATE:                                              */
    /*                                                                */
    /* Allow or disable termination of process                        */
    /******************************************************************/
    case WM_EQF_TERMINATE:
      mResult = MRFROMSHORT( FALSE );          // = continue with close
      break;

    /******************************************************************/
    /* WM_INITMENU:                                                   */
    /*                                                                */
    /* Enable/Disable actionbar items                                 */
    /******************************************************************/
    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    /******************************************************************/
    /* other messages:                                                */
    /*                                                                */
    /* requested from generic process window procedure using          */
    /* asMsgsWanted array in communication area                       */
    /******************************************************************/
    case WM_TIMER:
      pDexIda = (PDEXIDA) pCommArea->pUserIDA;
      strcpy( pCommArea->szEntry, pDexIda->szEntryTxt );
      strcpy( pCommArea->szText, pDexIda->szToExternalFileText );
      OEMTOANSI( pCommArea->szEntry );
      break;

    case WM_EQF_SHUTDOWN:            // come here before EQF_TERMINATE
      pDexIda = (PDEXIDA)pCommArea->pUserIDA;
      pDexIda->fComplete = FALSE;
      pDexIda->fShutdown = TRUE;
      mResult = (MRESULT)TRUE;
      break;

    case WM_EQF_PROCESSTASK:
      //process a term at a time
      pDexIda = (PDEXIDA) pCommArea->pUserIDA;
      DexProcess( hwnd, pDexIda );
      break;

  } /* endswitch */
  return( mResult );
}


//This function is the main export dictionary loop which calls up all processing
//functions and releases all allocated memory when processing is terminated.
//Called with the ida containing all global structures and variables and the
//handle of the standard window.

BOOL DexProcess( HWND hwnd,
                 PDEXIDA pDexIda )
{
   BOOL    fOK = TRUE;                //progress indication
   BOOL    fTerminate = FALSE;


   switch ( pDexIda->DexTask )
   {
      case PREPARE :
         fOK = DexpInit( pDexIda, hwnd );
         pDexIda->DexTask = INITIALIZE;
         break;

      case INITIALIZE:
         PrepareCode( pDexIda );
         pDexIda->DexTask = MAPTABLE;
         break;

      case MAPTABLE:
         fOK = WriteMapTable( pDexIda );
         if ( fOK )
         {
            pDexIda->DexTask = GET_ENTRY;
         } /* endif */
         break;
      case GET_ENTRY:
         fOK = GetNextEntry( pDexIda );
         break;
      case EXPORT_ENTRY:
         fOK = ExportEntry( pDexIda );
         if ( fOK )
         {
            pDexIda->DexTask = GET_ENTRY;
         } /* endif */
         break;
      case CLOSE_PROCESS:
         //does no clean up! Only contains msgs if process is to cancelled
         //and if completed then msg is issued
         fOK = DexClose( pDexIda );
         break;
      default:
         break;
   } /* endswitch */

   if ( pDexIda->fShutdown )
      fOK = FALSE;

   if ( pDexIda->fKill )     //set at wm_close
   {
      pDexIda->usKilledTask = (USHORT) pDexIda->DexTask;
      pDexIda->DexTask = CLOSE_PROCESS;
   } /* endif */

   if ( !fOK && !fTerminate && (hwnd != HWND_FUNCIF))
   {
      EqfRemoveObject( TWBFORCE, hwnd);     //remove std window
   } /* endif */

   if ( fOK && (hwnd != HWND_FUNCIF) )
   {
     UtlDispatch();
     WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
   } /* endif */

   return( fOK );
}

static
VOID DexCleanUp( PDEXIDA pDexIda )
{
   PSZ_W       pDictNameW;    //ptr to dict name string
   PSZ         pDictName;
   BOOL        fDictWasOpen = FALSE;

   //close sgml file
   if ( pDexIda->hOut )
   {
      UtlClose( pDexIda->hOut, FALSE );
      pDexIda->hOut = NULL;
   } /* endif */

   //close asd dict
   if ( pDexIda->hDict )
   {
     AsdClose( pDexIda->hUser,
                pDexIda->hDict );
     pDexIda->hDict = NULL;
     fDictWasOpen = TRUE;
   } /* endif */

   //close Nlp services
   AsdEnd( pDexIda->hUser );
   pDexIda->hUser = NULL;

   // delete sgml file if something went drastically wrong and user did not skip entries
   // intentionally
   if ( !pDexIda->fComplete && !pDexIda->fSkipped )
   {
      UtlDelete( pDexIda->szSgmlDictName, 0L, FALSE );
   }

   else if ( (pDexIda->hwndError != HWND_FUNCIF) &&
             !ISBATCHHWND(pDexIda->hwndError) )
   {
      if ( pDexIda->fHalfCompleted )
      {
        //issue message that export was terminated prematurely
       // OEMTOANSI(pDexIda->ucTermBuf);
        pDictNameW = pDexIda->ucTermBuf;
        UtlErrorW ( ERROR_DEX_HALFCOMPLETE, MB_OK, 1, &pDictNameW, EQF_INFO, TRUE );
      }
      else
      {
        if ( fDictWasOpen && !pDexIda->pDictList )
        {
          //issue message that export has been completed
          pDictName = pDexIda->szName;
          OEMTOANSI(pDexIda->szName);
          if ( pDexIda->fSkipped )
          {
            UtlError ( ERROR_DEX_INCOMPLETE, MB_OK, 1, &pDictName, EQF_INFO );
          }
          else
          {
            UtlError ( ERROR_DEX_COMPLETE, MB_OK, 1, &pDictName, EQF_INFO );
          } /* endif */
        } /* endif */
      } /* endif */
   } /* endif */

   //free allocated memory
   if ( pDexIda->pBuffer )
     UtlAlloc( (PVOID *) &pDexIda->pBuffer, 0L, 0L, NOMSG );
   if ( pDexIda->pTagBuffer )
     UtlAlloc( (PVOID *) &pDexIda->pTagBuffer, 0L, 0L, NOMSG );
   if ( pDexIda->pucData )
     UtlAlloc( (PVOID *) &pDexIda->pucData, 0L, 0L, NOMSG );
   if ( pDexIda->pucNewData )
     UtlAlloc( (PVOID *) &pDexIda->pucNewData, 0L, 0L, NOMSG );
   if ( pDexIda->pWriteBuffer )
     UtlAlloc( (PVOID *) &(pDexIda->pWriteBuffer), 0L, 0L, NOMSG );
   if ( pDexIda->pASCIIBuffer )
     UtlAlloc( (PVOID *) &(pDexIda->pASCIIBuffer), 0L, 0L, NOMSG );

   //remove symbol so dict free for further use. Set in the init
   //procedure where a test is made to see if dict is locked
   //by another process. Symbol can only be removed if dict
   //is not locked.
   if ( pDexIda->fFree )
   {
      UtlMakeEQFPath( pDexIda->szPath, NULC, SYSTEM_PATH,(PSZ) NULP );
      Utlstrccpy( pDexIda->szString,
                  UtlGetFnameFromPath( pDexIda->szDictName ), DOT );
      sprintf( pDexIda->szFilePath, "%s\\%s%s",
               pDexIda->szPath, pDexIda->szString, EXT_OF_DICTPROP );
      REMOVESYMBOL( pDexIda->szFilePath );
   }

   /**************************************************************/
   /* Tell DDE handler that we are thru ...                      */
   /**************************************************************/
   if ( pDexIda->pDicExp != NULL )
   {
     if ( !pDexIda->fComplete  )
     {
       pDexIda->pDicExp->DDEReturn.usRc = UtlGetDDEErrorCode( pDexIda->hwndError );
     } /* endif */
     WinPostMsg( pDexIda->pDicExp->hwndOwner, WM_EQF_DDE_ANSWER,
                 NULL, &pDexIda->pDicExp->DDEReturn );

   } /* endif */

}

//This function issues messages when the process is cancelled or completed and
//branches back to the main export loop for correct termination. Returns TRUE
//if process is to be terminated.

static
BOOL DexClose( PDEXIDA pDexIda )
{
   USHORT  usRc;                      //return code
   BOOL    fOK;                       //progress indication
   ULONG   ulLen = 0;

   if ( !ISBATCHHWND(pDexIda->hwndError) &&
        (pDexIda->hwndError != HWND_FUNCIF) &&
         pDexIda->fKill )  // kill process
   {
      pDexIda->fKill = FALSE;
      usRc = UtlError ( ERROR_DEX_CANCEL,
                        MB_YESNO,
                        0,
                        NULL,
                        EQF_QUERY );

      if ( usRc == MBID_NO )
      {
         pDexIda->DexTask = (DEXTASK) pDexIda->usKilledTask;   //remeber where left off
         fOK = TRUE;
      }
      else
      {
         //normal close of exporting process
         //add end dictionary tag
         ulLen = sprintf( pDexIda->pASCIIBuffer, "</%s>\r\n",
                  strlwr( DimpTokenTable[DICT_TOKEN].szName) );
         ASCII2UnicodeBuf(pDexIda->pASCIIBuffer, pDexIda->pWriteBuffer,
                          ulLen, pDexIda->ulOemCP  );
         pDexIda->fLast = TRUE;
         fOK = FillDexBuffer( pDexIda,
                              pDexIda->pWriteBuffer, ulLen );
         if (pDexIda->pCurrent != pDexIda->pBuffer )
         {
           // force that rest in buffer is written too
           pDexIda->fLast = TRUE;
           fOK = FillDexBuffer( pDexIda, pDexIda->pWriteBuffer, 0);
         }

         if ( !fOK )
         {
            pDexIda->fComplete = FALSE;     //error writing out data
         }
         else
         {
            fOK = FALSE;
         } /* endif */
      } /* endif */
   }
   else
   {
      //normal close of exporting process
      //add end dictionary tag
      ulLen = sprintf( pDexIda->pASCIIBuffer, "</%s>\r\n",
               strlwr( DimpTokenTable[DICT_TOKEN].szName ));
      pDexIda->pASCIIBuffer[ulLen] = EOS;
      ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pDexIda->pWriteBuffer, ulLen, pDexIda->ulOemCP  );
      pDexIda->fLast = TRUE;
      fOK = FillDexBuffer( pDexIda,
                           pDexIda->pWriteBuffer, ulLen );
      if (pDexIda->pCurrent != pDexIda->pBuffer )
      {
           // force that rest in buffer is written too
           pDexIda->fLast = TRUE;
           fOK = FillDexBuffer( pDexIda, pDexIda->pWriteBuffer, 0);
      }
      if ( fOK )
      {
         //set slider to 100%
         if ( pDexIda->hwndError != HWND_FUNCIF )
         {
           WinSendMsg( pDexIda->hwnd, WM_EQF_UPDATESLIDER,
                       MP1FROMSHORT(100), NULL );
         } /* endif */

        // continue with next dictionary if any
        if ( pDexIda->pDictList )
        {
          pDexIda->pActiveDict++;
          if ( pDexIda->pActiveDict->szName[0] == EOS ) pDexIda->pActiveDict = NULL;
        } /* endif */

        // terminate current dictionary export
        if ( pDexIda->pActiveDict )
        {
          DexCleanUp( pDexIda );
        } /* endif */

        // prepare next dictionary for export
        if ( pDexIda->pActiveDict )
        {
          BOOL fOK = TRUE;
          BOOL fCancelled = FALSE;

          do
          {
            PSZ pszMsgTable[1];
            ULONG ulLength = 0;
			HMODULE hResMod;
			hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

            fOK = TRUE;

            strcpy( pDexIda->szDictName, pDexIda->pActiveDict->szObjName );
            Utlstrccpy( pDexIda->szShortName, UtlGetFnameFromPath( pDexIda->szDictName ), DOT );

            UtlSplitFnameFromPath( pDexIda->szSgmlDictName );
            strcat( pDexIda->szSgmlDictName, BACKSLASH_STR );
            strcat( pDexIda->szSgmlDictName, pDexIda->pActiveDict->szName );
            strcat( pDexIda->szSgmlDictName, SGML_EXT );

            strcpy( pDexIda->szName, pDexIda->pActiveDict->szName );
            ANSITOOEM( pDexIda->szName );


            // update to external file text
            pszMsgTable[0] =  pDexIda->szSgmlDictName;
            LOADSTRING( NULLHANDLE, hResMod, IDS_DEX_EXPORT_DICTNAME, pDexIda->szString );
            DosInsMessage( pszMsgTable, 1, pDexIda->szString, strlen(pDexIda->szString),
                          pDexIda->szToExternalFileText,
                          (sizeof(pDexIda->szToExternalFileText) - 1), &ulLength );
            pDexIda->szToExternalFileText[ulLength] = EOS;


            // update process window titlebar text
            pszMsgTable[0] = pDexIda->szName;
            LOADSTRING( NULLHANDLE, hResMod, IDS_DEX_TITLEBAR, pDexIda->szString );
            DosInsMessage( pszMsgTable, 1, pDexIda->szString, strlen(pDexIda->szString),
                          pDexIda->szTitle, (sizeof (pDexIda->szTitle ) - 1),
                          &ulLength );
            pDexIda->szTitle[ulLength] = EOS;
            if ( pDexIda->hwnd && (pDexIda->hwnd != HWND_FUNCIF) )
            {
              SETTEXTHWND( pDexIda->hwnd, pDexIda->szTitle );
              SETTEXT( pDexIda->hwnd, ID_EXPORTDICT_TEXT, pDexIda->szToExternalFileText );
            } /* endif */

            // call generic dictionary export prepare function
            fOK = DicCheckAndPrepareExport( pDexIda, TRUE, &fCancelled );

            if ( !fOK )
            {
              if ( fCancelled )
              {
                pDexIda->pActiveDict = NULL;
                pDexIda->fComplete = FALSE;
              }
              // continue with next dictionary if any
              else if ( pDexIda->pDictList )
              {
                // mark as not completed
                pDexIda->pActiveDict->szName[0] = X15;

                // contine with next dictionary
                pDexIda->pActiveDict++;
                if ( pDexIda->pActiveDict->szName[0] == EOS ) pDexIda->pActiveDict = NULL;
              }
              else
              {
                pDexIda->pActiveDict = NULL;
              } /* endif */
            } /* endif */
          } while ( !fOK && pDexIda->pActiveDict );
        } /* endif */

        // start next organize or terminate organize process
        if ( pDexIda->pActiveDict )
        {
          pDexIda->DexTask = PREPARE;
        }
        else
        {
          // leave processing loop and show completion message
          if ( pDexIda->fComplete && pDexIda->pDictList )
          {
            PSZ pszDictNames = NULL;  // buffer for dictionary name list

            // compute size of required buffer
            PSELDICTINFO pNext = pDexIda->pDictList;
            int iSize = 5;
            while ( pNext->szName[0] != EOS )
            {
              iSize += strlen(pNext->szName) + 5;
              pNext++;
            } /* endwhile */

            // allocate buffer
            UtlAlloc( (PVOID *)&pszDictNames, 0L, iSize, ERROR_STORAGE );

            // fill buffer with dict names
            if ( pszDictNames )
            {
              PSELDICTINFO pNext = pDexIda->pDictList;
              PSZ pszCurPos = pszDictNames;
              while ( pNext->szName[0] != EOS )
              {
                if ( pNext->szName[0] != X15 )
                {
                  int iNameLen = strlen( pNext->szName );
                  *pszCurPos++ = '\"';
                  strcpy( pszCurPos, pNext->szName );
                  pNext++;
                  pszCurPos += iNameLen;
                  *pszCurPos++ = '\"';
                  if ( pNext->szName[0] != EOS )
                  {
                    // add delimiters for following dict name
                    *pszCurPos++ = ',';
                    *pszCurPos++ = ' ';
                  } /* endif */
                }
                else
                {
                  pNext++;
                } /* endif */
              } /* endwhile */
            } /* endif */

             UtlError ( ERROR_DEX_COMPLETE_MUL, MB_OK, 1, &pszDictNames, EQF_INFO );
             if ( pszDictNames ) UtlAlloc( (PVOID *)&pszDictNames, 0L, 0L, NOMSG );
          } /* endif */
          fOK = FALSE;
        } /* endif */
      }
      else
      {
         pDexIda->fComplete = FALSE;     //error writing out data
         fOK = FALSE;
      } /* endif */

   } /* endif */
   return( fOK ); //indication of whether to continue in work loop or not
}

//This function retrieves the next entry from the dictionary and if successful
//branches to the routine that adds the sgml syntax.

static
BOOL GetNextEntry( PDEXIDA pDexIda )
{
   HDCB     hDictFound;           //indication of which dict was selected
   USHORT   usRc;                 //return code
   LONG     lNewSliderPos;        //new slider position
   BOOL     fOK = TRUE;           //success indicator
   ULONG    ulTermNum;            // number of term being retrieved
   PSZ      pszTerm;              //pointer to term string
   BOOL     fContinue = FALSE;    // TRUE = continue after error

   //correct slider position
   lNewSliderPos = pDexIda->ulTermNum * 100L / pDexIda->ulMaxDictEntries;

   if ( (lNewSliderPos != pDexIda->lSliderPos) &&
        (pDexIda->hwndError != HWND_FUNCIF)  )
   {
      pDexIda->lSliderPos = lNewSliderPos;
      WinSendMsg( pDexIda->hwnd, WM_EQF_UPDATESLIDER,
                  MP1FROMSHORT((SHORT)lNewSliderPos), NULL );
   } /* endif */

   do
   {
      fOK = TRUE;

      //get next entry in asd dict
      usRc = AsdNxtTermW( pDexIda->hDict,       //in, dict handle
                          pDexIda->hUser,       //in, user handle
                          pDexIda->ucTermBuf,    //out, matching term(PSZ_W)
                          &ulTermNum,           //out, term number
                          &pDexIda->ulDataLen,  //out, length of above data in # of w's
                          &hDictFound );        //out, which dict had a match

      if ( usRc == LX_EOF_ASD )  //end of dictionary
      {
          pDexIda->DexTask = CLOSE_PROCESS;
      }
      else if ( usRc == LX_RC_OK_ASD )
      {
          //update std window with entry
          Unicode2ASCIIBuf( pDexIda->ucTermBuf, pDexIda->szEntryTxt,
                            (USHORT)UTF16strlenCHAR(pDexIda->ucTermBuf)+1,
                            sizeof(pDexIda->szEntryTxt), pDexIda->ulOemCP  );
          pDexIda->szEntryTxt[STRINGLEN-1] =  EOS;

          //get entry
          usRc = AsdRetEntryW( pDexIda->hDict,
                              pDexIda->hUser,
                              pDexIda->ucTermBuf,
                              &ulTermNum,
                              pDexIda->pucData,
                              &pDexIda->ulDataLen,
                              &hDictFound );

          if ( usRc != LX_RC_OK_ASD )
          {
            PSZ_W  pszParms[2];
            CHAR_W szRC[10];

            swprintf( szRC, L"%u", usRc );

            pszParms[0] = pDexIda->ucTermBuf;
            pszParms[1] = szRC;

            if ( UtlErrorW( ERROR_DEX_TERMCORRUPTED, MB_OKCANCEL, 2, pszParms, EQF_ERROR, TRUE ) == MBID_OK )
            {
              fContinue = TRUE;
              pDexIda->fSkipped = TRUE;
            }
            else
            {
              fOK = FALSE;
            } /* endif */
          }
          else
          {
            pDexIda->ulTermNum++;
            pDexIda->DexTask = EXPORT_ENTRY;   //write out next entry
          } /* endif */
      }
      else
      {
          //dam error
          pszTerm =  QDAMErrorString( usRc, pDexIda->pszMsgError );
          usRc =  UtlErrorHwnd( usRc, MB_CANCEL, 1, &pszTerm, QDAM_ERROR,
                                pDexIda->hwndError );
          fOK = FALSE;
      } /* endif */
   } while ( (usRc != LX_EOF_ASD) && (usRc != LX_RC_OK_ASD) && fContinue );

   if ( !fOK ) pDexIda->fHalfCompleted = TRUE;                       //1130a

   return ( fOK );
}

//This function open files, allocates required memory, initializes the
//dictionary structure and determines the size of dictionary (number of entries).
//Returns TRUE if all went well.

static
BOOL DexpInit ( PDEXIDA pDexIda, HWND hwnd )
{
   BOOL             fOK = TRUE;             //True so far
   USHORT           usRc;                   //Return code
   PSZ              pszDict;                //pointer to dict name
   USHORT           usErrDict;              //dict error code
   SHORT            sRC;                    //return code
   EQFINFO          ErrorInfo;              //property error information
   PVOID            hDictProp;              //handle of dict properties
   PPROPDICTIONARY  pDictProp;              //pointer to dict properties
   SHORT            sItem;                  //list box item

   //allocate memory for output buffer
   fOK = UtlAllocHwnd( (PVOID *) &(pDexIda->pBuffer),
                       0l, (LONG) M_SIZE, ERROR_STORAGE, pDexIda->hwndError );
   if ( fOK )
   {
      pDexIda->pCurrent = pDexIda->pBuffer;
      pDexIda->pBufferEnd = pDexIda->pBuffer + M_SIZE-1;
   }

   //allocate memory to hold complete tag info
   if ( fOK )
   {
      fOK = UtlAlloc( (PVOID *) &(pDexIda->pTagBuffer),
                      0l, (LONG) M_SIZE, ERROR_STORAGE );
      pDexIda->pTag = pDexIda->pTagBuffer;
   } /* endif */

   //allocate memory for output buffer for write procedures
   if ( fOK )
   {
      fOK = UtlAllocHwnd( (PVOID *) &(pDexIda->pWriteBuffer),
                      0l, (LONG) BUFFER_LENGTH, ERROR_STORAGE,
                      pDexIda->hwndError );
   } /* endif */
   if ( fOK )
   {
      fOK = UtlAllocHwnd( (PVOID *) &(pDexIda->pASCIIBuffer),
                         0l, (LONG) BUFFER_LENGTH, ERROR_STORAGE,
                         pDexIda->hwndError );
   } /* endif */

   if ( fOK )
   {
      fOK = UtlAllocHwnd( (PVOID *) &(pDexIda->pucData),
                       0L, (LONG) M_SIZE, ERROR_STORAGE,pDexIda->hwndError);
   } /* endif */

   pDexIda->fComplete = TRUE;          //initialize bool value

   if ( fOK )
   {
      fOK = UtlAllocHwnd( (PVOID *) &(pDexIda->pucNewData),
                       0L, (LONG) M_SIZE, ERROR_STORAGE,pDexIda->hwndError);
      pDexIda->lpucNewDataSize = M_SIZE;
   } /* endif */


   if ( fOK )
   {
      //check if symbol exists (if dict in use by another process)
      UtlMakeEQFPath( pDexIda->szPath, NULC, SYSTEM_PATH, (PSZ)NULP );
      Utlstrccpy( pDexIda->szString,
                  UtlGetFnameFromPath( pDexIda->szDictName ), DOT );
      sprintf( pDexIda->szFilePath, "%s\\%s%s",
               pDexIda->szPath, pDexIda->szString, EXT_OF_DICTPROP );
      sRC = QUERYSYMBOL( pDexIda->szFilePath );

      if ( sRC != -1 )
      {
         Utlstrccpy( pDexIda->szString,
                     UtlGetFnameFromPath( pDexIda->szDictName ), DOT );
         pszDict = pDexIda->szString;
         UtlErrorHwnd( ERROR_DICT_LOCKED, MB_CANCEL,
                   1, &pszDict, EQF_ERROR, pDexIda->hwndError );
         fOK = FALSE;
         pDexIda->fComplete = FALSE;         //1091a
      }
      else
      {
         SETSYMBOL( pDexIda->szFilePath );
         pDexIda->fFree = TRUE;
      } /* endif */

      if ( fOK )
      {
         //open troja dictionary properties

         pszDict = pDexIda->szShortName;

         PROPNAME ( pDexIda->szString, pszDict );
         hDictProp = OpenProperties( pDexIda->szString, NULL,
                                     PROP_ACCESS_WRITE, &ErrorInfo);
         if ( !hDictProp )
         {
            pszDict = pDexIda->szName;
            UtlErrorHwnd( ERROR_OPENING_PROPS, MB_CANCEL, 1, &pszDict, EQF_ERROR,
                          pDexIda->hwndError );

            fOK = FALSE;

            //return sitem in dict list window
            sItem = QUERYSELECTION( pDexIda->DictLBhwnd,
                                    PID_DICTIONARY_LB );
            if ( sItem != LIT_NONE )
            {
               //send msg to grey out dict in dict list window
               CLBSETITEMSTATE( pDexIda->DictLBhwnd, PID_DICTIONARY_LB,
                                MP1FROMSHORT(sItem), MP2FROMSHORT(FALSE) );
            } /* endif */
         }
         else
         {
            pDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hDictProp );

            //store particular prop values required later
            strcpy( pDexIda->szSourceLang, pDictProp->szSourceLang );
            if(pDictProp->szLongDesc[0] != EOS)
            {
                strcpy( pDexIda->szLongDesc, pDictProp->szLongDesc );
            }
            else
            {
                strcpy( pDexIda->szLongDesc, pDictProp->szDescription );
            }
            pDexIda->fCopyRight = pDictProp->fCopyRight;
            pDexIda->fReadOnly = pDictProp->fProtected;

            //fill msg parameter array for QDAM error messages
            if ( pDictProp->szLongName[0] )
            {
              strcpy( pDexIda->szPathContent, pDictProp->szLongName );
            }
            else
            {
              strcpy( pDexIda->szPathContent, pDexIda->szShortName );
            } /* endif */
            pDexIda->pszMsgError[0] = pDexIda->szPathContent;
            //current entry
            UTF16strcpy( pDexIda->ucTermBuf, EMPTY_STRINGW );    //initialize string

            pDexIda->pszMsgError[1] = EMPTY_STRING;
            //dictionary drive
            pDexIda->szDrive[0] = pDictProp->szDictPath[0];
            pDexIda->szDrive[1] = EOS;
            pDexIda->pszMsgError[2] = pDexIda->szDrive;
            //server name
            strcpy( pDexIda->szFindPath, pDictProp->szServer );
            pDexIda->pszMsgError[3] = pDexIda->szFindPath;
            pDexIda->pszMsgError[4] = pDexIda->szSourceLang;


            usRc = AsdBegin( 2,  &pDexIda->hUser );
            if ( usRc != LX_RC_OK_ASD )
            {
               //dam error
               pszDict =  QDAMErrorString( usRc, pDexIda->pszMsgError );
               UtlErrorHwnd ( usRc, MB_CANCEL, 1, &pszDict, QDAM_ERROR,
                            pDexIda->hwndError );
               fOK = FALSE;
            }
            else
            {
               //open ASD dictionary
               UtlMakeEQFPath( pDexIda->szString, NULC,
                               PROPERTY_PATH, NULL );
               strcat( pDexIda->szString, BACKSLASH_STR );
               pszDict = UtlGetFnameFromPath( pDexIda->szDictName );
               Utlstrccpy( pDexIda->szString +
                           strlen(pDexIda->szString),
                           pszDict, DOT );
               strcat( pDexIda->szString, EXT_OF_DICTPROP );
               pszDict = pDexIda->szString;
               usRc = AsdOpen( pDexIda->hUser,   // user ctl block handle
                               ASD_LOCKED,       // open flags
                               1,                // nr of dict in ppszDict
                               &pszDict,         // dictionary properties
                               &pDexIda->hDict,  // dict ctl block handle
                               &usErrDict );     // number of failing dict

               if ( usRc != LX_RC_OK_ASD )
               {
                  //dam error
                  pszDict =  QDAMErrorString( usRc, pDexIda->pszMsgError );
                  UtlErrorHwnd ( usRc, MB_CANCEL, 1, &pszDict, QDAM_ERROR,
                               pDexIda->hwndError );
                  fOK = FALSE;
                  pDexIda->fComplete = FALSE;

                  //if not remote and access denied then grey out and allow delete
                  if ( pDictProp->szServer[0] == NULC )
                  {
                     if ( (usRc == BTREE_ACCESS_ERROR) ||
                        (usRc == BTREE_OPEN_FAILED) ||
                        (usRc == BTREE_FILE_NOTFOUND) ||
                        (usRc == BTREE_INVALID_DRIVE) )
                     {
                        //return sitem in dict list window
                        sItem = QUERYSELECTION( pDexIda->DictLBhwnd,
                                                PID_DICTIONARY_LB );
                        if ( sItem != LIT_NONE )
                        {
                           //send msg to grey out dict in dict list window
                           CLBSETITEMSTATE( pDexIda->DictLBhwnd,
                                            PID_DICTIONARY_LB,
                                            MP1FROMSHORT (sItem),
                                            MP2FROMSHORT(FALSE) );
                        } /* endif */
                     } /* endif */
                  } /* endif */
               }
               else
               {
                     //get original dictionary properties
                     AsdRetPropPtr( pDexIda->hUser, pDexIda->hDict,
                                    &pDexIda->pProp );
               } /* endif */
            } /* endif */
         } /* endif */

         if ( hDictProp )
         {
            CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo);
         } /* endif */
      } /* endif */
   } /* endif */

   // Setup fields-per-level and first-field-of-level array
   if ( fOK )
   {
      QLDBFillFieldTables( pDexIda->pProp, pDexIda->ausNoOfFields,
                             pDexIda->ausFirstFieldOfLevel );
   } /* endif */

   if ( fOK )
   {
      usRc = AsdNumEntries( pDexIda->hDict,           //in, dict handle
                            &pDexIda->ulMaxDictEntries );  //out, num of words
      pDexIda->ulMaxDictEntries = max( pDexIda->ulMaxDictEntries, 1);
      pDexIda->ulTermNum = 0;
      if ( usRc != LX_RC_OK_ASD )
      {
         //dam error
         pszDict =  QDAMErrorString( usRc, pDexIda->pszMsgError );
         UtlErrorHwnd ( usRc, MB_CANCEL, 1, &pszDict, QDAM_ERROR,
                      pDexIda->hwndError );
         fOK = FALSE;
         pDexIda->fComplete = FALSE;
      }
   } /* endif */

   if ( fOK )
   {
      pDexIda->DexTask = INITIALIZE; //initialize next task
   } /* endif */

   //create timer
   if ( pDexIda->hwndError != HWND_FUNCIF )
   {
     WinStartTimer (WinQueryAnchorBlock(hwnd),
                    hwnd,
                    TIMER,
                    TIMEOUT);            // set timer
   } /* endif */

   return( fOK );
}

static
VOID AddCrLf
(
  PDEXIDA pDexIda,
  PSZ_W   pucData,
  PUSHORT pusDataLen                 // # of w's
)
{
   USHORT    usPos = 0;
   PSZ_W    pucNewData;
   ULONG    ulLen;

   pucNewData = pDexIda->pucNewData;
   ulLen = *pusDataLen;
   while ( ulLen-- > 0 )
   {
      usPos++;
      *pucNewData++ = *pucData++;
      if ( usPos >= LINELIMIT )
      {
         while ( *pucData != BLANK && ulLen > 0 )
         {
            ulLen--;
            usPos++;
            *pucNewData++ = *pucData++;
         } /* endif */
         *pucNewData++ = CR;
         *pucNewData++ = LF;
         usPos = 0;
      } /* endif */
   } /* endwhile */
   *pucNewData = EOS;
   *pusDataLen = (USHORT) UTF16strlenCHAR( pDexIda->pucNewData );

}

//This functions put the correct tags around the data drawn from the dictionary
//and calls up FillDexBuffer which writes the data out on to a file.
//Returns TRUE if all went well.

static
BOOL ExportEntry( PDEXIDA pDexIda )
{
   BOOL           fOK = TRUE;          // success indicator
   BOOL           fNoMessage = FALSE;  // display error message
   USHORT         usDataLen;           // length of data
   USHORT         usLdbRc;             // return code of LDB functions
   USHORT         usLevel = 0;         // level of current node
   USHORT         usLastLevel = 0;     // level of last written level
   USHORT         usField;             // number of currently processed field
   USHORT         usCode;              // number of entry in code table
   USHORT         usI;                 // general loop index
   USHORT         usRc;                // return code
   PVOID          pLdbTree;            // pointer to LDB tree
   ULONG          ulCurLen = 0L;
   ULONG          ulNeededLen = 0L;

   pLdbTree = NULL;
   usLdbRc = QLDBRecordToTree( pDexIda->ausNoOfFields, pDexIda->pucData,
                               pDexIda->ulDataLen, &pLdbTree );
   if ( usLdbRc )
   {
     usRc =  UtlErrorHwnd ( usLdbRc, MB_OKCANCEL, 0, NULL, QLDB_ERROR,
                          pDexIda->hwndError );
     if ( usRc == MBID_CANCEL )
     {
       fOK = FALSE;
       pDexIda->fComplete = FALSE;
     } /* endif */

     fNoMessage = TRUE;  //don't display another error message
   }
   else
   {
     if ( fOK )
     {
       // Write empty line to SGML File
       fOK = FillDexBuffer( pDexIda, CRLF_STRINGW, (SHORT)UTF16strlenCHAR( CRLF_STRINGW ) );

       // Get current (=first node)
       usLdbRc = QLDBCurrNode( pLdbTree, pDexIda->apszField, &usLevel );
     } /* endif */

     // Process nodes while no error occurs and tree has more nodes
     while ( fOK &&
             (usLdbRc == QLDB_NO_ERROR) &&
             (usLevel < QLDB_END_OF_TREE) )
     {
       // Write any end tags for open levels to SGML file
       while ( fOK && (usLastLevel >= usLevel) )
       {
         usLastLevel--;
         if ( pDexIda->ausLevelToCode[usLastLevel] != (USHORT)-1 )
         {
           usCode  = pDexIda->ausLevelToCode[usLastLevel];
           fOK = FillDexBuffer( pDexIda,
                                pDexIda->CodeTags[usCode].pEndTag,
                                (SHORT)UTF16strlenCHAR( pDexIda->CodeTags[usCode].pEndTag ));
         } /* endif */
       } /* endwhile */

       // Write level start tag to SGML file
       if ( pDexIda->ausLevelToCode[usLevel-1] != (USHORT)-1 )
       {
        usCode  = pDexIda->ausLevelToCode[usLevel-1];
        fOK = FillDexBuffer( pDexIda,
                              pDexIda->CodeTags[usCode].pStartTag,
                              (SHORT)UTF16strlenCHAR( pDexIda->CodeTags[usCode].pStartTag ));
       } /* endif */
       usLastLevel = usLevel;

       // Write fields of current node to SGML file
       usI = 0;
       while ( fOK && (usI < pDexIda->ausNoOfFields[usLevel-1]) )
       {
         if ( pDexIda->apszField[usI] && *(pDexIda->apszField[usI])  )
         {
           // Write start tag for field data
           usField = pDexIda->ausFirstFieldOfLevel[usLevel-1] + usI;
           usCode  = pDexIda->ausFieldToCode[usField];
           fOK = FillDexBuffer( pDexIda,
                                pDexIda->CodeTags[usCode].pStartTag,
                                (SHORT)UTF16strlenCHAR( pDexIda->CodeTags[usCode].pStartTag ));

           // Write field data
           if ( fOK )
           {
             usDataLen = (USHORT)UTF16strlenCHAR( pDexIda->apszField[usI] );
             AddCrLf( pDexIda,
                      pDexIda->apszField[usI], &usDataLen );  // usDataLen in # of w's
             // now pDexIda->pucNewData holds the stuff

             if ( pDexIda->usExpFormat == DICT_FORMAT_XML_UTF8 ) 
                DexEscapeXMLChars( pDexIda->pucNewData, &usDataLen ) ;

             ulCurLen = sizeof(pDexIda->pWriteBuffer);
             ulNeededLen = usDataLen * sizeof(CHAR_W);
             if ((ulNeededLen >= ulCurLen))
             {
                fOK = UtlAllocHwnd( (PVOID *) &(pDexIda->pWriteBuffer),
                                     ulCurLen, ulCurLen + BUFFER_LENGTH + ulNeededLen,
                                     ERROR_STORAGE, pDexIda->hwndError );
             }
             if (fOK)
             {
               memcpy((PBYTE) pDexIda->pWriteBuffer, (PBYTE)pDexIda->pucNewData,
                        ulNeededLen );
             }

             // pucNewData is needed as conversion buffer during FillDexBuffer
             fOK = FillDexBuffer( pDexIda,
                                  pDexIda->pWriteBuffer,
                                  usDataLen );
           } /* endif */

           // Write field data end tag
           if ( fOK )
           {
              fOK = FillDexBuffer( pDexIda,
                                   pDexIda->CodeTags[usCode].pEndTag,
                                   (SHORT)UTF16strlenCHAR( pDexIda->CodeTags[usCode].pEndTag ));
           } /* endif */
         } /* endif */
         usI++;
       } /* endwhile */

       // Get next node of node tree
       usLdbRc = QLDBNextNode( pLdbTree, pDexIda->apszField, &usLevel );
     } /* endwhile */

     QLDBDestroyTree( &pLdbTree );
   } /* endif */

   // Write any end tags for open levels to SGML file
   while ( fOK && (usLastLevel > 0) )
   {
     usLastLevel--;
     if ( pDexIda->ausLevelToCode[usLastLevel] != (USHORT)-1 )
     {
       usCode  = pDexIda->ausLevelToCode[usLastLevel];
       fOK = FillDexBuffer( pDexIda,
                            pDexIda->CodeTags[usCode].pEndTag,
                            (SHORT)UTF16strlenCHAR( pDexIda->CodeTags[usCode].pEndTag ));
     } /* endif */
   } /* endwhile */

   if ( (usLdbRc != QLDB_NO_ERROR) && !fNoMessage )           //2msga
   {
      PSZ_W  pszParms[2];
      CHAR_W szRC[10];

      // while writing out data
      // UtlErrorHwnd( ERROR_WRITING_TO_SGML_FILE, MB_CANCEL, 0, NULL, EQF_ERROR,
      //        pDexIda->hwndError );

      //error in LDB processing is propably caused by a corrupted entry
      swprintf( szRC, L"%u", usLdbRc );

      pszParms[0] = pDexIda->ucTermBuf;
      pszParms[1] = szRC;

      if ( UtlErrorHwndW( ERROR_DEX_TERMCORRUPTED, MB_OKCANCEL, 2, pszParms, EQF_ERROR, pDexIda->hwndError, TRUE ) == MBID_OK )
      {
        pDexIda->fSkipped = TRUE;
      }
      else
      {
        fOK = FALSE;
      } /* endif */
      pDexIda->fComplete = FALSE;
   }
   return ( fOK );
}

//This function fills the codetag table for level and field tags as taken
//from the dictionary profile.

static
VOID PrepareCode( PDEXIDA pDexIda )
{
   USHORT usField = 0;   //field number index
   PCODETAGSW pCodeTags;  //struct with codes and entry field names
   SHORT  sLevel = 0;    //current level in tree (HOM, SENSE, etc.)
   PPROFENTRY   pProfEntry;         //ptr to a profile entry


   // intialize level to code table
   {
     int i;
     for( i = 0; i < 4; i++ )
     {
       pDexIda->ausLevelToCode[i] = (USHORT)-1;
     } /* endfor */
   }

   //position at beginning of structure containing field names and matching tags
   pCodeTags = &pDexIda->CodeTags[0];
   pDexIda->usMaxNodes = 0;

   //position at beginning of profile entries(=dictionary fields)
   pProfEntry = pDexIda->pProp->ProfEntry;

   //loop through dictionary profile and create required tags
   while ( usField < pDexIda->pProp->usLength )
   {
      //add level tag(s) if required
      while ( sLevel < (SHORT)pProfEntry->usLevel )
      {
        sLevel++;
        pDexIda->ausLevelToCode[sLevel-1] = (USHORT)(pCodeTags - pDexIda->CodeTags);
        pCodeTags->sFieldNumber = -1; // no field for this entry
        GetLevelName( pDexIda, pCodeTags, sLevel );
        pCodeTags++;
        pDexIda->usMaxNodes++;
      } /* endwhile */

      //add dictionary field tag
      pCodeTags->sLevel = sLevel;         //store level
      pCodeTags->sFieldNumber = usField; // set field number of this entry
      DicGetUserName( pDexIda, pCodeTags, pProfEntry );
      pDexIda->ausFieldToCode[usField] = (USHORT)(pCodeTags - pDexIda->CodeTags);


      //proceed with next entry
      pCodeTags++;
      pProfEntry++;
      usField++;
      pDexIda->usMaxNodes++;
   } /* endwhile */
}

//Sets up the sgml syntax around level data, e.g.hom or sense and stores this
//in pCodetags (structure set up for output to file).

static
VOID GetLevelName( PDEXIDA pDexIda,
                   PCODETAGSW pCodeTags,
                   SHORT sLevel )
{
   SHORT   sToken = 0;        //token enum id
   ULONG   ulLen;         //returned length from sprintf
   CHAR    szName[STYLE_LEN*2];

   switch ( sLevel )
   {
      case FIRST_LEVEL:
        sToken = (SHORT) DimpTokenTable[ENTRY_TOKEN].sTag;
      break;
      case SECOND_LEVEL:
        sToken = (SHORT)DimpTokenTable[HOM_TOKEN].sTag;
      break;
      case THIRD_LEVEL:
        sToken = (SHORT) DimpTokenTable[SENSE_TOKEN].sTag;
      break;
      case FOURTH_LEVEL:
        sToken = (SHORT) DimpTokenTable[TARGET_TOKEN].sTag;
      break;
   } /* endswitch */

   //remember start and end tag with indentation
   pCodeTags->sLevel = sLevel;         //store level
   pCodeTags->pStartTag = pDexIda->pTag;  // store start of tag
   if ( sLevel != FIRST_LEVEL )        // do not indent first level tag
   {
     UTF16memset( pDexIda->pTag,' ', (USHORT)(sLevel * INDENT));
     pDexIda->pTag += sLevel * INDENT;
   } /* endif */
   ulLen = sprintf( szName /*pDexIda->pTag*/, "<%s>\r\n",
                          strlwr( DimpTokenTable[ sToken ].szName) );
   ASCII2UnicodeBuf( szName, pDexIda->pTag, ulLen, pDexIda->ulOemCP  );

   pDexIda->pTag += ulLen + 1;            // skip this tag
   pCodeTags->pEndTag = pDexIda->pTag;  // store end tag
   if ( sLevel != FIRST_LEVEL )        // do not indent first level tag
   {
     UTF16memset( pDexIda->pTag,' ', (USHORT)(sLevel * INDENT) );
     pDexIda->pTag += sLevel * INDENT;
   } /* endif */
   ulLen = sprintf( szName /*pDexIda->pTag*/, "</%s>\r\n",
                          strlwr( DimpTokenTable[ sToken ].szName) );
   ASCII2UnicodeBuf( szName, pDexIda->pTag, ulLen, pDexIda->ulOemCP  );
   pDexIda->pTag += ulLen + 1;            // skip this tag
   *pDexIda->pTag = NULC;                 // insert null in username
   // @@ TODO: debug here!
   //pCodeTags->pUserName = pDexIda->pTag;  // insert username
   pCodeTags->pUserName = EOS;
   pDexIda->pTag += 1;                    // skip this tag
}

//Sets up the sgml syntax around entry data and stores the information in
//pCodeTags(output structure). Returns this structure.

static
PCODETAGSW DicGetUserName( PDEXIDA pDexIda,
                       PCODETAGSW pCodeTags,
                       PPROFENTRY pProfEntry )
{
   PTOKENFIELD  pTokenField;        //ptr to table with SysName and
                                    //corresponding token
   BOOL         fFound = FALSE;     //found indicator
   ULONG        ulLen;              //returned length from sprintf
   SHORT        sToken = 0;             //token id
   CHAR         szName[STYLE_LEN+2*TAGLENGTH];

   pCodeTags->pSysName    = pProfEntry->chSystName;
   pCodeTags->pUserName   = pProfEntry->chUserName;
   pCodeTags->usDisplay   = pProfEntry->usDisplay;
   pCodeTags->usStatus    = pProfEntry->usStatus;
   pCodeTags->fVital      = pProfEntry->fVital;
   pCodeTags->fAutLookup  = pProfEntry->fAutLookup;
   pCodeTags->usEntryType = pProfEntry->usEntryFieldType;
   pTokenField            = &TokenField[0];

   fFound = FALSE;
   while ( !fFound && pTokenField->sTokenid != LAST_TAG )
   {
      //ASCII2UnicodeBuf( pTokenField->chSysName, szTemp, 2*TAGLENGTH );
      if ( stricmp( pCodeTags->pSysName, pTokenField->chSysName ) == 0 )
      {
         fFound = TRUE;
         sToken = pTokenField->sTokenid;        // remember tokenid
                                                // insert indentation
         UTF16memset( pDexIda->pTag, ' ' , (USHORT)(pCodeTags->sLevel * INDENT));
         pCodeTags->pStartTag = pDexIda->pTag;  // store start of tag
         pDexIda->pTag += pCodeTags->sLevel * INDENT;
         ulLen = sprintf( szName, /*pDexIda->pTag,*/ "<%s>",
                       strlwr( DimpTokenTable[ sToken ].szName ));
         ASCII2UnicodeBuf( szName, pDexIda->pTag, ulLen, pDexIda->ulOemCP  );
         pDexIda->pTag += ulLen + 1;            // skip this tag
      }
      else
      {
         pTokenField++;
      } /* endif */
   } /* endwhile */

   if ( !fFound ) //user defined entry field which requires a running id
   {
      if ( pCodeTags->sLevel == FOURTH_LEVEL )
      {
         sToken = TUSER_TOKEN;            // we are a 'Tuser'....
      }
      else
      {
         sToken = EUSER_TOKEN;            // we are a 'euser'....
      } /* endif */
      // insert indentation
      UTF16memset( pDexIda->pTag, ' ' , (USHORT)(pCodeTags->sLevel * INDENT));
      pCodeTags->pStartTag = pDexIda->pTag;  // store start of tag
      pDexIda->pTag += pCodeTags->sLevel * INDENT;
      if ( pDexIda->usExpFormat == DICT_FORMAT_XML_UTF8 ) {
         ulLen = sprintf( szName /*pDexIda->pTag*/, "<%s %s\"%d\">",
                          strlwr( DimpTokenTable[ sToken ].szName),
                          ID_TOKEN,
                          pProfEntry->sId );
      } else {
         ulLen = sprintf( szName /*pDexIda->pTag*/, "<%s %s%d>",
                          strlwr( DimpTokenTable[ sToken ].szName),
                          ID_TOKEN,
                          pProfEntry->sId );
      }
      ASCII2UnicodeBuf( szName, pDexIda->pTag, ulLen, pDexIda->ulOemCP );
      pDexIda->pTag += ulLen + 1;            // skip this tag
   } /* endif */

   ulLen = sprintf( szName,  "</%s>\r\n",
                       strlwr( DimpTokenTable[ sToken ].szName ));
   ASCII2UnicodeBuf( szName, pDexIda->pTag, ulLen,pDexIda->ulOemCP  );
   pCodeTags->pEndTag = pDexIda->pTag;    // insert end tag
   pDexIda->pTag += ulLen + 1;            // skip this tag

   return( pCodeTags );
}

//Writes the buffer containing the sgml syntax plus data out onto file. Returns
//TRUE if all went well.

static
BOOL FillDexBuffer( PDEXIDA pDexIda,       //ptr to structure
                    PSZ_W   pData,             //string to be written
                    ULONG   ulLength )      //length of string in # of w's
{
   BOOL      fOK = TRUE;   //process indicator
   ULONG     ulFree;       //number of bytes still free
   ULONG     ulMaxWrite;   //max number of bytes that can be filled
   ULONG     ulWritten;    //written bytes
   USHORT    usDosRc;      //DOS return code
   ULONG     ulLenBytes = ulLength * sizeof(CHAR_W);
   PSZ       pConvData = (PSZ) pDexIda->pucNewData;
   ULONG     ulBytesToWrite = 0;
   BOOL      fCopied = FALSE;

   // if there is space copy into buffer
     ulFree = pDexIda->pBufferEnd -  pDexIda->pCurrent;
     if ((ulLenBytes > 0 )&& (ulLenBytes <= ulFree))
     {
        memcpy( pDexIda->pCurrent, (PBYTE)pData, ulLenBytes );
        pDexIda->pCurrent += ulLenBytes;  //update next starting pos in mem block
        fCopied = TRUE;
     }

     // if at end of dictionary or there is not enough space write buffer out
     if ( (ulFree < ulLenBytes) || ( pDexIda->fLast))
     {
        ulMaxWrite = pDexIda->pCurrent - pDexIda->pBuffer;

        switch ( pDexIda->usExpFormat )
        {
          case DICT_FORMAT_SGML_ANSI:
            { LONG lRc = 0;
             ulBytesToWrite = UtlDirectUnicode2AnsiBuf( (PSZ_W)pDexIda->pBuffer,
                                           pConvData,
                                           (USHORT)(ulMaxWrite / sizeof(CHAR_W)),
                                           pDexIda->lpucNewDataSize,
                                           pDexIda->ulAnsiCP, TRUE, &lRc );
                 usDosRc = (USHORT)lRc;
		    }
             pConvData[ulBytesToWrite] = EOS;
             usDosRc = UtlWriteHwnd( pDexIda->hOut, pConvData, ulBytesToWrite,
                       &ulWritten, TRUE, pDexIda->hwndError );
            break;
          case DICT_FORMAT_SGML_ASCII:
             ulBytesToWrite = Unicode2ASCIIBuf( (PSZ_W)pDexIda->pBuffer,
                                                pConvData,
                                                (USHORT)(ulMaxWrite / sizeof(CHAR_W)),
                                                pDexIda->lpucNewDataSize,
                                                pDexIda->ulOemCP );
             pConvData[ulBytesToWrite] = EOS;
             usDosRc = UtlWriteHwnd( pDexIda->hOut, pConvData, ulBytesToWrite,
                       &ulWritten, TRUE, pDexIda->hwndError );
            break;
          case DICT_FORMAT_XML_UTF8:
             pConvData[ulMaxWrite/sizeof(CHAR_W)] = NULL ;
             ulBytesToWrite = Unicode2UTF8BufEx( (PSZ_W)pDexIda->pBuffer,
                                                 (PSZ)pConvData,
                                                 ulMaxWrite / sizeof(CHAR_W),
                                                 pDexIda->lpucNewDataSize,
                                                 NULL, NULL ) ;
             pConvData[ulBytesToWrite] = EOS;
             usDosRc = UtlWriteHwnd( pDexIda->hOut, pConvData, ulBytesToWrite,
                       &ulWritten, TRUE, pDexIda->hwndError );
            break;
          case DICT_FORMAT_SGML_UNICODE:
          default:
            // no conversion needed
             ulBytesToWrite = ulMaxWrite;
             usDosRc = UtlWriteHwnd( pDexIda->hOut, pDexIda->pBuffer, ulBytesToWrite,
                                          &ulWritten, TRUE, pDexIda->hwndError );
            break;
        } /*endswitch */
        fOK = (( usDosRc == 0 ) && ( ulBytesToWrite == ulWritten ));
        if ( !fOK )
        {
           pDexIda->fComplete = FALSE;
        } /* endif */
        pDexIda->pCurrent = pDexIda->pBuffer; //reset buffer
        ulFree = pDexIda->pBufferEnd -  pDexIda->pCurrent;
     } /* endif */

     if (!fCopied)
     {
       if ( ulLenBytes <= ulFree )
       {
         if ( ulLenBytes > 0 )
         {
           memcpy( pDexIda->pCurrent, (PBYTE)pData, ulLenBytes );
           pDexIda->pCurrent += ulLenBytes;  //update next starting pos in mem block
           fCopied = TRUE;
         } /* endif */
       }
       else
       {
         fOK = FALSE;
       } /* endif */
     }  /* endif */
   return( fOK );
}

//This function sets up the sgml syntax for header information and calls FillDexBuffer
//which writes the buffer out to file.

static
BOOL WriteHeader ( PDEXIDA pDexIda )
{
   PSZ_W   pWriteBuffer;               //pointer to output buffer
   BOOL    fOK = TRUE;                 //process indicator
   ULONG   ulLen;                      //returned length of sprintf string

   pWriteBuffer = pDexIda->pWriteBuffer;
   if ( pDexIda->szSourceLang[0] != EOS )
   {
      ulLen = sprintf( pDexIda->pASCIIBuffer, " <%s>\r\n",
                          strlwr( DimpTokenTable[HEADER_TOKEN].szName ));
      ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen,  pDexIda->ulOemCP );
      fOK = FillDexBuffer( pDexIda,
                           pWriteBuffer,
                           ulLen );


      if ( fOK && pDexIda->szSourceLang[0] != EOS )
      {
         ulLen = sprintf( pDexIda->pASCIIBuffer, "  <%s>%s<%s>\r\n",
                          strlwr( DimpTokenTable[SOURCE_TOKEN].szName ),
                          pDexIda->szSourceLang,
                          strlwr( DimpTokenTable[ESOURCE_TOKEN].szName ));
         ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen,  pDexIda->ulOemCP  );
         fOK = FillDexBuffer( pDexIda,
                              pWriteBuffer,
                              ulLen );
      }

      if ( fOK && pDexIda->szLongDesc[0] != EOS)
      {
        ulLen = sprintf( pDexIda->pASCIIBuffer, "  <%s>%s<%s>\r\n",
                         strlwr( DimpTokenTable[DESCRIPTION_TOKEN].szName ),
                         pDexIda->szLongDesc,
                         strlwr( DimpTokenTable[EDESCRIPTION_TOKEN].szName ));
        ASCII2UnicodeBuf(pDexIda->pASCIIBuffer, pWriteBuffer, ulLen, pDexIda->ulOemCP  );
        // recalc of ulLen nec. if Description contains DBCS characters (IV000026)
        ulLen = UTF16strlenCHAR(pWriteBuffer);
        fOK = FillDexBuffer( pDexIda,
                             pWriteBuffer,
                             ulLen );
      }

      if (fOK )
      {
		CHAR  chCodePageBuf[100];
		switch(pDexIda->usExpFormat)
		{
			case DICT_FORMAT_SGML_ANSI:
			  sprintf(&chCodePageBuf[0], "ANSI=%i", pDexIda->ulAnsiCP);
			break;
			case DICT_FORMAT_SGML_ASCII:
			  sprintf(&chCodePageBuf[0], "ASCII=%i", pDexIda->ulOemCP);
			break;
			case DICT_FORMAT_SGML_UNICODE:
			  sprintf(&chCodePageBuf[0], "UTF16");
			break;
            case DICT_FORMAT_XML_UTF8:
              sprintf(&chCodePageBuf[0], "UTF8");
            break;
	    }
		ulLen = sprintf(pDexIda->pASCIIBuffer, "  <%s>%s<%s>\r\n",
		                strlwr( DimpTokenTable[CODEPAGE_TOKEN].szName ),
		                &chCodePageBuf[0],
		                strlwr( DimpTokenTable[ECODEPAGE_TOKEN].szName ));
		ASCII2UnicodeBuf(pDexIda->pASCIIBuffer, pWriteBuffer, ulLen+1, pDexIda->ulOemCP  );

        ulLen = UTF16strlenCHAR(pWriteBuffer);
        fOK = FillDexBuffer( pDexIda,
                             pWriteBuffer,
                             ulLen );
      }
      if ( fOK )
      {
         ulLen = sprintf( pDexIda->pASCIIBuffer, " <%s>\r\n",
                          strlwr( DimpTokenTable[EHEADER_TOKEN].szName ));
         ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen, pDexIda->ulOemCP  );
         fOK = FillDexBuffer( pDexIda,
                              pWriteBuffer,
                              ulLen );
      }
   }
   return ( fOK );
}


//This function sets up the sgml syntax for the maptable and calls FillDexBuffer
//which writes the buffer out to file.

static
BOOL WriteMapTable ( PDEXIDA pDexIda )
{
   BOOL    fOK;                        //process indicator
   USHORT  usIndex = 1;                //counter
   PCODETAGSW pCodeTags;               //ptr to tag & matching field name struct
   ULONG      ulLen;                   //returned length of sprintf string
   CHAR_W     szAttr[20] ;             //template for attributes with no value
   CHAR_W     szAttrParm[20] ;         //template for attributes with a value
   PSZ_W      pWriteBuffer;            //pointer to output buffer
   PSZ_W      pTempBuffer;             //pointer to output buffer
   SHORT      sHighestLevel = 0;       // highest level of fields in map table

   pWriteBuffer = pDexIda->pWriteBuffer;

   if ( pDexIda->usExpFormat == DICT_FORMAT_XML_UTF8 ) {
      strcpy( pDexIda->pASCIIBuffer, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" ) ;
      ulLen = strlen( pDexIda->pASCIIBuffer ) ;
      ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer+1, ulLen, pDexIda->ulOemCP  );
      *pWriteBuffer = L'\xFEFF' ;
      fOK = FillDexBuffer( pDexIda,
                           pWriteBuffer,
                           ulLen+1 );
      wcscpy( szAttr,     L" %s=\"1\"" ) ;
      wcscpy( szAttrParm, L" %s\"%d\"" ) ;
   } else {
      wcscpy( szAttr,     L" %s" ) ;
      wcscpy( szAttrParm, L" %s%d" ) ;
   }

   ulLen = sprintf( pDexIda->pASCIIBuffer, "<%s>\r\n",
                          strlwr( DimpTokenTable[DICT_TOKEN].szName ));
   ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen, pDexIda->ulOemCP  );
   fOK = FillDexBuffer( pDexIda,
                        pWriteBuffer,
                        ulLen );
   if ( fOK )
      fOK = WriteHeader( pDexIda);

   if ( fOK )
   {
      ulLen = sprintf( pDexIda->pASCIIBuffer, "\r\n<%s>\r\n",
                          strlwr( DimpTokenTable[MAPTABLE_TOKEN].szName ));
      ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen, pDexIda->ulOemCP  );
      fOK = FillDexBuffer( pDexIda,
                           pWriteBuffer,
                           ulLen );

   } /* endif */

   pCodeTags = &pDexIda->CodeTags[1]; //point to second entry in structure
   while ( usIndex < pDexIda->usMaxNodes && fOK )
   {
      if ( pCodeTags->sFieldNumber != -1 )
      {
        CHAR_W szTempString[40];

         //normal entry with display, entry type and status or user-defined
         //with additional id
         UTF16memset(pWriteBuffer, NULC, sizeof(pWriteBuffer) / sizeof(CHAR_W) );
         UTF16strcpy( pWriteBuffer, pCodeTags->pStartTag );
         pTempBuffer = pWriteBuffer + UTF16strlenCHAR( pCodeTags->pStartTag) - 1;

         wcscpy( szTempString, DISPLAY_TOKENW );
         swprintf( pTempBuffer, szAttrParm,
                         UtlLowerW( szTempString ), pCodeTags->usDisplay );

         if ( pCodeTags->fVital )
         {
           wcscpy( szTempString, VITAL_TOKENW );
           swprintf( pTempBuffer + UTF16strlenCHAR(pTempBuffer), szAttr,
                    UtlLowerW( szTempString ) );
         } /* endif */

         if ( pCodeTags->fAutLookup )
         {
           wcscpy( szTempString, AUTLOOKUP_TOKENW );
           swprintf( pTempBuffer + UTF16strlenCHAR(pTempBuffer), szAttr,
                    UtlLowerW( szTempString ) );
         } /* endif */

         wcscpy( szTempString, ENTRYTYPE_TOKENW );
         swprintf( pTempBuffer + UTF16strlenCHAR(pTempBuffer), szAttrParm,
                  UtlLowerW( szTempString ), pCodeTags->usEntryType );
         wcscat( pTempBuffer, L">" ) ;

         fOK = FillDexBuffer( pDexIda,
                              pWriteBuffer,
                              (SHORT)UTF16strlenCHAR( pWriteBuffer ) );
      }
      else
      {
         //level tag e.g. hom
         fOK = FillDexBuffer( pDexIda,
                              pCodeTags->pStartTag,
                              (SHORT)UTF16strlenCHAR( pCodeTags->pStartTag ));
      } /* endif */
      
      if ( pCodeTags->sLevel > sHighestLevel )
      {
        sHighestLevel = pCodeTags->sLevel;
      } /* endif */

      if ( fOK && pCodeTags->pUserName)
      {
        ulLen = strlen( pCodeTags->pUserName);
        ASCII2UnicodeBuf( pCodeTags->pUserName, pWriteBuffer, ulLen, pDexIda->ulOemCP );
        fOK = FillDexBuffer( pDexIda, pWriteBuffer, ulLen );
      } /* endif */

      if ( fOK && ( pCodeTags->sFieldNumber != -1) )
      {
         fOK = FillDexBuffer( pDexIda,
                              pCodeTags->pEndTag,
                              (SHORT)UTF16strlenCHAR( pCodeTags->pEndTag ));
      } /* endif */

      pCodeTags++;
      usIndex++;
   } /* endwhile */

   if ( fOK && (sHighestLevel >= 4) )
   {
      ulLen = sprintf( pDexIda->pASCIIBuffer, "        </%s>\r\n",
                          strlwr( DimpTokenTable[TARGET_TOKEN].szName ));
      ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen, pDexIda->ulOemCP  );
      fOK = FillDexBuffer( pDexIda,
                           pWriteBuffer,
                           ulLen );

   } /* endif */

   if ( fOK && (sHighestLevel >= 3) )
   {
      ulLen = sprintf( pDexIda->pASCIIBuffer, "      </%s>\r\n",
                          strlwr( DimpTokenTable[SENSE_TOKEN].szName ));
      ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen, pDexIda->ulOemCP  );

      fOK = FillDexBuffer( pDexIda,
                           pWriteBuffer,
                           ulLen );

   } /* endif */

   if ( fOK && (sHighestLevel >= 2) )
   {
      ulLen = sprintf( pDexIda->pASCIIBuffer, "    </%s>\r\n",
                          strlwr( DimpTokenTable[HOM_TOKEN].szName ));
      ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen, pDexIda->ulOemCP  );
      fOK = FillDexBuffer( pDexIda,
                           pWriteBuffer,
                           ulLen );

   } /* endif */

   if ( fOK )
   {
      ulLen = sprintf( pDexIda->pASCIIBuffer, "</%s>\r\n",
                          strlwr( DimpTokenTable[MAPTABLE_TOKEN].szName ));
      ASCII2UnicodeBuf( pDexIda->pASCIIBuffer, pWriteBuffer, ulLen, pDexIda->ulOemCP );
      fOK = FillDexBuffer( pDexIda,
                           pWriteBuffer,
                           ulLen );

   } /* endif */

   if ( !fOK )
   {
      //error while writing out maptable data
      UtlErrorHwnd( ERROR_WRITING_TO_SGML_FILE, MB_CANCEL, 0, NULL, EQF_ERROR,
                  pDexIda->hwndError );
      pDexIda->fComplete = FALSE;
   } /* endif */

   return( fOK );
}


/**********************************************************************/
/* Export a dictionary in batch mode                                  */
/**********************************************************************/
VOID DicBatchExp
(
  HWND   hwnd,                  // dictionary list window handle
  PVOID  pBatchExp              // command line dict export structure
)
{
  BOOL         fOK = TRUE;                      // error flag
  PDEXIDA      pDexIda;                         // Dict export ida
  USHORT       usRc;                            // Return code
  PSZ          pszName;                         // error code string
  HPROP        hDictProp;                       // dict handle
  EQFINFO      ErrorInfo;                       // error msg
  PSZ          pFile;                           // ptr to string
  PDDEDICEXP   pDicExp = (PDDEDICEXP)pBatchExp;

  hwnd;
  //allocate dict export ida
  fOK = (UtlAlloc( (PVOID *) &pDexIda, 0L, (LONG) sizeof(DEXIDA), NOMSG ));
  if ( !fOK )
  {
    pDicExp->DDEReturn.usRc = ERROR_NOT_ENOUGH_MEMORY;
    UtlErrorHwnd( pDicExp->DDEReturn.usRc, MB_CANCEL, 0,
                  0, EQF_ERROR, pDicExp->hwndErrMsg);
  }
  else
  {
    //if file is not fully qualified -- qualify it with current
    //directory of requesting application (DDE Client)
    pFile = UtlGetFnameFromPath( pDicExp->szOutFile );

    if ( !pFile )
    {
      fOK = UtlInsertCurDir( pDicExp->szOutFile, &(pDicExp->ppCurDirArray),
                             pDexIda->szSgmlDictName );
      if ( !fOK )
      {
        //passed name is not valid
        fOK = FALSE;
        pDicExp->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
        pszName = pDicExp->szOutFile;
        UtlErrorHwnd( pDicExp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszName, EQF_ERROR, pDicExp->hwndErrMsg );
      } /* endif */
    }
    else
    {
      if ( strlen( pDicExp->szOutFile ) < sizeof(pDexIda->szSgmlDictName) )
      {
        strcpy( pDexIda->szSgmlDictName, pDicExp->szOutFile );
      }
      else
      {
        //passed name is not valid
        fOK = FALSE;
        pDicExp->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
        pszName = pDicExp->szOutFile;
        UtlErrorHwnd( pDicExp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszName, EQF_ERROR, pDicExp->hwndErrMsg );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( !pDicExp->fOverWrite && UtlFileExist( pDexIda->szSgmlDictName ))
    {
       //display file exist message
       PSZ pszError =  UtlGetFnameFromPath( pDexIda->szSgmlDictName );
       pDicExp->DDEReturn.usRc = FILE_EXISTS;
       usRc = UtlErrorHwnd( pDicExp->DDEReturn.usRc, MB_CANCEL,
                 1, &pszError, EQF_ERROR, pDicExp->hwndErrMsg );
       fOK = FALSE;
    }
    else
    {
      // open/create file
      {
        //if name valid open file or create new one
        USHORT usAction;
        USHORT usRc = UtlOpenHwnd( pDexIda->szSgmlDictName,
                                   &(pDexIda->hOut), &usAction,
                                   0L, FILE_NORMAL,
                                   FILE_TRUNCATE |
                                   FILE_CREATE,
                                   OPEN_ACCESS_READWRITE |
                                   OPEN_SHARE_DENYWRITE,
                                   0L, TRUE, pDicExp->hwndErrMsg );
        if ( usRc != NO_ERROR )
        {
          fOK = FALSE;
          pDicExp->DDEReturn.usRc = usRc;
        } /* endif */
      }
    } /* endif */
  } /* endif */


  if ( fOK )
  {
    //open properties of existing dict to check if dictionary exists
    pszName = pDicExp->szName;   //name of existing asd dict
    PROPNAME ( pDexIda->szString, pszName );
    hDictProp = OpenProperties( pDexIda->szString, NULL,
                   PROP_ACCESS_READ, &ErrorInfo);

    if ( !hDictProp )
    {
      //dict props of existing dict cannot be opened
      fOK = FALSE;
      pDicExp->DDEReturn.usRc = EQFS_FILE_OPEN_FAILED;
      pszName = pDicExp->szName;   //name of existing asd dict
      UtlErrorHwnd( pDicExp->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszName, EQF_ERROR, pDicExp->hwndErrMsg );
    }
    else
    {
      PPROPDICTIONARY pProp = (PPROPDICTIONARY)MakePropPtrFromHnd( hDictProp );
      strcpy( pDexIda->szDictName, pProp->szDictPath );
      Utlstrccpy( pDexIda->szShortName, pProp->PropHead.szName, DOT );
      CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Check if the dictionary is locked                               */
  /*******************************************************************/
  if ( fOK )
  {
     HWND         hwndObj;

     UtlMakeEQFPath( pDexIda->szPath, NULC, SYSTEM_PATH, (PSZ) NULP );
     sprintf( pDexIda->szFilePath, "%s\\%s%s",
              pDexIda->szPath, pDicExp->szName, EXT_OF_DICTPROP );
     hwndObj = EqfQueryObject( pDexIda->szFilePath, clsDICTEXP, 0 );
     if ( hwndObj != NULLHANDLE )
     {
        fOK = FALSE;
        pDicExp->DDEReturn.usRc = ERROR_DICT_LOCKED;
        pszName = pDicExp->szName;
        UtlErrorHwnd( pDicExp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszName, EQF_ERROR, pDicExp->hwndErrMsg );
     }  /* endif */
  } /* endif */

  //init import processing
  if ( fOK )
  {
    //copy passed data
    strcpy( pDexIda->IdaHead.szObjName,
            UtlGetFnameFromPath( pDexIda->szSgmlDictName ) );
    pDexIda->IdaHead.pszObjName = pDexIda->IdaHead.szObjName;
    pDexIda->hwndError = pDicExp->hwndErrMsg;
    pDexIda->pDicExp = pDicExp;
    pDexIda->fAscii = TRUE;
    strcpy( pDexIda->szName, pDicExp->szName );
    strncpy( pDexIda->szDrive, pDexIda->szSgmlDictName, 2 );
    pDexIda->szDrive[2] = EOS;
    strcpy( pDexIda->szPath, pDexIda->szSgmlDictName );
    UtlSplitFnameFromPath( pDexIda->szPath );
    pDexIda->usExpFormat = DICT_FORMAT_SGML_ASCII;
    pDexIda->ulOemCP = GetLangOEMCP(NULL);      // retrieve CP from system preferences lang
    pDexIda->ulAnsiCP = GetLangAnsiCP(NULL);

    // create an invisible process window
    fOK = CreateProcessWindow2( pDexIda->IdaHead.pszObjName,
                                DicExportCallBack, pDexIda, FALSE );
  } /* endif */

  if ( !fOK )
  {
    //notify caller that something went wrong
    WinPostMsg( pDicExp->hwndOwner, WM_EQF_DDE_ANSWER,
                NULL, MP2FROMP( &pDicExp->DDEReturn ) );
  } /* endif */
} /* end of function DicBatchExp */


#ifdef FUNCCALLIF
USHORT DicFuncExportDict
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszDictName,             // name of dictionary
  LONG        lOptions,                // dictionary export options or 0L
  PSZ         pszOutFile               // fully qualified name of output file
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new export or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new import
    usRC = DicFuncPrepExport( pData, pszDictName, lOptions, pszOutFile );
  }
  else
  {
    // continue current import process
    usRC = DicFuncExportProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function DicFuncExportDict*/

// prepare the batch export of a dictionary
USHORT DicFuncPrepExport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszDictName,             // name of dictionary
  LONG        lOptions,                // dictionary export options or 0L
  PSZ         pszOutFile               // fully qualified name of output file
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fOK = TRUE;              // error flag
  PDEXIDA     pDexIda = NULL;          // Dict export ida
  PSZ         pszName;                 // error code string
  HPROP       hDictProp;               // dict handle
  EQFINFO     ErrorInfo;               // error msg

  // set continue flag
  pData->fComplete = FALSE;

  // allocate dict export ida
  fOK = (UtlAlloc( (PVOID *) &pDexIda, 0L, (LONG) sizeof(DEXIDA), NOMSG ));
  if ( !fOK )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
  }
  else
  {
    pData->pvDicExportIda = (PVOID)pDexIda;
  } /* endif */

  // check if output file has been specified
  if ( fOK )
  {
    if ( (pszOutFile == NULL) || (*pszOutFile == EOS) )
    {
      fOK = FALSE;
      usRC = ERROR_NO_EXPORT_NAME;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check output file name
  if ( fOK )
  {
    if ( strlen( pszOutFile ) < sizeof(pDexIda->szSgmlDictName) )
    {
      strcpy( pDexIda->szSgmlDictName, pszOutFile );
    }
    else
    {
      //passed name is not valid
      fOK = FALSE;
      usRC = UTL_PARAM_TOO_LONG;
      pszName = pszOutFile;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszName, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( !(lOptions & OVERWRITE_OPT)  &&
         UtlFileExist( pDexIda->szSgmlDictName ))
    {
       //display file exist message
       PSZ pszError =  UtlGetFnameFromPath( pDexIda->szSgmlDictName );
       usRC = FILE_EXISTS;
       UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszError, EQF_ERROR, HWND_FUNCIF );
       fOK = FALSE;
    }
    else
    {
      // open/create file
      {
        // if name valid open file or create new one
        USHORT usAction;
        USHORT usRc = UtlOpenHwnd( pDexIda->szSgmlDictName,
                                   &(pDexIda->hOut), &usAction,
                                   0L, FILE_NORMAL,
                                   FILE_TRUNCATE |
                                   FILE_CREATE,
                                   OPEN_ACCESS_READWRITE |
                                   OPEN_SHARE_DENYWRITE,
                                   0L, TRUE, HWND_FUNCIF );
        if ( usRc != NO_ERROR )
        {
          fOK = FALSE;
          usRC = UtlQueryUShort( QS_LASTERRORMSGID );
        } /* endif */
      }
    } /* endif */
  } /* endif */

  // check if dictionary has been specified
  if ( fOK )
  {
    if ( (pszDictName == NULL) || (*pszDictName == EOS) )
    {
      fOK = FALSE;
      usRC = NO_NEW_DICTIONARY_SELECTED;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // process name of dictionary being exported
  if ( fOK )
  {
    BOOL fIsNew = FALSE;         // is-new flag
    ANSITOOEM( pszDictName );
    strcpy( pDexIda->szName, pszDictName );
    ObjLongToShortName( pszDictName, pDexIda->szShortName, DICT_OBJECT, &fIsNew );
    OEMTOANSI( pszDictName );
  } /* endif */


  if ( fOK )
  {
    //open properties of existing dict to check if dictionary exists
    pszName = pDexIda->szShortName;   //name of existing asd dict
    PROPNAME ( pDexIda->szString, pszName );
    hDictProp = OpenProperties( pDexIda->szString, NULL,
                   PROP_ACCESS_READ, &ErrorInfo);

    if ( !hDictProp )
    {
      //dict props of existing dict cannot be opened
      fOK = FALSE;
      usRC = EQFS_FILE_OPEN_FAILED;
      pszName = pszDictName;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszName, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      PPROPDICTIONARY pProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hDictProp );
      strcpy( pDexIda->szDictName, pProp->szDictPath );
      CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */

  // check if the dictionary is locked
  if ( fOK )
  {
     HWND         hwndObj;

     UtlMakeEQFPath( pDexIda->szPath, NULC, SYSTEM_PATH, (PSZ) NULP );
     sprintf( pDexIda->szFilePath, "%s\\%s%s",
              pDexIda->szPath, pDexIda->szShortName, EXT_OF_DICTPROP );
     hwndObj = EqfQueryObject( pDexIda->szFilePath, clsDICTEXP, 0 );
     if ( hwndObj != NULLHANDLE )
     {
        fOK = FALSE;
        usRC = ERROR_DICT_LOCKED;
        pszName = pszDictName;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszName, EQF_ERROR, HWND_FUNCIF );
     }  /* endif */
  } /* endif */

  //init import processing
  if ( fOK )
  {
    // set dictionary export format (default = Unicode)
    if ( lOptions & ASCII_OPT )
    {
      pDexIda->usExpFormat = DICT_FORMAT_SGML_ASCII;
    }
    else if ( lOptions & ANSI_OPT )
    {
      pDexIda->usExpFormat = DICT_FORMAT_SGML_ANSI;
    }
    else if ( lOptions & DXT_UTF8_OPT )
    {
      pDexIda->usExpFormat = DICT_FORMAT_XML_UTF8;
    }
    else
    {
      pDexIda->usExpFormat = DICT_FORMAT_SGML_UNICODE;
      // write unicode prefix
	  if ( fOK  )
	  {
	       ULONG ulWritten;
	       UtlWriteL( pDexIda->hOut, UNICODEFILEPREFIX, strlen(UNICODEFILEPREFIX),
	                 &ulWritten, FALSE );
      } /* endif */

    } /* endif */

    //copy passed data
    strcpy( pDexIda->IdaHead.szObjName,
            UtlGetFnameFromPath( pDexIda->szSgmlDictName ) );
    pDexIda->IdaHead.pszObjName = pDexIda->IdaHead.szObjName;
    pDexIda->hwndError = HWND_FUNCIF;
    pDexIda->pDicExp = NULL;
    pDexIda->fAscii = TRUE;
    strncpy( pDexIda->szDrive, pDexIda->szSgmlDictName, 2 );
    pDexIda->szDrive[2] = EOS;
    strcpy( pDexIda->szPath, pDexIda->szSgmlDictName );
    UtlSplitFnameFromPath( pDexIda->szPath );
    pDexIda->ulOemCP = GetLangOEMCP(NULL);      // retrieve CP from system preferences lang
    pDexIda->ulAnsiCP = GetLangAnsiCP(NULL);
  } /* endif */

  // cleanup in case of errors
  if ( !fOK )
  {
    if ( pDexIda )
    {
      if ( pDexIda->hOut )
      {
        UtlClose( pDexIda->hOut, FALSE );
      } /* endif */
      UtlAlloc( (PVOID *)&pDexIda, 0L, 0L, NOMSG );
    } /* endif */
    pData->fComplete = TRUE;
  } /* endif */
  return( usRC );
} /* end of function DicFuncPrepExport */

// batch dictionary export process
USHORT DicFuncExportProcess
(
  PFCTDATA    pData                    // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PDEXIDA     pDexIda = NULL;          // Dict export ida

  pDexIda = (PDEXIDA) pData->pvDicExportIda;

  if ( !DexProcess( HWND_FUNCIF, pDexIda ) )
  {
    DexCleanUp( pDexIda );
    pData->fComplete = TRUE;
    if ( pDexIda->fHalfCompleted )
    {
      usRC = UtlQueryUShort( QS_LASTERRORMSGID );
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function DicFuncExportProcess */


#else
USHORT DicFuncExportDict( USHORT usDummy );
USHORT DicFuncExportDict( USHORT usDummy ) { return( 0 ); }
#endif

BOOL DicCheckAndPrepareExport
(
  PDEXIDA     pDexIda,                 // export IDA
  BOOL        fMsg,                    // TRUE = show error messages in case of errors
  PBOOL       pfCancelled              // callers cancelled flag
)
{
  BOOL        fOK = TRUE;              // function return code
  PSZ         pszError;                // general error parameter
  
  *pfCancelled = FALSE;

  if ( fOK )
  {
      HPROP hDictProp;
      EQFINFO     ErrorInfo;                   //error return code

      //open dictionary properties to get fully qualified dictionary path
      //and copyright and protection information
      {
        BOOL fIsNew = FALSE;         // folder-is-new flag
        ObjLongToShortName( pDexIda->szName, pDexIda->szShortName,
                            DICT_OBJECT, &fIsNew );
      }
      pszError = pDexIda->szName; //dict for export without extension
      PROPNAME ( pDexIda->szString, pDexIda->szShortName );
      hDictProp = OpenProperties( pDexIda->szString, NULL,
                                  PROP_ACCESS_WRITE, &ErrorInfo);
      if ( hDictProp )
      {
        PPROPDICTIONARY pDictProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hDictProp );
        if ( pDictProp->fCopyRight && fOK )
        {
           //dict is copyrighted and export not permitted
           pszError = pDexIda->szName;
           if ( fMsg ) UtlError( ERROR_SYSTDICT_COPYRIGHTED, MB_CANCEL, 1, &pszError, EQF_ERROR );
           fOK = FALSE;
        }
        else
        {
           //Is the dictionary protected?
           if ( pDictProp->fProtected && fOK )
           {
              //authorization disallowed
              //dict is protected and export isn't permitted
              pszError = pDexIda->szName;
              if ( fMsg ) UtlError( ERROR_EXPORTDICT_PROTECTED, MB_CANCEL, 1, &pszError, EQF_ERROR );
              fOK = FALSE;
           } /* endif */
        } /* endif */

        //get fully qualified path of asd file
        strcpy(pDexIda->szDictName, pDictProp->szDictPath );

        //check if enough room for export, get size of asd dict file
        if ( fOK )
        {
          USHORT usError = NO_ERROR;
          USHORT usRc = NO_ERROR;
          LONG lBytesShort;
          PSZ pszDrive = pDexIda->szDrive;

          usRc = UtlCheckSpaceForFileEx( pDictProp->szDictPath, 100, 0L,
                                         &pszDrive, &lBytesShort, FALSE, &usError );
          if ( usError != NO_ERROR )
          {
            usRc = FALSE; // function failed
          }
          else if ( !usRc )
          {
            // low on storage condition
            usRc = TRUE;
            lBytesShort = 1;
          } /* endif */

          if ( !usRc )
          {
            //error handling
            if ( fMsg )
            {
              usRc = DictRcHandling( usError, pDictProp->szDictPath, NULLHANDLE,
                                     pDictProp->szServer );
            } /* endif */
            fOK = FALSE;           //error occurred with server code
          }
          else
          {
            if ( usRc && lBytesShort )
            {
              pszError = pDexIda->szDrive;
              if ( fMsg )
              {
                usRc = UtlError( NO_SPACE_FOR_EXPORT, MB_YESNOCANCEL,
                                 1, &pszError, EQF_QUERY );
                fOK = ( usRc == MBID_YES );
              }
              else
              {
                fOK = TRUE;            // continue anyway
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */

        //close properties
        CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo );
      }
      else
      {
         //dict props cannot be opened
         pszError = pDexIda->szName;
         if ( fMsg ) UtlError( ERROR_OPENING_PROPS, MB_CANCEL,1, &pszError, EQF_ERROR );
         fOK = FALSE;
      }
  } /* endif */

  // overwrite confirmation check
  if ( fOK && UtlFileExist(pDexIda->szSgmlDictName) )
  {
    if ( (pDexIda->hwnd != HWND_FUNCIF) && !pDexIda->fYesToAll )
    {
       //display file exist message
       PSZ pszError =  UtlGetFnameFromPath( pDexIda->szSgmlDictName );
       USHORT usMBCode = UtlErrorHwnd( FILE_EXISTS, 
         ( pDexIda->pDictList ) ? (MB_EQF_YESTOALL | MB_DEFBUTTON3) : (MB_DEFBUTTON2 | MB_YESNO), 
         1, &pszError, EQF_WARNING, pDexIda->hwnd );
       if ( usMBCode == MBID_EQF_YESTOALL )
       {
         pDexIda->fYesToAll = TRUE;
         usMBCode = MBID_YES;
       } /* endif */
       if ( usMBCode != MBID_YES)
       {
         fOK = FALSE;
         if ( usMBCode == MBID_CANCEL )
         {
           *pfCancelled = TRUE;
         } /* endif */
       } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
     //if name valid open file or create new one
     USHORT usAction;
     USHORT usRc = UtlOpen( pDexIda->szSgmlDictName, &(pDexIda->hOut), &usAction,
                     0L, FILE_NORMAL, FILE_TRUNCATE | FILE_CREATE,
                     OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, fMsg );
     fOK = (usRc == NO_ERROR);
  } /* endif */

  // write unicode prefix to output file if necessary
  if ( fOK && (pDexIda->usExpFormat == DICT_FORMAT_SGML_UNICODE) )
  {
     ULONG ulWritten;
     UtlWriteL( pDexIda->hOut, UNICODEFILEPREFIX, strlen(UNICODEFILEPREFIX),
               &ulWritten, FALSE );
  } /* endif */

  return( fOK );
} /* end of function DicCheckAndPrepareExport */

// escape characters in segment data to form valid XML 
VOID DexEscapeXMLChars( PSZ_W pszText, USHORT *pusLength )
{
  while ( *pszText )
  {
    if ( *pszText == L'&' )
    {
       memmove( pszText+5, pszText+1, (wcslen(pszText+1)+1)*sizeof(CHAR_W) ) ; 
       wcsncpy( pszText, L"&amp;", 5 );
       *pusLength += 4 ;
    }
    else if ( *pszText == L'<' )
    {
       memmove( pszText+4, pszText+1, (wcslen(pszText+1)+1)*sizeof(CHAR_W) ) ; 
       wcsncpy( pszText, L"&lt;", 4 );
       *pusLength += 3 ;
    }
    else if ( *pszText == L'>' )
    {
       memmove( pszText+4, pszText+1, (wcslen(pszText+1)+1)*sizeof(CHAR_W) ) ; 
       wcsncpy( pszText, L"&gt;", 4 );
       *pusLength += 3 ;
    }
//  Escaping quotes is only necessary in attribute values.
//  else if ( *pszText == L'\"' )
//  {
//     wmemmove( pszText+6, pszText+1, (wcslen(pszText+1)+1)*sizeof(CHAR_W) ) ;
//     wcsncpy( pszText, L"&quot;", 6 );                           
//     *pusLength += 5 ;                                      
//  }
//  else if ( *pszText == L'\'' )
//  {
//     wmemmove( pszText+6, pszText+1, (wcslen(pszText+1)+1)*sizeof(CHAR_W) ) ;
//     wcsncpy( pszText, L"&apos;", 6 );                           
//     *pusLength += 5 ;                                      
//  }
    else if ( (*pszText == L'\x1F') || (*pszText == L'\t') )
    {
      // suppress some special characters
      *pszText = L' ';
    }
    pszText++;
  } /*endwhile */
  *pszText = 0;
}
