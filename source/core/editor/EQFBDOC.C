/*! \file
	Description: This module contains the routines that work on documents and lines.

	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_TM               // public translation memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdoc00.h"             // private document handler include file
//#define INIT_TABLES               // forces EQFBCONF.H to initialize data
#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFB.ID"                // Translation Processor IDs
#include <eqfentity.h>            // entity processing defines

#include "EQFHLOG.H"                 // defines for history log processing

static VOID EQFBSetStatus ( PTBDOCUMENT );
static void   EQFBPrepTRNoteSegs(PTBDOCUMENT, PTBDOCUMENT);
static SHORT EQFBAllocSeg ( PTBSEGMENT, PTBSEGMENT );
static VOID  EQFBAddLFSeg ( PTBDOCUMENT, PTBSEGMENT, PTBSEGMENT, USHORT );
static VOID  EQFBGotoTRNoteSeg ( PTBDOCUMENT, ULONG );
static ULONG EQFBFindTRNoteSeg ( PTBDOCUMENT, ULONG );
static VOID EQFBCompactPlusOne( PTBSEGMENT, PSTARTSTOP, USHORT, PUSHORT );
static USHORT EQFBCheckCompactLF (PTBDOCUMENT,PTBSEGMENT,USHORT);
static VOID    EQFBReparseStartStop( PSTARTSTOP *, USHORT, SHORT);
static VOID    EQFBReparseHLType( PSTARTSTOP *, USHORT, SHORT, PTBSEGMENT);
USHORT EQFBFilePrint ( PTBDOCUMENT  pDoc );




//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBDocLoad
//-----------------------------------------------------------------------------
// Function call:     EQFBDocLoad(PLOADSTRUCT)
//-----------------------------------------------------------------------------
// Description:       load a new document from disk if a disk file exists
//                    otherwise a new blank document is created
//-----------------------------------------------------------------------------
// Parameters:        PLOADSTRUCT pLoad    structure containing doc's name
//-----------------------------------------------------------------------------
// Returncode type:   SHORT
//-----------------------------------------------------------------------------
// Returncodes:       sRc   0 success
//-----------------------------------------------------------------------------
// Function flow:     insert a new document into the ring
//                    load document from disk
//                    if either the name is empty string or file
//                    cannot be found a new blank doc is created
//                    if ok so far
//                      set flags and window size
//                      set row and columnsof currently available size
//                      goto begin of file
//                    endif
//-----------------------------------------------------------------------------
 SHORT EQFBDocLoad( PLOADSTRUCT pLoad)
 {
   SHORT  sRC;                         // functions's return value
   PSZ    pName;                       // pointer to file name
   BOOL   fAutoOld;                    // remember fAUtoLineWRap
//   PLOADEDTABLE pLoadedTable;

//   pLoadedTable = (PLOADEDTABLE) pLoad->pDoc->pQFTagTable;

   /* First insert a new document into the ring */
   if ((sRC = EQFBDocInsert( pLoad )) != 0)
      return sRC;

   /* The document's name may have been changed when it was created */
   /* to ensure it is in an operating system unique form.  Pick up  */
   /* the new name.                                                 */
   pName = pLoad->pDoc->szDocName;

   /* Now the document is loaded from disk.  If either the name is  */
   /* empty string, or the file cannot be found on disk a new blank */
   /* document is created.                                          */
   if ( *pName != '\0' )
   {
      sRC = EQFBFileReadExW(pName, pLoad->pDoc, 0);
   } /* endif */

   // load any entity information
   if ( pLoad->docType == STARGET_DOC )
   {
     if ( UtlQueryUShort( QS_ENTITYPROCESSING ) )
     {
       if ( isEntityMarkup( pLoad->pszTagTable ) )
       {
         CHAR szFolObjName[MAX_EQF_PATH];

         strcpy( szFolObjName, pLoad->pFileName );
         UtlSplitFnameFromPath( szFolObjName );
         UtlSplitFnameFromPath( szFolObjName );

         pLoad->pDoc->lEntity = LoadEntities( szFolObjName );
       } /* endif */
     } /* endif */
   } /* endif */


   // If loading was successful, set 'changes' flag false and set window size
   if (sRC >= 0)
   {
     KEYPROFTABLE* pKeyTable = get_KeyTable();
	 USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
     EQFBAdaptKeyTable(pKeyTable, pLoad->pDoc);
     pLoad->pDoc->flags.changed = FALSE;
     pLoad->pDoc->pBlockMark = get_EQFBBlockMark(); // point to blockmark struct
     pLoad->pDoc->pUserSettings = pEQFBUserOpt;   // point to user settings
     pLoad->pDoc->pUserSettings->fNoCUASel = 0;    // assure that it is 0 always
     pEQFBUserOpt->fNoCUASel = 0;
     EQFBBidiLRSwap( FALSE );        // reset bidi flags -- will be set
                                                  // later on again if we deal with a bidi file
     pLoad->pDoc->pstEQFGen  = pLoad->pstEQFGen;  // set pointer to generic struct

     fAutoOld = pLoad->pDoc->fAutoLineWrap;
     pLoad->pDoc->fAutoLineWrap = FALSE;

     EQFBDocPosOnScreen( pLoad );
     SetScrollbar( pLoad->pDoc );
     if (IsDBCS_CP(pLoad->pDoc->ulOemCodePage) && pEQFBUserOpt->UserOptFlags.bConvSOSI )
     {
       EQFBFileInsertSOSI(pLoad->pDoc);
     } /* endif */

     pLoad->pDoc->fAutoLineWrap = (EQF_BOOL) fAutoOld;
     if (pLoad->pDoc->fLineWrap && pLoad->pDoc->fAutoLineWrap )
     {
       pLoad->pDoc->sRMargin = (SHORT)pLoad->pDoc->lScrnCols;
       pEQFBUserOpt->sRMargin = (SHORT)pLoad->pDoc->lScrnCols;
       EQFBSoftLFInsert(pLoad->pDoc);
     } /* endif */


#ifdef T004422_BACKSAVE
     if (pEQFBUserOpt->fBackSave && (pLoad->pDoc->docType==STARGET_DOC))
     {
       // save current time as time of last save
       UtlTime(&(pLoad->pDoc->lTimeLastFileSave));
       /***************************************************************/
       /* only thread should be running, autosave is not nec right now*/
       /***************************************************************/
       EQFBWorkThreadTask(pLoad->pDoc, THREAD_AUTOSAVE);
     } /* endif */
#endif

     EQFBFuncTopDoc( pLoad->pDoc );    // get to the beginning
         ReAllocArabicStruct( pLoad->pDoc ); // check if we have to prepare for Arabic
     EQFBRefreshScreen( pLoad->pDoc );
   } /* endif */

   // start segment properties window if it was active at the time the editor was closed
   if ( (sRC >= 0) && (pLoad->pDoc->docType == STARGET_DOC) )
   {
     if ( MDDialogWasActive() )
     {
       MDStartDialog( pLoad->pDoc );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* in case of error conditions remove document from ring           */
   /*******************************************************************/
   if ( sRC < 0 )
   {
     EQFBDocDelete( pLoad->pDoc );
   } /* endif */
   return sRC;
 }


//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBDocInsert
//-----------------------------------------------------------------------------
// Function call:     EQFBDocInsert(PLOADSTRUCT)
//-----------------------------------------------------------------------------
// Description:       insert a new document in the ring
//                    Documents are held in a ring. This routine
//                    will create a new document structure and
//                    insert it in the ring and create a new display window
//-----------------------------------------------------------------------------
// Parameters:        PLOADSTRUCT pLoad        doc's load struct
//-----------------------------------------------------------------------------
// Returncode type:   SHORT
//-----------------------------------------------------------------------------
// Returncodes:       0            - success
//                    ERR_NOMEMORY - out of memory
//                    ERR_WNDCREATE- error during create of window/hps/hvps
//-----------------------------------------------------------------------------
// Function flow:     allocate space for new document structure
//                    if ok
//                      set new documents name
//                      add new document into the ring
//                      set initial values to the document fields
//                      load tag table
//                      create doc instance window
//                       (new doc becomes current document)
//-----------------------------------------------------------------------------
 SHORT EQFBDocInsert(PLOADSTRUCT pLoad)
 {
    PTBDOCUMENT pNewDoc;

    CHAR        szTable[MAX_EQF_PATH]; // fully qualified tag table name

    SHORT       sRc = 0;               // return code
    USHORT      usLen;                 // length of string
    ULONG       ulSysPrefOEMCP = 0L;
    ULONG       ulSysPrefAnsiCP = 0L;
    USHORT*     pusRightMargin = get_usRightMargin();
	USEROPT*    pEQFBUserOpt = get_EQFBUserOpt();

   /* Obtain space for the new document structure */
   if (! UtlAlloc((PVOID *) &pNewDoc, 0L, (LONG) sizeof( TBDOCUMENT), ERROR_STORAGE) )
   {
      return ERR_NOMEMORY;
   } /* endif */
   if (! UtlAlloc((PVOID *) &(pNewDoc->pEQFBWorkSegmentW), 0L,
                    (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE ))
   {
     return ERR_NOMEMORY;
   }

   if (*(pLoad->pFileName) != '\0' )
   {
     /* Set the new document's name */
     EQFBSysFilename( pNewDoc->szDocName, pLoad->pFileName );
   } /* endif */

   /* Add the new document into the ring */
   if ( pLoad->pDoc == NULL)
   {
      pNewDoc->next = pNewDoc;
      pNewDoc->prev = pNewDoc;

      pNewDoc->pstEQFGen = pLoad->pstEQFGen;
   }
   else
   {
      pNewDoc->next = pLoad->pDoc;
      pNewDoc->prev = pLoad->pDoc->prev;
      pLoad->pDoc->prev->next = pNewDoc;
      pLoad->pDoc->prev = pNewDoc;
      pNewDoc->pstEQFGen = pLoad->pDoc->pstEQFGen;
   }

   /* Set initial values to the document fields */
   pNewDoc->lCursorRow  = 0;
   pNewDoc->ulVScroll = 1;
   pNewDoc->lCursorCol  = 0;
   pNewDoc->lDBCSCursorCol = 0;
   pNewDoc->lSideScroll = 0;
   pNewDoc->fImeStartComposition = FALSE;  // test for P017862

   if (*pusRightMargin == AUTOSIZE )
   {
     pNewDoc->fAutoLineWrap = TRUE;
     pNewDoc->fLineWrap = FALSE;
   }
   else
   {
     pNewDoc->sRMargin = *pusRightMargin;            // set right margin value
     pNewDoc->fAutoLineWrap = FALSE;
   } /* endif */

   ulSysPrefOEMCP = GetLangOEMCP( NULL);
   ulSysPrefAnsiCP = GetLangAnsiCP( NULL);
   pNewDoc->flags.changed = FALSE;
   pNewDoc->Redraw    = REDRAW_ALL;
   pNewDoc->docType   = pLoad->docType;     // store the document type
 //OLD:  pNewDoc->EQFBFlags.Reflow = (USHORT) pLoad->fReflow;  //allow reflow (insert/split)
   pNewDoc->usTagEnd = '.';                 //should be set as TAG_END_CHAR
   ASCII2Unicode(pEQFBUserOpt->szInTagAbbr, pNewDoc->szInTagAbbrW, ulSysPrefOEMCP ); //set abbreviations for SHRINK
   ASCII2Unicode(pEQFBUserOpt->szOutTagAbbr, pNewDoc->szOutTagAbbrW, ulSysPrefOEMCP ); //set abbreviations for SHRINK
   UTF16strcpy(pNewDoc->szInTagLFAbbrW,pNewDoc->szInTagAbbrW);  //set abbreviations for SHRINK
   UTF16strcpy(pNewDoc->szOutTagLFAbbrW,pNewDoc->szOutTagAbbrW);           // and compact
   usLen = (USHORT) UTF16strlenCHAR(pNewDoc->szInTagLFAbbrW);
   pNewDoc->szInTagLFAbbrW[usLen] = LF;
   pNewDoc->szInTagLFAbbrW[usLen+1] = EOS;

   usLen = (USHORT) UTF16strlenCHAR(pNewDoc->szOutTagLFAbbrW);
   pNewDoc->szOutTagLFAbbrW[usLen] = LF;
   pNewDoc->szOutTagLFAbbrW[usLen+1] = EOS;
   pNewDoc->DispStyle = (DISPSTYLE)pEQFBUserOpt->DispTrans;

   ASCII2Unicode(pEQFBUserOpt->chTRNoteAbbr, pNewDoc->chTRNoteLFAbbrW, ulSysPrefOEMCP);
   usLen = (USHORT) UTF16strlenCHAR(pNewDoc->chTRNoteLFAbbrW);
   pNewDoc->chTRNoteLFAbbrW[usLen] = LF;
   pNewDoc->chTRNoteLFAbbrW[usLen+1] = EOS;
   pNewDoc->bOperatingSystem = (BYTE) UtlGetOperatingSystemInfo();
// get the visible whitespaces in Unicode -- remember, they are stored in ANSI
   UtlGetUTF16VisibleWhiteSpace(pNewDoc, pEQFBUserOpt, ulSysPrefAnsiCP);
   {
     /*****************************************************************/
     /* determine source and target language type                     */
     /*****************************************************************/
     PDOCUMENT_IDA pIdaDoc = NULL;
     if ( pNewDoc->pstEQFGen )
     {
        pIdaDoc  = (PDOCUMENT_IDA) ((PSTEQFGEN)(pNewDoc->pstEQFGen))->pDoc;
     }
     if ( pIdaDoc)
     {
       if (pNewDoc->docType == SSOURCE_DOC  )
       {
          pNewDoc->usLangTypeSrc = MorphGetLanguageType( pIdaDoc->szDocSourceLang );
          // for original document, do not bother about target lang!!
          pNewDoc->usLangTypeTgt = pNewDoc->usLangTypeSrc;
       }
       else
       {
         pNewDoc->usLangTypeTgt = MorphGetLanguageType( pIdaDoc->szDocTargetLang );
         pNewDoc->usLangTypeSrc = MorphGetLanguageType( pIdaDoc->szDocSourceLang );
       }
     }
     EQFBDocSetCodePage(pNewDoc, pIdaDoc);

     // remember document long name
     if (pNewDoc->docType != OTHER_DOC )
     {
       strcpy( pNewDoc->szDocLongName, pIdaDoc->szDocLongName );
     }
   }
   if (pEQFBUserOpt->fInitCrsMode  )
   {
     pNewDoc->usCursorType = CURSOR_INSERT;
     pNewDoc->EQFBFlags.inserting = TRUE;
   }
   else
   {
     pNewDoc->usCursorType = CURSOR_REPLACE;
   } /* endif */
   EQFBSysInit( pNewDoc );             // set new cursor shapes
   pNewDoc->ulEyeCatcher = TPRO_EYECATCHER;

   if ( pNewDoc->docType == STARGET_DOC )   //alloc space for UNDO function
   {
      //if alloc fails, message is displayed, user can go ahead,
      //but UNDO is not possible
      if ( UtlAlloc((PVOID *)&pNewDoc->pUndoSegW,
                    0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W),ERROR_STORAGE) )
      {
        pNewDoc->fUndoState = FALSE;           //init fUndoState
        pNewDoc->usUndoSegOff = 0;
      } /* endif */
   } /* endif */
   // load document tag tables
   if ( *pLoad->pszEQFTagTable )
   {
      strcpy( szTable, pLoad->pszEQFTagTable );
   }
   else
   {
      strcpy( szTable, DEFAULT_QFTAG_TABLE );
   } /* endif */
   sRc = (SHORT) TALoadTagTable( szTable,
                                 (PLOADEDTABLE *) &pNewDoc->pQFTagTable,
                                 TRUE, FALSE );
   if ( !sRc )
   {
       if ( *pLoad->pszTagTable )
       {
          strcpy( szTable, pLoad->pszTagTable );
       }
       else
       {
          strcpy( szTable, DEFAULT_DOCUMENT_TABLE );
       } /* endif */
       sRc = (SHORT) TALoadTagTable( szTable,
                                     (PLOADEDTABLE *)&pNewDoc->pDocTagTable,
                                     FALSE, FALSE );

       /***************************************************************/
       /* check if special user exit handling is necessary,           */
       /*  i.e. load TAPrepProtectTable ....                          */
       /***************************************************************/
       if ( !sRc )
       {
		   PLOADEDTABLE pTagTable = NULL;
		   pTagTable    = (PLOADEDTABLE) pNewDoc->pDocTagTable;
  		 if ( pTagTable->fReflow == TAGREFLOW_NO )
	  	 {
		  	 pNewDoc->EQFBFlags.Reflow = FALSE;
       }
	     else
	     {
			   pNewDoc->EQFBFlags.Reflow = TRUE;
	     }


       // test test 
         TALoadEditUserExit( pNewDoc->pDocTagTable, szTable,
                             &pNewDoc->hModule,
                             &pNewDoc->pfnUserExit,
                             (PFN*)&pNewDoc->pfnCheckSegExit,
                             (PFN*)&pNewDoc->pfnShowTrans,
                             (PFN*)&pNewDoc->pfnTocGoto,
                             (PFN*)&pNewDoc->pfnGetSegContext, 
                             NULL, 
                             (PFN*)&pNewDoc->pfnFormatContext, 
                             NULL,
                             &pNewDoc->pfnUserExitW,
                             (PFN*)&pNewDoc->pfnCheckSegExitW,
                             (PFN*)&pNewDoc->pfnCheckSegExExitW );
       } /* endif */
   } /* endif */

   if ( sRc )                // file could not be accessed
   {
      PSZ pErr = szTable;
      UtlError(ERROR_FILE_ACCESS_ERROR, MB_CANCEL,
               1, &pErr, EQF_ERROR);
   }
   else
   {
      /* New document becomes the current document */
      pLoad->pDoc = pNewDoc;

      if ( !EQFBDocWndCreate( pNewDoc, pLoad )) // create the document inst wnd.
      {
         sRc = ERR_WNDCREATE;
      }
      else
      {
         if ( pNewDoc->docType == STARGET_DOC )
         {
			 if (pNewDoc->EQFBFlags.Reflow ||
			      (!pNewDoc->EQFBFlags.Reflow && pNewDoc->fAutoLineWrap ))
			 {
                                                      // copy line wrap def. settings
               pNewDoc->fLineWrap = pEQFBUserOpt->fLineWrap;
		     }
         } /* endif */
         if (pNewDoc->docType == SSOURCE_DOC && pNewDoc->fAutoLineWrap )
         {
           pNewDoc->fLineWrap = pEQFBUserOpt->fLineWrap;
         } /* endif */
         EQFBSysInit( pNewDoc );
      } /* endif */
   } /* endif */


   return sRc;
 }

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBDocDelete
//-----------------------------------------------------------------------------
// Function call:     EQFBDocDelete(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       delete the current document from the ring
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc  ptr to document structure
//-----------------------------------------------------------------------------
// Returncode type:   PTBDOCUMENT
//-----------------------------------------------------------------------------
// Returncodes:       pDoc  ptr to new current doc in ring
//-----------------------------------------------------------------------------
// Function flow:     remove block if any in current document
//                    switch (type of current document)
//                     case other document:
//                       remove doc from rign and activate target doc
//                     case source document:
//                       remove document from ring
//                     case target document:
//                       close all other docs and then close target doc
//                       if no other target in loaded list pending
//-----------------------------------------------------------------------------
PTBDOCUMENT EQFBDocDelete
(
   PTBDOCUMENT   pDoc
)
{
   PTBDOCUMENT     pNextDoc;            // ptr to document currently deleted
   PEQFBBLOCK      pstBlock;           // block mark struct
   PFINDDATA pFindData = EQFBGetFindData();

   pNextDoc = pDoc -> next;
   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
   if ( pstBlock && (pstBlock->pDoc == pDoc) )   // remove mark if for current document
   {
      memset( pstBlock, 0, sizeof( EQFBBLOCK ));
   } /* endif */

   if ( pDoc && pFindData &&
        (pFindData->pDoc == pDoc) && pFindData->hwndFindDlg )
   {
     EQFBFindCloseModeless( pFindData->hwndFindDlg, 0, 0L );
   } /* endif */
   if ( pDoc )
   {
     EQFBCFindTerminate( pDoc );
   } /* endif */               

   switch ( pDoc->docType )
   {
     case OTHER_DOC:
     case TRNOTE_DOC:
        // remove document from ring and activate target doc
        pDoc = EQFBRemoveDoc ( pDoc );
        while ( pDoc->docType != STARGET_DOC )
        {
           pDoc = pDoc->next;
           if ( pDoc == pNextDoc )
           {
              break;
           } /* endif */
        } /* endwhile */

        PostMessage(((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
                      0,
                      MP2FROMHWND(((PSTEQFGEN)pDoc->pstEQFGen)->hwndEditorTgt));

        break;
     case SSOURCE_DOC :
        pDoc = EQFBRemoveDoc( pDoc );
        break;
     case STARGET_DOC:
        //  close all other docs and than close target doc.
        {
          PSZ pFileName, pSTarget;
          /*****************************************************************/
          /* indicate that thread should stop                              */
          /*****************************************************************/
          pDoc->fThreadKill = TRUE;
	      DosSleep( 0L );  // give thread a slice


		  // remove source doc if loaded
          if ( pDoc->twin )
          {
            EQFBRemoveDoc( pDoc->twin );                     // remove source document
          } /* endif */
          /************************************************************/
          /* generate object name and remove it from our list         */
          /************************************************************/
          if ( pDoc->pInBuf )
          {
            strcpy( (PSZ) pDoc->pInBuf, pDoc->szDocName );
            pFileName = UtlSplitFnameFromPath( (PSZ) pDoc->pInBuf );
            pSTarget = UtlGetFnameFromPath( (PSZ) pDoc->pInBuf );
            if ( pSTarget && pFileName )
            {
              pFileName = UtlGetFnameFromPath( pDoc->szDocName );
              *pSTarget = EOS;
              *(pSTarget-1) = EOS;
              EQFCONVERTFILENAMES( (PSZ) pDoc->pInBuf, pFileName, pSTarget );
              *(pSTarget -1 )= BACKSLASH;
              EQF_XDOCREMOVE( (PSTEQFGEN)pDoc->pstEQFGen, (PSZ) pDoc->pInBuf );
            } /* endif */
          } /* endif */

          pDoc = EQFBRemoveDoc( pDoc );
          if (pDoc )
          {
            /************************************************************/
            /* if no other translation environment, get rid of all docs.*/
            /************************************************************/
            PTBDOCUMENT pStartDoc = pDoc;
            if ( pDoc->docType != STARGET_DOC )
            {
              while ( pDoc->next != pStartDoc && pDoc->docType != STARGET_DOC )
              {
                pDoc = pDoc->next;
              } /* endwhile */
            } /* endif */
            if ( pDoc->docType != STARGET_DOC )
            {
              while ( pDoc )
              {
                 pDoc = EQFBRemoveDoc( pDoc );
              } /* endwhile */
            } /* endif */

            /**********************************************************/
            /* activate this document                                 */
            /**********************************************************/
            if ( pDoc && pDoc->docType == STARGET_DOC  )
            {
              ActTransEnv( pDoc );
            } /* endif */
          } /* endif */
        }

        break;
   } /* endswitch */

   return pDoc;
 }

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBRemoveDoc
//-----------------------------------------------------------------------------
// Function call:     EQFBRemoveDoc(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       delete current document from ring and
//                    free all related resources
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//-----------------------------------------------------------------------------
// Returncode type:   PTBDOCUMENT
//-----------------------------------------------------------------------------
// Returncodes:       pDoc     ptr to new current document
//-----------------------------------------------------------------------------
// Function flow:     get document out of the ring
//                    destroy the window
//                    free all other space associated with this document
//                    delete the document window
//                    delete the document instance
//-----------------------------------------------------------------------------
PTBDOCUMENT EQFBRemoveDoc
(
   PTBDOCUMENT  pDoc
)
{
   PTBDOCUMENT pDocNext;               // pointer to previous document

   if ( pDoc )
   {
     /*****************************************************************/
     /* indicate that thread should stop                              */
     /*****************************************************************/
     pDoc->fThreadKill = TRUE;
	 DosSleep( 0L );  // give thread a slice
     /*****************************************************************/
     /* wait until current thread task is finished                    */
     /*****************************************************************/
     if ( pDoc->fThreadAct )
     {
       USHORT usI = 0;
       while ( pDoc->fThreadAct && usI<500)
       {
         DosSleep( 100L );
         DosSleep( 0L );
         usI++;
       } /* endwhile */
     } /* endif */

     pDocNext = pDoc->next;
     if ( pDocNext == pDocNext->next )
     {
        pDocNext = NULL;                     // no other doc available
     }
     else
     {
                                             // get it out of the ring
        pDocNext->prev = pDoc->prev;
        (pDoc->prev)->next = pDocNext;
     } /* endif */

     ANCHORWNDIDA( pDoc->hwndFrame, NULL );
     WinDestroyWindow( pDoc->hwndFrame );   // destroy the window

     // close any open segment properties window
     if ( pDoc->docType == STARGET_DOC )
     {
       MDEndDialog( pDoc );
     } /* endif */

     /* Free up the document space */
     EQFBFreeDoc( &pDoc, 0L );

   }
   else
   {
     /*****************************************************************/
     /* no further document available                                 */
     /*****************************************************************/
     pDocNext = NULL;
   } /* endif */

   return( pDocNext );
}

void EQFBFreeSegTables( PTBSEGMENTTABLE *ppSegTable, ULONG ulNumOfTables, PSZ_W pszWorkSeg );

// new function to clear all memory occupied by a document
void EQFBFreeDoc( PTBDOCUMENT *ppDoc, ULONG ulOptions )
{
  PEQFBBLOCK  pstBlock;               // pointer to block struct

  PTBDOCUMENT pDoc = *ppDoc;

  if ( pDoc )
  {
     /* Free document tag tables                                        */
     if ( !(ulOptions & EQFBFREEDOC_NOTAGTABLEFREE) )
     {
       if ( pDoc->pQFTagTable )
       {
         TAFreeTagTable( (PLOADEDTABLE)pDoc->pQFTagTable );
         pDoc->pQFTagTable = NULL;
       } /* endif */
       if ( pDoc->pDocTagTable )
       {
         TAFreeTagTable( (PLOADEDTABLE)pDoc->pDocTagTable );
         pDoc->pDocTagTable = NULL;
       } /* endif */
     } /* endif */


     UtlAlloc((PVOID *) &pDoc->pInBuf, 0L, 0L, NOMSG );
     UtlAlloc((PVOID *) &pDoc->pTokBuf, 0L, 0L, NOMSG );

     EQFBFreeSegTables( &(pDoc->pSegTables), pDoc->ulSegTables, pDoc->pEQFBWorkSegmentW );
     EQFBFreeSegTables( &(pDoc->pAddSegTables), pDoc->ulAddSegTables, pDoc->pEQFBWorkSegmentW );

     UtlAlloc((PVOID *) &pDoc->pUndoSeg, 0L, 0L, NOMSG );   //free storage of Undo
     UtlAlloc((PVOID *) &pDoc->pUndoSegW, 0L, 0L, NOMSG );   //free storage of Undo
     UtlAlloc((PVOID *) &pDoc->pSegmentBuffer, 0L, 0L, NOMSG);
     UtlAlloc((PVOID *) &pDoc->pSegmentBufferW, 0L, 0L, NOMSG);
     if (pDoc->pContext) UtlAlloc((PVOID *)&(pDoc->pContext),0L,0L,NOMSG);
     if (pDoc->pWSList) UtlAlloc((PVOID *)&(pDoc->pWSList), 0L, 0, NOMSG);

     if ( pDoc->pBidiStruct && pDoc->pBidiStruct->usTimerID )
     {
       KillTimer( pDoc->hwndClient, pDoc->pBidiStruct->usTimerID );
     } /* endif */
     if ( pDoc->pBidiStruct && pDoc->pBidiStruct->pPool )
     {
       PoolDestroy( pDoc->pBidiStruct->pPool );
     } /* endif */
     if ( pDoc->pDispFileRTF )
     {
       UtlAlloc((PVOID *) &(pDoc->pDispFileRTF->pBufferOverflow), 0L, 0L, NOMSG);
       UtlAlloc((PVOID *) &pDoc->pDispFileRTF, 0L, 0L, NOMSG);
     } /* endif */
     if ( pDoc->pBidiStruct ) UtlAlloc((PVOID *) &pDoc->pBidiStruct, 0L, 0L, NOMSG);
     if ( pDoc->pArabicStruct )
     {
       UtlAlloc( (PVOID *)&pDoc->pArabicStruct->plCaretPos, 0L, 0L, NOMSG );
       UtlAlloc( (PVOID *)&pDoc->pArabicStruct, 0L, 0L, NOMSG );
     }
     pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
     if ( pstBlock && (pstBlock->pDoc == pDoc) )
     {
        memset( pstBlock, 0, sizeof( EQFBBLOCK ));
     } /* endif */
     if ( pDoc->hModule && !(ulOptions & EQFBFREEDOC_NOPROTECTFREE) )
     {
       TAEndProtectTable( &pDoc->hModule,
                          &pDoc->pfnUserExit,
                          (PFN*) &pDoc->pfnCheckSegExit,
                          (PFN*) &pDoc->pfnShowTrans,
                          (PFN*) &pDoc->pfnTocGoto,
                          &pDoc->pfnUserExitW,
                          (PFN*) &pDoc->pfnCheckSegExitW);
     } /* endif */
     if ( pDoc->pEQFBWorkSegmentW ) UtlAlloc((PVOID *) &pDoc->pEQFBWorkSegmentW, 0L, 0L, NOMSG);
     if ( pDoc->pContext )  UtlAlloc((PVOID *) &pDoc->pContext, 0L, 0L, NOMSG );
     if ( pDoc->pWSList) UtlAlloc((PVOID *)&(pDoc->pWSList), 0L, 0, NOMSG);
     if (pDoc->lEntity) FreeEntitites( pDoc->lEntity);

     if ( ! (ulOptions & EQFBFREEDOC_NODOCIDAFREE) )
     {
        UtlAlloc((PVOID *) &pDoc, 0L, 0L, NOMSG );
        *ppDoc = NULL;
     }
  } /* endif */
} /* end of function EQFBFreeDoc */

// helper function to free segtable data
void EQFBFreeSegTables( PTBSEGMENTTABLE *ppSegTable, ULONG ulNumOfTables, PSZ_W pszWorkSeg )
{
  USHORT          usI, usJ;           // loop counter
  PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
  PTBSEGMENT      pSegment;           // ptr for segment deleting

  if ( ppSegTable && *ppSegTable )
  {
    pSegTable = *ppSegTable;
    for ( usI = 0; usI < ulNumOfTables; usI++ )
    {
      pSegment = pSegTable->pSegments;
      for ( usJ = 0; usJ < pSegTable->ulSegments; usJ++ )
      {
         if ( pSegment->pData )   UtlAlloc((PVOID *) &pSegment->pData, 0L, 0L, NOMSG );
         if ( pSegment->pDataW == pszWorkSeg)
         {
           // something in the workseg in/out failed, do not free this segment...
           pSegment->pDataW = NULL;
         }
         else
         {
           if ( pSegment->pDataW )  UtlAlloc((PVOID *) &pSegment->pDataW, 0L, 0L, NOMSG );
         } /* endif */
         if ( pSegment->pusBPET ) UtlAlloc((PVOID *) &pSegment->pusBPET, 0L, 0L, NOMSG );
         if ( pSegment->pusHLType ) UtlAlloc((PVOID *) &pSegment->pusHLType, 0L, 0L, NOMSG );
         if ( pSegment->pContext )  UtlAlloc((PVOID *) &pSegment->pContext, 0L, 0L, NOMSG );
         if ( pSegment->pvMetadata )  UtlAlloc((PVOID *) &pSegment->pvMetadata, 0L, 0L, NOMSG );
         pSegment++;
      } /* endfor */
      UtlAlloc((PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
      pSegTable++;
    } /* endfor */
    UtlAlloc((PVOID *) ppSegTable, 0L, 0L, NOMSG );
  } /* endif */
} /* end of function EQFBFreeSegTable */


//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBDocSave
//-----------------------------------------------------------------------------
// Function call:     EQFBDocSave(PTBDOCUMENT,PSZ, BOOL )
//-----------------------------------------------------------------------------
// Description:       save the current document
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc ptr to document instance
//                    PSZ pszFileName
//                    BOOL fAskForSave - ask question if saved seg to be saved?
//-----------------------------------------------------------------------------
// Returncode type:   SHORT
//-----------------------------------------------------------------------------
// Returncodes:       sRc        return codes from EQFBFileWrite
//-----------------------------------------------------------------------------
// Function flow:     restore old status of current segment first
//                    save last segment if in target and changes in workseg
//                    check whether document is translated
//                    write file to disk
//                    set pSeg->pData to worksegment and reparse it again
//                    (nec to allow NO during save)
//-----------------------------------------------------------------------------

SHORT EQFBDocSave( PTBDOCUMENT pDoc, PSZ pszFileName, BOOL fAskForSave )
{
   SHORT sRC = 0;                      // function's return code
   USHORT  usResult;
   PTBSEGMENT  pSourceSeg;             // pointer to segment
   PSZ_W       pData;                  // pointer to source data
   HWND        hwndTemp;                 // temp. window handle
   QSTATUS     qStatusTemp = QF_XLATED;  // temp. status
   ULONG       ulTempActSeg;
   DOCFLAGS    EQFBFlags;                // structure of flags
   COUNTFLAG   TempCountFlag;
   USHORT      usTempModWords = 0;
   SEGFLAGS    TempSegFlag;

   /*******************************************************************/
   /* restore old status of current segment first                     */
   /* This avoids to get QFC segments and not knowing what's the      */
   /* real status of the segment.                                     */
   /*******************************************************************/

   /*******************************************************************/
   /* save contents of relevant data                                  */
   /*******************************************************************/
   if ( pDoc->pTBSeg )
   {
     qStatusTemp = (QSTATUS)pDoc->pTBSeg->qStatus;
     usTempModWords = pDoc->pTBSeg->usModWords;
     memcpy(&TempCountFlag, &pDoc->pTBSeg->CountFlag, sizeof (COUNTFLAG));
     memcpy(&TempSegFlag, &pDoc->pTBSeg->SegFlags, sizeof(SEGFLAGS));
   } /* endif */
   ulTempActSeg = pDoc->tbActSeg.ulSegNum;       // store act. seg num..
   memcpy( &EQFBFlags, &pDoc->EQFBFlags, sizeof( DOCFLAGS ));

   if ( pDoc->pTBSeg && pDoc->tbActSeg.ulSegNum )    // get info if not post edit
   {
      pDoc->pTBSeg->qStatus = pDoc->tbActSeg.qStatus;
   } /* endif */


   /*******************************************************************/
   /* save last segment if in target and any changes to work seg      */
   /*******************************************************************/
   if ( (pDoc->docType == STARGET_DOC ) &&
       (pDoc->EQFBFlags.workchng || pDoc->fFuzzyCopied ||
            (pDoc->pTBSeg && pDoc->pTBSeg->SegFlags.UnTrans) ) )
   {
     usResult = MBID_NO;
     hwndTemp = pDoc->hwndFrame;
     if ( ( pDoc->EQFBFlags.workchng || pDoc->fFuzzyCopied)
          && !pDoc->fErrorProcessed && fAskForSave )
     {
       if ( !pDoc->EQFBFlags.PostEdit )
       {
         // issue warning determining if user wants to save or not
         usResult = UtlError( TB_CHANGESEGMENT, MB_YESNO, 0, NULL, EQF_QUERY);

         // refresh metadata for current segment
         MDGetMetadata( pDoc, &(pDoc->tbActSeg), (usResult == MBID_YES ) );
       }
       else
       {
         usResult = MBID_YES;
       } /* endif */
     }
     else
     {
       // refresh metadata for current segment
       if ( !pDoc->EQFBFlags.PostEdit )
       {
         MDGetMetadata( pDoc, &(pDoc->tbActSeg), FALSE );
       } /* endif */
     } /* endif */

     pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );

     if ( pDoc )
     {
       switch ( usResult )
       {
         case MBID_YES:
            pDoc->EQFBFlags.NoEmptySegCheck = TRUE;
            if ( ! EQFBSaveSeg( pDoc ))          // save current segment
            {
               EQFBFuncUndo( pDoc );                       // force an undo
            } /* endif */

            /* reset change flag (we saved segment)                   */
            EQFBFlags.workchng = FALSE;
            break;
         default:                                // ignore request
            if ( pDoc->tbActSeg.ulSegNum )       // get info if not post edit
            {
               pDoc->pTBSeg->qStatus = pDoc->tbActSeg.qStatus;
            } /* endif */

            if ( pDoc->pTBSeg && (pDoc->pTBSeg->qStatus != QF_XLATED))
            {
              pDoc->pTBSeg->SegFlags.Typed = FALSE;

              pDoc->pTBSeg->SegFlags.Copied = FALSE;
              pDoc->pTBSeg->usModWords = 0;
              memset(&pDoc->pTBSeg->CountFlag, 0,
                      sizeof( pDoc->pTBSeg->CountFlag));
            }
            if ( pDoc->pSaveSegW )                          /* @KIT945A */
            {
              pDoc->pTBSeg->pDataW = pDoc->pSaveSegW;
              if (pDoc->hwndRichEdit && pDoc->tbActSeg.ulSegNum)
              {
                EQFBSetWorkSegRTF( pDoc, pDoc->tbActSeg.ulSegNum, pDoc->pTBSeg->pDataW );
              }
            } /* endif */                                  /* @KIT945A */

            // if untranslate active,  use original segment
            // and copy it as source of translation
            if ( pDoc->pTBSeg && (pDoc->pTBSeg->SegFlags.UnTrans)
                 && pDoc->tbActSeg.ulSegNum )                //@@P015155 untranslate active
            {
              USHORT  usBytesInSrc = 0;
               pSourceSeg = EQFBGetSegW(pDoc->twin, pDoc->tbActSeg.ulSegNum);
               usBytesInSrc = (USHORT) UTF16strlenBYTE( pSourceSeg->pDataW ) + sizeof(CHAR_W);
               if ( pSourceSeg &&
                    UtlAlloc((PVOID *)&pData,0L,
                            (LONG) max(usBytesInSrc, MIN_ALLOC),
                             ERROR_STORAGE) )
               {
                  UtlAlloc((PVOID *) &(pDoc->pTBSeg->pDataW) , 0L, 0L, NOMSG );
                  pDoc->pTBSeg->pDataW = pData;
                  memcpy( (PBYTE) pData,(PBYTE)pSourceSeg->pDataW, usBytesInSrc );
                  *(pData + pSourceSeg->usLength) = EOS;
                  pDoc->pSaveSegW = NULL;
               } /* endif */
               pDoc->pTBSeg->SegFlags.UnTrans = FALSE;         // reset untrans flag
            } /* endif */

            EQFBCompSeg( pDoc->pTBSeg );

            break;
       } /* endswitch */
     } /* endif */
   }
   else
   {
     if ( pDoc->pTBSeg && (pDoc->pTBSeg->qStatus != QF_XLATED))
     {
       pDoc->pTBSeg->usModWords = 0;
       memset(&pDoc->pTBSeg->CountFlag, 0,
               sizeof( pDoc->pTBSeg->CountFlag));
     } /* endif */
   } /* endif */

   if ( pDoc )
   {
     if ( pDoc->flags.changed )
     {
       PDOCUMENT_IDA pIdaDoc = (PDOCUMENT_IDA)((PSTEQFGEN)(pDoc->pstEQFGen))->pDoc;

       if ( pDoc->fStoreInUnicode )
       {
        pIdaDoc->fSTargetInUnicode = TRUE;
       }

        EQFBSetStatus( pDoc );
        if (pDoc->docType == STARGET_DOC )
        {
          // do any entity processing
          if ( UtlQueryUShort( QS_ENTITYPROCESSING ) )
          {
            if ( isEntityMarkup( pIdaDoc->szDocFormat ) )
            {
              ScanForEntities( pDoc );
            } /* endif */
          } /* endif */

          sRC = EQFBFileWriteEx( pszFileName, pDoc, DOCSAVE_LOGTASK2, 0 );
        }
        else
        {
          sRC = EQFBFileWriteEx( pszFileName, pDoc, 0, 0 );
        } /* endif */
     }
     else if ( pDoc->docType == STARGET_DOC )
     {
       // save metadata in any case
       MDWriteMetaData( pDoc );
     } /* endif */

     /*****************************************************************/
     /* restore old contents of relevant data                         */
     /*****************************************************************/
     if ( pDoc->pTBSeg )
     {
       pDoc->pTBSeg->qStatus = (USHORT)qStatusTemp;
       pDoc->pTBSeg->usModWords = usTempModWords;
       memcpy( &pDoc->pTBSeg->CountFlag, &TempCountFlag, sizeof (COUNTFLAG));
       memcpy( &pDoc->pTBSeg->SegFlags, &TempSegFlag, sizeof(SEGFLAGS));
     } /* endif */

     pDoc->tbActSeg.ulSegNum = ulTempActSeg ;   // restore act. seg num..
     memcpy( &pDoc->EQFBFlags, &EQFBFlags, sizeof( DOCFLAGS ));

     // set current pSeg->pData to worksegment and reparse it again
     // necessary to allow NO (if seg changed) during save
     if ( pDoc->EQFBFlags.workchng && pDoc->pTBSeg &&
          (pDoc->pTBSeg->pDataW != pDoc->pEQFBWorkSegmentW) )       /* @KIT0886A */
     {
        pDoc->pSaveSegW = pDoc->pTBSeg->pDataW;              // store it here
        pDoc->pTBSeg->pDataW = pDoc->pEQFBWorkSegmentW;
        EQFBCompSeg( pDoc->pTBSeg );
     } /* endif */
     pDoc->Redraw = REDRAW_ALL;  // refresh of screen necessary
   } /* endif */

   return ( sRC );
}

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBLineUp
//-----------------------------------------------------------------------------
// Function call:     EQFBLineUp(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       check if a previous line exists (via TBRowOffset table)
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDOc  pointer to document instance
//-----------------------------------------------------------------------------
// Returncode type:   SHORT
//-----------------------------------------------------------------------------
// Returncodes:       0  success
//                    WARN_NONEXT no next line
//-----------------------------------------------------------------------------
// Function flow:     if segment number in TBRowOffset table is 0,
//                      return WARN_NONEXT
//                    else
//                      return 0
//                    endif
//-----------------------------------------------------------------------------


 SHORT EQFBLineUp
 (
    PTBDOCUMENT   pDoc                 // pointer to document instance
 )

 {
    SHORT sRc = 0;               // return code
    PTBROWOFFSET pTBRow;         // pointer to row structure

    pTBRow = pDoc->TBRowOffset;  // screen - row offset
    if ( pTBRow->ulSegNum == 0)
    {
       sRc = WARN_NONEXT;
    } /* endif */
    return sRc;
 }

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBLineDown
//-----------------------------------------------------------------------------
// Function call:     EQFBLineDown(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       check if a next line is available
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDOc ptr to document instance
//-----------------------------------------------------------------------------
// Returncode type:   SHORT
//-----------------------------------------------------------------------------
// Returncodes:       0            - success
//                    WARN_NONEXT  - no next line
//-----------------------------------------------------------------------------
// Function flow:     if segment number is 0 in the ROwOffset table of
//                    the next cursor row
//                      no next line available
//                    else
//                      return 0
//-----------------------------------------------------------------------------

 SHORT EQFBLineDown
 (
    PTBDOCUMENT  pDoc             // pointer to document ida
 )

 {
    SHORT sRc = 0;               // return code
    PTBROWOFFSET pTBRow;         // pointer to row structure

    pTBRow = pDoc->TBRowOffset+pDoc->lScrnRows+1 ;  // screen - row table offset
    if ( pTBRow->ulSegNum == 0)
    {
       sRc = WARN_NONEXT;
    } /* endif */
    return sRc;
 }

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBDocPrint
//-----------------------------------------------------------------------------
// Function call:     EQFBDocPrint(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       print a document
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc ptr to document instance
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     call EQFBFilePrint
//-----------------------------------------------------------------------------

VOID EQFBDocPrint( PTBDOCUMENT pDoc )
{
   EQFBFilePrint( pDoc );
}

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBSetStatus
//-----------------------------------------------------------------------------
// Function call:     EQFBSetStatus ( PTBDOCUMENT )
//-----------------------------------------------------------------------------
// Description:       this function will set the status of the document
//                    i.e. if it is translated, how much is translated, etc.
//
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     loop thru all segments and update the sums of transl.
//                    copied and modified segments.
//-----------------------------------------------------------------------------
static
VOID EQFBSetStatus
(
  PTBDOCUMENT  pDoc
)
{
  ULONG   ulSegNum;           // segment number
  USHORT  usSumTrans = 0;     // number of translated segments
  USHORT  usSumUnTrans = 0;   // number of untranslated segments
  USHORT  usSumPropChng= 0;   // number of modified TM copies
  USHORT  usSumFromScratch=0; // number of segments transl. from scratch
  USHORT  usSumPropCopy =  0; // number of proposals copied unchanged
  USHORT  usRateOfCompl;      // completion rate in %
  PTBSEGMENT       pSeg;      // ptr to segment
  PSTEQFGEN   pstEQFGen;      // pointer to generic structure

  ulSegNum = 1;
  pSeg = EQFBGetSegW(pDoc, ulSegNum);
  while ( pSeg )
  {
    /******************************************************************/
    /* ignore nopïs count all others                                  */
    /******************************************************************/
    switch ( pSeg->qStatus )
    {
      case  QF_NOP:                             // segment not counted
        break;

      case  QF_TOBE:
      case  QF_ATTR:
      case  QF_CURRENT:
        if ( pSeg->SegFlags.Joined )
        {
          /************************************************************/
          /* count joined segments as translated -- otherwise our     */
          /* counts are incorrect...                                  */
          /************************************************************/
          usSumTrans ++;
        }
        else
        {
          usSumUnTrans ++;                        // segment untranslated
        } /* endif */

        break;

      case  QF_XLATED:
        usSumTrans ++;                          // segment translated
        if ( pSeg->SegFlags.Typed )
        {
          if ( pSeg->SegFlags.Copied )
          {
            usSumPropChng++;
          }
          else
          {
            usSumFromScratch++;
          } /* endif */
        }
        else if ( pSeg->SegFlags.Copied )
        {
            usSumPropCopy++;
        } /* endif */
        break;
      default :
        break;
    } /* endswitch */
    ulSegNum ++;
    pSeg = EQFBGetSegW(pDoc, ulSegNum);
  } /* endwhile */

  /********************************************************************/
  /* check that file contains something to be transl. or already trans*/
  /********************************************************************/
  if ( usSumUnTrans || usSumTrans )
  {
    usRateOfCompl = (USHORT)(((LONG)usSumTrans*100) /
                                  (usSumUnTrans+usSumTrans));
    /******************************************************************/
    /* check that we set compl.rate to 1% if transl. started work.    */
    /* i.e. if he touched the file                                    */
    /******************************************************************/
    if ( !usRateOfCompl )                 // && usSumTrans )
    {
      usRateOfCompl = 1;
    } /* endif */
  }
  else
  {
    usRateOfCompl = 100;
  } /* endif */


  /********************************************************************/
  /* set translated flag if NO untranslated segment                   */
  /********************************************************************/
  pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
  if ( !usSumUnTrans )
  {
     WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_ULATED, (WPARAM)NULL, (LPARAM)NULL);
     WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_XLATED, (WPARAM)NULL, (LPARAM)NULL);
  } /* endif */

  /********************************************************************/
  /* update status fields for document                                */
  /********************************************************************/

  WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_STATUS,
              (WPARAM)MP1FROMSHORT(EQF_DOC_COMPLRATE),
              (LPARAM)MP2FROMSHORT ( usRateOfCompl ) );

  WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_STATUS,
              MP1FROMSHORT(EQF_DOC_SEGFROMSCRATCH),
              MP2FROMSHORT( usSumFromScratch ) );

  WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_STATUS,
              MP1FROMSHORT(EQF_DOC_SEGMODIFIED),
              MP2FROMSHORT ( usSumPropChng ) );

  WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_STATUS,
              MP1FROMSHORT(EQF_DOC_SEGCOPIED),
              MP2FROMSHORT ( usSumPropCopy ) );

  if ( usSumTrans == 0 )
  {
    WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_STATUS,
                MP1FROMSHORT(EQF_DOC_NOTTOUCHED),
                MP2FROMSHORT ( 0 ) );
  } /* endif */

  WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_STATUS,
              MP1FROMSHORT(EQF_DOC_UPDATED),
              MP2FROMSHORT ( 0 ) );
} /* end of function EQFBSetStatus */


//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBFuncOpenTRNote
//-----------------------------------------------------------------------------
// Function call:     EQFBFuncOpenTRNote(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       this function opens a window and fills it with
//                    the TRNotes of the current document
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     loop thru all segments and fill SegTable with all trnotes
//
//-----------------------------------------------------------------------------

VOID EQFBFuncOpenTRNote
(
  PTBDOCUMENT  pDoc
)
{
   LOADSTRUCT LoadStruct;             // load structure
   PSTEQFGEN     pstEQFGen;           // pointer to generic edit structure
   PTBDOCUMENT   pTRNoteDoc;
   SHORT         rc = 0;
   PTBDOCUMENT   pDocCurrent;

   /*******************************************************************/
   /* check whether translators note is already available             */
   /*******************************************************************/
   pDocCurrent = pDoc->next;
   while ((pDocCurrent->docType != TRNOTE_DOC) && (pDocCurrent != pDoc ))
   {
     pDocCurrent = pDocCurrent->next;
   } /* endwhile */
   if (pDocCurrent->docType == TRNOTE_DOC )
   {
     if ( pDocCurrent->twin != pDoc  )
     {
       /***************************************************************/
       /* another TRNOTE doc is loaded -- force reload of document..  */
       /***************************************************************/
       EQFBRemoveDoc ( pDocCurrent );
       EQFBFuncOpenTRNote( pDoc );
     }
     else
     {
       if ( !pDoc->EQFBFlags.PostEdit )       // reposition&activate
       {
         EQFBGotoTRNoteSeg(pDocCurrent,pDoc->tbActSeg.ulSegNum);
       }
       else
       {
         EQFBGotoTRNoteSeg(pDocCurrent,pDoc->TBCursor.ulSegNum);
       } /* endif */
       pDocCurrent->Redraw |= REDRAW_ALL;   // force repaint...
       EQFBScreenData( pDocCurrent );
       WinShowWindow( pDocCurrent->hwndFrame, TRUE );
       BringWindowToTop( pDocCurrent->hwndFrame );
     } /* endif */
   }
   else
   {
     pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
     LoadStruct.hwndParent = pstEQFGen->hwndTWBS;
     LoadStruct.pDoc = pDoc;        // set base document
     LoadStruct.pstEQFGen = pstEQFGen;
     LoadStruct.fsFlagStyle = EQF_SWP_MOVE | EQF_SWP_SIZE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE;
     LoadStruct.fReadOnly = TRUE;

     if ( pstEQFGen->flEditOtherStyle )
     {
          LoadStruct.flFrameStyle = (pstEQFGen->flEditOtherStyle & AVAILSTYLES) |
                                       FCF_SIZEBORDER;
     }
     else
     {
          LoadStruct.flFrameStyle = FCF_TITLEBAR   | FCF_MENU   | FCF_SYSMENU |
                                    FCF_SIZEBORDER | FCF_MAXBUTTON |
                                    FCF_VERTSCROLL | FCF_HORZSCROLL;
     } /* endif */

     // set size an position to same values as active window
     LoadStruct.rclPos = pstEQFGen->rclSource;

     // move window one titlebar height down
     EQFBValidatePositions( &LoadStruct.rclPos, TRNOTE_DOC );

     LoadStruct.pFileName = EMPTY_STRING;
     LoadStruct.docType = TRNOTE_DOC;
     LoadStruct.pszEQFTagTable = (PSZ)pstEQFGen->szEQFTagTable;
     LoadStruct.pszTagTable = (PSZ)pstEQFGen->szTagTable;

     EQFBDocLoad( &LoadStruct);        // load document and pass LoadStruct
     pTRNoteDoc = LoadStruct.pDoc;
     pTRNoteDoc->twin = pDoc;          // anchor translation document as twin
     EQFBPrepTRNoteSegs(pDoc, LoadStruct.pDoc);
     if ( !(pTRNoteDoc->pTokBuf) )
     {
        UtlAlloc((PVOID *) &pTRNoteDoc->pTokBuf, 0L,
                 (LONG) TOK_BUFFER_SIZE, ERROR_STORAGE );
     } /* endif */
     rc = ( pTRNoteDoc->pTokBuf ) ? 0 : ERR_NOMEMORY;
     if (!rc )
     {
       LoadStruct.pDoc->fLineWrap = TRUE;
       LoadStruct.pDoc->fAutoLineWrap = TRUE;
       LoadStruct.pDoc->sRMargin = (SHORT)LoadStruct.pDoc->lScrnCols;
       EQFBSoftLFInsert(LoadStruct.pDoc);

       EQFBFuncTopDoc(LoadStruct.pDoc);
       if ( !pDoc->EQFBFlags.PostEdit )
       {
         EQFBGotoTRNoteSeg(LoadStruct.pDoc,pDoc->tbActSeg.ulSegNum);
       }
       else
       {
         EQFBGotoTRNoteSeg(LoadStruct.pDoc,pDoc->TBCursor.ulSegNum);
       } /* endif */
     } /* endif */

      LoadStruct.pDoc->Redraw |= REDRAW_ALL;   // force repaint...
      /****************************************************************/
      /* set the display style and disable the SpellCheck ...         */
      /****************************************************************/
      LoadStruct.pDoc->DispStyle = DISP_PROTECTED;   // protect SGML tags

      /****************************************************************/
      /* set focus on newly loaded document...                        */
      /****************************************************************/
      SendMessage( pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                     0, MP2FROMHWND( LoadStruct.pDoc->hwndFrame ));

     EQFBScreenData( LoadStruct.pDoc );
   } /* endif */

} /* end of function EQFBFuncOpenTRNote */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBPrepTRNoteSegs
//-----------------------------------------------------------------------------
// Function call:     EQFBPrepTRNoteSegs(PTBDOCUMENT, PTBDOCUMENT )
//-----------------------------------------------------------------------------
// Description:       fill segmenttable with TRNotes
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     loop thru all segments and fill SegTable with all trnotes
//
//-----------------------------------------------------------------------------
static
VOID EQFBPrepTRNoteSegs
(
  PTBDOCUMENT  pDoc,
  PTBDOCUMENT  pTRNoteDoc
)
{
  ULONG       ulSegNum = 1;
  PTBSEGMENT  pSeg;
  USHORT      usTRNoteSegNum = 1;
  USHORT      usRC = NO_ERROR;
  USHORT      usColPos = 0;
  PSTARTSTOP  pstCurrent;
  BOOL        fTRNoteFound = FALSE;
  TBSEGMENT   tbSegment;
  TBSEGMENT   tbPrepSeg;
  PLOADEDTABLE   pTagTable;
  USHORT         usCommentStartLen;
  USHORT         usCommentEndLen;
  CHAR           chNoNote[MAX_DESCRIPTION];
  CHAR_W         chNoNoteW[MAX_DESCRIPTION];
  BOOL           fToggleColor = TRUE;
  ULONG            ulSysPrefOEMCP = 0L;
  ULONG            ulTRNote1LenChar = 0L;
  ULONG            ulTRNote2LenChar = 0L;

  ulSysPrefOEMCP = GetLangOEMCP( NULL);

  pTagTable    = (PLOADEDTABLE) pDoc->pDocTagTable;
  usCommentStartLen = (USHORT) strlen ( pTagTable->pTagTable->chStartText) + 1;
  usCommentEndLen = (USHORT) strlen ( pTagTable->pTagTable->chEndText) + 1;

  ulTRNote1LenChar = pTagTable->ulTRNote1Len / sizeof(CHAR_W);
  ulTRNote2LenChar = pTagTable->ulTRNote2Len / sizeof(CHAR_W);

  memset( &tbSegment, 0, sizeof(TBSEGMENT) );
  tbSegment.pDataW = EMPTY_STRINGW;
  if (EQFBAllocSeg(&tbSegment, &tbPrepSeg))
  {
    EQFBAddSegW(pTRNoteDoc, &tbPrepSeg );
  } /* endif */

  pSeg = EQFBGetSegW(pDoc->twin, ulSegNum);
  while ( pSeg )
  {
    fTRNoteFound = FALSE;
    if (pDoc->hwndRichEdit && (pSeg->pusBPET) )
    {
      UtlAlloc((PVOID *) &pSeg->pusBPET, 0L, 0L, NOMSG );
    } /* endif */
     if (pSeg->pusBPET == NULL )
     {
       usRC = TACreateProtectTableW(pSeg->pDataW,
                                   pDoc->pDocTagTable,
                                   usColPos,
                                   (PTOKENENTRY) pDoc->pTokBuf,
                                   TOK_BUFFER_SIZE,
                                   (PSTARTSTOP * ) &(pSeg->pusBPET),
                                   pDoc->pfnUserExit,
                                   pDoc->pfnUserExitW, pDoc->twin->ulOemCodePage);
     } /* endif */
     if (!usRC )
     {
       pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
       while ( ( pstCurrent->usType != 0) &&
               (pstCurrent->usType != TRNOTE_CHAR) )
       {
         pstCurrent++;
       } /* endwhile */
       if (pstCurrent->usType == TRNOTE_CHAR )
       {
         memset( &tbSegment, 0, sizeof( tbSegment ));
         fTRNoteFound = TRUE;
         /*************************************************************/
         /* decide whether it is a note of level 1 or 2               */
         /*************************************************************/
         if (memicmp((PBYTE)pTagTable->chTrnote1TextW,
                    (PBYTE)(pSeg->pDataW + (pstCurrent->usStart) + usCommentStartLen),
                    pTagTable->ulTRNote1Len ) == 0 )
         {

           tbSegment.pDataW = pSeg->pDataW + pstCurrent->usStart
                           + usCommentStartLen + ulTRNote1LenChar;

           tbSegment.usLength = (USHORT)(pstCurrent->usStop - pstCurrent->usStart + 1
                              - usCommentStartLen - ulTRNote1LenChar
                              - usCommentEndLen);

           tbSegment.qStatus = (USHORT)((fToggleColor) ? QF_TRNOTE_L1_1:QF_TRNOTE_L1_2);
           fToggleColor = !fToggleColor;
         }
         else
         {
           tbSegment.pDataW = pSeg->pDataW + pstCurrent->usStart
                           + usCommentStartLen + ulTRNote2LenChar;

           tbSegment.usLength = (USHORT)(pstCurrent->usStop - pstCurrent->usStart + 1
                              - usCommentStartLen - ulTRNote2LenChar
                              - usCommentEndLen);
           tbSegment.qStatus = QF_TRNOTE_L2;

         } /* endif */

         tbSegment.ulSegNum = usTRNoteSegNum;
         tbSegment.usOrgLength = tbSegment.usLength;
         /*************************************************************/
         /* store is usShrinkLen real pSeg->ulSegNum                  */
         /*************************************************************/
         tbSegment.ulShrinkLen = pSeg->ulSegNum;

         if (EQFBAllocSeg(&tbSegment, &tbPrepSeg))
         {
           EQFBAddSegW(pTRNoteDoc, &tbPrepSeg );
           usTRNoteSegNum ++;
         } /* endif */
       } /* endif */
     } /* endif */

    ulSegNum ++;
    pSeg = EQFBGetSegW(pDoc->twin, ulSegNum);
  } /* endwhile */
  /********************************************************************/
  /* add lf at end of seg                                             */
  /********************************************************************/
  if (usTRNoteSegNum  > 1 )
  {
    EQFBAddLFSeg (pTRNoteDoc,&tbSegment,&tbPrepSeg,usTRNoteSegNum );
//    usTRNoteSegNum++;
  }
  else
  {
    /******************************************************************/
    /* if no trnote found in document, "no entry available" is dis-   */
    /* played in trnotes window                                       */
    /******************************************************************/
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    WinLoadString ((HAB) UtlQueryULong( QL_HAB ), hResMod, SID_NO_ENTRY,
                   sizeof(chNoNote), chNoNote );
    /*************************************************************/
    /* convert string to OEM, because we assume everything is    */
    /* in OEM and convert it only during display....             */
    /*************************************************************/
    AnsiToOem( chNoNote, chNoNote );

    tbSegment.ulSegNum = usTRNoteSegNum;
    tbSegment.pusBPET = NULL;
    tbSegment.usLength = (USHORT) strlen(chNoNote);
    tbSegment.usOrgLength = tbSegment.usLength;
    tbSegment.pDataW = ASCII2Unicode( chNoNote, chNoNoteW, pDoc->ulOemCodePage );
    tbSegment.qStatus = QF_TRNOTE_L1_1;

    if (EQFBAllocSeg(&tbSegment, &tbPrepSeg))
    {
      EQFBAddSegW(pTRNoteDoc, &tbPrepSeg );
      usTRNoteSegNum++;
    } /* endif */

  } /* endif */
  pTRNoteDoc->ulMaxSeg = usTRNoteSegNum;

} /* end of function EQFBFuncPrepTRNoteSegs */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBAllocSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBAllocSeg(PTBSEGMENT, PTBSEGMENT)
//-----------------------------------------------------------------------------
// Description:       allocate space for trnotes segments
//-----------------------------------------------------------------------------
// Parameters:        PTBSEGMENT     pTBFromSeg
//                    PTBSEGMENT     pTBToSeg
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:
//-----------------------------------------------------------------------------
static
SHORT EQFBAllocSeg
(
  PTBSEGMENT   pTBFromSeg,
  PTBSEGMENT   pTBToSeg
)
{
  PSZ_W   pData;
  BOOL    fResult = TRUE;
  USHORT  usLen;

  memcpy(pTBToSeg, pTBFromSeg, sizeof(TBSEGMENT));

  usLen = pTBFromSeg->usLength;

  fResult = UtlAlloc( (PVOID *) &pData, 0L,
                   (LONG)max((usLen+3)*sizeof(CHAR_W), MIN_ALLOC), NOMSG );
  if (pData )
  {
    UTF16strncpy(pData, pTBFromSeg->pDataW, usLen);
    pTBToSeg->pDataW = pData;
    if (usLen && (*(pTBFromSeg->pDataW + usLen-1 ) != '\n'))
    {
      *(pTBToSeg->pDataW+usLen) = '\n';
      *(pTBToSeg->pDataW + usLen+1) = EOS;
      pTBToSeg->usLength ++;
    } /* endif */

  } /* endif */
  return ((SHORT) fResult);
} /* end of function EQFBAllocSeg           */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBAddLFSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBAddLFSeg(PTBSEGMENT, PTBSEGMENT)
//-----------------------------------------------------------------------------
// Description:       allocate space for trnotes segments
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    pTRNoteDoc
//                    PTBSEGMENT     pTbSegment
//                    PTBSEGMENT     ptbPrepSeg
//                    USHORT         usTRNoteSegNum
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     fill    ptbSegment
//                    Alloc space for ptbPrepSeg and fill it
//                    if ok: add ptbPrepSeg to TRNoteDoc segment table
//-----------------------------------------------------------------------------
static
VOID  EQFBAddLFSeg
(
  PTBDOCUMENT  pTRNoteDoc,
  PTBSEGMENT   ptbSegment,
  PTBSEGMENT   ptbPrepSeg,
  USHORT       usTRNoteSegNum
)
{
  memset( ptbSegment, 0, sizeof(TBSEGMENT) );
  ptbSegment->pDataW = L"\n";
  ptbSegment->usLength = (USHORT) UTF16strlenCHAR(ptbSegment->pDataW);
  ptbSegment->qStatus = QF_TRNOTE_L1_1;
  ptbSegment->ulSegNum = usTRNoteSegNum;
  if (EQFBAllocSeg(ptbSegment, ptbPrepSeg))
  {
    EQFBAddSeg(pTRNoteDoc, ptbPrepSeg );
  } /* endif */
  return ;
} /* end of function EQFBAddLFSeg           */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBGotoTRNoteSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBGotoTRNoteSeg(PTBDOCUMENT, ulSegNum)
//-----------------------------------------------------------------------------
// Description:       position and activate active trnote
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    pTBDoc
//                    USHORT         ulSegNum   seg for which trnote searched
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     while not found and not at end of TRNote document
//                      get next segment
//                      check reference segment number ( in field ulShrinkLen!)
//                      if reference segment is equal to requested segnum
//                      or if reference segnum passed already the requested
//                      segnum
//                        found!
//                   endwhile
//                   if found
//                   set QF_CURRENT to segnum found
//                   reset coloring in rest of trnote document
//-----------------------------------------------------------------------------
static
VOID  EQFBGotoTRNoteSeg
(
  PTBDOCUMENT  pTRNoteDoc,
  ULONG        ulSegNum
)
{
  BOOL        fFound = FALSE;
  ULONG       ulTRNoteSegNum = 0;
  PTBSEGMENT  pSeg;
  ULONG       ulFoundSegNum = 0;

  ulFoundSegNum = EQFBFindTRNoteSeg(pTRNoteDoc, ulSegNum);

  if (ulFoundSegNum )
  {
    /******************************************************************/
    /* remove QF_CURRENT from old current seg                         */
    /******************************************************************/
    while (!fFound && (ulTRNoteSegNum <= pTRNoteDoc->ulMaxSeg ))
    {
      pSeg = EQFBGetSegW(pTRNoteDoc, ulTRNoteSegNum);
      if (pSeg && (pSeg->qStatus == QF_CURRENT))
      {
        pSeg->qStatus = pTRNoteDoc->tbActSeg.qStatus;    // reset to old status
        fFound = TRUE;
      } /* endif */
      ulTRNoteSegNum++;
    } /* endwhile */
    ulTRNoteSegNum = 0;
    fFound = FALSE;
    while (!fFound && (ulTRNoteSegNum <= pTRNoteDoc->ulMaxSeg ))
    {
      pSeg = EQFBGetSegW(pTRNoteDoc, ulTRNoteSegNum);
      if (pSeg && (ulTRNoteSegNum == ulFoundSegNum ))
      {
        pTRNoteDoc->tbActSeg.qStatus = pSeg->qStatus;  // remember old status
        pSeg->qStatus = QF_CURRENT;                    // set active segment
        fFound = TRUE;
      } /* endif */
      ulTRNoteSegNum++;
    } /* endwhile */
    EQFBGotoSeg(pTRNoteDoc, ulFoundSegNum, 0);
  } /* endif */
  return ;
} /* end of function EQFBGotoTRNoteSeg           */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBFindTRNoteSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBFindTRNoteSeg(PTBDOCUMENT, ulSegNum)
//-----------------------------------------------------------------------------
// Description:       find trnote seg which belongs to ulSegNum in STARGET_DOC
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    pTBDoc     TRNOTE document
//                    USHORT         ulSegNum   seg for which trnote searched
//-----------------------------------------------------------------------------
// Returncode type:   USHORT  segnum    if found
//                            0   if none found
//-----------------------------------------------------------------------------
// Function flow:     while not found and not at end of TRNote document
//                      get next segment
//                      check reference segment number ( in field ulShrinkLen!)
//                      if reference segment is equal to requested segnum
//                      or if reference segnum passed already the requested
//                      segnum
//                        found!
//                   endwhile
//                   if found
//                   set QF_CURRENT to segnum found
//                   reset coloring in rest of trnote document
//-----------------------------------------------------------------------------
static
ULONG  EQFBFindTRNoteSeg
(
  PTBDOCUMENT  pTRNoteDoc,
  ULONG        ulSegNum
)
{
  BOOL        fFound = FALSE;
  ULONG       ulTRNoteSegNum = 0;
  ULONG       ulMaxTRNote = 0;
  PTBSEGMENT  pSeg;
  ULONG       ulLastSegNum = 0;
  ULONG       ulFoundSegNum = 0;

  ulMaxTRNote = pTRNoteDoc->ulMaxSeg;

  while (!fFound && (ulTRNoteSegNum < ulMaxTRNote ))
  {
    pSeg = EQFBGetSegW(pTRNoteDoc, ulTRNoteSegNum);
    if (pSeg  )
    {
      if (pSeg->ulShrinkLen != 0 )
      {
        if (pSeg->ulShrinkLen == ulSegNum )
        {
          fFound = TRUE;
          ulFoundSegNum = ulTRNoteSegNum;
        }
        else if (pSeg->ulShrinkLen > ulSegNum )
        {
          fFound = TRUE;
          if (ulLastSegNum )
          {
            ulFoundSegNum = ulTRNoteSegNum - 1;
          }
          else
          {
             ulFoundSegNum = ulTRNoteSegNum;
          } /* endif */
        }
        else               //pSeg->ulShrinkLen < ulSegNum
        {
          ulLastSegNum = pSeg->ulShrinkLen;   // remember it, goon searching
        } /* endif */
      } /* endif */
    } /* endif */
    ulTRNoteSegNum++;
  } /* endwhile */
  if (!fFound && (ulTRNoteSegNum == ulMaxTRNote) && ulLastSegNum)
  {
    fFound = TRUE;
    ulFoundSegNum = ulTRNoteSegNum -1;
  } /* endif */
  return (ulFoundSegNum);
} /* end of function EQFBFindTRNoteSeg           */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBOnTRNote
//-----------------------------------------------------------------------------
// Function call:     EQFBOnTRNote(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       if cursor on TRNOTE: display TRNOTE, else do nothing
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    pDoc
//-----------------------------------------------------------------------------
// Returncode type:  BOOL
//-----------------------------------------------------------------------------
// Returncodes:       TRUE         - cursor on TRNote, display TRNOte
//                    FALSE        - cursor not on TRNote
//-----------------------------------------------------------------------------
// Function flow:     if cursor in TRNOTE_DOC:
//                       find STARGET_DOC
//                       position STARGET_DOC at segnum which is referenced
//                       ( ulShrinkLen!) by TRNOTE-segnum of cursor position
//                    else
//                      if cursor on a TRNote indicator
//                         open TRNote window and display TRNOTE
//                      endif
//                    endif
//-----------------------------------------------------------------------------
BOOL  EQFBOnTRNote
(
  PTBDOCUMENT  pDoc
)
{
  BOOL         fOnTRNote = FALSE;
  USHORT       usType_Char;
  PTBSEGMENT   pSeg;
  PTBDOCUMENT  pDocCurrent;
  ULONG        ulFoundSegNum = 0;

  if (pDoc->docType == TRNOTE_DOC )
  {
    /******************************************************************/
    /* activate the TRNote on which the dblclick happened             */
    /* and position to corresponding segm. in transl. window          */
    /******************************************************************/
    pSeg = EQFBGetSegW(pDoc, pDoc->TBCursor.ulSegNum);
    if (pSeg && pSeg->ulShrinkLen )
    {
        pDocCurrent = pDoc->twin;

        if (pDocCurrent->docType == STARGET_DOC )
        {
          EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, 0);
          pDoc->Redraw |= REDRAW_ALL;            // force repaint...

          fOnTRNote = TRUE;

          EQFBGotoSeg( pDocCurrent, pSeg->ulShrinkLen, 0);
          pDocCurrent->Redraw |= REDRAW_ALL;   // force repaint...
          EQFBScreenData(pDoc);
          EQFBRefreshScreen(pDocCurrent);

          PostMessage(((PSTEQFGEN)pDocCurrent->pstEQFGen)->hwndTWBS,
                        WM_EQF_SETFOCUS, 0,
                        MP2FROMHWND(pDocCurrent->hwndFrame));
        } /* endif */
    } /* endif */
  }
  else if (pDoc->docType == STARGET_DOC)
  {
    pSeg = EQFBGetSegW(pDoc, pDoc->TBCursor.ulSegNum);
    if (pSeg )
    {
      usType_Char = EQFBCharType(pDoc,pSeg,pDoc->TBCursor.usSegOffset);
      if (usType_Char == TRNOTE_CHAR )
      {
        fOnTRNote = TRUE;
         pDocCurrent = pDoc->next;
         while ((pDocCurrent->docType != TRNOTE_DOC) && (pDocCurrent != pDoc ))
         {
           pDocCurrent = pDocCurrent->next;
         } /* endwhile */

         if (pDocCurrent->docType == TRNOTE_DOC )
         {
           /**************************************************************/
           /* if TRNOTE_DOC already exists, reposition to corresponding  */
           /* seg in TRNOTE_DOC                                          */
           /**************************************************************/
           ulFoundSegNum = EQFBFindTRNoteSeg (pDocCurrent,
                                              pDoc->TBCursor.ulSegNum);
           if (ulFoundSegNum )
           {
             EQFBGotoSeg( pDocCurrent, ulFoundSegNum, 0);
             pDocCurrent->Redraw |= REDRAW_ALL;   // force repaint...
             EQFBScreenData(pDocCurrent);
             EQFBRefreshScreen(pDocCurrent);
           } /* endif */
           WinShowWindow( pDocCurrent->hwndFrame, TRUE );

           PostMessage(((PSTEQFGEN)pDocCurrent->pstEQFGen)->hwndTWBS,
                         WM_EQF_SETFOCUS, 0,
                         MP2FROMHWND(pDocCurrent->hwndFrame));
         }
         else
         {
            /*************************************************************/
            /* if TRNOTE_DOC does not exist, create it and activate seg  */
            /*************************************************************/
           EQFBFuncOpenTRNote(pDoc);
         } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  return (fOnTRNote);
} /* end of function EQFBOnTRNote           */


//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBFilePrint
//-----------------------------------------------------------------------------
// Function call:     EQFBFilePrint(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       use the segment table information and print the document
//
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc        - ptr to active doc structure
//-----------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_WRITEFILE  - error writing file
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Side effects:      calls to the following routines:
//                          UtlPrintOpen, UtlPrintLine, UtlPrintClose
//-----------------------------------------------------------------------------
// Function flow:     - open printer
//                    - get the EQF segment table information
//                    - scan through the segment table and print the text
//                      data of the segments
//                    - close printer
//-----------------------------------------------------------------------------
USHORT EQFBFilePrint
(
   PTBDOCUMENT  pDoc                     // ptr to document control structure
)
{
   HPRINT      hPrint = NULLHANDLE;    // print handle
   ULONG       ulI;                    // general loop index
   BOOL        fOK = TRUE;             // internal OK flag
   PTBSEGMENT  pSeg;                   // ptr to segment
   PSZ_W       pszLineFeed;            // position of next linefeed character
   PSZ_W       pszBuffer;              // pointer to print buffer
   PSZ_W       pszSource;              // pointer into segment data
   PSZ_W       pszTarget;              // pointer into print buffer
   ULONG       ulFilled;               // # of bytes in print buffer
   ULONG       ulLength;               // length of currently processed string
   CHAR_W      chChar;                 // temp character

   // allocate the print line buffer
   fOK = UtlAlloc((PVOID *) &pszBuffer, 0L, (LONG) (MAX_SEGMENT_SIZE+2)* sizeof(CHAR_W), ERROR_STORAGE );
   pszTarget = pszBuffer;              // start at buffer begin
   ulFilled = 0L;                       // nothing in print buffer yet


   // open the printer device context
   if ( fOK )
   {
      CHAR chTitle[ MAX_PATH144 ];
      CHAR chEqfName[ MAX_EQF_PATH ];
      CHAR chText[ 40 ];
      PSZ  pFolder, pFile;
	  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

      switch ( pDoc->docType )
      {
        case SSOURCE_DOC:
        case STARGET_DOC:
          chTitle[ 0 ] = EOS;
          strcpy( chEqfName, pDoc->szDocName );
          pFile = UtlGetFnameFromPath( chEqfName );
          if ( pFile )
          {
            /**********************************************************/
            /* chEQFName = "c:\eqf\sample1.f00\sTarget\"              */
            /**********************************************************/
            pFolder = pFile - 1;
            *pFolder = EOS;
            pFolder = UtlGetFnameFromPath( chEqfName );
            if ( pFolder )
            {
              pFolder -= 5;
              *pFolder = EOS;
              pFolder = UtlGetFnameFromPath( chEqfName );
            } /* endif */
            LOADSTRING( (HAB) UtlQueryULong( QL_HAB ),
                        hResMod, IDS_TB_PRINTFILE_PRTJOB, chText );
            sprintf( chTitle, chText, pFolder, pDoc->szDocLongName );
          } /* endif */
          break;
        case  OTHER_DOC:
          strcpy( chTitle, pDoc->szDocName );
          break;
        case TRNOTE_DOC:
          LOADSTRING( (HAB) UtlQueryULong( QL_HAB ),
                      hResMod, SID_TRNOTE_TITLE, chText );
          break;
        default :
          strcpy( chTitle, pDoc->szDocName );
          break;
      } /* endswitch */

      fOK = UtlPrintOpen( &hPrint, chTitle, NULLHANDLE );

      /****************************************************************/
      /* Print title string as header text ...                        */
      /****************************************************************/
      if ( fOK )
      {
    CHAR_W chTitleW[ 40 ];
    ASCII2Unicode( chTitle, chTitleW, pDoc->ulOemCodePage );
        if ( pDoc->ulOemCodePage == 874L )
        {
            UtlPrintSetAnsiConv( hPrint, FALSE );   // conversion disturbs Thai chars
        }

        fOK = UtlPrintLineW( hPrint, chTitleW );
              UtlPrintLineW( hPrint, EMPTY_STRINGW );
      } /* endif */
   } /* endif */


   // get the EQF segment table information
   ulI = 0;                                                      /* @KAT0028C */

   // scan through the segment table and write text data to the printer
   if ( fOK )
   {
     do {
        // get pointer to current segment
        ulI ++;                                                  /* @KAT0028A */
        pSeg = EQFBGetVisSeg(pDoc, &ulI);                        /* @KAT0028C */

        if ( pSeg && pSeg->pDataW )
        {
          CHAR_W bEnd = 0;
          CHAR_W c;
          CHAR_W chTemp;
           // start at begin of segment data
           pszSource = pSeg->pDataW;

           ulLength = UTF16strlenCHAR( pszSource );
           if (ulLength > pSeg->usLength)
           {
             bEnd = pSeg->pDataW[ pSeg->usLength ];
             pSeg->pDataW[ pSeg->usLength ] = EOS;
           } /* endif */

           // add data up to next linefeed or softlf character to print line
           pszLineFeed = pszSource;
           while ( ((c = *pszLineFeed)!= NULC) && (c != LF) && (c!=SOFTLF_CHAR) )
           {
             pszLineFeed++;
           } /* endwhile */

           while ( *pszLineFeed && fOK )
           {
              chTemp = *pszLineFeed;
              *pszLineFeed = LF;

              pszLineFeed ++;
              chChar = *pszLineFeed;   // store character
              *pszLineFeed = EOS;
              ulLength = UTF16strlenCHAR( pszSource );
              if ( ulLength + ulFilled >= MAX_SEGMENT_SIZE )
              {
                 fOK = UtlPrintLineW( hPrint, pszBuffer );
                 ulFilled = 0;
                 *pszBuffer = EOS;
                 pszTarget = pszBuffer;
              } /* endif */
              if ( fOK  )
              {
                UTF16strcat( pszTarget, pszSource );
                fOK = UtlPrintLineW( hPrint, pszBuffer );
                ulFilled = 0;
                *pszBuffer = EOS;
                pszTarget = pszBuffer;
                *(pszLineFeed-1) = chTemp;
                *pszLineFeed = chChar;        // restore original character
                pszSource = pszLineFeed ;
                while ( ((c = *pszLineFeed) != NULC) && (c != LF) && (c!=SOFTLF_CHAR) )
                {
                  pszLineFeed++;
                } /* endwhile */
              } /* endif */
           } /* endwhile*/

           if ( fOK )
           {
             ulLength = UTF16strlenCHAR( pszSource );
             if ( ulLength + ulFilled >= MAX_SEGMENT_SIZE )
             {
                fOK = UtlPrintLineW( hPrint, pszBuffer );
                ulFilled = 0L;
                *pszBuffer = EOS;
                pszTarget = pszBuffer;
             } /* endif */
             UTF16strcat( pszTarget, pszSource );
             ulFilled += ulLength;
           } /* endif */
           if (bEnd)
           {
             pSeg->pDataW[pSeg->usLength] = bEnd;
             bEnd = EOS;
           } /* endif */
        } /* endif */
     } while ( fOK && (ulI < pDoc->ulMaxSeg) ); /* enddo */
   } /* endif */

   if ( hPrint )         UtlPrintClose( hPrint );
   if ( pszBuffer )
   {
      UtlAlloc((PVOID *) &pszBuffer, 0L, 0L, NOMSG );
   } /* endif */

   return( ( fOK ) ? 0 : ERR_NOMEMORY );
} /* end of EQFBFilePrint */

static USHORT EQFBCheckInAbbrev ( PTBSEGMENT, PSTARTSTOP, USHORT );

USHORT EQFBCharType
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   PTBSEGMENT  pSeg,                    // number of active segment
   USHORT      usOffs                   // offset in active segment
)
{
   USHORT     usType= UNPROTECTED_CHAR;  // type of active character
   PSTARTSTOP  pstCurrent= NULL;         // ptr to entries of start/stop table
   static USHORT usMsg = ERROR_STORAGE;  // display message the first time
   HWND        hwndTemp;                 // temp. window handle
   PSTEQFGEN   pstEQFGen;                // pointer to generic structure
   DISPSTYLE   DispStyle;                // local display style active
   ULONG       ulInTagAbbrLen;           // length of compact inline tag
   USHORT      usStart;                  // holds pstCurrent->usStart
   USHORT      usRC = NO_ERROR;          // no error
   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

   /*******************************************************************/
   /* correct local display style if current segment is expanded      */
   /*******************************************************************/
   if (pSeg )
   {
     DispStyle = (pSeg->SegFlags.Expanded) ?
                        DISP_PROTECTED : (pDoc->DispStyle);


     if ( pSeg->pusBPET == NULL )
     {
	   usRC = TAIteratedCreateProtectTableW(pDoc, pSeg, 10);

       // issue a close request to the translation environment window
       if ( pDoc->pstEQFGen && usRC )
       {
         if ( !pDoc->fErrorProcessed )
         {
           UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
         } /* endif */
         pDoc->fErrorProcessed = TRUE;       // error is processed
         pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
         hwndTemp = pDoc->hwndFrame;
         UtlDispatch();                       // display error message
         pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );
         WinPostMsg( pstEQFGen->hwndTWBS, WM_CLOSE, NULL, NULL );
       } /* endif */
     } /* endif */

     if ( pDoc && !usRC)
     {
       if (pSeg->pusBPET )
       {
          // look for position in start/stop table
          pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
          while ( (pstCurrent->usType != 0) && (usOffs > pstCurrent->usStop) )
          {
             pstCurrent++;
          } /* endwhile */
          if (pstCurrent->usType != 0)
          {
             usType = pstCurrent->usType;
          }
          else
          {
             usType = ENDOFSEG_CHAR;
          } /* endif */
       }
       else
       {
          usType = UNPROTECTED_CHAR;          // use this as default
       } /* endif */
       if (pSeg->usLength <= usOffs )
       {
         usType = ENDOFSEG_CHAR;
       }
       else if ((usType != TRNOTE_CHAR) && (*(pSeg->pDataW+usOffs) == '\n' ) )
       {
         usType = LINEBREAK_CHAR;
         if ( (DispStyle == DISP_COMPACT) && (pSeg->qStatus != QF_NOP) )
         {
           /************************************************************/
           /* check whether linebreak is in a inline tag and whether   */
           /* it is hidden or not                                      */
           /************************************************************/
           usType = EQFBCheckCompactLF(pDoc,pSeg,usOffs);
         } /* endif */
         if (DispStyle == DISP_SHORTEN )
         {
           /*************************************************************/
           /* check whether linebreak is hidden or not                  */
           /*************************************************************/
           usType = EQFBCheckCompactLF(pDoc, pSeg, usOffs);
         } /* endif */
       } /* endif */

       if ( pDoc->fLineWrap && pDoc->fAutoLineWrap
            &&  (usType != ENDOFSEG_CHAR)
            &&  (*(pSeg->pDataW+usOffs) == SOFTLF_CHAR))
       {
         if (*(pSeg->pDataW+usOffs+1) == SOFTLF_CHAR)
         {
           /*************************************************************/
           /* next char is SOFTLF_CHAR too; use usType of BPET          */
           /*************************************************************/
         }
         else if ( usOffs && (*(pSeg->pDataW+usOffs-1) == SOFTLF_CHAR) )
         {
           /*************************************************************/
           /* double SOFTLF : display first one, hide 2nd one           */
           /* ( if AutoLinewrap is ON, and SOFTLF occurrs in the file,  */
           /* it has been duplicated )                                  */
           /*************************************************************/
           usType = HIDDEN_CHAR;                //prev.char is SOFTLF too
         }
         else
         {
           usType = LINEBREAK_CHAR;             // single SOFTLF is softlinefeed
         } /* endif */
       } /* endif */

       //--- correct QF_NOP depending on current display style ---
       if ( DispStyle == DISP_HIDE && (pSeg->qStatus == QF_NOP) )
       {
          switch ( usType )
          {
             case PROTECTED_CHAR:
             case UNPROTECTED_CHAR:
                usType = HIDDEN_CHAR;
                break;
             default:
                break;
          } /* endswitch */
       } /* endif */

       if (DispStyle == DISP_SHORTEN )
       {
         EQFBCompactPlusOne( pSeg, pstCurrent, usOffs, &usType);
       }
       else
       {
         //--- correct usType depending on current display style ---
         if ( usType == PROTECTED_CHAR )
         {
            switch ( DispStyle )
            {
               case DISP_HIDE:
                  usType = HIDDEN_CHAR;
                  break;
                case  DISP_COMPACT:
                  ulInTagAbbrLen = strlen (pEQFBUserOpt->szInTagAbbr);
                  if ( pSeg->qStatus != QF_NOP )       //now inline tag!!
                  {
                    usStart = pstCurrent->usStart;
                    if ( usStart == pstCurrent->usStop)
                    {
            // leave character as it is...
          }
          else
                    if ( (usStart <= usOffs) &&
                         (usOffs <= (usStart + ulInTagAbbrLen-1)) )
                    {

                      if ( (pstCurrent == (PSTARTSTOP) pSeg->pusBPET) ||
                            ((pstCurrent-1)->usType != PROTECTED_CHAR) )
                      {
                        usType = COMPACT_CHAR;
                      }
                      else
                      {
                        usType = HIDDEN_CHAR;
                      } /* endif */
                    }
                    else
                    {
                      usType = HIDDEN_CHAR;
                    } /* endif */
                  } /* endif */
                  break;
      //       case DISP_PROTECTED:
      //          usType = PROTECTED_CHAR;
      //          break;
               case DISP_UNPROTECTED:
                  usType = UNPROTECTED_CHAR;
                  break;
            } /* endswitch */
         } /* endif */
       } /* endif */

       /*****************************************************************/
       /* correct type TRNOTE_CHAR depending on current display style   */
       /*****************************************************************/
       if (usType == TRNOTE_CHAR )
       {
         usType = EQFBCheckInAbbrev( pSeg, pstCurrent, usOffs);
       } /* endif */
     } /* endif */
   }
   else
   {
     usType = UNPROTECTED_CHAR;  // should not occur!
   } /* endif */

   return( usType );
} /* end of EQFBCharType */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckInAbbrev
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckInAbbrev  (PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       check whether current offset is in visible part of trnote
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    PTBSEGMENT  pSeg
//                    PSTARTSTOP  pstCurrent
//                    USHORT      usOffs
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       usType     TRNOTE_CHAR  if at begin of  tag
//                               HIDDEN_CHAR  else
//-----------------------------------------------------------------------------
// Prerequesits:      start/stop table exists
//-----------------------------------------------------------------------------
// Function flow:     if at start of trnote: return TRNOTE_CHAR
//                    else
//                      if trnote contains a linebreak: return LINEBREAK_CHAR
//                      else return HIDDEN_CHAR
//-----------------------------------------------------------------------------
static
USHORT EQFBCheckInAbbrev
(
   PTBSEGMENT  pSeg,                    // number of active segment
   PSTARTSTOP  pstCurrent,              // start-stop region of offset
   USHORT      usOffs                   // offset in active segment
)
{
   ULONG      ulLen;
   USHORT     usStart;
   USHORT     usType;
   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

   ulLen = strlen ( pEQFBUserOpt->chTRNoteAbbr);
   usStart = pstCurrent->usStart;
   if ((usStart <= usOffs) &&
       (usOffs <= (usStart + ulLen - 1))  )
   {
      usType = TRNOTE_CHAR;     // type TRNOTE remains
   }
   else
   {
     if (  (usOffs == pSeg->usLength-1) &&
            ( *(pSeg->pDataW + usOffs ) == '\n') )
     {
       usType = LINEBREAK_CHAR;
     }
     else
     {
       usType = HIDDEN_CHAR;
     } /* endif */
   } /* endif */
   /*******************************************************************/
   /* is it nec to distinguish between inline and NOP segments???     */
   /*******************************************************************/
   return( usType );
} /* end of function EQFBCheckInAbbrev */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCompactPlusOne
//-----------------------------------------------------------------------------
// Function call:     EQFBCompactPlusOne (PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       correct usType for Compact plus one style
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    PTBSEGMENT  pSeg
//                    PSTARTSTOP  pstCurrent
//                    USHORT      usOffs
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       usType     PROTECTED_CHAR  at begin of  tag
//                               HIDDEN_CHAR  else
//-----------------------------------------------------------------------------
// Prerequesits:      start/stop table exists
//-----------------------------------------------------------------------------
// Function flow:     if at start o
//-----------------------------------------------------------------------------
static
VOID EQFBCompactPlusOne
(
   PTBSEGMENT  pSeg,                    // number of active segment
   PSTARTSTOP  pstCurrent,              // start-stop region of offset
   USHORT      usOffs,                           // offset in active segment
   PUSHORT     pusType
)
{
   USHORT     usStart = 0;
   USHORT     usType = *pusType;
   BOOL       fBlankFound = FALSE;
   BOOL       fCheckNec = FALSE;       // do not check type
   USHORT     usShortStart = 0;
   USHORT     usStop = 0;

   /********************************************************/
   /* display first word( blank and colon are word delimiter */
   /* and last character of tag, hide all other characters   */
   /********************************************************/
   /********************************************************/
   /* find start of protected parts                        */
   /********************************************************/
   if (pSeg->qStatus == QF_NOP )
   {
     if ((usType == PROTECTED_CHAR) || (usType == UNPROTECTED_CHAR) )
     {
       fCheckNec = TRUE;
       usStart = 0;
       usStop = pSeg->usLength-1;
     } /* endif */
   }
   else
   {
     if (usType == PROTECTED_CHAR )
     {
       if (pstCurrent->usStop - pstCurrent->usStart >= 10 )
       {
         usStart = pstCurrent->usStart;
         usStop = pstCurrent->usStop;
         fCheckNec = TRUE;
       } /* endif */
     } /* endif */
   } /* endif */
   /***************************************************************/
   /* find start of SHORTEN_CHAR: it is end of 1st word, but at   */
   /* most the tenth character                                    */
   /***************************************************************/
   if (fCheckNec )
   {
     while ( !fBlankFound )      // find start of SHORTEN_CHAR
     {
       if (usShortStart > 10 )
       {
         fBlankFound = TRUE;
       }
       else if (*(pSeg->pDataW + usStart + usShortStart) == BLANK )
       {
           fBlankFound = TRUE;
       } /* endif */
       usShortStart++;
     } /* endwhile */
     /***************************************************************/
     /* usOffs < usStart:  char is protected                        */
     /* usStart-1 <= usOffs <=  usStart+1: char is SHORTEN_CHAR("...")*/
     /* usStart+1 < usOffs: char is HIDDEN                          */
     /* last char of NOP or tag: PROTECTED                          */
     /***************************************************************/
     usShortStart = (USHORT)(usShortStart + usStart);
     // P018515: assure that tag does not become longer!

		 if (usOffs == usStop)
		 { // last char of NOP or last char of inline tag
		   usType = PROTECTED_CHAR;
		 }
		 else if (usOffs > usShortStart + 1 )
		 {
		   usType = HIDDEN_CHAR;
		 }
	     else if ((usShortStart + 1 < usStop) &&
	         ((usOffs == usShortStart + 1) || ( usOffs == usShortStart )
					 || ( usOffs == usShortStart-1 ) ))
		 {
		   usType = SHORTEN_CHAR;
		 }
		 else
		 {  // avoid UNPROTECTED_CHAR in NOP in compact+1 style!!!
		   usType = PROTECTED_CHAR;
		 } /* endif */
   } /* endif */

   *pusType = usType;
   return;
} /* end of function EQFBCompactPlusOne */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckCompactLF
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckCompactLF (PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       check whether current linefeed is the 1st in block
//                    of inline tags
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       usType     LINEBREAK_CHAR  if 1st LF in tag
//                               HIDDEN_CHAR  else
//-----------------------------------------------------------------------------
// Prerequesits:      start/stop table exists
//-----------------------------------------------------------------------------
// Function flow:     _
//-----------------------------------------------------------------------------
static
USHORT EQFBCheckCompactLF
(
   PTBDOCUMENT pDoc,
   PTBSEGMENT  pSeg,                    // number of active segment
   USHORT      usOffs                   // offset in active segment
)
{
  PSTARTSTOP pstCurrent;                // ptr to entries in start/stop table
  PSTARTSTOP pstTagBlockBegin;          // ptr to entries in start stop table
  BOOL       fLineFeed = FALSE;
  USHORT     usType = LINEBREAK_CHAR;
  PSZ_W      pTemp;                     // ptr to data string in segment
  PSZ_W      pDataOffs;

  pDoc;

   pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
   while ( (pstCurrent->usType != 0)
             && (usOffs > (pstCurrent->usStop + 1)) )
   {
      pstCurrent++;
   } /* endwhile */
   if ( pstCurrent->usType == PROTECTED_CHAR )
   {
     /*****************************************************************/
     /* loop back til begin of block of inline tags                   */
     /*****************************************************************/
     pstTagBlockBegin = pstCurrent;
     while ( pstTagBlockBegin->usType == PROTECTED_CHAR
             && pstTagBlockBegin != (PSTARTSTOP)pSeg->pusBPET )
     {
       pstTagBlockBegin --;
     } /* endwhile */
     if ( pstTagBlockBegin->usType != PROTECTED_CHAR )
     {
       pstTagBlockBegin ++;    //we went 1 too far
     } /* endif */

     pDataOffs = pSeg->pDataW + usOffs;
     pTemp = pSeg->pDataW + (pstTagBlockBegin->usStart);
     /*****************************************************************/
     /* find 1st LF in this block                                     */
     /*****************************************************************/
     while ( !fLineFeed && (pTemp < pDataOffs) )
     {
       fLineFeed = ((*pTemp) == LF );
       pTemp++;
     } /* endwhile */
     if ( fLineFeed )
     {
       usType = HIDDEN_CHAR;     //current LF is not 1st in block of
     } /* endif */               //inline tags
   } /* endif */
   return( usType );
} /* end of function EQFBCheckCompactLF */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBDiffTag
//-----------------------------------------------------------------------------
// Function call:     EQFBDiffTag(PTBDOCUMENT,USHORT,USHORT)
//-----------------------------------------------------------------------------
// Description:       The function evaluates whether the current character
//                    and the previous character belong to different tags
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDoc   ptr to the structure describing the
//                                         active segment
//                    USHORT        usSeg  entry number of active segment
//                    USHORT        usOffs offset of current character in
//                                         active segment
//-----------------------------------------------------------------------------
// Returncode type:   BOOL
//-----------------------------------------------------------------------------
// Returncodes:       TRUE      different tag
//                    FALSE     the same tag
//-----------------------------------------------------------------------------
// Prerequesits:      document tag table must have been loaded
//-----------------------------------------------------------------------------
// Function flow:     init return indicator to FALSE
//                    get start of start/stop table
//                    find start/stop token which contains current offset
//                    if offset is equal a start of a token
//                      previous character belongs to another tag
//                      set indicator fDifferent = TRUE
//                      force reparse
//-----------------------------------------------------------------------------

BOOL EQFBDiffTag
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   ULONG       ulSeg,                   // number of active segment
   USHORT      usOffs                   // offset in active segment
)
{
   BOOL fDifferent = FALSE;            // same tag
   PTBSEGMENT pSeg;                   // ptr to segment
   PSTARTSTOP pstCurrent;             //ptr to current start/stop entry

   pSeg = EQFBGetSegW( pDoc, ulSeg );
   pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
   if (pstCurrent && pDoc->hwndRichEdit )
   {
      UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);
   } /* endif */

   if ( pstCurrent )
   {
      while ((pstCurrent->usType) != 0 &&
                   (pstCurrent->usStart) < usOffs )          /* @KIT0965M */
      {
         pstCurrent++;
      } /* endwhile */
                                           //if usOffs is start of entry,
      if  (pstCurrent ->usStart == usOffs) //previous char belongs to other tag
      {
         fDifferent = TRUE;
         // force reparse
         UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);
      } /* endif */
   } /*endif*/
   return( fDifferent);
} /* end of EQFBDiffTag */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBDiffProtectTag
//-----------------------------------------------------------------------------
// Function call:     EQFBDiffProtectTag(PTBDOCUMENT,USHORT,USHORT)
//-----------------------------------------------------------------------------
// Description:       checks whether typing allowed if cursor right of text
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc,       ptr to active document
//                    USHORT      usSeg,      number of active segment
//                    USHORT      usOffs      offset in active segment
//-----------------------------------------------------------------------------
// Returncode type:   BOOL
//-----------------------------------------------------------------------------
// Returncodes:       TRUE   typing allowed
//                    FALSE  typing not allowed
//-----------------------------------------------------------------------------
// Function flow:     init return value to true
//                    if displaystyle is not unprotected
//                      if startstop table exists
//                        find start/stop node of current offset
//                        if type of current start/stop is protected
//                        and cursor is not equal stop of token
//                          typing not allowed (return = FALSE)
//                        endif
//                      endif
//                    endif
//-----------------------------------------------------------------------------

BOOL EQFBDiffProtectTag
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   ULONG       ulSeg,                   // number of active segment
   USHORT      usOffs                            // offset in active segment
)
{
   BOOL fOK = TRUE;            // typing allowed
   PTBSEGMENT pSeg;                   // ptr to segment
   PSTARTSTOP pstCurrent;             //ptr to current start/stop entry

   if ( pDoc->DispStyle != DISP_UNPROTECTED)
   {
      pSeg = EQFBGetSegW( pDoc, ulSeg );
      pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
      if (pstCurrent && pDoc->hwndRichEdit )
      {
         UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);
      } /* endif */
      if ( pstCurrent )
      {
         while ((pstCurrent->usType) != 0 &&
                      (pstCurrent->usStart) <= usOffs )
         {
            pstCurrent++;
         } /* endwhile */

         if ( (pstCurrent->usType != 0)
             && (usOffs != (pstCurrent-1)->usStop)
             && ((pstCurrent-1)->usType == PROTECTED_CHAR) )
         {
           fOK = FALSE;
         } /* endif */
         //P016804: add the following if-clause:
         if ( (pstCurrent->usType != 0)
		      && (usOffs == (pstCurrent-1)->usStop)
		      && ((pstCurrent-1)->usType == PROTECTED_CHAR)
		      && (*(pSeg->pDataW+usOffs) == '\n') )
		 {
	        fOK = FALSE;
         } /* endif */
      } /*endif*/
   } /* endif */
   return( fOK);
} /* end of function EQFBDiffProtectTag */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBReparse
//-----------------------------------------------------------------------------
// Function call:     EQFBReparse(PTBDOCUMENT, PTBSEGMENT,USHORT,SHORT)
//-----------------------------------------------------------------------------
// Description:       adjust all start-stop points by sDiff ,
//                    starting at the current position in the
//                    segment table
//-----------------------------------------------------------------------------
// Parameters:        PTBSEGMENT    pSeg   - ptr to segment
//                    USHORT        usOffs - current offset in segment data
//                    SHORT         sDiff  - differenz how much to shift
//                                           (sDiff = 1 : insert  char
//                                            sDiff = -1 : delete char
//                                            sDiff = 10 : insert 10 blanks
//                                                      at right end of line
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     if at begin of segment
//                      tokenize nec.
//                    else
//                      search for current position in start/stop table
//                      if end reached and last triple is protected
//                        force a recompute
//                      else
//                        adjust all following start/stops by sDiff
//                      endif
//                      if a character was deleted
//                        remove one start/stop entry if empty
//                        adjust start/stop table
//                      endif
//                    endif
//-----------------------------------------------------------------------------

VOID EQFBReparse
(
   PTBDOCUMENT pDoc,
   PTBSEGMENT  pSeg,                    // ptr to active segment
   USHORT      usOffs,                  // current offset in segment data
   SHORT       sDiff                    // differenz how much to shift
)
{
// at begin of segment status cannot be determined -> tokenize nec.
   if ((usOffs == 0) || ((pDoc->lFontLangInfo & GCP_REORDER ) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay))
   {
      UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG); // free old segment table
   }
   else
   {
       //search for current position in start/stop table
       EQFBReparseStartStop((PSTARTSTOP *) &pSeg->pusBPET, usOffs, sDiff);
   } /* endif */

   if (pSeg->pusHLType)
   {
 	   EQFBReparseHLType((PSTARTSTOP *) &pSeg->pusHLType, usOffs, sDiff, pSeg);
   }  /* endif */

   return;
} /* end of EQFBReparse */

VOID
EQFBReparseStartStop
(
	PSTARTSTOP *    ppstStart,
	USHORT   		usOffs,					// current offset in segment data
	SHORT			sDiff    			    // difference how much to shift
)
{
	PSTARTSTOP  pstStart =*ppstStart;
	PSTARTSTOP  pstCurrent = pstStart;
    if ( pstCurrent )
    {
          while ((pstCurrent->usType) != 0 &&
                       (pstCurrent->usStart) <= usOffs )
          {
             pstCurrent++;
          } /* endwhile */
          //  if last triple was protected, so force a recompute
          if  ( ( (pstCurrent-1)->usType == PROTECTED_CHAR) || (pstCurrent == pstStart) )
          {
             // force reparse, i.e. free memory and set return value
             UtlAlloc((PVOID *)&(pstStart) ,0L ,0L , NOMSG);
             *ppstStart = NULL;
          }
          else
          {
			    pstCurrent--;
                pstCurrent->usStop  = (USHORT)(pstCurrent->usStop + sDiff);  // adjust end position
                                                     // adjust all others
                pstCurrent++;
                while ((pstCurrent->usType) != 0)
                {
                   pstCurrent->usStart = (USHORT)(pstCurrent->usStart + sDiff);
                   pstCurrent->usStop  = (USHORT)(pstCurrent->usStop + sDiff);
                   pstCurrent ++;
                } /* endwhile */
          } /* endif */
    } /* endif */

    // if character deleted, check if one triple has become empty
    pstCurrent = pstStart;
    if ( pstCurrent && sDiff < 0 )
    {
	  while ( pstCurrent->usType )
	  {
		 if ( pstCurrent->usStart > pstCurrent->usStop )
		 {
			// current entry has to be removed
			while ( pstCurrent->usType )
			{
			   pstCurrent = (pstCurrent + 1);
			   pstCurrent++;
			} /* endwhile */
		 }
		 else
		 {
			pstCurrent ++;
		 } /* endif */
	  } /* endwhile */
    } /* endif */

	return;
}


VOID
EQFBReparseHLType
(
	PSTARTSTOP *    ppstStart,
	USHORT   		usOffs,							// current offset in segment data
	SHORT			sDiff,							// difference how much to shift
	PTBSEGMENT      pSeg
)
{   PSTARTSTOP  pstStart =*ppstStart;
	PSTARTSTOP  pstCurrent = pstStart;
	BOOL        fResetAll = FALSE;

    if ( pstCurrent )
    {
        while ((pstCurrent->usType) != 0 &&
                       (pstCurrent->usStart) <= usOffs )
        {
             pstCurrent++;
        } /* endwhile */

        if (pstCurrent != pstStart)
        {
            pstCurrent--;
            if (pstCurrent->usStop >= usOffs )
            {
				if ( pstCurrent->usStop + sDiff >= 0 )
				{
	   			  pstCurrent->usStop = (USHORT)(pstCurrent->usStop + sDiff);
			    }
			    else
			    {
					fResetAll = TRUE;
			    }
		    }
		    pstCurrent++;
	    }

    	  // adjust all others
    	  while (!fResetAll && ((pstCurrent->usType) != 0))
    	  {
    		 if (pstCurrent->usStart + sDiff >= 0)
    		 {
    		   pstCurrent->usStart = (USHORT)(pstCurrent->usStart + sDiff);
    		   pstCurrent->usStop  = (USHORT)(pstCurrent->usStop + sDiff);
    		   pstCurrent ++;
    		 }
    		 else
    		 {
    			 fResetAll = TRUE;
    		 }
    	  } /* endwhile */

	    if (fResetAll)
	    {
			UtlAlloc((PVOID *)&(pstStart) ,0L ,0L , NOMSG);
			*ppstStart = NULL;
			pSeg->SegFlags.Spellchecked = FALSE;
	    }
    } /* endif */

    // if character deleted, check if one triple has become empty
    pstCurrent = pstStart;
    if ( pstCurrent && sDiff < 0 )
    {
	  while ( pstCurrent->usType )
	  {
		 if ( pstCurrent->usStart > pstCurrent->usStop )
		 {
			// current entry has to be removed
			while ( pstCurrent->usType )
			{
			   pstCurrent = (pstCurrent + 1);
			   pstCurrent++;
			} /* endwhile */
		 }
		 else
		 {
			pstCurrent ++;
		 } /* endif */
	  } /* endwhile */
    } /* endif */

	return;
}
