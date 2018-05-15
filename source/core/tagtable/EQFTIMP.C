// *************************** Prolog *********************************
//
//               Copyright (C) 1990-2016, International Business Machines      
//               Corporation and others. All rights reserved         
//
//  Short description: TagTable Import
//
//  Author: R. Krome
//
//  Description:       This program provides the functions to generate
//                     the internal format of a tagtable
//
//  Calling Hierarchy:
//
//  Entry point:  TagTableImport()
//
//  Changes:
//
// *********************** End Prolog *********************************

#pragma pack(1)

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file

#include <process.h>
#include <stddef.h>
#include <ctype.h>
#include "eqftimp.id"
#define INITVAR
#include "eqftimp.h"              // markup table import
#include "eqftag00.h"             // markup table plugin

//Declare external variables
#define TIMP "Tagtable import"

/**********************************************************************/
/* Macro to convert hex digits to numeric values.                     */
/* (the hex digit must be converted to uppercase)                     */
/**********************************************************************/
#define HEXTONUM( char ) \
  (( char < 'A' ) ? ( char - '0' ) : ( char - 'A' + 10) )

// Function prototypes for private C functions

HFILE            UtlOpenFile ( PSZ, PULONG );
BOOL             GetStyleName ( PCHOICELIST, PSZ *, PSZ, SHORT );
static BOOL      FillBuffer( PBUFFER, PSZ, USHORT, BOOL );
BOOL             TagImpProcess( HWND hwnd, PTAGIMPORT pTagImport);
PMEMBLOCK        UtlGetNextMemBlock ( PMEMBLOCK * );
BOOL             BuildTagTable ( PTAGIMPORT );
static BOOL      ReadSgml( PTAGIMPORT );
static BOOL      TagImpInit ( PTAGIMPORT );
static BOOL      WriteTagTable ( PTAGIMPORT );
static VOID      SkipTag( PTAGIMPORT, PTOKENENTRY *);
static VOID      TagImpTerm(PTAGIMPORT, BOOL, HWND);
static BOOL      TagImpClose(PTAGIMPORT );
static BOOL      AttrMatch ( PTAGIMPORT, PTOKENENTRY *, PCHOICELIST, BOOL *, PUSHORT );
static BOOL      TextMatch ( PTAGIMPORT, PTOKENENTRY *, BOOL *, PSZ *, SHORT, PUSHORT );
static BOOL      IntMatch ( PTAGIMPORT, PTOKENENTRY *, BOOL *, PUSHORT );
static BOOL      NormString ( PSZ *, BOOL );
static BOOL      ValidTag( PTAGIMPORT, PTOKENENTRY *, PBOOL);
static BOOL      AddToTagList( PTAGIMPORT, PTOKENENTRY * );
static BOOL      AddToAttrList( PTAGIMPORT, PTOKENENTRY * );
static BOOL      PrepareAttr ( PTAGIMPORT, PTAGTABLE, PULONG );
static BOOL      PrepareTags ( PTAGIMPORT, PTAGTABLE, PULONG );
static BOOL      AddDelimit(PTAGIMPORT, PSZ, USHORT, USHORT);
static BOOL      AddStartPos( PTAGIMPORT, USHORT, USHORT, USHORT);
static BOOL      SaveString ( PTAGIMPORT, USHORT, PSZ *, BOOL );
static BOOL      SaveExit ( PTAGIMPORT, PTOKENENTRY );
static BOOL      SaveEndDelim ( PTAGIMPORT, PTOKENENTRY, PSZ * );
static BOOL      AllZero ( PSZ );
VOID             SetTagDefaults(PTERMENTRY);
VOID             SetAttributeDefaults(PATTRENTRY);

static VOID      SetTRNoteDefaults(PTRNOTEENTRY);
static BOOL      FillTRNote(PTAGIMPORT, PTOKENENTRY *, PSZ );

MRESULT          TagImportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
static  SHORT    TagRemoveEscChars( PSZ pszString );
static BOOL CreateTagTable ( PTAGIMPORT pTagImport, PTAGTABLE *ppTable );

BOOL TagImpAddToTagList( PTAGIMPORT pTagImport, PTERMENTRY pNewTag );
BOOL TagImpAddToAttrList( PTAGIMPORT pTagImport,  PATTRENTRY pNewAttr );

UINT_PTR CALLBACK TagImportOpenFileHook( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );


VOID TagTableImport (HWND hwnd, PSZ pSelName )
{
   BOOL       fOK = TRUE;                       // success indicator
   PTAGIMPORT pTagImport;                       // ida for tag import

   // create Import Ida
   fOK = (UtlAlloc( (PVOID *) &pTagImport, 0L, (LONG) sizeof(TAGIMPORT), ERROR_STORAGE ));

   if ( fOK )
   {
     PPROPTAGTABLE pProp;                     // ptr to dictionary properties
     EQFINFO     ErrorInfo;                   //property error information
     OPENFILENAME OpenStruct;

      //remember selected tagtable in Ida
      if ( *pSelName != EOS )
      {
          strcpy( pTagImport->szTargetName, pSelName );
      } /* endif */

      // get last used values
      UtlMakeEQFPath( pTagImport->szDummy, NULC, SYSTEM_PATH, (PSZ) NULP );
      pTagImport->hTagListProp = OpenProperties( TAGTABLE_PROPERTIES_NAME, pTagImport->szDummy, PROP_ACCESS_READ, &ErrorInfo );
      if( pTagImport->hTagListProp )
      {
        PPROPTAGTABLE pProp = (PPROPTAGTABLE) MakePropPtrFromHnd( pTagImport->hTagListProp );
        pTagImport->Controls.chSavedDrive = pProp->chTimpDrive;
        if ( pProp->chTimpPath[0] == EOS  )
        {
          strcpy( pProp->chTimpPath, pProp->chOldTimpPath );
        } /* endif */
        strcpy( pTagImport->Controls.szSavedPath, pProp->chTimpPath );
      }

     // load import dialog
     //DIALOGBOX( EqfQueryTwbClient(), IMPDLG,
     //           hResMod, ID_TAGIMP_DLG, pTagImport, fOK );

    // set last used path
    //strcpy( pDimpIda->Controls.szSavedPath, pProp->chDimpPath  );
    memset( &OpenStruct, 0, sizeof(OpenStruct) );
    OpenStruct.lStructSize = sizeof(OpenStruct);
    OpenStruct.hwndOwner = QUERYACTIVEWINDOW();
    OpenStruct.lpstrInitialDir = pTagImport->Controls.szSavedPath ;
    OpenStruct.lpstrFilter = "Exported Tag Table (*.TBX)\0*.TBX\0\0";
    OpenStruct.lpstrCustomFilter = NULL;
    OpenStruct.nFilterIndex = 1; 
    //strcpy( pTagImport->szPathContent, pTagImport->szTargetName );
    OpenStruct.lpstrFile = pTagImport->szPathContent;
    OpenStruct.nMaxFile = sizeof(pTagImport->szPathContent) - 1;
    OpenStruct.lpstrFileTitle = NULL;
    OpenStruct.nMaxFileTitle = 0;
    OpenStruct.lpstrTitle = "Tag Table Import";
    OpenStruct.lpfnHook = TagImportOpenFileHook;
    OpenStruct.lCustData = (LONG)pTagImport;
    OpenStruct.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_ENABLEHOOK | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
    fOK = GetOpenFileName( &OpenStruct );
    if ( fOK )
    {
      //save last used values
      if( SetPropAccess( pTagImport->hTagListProp, PROP_ACCESS_WRITE))
      {
        pProp = (PPROPTAGTABLE) MakePropPtrFromHnd( pTagImport->hTagListProp);
        strcpy( pProp->chTimpPath, pTagImport->szPathContent );
        pProp->chTimpDrive = pTagImport->szDrive[0];
        SaveProperties( pTagImport->hTagListProp, &ErrorInfo);
      } /* endif */
    } /* endif */
   } /* endif */

   if ( fOK && WinIsWindow( (HAB)NULL, hwnd))   // WM_DESTROYed ?
   {
      strcpy (pTagImport->szSourceName, pTagImport->szPathContent);
      strcat( pTagImport->szSourceName, BACKSLASH_STR );
      strcat( pTagImport->szSourceName, pTagImport->Controls.szSelectedName );
      strcpy( pTagImport->IdaHead.szObjName, pTagImport->szTargetName );
      pTagImport->IdaHead.pszObjName =  pTagImport->IdaHead.szObjName;
      fOK = CreateProcessWindow( pTagImport->IdaHead.szObjName,
                                 TagImportCallBack, pTagImport );
   } /* endif */
   if ( !fOK && pTagImport )
   {
     UtlAlloc( (PVOID *) &pTagImport, 0L, 0L, NOMSG) ;
   } /* endif */
} /* end of TagTableImport */

MRESULT TagImportCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  PTAGIMPORT      pTagImport;          // pointer to instance area
  MRESULT         mResult = FALSE;     // return code for handler proc

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
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        pTagImport          = (PTAGIMPORT) PVOIDFROMMP2(mp2);
        pCommArea->pUserIDA = pTagImport;
        pTagImport->hwnd    = hwnd;

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_TAGIMP_WINDOW;
        pCommArea->sProcessObjClass = clsTAGIMP;
        pCommArea->Style            = PROCWIN_TEXTSLIDER;
        pCommArea->sTextID          = ID_IMPORTTAG_TXT;
        LOADSTRING( NULLHANDLE, hResMod, IDS_TAGIMPORT_TAGTXT, pCommArea->szText );
        strcat( pCommArea->szText, pTagImport->szSourceName );
        pCommArea->sSliderID        = ID_IMPORTTAGSLIDER;
        LOADSTRING( NULLHANDLE, hResMod, IDS_TAGIMPORT_TITLEBAR, pCommArea->szTitle );
        Utlstrccpy( pCommArea->szTitle + strlen(pCommArea->szTitle),
                    UtlGetFnameFromPath( pTagImport->szTargetName ),
                    DOT );
        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_MARKUPIMPICON); //hiconMARKUPIMP;
        pCommArea->fNoClose         = FALSE;
        pCommArea->swpSizePos.x     = 100;
        pCommArea->swpSizePos.y     = 100;
        pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 60;
        pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 14;
        pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
        pCommArea->asMsgsWanted[1]  = 0;
        pCommArea->usComplete       = 0;
      }
      break;


      /****************************************************************/
      /* Start processing by posting WM_EQF_PROCESSTASK               */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      pTagImport = (PTAGIMPORT) pCommArea->pUserIDA;
      pTagImport->Task = INIT_IMPORT;
      WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
      break;

    /******************************************************************/
    /* WM_CLOSE:                                                      */
    /*                                                                */
    /* Prepare/initialize shutdown of process                         */
    /******************************************************************/
    case WM_CLOSE:
      pTagImport = (PTAGIMPORT) pCommArea->pUserIDA;
      if ( pTagImport )
      {
         pTagImport->fKill = TRUE;
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
      pTagImport = (PTAGIMPORT) pCommArea->pUserIDA;
      if ( pTagImport )
      {
        if ( pTagImport->pTagTable )
        {
          TAFreeTagTable( pTagImport->pTagTable );
        } /* endif */
        // invalidate tagtable to avoid further usage...
        if ( pTagImport->IdaHead.pszObjName )
        {
          TAInvalidateTagTable( pTagImport->IdaHead.pszObjName );
        } /* endif */
      } /* endif */
      UtlAlloc( (PVOID *) &pTagImport, 0L, 0L, NOMSG);
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
    case WM_EQF_PROCESSTASK:
      pTagImport = (PTAGIMPORT) pCommArea->pUserIDA;
      if ( TagImpProcess( hwnd, pTagImport ) )
      {
        UtlDispatch();
        WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
      } /* endif */
      break;
  } /* endswitch */
  return( mResult );
}

BOOL TagImpProcess( HWND hwnd, PTAGIMPORT pTagImport)
{
   BOOL      fOK = TRUE;                        // success indicator
   USHORT    usDosRc;                           // return code from DOS function
   ULONG     ulFilePosition;                    // position in source file
   LONG      lNumber;                           // new tick position of slider
   char      *pszMarkupName;

   if (pTagImport->fKill)    // kill flag has been set in the meantime
   {                         // i.e. user wants to cancel import
      pTagImport->usKilledTask = (USHORT)pTagImport->Task; // remeber last task
      pTagImport->Task = CLOSE_PROCESS;
   } /* endif */

   switch (pTagImport->Task)  // switch on task
   {
      case INIT_IMPORT:
        fOK = TagImpInit ( pTagImport);              // init processing
        pTagImport->Task = READ_SGML;
        break;

      case READ_SGML:
        fOK = ReadSgml( pTagImport );   // read and tokenize data
        if (fOK )
           pTagImport->Task = BUILD_ENTRY;
        break;

      case BUILD_ENTRY:
        fOK = BuildTagTable ( pTagImport );   // analyse token block

        // update slider position
        if ( fOK )
        {
           usDosRc = UtlChgFilePtr( pTagImport->hInFile, 0L, 1,
                                    &ulFilePosition,
                                    TRUE );
           lNumber = (LONG)(ulFilePosition * 100 / pTagImport->ulFileSize);

           WinSendMsg( pTagImport->hwnd, WM_EQF_UPDATESLIDER,
                       MP1FROMSHORT((SHORT)lNumber), NULL );
        } /* endif */

        // set next task according to whether end of file is reached
        if (( pTagImport->pRest != NULL || !pTagImport->fAll) && fOK &&
              !pTagImport->fFinished )
        {
           pTagImport->Task = READ_SGML;

           // move rest of text buffer to begin if there is a rest
           if (pTagImport->pRest != NULL)
           {
              pTagImport->usRest = (USHORT)(pTagImport->pFileBlock->usUsedSize -
                 (pTagImport->pRest - (PSZ)(pTagImport->pFileBlock->pDataArea)));

              memmove( pTagImport->pFileBlock->pDataArea,  // move rest to begin
                       pTagImport->pRest,
                       pTagImport->usRest);
           }
           else
           {
              pTagImport->usRest = 0;
           } /* endif */
        }
        else
        {
           pTagImport->Task = WRITE_TABLE;
        } /* endif */
        break;

      case WRITE_TABLE:
        fOK = WriteTagTable( pTagImport );    // create tagtable and write
        pTagImport->Task = END_IMPORT;
        if ( fOK ) {
           pszMarkupName = strrchr( pTagImport->szTargetName, '\\' ) ;
           strcpy( pTagImport->szWork, pszMarkupName+1 ) ;
           pszMarkupName = strrchr( pTagImport->szWork, '.' ) ;
           *(pszMarkupName) = NULL ;
           if ( ! MUUpdateMarkupTableFiles( pTagImport->szWork,
                             "UserMarkupTablePlugin", 
                             pTagImport->szDescription, pTagImport->szDescription, NULL, 
                             pTagImport->szTargetName, pTagImport->szSegmentExit, NULL ) ) {
              fOK = FALSE ;
           }
        }
        break;

      case CLOSE_PROCESS:
        fOK = TagImpClose(pTagImport);       // check whether really close
        break;

      default:
        break;

   } /* endswitch */

   /*******************************************************************/
   /* Termination routine in case of errors or at end of import       */
   /*******************************************************************/
   if ( !fOK || (pTagImport->Task == END_IMPORT) )
   {
     TagImpTerm(pTagImport, fOK, hwnd);     // do cleanup
     fOK = FALSE;                           // force end of import
   } /* endif */
   return( fOK );
} /* end of TagImpProcess */

static
BOOL TagImpInit ( PTAGIMPORT pTagImport)
{
   BOOL      fOK = TRUE;                        // success indicator
   USHORT    usDosRc;                           // return code from DOS function
   USHORT    usAction;                          // used for Dos function
   USHORT    usI;

   // allocate memory for tag and attribute strings
   fOK = UtlAlloc( (PVOID *) &(pTagImport->pStart), 0L, (LONG) MAX_SIZE, ERROR_STORAGE);

   if (fOK)
   {
     pTagImport->pBuffer = pTagImport->pStart;  // set starting point

     // allocate memory for text buffer
     fOK = UtlAlloc( (PVOID *) &(pTagImport->pFileBlock), 0L, (LONG) MEMBLOCKSIZE,
                     ERROR_STORAGE);
     pTagImport->pFileBlock->usDataSize = MEMBLOCKSIZE;
   } /* endif */

   if (fOK)
   {
     pTagImport->pFileBlock->usDataSize = MEMBLOCKSIZE - sizeof(MEMBLOCK);
     pTagImport->pFileBlock->pDataArea = (PBYTE) pTagImport->pFileBlock + sizeof(MEMBLOCK);

     // allocate memory for token buffer
     fOK = UtlAlloc( (PVOID *) &(pTagImport->pTokenBuffer), 0L, (LONG) MEMBLOCKSIZE, NOMSG);
   } /* endif */

   if ( fOK )
   {
      // load internal format table
      TALoadTagTable( TIMPTAGTABLE,
                      &pTagImport->pTagTable,
                      TRUE, TRUE );
      fOK = (pTagImport->pTagTable != NULL);
    } /* endif */

    if (fOK)
    {
      // allocate space for tag delimiter structure
      fOK = UtlAlloc( (PVOID *) &(pTagImport->pDelimit),
                     0L,
                     (LONG) (MAXDELIMIT * sizeof(DELIMIT)),
                     ERROR_STORAGE);
    } /* endif */

    if (fOK)
    {
      // allocate space for tag delpos structure
      fOK = UtlAlloc( (PVOID *) &(pTagImport->pDelPos),
                     0L,
                     (LONG) (MAXDELIMIT * sizeof(DELPOS)),
                     ERROR_STORAGE);
    } /* endif */

    if (fOK)
    {
      pTagImport->sMaxDelimit = MAXDELIMIT;
      pTagImport->sMaxStartPos = MAXDELIMIT;
      pTagImport->Stack.sCurrent = 0;              // init stack
      pTagImport->Stack.sStack[0] = FIRST_TOKEN;

      pTagImport->usRest = 0;
      pTagImport->pInFile = pTagImport->szSourceName;  // save input
      pTagImport->pOutFile = pTagImport->szTargetName; // and output file name
      memcpy(pTagImport->sOrgToken, TAGFORMATTABLE, sizeof(TAGFORMATTABLE));

                                                      // open input file
      pTagImport->hInFile = UtlOpenFile( pTagImport->pInFile,
                            &(pTagImport->ulFileSize) );

      if ( pTagImport->hInFile == NULLHANDLE )
      {
         fOK = FALSE;
      }
      else
      {
         if ( pTagImport->fNewTable )
         {
            usDosRc = UtlOpen(pTagImport->pOutFile,      // open target file
                              &(pTagImport->hOutFile),   // handle to target file
                              &usAction,
                              0L,                        // Create file size
                              FILE_NORMAL,               // Normal attribute
                              OPEN_ACTION_REPLACE_IF_EXISTS |
                                     OPEN_ACTION_CREATE_IF_NEW,
                              OPEN_ACCESS_WRITEONLY |
                              OPEN_SHARE_DENYNONE,
                              0L,                         // Reserved
                              FALSE);                     // error handling
            if ( usDosRc != 0 )
            {
                UtlError( ERROR_ACCESS_DENIED_MSG,
                          MB_CANCEL,
                          1,
                          &(pTagImport->pOutFile),
                          EQF_ERROR);
            }
            fOK = (usDosRc == 0);
         }
         else
         {
            // build temporary name for new tag table
            usI = 1;        // number for temporary file name
            do {
               UtlMakeEQFPath( pTagImport->szTempName,
                               NULC, TABLE_PATH, NULL );
               sprintf( pTagImport->szTempName + strlen(pTagImport->szTempName ),
                        "\\$TAG%3.3d%s", usI, EXT_OF_FORMAT );
               usI++;
            } while ( (usI < 999) &&
                      UtlFileExist( pTagImport->szTempName ) ); /* enddo */

            usDosRc = UtlOpen( pTagImport->szTempName,    // open target file
                               &(pTagImport->hOutTempFile), // hdl to temp file
                               &usAction,
                               0L,                        // Create file size
                               FILE_NORMAL,               // Normal attribute
                               OPEN_ACTION_REPLACE_IF_EXISTS |
                                     OPEN_ACTION_CREATE_IF_NEW,
                               OPEN_ACCESS_WRITEONLY |
                               OPEN_SHARE_DENYNONE,
                               0L,                         // Reserved
                               TRUE );                     // error handling

            fOK = (usDosRc == 0);
         } /* endif */
      } /* endif */
   } /* endif */

   return (fOK);
} /* end of TagImpInit */

//Read in SGML data block

static
BOOL ReadSgml
(
   PTAGIMPORT pTagImport               // tag import structure
)
{
   ULONG   ulBytesRead;                // no of bytes read from input file
   ULONG   ulFreeBuffer;               // number of bytes free
   USHORT  usLastColumn = 0;           // last processed column
   BOOL    fOK = TRUE;                 // success indicator
   USHORT  usDosRc;                    // rc from DOS functions
   PSZ     pszMsgTable[2];
   CHAR    szBuffer[10];               // itoa buffer for return value

   ulFreeBuffer = pTagImport->pFileBlock ->usDataSize -
                  pTagImport->usRest - 1;

   usDosRc = UtlReadL( pTagImport->hInFile,      // read a chunk of data
                      pTagImport->pFileBlock ->pDataArea + pTagImport->usRest ,
                      ulFreeBuffer,
                      &ulBytesRead,
                      TRUE );
   fOK = ( usDosRc == 0 );

   if ( !fOK )
   {
      pszMsgTable[0] = pTagImport->pInFile;
      pszMsgTable[1] = itoa( usDosRc, szBuffer, 10 );
      UtlError( ERROR_FILE_READ_ERROR,
                MB_CANCEL,
                2,
                pszMsgTable,
                EQF_ERROR);
   }
   else
   {
      pTagImport->fAll = ( ulBytesRead != ulFreeBuffer );   // end of file reached

      pTagImport->pFileBlock ->usUsedSize = (USHORT)(ulBytesRead + pTagImport->usRest);
      *(pTagImport->pFileBlock ->pDataArea + pTagImport->usRest + ulBytesRead)
         = EOS;

      /****************************************************************/
      /* We have to use EQFTagTokenize instead of TATagTokenize as    */
      /* TATagTokenize has problems with quoted strings containing    */
      /* end delimiter characters ...                                 */
      /****************************************************************/
      EQFTagTokenize( (PSZ)(pTagImport->pFileBlock ->pDataArea),
                     pTagImport->pTagTable->pTagTable,
                     pTagImport->fAll,
                     &pTagImport->pRest,
                     &usLastColumn,
                     (PTOKENENTRY) pTagImport->pTokenBuffer,
                     MEMBLOCKSIZE/sizeof(TOKENENTRY));

      if ( pTagImport->pRest == (PSZ)(pTagImport->pFileBlock ->pDataArea ))
      {
         UtlError( ERROR_SGML_TAG,
                   MB_CANCEL,
                   1,
                   &(pTagImport->pInFile),
                   EQF_ERROR);
         fOK = FALSE;                           // set error indication
      } /* endif */
   } /* endif */

   return ( fOK );
} /* end of ReadSgml */

static
VOID TagImpTerm( PTAGIMPORT pTagImport, BOOL fOK, HWND hwnd )
{
   USHORT        usRc;

   //close files
   if ( pTagImport->hInFile )
      UtlClose( pTagImport->hInFile, FALSE );
   if ( pTagImport->hOutFile )
      UtlClose( pTagImport->hOutFile, FALSE );
   if ( pTagImport->hOutTempFile )
      UtlClose( pTagImport->hOutTempFile, FALSE );

   //delete existing tag table if tag import went well
   if ( !pTagImport->fNewTable )
   {
      if ( fOK )//delete existing tag table, move temp tag table to existing one
      {
         usRc = UtlDelete( pTagImport->szTargetName , 0L, TRUE );

         //rename temp tag table to original existing tag table
         if ( !usRc )
         {
            usRc = UtlMove( pTagImport->szTempName, pTagImport->szTargetName,
                            0L, TRUE );
         }
      }
      else //delete temp tag table
      {
         usRc = UtlDelete( pTagImport->szTempName , 0L, FALSE );
      } /* endif */
   }
   else  //delete newly created tag table if something went wrong
   {
      if ( !fOK )
      {
         UtlDelete( pTagImport->szTargetName, 0L , FALSE );
      } /* endif */
   } /* endif */

   if (fOK)
   {
     //set slider to 100%
     WinSendMsg( pTagImport->hwnd, WM_EQF_UPDATESLIDER,
                 MP1FROMSHORT(100), NULL );

     // issue message that import has been completed
     UtlError (ERROR_TIMP_COMPLETE, MB_OK, 0, NULL, EQF_INFO);

     if (pTagImport->fNewTable)  // a new table has been created
     {
         //send msg to update tagtable window
         EqfSend2AllHandlers( WM_EQFN_CREATED,
                              MP1FROMSHORT( clsTAGTABLE ),
                              MP2FROMP(pTagImport->szTargetName) );
     }
     else
     {
       //notify translation environment that a the given tagtable has been
       //modified
       EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                            MP1FROMSHORT( clsTAGIMP ),
                            MP2FROMP(pTagImport->szTargetName) );
     } /* endif */
   } /* endif */

   // free allocated memory
   if ( pTagImport->pStart )
     UtlAlloc( (PVOID *) &(pTagImport->pStart), 0L, 0L, NOMSG);
   if ( pTagImport->pTokenBuffer )
     UtlAlloc( (PVOID *) &(pTagImport->pTokenBuffer), 0L, 0L, NOMSG);
   if ( pTagImport->pFileBlock )
     UtlAlloc( (PVOID *) &(pTagImport->pFileBlock), 0L, 0L, NOMSG);
   if ( pTagImport->pDelimit )
     UtlAlloc( (PVOID *) &(pTagImport->pDelimit), 0L, 0L, NOMSG);
   if ( pTagImport->pDelPos )
     UtlAlloc( (PVOID *) &(pTagImport->pDelPos), 0L, 0L, NOMSG);

   EqfRemoveObject(TWBFORCE, hwnd);

} /* end of TagImpTerm */

static
BOOL TagImpClose(PTAGIMPORT pTagImport)
{
   USHORT   usRc;                      // return form UtlError
   BOOL     fOK = TRUE;                // really terminate import?

   if (pTagImport->fKill)
   {
      pTagImport->fKill = FALSE;
      usRc = UtlError (ERROR_TIMP_CANCEL,
                       MB_YESNO,
                       0,
                       NULL,
                       EQF_QUERY);

      if (usRc == MBID_NO)
      {
         pTagImport->Task = (IMEXTASK) pTagImport->usKilledTask;
      }
      else
      {
         fOK =  FALSE;
      } /* endif */
   } /* endif */

   return (fOK);

} /* end of TagImpClose */

BOOL BuildTagTable
(
   PTAGIMPORT pTagImport               // ptr to tagimport structure
)
{
   PTOKENENTRY pToken;                 // pointer into token table
   PTERMENTRY  pTermEntry;             // pointer into term table
   PATTRENTRY  pAttrEntry;             // pointer to Attribute structure
   PSZ         pDummy;                 // temp string pointer
   PSZ         pszErrParm;             // parameter for UtlError call
   BOOL        fOK  = TRUE;            // currently all is okay
   SHORT       sToken;                 // original token
   BOOL        fValid;                 // validation indicator
   BOOL        fSuccess;               // success indicator
   USHORT      usTRPREF = 0;
   PTRNOTEENTRY pTRNoteEntry;
   pTRNoteEntry = &(pTagImport->TRNoteEntry);

   pToken = (PTOKENENTRY) pTagImport->pTokenBuffer;
   pTermEntry  = &(pTagImport->TermEntry);
   pAttrEntry  = &(pTagImport->AttrEntry);

   while ( (pToken->sTokenid != ENDOFLIST ) && fOK && !pTagImport->fFinished )
   {
      if (pToken->sTokenid >= 0 )                  // one of our tags
      {
        fOK = ValidTag( pTagImport, &pToken, &fValid ); // check if tag is valid
        if ( fValid )
        {
           sToken = pTagImport->sOrgToken[pToken->sTokenid];  // map to org. token
           switch (  sToken )
           {
              case SEGEXIT_TOKEN:
                /* store name of segmentexit in tagtable */
                fOK = TextMatch( pTagImport, &pToken, &fSuccess, &pDummy, -1, NULL );
                if ( fSuccess )
                {
                   fOK = SaveExit( pTagImport, pToken);
                   pTagImport->fSegExit = TRUE;
                } /* endif */
                break;

              case TAG_TOKEN :
                 /* initialize the term table entry                                */
                 SetTagDefaults(pTermEntry);
                 pTagImport->Stack.sCurrent ++;      // store new token
                 pTagImport->Stack.sStack[pTagImport->Stack.sCurrent] = sToken;
                 pToken ++;                          // skip this token
                 pTagImport->fTag = TRUE;            // we are dealing with tags
                 break;

              case ETAG_TOKEN :
                 // make consistency check and add it to termtable if okay
                 fOK = AddToTagList( pTagImport, &pToken );
                 if ( fOK )
                 {
                    pToken++;                         // skip token
                    pTagImport->Stack.sCurrent --;   // get rid of uppermost elem.
                 } /* endif */
                 break;

              case TAGTABLE_TOKEN:
                 pTagImport->Stack.sCurrent = 0;     // init stack
                 pTagImport->Stack.sStack[ 0 ] = sToken;
                 pToken++;                           // skip this token
                 break;

              case STRING_TOKEN:
                 if ( pTagImport->fTag )
                 {
                   USHORT usLength = 0;
                   fOK = TextMatch( pTagImport, &pToken, &fSuccess, &(pTermEntry->pText), ESTRING_TOKEN, &usLength );
                   if ( fSuccess )
                   { 
                     fOK = SaveString( pTagImport, usLength, &(pTermEntry->pText), TRUE );
                   } /* endif */
                 }
                 else
                 {
                   fOK = TextMatch( pTagImport, &pToken, &fSuccess, &(pAttrEntry->pText), -1, NULL );
                   if ( fSuccess )
                   {
                      fOK = SaveString( pTagImport, pToken->usLength, &(pAttrEntry->pText), TRUE );
                   } /* endif */
                 } /* endif */
                 break;

              case SEGINFO_TOKEN:
                 {
                   USHORT usStyle;
                   fOK = AttrMatch( pTagImport, &pToken,
                                    SEGSTYLES, &fSuccess,
                                    &usStyle );
                   pTermEntry->Segbreak = (SEGBREAK) usStyle;
                 }
                 break;

              case ASSTEXT_TOKEN:
                 {
                   USHORT usStyle;
                   fOK = AttrMatch( pTagImport, &pToken,
                                    ASSTEXTSTYLES, &fSuccess,
                                    &usStyle );
                   pTermEntry->Asstext = (ASSTEXT) usStyle;
                 }
                 break;

              case TYPE_TOKEN:
                 {
                   USHORT usStyle;
                   fOK = AttrMatch( pTagImport, &pToken,
                                    TYPESTYLES, &fSuccess,
                                    &usStyle );
                   pTermEntry->Tagtype = (TAGTYPE) usStyle;
                 }
                 break;

              case ATTRINFO_TOKEN:
                 {
                   USHORT fTemp;

                   fOK = AttrMatch( pTagImport, &pToken, YESNOSTYLES, &fSuccess,
                                    &fTemp );
                   pTermEntry->fAttr = (BOOL)fTemp;
                 }
                 break;
              case TRANSINFO_TOKEN:
                 if (pTagImport->fTag)
                 {
                   USHORT usStyle;
                   fOK = AttrMatch( pTagImport, &pToken, YESNOSTYLES, &fSuccess,
                                    &usStyle );
                   pTermEntry->BitFlags.fTranslate = usStyle;

                 }
                 else
                 {
                   USHORT usStyle;
                   fOK = AttrMatch( pTagImport, &pToken, YESNOSTYLES, &fSuccess,
                                    &usStyle );
                   pAttrEntry->BitFlags.fTranslate = usStyle;
                 } /* endif */
                 break;

              case COLUMPOS_TOKEN:
                 fOK = IntMatch( pTagImport, &pToken, &fSuccess,
                                 &(pTermEntry->usPosition) );
                 break;

              case ADDINFO_TOKEN:
                 if (pTagImport->fTag)
                 {
                   USHORT usAddInfo;
                   fOK = IntMatch( pTagImport, &pToken, &fSuccess,
                                 &usAddInfo );
                   pTermEntry->BitFlags.AddInfo = usAddInfo;
                 }
                 else
                 {
                   USHORT usAddInfo;
                   fOK = IntMatch( pTagImport, &pToken, &fSuccess,
                                 &usAddInfo );
                   pAttrEntry->BitFlags.AddInfo = usAddInfo;
                 } /* endif */
                 break;

              case LENGTH_TOKEN:
                 if (pTagImport->fTag)
                 {
                   fOK = IntMatch( pTagImport, &pToken, &fSuccess,
                                   &(pTermEntry->usLength ));
                 }
                 else
                 {
                   fOK = IntMatch( pTagImport, &pToken, &fSuccess,
                                   &(pAttrEntry->usLength ));
                 } /* endif */
                 break;

              case ENDDELIM_TOKEN:
                 if ( pTagImport->fTag )
                 {
                    fOK = TextMatch( pTagImport, &pToken, &fSuccess, &(pTermEntry->pEndDelim), -1, NULL );
                   if ( fSuccess )
                   {
                     fOK = SaveEndDelim( pTagImport,
                                         pToken,
                                         &(pTermEntry->pEndDelim));
                   } /* endif */

                 }
                 else
                 {
                    fOK = TextMatch( pTagImport, &pToken, &fSuccess, &(pAttrEntry->pEndDelim), -1, NULL );
                   if ( fSuccess )
                   {
                     fOK = SaveEndDelim( pTagImport,
                                         pToken,
                                         &(pAttrEntry->pEndDelim) );
                   } /* endif */
                 } /* endif */
                 break;

              case ATTRIBUTE_TOKEN :
                 /* initialize the attribute entry                                */
                 SetAttributeDefaults(pAttrEntry);
                 pTagImport->Stack.sCurrent ++;      // store new token
                 pTagImport->Stack.sStack[pTagImport->Stack.sCurrent] = sToken;
                 pToken ++;                          // skip this token
                 pTagImport->fTag = FALSE;                       // attribute
                 break;

              case EATTRIBUTE_TOKEN:
                 fOK = AddToAttrList( pTagImport, &pToken );  // add to attribute list
                 if ( fOK )
                 {
                    pToken++;                     // skip token
                    pTagImport->Stack.sCurrent --;// get rid of uppermost elem.
                 }
                 break;

               case TRNOTE_TOKEN:
                 /*****************************************************/
                 /* initialize the trnote entry                       */
                 /*****************************************************/
                 usTRPREF = 0;
                 SetTRNoteDefaults(pTRNoteEntry);
                 pTagImport->Stack.sCurrent ++;
                 pTagImport->Stack.sStack[pTagImport->Stack.sCurrent] = sToken;
                 pToken++;
                 break;

               case TRTAGSTART_TOKEN:
                 fOK = FillTRNote(pTagImport, &pToken, pTRNoteEntry->chStartText);
                 break;
               case TRTAGEND_TOKEN:
                 fOK = FillTRNote(pTagImport, &pToken, pTRNoteEntry->chEndText);
                 break;


               case TRPREF_TOKEN:
                 usTRPREF++;
                 if (usTRPREF == 1 )
                 {
                     fOK = FillTRNote(pTagImport, &pToken,
                                      pTRNoteEntry->chTrnote1Text);
                 }
                 else if (usTRPREF == 2)
                 {
                     fOK = FillTRNote(pTagImport, &pToken,
                                      pTRNoteEntry->chTrnote2Text);
                 }
                 else
                 {
                   /***************************************************/
                   /* only two levels of TRNotes supported            */
                   /* skip this level                                 */
                   /***************************************************/
                 } /* endif */
                 break;

              case CLASSID_TOKEN:
                 {
                   USHORT usClassId;

                   fOK = AttrMatch( pTagImport, &pToken, CLASSIDSTYLES, &fSuccess,
                                    &usClassId );
                   if ( fOK )
                   {
                     pTermEntry->TagClassId = (TAGCLASSID) usClassId;
                   } /* endif */
                 }
                 break;

              case TOKENID_TOKEN:
                 if (pTagImport->fTag)
                 {
                   fOK = IntMatch( pTagImport, &pToken, &fSuccess,
                                   &(pTermEntry->usFixTokenId ));
                 }
                 else
                 {
                   fOK = IntMatch( pTagImport, &pToken, &fSuccess,
                                   &(pAttrEntry->usFixTokenId ));
                 } /* endif */
                 break;


              case COMMENT_TOKEN:
                 fOK = TextMatch( pTagImport, &pToken,  &fSuccess, &pDummy, -1, NULL );
                 break;


              case ETRTAGSTART_TOKEN:
              case ETRTAGEND_TOKEN:
              case ETRPREF_TOKEN:
              case ETRNOTE_TOKEN:
              case ECLASSID_TOKEN:
              case ETOKENID_TOKEN:

              case ESTRING_TOKEN:                      // end tags
              case ESEGINFO_TOKEN:
              case EASSTEXT_TOKEN:
              case ETYPE_TOKEN:
              case ECOLUMPOS_TOKEN:
              case EENDDELIM_TOKEN:
              case ECOMMENT_TOKEN:
              case ELENGTH_TOKEN:
              case ETRANSINFO_TOKEN:
              case EATTRINFO_TOKEN:
              case ESEGEXIT_TOKEN:
              case EADDINFO_TOKEN:
              case EDESCRIPTION_TOKEN:
              case EDESCRNAME_TOKEN:
              case ECHARSET_TOKEN:
              case ESINGLESUBST_TOKEN:
              case EMULTSUBST_TOKEN:
              case EUSEUNICODE_TOKEN:
              case EREFLOW_TOKEN:
                 pToken++;                           // skip this token
                 pTagImport->Stack.sCurrent --;      // get rid of uppermost elem.
                 break;

              case ETAGTABLE_TOKEN:
                 pToken++;                           // skip this token
                 pTagImport->Stack.sCurrent --;      // get rid of uppermost elem.
                 pTagImport->fFinished = TRUE;
                 break;

              case DESCRIPTION_TOKEN:
                fOK = TextMatch( pTagImport, &pToken, &fSuccess, &pDummy, -1, NULL );
                if ( fSuccess )
                {
                  if ( (pToken->usLength + 1) > MAX_DESCRIPTION )
                  {
                    fOK = FALSE;
                    pszErrParm = TokenTable[DESCRIPTION_TOKEN].szName ;
                    UtlError( ERROR_TAGIMP_DATATOOBIG, MB_CANCEL, 1, &pszErrParm, EQF_ERROR);
                  }
                  else
                  {
                    memcpy( pTagImport->szDescription, pToken->pDataString, pToken->usLength );
                    pTagImport->szDescription[pToken->usLength] = EOS;
                  } /* endif */
                } /* endif */
                break;

              case DESCRNAME_TOKEN:
                fOK = TextMatch( pTagImport, &pToken, &fSuccess, &pDummy, -1, NULL );
                if ( fSuccess )
                {
                  if ( (pToken->usLength + 1) > MAX_DESCRIPTION )
                  {
                    fOK = FALSE;
                    pszErrParm = TokenTable[DESCRNAME_TOKEN].szName ;
                    UtlError( ERROR_TAGIMP_DATATOOBIG, MB_CANCEL, 1, &pszErrParm, EQF_ERROR);
                  }
                  else
                  {
                    memcpy( pTagImport->szDescriptiveName, pToken->pDataString, pToken->usLength );
                    pTagImport->szDescriptiveName[pToken->usLength] = EOS;
                  } /* endif */
                } /* endif */
                break;

              case SINGLESUBST_TOKEN:
                fOK = TextMatch( pTagImport, &pToken, &fSuccess, &pDummy, -1, NULL );
                if ( fSuccess )
                {
                  if ( pToken->usLength != 1 )
                  {
                    fOK = FALSE;
                    pszErrParm = TokenTable[SINGLESUBST_TOKEN].szName ;
                    UtlError( ERROR_TAGIMP_DATATOOBIG, MB_CANCEL, 1, &pszErrParm, EQF_ERROR);
                  }
                  else
                  {
                    pTagImport->chSingleSubst = pToken->pDataString[0];
                  } /* endif */
                } /* endif */
                break;

              case MULTSUBST_TOKEN:
                fOK = TextMatch( pTagImport, &pToken, &fSuccess, &pDummy, -1, NULL );
                if ( fSuccess )
                {
                  if ( pToken->usLength != 1 )
                  {
                    fOK = FALSE;
                    pszErrParm = TokenTable[MULTSUBST_TOKEN].szName ;
                    UtlError( ERROR_TAGIMP_DATATOOBIG, MB_CANCEL, 1, &pszErrParm, EQF_ERROR);
                  }
                  else
                  {
                    pTagImport->chMultSubst = pToken->pDataString[0];
                  } /* endif */
                } /* endif */
                break;

              case CHARSET_TOKEN:
                {
                  USHORT usCharSet;
                  fOK = AttrMatch( pTagImport, &pToken, CHARSETSTYLES, &fSuccess,
                                   &usCharSet );
                  if ( fOK )
                  {
                    pTagImport->usCharacterSet = usCharSet;
                  } /* endif */
                }
                break;

              case USEUNICODE_TOKEN:
                 {
                  USHORT usFlag;
                  fOK = AttrMatch( pTagImport, &pToken, YESNOSTYLES, &fSuccess,
                                   &usFlag );
                  if ( fOK )
                  {
                    pTagImport->fUseUnicodeForSegFile = usFlag;
                  } /* endif */
                }
                break;
              case REFLOW_TOKEN:
                 {
                  USHORT usFlag;
                  fOK = AttrMatch( pTagImport, &pToken, REFLOWYESNOSTYLES, &fSuccess,
					               &usFlag );
				  if (fOK)
				  {
					  pTagImport->fReflow = usFlag;
                  }
			     }
                break;
             default:
                pToken ++;                            // should not happen....
                break;
          } /* endswitch */
        } /* endif */
      }
      else
      {
         pToken++ ;              // skip it, white space, etc
      } /* endif */
      UtlDispatch();
   } /* endwhile */
   return ( fOK );
} /* end of BuildTagTable */

//check if the passed token will be an allowed attribute

static
BOOL  AttrMatch ( PTAGIMPORT   pTagImport,      // tag import structure
                  PTOKENENTRY  * ppToken,       // pointer into token table
                  PCHOICELIST  pChoice,         // allowed values
                  BOOL         *pfSuccess,      // processing successfull?
                  PUSHORT      psValue )        // found style value
{
   LONG    lLength;                             // length
   BOOL    fFound = FALSE;                      // currently no entry found
   USHORT  usResult;                            // return value from errorbox
   SHORT   sToken;                              // tokenid
   PTOKENENTRY pToken;                          // pointer to token
   PSTYLEENTRY pStyle;                          // pointer to style entries
   BOOL    fOK = TRUE;                          // true so far
   BOOL    fAlloc = FALSE;                      // new memory allocated?
   PSZ     pStart, pString;                     // pointer to string data
   USHORT  usLength;


   pToken = ++(*ppToken);                       // skip text token
   *pfSuccess = TRUE;

   lLength = pToken->usLength + 1;        // allocate temp storage if required

   if (lLength > MAX_TAGSIZE)
   {
     fOK = UtlAlloc( (PVOID *) &pString, 0L, lLength, ERROR_STORAGE );
     fAlloc = fOK;
     *pfSuccess = fOK;
   }
   else
   {
      pString = pTagImport->szWork;
   } /* endif */

   pStart = pString;
   if ( fOK )
   {                                             // copy string
      memcpy( pString, pToken->pDataString , (USHORT) lLength );
      *(pString + lLength - 1) = EOS;
      fOK = NormString( &pString, TRUE );             // include uppercasing...

      if ( fOK )
      {
        if ( pToken->sTokenid == TEXT_TOKEN)
        {
          pTagImport->Stack.sCurrent++;                // put on stack
          sToken   = pTagImport->sOrgToken[(pToken-1)->sTokenid];  // map to org. token
          pTagImport->Stack.sStack[pTagImport->Stack.sCurrent] = sToken;
          pStyle = AttrStyles;

          while ( (pChoice->Style != LAST_STYLE) && !fFound )
          {
             if ( strcmp((pStyle+pChoice->Style)->szName, pString) == 0)
             {
                *psValue = (pStyle+pChoice->Style)->sValue;  // value found
                fFound = TRUE;
             }
             else
             {
                pChoice++;                             // check with the next one??
             } /* endif */
          } /* endwhile */

          if ( !fFound )
          {
             usLength = min( pToken->usLength, BUFFERLEN-1 );
             *(pString + usLength - 1) = EOS;
             usResult = UtlError( ERROR_TAGIMP_INVSTYLE, MB_YESNOCANCEL,
                                  1, &pString, EQF_QUERY);
             fOK = ( usResult == MBID_YES );       // user wants to goon
             *pfSuccess = FALSE;
             SkipTag( pTagImport, ppToken );      // skip rest of tag
          } /* endif */
        }
        else if ((pToken->sTokenid == ENDOFLIST) &&
                 (!pTagImport->fAll || pTagImport->pRest) )
        {
           pTagImport->pRest = (pToken-1)->pDataString;
           *pfSuccess = FALSE;

        }
        else
        {
           usLength = min( pToken->usLength, BUFFERLEN-1 );
           *(pString + usLength - 1) = EOS;
           usResult = UtlError( ERROR_TAGIMP_INVSTYLE, MB_YESNOCANCEL,
                                1, &pString, EQF_QUERY);
           fOK = ( usResult == MBID_YES );         // user wants to goon
           *pfSuccess = FALSE;
           SkipTag( pTagImport, ppToken );        // skip rest of tag
        } /* endif */
      } /* endif */
   } /* endif */

   if (fAlloc)
   {
     UtlAlloc( (PVOID *) &pStart, 0L, 0L, NOMSG );  // free allocated memory
   } /* endif */

   return ( fOK );                              // pass back the value
} /* end of AttrMatch */

//check if the passed token will be an allowed integer value

static
BOOL  IntMatch ( PTAGIMPORT   pTagImport,      // tag import structure
                  PTOKENENTRY  * ppToken,       // pointer into token table
                  BOOL         * pfSuccess,     // processing successful?
                  PUSHORT      pusValue )        // found style value
{
   LONG    lLength;                             // length
   USHORT  usResult;                            // return value from errorbox
   SHORT   sToken;                              // tokenid
   PTOKENENTRY pToken;                          // pointer to token
   BOOL    fOK = TRUE;                          // true so far
   BOOL    fAlloc = FALSE;                      // new memory allocated?
   PSZ     pStart, pString;                     // pointer to string data
   USHORT  usLength;


   pToken = ++(*ppToken);                       // skip text token
   *pfSuccess = TRUE;

   lLength = pToken->usLength + 1;        // allocate temp storage if required

   if (lLength > MAX_TAGSIZE)
   {
     fOK = UtlAlloc( (PVOID *) &pString, 0L, lLength, ERROR_STORAGE );
     fAlloc = fOK;
     *pfSuccess = fOK;
   }
   else
   {
      pString = pTagImport->szWork;
   } /* endif */

   pStart = pString;
   if ( fOK )
   {                                             // copy string
      memcpy( pString, pToken->pDataString , (USHORT) lLength );
      *(pString + lLength - 1) = EOS;
      fOK = NormString( &pString, TRUE );             // include uppercasing...

      if ( fOK )
      {
        if (pToken->sTokenid == TEXT_TOKEN)
        {
           pTagImport->Stack.sCurrent++;                // put on stack
           sToken   = pTagImport->sOrgToken[(pToken-1)->sTokenid];  // map to org. token
           pTagImport->Stack.sStack[pTagImport->Stack.sCurrent] = sToken;

           // check if string consists of zero characters only
           if ( !AllZero(pString) )
           {
              *pusValue = (USHORT)(atoi(pString));
              if (*pusValue == 0)
              {
                 usLength = min( pToken->usLength, BUFFERLEN-1 );
                 *(pString + usLength - 1) = EOS;
                 usResult = UtlError( ERROR_TAGIMP_INVSTYLE,
                                      MB_YESNOCANCEL,
                                      1,
                                      &pString,
                                      EQF_QUERY);
                 fOK = ( usResult == MBID_YES );         // user wants to goon
                 SkipTag( pTagImport, ppToken );        // skip rest of tag
                 *pfSuccess = FALSE;
              } /* endif */
           }
           else
           {
              *pusValue = 0;
           } /* endif */
        }
        else if ((pToken->sTokenid == ENDOFLIST) &&
                 (!pTagImport->fAll || pTagImport->pRest) )
        {
           pTagImport->pRest = (pToken-1)->pDataString;
           *pfSuccess = FALSE;
        }
        else
        {
           usLength = min( pToken->usLength, BUFFERLEN-1 );
           *(pString + usLength - 1) = EOS;
           usResult = UtlError( ERROR_TAGIMP_INVSTYLE,
                                MB_YESNOCANCEL,
                                1,
                                &pString,
                                EQF_QUERY);
           fOK = ( usResult == MBID_YES );         // user wants to goon
           SkipTag( pTagImport, ppToken );        // skip rest of tag
           *pfSuccess = FALSE;

        } /* endif */
      } /* endif */
   } /*endif */
   if (fAlloc)
   {
     UtlAlloc( (PVOID *) &pStart, 0L, 0L, NOMSG );  // free allocated memory
   } /* endif */

   return ( fOK );                              // pass back the value
}

//strip white space characters and uppercase it (if requested)

static
BOOL NormString ( PSZ   * ppString,    // address of input pointer
                  BOOL fUpper )        // uppercasing requested ??
{
   PSZ pTemp;                          // temporary string pointer
   BOOL  fEnd;                         // end indication flag
   BOOL  fOK = TRUE;                   // function return code

   // remove leading and trailing whitspace, perform upper case conversion
   pTemp = *ppString;                  // store string temp.
   if ( pTemp != NULL)
   {
      if ( fUpper )
      {
         UtlUpper( pTemp );            // normalize string
      } /* endif */

      while ( (*pTemp == ' ') || (*pTemp == CR) || (*pTemp == LF) )
      {
         pTemp ++;
      } /* endwhile */

      *ppString = pTemp;                        // remember where to start

      fEnd = FALSE;                             // not at end
      pTemp += strlen( pTemp ) - 1;

      while ( (pTemp > *ppString) && !fEnd)     // skip white space characters
      {
          switch ( *pTemp)
          {
             case CR:
             case LF:
             case ' ':
               pTemp --;
               break;

              default:
               fEnd = TRUE;
               *(pTemp + 1) = EOS;
               break;
          } /* endswitch */
      } /* endwhile */
   }
   else
   {
      *ppString = EMPTY_STRING;                 // let it be empty string
   } /* endif */

   // check for quoted strings and remove the quotes enclosing the string
   pTemp = *ppString;
   if ( *pTemp == QUOTE )
   {
     LONG	    lLen = strlen(pTemp);

     // check if there is an ending quote
     if ( (lLen > 1) && (pTemp[lLen-1] == QUOTE) )
     {
       // seems to be O.K. so remove quotes enclosing the string
       pTemp[lLen-1] = EOS;
       *ppString = pTemp + 1;
     }
     else
     {
       // time for a warning, maybe in a a later release...
     } /* endif */
   } /* endif */

   // reduce repeated quotes to single ones
   if ( fOK )
   {
     CHAR chLast = BLANK;
     PSZ pszTarget = pTemp = *ppString;

     while ( *pTemp )
     {
       if ( *pTemp == QUOTE )
       {
         if ( chLast == QUOTE )
         {
           // ignore second quote
           pTemp++;
           chLast = BLANK;
         }
         else
         {
           // leave first quote in string
           *pszTarget++ = *pTemp++;
           chLast = QUOTE;
         } /* endif */
       }
       else
       {
         // copy normal characters
         *pszTarget++ = *pTemp++;
         chLast = BLANK;
       } /* endif */
     } /* endwhile */

     // complete string
     *pszTarget = EOS;
   } /* endif */

   return( fOK );
}

//check if the passed token will be a text

static
BOOL  TextMatch ( PTAGIMPORT  pTagImport,       // TagImport structure
                  PTOKENENTRY *ppToken,         // pointer into token table
                  BOOL        *pfSuccess,       // text found successfully?
                  PSZ         * ppString,       // pointer to string
                  SHORT       sStopToken,       // if >= 0 loop until stop token found
                  PUSHORT     pusLength)        // length of found string
{
   USHORT usResult;                             // return value from error box
   PTOKENENTRY pToken;                          // pointer to token
   SHORT   sToken;                              // original token id
   SHORT   sCurToken;                           // original token id of curren token
   BOOL  fOK = TRUE;                            // success indicator
   USHORT usLength;
   PSZ    pError;

   *pfSuccess = TRUE;
   if ( pusLength != NULL ) *pusLength = 0;
   *ppString = NULL;

   do
   {
      pToken = ++(*ppToken);                       // check for text token
      sCurToken = pTagImport->sOrgToken[pToken->sTokenid];  // map to org. token


      if (pToken->sTokenid == TEXT_TOKEN )
      {
          if ( *ppString == NULL ) // first token if this area
          {
            pTagImport->Stack.sCurrent++;                // put on stack
            sToken   = pTagImport->sOrgToken[(pToken-1)->sTokenid];  // map to org. token
            pTagImport->Stack.sStack[pTagImport->Stack.sCurrent] = sToken;
            *ppString = pToken->pDataString;          // get pointer to string data
          }
          if ( pusLength != NULL ) *pusLength = *pusLength + pToken->usLength;
      }
      else if ( (sStopToken >= 0) && (sCurToken != sStopToken)  )
      {
          if ( *ppString == NULL ) // first token if this area
          {
            pTagImport->Stack.sCurrent++;                // put on stack
            sToken   = pTagImport->sOrgToken[(pToken-1)->sTokenid];  // map to org. token
            pTagImport->Stack.sStack[pTagImport->Stack.sCurrent] = sToken;
            *ppString = pToken->pDataString;          // get pointer to string data
          }
          pToken->sTokenid = TEXT_TOKEN;  //treat as text token 
          if ( pusLength != NULL ) *pusLength = *pusLength + pToken->usLength;
      }
      else if ((pToken->sTokenid == ENDOFLIST) && (!pTagImport->fAll || pTagImport->pRest) )
      {
        pTagImport->pRest = (pToken-1)->pDataString; // start last token in new
        *pfSuccess = FALSE;                          // text block
      }
      else
      {
          usLength = min( pToken->usLength, BUFFERLEN-1 );
          memcpy( pTagImport->chWorkBuffer,
                  pToken->pDataString, usLength );
          pTagImport->chWorkBuffer[usLength] = EOS;
          pError = pTagImport->chWorkBuffer;          // get address of working string
          usResult = UtlError( ERROR_TAGIMP_INVSTYLE,
                              MB_YESNOCANCEL,
                              1,
                              &pError,
                              EQF_QUERY);
          fOK = ( usResult == MBID_YES );         // user wants to goon
          *pfSuccess = FALSE;
          SkipTag( pTagImport, ppToken );        // skip rest of tag

      } /* endif */
      pToken++;
      sToken = pTagImport->sOrgToken[pToken->sTokenid];  // map to org. token
   } while ( (sStopToken >= 0) && (*pfSuccess == TRUE) && (sToken != sStopToken) &&  (sToken != ENDOFLIST)); /* enddo */    


   return ( fOK );                           // pass back the value
}

//Skip Tag Token

static
VOID SkipTag( PTAGIMPORT  pTagImport,        // pointer to tag import struct.
              PTOKENENTRY *ppToken )         // pointer into token table
{
   SHORT sTokenId;                           // token id
   SHORT sEndTag;                            // deepest level

   sTokenId = pTagImport->sOrgToken[(*ppToken)->sTokenid];  // map to org. token

   if ( pTagImport->Stack.sCurrent > 0  )
   {
      sEndTag = pTagImport->Stack.sStack[1] + 1; // find end token of this list
                                                // end token follows start token
                                                // in define
      while ( (sTokenId != ENDOFLIST) && (sTokenId != sEndTag) )
      {
         (*ppToken)++;                       // point to next token
         if ((*ppToken)->sTokenid < 0)
         {
            sTokenId = (*ppToken)->sTokenid;
         }
         else
         {
           sTokenId = pTagImport->sOrgToken[(*ppToken)->sTokenid];  // map it
         } /* endif */
      } /* endwhile */

      if (sTokenId != ENDOFLIST)
      {
        pTagImport->Stack.sCurrent = 1;           // reset stack
      } /* endif */
   }
   else
   {
      (*ppToken)++;
   } /* endif */

}

//Is the passed token an allowed one?

static
BOOL ValidTag( PTAGIMPORT pTagImport,        // pointer into tag import
               PTOKENENTRY * ppToken,        // pointer to token
               PBOOL       pfValid)          // validation indicator
{
   PTAGENTRY  pTagEntry;                     // pointer to first tag entry
   SHORT      sTokenId;                      // token id
   SHORT      sLastToken;                    // Last Token
   PSHORT     psValid;                       // point to valid tag
   BOOL       fFound = FALSE;                // no tag found yet
   BOOL       fOK = TRUE;                    // success indicator
   USHORT     usResult;                      // return value from msg box
   PTOKENENTRY  pToken;                      // pointer to token entry
   USHORT     usLength;
   PSZ         pError;


   *pfValid = FALSE;                         // no valid tag yet
   sTokenId = pTagImport->sOrgToken[(*ppToken)->sTokenid];  // map to org. token
   pToken = *ppToken;

   if ( pTagImport->Stack.sCurrent > -1 )
   {
      sLastToken = pTagImport->Stack.sStack[ pTagImport->Stack.sCurrent ];
      pTagEntry = &(TokenTable[sLastToken]);

      psValid = (PSHORT)&(pTagEntry->usValid[ 0 ]);
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
         usLength = min( pToken->usLength, BUFFERLEN-1 );
         memcpy( pTagImport->chWorkBuffer,
                 pToken->pDataString, usLength );
         pTagImport->chWorkBuffer[usLength] = EOS;
         pError = pTagImport->chWorkBuffer;          // get address of working string
         usResult = UtlError( ERROR_TAGIMP_INVTAG,
                              MB_YESNOCANCEL,
                              1,
                              &pError,
                              EQF_QUERY);
         fOK = ( usResult == MBID_YES );         // user wants to goon
         SkipTag( pTagImport, ppToken );        // skip rest of tag
      } /* endif */
   } /* endif */
   return ( fOK );                                 // return result value
}

//Create and Write Tag table

static
BOOL WriteTagTable ( PTAGIMPORT pTagImport)  // output file name
{
   BOOL       fOK = TRUE;                    // pointer to tag
   PTAGTABLE  pTable = NULL;                 // pointer to tag table
   USHORT     usType;
   USHORT     usDosRc;                       // dos return codes
   USHORT     usWritten;                     // number of bytes written
   CHAR       szDrive[2];
   PSZ        pszTemp;
   ULONG      ulTags = pTagImport->usTagCount;

   // calculate number of tags
   for (usType = 0 ; usType < 29  ; usType ++ )
   {
      ulTags += pTagImport->usNumTag[usType];
   } /* endfor */

   if ( ulTags == 0 && pTagImport->usAttrCount == 0 &&
        !pTagImport->fSegExit )
   {
      //issue message that table is empty
      UtlError(ERROR_INVALID_FORMAT, MB_CANCEL, 0, NULL, EQF_ERROR);
      fOK = FALSE;
   } /* endif */

   if ( fOK )
   {
     fOK = CreateTagTable( pTagImport, &pTable )  ;
   } /* endif */

   if ( fOK )
   {
      if ( pTagImport->fNewTable )
      {
         usDosRc =  UtlWrite( pTagImport->hOutFile,
                              pTable,
                              (USHORT) pTable->uLength,
                              &usWritten,
                              TRUE  );
      }
      else
      {
         usDosRc =  UtlWrite( pTagImport->hOutTempFile,
                              pTable,
                              (USHORT) pTable->uLength,
                              &usWritten,
                              TRUE  );
      } /* endif */

      fOK = (usDosRc == 0) && (usWritten == pTable->uLength);
      if ( usWritten != pTable->uLength )
      {
         szDrive[0] = pTagImport->szTargetName[0];
         szDrive[1] = '\0';
         pszTemp = szDrive;
         UtlError( NO_SPACE_FOR_TAGIMPORT, MB_CANCEL,
                   1, &pszTemp, EQF_ERROR );

      } /* endif */
   } /* endif */

   // free meory of tagtable
   UtlAlloc( (PVOID *) &(pTable), 0L, 0L, NOMSG) ;

   return( fOK );
}


static
BOOL CreateTagTable ( PTAGIMPORT pTagImport, PTAGTABLE *ppTable )
{
   BOOL       fOK = TRUE;                    // pointer to tag
   PBYTE      pByte = NULL;                  // pointer to memory block
   PTAGTABLE  pTable = NULL;                 // pointer to tag table
   PTAGSTART  pTagstart;
   PTAG       pTag;
   PATTRIBUTE pAttribute;
   PSZ        pszErrParm;                    // parameter for UtlError call
   ULONG      ulCurOffset;                   // current offset in table
   USHORT     uTableoffs;                    // offset into table
   USHORT     usType;
   LONG       lTableLength;                  // length of table
   ULONG      ulTextlen = 0;                 // length of text strings
   INT        i;
   PSTARTPOS  pStartpos;

   // calculate number of tags
   for (usType = 0 ; usType < 29  ; usType ++ )
   {
      pTagImport->usTagCount = pTagImport->usTagCount + pTagImport->usNumTag[usType];
   } /* endfor */


   if ( fOK )
   {
      ulTextlen = (pTagImport->pBuffer - pTagImport->pStart) ;
      lTableLength = (LONG)sizeof(TAGTABLE) +
                     (LONG)(pTagImport->usTagCount) * (LONG)sizeof(TAG) +
                     (LONG)(pTagImport->usAttrCount) * (LONG)sizeof(ATTRIBUTE) +
                     (LONG)(pTagImport->usTagCount+pTagImport->usAttrCount)*sizeof(TAGADDINFO)+
                     (LONG)ulTextlen +
                     (LONG)(pTagImport->sActiveDelimit+1) * (LONG)sizeof(TAGSTART) +
                     (LONG)(pTagImport->sActiveStartPos+1) * (LONG)sizeof(STARTPOS);
      if ( lTableLength > MAX_SIZE )
      {
         fOK = FALSE;                                 // error
         sprintf( pTagImport->szDummy, "MAX=%ld", MAX_SIZE ) ;
         pszErrParm = pTagImport->szDummy ;
         UtlError( ERROR_TAGIMP_DATATOOBIG,             // string will not fit in
                   MB_CANCEL,                         // buffer
                   1,
                   &pszErrParm,
                   EQF_ERROR);
      } /* endif */
   } /* endif */

   if ( fOK )
   {
      // allocate storage for tagtable
      fOK = UtlAlloc( (PVOID *) &pByte, 0L,  MAX_SIZE, ERROR_STORAGE);
   } /* endif */

   if ( fOK )
   {
      pTable = ( TAGTABLE *) pByte;
      pTable->uNumTags = pTagImport->usTagCount;           // number of tags
      pTable->usVersion  = TAGTABLE_VERSION;
      memcpy(pTable->szSegmentExit, pTagImport->szSegmentExit, MAX_FILESPEC);

      /****************************************************************/
      /* check if we are dealing with TRNotes                         */
      /****************************************************************/
      if (pTagImport->TRNoteEntry.chStartText[0] &&
            pTagImport->TRNoteEntry.chEndText[0] )
      {
        strcpy(pTable->chTrnote1Text, pTagImport->TRNoteEntry.chTrnote1Text);
        strcpy(pTable->chTrnote2Text, pTagImport->TRNoteEntry.chTrnote2Text);
        strcpy(pTable->chStartText, pTagImport->TRNoteEntry.chStartText);
        strcpy(pTable->chEndText, pTagImport->TRNoteEntry.chEndText);
      } /* endif */

      pTable->fUseUnicodeForSegFile = pTagImport->fUseUnicodeForSegFile;
      pTable->chSingleSubst         = pTagImport->chSingleSubst;
      pTable->chMultSubst           = pTagImport->chMultSubst;
      pTable->usCharacterSet        = pTagImport->usCharacterSet;
      strcpy( pTable->szDescription,  pTagImport->szDescription );
      strcpy( pTable->szDescriptiveName,  pTagImport->szDescriptiveName );
      pTable->fReflow               = pTagImport->fReflow;

      uTableoffs = sizeof(TAGTABLE) ;

      /****************************************************************/
      /* prepare space for additional info ...                        */
      /****************************************************************/
      pTable->uAddInfos = uTableoffs;
      uTableoffs += (pTagImport->usTagCount+pTagImport->usAttrCount) *
                                                         sizeof(TAGADDINFO);

      pTable->stFixTag.uOffset = uTableoffs; // offset of fixed tags in table
                                             // number of fixed tags
      pTable->stFixTag.uNumber = pTagImport->usNumTag[0];
      ulCurOffset = uTableoffs + pTable->stFixTag.uNumber * sizeof(TAG);

                                             // prepare tags for table
      PrepareTags( pTagImport, pTable, &ulCurOffset );

      pTable->stAttribute.uOffset = (USHORT)ulCurOffset; // offset of attributes
      pTable->stAttribute.uNumber = pTagImport->usNumAttr[0];

      ulCurOffset += pTable->stAttribute.uNumber * sizeof(ATTRIBUTE);
                                             // prepare attributes for table

      PrepareAttr( pTagImport, pTable, &ulCurOffset );

      memcpy( pByte + ulCurOffset,            // store text strings
              pTagImport->pStart,
              ulTextlen);
      pTable->uTagNames = (USHORT)ulCurOffset;

      ulCurOffset += ulTextlen;
      // calculate correct table length
      pTable->uLength = sizeof(TAGTABLE);
      pTable->uLength += (pTagImport->usTagCount+pTagImport->usAttrCount) *
                                                          sizeof(TAGADDINFO);
      pTable->uLength += (pTagImport->usTagCount ) * sizeof(TAG);
      pTable->uLength += (pTagImport->usAttrCount) * sizeof(ATTRIBUTE);
      pTable->uLength  = (USHORT)(pTable->uLength + ulTextlen);
      pTable->uLength += (pTagImport->sActiveDelimit+1) * sizeof(TAGSTART);
      pTable->uLength += (pTagImport->sActiveStartPos+1) * sizeof(STARTPOS);

      strcpy(pTable->chId, FORMATID);

      pTag = (PTAG) (pByte + pTable->stFixTag.uOffset);

      pTagstart = (TAGSTART *) (pByte + ulCurOffset);

      i = 0;
       do {
          pTagstart[i].chFirstchar = pTagImport->pDelimit[i].chFirst;
          pTagstart[i].usLength = pTagImport->pDelimit[i].usLength;
          pTagstart[i].uEndDelim =
                pTag[pTagImport->pDelimit[i].uEnd].uEndDelimOffs
                + pTable->uTagNames;
          i++;
       } while (pTagImport->pDelimit[i].chFirst != '\0');

      pTable->uTagstart = (USHORT)ulCurOffset;

      ulCurOffset += (pTagImport->sActiveDelimit+1) * sizeof (TAGSTART);

      pStartpos = (STARTPOS *) (pByte + ulCurOffset);

      i = 0;
       do {
          pStartpos[i].usPosition = pTagImport->pDelPos[i].usPosition;
          pStartpos[i].usLength = pTagImport->pDelPos[i].usLength;
          pStartpos[i].uEndDelim =
                pTag[pTagImport->pDelPos[i].uEnd].uEndDelimOffs
                + pTable->uTagNames;
          i++;
       } while (pTagImport->pDelPos[i].usPosition != 0);

      pTable->uStartpos = (USHORT)ulCurOffset;

      ulCurOffset += (pTagImport->sActiveStartPos+1) * sizeof (STARTPOS);

      if (pTagImport->usAttrCount > 0)
      {
         // there should be only one set of attribute end delimiters,
         // include consistency check later
         pAttribute = (PATTRIBUTE) ( pByte  + pTable->stAttribute.uOffset) ;
         pTable->uAttrEndDelim = pAttribute[0].uEndDelimOffs + pTable->uTagNames;
         pTable->uAttrLength = pAttribute[0].usLength;
      }
      else
      {
         // take tag end delimiters as dummy delimiters, think about it
         // more thoroughly later
         pTable->uAttrEndDelim = pTag[0].uEndDelimOffs  + pTable->uTagNames;

      } /* endif */

   } /* endif */

   *ppTable = pTable;

   return( fOK );
}

PMEMBLOCK UtlGetNextMemBlock
(
   PMEMBLOCK *ppNextBlock              /* ptr to next block link              */
)
{
   PMEMBLOCK pNewBlock;                /* pointer to new memory block         */
   BOOL      fOK;                      // internal OK flag

   if ( *ppNextBlock != NULL )         /* is there already a memory block     */
   {
      pNewBlock = *ppNextBlock;        /* yes: use the block                  */
   }
   else
   {
                                       /* no: allocate a new one              */
      fOK = UtlAlloc( (PVOID *) &pNewBlock, 0L, (LONG) MEMBLOCKSIZE, NOMSG );
      if ( fOK )
      {
         pNewBlock->usBlockSize = MEMBLOCKSIZE;
         pNewBlock->usDataSize  = MEMBLOCKSIZE - sizeof(MEMBLOCK);
         pNewBlock->pDataArea   = (PBYTE) pNewBlock + sizeof(MEMBLOCK);
         pNewBlock->pNext       = NULL;
         *ppNextBlock = pNewBlock;     /* append new block to block chain     */
      } /* endif */
   } /* endif */
   return( pNewBlock );
} /* end of UtlGetNextMemBlock */

//Add entry into tag list -
//do it already sorted and divided up into normal and wildcards

static
BOOL AddToTagList( PTAGIMPORT pTagImport,
                   PTOKENENTRY *ppToken    )         // add entry to tag list
{
   BOOL       fOK = TRUE;                     // success indicator
   USHORT     usLength;                       // string length
   USHORT     usResult;                       // return from utlerror
   PSZ        pError;
   PTOKENENTRY pToken;
   BOOL       fError= FALSE;                   // flag for return from UtlError

   pToken = *ppToken;

   if (*(pTagImport->TermEntry.pText) == EOS) // no string specified for tag
   {
         usLength = min( pToken->usLength, BUFFERLEN-1 );
         memcpy( pTagImport->chWorkBuffer,
                 pToken->pDataString, usLength );
         pTagImport->chWorkBuffer[usLength] = EOS;
         pError = pTagImport->chWorkBuffer;          // get address of working string
         usResult = UtlError( ERROR_TAGIMP_INVTAG,
                              MB_YESNOCANCEL,
                              1,
                              &pError,
                              EQF_QUERY);
         fError = TRUE;
         fOK = ( usResult == MBID_YES );         // user wants to goon
         SkipTag( pTagImport, ppToken );        // skip rest of tag
   } /* endif */

   if ( fOK && !fError)
   {
      if (  (*(pTagImport->TermEntry.pEndDelim) == EOS)  &&
            (pTagImport->TermEntry.usLength == 0) )
      {
            fOK = FALSE;
            usLength = min( pToken->usLength, BUFFERLEN-1 );
            memcpy( pTagImport->chWorkBuffer,
                    pToken->pDataString, usLength );
            pTagImport->chWorkBuffer[usLength] = EOS;
            pError = pTagImport->chWorkBuffer;          // get address of working string
            usResult = UtlError( ERROR_TAGIMP_INVTAG,
                                 MB_YESNOCANCEL,
                                 1,
                                 &pError,
                                 EQF_QUERY);
            fError = TRUE;
            fOK = ( usResult == MBID_YES );         // user wants to goon
            SkipTag( pTagImport, ppToken );        // skip rest of tag
      } /* endif */
   } /* endif */

   if (fOK && !fError)
   {
     fOK = TagImpAddToTagList( pTagImport, &(pTagImport->TermEntry) );
   } /* endif */

   return( fOK );
}

//Add entry into attribute list
//-- do it already sorted and divided up into normal and wildcards

static
BOOL AddToAttrList( PTAGIMPORT pTagImport,
                    PTOKENENTRY *ppToken  )        // add entry to attrlist
{
   BOOL       fOK = TRUE;                          // success indicator
   PATTRENTRY pAttr;                               // pointer to current attr
   USHORT     usResult;                            // return from utlerror
   USHORT     usLength;
   PSZ        pError;
   PTOKENENTRY pToken;
   BOOL       fError = FALSE;

   pToken = *ppToken;

   if (*(pTagImport->AttrEntry.pText) == EOS)
   {
         usLength = min( pToken->usLength, BUFFERLEN-1 );
         memcpy( pTagImport->chWorkBuffer,
                 pToken->pDataString, usLength );
         pTagImport->chWorkBuffer[usLength] = EOS;
         pError = pTagImport->chWorkBuffer;          // get address of working string
         usResult = UtlError( ERROR_TAGIMP_INVTAG,
                              MB_YESNOCANCEL,
                              1,
                              &pError,
                              EQF_QUERY);
         fError = TRUE;
         fOK = ( usResult == MBID_YES );         // user wants to goon
         SkipTag( pTagImport, ppToken );        // skip rest of tag
   } /* endif */

   if ( fOK && !fError )
   {
     pAttr = &(pTagImport->AttrEntry);
     if ( (*(pAttr->pEndDelim) == EOS) && (pAttr->usLength == 0) )
     {
           usLength = min( pToken->usLength, BUFFERLEN-1 );
           memcpy( pTagImport->chWorkBuffer,
                   pToken->pDataString, usLength );
           pTagImport->chWorkBuffer[usLength] = EOS;
           pError = pTagImport->chWorkBuffer;          // get address of working string
           usResult = UtlError( ERROR_TAGIMP_INVTAG,
                                MB_YESNOCANCEL,
                                1,
                                &pError,
                                EQF_QUERY);
           fError = TRUE;
           fOK = ( usResult == MBID_YES );         // user wants to goon
           SkipTag( pTagImport, ppToken );        // skip rest of tag
       } /* endif */
   } /* endif */

   if ( fOK && !fError)
   {
     fOK = TagImpAddToAttrList( pTagImport, &(pTagImport->AttrEntry) );
   } /* endif */
   return( fOK );
}

//Prepare tags as requested for table

static
BOOL PrepareTags ( PTAGIMPORT pTagImport,       // pointer to import structure
                   PTAGTABLE  pTable,           // ptr to tagtable structure
                   PULONG    pulCurOffset)     // pointer to offset
{
      PUSHORT pusIndex;                      // pointer to index
      INT  j;                                // index
      BOOL fOK = TRUE;                       // success indicator
      USHORT  usTagCount = 0;                    // number of tags
      PTERMENTRY pTerm;                      // pointer to term entry
      PTERMENTRY pOldTerm;                   // pointer to term entry
      ULONG   ulLength;                      // length of string
      PTAG    pTag;                          // pointer to tags

      PTAGADDINFO pAddInfo;                  // additional tag info

      pusIndex = pTagImport->usNumTag;       // pointer to start.
      for (j=0; j < 27; j++ )
      {
         pTable->stTagIndex[j].uNumber = *(pusIndex+j+1);// store number of tags
         if ( *(pusIndex+j+1) != 0)
         {                                   // tags of this type available
            pTable->stTagIndex[j].uOffset = (USHORT)(*pulCurOffset);  // store offset
            *pulCurOffset += *(pusIndex+j+1) * sizeof(TAG);   // skip those tags
         }
         else
         {
            pTable->stTagIndex[j].uOffset = 0;            // no tag available
         } /* endif */
      } /* endfor */

      pTable->stVarStartTag.uNumber = *(pusIndex+28);   // store number of tags
      if ( *(pusIndex + 28) != 0 )                       // wildcard tags only
      {
         pTable->stVarStartTag.uOffset = (USHORT)*pulCurOffset;     // store offset
         *pulCurOffset += *(pusIndex+28) * sizeof(TAG);   // skip those tags
      }
      else
      {
         pTable->stVarStartTag.uOffset = 0;              // no such tags
      } /* endif */
                                                         // copy tags

      pTag = (PTAG) ((PBYTE) pTable  + pTable->stFixTag.uOffset) ;
      pAddInfo = (PTAGADDINFO) ((PBYTE) pTable  + pTable->uAddInfos) ;

      for (j = 0; j < 29; j++ )
      {
         pTerm = pTagImport->pTermStart[j];              // get pointer to start
         while ( pTerm != NULL && fOK)
         {                                               // get offset
            pTag->uTagnameOffs = (USHORT)(pTerm->pText - pTagImport->pStart);
                                                         // add new delimiters
            fOK = AddDelimit(pTagImport,
                             pTerm->pText,
                             pTerm->usLength,
                             usTagCount);

            if (j == 28 && fOK)   // tag starts with wildcard
            {
              fOK =  AddStartPos( pTagImport,
                                  pTerm->usPosition,
                                  pTerm->usLength,
                                  usTagCount);
            } /* endif */

            ulLength = strlen(pTerm->pEndDelim) + 1;     // copy enddelimiters
            pTag->uEndDelimOffs = (USHORT)(pTerm->pEndDelim - pTagImport->pStart);

            pTag->Tagtype    = (USHORT)pTerm->Tagtype;
            pTag->Segbreak   = (USHORT)pTerm->Segbreak;
            pTag->Asstext    = (USHORT)pTerm->Asstext;
            pTag->fAttr      = (EQF_BOOL)pTerm->fAttr;
            pTag->BitFlags.fTranslate = pTerm->BitFlags.fTranslate;
            pTag->BitFlags.AddInfo    = pTerm->BitFlags.AddInfo;
            if (pTagImport->TRNoteEntry.chStartText[0] )
            {
              pTag->BitFlags.fTRNote = (USHORT)
                 ( stricmp( pTagImport->TRNoteEntry.chStartText, pTerm->pText ) == 0 );
            } /* endif */
            if ((!pTag->BitFlags.fTRNote) &&
                 pTagImport->TRNoteEntry.chEndText[0] )
            {
              pTag->BitFlags.fTRNote = (USHORT)
                 ( stricmp( pTagImport->TRNoteEntry.chEndText, pTerm->pText ) == 0 );
            } /* endif */
            pAddInfo->ClassId = (USHORT)pTerm->TagClassId;
            pAddInfo->usFixedTagId = pTerm->usFixTokenId;
            pAddInfo++;
            pTag->usLength   = pTerm->usLength;
            pTag->usPosition = pTerm->usPosition;

            pTag++;                                       // next tag
            pOldTerm = pTerm;                             // remember last term
            pTerm = pTerm->pNext;                         // next term
            UtlAlloc( (PVOID *) &pOldTerm, 0L, 0L, NOMSG );   // free memory for old term
            usTagCount++;
         } /* endwhile */
      } /* endfor */
      return( fOK );
}

//Prepare attrs as requested for attribute table

static
BOOL PrepareAttr ( PTAGIMPORT pTagImport,       // pointer to import structure
                   PTAGTABLE  pTable,           // ptr to tagtable structure
                   PULONG     pulCurOffset)     // pointer to offset
{
   PUSHORT pusIndex;                       // pointer to index
   INT     j;                              // index
   BOOL    fOK = TRUE;                     // success indicator
   PATTRIBUTE  pAttribute;
   PATTRENTRY  pOldAttr;
   PATTRENTRY  pAttr;
   PTAG        pTag;
   PTAGADDINFO pAddInfo;

   pTagImport->usAttrCount = 0;

   pAddInfo = (PTAGADDINFO) ((PBYTE) pTable+pTable->uAddInfos+pTagImport->usTagCount*sizeof(TAGADDINFO)) ;
   pusIndex = pTagImport->usNumAttr;      // pointer to start.
   for (j=0; j < 27; j++ )
   {                                      // store number of attributes
      pTable->stAttributeIndex[j].uNumber = *(pusIndex+j+1);
      if ( *(pusIndex+j+1) != 0)
      {                                   // tags of this type available
                                          // store offset
         pTable->stAttributeIndex[j].uOffset = (USHORT)(*pulCurOffset);
                                          // skip those tags
         *pulCurOffset += *(pusIndex+j+1) * sizeof(ATTRIBUTE);
      }
      else
      {
         pTable->stAttributeIndex[j].uOffset = 0;     // no tag available
      } /* endif */
   } /* endfor */

   // copy attributes
   pAttribute = (PATTRIBUTE) ((PBYTE) pTable  + pTable->stAttribute.uOffset) ;
   pTag = (PTAG) ((PBYTE) pTable  + pTable->stFixTag.uOffset) ;

   for (j = 0; j < 28; j++ )
   {
      pAttr = pTagImport->pAttrStart[j];              // get pointer to start
      while ( pAttr != NULL)
      {
         pAttribute->uStringOffs = (USHORT)(pAttr->pText - pTagImport->pStart);
                                                      // add new delimiters

         pAttribute->uEndDelimOffs = (USHORT)(pAttr->pEndDelim - pTagImport->pStart);

         pAttribute->BitFlags.fTranslate = pAttr->BitFlags.fTranslate;
         pAttribute->BitFlags.AddInfo    = pAttr->BitFlags.AddInfo;
         pAttribute->usLength = pAttr->usLength;
         pAddInfo->ClassId = 0;
         pAddInfo->usFixedTagId = pAttr->usFixTokenId;
         pAddInfo++;
         pAttribute++;                                  // next tag
         pOldAttr = pAttr;
         pAttr = pAttr->pNext;                         // next term
         UtlAlloc( (PVOID *) &pOldAttr, 0L, 0L, NOMSG );   // free memory for old term
         pTagImport->usAttrCount++;
      } /* endwhile */
   } /* endfor */


   return( fOK );
}

//Add Delimiters

static
BOOL AddDelimit( PTAGIMPORT pTagImport,         // import tag structure
                 PSZ        pBuffer,            // string buffer
                 USHORT     usLength,           // length of entry
                 USHORT usTag)                  // current tag
{
   BOOL fFound = FALSE;
// anke 14.5.
//   char c;                                      // character
// eanke 14.5.
   BOOL fOK = TRUE;                             // success indicator
   PDELIMIT  pDelimit;                          // ptr to delimiter


   if ( pTagImport->sActiveDelimit > pTagImport->sMaxDelimit - 3 )
   {
      fOK = UtlAlloc( (PVOID *) &(pTagImport->pDelimit),
                     (LONG) (pTagImport->sMaxDelimit * sizeof(DELIMIT)),
                     (LONG) ((MAXDELIMIT + pTagImport->sMaxDelimit) *
                                 sizeof(DELIMIT)),
                     ERROR_STORAGE);
      if (fOK)
      {
         pTagImport->sMaxDelimit += MAXDELIMIT;
      } /* endif */
   } /* endif */

   if ( fOK )
   {
      pDelimit = (PDELIMIT) (pTagImport->pDelimit);        // point to delimiter start
      while (((pDelimit->chFirst) != '\0') && ! fFound)
      {
         if (pDelimit->chFirst == *pBuffer &&
             pDelimit->usLength == usLength &&
             pDelimit->uEnd == usTag)
         {
            fFound = TRUE;
         } /* endif */
         pDelimit ++;
      } /* endwhile */
      if (!fFound)
      {
         pDelimit->chFirst = *pBuffer;
         pDelimit->usLength = usLength;
         pDelimit->uEnd    = usTag;
         pTagImport->sActiveDelimit ++;
      } /* endif */
   } /* endif */
   return ( fOK );
}

static
BOOL AddStartPos( PTAGIMPORT pTagImport,         // import tag structure
                  USHORT     usPosition,         // start position
                  USHORT     usLength,           // length of entry
                  USHORT usTag)                  // current tag
{
   BOOL fFound = FALSE;
   BOOL fOK = TRUE;                             // success indicator
   PDELPOS  pDelPos;                          // ptr to delimiter


   if ( pTagImport->sActiveStartPos > pTagImport->sMaxStartPos - 3 )
   {
      fOK = UtlAlloc( (PVOID *) &(pTagImport->pDelPos),
                     (LONG) (pTagImport->sMaxStartPos * sizeof(DELPOS)),
                     (LONG) ((MAXDELIMIT + pTagImport->sMaxStartPos) *
                                 sizeof(DELPOS)),
                     ERROR_STORAGE);
      if (fOK)
      {
         pTagImport->sMaxStartPos += MAXDELIMIT;
      } /* endif */
   } /* endif */

   if ( fOK )
   {
      pDelPos = (PDELPOS) (pTagImport->pDelPos); // point to delimiter start
      while (((pDelPos->usPosition) != 0) && ! fFound)
      {
         if (pDelPos->usPosition == usPosition)
         {
            fFound = TRUE;
         } /* endif */
         pDelPos ++;
      } /* endwhile */
      if (!fFound)
      {
         pDelPos->usPosition = usPosition;
         pDelPos->usLength = usLength;
         pDelPos->uEnd    = usTag;
         pTagImport->sActiveStartPos ++;
      } /* endif */
   } /* endif */
   return ( fOK );
}

//Save the string in a previously allocated buffer

static
BOOL SaveString ( PTAGIMPORT pTagImport,           // pointer to tag structure
                  USHORT     usLength,             // length of string
                  PSZ        * ppString,           // ptr to string
                  BOOL       fEscHandling )        // check for and handle
                                                   // escape sequences
{
   LONG   lLength;                                 // length of string
   BOOL   fOK = TRUE;                              // success indicator
   PSZ    pString;                                 // pointer to string
   PSZ    pAllocString = NULL;
   PSZ    pszErrParm;                              // parameter for UtlError call

   lLength = (LONG)usLength + 1;         // allocate temp storage
   if (lLength > MAX_TAGSIZE)
   {
     fOK = UtlAlloc( (PVOID *) &pString, 0L, lLength, ERROR_STORAGE );
     pAllocString = pString;
   }
   else
   {
      pString = pTagImport->szWork;
   } /* endif */

   if ( fOK )
   {                                             // copy string
      memcpy( pString, *ppString, (USHORT) lLength );
      *(pString + lLength - 1) = '\0';
      fOK = NormString( &pString, TRUE );                  // include uppercasing...
      if ( fOK )
      {
        if ( fEscHandling )
        {
          TagRemoveEscChars(pString);
        } /* endif */

        lLength = strlen(pString) + 1;

        if ( pTagImport->pBuffer + lLength > pTagImport->pStart + MAX_SIZE )
        {
           fOK = FALSE;                                 // error
           sprintf( pTagImport->szDummy, "MAX=%ld", MAX_SIZE ) ;
           pszErrParm = pTagImport->szDummy ;
           UtlError( ERROR_TAGIMP_DATATOOBIG,             // string will not fit in
                     MB_CANCEL,                         // buffer
                     1,
                     &pszErrParm,
                     EQF_ERROR);
        }
        else
        {
           memcpy( pTagImport->pBuffer, pString, (USHORT) lLength );
           *ppString = pTagImport->pBuffer;          // point to buffer
           pTagImport->pBuffer += (USHORT) lLength;  // advance buffer pointer
        } /* endif */
      } /* endif */
      if (pAllocString)
      {
        UtlAlloc( (PVOID *) &pAllocString, 0L, 0L, NOMSG );         // free message
      } /* endif */

   } /* endif */

   return (fOK);
}

//Save the string to the tagimport ida

static
BOOL SaveExit ( PTAGIMPORT pTagImport,           // pointer to tag structure
                PTOKENENTRY pToken)              // ptr to tokenlist
{
   LONG   lLength;                                 // length of string
   BOOL   fOK = TRUE;                              // success indicator
   BOOL   fAlloc = FALSE;                          // new memory allocated?
   PSZ    pString;                                 // pointer to string
   PSZ    pszErrParm;                              // parameter for UtlError call

   lLength = (LONG) pToken->usLength + 1;         // allocate temp storage
   if (lLength > MAX_TAGSIZE)
   {
     fOK = UtlAlloc( (PVOID *) &pString, 0L, lLength, ERROR_STORAGE );
     fAlloc = fOK;
   }
   else
   {
      pString = pTagImport->szWork;
   } /* endif */

   if ( fOK )
   {                                             // copy string
      memcpy( pString, pToken->pDataString, (USHORT) lLength );
      *(pString + lLength - 1) = EOS;
      fOK = NormString( &pString, TRUE );                  // include uppercasing...
      if ( fOK )
      {
        lLength = (LONG)strlen(pString) + 1L;

        if ( lLength > MAX_FILESPEC)
        {
           fOK = FALSE;                                 // error
           pszErrParm = TokenTable[SEGEXIT_TOKEN].szName ;
           UtlError( ERROR_TAGIMP_DATATOOBIG,             // string will not fit in
                     MB_CANCEL,                         // buffer
                     1,
                     &pszErrParm,
                     EQF_ERROR);
        }
        else
        {
           memcpy( pTagImport->szSegmentExit, pString, (USHORT) lLength );
        } /* endif */
      } /* endif */
      if (fAlloc)
      {
        UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );         // free message
      } /* endif */

   } /* endif */

   return (fOK);
}

//Save the enddelimter in a previously allocated buffer

static
BOOL SaveEndDelim ( PTAGIMPORT pTagImport,           // pointer to tag structure
                    PTOKENENTRY pToken,              // ptr to tokenlist
                    PSZ        * ppString)           // ptr to string
{
   LONG   lLength;                                 // length of string
   BOOL   fOK = TRUE;                              // success indicator
   PSZ    pString;                                 // pointer to string
   BOOL   fAlloc = FALSE;                          // new memory allocated?
   PSZ    pszWork;
   PSZ    pszErrParm;                              // parameter for UtlError call

   if (pToken != NULL)
   {                        // changed from 1 to 2 for testing purposes
     lLength = (LONG) pToken->usLength + 1;         // allocate temp storage
   }
   else
   {
     lLength = (LONG) strlen(*ppString) + 1;         // allocate temp storage
   } /* endif */

   if (lLength > MAX_TAGSIZE)
   {
     fOK = UtlAlloc( (PVOID *) &pString, 0L, lLength, ERROR_STORAGE );
     fAlloc = fOK;
   }
   else
   {
      pString = pTagImport->szWork;
   } /* endif */

   memset (pString, 0, (USHORT)lLength); //initialize pString to 0

   if ( fOK )
   {
      // copy ppstring contents in lLength to pstring
      memcpy( pString, *ppString, (USHORT) lLength );

      *(pString + lLength - 1) = EOS;

      // Normalize string only if string is in new (hex-encoded whitespace)
      // format or string is enclosed in quotes
      if ( (strchr( pString, BACKSLASH ) != NULL ) ||
           ((pString[0] == QUOTE) && (pString[lLength-2] == QUOTE)) )
      {
        fOK = NormString( &pString, TRUE );      // include uppercasing...
      }
      else
      {
         UtlUpper( pString );                    // do only upper-casing
      } /* endif */

      if ( fOK )
      {
        // replace any escape sequence strings
        TagRemoveEscChars( pString );
        lLength = (LONG)(strlen(pString) + 1);

        pszWork = pString;

        if ( pTagImport->pBuffer + lLength > pTagImport->pStart + MAX_SIZE)
        {
           fOK = FALSE;                           // error
           pszErrParm = TokenTable[ENDDELIM_TOKEN].szName ;
           UtlError( ERROR_TAGIMP_DATATOOBIG,       // string will not fit in
                     MB_CANCEL,                   // buffer
                     1,
                     &pszErrParm,
                     EQF_ERROR);
        }
        else
        {
           // +1 added here for testing pruposes
           memcpy( pTagImport->pBuffer, pString, (USHORT) lLength );
           *ppString = pTagImport->pBuffer;          // point to buffer
           pTagImport->pBuffer += (USHORT) lLength;          // advance buffer pointer
        } /* endif */
      } /* endif */

      if (fAlloc)
      {
        UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );         // free message
      } /* endif */

   } /* endif */

   return (fOK);
}

static BOOL AllZero ( PSZ    pString)
{
   BOOL    fZero= TRUE;                   // string consists of zero only

   while (fZero && *pString != EOS)
   {
      if (*pString != '0' )
      {
         fZero = FALSE;
      } /* endif */

      pString++;
   } /* endwhile */

   return (fZero);
} /* end of AllZero */

HFILE UtlOpenFile
(
   PSZ    pFileName,                   /* ptr to name of file                 */
   PULONG pulFileSize                  /* location to store the file length   */
)
{
   USHORT    usRC;                     /* return code of Dos... calls         */
   USHORT    usOpenAction;             /* action returnde bu DosOpen call     */
   HFILE     hFile = NULLHANDLE;       /* handle of file                      */

   /* open file                                                               */
   usRC = UtlOpen ( pFileName,
                    &hFile,
                    &usOpenAction,
                    0L,
                    FILE_NORMAL,
                    FILE_OPEN,
                    OPEN_ACCESS_READONLY |
                    OPEN_SHARE_DENYWRITE,
                    0L,
                    FALSE);

   if ( usRC != 0 )
   {
      UtlError( usRC, MB_CANCEL, 1, &pFileName, DOS_ERROR );
   }

   /* get file size                                                           */
   if ( usRC == 0 )
   {
      usRC = UtlGetFileSize( hFile, pulFileSize, TRUE );
     if ( usRC != 0 )
     {
        UtlError( usRC, MB_CANCEL, 1, &pFileName, DOS_ERROR );
     }
   } /* endif */

   /* return file handle                                                      */
   return( hFile );
} /* end of UtlOpenFile */


//+----------------------------------------------------------------------------+
//| SetTagDefaults                                                             |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//+----------------------------------------------------------------------------+
//|                                                                            |
//| Arguments:                                                                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
VOID  SetTagDefaults
(
   PTERMENTRY    pTermEntry
)
{
   memset( pTermEntry, 0, sizeof(TERMENTRY));
   pTermEntry->pText = EMPTY_STRING;
   pTermEntry->pEndDelim = EMPTY_STRING;
   pTermEntry->usColumn = 0;
   pTermEntry->Tagtype = STARTDEL_TAG;
   pTermEntry->Segbreak = SEG_NEUTRAL;
   pTermEntry->Asstext = NOEXPL_TEXT;
   pTermEntry->fAttr = FALSE;
   pTermEntry->usLength = 0;
   pTermEntry->usPosition = 0;
   pTermEntry->BitFlags.fTranslate = FALSE;
   pTermEntry->BitFlags.AddInfo    = 0;

} /* end of function SetTagDefaults */

//+----------------------------------------------------------------------------+
//| SetAttributeDefaults                                                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//+----------------------------------------------------------------------------+
//|                                                                            |
//| Arguments:                                                                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
VOID  SetAttributeDefaults
(
   PATTRENTRY    pAttrEntry
)
{
   memset( pAttrEntry, 0, sizeof(ATTRENTRY));
   pAttrEntry->usLength = 0;
   pAttrEntry->BitFlags.fTranslate = FALSE;
   pAttrEntry->BitFlags.AddInfo    = 0;
   pAttrEntry->pEndDelim = EMPTY_STRING;
   pAttrEntry->pText = EMPTY_STRING;

} /* end of function SetAttributeDefaults */

//+----------------------------------------------------------------------------+
//| SetTRNoteDefaults                                                          |
//+----------------------------------------------------------------------------+
//| Description:    Set the default strings to be used for TRNotes             |
//+----------------------------------------------------------------------------+
//|   Flow:         set the two default strings, to have a default if User does|
//|                 not set one in the tagtable                                |
//+----------------------------------------------------------------------------+
//| Arguments:      PTRNOTEENTRY  the TRNOTEENTRY structure                    |
//+----------------------------------------------------------------------------+
//| Returns:        VOID                                                       |
//+----------------------------------------------------------------------------+
static VOID  SetTRNoteDefaults
(
   PTRNOTEENTRY  pTRNoteEntry
)
{
   memset( pTRNoteEntry, 0, sizeof(TRNOTEENTRY));
   strcpy(pTRNoteEntry->chTrnote1Text, TRNOTE1_STRING);
   strcpy(pTRNoteEntry->chTrnote2Text, TRNOTE2_STRING);
} /* end of function SetTRNoteDefaults */

//+----------------------------------------------------------------------------+
//| FillTRNote                                                                 |
//+----------------------------------------------------------------------------+
//| Description:      fill the TRNote strings as specified in the tagtable     |
//+----------------------------------------------------------------------------+
//|   Flow:           use TextMatch to extract the next string                 |
//|                   fill in the text into the provided field or display      |
//|                     error message if too long                              |
//+----------------------------------------------------------------------------+
//| Arguments:        PTAGIMPORT      -- tagimport structure                   |
//|                   PTOKENENTRY     -- token entry                           |
//|                   PSZ             -- pointer to field to hold text string  |
//+----------------------------------------------------------------------------+
//| Returns:          TRUE            -- everything ok                         |
//|                   FALSE           -- string too long                       |
//+----------------------------------------------------------------------------+
static BOOL  FillTRNote
(
   PTAGIMPORT      pTagImport,
   PTOKENENTRY     *ppToken,
   PSZ             pTRNoteField
)
{
   PSZ         pDummy;
   PSZ         pszErrParm;             // parameter for UtlError call
   BOOL        fSuccess;               // success indicator
   PTOKENENTRY pToken;
   BOOL        fOK;


   fOK = TextMatch( pTagImport, ppToken, &fSuccess, &pDummy, -1, NULL );
   pToken = *ppToken;

   if ( fSuccess )
   {
     /***************************************************/
     /* is tag smaller than MAX_TRNOTE_DESC             */
     /***************************************************/
     if (pToken->usLength+1 < MAX_TRNOTE_DESC )
     {
       memcpy(pTRNoteField, pToken->pDataString, pToken->usLength );
       pTRNoteField [pToken->usLength] = EOS;
       TagRemoveEscChars(pTRNoteField);
     }
     else
     {
       fOK = FALSE;                                 // error
       pszErrParm = TokenTable[TRNOTE_TOKEN].szName ;
       UtlError( ERROR_TAGIMP_DATATOOBIG,             // string will not fit in
                 MB_CANCEL,                         // buffer
                 1,
                 &pszErrParm,
                 EQF_ERROR);
     } /* endif */
   } /* endif */
   return (fOK);
} /* end of function FillTRNote */

/**********************************************************************/
/* TagReplaceEscChars                                                 */
/* Replace any escape sequence in the given string with the char      */
/* value for the escape sequence.                                     */
/* Also removes QUOTEs if string is enclosed in quotes                */
/*   \\ ==> \                                                         */
/*   \r ==> 0x0D (CR)                                                 */
/*   \n ==> 0x0A (LF)                                                 */
/*   \t ==> 0x09 (Tab)                                                */
/*   \xCC ==> character for hex value CC                              */
/**********************************************************************/
static SHORT TagRemoveEscChars( PSZ pszString )
{
  PSZ  pszOut = pszString;
  PSZ  pszIn  = pszString;

  // inline copy characters of string replacing escape sequences
  while ( *pszIn != EOS )
  {
    if ( *pszIn == BACKSLASH )
    {
      // maybe escape sequences
      CHAR chID;

      pszIn++;
      chID = (CHAR)toupper(*pszIn);

      switch ( chID )
      {
        case BACKSLASH:
          // single backslash
          *pszOut++ = BACKSLASH;
          pszIn++;
          break;

        case 'N':
          // linefeed
          *pszOut++ = LF;
          pszIn++;
          break;

        case 'R':
          // carriage return+
          *pszOut++ = CR;
          pszIn++;
          break;

        case 'T':
          // tab character
          *pszOut++ = 0x09;
          pszIn++;
          break;

        case 'X':
          // hexadecimal encoded character
          if ( isxdigit(pszIn[1]) && isxdigit(pszIn[2])  )
          {
            CHAR chHex1 = (CHAR)toupper(pszIn[1]);
            CHAR chHex2 = (CHAR)toupper(pszIn[2]);
            BYTE bTemp;

            bTemp = (BYTE) HEXTONUM( chHex1 );
            bTemp = bTemp << 4;
            bTemp |= HEXTONUM( chHex2 );

            *pszOut++ = bTemp;
            pszIn += 3;
          }
          else
          {
            // incorrect syntax, leave string as-is
            *pszOut++ = BACKSLASH;
            *pszOut++ = *pszIn++;
          } /* endif */
          break;

        default:
          // unkown escape sequence, leave as-is
          *pszOut++ = BACKSLASH;
          *pszOut++ = *pszIn++;
      } /* endswitch */
    }
    else
    {
      // normal characters
      *pszOut++ = *pszIn++;
    } /* endif */
  } /* endwhile */

  // terminate string
  *pszOut = EOS;

  return( 0 );
} /* end of function TagRemoveEscChars */


// functions used by markup table properties dialog to construct markup tables

BOOL TagImpAddToAttrList( PTAGIMPORT pTagImport,  PATTRENTRY pNewAttr )
{
   BOOL       fOK = TRUE;                          // success indicator
   PATTRENTRY pDummy;                              // dummy term pointer
   PATTRENTRY pAttr;                               // pointer to current attr
   PATTRENTRY pAttrStart;                          // start of entry
   USHORT     usType = 0;                          // type of attribute
   CHAR       c;                                   // character used in wildcard
   BOOL       fError = FALSE;


   fOK = UtlAlloc( (PVOID *) &pAttr, 0L, (LONG) sizeof(ATTRENTRY), ERROR_STORAGE);

   if ( fOK && !fError )
   {
       memcpy( pAttr, pNewAttr, sizeof(ATTRENTRY) );

       if ( (strchr( pAttr->pText, CHAR_MULT_SUBST ) == NULL ) &&
              (strchr( pAttr->pText, CHAR_SNGL_SUBST ) == NULL ))
       {
          usType = 0;                               // normal tags
       }
//     else if ( ((c = *(pAttr->pText)) == CHAR_MULT_SUBST ) ||
//               (*(pAttr->pText) == CHAR_SNGL_SUBST )) // start with wild card
//     {
//        usType = 28;                              // only wildcard group
//     }
       else
       {
          c = *(pAttr->pText);
          toupper(c) ;
          if ( (c >= 'A') && (c <= 'Z') )
          {
             usType =  c - 'A' + 1;                 // wildcard within tag
          }
          else
          {
             usType = 27;
          } /* endif */
       } /* endif */

       if (*(pAttr->pEndDelim) == EOS)
       {
         // save empty string as default
         fOK = SaveEndDelim( pTagImport, NULL, &(pAttr->pEndDelim));
       } /* endif */
   } /* endif */

   if (fOK && !fError)
   {
      pAttrStart = pTagImport->pAttrStart[ usType ]; // set starting point
      pTagImport->usAttrCount++;
      pTagImport->usNumAttr[ usType ] ++;           // increase tag found

    if ( !pAttrStart )                           // no tag of this type yet
    {
       pTagImport->pAttrStart[ usType ] = pAttr;
    }
    else if (strcmp(pAttrStart->pText, pAttr->pText) > 0)
    {
       // insert term as first element
        pTagImport->pAttrStart[ usType ] = pAttr;
        pAttr->pNext = pAttrStart;
    }
    else
    {
       while ( pAttrStart->pNext != NULL )
       {
          if ( strcmp( pAttrStart->pNext->pText, pAttr->pText ) <= 0)
          {
             pAttrStart = pAttrStart->pNext;
          }
          else
          {
             break;
          } /* endif */
       } /* endwhile */

       pDummy = pAttrStart->pNext;
       pAttrStart->pNext = pAttr;
       pAttr->pNext = pDummy;

    } /* endif */
   } /* endif */
   return( fOK );
}



BOOL TagImpAddToTagList( PTAGIMPORT pTagImport, PTERMENTRY pNewTag )
{
   BOOL       fOK = TRUE;                     // success indicator
   ULONG      ulLength;                       // string length
   PTERMENTRY pTerm;                          // pointer to current term
   PTERMENTRY pPrevTerm;                      // pointer to previous term
   PTERMENTRY pTermStart;                     // start of entry
   USHORT     usType = 0;                         // type of term
   CHAR       c;                              // character used in wildcard
   BOOL       fError= FALSE;                   // flag for return from UtlError

   fOK = UtlAlloc( (PVOID *) &pTerm, 0L, (LONG) sizeof(TERMENTRY), ERROR_STORAGE);

   if ( fOK && !fError)
   {
      memcpy( pTerm, pNewTag , sizeof(TERMENTRY) );

      ulLength = strlen( pTerm->pText );

      if ( (strchr( pTerm->pText, CHAR_MULT_SUBST ) == NULL ) &&
             (strchr( pTerm->pText, CHAR_SNGL_SUBST ) == NULL ))
      {
         usType = 0;                               // normal tags
      }
      else if ( ((c = *(pTerm->pText)) == CHAR_MULT_SUBST ) ||
                (*(pTerm->pText) == CHAR_SNGL_SUBST )) // start with wild card
      {
         usType = 28;                              // only wildcard group
      }
      else
      {
         toupper(c) ;
         if ( (c >= 'A') && (c <= 'Z') )
         {
            usType =  c - 'A' + 1;                 // wildcard within tag
         }
         else
         {
            usType = 27;
         } /* endif */
      } /* endif */


      if (*(pTerm->pEndDelim) == EOS)  // tag not delimited by end characters
      {
        // save empty string as default
        fOK = SaveEndDelim( pTagImport, NULL, &(pTerm->pEndDelim));
      } /* endif */
   } /* endif */

   if (fOK && !fError)
   {
     pTermStart = pTagImport->pTermStart[ usType ]; // set starting point
     pTagImport->usNumTag[ usType ] ++;           // increase tag found

     if ( !pTermStart )                           // no tag of this type yet
     {
        pTagImport->pTermStart[ usType ] = pTerm;
     }
     else
     {
       if (strcmp(pTermStart->pText, pTerm->pText) > 0)
       {
         // insert term as first element
         pTagImport->pTermStart[ usType ] = pTerm;
         pTerm->pNext = pTermStart;
       }
       else
       {
         pPrevTerm = NULL;
         while ( pTermStart->pNext != NULL )
         {
            if ( strcmp( pTermStart->pNext->pText, pTerm->pText ) <= 0)
            {
               pPrevTerm = pTermStart;
               pTermStart = pPrevTerm->pNext;
            }
            else
            {
               break;
            } /* endif */
         } /* endwhile */

         if ((pTermStart->pNext != NULL) &&
             (pPrevTerm != NULL) &&
             (strcmp(pTermStart->pText, pTerm->pText) == 0))
         {
           pPrevTerm->pNext = pTerm;
           pTerm->pNext = pTermStart;
         }
         else
         {
           pTerm->pNext = pTermStart->pNext;
           pTermStart->pNext = pTerm;
         } /* endif */
       } /*endif*/
     } /* endif */
   } /* endif */

   return( fOK );
}

//////////////////////////////////////////////////////////////////
// external interface for creation of tag tables


// function allocating and preparing the TAGIMPORT structure
BOOL TagImpAllocTagImport( PTAGIMPORT *ppTagImport )
{
  BOOL fOK = TRUE;
  PTAGIMPORT pTagImport;

  fOK = (UtlAlloc( (PVOID *) &pTagImport, 0L, (LONG) sizeof(TAGIMPORT), ERROR_STORAGE ));

  // allocate space for tag and attributes names
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pTagImport->pStart), 0L, (LONG) MAX_SIZE, ERROR_STORAGE);
  if ( fOK ) pTagImport->pBuffer = pTagImport->pStart;  // set starting point

  // allocate space for tag delimiter structure
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pTagImport->pDelimit), 0L,
                     (LONG) (MAXDELIMIT * sizeof(DELIMIT)), ERROR_STORAGE);

  // allocate space for tag delpos structure
  if (fOK) fOK = UtlAlloc( (PVOID *) &(pTagImport->pDelPos), 0L,
                     (LONG) (MAXDELIMIT * sizeof(DELPOS)), ERROR_STORAGE);

  if (fOK)
  {
    pTagImport->sMaxDelimit = MAXDELIMIT;
    pTagImport->sMaxStartPos = MAXDELIMIT;
    memcpy( pTagImport->sOrgToken, TAGFORMATTABLE, sizeof(TAGFORMATTABLE) );
    *ppTagImport = pTagImport;
  }
  else
  {
    *ppTagImport = NULL;
  } /* endif */

  return( fOK );
}

// function freeing the TAGIMPORT structure and related data
BOOL TagImpFreeTagImport( PTAGIMPORT pTagImport )
{
  BOOL fOK = TRUE;
  int i = 0;

  for ( i = 0; i < 29; i++ )
  {
     PTERMENTRY pTermStart = pTagImport->pTermStart[i];
     while ( pTermStart != NULL )
     {
       PTERMENTRY pNext = pTermStart->pNext;
       UtlAlloc( (PVOID *)&pTermStart, 0L, 0L, NOMSG );
       pTermStart = pNext;
     } /* endwhile */
  } /* endfor */

  for (i = 0; i < 28; i++ )
  {
     PATTRENTRY pAttr = pTagImport->pAttrStart[i];
     while ( pAttr != NULL )
     {
       PATTRENTRY pNext = pAttr->pNext;
       UtlAlloc( (PVOID *)&pAttr, 0L, 0L, NOMSG );
       pAttr = pNext;
     } /* endwhile */
  } /* endfor */

  // free allocated memory
  if ( pTagImport->pStart )
     UtlAlloc( (PVOID *) &(pTagImport->pStart), 0L, 0L, NOMSG);
  if ( pTagImport->pTokenBuffer )
     UtlAlloc( (PVOID *) &(pTagImport->pTokenBuffer), 0L, 0L, NOMSG);
  if ( pTagImport->pFileBlock )
     UtlAlloc( (PVOID *) &(pTagImport->pFileBlock), 0L, 0L, NOMSG);
  if ( pTagImport->pDelimit )
     UtlAlloc( (PVOID *) &(pTagImport->pDelimit), 0L, 0L, NOMSG);
  if ( pTagImport->pDelPos )
     UtlAlloc( (PVOID *) &(pTagImport->pDelPos), 0L, 0L, NOMSG);
   UtlAlloc( (PVOID *)&pTagImport, 0L, 0L, NOMSG);

  return( fOK );
}

// function to add a tag (will be called by external processes)
BOOL TagImpAddTag( PTAGIMPORT pTagImport, PTERMENTRY pNewTag )
{
  BOOL fOK = TRUE;
  TOKENENTRY Token;

  // normalize and save tag string
  Token.pDataString = pNewTag->pText;
  Token.usLength = (USHORT)strlen(pNewTag->pText);
  fOK = SaveString ( pTagImport, Token.usLength, &(pNewTag->pText), TRUE );

  // normalize and save tag end delimiter
  if ( fOK && pNewTag->pEndDelim )
  {
    Token.pDataString = pNewTag->pEndDelim;
    Token.usLength = (USHORT)strlen(pNewTag->pEndDelim);
    fOK = SaveString ( pTagImport, Token.usLength, &(pNewTag->pEndDelim), TRUE );
  } /* endif */

  // add tag to the internal tag list
  fOK = TagImpAddToTagList( pTagImport, pNewTag );

  return( fOK );
}

// function to add a tag (will be called by external processes)
BOOL TagImpAddAttr( PTAGIMPORT pTagImport, PATTRENTRY pNewAttr )
{
  BOOL fOK = TRUE;
  TOKENENTRY Token;

  // normalize and save attribute string
  Token.pDataString = pNewAttr->pText;
  Token.usLength = (USHORT)strlen(pNewAttr->pText);
  fOK = SaveString( pTagImport, Token.usLength, &(pNewAttr->pText), TRUE );

  // normalize and save attribute end delimiter
  if ( fOK && pNewAttr->pEndDelim )
  {
    Token.pDataString = pNewAttr->pEndDelim;
    Token.usLength = (USHORT)strlen(pNewAttr->pEndDelim);
    fOK = SaveString( pTagImport, Token.usLength, &(pNewAttr->pEndDelim), TRUE );
  } /* endif */

  // add attribute to the internal attribute list
  fOK = TagImpAddToAttrList( pTagImport, pNewAttr );

  return( fOK );
}

// create a new in-memory tag table from the data in the TAGIMPORT structure
BOOL TagImpBuildtagTable( PTAGIMPORT pTagImport, PTAGTABLE *ppTagTable )
{
  BOOL fOK = TRUE;

  fOK = CreateTagTable( pTagImport, ppTagTable )  ;

  return( fOK );
}


// create a new Markup Table using the function I/F
USHORT MarkupFuncCreateMarkup
(
PSZ         pszInFile,               // Input TBX file
PSZ         pszOutFile               // Output TBL file
)
{
  PTAGIMPORT  pTagImport;              //ptr to global structure
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pszParm;                 // pointer for error parameters
  BOOL        fOK;


  // allocate storage for create IDA
  if ( usRC == NO_ERROR  )
  {
    if ( !UtlAllocHwnd( (PVOID *)&pTagImport, 0L, (LONG)sizeof(TAGIMPORT),
                        ERROR_STORAGE, HWND_FUNCIF ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // Check markup input file name
  if ( usRC == NO_ERROR )
  {
    if ( ( !UtlCheckPath( pszInFile, 0L, NULL  ) ) ||
         ( !UtlFileExist( pszInFile ) ) )
    {
      pszParm = pszInFile;
      usRC = FILE_NOT_EXISTS;
      UtlErrorHwnd(  usRC, MB_CANCEL, 1,
                     &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check markup output file name
  if ( usRC == NO_ERROR )
  {
    if ( !UtlCheckPath( pszOutFile, 0L, NULL  ) )
    {
      pszParm = pszOutFile;
      usRC = ERROR_FILENAME_NOT_VALID;
      UtlErrorHwnd(  usRC, MB_CANCEL, 1,
                     &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  strcpy( pTagImport->szSourceName, pszInFile ) ;
  strcpy( pTagImport->szTargetName, pszOutFile ) ;
  pTagImport->fNewTable = TRUE ;

  // Check markup output file name
  if ( usRC == NO_ERROR ) {
     fOK = TagImpInit ( pTagImport);              // init processing
     if ( ! fOK ) 
        usRC = ERROR_INVALID_FORMAT ;
  }

  pTagImport->fFinished = FALSE ;
  pTagImport->fAll = FALSE ;
  while ( ( ( pTagImport->pRest != NULL ) || 
            ( ! pTagImport->fAll        ) ) && 
          ( usRC == NO_ERROR ) &&
          ( ! pTagImport->fFinished ) )
  {
     fOK = ReadSgml( pTagImport );   // read and tokenize data
     if ( ! fOK ) 
        usRC = ERROR_INVALID_FORMAT ;


     if ( usRC == NO_ERROR ) {
        fOK = BuildTagTable ( pTagImport );   // analyse token block
        if ( ! fOK ) 
           usRC = ERROR_INVALID_FORMAT ;
     }


     // move rest of text buffer to begin if there is a rest
     if ( ( usRC == NO_ERROR ) &&
          ( pTagImport->pRest != NULL ) ) {
        pTagImport->usRest = (USHORT)(pTagImport->pFileBlock->usUsedSize -
           (pTagImport->pRest - (PSZ)(pTagImport->pFileBlock->pDataArea)));

        memmove( pTagImport->pFileBlock->pDataArea,  // move rest to begin
                 pTagImport->pRest,
                 pTagImport->usRest);
     } else {
        pTagImport->usRest = 0;
     }
  }

  if ( usRC == NO_ERROR ) {
     fOK = WriteTagTable( pTagImport );    // create tagtable and write
     if ( ! fOK ) 
        usRC = ERROR_INVALID_FORMAT ;
     pTagImport->Task = END_IMPORT;
  }

  fOK = TagImpClose(pTagImport);       // check whether really close

  /*******************************************************************/
  /* Termination routine in case of errors or at end of import       */
  /*******************************************************************/
  if ( !fOK || (pTagImport->Task == END_IMPORT) )
  {
    TagImpTerm(pTagImport, fOK, NULL);     // do cleanup
    fOK = FALSE;                           // force end of import
  } /* endif */

  // Cleanup
  if ( pTagImport ) 
  {
    UtlAlloc( (PVOID *) &pTagImport, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );
} /* end of function MarkupFuncCreateMarkup */

//
// hook procedure for standard file open dialog
//
UINT_PTR CALLBACK TagImportOpenFileHook
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

        PTAGIMPORT pTagImport = (PTAGIMPORT)pOf->lCustData;

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
        hwndToStatic = CreateWindow( 	"STATIC", "&To Tag Table:", WS_CHILD | WS_VISIBLE | SS_LEFT, rctFormatStatic.left, rctReadOnlyCheck.top, rctFormatStatic.right - rctFormatStatic.left,
        								rctFormatStatic.bottom - rctFormatStatic.top, hwndDialog, NULL,  (HINSTANCE)UtlQueryULong( QL_HAB ), NULL );
         if ( hwndToStatic ) 
         {
           SetWindowLong( hwndToStatic, GWL_ID, ID_TAGIMP_TO_TEXT );
           SetCtrlFnt( hwndDialog, GetCharSet(), ID_TAGIMP_TO_TEXT, 0 );
         } /* endif */

        // create "To" combo
        hwndToCombo = CreateWindow( "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_SORT | CBS_AUTOHSCROLL, 
                                     rctFormatCombo.left, rctReadOnlyCheck.top - 2, rctFormatCombo.right - rctFormatCombo.left, 120,
                                     hwndDialog, NULL, (HINSTANCE)UtlQueryULong( QL_HAB ), NULL );
         if ( hwndToCombo ) 
         {
           SetWindowLong( hwndToCombo, GWL_ID,  ID_TAGIMP_TO_LB );
           SetCtrlFnt( hwndDialog, GetCharSet(),  ID_TAGIMP_TO_LB, 0 );
         } /* endif */

        // hide read-only checkbox
        ShowWindow( hwndReadOnlyCheck, SW_HIDE );

        // correct Z_order of controls
        SetWindowPos( hwndToCombo, hwndFormatCombo, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOSENDCHANGING );

        // fill to combo box
        EqfSend2Handler( TAGTABLEHANDLER, WM_EQF_INSERTNAMES, (WPARAM)hwndToCombo, 0L );

        // remember handle of combo box
        pTagImport->hwndToCombo = hwndToCombo;

        // select the markuptable
        {
          SHORT sItem = 0;
          CBSEARCHSELECTHWND( sItem, hwndToCombo, pTagImport->szName );
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
          PTAGIMPORT pTagImport = (PTAGIMPORT)pOf->lCustData;

          switch ( pOfNotify->hdr.code )
          {
            case CDN_SELCHANGE :
              // selection change in file list, use first selected file as tag table name
                           
              // get name of first selected file 
              pTagImport->Controls.szSelectedName[0] = EOS;
              SendMessage( hwndDialog, CDM_GETSPEC, (WPARAM)sizeof(pTagImport->Controls.szSelectedName),
                           (LPARAM)pTagImport->Controls.szSelectedName );

              // use file name up to DOT as tag table name
              if ( pTagImport->Controls.szSelectedName[0] )
              {
                if ( pTagImport->Controls.szSelectedName[0] == '\"' )
                {
                  PSZ pszSource = pTagImport->Controls.szSelectedName + 1;
                  PSZ pszTarget = pTagImport->szName;
                  while ( *pszSource && (*pszSource != DOT) && (*pszSource != '\"') ) *pszTarget++ = *pszSource++;
                  *pszTarget = EOS;
                }
                else
                {
                  Utlstrccpy( pTagImport->szName, pTagImport->Controls.szSelectedName, DOT );
                } /* endif */
                SETTEXTHWND( pTagImport->hwndToCombo, pTagImport->szName );

              } /* endif */
              break;

            case CDN_FILEOK :
              // user pressed OK button

              // get currently selected tag table
              pTagImport->Controls.szSavedPath[0] = EOS;
              SendMessage( hwndDialog, CDM_GETFOLDERPATH, (WPARAM)sizeof(pTagImport->Controls.szSavedPath), (LPARAM)pTagImport->Controls.szSavedPath );
              strcpy( pTagImport->Controls.szPath, pTagImport->Controls.szSavedPath );
              strcpy( pTagImport->Controls.szPathContent, pTagImport->Controls.szSavedPath );

              // check selected to tag table
              if( !QUERYTEXTHWND( pTagImport->hwndToCombo, pTagImport->szTargetName ) )
              {
                UtlErrorHwnd( NO_NEW_DICTIONARY_SELECTED, MB_CANCEL, 0, NULL, EQF_WARNING, hdlg );
                SETFOCUSHWND( pTagImport->hwndToCombo );
                uiResult = 1;
                SetWindowLong( hdlg, DWL_MSGRESULT, ERROR_NO_MEM_NAME ); 
              }
              else
              {
                // if table already exists query user whether he wants to overwrite
                if ( MUGetMarkupTableFilePath( pTagImport->szTargetName, "UserMarkupTablePlugin", pTagImport->szWork, sizeof(pTagImport->szWork) ) &&
                     UtlFileExist( pTagImport->szWork ) )
                {
                   PSZ pszError =  pTagImport->szTargetName;
                   USHORT usRc = UtlError( ERROR_FILE_EXISTS_ALREADY, MB_YESNO | MB_DEFBUTTON2, 1, &pszError, EQF_QUERY );
                   if ( usRc != MBID_YES )
                   {
                     uiResult = 1;
                   } /* endif */

                } /* endif */

                //set values from controls ida
                strcpy( pTagImport->szPathContent, pTagImport->Controls.szPathContent );
                strcpy( pTagImport->szTargetName, pTagImport->szWork );
                pTagImport->fNewTable = !UtlFileExist( pTagImport->szTargetName );

                UtlStripBlanks( pTagImport->szName );
                ANSITOOEM( pTagImport->szName );
              } /* endif */
              break;
          } /*endswitch */
        } /* endif */
      }
      break;

    case WM_DESTROY:
      {
        HWND hwndDialog = GetParent( hdlg );
        DelCtrlFont( hwndDialog, ID_TAGIMP_TO_TEXT );
        DelCtrlFont( hwndDialog, ID_TAGIMP_TO_LB );
      }
      break;
    default:
        break;
  } /*endswitch */
  return( uiResult );
} /* end of function TagImportOpenFileHook */

