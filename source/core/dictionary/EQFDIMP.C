/*! \file
	Description: Dialog to import a dictionary.
	This program provides the end user dialog and processing of a dictionary for import.
	
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include <stdlib.h>
#include <time.h>

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM              // QDAM files
#include "eqf.h"                  // General Translation Manager include file

#define DICTPROP
#include "eqfdtag.h"              // include tag definitions
#include "eqfdde.h"
#include "EQFDICTI.H"             // Private include file of dictionary handler
#include "OtmDictionaryIF.H"
#include "eqfrdics.h"
#include "eqfdimp.id"
#include "eqfdic00.id"
#ifdef FUNCCALLIF
  #include "OTMFUNC.H"            // public defines for function call interface
  #include "EQFFUNCI.H"           // private defines for function call interface
#endif

extern HELPSUBTABLE hlpsubtblDicImpDlg[];


//declare functions
MRESULT DicImportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

static VOID DimpWork( HWND, PDIMPIDA );
static VOID DimpCleanUp ( PDIMPIDA );
static VOID FolderImportWork( HWND, PDIMPIDA );
VOID DictionaryImportWindow( HWND, PDIMPIDA );
VOID DictionaryFolderImport( HWND, PDIMPIDA, USHORT );
static BOOL FolderImportInit( HWND, PDIMPIDA );
static BOOL DictMerge ( PDIMPIDA );

static VOID FolderImportCopy
(
PDIMPIDA pDimpIda,
CHAR chDictDrive,
HWND hwnd,
BOOL fPropUpdate,
USHORT usLocation,
PSZ  pszFromDict,
PSZ  pszToDict
);

static BOOL CorrectProps(  PDIMPIDA, PSZ, BOOL, HWND );
static VOID UtlAddImportToPath( PDIMPIDA, PSZ );
static VOID UtlRemoveImportFromPath( PDIMPIDA, PSZ );
static BOOL DimpClose( PDIMPIDA );
static BOOL DimpRead( PDIMPIDA  );
static BOOL DimpWrite( PDIMPIDA );
static BOOL DimpProcess ( HWND, PDIMPIDA );
static BOOL DimpEntry ( PDIMPIDA, SHORT, PTOKENENTRY * );
static BOOL DimpMapTable ( HWND, PDIMPIDA, SHORT, PTOKENENTRY * );
static BOOL DimpHeader ( PDIMPIDA, SHORT, PTOKENENTRY * );
static BOOL DimpInit( HWND, PDIMPIDA );
static PVOID CreateTagTable ( PDIMPIDA, PSZ );
static BOOL TextMatch ( PDIMPIDA, PTOKENENTRY *, PSZ_W * );
static VOID SkipTag( PDIMPIDA, PTOKENENTRY *);
static BOOL ValidTag( PDIMPIDA, PTOKENENTRY *, PBOOL, SHORT);
static BOOL SaveString( PDIMPIDA, PSZ_W * , ULONG );
static BOOL CheckForEnd( PDIMPIDA, PTOKENENTRY, SHORT );
static BOOL FillMapTable( PDIMPIDA, PSZ_W , SHORT, SHORT,
                          USHORT, USHORT, BOOL, BOOL, USHORT );
static BOOL FillEntry( PDIMPIDA, PSZ_W , SHORT, SHORT, PTOKENENTRY );
static HFILE DimpOpenFile ( PDIMPIDA, PSZ, PULONG );
static BOOL IDMatch( PDIMPIDA, PTOKENENTRY *, PSZ_W *, PUSHORT  );
static PPROPDICTIONARY FillProfile( PDIMPIDA );
static VOID WriteSubTree ( PDIMPIDA );
static BOOL CompareProfiles( PDIMPIDA );
static BOOL NewMapEntry( PDIMPIDA );
static PSZ_W CheckAttribute( PDIMPIDA, PTOKENENTRY , PSZ_W );
static PSZ_W CheckAttributeXML( PDIMPIDA, PTOKENENTRY , PSZ_W );
static BOOL MapIdMatch( PDIMPIDA, PTOKENENTRY *, PSZ_W *, PUSHORT,
                        PUSHORT, PUSHORT, PBOOL, PBOOL, PUSHORT  );
static BOOL EndMapTableProcess( HWND, PDIMPIDA );
static VOID AddDate( PDIMPIDA );
static VOID ResolveXMLEntities( PSZ_W );

static PFNWP pfnwpDimpFrameProc;        // address of old frame proc

BOOL DicImportFileOpenDialog( PDIMPIDA pDimpIda );
UINT_PTR CALLBACK DicOpenFileHook( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
static PSZ_W Save3Strings( PDIMPIDA pDimpIda, PSZ_W pString1, PSZ_W pString2, PSZ_W pString3 );


USHORT DicFuncPrepImport
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszInFile,               // fully qualified name of input file
PSZ         pszDictName,             // name of dictionary
PSZ         pszPassword,             // password of dictionary
LONG        lOptions                 // dictionary import options
);
USHORT DicFuncImportProcess
(
PFCTDATA    pData                    // function I/F session data
);

static USHORT
HandleCodePageToken
(
	PDIMPIDA      pDimpIda,
	PTOKENENTRY * pToken,         // ptr to token table
	PSZ_W       * pDummyW,
	PBOOL         pfReadAgain,
	PBOOL         pfMsgDisplayed
);

static USHORT
DimpReadtoUnicode
(
	PDIMPIDA  pDimpIda,
	PULONG    pulFreeConvBuffer,
	PULONG    pulFilled,
	PULONG    pulFreeBuffer,
	PBOOL     pfMsgDisplayed
);
static USHORT
CheckForCodePage
(
	PDIMPIDA pDimpIda,
	PBOOL    pfReadAgain,
	PBOOL    pfMsgDisplayed
);


//This is the dictionary import main function which first calls up the dialog
//and initializes the actual import routine. It is called up with the handle
//of the 'dictionary list' window and a pointer to the selected dictionary
//in the 'dictionary list' window.

//Outline of SGML import to new dictionary or to an existing dictionary
//
//       if ( dict for import does not exist )
//       {
//         Import normally
//       }
//       else  ( merge of sgml into existing dict )
//       {
//          if ( existing dict is copyrighted )
//          {
//             Msg that file cannot be imported
//          }
//          else
//          {
//             if ( existing dict is protected )
//             {
//                Query password dialog
//             }
//             else
//             {
//                Merge dialog
//             } /* endif */
//          } /* endif */
//       } /* endif */
//
//Outline of folder dictionary import (either merge or copy)
//
//       if ( a troja dict exists )
//       {
//          if ( folder dict is protected )
//          {
//             if ( troja dict is protected )
//             {
//                copy if newer
//             }
//             else
//             {
//                no merge
//             }
//          }
//          else
//          {
//             if ( troja dict is protected )
//             {
//                query password dialog for troja dict
//             }
//             else
//             {
//                merge dialog
//             }
//          }
//       }
//       else //file doesn't exist
//       {
//          copy
//       }

VOID DictionaryImport
(
HWND hwnd,
PSZ pSelDictName,
BOOL fImpMerge,
HWND hwndErrMsg                      // parent handle for error message boxes
)
{
  PDIMPIDA        pDimpIda;                    // Dic import dialog IDA
  BOOL            fOK = TRUE;                  // return value
//  INT_PTR          usResult = 0;                // return value of WinDlgBox
  PSZ             pDictName;                   // pointer to dictionary fname
  USHORT          usDictNum = 0;

  // create DimpIda
  fOK = (UtlAllocHwnd( (PVOID *)&pDimpIda, 0L, (LONG) sizeof(DIMPIDA), ERROR_STORAGE,
                       hwndErrMsg ));

  if ( fOK )
  {
    if ( (hwndErrMsg == HWND_FUNCIF) || ISBATCHHWND(hwndErrMsg) )
    {
      pDimpIda->HWNDImpCmd = hwndErrMsg;
    }
    else
    {
      pDimpIda->HWNDImpCmd = hwnd;
    } /* endif */
    pDimpIda->ulOEMCP = GetLangOEMCP(NULL);  // use system preferences language
    pDimpIda->ulAnsiCP = GetLangAnsiCP(NULL);  // use system preferences lang.
    pDimpIda->DictLBhwnd = hwnd;
    pDimpIda->fImpMerge = fImpMerge;
    if ( fImpMerge )
    {
      //set merge entry flags
      pDimpIda->usFlags = MERGE_ADD | MERGE_USERPROMPT | MERGE_SOURCE_ASD;

      // Copy the invoking handler name to the IDA. The handler name will be
      // used to send the WM_EQFN_TASKDONE notification message at the end
      // of the process
      pDictName = strchr( pSelDictName, X15 );
      *pDictName = EOS;
      strcpy( pDimpIda->szInvokingHandler, pSelDictName );
      pSelDictName = pDictName + 1;

      pDimpIda->usDictNum = 0;  //initialize dict counter
      // Copy the dicts for import into FolderDict array
      while ( *pSelDictName )
      {
        pDictName = strchr( pSelDictName, X15 );
        if ( pDictName )
        {
          *pDictName = EOS;
        }
        else
        {
          pDictName = pSelDictName + strlen(pSelDictName) - 1;
        } /* endif */
        pDimpIda->usDictNum++;
        strcpy( pDimpIda->szFolderDicts[usDictNum], pSelDictName );
        usDictNum++;
        pSelDictName = pDictName + 1;
      } /* endwhile */

      pDimpIda->usDictLeft = pDimpIda->usDictNum;
      pDimpIda->usDictNum = 0;
      pDimpIda->fNextDict = TRUE;
      DictionaryFolderImport( hwnd, pDimpIda, pDimpIda->usDictNum );
    }
    else
    {
      //remember selected dictionary in Ida
      if ( pSelDictName != NULC )
      {
        strcpy( pDimpIda->szNewDictName, pSelDictName );
      } /* endif */

      //set merge entry flags
      pDimpIda->usFlags = MERGE_ADD | MERGE_USERPROMPT | MERGE_SOURCE_SGML;

      //get pointer to system properties
      pDimpIda->pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd() );

      memcpy(pDimpIda->sOrgToken, FORMATTABLE, sizeof(FORMATTABLE));

      if (  DicImportFileOpenDialog( pDimpIda ) )
      {
        DictionaryImportWindow( hwnd, pDimpIda );
      }
    } /* endif */
  } /* endif */
} /* DictionaryImport */

VOID DictionaryFolderImport( HWND hwnd, PDIMPIDA pDimpIda, USHORT usDictNum )
{
  HPROP           hFolderDictProp;              //dict handle
  HPROP           hTrojaDictProp;               //dict handle
  USHORT          usResult;                     //return value
  PSZ             pszDictName;                  //pointer to dict name
  PSZ             pszError;                     //pointer to error string
  EQFINFO         ErrorInfo;                    //error return code
  PPROPDICTIONARY pFolderDictProp = NULL;       //folder dict props
  PPROPDICTIONARY pTrojaDictProp;               //existing dict prop
  BOOL            fOK = TRUE;                   //success indicator
  BOOL            fProcessWindow = FALSE;       // TRUE = import process window
                                                // has been started
  BOOL            fDictIsNew = FALSE;           // TRUE if this is a new dictionary
  BOOL        fIsNew;

  hFolderDictProp = NULL;
  hTrojaDictProp = NULL;
  usResult = 0;

  pDimpIda->fNextDict = FALSE;

  //get fully qualified asd path(+imp dir) and set szNewDictName to this value
  strcpy( pDimpIda->szNewDictName, pDimpIda->szFolderDicts[usDictNum] );

  // extract import directory name
  {
    PSZ pszImportDir = strchr( pDimpIda->szNewDictName, '\\' );
    if ( pszImportDir != NULL ) pszImportDir = strchr( pszImportDir + 1, '\\' );
    if ( pszImportDir != NULL ) Utlstrccpy( pDimpIda->szImportDir + 1, pszImportDir + 1, '\\' );
    pDimpIda->szImportDir[0] = '\\';
    pDimpIda->ulImportDirLength = strlen(pDimpIda->szImportDir);
  }

  //build property path for folder dict
  pszDictName = UtlGetFnameFromPath( pDimpIda->szNewDictName );
  UtlMakeEQFPath( pDimpIda->szDictName, pDimpIda->szNewDictName[0],
                  PROPERTY_PATH, NULL );
  strcat( pDimpIda->szDictName, BACKSLASH_STR );
  Utlstrccpy( pDimpIda->szDictName +
              strlen(pDimpIda->szDictName),
              pszDictName, DOT );
  strcat( pDimpIda->szDictName, EXT_OF_DICTPROP );
  UtlAddImportToPath( pDimpIda, pDimpIda->szDictName );

  //pDimpIda->szDictName is the fully qualified property filename path
  //including property directory and import directory of the
  //import dict.
  //pDimpIda->szNewDictName is the fully qualified dictionary
  //filename path with the import directory

  //szName contains fname for error msg issued
  Utlstrccpy( pDimpIda->szName,
              UtlGetFnameFromPath( pDimpIda->szDictName ),
              DOT );
  pszError = pDimpIda->szName;

  // load properties of imported dictionary in order to access dict long name
  {
    PPROPDICTIONARY pProp = NULL;     // ptr to dict properties
    BOOL fOK;
    ULONG ulReturn;

    UtlMakeEQFPath( pDimpIda->szString, NULC,
                    PROPERTY_PATH, NULL );
    strcat( pDimpIda->szString, BACKSLASH_STR );
    Utlstrccpy( pDimpIda->szString +
                strlen(pDimpIda->szString ),
                pszDictName, DOT );
    strcat( pDimpIda->szString, EXT_OF_DICTPROP );

    pDimpIda->szString[0] = pDimpIda->szNewDictName[0];
    UtlAddImportToPath( pDimpIda, pDimpIda->szString );
    fOK = UtlLoadFileL( pDimpIda->szString, (PVOID *)&pProp, &ulReturn,
                       FALSE, TRUE );
    if ( fOK )
    {
      if ( pProp->szLongName[0] != EOS )
      {
        strcpy( pDimpIda->szLongName, pProp->szLongName );
      }
      else
      {
        Utlstrccpy( pDimpIda->szLongName, pProp->PropHead.szName, DOT );
      } /* endif */
    } /* endif */
    pszError = pDimpIda->szLongName;
  }

  //check existance of such a prop file (without import dir) in troja
  UtlMakeEQFPath( pDimpIda->szString, NULC,
                  PROPERTY_PATH, NULL );
  strcat( pDimpIda->szString, BACKSLASH_STR );
  Utlstrccpy( pDimpIda->szString +
              strlen(pDimpIda->szString ),
              pszDictName, DOT );
  strcat( pDimpIda->szString, EXT_OF_DICTPROP );

  ObjLongToShortName( pDimpIda->szLongName, pDimpIda->szShortName, DICT_OBJECT, &fIsNew );
  if ( fIsNew )
  {
    PDICTPARMS pDictParms = NULL;     // dialog parms
    PPROPDICTIONARY pProp = NULL;     // ptr to dict properties
    ULONG      ulReturn;              // return value of UtlLoadFile
    USHORT     usLocation = 0;        // location type of new dictionary
    CHAR       chTargetDrive;         // target drive of new dictionary

    chTargetDrive = NULC;
    /*****************************************************************/
    /* Load properties of dictionary being imported                  */
    /*****************************************************************/
    pDimpIda->szString[0] = pDimpIda->szNewDictName[0];
    UtlAddImportToPath( pDimpIda, pDimpIda->szString );
    fOK = UtlLoadFileL( pDimpIda->szString, (PVOID *)&pProp, &ulReturn,
                       FALSE, TRUE );

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *) &pDictParms, 0L,
                      max( (LONG)sizeof(DICTPARMS), (LONG)MIN_ALLOC), ERROR_STORAGE );
    } /* endif */

    if ( fOK )
    {
      fDictIsNew = TRUE;
      if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
      {
        /********************************************************/
        /* Call Dictionary new dialog in NEWFOLIMP mode         */
        /* In this mode only the target drive and the mode      */
        /* local/shared can be selected.                        */
        /********************************************************/
        pDictParms->usType = NEWFOLIMP;
        pDictParms->pszData = (PSZ)pProp;
        fOK = DictionaryNew( hwnd, pDictParms );

        // use short name of created dictionary
        if ( fOK )
        {
          Utlstrccpy( pDimpIda->szShortName, pProp->PropHead.szName, DOT );
        } /* endif */
      } /* endif */

    } /* endif */

    if ( pDictParms ) UtlAlloc( (PVOID *) &(pDictParms), 0L, 0L, NOMSG);
    if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG);

    /*****************************************************************/
    /* Get shared/remote flag and target drive of new dictionary     */
    /*****************************************************************/
    if ( fOK )
    {
      if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
      {
        HPROP    hProp;
        EQFINFO  ErrorInfo;
        CHAR     szDictName[MAX_FILESPEC];

        Utlstrccpy( szDictName, pszDictName, DOT );
        PROPNAME ( pDimpIda->szString, szDictName );
        hProp = OpenProperties( pDimpIda->szString, NULL,
                                PROP_ACCESS_WRITE, &ErrorInfo);
        if ( hProp )
        {
          pProp = (PPROPDICTIONARY)MakePropPtrFromHnd( hProp );

          usLocation = pProp->usLocation;
          chTargetDrive = pProp->szDictPath[0];

          CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
        }
      }
      else
      {
        // in nonDDE import mode always copy to folder import drive
        // and us local dictionaries
        usLocation = LOC_LOCAL;
        chTargetDrive = pDimpIda->szDictName[0];
      } /* endif */
    } /* endif */

    /*****************************************************************/
    /* Copy the data file of the imported dictionary                 */
    /*****************************************************************/
    if ( fOK )
    {
      CHAR szFromDict[MAX_FNAME];
      CHAR szToDict[MAX_FNAME];
      Utlstrccpy( szFromDict, UtlGetFnameFromPath( pDimpIda->szDictName ), DOT );
      strcpy( szToDict, pDimpIda->szShortName );

      // in nonDDE mode we need a property file update
      FolderImportCopy( pDimpIda, chTargetDrive, pDimpIda->HWNDImpCmd,
                        (pDimpIda->HWNDImpCmd == HWND_FUNCIF), usLocation, szFromDict, szToDict );
    } /* endif */
  } /* endif */

  if ( fOK && !fDictIsNew )
  {
    //check existance of such a prop file (without import dir) in troja
    UtlMakeEQFPath( pDimpIda->szString, NULC,PROPERTY_PATH, NULL );
    strcat( pDimpIda->szString, BACKSLASH_STR );
    strcat( pDimpIda->szString, pDimpIda->szShortName );
    strcat( pDimpIda->szString, EXT_OF_DICTPROP );

    //if prop file exists
    if ( UtlFileExist( pDimpIda->szString ) )
    {
      //update Import property file before trying to open properties
      // UtlAddImportToPath( pDimpIda, pDimpIda->szDictName );

      //update prop file to contain the \import dir so
      //that prop can be opened (TRUE = add import to path when opening props)

      fOK = CorrectProps( pDimpIda, pDimpIda->szDictName, TRUE, pDimpIda->HWNDImpCmd );

      //open folder dict props if ok - error handling in CorrectProps
      if ( fOK )
      {
        pszDictName = UtlGetFnameFromPath( pDimpIda->szDictName );
        PROPNAME( pDimpIda->szString, pszDictName );
        UtlAddImportToPath( pDimpIda, pDimpIda->szString );
        pDimpIda->szString[0] = pDimpIda->szDictName[0]; //import folder drive
        hFolderDictProp = OpenProperties( pDimpIda->szString, NULL,
                                          PROP_ACCESS_READ,
                                          &ErrorInfo);
        if ( !hFolderDictProp  )
        {
          //msg that folder dict props could not be opened
          UtlErrorHwnd( ERROR_OPENING_PROPS, MB_CANCEL, 1,
                        &pszError, EQF_ERROR, pDimpIda->HWNDImpCmd );
          fOK = FALSE;
        }
        else
        {
          pFolderDictProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hFolderDictProp );
        } /* endif */

        //open troja dict props
        if ( fOK )
        {
          PROPNAME ( pDimpIda->szString, pDimpIda->szShortName );
          hTrojaDictProp = OpenProperties( pDimpIda->szString, NULL,
                                           PROP_ACCESS_READ,
                                           &ErrorInfo);

          if ( !hTrojaDictProp )
          {
            //troja dict props cannot be opened
            UtlErrorHwnd( ERROR_OPENING_PROPS, MB_CANCEL, 1,
                          &pszError, EQF_ERROR, pDimpIda->HWNDImpCmd );
            fOK = FALSE;
          }
          else
          {
            pTrojaDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hTrojaDictProp );

            if ( pFolderDictProp->fCopyRight && !pTrojaDictProp->fCopyRight )
            {
              // do not allow merge of copyrighted dictionary into non-copyrighted
              // dictionary
              UtlErrorHwnd( ERROR_IMPORTDICT_COPYRIGHTED, MB_CANCEL,
                            1, &pszError, EQF_ERROR, pDimpIda->HWNDImpCmd );
              fOK = FALSE;
            }
            else if ( pFolderDictProp->fProtected )
            {
              if ( pTrojaDictProp->fProtected )
              {
                //merge not allowed/ignored
                UtlErrorHwnd( ERROR_TROJADICT_PROTECTED, MB_CANCEL,
                              1, &pszError, EQF_ERROR, pDimpIda->HWNDImpCmd );
                fOK = FALSE;
              }
              else  //troja dict not protected
              {
                //merge not allowed/ignored
                UtlErrorHwnd( ERROR_FOLDERDICT_PROTECTED, MB_CANCEL,
                              1, &pszError, EQF_ERROR, pDimpIda->HWNDImpCmd );
                fOK = FALSE;
              } /* endif */
            }
            else  //folder dict not protected
            {
              if ( pTrojaDictProp->fProtected )
              {
                //disallow merge as troja dict protected
                UtlErrorHwnd( ERROR_TROJADICT_PROTECTED, MB_CANCEL,
                              1, &pszError, EQF_ERROR, pDimpIda->HWNDImpCmd );
                fOK = FALSE;
              }
              else
              {
                //allow merge of both local and remote dicts
                //allow import as neither dicts is protected
                if ( !fDictIsNew && (pDimpIda->HWNDImpCmd != HWND_FUNCIF) && !ISBATCHHWND(pDimpIda->HWNDImpCmd) )
                {
                  usResult = UtlError( FOLDER_MERGE,
                                       MB_YESNO | MB_DEFBUTTON2,
                                       1, &pszError, EQF_QUERY );
                }
                else
                {
                  usResult = MBID_YES;
                } /* endif */

                //if user does not want to merge skip import
                if ( usResult == MBID_NO )
                {
                  fOK = FALSE;
                }
                else
                {
                  pDimpIda->fMerge = TRUE;
                  if ( pDimpIda->HWNDImpCmd == HWND_FUNCIF )
                  {
                    // do the merge right now ...
                    FolderImportInit( hwnd, pDimpIda );
                    while ( !pDimpIda->fNotOk && !pDimpIda->fStop )
                    {
                      FolderImportWork( hwnd, pDimpIda );
                    } /* endwhile */
                    DimpCleanUp( pDimpIda );
                  }
                  else
                  {
                    DictionaryImportWindow( hwnd, pDimpIda );
                    fProcessWindow = TRUE;
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    }
    else //file doesn't exists
    {
      //check if asd/asi exist on the selected drive for folder
      //import and if so issue warning that dict cannot be copied
      //to specified drive as it already exists there
      //otherwise copy folder dict

      // Check for dictionary data file (.ASD)
      UtlRemoveImportFromPath( pDimpIda,  pDimpIda->szNewDictName ); //from asd path
      Utlstrccpy( pDimpIda->szString, pDimpIda->szNewDictName, DOT );
      strcat( pDimpIda->szString, EXT_OF_DIC );
      if ( UtlFileExist( pDimpIda->szString ) )
      {
        fOK = FALSE;
      } /* endif */

      // Check for dictionary index file (.ASI)
      if ( fOK )
      {
        Utlstrccpy( pDimpIda->szString, pDimpIda->szNewDictName, DOT );
        strcat( pDimpIda->szString, EXT_OF_DICTINDEX );
        if ( UtlFileExist( pDimpIda->szString ) )
        {
          fOK = FALSE;
        } /* endif */
      } /* endif */

      // Check for shared dictionary data file (.RAD)
      if ( fOK )
      {
        Utlstrccpy( pDimpIda->szString, pDimpIda->szNewDictName, DOT );
        strcat( pDimpIda->szString, EXT_OF_SHARED_DIC );
        if ( UtlFileExist( pDimpIda->szString ) )
        {
          fOK = FALSE;
        } /* endif */
      } /* endif */

      // Check for shared dictionary index file (.RAI)
      if ( fOK )
      {
        Utlstrccpy( pDimpIda->szString, pDimpIda->szNewDictName, DOT );
        strcat( pDimpIda->szString, EXT_OF_SHARED_DICTINDEX );
        if ( UtlFileExist( pDimpIda->szString ) )
        {
          fOK = FALSE;
        } /* endif */
      } /* endif */

      if ( !fOK )
      {
        //warning that dict exists
        Utlstrccpy( pDimpIda->szString,
                    UtlGetFnameFromPath( pDimpIda->szNewDictName ),
                    DOT );
        pszError = pDimpIda->szString;
        UtlErrorHwnd( ERROR_DICT_EXISTS_ON_OTHERDRIVE, MB_CANCEL,
                      1, &pszError, EQF_ERROR, pDimpIda->HWNDImpCmd );
      } /* endif */

      if ( fOK )
      {
        CHAR szFromDict[MAX_FNAME];
        CHAR szToDict[MAX_FNAME];
        Utlstrccpy( szFromDict, UtlGetFnameFromPath( pDimpIda->szDictName ), DOT );
        strcpy( szToDict, szFromDict );

        Utlstrccpy( pDimpIda->szString,
                    UtlGetFnameFromPath( pDimpIda->szNewDictName ),
                    DOT );
        UtlAddImportToPath( pDimpIda, pDimpIda->szNewDictName ); //asd file
        UtlAddImportToPath( pDimpIda, pDimpIda->szDictName ); //prop file
        FolderImportCopy( pDimpIda, pDimpIda->szNewDictName[0], pDimpIda->HWNDImpCmd,
                          TRUE, LOC_LOCAL, szFromDict, szToDict );
      } /* endif */
    } /* endif */

    if ( hFolderDictProp )
      CloseProperties( hFolderDictProp, PROP_QUIT, &ErrorInfo );
    if ( hTrojaDictProp )
      CloseProperties( hTrojaDictProp, PROP_QUIT, &ErrorInfo );

  } /* endif */

  if ( !fProcessWindow )
  {
    // no process window has been started, so delete the imported
    // dictionary and send the TASKDONE notification here ...
    strcpy( pDimpIda->szNewDictName, pDimpIda->szFolderDicts[usDictNum] );
    UtlDelete( pDimpIda->szNewDictName, 0L, FALSE );
    if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
    {
      EqfPost2Handler( pDimpIda->szInvokingHandler,
                       WM_EQFN_TASKDONE,
                       MP1FROMSHORT( !pDimpIda->fNoImp ), 0L);
    } /* endif */
    UtlAlloc( (PVOID *) &(pDimpIda), 0L, 0L, NOMSG);
  } /* endif */
} /* end DictionaryFolderImport */


VOID DictionaryImportWindow( HWND hwnd, PDIMPIDA pDimpIda )
{
  BOOL       fOK = TRUE;

  hwnd;
  strcpy( pDimpIda->IdaHead.szObjName, pDimpIda->szNewDictName );
  pDimpIda->IdaHead.pszObjName =  pDimpIda->IdaHead.szObjName;

  /*******************************************************************/
  /* Check if the dictionary is locked                               */
  /*******************************************************************/
  if ( pDimpIda->fMerge )
  {
    HWND         hwndObj;

    hwndObj = EqfQueryObject( pDimpIda->IdaHead.szObjName, clsDICTIMP, 0 );
    if ( hwndObj != NULLHANDLE )
    {
      PSZ       pszDictName;

      Utlstrccpy( pDimpIda->szString,
                  UtlGetFnameFromPath( pDimpIda->szNewDictName ), DOT );
      pszDictName = pDimpIda->szString;
      UtlErrorHwnd( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszDictName, EQF_ERROR,
                    pDimpIda->HWNDImpCmd );
      fOK = FALSE;
    }  /* endif */
  } /* endif */

  /*******************************************************************/
  /* Start dictionary import by creating the import process window   */
  /*******************************************************************/
  if ( fOK )
  {
    fOK = CreateProcessWindow( pDimpIda->IdaHead.pszObjName,
                               DicImportCallBack, pDimpIda );
    if ( !fOK )
    {
      // Issue the system error message
      UtlErrorHwnd( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR, pDimpIda->HWNDImpCmd );
    } /* endif */
  } /* endif */

} /* DictionaryImportwindow */

/**********************************************************************/
/* The following function ist the callback function for the dictionary*/
/* process window. The functions is called by the generic process     */
/* window function to perform specific tasks for dictionary import    */
/**********************************************************************/
MRESULT DicImportCallBack
(
PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
HWND             hwnd,               // handle of process window
WINMSG           message,            // message to be processed
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  PDIMPIDA        pDimpIda;            // pointer to instance area
  MRESULT         mResult = FALSE;     // return code for handler proc
  HMODULE hResMod;

  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  switch ( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
      /**************************************************************/
      /* Anchor IDA                                                 */
      /**************************************************************/
      pDimpIda            = (PDIMPIDA)PVOIDFROMMP2(mp2);
      pDimpIda->hwnd      = hwnd;
      pCommArea->pUserIDA = pDimpIda;

      /****************************************************************/
      /* supply all information required to create the process        */
      /* window                                                       */
      /****************************************************************/
      pCommArea->sProcessWindowID = ID_DICTIMP_WINDOW;
      pCommArea->sProcessObjClass = clsDICTIMP;
      if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
      {
        pCommArea->Style          = PROCWIN_BATCH;
      }
      else
      {
        pCommArea->Style          = PROCWIN_TEXTSLIDERENTRY;
      } /* endif */
      pCommArea->sSliderID        = ID_DIMPSLIDER;
      pCommArea->sTextID          = ID_IMPORTDICT_TEXT;
      pCommArea->sEntryGBID       = ID_INDICT_TEXT;

      /****************************************************************/
      /* Build titlebar text                                          */
      /****************************************************************/
      {
        PSZ       pszMsgTable[1];
        ULONG     Length;

        Utlstrccpy( pDimpIda->szName,
                    UtlGetFnameFromPath( pDimpIda->szNewDictName ), DOT );
        pszMsgTable[0] =  pDimpIda->szName;
        LOADSTRING( NULLHANDLE, hResMod, IDS_DIMP_TITLEBAR, pDimpIda->szString );
        DosInsMessage( pszMsgTable, 1, pDimpIda->szString, strlen ( pDimpIda->szString ),
                       pCommArea->szTitle, (sizeof ( pCommArea->szTitle ) - 1),
                       &Length );
        pCommArea->szTitle[Length] = EOS;
      }

      /****************************************************************/
      /* Load groupbox title and groupbox text                        */
      /****************************************************************/
      LOADSTRING( NULLHANDLE, hResMod, IDS_DIMP_ENTRY_STATICTEXT,
                  pCommArea->szGroupBoxTitle );
      pCommArea->sEntryID         = ID_ENTRY_TEXT;
      strcpy( pCommArea->szEntry, pDimpIda->szEntryTxt );
      OEMTOANSI( pCommArea->szEntry );

      /****************************************************************/
      /* Fill text line above the slider control                      */
      /****************************************************************/
      if ( !pDimpIda->fImpMerge )
      {
        PSZ       pszMsgTable[1];
        ULONG     Length;

        pszMsgTable[0] = pDimpIda->szDictName;
        LOADSTRING( NULLHANDLE, hResMod, IDS_DIMP_IMPORT_DICTNAME, pDimpIda->szString );
        DosInsMessage( pszMsgTable, 1, pDimpIda->szString,
                       strlen( pDimpIda->szString), pCommArea->szText,
                       (sizeof(pCommArea->szText) - 1), &Length );
        pCommArea->szText[Length] = EOS;
      }
      else
      {
        LOADSTRING( NULLHANDLE, hResMod, IDS_DIMP_IMPORT_FOLDTEXT,
                    pCommArea->szText );
      } /* endif */

      pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_DICTIMPICON); //hiconDICTIMP;
      pCommArea->fNoClose         = FALSE;
      pCommArea->swpSizePos.x     = 100;
      pCommArea->swpSizePos.y     = 100;
      pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 50;
      pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 14;
      pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
      pCommArea->asMsgsWanted[1]  = WM_EQF_SHUTDOWN;
      pCommArea->asMsgsWanted[2]  = WM_TIMER;
      pCommArea->asMsgsWanted[3]  = 0;
      pCommArea->usComplete       = 0;
      break;



      /****************************************************************/
      /* Start processing                                             */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      pDimpIda =(PDIMPIDA) pCommArea->pUserIDA;
      WinStartTimer( (HAB)UtlQueryULong( QL_HAB ), hwnd, TIMER, TIMEOUT);

      if ( pDimpIda->fImpMerge )
      {
        FolderImportInit( hwnd, pDimpIda );
      }
      else
      {
        DimpInit( hwnd, pDimpIda );
      } /* endif */
      break;

      /******************************************************************/
      /* WM_CLOSE:                                                      */
      /*                                                                */
      /* Prepare/initialize shutdown of process                         */
      /******************************************************************/
    case WM_CLOSE:
      pDimpIda =(PDIMPIDA) pCommArea->pUserIDA;
      if ( pDimpIda )
      {
        pDimpIda->fKill = TRUE;
        pDimpIda->fNotCompleted = TRUE;
        mResult = MRFROMSHORT( TRUE );   // = do not close right now
      }
      else
      {
        mResult = MRFROMSHORT( FALSE );  // = continue with close
      } /* endif */
      break;

      /******************************************************************/
      /* WM_DESTROY:                                                    */
      /*                                                                */
      /* Cleanup all resources used by the process                      */
      /******************************************************************/
    case WM_DESTROY:
      pDimpIda =(PDIMPIDA) pCommArea->pUserIDA;
      WinStopTimer( (HAB)UtlQueryULong( QL_HAB ), hwnd, TIMER );
      DimpCleanUp( pDimpIda );
      if ( !pDimpIda->fShutdown )
      {
        if ( pDimpIda->fImpMerge )
        {
//          if ( pDimpIda->usDictLeft )
//          {
//             pDimpIda->fKill = FALSE;
//             pDimpIda->fStop = FALSE;
//          } /* endif */
          EqfPost2Handler( pDimpIda->szInvokingHandler,
                           WM_EQFN_TASKDONE,
                           MP1FROMSHORT( !pDimpIda->fNoImp ), 0L);
          UtlAlloc( (PVOID *) &(pDimpIda), 0L, 0L, NOMSG);
          pCommArea->pUserIDA = NULL;
        }
        else
        {
          if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
          {
            pDimpIda->pDicImpExp->DDEReturn.usRc = pDimpIda->usDDERc;
            EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_NEXTSTEP,
                             MP1FROMSHORT( TASK_END ),
                             MP2FROMP(pDimpIda->pDicImpExp) );
          } /* endif */
          {
            // check if there are more dicts waiting for import
            if ( pDimpIda->pszList  )
            {
              // remove first entry from list if any
              if ( *pDimpIda->pszList )
              {
                PSZ pszTarget = pDimpIda->pszList;
                PSZ pszSource = pszTarget + strlen(pszTarget) + 1;
                while ( *pszSource )
                {
                  while ( *pszSource ) *pszTarget++ = *pszSource++;
                  *pszTarget++ = *pszSource++;
                } /* endwhile */
                *pszTarget = EOS;
              } /* endif */

              // restart next import
              if (  !pDimpIda->fNotOk && !pDimpIda->fNotCompleted && *pDimpIda->pszList )
              {
                PDIMPIDA pNewIda = NULL;
                if ( UtlAlloc( (PVOID *) &(pNewIda), 0L, sizeof(DIMPIDA), NOMSG) )
                {
                  BOOL fIsNew = FALSE;
                  pNewIda->pszList = pDimpIda->pszList;
                  strcpy( pNewIda->szDictName, pDimpIda->szDictName );
                  UtlSplitFnameFromPath( pNewIda->szDictName );
                  strcat( pNewIda->szDictName, BACKSLASH_STR );
                  strcat( pNewIda->szDictName, pNewIda->pszList );
                  Utlstrccpy( pNewIda->szLongName, pNewIda->pszList, DOT );
                  ObjLongToShortName( pNewIda->szLongName, pNewIda->szShortName, DICT_OBJECT, &fIsNew );
                  strcpy( pNewIda->szName, pNewIda->szShortName );
                  strcpy( pNewIda->szPath, pDimpIda->szPath );
                  strcpy( pNewIda->szDrive, pDimpIda->szDrive );
                  UtlMakeEQFPath( pNewIda->szPropPath, NULC, PROPERTY_PATH, NULL );
                  strcat( pNewIda->szPropPath, BACKSLASH_STR );
                  strcat( pNewIda->szPropPath, pNewIda->szShortName );
                  strcat( pNewIda->szPropPath, EXT_OF_DICTPROP );
                  UtlMakeEQFPath( pNewIda->IdaHead.szObjName, pNewIda->szDrive[0], DIC_PATH, NULL );
                  strcat( pNewIda->IdaHead.szObjName, BACKSLASH_STR );
                  strcat( pNewIda->IdaHead.szObjName, pNewIda->szShortName );
                  strcat( pNewIda->IdaHead.szObjName, EXT_OF_DIC );
                  strcpy( pNewIda->szNewDictName, pNewIda->IdaHead.szObjName );
                  pNewIda->IdaHead.pszObjName = pNewIda->IdaHead.szObjName;
                  UtlMakeEQFPath( pNewIda->szSymbol, NULC, SYSTEM_PATH, NULL );
                  strcat( pNewIda->szSymbol, BACKSLASH_STR );
                  strcat( pNewIda->szSymbol, pNewIda->szShortName );
                  strcat( pNewIda->szSymbol, EXT_OF_DIC );
                  pNewIda->usImpMode = pDimpIda->usImpMode;
                  pNewIda->HWNDImpCmd = pDimpIda->HWNDImpCmd;
                  pNewIda->fAscii = TRUE;
                  pNewIda->fMerge = !fIsNew;
                  memcpy( pNewIda->sOrgToken, FORMATTABLE, sizeof(FORMATTABLE) );

                  DictionaryImportWindow( hwnd, pNewIda );

                } /* endif */
              }
              else
              {
                UtlAlloc( (PVOID *)&pDimpIda->pszList, 0L, 0L, NOMSG );
              } /* endif */
            } /* endif */
          }
          UtlAlloc( (PVOID *) &(pDimpIda), 0L, 0L, NOMSG) ;
          pCommArea->pUserIDA = NULL;
        } /* endif */
      }
      else
      {
        if ( pDimpIda->fImpMerge )
        {
          //set var to be able to notify folder import handler
          //that something went wrong
          pDimpIda->fNoImp = TRUE;

          //set var to leave while loop
          pDimpIda->usDictLeft = 0;
        }
        else
        {
          UtlAlloc( (PVOID *) &(pDimpIda), 0L, 0L, NOMSG) ;
          pCommArea->pUserIDA = NULL;
        } /* endif */
      } /* endif */


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
      pDimpIda =(PDIMPIDA) pCommArea->pUserIDA;
      strcpy( pCommArea->szEntry, pDimpIda->szEntryTxt );
      OEMTOANSI( pCommArea->szEntry );
      break;

    case WM_EQF_SHUTDOWN:            // come here before EQF_TERMINATE
      pDimpIda =(PDIMPIDA) pCommArea->pUserIDA;
      pDimpIda->fShutdown = TRUE;
      pDimpIda->fFree = TRUE;
      pDimpIda->fNotCompleted = TRUE;
      mResult = (MRESULT)FALSE;
      break;

    case WM_EQF_PROCESSTASK:
      //process a term at a time
      pDimpIda =(PDIMPIDA) pCommArea->pUserIDA;
      if ( pDimpIda->fImpMerge )
      {
        FolderImportWork( hwnd, pDimpIda );
      }
      else
      {
        DimpWork( hwnd, pDimpIda );
      } /* endif */
      break;

  } /* endswitch */
  return( mResult );
}


//function to udate property file - if bool flag is set to TRUE the
//path name is to include the directory IMPORT. This update - either with
//or without the import directory is necessary in order to be able to open
//the dict properties.

static
BOOL CorrectProps( PDIMPIDA pDimpIda, PSZ pszPropPath, BOOL fInclImportDir, HWND hwnd )
{
  BOOL        fOK = TRUE;            // internal OK flag
  HFILE       hFile = 0;             // code returned by DosXXX calls
  USHORT      usDosRC;               // code returned by DosXXX calls
  USHORT      usOpenAction;          // action performed by DosOpen
  ULONG       ulBytesWritten;        // # of bytes written to file
  ULONG       ulBytesRead;           // # of bytes read from file
  ULONG       ulFilePos;             // position of file pointer
  PPROPDICTIONARY pPropDict;         // ptr to dictionary properties

  // correct property file
  fOK = UtlAlloc( (PVOID *) &pPropDict, 0L,
                  (LONG) sizeof(PROPDICTIONARY), ERROR_STORAGE );
  if ( fOK )
  {
    usDosRC = UtlOpenHwnd( pszPropPath,
                           &hFile,
                           &usOpenAction, 0L,
                           FILE_NORMAL,
                           FILE_OPEN,
                           OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                           0L,
                           TRUE, hwnd );
    fOK = ( usDosRC == 0 );
  } /* endif */
  if ( fOK )
  {
    usDosRC = UtlReadHwnd( hFile,
                           pPropDict,
                           sizeof( PROPDICTIONARY ),
                           &ulBytesRead,
                           TRUE, hwnd );
    fOK = ( usDosRC == 0 );
  } /* endif */
  if ( fOK )
  {
    if ( !fInclImportDir )
    {
      //system drive as prop exist there already
      UtlMakeEQFPath( pPropDict->PropHead.szPath, NULC, SYSTEM_PATH, NULL );
    }
    else
    {
      //drive from where the properties originate plus the added IMPORT
      //directory
      UtlMakeEQFPath( pPropDict->PropHead.szPath, pszPropPath[0],
                      SYSTEM_PATH, NULL );

      //importpath is defined in the corresponding include file
      strcat( pPropDict->PropHead.szPath, pDimpIda->szImportDir );

      //correct asd and asi path
      pPropDict->szDictPath[0] = pszPropPath[0];                  //1161a
      pPropDict->szIndexPath[0] = pszPropPath[0];                 //1161a
      UtlAddImportToPath( pDimpIda, pPropDict->szIndexPath );
      UtlAddImportToPath( pDimpIda, pPropDict->szDictPath );
    } /* endif */

    //if originally a remote dictionary, remove server name, userid and
    //primary remote drive letter
    if ( pPropDict->szServer[0] != NULC )
      pPropDict->szServer[0] = NULC;
    if ( pPropDict->szUserid[0] != NULC )
      pPropDict->szUserid[0] = NULC;
    if ( pPropDict->chRemPrimDrive != NULC )
      pPropDict->chRemPrimDrive = NULC;


    usDosRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, TRUE );
    if ( !usDosRC )
      usDosRC = UtlWriteHwnd( hFile,
                              pPropDict,
                              sizeof( PROPDICTIONARY ),
                              &ulBytesWritten,
                              TRUE, hwnd );
    fOK = ( usDosRC == 0 );
  } /* endif */
  if ( hFile )
    UtlClose( hFile, FALSE );

  UtlAlloc( (PVOID *) &pPropDict, 0L, 0L, NOMSG );

  return( fOK );
} /* end of CorrectProps */

//copy import dict to drive as specified in chDictDrive - if dict exists then
//copy to where this file exists else copy to drive to which the folder is to
//be imported

static
VOID FolderImportCopy
(
PDIMPIDA pDimpIda,
CHAR chDictDrive,
HWND hwnd,
BOOL fPropUpdate,
USHORT usLocation,
PSZ  pszFromDict,
PSZ  pszToDict
)
{
  PPROPDICTIONARY pDictProp = NULL;     // pointer to asd dict properties
  HPROP           hDicProp = NULL;      // pointer to asd handle
  EQFINFO         ErrorInfo;            // error code for dict props
  PSZ             pName;                // pointer to dict fname
  BOOL            fOK = TRUE;           // success indicator
  USHORT          usRc;

  pszFromDict;
  if ( fPropUpdate )
  {
    //build prop path with given name and system drive
    UtlMakeEQFPath( pDimpIda->szPath, NULC, PROPERTY_PATH, (PSZ)NULP );
    strcpy( pDimpIda->szName, pszToDict );
    sprintf( pDimpIda->szString, "%s\\%s%s",
             pDimpIda->szPath, pDimpIda->szName, EXT_OF_DICTPROP );

    //move properties from import drive and directory to system drive
    usRc = UtlSmartMoveHwnd( pDimpIda->szDictName, pDimpIda->szString, TRUE,
                             hwnd );

    //update property file before trying to open properties
    //change prophead to not contain the \import dir so that props can be opened
    if ( !usRc )
      fOK = CorrectProps(  pDimpIda, pDimpIda->szString, FALSE, hwnd );
    else
      fOK = FALSE;

    if ( fOK )
    {
      pName = UtlGetFnameFromPath( pDimpIda->szString );
      PROPNAME ( pDimpIda->szString, pName );
      hDicProp = OpenProperties( pDimpIda->szString, NULL,
                                 PROP_ACCESS_READ,
                                 &ErrorInfo);
    } /* endif */

    if ( hDicProp && fOK )
    {
      if ( SetPropAccess( hDicProp, PROP_ACCESS_WRITE) )
      {
        pDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hDicProp );
      }
      else
      {
        fOK = FALSE;
      }
    }
    else
    {
      pName = UtlGetFnameFromPath( pDimpIda->szString );
      UtlErrorHwnd( ERROR_OPENING_PROPS, MB_CANCEL, 1, &pName, EQF_ERROR, hwnd );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  //build asd path and copy
  if ( fOK )
  {
    if ( chDictDrive != NULC )
    {
      UtlMakeEQFPath( pDimpIda->szPath, chDictDrive, DIC_PATH,
    		  (PSZ)NULP );
    }
    else
    {
      //system drive - default
      UtlMakeEQFPath( pDimpIda->szPath, pDimpIda->szDictName[0], DIC_PATH,
    		  (PSZ) NULP );
    } /* endif */
    sprintf( pDimpIda->szString, "%s\\%s%s",
             pDimpIda->szPath, pszToDict,
             (usLocation == LOC_LOCAL ) ? EXT_OF_DIC : EXT_OF_SHARED_DIC );

    if ( fPropUpdate )
    {
      //update property with fully qualified dict path (asd path)
      strcpy( pDictProp->szDictPath, pDimpIda->szString );
      pDictProp->usLocation = usLocation;
    } /* endif */

    //copy folder dict asd file
    UtlDelete( pDimpIda->szString, 0L, FALSE );
    usRc = UtlSmartMoveHwnd( pDimpIda->szNewDictName, pDimpIda->szString,
                             TRUE, hwnd );

    // handle dictionary index file
    if ( !usRc )
    {
      PSZ pszExt = strrchr( pDimpIda->szNewDictName, DOT );

      //build asi path and copy
      Utlstrccpy( pDimpIda->szDummy,
                  pDimpIda->szNewDictName, DOT );

      if ( (pszExt != NULL) && (strcmp( pszExt, EXT_OF_SHARED_DIC ) == 0) )
      {
        strcat( pDimpIda->szDummy, EXT_OF_SHARED_DICTINDEX );
      }
      else
      {
        strcat( pDimpIda->szDummy, EXT_OF_DICTINDEX );
      } /* endif */

      sprintf( pDimpIda->szString, "%s\\%s%s",
               pDimpIda->szPath, pszToDict,
               (usLocation == LOC_LOCAL ) ? EXT_OF_DICTINDEX :
               EXT_OF_SHARED_DICTINDEX );

      if ( fPropUpdate )
      {
        //update, save and close properties
        strcpy( pDictProp->szIndexPath, pDimpIda->szString );
        SaveProperties( hDicProp, &ErrorInfo);
        CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo );
        hDicProp = NULL;
      } /* endif */

      //copy folder dict index file
      UtlDelete( pDimpIda->szString, 0L, FALSE );
      usRc = UtlSmartMoveHwnd( pDimpIda->szDummy, pDimpIda->szString,
                               TRUE, hwnd );
    } /* endif */

    if ( !usRc )
    {
      if ( fPropUpdate )
      {
        //update dict list so that new dict name appears
        pName = UtlGetFnameFromPath( pDimpIda->szString );
        PROPNAME ( pDimpIda->szString, pName );
        EqfSend2AllHandlers( WM_EQFN_CREATED,
                             MP1FROMSHORT( clsDICTIONARY ),
                             MP2FROMP( pDimpIda->szString ) );
      } /* endif */
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( !fOK )
  {
    pDimpIda->fNoImp = TRUE;         //folder dict copy was not a success
  } /* endif */

  if ( hDicProp ) CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo );

}

static
BOOL FolderImportInit( HWND hwnd, PDIMPIDA pDimpIda )
{
  BOOL      fOK = TRUE;                     // success indicator
  PSZ       pDictName;                      // pointer to dict name
  HPROP     hFolderDictProp;                // handle to properties
  EQFINFO   ErrorInfo;                      // property error code
  USHORT    usRc;                           // return code
  USHORT    usErrDict;                      // return code
  SHORT     sRC;                            // return code

  // prepare parameter for error and warning messages
  pDictName = pDimpIda->szLongName;
  OEMTOANSI( pDictName );

  //check if symbol exists (if dict in use by another process)
  //szFindPath is the property path of the troja asd
  UtlMakeEQFPath( pDimpIda->szSymbol, pDimpIda->chDrive, SYSTEM_PATH,(PSZ) NULP );
  strcat( pDimpIda->szSymbol, BACKSLASH_STR );
  strcat( pDimpIda->szSymbol, pDimpIda->szShortName );

  strcat( pDimpIda->szSymbol, EXT_OF_DICTPROP );
  sRC = QUERYSYMBOL( pDimpIda->szSymbol );

  if ( sRC != -1 )
  {
    UtlErrorHwnd( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pDictName, EQF_ERROR, pDimpIda->HWNDImpCmd );
    fOK = FALSE;
    pDimpIda->fStop = TRUE;
  }
  else
  {
    SETSYMBOL( pDimpIda->szSymbol );
    pDimpIda->fFree = TRUE;
  } /* endif */

  if ( fOK )
  {
    //open properties of import dictionary if not copy and
    //compare maptables; issue warning if data may be lost
    //update property file before trying to open properties
    PROPNAME ( pDimpIda->szString, UtlGetFnameFromPath( pDimpIda->szDictName ) );
    pDimpIda->szString[0] = pDimpIda->szDictName[0];
    UtlAddImportToPath( pDimpIda, pDimpIda->szString );

    //folder asd props
    hFolderDictProp = OpenProperties( pDimpIda->szString, NULL,
                                      PROP_ACCESS_READ, &ErrorInfo);
    if ( !hFolderDictProp )
    {
      UtlErrorHwnd( ERROR_OPENING_PROPS, MB_CANCEL, 1, &pDictName,
                    EQF_ERROR, pDimpIda->HWNDImpCmd );
      fOK = FALSE;
      pDimpIda->fStop = TRUE;  //release allocated memory
    }
    else
    {
      //set sgmlprop ptr to folder import asd props
      pDimpIda->pSgmlProp = (PPROPDICTIONARY)MakePropPtrFromHnd( hFolderDictProp );
      fOK = CompareProfiles( pDimpIda);
      if ( pDimpIda->fStop )
      {
        fOK = FALSE;
        pDimpIda->fNotOk = TRUE;   //no terminating msg
      }
      else
      {
        if ( fOK == FALSE )
        {
          if ( (pDimpIda->HWNDImpCmd == HWND_FUNCIF) || ISBATCHHWND(pDimpIda->HWNDImpCmd) )
          {
            usRc = MBID_YES;
          }
          else
          {
            usRc = UtlError( DIFFERENT_PROFILES, MB_YESNO, 0,
                             NULL, EQF_QUERY );
          } /* endif */
          if ( usRc == MBID_NO )
          {
            pDimpIda->fStop = TRUE;  //release allocated memory
            fOK = FALSE;             //fall out of work loop
            pDimpIda->fNotOk = TRUE;
          }
          else
          {
            fOK = TRUE;
            pDimpIda->fMerge = TRUE;
          } /* endif */
        }
        else
        {
          pDimpIda->fMerge = TRUE;
        } /* endif */

        //fill msg parameter array for QDAM error messages in the
        //folder import section where ther is not remote at the moment
        //dictionary name
        Utlstrccpy( pDimpIda->szPathContent,
                    UtlGetFnameFromPath( pDimpIda->pSgmlProp->szDictPath ), DOT );
        pDimpIda->pszMsgError[0] = pDimpIda->szPathContent;
        //current entry
        pDimpIda->ucTermBuf[0] = EOS;                     //initialize string
        pDimpIda->ucErrorTerm[0] = EOS;                    //initialize string
        pDimpIda->pszMsgError[1] = EMPTY_STRING;
        //dictionary drive
        pDimpIda->szDrive[0] = pDimpIda->pSgmlProp->szDictPath[0];
        pDimpIda->szDrive[1] = EOS;
        pDimpIda->pszMsgError[2] = pDimpIda->szDrive;
        //server name  - empty as folder import can't be remote at the moment
        pDimpIda->pszMsgError[3] = EMPTY_STRING;
        //source language
        strcpy( pDimpIda->szSourceLang, pDimpIda->pSgmlProp->szSourceLang );
        strcpy( pDimpIda->szLongDesc, pDimpIda->pSgmlProp->szLongDesc );
        pDimpIda->pszMsgError[4] = pDimpIda->szSourceLang;

        CloseProperties( hFolderDictProp, PROP_QUIT, &ErrorInfo );
        pDimpIda->pSgmlProp = NULL;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //open the import dictionary
    usRc = AsdBegin( 2, &pDimpIda->hUser );
    if ( usRc != LX_RC_OK_ASD )
    {
      PSZ pDictName =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
      UtlErrorHwnd( usRc, MB_CANCEL, 1, &pDictName, QDAM_ERROR, pDimpIda->HWNDImpCmd );
      fOK = FALSE;
    }
    else
    {
      //open asd dict already in troja - full property path
      PSZ pszDictPath;
      UtlMakeEQFPath( pDimpIda->szString, NULC,
                      PROPERTY_PATH, NULL );
      strcat( pDimpIda->szString, BACKSLASH_STR );
      {
        PSZ pszExt = strrchr( pDimpIda->szDictName, DOT );
        strcat( pDimpIda->szString, pDimpIda->szShortName );
        if ( pszExt ) strcat( pDimpIda->szString, pszExt );
      }

      pszDictPath = pDimpIda->szString;
      usRc = AsdOpen( pDimpIda->hUser,  // in - user handle
                      ASD_LOCKED,       // in - open flags
                      1,                // in - number of dictionaries
                      &pszDictPath,     // in - dictionary name(s)
                      &pDimpIda->hDict1,// out - dictionary handle
                      &usErrDict );     // out - number of dict in error
      if ( usRc != LX_RC_OK_ASD )
      {
        PSZ pDictName =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
        UtlErrorHwnd( usRc, MB_CANCEL, 1, &pDictName, QDAM_ERROR, pDimpIda->HWNDImpCmd );
        fOK = FALSE;
        pDimpIda->fStop = TRUE;
        pDimpIda->fNotOk = TRUE;   //no terminating msg
      }
      else
      {
        //folder import file property name
        PSZ pDictName = pDimpIda->szDictName;
        usRc = AsdOpen( pDimpIda->hUser,  // in - user handle
                        ASD_NOINDEX,      // in - open flags
                        1,                // in - number of dictionaries
                        &pDictName,       // in - dictionary name(s)
                        &pDimpIda->hDict2,// out - dictionary handle
                        &usErrDict );     // out - number of dict in error
        if ( (usRc != LX_RC_OK_ASD) && (usRc != LX_RENUM_RQD_ASD) )
        {
          PSZ pDictName =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
          UtlErrorHwnd( usRc, MB_CANCEL, 1, &pDictName, QDAM_ERROR, pDimpIda->HWNDImpCmd );
          fOK = FALSE;
          pDimpIda->fStop = TRUE;
          pDimpIda->fNotOk = TRUE;   //no terminating msg
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //calculate how many entries in import folder dictionary
  if ( fOK )
  {
    usRc = AsdNumEntries( pDimpIda->hDict2,           //in, dict handle
                          &pDimpIda->ulMaxEntries );  //out, num of words
    if ( usRc != LX_RC_OK_ASD )
    {
      PSZ pDictName =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
      UtlErrorHwnd( usRc, MB_CANCEL, 1, &pDictName, QDAM_ERROR, pDimpIda->HWNDImpCmd );
      fOK = FALSE;
    }
    else
    {
      //initailze entry number counter
      pDimpIda->ulCurrentEntry = 1;

      pDimpIda->DimpTask = EQFIMP_GETWORD;    //initialize next task
      if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
      {
        WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
      } /* endif */
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Remove process window in case of errors                         */
  /*******************************************************************/
  if ( !fOK  && (pDimpIda->HWNDImpCmd != HWND_FUNCIF) )
  {
    EqfRemoveObject( TWBFORCE, hwnd);     //remove std window
  } /* endif */

  return( fOK );
} /* end FolderImportInit */

static
BOOL DictMerge ( PDIMPIDA pDimpIda )
{
  BOOL     fOK = TRUE;       //success indicator
  USHORT   usRc;             //return code
  LONG     lNewSliderPos;    //slider position
  PSZ_W    pucData = NULL;   //pointer to dict string
  HDCB     hDictFound;       //dict in which term was found
  PSZ_W    pTermW;            //pointer to term string
  PSZ      pTerm;

  //allocate memory for dictionary data string
  if ( fOK )
  {
    fOK = UtlAllocHwnd( (PVOID *)&pucData, 0L, (LONG)MAXI_SIZE * sizeof(CHAR_W), ERROR_STORAGE,
                        pDimpIda->HWNDImpCmd );
  } /* endif */

  if ( fOK )
  {
    //retrieve term
    usRc = AsdRetEntryW( pDimpIda->hDict2,      //folder dict handle
                        pDimpIda->hUser,       //control block
                        pDimpIda->ucTermBuf,   //head term
                        &pDimpIda->ulTermNum,  //term number
                        pucData,               //dictionary entry data
                        &pDimpIda->ulDataLen,  //length of dict data retrieved in # of w's
                        &hDictFound );         //which dict entry found in

    if (usRc != LX_RC_OK_ASD)
    {
      if ( (usRc == LX_WRD_NT_FND_ASD ) ||
           (usRc == LX_WRD_EXISTS_ASD ) )
      {
        //query user to skip entry
        if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
        {
          usRc = MBID_YES;
        }
        else
        {
          pTermW = pDimpIda->ucTermBuf;
          usRc = UtlErrorW( ERROR_SEARCH_ENTRY, MB_YESNO, 1,
                            &pTermW,
                            EQF_QUERY, TRUE );
        } /* endif */
        if ( usRc != MBID_YES )
        {
          fOK = FALSE;
          pDimpIda->fStop = TRUE;
        } /* endif */
      }
      else
      {
        pTerm =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
        UtlErrorHwnd( usRc, MB_CANCEL, 1, &pTerm, QDAM_ERROR,
                      pDimpIda->HWNDImpCmd );
        fOK = FALSE;
        pDimpIda->fStop = TRUE;
      } /* endif */
    }
    else
    {
      //merge term
      usRc = AsdMergeEntry( pDimpIda->hUser,        //control block
                            pDimpIda->hDict2,       //folder dict handle
                            pDimpIda->ucTermBuf,    //head term
                            pDimpIda->ulDataLen,    //length of dict data in # of w's
                            pucData,                //dictionary entry data
                            pDimpIda->hDict1,       //troja dict handle
                            &pDimpIda->usFlags );   //merge flag

      if (usRc == LX_MAX_ERR)  //merge dialog cancelled
      {
        fOK = FALSE;
        pDimpIda->fStop = TRUE;
      }
      else if ( usRc == LX_RC_OK_ASD )
      {

        pDimpIda->ulCurrentEntry++; //update entry number counter for slider
        pDimpIda->DimpTask = EQFIMP_GETWORD;
      }
      else
      {
        if ( (usRc == LX_WRD_NT_FND_ASD ) ||
             (usRc == LX_WRD_EXISTS_ASD ) )
        {
          //query user to skip entry

          pTermW = pDimpIda->ucTermBuf;
          usRc = UtlErrorHwndW( ERROR_SEARCH_ENTRY, MB_YESNO, 1, &pTermW,
                               EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
          if ( usRc != MBID_YES )
          {
            fOK = FALSE;
            pDimpIda->fStop = TRUE;
          } /* endif */
        }
        else
        {
          pTerm =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
          UtlErrorHwnd( usRc, MB_CANCEL, 1, &pTerm, QDAM_ERROR,
                        pDimpIda->HWNDImpCmd );
          fOK = FALSE;
          pDimpIda->fStop = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //update process window with entry
    Unicode2ASCIIBuf( pDimpIda->ucTermBuf, pDimpIda->szEntryTxt,
                      (USHORT)(UTF16strlenCHAR(pDimpIda->ucTermBuf)+1),
                      sizeof(pDimpIda->szEntryTxt) - 1, pDimpIda->ulOEMCP );
    pDimpIda->szEntryTxt[sizeof(pDimpIda->szEntryTxt)-1] = EOS;

    //update slider
    lNewSliderPos = pDimpIda->ulCurrentEntry * 100L /
                    pDimpIda->ulMaxEntries;

    if ( pDimpIda->lSliderPos < lNewSliderPos )
    {
      pDimpIda->lSliderPos = lNewSliderPos;
      WinSendMsg( pDimpIda->hwnd, WM_EQF_UPDATESLIDER,
                  MP1FROMSHORT( (SHORT)lNewSliderPos ), NULL );
    } /* endif */
  } /* endif */

  //cleanup
  if ( pucData )
  {
    UtlAlloc( (PVOID *) &pucData, 0L, 0L, NOMSG );
  } /* endif */

  return( fOK );
}

static
VOID FolderImportWork( HWND hwnd, PDIMPIDA pDimpIda )
{
  BOOL    fOK = TRUE;
  USHORT  usRc;
  HDCB    hDictFound;
  BOOL    fTerminate = FALSE;
  PSZ     pszError;

  if ( !pDimpIda->fStop )
  {
    switch ( pDimpIda->DimpTask )
    {
      case EQFIMP_GETWORD:
        usRc = AsdNxtTermW(
                         pDimpIda->hDict2,      //folder dict handle
                         pDimpIda->hUser,       //control block
                         pDimpIda->ucTermBuf,   //head term in w's
                         &pDimpIda->ulTermNum,  //term number
                         &pDimpIda->ulDataLen,  //length of data in # of w's
                         &hDictFound );         //dict in which term was found

        UTF16strcpy( pDimpIda->ucErrorTerm, pDimpIda->ucTermBuf );
        if (usRc == LX_EOF_ASD)
        {
          pDimpIda->DimpTask = EQFIMP_CLOSE;
        }
        else if ( usRc == LX_RC_OK_ASD )
        {
          pDimpIda->DimpTask = EQFIMP_MERGE;
        }
        else
        {
          pszError =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
          UtlErrorHwnd( usRc, MB_CANCEL, 1, &pszError, QDAM_ERROR,
                        pDimpIda->HWNDImpCmd );
          fOK = FALSE;
          pDimpIda->fStop = TRUE;
        } /* endif */
        break;
      case EQFIMP_MERGE:
        fOK = DictMerge( pDimpIda );
        break;
      case EQFIMP_CLOSE:
        fOK = DimpClose( pDimpIda );
        break;
      default:
        break;
    } /* endswitch */

    if ( fOK )
    {
      if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF ) UtlDispatch();

      if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
      {
        if ( !WinIsWindow( NULLHANDLE, hwnd ) )
        {
          fTerminate = TRUE;
          fOK = FALSE;
        }
        else
        {
          if ( pDimpIda->fShutdown )
          {
            fOK = FALSE;
            pDimpIda->fStop = TRUE;
          } /* endif */

          if ( pDimpIda->fKill )
          {
            pDimpIda->usKilledTask =(USHORT) pDimpIda->DimpTask;
            pDimpIda->DimpTask = EQFIMP_CLOSE;
          } /* endif */

          WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
        } /* endif */
      } /* endif */
    }
    else
    {
      // fOK is not set, seems an error occured
      fOK = fOK;           // ... a dummy statement to allow break points
    } /* endif */
  }
  else
  {
    // fStop has been set ....
    pDimpIda->fStop = pDimpIda->fStop;   // .. just a target for break points
  } /* endif */

  //release memory and cleanup
  if ( !fTerminate )
  {
    if ( pDimpIda->fStop && (pDimpIda->HWNDImpCmd != HWND_FUNCIF) )
    {
      EqfRemoveObject( TWBFORCE, hwnd);     //remove std window
    }
  } /* endif */
}

static
VOID UtlAddImportToPath( PDIMPIDA pIda, PSZ pszPath )
{
  PSZ pszImport;                      // ptr to position where IMPORT is inserted

  pszImport = strchr( pszPath, BACKSLASH );     // go to 1st backslash

  if ( pszImport )
  {
    pszImport = strchr( pszImport + 1, BACKSLASH ); // go to 2nd backslash
  } /* endif */

  if ( pszImport )
  {
    memmove( pszImport + pIda->ulImportDirLength, pszImport, strlen( pszImport ) + 1 );
    memcpy( pszImport, pIda->szImportDir, pIda->ulImportDirLength );
  } /* endif */
} /* end of UtlAddImportToPath */


static
VOID UtlRemoveImportFromPath( PDIMPIDA pIda, PSZ pszPath )
{
  PSZ pszImport;                      // ptr to position of IMPORTPATH

  pszImport = strstr( pszPath, pIda->szImportDir );
  if (pszImport )
  {
    memmove( pszImport,
             pszImport + strlen(pIda->szImportDir ),
             strlen(pszImport) - pIda->ulImportDirLength + 1 );
  } /* endif */
} /* endof UtlRemovemportFromPath */


//This function is the main import dictionary loop which calls up all processing
//functions and releases all allocated memory when processing is terminated.
//Called with the ida containing all global structures and variables and the
//handle of the standard window.

VOID DimpWork( HWND hwnd,
               PDIMPIDA pDimpIda )
{
  BOOL        fOK = TRUE;  //progress indicator
  BOOL        fTerminate = FALSE;

  if ( !pDimpIda->fStop )
  {
    switch ( pDimpIda->DimpTask )
    {
      case EQFDIMP_READ:
        fOK = DimpRead( pDimpIda );
        break;
      case EQFDIMP_WRITE:
        fOK = DimpWrite( pDimpIda );
        break;
      case EQFDIMP_PROCESS:
        fOK = DimpProcess( hwnd, pDimpIda );
        break;
      case EQFDIMP_CLOSE:
        fOK = DimpClose( pDimpIda );
        break;
      default:
        break;
    } /* endswitch */

    if ( fOK )
    {
      UtlDispatch();

      if ( !WinIsWindow( NULLHANDLE, hwnd ) )
      {
        fTerminate = TRUE;
        fOK = FALSE;
      }
      else
      {


        if ( pDimpIda->fShutdown )
        {
          fOK = FALSE;
          pDimpIda->fStop = TRUE;
        } /* endif */

        if ( pDimpIda->fKill )
        {
          pDimpIda->usKilledTask = (USHORT) pDimpIda->DimpTask;
          pDimpIda->DimpTask = EQFDIMP_CLOSE;
        } /* endif */

        WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !fTerminate )
  {
    if ( pDimpIda->fStop )
    {
      EqfRemoveObject( TWBFORCE, hwnd);       //remove process window
    } /* endif */
  } /* endif */
} /* endif */

VOID DimpCleanUp( PDIMPIDA pDimpIda )
{
  PSZ        pName;
  USHORT     usRc = LX_RC_OK_ASD;
  PSZ        pszMsgTable[2];
  BOOL       fOK = TRUE;

  //close dictionaries
  if ( pDimpIda->hDict1 )
  {
    usRc = AsdClose(pDimpIda->hUser, pDimpIda->hDict1 );

    if ( (usRc != LX_RC_OK_ASD) && (usRc != LX_RENUM_RQD_ASD) )
    {
      pName =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
      pDimpIda->usDDERc = usRc;
      UtlErrorHwnd( usRc, MB_CANCEL, 1,
                    &pName, QDAM_ERROR, pDimpIda->HWNDImpCmd );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( pDimpIda->hDict2 )
  {
    usRc = AsdClose(pDimpIda->hUser, pDimpIda->hDict2 );

    if ( (usRc != LX_RC_OK_ASD) && (usRc != LX_RENUM_RQD_ASD) )
    {
      pName =  QDAMErrorString( usRc, pDimpIda->pszMsgError );
      pDimpIda->usDDERc = usRc;
      UtlErrorHwnd( usRc, MB_CANCEL, 1,
                    &pName, QDAM_ERROR, pDimpIda->HWNDImpCmd );
      pDimpIda->fNotOk = TRUE;
    } /* endif */
  } /* endif */

  //close Nlp services
  AsdEnd( pDimpIda->hUser );

  //delete asd dict if something went drastically wrong - AsdOpen and BuildAsd
  if ( pDimpIda->fNotOk && !pDimpIda->fMerge && fOK )
  {
    UtlMakeEQFPath( pDimpIda->szString, NULC, PROPERTY_PATH, NULL );
    strcat( pDimpIda->szString, BACKSLASH_STR );
    Utlstrccpy( pDimpIda->szName,
                UtlGetFnameFromPath( pDimpIda->szNewDictName ), DOT );
    strcat( pDimpIda->szString, pDimpIda->szName );
    strcat( pDimpIda->szString, EXT_OF_DICTPROP );

    AsdDelete( pDimpIda->szString );

    pName = UtlGetFnameFromPath( pDimpIda->szNewDictName );
    PROPNAME ( pDimpIda->szString, pName );
    if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
    {
      EqfSend2AllHandlers( WM_EQFN_DELETED,
                           MP1FROMSHORT( clsDICTIONARY ),
                           MP2FROMP(pDimpIda->szString) );
    } /* endif */
  } /* endif */

  if ( pDimpIda->fImpMerge && !pDimpIda->fMerge && fOK )
  {
    if ( !pDimpIda->fNotOk && !pDimpIda->fShutdown )
    {
      pName = UtlGetFnameFromPath( pDimpIda->szNewDictName );

      PROPNAME ( pDimpIda->szString, pName );
      if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
      {
        EqfSend2AllHandlers( WM_EQFN_CREATED,
                             MP1FROMSHORT( clsDICTIONARY ),
                             MP2FROMP(pDimpIda->szString) );
      } /* endif */
    } /* endif */
  } /* endif */

  //if command line interface not issue a terminating msg and if the
  //import was not completed entirely rather issue the error msg
  if ( (pDimpIda->HWNDImpCmd != HWND_FUNCIF) && !ISBATCHHWND(pDimpIda->HWNDImpCmd) )
  {
    if ( !pDimpIda->fImpMerge && !pDimpIda->fNotOk && fOK )
    {
      if ( pDimpIda->fNotCompleted )
      {
        pDimpIda->usDDERc = ERROR_DIMP_HALFCOMPLETE;
        UtlErrorHwnd( pDimpIda->usDDERc, MB_OK, 0,
                      NULL, EQF_INFO, pDimpIda->HWNDImpCmd );
      }
      else
      {
        if ( pDimpIda->hDict1 && pDimpIda->fFree )
        {
          OEMTOANSI(pDimpIda->szLongName);
          pszMsgTable[0] = pDimpIda->szLongName;
          pszMsgTable[1] = ltoa( pDimpIda->ulMaxEntries,
                                 pDimpIda->szDummy, 10 );
          pDimpIda->usDDERc = ERROR_DIMP_COMPLETE;
          UtlErrorHwnd( pDimpIda->usDDERc, MB_OK, 2,
                        pszMsgTable, EQF_INFO, pDimpIda->HWNDImpCmd );
          ANSITOOEM(pDimpIda->szLongName);
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //close input file
  if ( pDimpIda->hFile )
    UtlClose( pDimpIda->hFile, TRUE );

  /*******************************************************************/
  /* Free loaded tag table                                           */
  /*******************************************************************/
  if ( pDimpIda->pTagTable )
  {
    TAFreeTagTable((PLOADEDTABLE) pDimpIda->pTagTable );
  } /* endif */

  //free all allocated memory
  if ( pDimpIda->pInputFileBlock )
    UtlAlloc( (PVOID *) &pDimpIda->pInputFileBlock, 0L, 0L, NOMSG );
  if ( pDimpIda->pTokenBlock )
    UtlAlloc( (PVOID *) &pDimpIda->pTokenBlock, 0L, 0L, NOMSG );
  if ( pDimpIda->pMem )
    UtlAlloc( (PVOID *) &pDimpIda->pMem, 0L, 0L, NOMSG );
  if ( pDimpIda->pSgmlProp )
    UtlAlloc( (PVOID *) &pDimpIda->pSgmlProp, 0L, 0L, NOMSG );
  if ( pDimpIda->pMapEntry != NULL )
    UtlAlloc( (PVOID *) &pDimpIda->pMapEntry, 0L, 0L, NOMSG );
  if ( pDimpIda->pEntry != NULL )
    UtlAlloc( (PVOID *) &pDimpIda->pEntry, 0L, 0L, NOMSG );

  //remove symbol so dict free for further use
  if ( pDimpIda->fFree )
  {
    REMOVESYMBOL( pDimpIda->szSymbol );
  } /* endif */
} /* DimpCleanUp */

//This function does all initialization; open files, allocates required memory
//and initializes vital variables. Returns TRUE if all went well.

static
BOOL DimpInit( HWND hwnd, PDIMPIDA pDimpIda )
{
  BOOL fOK =  TRUE;                       // success indicator
  USHORT      usRc;                       // return code
  PSZ         pError;                     // pointer to error string
  SHORT       sRC;                        // message return code

  //check if symbol exists (if dict in use by another process)
  //only in the case of merge as when new the user may change the name
  //of the dict on the new dict dialog panel
  if ( pDimpIda->fMerge )
  {
    UtlMakeEQFPath( pDimpIda->szSymbol, NULC, SYSTEM_PATH,(PSZ) NULP );
    strcat( pDimpIda->szSymbol, BACKSLASH_STR );
    Utlstrccpy( pDimpIda->szString,
                UtlGetFnameFromPath( pDimpIda->szNewDictName ), DOT );
    strcat( pDimpIda->szSymbol, pDimpIda->szString );
    strcat( pDimpIda->szSymbol, EXT_OF_DICTPROP );
    sRC = QUERYSYMBOL( pDimpIda->szSymbol );
    if ( sRC != -1 )
    {
      pError = pDimpIda->szString;
      pDimpIda->usDDERc = ERROR_DICT_LOCKED;
      UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 1,
                    &pError, EQF_ERROR, pDimpIda->HWNDImpCmd );
      fOK = FALSE;
      pDimpIda->fStop = TRUE;
    }
    else
    {
      SETSYMBOL( pDimpIda->szSymbol );
      pDimpIda->fFree = TRUE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //open input file, i.e. the sgml file
    pDimpIda->hFile = DimpOpenFile( pDimpIda,
                                    pDimpIda->szDictName,
                                    &(pDimpIda->ulSize) );

    fOK = ( pDimpIda->hFile != 0 );   //if 0 then false else true
    pDimpIda->fFirstRead = TRUE;
  } /* endif */

  //allocate space for input file
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pDimpIda->pInputFileBlock),
                    0l, (LONG) MAXI_SIZE * sizeof(CHAR_W), NOMSG );
    pDimpIda->ulUsed = MAXI_SIZE * sizeof(CHAR_W);              // used size of memory
  } /* endif */
  if ( fOK )
  {
      fOK = UtlAlloc( (PVOID *) &(pDimpIda->pConvBuffer),
                      0l, (LONG) MAXI_SIZE, NOMSG );
      pDimpIda->ulUsedInConv = MAXI_SIZE;              // used size of memory
  } /* endif */


  //allocate space for token list
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pDimpIda->pTokenBlock),
                    0l, (LONG)(MAXI_SIZE * 3), NOMSG );
  } /* endif */

  //allocate space for string buffer area
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) (PSZ *) &(pDimpIda->pMem),
                    0l, (LONG) sizeof(MEM), NOMSG );
  } /* endif */

  if ( !fOK && !pDimpIda->fStop )    //only if alloc error
  {
    pDimpIda->usDDERc = ERROR_NOT_ENOUGH_MEMORY;
    UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 0,
                  0, EQF_ERROR, pDimpIda->HWNDImpCmd );
  } /* endif */

  if ( fOK )
  {
    pDimpIda->pMem->ulAvail = MAXI_SIZE;       // available size in # of w's

    //read in tag table list with which to create token list
    usRc = TALoadTagTable( DIMPTAGTABLE, (PLOADEDTABLE *)&pDimpIda->pTagTable,
                           TRUE, FALSE );

    if ( !usRc )
    {
      pDimpIda->Stack.sCurrent = 0;              // init stack
      pDimpIda->Stack.sStack[0] = FIRST_TOKEN;
    }
    else
    {
      fOK = FALSE;                  //tag table could not be loaded
      pDimpIda->usDDERc = ERROR_INTERNAL;
      UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
    } /* endif */
  } /* endif */

  if ( !fOK )
    pDimpIda->fStop = TRUE;

  //begin with top most section in dict, i.e. dict tag
  pDimpIda->sSection = DICT_SECTION;

  //initialize slider position
  pDimpIda->lSliderPos = 1;

  if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
  {
    if ( fOK )
    {
      pDimpIda->DimpTask = EQFDIMP_READ; //initialize next task
      WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
    }
    else
    {
      EqfRemoveObject( TWBFORCE, hwnd);
    } /* endif */
  } /* endif */
  return( fOK );
}

//This function issues messages when the process is cancelled or completed and
//branches back to the main import loop for correct termination. Returns TRUE
//if process is to be terminated.

static
BOOL DimpClose( PDIMPIDA pDimpIda )
{
  USHORT usRc;                  // error return code
  BOOL fOK = TRUE;              // leave work loop
  PSZ     pDictName;            // pointer to dict name string

  if ( pDimpIda->fKill )
  {
    pDimpIda->fKill = FALSE;
    usRc = UtlError ( ERROR_DIMP_CANCEL, MB_YESNO, 0, NULL, EQF_QUERY );
    if ( usRc == MBID_NO )
    {
      pDimpIda->DimpTask = (DIMPTASK)pDimpIda->usKilledTask;   //remeber where left off
    }
    else
    {
      pDimpIda->fStop = TRUE;       //stop processing and release memory
      fOK = FALSE;                  //leave DimpWork loop

      if ( pDimpIda->fImpMerge )
      {
        //set var to be able to notify folder import handler
        //that something went wrong
        pDimpIda->fNoImp = TRUE;

        //set var to leave while loop
        pDimpIda->usDictLeft = 0;
      } /* endif */
    } /* endif */
  }
  else
  {
    if ( !pDimpIda->fNotOk )
    {
      if ( pDimpIda->HWNDImpCmd != HWND_FUNCIF )
      {
        //set slider to 100%
        WinSendMsg( pDimpIda->hwnd, WM_EQF_UPDATESLIDER,
                    MP1FROMSHORT(100), NULL );
      } /* endif */

      if ( !pDimpIda->fImpMerge )
      {
        //count number of entries in dict
        usRc = AsdNumEntries( pDimpIda->hDict1,     //in, dict handle
                              &pDimpIda->ulMaxEntries ); //out, num of words

        Utlstrccpy( pDimpIda->szString,
                    UtlGetFnameFromPath( pDimpIda->szNewDictName ), DOT );

        if ( usRc != LX_RC_OK_ASD )
        {
          //dam error
          pDictName = QDAMErrorString( usRc, pDimpIda->pszMsgError );
          pDimpIda->usDDERc = usRc;
          UtlErrorHwnd( usRc, MB_CANCEL, 1,
                        &pDictName, QDAM_ERROR, pDimpIda->HWNDImpCmd );
        } /* endif */
      }
      else
      {
        //finished folder import/merge so update counters
        pDimpIda->usDictNum++;
        pDimpIda->usDictLeft--;
        pDimpIda->fNextDict = TRUE;
      } /* endif */
      pDimpIda->fStop = TRUE;       //stop processing and release memory
      fOK = FALSE;     //leave DimpWork loop
    } /* endif */
  } /* endif */
  return( fOK );
}

//Reads sgml-based dictionary file and tokenizes file. Returns TRUE if all went
//well.

static
BOOL DimpRead( PDIMPIDA pDimpIda )
{
  ULONG  ulFilled = 0L;        // how much mem filled in pInputFileBlock
  ULONG  ulFreeBuffer = 0;      // how much mem free in pInputFileBlock
  ULONG  ulFreeConvBuffer = 0;    // how much bytes can be filled in ConvBuffer
  USHORT usColPos = 0;        // buffer for tokenizer
  USHORT usRc;                // return code
  BOOL   fOK = TRUE;          // success indicator
  PTOKENENTRY pToken;         // ptr to token table
  PSZ_W  pRest;                // temp pointer to rest of untokenized data...


  //move rest to beginning of memory block
  if ( pDimpIda->pRest != NULL )
  {
    // get # of bytes which are left
    ulFilled  = pDimpIda->ulUsed - ((PBYTE)pDimpIda->pRest - (PBYTE)pDimpIda->pInputFileBlock);
    memmove( pDimpIda->pInputFileBlock, (PBYTE)pDimpIda->pRest, ulFilled );
  } /* endif */

  //determine remaining space in memory block
  ulFreeBuffer = ((MAXI_SIZE * sizeof(CHAR_W)) - ulFilled - 2L);                      /* @KIT1340C */
  // to be sure that data fits into pInputFileBlock after conversion:
  ulFreeConvBuffer = min((ulFreeBuffer / sizeof(CHAR_W)), MAXI_SIZE);

  //set slider position
  usRc = UtlChgFilePtr( pDimpIda->hFile, 0L, 1,
                        &pDimpIda->ulFilePosition, FALSE);

  if ( usRc )
  {
    LONG lNewSliderPos;

    lNewSliderPos = (LONG)(pDimpIda->ulFilePosition * 100 /
                           pDimpIda->ulSize );
    if ( (pDimpIda->HWNDImpCmd != HWND_FUNCIF) &&
         (pDimpIda->lSliderPos < lNewSliderPos) )
    {
      pDimpIda->lSliderPos = lNewSliderPos;
      WinSendMsg( pDimpIda->hwnd, WM_EQF_UPDATESLIDER,
                  MP1FROMSHORT( (SHORT)lNewSliderPos ), NULL );
    } /* endif */
  } /* endif */

  //read in data in size of ulFreeConvBuffer
  if ( !pDimpIda->fAll )
  {
    BOOL fMsgDisplayed = FALSE;

    // the input file is not yet complete continue reading into memory
    usRc = DimpReadtoUnicode(pDimpIda, &ulFreeConvBuffer, &ulFilled,
                              &ulFreeBuffer, &fMsgDisplayed);

    if (pDimpIda->fFirstRead)
    {
		// check for codepage identifier - if another cp must be used, clear buffer
		// and read in everything again
		BOOL fReadAgain = FALSE;
		usRc = CheckForCodePage(pDimpIda, &fReadAgain, &fMsgDisplayed);
		if (!usRc && fReadAgain)
		{ // undo the last character read...
			 ULONG     ulNewPos;                // file pointer position

            UtlChgFilePtr( pDimpIda->hFile, 0L, FILE_BEGIN, &ulNewPos, FALSE );

			// adjust counters
			pDimpIda->ulUsedInConv = 0;
			pDimpIda->ulUsed = 0;
			pDimpIda->fAll = 0;
			ulFilled = 0;
			ulFreeBuffer = ((MAXI_SIZE * sizeof(CHAR_W)) - ulFilled - 2L);
            ulFreeConvBuffer = min((ulFreeBuffer / sizeof(CHAR_W)), MAXI_SIZE);
            fMsgDisplayed = FALSE;
			usRc = DimpReadtoUnicode(pDimpIda, &ulFreeConvBuffer, &ulFilled,
			                         &ulFreeBuffer, &fMsgDisplayed);
	    }
    }

    pDimpIda->fFirstRead = FALSE;

    if ( usRc )      // read failed - display error message
    {
      if (!fMsgDisplayed)
      {
        PSZ pszFileName;

        pDimpIda->usDDERc = usRc;
        pszFileName = pDimpIda->szDictName;
        UtlErrorHwnd( usRc, MB_CANCEL, 1,
                      &pszFileName, DOS_ERROR, pDimpIda->HWNDImpCmd );
      }
      pDimpIda->usDDERc = usRc;
      pDimpIda->fStop = TRUE;  // premature close, release memory
      pDimpIda->fNotOk = TRUE;
      fOK = FALSE;
    }
    else
    {                                            // end of input reached
       PSZ_W  pTemp;
      /*************************************************************/
      /* set end of data indication                                */
      /*************************************************************/
      pTemp = (PSZ_W)(pDimpIda->pInputFileBlock+pDimpIda->ulUsed);
      *pTemp = '\0';

      pRest = pDimpIda->pRest;
      TAFastTokenizeW( (PSZ_W)pDimpIda->pInputFileBlock,       // is CHAR_W
                      (PLOADEDTABLE) pDimpIda->pTagTable,
                      pDimpIda->fAll,
                      &(pDimpIda->pRest),              // is W!
                      &usColPos,
                      (PTOKENENTRY) pDimpIda->pTokenBlock,
                      (MAXI_SIZE * 3) / sizeof(TOKENENTRY) );

      pDimpIda->pToken = (PTOKENENTRY) pDimpIda->pTokenBlock;

      // reset fAll if still rest is available
      if ( pDimpIda->fAll && pDimpIda->pRest && (pDimpIda->pRest != pRest) )
      {
        pDimpIda->fAll = FALSE;
      } /* endif */

      pToken = pDimpIda->pToken;
      //initialize next task
      pDimpIda->DimpTask = EQFDIMP_PROCESS;
    } /* endif */
  }
  else
  {
    pDimpIda->DimpTask = EQFDIMP_CLOSE;
  } /* endif */
  return( fOK );
}

//Writes correctly processed sgml entry to dictionary. Returns TRUE if all went
//well.

static
BOOL DimpWrite( PDIMPIDA pDimpIda )
{
  USHORT     usRc;                 // ASD return code
  ULONG      ulDataLen;            // data length
  USHORT     usStatus;             // status
  USHORT     usResult;             // return code from Error display
  PSZ_W      pErrorW;               // pointer to error
  PSZ        pError;
  PSZ_W      pucRecord;            // pointer to QLDB record
  BOOL       fOK = TRUE;           // success indicator

  //Encode Entry for saving
  usStatus = QLDBTreeToRecord( pDimpIda->phTree, &pucRecord, &ulDataLen );

  if ( usStatus != QLDB_NO_ERROR )
  {
    if ( (pDimpIda->HWNDImpCmd == HWND_FUNCIF) ||
         ISBATCHHWND(pDimpIda->HWNDImpCmd))
    {
      //msg must be suppressed if cmd line import function and
      //operating  must stop
      pDimpIda->usDDERc = usStatus;
      UtlErrorHwnd( usStatus, MB_OKCANCEL, 0, NULL, QLDB_ERROR, pDimpIda->HWNDImpCmd );
      pDimpIda->fStop = TRUE;    //premature close, release memory
      fOK = FALSE;               //does no wish to continue with next entry
    }
    else
    {
      usResult = UtlError ( usStatus, MB_OKCANCEL,
                            0, NULL, QLDB_ERROR );
      if ( usResult == MBID_CANCEL )
      {
        pDimpIda->fStop = TRUE;    //premature close, release memory
        fOK = FALSE;               //does no wish to continue with next entry
      }
    } /* endif */
  }
  else
  {
    //SaveEntry
    if ( pDimpIda->usImpMode == DICT_FORMAT_XML_UTF8 ) 
       ResolveXMLEntities( (PSZ_W)pDimpIda->ucTermBuf ) ;

    usRc = AsdMergeEntry( pDimpIda->hUser,      //control block
                          pDimpIda->hDict1,     //troja dict handle
                          pDimpIda->ucTermBuf,  //head term in w's
                          ulDataLen,     //length of dict data in # of w's
                          pucRecord,            //dict data in w's
                          pDimpIda->hDict1,     //troja dict handle
                          &pDimpIda->usFlags );          //merge flags
    UTF16strcpy( pDimpIda->ucErrorTerm, pDimpIda->ucTermBuf );

    if (usRc == LX_MAX_ERR)  //merge dialog cancelled
    {
      fOK = FALSE;
      pDimpIda->fStop = TRUE;
      pDimpIda->fNotCompleted = TRUE;
    }
    else if ( usRc != LX_RC_OK_ASD )
    {
      if ( (usRc == LX_WRD_NT_FND_ASD ) ||                            //$1222a
           (usRc == LX_WRD_EXISTS_ASD ) )                             //$1222a
      {
        pErrorW = pDimpIda->ucTermBuf;
        if ( (pDimpIda->HWNDImpCmd == HWND_FUNCIF) ||
             ISBATCHHWND(pDimpIda->HWNDImpCmd))
        {
          //msg must be suppressed if cmd line import function and
          //operating  must stop
          pDimpIda->usDDERc = ERROR_SEARCH_ENTRY;
          UtlErrorHwndW( pDimpIda->usDDERc, MB_YESNO,
                        1, &pErrorW, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
          fOK = FALSE;
          pDimpIda->fStop = TRUE;
          pDimpIda->fNotCompleted = TRUE;
        }
        else
        {
          usRc = UtlErrorW( ERROR_SEARCH_ENTRY, MB_YESNO, 1, &pErrorW,  //$1222a
                           EQF_QUERY, TRUE );                                //$1222a
          if ( usRc != MBID_YES )                                     //$1222a
          {
            //$1222a
            fOK = FALSE;                                              //$1222a
            pDimpIda->fStop = TRUE;                                   //$1222a
            pDimpIda->fNotCompleted = TRUE;                           //$1222a
          } /* endif */                                               //$1222a
        } /* endif */
      }
      else
      {
        pError = QDAMErrorString( usRc, pDimpIda->pszMsgError );
        pDimpIda->usDDERc = usRc;
        UtlErrorHwnd( usRc, MB_CANCEL, 1, &pError, QDAM_ERROR,
                      pDimpIda->HWNDImpCmd );
        fOK = FALSE;
        pDimpIda->fStop = TRUE;
        pDimpIda->fNotCompleted = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //continue with the entry section in dict, i.e. entry tags in DimpProcess
    pDimpIda->sSection = ENTRY_SECTION;

    //continue tag and data processing
    pDimpIda->DimpTask = EQFDIMP_PROCESS;  // process next record
  } /* endif */

  //cleanup
  if ( pucRecord )
  {
    UtlAlloc( (PVOID *) &pucRecord, 0L, 0L, NOMSG );
  } /* endif */

  QLDBDestroyTree( &pDimpIda->phTree );

  return( fOK );
} /* end DimpWrite */

//This function processes all the sgml tags (realized as tokens after
//tokenization) and builds the corresponding
//signature record in the dictionary
//and stores entry information in the correct
//format for subsequent addition to the dictionary. Returns TRUE if an entry
//was successfully processed.

static
BOOL DimpProcess ( HWND hwnd, PDIMPIDA pDimpIda )
{
  PTOKENENTRY     pToken;                 // pointer into token table
  BOOL            fOK  = TRUE;            // currently all is okay
  SHORT           sToken;                 // original token
  PSZ             pError;                 // pointer to string


  pToken  = pDimpIda->pToken;

  //as the dict tag table contains both dict and tagtable tags a condition has
  //to be included that checks if the file begins with a dict tag
  //as otherwise the program begins to read the file and as tags overlap
  //confusion breaks lose! Only test this once - hence the boolean flag
  if ( !pDimpIda->fIsSgml )
  {
	if (pToken->sTokenid >=0)
	{
        sToken = pDimpIda->sOrgToken[pToken->sTokenid] ;
    }
    else
    {
		sToken = pToken->sTokenid;
    }
    if ( sToken != DICT_TOKEN )
    {
      pToken->sTokenid = ENDOFLIST;
      pDimpIda->fValid = FALSE;
    }
    else
    {
      pDimpIda->fIsSgml = TRUE;
    } /* endif */
  } /* endif */

  while ( (pToken->sTokenid != ENDOFLIST ) && fOK )
  {
    if (pToken->sTokenid >= 0 )                     // one of our tags
    {
      //map token
      sToken = pDimpIda->sOrgToken[pToken->sTokenid];

      //check if tag valid
      fOK = ValidTag( pDimpIda, &pToken, &pDimpIda->fValid, sToken );

      if ( pDimpIda->fValid )
      {
        switch ( pDimpIda->sSection )
        {
          case DICT_SECTION:
            switch ( sToken )
            {
              case DICT_TOKEN:
                pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
                = sToken;
                pDimpIda->sSection = HEADER_SECTION;
                break;
            } /* endswitch */
            break;
          case  HEADER_SECTION:
            fOK = DimpHeader( pDimpIda, sToken, &pToken );
            break;
          case MAPTABLE_SECTION:
            fOK = DimpMapTable( hwnd, pDimpIda, sToken, &pToken );
            break;
          case ENTRY_SECTION:
            fOK = DimpEntry( pDimpIda, sToken, &pToken );
            break;
        } /* endswitch */
        pToken++;
      } /* endif */
    }
    else
    {
      pToken++;
    } /* endif */
  } /* endwhile */

  if ( (pToken->sTokenid == ENDOFLIST) && fOK )  // end of token list
  {
    if ( pDimpIda->fValid )
    {
      pDimpIda->DimpTask = EQFDIMP_READ;       // set default next task
    }
    else
    {
      // error message that invalid file for import, not sgml
      pError =  UtlGetFnameFromPath( pDimpIda->szDictName);
      pDimpIda->usDDERc = INVALID_FILE_FOR_IMPORT;
      UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
                    1, &pError, EQF_ERROR, pDimpIda->HWNDImpCmd );
      pDimpIda->fStop = TRUE;      //release allocated memory
      pDimpIda->fNotOk = TRUE;     //import cannot proceed
      fOK = FALSE;                 //fall out of while loop
    } /* endif */
  } /* endif */

  pDimpIda->pToken = pToken;                    // save token position

  //continue in dimpwork loop
  if ( !pDimpIda->fStop )
  {
    fOK = TRUE;
  } /* endif */

  return( fOK );
} /* end DimpProcess */

static
BOOL DimpEntry ( PDIMPIDA pDimpIda, SHORT sToken, PTOKENENTRY *ppToken )
{
  BOOL         fOK = TRUE;            //success indicator
  PSZ_W        pDummy;                //pointer to string
  USHORT       usI;                   //user defined field number
  PMEM         pMem;                  //pointer to mem struct
  LONG         lNewSliderPos;         //slider postion
  PTOKENENTRY  pToken;                //pointer to token
  PTOKENENTRY  pNextToken;            //pointer to token

  pToken = ( *ppToken );
  switch ( sToken )
  {
    case TARGET_TOKEN:
      pDimpIda->usActiveLevel = TARGET_LEVEL;
      pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
      = sToken;
      break;
    case HOM_TOKEN:
      pDimpIda->usActiveLevel = POS_LEVEL;
      pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
      = sToken;
      break;
    case SENSE_TOKEN:
      pDimpIda->usActiveLevel = SENSE_LEVEL;
      pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
      = sToken;
      break;
    case ETARGET_TOKEN:
      if (pDimpIda->usActiveLevel == TARGET_LEVEL)
      {
        WriteSubTree(pDimpIda);
      } /* endif */
      pDimpIda->usSubTree = TARGET_LEVEL;
      pDimpIda->Stack.sCurrent--;   // enable old settings
      break;
    case ESENSE_TOKEN:
      if (pDimpIda->usActiveLevel == SENSE_LEVEL)
      {
        WriteSubTree(pDimpIda);
      } /* endif */
      pDimpIda->usSubTree = SENSE_LEVEL;
      pDimpIda->Stack.sCurrent--;   // enable old settings
      break;
    case EHOM_TOKEN:
      if (pDimpIda->usActiveLevel == POS_LEVEL)
      {
        WriteSubTree(pDimpIda);
      } /* endif */
      pDimpIda->usSubTree = POS_LEVEL;
      pDimpIda->Stack.sCurrent--;   // enable old settings
      break;
    case ENTRY_TOKEN:
      // check if entry is complete
      fOK = CheckForEnd( pDimpIda, pToken, EENTRY_TOKEN );
      if ( fOK )
      {
        pDimpIda->usActiveLevel = HW_LEVEL;
        pDimpIda->usSubTree = HW_LEVEL;
        pMem = pDimpIda->pMem;   // clear last entry data
        // init ulUsed many char_w's
        UTF16memsetL( pMem->chBuffer, NULC, pMem->ulUsed );
        pMem->ulUsed = 0;
        pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
        = sToken;
        memset( pDimpIda->pEntry,
                NULC,
                pDimpIda->usMapNext * sizeof(PSZ));
        pDimpIda->usAddNodesOffset = 0;

        //add dates if date fields exist in maptable
        AddDate( pDimpIda );
      } /* endif */
      break;
    case EENTRY_TOKEN:
      if ( pDimpIda->ucTermBuf[0] != NULC ) //headword not empty
      {
        if (pDimpIda->usActiveLevel == HW_LEVEL)
        {
          WriteSubTree(pDimpIda);
        } /* endif */
        pDimpIda->Stack.sCurrent --; // get rid of uppermost elem.
        fOK = FALSE;              // leave while loop and write entry

        //correct slider position
        lNewSliderPos = pDimpIda->ulFilePosition +
                        (LONG)((PSZ)pDimpIda->pToken - (PSZ)pDimpIda->pTokenBlock) ;
        lNewSliderPos = lNewSliderPos * 100 / pDimpIda->ulSize;

        if ( (pDimpIda->HWNDImpCmd != HWND_FUNCIF) &&
             (pDimpIda->lSliderPos < lNewSliderPos) )
        {
          pDimpIda->lSliderPos = lNewSliderPos;
          WinSendMsg ( pDimpIda->hwnd, WM_EQF_UPDATESLIDER,
                       MP1FROMSHORT((SHORT)lNewSliderPos), NULL );
        } /* endif */

        //initialize next task
        pDimpIda->DimpTask = EQFDIMP_WRITE;
      }
      else
      {
        pDimpIda->Stack.sCurrent --; // get rid of uppermost elem.

        //continue with the entry section in dict, i.e. entry tags in DimpProcess
        pDimpIda->sSection = ENTRY_SECTION;

        //continue tag and data processing
        pDimpIda->DimpTask = EQFDIMP_PROCESS;  // process next record
      } /* endif */
      break;
    case EDICT_TOKEN:
      pDimpIda->Stack.sCurrent --;// get rid of uppermost elem.
      fOK = FALSE;                // leave while loop
      pDimpIda->DimpTask = EQFDIMP_CLOSE;
      break;
    case TUSER_TOKEN:                       // user tags
    case EUSER_TOKEN:
      if ( ((pToken+2)->sTokenid >= 0) &&
           ((pDimpIda->sOrgToken[(pToken+2)->sTokenid] == EEUSER_TOKEN) ||
           (pDimpIda->sOrgToken[(pToken+2)->sTokenid] == ETUSER_TOKEN) ))
      {
        //get token id and add to token id stack need to check validity
        //of tags
        sToken   = pDimpIda->sOrgToken[(pToken)->sTokenid];
        pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent] = sToken;
        pToken++;      //position at end tag - skip id=xx
      }
      else
      {
        fOK = IDMatch( pDimpIda, &pToken, &pDummy, &usI );

        if ( fOK && pDummy )
        {
          fOK = FillEntry( pDimpIda, pDummy, sToken, (SHORT) usI, *ppToken );
        } /* endif */
        pToken--;                    //token pos is updated at ent of loop so back one here
      } /* endif */
      break;
    case EEUSER_TOKEN:
    case ETUSER_TOKEN:
    case EHDTERM_TOKEN:               // make new settings
      pDimpIda->Stack.sCurrent--;   // enable old settings
      break;
    case HDTERM_TOKEN:               // make new settings
      pNextToken = (pToken+1);
      if ( ((pNextToken)->sTokenid >= 0) &&
            (pDimpIda->sOrgToken[pNextToken->sTokenid] == EHDTERM_TOKEN ))
      {
        //skip entry as headword empty
        //position on the end entry token
        while ( (pNextToken->sTokenid < 0 ) ||
                (pDimpIda->sOrgToken[pNextToken->sTokenid] != EENTRY_TOKEN  ))
        {
          pToken++;
          pNextToken = (pToken+1);
        } /* endwhile */
      }
      else
      {
        fOK = TextMatch( pDimpIda, &pToken, &pDummy );

        if ( fOK && pDummy )
        {
          fOK = FillEntry( pDimpIda, pDummy, sToken, SYSFIELD, *ppToken );
          UTF16strncpy ( pDimpIda->ucTermBuf, pDummy, MAX_TERM_LEN - 1);
          UTF16strcpy( pDimpIda->ucErrorTerm, pDimpIda->ucTermBuf );
        }
      } /* endif */
      break;
    default:
      if ( ((sToken >= COMMENT_TOKEN) && (sToken <= TUSAGE_TOKEN)) ||
           (sToken == ESTYLE_TOKEN) || (sToken == TSTYLE_TOKEN)  )
        //starting tags
      {
        //check if start tag is immediately followed by its end tag
        //with no text in between and if so then don't do anything
        //other than move on to end tag
        pNextToken = (pToken+1);
        if ((pNextToken->sTokenid >=0) &&
            ((pDimpIda->sOrgToken[pNextToken->sTokenid] >= ECOMMENT_TOKEN) &&
            (pDimpIda->sOrgToken[pNextToken->sTokenid] <= ETUSAGE_TOKEN)))
        {
          //get token id and add to token id stack need to check validity
          //of tags
          sToken   = pDimpIda->sOrgToken[(pToken)->sTokenid];
          pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent] = sToken;
        }
        else
        {
          fOK = TextMatch( pDimpIda, &pToken, &pDummy );

          if ( fOK && pDummy )
            fOK = FillEntry( pDimpIda, pDummy, sToken, SYSFIELD, *ppToken );
        } /* endif */
      }
      else
      {
        if (  ((sToken >= ECOMMENT_TOKEN) && (sToken <= ETUSAGE_TOKEN)) ||
              (sToken == EESTYLE_TOKEN) || (sToken == ETSTYLE_TOKEN)  )
          // end tags
          pDimpIda->Stack.sCurrent--;   // take from Stack
      } /* endif */
      break;
  } /* endswitch */
  *ppToken = pToken;
  return( fOK );
} /* end DimpEntry */

static
BOOL DimpHeader ( PDIMPIDA pDimpIda, SHORT sToken, PTOKENENTRY *ppToken )
{
  PSZ      pError;                    //pointer to error string
  PSZ_W    pDummyW;                    //pointer to string
  BOOL     fOK=TRUE;                  //success indiactor
  PTOKENENTRY  pToken;                //pointer to token
  PSZ          pDummy;

  pToken = ( *ppToken );

  switch (  sToken )
  {
    case HEADER_TOKEN:
      fOK = CheckForEnd( pDimpIda, pToken, EHEADER_TOKEN );
      if ( fOK )
      {
        pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
        = sToken;
      }
      else
      {
        pDimpIda->usDDERc = ERROR_DIMP_INVHEADER;
        UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
                      0, NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
        pDimpIda->fStop = TRUE;     //premature close, release memory
        pDimpIda->fNotOk = TRUE;    //don't issue complete msg
      } /* endif */
      break;
    case EHEADER_TOKEN:
      //check if source lang has been added as this field is
      //obligatory
      if ( pDimpIda->szSourceLang[0] != NULC )
      {
        pDimpIda->Stack.sStack[--pDimpIda->Stack.sCurrent] = sToken;
        pDimpIda->sSection = MAPTABLE_SECTION;
      }
      else
      {
        Utlstrccpy( pDimpIda->szName,
                    UtlGetFnameFromPath( pDimpIda->szDictName ),
                    DOT );
        pError = pDimpIda->szName;
        pDimpIda->usDDERc = ERROR_SOURCE_LANG_MISSING;
        UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 1,
                      &pError, EQF_ERROR, pDimpIda->HWNDImpCmd );
        pDimpIda->fStop = TRUE; //release alloc mem
        fOK = FALSE;      //fall out of work loop
      } /* endif */
      break;
    case TYPE_TOKEN:
    case LTARGET_TOKEN:
    case READONLY_TOKEN:
    case CREATEDATE_TOKEN:
      pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
      = sToken;
      break;
    case CODEPAGE_TOKEN:
      fOK = CheckForEnd( pDimpIda, pToken, ECODEPAGE_TOKEN );
	  if ( fOK )
      {
		fOK = TextMatch( pDimpIda, &pToken, &pDummyW );
      }
      break;
    case ETYPE_TOKEN:
    case ECREATEDATE_TOKEN:
    case ECOPYRIGHT_TOKEN:
    case EREADONLY_TOKEN:
    case ESOURCE_TOKEN:
    case ELTARGET_TOKEN:
    case EDESCRIPTION_TOKEN:
    case ECODEPAGE_TOKEN:
      pDimpIda->Stack.sCurrent--;   // enable old settings
      break;
    case SOURCE_TOKEN:
      fOK = TextMatch( pDimpIda, &pToken, &pDummyW );

      if ( fOK && pDummyW )
      {
        while ( *pDummyW == ' ' )
          *pDummyW++;
        Unicode2ASCIIBuf( pDummyW,pDimpIda->szDummy,
                          (USHORT)(UTF16strlenCHAR(pDummyW)+1),
                          MAX_LONGPATH, pDimpIda->ulOEMCP );

        if ( pDimpIda->szDummy[0] == EOS )
        {
          Utlstrccpy( pDimpIda->szName,
                      UtlGetFnameFromPath( pDimpIda->szDictName ),
                      DOT );
          pError = pDimpIda->szName;
          pDimpIda->usDDERc = ERROR_SOURCE_LANG_MISSING;
          UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
                        1, &pError, EQF_ERROR, pDimpIda->HWNDImpCmd );
          pDimpIda->fStop = TRUE; //release alloc mem
          fOK = FALSE;      //fall out of work loop
        }
        else
        {
          //see if source lang is available
          fOK = isValidLanguage( pDimpIda->szDummy, TRUE );
          if ( fOK )
          {
            strcpy( pDimpIda->szSourceLang, pDimpIda->szDummy );
          }
          else
          {
            PSZ pszParm = pDimpIda->szDummy;
            pDimpIda->usDDERc = ERROR_BAD_LANG_NAME;
            UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
                          1, &pszParm, EQF_ERROR, pDimpIda->HWNDImpCmd );
            pDimpIda->fStop = TRUE;  //rel alloc mem
            fOK = FALSE;      //fall out of work loop
            pDimpIda->fNotOk = TRUE;
          } /* endif */
        } /* endif */
      } /* endif */
      break;

    case DESCRIPTION_TOKEN:
      fOK = CheckForEnd( pDimpIda, pToken, EDESCRIPTION_TOKEN );

      fOK = TextMatch( pDimpIda, &pToken, &pDummyW );

      if ( fOK && pDummyW )
      {
        while ( *pDummyW == ' ' )
          *pDummyW++;
        Unicode2ASCIIBuf( pDummyW,pDimpIda->szDummy, (USHORT)(UTF16strlenCHAR(pDummyW)+1),
                          MAX_LONGPATH, pDimpIda->ulOEMCP );
        {
          strcpy( pDimpIda->szLongDesc, pDimpIda->szDummy );
        }
      } /* endif */

      break;

    case COPYRIGHT_TOKEN:
      fOK = TextMatch( pDimpIda, &pToken, &pDummyW );

      if ( fOK && pDummyW )
      {
        while ( *pDummyW == ' ' )
          *pDummyW++;

        pDimpIda->fCopyRight = (UTF16stricmp( pDummyW, YES_STRW ) == 0);
      } /* endif */
      break;
  } /* endswitch */
  return( fOK );
} /* end DimpHeader */

static
BOOL DimpMapTable ( HWND hwnd, PDIMPIDA pDimpIda,
                    SHORT sToken, PTOKENENTRY *ppToken )
{
  BOOL         fOK = TRUE;            //success indiactor
  PSZ_W        pDummyW;                //pointer to string
  USHORT       usI;                   //user defined field number
  USHORT       usDisplay;             //display level
  USHORT       usStatus;              //ro/rw
  USHORT       usEntryType;           //large or small entry field
  BOOL         fVital;                //entry field vital?
  BOOL         fAutLookup;            //use field in automatic lookup
  PTOKENENTRY  pToken;                //pointer to token

  pToken = ( *ppToken );

  switch (  sToken )
  {
    case TARGET_TOKEN:
      pDimpIda->usActiveLevel = TARGET_LEVEL;
      pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
      = sToken;
      break;
    case HOM_TOKEN:
      pDimpIda->usActiveLevel = POS_LEVEL;
      pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
      = sToken;
      break;
    case SENSE_TOKEN:
      pDimpIda->usActiveLevel = SENSE_LEVEL;
      pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent]
      = sToken;
      break;
    case MAPTABLE_TOKEN :
      fOK = CheckForEnd( pDimpIda, pToken, EMAPTABLE_TOKEN );
      if ( fOK )
      {
        pDimpIda->usActiveLevel = HW_LEVEL;  // init to headword level
        pDimpIda->usLevel = 1;               // initialize level

        //allocate space for maptable entries
        fOK = UtlAlloc ((PVOID *)&pDimpIda->pMapEntry, 0L,
                        (LONG)sizeof(MAPENTRY) * (LAST_TAG + MAX_USER), NOMSG );
      } /* endif */
      if ( fOK )
      {
        //put on stack and skip token
        pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent] =
        sToken;
        pDimpIda->usMapAvail = LAST_TAG + MAX_USER;
        pDimpIda->usMapNext = 0;
      }
      else
      {
        pDimpIda->usDDERc = ERROR_DIMP_INVMAP;
        UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
                      0, NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
        pDimpIda->fStop = TRUE;   //premature close, release memory //$1131a
        fOK = FALSE;              //leave dimpwork loop      //$1131a
        pDimpIda->fNotOk = TRUE;  //don't issue complete msg //$1131a
      } /* endif */
      break;
    case EMAPTABLE_TOKEN:
      pDimpIda->Stack.sStack[--pDimpIda->Stack.sCurrent] = sToken;
      pDimpIda->sSection = ENTRY_SECTION;
      fOK = EndMapTableProcess( hwnd, pDimpIda );
      break;
    case TUSER_TOKEN:                       // user tags
    case EUSER_TOKEN:
      fOK = MapIdMatch( pDimpIda, &pToken, &pDummyW, &usI,    // param 3 = w
                        &usDisplay, &usStatus, &fVital, &fAutLookup,
                        &usEntryType );

      if ( fOK && pDummyW )
      {
        // pDummy points to chUserName of field which is stored in ASCII!
        fOK = FillMapTable( pDimpIda, pDummyW,                   // param 2 = w
                            sToken, (SHORT) usI, usDisplay,
                            usStatus, fVital, fAutLookup, usEntryType );
      } /* endif */
      break;
    case EEUSER_TOKEN:
    case ETUSER_TOKEN:
      pDimpIda->Stack.sCurrent--;   // enable old settings
      break;
    default:
      if ( ((sToken >= COMMENT_TOKEN) && (sToken <= TUSAGE_TOKEN)) ||
            (sToken == TSTYLE_TOKEN) ||
            (sToken == ESTYLE_TOKEN) )
      {
        //system tags

        fOK = MapIdMatch( pDimpIda, &pToken, &pDummyW, &usI,   //param 3 =  w
                          &usDisplay, &usStatus, &fVital, &fAutLookup,
                          &usEntryType );

        if ( fOK && pDummyW )
        {
          fOK = FillMapTable( pDimpIda, pDummyW, sToken,         //param 2 = w
                              SYSFIELD, usDisplay, usStatus, fVital, fAutLookup,
                              usEntryType );
        }
        pToken--;
      }
      else
      {
        if (  ((sToken >= ECOMMENT_TOKEN) && (sToken <= ETUSAGE_TOKEN)) ||
              (sToken == ETSTYLE_TOKEN) ||
              (sToken == EESTYLE_TOKEN) )
        {
          pDimpIda->Stack.sCurrent--;   // take from Stack
        } /* endif */
         
      } /* endif */
      break;
  } /* endswitch */
  *ppToken = pToken;
  return( fOK );
} /* end DimpMapTable */

static
BOOL EndMapTableProcess ( HWND hwnd, PDIMPIDA pDimpIda )
{
  BOOL     fOK = TRUE;                // success indicator
  EQFINFO  ErrorInfo;                 // property error code
  HPROP    hSgmlProp = NULL;          // handle to sgml properties
  USHORT   usRc;                      // return code
  PSZ      pName;                     // pointer to string
  PSZ      pszDict;                   // pointer to string
  USHORT   usErrDict;                 // dict in error
  USHORT   usStatus;                  // return code for QLDB funct
  PDICTPARMS pDictParms;              // dialog parms
  PPROPDICTIONARY pSgmlProp;          // ptr to dict properties
  SHORT    sRC;                       // msg return code
  PSZ      pszMsgTable[2];            // error msg parameter list
  SHORT    sItem;                     // list box item

  //fill property  structure with new sgml file data
  pDimpIda->pSgmlProp = FillProfile( pDimpIda );

  if ( pDimpIda->pSgmlProp != NULL )
  {
    if ( pDimpIda->fMerge )
    {
      fOK = CompareProfiles( pDimpIda);
      if ( pDimpIda->fStop )
      {
        fOK = FALSE;
        pDimpIda->fNotOk = TRUE;   //no terminating msg
      }
      else
      {
        if ( fOK == FALSE )
        {
          if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
          {
            fOK = TRUE;   //continue regardless of the fact that the
                          //entry structures differ
          }
          else
          {
            usRc = UtlError( DIFFERENT_PROFILES, MB_YESNO, 0, NULL,
                             EQF_QUERY );
            if ( usRc == MBID_NO )
            {
              fOK = FALSE;               //fall out of work loop
            }
            else
            {
              fOK = TRUE;
            }
          } /* endif */
        } /* endif */
      } /* endif */
    }
    else
    {
      //fill other dict properties
      pSgmlProp = pDimpIda->pSgmlProp;
      pSgmlProp->PropHead.usClass = PROP_CLASS_DICTIONARY;
      pSgmlProp->PropHead.chType = PROP_TYPE_INSTANCE;
      pName =  UtlGetFnameFromPath( pDimpIda->szPropPath);
      strcpy( pSgmlProp->PropHead.szName, pName );
      UtlMakeEQFPath(pSgmlProp->PropHead.szPath,
                     NULC, SYSTEM_PATH, NULL );

      // build base part of dictionary name (w/o extension)
      UtlMakeEQFPath( pSgmlProp->szDictPath,
                      NULC,    //system drive - later selected drive
                      DIC_PATH, NULL );
      strcat( pSgmlProp->szDictPath, BACKSLASH_STR );
      Utlstrccpy( pSgmlProp->szDictPath +
                  strlen( pSgmlProp->szDictPath ),
                  pSgmlProp->PropHead.szName, DOT );
      strcpy( pSgmlProp->szIndexPath,
              pSgmlProp->szDictPath );

      // add dictionary extension to dictionary path
      strcat( pSgmlProp->szDictPath, EXT_OF_DIC );

      // add dictioanry extension to dictionary index path
      strcat( pSgmlProp->szIndexPath, EXT_OF_DICTINDEX );

      //call up new dialog with entry structure
      fOK = UtlAlloc( (PVOID *) &pDictParms, 0L,
                      max( (LONG)sizeof(DICTPARMS), (LONG)MIN_ALLOC), ERROR_STORAGE );
      if ( fOK )
      {
        pDictParms->usType = NEW;
        pDictParms->pszData = (PSZ)pDimpIda->pSgmlProp;
        if ( strcmp( pDimpIda->szLongName, pDimpIda->szShortName) != 0 )
        {
          strcpy( pDimpIda->pSgmlProp->szLongName, pDimpIda->szLongName );
        } /* endif */

        if ( hwnd == HWND_FUNCIF )
        {
          // create dictionary without user interaction

          CHAR szObjName[MAX_EQF_PATH];
          PPROPDICTIONARY pProp = NULL;
          HPROP hProp = 0;
          EQFINFO ErrorInfo = 0;

          pProp = (PPROPDICTIONARY)pDictParms->pszData;
          pProp->usVersion = BTREE_VERSION3;
          pProp->usLocation = LOC_LOCAL;
          UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
          strcat( szObjName, BACKSLASH_STR );
          strupr( pProp->PropHead.szName );
          strcat( szObjName, pProp->PropHead.szName );

          //create dictionary properties
          hProp = CreateProperties( szObjName, NULL, PROP_CLASS_DICTIONARY, &ErrorInfo);
          if ( !hProp ) fOK = FALSE;
          if ( fOK ) fOK = (PutAllProperties( hProp, pProp, &ErrorInfo ) == 0);
          if ( fOK ) fOK = ( SaveProperties( hProp, &ErrorInfo ) == 0);
          if ( hProp ) CloseProperties( hProp, PROP_QUIT, &ErrorInfo );

          // create dictionary
          if ( fOK )
          {
            HUCB hUser = 0;

            USHORT usRc = AsdBegin( 2, &hUser );

            if ( usRc != LX_RC_OK_ASD )
            {
              CHAR szError[256];
              PSZ pError =  szError;

              pError =  QDAMErrorString( usRc, &pError );
              UtlErrorHwnd( usRc, MB_CANCEL, 1, &pError, QDAM_ERROR, hwnd );
              fOK = FALSE;
            }
            else
            {
              HDCB hDict = 0; 

              UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
              strcat( szObjName, BACKSLASH_STR );
              strupr( pProp->PropHead.szName );
              strcat( szObjName, pProp->PropHead.szName );

              usRc = AsdBuild( hUser, FALSE, &hDict, szObjName );

              if ( usRc != LX_RC_OK_ASD )
              {
                CHAR szError[256];
                PSZ pError =  szError;

                pError =  QDAMErrorString( usRc, &pError );
                UtlErrorHwnd( usRc, MB_CANCEL, 1, &pError, QDAM_ERROR, hwnd );
                fOK = FALSE;
              }
              else
              {
                AsdClose( hUser, hDict );
              } /* endif */
              AsdEnd( hUser );
            } /* endif */
          } /* endif */
        }
        else
        {
          fOK = DictionaryNew( hwnd, pDictParms );

          if ( fOK )
          {
            /********************************************************/
            /* Fix by XQG:                                          */
            /* Save dictionary name BEFORE free of memory area      */
            /* containing the name!                                 */
            /********************************************************/
            pName = pDictParms->pszData;
            PROPNAME( pDimpIda->szString, pName );
          } /* endif */
        } /* endif */


        UtlAlloc( (PVOID *) &pDimpIda->pSgmlProp, 0L, 0L, NOMSG ); // not used anymore

        if ( fOK )
        {
          //open dictionary properties - they may have changed on the
          //new dict dialog
          hSgmlProp = OpenProperties( pDimpIda->szString, NULL,
                                      PROP_ACCESS_READ, &ErrorInfo);
          if ( !hSgmlProp )
          {
            UtlError( ERROR_OPENING_PROPS, MB_CANCEL, 1,
                      &pName, EQF_ERROR );
            fOK = FALSE;
          }
          else
          {
            pDimpIda->pSgmlProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hSgmlProp );

            //register obj - user may have changed name
            strcpy( pDimpIda->IdaHead.szObjName,
                    pDimpIda->pSgmlProp->szDictPath );
            pDimpIda->IdaHead.pszObjName =
            pDimpIda->IdaHead.szObjName;
            usRc = (USHORT)EqfChangeObjectName( hwnd, pDimpIda->IdaHead.szObjName );
            if ( usRc == ErrObjM_AlreadyReg )
            {
              Utlstrccpy( pDimpIda->szString,
                          UtlGetFnameFromPath( pDimpIda->szNewDictName ), DOT );
              pName = pDimpIda->szString;
              UtlError( ERROR_DICT_LOCKED, MB_CANCEL,
                        1, &pName, EQF_ERROR );
              fOK = FALSE;
            }  /* endif */

            //lock dict
            if ( fOK )
            {
              UtlMakeEQFPath( pDimpIda->szSymbol, NULC, SYSTEM_PATH,(PSZ) NULP );
              strcat( pDimpIda->szSymbol, BACKSLASH_STR );
              Utlstrccpy( pDimpIda->szString,
                          UtlGetFnameFromPath( pDimpIda->pSgmlProp->szDictPath ),
                          DOT );
              strcat( pDimpIda->szSymbol, pDimpIda->szString );
              strcat( pDimpIda->szSymbol, EXT_OF_DICTPROP );
              sRC = QUERYSYMBOL( pDimpIda->szSymbol );
              if ( sRC != -1 )
              {
                pName = pDimpIda->szString;
                UtlError( ERROR_DICT_LOCKED, MB_CANCEL,
                          1, &pName, EQF_ERROR );
                fOK = FALSE;
              }
              else
              {
                SETSYMBOL( pDimpIda->szSymbol );
                pDimpIda->fFree = TRUE;
              } /* endif */
            } /* endif */

            //build sznewdictname
            if ( fOK )
            {
              strcpy( pDimpIda->szNewDictName,
                      pDimpIda->pSgmlProp->szDictPath );

              //update proppath
              UtlMakeEQFPath( pDimpIda->szString, NULC,
                              PROPERTY_PATH, NULL );
              Utlstrccpy( pDimpIda->szName,
                          UtlGetFnameFromPath( pDimpIda->szNewDictName ), DOT );
              sprintf( pDimpIda->szPropPath, "%s\\%s%s",
                       pDimpIda->szString, pDimpIda->szName,
                       EXT_OF_DICTPROP );

              //remember if remote or local
              if ( pDimpIda->pSgmlProp->szServer[0] != NULC )
                pDimpIda->fRemote = TRUE;

              //build new mapentry with AsdDictProp info
              pDimpIda->usMapNext = pDimpIda->pSgmlProp->usLength;
              pDimpIda->fStop = NewMapEntry( pDimpIda );
            } /* endif */
          } /* endif */

          //if name different change title bar
          if ( fOK )
          {
            ULONG MsgLen;
			HMODULE hResMod;

			hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

            pszMsgTable[0] =  pDimpIda->szName;
            LOADSTRING( NULLHANDLE, hResMod, IDS_DIMP_TITLEBAR, pDimpIda->szString );
            DosInsMessage( pszMsgTable, 1, pDimpIda->szString,
                           strlen ( pDimpIda->szString ),
                           pDimpIda->szTitle,
                           (sizeof ( pDimpIda->szTitle ) - 1),
                           &MsgLen );
            pDimpIda->szTitle[MsgLen] = EOS;

            //update title bar text
            SETTEXTHWND( pDimpIda->hwnd, pDimpIda->szTitle );
            //release memory
          } /* endif */
        } /* endif */
        UtlAlloc( (PVOID *) &(pDictParms), 0L, 0L, NOMSG);
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      //fill msg parameter array for QDAM error messages in the
      //folder import section where ther is not remote at the moment
      //dictionary name
      pDimpIda->pszMsgError[0] = Utlstrccpy( pDimpIda->szPathContent,
                                             UtlGetFnameFromPath( pDimpIda->pSgmlProp->szDictPath ), DOT );
      pDimpIda->pszMsgError[1] = EMPTY_STRING;
      //dictionary drive
      pDimpIda->szDrive[0] = pDimpIda->pSgmlProp->szDictPath[0];
      pDimpIda->szDrive[1] = EOS;
      pDimpIda->pszMsgError[2] = pDimpIda->szDrive;
      //server name  - empty as folder import can't be remote at the moment
      strcpy( pDimpIda->szFindPath, pDimpIda->pSgmlProp->szServer );
      pDimpIda->pszMsgError[3] = pDimpIda->szFindPath;
      //dictionary source lang                                      //srcla
      pDimpIda->pszMsgError[4] = pDimpIda->szSourceLang;            //srcla
    } /* endif */
  }
  else
  {
    fOK = FALSE;             //fall out of work loop
  } /* endif */

  if ( fOK )  //merge and new
  {
    //init DAM/TOLSTOY
    usRc = AsdBegin( 2, &pDimpIda->hUser );

    if ( usRc != LX_RC_OK_ASD )
    {
      pName = QDAMErrorString( usRc, pDimpIda->pszMsgError );
      pDimpIda->usDDERc = usRc;
      UtlErrorHwnd( usRc, MB_CANCEL, 1, &pName, QDAM_ERROR,
                    pDimpIda->HWNDImpCmd );
      fOK = FALSE;
    } /* endif */

    fOK = UtlAlloc( (PVOID *) &pDimpIda->pEntry, 0L,
                    max( (LONG)(pDimpIda->usMapNext * sizeof(PSZ)), MIN_ALLOC ), NOMSG);
    if ( !fOK )
    {
      pDimpIda->usDDERc = ERROR_NOT_ENOUGH_MEMORY;
      UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
      pDimpIda->fStop = TRUE;
    } /*endif */
    else
    {
      pszDict = pDimpIda->szPropPath;
      //open asd dictionary
      usRc = AsdOpen( pDimpIda->hUser, // user ctl block handle
                      ASD_LOCKED,      // open flags
                      1,               // nr of dict in ppszDict
                      &pszDict,        // dictionary properties
                      &pDimpIda->hDict1,// dict ctl block handle
                      &usErrDict );    // number of failing dict

      if ( usRc == LX_RC_OK_ASD )
      {
        usStatus = QLDBFillFieldTables( pDimpIda->pSgmlProp,
                                        pDimpIda->ausNoOfFields,
                                        NULL );
        if ( usStatus != QLDB_NO_ERROR )
        {
          pDimpIda->usDDERc = usStatus;
          UtlErrorHwnd( usStatus, MB_CANCEL,
                        0, NULL, QLDB_ERROR, pDimpIda->HWNDImpCmd );
          fOK = FALSE;  //does not wish to continue with next entry
        } /* endif */
      }
      else
      {
        pName = QDAMErrorString( usRc, pDimpIda->pszMsgError );
        pDimpIda->usDDERc = usRc;
        UtlErrorHwnd( usRc, MB_CANCEL,
                      1, &pName, QDAM_ERROR, pDimpIda->HWNDImpCmd );
        fOK = FALSE;

        if ( usRc == LX_PROTECTED_ASD )
        {
          REMOVESYMBOL( pDimpIda->szSymbol );
          pDimpIda->fFree = FALSE;
        } /* endif */

        //if not remote and access denied then grey out and allow delete
        if ( pDimpIda->pSgmlProp->szServer[0] == NULC )
        {
          if ( (usRc == BTREE_ACCESS_ERROR) ||
               (usRc == BTREE_FILE_NOTFOUND) ||
               (usRc == BTREE_INVALID_DRIVE) )
          {
            //return sitem in dict list window
            sItem = QUERYSELECTION( pDimpIda->DictLBhwnd,
                                    PID_DICTIONARY_LB );
            if ( sItem != LIT_NONE )
            {
              //send msg to grey out dict in dict list window
              CLBSETITEMSTATE( pDimpIda->DictLBhwnd,
                               PID_DICTIONARY_LB, sItem, FALSE );
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // if ok, close properties
  if ( hSgmlProp )
  {
    CloseProperties( hSgmlProp, PROP_QUIT, &ErrorInfo);
    pDimpIda->pSgmlProp = NULL;
  } /* endif */

  if ( !fOK  )
  {
    pDimpIda->fStop = TRUE;
    pDimpIda->fNotOk = TRUE;   //no terminating msg
  } /* endif */
  return( fOK );
} /* end End_MapTableProcess */


//This function compares profiles when the user has specified that he wishes
//to merge two dictionaries and issues a warning when the profiles are not
//identical. Return TRUE if profiles identical, else FALSE.

static
BOOL CompareProfiles( PDIMPIDA pDimpIda ) //pointer ida
{
  PPROFENTRY    pAsdProfEntry;     //pointer to asd profile map entry
  PPROFENTRY    pSgmlProfEntry;    //pointer to asd profile map entry
  USHORT        usCount1 = 1;      //counter for dict1 entries
  USHORT        usCount2 = 1;      //counter for dict2 entries
  BOOL          fOK = TRUE;        //success indicator
  EQFINFO       ErrorInfo;         //property error information
  PVOID         hProp;             //handle of dict properties
  PPROPDICTIONARY pAsdDictProp;    //pointer to dict properties
  PSZ           pError;
  PSZ           pszMsgTable[3];     //error msg parameter list
  USHORT        usRc;
  CHAR          szBuffer[MAX_LONGPATH];

  //read asd dict profile (not new sgml file profile)
  //open dictionary properties
  PROPNAME( pDimpIda->szString, pDimpIda->szShortName );
  hProp = OpenProperties( pDimpIda->szString, NULL,
                          PROP_ACCESS_READ, &ErrorInfo);
  if ( !hProp )
  {
    pError = UtlGetFnameFromPath( pDimpIda->szNewDictName );
    pDimpIda->usDDERc = ERROR_OPENING_PROPS;
    UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
                  1, &pError, EQF_ERROR, pDimpIda->HWNDImpCmd );
    pDimpIda->fStop = TRUE;
  }
  else
  {
    pAsdDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hProp );

    //compare map entries
    pAsdProfEntry = pAsdDictProp->ProfEntry; //ptr to first asd prof entry
    pSgmlProfEntry = pDimpIda->pSgmlProp->ProfEntry; //ptr to first sgml prof entry

    //compare source languages                                     //$1020a
    if ( stricmp( pAsdDictProp->szSourceLang,                      //$1020a
                  pDimpIda->pSgmlProp->szSourceLang ) != 0 )       //$1020a
    {
      //$1020a
      //continue regardless of the fact that the entry structures differ
      //if import called in batch mode
      if ( !ISBATCHHWND(pDimpIda->HWNDImpCmd) )
      {
        //source languages differ so choose whether to continue      //$1020a
        Utlstrccpy( pDimpIda->szName,                                //$1020a
                    UtlGetFnameFromPath( pDimpIda->szNewDictName ), DOT ); //$1020a
        pszMsgTable[0] = pDimpIda->szName;                             //$1020a
        pszMsgTable[1] = pAsdDictProp->szSourceLang;                   //$1020a
        pszMsgTable[2] = pDimpIda->pSgmlProp->szSourceLang;            //$1020a
        usRc = UtlError( ERROR_DICT_DIFF_SOURCE_LANG, MB_YESNO, 3,     //$1020a
                         pszMsgTable, EQF_QUERY );                     //$1020a
        if ( usRc == MBID_NO )                                         //$1020a
        {
          //$1020a
          pDimpIda->fStop = TRUE;                                      //$1020a
        } /* endif */                                                  //$1020a
      } /* endif */
    } /* endif */                                                    //$1020a




    if ( !pDimpIda->fStop )
    {
      if ( pAsdDictProp->usLength == pDimpIda->pSgmlProp->usLength )
      {
        while ( usCount1 <= pAsdDictProp->usLength && fOK )
        {
          if ( stricmp( pAsdProfEntry->chSystName,
                        pSgmlProfEntry->chSystName) == 0 )
          {
            pAsdProfEntry++;  //get next asd dict entry for comparison
            usCount1++;       //increase asd dict counter
            pSgmlProfEntry = pDimpIda->pSgmlProp->ProfEntry; //set to first
            usCount2 = 1;     //reset counter to 1
            fOK = TRUE;
          }
          else  //look in next SgmlProfEntry for possible equivalence
          {
            if ( usCount2 <= pDimpIda->pSgmlProp->usLength )
            {
              pSgmlProfEntry++; //get next sgml prof entry
              usCount2++;       //increase sgml dict prof entry counter
            }
            else   // sTokenId not in SgmlProfEntry
              fOK = FALSE;
          } /* endif */
        } /* endwhile */
      }
      else
        fOK = FALSE;
    } /* endif */

    //use length of asd property
    pDimpIda->usMapNext = pAsdDictProp->usLength;

    //use asd dict props if merge yes
    if(pAsdDictProp->szLongDesc[0] == EOS)
    {
       strcpy( pAsdDictProp->szLongDesc, pDimpIda->pSgmlProp->szLongDesc);
       SetPropAccess(hProp, PROP_ACCESS_WRITE);
       SaveProperties(hProp, &ErrorInfo);

       PROPNAME( szBuffer, pDimpIda->szName );
       EqfSend2Handler( DICTIONARYHANDLER,
                        WM_EQFN_PROPERTIESCHANGED,
                        MP1FROMSHORT( PROP_CLASS_DICTIONARY ),
                        MP2FROMP(szBuffer ));

    }
    memcpy( pDimpIda->pSgmlProp, pAsdDictProp, sizeof(PROPDICTIONARY) );

    //build new mapentry with AsdDictProp info
    if ( !pDimpIda->fImpMerge )
      pDimpIda->fStop = NewMapEntry( pDimpIda );

  } /* endif */

  // if ok, close properties
  if ( hProp )
  {
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo);
  } /* endif */

  return( fOK );
}

static
BOOL NewMapEntry ( PDIMPIDA pDimpIda )
{
  USHORT        usI = 0;
  PPROFENTRY    pPropEntry;     //pointer to asd profile map entry
  BOOL          fOK;
  PMAPENTRY     pMapEntry;
  PTOKENFIELD   pTokenField;

  UtlAlloc( (PVOID *) &pDimpIda->pMapEntry, 0L, 0L, ERROR_STORAGE ); //release memory
  fOK = UtlAlloc( (PVOID *) (PSZ_W *) &(pDimpIda->pMapEntry),
                  0L, (LONG)(sizeof(MAPENTRY) * pDimpIda->usMapNext), NOMSG );
  if ( !fOK )
  {
    pDimpIda->fStop = TRUE;
  }
  else
  {
    pMapEntry = pDimpIda->pMapEntry;
    pPropEntry = pDimpIda->pSgmlProp->ProfEntry;

    //initialize level begins
    pDimpIda->usHomLevel = 0;
    pDimpIda->usSenseLevel = 0;
    pDimpIda->usTargetLevel = 0;

    while ( usI < pDimpIda->usMapNext )
    {
      if ( pPropEntry->sTokenId == 0 )
      {
        pTokenField = &(TokenField[0]);
        while ( pTokenField->sTokenid != LAST_TAG && fOK )
        {
          if ( stricmp ( pPropEntry->chSystName,
                         pTokenField->chSysName ) == 0 )
          {
            pMapEntry->sTokenId = pTokenField->sTokenid;       //token id
            fOK = FALSE;
          }
          else
          {
            pTokenField++;
          } /* endif */
        } /* endwhile */
      }
      else
      {
        pMapEntry->sTokenId = pPropEntry->sTokenId;
      } /* endif */

      pMapEntry->sId = pPropEntry->sId;                 //system entry field
      pMapEntry->usDisplay = pPropEntry->usDisplay;     //display level
      pMapEntry->usStatus = pPropEntry->usStatus;       //authorization
      pMapEntry->fVital = pPropEntry->fVital;           //obligatory field
      pMapEntry->fAutLookup = pPropEntry->fAutLookup;   //aut. lookup display flag
      pMapEntry->usEntryType = pPropEntry->usEntryFieldType; //system entry field type
      //remember where the hom, sense, target entry fields begin
      //as this is necessary for qldbaddsubtree
      if ( pPropEntry->usLevel > 1 )
      {
        if ( (pPropEntry-1)->usLevel != pPropEntry->usLevel )
        {
          switch ( pPropEntry->usLevel )
          {
            case 2:   //homonyn level entry fields
              pDimpIda->usHomLevel = usI;
              break;
            case 3:   //sense level entry fields
              pDimpIda->usSenseLevel = usI;

              //if either hom level is empty then level never
              //gets set so important to set here to target level value
              if ( pDimpIda->usHomLevel == 0 )
                pDimpIda->usHomLevel = pDimpIda->usSenseLevel;
              break;
            case 4:   //target level entry fields
              pDimpIda->usTargetLevel = usI;

              //if either hom or sense level are empty then levels never
              //get set so important to set here to target level value
              if ( pDimpIda->usSenseLevel == 0 )
                pDimpIda->usSenseLevel = pDimpIda->usTargetLevel;
              if ( pDimpIda->usHomLevel == 0 )
                pDimpIda->usHomLevel = pDimpIda->usTargetLevel;
              break;
          } /* endswitch */
        } /* endif */
      } /* endif */
      pMapEntry->usLevel = pPropEntry->usLevel;
      pMapEntry->pData = L"";

      //update counters
      usI++;
      pMapEntry++;
      pPropEntry++;
      fOK = TRUE;
    } /* endwhile */
  } /* endif */
  return( pDimpIda->fStop );
}

//This function fills the a structure with the allowed entry fields (MapEntry)
//which is used as a check for validity each time an entry is processed and
//if the data is valid stores it for later addition to the dictionary.
//This function returns TRUE if all went well.

static
BOOL FillMapTable( PDIMPIDA pDimpIda,      // pointer to dictionary import
                   PSZ_W    pDummyW,        // passed string  ( chUserName of field)
                   SHORT    sToken,        // token value
                   SHORT    sId,           // value of the attribute id=
                   USHORT   usDisplay,     // value of the attribute DISPLEVEL=
                   USHORT   usStatus,      // value of the attribute STATUS=
                   BOOL     fVital,        // indicates if field oblig. or not
                   BOOL     fAutLookup,    // display field in aut. lookup
                   USHORT   usEntryType )  // value of the attribute ENTRYTYPE=
{
  PMAPENTRY   pMapEntry;              // pointer to map entry
  BOOL        fOK = TRUE;             // success indicator

  pMapEntry = pDimpIda->pMapEntry;    // get index into MAPTABLE

  // set index to first entry of level
  switch (pDimpIda->usActiveLevel)
  {
    case POS_LEVEL:
      pDimpIda->usHomLevel = pDimpIda->usMapNext;
      pDimpIda->usActiveLevel = HW_LEVEL;
      pDimpIda->usLevel = 2;
      break;
    case SENSE_LEVEL:
      pDimpIda->usSenseLevel = pDimpIda->usMapNext;
      pDimpIda->usActiveLevel = HW_LEVEL;

      //if either hom level is empty then level never
      //gets set so important to set here to target level value
      if ( pDimpIda->usHomLevel == 0 )
        pDimpIda->usHomLevel = pDimpIda->usSenseLevel;

      pDimpIda->usLevel = 3;
      break;
    case TARGET_LEVEL:
      pDimpIda->usTargetLevel = pDimpIda->usMapNext;
      pDimpIda->usActiveLevel = HW_LEVEL;

      //if either hom or sense level are empty then levels never
      //get set so important to set here to target level value
      if ( pDimpIda->usSenseLevel == 0 )
        pDimpIda->usSenseLevel = pDimpIda->usTargetLevel;
      if ( pDimpIda->usHomLevel == 0 )
        pDimpIda->usHomLevel = pDimpIda->usTargetLevel;

      pDimpIda->usLevel = 4;
      break;
  } /* endswitch */

  //fill MapTable
  pMapEntry += pDimpIda->usMapNext;        //get next entry
  pMapEntry->sTokenId = sToken;
  pMapEntry->sId = sId;                         //system entry field
  if ( usDisplay >= 4 )                    //display
  {
    pMapEntry->usDisplay = usDisplay - 4;

  }
  else
  {
  } /* endif */
  pMapEntry->usDisplay = usDisplay;        //display level
  pMapEntry->usStatus = usStatus;          //authorization
  pMapEntry->fVital = fVital;              //obligatory field
  pMapEntry->fAutLookup = fAutLookup;      //obligatory field
  pMapEntry->usEntryType = usEntryType;    //system entry field type
  pMapEntry->usLevel = pDimpIda->usLevel;
  pMapEntry->pData = pDummyW;
  pDimpIda->usMapNext++;                   //ptr to next entry

  //check if enough space for next entry
  if ( pDimpIda->usMapNext >= pDimpIda->usMapAvail )
  {
    fOK = UtlAlloc ( (PVOID *) &(pDimpIda->pMapEntry),
                     (LONG) (sizeof(MAPENTRY) * pDimpIda->usMapAvail),
                     (LONG) (sizeof(MAPENTRY) *
                             (MAX_USER+pDimpIda->usMapAvail)),
                     NOMSG );
    if ( fOK )
    {
      pDimpIda->usMapAvail = pDimpIda->usMapAvail + MAX_USER;
    }
    else
    {
      pDimpIda->usDDERc = ERROR_NOT_ENOUGH_MEMORY;
      UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
    } /* endif */
  } /* endif */
  return( fOK );
} /* end FillMapTable */

static
VOID AddDate( PDIMPIDA pDimpIda )      // pointer to dictionary import
{
  USHORT      usI = 0;                // index
  PSZ_W       * ppData;               // pointer to data
  LONG        lTime;                  // date/time as long value
  PMAPENTRY   pMapEntry;              // pointer to map entry

  pMapEntry = pDimpIda->pMapEntry;    // get index into MAPTABLE

  //get present date
  UtlTime( &lTime );
  UtlLongToDateStringW( lTime, pDimpIda->szDateW, sizeof(pDimpIda->szDateW) / sizeof(CHAR_W) );

  pMapEntry = pDimpIda->pMapEntry;    // get index into MAPTABLE
  while ( (usI < pDimpIda->usMapNext) )
  {
    switch ( (pMapEntry+usI)->sTokenId )
    {
      case ECRDATE_TOKEN:
      case ELUPDATE_TOKEN:
      case TCRDATE_TOKEN:
      case TLUPDATE_TOKEN:
        ppData = (PSZ_W *) &(pDimpIda->pEntry[ usI * sizeof(PSZ)]);
        *ppData = pDimpIda->szDateW;
        break;
    } /* end switch */
    usI++;
  } /* endwhile */
} /* end AddDate */

static
BOOL FillEntry( PDIMPIDA pDimpIda,      // pointer to dictionary import
                PSZ_W    pDummyW,        // passed string
                SHORT    sToken,        // token value
                SHORT    sId,           // value of the attribute id=
                PTOKENENTRY pToken)     // field start token (only used for any error message)
{
  PMAPENTRY   pMapEntry;              // pointer to map entry
  BOOL        fOK = TRUE;             // success indicator
  BOOL        fFound;                 // entry found in maptable
  USHORT      usI = 0;                // index
  USHORT      usResult;               // return value from error return
  PSZ_W       * ppData;               // pointer to data

  pMapEntry = pDimpIda->pMapEntry;    // get index into MAPTABLE

  fFound = FALSE;
  while ( (usI < pDimpIda->usMapNext) && !fFound )
  {                                  // does token and fieldtype match??
    if ( ((pMapEntry+usI)->sTokenId == sToken )
         && ((pMapEntry+usI)->sId == sId))
    {
      fFound = TRUE;
    }
    else
    {
      usI++;            // point to next entry
    } /* endif */
  } /* endwhile */
  // save string and set pointer
  if ( fFound )
  {
    ppData = (PSZ_W *) &(pDimpIda->pEntry[ usI * sizeof(PSZ)]);
    if ( *ppData != NULL )
    {
      // append the new data to the existing one
      *ppData = Save3Strings( pDimpIda, *ppData, L"; ", pDummyW );
    }
    else
    {
      // use new data
      *ppData = pDummyW;
    }
  }
  else
  {
    if ( !pDimpIda->fIgnoreFieldNotInDictError && !pDimpIda->fNoError && !pDimpIda->fMerge )
    {
      if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
      {
        pDimpIda->fNoError = TRUE;
      }
      else
      {
        wcsncpy( pDimpIda->szErrorMsgParm, pToken->pDataStringW, pToken->usLength );
        pDimpIda->szErrorMsgParm[pToken->usLength] = 0;

        if ( sId != SYSFIELD )
        {
          swprintf( pDimpIda->szErrorMsgParm + wcslen(pDimpIda->szErrorMsgParm), L" id=%d", sId );
        }

        PSZ_W pszParms[2];
        pszParms[0] = ( pDimpIda->szErrorMsgParm[0] == L'<' ) ? pDimpIda->szErrorMsgParm + 1 : pDimpIda->szErrorMsgParm;
        pszParms[1] = pDummyW;

        usResult = UtlErrorW( ERROR_DIMP_NOTINMAP, MB_EQF_YESTOALL, 2, pszParms, EQF_QUERY, TRUE );

        if ( usResult == MBID_EQF_YESTOALL )
        {
          pDimpIda->fIgnoreFieldNotInDictError = TRUE;
          fOK = TRUE;
        }
        else if ( usResult == MBID_YES )
        {
          fOK = TRUE;
        }
        else
        {
          fOK = FALSE;
        }

        if ( !fOK )
        {
          pDimpIda->fStop = TRUE;     //stop dimpwork loop
          pDimpIda->fNotOk = TRUE;    //delete all dict files in cleanup
        } /* endif */
      } /* endif */
    }
    else
    {
      fOK = TRUE;
    } /* endif */
  } /* endif */
  return( fOK );
} /* end FillEntry */

//Checks whether the value of token attributes is valid and stores it. E.g.
//DISPLEVEL can only have the values 0,1,2,3 and 4 to 7.
//This function is called up in MapIdMatch and returns the valid attribute value.

static
PSZ_W CheckAttribute( PDIMPIDA pDimpIda,
                      PTOKENENTRY pToken,
                      PSZ_W       pString )
{
  PSZ_W    pRetCharW = NULL;    //either display, status or entry type char
  CHAR_W   chCharW;      //value of attribute
  SHORT    sAttribute;
  PSZ_W    pDataW;       // pointer to text string

  sAttribute = pDimpIda->sOrgToken[pToken->sTokenid];
  pDataW = pToken->pDataStringW;
  pDataW += UTF16strlenCHAR( DimpTokenTable[sAttribute].szNameW );
  if ( wcschr( L"\"\'", *pDataW ) ) 
     ++pDataW ;
  chCharW = *pDataW;
  if ( pString != NULL)
  {
    pRetCharW = UTF16strchr( pString, chCharW );
  } /* endif */

  return( pRetCharW );  //return valid attribute value string
} /* CheckAttribute */

//Checks whether the value of token XML attributes is valid and stores it. E.g.
//AUTLOOKUP can only have the values 0 or 1.
//This function is called up in MapIdMatch and returns the valid attribute value.

static
PSZ_W CheckAttributeXML( PDIMPIDA pDimpIda,
                         PTOKENENTRY pToken,
                         PSZ_W       pString )
{
  PSZ_W    pRetCharW = NULL;    //either display, status or entry type char
  CHAR_W   chCharW;             //value of attribute
  CHAR_W   chQuoteW = NULL;     //Quote char (if surrounding by quotes)
  SHORT    sAttribute;
  PSZ_W    pDataW;              // pointer to text string

  sAttribute = pDimpIda->sOrgToken[pToken->sTokenid];
  pDataW = pToken->pDataStringW;
  pDataW += UTF16strlenCHAR( DimpTokenTable[sAttribute].szNameW );
  if ( *pDataW == L'=' ) 
  {
    ++pDataW ;
    if ( wcschr( L"\"\'", *pDataW ) ) { 
       chQuoteW = *pDataW ;
       ++pDataW ;
    }
    chCharW = *pDataW;
    if ( pString != NULL ) 
    {
      if ( chCharW == chQuoteW )     // Empty quoted value
         chCharW = L'0' ;
      pRetCharW = UTF16strchr( pString, chCharW );
    }
  } else {
     chCharW = L'1' ;               // Only keyword, assume it is set on  5-12-15 
  }

  return( pRetCharW );  //return valid attribute value string
} /* CheckAttributeXML */

//This function is called up while the maptable is being processed (which
//determines the entry structure) and checks the attributes id=, displevel=,
//status=, entrytype= and saves the user name of the system field. Returns TRUE if
//all went well, else error message.

static
BOOL MapIdMatch( PDIMPIDA pDimpIda,
                 PTOKENENTRY *ppToken,
                 PSZ_W   *ppString,
                 PUSHORT pusId,
                 PUSHORT pusDisplay,
                 PUSHORT pusStatus,
                 PBOOL pfVital,
                 PBOOL pfAutLookup,
                 PUSHORT pusEntryType )
{
  PTOKENENTRY pToken;        //pointer to token
  BOOL        fOK = TRUE;    //success indicator
  USHORT      usResult;      //error handling return code
  PSZ_W         pValueW;        //return value of attribute
  SHORT       sToken;        //tokenid
  PSZ_W         pDataW;         //pointer to text string
//PSZ           pData;

  *pusId = (USHORT)SYSFIELD;
  *pusDisplay = 0;
  *pusStatus = 0;
  *pusEntryType = 0;
  *pfVital = FALSE;
  *pfAutLookup = FALSE;
  *ppString = NULL;

  pToken = (*ppToken);       //get contents of pointer
  if (pToken->sTokenid>=0)
  {
    sToken = pDimpIda->sOrgToken[pToken->sTokenid];
  }
  else
  {
	  sToken = pToken->sTokenid;
  }
  pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent] = sToken;
  pToken++;                  //point to next token

  while ( pToken->sTokenid != TEXT_TOKEN && fOK )
  {
    switch ( pDimpIda->sOrgToken[pToken->sTokenid] )
    {
      case ID_AUTLOOKUP:
        *pfAutLookup = TRUE;
        pValueW = CheckAttributeXML( pDimpIda, pToken, AUTLOOKUPW );
        if ( pValueW != NULL )
        {
          if ( *pValueW == L'0' )
            *pfAutLookup = FALSE ;
          pToken++ ;
        }
        break;
      case ID_VITAL:
        *pfVital = TRUE;
        pValueW = CheckAttributeXML( pDimpIda, pToken, VITALW );
        if ( pValueW != NULL )
        {
          if ( *pValueW == L'0' )
            *pfVital = FALSE ;
          pToken++ ;
        }
        break;
      case ID_DISPLAY:
        pValueW = CheckAttribute( pDimpIda, pToken, DISPLEVELW );
        if ( pValueW != NULL )
        {
          *pusDisplay =  *pValueW - '0';
        }
        else
        {
          //incorrect attribute value
          fOK = FALSE;
        } /* endif */
        break;
      case ID_ENTRYTYPE:
        pValueW = CheckAttribute( pDimpIda, pToken, ENTRYTYPEW );
        if ( pValueW != NULL )
        {
          *pusEntryType = *pValueW - '0';
        }
        else
        {
          //incorrect attribute value
          fOK = FALSE;
        } /* endif */
        break;
      case ID_STATUS:
        pValueW = CheckAttribute( pDimpIda, pToken, STATUSW );
        if ( pValueW != NULL )
        {
          *pusStatus = *pValueW - '0';
        }
        else
        {
          //incorrect attribute value
          fOK = FALSE;
        } /* endif */
        break;
      case DICT_ID_ATTR:
        pDataW = pToken->pDataStringW;
        pDataW += UTF16strlenCHAR( DimpTokenTable
                         [pDimpIda->sOrgToken[pToken->sTokenid]].szNameW );
        if ( wcschr( L"\"\'", *pDataW ) ) 
           ++pDataW ;
        //*pusId = atoi( pData );
        *pusId = (USHORT)_wtoi(pDataW );
        break;
      default:
        fOK = FALSE;
    } /* endswitch */
    pToken++;
  } /* endwhile */

  if ( (pToken->sTokenid == TEXT_TOKEN) && fOK )
  {
    if ( pToken->usLength > DICTENTRYLENGTH-1 )
    {
      if ( !pDimpIda->fMerge )
      {
        PSZ_W pszParm = pDimpIda->chWorkBuffer;

        //issue msg that entry field name is too long and that if the user
        //opts to continue, the name will be truncated
        wcsncpy( pDimpIda->chWorkBuffer, pToken->pDataStringW, pToken->usLength );
        pDimpIda->chWorkBuffer[pToken->usLength+1] = EOS;
        usResult = UtlErrorW( ERROR_FIELDNAME_TRUNCATED, MB_YESNO, 1,
                              &pszParm, EQF_QUERY, TRUE );
        if ( usResult == MBID_YES )
        {
          *ppString = pToken->pDataStringW;  // get pointer to string data
          fOK = SaveString( pDimpIda, ppString, pToken->usLength );
        }
        else
        {
          pDimpIda->fStop = TRUE;          //premature close, relesase memory
          pDimpIda->fNotOk = TRUE;
        } /* endif */
      }
      else
      {
        *ppString = pToken->pDataStringW;  // get pointer to string data
        fOK = SaveString( pDimpIda, ppString, pToken->usLength );
      } /* endif */
    }
    else
    {
      *ppString = pToken->pDataStringW;          // get pointer to string data
      fOK = SaveString( pDimpIda, ppString, pToken->usLength );
    } /* endif */
  } /* endif */

  if ( !fOK )  //incorrect attribute value/style
  {
    pDimpIda->usDDERc = ERROR_MAP_INVSTYLE;
    UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 0,
                  NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
    pDimpIda->fStop = TRUE;          //premature close, relesase memory
    fOK = FALSE;                     //set function return value
    pDimpIda->fNotOk = TRUE;
  } /* endif */

  if ( pDimpIda->fStop )
    fOK = FALSE;

  *ppToken = pToken;
  return( fOK );
}  /* end MapIdMatch */

//Checks the id= attribute in user defined entry fields during entry processing
//and returns TRUE if the attribute was valid and the data string correctly
//saved.

static
BOOL IDMatch( PDIMPIDA pDimpIda,          // ptr to structure
              PTOKENENTRY *ppToken,       // token table
              PSZ_W *ppString,              // ptr to string
              PUSHORT  pusId )            // specified id
{
  BOOL    fOK;                           // success indicator
  SHORT   sToken;                        // original token id
  SHORT   sAttribute;                    // attribute id
  PTOKENENTRY pToken;                    // pointer to token
  USHORT  usResult;                      // return from error
  PSZ_W   pData;                         // pointer to text string
  USHORT  usLength;                      // smaller of two values
  PSZ_W   pMsgError[2];                  // pointer to error data

  *ppString = NULL;                      // initialise string
  pToken = (*ppToken);                   // get contents of pointer
  if (pToken->sTokenid>=0)
  {
    sToken = pDimpIda->sOrgToken[pToken->sTokenid];
  }
  else
  {
    sToken = pToken->sTokenid;
  }
  if ((pToken+1)->sTokenid>=0)
  {
    sAttribute = pDimpIda->sOrgToken[(pToken+1)->sTokenid];

  }
  else
  {
	sAttribute = (pToken+1)->sTokenid;
  }

  if ( ( sAttribute == DICT_ID_ATTR )
       && (( pToken+2)->sTokenid == TEXT_TOKEN))
  {
    pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent] = sToken;
    *ppString = (pToken+2)->pDataStringW;
    fOK = SaveString( pDimpIda, ppString, (pToken+2)->usLength );
    if ( fOK )
    {
      pData = (pToken+1)->pDataStringW;    // point to text of attribute
//         pData[(pToken+1)->usLength-1] = EOS;
      pData += UTF16strlenCHAR( DimpTokenTable[sAttribute].szNameW );
      if ( wcschr( L"\"\'", *pData ) ) 
         ++pData ;
      *pusId = (USHORT)_wtoi( pData );             // convert to integer
      *ppToken += 3;                      // skip tagid and text
    } /* endif */
  }
  else
  {
    if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
    {
      pDimpIda->fStop = TRUE;            //premature close, release memory
      fOK = FALSE;
      usLength = min( pToken->usLength, BUFFERLEN-1 );
      memcpy( (PBYTE)pDimpIda->chWorkBuffer,
              (PBYTE)pToken->pDataStringW, usLength );
      pDimpIda->chWorkBuffer[usLength] = EOS;
      pMsgError[0] = pDimpIda->chWorkBuffer;
      pMsgError[1] = pDimpIda->ucErrorTerm;
      pDimpIda->usDDERc = ERROR_DIMP_INVSTYLE;
      UtlErrorHwndW( pDimpIda->usDDERc, MB_YESNO, 2,
                    pMsgError, EQF_ERROR, pDimpIda->HWNDImpCmd, TRUE );
    }
    else
    {
      usLength = min( pToken->usLength, BUFFERLEN-1 );
      memcpy( (PBYTE)pDimpIda->chWorkBuffer,
              (PBYTE)pToken->pDataStringW, usLength * sizeof(CHAR_W) );
      pDimpIda->chWorkBuffer[usLength] = EOS;
      pMsgError[0] = pDimpIda->chWorkBuffer;
      pMsgError[1] = pDimpIda->ucErrorTerm;
      usResult = UtlErrorW ( ERROR_DIMP_INVSTYLE,
                             MB_YESNO, 2, pMsgError, EQF_QUERY, TRUE );

      if ( usResult == MBID_NO )
      {
        pDimpIda->fStop = TRUE;            //premature close, release memory
        fOK = FALSE;
      }
      else                                  //user wants to go on
      {
        SkipTag( pDimpIda, ppToken );      // skip rest of tag
        fOK = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */
  return( fOK );
}  /* end IDMatch */

//Opens a file and returns the length of the file and the handle of the file.

static
HFILE DimpOpenFile
(
PDIMPIDA pDimpIda,                  // ptr to ida
PSZ    pFileName,                   // ptr to name of file
PULONG pulFileSize                  // location to store the file length
)
{
  USHORT    usRC;                     // return code of Dos... calls
  USHORT    usOpenAction;             // action returned by DosOpen call
  HFILE     hFile;                    // handle of file

  // open file
  usRC = UtlOpen ( pFileName, &hFile, &usOpenAction,
                   0L, FILE_NORMAL, FILE_OPEN,
                   OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );

  // get file info
  if ( usRC == 0 )
  {
    *pulFileSize = 0L;
    usRC = UtlGetFileSize( hFile, pulFileSize, FALSE );
  } /* endif */

  if ( usRC != 0 )
  {
    pDimpIda->fStop = TRUE;
    pDimpIda->fNotOk = TRUE;
    pDimpIda->usDDERc = usRC;
    UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 1,
                  &pFileName, DOS_ERROR, pDimpIda->HWNDImpCmd );
  } /* endif */

  return( hFile );                    // return file handle
} /* end of DimpOpenFile */

//Saves data between tokens in the entries section of the sgml-based
//dictionary file. The tokens represent system entry fields and not
//user-defined fields that would contain an id. Returns TRUE if the data is
//successfully saved.

static
BOOL  TextMatch ( PDIMPIDA    pDimpIda,         // dictionary import struct
                  PTOKENENTRY *ppToken,         // pointer into token table
                  PSZ_W       * ppString)       // pointer to string
{
  USHORT usResult;                             // return value from error box
  PTOKENENTRY pToken;                          // pointer to token
  SHORT   sToken;                              // original token id
  BOOL  fOK = TRUE;                            // success indicator
  USHORT usLength;                             // returns the smaller value
  PSZ_W  pMsgError[2];                            // pointer to error data

  *ppString = NULL;                           // string not set yet
  pToken = ++(*ppToken);                      // check for text token

  if (pToken->sTokenid == TEXT_TOKEN)
  {
	  if ((pToken-1)->sTokenid>=0)
	  {
	      sToken = pDimpIda->sOrgToken[(pToken-1)->sTokenid];
	  }
	  else
	  {
	      sToken = (pToken-1)->sTokenid;
      }

    pDimpIda->Stack.sStack[++pDimpIda->Stack.sCurrent] = sToken;
    *ppString = pToken->pDataStringW;          // get pointer to string data
    fOK = SaveString( pDimpIda, ppString, pToken->usLength );
  }
  else
  {
    if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
    {
      pDimpIda->fStop = TRUE;             //premature close, relesase memory
      fOK = FALSE;                        //set function return value
      pDimpIda->fNotCompleted = TRUE;     //import terminated in the middle
      usLength = min( pToken->usLength, BUFFERLEN-1 );
      memcpy( (PBYTE)pDimpIda->chWorkBuffer,
              (PBYTE)pToken->pDataStringW, usLength );
      pDimpIda->chWorkBuffer[usLength] = EOS;
      pMsgError[0] = pDimpIda->chWorkBuffer;
      pMsgError[1] = pDimpIda->ucErrorTerm;
      pDimpIda->usDDERc = ERROR_DIMP_INVSTYLE;
      UtlErrorHwndW( pDimpIda->usDDERc, MB_YESNO, 2,
                     pMsgError, EQF_ERROR, pDimpIda->HWNDImpCmd, TRUE );
    }
    else
    {
      usLength = min( pToken->usLength, BUFFERLEN-1 );
      memcpy( (PBYTE)pDimpIda->chWorkBuffer,
              (PBYTE)pToken->pDataStringW, usLength );
      pDimpIda->chWorkBuffer[usLength] = EOS;
      pMsgError[0] = pDimpIda->chWorkBuffer;
      pMsgError[1] = pDimpIda->ucErrorTerm;
      usResult = UtlErrorW ( ERROR_DIMP_INVSTYLE, MB_YESNO,
                             2, pMsgError, EQF_QUERY , TRUE);
      if ( usResult == MBID_NO )
      {
        pDimpIda->fStop = TRUE;             //premature close, relesase memory
        fOK = FALSE;                        //set function return value
        pDimpIda->fNotCompleted = TRUE;     //import terminated in the middle
      }
      else                                   //user wants to go on
      {
        SkipTag( pDimpIda, ppToken );    // skip rest of tag
        fOK = TRUE;                      //set function return value
      } /* endif */
    } /* endif */
  } /* endif */
  return( fOK );                           // pass back the value
}

//This function is called up when an error occurs while reading an entry in the
//sgml-based format and the user opts for ignoring the error and continuing
//with the next entry. The pointer is set on the next entry and processing
//continues in the function DimpProcess.

static
VOID SkipTag( PDIMPIDA    pDimpIda,          // pointer to import struct.
              PTOKENENTRY *ppToken )         // pointer into token table
{
  SHORT sTokenId;                           // token id
  SHORT sEndTag;                            // deepest level

  if ((*ppToken)->sTokenid>=0)
  {
    sTokenId = pDimpIda->sOrgToken[(*ppToken)->sTokenid];
  }
  else
  {
    sTokenId = (*ppToken)->sTokenid;
  }

  sEndTag = pDimpIda->sSkipToken;          // find end token of this list
  while ( sTokenId != sEndTag)
  {
    (*ppToken)++;                       // point to next token
    if ((*ppToken)->sTokenid>=0)
	{
	    sTokenId = pDimpIda->sOrgToken[(*ppToken)->sTokenid];
	}
	else
	{
	    sTokenId = (*ppToken)->sTokenid;
    }
   } /* endwhile */
  (*ppToken)++; //position at next new entry or maptable
  pDimpIda->Stack.sCurrent = pDimpIda->sSkipStack; // reset stack
}

//Checks whether tag is a valid tag for dictionary export and issues a warning
//if not. Returns TRUE if tag is valid else processing is stopped and the
//main processing loop is left.

static
BOOL ValidTag( PDIMPIDA    pDimpIda,         // pointer into tag import
               PTOKENENTRY * ppToken,        // pointer to token
               PBOOL       pfValid,          // validation indicator
               SHORT       sTokenId )        // current token
{
  PDIMPTAGENTRY pDimpTagEntry;              // pointer to first tag entry
  SHORT      sLastToken;                    // Last Token
  PSHORT     psValid;                       // point to valid tag
  BOOL       fFound = FALSE;                // no tag found yet
  BOOL       fOK = TRUE;                    // success indicator
  USHORT     usResult;                      // return value from msg box
  PTOKENENTRY  pToken;                      // pointer to token entry
  USHORT     usLength;                      // smaller of two values
  PSZ_W      pErrorW;                        // pointer to error data
  PSZ        pError;
  PSZ_W      pMsgError[2];                  // pointer to error data

  if ( pDimpIda->Stack.sCurrent > -1 )
  {
    *pfValid = FALSE;                         // no valid tag yet

    sLastToken = pDimpIda->Stack.sStack[ pDimpIda->Stack.sCurrent ];
    pDimpTagEntry = &(DimpTokenTable[sLastToken]);

    psValid = (PSHORT)&(pDimpTagEntry->usValid[ 0 ]);
    fFound = FALSE;
    while ( *psValid != 0 && !fFound )
    {
      if ( *psValid == sTokenId )
      {
        fFound = TRUE;                // allowed tag
        *pfValid = TRUE;              // tag is valid
      }
      else
      {
        psValid ++;
      } /* endif */
    } /* endwhile */

    if ( !fFound )
    {
      pToken = *ppToken;
      usLength = min( (*ppToken)->usLength, BUFFERLEN-1 );
      memcpy( (PBYTE)pDimpIda->chWorkBuffer,
              (PBYTE)((*ppToken)->pDataStringW),
              usLength * sizeof(CHAR_W) );
      pDimpIda->chWorkBuffer[usLength] = EOS;
      pErrorW = pDimpIda->chWorkBuffer;

      if ( pDimpIda->sSection == MAPTABLE_SECTION ) //erroneous maptable tag
      {
        if ( pDimpIda->usMapNext > 0 )
        {
          pDimpIda->usDDERc = ERROR_DIMP_INVMAP;
          UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
                        0, NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
          pDimpIda->fStop = TRUE;   //premature close, release memory
          fOK = FALSE;              //leave dimpwork loop
          pDimpIda->fNotOk = TRUE;  //don't issue complete msg
        }
        else
        {
          //maptable missing
          Utlstrccpy( pDimpIda->szName,
                      UtlGetFnameFromPath( pDimpIda->szDictName ), DOT );
          pError = pDimpIda->szName;
          pDimpIda->usDDERc = ERROR_MAPTABLE_MISSING;
          UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
                        1, &pError, EQF_ERROR, pDimpIda->HWNDImpCmd );
          pDimpIda->fStop = TRUE;  //release alloc mem
          pDimpIda->fNotOk = TRUE; //don't issue complete msg
          fOK = FALSE;             //fall out of work loop
        } /* endif */
      }
      else if ( pDimpIda->sSection == HEADER_SECTION ) //erroneous header
      {
        pDimpIda->usDDERc = ERROR_DIMP_INVHEADER;
        UtlErrorHwndW( pDimpIda->usDDERc, MB_CANCEL,
                       1, &pErrorW, EQF_ERROR, pDimpIda->HWNDImpCmd, TRUE );
        pDimpIda->fStop = TRUE;     //premature close, release memory
        fOK = FALSE;                //leave dimpwork loop
        pDimpIda->fNotOk = TRUE;    //don't issue complete msg
      }
      else
      {
        pMsgError[0] = pDimpIda->chWorkBuffer;
        pMsgError[1] = pDimpIda->ucErrorTerm;

        if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
        {
          pDimpIda->usDDERc = ERROR_DIMP_INVTAG;
          UtlErrorHwndW( pDimpIda->usDDERc, MB_YESNO,
                         2, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
          pDimpIda->fStop = TRUE;   //premature close, release memory
          fOK = FALSE;
          pDimpIda->fNotCompleted = TRUE;  //import terminated in the middle
        }
        else
        {
          usResult = UtlErrorW ( ERROR_DIMP_INVTAG, MB_YESNO,
                                 2, pMsgError, EQF_QUERY, TRUE );
          if ( usResult == MBID_NO )
          {
            pDimpIda->fStop = TRUE;   //premature close, release memory
            fOK = FALSE;
            pDimpIda->fNotCompleted = TRUE; //import terminated in the middle
          }
          else   //user wants to go on
          {
            SkipTag( pDimpIda, ppToken );  //skip rest of tag
            fOK = TRUE;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  return( fOK );                              // return result value
}

//This Function is always called up when an entry token is detected (beginning
//of an entry). It checks for the end entry token to ensure that an entire entry
//is processed in one block. If the end entry token can not be found DimpRead
//is called up which reads in the next sgml block. Returns TRUE if the end entry
//token is found and FALSE if a new block has to be read in. FALSE leaves the
//DimpProcess routine and returns to the main import loop.

static
BOOL CheckForEnd( PDIMPIDA pDimpIda,         // pointer to dictionary import
                  PTOKENENTRY pToken,        // token entry
                  SHORT    sEndToken)        // end of entry
{
  BOOL        fOK = TRUE;                   // success indicator
  SHORT       sToken;                       // token id
  PTOKENENTRY pStartToken;                  // passed start token
  BOOL        fFound = FALSE;               // false so far

  pStartToken = pToken;                     // save start token

  while ( !fFound && (pToken->sTokenid != ENDOFLIST ))
  {
    if (pToken->sTokenid > 0)              // valid tag
    {
      sToken = pDimpIda->sOrgToken[ pToken->sTokenid ];
      fFound = ( sToken == sEndToken );  // end token found
    } /* endif */
    if ( !fFound )
    {
      pToken++;                           // point to next token
    } /* endif */
  } /* endwhile */

  if ( !fFound || (pToken++)->sTokenid == ENDOFLIST ) // either middle of entry
  {
    // and end of memory block or next
    // tag is end of memory block
    //@@ DEBUG: is this statement nec??
    pDimpIda->pRest = pStartToken->pDataStringW;
    pDimpIda->DimpTask = EQFDIMP_READ;     // read next block
    fOK = FALSE;
  }
  else
  {
    pDimpIda->sSkipToken = sEndToken;      //store token
    pDimpIda->sSkipStack = pDimpIda->Stack.sCurrent;  //position stack properly
  } /* endif */
  return(fOK);
}

//Writes the data of an entry (data between tags) into a buffer which when the
//end entry tag is detected is written to the dictionary. Returns TRUE if all
//goes well.

static
BOOL SaveString( PDIMPIDA pDimpIda,          // buffer area
                 PSZ_W    * ppString ,            // string to be saved
                 ULONG   ulLength)          // length of passed string in # of w's
{ //@@ in DEBUG check usLength!!
  BOOL   fOK = TRUE;                        // true so far
  PMEM   pMem;                              // pointer to memory block
  PSZ_W    pszSource;
  PSZ_W    pszTarget;

  pMem = pDimpIda->pMem;                    // get address to pointer struct.
  // usAvail u.usUsed is in number of w's
  if ( pMem->ulAvail > (pMem->ulUsed + ulLength ))
  {
    pszSource = *ppString;
    pszTarget = &(pMem->chBuffer[ pMem->ulUsed ]);
    *ppString = &(pMem->chBuffer[ pMem->ulUsed ]);

    while ( ulLength > 0 )
    {
      if ( *pszSource != LF && *pszSource != CR )
      {
        *pszTarget++ = *pszSource++;
        pMem->ulUsed++;
      }
      else
      {
        *pszSource++;
        if ( (*(pszTarget-1) != BLANK) &&
             (*pszSource != BLANK) &&
             (*pszSource != LF) &&
             (*pszSource != CR) )
        {
          *pszTarget++ = BLANK;
          pMem->ulUsed++;
        } /* endif */
      }
      ulLength--;
    } /* endwhile */
    pMem->chBuffer[pMem->ulUsed++] = EOS;
    //remove all blanks
    UtlStripBlanksW( *ppString );               //1136a
  }
  else
  {
    PSZ pszParm = pDimpIda->szEntryTxt;
    WideCharToMultiByte( CP_OEMCP, 0, pDimpIda->ucErrorTerm, -1, pDimpIda->szEntryTxt, sizeof(pDimpIda->szEntryTxt), NULL, NULL );
    pDimpIda->usDDERc = ERROR_DIMP_DATATOOBIG;
    UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 1, &pszParm, EQF_ERROR, pDimpIda->HWNDImpCmd );
    pDimpIda->fStop = TRUE; //premature close, release memory
  } /* endif */
  return( fOK );
}

static
PSZ_W Save3Strings( PDIMPIDA pDimpIda,          // buffer area
                   PSZ_W    pString1,           // first string to be saved
                   PSZ_W    pString2,           // second string to be saved
                   PSZ_W    pString3 )          // third string to be saved
{ 
  PSZ_W    pszSource;
  PSZ_W    pszTarget;
  PSZ_W    pszReturn = NULL;
  PMEM     pMem = pDimpIda->pMem;                    // get address to pointer struct.
  PSZ_W    ptr[4]; 
  int i = 0;
  ULONG ulLength = 1;

  ptr[0] = pString1; ptr[1] = pString2; ptr[2] = pString3; ptr[3] = NULL; 

  while ( ptr[i] != NULL )
  {
    ulLength = ulLength + wcslen(ptr[i]);
    i++;
  } /* endwhile */

  // usAvail u.usUsed is in number of w's
  if ( pMem->ulAvail > (pMem->ulUsed + ulLength ))
  {
    i = 0;

    pszTarget = &(pMem->chBuffer[ pMem->ulUsed ]);
    pszReturn = pszTarget;

    while ( ptr[i] != NULL )
    {
      pszSource = ptr[i];

      while ( *pszSource )
      {
        if ( *pszSource != LF && *pszSource != CR )
        {
          *pszTarget++ = *pszSource++;
          pMem->ulUsed++;
        }
        else
        {
          *pszSource++;
          if ( (*(pszTarget-1) != BLANK) && (*pszSource != BLANK) && (*pszSource != LF) && (*pszSource != CR) )
          {
            *pszTarget++ = BLANK;
            pMem->ulUsed++;
          } /* endif */
        }
      } /* endwhile */

      i++;
    } /* endwhile */

    pMem->chBuffer[pMem->ulUsed++] = EOS;

    //remove all blanks
    UtlStripBlanksW( pszReturn );              
  }
  else
  {
    PSZ pszParm = pDimpIda->szEntryTxt;
    WideCharToMultiByte( CP_OEMCP, 0, pDimpIda->ucErrorTerm, -1, pDimpIda->szEntryTxt, sizeof(pDimpIda->szEntryTxt), NULL, NULL );
    pDimpIda->usDDERc = ERROR_DIMP_DATATOOBIG;
    UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 1, &pszParm, EQF_ERROR, pDimpIda->HWNDImpCmd );
    pDimpIda->fStop = TRUE; //premature close, release memory
  } /* endif */
  return( pszReturn );
}


//Fills the profile structure (entry field structure) in the ida and a xxx.pro
//file (xxx stand for the name given to the dictionary). Returns the profile
//structure.

static
PPROPDICTIONARY FillProfile( PDIMPIDA pDimpIda )
{
  PPROFENTRY pProfEntry;       // pointer to a user profile entry structure
  PMAPENTRY pMapEntry;         // pointer to tag and username correspondance
  PTOKENFIELD pTokenField;     // pointer to tokens
  USHORT usCount, usNo, usLevel; // counters
  BOOL fFound;                 // flag for tag found or not
  BOOL fOK = TRUE;             // success indicator

  //allocate space for profile header
  fOK = UtlAlloc( (PVOID *) &(pDimpIda->pSgmlProp),
                  0L, (LONG)sizeof(PROPDICTIONARY), NOMSG );
  if ( !fOK )
  {
    pDimpIda->usDDERc = ERROR_NOT_ENOUGH_MEMORY;
    UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 0,
                  NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
  }
  else
  {
    //set profile header and allocate space for profile entries
    if ( pDimpIda->usMapNext <= MAX_FIELDS_FOR_DICT )
    {
      memcpy( pDimpIda->pSgmlProp, &SgmlProp, sizeof(PROPDICTIONARY) );
      pDimpIda->pSgmlProp->usLength = pDimpIda->usMapNext; // number of entries
      strcpy( pDimpIda->pSgmlProp->szSourceLang, pDimpIda->szSourceLang );
      strcpy( pDimpIda->pSgmlProp->szLongDesc, pDimpIda->szLongDesc );
      pDimpIda->pSgmlProp->fCopyRight = (EQF_BOOL)pDimpIda->fCopyRight;
      pDimpIda->pSgmlProp->fProtected = FALSE;
      pMapEntry = pDimpIda->pMapEntry;
      pProfEntry = pDimpIda->pSgmlProp->ProfEntry;
      usCount = 0;
      usNo = 0;
      // loop in MapEntry
      while ( usCount < pDimpIda->usMapNext )
      {
        fFound = FALSE;
        pTokenField = &(TokenField[0]);
        // loop in TokenField
        while ( !fFound && (pTokenField->sTokenid != LAST_TAG) )
        {
          if ( pMapEntry->sTokenId != pTokenField->sTokenid )
          {
            pTokenField++;
          }
          else
          {
            fFound = TRUE;
            strcpy( pProfEntry->chSystName, pTokenField->chSysName );

            pProfEntry->fVital = FALSE;

            Unicode2ASCIIBuf( pMapEntry->pData, pProfEntry->chUserName,
                              (USHORT)(UTF16strlenCHAR(pMapEntry->pData)+1),
                              DICTENTRYLENGTH, pDimpIda->ulOEMCP );
            pProfEntry->sId = pMapEntry->sId;
            pProfEntry->sTokenId = pMapEntry->sTokenId;
            pProfEntry->usLevel = pTokenField->usLevel;
            //either default settings or as set down in map table
            if ( pMapEntry->usStatus == 0 )
              pProfEntry->usStatus = pTokenField->usSysStatus;
            else
              pProfEntry->usStatus = pMapEntry->usStatus;

            if ( pMapEntry->usDisplay == 0 )
              pProfEntry->usDisplay = pTokenField->usDisplay;
            else
              pProfEntry->usDisplay = pMapEntry->usDisplay;

            if ( pMapEntry->usEntryType == 0 )
              pProfEntry->usEntryFieldType = pTokenField->usEntryFieldType;
            else
              pProfEntry->usEntryFieldType = pMapEntry->usEntryType;

            pProfEntry->usSystStatus = pTokenField->usSysStatus;
            pProfEntry->fSysVital = pProfEntry->fVital;
            usLevel = pProfEntry->usLevel;
            pProfEntry->fAutLookup = (EQF_BOOL)pMapEntry->fAutLookup;
            pProfEntry++;
          } /* endif */
        } /* endwhile */

        if ( !fFound )     //no TokenEntry found, i.e userfield, use default
        {
          CHAR   chTemp[DICTENTRYLENGTH];
          sprintf( pProfEntry->chSystName, "UserName%d", usNo++ );
          memset( chTemp, NULC, DICTENTRYLENGTH );
          Unicode2ASCIIBuf( pMapEntry->pData, chTemp, (USHORT)(UTF16strlenCHAR(pMapEntry->pData)+1),
                            DICTENTRYLENGTH, pDimpIda->ulOEMCP );
          strncpy( pProfEntry->chUserName, chTemp, DICTENTRYLENGTH );
          pProfEntry->chUserName[DICTENTRYLENGTH-1] = EOS;

          pProfEntry->sId = pMapEntry->sId;
          pProfEntry->sTokenId = pMapEntry->sTokenId;
          pProfEntry->usLevel = pMapEntry->usLevel;
          //either default settings or as set down in map table
          if ( pMapEntry->usStatus == 0 )
            pProfEntry->usStatus = RW_STATUS;
          else
            pProfEntry->usStatus = pMapEntry->usStatus;

          if ( pMapEntry->usDisplay == 0 )
            pProfEntry->usDisplay = THIRD_DISPLAY;
          else
            pProfEntry->usDisplay = pMapEntry->usDisplay;

          if ( pMapEntry->usEntryType == 0 )
            pProfEntry->usEntryFieldType = 1;
          else
            pProfEntry->usEntryFieldType = pMapEntry->usEntryType;
          pProfEntry->usSystStatus = RW_STATUS;
          pProfEntry->fSysVital = FALSE;
          pProfEntry->fVital = FALSE;
          pProfEntry->fAutLookup = (USHORT)pMapEntry->fAutLookup;
          pProfEntry++;
        } /* endif */
        usCount++;
        pMapEntry++;
      } /* endif */
    }
    else
    {
      //too many entry fields in maptable
      pDimpIda->usDDERc = ERROR_MAPTABLE_TOO_LONG;
      UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
      pDimpIda->fStop = TRUE;     //premature close, release memory
      fOK = FALSE;
      pDimpIda->fKill = FALSE;
      pDimpIda->pSgmlProp = NULL;
    } /* endif */
  } /* endif */
  return( pDimpIda->pSgmlProp );
}

//Includes level data, e.g. hom or target and calls LDBFillstruct which fills
//the entry structure for addition to the dictionary.

static VOID WriteSubTree ( PDIMPIDA pDimpIda)
{
  PSZ_W      * ppData;                  // dict entry data
  USHORT     usI;                     // counter
  SHORT      sLevel = 0;              // actual insert level
  USHORT     usRC;                    // QLDB return code
  USHORT     usResult;                // ulterror return code

  ppData = (PSZ_W *) &(pDimpIda->pEntry[ 0 ]);
  usI = 0;
  while ( usI < pDimpIda->usMapNext )
  {
    ppData = (PSZ_W *) &(pDimpIda->pEntry[ usI * sizeof (PSZ) ]);
    if ( *ppData == NULL )
    {
      *ppData = EMPTY_STRINGW;      // set to empty string
    } /* endif */
    else 
    {
      if ( pDimpIda->usImpMode == DICT_FORMAT_XML_UTF8 ) 
         ResolveXMLEntities( (PSZ_W)*ppData ) ;
    }
    usI++;                           // point to next field
  } /* endwhile */

  switch (pDimpIda->usSubTree)
  {
    case HW_LEVEL:
      sLevel = 1;
      ppData = (PSZ_W *) &(pDimpIda->pEntry[ 0 ]);
      Unicode2ASCIIBuf( *ppData, pDimpIda->szEntryTxt,
                        (USHORT)(UTF16strlenCHAR(*ppData)+1),
                        sizeof(pDimpIda->szEntryTxt) - 1, pDimpIda->ulOEMCP);
      pDimpIda->szEntryTxt[STRINGLEN-1] = EOS;
      break;
    case POS_LEVEL:
      sLevel = 2;
      ppData = (PSZ_W *) &(pDimpIda->pEntry[
                                           pDimpIda->usHomLevel * sizeof(PSZ) ]);
      break;
    case SENSE_LEVEL:
      sLevel = 3;
      ppData = (PSZ_W *) &(pDimpIda->pEntry[
                                           pDimpIda->usSenseLevel * sizeof(PSZ) ]);
      break;
    case TARGET_LEVEL:
      sLevel = 4;
      ppData = (PSZ_W *) &(pDimpIda->pEntry[
                                           pDimpIda->usTargetLevel  * sizeof(PSZ) ]);
      break;
  } /*endswitch */

  if ( sLevel == 1 )
  {
    pDimpIda->phTree = NULL;
    usRC = QLDBCreateTree( pDimpIda->ausNoOfFields, ppData, &pDimpIda->phTree );
  }
  else
  {
    usRC = QLDBAddSubtree( pDimpIda->phTree, sLevel, ppData );
  } /* endif */

  if ( usRC != QLDB_NO_ERROR )
  {
    if ( ISBATCHHWND(pDimpIda->HWNDImpCmd) )
    {
      pDimpIda->usDDERc = usRC;
      UtlErrorHwnd( usRC, MB_OKCANCEL, 0, NULL, QLDB_ERROR,
                    pDimpIda->HWNDImpCmd );
      pDimpIda->fStop = TRUE;     //premature close, release memory
    }
    else
    {
      usResult = UtlError ( usRC, MB_OKCANCEL, 0, NULL, QLDB_ERROR );
      if ( usResult == MBID_CANCEL )
      {
        pDimpIda->fStop = TRUE;     //premature close, release memory
      } /* endif */
    } /* endif */
  } /* endif */
  memset( pDimpIda->pEntry, NULC, pDimpIda->usMapNext * sizeof(PSZ) );
} /* end WriteSubTree */



// Resolve XML entities in segment data
VOID ResolveXMLEntities( PSZ_W pszText )
{
  while ( *pszText )
  {
    if ( *pszText == L'&' )
    {
       if ( ! wcsncmp( pszText, L"&amp;", 5 ) ) {
          memmove( pszText+1, pszText+5, (wcslen(pszText+5)+1)*sizeof(CHAR_W) ) ; 
       } else
       if ( ! wcsncmp( pszText, L"&lt;", 4 ) ) {
          memmove( pszText+1, pszText+4, (wcslen(pszText+4)+1)*sizeof(CHAR_W) ) ; 
          *pszText = L'<' ;
       } else
       if ( ! wcsncmp( pszText, L"&gt;", 4 ) ) {
          memmove( pszText+1, pszText+4, (wcslen(pszText+4)+1)*sizeof(CHAR_W) ) ; 
          *pszText = L'>' ;
       } else
       if ( ! wcsncmp( pszText, L"&apos;", 6 ) ) {
          memmove( pszText+1, pszText+6, (wcslen(pszText+6)+1)*sizeof(CHAR_W) ) ; 
          *pszText = L'\'' ;
       } else
       if ( ! wcsncmp( pszText, L"&quot;", 6 ) ) {
          memmove( pszText+1, pszText+6, (wcslen(pszText+6)+1)*sizeof(CHAR_W) ) ; 
          *pszText = L'\"' ;
       }
    }
    pszText++;
  } /*endwhile */

  *pszText = L'\0';
}



VOID DicBatchImp
(
HWND   hwnd,                  //dictionary list window handle
PVOID  pBatchImp              //command line dict import structure
)
{
  BOOL         fOK = TRUE;                      // error flag
  PDIMPIDA     pDimpIda;                        // Dict import ida
  USHORT       usActionTaken;                   // UtlOpen output
  HFILE        hfFileHandle;                    // DOS file handle
  USHORT       usRc;                            // Return code
  PSZ          pszName;                         // error code string
  PPROPDICTIONARY pDictProp;                    // ptr to dict prop
  HPROP        hDictProp;                       // dict handle
  LONG         lBytesShort;                     // free space indicator
  EQFINFO      ErrorInfo;                       // error msg
  PSZ          pFile;                           // ptr to string
  PDICIMPEXP   pDicImpExp = (PDICIMPEXP) pBatchImp;

  hwnd;
  //allocate dict import ida
  fOK = (UtlAlloc( (PVOID *) &pDimpIda, 0L, (LONG) sizeof(DIMPIDA), NOMSG ));
  if ( !fOK )
  {
    pDicImpExp->DDEReturn.usRc = ERROR_NOT_ENOUGH_MEMORY;
    UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 0,
                  0, EQF_ERROR, pDicImpExp->hwndErrMsg );
  }
  else
  {
    //if file is not fully qualified -- qualify it with current
    //directory of requesting application (DDE Client)
    pFile = UtlGetFnameFromPath( pDicImpExp->chListName );

    if ( !pFile )
    {
      if ( strlen( pDicImpExp->chListName ) < MAX_FILESPEC )
      {
        sprintf( pDimpIda->szDictName, "%s\\%s", pDicImpExp->chCurDir,
                 pDicImpExp->chListName );
      }
      else
      {
        //passed name is not valid
        fOK = FALSE;
        pDicImpExp->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
        pszName = pDicImpExp->chListName;
        UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
      } /* endif */
    }
    else
    {
      if ( strlen( pDicImpExp->chListName ) < sizeof( pDimpIda->szDictName ) )
      {
        strcpy( pDimpIda->szDictName, pDicImpExp->chListName );
      }
      else
      {
        //passed name is not valid
        fOK = FALSE;
        pDicImpExp->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
        pszName = pDicImpExp->chListName;
        UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      //call UtlOpen to open file for existence
      usRc =  UtlOpenHwnd( pDimpIda->szDictName,// full specified input file
                           &hfFileHandle,        // dos file handle
                           &usActionTaken,       // output from UtlOpen
                           0L,                   // file size
                           FILE_NORMAL,                // Normal attribute
                           OPEN_ACTION_OPEN_IF_EXISTS, // Open if exists
                           OPEN_SHARE_DENYWRITE,       // Deny Write access
                           0L,                   // reserved, must be 0
                           FALSE,                // no error handling
                           pDicImpExp->hwndErrMsg );
      if ( !usRc )
      {
        UtlClose( hfFileHandle, FALSE );
      }
      else
      {
        fOK = FALSE;
        pDicImpExp->DDEReturn.usRc = FILE_NOT_EXISTS;
        pszName = pDicImpExp->chListName;
        UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      //open properties of existing dict to check if drive is accessible,
      //copyright and protection flags
      pszName = pDicImpExp->chDictName;   //name of existing asd dict
      PROPNAME ( pDimpIda->szString, pszName );
      hDictProp = OpenProperties( pDimpIda->szString, NULL,
                                  PROP_ACCESS_WRITE, &ErrorInfo);

      if ( hDictProp )
      {
        pDictProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hDictProp );

        //if dictionary is remote check if drive has not been removed
        //via the configure drives option on the file pulldown
        if ( fOK )
        {
          //check if the dict is copyrighted or not
          if ( pDictProp->fCopyRight ) //dict copyrighted
          {
            //dict is copyrighted and merge not permitted
            fOK = FALSE;
            pDicImpExp->DDEReturn.usRc = ERROR_SYSTDICT_COPYRIGHTED;
            pszName = pDicImpExp->chDictName;   //name of existing asd dict
            UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                          &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
          }
          else
          {
            //Is the dictionary protected
            if ( pDictProp->fProtected )
            {
              //if a password was entered check if correct one
              UtlStripBlanks( pDicImpExp->chDicPassWord );
              if ( pDicImpExp->chDicPassWord[0] != EOS )
              {
                if ( !DicGetCheckPassword( pDicImpExp->chDicPassWord,
                                           &(pDictProp->ulPassWord), TRUE ) )
                {
                  fOK = FALSE;
                  pDicImpExp->DDEReturn.usRc = ERRMSG_WRONG_PASSWORD;
                  pszName = pDicImpExp->chDictName; //name of existing asd dict
                  UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                                &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
                } /* endif */
              }
              else
              {
                //dict is protected and no password was entered so merge/import
                //is not permitted
                fOK = FALSE;
                pDicImpExp->DDEReturn.usRc = ERROR_FOLDERDICT_PROTECTED;
                pszName = pDicImpExp->chDictName; //name of existing asd dict
                UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                              &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
              } /* endif */
            } /* endif */
          } /* endif */

          if ( fOK )
          {
            //check if enough space for import
            CHAR  szDrive[2];

            szDrive[0] = pDictProp->szDictPath[0];
            szDrive[1] = EOS;
            pszName = szDrive;
            {
              usRc = UtlCheckSpaceForFile( "", 100, MIN_DICT_SPACE, &pszName,
                                           &lBytesShort, FALSE );
            } /* endif */

            if ( !usRc )
            {
              fOK = FALSE;
              pDicImpExp->DDEReturn.usRc = ERROR_DICT_ACCESS_DENIED_MSG;
              pszName = pDicImpExp->chDictName;   //name of existing asd dict
              UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                            &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
            }
            else
            {
              if ( usRc && lBytesShort )
              {
                fOK = FALSE;
                pDicImpExp->DDEReturn.usRc = ERROR_WRITE_DISK_FULL;
                pszName = pDicImpExp->chDictName;   //name of existing asd dict
                UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                              &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */

        //fully qualified asd file name on szNewDictName in ida
        strcpy( pDimpIda->szNewDictName, pDictProp->szDictPath );

        CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo );
      }
      else
      {
        //dict props of existing dict cannot be opened
        fOK = FALSE;
        pDicImpExp->DDEReturn.usRc = EQFS_FILE_OPEN_FAILED;
        pszName = pDicImpExp->chDictName;   //name of existing asd dict
        UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      //fill asd and sgml paths correctly in import ida
      //fill szPropPath with fully qualified property filename of asd dict
      UtlMakeEQFPath( pDimpIda->szPath, NULC, PROPERTY_PATH, NULL );
      sprintf( pDimpIda->szPropPath, "%s\\%s%s",
               pDimpIda->szPath, pDicImpExp->chDictName, EXT_OF_DICTPROP );

      //set fmerge bool flag to true
      pDimpIda->fMerge = TRUE;

      //set hwnd to show command line interface
      pDimpIda->HWNDImpCmd = pDicImpExp->hwndErrMsg;


      //set dictionary entry merge options in usFlags in ida
      pDimpIda->usFlags |= MERGE_NOUSERPROMPT | MERGE_SOURCE_SGML;

      switch ( pDicImpExp->usOption )
      {
        case DDE_COMBINE :
          pDimpIda->usFlags |= MERGE_ADD;
          break;
        case DDE_IGNORE :
          pDimpIda->usFlags |= MERGE_NOREPLACE;
          break;
        case DDE_REPLACE :
          pDimpIda->usFlags |= MERGE_REPLACE;
          break;
        default :
          break;
      } /* endswitch */

      /*******************************************************************/
      /* Check if the dictionary is locked                               */
      /*******************************************************************/
      if ( fOK )
      {
        HWND         hwndObj;

        strcpy( pDimpIda->IdaHead.szObjName, pDimpIda->szNewDictName );
        pDimpIda->IdaHead.pszObjName =  pDimpIda->IdaHead.szObjName;
        hwndObj = EqfQueryObject( pDimpIda->IdaHead.szObjName, clsDICTIMP, 0 );
        if ( hwndObj != NULLHANDLE )
        {
          fOK = FALSE;
          pDicImpExp->DDEReturn.usRc = ERROR_DICT_LOCKED;
          pszName = pDicImpExp->chDictName;   //name of existing asd dict
          UtlErrorHwnd( pDicImpExp->DDEReturn.usRc, MB_CANCEL, 1,
                        &pszName, EQF_ERROR, pDicImpExp->hwndErrMsg );
        }  /* endif */
      } /* endif */

      //init import processing
      if ( fOK )
      {
        //copy passed structure
        memcpy( &pDimpIda->pDicImpExp, &pDicImpExp,
                sizeof(PDICIMPEXP) );
        memcpy(pDimpIda->sOrgToken, FORMATTABLE, sizeof(FORMATTABLE));

        // create an invisible process window
        strcpy( pDimpIda->IdaHead.szObjName, pDimpIda->szNewDictName );
        pDimpIda->IdaHead.pszObjName =  pDimpIda->IdaHead.szObjName;
        fOK = CreateProcessWindow2( pDimpIda->IdaHead.pszObjName,
                                    DicImportCallBack, pDimpIda, FALSE );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !fOK )
  {
    //notify handler that something went wrong
    EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_NEXTSTEP,
                     MP1FROMSHORT( TASK_END ), MP2FROMP(pDicImpExp) );
  } /* endif */
} /* DicCmdImp */

#ifdef FUNCCALLIF
USHORT DicFuncImportDict
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszInFile,               // fully qualified name of input file
PSZ         pszDictName,             // name of dictionary
PSZ         pszPassword,             // password of dictionary
LONG        lOptions                 // dictionary import options
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new import or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new import
    usRC = DicFuncPrepImport( pData, pszInFile, pszDictName,
                              pszPassword, lOptions );
  }
  else
  {
    // continue current import process
    usRC = DicFuncImportProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function DicFuncImportDict */

// Prepare the import of a dictionary in function call mode
USHORT DicFuncPrepImport
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszInFile,               // fully qualified name of input file
PSZ         pszDictName,             // name of dictionary
PSZ         pszPassword,             // password of dictionary
LONG        lOptions                 // dictionary import options
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fOK = TRUE;              // error flag
  PDIMPIDA    pDimpIda;                // Dict import ida
  USHORT      usRc;                    // Return code
  LONG        lBytesShort;             // free space indicator
  EQFINFO     ErrorInfo;               // error msg

  // set continue flag
  pData->fComplete = FALSE;

  // allocate dict import ida
  fOK = (UtlAlloc( (PVOID *) &pDimpIda, 0L, (LONG) sizeof(DIMPIDA), NOMSG ));
  if ( !fOK )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, 0, EQF_ERROR, HWND_FUNCIF );
  }
  else
  {
    pData->pvDicImportIda = (PVOID)pDimpIda;
  } /* endif */

  // check input file name
  if ( fOK )
  {
    if ( (pszInFile == NULL) || (*pszInFile == EOS) )
    {
      UtlErrorHwnd( FUNC_MANDINFILE, MB_CANCEL, 0, 0, EQF_ERROR, HWND_FUNCIF );
      usRC = FUNC_MANDINFILE;
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( strlen( pszInFile ) < sizeof( pDimpIda->szDictName ) )
    {
      strcpy( pDimpIda->szDictName, pszInFile );
    }
    else
    {
      //passed name is not valid
      PSZ         pszName;                 // error code string
      fOK = FALSE;
      usRC = UTL_PARAM_TOO_LONG;
      pszName = pszInFile;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszName, EQF_ERROR, HWND_FUNCIF );
    } /* endif */

    if ( fOK )
    {
      //call UtlOpen to open file for existence
      USHORT      usActionTaken;           // UtlOpen output
      HFILE       hfFileHandle;            // DOS file handle
      usRc =  UtlOpenHwnd( pDimpIda->szDictName,// full specified input file
                           &hfFileHandle,        // dos file handle
                           &usActionTaken,       // output from UtlOpen
                           0L,                   // file size
                           FILE_NORMAL,                // Normal attribute
                           OPEN_ACTION_OPEN_IF_EXISTS, // Open if exists
                           OPEN_SHARE_DENYWRITE,       // Deny Write access
                           0L,                   // reserved, must be 0
                           FALSE,                // no error handling
                           HWND_FUNCIF );
      if ( !usRc )
      {
        UtlClose( hfFileHandle, FALSE );
      }
      else
      {
        PSZ         pszName;                 // error code string
        fOK = FALSE;
        usRC = FILE_NOT_EXISTS;
        pszName = pszInFile;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszName, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */

    // check dictionary name
    if ( fOK )
    {
      if ( (pszDictName == NULL) || (*pszDictName == EOS) )
      {
        usRC = NO_NEW_DICTIONARY_SELECTED;
        UtlErrorHwnd( NO_NEW_DICTIONARY_SELECTED, MB_CANCEL, 0, 0, EQF_ERROR, HWND_FUNCIF );
        fOK = FALSE;
      } /* endif */
    } /* endif */

    // process name of dictionary being imported
    if ( fOK )
    {
      BOOL fIsNew = FALSE;         // is-new flag
      ANSITOOEM( pszDictName );
      strcpy( pDimpIda->szLongName, pszDictName );
      ObjLongToShortName( pszDictName, pDimpIda->szShortName, DICT_OBJECT, &fIsNew );
      OEMTOANSI( pszDictName );
     } /* endif */

    if ( fOK )
    {
      //open properties of existing dict to check if drive is accessible,
      //copyright and protection flags
      PPROPDICTIONARY pDictProp;           // ptr to dict prop
      HPROP       hDictProp;               // dict handle


      PSZ pszName = pszDictName;           // name of existing asd dict
      PROPNAME ( pDimpIda->szString, pDimpIda->szShortName );
      hDictProp = OpenProperties( pDimpIda->szString, NULL,
                                  PROP_ACCESS_WRITE, &ErrorInfo);

      if ( hDictProp )
      {
        pDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hDictProp );

        //set fmerge bool flag to true
        pDimpIda->fMerge = TRUE;

        //if dictionary is remote check if drive has not been removed
        //via the configure drives option on the file pulldown

        if ( fOK )
        {
          //check if the dict is copyrighted or not
          if ( pDictProp->fCopyRight ) //dict copyrighted
          {
            //dict is copyrighted and merge not permitted
            fOK = FALSE;
            usRC = ERROR_SYSTDICT_COPYRIGHTED;
            pszName = pszDictName;   //name of existing asd dict
            UtlErrorHwnd( usRC, MB_CANCEL, 1,
                          &pszName, EQF_ERROR, HWND_FUNCIF );
          }
          else
          {
            //Is the dictionary protected
            if ( pDictProp->fProtected )
            {
              //if a password was entered check if correct one
              if ( (pszPassword != NULL) && (pszPassword[0] != EOS) )
              {
                if ( !DicGetCheckPassword( pszPassword,
                                           &(pDictProp->ulPassWord), TRUE ) )
                {
                  fOK = FALSE;
                  usRC = ERRMSG_WRONG_PASSWORD;
                  pszName = pszDictName; //name of existing asd dict
                  UtlErrorHwnd( usRC, MB_CANCEL, 1,
                                &pszName, EQF_ERROR, HWND_FUNCIF );
                } /* endif */
              }
              else
              {
                //dict is protected and no password was entered so merge/import
                //is not permitted
                fOK = FALSE;
                usRC = ERROR_FOLDERDICT_PROTECTED;
                pszName = pszDictName; //name of existing asd dict
                UtlErrorHwnd( usRC, MB_CANCEL, 1,
                              &pszName, EQF_ERROR, HWND_FUNCIF );
              } /* endif */
            } /* endif */
          } /* endif */

          if ( fOK )
          {
            //check if enough space for import
            CHAR  szDrive[2];

            szDrive[0] = pDictProp->szDictPath[0];
            szDrive[1] = EOS;
            pszName = szDrive;
            {
              usRc = UtlCheckSpaceForFile( "", 100, MIN_DICT_SPACE, &pszName,
                                           &lBytesShort, FALSE );
            } /* endif */

            if ( !usRc )
            {
              fOK = FALSE;
              usRC = ERROR_DICT_ACCESS_DENIED_MSG;
              pszName = pszDictName;   //name of existing asd dict
              UtlErrorHwnd( usRC, MB_CANCEL, 1,
                            &pszName, EQF_ERROR, HWND_FUNCIF );
            }
            else
            {
              if ( usRc && lBytesShort )
              {
                fOK = FALSE;
                usRC = ERROR_WRITE_DISK_FULL;
                pszName = pszDictName;   //name of existing asd dict
                UtlErrorHwnd( usRC, MB_CANCEL, 1,
                              &pszName, EQF_ERROR, HWND_FUNCIF );
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */

        //fully qualified asd file name on szNewDictName in ida
        strcpy( pDimpIda->szNewDictName, pDictProp->szDictPath );

        CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo );
      }
      else
      {
        // GQ: continue if dict does not exist, will be created later on...
        UtlMakeEQFPath( pDimpIda->szNewDictName, NULC, PROPERTY_PATH, NULL );
        strcat( pDimpIda->szNewDictName, BACKSLASH_STR );
        strcat( pDimpIda->szNewDictName, pDimpIda->szShortName );
        strcat( pDimpIda->szNewDictName, EXT_OF_DICTPROP );
          

        // but check if passed name is a valid one
        if ( !UtlCheckLongName( pszDictName ) )
        {
          UtlErrorHwnd(  ERROR_INV_LONGNAME, MB_CANCEL, 1, &pszDictName, EQF_ERROR, HWND_FUNCIF );
          usRC = ERROR_MEM_NAME_INVALID;
          fOK = FALSE;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      //fill asd and sgml paths correctly in import ida
      //fill szPropPath with fully qualified property filename of asd dict
      UtlMakeEQFPath( pDimpIda->szPath, NULC, PROPERTY_PATH, NULL );
      sprintf( pDimpIda->szPropPath, "%s\\%s%s",
               pDimpIda->szPath, pDimpIda->szShortName, EXT_OF_DICTPROP );

      //set hwnd to show command line interface
      pDimpIda->HWNDImpCmd = HWND_FUNCIF;

      // set dictionary import format (default = Unicode)
      if ( lOptions & ASCII_OPT )
      {
        pDimpIda->usImpMode = DICT_FORMAT_SGML_ASCII;
      }
      else if ( lOptions & ANSI_OPT )
      {
        pDimpIda->usImpMode = DICT_FORMAT_SGML_ANSI;
      }
      else if ( lOptions & DXT_UTF8_OPT )
      {
        pDimpIda->usImpMode = DICT_FORMAT_XML_UTF8;
      }
      else
      {
        pDimpIda->usImpMode = DICT_FORMAT_SGML_UNICODE;
      } /* endif */

      //set dictionary entry merge options in usFlags in ida
      pDimpIda->usFlags |= MERGE_NOUSERPROMPT | MERGE_SOURCE_SGML;

      if ( lOptions & COMBINE_OPT )
      {
        pDimpIda->usFlags |= MERGE_ADD;
      }
      else if ( lOptions & REPLACE_OPT )
      {
        pDimpIda->usFlags |= MERGE_REPLACE;
      }
      else if ( lOptions & IGNORE_OPT )
      {
        pDimpIda->usFlags |= MERGE_NOREPLACE;
      } /* endif */

      // check if the dictionary is locked
      if ( fOK )
      {
        HWND         hwndObj;

        strcpy( pDimpIda->IdaHead.szObjName, pDimpIda->szNewDictName );
        pDimpIda->IdaHead.pszObjName =  pDimpIda->IdaHead.szObjName;
        hwndObj = EqfQueryObject( pDimpIda->IdaHead.szObjName, clsDICTIMP, 0 );
        if ( hwndObj != NULLHANDLE )
        {
          PSZ pszName = pszDictName;   //name of existing asd dict
          fOK = FALSE;
          usRC = ERROR_DICT_LOCKED;
          UtlErrorHwnd( usRC, MB_CANCEL, 1,
                        &pszName, EQF_ERROR, HWND_FUNCIF );
        }  /* endif */
      } /* endif */

      // init import processing
      if ( fOK )
      {
        memcpy(pDimpIda->sOrgToken, FORMATTABLE, sizeof(FORMATTABLE));
        strcpy( pDimpIda->IdaHead.szObjName, pDimpIda->szNewDictName );
        pDimpIda->IdaHead.pszObjName =  pDimpIda->IdaHead.szObjName;
        pData->DicImportNextTask = FCTPHASE_INIT;
        pDimpIda->ulOEMCP = GetLangOEMCP(NULL);  // use system preferences language
        pDimpIda->ulAnsiCP = GetLangAnsiCP(NULL);  // use system preferences lang.
      } /* endif */
    } /* endif */
  } /* endif */

  // cleanup in case of errors
  if ( !fOK )
  {
    if ( pDimpIda ) UtlAlloc( (PVOID *)&pDimpIda, 0L, 0L, NOMSG );
    pData->fComplete = TRUE;
  } /* endif */

  return( usRC );
} /* end of function DicFuncPrepImport */

// Import a dictionary in function call mode
USHORT DicFuncImportProcess
(
PFCTDATA    pData                    // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  PDIMPIDA pDimpIda = (PDIMPIDA)pData->pvDicImportIda;

  switch ( pData->DicImportNextTask )
  {
    case FCTPHASE_INIT:
      if ( pDimpIda->fImpMerge )
      {
        FolderImportInit( HWND_FUNCIF, pDimpIda );
      }
      else
      {
        DimpInit( HWND_FUNCIF, pDimpIda );
      } /* endif */
      if ( pDimpIda->fNotOk )
      {
        usRC = pDimpIda->usDDERc;
        DimpCleanUp( pDimpIda );
        pData->fComplete = TRUE;
      }
      else
      {
        pData->DicImportNextTask = FCTPHASE_PROCESS;

      } /* endif */
      break;
    case FCTPHASE_PROCESS:
      if ( pDimpIda->fImpMerge )
      {
        FolderImportWork( HWND_FUNCIF, pDimpIda );
      }
      else
      {
        DimpWork( HWND_FUNCIF, pDimpIda );
      } /* endif */
      if ( pDimpIda->fNotOk )
      {
        usRC = pDimpIda->usDDERc;
        DimpCleanUp( pDimpIda );
        pData->fComplete = TRUE;
      }
      else if ( pDimpIda->fStop )
      {
        pData->DicImportNextTask = FCTPHASE_CLEANUP;
      } /* endif */
      break;
    case FCTPHASE_CLEANUP :
      usRC = pDimpIda->usDDERc;
      DimpCleanUp( pDimpIda );
      pData->fComplete = TRUE;
      break;
  } /* endswitch */

  return( usRC );
} /* end of function DicFuncImportProcess */

#else
USHORT DicFuncImportDict( USHORT );
USHORT DicFuncImportDict( USHORT usDummy ){ return( 0 );}
#endif


static USHORT
HandleCodePageToken
(
	PDIMPIDA      pDimpIda,
	PTOKENENTRY * ppToken,         // ptr to token table
	PSZ_W       * ppDummyW,
	PBOOL        pfReadAgain,
	PBOOL        pfMsgDisplayed
)
{
  PSZ_W       pTmpW = NULL;
  ULONG       ulCP = 0;
  USHORT      usImpMode = 0;
  BOOL        fOK = TRUE;
  PTOKENENTRY pToken;                          // pointer to token
  PSZ_W       pStringW = NULL;
  PSZ_W       pMsgError[2];                // ptr to error array
  USHORT      usRc = NO_ERROR;
  CHAR_W      chTmpW[200];
  USHORT      usMaxLen = 200;
  CHAR_W      chCPW[20];

  pToken = (*ppToken);                      // check for text token
  pStringW = NULL;                           // string not set yet

  memset(&chTmpW[0], 0, usMaxLen * sizeof(CHAR_W));
  memset(&chCPW[0], 0, 20*sizeof(CHAR_W));

  fOK = CheckForEnd( pDimpIda, pToken, ECODEPAGE_TOKEN );
  if ( fOK )
  {
    pToken++;                      // check for text token
    if (pToken->sTokenid == TEXT_TOKEN)
    {
        if (pToken->usLength <= usMaxLen - 1)
        {
			usMaxLen = pToken->usLength;
	    }
		memcpy( &chTmpW[0], pToken->pDataStringW, usMaxLen * sizeof(CHAR_W));
		chTmpW[usMaxLen] = EOS;
		pStringW = &chTmpW[0];
    }

	if ( fOK && pStringW )
	{
	   while ( *pStringW == ' ' ) *pStringW++;

	   if (memicmp(  pStringW, L"ANSI=", 5 * sizeof(CHAR_W) ) == 0)
	   {
		  usImpMode = DICT_FORMAT_SGML_ANSI;
		  pTmpW = pStringW + 5;
		  ulCP = _wtoi(pTmpW );
		  if ((usImpMode != pDimpIda->usImpMode) )
		  {
            if (pDimpIda->usImpMode == DICT_FORMAT_SGML_UNICODE)
            {
              pDimpIda->usDDERc = NO_VALID_ASCIIANSIFORMAT;
              pMsgError[0] = L"ANSI";
              usRc = UtlErrorHwndW( pDimpIda->usDDERc, MB_CANCEL, 1, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
              *pfMsgDisplayed = TRUE;
              fOK = FALSE;
            }
            else
			if (pDimpIda->usImpMode == DICT_FORMAT_XML_UTF8)
		    {
	  	      pDimpIda->usDDERc = NO_VALID_ASCIIANSIFORMAT;
		      pMsgError[0] = L"ANSI";
		      usRc = UtlErrorHwndW( pDimpIda->usDDERc, MB_CANCEL, 1, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
   			  *pfMsgDisplayed = TRUE;
		      fOK = FALSE;
		    }
		    else
		    { // pDimpIda->usImpMode == DICT_FORMAT_SGML_ASCII
		      if ( (pDimpIda->HWNDImpCmd == HWND_FUNCIF) || ISBATCHHWND(pDimpIda->HWNDImpCmd) )
			    {
			        usRc = MBID_OK;
			    }
			    else
          {
				  pDimpIda->usDDERc = WARNING_DIMP_FORMAT;
				  pMsgError[0] = pStringW;
				  pMsgError[1] = L"SGML ASCII";

				  usRc = UtlErrorHwndW( pDimpIda->usDDERc, MB_OKCANCEL | MB_DEFBUTTON1,
									2, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
				  *pfMsgDisplayed = TRUE;
			  }

			  fOK = ( usRc == MBID_OK );
			  if (usRc == MBID_OK)
			  {
				  *pfReadAgain = TRUE;
				  usRc = NO_ERROR;
			  }
		    }
		  }
		  if ( fOK && ulCP != pDimpIda->ulAnsiCP)
		  {
    	  // make consistency check whether the cp is correct!
            CHAR_W c_W[10];
			PSZ   pTst = "abcdefgh";
			LONG  lBytesLeft = 0L;
			LONG  lRc = 0;
			ULONG ulOutPut = 0;
			PSZ_W pTstW = &c_W[0];

	        ulOutPut = UtlDirectAnsi2UnicodeBuf( (PSZ) pTst, (PSZ_W) pTstW, 8,
	                                ulCP, FALSE, &lRc, &lBytesLeft);
            if ((ulOutPut == 0) || (ulCP == 0))
            {// given ulCP is URX, since test conversion returns an empty string!
			  pDimpIda->usDDERc = NO_VALID_ASCIIANSICP;
              pMsgError[0] = pTmpW;
              usRc = UtlErrorHwndW( pDimpIda->usDDERc,  MB_CANCEL,
			      		  1, pMsgError, EQF_ERROR, pDimpIda->HWNDImpCmd, TRUE );
			  fOK = FALSE;
			  *pfMsgDisplayed = TRUE;
		    }
		    else
		    {
			  if ( (pDimpIda->HWNDImpCmd == HWND_FUNCIF) || ISBATCHHWND(pDimpIda->HWNDImpCmd) )
			  {
			     usRc = MBID_OK;
			  }
			  else
              {
			     pDimpIda->usDDERc = WARNING_DIMP_CODEPAGE;
			     pMsgError[0] = pTmpW;
			     pMsgError[1] = _itow( pDimpIda->ulOEMCP, chCPW, 10 );
			     usRc = UtlErrorHwndW( pDimpIda->usDDERc,  MB_OKCANCEL | MB_DEFBUTTON1,
			   			  2, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
			     *pfMsgDisplayed = TRUE;
			  }
			  fOK = (usRc == MBID_OK);
			  if (usRc == MBID_OK)
			  {
			    *pfReadAgain = TRUE;
			    usRc = NO_ERROR;
			  }
		    }
		  }
		  if (fOK)
		  {
			 pDimpIda->ulAnsiCP = ulCP;
			 pDimpIda->usImpMode = usImpMode;
		  }
		  else
		  {
			 pDimpIda->fStop = TRUE;     //premature close, release memory
			 pDimpIda->fNotOk = TRUE;    //don't issue complete msg
		  }
	   }
	   else if (memicmp( pStringW, L"ASCII=", 6 * sizeof(CHAR_W) ) == 0)
	   {
		   usImpMode = DICT_FORMAT_SGML_ASCII;
		   pTmpW = pStringW + 6;
		   ulCP = (ULONG)_wtoi(pTmpW);
		   if((usImpMode != pDimpIda->usImpMode) )
		   {
            if (pDimpIda->usImpMode == DICT_FORMAT_SGML_UNICODE)
            {
                  pDimpIda->usDDERc = NO_VALID_ASCIIANSIFORMAT;
                  pMsgError[0] = L"ASCII";
                  usRc = UtlErrorHwndW( pDimpIda->usDDERc, MB_CANCEL,
                                  1, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
                  fOK = FALSE;
                  *pfMsgDisplayed = TRUE;
            }
            else
		    if (pDimpIda->usImpMode == DICT_FORMAT_XML_UTF8)
		    {
				  pDimpIda->usDDERc = NO_VALID_ASCIIANSIFORMAT;
				  pMsgError[0] = L"ASCII";
				  usRc = UtlErrorHwndW( pDimpIda->usDDERc, MB_CANCEL,
								  1, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
				  fOK = FALSE;
				  *pfMsgDisplayed = TRUE;
		    }
		    else
		    {
			    if ( (pDimpIda->HWNDImpCmd == HWND_FUNCIF) || ISBATCHHWND(pDimpIda->HWNDImpCmd) )
			    {
				  usRc = MBID_OK;
			    }
			    else
			    {
				  pDimpIda->usDDERc = WARNING_DIMP_FORMAT;
				  pMsgError[0] = pStringW;
				  pMsgError[1] = L"SGML ANSI";

				  usRc = UtlErrorHwndW( pDimpIda->usDDERc, MB_OKCANCEL | MB_DEFBUTTON1,
									  2, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
				  *pfMsgDisplayed = TRUE;
			    }

			  fOK = ( usRc == MBID_OK );
			  if (usRc == MBID_OK)
			  {
			    *pfReadAgain = TRUE;
			    usRc = NO_ERROR;
			  }

		   }
	     }
		 if ( fOK && ulCP != pDimpIda->ulOEMCP)
		 {// make consistency check whether the cp is correct!
            CHAR_W c_W[10];
			PSZ   pTst = "abcdefgh";
			ULONG ulOutPut = 0;
			PSZ_W pTstW = &c_W[0];

	        ulOutPut = ASCII2UnicodeBuf( (PSZ) pTst, (PSZ_W) pTstW, 8, ulCP);

            if ((ulOutPut == 0) || (ulCP == 0))
            {// given ulCP is URX, since test conversion returns an empty string!
				pDimpIda->usDDERc = NO_VALID_ASCIIANSICP;
                pMsgError[0] = pTmpW;
                usRc = UtlErrorHwndW( pDimpIda->usDDERc,  MB_CANCEL,
						  1, pMsgError, EQF_ERROR, pDimpIda->HWNDImpCmd, TRUE );
			    fOK = FALSE;
			    *pfMsgDisplayed = TRUE;
		    }
		    else
		    {
				if ( (pDimpIda->HWNDImpCmd == HWND_FUNCIF) || ISBATCHHWND(pDimpIda->HWNDImpCmd) )
				{
				    usRc = MBID_OK;
				}
				else
                {
					pDimpIda->usDDERc = WARNING_DIMP_CODEPAGE;
					pMsgError[0] = pTmpW;
					pMsgError[1] = _itow(pDimpIda->ulOEMCP, &chCPW[0], 10);
					usRc = UtlErrorHwndW( pDimpIda->usDDERc,  MB_OKCANCEL | MB_DEFBUTTON1,
								  2, pMsgError, EQF_QUERY, pDimpIda->HWNDImpCmd, TRUE );
					*pfMsgDisplayed = TRUE;
			     }
			     fOK = (usRc == MBID_OK);
    		 	if (usRc == MBID_OK)
				{
				  *pfReadAgain = TRUE;
				  usRc = NO_ERROR;
				}
		    }
		 }
		 if (fOK)
		 {
			 pDimpIda->ulOEMCP = ulCP;
			 pDimpIda->usImpMode = usImpMode;
		 }
		 else
		 {
			 pDimpIda->fStop = TRUE;     //premature close, release memory
			 pDimpIda->fNotOk = TRUE;    //don't issue complete msg
		 }
	   }
	   else
	   if (memicmp( pStringW, L"UTF16", 5 * sizeof(CHAR_W) ) == 0)
	   {
		    usImpMode = DICT_FORMAT_SGML_UNICODE;

		    if ((usImpMode != pDimpIda->usImpMode) )
		    {
		      pDimpIda->usDDERc = ERROR_DIMP_UTF16WRONG;
		      usRc = UtlErrorHwndW( pDimpIda->usDDERc, MB_CANCEL,
								    0, NULL, EQF_ERROR, pDimpIda->HWNDImpCmd, TRUE );
		      fOK = 0;
		      *pfMsgDisplayed = TRUE;
		      pDimpIda->fStop = TRUE;     //premature close, release memory
		      pDimpIda->fNotOk = TRUE;    //don't issue complete msg
		    }
	   }
	   else
	   if (memicmp( pStringW, L"UTF8", 4 * sizeof(CHAR_W) ) == 0)
	   {
		    usImpMode = DICT_FORMAT_XML_UTF8;

		    if ((usImpMode != pDimpIda->usImpMode) )
		    {
		      pDimpIda->usDDERc = ERROR_DIMP_UTF8WRONG;   
		      usRc = UtlErrorHwndW( pDimpIda->usDDERc, MB_CANCEL,
								    0, NULL, EQF_ERROR, pDimpIda->HWNDImpCmd, TRUE );
		      fOK = 0;
		      *pfMsgDisplayed = TRUE;
		      pDimpIda->fStop = TRUE;     //premature close, release memory
		      pDimpIda->fNotOk = TRUE;    //don't issue complete msg
		    }
	   }
	   else
	   {  // error - this should not occur...
		    pDimpIda->usDDERc = ERROR_DIMP_INVHEADER;
		    usRc = UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
					      0, NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
		    pDimpIda->fStop = TRUE;     //premature close, release memory
		    pDimpIda->fNotOk = TRUE;    //don't issue complete msg
		    *pfMsgDisplayed = TRUE;
	   }
	}
  }
  else
  {
	pDimpIda->usDDERc = ERROR_DIMP_INVHEADER;
	usRc = UtlErrorHwnd( pDimpIda->usDDERc, MB_CANCEL,
				  0, NULL, EQF_ERROR, pDimpIda->HWNDImpCmd );
	pDimpIda->fStop = TRUE;     //premature close, release memory
	pDimpIda->fNotOk = TRUE;    //don't issue complete msg
	*pfMsgDisplayed = TRUE;
  } /* endif */
  *ppDummyW = pStringW;
  return(usRc);
}


static USHORT
DimpReadtoUnicode
(
	PDIMPIDA  pDimpIda,
	PULONG    pulFreeConvBuffer,
	PULONG    pulFilled,
	PULONG    pulFreeBuffer,
	PBOOL     pfMsgDisplayed
)
{
	ULONG   ulFreeConvBuffer = *pulFreeConvBuffer;
	ULONG   ulFilled = *pulFilled;
	ULONG   ulFreeBuffer = *pulFreeBuffer;
	BOOL    fMsgDisplayed = *pfMsgDisplayed;
	USHORT  usRc = NO_ERROR;
	ULONG   ulConverted = 0;

    switch ( pDimpIda->usImpMode )
    {
      case DICT_FORMAT_SGML_ANSI :
        {
          usRc = UtlReadL( pDimpIda->hFile, pDimpIda->pConvBuffer, ulFreeConvBuffer,
                           &(pDimpIda->ulUsedInConv), FALSE );
          if ( usRc == NO_ERROR )
          { // ensure that there are no DBCS characters splitted at the end of
            // the data just read into memory
            LONG lRc = 0;
            LONG  lBytesLeft = 0;
            if ( pDimpIda->ulUsedInConv != 0 )
            {
                *(pDimpIda->pConvBuffer + pDimpIda->ulUsedInConv) = '\0';

				ulConverted = UtlDirectAnsi2UnicodeBuf((PSZ)(pDimpIda->pConvBuffer),
                                   (PSZ_W)(pDimpIda->pInputFileBlock + ulFilled),
                                    pDimpIda->ulUsedInConv,
									pDimpIda->ulAnsiCP, TRUE, &lRc, &lBytesLeft );
                usRc = (USHORT)lRc;
				if (lBytesLeft )
				{
				  // undo the read of the bytes left
                  // reposition file pointer
                  ULONG ulCurrentPos = 0;
                  UtlChgFilePtr( pDimpIda->hFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
                  ulCurrentPos -= lBytesLeft;
                  UtlChgFilePtr( pDimpIda->hFile, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);

                  // adjust counters
                  pDimpIda->ulUsedInConv -= lBytesLeft;
                  ulFreeConvBuffer -= lBytesLeft;

                  // return is number of wide chars written to pszBuffer
                  *(pDimpIda->pConvBuffer + pDimpIda->ulUsedInConv) = '\0';
			    } /* endif */
            } /* endif */
            pDimpIda->ulUsed = (ulFilled + (ulConverted * sizeof(CHAR_W))); // # of bytes in InputFileBlock
          } /* endif */
          if ( !usRc ) pDimpIda->fAll = (pDimpIda->ulUsedInConv != ulFreeConvBuffer );
        }
        break;

      case DICT_FORMAT_SGML_UNICODE :
        {
          ULONG    ulBytesRead = 0;
          usRc = UtlReadL( pDimpIda->hFile,
                           pDimpIda->pInputFileBlock + ulFilled,
                            ulFreeBuffer,
                            &ulBytesRead,
                           /* &(pDimpIda->ulUsed),    */   // # of bytes read in
                            FALSE );
          if ( !usRc ) pDimpIda->fAll = (ulBytesRead != ulFreeBuffer );
           // if first read skip any unicode text prefix
          if ( (usRc == NO_ERROR) && pDimpIda->fFirstRead )
          {  // 1st part not yet read
            PSZ pszPrefix = UNICODEFILEPREFIX;
            int iLen = strlen(pszPrefix);
            if ( memcmp( pDimpIda->pInputFileBlock, pszPrefix, iLen ) == 0 )
            {
              // skip prefix ...
              // usFilled must be 0 here!!
              ulBytesRead -= iLen;
              memmove( pDimpIda->pInputFileBlock,
                       pDimpIda->pInputFileBlock + iLen,
                        ulBytesRead );
            }
            else
            {
               PSZ pszImportFile = pDimpIda->szDictName;
               fMsgDisplayed = TRUE;
               usRc = UtlErrorHwnd( NO_VALID_UNICODEFORMAT, MB_YESNO, 1, &pszImportFile,
                       EQF_WARNING, pDimpIda->HWNDImpCmd );

               if ( usRc == MBID_NO )
               {
                 usRc = ERROR_INVALID_DATA;
               }
               else
               {
                 usRc = NO_ERROR;
               } /* endif */
            } /* endif */
          } /* endif */

          pDimpIda->ulUsed = (ulFilled) + ulBytesRead;
        }
        break;

      case DICT_FORMAT_XML_UTF8 :
        {
          usRc = UtlReadL( pDimpIda->hFile, pDimpIda->pConvBuffer, ulFreeConvBuffer,
                           &(pDimpIda->ulUsedInConv), FALSE );
          if ( usRc == NO_ERROR )
          { // ensure that there are no UTF-8 characters split at the end of
            // the data just read into memory
            LONG  lRc = 0;
            LONG  lBytesLeft = 0;
            LONG  lBytesRemoved = 0;

            if ( pDimpIda->ulUsedInConv != 0 )
            {
              int i;
              for( i=pDimpIda->ulUsedInConv-1 ; 
                   i>0 && *(pDimpIda->pConvBuffer+i)!= '>' ; --i ) ;
              *(pDimpIda->pConvBuffer + i + 1) = '\0';
              lBytesLeft = pDimpIda->ulUsedInConv - i ;
              if (lBytesLeft )
              {
                // undo the read of the bytes left
                // reposition file pointer
                ULONG ulCurrentPos = 0;
                UtlChgFilePtr( pDimpIda->hFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
                ulCurrentPos -= lBytesLeft;
                UtlChgFilePtr( pDimpIda->hFile, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);
              
                // adjust counters
                pDimpIda->ulUsedInConv -= lBytesLeft;
                ulFreeConvBuffer -= lBytesLeft;
              
                // return is number of wide chars written to pszBuffer
                *(pDimpIda->pConvBuffer + pDimpIda->ulUsedInConv) = '\0';

                if ( ! strncmp((char*)(pDimpIda->pConvBuffer), "\xEF\xBB\xBF", 3 ) ) {
                   PSZ pTemp ;
                   pTemp = (char*)(pDimpIda->pConvBuffer);
                   memmove( pTemp, pTemp+3, strlen(pTemp+3)+1 ) ;
                   lBytesRemoved += 3 ;
                }
                if ( ! strncmp( (char*)(pDimpIda->pConvBuffer), "<?xml", 5 ) ) {
                   PSZ pTemp ;
                   pTemp = strstr( (char*)(pDimpIda->pConvBuffer), "?>" ) ;
                   if ( pTemp ) {
                      for( pTemp+=2 ; *pTemp && isspace(*pTemp) ; ++pTemp ) ;
                      memmove( (char*)(pDimpIda->pConvBuffer), pTemp, strlen(pTemp)+1 ) ; 
                      lBytesRemoved += pTemp - (char*)(pDimpIda->pConvBuffer) ;
                   }
                }
                pDimpIda->ulUsedInConv -= lBytesRemoved;
                ulFreeConvBuffer -= lBytesRemoved;
              } /* endif */
              (ULONG)ulConverted = UTF82UnicodeBufEx( (char*)pDimpIda->pConvBuffer,
                                               (wchar_t*)(pDimpIda->pInputFileBlock + ulFilled),
                                               (long)pDimpIda->ulUsedInConv,
                                               (int)FALSE, &lRc, &lBytesLeft ) ;
              *(pDimpIda->pConvBuffer + pDimpIda->ulUsedInConv)= EOS;
              if ( lRc != NO_ERROR ) {
                 usRc = ERROR_DIMP_UTF8WRONG ; 
              }

            }
            pDimpIda->ulUsed = (ulFilled + (ulConverted * sizeof(CHAR_W))); // # of bytes in InputFileBlock
          } /* endif */

          if ( !usRc ) pDimpIda->fAll = (pDimpIda->ulUsedInConv != ulFreeConvBuffer );
        }
        break;

      case DICT_FORMAT_SGML_ASCII:
      default:
        {
            usRc = UtlReadL( pDimpIda->hFile, pDimpIda->pConvBuffer, ulFreeConvBuffer,
                            &(pDimpIda->ulUsedInConv), FALSE );
            if ( usRc == NO_ERROR )
            {
              // ensure that there are no DBCS characters splitted at the end of
              // the data just read into memory
              if ( pDimpIda->ulUsedInConv != 0 )
              {
                BYTE bTest = (BYTE)pDimpIda->pConvBuffer[pDimpIda->ulUsedInConv-1];
                if ( IsDBCSLeadByteEx( pDimpIda->ulOEMCP, bTest ) )
                {
                  int iTry = 5;
                  ULONG ulUnicodeChars = 0;
                  PSZ_W pszBuffer = (PSZ_W)(pDimpIda->pInputFileBlock + ulFilled);

                  while (  iTry > 0  && pDimpIda->ulUsedInConv)
                  {
                    // undo the last character read...
                    // reposition file pointer
                    ULONG ulCurrentPos = 0;
                    UtlChgFilePtr( pDimpIda->hFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
                    ulCurrentPos--;
                    UtlChgFilePtr( pDimpIda->hFile, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);

                    // adjust counters
                    pDimpIda->ulUsedInConv--;
                    ulFreeConvBuffer--;

                    // return is number of wide chars written to pszBuffer
                    *(pDimpIda->pConvBuffer + pDimpIda->ulUsedInConv) = '\0';
                    ulUnicodeChars = ASCII2UnicodeBuf( (PSZ)(pDimpIda->pConvBuffer),
                                                       pszBuffer, pDimpIda->ulUsedInConv,
                                                       pDimpIda->ulOEMCP);
                    if ( ulUnicodeChars && pszBuffer[ulUnicodeChars-1]   == 0)
                    {
                      // try again, we probably found a 2nd DBCSbyte which might be in the range of a 1st byte, too
                      iTry--;
                    }
                    else
                    {
                      // leave loop and set ulBytesRead
                      ulConverted = ulUnicodeChars;
                      iTry = 0;
                    } /* endif */
                  } /* endwhile */

                  if ( iTry == 0 && ulConverted && pszBuffer[ulUnicodeChars-1] == 0)
                  {
                      // something went totally wrong
                      ulConverted = ulUnicodeChars-1;
                  }
                }
                else
                {
                  // non DBCS case --- no problem at all
                  *(pDimpIda->pConvBuffer + pDimpIda->ulUsedInConv) = '\0';
                  ulConverted = ASCII2UnicodeBuf( (PSZ)(pDimpIda->pConvBuffer),
                                     (PSZ_W)(pDimpIda->pInputFileBlock + ulFilled),
                                      pDimpIda->ulUsedInConv, pDimpIda->ulOEMCP );
                } /* endif */
              } /* endif */
              pDimpIda->ulUsed = ulFilled + (ulConverted * sizeof(CHAR_W));
            } /* endif */
            if ( !usRc ) pDimpIda->fAll = (pDimpIda->ulUsedInConv != ulFreeConvBuffer );
        }
        break;
      } /* endswitch */

    *pulFreeConvBuffer = ulFreeConvBuffer;
	*pulFilled = ulFilled;
	*pulFreeBuffer = ulFreeBuffer;
	*pfMsgDisplayed = fMsgDisplayed;

	return (usRc);
}

static USHORT
CheckForCodePage
(
	PDIMPIDA pDimpIda,
	PBOOL    pfReadAgain,
	PBOOL    pfMsgDisplayed
)
{
  USHORT usRc = NO_ERROR;

  PSZ_W       pTemp;
  PSZ_W       pRest = NULL;
  USHORT      usColPos = 0;
  PSZ_W       pDummyW;                    //pointer to string
  PTOKENENTRY pToken;
  BOOL        fFound = FALSE;
  SHORT       sToken;

  /*************************************************************/
  /* set end of data indication                                */
  /*************************************************************/
  pTemp = (PSZ_W)(pDimpIda->pInputFileBlock+pDimpIda->ulUsed);
  *pTemp = '\0';

  TAFastTokenizeW( (PSZ_W)pDimpIda->pInputFileBlock,       // is CHAR_W
				(PLOADEDTABLE) pDimpIda->pTagTable,
				pDimpIda->fAll,
				&pRest,
				&usColPos,
				(PTOKENENTRY) pDimpIda->pTokenBlock,
			  (MAXI_SIZE * 3) / sizeof(TOKENENTRY) );  

  if ( pRest != (PSZ_W)pDimpIda->pInputFileBlock )                // any tokens found ???
  {
    pDimpIda->pToken = (PTOKENENTRY) pDimpIda->pTokenBlock;

    pToken = (PTOKENENTRY) pDimpIda->pTokenBlock;
    if (pToken->sTokenid >=0)
	  {
	    sToken = pDimpIda->sOrgToken[pToken->sTokenid] ;
	  }
	  else
	  {
		  sToken = pToken->sTokenid;
	  }

    while (pToken && !fFound && (pToken->sTokenid != ENDOFLIST) )
    {
		  switch (  sToken )
		  {
		    case CODEPAGE_TOKEN:
			  usRc = HandleCodePageToken(pDimpIda, &pToken, &pDummyW,
			                            pfReadAgain, pfMsgDisplayed);
			  fFound = TRUE;
		    break;
            case EHEADER_TOKEN:
              fFound = TRUE;     // end the loop - no occurrence of CodePageToken
            break;
		  }

		  pToken++;
		  if (pToken->sTokenid >=0)
		  {
		    sToken = pDimpIda->sOrgToken[pToken->sTokenid] ;
		  }
		  else
		  {
			  sToken = pToken->sTokenid;
    	  }
      } // endwhile
  } /* endif */

	return(usRc);
}

// memory import using standard file open dialog
BOOL DicImportFileOpenDialog
(
  PDIMPIDA pDimpIda
)
{
  BOOL fOK = TRUE;
  EQFINFO ErrorInfo;
  OPENFILENAME OpenStruct;
  PSZ pszFileBuffer = NULL;                         // buffer for returned file names
#define FILEBUFFERSIZE 8096

  // initial processing
  pDimpIda->usImpMode = DICT_FORMAT_SGML_ASCII;           // set default value
  memset( &OpenStruct, 0, sizeof(OpenStruct) );
  OpenStruct.lStructSize = sizeof(OpenStruct);
  OpenStruct.hwndOwner = QUERYACTIVEWINDOW();


  // access last used values
  if ( fOK )
  {
    EQFINFO ErrorInfo = 0;

    //open dictionary list properties
    UtlMakeEQFPath( pDimpIda->szDummy, NULC, SYSTEM_PATH,(PSZ) NULP );
    pDimpIda->hDictListProp = OpenProperties( DICT_PROPERTIES_NAME, pDimpIda->szDummy, PROP_ACCESS_READ, &ErrorInfo );
    if ( !pDimpIda->hDictListProp )
    {
      //error opening properties
      PSZ pszError = DICT_PROPERTIES_NAME;
      UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszError, EQF_ERROR);
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // apply last used values
  if ( fOK )
  {
    PPROPDICTLIST pProp =(PPROPDICTLIST) MakePropPtrFromHnd( pDimpIda->hDictListProp );

    // set last used path
    strcpy( pDimpIda->Controls.szSavedPath, pProp->chDimpPath  );
    OpenStruct.lpstrInitialDir = pDimpIda->Controls.szSavedPath ;

    if ( pProp->usLastImpFormat ) pDimpIda->usImpMode = pProp->usLastImpFormat;
    
    // fill import formats select last used format
    OpenStruct.lpstrFilter = "SGML ANSI (*.SGM)\0*.SGM\0SGML ASCII (*.SGM)\0*.SGM\0SGML UTF-16 (*.SGM)\0*.SGM\0XML UTF-8 (*.DXT)\0*.DXT\0\0";
    OpenStruct.lpstrCustomFilter = NULL;
    switch ( pDimpIda->usImpMode )
    {
      case DICT_FORMAT_SGML_ANSI:    OpenStruct.nFilterIndex = 1; break;
      case DICT_FORMAT_SGML_ASCII:   OpenStruct.nFilterIndex = 2; break;
      case DICT_FORMAT_SGML_UNICODE: OpenStruct.nFilterIndex = 3; break;
      case DICT_FORMAT_XML_UTF8:     OpenStruct.nFilterIndex = 4; break;
      default:                       OpenStruct.nFilterIndex = 1; break;
    } /*endswitch */
  } /*end if*/

  // allocate file list buffer
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pszFileBuffer, 0, FILEBUFFERSIZE, ERROR_STORAGE );
  } /* endif */

  // prepare and show standard file open dialog
  if ( fOK )
  {
    strcpy( pszFileBuffer, pDimpIda->Controls.szSelectedName );
    OpenStruct.lpstrFile = pszFileBuffer;
    OpenStruct.nMaxFile = FILEBUFFERSIZE - 1;
    OpenStruct.lpstrFileTitle = NULL;
    OpenStruct.nMaxFileTitle = 0;
    OpenStruct.lpstrTitle = "Dictionary Import";
    OpenStruct.lpfnHook = DicOpenFileHook;
    OpenStruct.lCustData = (LONG)pDimpIda;
    OpenStruct.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_ALLOWMULTISELECT |
                      OFN_ENABLEHOOK | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;

    if ( GetOpenFileName( &OpenStruct ) )
    {
      fOK = TRUE;

      // the name of the first selected dictionary has been copied to szSelectedName in our hook procedure

      // get import format 
      pDimpIda->fAscii = TRUE; // fAscii means import in external format ...
      switch ( OpenStruct.nFilterIndex )
      {
        case 1:  pDimpIda->usImpMode = DICT_FORMAT_SGML_ANSI; break;
        case 2:  pDimpIda->usImpMode = DICT_FORMAT_SGML_ASCII; break;
        case 3:  pDimpIda->usImpMode = DICT_FORMAT_SGML_UNICODE; break;
        case 4:  pDimpIda->usImpMode = DICT_FORMAT_XML_UTF8; break;
        default: pDimpIda->usImpMode = DICT_FORMAT_SGML_ANSI; break;
      } /*endswitch */
      PSZ pszFileExt = strrchr( pDimpIda->szDictName, '.' );
      if ( ( pszFileExt ) &&
           ( ! stricmp( pszFileExt, ".DXT" ) ) ) {
         pDimpIda->usImpMode = DICT_FORMAT_XML_UTF8; 
      }

      //save last used values
      if ( SetPropAccess( pDimpIda->hDictListProp, PROP_ACCESS_WRITE))
      {
        EQFINFO ErrorInfo = 0;
        PPROPDICTLIST pProp =(PPROPDICTLIST) MakePropPtrFromHnd( pDimpIda->hDictListProp);

        //save path
        sprintf ( pProp->chDimpPath, "%s%s%s", pDimpIda->szPath, DEFAULT_PATTERN_NAME, DEFAULT_PATTERN_EXT );

        //save format
        pProp->usLastImpFormat = pDimpIda->usImpMode;

        //save dict list properties
        SaveProperties( pDimpIda->hDictListProp, &ErrorInfo);
      }/* endif */
    }
    else
    {
      int iRC = GetLastError();
      iRC = iRC;     // place to set breakpoints
      fOK = FALSE;
    } /* endif */
  } /* endif */

  //close dictionary list properties
  if ( pDimpIda->hDictListProp ) CloseProperties( pDimpIda->hDictListProp, PROP_QUIT, &ErrorInfo );

  // cleanup
  if ( pszFileBuffer ) UtlAlloc( (PVOID *)&pszFileBuffer, 0, 0, NOMSG );

  return( fOK );

} /* end of function DicImportFileOpenDialog */

//
// hook procedure for standard file open dialog
//
UINT_PTR CALLBACK DicOpenFileHook
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
        RECT rctFormatStatic, rctReadOnlyCheck, rctFormatCombo;
        HWND hwndDialog = NULLHANDLE;
        HWND hwndFormatStatic = NULLHANDLE;
        HWND hwndReadOnlyCheck = NULLHANDLE;
        HWND hwndToStatic  = NULLHANDLE;
        HWND hwndFormatCombo = NULLHANDLE;
        HWND hwndToCombo = NULLHANDLE;

        // get LOAD IDA
        PDIMPIDA pDimpIda = (PDIMPIDA)pOf->lCustData;

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
        hwndToStatic = CreateWindow( 	"STATIC",
        								"&To dictionary:",
        								WS_CHILD | WS_VISIBLE | SS_LEFT,
        								rctFormatStatic.left,
        								rctReadOnlyCheck.top,
        								rctFormatStatic.right - rctFormatStatic.left,
        								rctFormatStatic.bottom - rctFormatStatic.top,
        								hwndDialog,
        								NULL,
        								(HINSTANCE)UtlQueryULong( QL_HAB ),
        								NULL );
         if ( hwndToStatic ) 
         {
           SetWindowLong( hwndToStatic, GWL_ID, ID_DICTIMP_TO_TEXT );
           SetCtrlFnt( hwndDialog, GetCharSet(), ID_DICTIMP_TO_TEXT, 0 );
         } /* endif */

        // create "To" combo
        hwndToCombo = CreateWindow( "COMBOBOX", "", 
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_SORT | CBS_AUTOHSCROLL, 
                                     rctFormatCombo.left, rctReadOnlyCheck.top - 2, 
                                     rctFormatCombo.right - rctFormatCombo.left,
                                     120,
                                     hwndDialog, NULL, (HINSTANCE)UtlQueryULong( QL_HAB ),
                                     NULL );
         if ( hwndToCombo ) 
         {
           SetWindowLong( hwndToCombo, GWL_ID,  ID_DICTIMP_TO_LB );
           SetCtrlFnt( hwndDialog, GetCharSet(),  ID_DICTIMP_TO_LB, 0 );
         } /* endif */

        // hide read-only checkbox
        ShowWindow( hwndReadOnlyCheck, SW_HIDE );

        // correct Z_order of controls
        SetWindowPos( hwndToCombo, hwndFormatCombo, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOSENDCHANGING );

        // fill to combo box
        EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES, (WPARAM)hwndToCombo, 0L );

        // remember handle of combo box
        pDimpIda->hwndToCombo = hwndToCombo;

        // select the dictionary
        {
          SHORT sItem = 0;
          CBSEARCHSELECTHWND( sItem, hwndToCombo, pDimpIda->szNewDictName );
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
          PDIMPIDA pDimpIda = (PDIMPIDA)pOf->lCustData;
          int iItems = 0;                        // number of selected items

          switch ( pOfNotify->hdr.code )
          {
            case CDN_SELCHANGE :
              // selection change in file list, use first selected file as dictionary name
                           
              // get name of first selected file 
              pDimpIda->Controls.szSelectedName[0] = EOS;
              SendMessage( hwndDialog, CDM_GETSPEC, (WPARAM)sizeof(pDimpIda->Controls.szSelectedName),
                           (LPARAM)pDimpIda->Controls.szSelectedName );

              // use file name up to DOT as dictionary name
              if ( pDimpIda->Controls.szSelectedName[0] )
              {
                if ( pDimpIda->Controls.szSelectedName[0] == '\"' )
                {
                  PSZ pszSource = pDimpIda->Controls.szSelectedName + 1;
                  PSZ pszTarget = pDimpIda->szNewDictName;
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
                  Utlstrccpy( pDimpIda->szNewDictName, pDimpIda->Controls.szSelectedName, DOT );
                  iItems = 1; //  only one file selected
                } /* endif */
                SETTEXTHWND( pDimpIda->hwndToCombo, pDimpIda->szNewDictName );

                // hide or show combo depending on selected items
                if ( iItems > 1 )
                {
                  ShowWindow( pDimpIda->hwndToCombo, SW_HIDE );
                }
                else
                {
                  ShowWindow( pDimpIda->hwndToCombo, SW_SHOW );
                } /* endif */
              } /* endif */
              break;

            case CDN_FILEOK :
              // user pressed OK button

              // get currently selected folder
              pDimpIda->Controls.szSavedPath[0] = EOS;
              SendMessage( hwndDialog, CDM_GETFOLDERPATH, (WPARAM)sizeof(pDimpIda->Controls.szSavedPath),
                          (LPARAM)pDimpIda->Controls.szSavedPath );
              strcpy( pDimpIda->Controls.szPath, pDimpIda->Controls.szSavedPath );
              strcpy( pDimpIda->Controls.szPathContent, pDimpIda->Controls.szSavedPath );

              // free any previously allocated file list buffer
              if ( pDimpIda->pszList )
              {
                UtlAlloc( (PVOID *)&(pDimpIda->pszList), 0, 0, NOMSG );
                pDimpIda->pszList = NULL;
              } /* endif */

              // get selected files
              {
                // allocate a rather large file list buffer as we do not know how much files are selected
                int iAlloc = 8096;
                int iStrLen = 0;
                int iItems = 0;
                UtlAlloc( (PVOID *)&(pDimpIda->pszList), 0, iAlloc, ERROR_STORAGE );

                // get currently selected file(s)
                iStrLen = SendMessage( hwndDialog, CDM_GETSPEC, (WPARAM)iAlloc, (LPARAM)pDimpIda->pszList );

                // count the number of files
                if ( iStrLen > 0 )
                {
                  // if the list starts with a double-quote look we have files enclosed in
                  // double quotes, convert them to a 0 terminated list
                  if ( pDimpIda->pszList[0] == '\"' )
                  {
                    PSZ pszSource = pDimpIda->pszList + 1; 
                    PSZ pszTarget = pDimpIda->pszList; 
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
                  strcpy( pDimpIda->Controls.szPathContent, pDimpIda->Controls.szSavedPath );
                  strcat( pDimpIda->Controls.szPathContent, "\\" );
                  strcat( pDimpIda->Controls.szPathContent, pDimpIda->pszList );
                  strcpy( pDimpIda->szPathContent, pDimpIda->Controls.szPathContent );

                  // for a single file free pszList  
                  if ( iItems == 1 )
                  {
                    UtlAlloc( (PVOID *)&(pDimpIda->pszList), 0, 0, NOMSG );
                    pDimpIda->pszList = NULL;
                  } /* endif */
                } /* endif */
              }

              // if multiple import files were selected use name of first selected file as TM name
              if ( pDimpIda->pszList )
              {
                Utlstrccpy( pDimpIda->szNewDictName, pDimpIda->pszList, DOT );
                SETTEXTHWND( pDimpIda->hwndToCombo, pDimpIda->szNewDictName );
                UtlSplitFnameFromPath( pDimpIda->Controls.szPathContent );
                strcat( pDimpIda->Controls.szPathContent, BACKSLASH_STR );
                strcat( pDimpIda->Controls.szPathContent, pDimpIda->pszList );
              } /* endif */

              // check selected to dictionary
              if( !QUERYTEXTHWND( pDimpIda->hwndToCombo, pDimpIda->szNewDictName ) )
              {
                UtlErrorHwnd( NO_NEW_DICTIONARY_SELECTED, MB_CANCEL, 0, NULL, EQF_WARNING, hdlg );
                SETFOCUSHWND( pDimpIda->hwndToCombo );
                uiResult = 1;
                SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_NO_MEM_NAME ); 
              }
              else
              {
                SHORT sIndexItem = 0;
                BOOL fIsNew = FALSE;         // is-new flag

                //set values from controls ida
                strcpy( pDimpIda->szPathContent, pDimpIda->Controls.szPathContent );

                //check if dict name valid os2 filename, only filename without ext
                UtlStripBlanks( pDimpIda->szNewDictName );
                ANSITOOEM( pDimpIda->szNewDictName );
                strcpy( pDimpIda->szLongName, pDimpIda->szNewDictName );
                ObjLongToShortName( pDimpIda->szNewDictName, pDimpIda->szShortName, DICT_OBJECT, &fIsNew );
                OEMTOANSI( pDimpIda->szNewDictName );
                sIndexItem = (fIsNew) ? LIT_NONE : 0;
                UtlMakeEQFPath( pDimpIda->szString, NULC, PROPERTY_PATH, NULL );
                sprintf( pDimpIda->szPropPath, "%s\\%s%s", pDimpIda->szString, pDimpIda->szShortName, EXT_OF_DICTPROP );

                //if dict already exists query user whether he wants to merge
                //sgml data into specified dictionary
                if ( sIndexItem != LIT_NONE && sIndexItem != LIT_ERROR && fOk )
                {
                  EQFINFO ErrorInfo = 0;
                  HPROP hDictProp = NULL;

                  //open properties
                  PROPNAME ( pDimpIda->szString, pDimpIda->szShortName );
                  hDictProp = OpenProperties( pDimpIda->szString, NULL, PROP_ACCESS_WRITE, &ErrorInfo);

                  if ( hDictProp )
                  {  
                    PPROPDICTIONARY pDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hDictProp );

                    //if dictionary is remote check if drive has not been removed
                    //via the configure drives option on the file pulldown
                    if ( fOk )
                    {
                      USHORT usResult;
                      PSZ pszMsgTable[3];
                      PSZ  pszTemp ;

                      pszMsgTable[0] = pDimpIda->szNewDictName;
                      pszMsgTable[1] = pDimpIda->szPatternName;
                      if ( ! pDimpIda->szPatternName[0] ) {
                         pszTemp = strrchr( pDimpIda->Controls.szPathContent, BACKSLASH ) ;
                         if ( pszTemp ) 
                            pszMsgTable[1] = pszTemp + 1 ;
                      }
                      usResult = UtlErrorHwnd( MERGE_DICTIONARIES, MB_YESNO, 2, pszMsgTable, EQF_QUERY, hdlg );

                      //if user wants to create new dict then place focus on combo box entry field and delete contents
                      if ( usResult == MBID_NO )
                      {
                        SETFOCUSHWND( pDimpIda->hwndToCombo );
                        uiResult = 1;
                        SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_NO_MEM_NAME ); 
                        SETTEXTHWND( pDimpIda->hwndToCombo , "" );
                        fOk = FALSE;
                      }
                      else
                      {
                        //check if the dict is copyrighted or not
                        if ( pDictProp->fCopyRight ) //dict copyrighted
                        {
                          //dict is copyrighted and merge not permitted
                          PSZ pszTemp = pDimpIda->szNewDictName;
                          UtlErrorHwnd( ERROR_SYSTDICT_COPYRIGHTED, MB_CANCEL, 1, &pszTemp, EQF_ERROR, hdlg );
                          SETFOCUSHWND( pDimpIda->hwndToCombo );
                          uiResult = 1;
                          SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_NO_MEM_NAME ); 
                          fOk = FALSE;
                        }
                        else
                        {
                          //Is the dictionary protected?
                          if ( pDictProp->fProtected )
                          {
							HMODULE hResMod;
							hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                            //call up password checking dialog
                            DIALOGBOX( hdlg, DICT1PASSWORDDLG, hResMod, ID_DICTPASSWORD_DLG, pDictProp, fOk );
                            if ( fOk )
                            {
                              //allow merge
                              pDimpIda->fMerge = TRUE;
                            } /* endif */
                          }
                          else
                          {
                            pDimpIda->fMerge = TRUE; //dict not protected
                          } /* endif */
                        } /* endif */

                        if ( fOk )
                        {
                          //check if enough space for import
                          CHAR szDrive[4];
                          USHORT usRc;
                          LONG lBytesShort = 0;
                          PSZ pszTemp = szDrive;

                          szDrive[0] = pDictProp->szDictPath[0];
                          szDrive[1] = EOS;
                          usRc = UtlCheckSpaceForFile( "", 100, MIN_DICT_SPACE, &pszTemp, &lBytesShort, FALSE );

                          if ( !usRc )
                          {
                            //error handling
                            usRc = DictRcHandling( usResult, pDictProp->szDictPath, NULLHANDLE, pDictProp->szServer );
                            fOk = FALSE;    //error occurred with server code
                            if ( (usRc == ERROR_INVALID_DRIVE) || (usRc == ERROR_PATH_NOT_FOUND) )
                            {
                              //remove entry from to dictionary list box
                              SETTEXTHWND( pDimpIda->hwndToCombo, EMPTY_STRING );
                              SETFOCUSHWND( pDimpIda->hwndToCombo );
                              uiResult = 1;
                              SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_NO_MEM_NAME ); 

                              //return sindexitem in dict list window
                              sIndexItem = QUERYSELECTION( pDimpIda->DictLBhwnd, PID_DICTIONARY_LB );
                              if ( sIndexItem != LIT_NONE )
                              {
                                CLBSETITEMSTATE( pDimpIda->DictLBhwnd, PID_DICTIONARY_LB, sIndexItem, FALSE );
                              } /* endif */
                            } /* endif */
                          }
                          else
                          {
                            if ( usRc && lBytesShort )
                            {
                              PSZ pMsgError[4];
                              pMsgError[0] = pDimpIda->szNewDictName;
                              pMsgError[1] = pszTemp; //drive letter
                              pMsgError[2] = ltoa( lBytesShort, pDimpIda->szString, 10 );
                              usRc = UtlErrorHwnd( NO_SPACE_FOR_IMPORT, MB_YESNO | MB_DEFBUTTON2, 3, 
                                                   pMsgError, EQF_QUERY, hdlg );
                              fOk = ( usRc == MBID_YES );
                              uiResult = 1;
                              SetWindowLong( hdlg, DWL_MSGRESULT, NO_SPACE_FOR_IMPORT ); 
                            } /* endif */
                          } /* endif */
                        } /* endif */
                      } /* endif */
                    } /* endif */
                    CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo );
                  }
                  else
                  {
                    //dict props of existing dict cannot be opened
                    PSZ pszServer;
                    PSZ pszTemp = pDimpIda->szNewDictName;
                    UtlErrorHwnd( ERROR_OPENING_PROPS, MB_CANCEL, 1, &pszTemp, EQF_ERROR, hdlg );
                    uiResult = 1;
                    SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_OPENING_PROPS ); 
                    fOk = FALSE;

                    sIndexItem = QUERYSELECTION( pDimpIda->DictLBhwnd, PID_DICTIONARY_LB );
                    QUERYITEMTEXT( pDimpIda->DictLBhwnd, PID_DICTIONARY_LB , sIndexItem, pDimpIda->szString );
                    pszServer = UtlParseX15( pDimpIda->szString, DIC_SERVER_IND);
                    if ( pszServer[0] == NULC )
                    {
                      //grey out dictionary as it cannot be accessed
                      CLBSETITEMSTATE( pDimpIda->DictLBhwnd, PID_DICTIONARY_LB, sIndexItem, FALSE );
                    } /* endif */
                  } /* endif */
                } /* endif */
                if ( fOk )
                {
                  //fully qualified asd filename on szNewDictName
                  //system drive and dict path - later drive should be selectable
                  UtlMakeEQFPath( pDimpIda->szString, NULC, DIC_PATH, (PSZ)NULP );
                  strcat( pDimpIda->szString, BACKSLASH_STR );
                  strcat( pDimpIda->szString, pDimpIda->szShortName );
                  strcat( pDimpIda->szString, EXT_OF_DIC );
                  strcpy( pDimpIda->szNewDictName, pDimpIda->szString );

                  //fully qualified sgml filename onto szDictName
                  strupr( pDimpIda->szPathContent );
                  strcpy( pDimpIda->szDictName, pDimpIda->szPathContent );
                } /* endif */
              } /* endif */
              break;
          } /*endswitch */
        } /* endif */
      }
      break;

    case WM_DESTROY:
      {
        HWND hwndDialog = GetParent( hdlg );
        DelCtrlFont( hwndDialog, ID_DICTIMP_TO_TEXT );
        DelCtrlFont( hwndDialog, ID_DICTIMP_TO_LB );
      }
      break;
    default:
        break;
  } /*endswitch */
  return( uiResult );
} /* end of function DicOpenFileHook */

