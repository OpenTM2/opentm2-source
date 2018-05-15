// *************************** Prolog *********************************
//
//               Copyright (C) 1990-2013, International Business Machines      
//               Corporation and others. All rights reserved         
//
//  Short description: TagTable Export
//
//  Author:
//
//  Description:       This program provides the functions to generate
//                     the external format of a tagtable
//
//  Calling Hierarchy:
//
//  Entry point:  TagTableExport()
//
//  Changes:
//
// *********************** End Prolog *********************************
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file

#include "eqftexp.id"             // markup table export IDs
#include "eqftimp.h"              // markup table import
#include "eqftexp.h"              // markup table export
#include "eqftag00.h"             // markup table plugin

static BOOL GetStyleName ( PCHOICELIST, PSZ *, PSZ, SHORT );
static BOOL FillBuffer( PBUFFER, PSZ, ULONG, BOOL ); // fill buffer and write
static BOOL TagExpInit ( PTAGEXPORT );
static BOOL WriteTag( PTAGEXPORT );
static BOOL WriteAttribute( PTAGEXPORT );
static VOID TagExpProcess( HWND, PTAGEXPORT, USHORT );
static VOID TagExpTerm(PTAGEXPORT, HWND);

MRESULT TagExportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

//dialog prototypes
INT_PTR CALLBACK EXPDLG( HWND, WINMSG, WPARAM, LPARAM );
MRESULT ExpCommand( HWND, SHORT, SHORT );

VOID TagTableExport( HWND hwnd, PSZ pSelName )
{
   BOOL            fOK = TRUE;                  // return value
   PTAGEXPORT      pTagExport;                  // export tagtable ida

   //create Export Ida
   fOK = (UtlAlloc( (PVOID *) &pTagExport, 0L, (LONG) sizeof(TAGEXPORT), ERROR_STORAGE ));
   if ( fOK )
   {
      //remember selected tagtable in Ida
      if ( pSelName != NULC )
      {
          strcpy( pTagExport->szName, pSelName );
      } /* endif */
      if ( ! isMarkupExportable( pTagExport->szName, NULL ) ) {
         UtlAlloc( (PVOID *) &pTagExport, 0L, 0L, NOMSG) ;
         fOK = FALSE ;
      }
   } /* endif */

   if (fOK)
   {
      // call standard save as dialog
      {
        BOOL fDone = TRUE;
        BOOL fExtAdded = FALSE;

        OPENFILENAME OpenFileName;

        // get last used values
        {
          EQFINFO       ErrorInfo;                   //error return codes

          UtlMakeEQFPath( pTagExport->szDummy, NULC, SYSTEM_PATH, (PSZ) NULP );   
          pTagExport->hTagListProp = OpenProperties( TAGTABLE_PROPERTIES_NAME, pTagExport->szDummy, PROP_ACCESS_READ, &ErrorInfo );
          if( pTagExport->hTagListProp )
          {
            PPROPTAGTABLE pProp = (PPROPTAGTABLE) MakePropPtrFromHnd( pTagExport->hTagListProp );
            if ( pProp->chTexPath[0] == EOS  )
            {
              strcpy( pProp->chTexPath, pProp->chOldTexPath );
            } /* endif */
            pTagExport->ControlsIda.chSavedDrive = pProp->chTexDrive;
            strcpy( pTagExport->ControlsIda.szSavedPath, pProp->chTexPath );
            pTagExport->ControlsIda.usSavedFormat = EXTERNAL;
          }
        }

        memset( &OpenFileName, 0, sizeof(OpenFileName) );
        OpenFileName.lStructSize        = sizeof(OpenFileName);
        OpenFileName.hwndOwner          = EqfQueryTwbClient();
        OpenFileName.hInstance          = NULLHANDLE;
        OpenFileName.lpstrFilter        = "Exported Tag Table (*.TBX)\0*.TBX\0\0";
        OpenFileName.lpstrCustomFilter  = NULL;
        OpenFileName.nMaxCustFilter     = 0;
        OpenFileName.nFilterIndex       = 1;
        strcpy( pTagExport->ControlsIda.szPathContent, pTagExport->szName );

        OEMTOANSI( pTagExport->ControlsIda.szPathContent );
        OpenFileName.lpstrFile          = pTagExport->ControlsIda.szPathContent;
        OpenFileName.nMaxFile           = sizeof(pTagExport->ControlsIda.szPathContent);
        OpenFileName.lpstrFileTitle     = NULL;
        OpenFileName.nMaxFileTitle      = 0;
        OpenFileName.lpstrInitialDir    = pTagExport->ControlsIda.szSavedPath;
        sprintf( pTagExport->szTitle, "Export Markup Table %s", pTagExport->szName );
        OpenFileName.lpstrTitle         = pTagExport->szTitle;
        OpenFileName.Flags              = OFN_ENABLESIZING | OFN_EXPLORER | OFN_LONGNAMES | OFN_NODEREFERENCELINKS | OFN_NOTESTFILECREATE | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
        OpenFileName.nFileOffset        = 0;
        OpenFileName.nFileExtension     = 0;
        OpenFileName.lpstrDefExt        = "TBX";
        OpenFileName.lCustData          = 0L;
        OpenFileName.lpfnHook           = NULL;
        OpenFileName.lpTemplateName     = NULL;

        do
        {
          fOK = (GetSaveFileName( &OpenFileName ) != 0 );

          if ( fOK )
          {
            // set export file extension if none specified by user
            {
              // find file name within full path
              PSZ pszFileName = strrchr( pTagExport->ControlsIda.szPathContent, '\\' );
              if ( pszFileName != NULL )
              {
                pszFileName++;               
              }
              else
              {
                pszFileName = pTagExport->ControlsIda.szPathContent;
              } /* endif */

              // set default extension if none specified
              if ( strchr( pszFileName, '.' ) == NULL )
              {
                strcat( pTagExport->ControlsIda.szPathContent, ".TBX" );
              } /* endif */
            }

            // get overwrite confirmation (has to be done outside the dialog as the file extension
            // is not available within the SaveAs dialog
            fDone = TRUE;
            if ( fOK && UtlFileExist(pTagExport->ControlsIda.szPathContent)  )
            {
              PSZ pszParms[2]; 

              pszParms[0] = pTagExport->ControlsIda.szPathContent;
              if ( UtlError( FILE_EXISTS, MB_OKCANCEL | MB_DEFBUTTON2, 1, pszParms, EQF_QUERY ) != MBID_OK )
              {
                PSZ pszName; 

                fDone = FALSE;                   // redo SaveAs dialog

                strcpy ( pTagExport->szDummy, pTagExport->ControlsIda.szPathContent );
                pszName = UtlSplitFnameFromPath( pTagExport->szDummy );
                if ( fExtAdded )
                {
                  Utlstrccpy( pTagExport->ControlsIda.szPathContent, pszName, DOT );
                }
                else
                {
                  strcpy( pTagExport->ControlsIda.szPathContent, pszName );
                } /* endif */
              } /* endif */
            } /* endif */

            // prepare export and do some checking
            if ( fDone ) // no problem with overwrite confirmation ?
            {
              USHORT usRc = 0;
              USHORT usAction = 0;

              //get values set in ControlsIda via dialog utilities
              strcpy( pTagExport->szPath, pTagExport->ControlsIda.szPathContent );
              UtlSplitFnameFromPath( pTagExport->szPath );
              strcpy( pTagExport->szTargetName, pTagExport->ControlsIda.szPathContent );
              strncpy( pTagExport->szDrive, pTagExport->ControlsIda.szPathContent, 2 );
              pTagExport->szDrive[2] = 0;

              // save last used values
              if( pTagExport->hTagListProp )
              {
                  if( SetPropAccess( pTagExport->hTagListProp, PROP_ACCESS_WRITE))
                  {
                    EQFINFO ErrorInfo;                  
                    PPROPTAGTABLE pProp = (PPROPTAGTABLE) MakePropPtrFromHnd( pTagExport->hTagListProp);
                    strcpy( pProp->chTexPath, pTagExport->szPath );
                    pProp->chTexDrive = pTagExport->szDrive[0];
                    SaveProperties( pTagExport->hTagListProp, &ErrorInfo);
                  } /* endif */
              } /* endif */
            

              //check for valid file name specification
              usRc = UtlOpen( pTagExport->szTargetName, &(pTagExport->stBuffer.hTarget), &usAction, 0L, FILE_NORMAL,
                              FILE_TRUNCATE | FILE_CREATE, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, FALSE );
              if ( usRc != NO_ERROR )
              {
                //display error message that file specification is incorrect
                PSZ pszError = pTagExport->szTargetName;
                UtlError( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszError, EQF_ERROR );
                fDone = FALSE;
              }

              if ( fDone )
              {
                 if ( !MUGetMarkupTableFileName( pTagExport->szName, NULL, pTagExport->szSourceName, MAX_EQF_PATH ) ) 
                 {
                   fDone = FALSE;
                   /*  TODO   DAW  */
                 }
              }

              if ( fDone )
              {
                //check if enough room for export, get size of MUT
                HDIR hDirHandle = HDIR_CREATE;    
                USHORT usCount = 0;
                usRc = UtlFindFirst( pTagExport->szSourceName, &hDirHandle, FILE_NORMAL, &(pTagExport->ResultBuf), sizeof( pTagExport->ResultBuf), &usCount, 0L, FALSE);
                if ( !usRc )
                {
                  // close file search handle
                  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

                  if ( RESBUFSIZE(pTagExport->ResultBuf) == 0 ) //size = zero bytes
                  {
                      PSZ pszError = pTagExport->szName;
                      UtlError( ERROR_FILE_EXP_SIZE, MB_CANCEL, 1, &pszError, EQF_ERROR );
                      fOK = FALSE;
                  }
                }
                else
                {
                  PSZ pszError = pTagExport->szName;
                  UtlError( ERROR_FILE_ACCESS_ERROR, MB_CANCEL, 1, &pszError, EQF_ERROR );
                  fDone = FALSE;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif fOK */
        } while ( !fDone );
      }
   } /* endif */

   if ( fOK )
   {
     // export for this tagtable already started?
     HWND          hwndExport;

     strcpy( pTagExport->IdaHead.szObjName, pTagExport->szSourceName );
     pTagExport->IdaHead.pszObjName =  pTagExport->IdaHead.szObjName;
     hwndExport = EqfQueryObject( pTagExport->IdaHead.szObjName,
                                  clsTAGTABLE, 0);
     if( hwndExport )
     {
       ActivateMDIChild( hwndExport );
       UtlAlloc( (PVOID *) &pTagExport, 0L, 0L, NOMSG) ;
       fOK = FALSE;
     } /* endif */
   } /* endif */

   // close any open properties
   if ( pTagExport && pTagExport->hTagListProp )
   {
     EQFINFO       ErrorInfo;                   //error return codes
     CloseProperties( pTagExport->hTagListProp, PROP_QUIT, &ErrorInfo );
     pTagExport->hTagListProp = NULL;
   } /* endif */

   if ( fOK && WinIsWindow( (HAB)NULL, hwnd) )
   {
      fOK = CreateProcessWindow( pTagExport->IdaHead.szObjName,
                                 TagExportCallBack, pTagExport );
      if ( !fOK  )
      {
         // free memory of export ida in case of error
         UtlAlloc( (PVOID *) &pTagExport, 0L, 0L, NOMSG) ;
      } /* endif */
   } /* endif */
} /* end of TagTableExport */

MRESULT TagExportCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  PTAGEXPORT      pTagExport;          // pointer to instance area
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
        pTagExport          = (PTAGEXPORT) PVOIDFROMMP2(mp2);
        pCommArea->pUserIDA = pTagExport;
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_TAGEXP_WINDOW;
        pCommArea->sProcessObjClass = clsTAGEXP;
        pCommArea->Style            = PROCWIN_TEXTSLIDER;
        pCommArea->sTextID          = ID_EXPORTTAG_TXT;

        LOADSTRING( NULLHANDLE, hResMod, IDS_TAGEXPORT_TAGTXT, pCommArea->szText );
        strcat( pCommArea->szText, pTagExport->szTargetName );

        pCommArea->sSliderID        = ID_TAGSLIDER;

        LOADSTRING( NULLHANDLE, hResMod, IDS_TAGEXPORT_TITLEBAR, pCommArea->szTitle );
        Utlstrccpy( pCommArea->szTitle + strlen(pCommArea->szTitle),
                    UtlGetFnameFromPath( pTagExport->szSourceName ),
                    DOT );

        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_MARKUPEXPICON); //hiconMARKUPEXP;
        pCommArea->fNoClose         = FALSE;
        pCommArea->swpSizePos.x     = 100;
        pCommArea->swpSizePos.y     = 100;
        pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 60;
        pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 14;
        pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
        pCommArea->asMsgsWanted[1]  = WM_EQF_ABOUTTODELETE;
        pCommArea->asMsgsWanted[2]  = 0;
        pCommArea->usComplete       = 0;
      }
      break;


      /****************************************************************/
      /* Start processing by posting WM_EQF_PROCESSTASK               */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      pTagExport = (PTAGEXPORT) pCommArea->pUserIDA;
      pTagExport->fOK = TagExpInit(pTagExport);    // init export process

      if (pTagExport->fOK)
      {
         WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(WRITE_TAG), 0l );
      }
      else
      {
         WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(CLOSE_PROCESS), 0l );
      } /* endif */
      break;

    /******************************************************************/
    /* WM_CLOSE:                                                      */
    /*                                                                */
    /* Prepare/initialize shutdown of process                         */
    /******************************************************************/
    case WM_CLOSE:
      pTagExport = (PTAGEXPORT) pCommArea->pUserIDA;
      if ( pTagExport )
      {
         pTagExport->fKill = TRUE;
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
      pTagExport = (PTAGEXPORT) pCommArea->pUserIDA;
      if ( pTagExport )
      {
        TagExpTerm( pTagExport, hwnd );            // do cleanup
      } /* endif */
      UtlAlloc( (PVOID *) &pTagExport, 0L, 0L, NOMSG);
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
      pTagExport = (PTAGEXPORT) pCommArea->pUserIDA;
      TagExpProcess( hwnd, pTagExport, SHORT1FROMMP1(mp1) );
      break;

    case WM_EQF_ABOUTTODELETE:
      pTagExport = (PTAGEXPORT) pCommArea->pUserIDA;
      if( SHORTFROMMP1( mp1) == clsTAGTABLE )
      {
        if( !strcmp( (PSZ)mp2, pTagExport->IdaHead.szObjName))
        {
          mResult = (MRESULT)TRUE;    // do not delete while export is running
        } /* endif */
      } /* endif */
      break;

  } /* endswitch */
  return( mResult );
}

static VOID TagExpProcess( HWND hwnd, PTAGEXPORT pTagExport, USHORT usTask)
{
   LONG      lNumber;                           // number of ticks to be set
   USHORT    usRc;                              // return from UtlError

   if (pTagExport->fKill)
   {
      pTagExport->fKill = FALSE;
      usRc = UtlError( ERROR_TEX_CANCEL, MB_YESNO, 0, NULL, EQF_QUERY);

      if (usRc != MBID_NO)
      {
         pTagExport->fOK =  FALSE;
         usTask = CLOSE_PROCESS;
      } /* endif */
   } /* endif */

   switch (usTask)     // switch on different tasks
   {

      case WRITE_TAG:
        // write tag
        if ( pTagExport->usTagCount > 0 )
           pTagExport->fOK = WriteTag(pTagExport);   // write next tag

        if ( pTagExport->fOK ||
             ( *(pTagExport->pTagTable->szSegmentExit)
               && pTagExport->usTagCount == 0 ) )
        {

           if ( pTagExport->usTagCount > 0 )
           {
              // update slider bar window
              lNumber = (LONG)(pTagExport->usCount) * 100L
                        / (LONG)pTagExport->usMarkupCount;
              WinSendMsg( hwnd, WM_EQF_UPDATESLIDER,
                          MP1FROMSHORT((SHORT)lNumber ), NULL );
           } /* endif */

           // determine next task
           if (pTagExport->usCount >= pTagExport->usTagCount)
           {
              pTagExport->usCount = 0;
              if (pTagExport->usAttrCount == 0)
              {
                 usTask = CLOSE_PROCESS;
                 pTagExport->fStop = TRUE;
              }
              else
              {
                 usTask = WRITE_ATTRIBUTE;
              } /* endif */
           } /* endif */
        } /* endif */
        break;

      case WRITE_ATTRIBUTE:
        //write attribute
        if ( pTagExport->usAttrCount > 0 )
           pTagExport->fOK = WriteAttribute(pTagExport);    // write next attribute

        if (pTagExport->fOK)
        {
           //update slider bar window

           lNumber = ((LONG)(pTagExport->usCount + pTagExport->usTagCount)
                     * 100L / (LONG)pTagExport->usMarkupCount );
           WinSendMsg( hwnd, WM_EQF_UPDATESLIDER,
                       MP1FROMSHORT((SHORT)lNumber ), NULL );

           // determine if processing complete
           if (pTagExport->usCount >= pTagExport->usAttrCount)
           {
              usTask = CLOSE_PROCESS;
              pTagExport->fStop = TRUE;
           } /* endif */
        } /* endif */
        break;
   } /* endswitch */

   if ( (usTask == CLOSE_PROCESS) || !pTagExport->fOK )
   {
      EqfRemoveObject(TWBFORCE, hwnd);
   }
   else
   {
      UtlDispatch();
      WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(usTask), 0l );
   } /* endif */
} /* end of TagExpProcess */

static BOOL FillBuffer( PBUFFER  pstBuffer,   // pointer to buffer structure
                        PSZ      pData,      // string to be written
                        ULONG    ulLength,   // length of string
                        BOOL     fLast   )   // last call
{
   BOOL     fOK = TRUE;                      // process indicator
   ULONG   ulFree;                          // number of bytes still free
   ULONG   ulWrite;                         // number of bytes to be filled
   USHORT   usDosRc;                         // return code from Dosfunctions
   ULONG   ulWritten;                       // written bytes

   while ( (( ulLength > 0) || fLast) && fOK )
   {
      ulFree = pstBuffer->pEnd - pstBuffer->pCurrent;// number of bytes left in buffer
      ulWrite = min(ulFree, ulLength);       // number to be written
      memcpy( pstBuffer->pCurrent, pData, ulWrite);
      pstBuffer->pCurrent += ulWrite;              // update end pointer
      ulLength      -= ulWrite;
      pData         += ulWrite;

      if ( ( ulWrite < ulLength ) || fLast )
      {
         ulWrite = pstBuffer->pCurrent - pstBuffer->pStart;
         usDosRc = UtlWriteL( pstBuffer->hTarget,
                             pstBuffer->pStart,
                             ulWrite,
                             &ulWritten,
                             TRUE );
         fOK = ((usDosRc == 0) && (ulWrite == ulWritten));
         if ( !fOK )
         {
            UtlError( ERROR_PROCESS_STOPPED, MB_CANCEL, 0, NULL, EQF_ERROR );
         }

         pstBuffer->pCurrent = pstBuffer->pStart; // reset buffer
         fLast         = FALSE;              // last write out done - Stop!
      } /* endif */
   } /* endwhile */
   return ( fOK );                           // return result of write
}

static
BOOL  GetStyleName ( PCHOICELIST  pChoice,         // allowed values
                     PSZ          * ppStyleName,   // character string found
                     PSZ          pTagString,      // string of current tag
                     SHORT        sValue )        // found style value
{
   BOOL    fFound = FALSE;                      // currently no entry found
   BOOL    fOK = TRUE;                          // success indicator
   USHORT  usResult;                            // return from UtlError

   fFound = FALSE;

   *ppStyleName = NULL;
   while ( (pChoice->Style != LAST_STYLE) && !fFound )
   {

      if (AttrStyles[pChoice->Style].sValue == sValue )
      {
         fFound = TRUE;
         *ppStyleName = AttrStyles[pChoice->Style].szName;
      }
      else
      {
         pChoice ++;
      } /* endif */
   } /* endwhile */

    if ( !fFound )
    {
       usResult = UtlError( ERROR_TAGIMP_INVSTYLE,
                            MB_YESNOCANCEL,
                            1,
                            &(pTagString),
                            EQF_QUERY);
       fOK = ( usResult == MBID_YES );       // user wants to goon
    } /* endif */

   return ( fOK );                              // pass back the value
} /* end of GetStyleName */

static
BOOL TagExpInit (PTAGEXPORT  pTagExport)
{
   BOOL          fOK;                 // processing state
   INT           i;                   // index variable
   PTAGTABLE     pTagTable;           // pointer to tagtable
   ULONG         ulLength;            // length of string to be stored
   ULONG         ulBytesRead;

   pTagTable = NULL;

   fOK =  UtlLoadFileL(pTagExport->szSourceName, (PVOID *)&(pTagTable),
                      &ulBytesRead, FALSE, TRUE);

   if (fOK)
   {
      pTagExport->pOffset = pTagTable->uTagNames +
                            (PBYTE)pTagTable;
      pTagExport->fStop = FALSE;
      pTagExport->usCount = 0;
      pTagExport->usTagCount = pTagTable->uNumTags;
      pTagExport->pTag = (PTAG) ( ((PBYTE) (pTagTable)) +
                                    pTagTable->stFixTag.uOffset);
      pTagExport->pAttribute = (PATTRIBUTE) ( ((PBYTE) (pTagTable)) +
                                   pTagTable->stAttribute.uOffset);


      pTagExport->usAttrCount = pTagTable->stAttribute.uNumber;
      for ( i=0 ; i <= 26 ; i++ )
      {
          pTagExport->usAttrCount = pTagExport->usAttrCount +
                   pTagTable->stAttributeIndex[i].uNumber;
      } /* endfor */

      pTagExport->usMarkupCount = pTagExport->usTagCount +
                                  pTagExport->usAttrCount;

      // allocate buffer for sgml file
      fOK = UtlAlloc( (PVOID *) &(pTagExport->stBuffer.pStart),
                      0L,
                      (LONG) BUFFERSIZE ,
                      ERROR_STORAGE);

   } /* endif */

   if (fOK)
   {
      pTagExport->stBuffer.pCurrent = pTagExport->stBuffer.pStart;
      pTagExport->stBuffer.pEnd = pTagExport->stBuffer.pStart + BUFFERSIZE - 1;
      if (pTagExport->usTagCount > 0)
      {
        pTagExport->Task = WRITE_TAG;
      }
      else if (pTagExport->usAttrCount > 0)
      {
        pTagExport->Task = WRITE_ATTRIBUTE;
      }
      else
      {
         if (!*(pTagTable->szSegmentExit))
         {
            fOK = FALSE;
            //issue error message that tagtable is empty
            UtlError(ERROR_TABLE_EMPTY, MB_CANCEL, 0, NULL, EQF_ERROR);
         } /* endif */
      } /* endif */
   } /* endif */

   if (fOK)
   {  // allocate buffer for formatted string
      fOK = UtlAlloc( (PVOID *) &(pTagExport->pString),
                      0L,
                      (LONG) BUFFERSIZE ,
                      ERROR_STORAGE);
   } /* endif */

   if (fOK)
   {
      // write tagtable tag
      ulLength = sprintf( pTagExport->pString,
                          "<%s>\r\n",
                          TokenTable[TAGTABLE_TOKEN].szName);
      fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                        ulLength, FALSE );
   } /* endif */

   if (fOK)
   {
      // write segmentexit tag
      if (*(pTagTable->szSegmentExit))
      {
          ulLength = sprintf( pTagExport->pString,
                              "<%s>%s<%s>\r\n",
                              TokenTable[SEGEXIT_TOKEN].szName,
                              pTagTable->szSegmentExit,
                              TokenTable[ESEGEXIT_TOKEN].szName);
          fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                            ulLength, FALSE );
      } /* endif */
      /****************************************************************/
      /* write TRNote Tagging, i.e. the <TRNOTE>, the start text,     */
      /* the endtext and the preference settings (all are part of     */
      /* our tagtable structure and filled during Tagtable import).   */
      /*   <TRNOTE>                                                   */
      /*     <TRTAGSTART>:ANNOT</TRTAGSTART>                          */
      /*     <TRTAGEND>:EANNOT</TRTAGEND>                             */
      /*     <TRPREF>TRNOTE1:</TRPREF>                                */
      /*     <TRPREF>TRNOTE2:</TRPREF>                                */
      /*   </TRNOTE>                                                  */
      /*                                                              */
      /****************************************************************/
      if ( fOK && (pTagTable->usVersion >= ADDINFO_VERSION) &&
          (*(pTagTable->chTrnote1Text) || *(pTagTable->chTrnote2Text)) )
      {
          ulLength = sprintf( pTagExport->pString,
                              "<%s>\r\n",
                              TokenTable[TRNOTE_TOKEN].szName);
          fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                            ulLength, FALSE );

          /************************************************************/
          /* fill in relevant tagging information                     */
          /************************************************************/
          if (fOK)
          {
            ulLength = sprintf( pTagExport->pString,
                                "  <%s>%s<%s>\r\n",
                                TokenTable[TRTAGSTART_TOKEN].szName,
                                pTagTable->chStartText,
                                TokenTable[ETRTAGSTART_TOKEN].szName);
            fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                              ulLength, FALSE );
          } /* endif */

          if (fOK)
          {
            ulLength = sprintf( pTagExport->pString,
                                "  <%s>%s<%s>\r\n",
                                TokenTable[TRTAGEND_TOKEN].szName,
                                pTagTable->chEndText,
                                TokenTable[ETRTAGEND_TOKEN].szName);
            fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                              ulLength, FALSE );
          } /* endif */


         /*************************************************************/
         /* fill in TRPref1                                           */
         /*************************************************************/
         if ( fOK )
         {
            ulLength = sprintf( pTagExport->pString,
                                "  <%s>%s<%s>\r\n",
                                TokenTable[TRPREF_TOKEN].szName,
                                pTagTable->chTrnote1Text,
                                TokenTable[ETRPREF_TOKEN].szName);
            fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                              ulLength, FALSE );
         } /* endif */
         if ( fOK )
         {
            ulLength = sprintf( pTagExport->pString,
                                "  <%s>%s<%s>\r\n",
                                TokenTable[TRPREF_TOKEN].szName,
                                pTagTable->chTrnote2Text,
                                TokenTable[ETRPREF_TOKEN].szName);
            fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                              ulLength, FALSE );
         } /* endif */
         if ( fOK )
         {
          ulLength = sprintf( pTagExport->pString,
                              "<%s>\r\n",
                              TokenTable[ETRNOTE_TOKEN].szName);
          fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                            ulLength, FALSE );
         } /* endif */
      } /* endif */

      /****************************************************************/
      /* write new fields started in version 3 of tag tables:         */
      /*   <DESCRIPTION>description</DESCRIPTION>                     */
      /*   <DESCRNAME>descriptive name</DESCRNAME>                    */
      /*   <CHARSET>charset</CHARSET>                                 */
      /*   <SINGLESUBST>char/SINGLESUBST>                             */
      /*   <MULTSUBST>char</MULTSUBST>                                */
      /*   <USEUNICODE>YES|NO</USEUNICODE>                            */
      /*                                                              */
      /****************************************************************/
      if ( fOK && (pTagTable->usVersion >= TAGTABLE_VERSION3) )
      {
        PSZ   pStyle;
        PSZ   pTagString;
        TAG   Tag;

        Tag = pTagExport->pTag[pTagExport->usCount];
        pTagString =  (PSZ)(Tag.uTagnameOffs + pTagExport->pOffset);

        if ( pTagTable->szDescription[0] )
        {
          ulLength = sprintf( pTagExport->pString,
                              "  <%s>%s<%s>\r\n",
                              TokenTable[DESCRIPTION_TOKEN].szName,
                              pTagTable->szDescription,
                              TokenTable[EDESCRIPTION_TOKEN].szName);
          fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                            ulLength, FALSE );
        } /* endif */

        if ( fOK && pTagTable->szDescriptiveName[0] )
        {
          ulLength = sprintf( pTagExport->pString,
                              "  <%s>%s<%s>\r\n",
                              TokenTable[DESCRNAME_TOKEN].szName,
                              pTagTable->szDescriptiveName,
                              TokenTable[EDESCRNAME_TOKEN].szName);
          fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                            ulLength, FALSE );
        } /* endif */

        // put out single and multiple substitution characters
        if ( fOK && pTagTable->chMultSubst )
        {
          ulLength = sprintf( pTagExport->pString,
                              "  <%s>%c<%s>\r\n",
                              TokenTable[MULTSUBST_TOKEN].szName,
                              pTagTable->chMultSubst,
                              TokenTable[EMULTSUBST_TOKEN].szName);
          fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                            ulLength, FALSE );
        } /* endif */
        if ( fOK && pTagTable->chSingleSubst )
        {
          ulLength = sprintf( pTagExport->pString,
                              "  <%s>%c<%s>\r\n",
                              TokenTable[SINGLESUBST_TOKEN].szName,
                              pTagTable->chSingleSubst,
                              TokenTable[ESINGLESUBST_TOKEN].szName);
          fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                            ulLength, FALSE );
        } /* endif */

        // select unicode style token
        if ( fOK && pTagTable->fUseUnicodeForSegFile)
        {
          fOK = GetStyleName ( YESNOSTYLES, &pStyle, pTagString, (SHORT)pTagTable->fUseUnicodeForSegFile);

          if (pStyle && fOK)
          {
            ulLength = sprintf( pTagExport->pString,
                                "  <%s>%s<%s>\r\n",
                                 TokenTable[USEUNICODE_TOKEN].szName,
                                 pStyle,
                                 TokenTable[EUSEUNICODE_TOKEN].szName);

            fOK = FillBuffer( &(pTagExport->stBuffer),
                              pTagExport->pString, ulLength, FALSE );
          } /* endif */
        } /* endif */

        // select CHARSET token and write it out
        if ( fOK )
        {
          fOK = GetStyleName ( CHARSETSTYLES, &pStyle, pTagString, pTagTable->usCharacterSet);

          if (pStyle && fOK)
          {
            ulLength = sprintf( pTagExport->pString,
                                "  <%s>%s<%s>\r\n",
                                 TokenTable[CHARSET_TOKEN].szName,
                                 pStyle,
                                 TokenTable[ECHARSET_TOKEN].szName);

            fOK = FillBuffer( &(pTagExport->stBuffer),
                              pTagExport->pString, ulLength, FALSE );
          } /* endif */
        } /* endif */
        // select reflow style token (YES== Reflow=1, NO == Reflow = 2, NotSpecified00 Reflow=0)
        if ( fOK && pTagTable->fReflow)
        {
          fOK = GetStyleName ( REFLOWYESNOSTYLES, &pStyle, pTagString, (SHORT)pTagTable->fReflow);

          if (pStyle && fOK)
          {
            ulLength = sprintf( pTagExport->pString,
                                "  <%s>%s<%s>\r\n",
                                 TokenTable[REFLOW_TOKEN].szName,
                                 pStyle,
                                 TokenTable[EREFLOW_TOKEN].szName);

            fOK = FillBuffer( &(pTagExport->stBuffer),
                              pTagExport->pString, ulLength, FALSE );
          } /* endif */
        } /* endif */
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* support extended tags..                                         */
   /*******************************************************************/
   if ( fOK && (pTagTable->usVersion >= ADDINFO_VERSION))
   {
      pTagExport->pTagAddInfos = (PTAGADDINFO)((PBYTE)pTagTable + pTagTable->uAddInfos);
   }
   else
   {
     pTagExport->pTagAddInfos = NULL;
   } /* endif */
   // save pointer to tagtable
   pTagExport->pTagTable = pTagTable;

   return (fOK);
} /* end of function TagExpInit */

static
BOOL WriteTag(PTAGEXPORT  pTagExport)
{
   BOOL          fOK = TRUE;
   PSZ           pTagString;
   PSZ           pStyle;
   CHAR          szIntBuffer[20];
   ULONG         ulLength;
   TAG           Tag;
   TAGADDINFO    TagAddInfo;

   Tag = pTagExport->pTag[pTagExport->usCount];
   pTagString =  (PSZ)(Tag.uTagnameOffs + pTagExport->pOffset);

   // write tag name
   ulLength = sprintf( pTagExport->pString,
                       "<%s>\r\n",
                       TokenTable[TAG_TOKEN].szName);
   fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                     ulLength, FALSE );

   if (fOK)
   {
     // write tag string
     TagInsEscapeChars( (PSZ)(pTagExport->pOffset + Tag.uTagnameOffs),
                        pTagExport->szDummy );

     ulLength = sprintf( pTagExport->pString,
                         "  <%s>%s<%s>\r\n",
                         TokenTable[STRING_TOKEN].szName,
                         pTagExport->szDummy,
                         TokenTable[ESTRING_TOKEN].szName);

     fOK = FillBuffer( &(pTagExport->stBuffer),
                       pTagExport->pString, ulLength, FALSE );
   } /* endif */

   if (fOK)
   {
     // write tag end delimiters
     if (*(Tag.uEndDelimOffs + pTagExport->pOffset) != EOS)
     {
       TagInsEscapeChars( (PSZ)(pTagExport->pOffset + Tag.uEndDelimOffs),
                          pTagExport->szDummy );

       ulLength = sprintf( pTagExport->pString,
                           "  <%s>%s<%s>\r\n",
                           TokenTable[ENDDELIM_TOKEN].szName,
                           pTagExport->szDummy,
                           TokenTable[EENDDELIM_TOKEN].szName);

       fOK = FillBuffer( &(pTagExport->stBuffer),
                         pTagExport->pString, ulLength, FALSE );
     } /* endif */
   } /* endif */

   if (fOK)
   {
     // write TagType
     fOK = GetStyleName (  TYPESTYLES, &pStyle, pTagString, Tag.Tagtype);

     if (pStyle && fOK)
     {
       if (Tag.Tagtype != STARTDEL_TAG)
       {
         ulLength = sprintf( pTagExport->pString,
                             "  <%s>%s<%s>\r\n",
                             TokenTable[TYPE_TOKEN].szName,
                             pStyle,
                             TokenTable[ETYPE_TOKEN].szName);

         fOK = FillBuffer( &(pTagExport->stBuffer),
                           pTagExport->pString, ulLength, FALSE );
       } /* endif */
     } /* endif */
   } /* endif */

   if (fOK)
   {
     // write Segmentinfo
     fOK = GetStyleName (  SEGSTYLES, &pStyle, pTagString, Tag.Segbreak);

     if (pStyle && fOK)
     {
       if (Tag.Segbreak != SEG_NEUTRAL)
       {
         ulLength = sprintf( pTagExport->pString,
                             "  <%s>%s<%s>\r\n",
                             TokenTable[SEGINFO_TOKEN].szName,
                             pStyle,
                             TokenTable[ESEGINFO_TOKEN].szName);

         fOK = FillBuffer( &(pTagExport->stBuffer),
                           pTagExport->pString, ulLength, FALSE );
       } /* endif*/
     } /* endif */
   } /* endif */

   if (fOK)
   {
     // write associated text info
     fOK = GetStyleName ( ASSTEXTSTYLES, &pStyle, pTagString, Tag.Asstext);

     if (pStyle && fOK)
     {
       if (Tag.Asstext != NOEXPL_TEXT)
       {
         ulLength = sprintf( pTagExport->pString,
                             "  <%s>%s<%s>\r\n",
                             TokenTable[ASSTEXT_TOKEN].szName,
                             pStyle,
                             TokenTable[EASSTEXT_TOKEN].szName);

         fOK = FillBuffer( &(pTagExport->stBuffer),
                           pTagExport->pString, ulLength, FALSE );
       } /* endif */
     } /* endif */
   } /* endif */

   if (fOK)
   {
     // write attribute info
     fOK = GetStyleName ( YESNOSTYLES, &pStyle, pTagString, Tag.fAttr);

     if (pStyle && fOK)
     {
       if (Tag.fAttr == TRUE)
       {
         ulLength = sprintf( pTagExport->pString,
                             "  <%s>%s<%s>\r\n",
                             TokenTable[ATTRINFO_TOKEN].szName,
                             pStyle,
                             TokenTable[EATTRINFO_TOKEN].szName);

         fOK = FillBuffer( &(pTagExport->stBuffer),
                           pTagExport->pString, ulLength, FALSE );
       } /* endif */
     } /* endif */
   } /* endif */

   if (fOK)
   {
     // write length of tag
     pStyle = itoa(Tag.usLength, szIntBuffer, 10);
     if (Tag.usLength != 0)
     {
       ulLength = sprintf( pTagExport->pString,
                           "  <%s>%s<%s>\r\n",
                           TokenTable[LENGTH_TOKEN].szName,
                           pStyle,
                           TokenTable[ELENGTH_TOKEN].szName);

       fOK = FillBuffer( &(pTagExport->stBuffer),
                         pTagExport->pString, ulLength, FALSE );
     } /* endif */
   } /* endif */

   if (fOK)
   {
     // write column position
     pStyle = itoa(Tag.usPosition, szIntBuffer, 10);
     if (Tag.usPosition != 0)
     {
       ulLength = sprintf( pTagExport->pString,
                           "  <%s>%s<%s>\r\n",
                           TokenTable[COLUMPOS_TOKEN].szName,
                           pStyle,
                           TokenTable[ECOLUMPOS_TOKEN].szName);

       fOK = FillBuffer( &(pTagExport->stBuffer),
                         pTagExport->pString, ulLength, FALSE );
     } /* endif */
   } /* endif */

   if (fOK)
   {
     // write translate info
     SHORT sStyle = Tag.BitFlags.fTranslate;

     fOK = GetStyleName ( YESNOSTYLES, &pStyle, pTagString, sStyle );

     if (pStyle)
     {
       if (Tag.BitFlags.fTranslate == TRUE)
       {
         ulLength = sprintf( pTagExport->pString,
                             "  <%s>%s<%s>\r\n",
                             TokenTable[TRANSINFO_TOKEN].szName,
                             pStyle,
                             TokenTable[ETRANSINFO_TOKEN].szName);

         fOK = FillBuffer( &(pTagExport->stBuffer),
                           pTagExport->pString, ulLength, FALSE );
       } /* endif */
     } /* endif */
   } /* endif */

   if ( fOK && (Tag.BitFlags.AddInfo != 0) )
   {
     // write additional tag info
     pStyle = itoa(Tag.BitFlags.AddInfo, szIntBuffer, 10);
     ulLength = sprintf( pTagExport->pString,
                         "  <%s>%s<%s>\r\n",
                         TokenTable[ADDINFO_TOKEN].szName,
                         pStyle,
                         TokenTable[EADDINFO_TOKEN].szName);

     fOK = FillBuffer( &(pTagExport->stBuffer),
                       pTagExport->pString, ulLength, FALSE );
   } /* endif */

   if (fOK && pTagExport->pTagAddInfos )
   {
     TagAddInfo = pTagExport->pTagAddInfos[pTagExport->usCount];
     /*****************************************************************/
     /* write CLASSID if necessary                                    */
     /*****************************************************************/
     if ( TagAddInfo.ClassId != CLS_DEFAULT )
     {
       fOK = GetStyleName (  CLASSIDSTYLES, &pStyle, pTagString,
                             TagAddInfo.ClassId);
       if ( fOK )
       {
         ulLength = sprintf( pTagExport->pString,
                             "  <%s>%s<%s>\r\n",
                             TokenTable[CLASSID_TOKEN].szName,
                             pStyle,
                             TokenTable[ECLASSID_TOKEN].szName);

         fOK = FillBuffer( &(pTagExport->stBuffer),
                           pTagExport->pString, ulLength, FALSE );
       } /* endif */
     } /* endif */
     /*****************************************************************/
     /* write tag TOKENID if necessary..                              */
     /*****************************************************************/
     if (fOK && (TagAddInfo.usFixedTagId != 0) )
     {
       // write external unique id
       pStyle = itoa(TagAddInfo.usFixedTagId, szIntBuffer, 10);
       ulLength = sprintf( pTagExport->pString,
                           "  <%s>%s<%s>\r\n",
                           TokenTable[TOKENID_TOKEN].szName,
                           pStyle,
                           TokenTable[ETOKENID_TOKEN].szName);

       fOK = FillBuffer( &(pTagExport->stBuffer),
                         pTagExport->pString, ulLength, FALSE );
     } /* endif */
   } /* endif */

   if (fOK)
   {

     ulLength = sprintf( pTagExport->pString,
                         "<%s>\r\n",
                         TokenTable[ETAG_TOKEN].szName);

     fOK = FillBuffer( &(pTagExport->stBuffer),
                       pTagExport->pString, ulLength, FALSE );
   } /* endif */

   (pTagExport->usCount) ++;

   return (fOK);

} /* end of function WriteTag */

static
BOOL WriteAttribute( PTAGEXPORT pTagExport)
{
  BOOL          fOK;
  ATTRIBUTE     Attribute;
  ULONG         ulLength;
  PSZ           pStyle = NULL;
  CHAR      szIntBuffer[20];
  PSZ       pTagString = TokenTable[ATTRIBUTE_TOKEN].szName;

  Attribute = pTagExport->pAttribute[pTagExport->usCount];
  // write attribute name
  ulLength = sprintf( pTagExport->pString,
                      "<%s>\r\n",
                      TokenTable[ATTRIBUTE_TOKEN].szName);
  fOK = FillBuffer( &(pTagExport->stBuffer),
                    pTagExport->pString, ulLength, FALSE );

  if (fOK)
  {
     // write attribute string
     TagInsEscapeChars( (PSZ)(pTagExport->pOffset + Attribute.uStringOffs),
                        pTagExport->szDummy );

     ulLength = sprintf( pTagExport->pString,
                         "  <%s>%s<%s>\r\n",
                         TokenTable[STRING_TOKEN].szName,
                         pTagExport->szDummy,
                         TokenTable[ESTRING_TOKEN].szName);

     fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                      ulLength, FALSE );
  } /* endif */

  if (fOK)
  {
     // write attribute end delimiters
     if (*(Attribute.uEndDelimOffs + pTagExport->pOffset) != EOS)
     {
       TagInsEscapeChars( (PSZ)(pTagExport->pOffset + Attribute.uEndDelimOffs),
                          pTagExport->szDummy );
       ulLength = sprintf( pTagExport->pString,
                           "  <%s>%s<%s>\r\n",
                           TokenTable[ENDDELIM_TOKEN].szName,
                           pTagExport->szDummy,
                           TokenTable[EENDDELIM_TOKEN].szName);

       fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                           ulLength, FALSE );
     } /* endif */
  } /* endif */

  if (fOK)
  {
     // write attribute length
     pStyle = itoa(Attribute.usLength, szIntBuffer, 10);
     if (Attribute.usLength != 0)
     {
       ulLength = sprintf( pTagExport->pString,
                           "  <%s>%s<%s>\r\n",
                           TokenTable[LENGTH_TOKEN].szName,
                           pStyle,
                           TokenTable[ELENGTH_TOKEN].szName);

       fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                         ulLength, FALSE );
     } /* endif */
  } /* endif */


  if (fOK)
  {
     // write attribute translate info
     SHORT sStyle = Attribute.BitFlags.fTranslate;

     fOK = GetStyleName ( YESNOSTYLES, &pStyle, pTagString, sStyle );

      if (pStyle && fOK)
      {
        if (Attribute.BitFlags.fTranslate == TRUE)
        {
          ulLength = sprintf( pTagExport->pString,
                              "  <%s>%s<%s>\r\n",
                              TokenTable[TRANSINFO_TOKEN].szName,
                              pStyle,
                              TokenTable[ETRANSINFO_TOKEN].szName);

          fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                            ulLength, FALSE );
        } /* endif */
      } /* endif */
  } /* endif */

  if (fOK)
  {
     // write additional info of attribute
     pStyle = itoa(Attribute.BitFlags.AddInfo, szIntBuffer, 10);
     if (Attribute.BitFlags.AddInfo != 0)
     {
       ulLength = sprintf( pTagExport->pString,
                           "  <%s>%s<%s>\r\n",
                           TokenTable[ADDINFO_TOKEN].szName,
                           pStyle,
                           TokenTable[EADDINFO_TOKEN].szName);

       fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                         ulLength, FALSE );
     } /* endif */
  } /* endif */


  if (fOK)
  {
     // write end attribute
     ulLength = sprintf( pTagExport->pString,
                         "<%s>\r\n",
                         TokenTable[EATTRIBUTE_TOKEN].szName,
                         pStyle );

     fOK = FillBuffer( &(pTagExport->stBuffer), pTagExport->pString,
                       ulLength, FALSE );
  } /* endif */

  pTagExport->usCount++;
  return (fOK);
} /* end of function WriteAttribute */

static
VOID TagExpTerm(PTAGEXPORT pTagExport, HWND hwnd)
{
   ULONG      ulLength;                  // length of string
   BOOL       fOK;

   fOK = pTagExport->fOK && (pTagExport->fStop);

   if (fOK)  // export terminated normally
   {
      ulLength = sprintf( pTagExport->pString,
                          "<%s>\r\n",
                          TokenTable[ETAGTABLE_TOKEN].szName);
      fOK = FillBuffer( &(pTagExport->stBuffer),
                        pTagExport->pString, ulLength, TRUE );
   } /* endif */

   /***************************************************************/
   /* close file before the completion message appears            */
   /* (fix for KIT0033)                                           */
   /***************************************************************/
   if (pTagExport->stBuffer.hTarget != NULLHANDLE )
   {
     UtlClose( pTagExport->stBuffer.hTarget, FALSE );
     pTagExport->stBuffer.hTarget = NULLHANDLE;
   } /* endif */

   if (fOK)
   {
     //set slider to 100%
     WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(100), NULL );

     //issue message that export has been completed
     UtlError (ERROR_TEX_COMPLETE, MB_OK, 0, NULL, EQF_INFO);
   } /* endif */

   //free allocated memory
   UtlAlloc( (PVOID *)&(pTagExport->stBuffer.pStart), 0L, 0L, NOMSG);

   UtlAlloc( (PVOID *)&(pTagExport->pString), 0L, 0L, NOMSG);

   UtlAlloc( (PVOID *)&(pTagExport->pTagTable), 0L, 0L, NOMSG);

   //if processing went wrong, delete target file
   if (!fOK)
   {
      UtlDelete( pTagExport->szTargetName, 0L , FALSE );
   } /* endif */
} /* end of TagExpTerminate */

/**********************************************************************/
/* TagInsEscapeChars                                                  */
/* Replace the following specific characters with escape sequences    */
/* and place single quotes around the string if first character is    */
/* a blank                                                            */
/*   \          ==> \\                                                */
/*   0x0D (CR)  ==> \r                                                */
/*   0x0A (LF)  ==> \n                                                */
/*   0x09 (Tab) ==> \t                                                */
/*   other chars below 0x20 ==> \xCC where CC is the hexadecimal      */
/*                                   representation of the character  */
/**********************************************************************/
SHORT TagInsEscapeChars
(
  PSZ         pszIn,                   // ptr to input string
  PSZ         pszOut                   // ptr to output buffer
)
{
  BOOL        fQuotes = FALSE;
  LONG        lLen = strlen(pszIn);
  PBYTE       pbIn = (PBYTE)pszIn;

  /********************************************************************/
  /* We need quotes around the string if the string starts or ends    */
  /* with blanks or contains quotes                                   */
  /********************************************************************/
  if ( lLen )
  {
    /******************************************************************/
    /*                                                                */
    /******************************************************************/
    if ( (pbIn[0] == SPACE) || (pbIn[lLen-1] == SPACE) )
    {
      fQuotes = TRUE;
    }
    else
    {
      PSZ pszTemp = (PSZ)pbIn;
      while ( (*pszTemp != EOS) && (*pszTemp != QUOTE) )
      {
        pszTemp++;
      } /* endwhile */
      fQuotes = (*pszTemp != EOS);
    } /* endif */
  } /* endif */

  // add starting quote if required
  if ( fQuotes ) *pszOut++ = QUOTE;

  // scan input data and copy to output buffer
  while ( *pbIn != EOS )
  {
    if ( *pbIn == QUOTE )
    {
      *pszOut++ = QUOTE;
      *pszOut++ = QUOTE;
      pbIn++;
    }
    else if ( *pbIn == CR )
    {
      *pszOut++ = BACKSLASH;
      *pszOut++ = 'r';
      pbIn++;
    }
    else if ( *pbIn == LF )
    {
      *pszOut++ = BACKSLASH;
      *pszOut++ = 'n';
      pbIn++;
    }
    else if ( *pbIn == '\t' )
    {
      *pszOut++ = BACKSLASH;
      *pszOut++ = 't';
      pbIn++;
    }
    else if ( *pbIn == BACKSLASH )
    {
      *pszOut++ = BACKSLASH;
      *pszOut++ = BACKSLASH;
      pbIn++;
    }
    else if ( *pbIn < 0x20 )
    {
      CHAR szHexBuf[5];

      *pszOut++ = BACKSLASH;
      *pszOut++ = 'x';
      sprintf( szHexBuf, "%2.2X", *pbIn++ );
      *pszOut++ = szHexBuf[0];
      *pszOut++ = szHexBuf[1];
    }
    else
    {
      *pszOut++ = *pbIn++;
    } /* endif */
  } /* endwhile */

  // add closing quote if required
  if ( fQuotes ) *pszOut++ = QUOTE;

  // add string terminator
  *pszOut = EOS;

  return( 0 );
} /* end of function TagInsEscapeChars */

