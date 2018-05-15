/*! \brief EQFIANA1.C - Analysis(1) dialog, to invoke analysis
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
	Description  : Dialog to get input for analysis and start analysis
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_FOLDER           // folder related stuff
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFDDE.H"               // Batch mode definitions
#include "eqftai.h"               // Private include file for Text Analysis
#include <eqfiana1.id>            // analysis dialog ids
//  #include "commctrl.h"          // common controls
  #include "eqfstart.h"
#include "OtmDictionaryIF.H"

// define this to force the MT output in nFluent XML format (this same define must be set in EQFTSEGM.C and EQFFTAPH2.C as well) 
#define NFLUENT_MT_IF


// access list of allowed source and target languages
#define MTLIST_ALLOWEDTARGETLANG
#define MTLIST_ALLOWEDSOURCELANG
#include <eqfmtlst.h>
#include <eqfaprof.h>

  extern HELPSUBTABLE hlpsubtblAnaPropDlg[];
  extern HELPSUBTABLE hlpsubtblAnaPropGeneral[];
  extern HELPSUBTABLE hlpsubtblAnaPropAutoSubst[];
  extern HELPSUBTABLE hlpsubtblAnaPropMisc[];
  extern HELPSUBTABLE hlpsubtblAnaPropList[];

//--- internal function prototypes ---
static VOID    HandleOkButton( HWND, PAIDA, PPROPFOLDER );
static VOID    Iana1CleanUp( PAIDA, BOOL );
static VOID    FillSelectedLb( HWND, HWND, USHORT );
static VOID    CopyGeneralInfo2TAI( PAIDA, PPROPFOLDER );
static VOID    Close_proc( HWND, WPARAM);
static VOID    InsertItemToLbIfNotExist( HWND, PSZ, USHORT );
static VOID    InsertItemToLb1IfExistInLb2( HWND, HWND, PSZ );
static BOOL    GetNumberOfSelectedMdbs( PAIDA, HWND );
static BOOL    Files2BeAnalyzed2Analysis( PAIDA, HWND );
static VOID    Dicts2Analysis( PAIDA, PPROPFOLDER, HWND );
static VOID    EnableDisableTMListBoxes( HWND, BOOL );
static VOID    SetValuesNewTermsList(HWND, PAIDA);
static VOID    SetValuesFoundTermsList(HWND, PAIDA);
static VOID    SetValuesExclusionList(HWND, PAIDA);
static VOID    EnableDisableNewTermsList(HWND, BOOL);
static VOID    EnableDisableFoundTermsList(HWND, BOOL);

// modes for HandleOkButtonAna2 function:
#define ANA2_STANDARD_MODE    0
#define ANA2_PAGESWITCH_MODE  1
#define ANA2_FINALCHECK_MODE  2

static BOOL    HandleOkButtonAna2(HWND, PAIDA, PPROPFOLDER, SHORT );

static VOID    GetLastUsedAnalysis2Values(PAIDA, PPROPFOLDER);
static VOID    FillDropDownAddDict(HWND, PAIDA);
static VOID    FillDropDownExclDict(HWND, PAIDA);
static BOOL    GetNumberOfSelectedDicts(PAIDA, HWND);
static VOID    SaveAnalysis2ToAIda(HWND, PAIDA);
static VOID    SaveAIda2ToProp(PPROPFOLDER, PAIDA);
static VOID    SaveAIda2ToTAINPUT(PTAINPUT, PAIDA);
static VOID    GetTermsListNames(HWND, CHAR *);
static MRESULT APIENTRY EFSubclassProc(HWND, USHORT, WPARAM, LPARAM);
static VOID    SavePropToTAINPUT(PPROPFOLDER, PAIDA, PTAINPUT);
static BOOL    CheckIfListNameExist(PSZ, PSZ);


/*
+-----------------------------------------------------------------------------+
 Name......: SavePropToTAINPUT

 Purpose...: Save last used values. They were needed when the user press the
             Dialog(2)-CANCEL Button and call list processing.

 Parameters: 1.PPROPFOLDER - pointer to folder properties
             2.PAIDA       - pointer to analysis instance data area
             3.PTAINPUT    - pointer to analysis interface structure

 Returns...: none

 Sample....: -----
+-----------------------------------------------------------------------------+
*/
static   VOID     SavePropToTAINPUT(
                                   PPROPFOLDER ppropFold, // folder properties
                                   PAIDA       pAIda,     // analysis instance
                                   PTAINPUT    pTAI       // analysis interface
                                   )
{
   register   USHORT       i;

   if (ppropFold->fAddTermsToDicCb)
   {
     strcpy( pTAI->szOutDictName, ppropFold->szAddTermsToDicDd);
   }
   else
   {
       *(pTAI->szOutDictName) = EOS;
   } /* endif */


   if (ppropFold->fCreateNewTermsCb)
   {
       GETEQFLISTPATH(pTAI->szNTLname);// append the filename of the list.
       strcat(pTAI->szNTLname, BACKSLASH_STR);
       strcat(pTAI->szNTLname, ppropFold->szCreateNewTermsDd);
       strcat(pTAI->szNTLname, EXT_OF_NEWTERMS_LIST);

       pTAI->fNTLMwterm         = ppropFold->fNTLMwtermCb;
       pTAI->usMWTOption        = ppropFold->usMWTOption;
       pTAI->fNTLcontext        = ppropFold->fNTLcontextCb;
       pTAI->usNTLNumOccurences = ppropFold->usNTLNumOccurences;
   }
   else
   {
       *(pTAI->szNTLname) = EOS;
       pTAI->fNTLMwterm         = 0;
       pTAI->fNTLcontext        = 0;
       pTAI->usNTLNumOccurences = 0;
       pTAI->usMWTOption        = 0;
   }


   // save values of Found Terms List
   if (ppropFold->fCreateFndTermsCb)
   {
       GETEQFLISTPATH(pTAI->szFTLname);// append the filename of the list.
       strcat(pTAI->szFTLname, BACKSLASH_STR);
       strcat(pTAI->szFTLname, ppropFold->szCreateFndTermsDd);
       strcat(pTAI->szFTLname, EXT_OF_FOUNDTERMS_LIST);

       pTAI->fFTLcontext        = ppropFold->fFTLcontextCb;
       pTAI->usFTLNumOccurences = ppropFold->usFTLNumOccurences;
   }
   else
   {
       *(pTAI->szFTLname) = EOS;
       pTAI->fFTLcontext        = 0;
       pTAI->usFTLNumOccurences = 0;
   }

   // save values of Exclusion List
   if (ppropFold->fExTermsInListCb)
   {
        GETEQFLISTPATH(pTAI->szExclusionList);
        strcat(pTAI->szExclusionList, BACKSLASH_STR);
        strcat(pTAI->szExclusionList, ppropFold->szExTermsInListDd);
        strcat(pTAI->szExclusionList, EXT_OF_EXCLUSION);
        pAIda->fExTermsInListCb = TRUE;
        strcpy( pAIda->szExTermsInListDd, ppropFold->szExTermsInListDd );

   }
   else
       *(pTAI->szExclusionList) = EOS;

   // save values of Exclusion Dictionary
   if (ppropFold->fExTermsInDicCb)
   {
       strcpy(pTAI->szExclDictname, ppropFold->szExTermsInDicDd);
   }
   else
       *(pTAI->szExclDictname) = EOS;

   pTAI->stInputDict.usNumber = 0;

   for (i = 0; i < SAVE_DIC_NUM; i++)
   {
      if (*(ppropFold->szSavedDlgIanaInDic[i]) == EOS)
          break;

      strcpy(pAIda->Dict.szDictName[i], ppropFold->szSavedDlgIanaInDic[i]);
      pTAI->stInputDict.usNumber++;
   }

   return;
}


/*
+------------------------------------------------------------------------------+
  function IanaCleanUp
    - close folder properties
    - free instance area
    - free analysis interface in dependency of passed flag
+------------------------------------------------------------------------------+
*/
static
VOID Iana1CleanUp( PAIDA pAIda, BOOL fTAI )
{
   ULONG       ulErrorInfo;                 //error indicator from PRHA

   // close folder properties
   CloseProperties( pAIda->hpropFolder, 0, &ulErrorInfo);

   //--- free memory of analysis interface in dependency of fTAI
   //--- fTAI : FALSE -> free analysis interface ( analysis instance will
   //---                                           not be started         )
   //---      : TRUE  -> do not free analysis interface ( analysis instacnc will
   //---                                                  be started           )
   if ( ! fTAI )
   {
     if ( pAIda->pTAI )
     {
       if ( pAIda->pTAI->pPool ) PoolDestroy( pAIda->pTAI->pPool );
       if ( pAIda->pTAI->apszLongNames )
         UtlAlloc( (PVOID *)&(pAIda->pTAI->apszLongNames), 0L, 0L, NOMSG );
       if ( pAIda->pTAI->apszAlias )
         UtlAlloc( (PVOID *)&(pAIda->pTAI->apszAlias), 0L, 0L, NOMSG );
     } /* endif */
     UtlAlloc( (PVOID *) &pAIda->pTAI, 0L, 0L, NOMSG );
   } /* endif */
      UtlAlloc( (PVOID *) &pAIda, 0L, 0L, NOMSG );
   return;
}/*end Iana1CleanUp */

/*
+------------------------------------------------------------------------------+
  function CopyRest2ToTAI
     copy additional data to analysis interface
+------------------------------------------------------------------------------+
*/
static
VOID CopyGeneralInfo2TAI( PAIDA pAIda, PPROPFOLDER ppropFolder )
{
   PSZ  pszFolder;

   // copy folder name to analysis interface
   strcpy ( pAIda->pTAI->szFolder, pAIda->szFolderObjName );

   //--- copy folder property name to analysis interface
   strcpy( pAIda->pTAI->szFolderProp, pAIda->szFolderProp );

   //path for source files
   // get folder name
   pszFolder =  UtlGetFnameFromPath( pAIda->szFolderObjName);

   UtlMakeEQFPath( pAIda->pTAI->szSOURCE_Path, pAIda->szFolderObjName[0],
                   DIRSOURCEDOC_PATH,
                   pszFolder);

   //path for segmented source
   UtlMakeEQFPath( pAIda->pTAI->szSEGSOURCE_Path, pAIda->szFolderObjName[0],
                   DIRSEGSOURCEDOC_PATH,
                   pszFolder);

   //path for segmented target
   UtlMakeEQFPath( pAIda->pTAI->szSEGTARGET_Path,pAIda->szFolderObjName[0],
                   DIRSEGTARGETDOC_PATH,
                   pszFolder);

   //path for segmented new found matches
   UtlMakeEQFPath( pAIda->pTAI->szSEGNEWMATCH_Path,pAIda->szFolderObjName[0],
                   DIRSEGNOMATCH_PATH,
                   pszFolder);

   UtlMakeEQFPath( pAIda->pTAI->szMACHTRANS_Path, pAIda->szFolderObjName[0], DIRSEGMT_PATH, pszFolder);

   //--- save segement tags to ida
   memcpy(pAIda->pTAI->TATag, TADummyTag, sizeof(TATAG) * MAX_SEG_CLASSES);
   TATATag2Unicode( pAIda->pTAI->TATag, pAIda->pTAI->TATagW );

   //--- match level form folder properties
   //later folder property
   pAIda->pTAI->sMatchLevel = ppropFolder->usMatchLevel;

   //file name of Tagtable
   strcpy( pAIda->pTAI->szTagTableName, ppropFolder->szFormat );

   return;
}/*CopyGeneralInfo2TAI*/
/*
+------------------------------------------------------------------------------+
  function Close_proc
   - return pointer of analysis interface to analysis handler
   - call Iana1Cleanup for cleanup processing
   - destroy dialog
+------------------------------------------------------------------------------+
*/
static
VOID  Close_proc( HWND hwnd, WPARAM mp1 )
{
   PAIDA        pAIda;                      //instance area of dialog

   pAIda = ACCESSDLGIDA( hwnd, PAIDA );
   if ( pAIda )
   {
      pAIda->pA->ProcParm = pAIda->pTAI;
      Iana1CleanUp( pAIda, SHORT1FROMMP1( mp1 ) );
   }/*end if*/
   DelCtrlFont(hwnd, DID_AN_ANALYZE_LB );
   WinDismissDlg( hwnd, SHORT1FROMMP1( mp1 ) );
}/*end  Close_proc */

/*
+------------------------------------------------------------------------------+
  function InsertItemToLb1IfExistInLb2
    insert passed itemtext into listbox Lb1 if also exists in Lb2
+------------------------------------------------------------------------------+
*/
static
VOID InsertItemToLb1IfExistInLb2( HWND hwndLb1,
                                  HWND hwndLb2,
                                  PSZ  pszString     )
{
   SHORT sIndex;                            //index of LB item

   //get index of item text listbox Lb2
   sIndex = SEARCHITEMHWND( hwndLb2, pszString );
   if ( sIndex != LIT_NONE)                        //item exists in listbox Lb2
   {
      //insert item in listbox Lb1
      INSERTITEMENDHWND( hwndLb1, pszString );
   } /* endif */
   return;
}/* InsertItemToLb1IfExistInLb2 */

/*
+------------------------------------------------------------------------------+
  function InsertItemToLbIfNotExist
    Insert item Text to listbox if not already exists
+------------------------------------------------------------------------------+
*/
static
VOID InsertItemToLbIfNotExist( HWND hwndLb,
                               PSZ  pszString,
                               USHORT usMsg )
{
   SHORT      sIndex;                     //index of selected item in listb.

   //check if passed item text exists in LB
   sIndex = SEARCHITEMHWND( hwndLb, pszString );
   if ( sIndex == LIT_NONE )              //passed string not exists in LB
   {
      //include passed item text into LB
      INSERTITEMENDHWND( hwndLb, pszString );
    }
    else
    {
      if ( usMsg )
      {
        UtlError( usMsg, MB_OK, 0, (PSZ *)0, EQF_INFO );
      } /* endif */
    } /* endif */
   return;
}/* end InsertItemToLbIfNotExist */

/*
+------------------------------------------------------------------------------+
   function FillSelectedLb
     get selected item in available Lb1
     insert item to selected LB if not exist there
+------------------------------------------------------------------------------+
*/
static
VOID FillSelectedLb( HWND hwndSelectedLb, HWND hwndAvailableLb,
                     USHORT usMsg )
{
   SHORT sIndex;                            //index of LB item
   static CHAR  szString[MAX_LONGPATH];     //text of LB item

   //get index of selected item in hwndAvailableLb
   sIndex = QUERYSELECTIONHWND( hwndAvailableLb );

   //get name of selected item in available LB
   if ( sIndex != LIT_NONE )
   {
     QUERYITEMTEXTHWND( hwndAvailableLb, sIndex, szString );
     InsertItemToLbIfNotExist( hwndSelectedLb, szString, usMsg );
   } /* endif */

   return;
} /*FillSelectedLb */

/*
+------------------------------------------------------------------------------+
   function GetNumberOfSelectedMdbs
     get number of memorydatabases in selected listbox
+------------------------------------------------------------------------------+
*/
static
BOOL GetNumberOfSelectedMdbs( PAIDA pAIda, HWND hwnd )
{
   BOOL   fOk = TRUE;

   //-- if add segment to to TM flag is set
   if ( pAIda->pTAI->fInsertToTM )
   {
      //--- get number selected memory databases ---
      pAIda->pTAI->stInputMemDb.usNumber = QUERYITEMCOUNT( hwnd,
                                                           DID_AN_MDBSEL_LB );
      //--- if no tm selected and TM CB and LB's are not disabled
      if ( pAIda->pTAI->stInputMemDb.usNumber == 0 )
      {
         //display error message
         UtlErrorHwnd( ERROR_NO_INMDB_SELECTED, MB_CANCEL,
                       0, NULL, EQF_ERROR, hwnd);
         fOk = FALSE;
       }/*end if*/
    }/*end if*/
    return ( fOk );
}/* end GetNumberOfSelectedMdbs */


/*
+------------------------------------------------------------------------------+
   function GetNumberOfSelectedDicts
     get number of dictionaries in selected listbox
+------------------------------------------------------------------------------+
*/
static   BOOL       GetNumberOfSelectedDicts(PAIDA pAIda, HWND hwnd)
{
         BOOL       fOk = TRUE;
         USHORT     usResponse;

                                        //get number selected dictionaries
    pAIda->pTAI->stInputDict.usNumber = QUERYITEMCOUNT(hwnd, DID_AN2_DICTSEL_LB);

    if (pAIda->pTAI->stInputDict.usNumber == 0)
    {
                                         //display error message
        usResponse = UtlErrorHwnd( ERROR_NO_INDIC_SELECTED, MB_CANCEL,
                                   0, NULL, EQF_ERROR, hwnd);
        fOk = FALSE;
        if (!fOk)
           SETFOCUS( hwnd, DID_AN2_DICTAVA_LB );

     } /* end if */

 return (fOk);

}/*end GetNumberOfSelectedDicts*/

/*
+------------------------------------------------------------------------------+
   function Files2BeAnalyzed2Analysis
     - save filenames of files to be analyzed to Analysis Interface
+------------------------------------------------------------------------------+
*/
static
BOOL Files2BeAnalyzed2Analysis( PAIDA pAIda, HWND hwnd )
{
   SHORT         sIndex;               // index of files to be analyzed
   BOOL          fOk = TRUE;           // Error indicator (go on flag)
   BOOL          usResponse;           // return from UtlError
   HPROP         hPropDocument;        // handle to document properties
   PPROPDOCUMENT pPropDocument;        // pointer to documnet properties
   ULONG         ulErrorInfo;          // error indicator from PRHA
   SHORT         sOrgFiles;            // original number of analysis files
   HWND          hwndLB;               // listbox to be used for documents

   sOrgFiles = pAIda->pTAI->stSourcefiles.usNumber;
   hwndLB = pAIda->pA->hwndDocLB;

   if ( fOk )
   {
     // allocate pointer array for long names
     LONG lSize = (LONG)sizeof(PSZ) * (LONG)sOrgFiles;
     if ( lSize < MIN_ALLOC) lSize = MIN_ALLOC;
     fOk = UtlAlloc( (PVOID *)&(pAIda->pTAI->apszLongNames), 0L, lSize,
                     ERROR_STORAGE );
   } /* endif */

   if ( fOk )
   {
     // allocate pointer array for alias names
     LONG lSize = (LONG)sizeof(PSZ) * (LONG)sOrgFiles;
     if ( lSize < MIN_ALLOC) lSize = MIN_ALLOC;
     fOk = UtlAlloc( (PVOID *)&(pAIda->pTAI->apszAlias), 0L, lSize,
                     ERROR_STORAGE );
   } /* endif */

   // create string pool
   if ( fOk )
   {
     pAIda->pTAI->pPool = PoolCreate( 32000 );
     fOk = (pAIda->pTAI->pPool != NULL);
   } /* endif */

   if ( fOk )
   {
     SHORT sFileNum;                   // number of file in file table

     sIndex = 0;
     sFileNum = 0;
     while ( sIndex < (SHORT)pAIda->pTAI->stSourcefiles.usNumber && fOk)
     {
        //--- get selected files to be analyzed
        QUERYITEMTEXTHWND( hwndLB, sIndex, pAIda->szString );                  \

        //--- copy selected file to be analyzed to analysis interface
        strcpy( pAIda->pszAct, pAIda->szString );

        // open document proeprties
        hPropDocument = OpenProperties( pAIda->szString,
                                        pAIda->szFolderObjName,
                                        PROP_ACCESS_READ, &ulErrorInfo );
        if( hPropDocument )
        {
          pPropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hPropDocument );

          // store long file name in string buffer
          if ( pPropDocument->szLongName[0] != EOS )
          {
            strcpy( pAIda->szString, pPropDocument->szLongName );
            OEMTOANSI( pAIda->szString );
          }
          else
          {
            strcpy( pAIda->szString, pAIda->pszAct  );
          } /* endif */

          //--- check if file is currently in use
          sprintf( pAIda->szDocObjName, "%s\\%s",pAIda->szFolderObjName,
                   pAIda->pszAct );

          if ( EqfQueryObject( pAIda->szDocObjName, clsANY, 0 ) )
          {
             PSZ pszDocName = pAIda->szString;
             usResponse = UtlErrorHwnd( ERROR_DOC_INUSE,
                                        MB_YESNO, 1, &pszDocName,
                                        EQF_QUERY, hwnd);
             fOk = (usResponse !=MBID_NO);
          }
          else
          {
            if (pPropDocument->ulTouched )  // document has already been touched
            {
              if ( pAIda->pTAI->fYesToAll )
              {
                usResponse = MBID_YES;
              }
              else
              {
                PSZ pszDocName = pAIda->szString;

                if ( sOrgFiles > 1)
                {
                  usResponse = UtlErrorHwnd( ERROR_DOC_TOUCHED,
                                             MB_EQF_YESTOALL | MB_DEFBUTTON3,
                                             1, &pszDocName, EQF_QUERY, hwnd);
                }
                else
                {
                  usResponse = UtlErrorHwnd( ERROR_DOC_TOUCHED,
                                             MB_YESNOCANCEL | MB_DEFBUTTON2,
                                             1, &pszDocName, EQF_QUERY, hwnd);
                } /* endif */

                if ( usResponse == MBID_EQF_YESTOALL )
                {
                  pAIda->pTAI->fYesToAll = TRUE;
                  usResponse = MBID_YES;
                } /* endif */
              } /* endif */

            }
            else
            {
              usResponse = MBID_YES;
            } /* endif */

            switch (usResponse)
            {
               case MBID_YES:          // segment the file
                  // increase pointer
                  pAIda->pszAct += strlen( pAIda->pszAct ) + 1;
                  pAIda->pTAI->apszLongNames[sFileNum] =
                     PoolAddString( pAIda->pTAI->pPool, pAIda->szString );
                  if ( pPropDocument->szAlias[0] != EOS )
                  {
                    pAIda->pTAI->apszAlias[sFileNum] =
                       PoolAddString( pAIda->pTAI->pPool,
                                      pPropDocument->szAlias );
                  } /* endif */
                  sFileNum++;
                  break;

               case MBID_NO:           // ignore the file
                  DELETEITEMHWND( hwndLB, sIndex );
                  sIndex --;
                  pAIda->pTAI->stSourcefiles.usNumber --;
                  break;

                case MBID_CANCEL:       // cancel text analysis
                  fOk = FALSE;
                  break;
            } /* endswitch */
          } /* endif */
          CloseProperties( hPropDocument, PROP_QUIT, &ulErrorInfo);
        }
        else
        {
          if ( ulErrorInfo != Err_NoStorage )
          {
             usResponse = UtlErrorHwnd( ERROR_OPEN_PROPERTIES,
                                        MB_CANCEL, 1, &(pAIda->pszAct),
                                        EQF_ERROR, hwnd);
          } /* endif */
          fOk = FALSE;
        } /* endif */
        sIndex++;
     }/*end while */
   } /* endif */

   if ( fOk && (pAIda->pTAI->stSourcefiles.usNumber == 0) )
   {
     if ( sOrgFiles > 1 )
     {
       // display error message if more than one file is to be analyzed
       UtlErrorHwnd( ERROR_NODOCS_ANALYSIS, MB_CANCEL,
                     0, NULL, EQF_WARNING, hwnd);
     } /* endif */
     fOk = FALSE;
   }/*end if*/

   if (fOk)
   {
      //save offset to analysis interface
      pAIda->pTAI->stSourcefiles.ulOffset = pAIda->pszStart - (PSZ)(pAIda->pTAI);

      //save length to analysis interface
      pAIda->pTAI->stSourcefiles.ulLength = pAIda->pszAct - pAIda->pszStart;

      //--- get write access to folder prop. to save last used values
      //get write access to folder properties
      if ( !SetPropAccess( pAIda->hpropFolder, PROP_ACCESS_WRITE) )
      {
         usResponse = UtlErrorHwnd( ERROR_NO_LUVALUES_SAVED, MB_YESNO,
                                    0, NULL, EQF_QUERY, hwnd);

         fOk = ( usResponse != MBID_NO );
      } /* endif */
   }/* endif */
   return ( fOk );
}/* end of Files2BeAnalyzed2Analysis */


/*
+------------------------------------------------------------------------------+
   function Dicts2Analysis
     - get selected dictionaries
     - save names of selected dictionry to Analusis Interface
+------------------------------------------------------------------------------+
*/
static
VOID Dicts2Analysis( PAIDA pAIda, PPROPFOLDER ppropFolder, HWND hwnd)
{
   SHORT sIndex;                             //dictionary index

   // init last used values
   memset( ppropFolder->szSavedDlgIanaInDic, NULC,
           sizeof(ppropFolder->szSavedDlgIanaInDic) );

   for ( sIndex = 0; sIndex < (SHORT)pAIda->pTAI->stInputDict.usNumber;
         sIndex++ )
   {
      //get selected dictionaries
      QUERYITEMTEXT( hwnd, DID_AN2_DICTSEL_LB, sIndex, pAIda->szString );

      // copy selected dictionaries to  analysis interface
      strcpy(pAIda->Dict.szDictName[sIndex], pAIda->szString);

      //save 10 selected dictionaries as last used values
      if (sIndex < SAVE_DIC_NUM )
         strcpy(ppropFolder->szSavedDlgIanaInDic[sIndex], pAIda->szString);

   }/*end for*/

return;
}/* end of Dicts2Analysis */

/*
+------------------------------------------------------------------------------+
   function EnableDisableTMListBoxes
     - enable or disable the TM listboxes and in dependency of passed flag
+------------------------------------------------------------------------------+
*/
static
VOID EnableDisableTMListBoxes( HWND hwnd, BOOL fEnable )
{
  ENABLECTRL( hwnd, DID_AN_MDBAVA_LB,      fEnable );
  ENABLECTRL( hwnd, DID_AN_MDBSEL_LB,      fEnable );
  ENABLECTRL( hwnd, DID_AN_TEXT3,          fEnable );
  ENABLECTRL( hwnd, DID_AN_TEXT6,          fEnable );
  ENABLECTRL( hwnd, DID_AN_STOPATFIRSTEXACT_CB, fEnable );

}/* end of EnableDisableTMListBoxes */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name: SetValuesExclusionList
//+----------------------------------------------------------------------------+
// Description: Fills the possible values of the ExclusionList listbox, and
//              selects the default, if possible.
//+----------------------------------------------------------------------------+
// Parameters:
//             HWND - the window handle
//             PAIDA - ptr to data record for the analysis dialog
//+----------------------------------------------------------------------------+
// Returncode type: VOID
//+----------------------------------------------------------------------------+
// Prerequesits: PAIDA must be initialised.
//               HWND must be the window handle of the analysis dialog
//+----------------------------------------------------------------------------+
// Side effects: Fills the exclusion list listbox.
//+----------------------------------------------------------------------------+
static VOID SetValuesExclusionList(HWND hwnd, PAIDA pAIda)
{
    SHORT       sIndex;
    CHAR        szFile[MAX_EQF_PATH+MAX_FILESPEC];

    SETCHECK(hwnd, DID_AN2_EXTERMSINLIST_CB, pAIda->fExTermsInListCb);

    GETEQFLISTPATH(szFile);
    strcat(szFile, "\\*");
    strcat(szFile, EXT_OF_EXCLUSION);
    GetTermsListNames(WinWindowFromID(hwnd, DID_AN2_EXTERMSINLIST_DL), szFile);

    // check how much exclusionlists available
    sIndex = CBQUERYITEMCOUNT( hwnd, DID_AN2_EXTERMSINLIST_DL );
    if(sIndex != 0)
    {
      sIndex = CBSEARCHITEM( hwnd, DID_AN2_EXTERMSINLIST_DL,
                             pAIda->szExTermsInListDd );

      if (sIndex != LIT_NONE)             // if name found, select it
      {
        CBSELECTITEM(hwnd, DID_AN2_EXTERMSINLIST_DL, sIndex);
      }
      else                                         // else select first item
      {
        CBSELECTITEM(hwnd, DID_AN2_EXTERMSINLIST_DL, 0);
      } /* endif */

      ENABLECTRL( hwnd, DID_AN2_EXTERMSINLIST_DL, pAIda->fExTermsInListCb);
    }
    else
    {
    // no entries exclusion list available
    // disable checkbox for using exclusionlist
    SETCHECK_FALSE(hwnd, DID_AN2_EXTERMSINLIST_CB);
    ENABLECTRL(hwnd, DID_AN2_EXTERMSINLIST_CB, FALSE);
    ENABLECTRL(hwnd, DID_AN2_EXTERMSINLIST_DL, FALSE);
    }

    return;

}/* end of SetValuesExclusionList */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name: SetValuesNewTermList
//+----------------------------------------------------------------------------+
// Description: Fills the possible values of the new terms List listbox, adds
//              the current folder name as an option and sets the default.
//+----------------------------------------------------------------------------+
// Parameters:
//             HWND - the window handle
//             PAIDA - ptr to data record for the analysis dialog
//+----------------------------------------------------------------------------+
// Returncode type: VOID
//+----------------------------------------------------------------------------+
// Prerequesits: PAIDA must be initialised.
//               HWND must be the window handle of the analysis dialog
//+----------------------------------------------------------------------------+
// Side effects: Fills the new term list listbox.
//+----------------------------------------------------------------------------+
static VOID SetValuesNewTermsList(HWND hwnd, PAIDA pAIda)
{
    CHAR        szMinOcc[ANA_NO_OF_OCCURENCES+1];
    SHORT       sIndex;
    CHAR        szFile[MAX_EQF_PATH+MAX_FILESPEC];

    SETCHECK(hwnd, DID_AN2_NEWTERM_CB,     pAIda->fCreateNewTermsCb);
    SETCHECK(hwnd, DID_AN2_NEWLISTINFO_MWT_CB, pAIda->fNTLMwtermCb);
    SETCHECK(hwnd, DID_AN2_NEWLISTINFO_CB, pAIda->fNTLcontextCb);
    itoa(pAIda->usNTLNumOccurences, szMinOcc, 10);

    if (pAIda->usNTLNumOccurences != 0)
        SETTEXT( hwnd, DID_AN2_NEWMINOCCUR_EF, szMinOcc );


    // if no previous default New Terms file, make the folder name the default
    if (*(pAIda->szCreateNewTermsDd) == EOS)
        strcpy(pAIda->szCreateNewTermsDd, pAIda->szFolderName);

    // Put default name into list box
    CBINSERTITEM(hwnd, DID_AN2_NEWTERMDIC_DL, pAIda->szCreateNewTermsDd );

    GETEQFLISTPATH(szFile);
    strcat(szFile, "\\*");
    strcat(szFile, EXT_OF_NEWTERMS_LIST);
    GetTermsListNames(WinWindowFromID(hwnd, DID_AN2_NEWTERMDIC_DL), szFile);

    sIndex = CBSEARCHITEM( hwnd, DID_AN2_NEWTERMDIC_DL, pAIda->szCreateNewTermsDd );

    if (sIndex != LIT_NONE)             // if name found, select it
    {
       CBSELECTITEM(hwnd, DID_AN2_NEWTERMDIC_DL, sIndex);
    }
    else
    {
       // select first item in list
       CBSELECTITEM(hwnd, DID_AN2_NEWTERMDIC_DL, 0);
    } /* endif */

    EnableDisableNewTermsList(hwnd, pAIda->fCreateNewTermsCb);


     // fill MWT option drop-down list
    CBDELETEALL(hwnd, DID_AN2_NEWTERMOPT_MWT_DL);
    CBINSERTITEMEND( hwnd, DID_AN2_NEWTERMOPT_MWT_DL, STR_MWT_OPTION_1);
    CBINSERTITEMEND( hwnd, DID_AN2_NEWTERMOPT_MWT_DL, STR_MWT_OPTION_2);
    CBINSERTITEMEND( hwnd, DID_AN2_NEWTERMOPT_MWT_DL, STR_MWT_OPTION_3);


    if (pAIda->usMWTOption < 0 || pAIda->usMWTOption >= 3)
    {
      pAIda->usMWTOption = 0;
    }//end if

    CBSELECTITEM(hwnd, DID_AN2_NEWTERMOPT_MWT_DL, pAIda->usMWTOption);


return;
}/* end of SetValuesNewTermsList */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name: GetTermsListNames
//+----------------------------------------------------------------------------+
// Description: Insert filenames from the specified directory into the listbox
//              specified by HWND.
//+----------------------------------------------------------------------------+
// Parameters:
//             HWND - the window handle for the listbox being filled
//             CHAR* - ptr to directory name to be searched
//+----------------------------------------------------------------------------+
// Returncode type: VOID
//+----------------------------------------------------------------------------+
// Prerequesits: CHAR* must be a valid directory name
//               HWND must be the window handle of a listbox.
//+----------------------------------------------------------------------------+
// Side effects: Inserts the possible list names from the list directory
//+----------------------------------------------------------------------------+
static   VOID       GetTermsListNames(HWND hwnd, CHAR *pszDir)
{
  FILEFINDBUF FindBuf;
  HDIR       hSearch = HDIR_CREATE;
  USHORT     cb = 1;
  CHAR       szFile[MAX_FILESPEC];
  SHORT      sIndex;
  PSZ        pszName = RESBUFNAME(FindBuf);

  UtlFindFirst( pszDir, &hSearch, FILE_NORMAL, &FindBuf, sizeof(FindBuf), &cb, 0L, FALSE);

  while ( cb == 1 )
  {
    Utlstrccpy( szFile, pszName, DOT );

    sIndex = CBSEARCHITEMHWND( hwnd, szFile );

    if (sIndex == LIT_NONE)       // passed string not exists in LB
    {
      CBINSERTITEMHWND( hwnd, szFile );
    } /* endif */
    UtlFindNext( hSearch, &FindBuf, sizeof(FindBuf), &cb, FALSE );
  } /* endwhile */

  UtlFindClose( hSearch, FALSE );

  return;
}/* end of SetValuesNewTermsList */

/*
+------------------------------------------------------------------------------+
   function EnableDisableNewTermsList
     - enable or disable the new terms matches cb and the correspponding test
+------------------------------------------------------------------------------+
*/
static VOID EnableDisableNewTermsList(HWND hwnd, BOOL fEnable)
{
  ENABLECTRL( hwnd, DID_AN2_NEWTERMDIC_DL  , fEnable);
  ENABLECTRL( hwnd, DID_AN2_NEWTERMOPT_MWT_DL  , fEnable);
  ENABLECTRL( hwnd, DID_AN2_NEWLISTINFO_CB , fEnable);
  ENABLECTRL( hwnd, DID_AN2_NEWLISTINFO_MWT_CB , fEnable);
  ENABLECTRL( hwnd, DID_AN2_NEWMINOCCUR_TXT, fEnable);
  ENABLECTRL( hwnd, DID_AN2_NEWMINOCCUR_EF , fEnable);
  return;
}/* end of EnableDisableNewTermsList */


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name: SetValuesFoundTermList
//+----------------------------------------------------------------------------+
// Description: Fills the possible values of the Found terms list listbox, adds
//              the current folder name as an option and sets the default.
//+----------------------------------------------------------------------------+
// Parameters:
//             HWND - the window handle
//             PAIDA - ptr to data record for the analysis dialog
//+----------------------------------------------------------------------------+
// Returncode type: VOID
//+----------------------------------------------------------------------------+
// Prerequesits: PAIDA must be initialised.
//               HWND must be the window handle of the analysis dialog
//+----------------------------------------------------------------------------+
// Side effects: Fills the found term list listbox.
//+----------------------------------------------------------------------------+
static VOID SetValuesFoundTermsList(HWND hwnd, PAIDA pAIda)
{
    CHAR        szMinOcc[ANA_NO_OF_OCCURENCES+1];
    CHAR        szFile[MAX_EQF_PATH+MAX_FILESPEC];
    SHORT       sIndex;

    SETCHECK( hwnd, DID_AN2_FOUNDTERM_CB, pAIda->fCreateFndTermsCb );
    SETCHECK( hwnd, DID_AN2_FOUNDLISTINFO_CB, pAIda->fFTLcontextCb );

    itoa(pAIda->usFTLNumOccurences, szMinOcc, 10);

    if (pAIda->usFTLNumOccurences != 0)
        SETTEXT( hwnd, DID_AN2_FOUNDMINOCCUR_EF, szMinOcc);

    // if no previous default Fnd Terms file, make the folder name the default
    if (*(pAIda->szCreateFndTermsDd) == EOS)
        strcpy(pAIda->szCreateFndTermsDd, pAIda->szFolderName);

    // Put default name into list box
    CBINSERTITEM( hwnd, DID_AN2_FOUNDTERMDIC_DL, pAIda->szCreateFndTermsDd );

    GETEQFLISTPATH(szFile);
    strcat(szFile, "\\*");
    strcat(szFile, EXT_OF_FOUNDTERMS_LIST);
    GetTermsListNames(WinWindowFromID(hwnd, DID_AN2_FOUNDTERMDIC_DL), szFile);

    sIndex = CBSEARCHITEM( hwnd, DID_AN2_FOUNDTERMDIC_DL,
                           pAIda->szCreateFndTermsDd );

    if (sIndex != LIT_NONE)             // if name found, select it
    {
      CBSELECTITEM(hwnd, DID_AN2_FOUNDTERMDIC_DL, sIndex);
    }
    else
    {
      CBSELECTITEM(hwnd, DID_AN2_FOUNDTERMDIC_DL, 0);
    } /* endif */

    EnableDisableFoundTermsList(hwnd, pAIda->fCreateFndTermsCb);
  return;
}/* end of SetValuesFoundTermsList */
/*
+------------------------------------------------------------------------------+
   function EnableDisableFoundTermsList
     - enable or disable the new terms matches cb and the correspponding test
+------------------------------------------------------------------------------+
*/
static VOID EnableDisableFoundTermsList(HWND hwnd, BOOL fEnable)
{
  ENABLECTRL( hwnd, DID_AN2_FOUNDTERMDIC_DL  , fEnable);
  ENABLECTRL( hwnd, DID_AN2_FOUNDLISTINFO_CB , fEnable);
  ENABLECTRL( hwnd, DID_AN2_FOUNDMINOCCUR_TXT, fEnable);
  ENABLECTRL( hwnd, DID_AN2_FOUNDMINOCCUR_EF , fEnable);
  return;
}/* end of EnableDisableFoundTermsList */

/*
+------------------------------------------------------------------------------+
  function GetLastUsedAnalysis2Values
+------------------------------------------------------------------------------+
*/
static   VOID   GetLastUsedAnalysis2Values(PAIDA pAIda, PPROPFOLDER ppropFolder)

{
         PSZ        psz;                // pointer for string compare
                                        // folder properties

    // Add terms to dictionary checkbox (last used)
    pAIda->fAddTermsToDicCb = ppropFolder->fAddTermsToDicCb;

    //Add terms to Dictionaries drop down box (folder property)
    strcpy(pAIda->szAddTermsToDicDd, ppropFolder->szAddTermsToDicDd);

    //Exclude terms in list checkbox (last used)
    pAIda->fExTermsInListCb = ppropFolder->fExTermsInListCb;

    //Exclude terms in list drop down box (last used)
    strcpy(pAIda->szExTermsInListDd, ppropFolder->szExTermsInListDd);

    //Exclude terms in dictionary checkbox (last used)
    pAIda->fExTermsInDicCb = ppropFolder->fExTermsInDicCb;

    //Exclude terms in dictionary drop down box (last used)
    strcpy(pAIda->szExTermsInDicDd, ppropFolder->szExTermsInDicDd);

    //Create list of new terms checkbox (last used)
    pAIda->fCreateNewTermsCb = ppropFolder->fCreateNewTermsCb;

    //Create list of found terms checkbox (last used)
    pAIda->fCreateFndTermsCb = ppropFolder->fCreateFndTermsCb;

    //Create list od new/found terms drop down box (folder name + .ext)
    strcpy(pAIda->szString, ppropFolder->PropHead.szName);

    psz = strchr(pAIda->szString, DOT);

    if (psz != NULL)
    {
       *psz = NULC;
    } /* endif */
                                        // copy filename without extension
    Utlstrccpy( pAIda->szCreateNewTermsDd, ppropFolder->szCreateNewTermsDd, DOT );
    Utlstrccpy( pAIda->szCreateFndTermsDd, ppropFolder->szCreateFndTermsDd, DOT );

    // Minimum number of occurrences (last used) for list of new terms
    pAIda->usNTLNumOccurences   = ppropFolder->usNTLNumOccurences;

    // with multi-term information, checkbox for list of new terms
    pAIda->fNTLMwtermCb = ppropFolder->fNTLMwtermCb;
    pAIda->usMWTOption = ppropFolder->usMWTOption;

    // with context information, checkbox (last used) for list of new terms
    pAIda->fNTLcontextCb = ppropFolder->fNTLcontextCb;

    // Minimum number of occurrences (last used) for list of found terms
    pAIda->usFTLNumOccurences  = ppropFolder->usFTLNumOccurences;

    // with context information, checkbox (last used) for list of found terms
    pAIda->fFTLcontextCb = ppropFolder->fFTLcontextCb;

    return;

}/* GetLastUsedAnalysis2Values */


/*
+------------------------------------------------------------------------------+
  function FillDropDownAddDict
+------------------------------------------------------------------------------+
*/
static   VOID       FillDropDownAddDict(
                                        HWND        hwnd,
                                        PAIDA       pAIda
                                       )

{
register
         SHORT      sIndex;
         HWND       hwndDL;


    hwndDL = WinWindowFromID(hwnd, DID_AN2_ADDTERMDIC_DL);


   //get all available dict. and display them in available dict. LB
   EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES,
                     MP1FROMHWND(hwndDL), 0L);

                                 // search last used dictionary to add terms
   sIndex = CBSEARCHITEM( hwnd, DID_AN2_ADDTERMDIC_DL,
                          pAIda->szAddTermsToDicDd );


    if (sIndex != LIT_NONE)            // if name found, select it
    {
        CBSELECTITEM(hwnd, DID_AN2_ADDTERMDIC_DL, sIndex);
    }
    else                               // if name not found, select first item
    {
        CBSELECTITEM(hwnd, DID_AN2_ADDTERMDIC_DL, 0);
    } /* endif */
    return;
}

/*
+------------------------------------------------------------------------------+
  function FillDropDownExclDict
+------------------------------------------------------------------------------+
*/
static   VOID       FillDropDownExclDict    (
                                             HWND        hwnd,
                                             PAIDA       pAIda
                                            )

{
register
SHORT      sIndex;
HWND       hwndDL;


    hwndDL = WinWindowFromID(hwnd, DID_AN2_EXTERMSINDIC_DL);


   //get all available dict. and display them in available dict. LB

    EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES,
                     MP1FROMHWND(hwndDL), 0L);

                                 // search last used dictionary to add terms
   sIndex = CBSEARCHITEM( hwnd, DID_AN2_ADDTERMDIC_DL,
                          pAIda->szExTermsInDicDd );


    if (sIndex != LIT_NONE)             // if name found, select it
    {
        CBSELECTITEM(hwnd, DID_AN2_EXTERMSINDIC_DL, sIndex);
    }
    else                                // else select first item
    {
        CBSELECTITEM(hwnd, DID_AN2_EXTERMSINDIC_DL, 0);
    } /* endif */
    return;
}


/*
+------------------------------------------------------------------------------+
   function IsDictInInputs
     - Checks if a dictionary is in the list of input dictionaries
     - returns true if the dictionary is found.
+------------------------------------------------------------------------------+
*/
static BOOL IsDictInInputs( PAIDA pAIda, PSZ pszCheckDict)
{
   SHORT sIndex;                             //dictionary index
   BOOL fFound = FALSE;

   for ( sIndex = 0; sIndex < (SHORT)pAIda->pTAI->stInputDict.usNumber;
         sIndex++ )
      {
      fFound = (stricmp(pAIda->Dict.szDictName[sIndex], pszCheckDict) == 0);

      if (fFound)
        break;
      }

   return fFound;
}/* end of IsDictInInputs */


// Invalid file characters for FAT file systems (Allow "+=;,[]" for HPFS)
#define INVALID_FILE_CHARACTERS "<>+=;,.\"/\\[]"

/*
+------------------------------------------------------------------------------+
  function HandleOkButtonAna2
     this function handles the OK BUTTON
+------------------------------------------------------------------------------+
*/
static
BOOL HandleOkButtonAna2
(
  HWND hwnd ,
  PAIDA pAIda,
  PPROPFOLDER ppropFolder,
  SHORT sMode
)
{
    BOOL        fOk = TRUE;
    USHORT      usResponse;
    PSZ         pszTmp;
    HUCB        hUCB;                  // user control block handle
    HDCB        hDCB;                  // dictionary control block handle
    CHAR        szPropName[MAX_EQF_PATH]; // buffer for dictionary property name
    BOOL        fProtected;            // dictionary-is-protected flag
    USHORT      usRC;                  // return code of called functions
    USHORT      usErrDict;             // number of dictionary in error
    PSZ         pszPropName;           // pointer to property name
    PPROPDICTIONARY pDictProp = NULL;  // ptr to dictionary properties
    SHORT       sIndex;               // listbox item index

  //--- disable ok, cancel and help button more an button ---
  if ( sMode == ANA2_STANDARD_MODE )
  {
    ENABLECTRL( hwnd, PID_PB_OK     , FALSE);
    ENABLECTRL( hwnd, PID_PB_CANCEL , FALSE);
    ENABLECTRL( hwnd, PID_PB_HELP   , FALSE);
  } /* endif */

  SaveAnalysis2ToAIda(hwnd, pAIda);

   if (fOk && pAIda->fCreateNewTermsCb == 1)
   {
      if (*(pAIda->szCreateNewTermsDd) == EOS)
      {
         usResponse = UtlErrorHwnd( ERROR_NO_NTL_SELECTED, MB_CANCEL,
                                    0, NULL, EQF_ERROR, hwnd);
         fOk = FALSE;
         if (!fOk)
            SETFOCUS( hwnd, DID_AN2_NEWTERMDIC_DL );
            SETFOCUS( hwnd, DID_AN2_NEWTERMOPT_MWT_DL );
      } /* endif */

      if (fOk)
      {
         // Search for an invalid character in New Terms filename
         pszTmp = pAIda->szCreateNewTermsDd;
         pszTmp += strcspn(pAIda->szCreateNewTermsDd, INVALID_FILE_CHARACTERS);
         if (*pszTmp) // Found an invalid character
            {
            pszTmp = pAIda->szCreateNewTermsDd;
            usResponse = UtlErrorHwnd( INVALID_CHARACTER_IN_FILENAME,
                                       MB_CANCEL,
                                       1,
                                       &pszTmp,
                                       EQF_ERROR,
                                       hwnd);
            fOk = FALSE;
            SETFOCUS( hwnd, DID_AN2_NEWTERMDIC_DL );
            }
      } /* endif */

      if (fOk)
      {
         if (CheckIfListNameExist(pAIda->szCreateNewTermsDd, EXT_OF_NEWTERMS_LIST) == FALSE)
         {
            pszTmp = pAIda->szCreateNewTermsDd;
            usResponse = UtlErrorHwnd( NTL_FILE_EXISTS,
                                       MB_YESNO | MB_DEFBUTTON2,
                                       1,
                                       &pszTmp,
                                       EQF_QUERY,
                                       hwnd);
            fOk = (usResponse != MBID_NO);
            if (!fOk)
              SETFOCUS( hwnd, DID_AN2_NEWTERMDIC_DL );
         } /* endif */
      } /* endif */
   } /* endif */

   if (fOk && (pAIda->fCreateFndTermsCb == 1))
   {
      if (*(pAIda->szCreateFndTermsDd) == EOS)
         {
         usResponse = UtlErrorHwnd( ERROR_NO_FTL_SELECTED, MB_CANCEL,
                                    0, NULL, EQF_ERROR, hwnd);
         fOk = FALSE;
         SETFOCUS( hwnd, DID_AN2_FOUNDTERMDIC_DL );
         }

      if (fOk)
         {
         // Search for an invalid character in New Terms filename
         pszTmp = pAIda->szCreateFndTermsDd;
         pszTmp += strcspn(pAIda->szCreateFndTermsDd, INVALID_FILE_CHARACTERS);
         if (*pszTmp) // Found an invalid character
            {
            pszTmp = pAIda->szCreateFndTermsDd;
            usResponse = UtlErrorHwnd( INVALID_CHARACTER_IN_FILENAME,
                                       MB_CANCEL,
                                       1,
                                       &pszTmp,
                                       EQF_ERROR,
                                       hwnd);
            fOk = FALSE;
            SETFOCUS( hwnd, DID_AN2_FOUNDTERMDIC_DL );
            }
         }

      if (fOk)
         {
         if (CheckIfListNameExist(pAIda->szCreateFndTermsDd, EXT_OF_FOUNDTERMS_LIST) == FALSE)
            {
            pszTmp = pAIda->szCreateFndTermsDd;
            usResponse = UtlErrorHwnd( FTL_FILE_EXISTS,
                                       MB_YESNO | MB_DEFBUTTON2,
                                       1,
                                       &pszTmp,
                                       EQF_QUERY,
                                       hwnd);
            fOk = (usResponse != MBID_NO);
            if (!fOk)
               SETFOCUS( hwnd, DID_AN2_FOUNDTERMDIC_DL );
            }
         }
   } /* endif */

    // ----- searching in dictionaries ------------
   if ( fOk &&
        ( pAIda->fCreateNewTermsCb || pAIda->fAddTermsToDicCb ||
          pAIda->fCreateFndTermsCb ) )
   {
      fOk = GetNumberOfSelectedDicts(pAIda, hwnd);
   } /* end if */

   if (fOk)
   {
      Dicts2Analysis(pAIda, ppropFolder, hwnd);
   }

   /*******************************************************************/
   /* Ensure that no copyrighted dictionaries are used as input       */
   /* for found terms lists or found term dictionaries                */
   /*******************************************************************/
   if ( fOk && (pAIda->fAddTermsToDicCb || pAIda->fCreateFndTermsCb) )
   {
     /*****************************************************************/
     /* Initialize                                                    */
     /*****************************************************************/
     hUCB = NULL;
     hDCB = NULL;
     fProtected = FALSE;

     /*****************************************************************/
     /* Start an ASD session                                          */
     /*****************************************************************/
     usRC = AsdBegin( MAX_DICTS, &hUCB );

     /*****************************************************************/
     /* Loop over all input dictionaries                              */
     /*****************************************************************/
     sIndex = 0;
     while ( fOk &&
             (usRC == LX_RC_OK_ASD) &&
             (sIndex < (SHORT)pAIda->pTAI->stInputDict.usNumber ) )
     {
       /*****************************************************************/
       /* Build dictionary property name                                */
       /*****************************************************************/
       if ( usRC == LX_RC_OK_ASD )
       {
         UtlMakeEQFPath( szPropName, NULC, PROPERTY_PATH, NULL );
         strcat( szPropName, BACKSLASH_STR );
         strcat( szPropName, pAIda->Dict.szDictName[sIndex] );
         strcat( szPropName, EXT_OF_DICTPROP );
       } /* endif */

       /*****************************************************************/
       /* Open dictionary                                               */
       /*****************************************************************/
       if ( usRC == LX_RC_OK_ASD )
       {
         pszPropName = szPropName;
         usRC = AsdOpen( hUCB,
                         0,                   // no special open style
                         1,                   // no of dictionaries
                         &pszPropName,        // ptr to array of dict properties
                         &hDCB,               // dictionary handle
                         &usErrDict           // no of dictionaries in error
                         );
       } /* endif */

       /*****************************************************************/
       /* Get access to dictionary properties                           */
       /*****************************************************************/
       if ( usRC == LX_RC_OK_ASD )
       {
         usRC = AsdRetPropPtr( hUCB, hDCB, &pDictProp );
       } /* endif */

       /*****************************************************************/
       /* Issue error message for copyrighted dictionaries              */
       /*****************************************************************/
       if ( usRC == LX_RC_OK_ASD )
       {
         if ( pDictProp->fCopyRight )
         {
            pszTmp = pAIda->Dict.szDictName[sIndex];
            usResponse = UtlErrorHwnd( ERROR_COPYRIGHTDICT_IN_INPUT,
                                       MB_CANCEL,
                                       1,
                                       &pszTmp,
                                       EQF_ERROR,
                                       hwnd);
            fOk = FALSE;
            SETFOCUS( hwnd, DID_AN2_DICTSEL_LB );
            SELECTITEM( hwnd, DID_AN2_DICTSEL_LB, sIndex );
         } /* endif */
       } /* endif */

       /***************************************************************/
       /* Close dictionary                                            */
       /***************************************************************/
       if ( hDCB )     AsdClose( hUCB, hDCB );
       hDCB = NULL;

       /***************************************************************/
       /* Next dictionary                                             */
       /***************************************************************/
       sIndex++;
     } /* endwhile */

     /*****************************************************************/
     /* Cleanup                                                       */
     /*****************************************************************/
     if ( hUCB )     AsdEnd( hUCB );
   } /* endif */

   // Ensure that output dictionary is not in the INPUT DICT list
   // NOTE: This must be called after Dict2Analysis
   if (fOk && pAIda->fAddTermsToDicCb )
   {
      if (IsDictInInputs(pAIda, pAIda->szAddTermsToDicDd ) )
      {
         pszTmp = pAIda->szAddTermsToDicDd;                     /* 1@KIT1095A */
         usResponse = UtlErrorHwnd( ERROR_OUTDICT_IN_INPUTS, MB_CANCEL,
                                    1, &pszTmp,                 /* 1@KIT1095C */
                                    EQF_ERROR, hwnd);           /* 1@KIT1095A */
         fOk = FALSE;
         SETFOCUS( hwnd, DID_AN2_ADDTERMDIC_DL );
      } /* endif */
   } /* endif */

   // Ensure that output dictionary is neither protected nor
   // copyrighted
   // Note: Access to the properties is done via AsdOpen, this
   //       is not the fasted way to access the dictionary properties
   //       but it is the only one that ensures that local properties
   //       are updated automatically if the remote properties have been
   //       changed.
   //       No error handling is done if AsdBegin or AsdOpen fails. Errors
   //       in this area will be reported later on then the dictionaries
   //       are openend for the actual processing.
   if (fOk && pAIda->fAddTermsToDicCb )
   {
     /*****************************************************************/
     /* Initialize                                                    */
     /*****************************************************************/
     hUCB = NULL;
     hDCB = NULL;
     fProtected = FALSE;

     /*****************************************************************/
     /* Start an ASD session                                          */
     /*****************************************************************/
     usRC = AsdBegin( MAX_DICTS, &hUCB );

     /*****************************************************************/
     /* Build dictionary property name                                */
     /*****************************************************************/
     if ( usRC == LX_RC_OK_ASD )
     {
       BOOL fIsNew = FALSE;
       UtlMakeEQFPath( szPropName, NULC, PROPERTY_PATH, NULL );
       strcat( szPropName, BACKSLASH_STR );
       ObjLongToShortName( pAIda->szAddTermsToDicDd, szPropName + strlen(szPropName), DICT_OBJECT, &fIsNew );
       strcat( szPropName, EXT_OF_DICTPROP );
     } /* endif */

     /*****************************************************************/
     /* Open dictionary                                               */
     /*****************************************************************/
     if ( usRC == LX_RC_OK_ASD )
     {
       pszPropName = szPropName;
       usRC = AsdOpen( hUCB,
                       0,                   // no special open style
                       1,                   // no of dictionaries
                       &pszPropName,        // ptr to array of dict properties
                       &hDCB,               // dictionary handle
                       &usErrDict           // no of dictionaries in error
                       );
     } /* endif */

     /*****************************************************************/
     /* Get access to dictionary properties                           */
     /*****************************************************************/
     if ( usRC == LX_RC_OK_ASD )
     {
       usRC = AsdRetPropPtr( hUCB, hDCB, &pDictProp );
     } /* endif */

     /*****************************************************************/
     /* Set protected flag                                            */
     /*****************************************************************/
     if ( usRC == LX_RC_OK_ASD )
     {
      fProtected = pDictProp->fProtected || pDictProp->fCopyRight;
     } /* endif */

     /*****************************************************************/
     /* Cleanup                                                       */
     /*****************************************************************/
     if ( hDCB )     AsdClose( hUCB, hDCB );
     if ( hUCB )     AsdEnd( hUCB );

     /*****************************************************************/
     /* Issue error message for protected dicts                       */
     /*****************************************************************/
     if ( fProtected )
     {
       pszPropName = pAIda->szAddTermsToDicDd;
       UtlErrorHwnd( ERROR_ADDTODICT_PROTECTED, MB_CANCEL, 1, &pszPropName,
                     EQF_ERROR, hwnd);
       fOk = FALSE;
       SETFOCUS( hwnd, DID_AN2_ADDTERMDIC_DL );
     } /* endif */
   } /* endif */

   // Ensure that EXCLUSION DICT is not in the INPUT DICT list
   // NOTE: This must be called after Dict2Analysis
   if (fOk && (pAIda->fExTermsInDicCb == TRUE))
      {
      if (IsDictInInputs(pAIda, pAIda->szExTermsInDicDd))
         {
         pszTmp = pAIda->szExTermsInDicDd;
         usResponse = UtlErrorHwnd( ERROR_EXCLUSION_IN_INPUTS, MB_CANCEL,
                                    1, &pszTmp,
                                    EQF_ERROR, hwnd);
         fOk = FALSE;
         SETFOCUS( hwnd, DID_AN2_EXTERMSINDIC_DL );
         }
      }

   // Ensure that exclusion dictionary is not the output dictionary
   if (fOk && pAIda->fAddTermsToDicCb && pAIda->fExTermsInDicCb )
   {
     if ( stricmp( pAIda->szAddTermsToDicDd, pAIda->szExTermsInDicDd) == 0 )
     {
         pszTmp = pAIda->szExTermsInDicDd;
         usResponse = UtlErrorHwnd( ERROR_EXCLUSION_IS_OUTPUT, MB_CANCEL,
                                    1, &pszTmp,
                                    EQF_ERROR, hwnd);
         fOk = FALSE;
         SETFOCUS( hwnd, DID_AN2_EXTERMSINDIC_DL );
     }
   }

   /*******************************************************************/
   /* Check if a list is selected at all                              */
   /*******************************************************************/
   if ( fOk )
   {
     if ( sMode == ANA2_STANDARD_MODE )
     {
       if ( !pAIda->fCreateNewTermsCb && !pAIda->fCreateFndTermsCb &&
            !pAIda->fAddTermsToDicCb )
       {
         UtlErrorHwnd( ERROR_NO_LIST_SELECTION, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
         fOk = FALSE;
       } /* endif */
     } /* endif */
   } /* endif */

   if (fOk)
   {
     SaveAIda2ToProp(ppropFolder, pAIda);
     SaveAIda2ToTAINPUT(pAIda->pTAI, pAIda);
   }

   if (fOk && (sMode == ANA2_STANDARD_MODE) )
   {
      //close dialog, start analysis instance
      WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT( TRUE ), NULL );
   }/*end if fOk*/

   //--- enable ok, cancel and help button more an button ---
  if ( sMode == ANA2_STANDARD_MODE )
  {
    ENABLECTRL( hwnd, PID_PB_OK     , TRUE);
    ENABLECTRL( hwnd, PID_PB_CANCEL , TRUE);
    ENABLECTRL( hwnd, PID_PB_HELP   , TRUE);
  } /* endif */

  return ( fOk );
}

/*
+-----------------------------------------------------------------------------+
 Name......: CheckIfListNameExist

 Purpose...: Save values of analysis(2) dialog to analsysis instance area.

 Parameters: 1.PSZ - pointer to filename to check if it exists
             2.PSZ - pointer to extension of the filename

 Returns...: FALSE - file exists
             TRUE  - name not found

 Sample....: CheckIfListNameExist(pAIda->szCreateNewTermsDd);
+-----------------------------------------------------------------------------+
*/
static   BOOL       CheckIfListNameExist(
                                         PSZ  pszName, // filename to check
                                         PSZ  pszExt   // extension
                                        )
{
         FILEFINDBUF FindBuf;
         BOOL       fOk = TRUE;
         CHAR       szBuf[MAX_EQF_PATH];
         HDIR       hSearch = HDIR_CREATE;
         USHORT     cb = 1;


    GETEQFLISTPATH(szBuf);              // append the filename of the list.
    strcat(szBuf, BACKSLASH_STR);
    strcat(szBuf, pszName);
    strcat(szBuf, pszExt);


    fOk = UtlFindFirst(szBuf, &hSearch, FILE_NORMAL, &FindBuf,
                       sizeof(FindBuf), &cb, 0L, FALSE);

    UtlFindClose(hSearch, FALSE);


return(fOk);
}
/*
+-----------------------------------------------------------------------------+
 Name......: SaveAnalysis2ToAIda
 Purpose...: Save values of analysis(2) dialog to analsysis instance area.

 Parameters: 1.HWND  - Handle of Analysis(2) dialog box.
             2.PAIDA - pointer to analsysis instance area structure
 Returns...: none

 Sample....: SaveAnalysis2ToAIda(hwnd, pAIda);
+-----------------------------------------------------------------------------+
*/
static   VOID       SaveAnalysis2ToAIda(HWND hwnd, PAIDA pAIda)
{
    CHAR        szCb[ANA_NO_OF_OCCURENCES+1];
    SHORT       sIndex;

/*--Controls for Output Dictionary-------------------------------------------*/

                                    // get check box state output dictionary
    pAIda->fAddTermsToDicCb   = QUERYCHECK(hwnd, DID_AN2_ADDTERM_CB);

                                    // get index of selected item in DD-List
    sIndex = CBQUERYSELECTION(hwnd, DID_AN2_ADDTERMDIC_DL);

                                    // get text of selected item in DD-List
    CBQUERYITEMTEXT(hwnd, DID_AN2_ADDTERMDIC_DL, sIndex, pAIda->szAddTermsToDicDd);

/*--Controls for New Terms List----------------------------------------------*/
                                    // get check box state for new terms list
    pAIda->fCreateNewTermsCb  = QUERYCHECK(hwnd, DID_AN2_NEWTERM_CB);

                                    // get name of new terms list
    QUERYTEXT( hwnd, DID_AN2_NEWTERMDIC_DL, pAIda->szCreateNewTermsDd);
    UtlUpper( pAIda->szCreateNewTermsDd );             //@KWT0018A

                                   // get check box state for NTL multi-term info
    pAIda->fNTLMwtermCb      = QUERYCHECK(hwnd, DID_AN2_NEWLISTINFO_MWT_CB);


    pAIda->usMWTOption      =  CBQUERYSELECTION(hwnd, DID_AN2_NEWTERMOPT_MWT_DL);


                                    // get check box state for NTL context-info
    pAIda->fNTLcontextCb      = QUERYCHECK(hwnd, DID_AN2_NEWLISTINFO_CB);

                                    // get minimum number of NTL occurrences
    if ( QUERYTEXT( hwnd, DID_AN2_NEWMINOCCUR_EF, szCb) == 0)
        pAIda->usNTLNumOccurences = 0;
    else
        pAIda->usNTLNumOccurences = (USHORT)atoi(szCb);


/*--Controls for Found Terms List--------------------------------------------*/
                                    // get check box state for found terms list
    pAIda->fCreateFndTermsCb  = QUERYCHECK(hwnd, DID_AN2_FOUNDTERM_CB);

                                    // get name of found terms list
    QUERYTEXT( hwnd, DID_AN2_FOUNDTERMDIC_DL, pAIda->szCreateFndTermsDd);
    UtlUpper( pAIda->szCreateFndTermsDd );             //@KWT0018A

                                    // get check box state for FTL context-info
    pAIda->fFTLcontextCb      = QUERYCHECK(hwnd, DID_AN2_FOUNDLISTINFO_CB);

                                    // get minimum number of FTL occurrences
    if ( QUERYTEXT( hwnd, DID_AN2_FOUNDMINOCCUR_EF, szCb) == 0)
        pAIda->usFTLNumOccurences = 0;
    else
        pAIda->usFTLNumOccurences = (USHORT)atoi(szCb);

/*--Controls for Exclusion List ---------------------------------------------*/
                                    // get check box state of exclusion list
    pAIda->fExTermsInListCb   = QUERYCHECK(hwnd, DID_AN2_EXTERMSINLIST_CB);

    if (pAIda->fExTermsInListCb)
    {
      QUERYTEXT( hwnd, DID_AN2_EXTERMSINLIST_DL, pAIda->szExTermsInListDd );
    }
    else
      *(pAIda->szExTermsInListDd) = EOS;


/*--Controls for Exclusion Dictionary ---------------------------------------*/
                                    // get check box state of exclusion dict.
    pAIda->fExTermsInDicCb    = QUERYCHECK(hwnd, DID_AN2_EXTERMSINDIC_CB);

    if (pAIda->fExTermsInDicCb)
    {
      QUERYTEXT( hwnd, DID_AN2_EXTERMSINDIC_DL, pAIda->szExTermsInDicDd);
    }
    else
        *(pAIda->szExTermsInDicDd) = EOS;

   return;

}/*SaveAnalysis2ToAIda*/

/*
+-----------------------------------------------------------------------------+
 Name......: SaveAIda2ToProp
 Purpose...: Save values of analysis(2) dialog as last used values to
             folder properties.
 Parameters: 1.PPROPFOLDER - pointer to folder properties structure
             2.PAIDA    - pointer to analsysis instance area structure
 Returns...: none

 Sample....: SaveAIda2ToProp(ppropFolder, pAIda);
+-----------------------------------------------------------------------------+
*/
static   VOID       SaveAIda2ToProp(PPROPFOLDER ppropFolder, PAIDA pAIda)
{
   // save values of output dictionary
   ppropFolder->fAddTermsToDicCb   = (EQF_BOOL)pAIda->fAddTermsToDicCb;          // CB
   strcpy(ppropFolder->szAddTermsToDicDd,  pAIda->szAddTermsToDicDd);  // DDL

   // save values of New Terms List
   ppropFolder->fCreateNewTermsCb  = (EQF_BOOL)pAIda->fCreateNewTermsCb;         // CB
   ppropFolder->fNTLMwtermCb       = (EQF_BOOL)pAIda->fNTLMwtermCb;              // CB
   ppropFolder->usMWTOption        = pAIda->usMWTOption;               // DDL
   ppropFolder->fNTLcontextCb      = (EQF_BOOL)pAIda->fNTLcontextCb;             // CB
   ppropFolder->usNTLNumOccurences = pAIda->usNTLNumOccurences;        // EF
   strcpy(ppropFolder->szCreateNewTermsDd, pAIda->szCreateNewTermsDd); // DDL

   // save values of Found Terms List
   ppropFolder->fCreateFndTermsCb  = (EQF_BOOL)pAIda->fCreateFndTermsCb;         // CB
   ppropFolder->fFTLcontextCb      = (EQF_BOOL)pAIda->fFTLcontextCb;             // CB
   ppropFolder->usFTLNumOccurences = (EQF_BOOL)pAIda->usFTLNumOccurences;        // EF
   strcpy(ppropFolder->szCreateFndTermsDd, pAIda->szCreateFndTermsDd); // DDL

   // save values of Exclusion List
   ppropFolder->fExTermsInListCb   = (EQF_BOOL)pAIda->fExTermsInListCb;          // CB
   strcpy(ppropFolder->szExTermsInListDd,  pAIda->szExTermsInListDd);  // DDL

   // save values of Exclusion Dictionary
   ppropFolder->fExTermsInDicCb    = (EQF_BOOL)pAIda->fExTermsInDicCb;           // CB
   strcpy(ppropFolder->szExTermsInDicDd,   pAIda->szExTermsInDicDd);   // DDL

   return;

}/* end of SaveAIda2ToProp */

/*
+-----------------------------------------------------------------------------+
 Name......: SaveAIda2ToTAINPUT
 Purpose...: Save values of analysis(2) dialog to the interface structure
             TAINPUT. These values are used for the terms list processing.
 Parameters: 1.PTAINPUT - pointer to text analysis input structure
             2.PAIDA    - pointer to analsysis instance area structure
 Returns...: none

 Sample....: SaveAIda2ToTAINPUT(pAIda->pTAI, pAIda);
+-----------------------------------------------------------------------------+
*/
static   VOID       SaveAIda2ToTAINPUT(PTAINPUT pTAI, PAIDA pAIda)
{

/*--Output dictionary -------------------------------------------------------*/

   if (pAIda->fAddTermsToDicCb)        // if check box is selected copy name
   {
     strcpy(pTAI->szOutDictName, pAIda->szAddTermsToDicDd );
   }
   else
   {
       *(pTAI->szOutDictName) = EOS;
   } /* endif */

/*--New Terms List ----------------------------------------------------------*/

    if (pAIda->fCreateNewTermsCb)       // if check box is selected, create
    {                                   // full path to LIST directory and
        GETEQFLISTPATH(pTAI->szNTLname);// append the filename of the list.
        strcat(pTAI->szNTLname, BACKSLASH_STR);
        strcat(pTAI->szNTLname, pAIda->szCreateNewTermsDd);
        strcat(pTAI->szNTLname, EXT_OF_NEWTERMS_LIST);
                                        // set mininum number of occurrences
        pTAI->usNTLNumOccurences = pAIda->usNTLNumOccurences;
                                        // with or without multi-term information
        pTAI->fNTLMwterm = pAIda->fNTLMwtermCb;
                                        //set option for multi term
        pTAI->usMWTOption = pAIda->usMWTOption;
                                        // with or without context information
        pTAI->fNTLcontext = pAIda->fNTLcontextCb;
    }
    else
    {
        *(pTAI->szNTLname) = EOS;       // if check box is not selected
        pTAI->usNTLNumOccurences = 0;
        pTAI->fNTLMwterm = 0;
        pTAI->fNTLcontext = 0;
    }


/*--Found Terms List --------------------------------------------------------*/

    if (pAIda->fCreateFndTermsCb)       // if check box is selected, create
    {                                   // full path to LIST directory and
        GETEQFLISTPATH(pTAI->szFTLname);// append the filename of the list.
        strcat(pTAI->szFTLname, BACKSLASH_STR);
        strcat(pTAI->szFTLname, pAIda->szCreateFndTermsDd);
        strcat(pTAI->szFTLname, EXT_OF_FOUNDTERMS_LIST);

                                        // set mininum number of occurrences
        pTAI->usFTLNumOccurences = pAIda->usFTLNumOccurences;
                                        // with or without context information
        pTAI->fFTLcontext = pAIda->fFTLcontextCb;
    }
    else
    {
        *(pTAI->szFTLname) = EOS;       // if check box is not selected
        pTAI->usFTLNumOccurences = 0;
        pTAI->fFTLcontext = 0;
    }


/*--Exclusion List ----------------------------------------------------------*/

    if (pAIda->fExTermsInListCb)        // if check box is selected, create
    {                                   // full path to the LIST dircetory.
        GETEQFLISTPATH(pTAI->szExclusionList);
        strcat(pTAI->szExclusionList, BACKSLASH_STR);
        strcat(pTAI->szExclusionList, pAIda->szExTermsInListDd);
        strcat(pTAI->szExclusionList, EXT_OF_EXCLUSION);
        if (!UtlFileExist(pTAI->szExclusionList))
        {
           *(pTAI->szExclusionList) = EOS; // set end of string
        } /* endif */
    }
    else
        *(pTAI->szExclusionList) = EOS; // else set end of string


/*--Exclusion Dictionary-----------------------------------------------------*/

    if (pAIda->fExTermsInDicCb)         // if check box is selected, create
    {                                   // full path to DICTIONARY directory
        strcpy(pTAI->szExclDictname, pAIda->szExTermsInDicDd);
    }
    else
        *(pTAI->szExclDictname) = EOS;  // else set end of string.


    return;

} /* end of SaveAIda2ToTAINPUT */

/*---------------------------------------------------------------------------+
   Name:         UpdateSliderPosition
   Purpose:      Send a message to the a process window to update it's pos.
   Parameters:   1. HWND - The process window handle
                 2. ULONG - The new slider arm position
   Returns:      VOID -
   Comments:
   Samples:      UpdateSliderPosition(pTAInput->hwndProcWin, 1L);
+---------------------------------------------------------------------------*/
VOID UpdateSliderPosition( HWND hwnd, ULONG ulLength)
{
  WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT( (SHORT)ulLength ), NULL );
}


INT_PTR CALLBACK ANALYSISPROPDLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
   LONG     lTabCtrl;
   PAIDA    pIda;                      // dialog instance data area
   BOOL     fOK = TRUE;                // internal O.K. flag

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_ANAPROP_DLG, mp2 ); break;

      case WM_INITDLG:
         // declare variables
//         PPROPSYSTEM  pSysProp;                   // ptr to EQF system properties
//         PPROPFOLDER  ppropFolder;                //pointer to folder properties
//         ULONG        ulErrorInfo;                //error indicator from PRHA
//         USHORT       usNum;                      //counter of for loop
//         BOOL         fOk = TRUE;                 //boolean flag
//         PSZ          pError;                     // pointer for eqferror
//         PSZ          pszFolderName;              //temporary pointer to folder name

        // allocate and anchor our dialog IDA
        fOK = UtlAlloc( (PVOID *) &pIda, 0L, (ULONG)sizeof( AIDA ), ERROR_STORAGE );
        if ( fOK )
        {
          fOK = ANCHORDLGIDA( hwndDlg, pIda );
          if ( !fOK )                           //no access to ida
          {
            UtlErrorHwnd( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR, hwndDlg);
          } /* endif */
        } /* endif */

        if ( fOK )
        {
          // get pointer to dialog  interface to IDA
          pIda->pA = (PANALYSIS_IDA) PVOIDFROMMP2( mp2 );
        } /* endif */

        if ( fOK )
        {
          // allocate storage for analysis interface
          fOK = UtlAlloc( (PVOID *)&pIda->pTAI, 0L, (ULONG)sizeof( TAINPUT ),
                          ERROR_STORAGE);
        } /* endif */

        if ( fOK )
        {
          ULONG        ulErrorInfo;  // error indicator from PRHA

          pIda->pTAI->pszObjList = pIda->pA->pszObjList;
          pIda->pTAI->pszActiveFolder = pIda->pA->pszObjList;

          // get dictionary specification and save it to IDA
          GETDICTIONARYSPEC( pIda );

          // get foldername and save it to ida
          strcpy( pIda->szFolderObjName, pIda->pA->szFolder );

          // correct folder object name for subfolders
          strcpy( pIda->szParentObjName, pIda->szFolderObjName );
          if ( FolIsSubFolderObject( pIda->szFolderObjName ) )
          {
            // cut off subfolder name and property path
            UtlSplitFnameFromPath( pIda->szFolderObjName );
            UtlSplitFnameFromPath( pIda->szFolderObjName );
          } /* endif */

          // split folder name without extension from folder object name
          {
            PSZ pszFolderName = UtlGetFnameFromPath( pIda->szFolderObjName );
            Utlstrccpy( pIda->szFolderName, pszFolderName, DOT );
          }

          // build name for folder properties
          {
            PPROPSYSTEM  pSysProp;     // ptr to EQF system properties

            strcpy( pIda->szFolderProp, pIda->szFolderObjName );
            pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd());
            pIda->szFolderProp[0] = pSysProp->szPrimaryDrive[0];
          }

          // open folder properties to get last used values
          if( ( pIda->hpropFolder = OpenProperties
                                        ( pIda->szFolderProp, NULL,
                                          PROP_ACCESS_READ, &ulErrorInfo))== NULL)
          {
             if ( ulErrorInfo != Err_NoStorage )
             {
                PSZ pError = pIda->szFolderProp;
                //display error message
                UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1,
                              &pError, EQF_ERROR, hwndDlg);
             } /* endif */
             // set fOK to FALSE
             fOK = FALSE;
          } /* endif */

          if (fOK)
          {
            PPROPFOLDER ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( pIda->hpropFolder );
            SavePropToTAINPUT( ppropFolder, pIda, pIda->pTAI);
            pIda->fLSCalled = 0; //if flag=0 list settings dialog was not displayed
          } /* endif */
        } /* endif fOK */

        if ( !fOK )
        {
          //--- close analysis dialog, FALSE means: - do not start analysis instance
          Close_proc( hwndDlg, FALSE );
        }
        else
        {
          AnaPropertySheetLoad( hwndDlg, pIda );
        } /* endif */
        mResult = DIALOGINITRETURN( mResult );
        break;


      case WM_COMMAND:
         mResult = AnaPropCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_NOTIFY:
         mResult = AnaPropertySheetNotification( hwndDlg, mp1, mp2 );
         break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
         EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblAnaPropDlg[0] );
         mResult = TRUE;  // message processed
         break;


      case WM_EQF_CLOSE:
         //--- get rid off dialog ---
         pIda = ACCESSDLGIDA(hwndDlg, PAIDA);
         if ( pIda )
         {
           USHORT nItem = 0;
           /***********************************************************/
           /* free all allocated pages as well as the registration    */
           /* of the modeless dialog                                  */
           /***********************************************************/
           while ( pIda->hwndPages[nItem] )
           {
             UtlUnregisterModelessDlg( pIda->hwndPages[nItem] );
             DestroyWindow( pIda->hwndPages[nItem] );
             nItem++;
           } /* endwhile */
           pIda->pA->ProcParm = pIda->pTAI;
           Iana1CleanUp( pIda, SHORT1FROMMP1( mp1 ) );
         } /* endif */
         DISMISSDLG( hwndDlg, mp1 );
         break;

      case TCM_SETCURSEL:
        {
          TC_ITEM Item;
          HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_ANA_PROP_TABCTRL );
          pIda = ACCESSDLGIDA( hwndDlg, PAIDA);
          lTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, lTabCtrl, &Item );
          lTabCtrl = Item.lParam;
          ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_HIDE );
          TabCtrl_SetCurSel( hwndTabCtrl, mp1 );
          lTabCtrl = mp1;
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, lTabCtrl, &Item );
          lTabCtrl = Item.lParam;
          ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_SHOW );
        }
        break;
      case DM_GETDEFID:
      if ( GetKeyState(VK_RETURN) & 0x8000 )
      {
		USHORT nItem = 0;
        pIda = ACCESSDLGIDA(hwndDlg, PAIDA);

        // issue command to all active dialog pages
        nItem = 0;
        while ( pIda->hwndPages[nItem] )
        {
          PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                               DWL_DLGPROC );

          switch ( nItem )
          {
            // general  settings
            case 0:
              fOK =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            DM_GETDEFID , 0L);
              break;
            case 1:    // other settings
              fOK =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            DM_GETDEFID , 0L);
              break;
 			case 2:    // autosubst
              fOK =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            DM_GETDEFID , 0L);
              break;
 			case 3:    // lists settings
              fOK =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            DM_GETDEFID , 0L);
              break;

          } /* endswitch */
          nItem++;
        } /* endwhile */
      }// end if
      mResult = TRUE;
      break;
      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of ANALYSISPROPDLGPROC */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     AnaPropertySheetLoad
//+----------------------------------------------------------------------------+
// Function call:     AnaPropertySheetLoad( hwndDlg, mp2 );
//+----------------------------------------------------------------------------+
// Description:       handle changes on the tab page
//+----------------------------------------------------------------------------+
// Parameters:        HWND hwndDlg   handle of the dialog
//                    LPARAM  mp2    message parameter 2
//+----------------------------------------------------------------------------+
// Returncode type:   MRESULT
//+----------------------------------------------------------------------------+
// Returncodes:       return code from default window proc or FALSE
//+----------------------------------------------------------------------------+
// Function flow:     create any pages,
//                    load the tabctrl text
//                    load the (modeless) dialog, register it and position into
//                      tab area
//                    return
//+----------------------------------------------------------------------------+
BOOL AnaPropertySheetLoad
(
  HWND hwndDlg,
  PAIDA     pIda
)
{
  BOOL      fOK = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  CHAR      szBuffer[80];

  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    RECT rect;

    // remember adress of user area
    hInst = GETINSTANCE( hwndDlg );
    hwndTabCtrl = GetDlgItem( hwndDlg, ID_ANA_PROP_TABCTRL );
    pIda->hwndTabCtrl = hwndTabCtrl;
    GetClientRect( hwndTabCtrl, &rect );
    TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

    // leave some additional space at top
    rect.top += 20;
    MapWindowPoints( hwndTabCtrl, hwndDlg, (POINT *) &rect, 2 );


    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // create the appropriate TAB control and load the associated dialog
    LOADSTRING( hab, hResMod, IDS_ANAPROP_TAB_GENERAL, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
      CreateDialogParam( hInst,
                         MAKEINTRESOURCE( ID_ANAPROP_GENERAL_DLG ),
                         hwndDlg,
                         ANAPROP_GENERAL_DLGPROC,
                         (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    LOADSTRING( hab, hResMod, IDS_ANAPROP_TAB_OTHER, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
      CreateDialogParam( hInst,
                         MAKEINTRESOURCE( ID_ANAPROP_OTHER_DLG ),
                         hwndDlg,
                         ANAPROP_OTHER_DLGPROC,
                         (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    LOADSTRING( hab, hResMod, IDS_ANAPROP_TAB_AUTSUBST, szBuffer );
    TabCtrlItem.pszText = szBuffer;
//    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
      CreateDialogParam( hInst,
                         MAKEINTRESOURCE( ID_ANAPROP_AUTSUBST_DLG ),
                         hwndDlg,
                         ANAPROP_AUTSUBST_DLGPROC,
                         (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    LOADSTRING( hab, hResMod, IDS_ANAPROP_TAB_LISTS, szBuffer );
    TabCtrlItem.pszText = szBuffer;
//    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
      CreateDialogParam( hInst,
                         MAKEINTRESOURCE( ID_ANAPROP_LISTS_DLG ),
                         hwndDlg,
                         ANAPROP_LISTS_DLGPROC,
                         (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;


  } /* endif */

  // hide all dialog pages but the first one
  if ( fOK )
  {
    int i = 1;
    while ( pIda->hwndPages[i] )
    {
      ShowWindow( pIda->hwndPages[i], SW_HIDE );
      i++;
    } /* endwhile */
  } /* endif */

  if ( !fOK )
  {
    POSTEQFCLOSE( hwndDlg, FALSE );
  } /* endif */

  return fOK;
}

INT_PTR CALLBACK ANAPROP_GENERAL_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PAIDA     pIda;

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PAIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwndDlg, pIda );

      SetCtrlFnt( hwndDlg, GetCharSet(), DID_AN_ANALYZE_LB, 0);

      pIda->hwndAnalyzeLb = WinWindowFromID( hwndDlg, DID_AN_ANALYZE_LB );
      {
         //get access to folder properties
         PPROPFOLDER  ppropFolder;   // pointer to folder properties

         ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pIda->hpropFolder );

         if ( pIda->pTAI->pszObjList )
         {
           PSZ pszActFol = pIda->pTAI->pszObjList;
           while ( *pszActFol )
           {
             // add folder long names our document listbox
             SubFolObjectNameToName( pszActFol, pIda->szString );
             OEMTOANSI( pIda->szString );
             INSERTITEMHWND( pIda->hwndAnalyzeLb, pIda->szString );
             pszActFol += strlen(pszActFol) + 1;
           } /* endwhile */
           SetDlgItemText( hwndDlg, DID_AN_TEXT1, "Folders to be analyzed:" );
         }
         else
         {
           // add long names of documents to our document listbox
           SHORT sIndex;                  // listbox item buffer
           SHORT sNumOfDocs;              // number of documents in listbox
           HPROP         hPropDocument;   // handle to document properties
           PPROPDOCUMENT pPropDocument;   // pointer to document properties
           ULONG         ulErrorInfo;     // error indicator from PRHA

           sIndex = 0;
           sNumOfDocs =  QUERYITEMCOUNTHWND( pIda->pA->hwndDocLB );
           while ( sIndex < sNumOfDocs )
           {
             CHAR  szDocument[MAX_FILESPEC];

             QUERYITEMTEXTHWND( pIda->pA->hwndDocLB, sIndex, szDocument );
             hPropDocument = OpenProperties( szDocument, pIda->szFolderObjName,
                                               PROP_ACCESS_READ, &ulErrorInfo );
             if ( hPropDocument )
             {
               pPropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hPropDocument );

               if ( pPropDocument->szLongName[0] != EOS )
               {
                 strcpy( pIda->szString, pPropDocument->szLongName );
                 OEMTOANSI( pIda->szString );
                 INSERTITEMHWND( pIda->hwndAnalyzeLb, pIda->szString );
               }
               else
               {
                 INSERTITEMHWND( pIda->hwndAnalyzeLb, szDocument );
               } /* endif */
               CloseProperties( hPropDocument, PROP_QUIT, &ulErrorInfo );
             } /* endif */
             sIndex ++;
           } /* endwhile */
         } /* endif */

         // activate horizontal scrolling for document listbox
         UtlSetHorzScrollingForLB(pIda->hwndAnalyzeLb);

        // set add exact matches to document last used value
        SETCHECK( hwndDlg, DID_AN_ADDTOTARGET_CB, ppropFolder->fSavedDlgIanaChkAddSeg );

        //--- select add not found matches to external file
       if ( pIda->fDisableAddNoMatchStuff )
       {
         ENABLECTRL( hwndDlg, DID_AN_ADDTONEWFILE_CB, FALSE);
         ENABLECTRL( hwndDlg, DID_AN_MACHTRANS_CB, FALSE);
         ENABLECTRL( hwndDlg, DID_AN_MATCHLIST_CB, FALSE);
       }
       else
       {
         BOOL fAllowMachTrans = FALSE;
         BOOL fNoMachTrans = TRUE;               // default = no machine translation possible

         SETCHECK( hwndDlg, DID_AN_ADDTONEWFILE_CB, ppropFolder->fSavedDlgIanaChkNoMatch );

         // check if folder target language is in list of valid MT languages
         {
           int i = 0;
           while ( !fAllowMachTrans && (apszAllowedMTTargetLang[i] != NULL) )
           {
             if ( stricmp( ppropFolder->szTargetLang, apszAllowedMTTargetLang[i] ) == 0 )
             {
               fAllowMachTrans = TRUE;
             } /* endif */
             i++;
           } /* endwhile */
         }

         // check if MQSeries or trigger file for machine translation enabling exists
         // GQ: only enable MT option when trigger file exists
         {
// define this to force the MT output in nFluent XML format (this same define must be set in EQFIANA1.C and EQFFTAPH2.C as well) 
#ifdef NFLUENT_MT_IF
           CHAR szTriggerFile[MAX_EQF_PATH];

           UtlMakeEQFPath( szTriggerFile, NULC, PROPERTY_PATH, NULL );
           strcat( szTriggerFile, "\\EQFNFLUENT.TRG" );
           if ( UtlFileExist( szTriggerFile ) )
           {
             fNoMachTrans = FALSE;
           }
           if ( fNoMachTrans  )
           {
               ENABLECTRL( hwndDlg, DID_AN_MACHTRANS_CB, FALSE );
           } /* endif */
#else
           CHAR szTriggerFile[MAX_EQF_PATH];

           UtlMakeEQFPath( szTriggerFile, NULC, PROPERTY_PATH, NULL );
           strcat( szTriggerFile, "\\EQFMTTRGF.PRP" );
           if ( UtlFileExist( szTriggerFile ) )
           {
             fNoMachTrans = FALSE;
           //}
#endif
           //else
           //{
           //  LONG lMQSAvail = UtlQueryULong( QL_MTMQSAVAILABLE );
           //  if ( lMQSAvail == 1 )
           //  {
           //    // MQS is available
           //    fNoMachTrans = FALSE;
           //  }
           //  else if ( lMQSAvail == 2 )
           //  {
           //    // no MQS available
           //    fNoMachTrans = TRUE;
           //  }
           //  else
           //  {
           //    // variable not set, try to load DLL
           //    HMODULE hmodDll = NULLHANDLE;

           //     // check if MQSeries is installed
           //     if ( DosLoadModule( NULL, 0 , "EQFMTMQS.DLL", &hmodDll) == 0 )
           //     {
           //       fNoMachTrans = FALSE;
           //       DosFreeModule( hmodDll );
           //       UtlSetULong( QL_MTMQSAVAILABLE, 1 );
           //     }
           //     else
           //     {
           //       UtlSetULong( QL_MTMQSAVAILABLE, 2 );
           //     } /* endif */
           //  } /* endif */
            } /* endif */

         }

         // check if folder source language is in list of valid MT languages
#ifdef NFLUENT_MT_IF
#else
         if ( !fNoMachTrans && fAllowMachTrans )
         {
           int i = 0;
           fAllowMachTrans = FALSE;
           while ( !fAllowMachTrans && (apszAllowedMTSourceLang[i] != NULL) )
           {
             if ( stricmp( ppropFolder->szSourceLang, apszAllowedMTSourceLang[i] ) == 0 )
             {
               fAllowMachTrans = TRUE;
             } /* endif */
             i++;
           } /* endwhile */
         }
#endif

#ifdef NFLUENT_MT_IF
#else
         // if target language is valid and only a single folder is selected...
         if ( fNoMachTrans )
         {
           ENABLECTRL( hwndDlg, DID_AN_MACHTRANS_CB, FALSE );
         }
         else if ( fAllowMachTrans && !pIda->pTAI->pszObjList )
         {
           SETCHECK( hwndDlg, DID_AN_MACHTRANS_CB, ppropFolder->fSavedDlgIanaChkMachTrans );
         }
         else
         {
           ENABLECTRL( hwndDlg, DID_AN_MACHTRANS_CB, FALSE );
         } /* endif */
       } /* endif */
#endif

        // select add segments to TM cb in dependency of last used value
        SETCHECK( hwndDlg, DID_AN_MATCHCOUNT_CB, ppropFolder->fSavedDlgIanaTMMatch );

        // select add segments to TM cb in dependency of last used value
        SETCHECK( hwndDlg, DID_AN_REDUNDCOUNT_CB, ppropFolder->fSavedDlgIanaRedundCount );

        // select match list in dependency of last used value
        SETCHECK( hwndDlg, DID_AN_MATCHLIST_CB, ppropFolder->fSavedDlgIanaMatchList );

        // select balance list in dependency of last used value
        SETCHECK( hwndDlg, DID_AN_BALANCELIST_CB, ppropFolder->fSavedDlgIanaBalanceList );

        // select xmp/screen protection
        if ( ppropFolder->fSavedDlgIanaProtXmpScreen )
        {
          // old flag was affecting all individual flags
        }
        else
        {
          SETCHECK( hwndDlg, DID_AN_PROTXMP_CB, ppropFolder->fSavedDlgIanaProtXmp );
          SETCHECK( hwndDlg, DID_AN_PROTMSGNUM_CB, ppropFolder->fSavedDlgIanaProtMsgNum );
          SETCHECK( hwndDlg, DID_AN_PROTMETA_CB, ppropFolder->fSavedDlgIanaProtMeta );
          SETCHECK( hwndDlg, DID_AN_PROTSCREEN_CB, ppropFolder->fSavedDlgIanaProtScreen );
          SETCHECK( hwndDlg, DID_AN_PROTCODEBLOCK_CB, ppropFolder->fSavedDlgIanaProtCodeBlock );
        }



        SETCHECK( hwndDlg, DID_AN_PROFILE_CHK, ppropFolder->fSavedDlgIanaProfile );
        AnaProfListProfiles( GetDlgItem( hwndDlg, DID_AN_PROFILE_CB ), TRUE );
        if ( ppropFolder->fSavedDlgIanaProfile )
        {
          SHORT sItem = CBSEARCHITEM( hwndDlg, DID_AN_PROFILE_CB, ppropFolder->szSavedDlgIanaProfile );
          if ( sItem != LIT_NONE )
          {
            CBSELECTITEM( hwndDlg, DID_AN_PROFILE_CB, sItem  );
          }
          else
          {
            ENABLECTRL( hwndDlg, DID_AN_PROFILE_PB, FALSE );
            ENABLECTRL( hwndDlg, DID_AN_PROFILE_DELETE_PB, FALSE );
          } /* endif */
        }
        else
        {
          ENABLECTRL( hwndDlg, DID_AN_PROFILE_CB, FALSE );
          ENABLECTRL( hwndDlg, DID_AN_PROFILE_PB, FALSE );
          ENABLECTRL( hwndDlg, DID_AN_PROFILE_DELETE_PB, FALSE );
        } /* endif */

      }
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case DID_AN_PROFILE_CHK:
          if ( QUERYCHECK( hwndDlg, DID_AN_PROFILE_CHK ) )
          {
            ENABLECTRL( hwndDlg, DID_AN_PROFILE_CB, TRUE );
            pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
            QUERYTEXT( hwndDlg, DID_AN_PROFILE_CB, pIda->szProfile );
            UtlStripBlanks( pIda->szProfile  );
            if ( pIda->szProfile [0] != EOS )
            {
              ENABLECTRL( hwndDlg, DID_AN_PROFILE_PB, TRUE );
              ENABLECTRL( hwndDlg, DID_AN_PROFILE_DELETE_PB, TRUE );
            }
            else
            {
              ENABLECTRL( hwndDlg, DID_AN_PROFILE_PB, FALSE );
              ENABLECTRL( hwndDlg, DID_AN_PROFILE_DELETE_PB, FALSE );
            } /* endif */
          }
          else
          {
            ENABLECTRL( hwndDlg, DID_AN_PROFILE_CB, FALSE );
            ENABLECTRL( hwndDlg, DID_AN_PROFILE_PB, FALSE );
            ENABLECTRL( hwndDlg, DID_AN_PROFILE_DELETE_PB, FALSE );
          } /* endif */
          break;

        case DID_AN_PROFILE_CB:
          switch ( WMCOMMANDCMD( mp1, mp2 ) )
          {
            case CBN_EFCHANGE :
              pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
              QUERYTEXT( hwndDlg, DID_AN_PROFILE_CB, pIda->szProfile );
              UtlStripBlanks( pIda->szProfile  );
              ENABLECTRL( hwndDlg, DID_AN_PROFILE_PB, (pIda->szProfile[0] != EOS) );
              ENABLECTRL( hwndDlg, DID_AN_PROFILE_DELETE_PB, (pIda->szProfile[0] != EOS) );
              break;

            case CBN_SELCHANGE :
              {
                SHORT  sItem;              // combobox item index
                pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
                sItem = CBQUERYSELECTION( hwndDlg, DID_AN_PROFILE_CB );
                if ( sItem != LIT_NONE )
                {
                  CBQUERYITEMTEXT( hwndDlg, DID_AN_PROFILE_CB, sItem, pIda->szProfile );
                }
                else
                {
                  pIda->szProfile[0] = EOS;
                } /* endif */
                ENABLECTRL( hwndDlg, DID_AN_PROFILE_PB, (pIda->szProfile[0] != EOS) );
                ENABLECTRL( hwndDlg, DID_AN_PROFILE_DELETE_PB, (pIda->szProfile[0] != EOS) );
              }
              break;

          } /* endswitch */
          break;

        case DID_AN_PROFILE_PB:
          pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
          QUERYTEXT( hwndDlg, DID_AN_PROFILE_CB, pIda->szProfile );
          UtlStripBlanks( pIda->szProfile  );
          if ( AnaProfileCheckName( pIda->szProfile  , hwndDlg, TRUE ) )
          {
            if ( AnaProfileDialog( pIda->szProfile, hwndDlg ) )
            {
              // add profile to combobox if is not already there
              SHORT sItem = CBSEARCHITEM( hwndDlg, DID_AN_PROFILE_CB, pIda->szProfile );
              if ( sItem == LIT_NONE )
              {
                CBINSERTITEM( hwndDlg, DID_AN_PROFILE_CB, pIda->szProfile );
              } /* endif */\
            } /* endif */
          } /* endif */
          break;

        case DID_AN_PROFILE_DELETE_PB:
          pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
          QUERYTEXT( hwndDlg, DID_AN_PROFILE_CB, pIda->szProfile );
          UtlStripBlanks( pIda->szProfile  );
          if ( AnaProfileCheckName( pIda->szProfile, hwndDlg, TRUE ) )
          {
            PSZ pszParm = pIda->szProfile;
            USHORT usReply = UtlErrorHwnd( WARNING_DELETE_PROFILE, MB_OKCANCEL | MB_DEFBUTTON2, 1, &pszParm, EQF_QUERY, hwndDlg );

            if ( usReply == MBID_OK )
            {
              // delete profile
              AnaProfBuildFullProfileName( pIda->szProfile, pIda->szEqfPath );
              UtlDelete( pIda->szEqfPath, 0L, FALSE );

              // remove deleted profile from combobox
              {
                SHORT sItem = CBSEARCHITEM( hwndDlg, DID_AN_PROFILE_CB, pIda->szProfile );
                if ( sItem != LIT_NONE )
                {
                  SendDlgItemMessage( hwndDlg, DID_AN_PROFILE_CB, CB_DELETESTRING, sItem, 0L );
                } /* endif */
                SETTEXT( hwndDlg, DID_AN_PROFILE_CB, "" );
                ENABLECTRL( hwndDlg, DID_AN_PROFILE_PB, FALSE );
                ENABLECTRL( hwndDlg, DID_AN_PROFILE_DELETE_PB, FALSE  );
              }
            } /* endif */
          } /* endif */
          break;

        case DID_AN_ADDTOTARGET_CB:
          {
            pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
             pIda->fAutSubst      = (BOOL) QUERYCHECK( hwndDlg, DID_AN_ADDTOTARGET_CB );

            if ( pIda->fAutSubst )
            {
              // add autsubst settings tab
              HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
              TC_ITEM   TabCtrlItem;
              LONG      nItem = 0;
              CHAR      szBuffer[80];

              TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TAB_AUTSUBST, szBuffer );
              TabCtrlItem.pszText = szBuffer;
              TabCtrlItem.lParam  = 2;
              nItem = TabCtrl_GetItemCount( pIda->hwndTabCtrl );
              SendMessage( pIda-> hwndTabCtrl, TCM_INSERTITEM, nItem,
                           (LPARAM)&TabCtrlItem);
            }
            else
            {
              // remove autsubst settings tab
              TC_ITEM   TabCtrlItem;
              LONG      nItem = 0;
              LONG      lI;

              nItem = TabCtrl_GetItemCount( pIda->hwndTabCtrl );
              memset( &TabCtrlItem, 0, sizeof(TabCtrlItem) );
              TabCtrlItem.mask = TCIF_PARAM;
              lI = 0;
              while ( lI < nItem )
              {
                TabCtrl_GetItem( pIda->hwndTabCtrl, lI, &TabCtrlItem );
                if ( TabCtrlItem.lParam == 2 )
                {
                  TabCtrl_DeleteItem( pIda->hwndTabCtrl, lI );
                  lI = nItem + 1;     // force end of loop
                } /* endif */
                lI++;
              } /* endwhile */
            } /* endif */
          }
          break;

        case DID_AN_LISTSETTING_CB:
          {
            pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
            pIda->fListCreation  = (BOOL) QUERYCHECK( hwndDlg, DID_AN_LISTSETTING_CB );

            if ( pIda->fListCreation )
            {
              // add list settings tab
              HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
              TC_ITEM   TabCtrlItem;
              LONG      nItem = 0;
              CHAR      szBuffer[80];

              TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TAB_LISTS, szBuffer );
              TabCtrlItem.pszText = szBuffer;
              TabCtrlItem.lParam  = 3;
              nItem = TabCtrl_GetItemCount( pIda->hwndTabCtrl );
              SendMessage( pIda-> hwndTabCtrl, TCM_INSERTITEM, nItem,
                           (LPARAM)&TabCtrlItem);
            }
            else
            {
              // remove list settings tab
              TC_ITEM   TabCtrlItem;
              LONG      nItem = 0;
              LONG    lI;

              nItem = TabCtrl_GetItemCount( pIda->hwndTabCtrl );
              memset( &TabCtrlItem, 0, sizeof(TabCtrlItem) );
              TabCtrlItem.mask = TCIF_PARAM;
              lI = 0;
              while ( lI < nItem )
              {
                TabCtrl_GetItem( pIda->hwndTabCtrl, lI, &TabCtrlItem );
                if ( TabCtrlItem.lParam == 3 )
                {
                  TabCtrl_DeleteItem( pIda->hwndTabCtrl, lI );
                  lI = nItem + 1;     // force end of loop
                } /* endif */
                lI++;
              } /* endwhile */
            } /* endif */
          }
          break;

        case PID_PB_OK:
           /****************************************************************/
           /* if mp2 == 1L we have to validate the page, if it is 0L we    */
           /* have to copy the content of the dialog back into the struct. */
           /****************************************************************/
           if ( mp2 == 1L )
           {
             /**************************************************************/
             /* no checking for correct input necessary                    */
             /**************************************************************/
           }
           else
           {
             /**************************************************************/
             /* get the active settings ....                               */
             /**************************************************************/
             pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
             pIda->fListCreation  = (BOOL) QUERYCHECK( hwndDlg, DID_AN_LISTSETTING_CB );
             pIda->fAutSubst      = (BOOL) QUERYCHECK( hwndDlg, DID_AN_ADDTOTARGET_CB );
             pIda->fMatchCount    = (BOOL) QUERYCHECK( hwndDlg, DID_AN_MATCHCOUNT_CB );
             pIda->fRedundancyCounting = (BOOL) QUERYCHECK( hwndDlg, DID_AN_REDUNDCOUNT_CB );
             pIda->fSNOMATCH = (BOOL)QUERYCHECK( hwndDlg, DID_AN_ADDTONEWFILE_CB);
             pIda->fMachTrans = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_MACHTRANS_CB);
             pIda->fMatchList = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_MATCHLIST_CB);
             pIda->fBalanceList = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_BALANCELIST_CB);
             pIda->fProtXmp = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_PROTXMP_CB);
             pIda->fProtMsgNum = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_PROTMSGNUM_CB);
             pIda->fProtMeta = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_PROTMETA_CB);
             pIda->fProtScreen = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_PROTSCREEN_CB);
             pIda->fProtCodeBlock = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_PROTCODEBLOCK_CB);


             pIda->fProfile = (EQF_BOOL)QUERYCHECK( hwndDlg, DID_AN_PROFILE_CHK);
             if ( pIda->fProfile )
             {
               QUERYTEXT( hwndDlg, DID_AN_PROFILE_CB, pIda->szProfile );
             }
             else
             {
               pIda->szProfile[0] = EOS;
             } /* endif */

             // show confidential warning
#ifdef NFLUENT_MT_IF
#else
             if ( pIda->fMachTrans )
             {
               int iRC = MessageBox( hwndDlg, "Machine Translation must not be used for translating confidential information.\n\nPress the Cancel button, if your material is classified as \"IBM Confidential\".\nThis deselects the MT option.\n\nPress the OK button to continue translating with unclassified data.",
                                     "Warning",  MB_OKCANCEL | MB_DEFBUTTON2 );
               if ( iRC == MBID_CANCEL )
               {
                 SETCHECK_FALSE( hwndDlg, DID_AN_MACHTRANS_CB );
                 pIda->fMachTrans = FALSE;
               } /* endif */
             } /* endif */
#endif
           } /* endif */
           break;
      } /* endswitch */
      break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
         EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                &hlpsubtblAnaPropGeneral[0] );
         mResult = TRUE;  // message processed
         break;

    case WM_CLOSE:
       DelCtrlFont( hwndDlg, DID_AN_ANALYZE_LB );
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};

INT_PTR CALLBACK ANAPROP_AUTSUBST_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PAIDA     pIda;

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PAIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwndDlg, pIda );
      {
         //get access to folder properties
         PPROPFOLDER  ppropFolder;   // pointer to folder properties

         ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( pIda->hpropFolder );

         SETCHECK( hwndDlg, DID_AN_ADDLATESTMATCH_CB, ppropFolder->fUsedLatestExactMatch );
         SETCHECK( hwndDlg, DID_AN_AUTOJOIN_CB, ppropFolder->fAnalAutoJoin );
         SETCHECK( hwndDlg, DID_AN_EXACTCONTEXT_CB, ppropFolder->fExactContextTMMatch );
         SETCHECK( hwndDlg, DID_AN_IGNORE_IDENTICAL_CB, ppropFolder->fAnalIgnoreIdentical );
         SETCHECK( hwndDlg, DID_AN_IGNORE_PATH_CB, ppropFolder->fAnalIgnorePath );
         SETCHECK( hwndDlg,  DID_AN_LEADINGWS_CB, ppropFolder->fAnalLeadingWS );
         SETCHECK( hwndDlg, DID_AN_TRAILINGWS_CB, ppropFolder->fAnalTrailingWS );
         SETCHECK( hwndDlg, DID_AN_RESPECTCRLF_CB, ppropFolder->fAnalRespectCRLF );
         SETCHECK( hwndDlg, DID_AN_NOADDBLANK_CB, ppropFolder->fAnalNoAddBlank );
         SETCHECK( hwndDlg, DID_AN_IGNORE_COMMENT_CB, ppropFolder->fAnalIgnoreComment );

         // handling for replacement list fields
         if ( UtlQueryUShort( QS_SGMLDITAPROCESSING ) )
         {
           SHOWCONTROL( hwndDlg, DID_AN_REPLLIST_GB );
           SHOWCONTROL( hwndDlg, DID_AN_REPLLIST_CHK );
           SHOWCONTROL( hwndDlg, DID_AN_REPLLIST_TEXT );
           SHOWCONTROL( hwndDlg, DID_AN_REPLLIST_EF );
           SHOWCONTROL( hwndDlg, DID_AN_REPLLIST_BROWSE_PB );
           SETCHECK( hwndDlg, DID_AN_REPLLIST_CHK, ppropFolder->fAnalUseReplacementList );
           SETTEXTLIMIT( hwndDlg, DID_AN_REPLLIST_EF, sizeof(ppropFolder->szAnalReplacementList)-1 );
           SETTEXT( hwndDlg, DID_AN_REPLLIST_EF, ppropFolder->szAnalReplacementList );
           ENABLECTRL( hwndDlg, DID_AN_REPLLIST_TEXT, ppropFolder->fAnalUseReplacementList );
           ENABLECTRL( hwndDlg, DID_AN_REPLLIST_EF, ppropFolder->fAnalUseReplacementList );
           ENABLECTRL( hwndDlg, DID_AN_REPLLIST_BROWSE_PB, ppropFolder->fAnalUseReplacementList );
         }
         else
         {
           HIDECONTROL( hwndDlg, DID_AN_REPLLIST_GB );
           HIDECONTROL( hwndDlg, DID_AN_REPLLIST_CHK );
           HIDECONTROL( hwndDlg, DID_AN_REPLLIST_TEXT );
           HIDECONTROL( hwndDlg, DID_AN_REPLLIST_EF );
           HIDECONTROL( hwndDlg, DID_AN_REPLLIST_BROWSE_PB );
         } /* endif */
      }
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case DID_AN_REPLLIST_BROWSE_PB:
          {
            CHAR szList[MAX_LONGFILESPEC];
            OPENFILENAME OpenStruct;

            memset( &OpenStruct, 0, sizeof(OpenStruct) );
            OpenStruct.lStructSize = sizeof(OpenStruct);
            OpenStruct.hwndOwner = hwndDlg;
            QUERYTEXT( hwndDlg, DID_AN_REPLLIST_EF, szList );
            OpenStruct.lpstrFile = szList;
            OpenStruct.nMaxFile = sizeof(szList)-1;
            OpenStruct.lpstrFileTitle = NULL;
            OpenStruct.nMaxFileTitle = 0;
            OpenStruct.lpstrTitle = "Select a replacement list";
            OpenStruct.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
            if ( GetOpenFileName( &OpenStruct ) )
            {
              SETTEXT( hwndDlg, DID_AN_REPLLIST_EF, szList );
            } /* endif */
          }
          break;

        case DID_AN_REPLLIST_CHK:
          {
            BOOL fEnable = QUERYCHECK( hwndDlg, DID_AN_REPLLIST_CHK );
            ENABLECTRL( hwndDlg, DID_AN_REPLLIST_TEXT, fEnable );
            ENABLECTRL( hwndDlg, DID_AN_REPLLIST_EF, fEnable );
            ENABLECTRL( hwndDlg, DID_AN_REPLLIST_BROWSE_PB, fEnable );
          }
          break;

        case PID_PB_OK:
            pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
            pIda->fLatestMatch = QUERYCHECK( hwndDlg, DID_AN_ADDLATESTMATCH_CB );
            pIda->fAutoJoin = QUERYCHECK( hwndDlg, DID_AN_AUTOJOIN_CB );
            pIda->fExactContext = QUERYCHECK( hwndDlg, DID_AN_EXACTCONTEXT_CB );
            pIda->fIgnoreIdentical = QUERYCHECK( hwndDlg, DID_AN_IGNORE_IDENTICAL_CB );
            pIda->fIgnoreComment = QUERYCHECK( hwndDlg, DID_AN_IGNORE_COMMENT_CB );
            pIda->fIgnorePath = QUERYCHECK( hwndDlg, DID_AN_IGNORE_PATH_CB );
            pIda->fLeadingWS = QUERYCHECK( hwndDlg, DID_AN_LEADINGWS_CB );
            pIda->fTrailingWS = QUERYCHECK( hwndDlg, DID_AN_TRAILINGWS_CB );
            pIda->fRespectCRLF = QUERYCHECK( hwndDlg, DID_AN_RESPECTCRLF_CB );
            pIda->fNoAddBlank = QUERYCHECK( hwndDlg, DID_AN_NOADDBLANK_CB );
            pIda->fUseReplacementList = QUERYCHECK( hwndDlg, DID_AN_REPLLIST_CHK ); 
            QUERYTEXT( hwndDlg, DID_AN_REPLLIST_EF, pIda->szReplacementList ); 
            if ( pIda->fUseReplacementList && (pIda->szReplacementList[0] == 0) )
            {
              UtlErrorHwnd( ERROR_NO_REPLLIST_SELECTED, MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
              SETFOCUS( hwndDlg, DID_AN_REPLLIST_EF );
              mResult = TRUE;
            } /* endif */
            break;
      } /* endswitch */
      break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
         EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                &hlpsubtblAnaPropAutoSubst[0] );
         mResult = TRUE;  // message processed
         break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};

INT_PTR CALLBACK ANAPROP_LISTS_DLGPROC
(
   HWND hwnd,                          // handle of dialog window
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  PAIDA     pIda;                      // ptr to instance data area
  MRESULT  mResult = MRFROMSHORT( FALSE ); // result value of procedure


  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PAIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwnd, pIda );

      // get handles of our listboxes
      pIda->hwndDicAvaLb = WinWindowFromID( hwnd, DID_AN2_DICTAVA_LB);
      pIda->hwndDicSelLb = WinWindowFromID( hwnd, DID_AN2_DICTSEL_LB);

      // subclass entryfields to allow numerics only

      // set colors of numeric entry-fields
      SETCOLOR( hwnd, DID_AN2_NEWMINOCCUR_TXT, CLR_BLACK );
      SETCOLOR( hwnd, DID_AN2_FOUNDMINOCCUR_TXT, CLR_BLACK );

      // fill-in last used values
      {
        //get access to folder properties
        PPROPFOLDER  ppropFolder;   // pointer to folder properties

        ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pIda->hpropFolder );

        GetLastUsedAnalysis2Values( pIda, ppropFolder);

        FillDropDownAddDict( hwnd, pIda);
        FillDropDownExclDict( hwnd, pIda);

        SETCHECK( hwnd, DID_AN2_ADDTERM_CB, pIda->fAddTermsToDicCb );
        ENABLECTRL( hwnd, DID_AN2_ADDTERMDIC_DL, pIda->fAddTermsToDicCb);

        SetValuesNewTermsList( hwnd,   pIda );
        SetValuesFoundTermsList( hwnd, pIda );
        SetValuesExclusionList( hwnd, pIda );

        SETCHECK( hwnd, DID_AN2_EXTERMSINDIC_CB, pIda->fExTermsInDicCb );
        ENABLECTRL( hwnd, DID_AN2_EXTERMSINDIC_DL, pIda->fExTermsInDicCb );

        // get all available dict. and display them in available dict. LB
        EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES,
                         MP1FROMHWND(pIda->hwndDicAvaLb), 0L);

        //fill selected dictionaries LB with last used values
        {
          USHORT usNum;

          for ( usNum = 0; usNum < SAVE_DIC_NUM; usNum++ )
          {
             InsertItemToLb1IfExistInLb2( pIda->hwndDicSelLb,
                                          pIda->hwndDicAvaLb,
                                          ppropFolder->szSavedDlgIanaInDic[usNum]);
          }
        }
      } /* end of fill-i last used values */

      pIda->fLSCalled = 1; // if flag=1 list settings dialog was displayed
      break;

    case WM_COMMAND:
      {
		 SHORT sId = WMCOMMANDID( mp1, mp2 );
        SHORT sNotification = WMCOMMANDCMD( mp1, mp2 );
        switch ( sId )
        {
		  case DM_GETDEFID:
		  	/************************************************************/
			/* check if user pressed the ENTER key, but wants only to   */
			/* select/deselect an item of the listbox via a simulated   */
			/* (keystroke) double click.                                */
			/************************************************************/
		  	  {
		  		HWND   hwndFocus;          // handle of focus window

		  		if ( GetKeyState(VK_RETURN) & 0x8000 )
		  		{
		  		  hwndFocus = GetFocus();

		  		  if ( hwndFocus == GetDlgItem( hwnd, DID_AN2_DICTAVA_LB ) )
		  		  {
					   // add to selected or issuer error message
					   SHORT sItems = QUERYITEMCOUNT( hwnd, DID_AN2_DICTSEL_LB );
					   if ( sItems >= NUM_OF_FOLDER_DICS )
					   {
						 UtlErrorHwnd( ERROR_MAX_DICTS, MB_CANCEL,
									   0, NULL, EQF_ERROR, hwnd);
					   }
					   else
					   {
						 pIda = ACCESSDLGIDA( hwnd, PAIDA );
						 FillSelectedLb( pIda->hwndDicSelLb, pIda->hwndDicAvaLb,
										 INFO_DIC_SELECTED );
					   } /* endif */
			       }
			       else if ( hwndFocus == GetDlgItem( hwnd, DID_AN2_DICTSEL_LB))
			       {
					   pIda = ACCESSDLGIDA( hwnd, PAIDA );
					   DELETESELECTEDLBITEM( pIda->hwndDicSelLb );
		           } /* endif */
		         } /* endif */

		  	    mResult = TRUE;
		      }
			  break;
          case PID_PB_OK:
             {
               BOOL fOK;
               PPROPFOLDER  ppropFolder;   // pointer to folder properties

               pIda = ACCESSDLGIDA( hwnd, PAIDA );
               ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( pIda->hpropFolder );

               /****************************************************************/
               /* if mp2 == 1L we have to validate the page, if it is 0L we    */
               /* have to copy the content of the dialog back into the struct. */
               /****************************************************************/
               if ( mp2 == 1L )
               {
                 fOK = HandleOkButtonAna2( hwnd , pIda, ppropFolder,
                                           ANA2_PAGESWITCH_MODE );
               }
               else
               {
                 fOK = HandleOkButtonAna2( hwnd , pIda, ppropFolder,
                                           ANA2_FINALCHECK_MODE );
               } /* endif */
               mResult = !fOK;
             }
             break;

      case DID_AN2_NEWTERMOPT_MWT_DL:
        //---------------------------------------------------------
        if (sNotification == CBN_SELCHANGE)
        {
          // get access to ida
          pIda = ACCESSDLGIDA (hwnd, PAIDA);

        } // end if
        break;

          case DID_AN2_DICTAVA_LB:          //available dictionaries LB
             pIda = ACCESSDLGIDA( hwnd, PAIDA );

             if ( sNotification == LN_ENTER )  //item is selected
             {
               // add to selected or issuer error message
               SHORT sItems = QUERYITEMCOUNT( hwnd, DID_AN2_DICTSEL_LB );
               if ( sItems >= NUM_OF_FOLDER_DICS )
               {
                 UtlErrorHwnd( ERROR_MAX_DICTS, MB_CANCEL,
                               0, NULL, EQF_ERROR, hwnd);
               }
               else
               {
                 FillSelectedLb( pIda->hwndDicSelLb, pIda->hwndDicAvaLb,
                                 INFO_DIC_SELECTED );
               } /* endif */
             } /* endif */
             break;

          case DID_AN2_DICTSEL_LB:        //selection in selected dictionay listbox
             pIda = ACCESSDLGIDA( hwnd, PAIDA );

             if ( sNotification == LN_ENTER )
             {
                DELETESELECTEDLBITEM( pIda->hwndDicSelLb );
             } /* end if */
             break;

          case DID_AN2_ADDTERM_CB:
             if ( sNotification == BN_CLICKED )
             {
               // disable available and selected mdb listboxes and texts
               // in dependency of check state
               ENABLECTRL( hwnd, DID_AN2_ADDTERMDIC_DL, QUERYCHECK(hwnd, DID_AN2_ADDTERM_CB));
             } /* endif */
             break;

          case DID_AN2_NEWTERM_CB:
             if ( sNotification == BN_CLICKED )
             {
               // disable corresponding text in dependency of check state
               EnableDisableNewTermsList( hwnd,
                 QUERYCHECK( hwnd, DID_AN2_NEWTERM_CB) );
             }/* endif */
             break;

          //----------------------------------------------------------
          case DID_AN2_FOUNDTERM_CB:
             if ( sNotification == BN_CLICKED )
             {
               // disable corresponding text in dependency of check state
               EnableDisableFoundTermsList( hwnd,
                 QUERYCHECK( hwnd, DID_AN2_FOUNDTERM_CB ) );
             } /* endif */

             break;

          case DID_AN2_EXTERMSINLIST_CB:
             if ( sNotification == BN_CLICKED)
             {
                // disable available and selected mdb listboxes and texts
                // in dependency of check state
                ENABLECTRL( hwnd, DID_AN2_EXTERMSINLIST_DL,
                            QUERYCHECK(hwnd, DID_AN2_EXTERMSINLIST_CB));
             } /* endif */
             break;

          case DID_AN2_EXTERMSINDIC_CB:
             if ( sNotification == BN_CLICKED)
             {
               // disable available and selected mdb listboxes and texts
               // in dependency of check state
               ENABLECTRL( hwnd, DID_AN2_EXTERMSINDIC_DL,
                           QUERYCHECK(hwnd, DID_AN2_EXTERMSINDIC_CB));
             } /* endif */
             break;
        } /* endswitch */
      }
      break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
         EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                                &hlpsubtblAnaPropList[0] );
         mResult = TRUE;  // message processed
         break;

    default:
       mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return( mResult );
};

INT_PTR CALLBACK ANAPROP_OTHER_DLGPROC
(
   HWND hwnd,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PAIDA     pIda;

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PAIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwnd, pIda );
      pIda->hwndMdbAvaLb  = WinWindowFromID( hwnd, DID_AN_MDBAVA_LB );
      pIda->hwndMdbSelLb  = WinWindowFromID( hwnd, DID_AN_MDBSEL_LB );

      {
        USHORT usNum;
        PPROPFOLDER  ppropFolder;   // pointer to folder properties

        ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( pIda->hpropFolder );

        //--- get available TM's and display them in available TM LB
        EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES,
                         MP1FROMHWND( pIda->hwndMdbAvaLb ),
                         MP2FROMP( MEMORY_ALL ) );

        //--- check if available TM LB is empty
        usNum = QUERYITEMCOUNT( hwnd, DID_AN_MDBAVA_LB );
        if ( usNum == 0 )                     //no item exists in TM available LB
        {
          pIda->fDisableTMStuff = TRUE;
        }
        else                                  //item exists in TM available LB
        {
          // fill selected memory db LB with selected mdbs from folder properties
          if ( ppropFolder->szSavedDlgInMdb[0][0] != EOS )
          {
            for ( usNum = 0; usNum < SAVE_MDB_NUM; usNum++ )
            {
              InsertItemToLb1IfExistInLb2( pIda->hwndMdbSelLb, pIda->hwndMdbAvaLb,
                                           ppropFolder->szSavedDlgInMdb[usNum] );
            }/* endfor */
          }
          else
          {
            for ( usNum = 0; usNum < SAVE_MDB_NUM; usNum++ )
            {
              InsertItemToLb1IfExistInLb2( pIda->hwndMdbSelLb, pIda->hwndMdbAvaLb,
                                           ppropFolder->szSavedDlgIanaInMdb[usNum] );
            }/* endfor */
          } /* endif */
        } /* endif */

      // hide balance list checkbox if there is no balance directory
      {
        CHAR szBalanceDir[MAX_EQF_PATH];

        UtlMakeEQFPath( szBalanceDir, NULC, SYSTEM_PATH, NULL );
        strcat( szBalanceDir, "\\LOADBALANCE" );
        if ( !UtlDirExist( szBalanceDir ) )
        {
          HIDECONTROL( hwnd, DID_AN_BALANCELIST_CB );
        } /* endif */
      }

        // allow horizonal scrolling in memory listboxes
        UtlSetHorzScrollingForLB(pIda->hwndMdbAvaLb);
        UtlSetHorzScrollingForLB(pIda->hwndMdbSelLb);

        //select add segments to TM cb in dependency of last used value
        SETCHECK( hwnd, DID_AN_ADDTOMEMORY_CB, ppropFolder->fSavedDlgIanaChkMdb );

        // set Start at first... checkbox
        SETCHECK( hwnd, DID_AN_STOPATFIRSTEXACT_CB, ppropFolder->fSavedDlgIanaStopAtFirstExact );

        // enable/disable MDB listboxes in dependency of last used value
        // of select add segments.... checkbox
        EnableDisableTMListBoxes( hwnd, ppropFolder->fSavedDlgIanaChkMdb );
      }

      if ( pIda->fDisableTMStuff )          //TM stuff should be disabled
      {
        // disable add segments to TM CB and the corresponding LB's
        // disable add segments to TM CB and the corresponding LB's
        ENABLECTRL( hwnd, DID_AN_ADDTOMEMORY_CB, FALSE );
        EnableDisableTMListBoxes( hwnd, FALSE );
      } /* endif */


      break;

    case WM_COMMAND:
      {
        SHORT sId = WMCOMMANDID( mp1, mp2 );
        SHORT sNotification = WMCOMMANDCMD( mp1, mp2 );

        switch ( sId )
        {
		  case DM_GETDEFID:
		  	/************************************************************/
			/* check if user pressed the ENTER key, but wants only to   */
			/* select/deselect an item of the listbox via a simulated   */
			/* (keystroke) double click.                                */
			/************************************************************/
			  {
				HWND   hwndFocus;          // handle of focus window

				if ( GetKeyState(VK_RETURN) & 0x8000 )
				{
				  hwndFocus = GetFocus();

				  if ( hwndFocus == GetDlgItem( hwnd, DID_AN_MDBAVA_LB ) )
				  {
					   // add to selected or issuer error message
					   SHORT sItems = QUERYITEMCOUNT( hwnd, DID_AN_MDBSEL_LB );
					   if ( sItems >= NUM_OF_FOLDER_DICS )
					   {
						 UtlErrorHwnd( ERROR_MAX_DICTS, MB_CANCEL,
									   0, NULL, EQF_ERROR, hwnd);
					   }
					   else
					   {
						 pIda = ACCESSDLGIDA( hwnd, PAIDA );
						 //fill selected input memory db LB
						   FillSelectedLb( pIda->hwndMdbSelLb, pIda->hwndMdbAvaLb,
										   INFO_MEM_SELECTED );

						   // allow horizonal scrolling in memory listboxe
                          UtlSetHorzScrollingForLB(pIda->hwndMdbSelLb);
					   } /* endif */
				   }
				   else if ( hwndFocus == GetDlgItem( hwnd, DID_AN_MDBSEL_LB))
				   {
					   pIda = ACCESSDLGIDA( hwnd, PAIDA );
					   DELETESELECTEDLBITEM( pIda->hwndMdbSelLb );
				   } /* endif */
				 } /* endif */

				mResult = TRUE;
			  }
			  break;

          case PID_PB_OK:
            {
             BOOL fOK;

             pIda = ACCESSDLGIDA( hwnd, PAIDA );
             pIda->fAddToMem  = (BOOL) QUERYCHECK( hwnd, DID_AN_ADDTOMEMORY_CB );
             pIda->fStopAtFirstExact = (EQF_BOOL)QUERYCHECK( hwnd, DID_AN_STOPATFIRSTEXACT_CB );

             pIda->pTAI->fInsertToTM = pIda->fAddToMem;
             if ( pIda->fAddToMem )
             {
               //--- get number of selected mdbs, if none selected and the add segments
               //--- to TM cb is selected display message, that no TM is selected
               fOK = GetNumberOfSelectedMdbs( pIda, hwnd );
               mResult = !fOK;
             } /* endif */

             /****************************************************************/
             /* if mp2 == 1L we have to validate the page, if it is 0L we    */
             /* have to copy the content of the dialog back into the struct. */
             /****************************************************************/
             if ( mp2 == 1L )
             {
             }
             else
             {
               //get selected memory dbs
               SHORT sIndex;

               if ( pIda->pTAI->stInputMemDb.usNumber > SAVE_MDB_NUM )
               {
                 pIda->pTAI->stInputMemDb.usNumber = SAVE_MDB_NUM;
               } /* endif */

               for ( sIndex = 0; sIndex < (SHORT)pIda->pTAI->stInputMemDb.usNumber;
                     sIndex++ )
               {
                  QUERYITEMTEXT( hwnd, DID_AN_MDBSEL_LB, sIndex,
                                 pIda->szSelectedMems[sIndex] );
                  ANSITOOEM( pIda->szSelectedMems[sIndex] );
               }/*end for*/

             } /* endif */
            }
            break;

          case ( DID_AN_MDBAVA_LB ) :              //available memory db LB
             pIda = ACCESSDLGIDA( hwnd, PAIDA );

             if ( sNotification == LN_ENTER )  //item is selected
             {
                // If there are NUM_OF_FOLDER_DICS selected already
                SHORT sItems = QUERYITEMCOUNT( hwnd, DID_AN_MDBSEL_LB );
                if ( sItems >= NUM_OF_FOLDER_DICS )
                  // Display ERROR_MAX_DICTS
                  UtlErrorHwnd( ERROR_MAX_DICTS, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                else
                  //fill selected input memory db LB
                  FillSelectedLb( pIda->hwndMdbSelLb, pIda->hwndMdbAvaLb,
                                  INFO_MEM_SELECTED );

                  // allow horizonal scrolling in memory listboxe
                  UtlSetHorzScrollingForLB(pIda->hwndMdbSelLb);
             } /* endif */
             break;

          case ( DID_AN_MDBSEL_LB ) :   //selection in selected memory db listbox
             pIda = ACCESSDLGIDA( hwnd, PAIDA );

             if ( sNotification == LN_ENTER )  //item is selected
             {
                //delete selected item in memory db selected LB
                DELETESELECTEDLBITEM( pIda->hwndMdbSelLb );
             }/*end if*/
             break;

          case ( DID_AN_ADDTOMEMORY_CB  ) :   //add segments to tm CB is selected
             if ( sNotification == BN_CLICKED )  //cb is selected
             {
                //  disable available and selected mdb listboxes and texts
                // in dependency of check state
                EnableDisableTMListBoxes( hwnd, QUERYCHECK( hwnd, DID_AN_ADDTOMEMORY_CB ) );
             } /* endif */
             break;
        } /* endswitch */
      }
      break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
         EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                                &hlpsubtblAnaPropMisc[0] );
         mResult = TRUE;  // message processed
         break;

    default:
       mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     AnaPropCommand
//+----------------------------------------------------------------------------+
// Function call:     AnaPropCommand( hwndDlg, mp1, mp2);
//+----------------------------------------------------------------------------+
// Description:       Handle WM_COMMAND message of property sheet dialog
//+----------------------------------------------------------------------------+
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//+----------------------------------------------------------------------------+
// Returncode type:   MRESULT
//+----------------------------------------------------------------------------+
// Returncodes:       return code from default window proc or FALSE
//+----------------------------------------------------------------------------+
MRESULT AnaPropCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
   PAIDA pIda;                         // ptr to dialog IDA
   BOOL fOK = TRUE;

   mp2;

   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
	  case PID_PB_HELP:
	    mResult = UtlInvokeHelp();
	    break;
      case PID_PB_OK:
        // get the active settings and prepare analysis.
        {
//   USHORT  usBufferSize;               //storage size  fo IF buffer
//   ULONG   ulErrorInfo;                //error indicator from PRHA
//   BOOL    fOk = TRUE;                 //boolean flag
//   PSZ     pszTemp;                    //temporary used string pointer
//   SHORT   sIndex;
//   USHORT  usResponse;
          USHORT nItem = 0;
          PPROPFOLDER  ppropFolder;    // pointer to folder properties
          HWND hwndSavedDocsLB = WinCreateWindow( hwndDlg, WC_LISTBOX, "",
                                        WS_CHILDWINDOW | LBS_STANDARD,
                                        0, 0, 10, 10, hwndDlg,
                                        HWND_TOP, 4711, NULL, NULL);

          // access our IDA and folder properties
          pIda = ACCESSDLGIDA( hwndDlg, PAIDA);
          ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( pIda->hpropFolder );

          // disable ok, cancel and help button
          ENABLECTRL( hwndDlg, PID_PB_OK, FALSE );
          ENABLECTRL( hwndDlg, PID_PB_CANCEL, FALSE );
          ENABLECTRL( hwndDlg, PID_PB_HELP, FALSE );

          // save documents of short name listbox
          UtlCopyListBox( hwndSavedDocsLB, pIda->pA->hwndDocLB );

          // issue command to all active dialog pages
          while ( pIda->hwndPages[nItem] && fOK )
          {
            PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                                 DWL_DLGPROC );

            switch ( nItem )
            {
              // general analysis settings
              case 0:
                fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                               PID_PB_OK, 0L);
                break;

              // misc. analysis settings
              case 1:
                fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                               PID_PB_OK, 0L);
                break;


              // autsubst analysis settings
              case 2:
                if ( pIda->fAutSubst )
                {
                  fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                                 PID_PB_OK, 0L);
                } /* endif */
                break;

              // list settings
              case 3:
                if ( pIda->fListCreation )
                {
                  fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                                 PID_PB_OK, 0L);
                } /* endif */
                break;

            } /* endswitch */
            nItem++;
         } /* endwhile */

         // check if list creation requested but no list
         if ( fOK )
         {
           if ( pIda->fListCreation )
           {
             if ( !pIda->fCreateNewTermsCb && !pIda->fCreateFndTermsCb &&
                  !pIda->fAddTermsToDicCb )
             {
               UtlErrorHwnd( ERROR_NO_LIST_SELECTION, MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
               fOK = FALSE;
             } /* endif */
           } /* endif */
         } /* endif */

         // create new MT job properties if MT processing requested
#ifdef NFLUENT_MT_IF
#else
         if ( fOK && pIda->fMachTrans )
         {
           memset( &(pIda->pTAI->stMTData ), 0, sizeof(pIda->pTAI->stMTData) );
           Utlstrccpy( pIda->pTAI->stMTData.szFolShortName, UtlGetFnameFromPath( pIda->szFolderProp ), DOT );
           FolQueryInfo( pIda->szFolderObjName, NULL, NULL, pIda->pTAI->stMTData.szSourceLang,
                         pIda->pTAI->stMTData.szTargetLang, FALSE );
           SubFolObjectNameToName( pIda->szFolderObjName, pIda->pTAI->stMTData.szFolderName );
           pIda->pTAI->stMTData.hwndParent = hwndDlg;
           fOK = (BOOL)EqfSend2Handler( MTLISTHANDLER, WM_EQF_PROCESSTASK,
                                        MP1FROMSHORT( PID_FILE_MI_NEW ),
                                        MP2FROMP( &(pIda->pTAI->stMTData ) ) );
         } /* endif */
#endif

         if (fOK)
         {
           //--- get number of items in listboxes of Analysis(1) dialog ---
           // get number of files to be analyzed
           pIda->pTAI->stSourcefiles.usNumber =
                          QUERYITEMCOUNTHWND( pIda->pA->hwndDocLB );

           if ( pIda->pTAI->stSourcefiles.usNumber == 0 )  //no files in analysis LB
           {                                                //that happens if a folder
             UtlErrorHwnd( ERROR_NODOCS_ANALYSIS,          // is empty
                           MB_CANCEL, 0, NULL, EQF_WARNING, hwndDlg );
             fOK = FALSE;
           } /* endif */
         } /* endif */

         if ( fOK )
         {
            //--- if add segments to TM stuff is disabled because no TM are exists
            if ( pIda->fDisableTMStuff )
            {
               //--- set add segments to TM flag to FALSE
               pIda->pTAI->fInsertToTM = FALSE;
            }
            else
            {
               //get status of add segments to transl. memory CB and save them folder
               //properties and analysis interface
               ppropFolder->fSavedDlgIanaChkMdb = (EQF_BOOL)pIda->fAddToMem;
               pIda->pTAI->fInsertToTM = ppropFolder->fSavedDlgIanaChkMdb;
            }/*endif*/


            //get status of add exact matches to document CB and save them folder prop.
            //and analysis interface
            ppropFolder->fSavedDlgIanaChkAddSeg = (EQF_BOOL)pIda->fAutSubst;
            pIda->pTAI->fInsertTMMatches = ppropFolder->fSavedDlgIanaChkAddSeg;
            ppropFolder->fUsedLatestExactMatch = (EQF_BOOL)pIda->fLatestMatch;
            pIda->pTAI->fUseLatestTMMatch = ppropFolder->fUsedLatestExactMatch;

            ppropFolder->fExactContextTMMatch = (EQF_BOOL)pIda->fExactContext;
            pIda->pTAI->fExactContextTMMatch = ppropFolder->fExactContextTMMatch;

            ppropFolder->fAnalAutoJoin = (EQF_BOOL)pIda->fAutoJoin;
            pIda->pTAI->fAutoJoin = ppropFolder->fAnalAutoJoin;

            ppropFolder->fAnalIgnoreIdentical = (EQF_BOOL)pIda->fIgnoreIdentical;
            pIda->pTAI->fIgnoreIdentical = ppropFolder->fAnalIgnoreIdentical;

            ppropFolder->fAnalIgnoreComment = (EQF_BOOL)pIda->fIgnoreComment;
            pIda->pTAI->fIgnoreComment = ppropFolder->fAnalIgnoreComment;

            ppropFolder->fAnalIgnorePath = (EQF_BOOL)pIda->fIgnorePath;
            pIda->pTAI->fIgnorePath = ppropFolder->fAnalIgnorePath;

            ppropFolder->fAnalLeadingWS = (EQF_BOOL)pIda->fLeadingWS;
            pIda->pTAI->fLeadingWS = ppropFolder->fAnalLeadingWS;

            ppropFolder->fAnalTrailingWS = (EQF_BOOL)pIda->fTrailingWS;
            pIda->pTAI->fTrailingWS = ppropFolder->fAnalTrailingWS;

            ppropFolder->fAnalRespectCRLF= pIda->fRespectCRLF;
            pIda->pTAI->fRespectCRLF = ppropFolder->fAnalRespectCRLF;

            ppropFolder->fAnalNoAddBlank = pIda->fNoAddBlank;
            pIda->pTAI->fNoAddBlank = ppropFolder->fAnalNoAddBlank;

            ppropFolder->fAnalUseReplacementList = pIda->pTAI->fUseReplacementList  = pIda->fUseReplacementList;
            strcpy( pIda->pTAI->szReplacementList, pIda->szReplacementList );
            strcpy( ppropFolder->szAnalReplacementList, pIda->szReplacementList );

            //get status of get new matches into external file
            ppropFolder->fSavedDlgIanaChkNoMatch = (EQF_BOOL)pIda->fSNOMATCH;
            pIda->pTAI->fInsertNewMatch = ppropFolder->fSavedDlgIanaChkNoMatch;

            ppropFolder->fSavedDlgIanaTMMatch = (EQF_BOOL)pIda->fMatchCount;
            pIda->pTAI->fTMMatch         = ppropFolder->fSavedDlgIanaTMMatch;

            ppropFolder->fSavedDlgIanaRedundCount = (EQF_BOOL)pIda->fRedundancyCounting;
            pIda->pTAI->fRedundCount = ppropFolder->fSavedDlgIanaRedundCount;

            ppropFolder->fSavedDlgIanaChkMachTrans = pIda->fMachTrans;
            pIda->pTAI->fMachTrans = ppropFolder->fSavedDlgIanaChkMachTrans;

            ppropFolder->fSavedDlgIanaMatchList = pIda->fMatchList;
            pIda->pTAI->fMatchList = ppropFolder->fSavedDlgIanaMatchList;

            ppropFolder->fSavedDlgIanaProtXmpScreen = FALSE;

            ppropFolder->fSavedDlgIanaProtXmp = pIda->fProtXmp;
            pIda->pTAI->fProtXmp = ppropFolder->fSavedDlgIanaProtXmp;

            ppropFolder->fSavedDlgIanaProtMsgNum = pIda->fProtMsgNum;
            pIda->pTAI->fProtMsgNum = ppropFolder->fSavedDlgIanaProtMsgNum;

            ppropFolder->fSavedDlgIanaProtMeta = pIda->fProtMeta;
            pIda->pTAI->fProtMeta = ppropFolder->fSavedDlgIanaProtMeta;

            ppropFolder->fSavedDlgIanaProtScreen = pIda->fProtScreen;
            pIda->pTAI->fProtScreen = ppropFolder->fSavedDlgIanaProtScreen;

            ppropFolder->fSavedDlgIanaProtCodeBlock = pIda->fProtCodeBlock;
            pIda->pTAI->fProtCodeBlock = ppropFolder->fSavedDlgIanaProtCodeBlock;

            ppropFolder->fSavedDlgIanaStopAtFirstExact = pIda->fStopAtFirstExact;
            pIda->pTAI->fStopAtFirstExact       = ppropFolder->fStopAtFirstExact;
            pIda->pTAI->fStopAtFirstExactAddMem = ppropFolder->fSavedDlgIanaStopAtFirstExact;

            ppropFolder->fSavedDlgIanaBalanceList = pIda->fBalanceList;
            pIda->pTAI->fBalanceList = ppropFolder->fSavedDlgIanaBalanceList;
            if ( pIda->pTAI->fBalanceList )
            {
              // ensure that TM match counting is peformed when balance list is selected
              pIda->pTAI->fTMMatch = TRUE;
            } /* endif */

            ppropFolder->fSavedDlgIanaProfile = pIda->fProfile;
            pIda->pTAI->fProfile = pIda->fProfile;
            strcpy( ppropFolder->szSavedDlgIanaProfile, pIda->szProfile );
            if ( pIda->fProfile)
            {
              strcpy( pIda->pTAI->szProfile, pIda->szProfile );
            }
            else
            {
              pIda->pTAI->szProfile[0] = EOS;
            } /* endif */
         } /* endif fOk */

         if ( fOK )
         {
            //--- set number of morphological dictionarie to one
            pIda->pTAI->stMorphDict.usNumber = 1;

            //---reallocate analysis interface and add data storage
            //calculate size to be needed
            {
              LONG lBufferSize = ( (LONG)(pIda->pTAI->stSourcefiles.usNumber +
                                          pIda->pTAI->stInputDict.usNumber   +
                                          pIda->pTAI->stInputMemDb.usNumber  +
                                          pIda->pTAI->stMorphDict.usNumber) *
                                    (LONG)MAX_LONGFILESPEC );
              fOK = UtlAlloc( (PVOID *)&pIda->pTAI, (LONG)sizeof( TAINPUT ),
                              (LONG)sizeof(TAINPUT) + lBufferSize, ERROR_STORAGE);
            }
         } /* endif fOk */

         if ( fOK )
         {
            //-----copy rest of needed data to analysis interface
            CopyGeneralInfo2TAI( pIda, ppropFolder );

            //set actual pointer to analysis IF data buffer
            pIda->pszAct = (PSZ)(pIda->pTAI) + sizeof( TAINPUT );

            //--- set pointer actual pointer
            pIda->pszStart = pIda->pszAct;

            //--- save  files to be analyzed and save them to analysis interface
            fOK = Files2BeAnalyzed2Analysis( pIda, hwndDlg );
         }/*end if fOk*/

         if ( fOK )
         {
           SHORT sIndex;

           //--- save actual pointer to start pointer
           pIda->pszStart = pIda->pszAct;

           //--- save selected dictionaries to analysis interface
           for ( sIndex = 0; sIndex < (SHORT)pIda->pTAI->stInputDict.usNumber; sIndex++)
           {
             strcpy( pIda->pszAct, pIda->Dict.szDictName[sIndex] );
             pIda->pszAct += strlen(pIda->pszAct) + 1;
            } /* endfor */

            //save offset to analysis interface
            pIda->pTAI->stInputDict.ulOffset = pIda->pszStart - (PSZ)(pIda->pTAI);

            //save length to analysis interface
            pIda->pTAI->stInputDict.ulLength = pIda->pszAct - pIda->pszStart;

            //--- save actual pointer to start pointer
            pIda->pszStart = pIda->pszAct;

            //--- save selected input mdbs to analysis interface
            {
               SHORT sIndex;

               // init last used values
               memset ( ppropFolder->szSavedDlgIanaInMdb, NULC, sizeof(ppropFolder->szSavedDlgIanaInMdb) );
               memset ( ppropFolder->szSavedDlgInMdb, NULC, sizeof(ppropFolder->szSavedDlgInMdb) );

               for ( sIndex = 0; sIndex < (SHORT)pIda->pTAI->stInputMemDb.usNumber;
                     sIndex++ )
               {
                  //get selected memory dbs
                  strcpy( pIda->pszAct, pIda->szSelectedMems[sIndex] );

                  //increase pointer
                  pIda->pszAct += strlen( pIda->pszAct ) + 1;

                  //save 10 selected memory dbs as last used values
                  if ( sIndex < SAVE_MDB_NUM )
                  {
                    strcpy( ppropFolder->szSavedDlgInMdb[ sIndex ], pIda->szSelectedMems[sIndex] );
                  }/*end if*/

               }/*end for*/

               //save offset to analysis buffer
               pIda->pTAI->stInputMemDb.ulOffset = pIda->pszStart - (PSZ)(pIda->pTAI);

               //save length to analysis interface
               pIda->pTAI->stInputMemDb.ulLength = pIda->pszAct - pIda->pszStart;
            }

            //--- save actual pointer to start pointer
            pIda->pszStart = pIda->pszAct;

            //--- get pointer to language from mdb properties
            {
              UtlParseX15( pIda->szMemProp, LANG_LANGUAGE_IND );

                 //--- save default morphological dicitonary to analysis interface
              strcpy( pIda->pszStart, EMPTY_STRING );
            }

            //--- save length of language to analysis interface
            pIda->pTAI->stMorphDict.ulLength = strlen( pIda->pszStart );

            //--- save offset to language analysis interface
            pIda->pTAI->stMorphDict.ulOffset = pIda->pszStart - (PSZ)( pIda->pTAI);


            if ( fOK )
            {
               //save folder properties
               ULONG ulErrorInfo;

               if( SaveProperties( pIda->hpropFolder, &ulErrorInfo ) )
               {
                 UtlErrorHwnd( ERROR_DIALOG_VALUES_NOT_SAVED, MB_OK,
                               0, NULL, EQF_WARNING, hwndDlg );
               } /* endif */
            } /* endif */

            //reset access mode to save folder properties
            ResetPropAccess( pIda->hpropFolder, PROP_ACCESS_WRITE );

            if ( fOK )
            {
               //close dialog, start analysis instance
               WinPostMsg( hwndDlg, WM_EQF_CLOSE, MP1FROMSHORT( TRUE ), NULL );
            } /* endif fOk */
         } /* endif fOk */


         // check the state of the "List settings" check box to enable or disable
         // the "Settings" pushbutton
         pIda->pTAI->fNTLProcessing = (fOK && pIda->fListCreation );


         //--- enable ok, cancel and help button
         ENABLECTRL( hwndDlg, PID_PB_OK, TRUE );
         ENABLECTRL( hwndDlg, PID_PB_CANCEL, TRUE );
         ENABLECTRL( hwndDlg, PID_PB_HELP, TRUE );

         if ( !fOK )
         {
           /*****************************************************************/
           /* Restore documents in documents listbox                        */
           /*****************************************************************/
           DELETEALLHWND( pIda->pA->hwndDocLB );
           UtlCopyListBox( pIda->pA->hwndDocLB, hwndSavedDocsLB );
         } /* endif */
         if ( hwndSavedDocsLB != NULLHANDLE ) WinDestroyWindow( hwndSavedDocsLB );
        }
        break;

      case PID_PB_CANCEL:
      case DID_CANCEL:
        POSTEQFCLOSE( hwndDlg, FALSE );
        break;

      default:
         mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         break;
   } /* endswitch */

   return( mResult );
} /* end of AnaPropCommand */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     AnaPropertySheetNotification
//+----------------------------------------------------------------------------+
// Function call:     AnaPropertySheetNotifiaction( hwndDlg, mp1, mp2);
//+----------------------------------------------------------------------------+
// Description:       handle changes on the tab page
//+----------------------------------------------------------------------------+
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//+----------------------------------------------------------------------------+
// Returncode type:   MRESULT
//+----------------------------------------------------------------------------+
// Returncodes:       return code from default window proc or FALSE
//+----------------------------------------------------------------------------+
// Function flow:     switch ( pNMHdr->code )
//                      case TCN_SELCHANGE:
//                        activate new page
//                      case TCN_SELCHANGING
//                        hide the dialog
//                    return
//+----------------------------------------------------------------------------+
MRESULT AnaPropertySheetNotification
(
  HWND hwndDlg,
  WPARAM  mp1,
  LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  LONG      lTabCtrl;
  MRESULT      mResult = FALSE;
  PAIDA     pIda;
  pNMHdr = (LPNMHDR)mp2;

  mp1;
  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwndDlg, PAIDA);
      if ( pIda )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_ANA_PROP_TABCTRL );
        lTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, lTabCtrl, &Item );
        lTabCtrl = Item.lParam;
        ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA( hwndDlg, PAIDA );
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        TC_ITEM Item;
        PFNWP pfnWp;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_ANA_PROP_TABCTRL );
        lTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, lTabCtrl, &Item );
        lTabCtrl = Item.lParam;
        pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ lTabCtrl ], DWL_DLGPROC );

        mResult = pfnWp( pIda->hwndPages[lTabCtrl], WM_COMMAND,
                         PID_PB_OK, 1L);
        if ( mResult )
        {
          /************************************************************/
          /* stick on the side                                        */
          /* we have to post the request again since one of the system*/
          /* routines thinks that we still want to change the page..  */
          /************************************************************/
          WinPostMsg( hwndDlg, TCM_SETCURSEL, lTabCtrl, 0L );
        } /* endif */
        ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_HIDE );
      } /* endif */
      break;
    case TTN_NEEDTEXT:
      {
        TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) mp2;
        if ( pToolTipText )
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          TC_ITEM Item;
          HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_ANA_PROP_TABCTRL );
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
          switch ( (SHORT)Item.lParam )
          {
            case 0:      // first page
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TTIP_GENERAL,
                          pToolTipText->szText );
              break;
            case 1:      // second page
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TTIP_OTHER,
                          pToolTipText->szText );
              break;
            case 2:      // third page
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TTIP_AUTSUBST,
                          pToolTipText->szText );
              break;
            case 3:      // fourth page
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TTIP_LISTS,
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
} /* end of function AnapropertySheetNotification */
