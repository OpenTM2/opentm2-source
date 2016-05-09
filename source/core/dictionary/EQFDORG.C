//+----------------------------------------------------------------------------+
//|EQFDORG.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: This program reorganizes a DAM dictionary.                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file

#include <eqfdtag.h>              // include tag definitions
#include "eqfdde.h"
#include "eqfdicti.h"             // Private include file of dictionary handler
#include "OtmDictionaryIF.H"
#include "eqfdic00.id"
#include "eqfdimp.id"
#include "eqfrdics.h"

//function prototypes
MRESULT DicOrgCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

static VOID DorgCleanUp( PDORGIDA, HWND );
static BOOL EndLocalOrganize( PDORGIDA );
static VOID DeleteAndRepeat( PDORGIDA );

static BOOL DicPrepDictForOrganize
(
  PDORGIDA    pDorgIda,                // dictionary organize IDA
  PSZ         pSelDictName,            // name of dictionary being organized
  HWND        hwnd                     // parent window handle
);


//Dictionary organize reorganizes a dictionary; a new dictionary is created
//and the entries in the dictionary that is to be organized are copied
//into the new one entry by entry. The new dictionary is given the name of
//the old one and the old one erased.

VOID DictionaryOrganize( HWND hwnd, PSZ pSelDictName, PVOID pDictList )
{
  PDORGIDA        pDorgIda;                    // Dic organize dialog IDA
  BOOL            fOK = TRUE;                  // return value

  // create DorgIda
  fOK = (UtlAlloc( (PVOID *) &pDorgIda, 0L, (LONG) sizeof(DORGIDA), ERROR_STORAGE ));

  if ( fOK )
  {
    pDorgIda->pDictList = (PSELDICTINFO)pDictList;
    pDorgIda->pActiveDict = pDorgIda->pDictList;
  } /* endif */

  // prepare dictionary for organize
  if ( fOK )
  {
    pDorgIda->ulOemCP = GetLangOEMCP(NULL);      // get system preferences lang
    fOK = DicPrepDictForOrganize( pDorgIda, pSelDictName, hwnd );
  } /* endif */

  if ( fOK )
  {
    if( WinIsWindow( (HAB)NULL, hwnd) )
    {
      strcpy( pDorgIda->IdaHead.szObjName, pDorgIda->szOriginalProps );
      pDorgIda->IdaHead.pszObjName =  pDorgIda->IdaHead.szObjName;
      fOK = CreateProcessWindow( pDorgIda->IdaHead.pszObjName,
                                 DicOrgCallBack, pDorgIda );
    } /* endif */
  } /* endif */

  if ( !fOK )
  {
    if ( pDictList ) UtlAlloc( (PVOID *)&pDictList, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &(pDorgIda), 0L, 0L, NOMSG) ;
  } /* endif */
}

//This function creates the dictionary organize standard window and deals with all
//messages related to the standard window. Returns mResult which indicates whether
//processing was successful or not.

MRESULT DicOrgCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
   PDORGIDA        pDorgIda;          // pointer to instance area
   MRESULT         mResult = FALSE;   // return code for handler proc
   HDCB            hDictFound;        // handle of dict in which term was found
   USHORT          usSrcRC, usTrgRC;  // nlp call return codes
   USHORT          usRc;              // error utility return code
   BOOL            fOK = TRUE;        // success indicator
   BOOL            fKeep = FALSE;     // keep new copy or not
   ULONG           ulTermNum;         // number of dict term
   PSZ             pszDict;           // pointer to dict name
   PSZ             pszTerm;           // pointer to term string
   USHORT          usLDBRc;           // QLDB return code
   PVOID           phLDBTree;         // QLDB tree handle
   PCHAR_W         pucNewRecord = NULL;   // new QLDB record
   ULONG           ulNewRecLen = 0;       // new QLDB record length
   LONG            lNewSliderPos;     // new slider arm position

   switch( message )
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
       pDorgIda            =(PDORGIDA) PVOIDFROMMP2(mp2);
       pCommArea->pUserIDA = pDorgIda;

       /****************************************************************/
       /* supply all information required to create the process        */
       /* window                                                       */
       /****************************************************************/
       pCommArea->sProcessWindowID = ID_DICTORG_WINDOW;
       pCommArea->sProcessObjClass = clsDICTORG;
       pCommArea->Style            = PROCWIN_SLIDERENTRY;
       pCommArea->sSliderID        = ID_DORGSLIDER;
       pCommArea->sEntryGBID       = ID_DORG_ENTRY_GB;
       LOADSTRING( NULLHANDLE, hResMod, IDS_DORG_ENTRY_GB,
                   pCommArea->szGroupBoxTitle );
       pCommArea->sEntryID         = ID_DORG_ENTRY_TEXT;
       pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_DICTORGICON); //hiconDICTORG;
       pCommArea->fNoClose         = FALSE;
       pCommArea->swpSizePos.x     = 100;
       pCommArea->swpSizePos.y     = 100;
       pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 50;
       pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 11;
       pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
       pCommArea->asMsgsWanted[1]  = WM_EQF_SHUTDOWN;
       pCommArea->asMsgsWanted[2]  = WM_TIMER;
       pCommArea->asMsgsWanted[3]  = 0;
       pCommArea->usComplete       = 0;

       {
         PSZ       pszMsgTable[1];     // message array
         ULONG     Length;             // length indicator

         pszMsgTable[0] = pDorgIda->szLongName;

         LOADSTRING( NULLHANDLE, hResMod, IDS_DORG_TITLEBAR, pDorgIda->szString );
         DosInsMessage( pszMsgTable, 1, pDorgIda->szString, strlen ( pDorgIda->szString ),
                        pCommArea->szTitle, (sizeof ( pCommArea->szTitle ) - 1),
                        &Length );
         pCommArea->szTitle[Length] = EOS;
       }
	   }
       break;

     case WM_TIMER:
       pDorgIda =(PDORGIDA) pCommArea->pUserIDA;
       strcpy( pCommArea->szEntry, pDorgIda->szEntryTxt );
       OEMTOANSI( pCommArea->szEntry );
       break;

     case WM_EQF_SHUTDOWN:
       pDorgIda =(PDORGIDA) pCommArea->pUserIDA;
       pDorgIda->fNoOrgError = FALSE;
       mResult = (MRESULT)FALSE;
       break;

    /******************************************************************/
    /* WM_CLOSE:                                                      */
    /*                                                                */
    /* Prepare/initialize shutdown of process                         */
    /******************************************************************/
     case WM_CLOSE:
       // mp1 :  TRUE  WM_CLOSE posted by user - ask for confirm
       //        FALSE close it, because invoked from system
       // mp2:   not used any longer
       // usRc is set per default to MBID_YES, i.e. go into cleanup

       pDorgIda =(PDORGIDA) pCommArea->pUserIDA;
       usRc = MBID_YES;

       if ( !SHORT1FROMMP1( mp1) )
       {
          //cancel processing?
          usRc = UtlError ( ERROR_DORG_CANCEL, MB_YESNO, 0, NULL, EQF_QUERY );
          if ( usRc == MBID_YES )
            pDorgIda->fNoMsg = TRUE;
       } /* endif */

       if ( usRc == MBID_YES )
       {
          pDorgIda->fKill = TRUE;
          EqfRemoveObject( TWBFORCE, hwnd);
          mResult = MRFROMSHORT( FALSE );  // = continue with close
       }
       else
       {
          mResult = MRFROMSHORT( TRUE );   // = do not close right now
       } /* endif */
       break;

    /******************************************************************/
    /* WM_EQF_TERMINATE:                                              */
    /*                                                                */
    /* Allow or disable termination of process                        */
    /******************************************************************/
     case WM_EQF_TERMINATE:
       mResult = MRFROMSHORT( FALSE );           // = continue with close
       break;

    /******************************************************************/
    /* WM_DESTROY:                                                    */
    /*                                                                */
    /* Cleanup all resources used by the process                      */
    /******************************************************************/
     case WM_DESTROY :
       pDorgIda =(PDORGIDA) pCommArea->pUserIDA;
       WinStopTimer( WinQueryAnchorBlock(hwnd), hwnd, TIMER );
       DorgCleanUp( pDorgIda, hwnd );
       if ( pDorgIda->pDictList ) UtlAlloc( (PVOID *) &(pDorgIda->pDictList), 0L, 0L, NOMSG) ;
       UtlAlloc( (PVOID *) &(pDorgIda), 0L, 0L, NOMSG) ;
       pCommArea->pUserIDA = NULL;
       break;

     case WM_EQF_INITIALIZE:
       pDorgIda =(PDORGIDA) pCommArea->pUserIDA;
       WinStartTimer( (HAB)UtlQueryULong( QL_HAB), hwnd, TIMER, TIMEOUT);
       DorgInit( hwnd, pDorgIda );
       UtlDispatch();
       break;

     case WM_EQF_PROCESSTASK:
       //process one term a message starting at end of file and working
       //up to top
       pDorgIda =(PDORGIDA) pCommArea->pUserIDA;
       if ( !pDorgIda->fKill )     //processing is not to be stopped
       {
         BOOL fSkip = FALSE;           // TRUE = skip corrupted entry

         if (pDorgIda->usVersion >= BTREE_VERSION2 )
         {
            usSrcRC = AsdNxtTermW( pDorgIda->hDict1,
                                   pDorgIda->hUser,
                                   pDorgIda->ucTermBuf,
                                   &ulTermNum,
                                   &pDorgIda->ulDataLen,  // in # of w's
                                   &hDictFound );
         }
         else
         {        // dict is not in Unicode yet!
            usSrcRC = AsdNxtTerm( pDorgIda->hDict1,
                               pDorgIda->hUser,
                               (PUCHAR)pDorgIda->ucASCIITermBuf,
                               &ulTermNum,
                               &pDorgIda->ulASCIIDataLen,
                               &hDictFound );
         }

         if ( usSrcRC == LX_RC_OK_ASD )
         {
            //retrieve one term from either Unicode or old dict
             if (pDorgIda->usVersion >= BTREE_VERSION2 )
             {
                usSrcRC = AsdRetEntryW( pDorgIda->hDict1,
                                   pDorgIda->hUser,
                                   pDorgIda->ucTermBuf,
                                   &ulTermNum,
                                   pDorgIda->pucDictData,
                                   &pDorgIda->ulDataLen, // of w's
                                   &hDictFound );
             }
             else
             {
                usSrcRC = AsdRetEntry( pDorgIda->hDict1,
                                       pDorgIda->hUser,
                                       (PUCHAR)pDorgIda->ucASCIITermBuf,
                                       &ulTermNum,
                                       (PUCHAR)pDorgIda->pucASCIIDictData,
                                       &pDorgIda->ulASCIIDataLen,
                                       &hDictFound );

               // now convert whole pDorgIda->pucASCIIDictData to Unicode!
               ASCII2UnicodeBuf( pDorgIda->pucASCIIDictData,
                                 pDorgIda->pucDictData,
                                 (USHORT) pDorgIda->ulASCIIDataLen, pDorgIda->ulOemCP );
               ASCII2Unicode( pDorgIda->ucASCIITermBuf, pDorgIda->ucTermBuf, pDorgIda->ulOemCP );
               // @@ DEBUG: check the following statement!!
               pDorgIda->ulDataLen = pDorgIda->ulASCIIDataLen;
             } /* endif */

             // skip entry if it could not be retreived
             if (usSrcRC != LX_RC_OK_ASD)
             {
               fSkip = TRUE;
               pDorgIda->ulSkipped++;
             } /* endif */

             //convert record to a QLDB tree and vice-versa
             if ( !fSkip && (usSrcRC == LX_RC_OK_ASD) )
             {
               phLDBTree = NULL;
               pucNewRecord = NULL;
               usLDBRc = QLDBRecordToTree( pDorgIda->ausNoOfFields,
                                           pDorgIda->pucDictData,
                                           pDorgIda->ulDataLen,
                                           &phLDBTree );

               // try to access first node (will fail if record is corrupted )
               if ( usLDBRc == QLDB_NO_ERROR )
               {
                 USHORT usLevel;
                 usLDBRc = QLDBCurrNode( phLDBTree, pDorgIda->apszField, &usLevel );
               } /* endif */

               if ( usLDBRc == QLDB_NO_ERROR )
               {
                  usLDBRc = QLDBTreeToRecord( phLDBTree,
                                              &pucNewRecord,
                                              &ulNewRecLen );
               } /* endif */

               if ( phLDBTree )
               {
                  QLDBDestroyTree( &phLDBTree );
               } /* endif */

               if ( usLDBRc != QLDB_NO_ERROR )
               {
                  //entry seems to be corrupted
                  usSrcRC = LX_UNEXPECTED_ASD;
                  fSkip = TRUE;
                  pDorgIda->ulSkipped++;
               } /* endif */
            } /* endif */

            //insert term in temporary dictionary Dict2
            if ( fSkip )
            {
              // ignore corrupted entry
            }
            else if ( usSrcRC == LX_RC_OK_ASD ) 
            {
               //update std window with entry
               Unicode2ASCII( pDorgIda->ucTermBuf, pDorgIda->szEntryTxt, pDorgIda->ulOemCP );
               pDorgIda->szEntryTxt[STRINGLEN-1] =  EOS;

               usTrgRC = AsdInsEntry( pDorgIda->hUser,
                                      pDorgIda->hDict2,
                                      pDorgIda->ucTermBuf,
                                      pucNewRecord,
                                      ulNewRecLen,
                                      &ulTermNum );
               UtlAlloc( (PVOID *) &pucNewRecord, 0L, 0L, NOMSG );

               if ( usTrgRC == LX_RC_OK_ASD )
               {
                pDorgIda->ulCurrentEntry++;       //update counter
               } /* endif */

               if ( usTrgRC != LX_RC_OK_ASD )
               {
                 if ( usTrgRC != LX_WRD_EXISTS_ASD )
                 {
                   //duplicate headwords in original dict and as headwords
                   //have to be unique this would lead to an error, so skip
                   //entry

                   //dam error in AsdInsEntry
                   pszTerm = QDAMErrorString( usTrgRC, pDorgIda->pszMsgError );
                   UtlError ( usTrgRC, MB_CANCEL, 1, &pszTerm, QDAM_ERROR );
                   fOK = FALSE;
                  } /* endif */
               } /* endif */
            }
            else if ( usSrcRC == LX_RENUM_RQD_ASD )
            {
               pszDict = pDorgIda->szLongName;

               usRc = UtlError ( ERROR_DICT_CORRUPTED, MB_YESNO,
                                 1, &pszDict, EQF_QUERY );
               fOK = FALSE;

               if ( usRc == MBID_YES )
               {
                  //user wants to keep the partly organized dict in the case of
                  //dict corrupted message
                  fKeep = TRUE;
               } /* endif */
            }
            else
            {
               if ( (usSrcRC == LX_WRD_NT_FND_ASD ) ||
                    (usSrcRC == LX_WRD_EXISTS_ASD ) )
               {
                 Unicode2ASCII( pDorgIda->ucTermBuf, pDorgIda->ucAnsiTerm, pDorgIda->ulOemCP );
                 OEMTOANSI( pDorgIda->ucAnsiTerm );
                 pszTerm = pDorgIda->ucAnsiTerm;
                 usRc = UtlError( ERROR_SEARCH_ENTRY, MB_YESNO, 1, &pszTerm,
                                  EQF_QUERY );
                 if ( usRc != MBID_YES )
                  fOK = FALSE;
               }
               else
               {
                 //dam error in AsdInsEntry
                 pszTerm = QDAMErrorString( usSrcRC, pDorgIda->pszMsgError );
                 UtlError ( usSrcRC, MB_CANCEL, 1, &pszTerm, QDAM_ERROR );
                 fOK = FALSE;
               } /* endif */
            } /* endif */

            if ( fOK )
            {
               //update slider
               lNewSliderPos = pDorgIda->ulCurrentEntry * 100 /
                               pDorgIda->ulMaxEntries;

               if ( pDorgIda->lSliderPos != lNewSliderPos )
               {
                 pDorgIda->lSliderPos = lNewSliderPos;
                 WinSendMsg( hwnd, WM_EQF_UPDATESLIDER,
                             MP1FROMSHORT((SHORT)lNewSliderPos), NULL );
               } /* endif */

               UtlDispatch();

               WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
            } /* endif */
         }
         else
         {
            if ( usSrcRC == LX_EOF_ASD )
            {
                pDorgIda->fNoOrgError = TRUE;

                // continue with next dictionary if any
                if ( pDorgIda->pDictList )
                {
                  pDorgIda->pActiveDict++;
                  if ( pDorgIda->pActiveDict->szName[0] == EOS ) pDorgIda->pActiveDict = NULL;
                } /* endif */

                // terminate current dictionary organize but suppress termination message
                if ( pDorgIda->pActiveDict )
                {
                  BOOL fOldMsgFlag = pDorgIda->fNoMsg;
                  pDorgIda->fNoMsg = TRUE;
                  DorgCleanUp( pDorgIda, hwnd );
                  pDorgIda->fNoMsg = fOldMsgFlag;
                } /* endif */

                // prepare dictionary for organize
                if ( pDorgIda->pActiveDict )
                {
                  BOOL fOK = TRUE;

                  ANSITOOEM( pDorgIda->pActiveDict->szName );
                  fOK = DicPrepDictForOrganize( pDorgIda, pDorgIda->pActiveDict->szName, hwnd );
                  OEMTOANSI( pDorgIda->pActiveDict->szName );

                  // shutdown if preparation failed
                  if ( !fOK )
                  {
                    pDorgIda->fNoOrgError = FALSE;
                    pDorgIda->pActiveDict = NULL;
                   } /* endif */
                } /* endif */

                // start next organize or terminate organize process
                if ( pDorgIda->pActiveDict )
                {
                  WinStopTimer( WinQueryAnchorBlock(hwnd), hwnd, TIMER );
                  WinPostMsg( hwnd, WM_EQF_INITIALIZE, NULL, NULL );
                }
                else
                {
                   //finish off
                   WinPostMsg( hwnd, WM_CLOSE, MP1FROMSHORT( TRUE ), NULL );
                } /* endif */
            }
            else if ( usSrcRC == LX_RENUM_RQD_ASD )
            {
               pszDict = pDorgIda->szLongName;
               usRc = UtlError ( ERROR_DICT_CORRUPTED, MB_YESNO,
                                 1, &pszDict, EQF_QUERY );
               fOK = FALSE;

               if ( usRc == MBID_YES )
               {
                  //user wants to keep the partly organized dict in the case of
                  //dict corrupted message
                  fKeep = TRUE;
               } /* endif */
            }
            else
            {
               pszDict =  QDAMErrorString( usSrcRC, pDorgIda->pszMsgError );
               UtlError ( usSrcRC, MB_CANCEL, 1, &pszDict, QDAM_ERROR );
               fOK = FALSE;
            } /* endif */
         } /* endif */

         if ( !fOK )
         {
            if ( fKeep )  //set when dict corrupted message issued
            {
               //cleanup with new dict copy
               pDorgIda->fNoMsg = TRUE;
               pDorgIda->fNoOrgError = TRUE;
               WinPostMsg( hwnd, WM_CLOSE, MP1FROMSHORT( TRUE ), NULL );
            }
            else
            {
               //cleanup with no new dict copy
               pDorgIda->fNoMsg = TRUE;
               pDorgIda->fNoOrgError = FALSE;
               WinPostMsg( hwnd, WM_CLOSE, MP1FROMSHORT( TRUE ), NULL );
            } /* endif */
         } /* endif */
       } /* endif */
       break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
       UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
       UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
       break;

    case WM_EQF_TOOLBAR_ENABLED:
      switch ( mp1 )
      {
        /**************************************************************/
        /* check for items to be enabled ..                           */
        /**************************************************************/
        case PID_VIEW_MI_DETAILS:
          mResult = MRFROMSHORT(TRUE);
          break;
        default:
          break;
      } /* endswitch */
      break;

   } /* endswitch */
   return( mResult );
}

//releases memory - if fOK is TRUE then the original dict is deleted
//and the new copy is used and if false the original is used and copy deleted
static
VOID DorgCleanUp( PDORGIDA pDorgIda, HWND hwnd )
{
   PSZ      pDictName;  //pointer to dict name string
   BOOL     fEnded;     //organize close and rename success indicator

   fEnded = EndLocalOrganize( pDorgIda );

   if ( !fEnded )
   {
     if (!pDorgIda->fNoMsg )
     {
       //issue message that organize went bad
       pDictName = pDorgIda->szLongName;
       UtlError( ERROR_DORG_FAILED, MB_CANCEL, 1, &pDictName, EQF_ERROR );
     } /* endif */
   }
   else
   {
     //set slider to 100%
     WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(100), NULL );
     pDorgIda->ulTotalTerms += pDorgIda->ulCurrentEntry - 1;
     if ( !pDorgIda->fNoMsg )
     {
       if ( pDorgIda->pDictList )
       {
         PSZ pszDictNames = NULL;    // buffer for dictionary names

         // show message for multiple dicts
         CHAR szNumOfTerms[20];          // buffer for number of terms organized
         PSZ apszMsgParms[2];            // ptr array for message pars

         // setup list of dict names for the display in the message box
         {
            // compute size of required buffer
            PSELDICTINFO pNext = pDorgIda->pDictList;
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
              PSELDICTINFO pNext = pDorgIda->pDictList;
              PSZ pszCurPos = pszDictNames;
              while ( pNext->szName[0] != EOS )
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
              } /* endwhile */
            } /* endif */
         }

         // prepare message parameters
         sprintf( szNumOfTerms, "%lu", pDorgIda->ulTotalTerms );
         apszMsgParms[0] = pszDictNames;
         apszMsgParms[1] = szNumOfTerms;           // number of terms
         UtlError ( ERROR_DORG_COMPLETE_MUL, MB_OK, 2, apszMsgParms, EQF_INFO );
         if ( pszDictNames ) UtlAlloc( (PVOID *)&pszDictNames, 0L, 0L, NOMSG );
       }
       else
       {
         CHAR szNumOfTerms[20];          // buffer for number of terms organized
         CHAR szSkippedTerms[20];        // buffer for number of skipped terms
         PSZ apszMsgParms[3];            // ptr array for message pars

         // prepare message parameters
         sprintf( szNumOfTerms, "%lu", pDorgIda->ulCurrentEntry - 1);
         sprintf( szSkippedTerms, "%lu", pDorgIda->ulSkipped );
         apszMsgParms[0] = pDorgIda->szLongName;
         apszMsgParms[1] = szNumOfTerms;           // number of terms
         apszMsgParms[2] = szSkippedTerms;         // number of skipped terms
         UtlError ( ERROR_DORG_COMPLETE2, MB_OK, 3, apszMsgParms, EQF_INFO );
       } /* endif */
     } /* endif */
   } /* endif */

   //remove symbol so dict free for further use
   if ( pDorgIda->fFree )
   {
      PROPNAME ( pDorgIda->szString, pDorgIda->szDictName );
      REMOVESYMBOL( pDorgIda->szString );
   } /* endif */

   //release memory
   if ( pDorgIda->pucDictData )
      UtlAlloc( (PVOID *) &pDorgIda->pucDictData, 0L, 0L, NOMSG );
   if ( pDorgIda->pucASCIIDictData )
      UtlAlloc( (PVOID *) &pDorgIda->pucASCIIDictData, 0L, 0L, NOMSG) ;
}


BOOL DorgInit( HWND hwnd, PDORGIDA pDorgIda )
{
  BOOL fOK = TRUE;                 //success indicator
  USHORT usErrDict;                //number of dictionary in error
  USHORT usSrcRC = LX_RC_OK_ASD,   //return code for dict to compress
         usTrgRC = LX_RC_OK_ASD;   //return code of temp dict
  PPROPDICTIONARY pPropOld = NULL; //ptr to dictioanry properties
  PPROPDICTIONARY pPropNew = NULL; //ptr to dictioanry properties
  PSZ             pszDict;         //pointer for dictionary name
  SHORT           sRC;             //return code
  SHORT           sItem;           //listbox item
  USHORT          usRc;            //return code
  CHAR            szDrive[2];          //drive letter
  PSZ             pDot;                //ptr to string

  pDorgIda->hDict1 = NULL;        // handle initialization
  pDorgIda->hDict2 = NULL;        // handle initialization

  //check if symbol exists (if dict in use by another process)
  PROPNAME ( pDorgIda->szString, pDorgIda->szDictName );
  sRC = QUERYSYMBOL( pDorgIda->szString );
  if ( sRC != -1 )
  {
    pszDict = pDorgIda->szLongName;

    UtlError( ERROR_DICT_LOCKED, MB_CANCEL,
              1, &pszDict, EQF_ERROR );
    fOK = FALSE;
    pDorgIda->fNoMsg = TRUE;
    pDorgIda->fFree = FALSE;
  }
  else
  {
    SETSYMBOL( pDorgIda->szString );
    pDorgIda->fFree = TRUE;
  } /* endif */

  if ( fOK )
  {
    //init DAM/TOLSTOY
    usSrcRC = AsdBegin( 2, &pDorgIda->hUser );

    if ( usSrcRC != LX_RC_OK_ASD ) //not enough memory
    {
      //dam error
      pszDict =  QDAMErrorString( usSrcRC, pDorgIda->pszMsgError );
      UtlError ( usSrcRC, MB_CANCEL, 1, &pszDict, QDAM_ERROR );
      fOK = FALSE;
    } /* endif */

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *) &pPropNew, 0l, (LONG) sizeof(PROPDICTIONARY),
                      ERROR_STORAGE );

      if ( fOK )
      {
        pszDict = pDorgIda->szOriginalProps;
        usSrcRC = AsdOpen( pDorgIda->hUser,   //in - user handle
                           ASD_LOCKED | ASD_NOINDEX, //in - open flags
                           1,                 //in - number of dictionaries
                           &pszDict,          //in - dictionary name(s)
                           &pDorgIda->hDict1, //out - dictionary handle
                           &usErrDict );      //out - num of dict in error

        if ( ! (( usSrcRC == LX_RC_OK_ASD ) ||
             ( usSrcRC == LX_RENUM_RQD_ASD ) ))
        {
          //dam error
          pszDict =  QDAMErrorString( usSrcRC, pDorgIda->pszMsgError );
          UtlError ( usSrcRC, MB_CANCEL, 1, &pszDict, QDAM_ERROR );
          fOK = FALSE;
          pDorgIda->fNoMsg = TRUE;
          if ( ( usSrcRC == BTREE_FILE_NOTFOUND ) ||
               ( usSrcRC == BTREE_OPEN_FAILED ) )
          {
            //if remote leave properties to be deleted if user wants to
            //and if a local dictionary grey out entry
            if ( !pDorgIda->fRemote )
            {
              //grey out entry in dict list box
              sItem = QUERYSELECTION( pDorgIda->DictLBhwnd, PID_DICTIONARY_LB );
              CLBSETITEMSTATE( hwnd, PID_DICTIONARY_LB, sItem, FALSE );
            } /* endif */
          } /* endif */
        }
        else
        {
          usSrcRC = LX_RC_OK_ASD;

            //get original dictionary properties
            AsdRetPropPtr( pDorgIda->hUser, pDorgIda->hDict1, &pPropOld );

            //copy old properties and change property head
            memcpy( pPropNew, pPropOld, sizeof(PROPDICTIONARY) );

            //update prophead.szname in properties
            pDot = strchr( pPropNew->PropHead.szName, DOT );
            if ( pDot )
            {
              strcpy( pDot, EXT_TMP_DICTPROP );
            }
            else
            {
              fOK = FALSE;
            } /* endif */

            //update asd dict path in properties
            pDot = strchr( pPropNew->szDictPath, DOT );
            if ( pDot )
            {
              strcpy( pDot, EXT_TMP_DIC );
            }
            else
            {
              fOK = FALSE;
            } /* endif */

            //update index file path in properties
            pDot = strchr( pPropNew->szIndexPath, DOT );
            if ( pDot )
            {
              strcpy( pDot, EXT_TMP_DICTINDEX );
            }
            else
            {
              fOK = FALSE;
            } /* endif */

            if ( fOK )
            {
              //build temporary name for new local dictionary property file
              Utlstrccpy( pDorgIda->szOrganizedProps, pDorgIda->szOriginalProps, DOT );
              strcat( pDorgIda->szOrganizedProps, EXT_TMP_DICTPROP );

              //delete possible old files
              DeleteAndRepeat( pDorgIda );

              // write properties to file (required for Asd functions)
              usRc = UtlWriteFile( pDorgIda->szOrganizedProps, sizeof(PROPDICTIONARY),
                                   pPropNew, FALSE );

              if ( usRc )
              {
                /* an error occured when writing the temporary property file */
                /* delete the file (local) and try again.                    */
                if ( pDorgIda->fRemote )
                {
                  /* delete only local property file */
                  UtlDelete ( pDorgIda->szOrganizedProps, 0L, FALSE);
                }
                else
                {
                  /* delete all possible temporary files */
                  DeleteAndRepeat( pDorgIda );
                } /* endif */

                usRc = UtlWriteFile( pDorgIda->szOrganizedProps,
                                     sizeof(PROPDICTIONARY), pPropNew, FALSE );
              } /* endif */

              if ( usRc )
              {
                szDrive[0] = pDorgIda->szOrganizedProps[0];
                szDrive[1] = EOS;
                pszDict = szDrive;
                UtlError( ERROR_PROP_WRITE_MSG, MB_CANCEL, 1,
                          &pszDict, EQF_ERROR);
                fOK = FALSE;
                pDorgIda->fNoMsg = TRUE;
              } /* endif */
            }
            else
            {
              //property path info incorrect
              //issue message in cleanup that organize of dict wasn't
              //possible - ERROR_DORG_FAILED message
              UtlError ( ERROR_INTERNAL, MB_CANCEL, 0, NULL, EQF_ERROR );
            } /* endif */

          if ( fOK )
          {
            //ensure that szOrganizedProps is the local temp property path
            Utlstrccpy( pDorgIda->szOrganizedProps, pDorgIda->szOriginalProps, DOT );
            strcat( pDorgIda->szOrganizedProps, EXT_TMP_DICTPROP );

            // create new dictionary
            usTrgRC = AsdBuild( pDorgIda->hUser,     //in - user handle
                                FALSE,               //in - guarded mode?
                                &pDorgIda->hDict2,   //out - dict handle
                                pDorgIda->szOrganizedProps ); //in - dict name

            if ( usTrgRC == LX_RC_OK_ASD )
            {
              //fill number-of-fields-per-level table
              QLDBFillFieldTables( pPropNew, pDorgIda->ausNoOfFields, NULL );
            }
            else
            {
              /********************************************************/
              /* display error message ....                           */
              /********************************************************/
              pszDict =  QDAMErrorString( usTrgRC, pDorgIda->pszMsgError );
              UtlError ( usTrgRC, MB_CANCEL, 1, &pszDict, QDAM_ERROR );
              pDorgIda->hDict2 = NULL;        // handle initialization
              pDorgIda->fNoMsg = TRUE;
              fOK = FALSE;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */

      //free all allocated memory
      if ( pPropNew != NULL )
        UtlAlloc( (PVOID *) &pPropNew, 0L, 0L, NOMSG );

      if ( fOK )
        fOK = UtlAlloc( (PVOID *) &pDorgIda->pucDictData, 0l,
                        (LONG) MAX_ALLOC, ERROR_STORAGE );

      fOK = (usSrcRC == LX_RC_OK_ASD) && (usTrgRC == LX_RC_OK_ASD) && fOK;
    } /* endif */
  } /* endif */

  //determine number of entries in dict
  if ( fOK )
  {
    usSrcRC  = AsdDictVersion( pDorgIda->hDict1, &pDorgIda->usVersion );
    if (pDorgIda->usVersion < BTREE_VERSION2 )
    {
      fOK = UtlAlloc( (PVOID *) &pDorgIda->pucASCIIDictData, 0l,
                        (LONG) MAX_ALLOC, ERROR_STORAGE );
    }

     usSrcRC = AsdNumEntries( pDorgIda->hDict1,           //in, dict handle
                              &pDorgIda->ulMaxEntries );  //out, num of words
     pDorgIda->ulMaxEntries = max( pDorgIda->ulMaxEntries, 1);

     if ( usSrcRC != LX_RC_OK_ASD )
     {
        //dam error
        pszDict =  QDAMErrorString( usSrcRC, pDorgIda->pszMsgError );
        UtlError ( usSrcRC, MB_CANCEL, 1, &pszDict, QDAM_ERROR );
        fOK = FALSE;
     }
  } /* endif */

  //initialize variable
  pDorgIda->ulCurrentEntry = 1;

  if ( fOK )
  {
     WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
  }
  else
  {
     //cleanup with no new dict copy
     pDorgIda->fNoOrgError = FALSE;
     WinPostMsg( hwnd, WM_CLOSE, MP1FROMSHORT( TRUE ), NULL );
  } /* endif */

  return( fOK );
}

static
VOID DeleteAndRepeat( PDORGIDA pDorgIda )
{
    //delete all temp/copy files in case some were left in directory
    //during forced shutdown (only local)
    //copy asd
    Utlstrccpy( pDorgIda->szString, pDorgIda->szOriginalAsd, DOT );
    strcat( pDorgIda->szString, EXT_TMP_DIC_COPY );
    UtlDelete( pDorgIda->szString, 0L, FALSE );
    //copy asi
    Utlstrccpy( pDorgIda->szString, pDorgIda->szOriginalAsd, DOT );
    strcat( pDorgIda->szString, EXT_TMP_DICTINDEX_COPY );
    UtlDelete( pDorgIda->szString, 0L, FALSE );
    //temp asd
    Utlstrccpy( pDorgIda->szString, pDorgIda->szOriginalAsd, DOT );
    strcat( pDorgIda->szString, EXT_TMP_DIC );
    UtlDelete( pDorgIda->szString, 0L, FALSE );
    //temp asi
    Utlstrccpy( pDorgIda->szString, pDorgIda->szOriginalAsd, DOT );
    strcat( pDorgIda->szString, EXT_TMP_DICTINDEX );
    UtlDelete( pDorgIda->szString, 0L, FALSE );
    //temp props
    UtlDelete( pDorgIda->szOrganizedProps, 0L, FALSE );
} /* endDeleteAndRepeat */


static
BOOL EndLocalOrganize( PDORGIDA pDorgIda )
{
  USHORT  usRc = 0;                    //return code
  PSZ     pszDict;                     //ptr to string
  USHORT  usRcDict2 = 0;               //return code

  //close dictionaries
  if ( pDorgIda->hDict2 )  //new copy after organize
     usRcDict2 = AsdClose(pDorgIda->hUser, pDorgIda->hDict2 );

  //close original dictionary
  if ( pDorgIda->hDict1 )  //new copy after organize
    usRc = AsdClose( pDorgIda->hUser, pDorgIda->hDict1 );

  //end services
  AsdEnd( pDorgIda->hUser );

  //delete dict1 if new copy of dict was successfully closed
  if ( pDorgIda->hDict1 && usRc != LX_RC_OK_ASD )
  {
    //dam error closing original dict
    pszDict =  QDAMErrorString( usRc, pDorgIda->pszMsgError );
    UtlError ( usRc, MB_CANCEL, 1, &pszDict, QDAM_ERROR );
    pDorgIda->fNoOrgError = FALSE;                          //clupa
  }
  else
  {
    //organized dict successfully closed and no bad errors during
    //organize process
    if ( usRcDict2 == LX_RC_OK_ASD && pDorgIda->fNoOrgError )
    {
      //build temporary copy of original dictionary asd
      Utlstrccpy( pDorgIda->szCopyAsd, pDorgIda->szOriginalAsd, DOT );
      strcat( pDorgIda->szCopyAsd, EXT_TMP_DIC_COPY );

      //rename original and give temp copy
      //first asd
      usRc = UtlMove( pDorgIda->szOriginalAsd, pDorgIda->szCopyAsd, 0L, FALSE );
      if ( !usRc )
      {
        //then asi
        Utlstrccpy( pDorgIda->szCopyAsi, pDorgIda->szOriginalAsi, DOT );
        strcat( pDorgIda->szCopyAsi, EXT_TMP_DICTINDEX_COPY );

        UtlMove( pDorgIda->szOriginalAsi, pDorgIda->szCopyAsi, 0L, FALSE );
      } /* endif */

      if ( !usRc )
      {
        //rename newly organized and give original name
        Utlstrccpy( pDorgIda->szOrganizedAsd, pDorgIda->szOriginalAsd, DOT );
        strcat( pDorgIda->szOrganizedAsd, EXT_TMP_DIC );

        //rename original and give temp copy
        //first asd
        usRc = UtlMove( pDorgIda->szOrganizedAsd, pDorgIda->szOriginalAsd, 0L, FALSE );
        if ( !usRc )
        {
          //then asi
          Utlstrccpy( pDorgIda->szOrganizedAsi, pDorgIda->szOriginalAsi, DOT );
          strcat( pDorgIda->szOrganizedAsi, EXT_TMP_DICTINDEX );

          usRc = UtlMove( pDorgIda->szOrganizedAsi, pDorgIda->szOriginalAsi, 0L, FALSE );
        } /* endif */

        if ( !usRc )
        {
          //delete temp asd and asi copy, and temp prop file as all went well
          UtlDelete( pDorgIda->szCopyAsd, 0L, FALSE );
          UtlDelete( pDorgIda->szCopyAsi, 0L, FALSE );
          UtlDelete( pDorgIda->szOrganizedProps, 0L, FALSE );
        }
        else
        {
          //renames failed so rename original to temp copy
          //first asd
          usRc = UtlMove( pDorgIda->szCopyAsd, pDorgIda->szOriginalAsd, 0L, FALSE );
          if ( !usRc )
          {
            //then asi
            usRc = UtlMove( pDorgIda->szCopyAsi, pDorgIda->szOriginalAsi, 0L, FALSE );
          } /* endif */

          //delete newly organized file - szOrganizedProps is prop path
          AsdDelete( pDorgIda->szOrganizedProps );
        } /* endif */
      }
      else
      {
        //rename of original to temp copy failed
        AsdDelete( pDorgIda->szOrganizedProps );
        pDorgIda->fNoOrgError = FALSE;                   //clupa
      } /* endif */
    }
    else
    {
      //asdclose of the newly organized dict failed or something went wrong
      //during organize which lead to a interruption and incomplete new
      //dictionary copy. The original is left as is.
      AsdDelete( pDorgIda->szOrganizedProps );
      pDorgIda->fNoOrgError = FALSE;                     //clupa
    } /* endif */
  } /* endif */

  if ( usRc )
    pDorgIda->fNoOrgError = FALSE;                       //clupa

  return( pDorgIda->fNoOrgError );                       //clupa
} /* endEndLocalOrganize */


// prepare a dictionary for the organize process
static BOOL DicPrepDictForOrganize
(
  PDORGIDA    pDorgIda,                // dictionary organize IDA
  PSZ         pSelDictName,            // name of dictionary being organized
  HWND        hwnd                     // parent window handle
)
{
  BOOL        fOK = TRUE;              // function return code
  USHORT          usRc;                        // return code
  USHORT          usError;                     // return code
  EQFINFO         ErrorInfo;                   // error code for dicts
  PPROPDICTIONARY pDictProp= NULL;             // ptr to dict props
  HPROP           hDictProp;                   // dict handle
  LONG            lBytesShort;                 // ammount of missing space
  PSZ             pszDrive;                    // ptr to drive string
  PSZ             pMsgError[2];                // ptr to error array
  SHORT           sIndexItem;                  // dict list box item
  PSZ             pszErrParm = NULL;

  // reset numbe rof skipped entries
  pDorgIda->ulSkipped = 0;

  // get short name of dictionary
  if ( fOK )
  {
    BOOL fIsNew = FALSE;         // is-new flag
    ObjLongToShortName( pSelDictName, pDorgIda->szDictName,
                        DICT_OBJECT, &fIsNew );
    strcpy( pDorgIda->szLongName, pSelDictName );
    OEMTOANSI( pDorgIda->szLongName );
    pszErrParm = pDorgIda->szLongName;
  } /* endif */

  // Check if dictionary is currently in use
  if ( fOK )
  {
    SHORT sRC;

    PROPNAME ( pDorgIda->szString, pDorgIda->szDictName );
    sRC = QUERYSYMBOL( pDorgIda->szString );
    if ( sRC != -1 )
    {
      UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszErrParm, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //remember selected dictionary in Ida
    if ( pSelDictName != NULL )
    {
      //create full dict property path name
      UtlMakeEQFPath( pDorgIda->szString, NULC, PROPERTY_PATH,(PSZ) NULP );
      sprintf( pDorgIda->szOriginalProps, "%s\\%s%s",
               pDorgIda->szString, pDorgIda->szDictName, EXT_OF_DICTPROP );


      //open dictionary properties
      PROPNAME ( pDorgIda->szString, pDorgIda->szDictName );
      hDictProp = OpenProperties( pDorgIda->szString, NULL,
                                 PROP_ACCESS_WRITE,
                                 &ErrorInfo);

      if ( hDictProp )
      {
         pDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hDictProp );

         if ( pDictProp->fCopyRight && fOK &&
             (pDictProp->usVersion >= BTREE_VERSION2))
         {
            //dict is copyrighted
            UtlError( ERROR_SYSTDICT_COPYRIGHTED, MB_CANCEL,
                      1, &pszErrParm, EQF_ERROR );
            fOK = FALSE;
         }
         else
         {
            //Is the dictionary protected?
            if ( pDictProp->fProtected && fOK )
            {
				HMODULE hResMod;
				hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
               //call up password checking dialog
               //fOK true allows organization
               DIALOGBOX( hwnd, DICT1PASSWORDDLG, hResMod,
                          ID_DICTPASSWORD_DLG, pDictProp, fOK ) ;
            } /* endif */
         } /* endif */
      }
      else
      {
         //dict props cannot be opened
         UtlError( ERROR_OPENING_PROPS, MB_CANCEL, 1, &pszErrParm,
                   EQF_ERROR );
         fOK = FALSE;

         sIndexItem = QUERYSELECTION( pDorgIda->DictLBhwnd, PID_DICTIONARY_LB );
         if ( !pDorgIda->fRemote )
         {
           //grey out dictionary as props cannot be accessed
           CLBSETITEMSTATE( pDorgIda->DictLBhwnd, PID_DICTIONARY_LB,
                            sIndexItem, FALSE );
         } /* endif */
      } /* endif */

      if ( fOK )
      {
        //remember asd and asi paths
        strcpy( pDorgIda->szOriginalAsd, pDictProp->szDictPath );
        strcpy( pDorgIda->szOriginalAsi, pDictProp->szIndexPath );

        if ( pDictProp->usLocation == LOC_SHARED )
        {
          usRc = UtlGetLANUserID( pDorgIda->szUserid,
                                  &usError, FALSE );
          // fOK = (usRc == NO_ERROR);
        } /* endif */

        if ( fOK )
        {
          //fill msg parameter array for QDAM error messages
          Utlstrccpy( pDorgIda->szFilePath,
             UtlGetFnameFromPath( pDictProp->szDictPath ), DOT );
          pDorgIda->pszMsgError[0] = pszErrParm;
          //current entry

          pDorgIda->pszMsgError[1] = EMPTY_STRING;
          //dictionary drive
          pDorgIda->szPath[0] = pDictProp->szDictPath[0];
          pDorgIda->szPath[1] = EOS;
          pDorgIda->pszMsgError[2] = pDorgIda->szPath;
          //server name
          pDorgIda->pszMsgError[3] = pDorgIda->szServer;
          //dictionary source lang
          strcpy( pDorgIda->szSourceLang, pDictProp->szSourceLang );
          pDorgIda->pszMsgError[4] = pDorgIda->szSourceLang;

          pszDrive = pDorgIda->szPath;
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
            usRc = DictRcHandling2( usError, pDictProp->szDictPath, NULLHANDLE,
                                    pDorgIda->szServer, pszErrParm );
            fOK = FALSE;                 //error occurred with server code
            if ( (usRc == BTREE_OPEN_FAILED) ||
                 (usRc == BTREE_ACCESS_ERROR) ||
                 (usRc == ERROR_PATH_NOT_FOUND) ||
                 (usRc == ERROR_NO_MORE_FILES) )
            {
              //return sitem in dict list window
              sIndexItem = QUERYSELECTION( hwnd, PID_DICTIONARY_LB );
              if ( sIndexItem != LIT_NONE )
              {
                if ( !pDorgIda->fRemote )
                {
                  //send msg to grey out dict in dict list window
                  CLBSETITEMSTATE( hwnd, PID_DICTIONARY_LB, sIndexItem, FALSE );
                } /* endif */
              } /* endif */
            } /* endif */
          }
          else
          {
            if ( usRc && lBytesShort )
            {
              pMsgError[0] = pszErrParm;
              pMsgError[1] = pDorgIda->szPath;
              usRc = UtlError( NO_SPACE_FOR_ORGANIZE, MB_YESNO |
                               MB_DEFBUTTON2, 2, pMsgError, EQF_INFO );
              if ( usRc != MBID_YES )
              {
                fOK = FALSE;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */

      if ( hDictProp )
       CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function DicprepDictForOrganize */
