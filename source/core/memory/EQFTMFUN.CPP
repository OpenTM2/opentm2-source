//+----------------------------------------------------------------------------+
//|EQFTMFUN.C     Translation Memory Maintenance functions                     |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:  A. Luedde                                                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|              This module contains functions that call the dialogs          |
//|              to start the TranslationManager maintenance functions.        |
//|              And does the queries from the TranslationMemory.              |
//|              First this module is called from TM/2.....                    |
//|              then it calls the editor which displays the queried           |
//|              segments and then the editor calls the query functions.       |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
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
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TP               // public translation processor definitions
#include <eqf.h>                  // General Translation Manager include file


#include "EQFTPI.H"               // private translation processor definitions
#include "EQFTMM.H"               // TMM include file  .
#include "EQFTMM.ID"              // TMM dialog include file
#include "EQFTMFUN.ID"
#include "time.h"                 // C library: time reladed functions
#include "core\PluginManager\OtmSharedMemory.h"
static OtmMemory *pMemoryDB;

MONTHTABLE MonthTable[] =
{
  { JANUARY,   "" },
  { FEBRUARY,  "" },
  { MARCH,     "" },
  { APRIL,     "" },
  { MAY,       "" },
  { JUNE,      "" },
  { JULY,      "" },
  { AUGUST,    "" },
  { SEPTEMBER, "" },
  { OCTOBER,   "" },
  { NOVEMBER,  "" },
  { DECEMBER,  "" },
  { MAXENTRY,  "" }
 };


BOOL CheckForMatchingSegment
(
   PTMIDA          pTMIda,             // ida of query
   PBOOL           pfSkip              // pointer to callers segment skip flag
);

static USHORT ProposalTypeToFlag(OtmProposal::eProposalType type);
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     StartTMFunct                                             |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = StartTMFunct( hwnd, pszFileName );                 |
//+----------------------------------------------------------------------------+
//|Description:       is called from WM_INIT of editor msg queue               |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwnd              handle of window         |
//|                   PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     get properties of TM                                     |
//|                   check if password was already entered                    |
//|                   if ( PW not already entered )                            |
//|                      call PW dialog                                        |
//|                      set PW (in "temp props")                              |
//|                   call TM query dialog                                     |
//|                   allocate buffers to be used for TM functions             |
//|                   open TM                                                  |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+
BOOL  StartTMFunct
(
   HWND            hwnd,               // handle of window
   PTMIDA          pTMIda              // ida of query
)
{
   BOOL            fOK = TRUE;         // success indicator
   MemoryFactory *pFactory = MemoryFactory::getInstance();

   if ( fOK )
   {
     // open TM properties and extract needed stuff
     fOK = GetTmProp2Ida( pTMIda );
   } /* endif */

   if ( fOK )
   {
     // query PW from temp props into Ida
     // (if not already entered PW is 0)
     fOK = UtlQueryString( QST_CONTROLDIR, pTMIda->szPassword, MAX_FNAME );
   } /* endif */


   if ( fOK )
   {
//   // check if PW was already entered
//   if ( strnicmp(pTMIda->szPassword, TM_PW_LAST_TRY, TM_PW_LENGTH) == 0 )
//   {
//     // error: already tried for 3 times
//     UtlErrorHwnd( ERROR_TMM_PW_LAST, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
//     fOK = FALSE;                    // quit maintenance functions
//   }
//   else
//   {
//     // if no pw set call dialog for entering pw
//     if ( strnicmp(pTMIda->szPassword, TM_PASSWORD, TM_PW_LENGTH) != 0 )
//     {
//       // call PW dialog
//       DIALOGBOX( hwnd, TMPASSWORDDLG, hResMod,
//                  ID_TM_PASSWORD_DLG, pTMIda, fOK );
//     } /* endif */
//   } /* endif */
   } /* endif */

   // open Translation Memory
   if ( fOK )
   {
     int iRC = 0;

     pTMIda->pMemory = pFactory->openMemory( NULL, pTMIda->szMemLongName, EXCLUSIVE, &iRC );
    
     if ( iRC == 0 )
     {
       // close TM in any case of exit
       pMemoryDB = pTMIda->pMemory;
       DosExitList( EXLST_ADD, (PFNEXITLIST)TMCLEANUP );
     }
     else
     {
       fOK = FALSE;
     }
   } /* endif */

   // create string pool for the document names and other
   if ( fOK )
   {
     pTMIda->pPool = PoolCreate( 16000 );
     fOK = (pTMIda->pPool != NULL);
   } /* endif */

   // setup list of documents, languages, and tagtables contained in TM
   if ( fOK )
   {
     fOK = TMMGetListFromTM( pTMIda, TMM_TAGTABLES );
     if ( fOK )
     {
       fOK = TMMGetListFromTM( pTMIda, TMM_DOCUMENTS );
     } /* endif */
     if ( fOK )
     {
       fOK = TMMGetListFromTM( pTMIda, TMM_LANGUAGES );
     } /* endif */

     if ( fOK )
     {
       fOK = TMMGetListFromTM( pTMIda, TMM_LONGNAMES );
     } /* endif */
   } /* endif */

   if ( fOK )
   {
     HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
     // call query dialog
     // set default for query source entries
     pTMIda->fLookup = TRUE;
     DIALOGBOX( hwnd, TMQUERYDLG, hResMod,
                ID_TM_QUERY99_DLG, pTMIda, fOK );
   } /* endif */

   if ( fOK )
   {
     /***************************************************************/
     /* Position to first record in TM                              */
     /***************************************************************/
     pTMIda->NextSegAddr.ulKey    = FIRST_KEY;
     pTMIda->NextSegAddr.usTarget = 1;
   }
   else if ( pTMIda->pMemory != NULL )
   {
     pFactory->closeMemory( pTMIda->pMemory );
     pTMIda->pMemory = NULL;
     pMemoryDB = NULL;
   } /* endif */

   return( fOK );

} /* end of function StartTMFunct */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetTmProp2Ida                                            |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = GetTmProp2Ida( pTMIda )                            |
//+----------------------------------------------------------------------------+
//|Description:       get TM props and save needed to TMIda                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Output parameter:  some entries of query ida are filled                     |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     get EQF drive                                            |
//|                   open TM properties                                       |
//|                   copy entries from properties into Ida                    |
//|                   close properties                                         |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+
BOOL  GetTmProp2Ida
(
   PTMIDA          pTMIda              // ida of query
)
{
  BOOL        fOK = TRUE;              // success indicator
  CHAR        szPropName[MAX_FILESPEC];// buffer for TM property name
  CHAR        szSysPath[MAX_EQF_PATH]; // system directory

  /********************************************************************/
  /* Open TM Properties                                               */
  /********************************************************************/
  strcpy( szPropName, pTMIda->szMemShortName );
  strcat( szPropName, EXT_OF_MEM );
  UtlMakeEQFPath( szSysPath, NULC, SYSTEM_PATH, NULL );

  MemoryFactory *pFactory = MemoryFactory::getInstance();
  OtmMemoryPlugin::PMEMORYINFO pInfo = new( OtmMemoryPlugin::MEMORYINFO );
  if(pFactory==NULL ||  pInfo==NULL)
  {
	fOK = FALSE;
  }
  
  /********************************************************************/
  /* get memory info                                                  */
  /********************************************************************/
  if ( fOK )
  {
	int rc = pFactory->getMemoryInfo(NULL,pTMIda->szMemLongName, pInfo );
    if(rc == 0)
	{
      strcpy( pTMIda->szFileName, pInfo->szFullPath );
      strcpy( pTMIda->szSourceLng, pInfo->szSourceLanguage);
	}
    else
	{
	  fOK = FALSE;
	}
	
	delete pInfo;
  } /* endif */
  {
     PPROPSYSTEM pSysProp;             // ptr to EQF system properties
     pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd());
     if ( pSysProp && pSysProp->szSystemPrefLang )
     {
       strcpy(pTMIda->szTargetLng, pSysProp->szSystemPrefLang);
     } /* endif */
  }
   return( fOK );

} /* end of function GetTmProp2Ida */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TMQUERYDLG                                               |
//+----------------------------------------------------------------------------+
//|Description:       opens the query dialog                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   USHORT        message           message id               |
//|                   MPARAM        mp1               message parameter        |
//|                   MPARAM        mp2               message parameter        |
//+----------------------------------------------------------------------------+
//|Output parameter:  query parameters are set in Ida of query                 |
//+----------------------------------------------------------------------------+
//| Returns:          USHORT        mResult                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     dependent on message do case processing:                 |
//|                   WM_INITDLG:                                              |
//|                      call function to initialize the dialog                |
//|                   WM_CLOSE:                                                |
//|                      destroy dialog                                        |
//|                   WM_CONTROL:                                              |
//|                      call function to handle control messages              |
//|                   WM_COMMAND:                                              |
//|                      call function to handle command messages              |
//|                   default:                                                 |
//|                      do default processing                                 |
//|                   endswitch                                                |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK TMQUERYDLG
(
   HWND            hwndDlg,            // handle of dialog window
   WINMSG          message,            // message id
   WPARAM          mp1,                // message parameter
   LPARAM          mp2                 // message parameter
)
{
   MRESULT         mResult = FALSE;    // return code
   PTMIDA          pTMIda;             // ida of query

   switch( message)
   {
      case ( WM_INITDLG ):
        ANCHORDLGIDA( hwndDlg, mp2 );
        pTMIda = (PTMIDA)mp2;

        // initialize the query dialog
        SetupQueryDlg( hwndDlg, pTMIda );
        mResult = DIALOGINITRETURN((MRESULT)TRUE);
        WinPostMsg( hwndDlg, WM_EQF_SETFOCUS, NULL, NULL );
        break;

      case WM_EQF_SETFOCUS:
        SETFOCUS( hwndDlg, ID_TM_QUERY_STRING_MLE );
        break;

      case ( WM_CLOSE ):
        //distroy dialog
        DelCtrlFont( hwndDlg, ID_TM_QUERY_STRING_MLE);
        WinDismissDlg( hwndDlg, SHORT1FROMMP1( mp1 ) );
        break;

      case ( WM_COMMAND ) :
        // handle command messages
        mResult = QueryWM_COMMAND( hwndDlg, WMCOMMANDID( mp1, mp2 ),
                                   WMCOMMANDCMD( mp1, mp2 ) );
        break;

      default:
        mResult = WinDefDlgProc ( hwndDlg, message, mp1, mp2 );
        break;
   } /* endswitch */

   return( mResult );

} /* end of function TMQUERYDLG */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TMSPECDATEDLG                                            |
//+----------------------------------------------------------------------------+
//|Description:       dialog where a date range can be specified               |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   USHORT        message           message id               |
//|                   MPARAM        mp1               message parameter        |
//|                   MPARAM        mp2               message parameter        |
//+----------------------------------------------------------------------------+
//|Output parameter:  date parameters from query ida are filled                |
//+----------------------------------------------------------------------------+
//| Returns:          USHORT        mResult                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     dependent on message do case processing:                 |
//|                   WM_INIT:                                                 |
//|                      call function to initialize the date dialog           |
//|                   WM_CLOSE:                                                |
//|                      destroy dialog                                        |
//|                   WM_COMMAND:                                              |
//|                      get ID and switch on it:                              |
//|                      DID_CANCEL:                                           |
//|                      ID_TM_DATE_CANCEL_PB:                                 |
//|                         post WM_CLOSE msg                                  |
//|                      ID_TM_DATE_ENTER_PB:                                  |
//|                         call function to query the date settings           |
//|                   default:                                                 |
//|                      do default processing                                 |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK TMSPECDATEDLG
(
   HWND            hwndDlg,            // handle of dialog window
   WINMSG          message,            // message id
   WPARAM          mp1,                // message parameter
   LPARAM          mp2                 // message parameter
)
{
   MRESULT         mResult = FALSE;    // return code
   BOOL            fOK = TRUE;         // success indicator
   PTMIDA          pTMIda;             // ida of query

   switch( message)
   {
      case WM_INITDLG:
        ANCHORDLGIDA( hwndDlg, mp2 );
        pTMIda = ACCESSDLGIDA( hwndDlg, PTMIDA );

        // initialize date dialog
        SetupDateDlg( hwndDlg, pTMIda );
        break;

      case WM_CLOSE:
        //distroy dialog
        WinDismissDlg( hwndDlg, SHORT1FROMMP1( mp1 ) );
        break;

      case WM_COMMAND:
        mResult = MRFROMSHORT( TRUE);

        switch (WMCOMMANDID( mp1, mp2 ))
        {
			case (ID_TM_DATE_HELP_PB):
			mResult = UtlInvokeHelp();
			break;
          case DID_CANCEL:
          case ID_TM_DATE_CANCEL_PB:
            WinPostMsg( hwndDlg, WM_CLOSE, NULL, NULL );
            break;

          case ID_TM_DATE_ENTER_PB:
            pTMIda = ACCESSDLGIDA( hwndDlg, PTMIDA );

            // query the settings to ida
            fOK = QueryDateSetting( hwndDlg, pTMIda );

            if ( fOK )
            {
              //dismiss dialog
              WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT( fOK ), NULL );
            } /* endif */
            break;
        } /* endswitch */
        break;
      default:
        mResult = WinDefDlgProc ( hwndDlg, message, mp1, mp2 );
        break;

   } /* endswitch */

   return( mResult );

} /* end of function TMSPECDATEDLG */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TMSPECDOCDLG                                             |
//+----------------------------------------------------------------------------+
//|Description:       dialog to specify a document from which translations     |
//|                   should result                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   USHORT        message           message id               |
//|                   MPARAM        mp1               message parameter        |
//|                   MPARAM        mp2               message parameter        |
//+----------------------------------------------------------------------------+
//|Output parameter:  pTMIda->szDoc2Search is filled                           |
//+----------------------------------------------------------------------------+
//| Returns:          USHORT        mResult                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     dependent on message do case processing:                 |
//|                   WM_INIT:                                                 |
//|                      set length of entryfield                              |
//|                      set doc name from ida to EF (may be empot)            |
//|                      set EF selected and focus on it                       |
//|                   WM_CLOSE:                                                |
//|                   WM_COMMAND:                                              |
//|                      get ID and switch on it:                              |
//|                      DID_CANCEL:                                           |
//|                      ID_TM_QUERY_DOC_CANCEL_PB                             |
//|                         post WM_CLOSE msg                                  |
//|                      ID_TM_DOC_QUERY_ENTER_PB:                             |
//|                         call function to query and verify the              |
//|                         entered document name                              |
//|                   default:                                                 |
//|                      do default processing                                 |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK TMSPECDOCDLG
(
   HWND            hwndDlg,            // handle of dialog window
   WINMSG          message,            // message id
   WPARAM          mp1,                // message parameter
   LPARAM          mp2                 // message parameter
)
{
   MRESULT         mResult = FALSE;    // return code
   BOOL            fOK = TRUE;         // success indicator
   PTMIDA          pTMIda;             // ida of query
   HWND           hLB;

   switch( message)
   {
      case WM_INITDLG:
        ANCHORDLGIDA( hwndDlg, mp2 );
        pTMIda = ACCESSDLGIDA( hwndDlg, PTMIDA );

        /**************************************************************/
        /* set limit of entry field                                   */
        /* set the string already set in Ida (may be empty)           */
        /* set text as selected and focus                             */
        /**************************************************************/

        SETTEXTLIMIT( hwndDlg, ID_TM_QUERY_DOC_EF, MAX_LONGFILESPEC - 1 );
        SETCHECK_TRUE(hwndDlg, ID_TM_QUERY_DOC_FILTER_RB);
        SETTEXT( hwndDlg, ID_TM_QUERY_DOC_EF, pTMIda->szDoc2Search );
        SETEFSEL( hwndDlg, ID_TM_QUERY_DOC_EF, 0, (MAX_FILESPEC - 1) );
        SETFOCUS( hwndDlg, ID_TM_QUERY_DOC_EF );

        /**************************************************************/
        /* Fill list box with document names                          */
        /**************************************************************/
        hLB = GetDlgItem(hwndDlg, ID_TM_QUERY_DOC_FILES_LB);
        SendMessage(hLB, LB_RESETCONTENT, 0, 0);

        {
          int iMaxDocs = pTMIda->pMemory->getNumOfDocumentNames();
          int i = 0;
          for ( i = 0; i < iMaxDocs; i++ )
          {
            if ( pTMIda->pMemory->getDocumentName( i, pTMIda->szBuffer, sizeof(pTMIda->szBuffer) ) != 0 )
            {
             INSERTITEMEND(hwndDlg, ID_TM_QUERY_DOC_FILES_LB, pTMIda->szBuffer );
            } /* endif */             
          } /* endfor */             
        }

        SetHorzScrollingForLB(hLB);

        // activate last used values
        SETCHECK( hwndDlg, ID_TM_QUERY_DOC_FILES_RB, pTMIda->fDocListGiven );
        SETCHECK( hwndDlg, ID_TM_QUERY_DOC_FILTER_RB, !pTMIda->fDocListGiven );
        if ( pTMIda->fDocListGiven && (pTMIda->pszDoc2SearchUprList != NULL) )
        {
          PSZ pszNextX15;        // position of next X15 char in string
          PSZ pszCurPos;               // current position within string
          SHORT  sItem;
          HWND   hwndLB = GetDlgItem( hwndDlg, ID_TM_QUERY_DOC_FILES_LB );

          pszCurPos = pTMIda->pszDoc2SearchUprList;
          while ( *pszCurPos != EOS )
          {
            pszNextX15 = strchr( pszCurPos, X15 );
            if ( pszNextX15 ) *pszNextX15 = EOS;

            sItem = (SHORT)SendMessage( hwndLB, LB_FINDSTRINGEXACT, 0, (LPARAM)pszCurPos );
            if ( sItem != LIT_NONE )
            {
              SendMessage( hwndLB, LB_SETSEL, 1, MAKELONG(sItem, 0) );   \
            } /* endif */

            if ( pszNextX15 )
            {
              *pszNextX15 = X15;
              pszCurPos = pszNextX15 + 1;
            }
            else
            {
              pszCurPos = pszCurPos + strlen(pszCurPos);
            } /* endif */
          } /* endwhile */
        } /* endif */
        break;

      case WM_CLOSE:
        //distroy dialog
        WinDismissDlg( hwndDlg, SHORT1FROMMP1( mp1 ) );
        break;

      case WM_COMMAND:
        mResult = MRFROMSHORT( TRUE);

        switch (WMCOMMANDID( mp1, mp2 ))
        {
		  case (ID_TM_QUERY_DOC_HELP_PB):
		    mResult = UtlInvokeHelp();
		    break;
          case DID_CANCEL:
          case ID_TM_QUERY_DOC_CANCEL_PB:
            WinPostMsg( hwndDlg, WM_CLOSE, NULL, NULL );
            break;

          case ID_TM_QUERY_DOC_ENTER_PB:
            pTMIda = ACCESSDLGIDA( hwndDlg, PTMIDA );

            // get and verify the document name
            fOK = QueryDocSetting( hwndDlg, pTMIda );

            if ( fOK )
            {
              //dismiss dialog
              WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT( fOK ), NULL );
            } /* endif */
            break;

          case ID_TM_QUERY_DOC_FILES_LB:
            pTMIda = ACCESSDLGIDA( hwndDlg, PTMIDA );

            // set 'from list' radiobutton if an item is selected
            if ( WMCOMMANDCMD( mp1, mp2 ) == LN_SELECT  )
            {
              SETCHECK_TRUE( hwndDlg, ID_TM_QUERY_DOC_FILES_RB );
              SETCHECK_FALSE( hwndDlg, ID_TM_QUERY_DOC_FILTER_RB );
            } /* endif */
            break;
        } /* endswitch */
        break;
      default:
        mResult = WinDefDlgProc ( hwndDlg, message, mp1, mp2 );
        break;

   } /* endswitch */

   return( mResult );

} /* end of function TMSPECDOCDLG */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QueryWM_CONTROL                                          |
//+----------------------------------------------------------------------------+
//|Function call:     mResult = QueryWM_CONTROL( hwndDlg, sID, sNotification );|
//+----------------------------------------------------------------------------+
//|Description:       this function handles the WM_CONTROL message of          |
//|                   dialog procedure TMQueryDlg                              |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   SHORT         sID               selected ID              |
//|                   SHORT         sNotification     indicator for action     |
//+----------------------------------------------------------------------------+
//| Returns:          USHORT        mResult                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     dependent on ID do case processing:                      |
//|                   ID_TM_QUERY_SOURCE_RB:                                   |
//|                      enable checkbox for more translations                 |
//|                   ID_TM_QUERY_TARGET_RB:                                   |
//|                      disable checkbox for more translations                |
//|                   ID_TM_QUERY_DATE_RB:                                     |
//|                      set checkbox                                          |
//|                      enable/disable set date                               |
//|                      enable/disable date pushbutton                        |
//|                   ID_TM_QUERY_DOC_RB:                                      |
//|                      set checkbox                                          |
//|                      enable/disable entered doc                            |
//|                      enable/disable doc pushbutton                         |
//|                   ID_TM_QUERY_MPLUS_CB/ID_TM_QUERY_MMINUS_CB:              |
//|                      make them mutually exclusive                          |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+
MRESULT QueryWM_CONTROL
(
   HWND            hwndDlg,            // handle of dialog window
   SHORT           sId,                // selected ID
   SHORT           sNotification       // indicator for action
)
{

   if ( sNotification == BN_CLICKED )
   {
     switch ( sId )
     {
       case ID_TM_QUERY_SOURCE_RB:
         // enable checkbox for more translations
         // more translations are only possible when searching for source
         ENABLECTRL( hwndDlg, ID_TM_QUERY_XTRANS_CB, TRUE );
         break;
       case ID_TM_QUERY_TARGET_RB:
         // disable checkbox for more translations
         ENABLECTRL( hwndDlg, ID_TM_QUERY_XTRANS_CB, FALSE );
         break;
       case ID_TM_QUERY_DATE_CB:
         // enable/disable pushbutton to specify date
         ENABLECTRL( hwndDlg, ID_TM_QUERY_DATE_PB,
                     QUERYCHECK(hwndDlg, ID_TM_QUERY_DATE_CB));
         // enable/disable static date field
         ENABLECTRL( hwndDlg, ID_TM_QUERY_DATE_STATIC,
                     QUERYCHECK(hwndDlg, ID_TM_QUERY_DATE_CB));
         break;
       case ID_TM_QUERY_DOC_CB:
         // enable/disable pushbutton to enter doc name
         ENABLECTRL( hwndDlg, ID_TM_QUERY_DOC_PB,
                     QUERYCHECK(hwndDlg, ID_TM_QUERY_DOC_CB));
         // enable/disable static doc name field
         ENABLECTRL( hwndDlg, ID_TM_QUERY_DOC_STATIC,
                     QUERYCHECK(hwndDlg, ID_TM_QUERY_DOC_CB));
         break;

       // make the following checkboxes mutually exclusive
       case ID_TM_QUERY_MPLUS_CB:
         if (QUERYCHECK(hwndDlg, ID_TM_QUERY_MPLUS_CB)) SETCHECK_FALSE(hwndDlg, ID_TM_QUERY_MMINUS_CB);
         break;
       case ID_TM_QUERY_MMINUS_CB:
         if (QUERYCHECK(hwndDlg, ID_TM_QUERY_MMINUS_CB)) SETCHECK_FALSE(hwndDlg, ID_TM_QUERY_MPLUS_CB);
         break;

     } /* endswitch */
   } /* endif */

   return ( FALSE );

} /* end of function QueryWM_CONTROL */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QueryWM_COMMAND                                          |
//+----------------------------------------------------------------------------+
//|Function call:     mResult = QueryWM_COMMAND( hwndDlg, sID, sNotification );|
//+----------------------------------------------------------------------------+
//|Description:       this function handles the WM_COMMAND message of          |
//|                   dialog procedure TMQueryDlg                              |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   SHORT         sID               selected ID              |
//|                   SHORT         sNotification     indicator for action     |
//+----------------------------------------------------------------------------+
//|Returns:           USHORT        mResult                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     dependent on ID do case processing:                      |
//|                   DID_CANCEL:                                              |
//|                   ID_TM_QUERY_CANCEL_PB:                                   |
//|                      post WM_CLOSE message                                 |
//|                   ID_TM_QUERY_DATE_PB:                                     |
//|                      get access to Ida                                     |
//|                      call date dialog                                      |
//|                      set selected date in static field                     |
//|                   ID_TM_QUERY_DOC_PB:                                      |
//|                      get access to ida                                     |
//|                      call document dialog                                  |
//|                      set entered document in static field                  |
//|                   ID_TM_QUERY_ENTER_PB:                                    |
//|                      get access to ida                                     |
//|                      fOK = GetQuerySettings (extract settings to ida)      |
//|                      if fOK                                                |
//|                        get entered search string from MLE                  |
//|                        post WM_CLOSE message                               |
//|                      else                                                  |
//|                        the entered number to be loaded is wrong            |
//|                        set entryfield for number to set selected and focus |
//|                   ID_TM_QUERY_SOURCE_RB:                                   |
//|                   ID_TM_QUERY_TARGET_RB:                                   |
//|                   ID_TM_QUERY_DATE_CB:                                     |
//|                   ID_TM_QUERY_DOC_CB:                                      |
//|                   ID_TM_QUERY_MPLUS_CB:                                    |
//|                   ID_TM_QUERY_MMINUS_CB:                                   |
//|                      call QueryWM_Control                                  |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+
MRESULT QueryWM_COMMAND
(
   HWND            hwndDlg,            // handle of dialog window
   SHORT           sId,                // selected ID
   SHORT           sNotification       // indicator for action
)
{
   PTMIDA          pTMIda;             // ida of query
   MRESULT         mResult = MRFROMSHORT(TRUE);       // function return value
   CHAR            szQueryDate[20];    // entered date as string

   switch ( sId )
   {
	 case (ID_TM_QUERY_HELP_PB):
	   mResult = UtlInvokeHelp();
	   break;
     case DID_CANCEL:
     case ID_TM_QUERY_CANCEL_PB:
       //dismiss dialog
       WinPostMsg( hwndDlg, WM_CLOSE, NULL, NULL );
       break;

     case ID_TM_QUERY_DATE_PB:
       {
         BOOL fOK = TRUE;
		 HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
         pTMIda = ACCESSDLGIDA( hwndDlg, PTMIDA );

         // call dialog where the date can be specified
         DIALOGBOX( hwndDlg, TMSPECDATEDLG,
                    hResMod, ID_TM_DATE_DLG, pTMIda, fOK );

         if ( fOK )
         {
           // copy the entered date from ida to string
           sprintf( szQueryDate, "%2.2d/%4.4d-%2.2d/%4.4d",
                    pTMIda->usStartMonth, pTMIda->usStartYear,
                    pTMIda->usEndMonth, pTMIda->usEndYear );

           // set date in static field of query dialog
           SETTEXT( hwndDlg, ID_TM_QUERY_DATE_STATIC, szQueryDate );
         } /* endif */
       }
       break;

     case ID_TM_QUERY_DOC_PB:
       {
         BOOL fOK = TRUE;
		 HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
         pTMIda = ACCESSDLGIDA( hwndDlg, PTMIDA );

         // call dialog where a document from which the
         // translations should result can be entered

         DIALOGBOX( hwndDlg, TMSPECDOCDLG,
                    hResMod, ID_TM_QUERY_DOC99_DLG, pTMIda, fOK );
         if ( fOK )
         {
           // set name of document from which entries should be searched for
           // in static field on query dialog
           if (pTMIda -> fDocListGiven) {
             SETTEXT(hwndDlg, ID_TM_QUERY_DOC_STATIC, FILE_LISTSEL);
           }
           else {
             SETTEXT( hwndDlg, ID_TM_QUERY_DOC_STATIC, pTMIda->szDoc2Search );
           }
         } /* endif */
       }
       break;

     case ID_TM_QUERY_ENTER_PB:
       {
         BOOL fOK = TRUE;
         pTMIda = ACCESSDLGIDA( hwndDlg, PTMIDA );

         // get query settings into Ida and set fOK if
         // number of entries to load is not correct
         fOK = GetQuerySettings( hwndDlg, pTMIda );

         if ( fOK )
         {
           LONG l;

           // get search string from MLE
           memset( pTMIda->szSearchString, 0, sizeof(pTMIda->szSearchString) );
           MLEGETTEXTW( hwndDlg, ID_TM_QUERY_STRING_MLE, pTMIda->szSearchString, l );
//           ANSITOOEM( pTMIda->szSearchString );

           UTF16strcpy( pTMIda->szOrgSearchString, pTMIda->szSearchString );
           if ( pTMIda->szSearchString[0] != EOS )
           {
             // when the search string is not empty
             if ( pTMIda->fExact )
             {
               // when exactly the entered string is searched for
               // get rid of CR and LF in search string
               CutCR_LF( pTMIda->szSearchString, TRUE, pTMIda->ulSystemPrefCP );
             }
             else
             {
               // when wildcard search is specified
               // get rid of CR and LF and check if more than one
               // asterisk is directly following each other
               AsterixCR_LF( pTMIda->szSearchString );
             } /* endif */
           } /* endif */

           //dismiss dialog
           WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT( TRUE ), NULL );
         }
         else
         {
           // display error returned through uiErrCode
           UtlErrorHwnd( (SHORT)pTMIda->uiErrCode, MB_CANCEL,
                         0, NULL, EQF_ERROR, hwndDlg );
           // set entryfield for entering the number selected and focus
           switch (pTMIda -> uiErrCode) {
             case ERROR_TMM_LOAD_TOO_SMALL:
               SETEFSEL( hwndDlg, ID_TM_QUERY_SEGNUM_EF, 0, 6 );
               SETFOCUS( hwndDlg, ID_TM_QUERY_SEGNUM_EF );
               break;
             case ERROR_TMM_SEARCHSEGNUM:
               SETEFSEL(hwndDlg, ID_TM_QUERY_SEARCH_NUM_EF, 0, 6);
               SETFOCUS(hwndDlg, ID_TM_QUERY_SEARCH_NUM_EF);
               break;
             case ERROR_TMM_SHOWPMSEGS:
               SETEFSEL(hwndDlg, ID_TM_QUERY_PMSEGS_EF, 0, 6);
               SETFOCUS(hwndDlg, ID_TM_QUERY_PMSEGS_EF);
               break;
           }
         } /* endif */
       }
       break;

     case ID_TM_QUERY_SOURCE_RB:
     case ID_TM_QUERY_TARGET_RB:
     case ID_TM_QUERY_DATE_CB:
     case ID_TM_QUERY_DOC_CB:
     case ID_TM_QUERY_MPLUS_CB:
     case ID_TM_QUERY_MMINUS_CB:
       // do processing in QueryWM_CONTROL
       mResult = QueryWM_CONTROL( hwndDlg, sId, sNotification );
       break;

      case ID_TM_QUERY_STRING_MLE:
         if ( sNotification == EN_KILLFOCUS )
         {
           ClearIME( hwndDlg );
         } /* endif */
         break;
   } /* endswitch */

   return ( mResult );

} /* end of function QueryWM_COMMAND */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     SetHorzScrollingForLB                                    |
//+----------------------------------------------------------------------------+
//|Function call:     SetHorzScrollingForLB( HWND hLB )                        |
//+----------------------------------------------------------------------------+
//|Description:       initialize the query dialog                              |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hLB               handle of listbox        |
//+----------------------------------------------------------------------------+
//|Output parameter:  none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     Set horizontal scrollbar to some                         |
//|                   reasonable size                                          |
//+----------------------------------------------------------------------------+

VOID SetHorzScrollingForLB(HWND hLB) {
  int iNumEntries, i;
  LONG lMaxWidth = 0;
  CHAR pszStrBuf[MAX_LONGFILESPEC];
  SIZE size;
  HDC hdc;

  iNumEntries = SendMessage(hLB, LB_GETCOUNT, 0, 0);
  hdc = GetDC(hLB);
  for (i = 0; i < iNumEntries; i++) {
    SendMessage(hLB, LB_GETTEXT, (WPARAM) i, (LPARAM) (LPCTSTR) pszStrBuf);
    GetTextExtentPoint32(hdc, pszStrBuf, strlen(pszStrBuf), &size);
    if (size.cx > lMaxWidth) lMaxWidth = size.cx;
  }
  ReleaseDC(hLB, hdc);
//  SendMessage(hLB, LB_SETHORIZONTALEXTENT, (lMaxWidth + GetSystemMetrics(SM_CXVSCROLL)) * 4 / (GetDialogBaseUnits() & 0xffff), 0);
  SendMessage(hLB, LB_SETHORIZONTALEXTENT, lMaxWidth + GetSystemMetrics(SM_CXVSCROLL), 0);
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     SetupQueryDlg                                            |
//+----------------------------------------------------------------------------+
//|Function call:     SetupQueryDlg( hwndDlg, pTMIda );                        |
//+----------------------------------------------------------------------------+
//|Description:       initialize the query dialog                              |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Output parameter:  some initials are set in ida                             |
//+----------------------------------------------------------------------------+
//|Function flow:     set initials in ida and                                  |
//|                   show them on query dialog                                |
//+----------------------------------------------------------------------------+
VOID SetupQueryDlg
(
   HWND            hwndDlg,            // handle of dialog window
   PTMIDA          pTMIda              // ida of query
)
{
   CHAR            szQueryDate[20];    // date range to be searched for
   CHAR            szSet[10];          // number of entries to be searched for
   LONG            lTime = 0L;         // current time in seconds
   struct tm       *timestruct = NULL; // current time in more detail

   /*******************************************************************/
   /* get name of TM and insert as static text                        */
   /*******************************************************************/
   OEMTOANSI( pTMIda->szMemLongName );
   SETTEXT( hwndDlg, ID_TM_QUERY_TM_STATIC, pTMIda->szMemLongName );
   ANSITOOEM( pTMIda->szMemLongName );

   // set control font under Windows (for DBCS display)
   SetCtrlFnt( hwndDlg, GetCharSet(), ID_TM_QUERY_STRING_MLE, 0 );

   // set date settings to false
   SETCHECK( hwndDlg, ID_TM_QUERY_DATE_CB, pTMIda->fDate );
   ENABLECTRL( hwndDlg, ID_TM_QUERY_DATE_PB, pTMIda->fDate );
   ENABLECTRL( hwndDlg, ID_TM_QUERY_DATE_STATIC, pTMIda->fDate );

   // set settings for document search to false
   SETCHECK( hwndDlg, ID_TM_QUERY_DOC_CB, pTMIda->fDoc );
   ENABLECTRL( hwndDlg, ID_TM_QUERY_DOC_PB, pTMIda->fDoc );
   ENABLECTRL( hwndDlg, ID_TM_QUERY_DOC_STATIC, pTMIda->fDoc );

   // gray out the "show +/- segments" section
   ENABLECTRL(hwndDlg, ID_TM_QUERY_PMSEGS1, FALSE);
   ENABLECTRL(hwndDlg, ID_TM_QUERY_PMSEGS_EF, FALSE);
   ENABLECTRL(hwndDlg, ID_TM_QUERY_PMSEGS2, FALSE);

   /*******************************************************************/
   /* check whether IDA is already filled with a date from where the  */
   /* search could go and insert the date as static text              */
   /* (IDA is never filled when no props will be generated...)        */
   /* when ida not already filled get current date to set it as end   */
   /* date start date is 1/92                                         */
   /*******************************************************************/
   if ( pTMIda->usStartMonth == 0 )
   {
     // Ida not already filled
     time( &lTime );                   // get current time
     timestruct = localtime( &lTime ); // set current time into struct

     // set date in IDA
     // this "date" is needed to be able to set it the next time
     // the query is called, not necessary for further functions
     pTMIda->usStartMonth = 1;
     pTMIda->usStartYear = START_YEAR;
     pTMIda->usEndMonth = (USHORT)(timestruct->tm_mon) + 1;
     pTMIda->usEndYear = (USHORT)timestruct->tm_year + NINETEEN;
     memset( timestruct, 0, sizeof(struct tm) );

     /*****************************************************************/
     /* get date range in seconds to ida                              */
     /* this timestamp can be compared with timestamp from segments   */
     /*****************************************************************/
     // get start date, with full first month
     timestruct->tm_year = (SHORT)(pTMIda->usStartYear - NINETEEN);
     timestruct->tm_mon = (SHORT)(pTMIda->usStartMonth);
     timestruct->tm_mday = 1;
     pTMIda->lStartDate = mktime( timestruct );

     // get end date with full last month
     timestruct->tm_year = (SHORT)(pTMIda->usEndYear - NINETEEN);
     timestruct->tm_mon = (SHORT)(pTMIda->usEndMonth);
     timestruct->tm_mday = 0;
     pTMIda->lEndDate = mktime( timestruct );
   } /* endif */

   // show date range
   sprintf( szQueryDate, "%2.2d/%4.4d-%2.2d/%4.4d",
            pTMIda->usStartMonth, pTMIda->usStartYear,
            pTMIda->usEndMonth, pTMIda->usEndYear );

   // set the date as static text to query dialog
   SETTEXT( hwndDlg, ID_TM_QUERY_DATE_STATIC, szQueryDate );

   // if already a document to be searched for is entered
   // set it as default, else set "*.*"
   if ( pTMIda->szDoc2Search[0] == EOS )
   {
     strcpy( pTMIda->szDoc2Search, DEFAULT_PATTERN );
     strcpy( pTMIda->szDoc2SearchUpr, DEFAULT_PATTERN );
   } /* endif */
   if ( pTMIda->fDocListGiven )
   {
     SETTEXT(hwndDlg, ID_TM_QUERY_DOC_STATIC, FILE_LISTSEL);
   }
   else
   {
     SETTEXT( hwndDlg, ID_TM_QUERY_DOC_STATIC, pTMIda->szDoc2Search );
   }

   /*******************************************************************/
   /* Fill markup combobox and select first markup                    */
   /*******************************************************************/
   {
     SHORT sI;

     for ( sI = 0; sI < pTMIda->sTagTables; sI++ )
     {
       CBINSERTITEM( hwndDlg, ID_TM_QUERY_MARKUP_CB,
                     (*pTMIda->ppTagTables)[sI] );
     } /* endfor */
     if ( pTMIda->szTagTable[0] != EOS )
     {
       SHORT sItem;
       CBSEARCHSELECT( sItem, hwndDlg, ID_TM_QUERY_MARKUP_CB, pTMIda->szTagTable );
     }
     else
     {
       CBSELECTITEM( hwndDlg, ID_TM_QUERY_MARKUP_CB, 0 );
     } /* endif */
   }

   /*******************************************************************/
   /* set checkmarks of radiobuttons                                  */
   /*******************************************************************/
   SETCHECK( hwndDlg, ID_TM_QUERY_SOURCE_RB, pTMIda->fLookup );
   SETCHECK( hwndDlg, ID_TM_QUERY_TARGET_RB, !pTMIda->fLookup );
   ENABLECTRL( hwndDlg, ID_TM_QUERY_XTRANS_CB, pTMIda->fLookup );
   SETCHECK( hwndDlg, ID_TM_QUERY_XTRANS_CB, pTMIda->fMoreTrans );
   SETCHECK( hwndDlg, ID_TM_QUERY_RESP_RB, pTMIda->fCase );
   SETCHECK( hwndDlg, ID_TM_QUERY_IGN_RB, !pTMIda->fCase );
   SETCHECK( hwndDlg, ID_TM_WILD_SEARCH_CB, !pTMIda->fExact );
   SETCHECK(hwndDlg, ID_TM_QUERY_MPLUS_CB, pTMIda->fMPlus);
   SETCHECK(hwndDlg, ID_TM_QUERY_MMINUS_CB, pTMIda->fMMinus );

   if ( pTMIda->ulSearchSegNum )
   {
     CHAR szSegNum[10];

     sprintf( szSegNum, "%u", pTMIda->ulSearchSegNum );

     SETTEXT( hwndDlg, ID_TM_QUERY_SEARCH_NUM_EF, szSegNum );
   } /* endif */


   /*******************************************************************/
   /* set default or value of Ida for number of segments to be loaded */
   /*******************************************************************/
   SETTEXTLIMIT( hwndDlg, ID_TM_QUERY_SEGNUM_EF, 5 );
   memset( szSet, 0, sizeof(szSet) );

   if ( pTMIda->ulSeg2Load )
   {
     // set value from ida
     _ltoa( pTMIda->ulSeg2Load, szSet, 10 );
   }
   else
   {
     // set default
     _ltoa( 99999, szSet, 10 );
     pTMIda->ulSeg2Load = 99999;
   } /* endif */
   SETTEXT( hwndDlg, ID_TM_QUERY_SEGNUM_EF, szSet );

   /*******************************************************************/
   /* set the length of MLE                                           */
   /*******************************************************************/
   MLESETTEXTLIMIT( hwndDlg, ID_TM_QUERY_STRING_MLE,
                    (sizeof(pTMIda->szSearchString)-1) );

   UTF16strcpy( pTMIda->szSearchString, pTMIda->szOrgSearchString );
   SETFOCUS( hwndDlg, ID_TM_QUERY_STRING_MLE );
//   OEMTOANSI( pTMIda->szSearchString );
   MLESETTEXTW( hwndDlg, ID_TM_QUERY_STRING_MLE, pTMIda->szSearchString );
//   ANSITOOEM( pTMIda->szSearchString );

   return;

} /* end of function SetupQueryDlg */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetQuerySettings                                         |
//+----------------------------------------------------------------------------+
//|Function call:     GetQuerySettings( pTMIda )                               |
//+----------------------------------------------------------------------------+
//|Description:       queries the settings made in query dlg                   |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Output parameter:  some flags and numbers are set in ida                    |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     get status for searching in source or target segments    |
//|                   if searching in source                                   |
//|                      get status of more translation checkbox               |
//|                   else                                                     |
//|                      search for more translations is FALSE                 |
//|                   get status of case sensitive/insensitive search          |
//|                   get status if searching using wildcard or not            |
//|                   get status if date range has to be recognized for search |
//|                   get status if translations should result from spec. doc  |
//|                   get number of entries to be loaded in editor             |
//|                      check the entered number if it is a digit             |
//|                      if it is no digit or too small                        |
//|                         fOK = FALSE                                        |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+
BOOL  GetQuerySettings
(
   HWND            hwndDlg,            // handle of dialog window
   PTMIDA          pTMIda              // ida of query
)
{
   CHAR            szSet[10];          // number queried from EF
   BOOL            fOK = TRUE;         // success indicator
   USHORT          i = 0;              // index counter
   UINT            uiBuffer;           // number conversion buffer

   pTMIda -> uiErrCode = 0;

   // search for a specific segment number
   QUERYTEXT(hwndDlg, ID_TM_QUERY_SEARCH_NUM_EF, szSet);
   if (strlen(szSet) > 5) fOK = FALSE;
   for (i = 0; fOK && i < strlen(szSet); i++) fOK = isdigit(szSet[i]);
   if (fOK) {
     uiBuffer = atoi(szSet);
     if (uiBuffer > 65535) fOK = FALSE;
     else pTMIda -> ulSearchSegNum = (USHORT) uiBuffer;
   }
   if (!fOK) pTMIda -> uiErrCode = ERROR_TMM_SEARCHSEGNUM;

   // get status if source or target entries have to be searched for
   pTMIda->fLookup = QUERYCHECK( hwndDlg, ID_TM_QUERY_SOURCE_RB );

   if ( pTMIda->fLookup )
   {
     // lookup in source entries, so get status if
     // only matches are searched for that contain more
     // than one translation
     pTMIda->fMoreTrans = QUERYCHECK( hwndDlg, ID_TM_QUERY_XTRANS_CB );
   }
   else
   {
     // lookup in target entries, so set "more trans" to FALSE
     pTMIda->fMoreTrans = FALSE;
   } /* endif */

   // respect or ignore case
   pTMIda->fCase = QUERYCHECK( hwndDlg, ID_TM_QUERY_RESP_RB );

   // search exact or use wildcard
   pTMIda->fExact = !QUERYCHECK( hwndDlg, ID_TM_WILD_SEARCH_CB );

   // search in specific date range
   pTMIda->fDate = QUERYCHECK(hwndDlg, ID_TM_QUERY_DATE_CB);

   // search entries resulting from sepcific document
   pTMIda->fDoc = QUERYCHECK(hwndDlg, ID_TM_QUERY_DOC_CB);

   // search for M-flagged segments
   pTMIda -> fMPlus = QUERYCHECK(hwndDlg, ID_TM_QUERY_MPLUS_CB);

   // search for segments w/o M-flag
   pTMIda -> fMMinus = QUERYCHECK(hwndDlg, ID_TM_QUERY_MMINUS_CB);

   // how many entries should be loaded into the editor
   QUERYTEXT( hwndDlg, ID_TM_QUERY_SEGNUM_EF, szSet );

   // check the entered number if it is a digit and not less than 5
   while ( fOK && (szSet[i] != EOS) )
   {
     fOK = isdigit( szSet[i] );
     i++;
   } /* endwhile */

   if ( fOK )
   {
     // set number to load in ida
     pTMIda->ulSeg2Load = (atol( szSet ));
     if ( (pTMIda->ulSeg2Load < 5) || (pTMIda->ulSeg2Load > MAX_SEGS_PER_LOAD) )
     {
       // don't quit the query dialog
       fOK = FALSE;
     } /* endif */
   } /* endif */
   if (!fOK && !pTMIda -> uiErrCode) pTMIda -> uiErrCode = ERROR_TMM_LOAD_TOO_SMALL;

   // get number of surrounding segments to display
   if (fOK) {
     QUERYTEXT(hwndDlg, ID_TM_QUERY_PMSEGS_EF, szSet);
     if (strlen(szSet) > 1) {
       pTMIda -> uiErrCode = ERROR_TMM_SHOWPMSEGS;
       fOK = FALSE;
     }
     else if (isdigit(szSet[0])) {
       pTMIda -> usPMSegs = (USHORT)(atoi(szSet));
       if (pTMIda -> usPMSegs > 5) {
         pTMIda -> uiErrCode = ERROR_TMM_SHOWPMSEGS;
         fOK = FALSE;
       }
     }
     else if (szSet[0]) {
       pTMIda -> uiErrCode = ERROR_TMM_SHOWPMSEGS;
       fOK = FALSE;
     }
     else pTMIda -> usPMSegs = 0;
   }

   /*******************************************************************/
   /* Get selected markup                                             */
   /*******************************************************************/
   if ( fOK )
   {
     QUERYTEXT( hwndDlg, ID_TM_QUERY_MARKUP_CB, pTMIda->szTagTable );
   } /* endif */

   return( fOK );

} /* end of function GetQuerySettings */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     SetupDateDlg                                             |
//+----------------------------------------------------------------------------+
//|Function call:     SetupDateDlg( hwndDlg, pTMIda );                         |
//+----------------------------------------------------------------------------+
//|Description:       initialize date dialog                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Output parameter:  date is set in ida                                       |
//+----------------------------------------------------------------------------+
//|Function flow:     set inititals in ida and show on dialog                  |
//+----------------------------------------------------------------------------+
VOID SetupDateDlg
(
   HWND            hwndDlg,            // handle of dialog window
   PTMIDA          pTMIda              // ida of query
)
{
   SHORT           sMonth = 0;         // month "counter"
   SHORT           sYear = START_YEAR; // year "counter"
   CHAR            szYear[MAX_MONTH_LEN];   // year string
   LONG            lTime = 0L;         // time in seconds
   struct tm       *timestruct = NULL; // ptr to more detailed time
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   // load month names from resource
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH01, MonthTable[0].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH02, MonthTable[1].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH03, MonthTable[2].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH04, MonthTable[3].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH05, MonthTable[4].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH06, MonthTable[5].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH07, MonthTable[6].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH08, MonthTable[7].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH09, MonthTable[8].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH10, MonthTable[9].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH11, MonthTable[10].szMonthName );
   LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_MONTH12, MonthTable[11].szMonthName );

   // get current time in seconds
   time( &lTime );
   timestruct = localtime( &lTime );
   pTMIda->lCurrentDate = lTime;

   // set months in listboxes
   while ( sMonth < MAXENTRY )
   {
     // insert month names in start month LB
     CBINSERTITEMEND( hwndDlg, ID_TM_DATE_START_MONTH_DL,
                    MonthTable[sMonth].szMonthName );

     // insert month names in end month LB
     CBINSERTITEMEND( hwndDlg, ID_TM_DATE_END_MONTH_DL,
                    MonthTable[sMonth].szMonthName );

     sMonth++;
   } /* endwhile */

   // set years in listboxes
   while ( sYear <= (NINETEEN + timestruct->tm_year) )
   {
     // get string from year
     _itoa( sYear, szYear, 10 );

     // insert year in start listboxes
     CBINSERTITEM( hwndDlg, ID_TM_DATE_START_YEAR_DL, szYear );
     // insert year in end listboxes
     CBINSERTITEM( hwndDlg, ID_TM_DATE_END_YEAR_DL, szYear );

     sYear++;
   } /* endwhile */

   if ( pTMIda->usStartMonth == 0 )
   {
     // set default settings as selected
     CBSELECTITEM( hwndDlg, ID_TM_DATE_START_MONTH_DL, JANUARY );
     CBSELECTITEM( hwndDlg, ID_TM_DATE_END_MONTH_DL, timestruct->tm_mon );

     CBSELECTITEM( hwndDlg, ID_TM_DATE_START_YEAR_DL, 0 );
     CBSELECTITEM( hwndDlg, ID_TM_DATE_END_YEAR_DL, (sYear - 1 - START_YEAR) );
   }
   else
   {
     // set settings from ida as selected
     CBSELECTITEM( hwndDlg, ID_TM_DATE_START_MONTH_DL, (pTMIda->usStartMonth - 1));
     CBSELECTITEM( hwndDlg, ID_TM_DATE_END_MONTH_DL, (pTMIda->usEndMonth - 1));

     CBSELECTITEM( hwndDlg, ID_TM_DATE_START_YEAR_DL, (pTMIda->usStartYear - START_YEAR) );
     CBSELECTITEM( hwndDlg, ID_TM_DATE_END_YEAR_DL, (pTMIda->usEndYear - START_YEAR) );
   } /* endif */

   return;

} /* end of function SetupDateDlg */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QueryDateSetting                                         |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = QueryDateSetting( hwndDlg, pTMIda );               |
//+----------------------------------------------------------------------------+
//|Description:       query the set date                                       |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Output parameter:  date is set in ida                                       |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     query settings from dialog and set in ida                |
//+----------------------------------------------------------------------------+
BOOL  QueryDateSetting
(
   HWND            hwndDlg,            // handle of dialog window
   PTMIDA          pTMIda              // ida of query
)
{
   BOOL            fOK = TRUE;         // success indicator
   struct tm       timestruct;         // detailed time structure

   // get start date
   pTMIda->usStartMonth =
          (USHORT)(CBQUERYSELECTION( hwndDlg, ID_TM_DATE_START_MONTH_DL )) + 1;
   pTMIda->usStartYear =
           (USHORT)(CBQUERYSELECTION( hwndDlg, ID_TM_DATE_START_YEAR_DL )) +
                                                                  START_YEAR;
   // get end date
   pTMIda->usEndMonth =
           (USHORT)(CBQUERYSELECTION( hwndDlg, ID_TM_DATE_END_MONTH_DL )) + 1;
   pTMIda->usEndYear =
           (USHORT)(CBQUERYSELECTION( hwndDlg, ID_TM_DATE_END_YEAR_DL )) +
                                                                  START_YEAR;
   // get start date in seconds
   memset( &timestruct, 0, sizeof(timestruct) );
   timestruct.tm_year = (SHORT)(pTMIda->usStartYear - NINETEEN);
   timestruct.tm_mon = (SHORT)(pTMIda->usStartMonth - 1);
   timestruct.tm_mday = 1;
   pTMIda->lStartDate = mktime( &timestruct );

   // get end date in seconds
   timestruct.tm_year = (SHORT)(pTMIda->usEndYear - NINETEEN);
   // take full end month
   timestruct.tm_mon = (SHORT)(pTMIda->usEndMonth);
   timestruct.tm_mday = 0;
   pTMIda->lEndDate = mktime( &timestruct );

   // check if a valid date is entered
   if ( pTMIda->lEndDate < pTMIda->lStartDate )
   {
     // warning: end date is earlier than start date
     UtlErrorHwnd( ERROR_TMM_EDATE_EARLY, MB_CANCEL,
                   0, NULL, EQF_ERROR, hwndDlg );
     fOK = FALSE;
   }
   else
   {
     if ( pTMIda->lStartDate > pTMIda->lCurrentDate )
     {
       // warning: end date is earlier than start date
       UtlErrorHwnd( ERROR_TMM_START_DATE, MB_CANCEL,
                     0, NULL, EQF_ERROR, hwndDlg );
       fOK = FALSE;
     } /* endif */
   } /* endif */

   return( fOK );

} /* end of function QueryDateSetting */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QueryDocSetting                                          |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = QueryDocSetting( hwndDlg, pTMIda );                |
//+----------------------------------------------------------------------------+
//|Description:       query the entered document name                          |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND          hwndDlg           handle of dialog window  |
//|                   PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Output parameter:  name is set in ida                                       |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     query entered text                                       |
//|                   check if it is a valid document name                     |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+
BOOL  QueryDocSetting
(
   HWND            hwndDlg,            // handle of dialog window
   PTMIDA          pTMIda              // ida of query
)
{
   BOOL            fOK = TRUE;         // success indicator
   int             iNumEntries, i;
   HWND            hLB;
   LONG            lListSize = 0;
   CHAR            szBuf[MAX_LONGFILESPEC];

   pTMIda -> fDocListGiven = QUERYCHECK(hwndDlg, ID_TM_QUERY_DOC_FILES_RB);
   if (pTMIda -> fDocListGiven) {
     hLB = GetDlgItem(hwndDlg, ID_TM_QUERY_DOC_FILES_LB);
     iNumEntries = SendMessage(hLB, LB_GETCOUNT, 0, 0);
     if (!iNumEntries) {
       // TBD: error handling (user error: no selection)
     }
     for (i = 0; i < iNumEntries; i++) {
       if (SendMessage(hLB, LB_GETSEL, (WPARAM) i, 0)) {
         lListSize += SendMessage(hLB, LB_GETTEXTLEN, (WPARAM) i, 0) + 1;
       }
     }
     lListSize++;
     if ( pTMIda->pszDoc2SearchUprList ) UtlAlloc((PVOID *) &(pTMIda->pszDoc2SearchUprList), 0L, 0L, NOMSG );
     UtlAlloc((PVOID *) &(pTMIda -> pszDoc2SearchUprList), 0, lListSize, ERROR_STORAGE);
     memset((void *) pTMIda -> pszDoc2SearchUprList, 0, lListSize);
     for (i = 0; i < iNumEntries; i++) {
       if (SendMessage(hLB, LB_GETSEL, (WPARAM) i, 0)) {
         SendMessage(hLB, LB_GETTEXT, (WPARAM) i, (LPARAM) szBuf);
         UtlUpper( szBuf );   // set individual file names to uppercase - uppercasing list destroys X15 character
         strcat(pTMIda -> pszDoc2SearchUprList, szBuf);
         pTMIda -> pszDoc2SearchUprList[strlen(pTMIda -> pszDoc2SearchUprList)] = X15;
       }
     }
   }

   memset( pTMIda->szBuffer, 0, sizeof(pTMIda->szBuffer) );

   //query the entered text
   QUERYTEXT( hwndDlg, ID_TM_QUERY_DOC_EF, pTMIda->szBuffer );
   UtlStripBlanks( pTMIda->szBuffer );

   if ( pTMIda->szBuffer[0] != EOS )
   {
   }
   else
   {
     // no document name entered, set default
     strcpy( pTMIda->szBuffer, DEFAULT_PATTERN );
     fOK = TRUE;
   } /* endif */

   if ( fOK )
   {
     // set document to be searched for in ida
     strcpy( pTMIda->szDoc2Search, pTMIda->szBuffer );
     strcpy( pTMIda->szDoc2SearchUpr, pTMIda->szDoc2Search );
     UtlUpper( pTMIda->szDoc2SearchUpr );

     //dismiss dialog
     WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT( fOK ), NULL );
   } /* endif */

   return( fOK );

} /* end of function QueryDocSetting */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QueryTMEntry                                             |
//+----------------------------------------------------------------------------+
//|Function call:     QueryTMEntry( pTMIda );                                  |
//+----------------------------------------------------------------------------+
//|Description:       queries one entry from TM and returns to editor          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Output parameter:  entries and belonging settings are set in Ida            |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     set structures for TmExtract                             |
//|                   extract next segment from TM                             |
//|                   if extract went well                                     |
//|                      check extracted segment against query settings        |
//|                   else                                                     |
//|                      if no more entry in TM to be extracted                |
//|                         set fOK TRUE                                       |
//|                         set percentage to 100                              |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+
BOOL  QueryTMEntry
(
   PTMIDA          pTMIda              // ida of query
)
{
   BOOL            fOK = TRUE;         // success indicator
   USHORT          usExtractRC = 0;    // returncode from TmExtract

   // get first/next entry from memory
   OtmProposal proposal;
   int iProgress = 0;
   if ( !pTMIda->fPrevious )
   {
      usExtractRC = (USHORT)pTMIda->pMemory->getFirstProposal( proposal,&iProgress );
      pTMIda->fPrevious = TRUE;
   }
   else
   {
      usExtractRC = (USHORT)pTMIda->pMemory->getNextProposal( proposal, &iProgress);
   } 
   fOK = (usExtractRC==0?TRUE:FALSE);
   
   if ( fOK )
   {
       // count extracted segments
        pTMIda->ulExtracted++;
		
       if ( fOK )
       {
         // set percentage of currently processed TM
         pTMIda->lPercent = min( iProgress, 100L ); 
         // check the extracted segment pair against query settings
         fOK = GetAndCompareSegment( pTMIda, proposal );
       } /* endif */
    }
    else
    {
       // set fStop in any case
       pTMIda->fStop = TRUE;

       if ( usExtractRC == OtmMemory::INFO_ENDREACHED )
       {
         // no more entry in TM
         // so fOK is TRUE and it can be continued
         fOK = TRUE;
         pTMIda->lPercent = 100L;
       } /* endif */
    } /* endif */

   return( fOK );
} /* end of function QueryTMEntry */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetAndCompareSegment                                     |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = GetAndCompareSegment( pTMIda, pstExtOut );         |
//+----------------------------------------------------------------------------+
//|Description:       compare extracted segment against query settings         |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMIDA        pTMIda            ida of query             |
//|                   PEXT_OUT      pstExtOut         output from TmExtract    |
//+----------------------------------------------------------------------------+
//|Output parameter:  flags in ida are updated                                 |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     copy extracted segments to ida                           |
//|                   copy extracted segments into temp strings                |
//|                   norm temp strings                                        |
//|                   check extracted segments against query settings          |
//|                   if query settings match                                  |
//|                      pTMIda->fFound = TRUE                                 |
//|                   else                                                     |
//|                      pTMIda->fFound = FALSE                                |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+
BOOL  GetAndCompareSegment
(
   PTMIDA          pTMIda,             // ida of query
   OtmProposal     &prop               // current proposal data
)
{
   BOOL            fOK = TRUE;         // success indicator
   BOOL            fSkip = FALSE;      // skip current segment

   pTMIda->fFound = FALSE;

   // extract proposal data
   prop.getSource( pTMIda->szSourceSeg, sizeof(pTMIda->szSourceSeg)/sizeof(CHAR_W) );
   prop.getTarget( pTMIda->szTargetSeg, sizeof(pTMIda->szTargetSeg)/sizeof(CHAR_W) );
   prop.getContext( pTMIda->szContext, sizeof(pTMIda->szContext)/sizeof(CHAR_W) );
   prop.getDocShortName( pTMIda->szSourceDoc, sizeof(pTMIda->szSourceDoc) );
   prop.getDocName(pTMIda->szLongName, sizeof(pTMIda->szLongName) );
   prop.getTargetLanguage(pTMIda->szTargetLng, sizeof(pTMIda->szTargetLng) );
   prop.getMarkup( pTMIda->szSegMarkup, sizeof(pTMIda->szSegMarkup) );
   pTMIda->ulSegNum = prop.getSegmentNum();
   pTMIda->usTranslationFlag = ProposalTypeToFlag(prop.getType());;
   pTMIda->lDate = prop.getUpdateTime();
   prop.getInternalKey( pTMIda->szProposalKey, sizeof(pTMIda->szProposalKey) );

   fOK = CheckForMatchingSegment( pTMIda, &fSkip );

   // check more-than-one-translation condition if requested
   if ( fOK && !fSkip && pTMIda->fMoreTrans )
   {
     if ( wcscmp ( pTMIda->szSourceSeg, pTMIda->szLastSourceSeg ) == 0 )
     {
       // O.K.: 2nd or more translation of same source
       // check for multiple target segments has been performed during the first
       // segment of this group

       fSkip = pTMIda->fSkipThisGroup; // use our skip flag for this group
     }
     else 
     {
       OtmProposal LookAheadProp;
       int iProgress = 0;

       // O.K.: 1st translation, more segments from same source may be following
       wcscpy( pTMIda->szLastSourceSeg, pTMIda->szSourceSeg );
       pTMIda->fSkipThisGroup = TRUE;

       // New: Look-ahead to check if the following segments have different
       //      translations. The following translations are from the same source
       //      and may have the same translations

       // save sequential access position
       pTMIda->pMemory->getSequentialAccessKey( pTMIda->szCurrentKey, sizeof(pTMIda->szCurrentKey) );

       // loop over the next proposals with the same source string and check translations
       BOOL fContinue = TRUE;
       do
       {
         LookAheadProp.clear();

         int iSuccess = pTMIda->pMemory->getNextProposal( LookAheadProp, &iProgress );

         //usExtractRC = pMemoryDB->extractW(pTMIda->szFileName,pstExtIn, pstExtOut, TRUE );
         fContinue = (iSuccess == 0);

         // check if next proposal has the same source
         if ( fContinue )
         {
           LookAheadProp.getSource( pTMIda->szTempSourceSeg, sizeof(pTMIda->szTempSourceSeg)/sizeof(CHAR_W) );
           if ( wcscmp( pTMIda->szSourceSeg, pTMIda->szTempSourceSeg ) != 0 )
           {
             // source test has changed so leave the loop
             fContinue = FALSE;
           }
         }

         // check if proposal has a different translation
         if ( fContinue )
         {
           LookAheadProp.getTarget( pTMIda->szTempSourceSeg, sizeof(pTMIda->szTempSourceSeg)/sizeof(CHAR_W) );
           if ( wcscmp( pTMIda->szTargetSeg, pTMIda->szTempSourceSeg ) != 0 )
           {
             pTMIda->fSkipThisGroup = FALSE;   // there are different translations in group
           } /* endif */
         }
       } while ( fOK && pTMIda->fSkipThisGroup && fContinue );

       fSkip = pTMIda->fSkipThisGroup;

       // resume old position in sequential access
       pTMIda->pMemory->setSequentialAccessKey( pTMIda->szCurrentKey );

     } /* endif */

   } /* endif */

   if ( fOK )
   {
     if ( !fSkip )
     {
       // one more entry to add in editor
       pTMIda->ulRunNum++;
       pTMIda->fFound = TRUE;
     } /* endif */
   } /* endif */

   return( fOK );

} /* end of function GetAndCompareSegment */

BOOL CheckForMatchingSegment
(
   PTMIDA          pTMIda,             // ida of query
   PBOOL           pfSkip              // pointer to callers segment skip flag
)
{
   BOOL fOK = TRUE;

   if ( fOK )
   {
     /*****************************************************************/
     /* Check if segment is for the currently selected markup         */
     /*****************************************************************/
     if ( strcmp( pTMIda->szTagTable, pTMIda->szSegMarkup ) != 0 )
     {
       *pfSkip = TRUE;                   // skip segment with wrong markup
     } /* endif */

     /*****************************************************************/
     /* if a check for date should be make                            */
     /* skip the segments which date is not in the range              */
     /*****************************************************************/
     if ( !*pfSkip && pTMIda->fDate )
     {
       if ( (pTMIda->lDate > pTMIda->lEndDate) ||
            (pTMIda->lDate < pTMIda->lStartDate) )
       {
         // trans date of segment not in query range
         *pfSkip = TRUE;
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* if resulting translations should come from specified doc      */
     /* compare document from which current translation results       */
     /* against entered search doc                                    */
     /*****************************************************************/
     if ( !*pfSkip && pTMIda->fDoc )
     {
       BOOL fMatch = FALSE;

       if ( pTMIda->fDocListGiven )
       {
          // check document name against names in our list

          PSZ pszNextX15;        // position of next X15 char in string
          PSZ pszCurPos;               // current position within string

          strcpy( pTMIda->szBuffer, pTMIda->szLongName );
          UtlUpper( pTMIda->szBuffer );

          pszCurPos = pTMIda->pszDoc2SearchUprList;
          while ( !fMatch && (*pszCurPos != EOS) )
          {
            pszNextX15 = strchr( pszCurPos, X15 );
            if ( pszNextX15 ) *pszNextX15 = EOS;
            fMatch = (strcmp( pTMIda->szBuffer, pszCurPos ) == 0);
            if ( pszNextX15 )
            {
              *pszNextX15 = X15;
              pszCurPos = pszNextX15 + 1;
            }
            else
            {
              pszCurPos = pszCurPos + strlen(pszCurPos);
            } /* endif */
          } /* endwhile */
       }
       else
       {

         if ( !fMatch )
         {
           strcpy( pTMIda->szBuffer, pTMIda->szLongName );
           fMatch = WildCompare( UtlUpper(pTMIda->szBuffer),
                                 pTMIda->szDoc2SearchUpr );
         } /* endif */

         if ( !fMatch )
         {
           fMatch = WildCompare( UtlUpper(pTMIda->szSourceDoc),
                                 pTMIda->szDoc2SearchUpr );
         } /* endif */
       } /* endif */

       *pfSkip = !fMatch;
     } /* endif */

     // check m-flag search flags
     if ( !*pfSkip )
     {
        if ( pTMIda->fMPlus && (pTMIda->usTranslationFlag == TRANSLFLAG_NORMAL) )
        {
          // skip segment, M-flag is not set
          *pfSkip = TRUE;
        }
        else if ( pTMIda->fMMinus && (pTMIda->usTranslationFlag != TRANSLFLAG_NORMAL)  )
        {
          // skip segment, M-flag is set
          *pfSkip = TRUE;
        } /* endif */
     } /* endif */

     // check segment number
     if ( !*pfSkip && (pTMIda->ulSearchSegNum != 0) )
     {
        if ((USHORT) pTMIda->ulSearchSegNum !=  pTMIda->ulSegNum )
        {
          // skip segment, segment number does not match
          *pfSkip = TRUE;
        } /* endif */
     } /* endif */

   } /* endif */

   // check for string
   if ( fOK && !*pfSkip )
   {
     /*****************************************************************/
     /* if a search string was entered do compare                     */
     /*****************************************************************/
     if ( pTMIda->szSearchString[0] != EOS )
     {
       UTF16strcpy( pTMIda->szSourceSegBuf, pTMIda->szSourceSeg );
       CutCR_LF( pTMIda->szSourceSegBuf, FALSE, pTMIda->ulSystemPrefCP );
       UTF16strcpy( pTMIda->szTargetSegBuf, pTMIda->szTargetSeg );
       CutCR_LF( pTMIda->szTargetSegBuf, FALSE, pTMIda->ulSystemPrefCP );

       // compare search string against extracted entry
       *pfSkip = CompareSearch2Seg( pTMIda, pTMIda->szSourceSegBuf, pTMIda->szTargetSegBuf );
     } /* endif */
   } /* endif */
   return( fOK );
} /* end of function CheckForMatchingSegment */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CompareSearch2Seg                                        |
//+----------------------------------------------------------------------------+
//|Function call:    fSkip = ComapreSearch2Seg( pTMIda, pSourceSeg, pTargetSeg)|
//+----------------------------------------------------------------------------+
//|Description:       compares search and extracted string                     |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMIDA        pTMIda            ida of query             |
//|                   PSZ           pSourceSeg                                 |
//|                   PSZ           pTargetSeg                                 |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
BOOL CompareSearch2Seg
(
   PTMIDA          pTMIda,
   PSZ_W           pSourceSeg,
   PSZ_W           pTargetSeg
)
{
   BOOL            fSkip = FALSE;

   if ( pTMIda->fExact )
   {
     if ( pTMIda->fLookup )
     {
       // compare with source
       if ( pTMIda->fCase )
       {
         fSkip = ExactCompare( pSourceSeg, pTMIda->szSearchString );
       }
       else
       {
         fSkip = ExactCompare( UtlUpperW(pSourceSeg),
                               UtlUpperW(pTMIda->szSearchString) );
       } /* endif */
       pTMIda->fFound = !fSkip;
     }
     else
     {
       // compare with target
       if ( pTMIda->fCase )
       {
         fSkip = ExactCompare( pTargetSeg, pTMIda->szSearchString );
       }
       else
       {
         fSkip = ExactCompare( UtlUpperW(pTargetSeg),
                               UtlUpperW(pTMIda->szSearchString) );
       } /* endif */
       pTMIda->fFound = !fSkip;
     } /* endif */
   }
   else
   {
     if ( pTMIda->fLookup )
     {
       // compare with source
       if ( pTMIda->fCase )
       {
         fSkip = !WildCompareW( pSourceSeg, pTMIda->szSearchString );
       }
       else
       {
         fSkip = !WildCompareW( UtlUpperW(pSourceSeg),
                               UtlUpperW(pTMIda->szSearchString) );
       } /* endif */
       pTMIda->fFound = !fSkip;
     }
     else
     {
       // compare with target
       if ( pTMIda->fCase )
       {
         fSkip = !WildCompareW( pTargetSeg, pTMIda->szSearchString );
       }
       else
       {
         fSkip = !WildCompareW( UtlUpperW(pTargetSeg),
                                UtlUpperW(pTMIda->szSearchString) );
       } /* endif */
       pTMIda->fFound = !fSkip;
     } /* endif */
   } /* endif */

   return( fSkip );

} /* end of function CompareSearch2Seg */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ExactCompare                                             |
//+----------------------------------------------------------------------------+
//|Function call:     fSkip = ExactCompare( pTMIda, pString )                  |
//+----------------------------------------------------------------------------+
//|Description:       compares if search string is equal to segment            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMIDA        pTMIda            ida of query             |
//|                   PSZ           pSegString                                 |
//+----------------------------------------------------------------------------+
//|Returns:           BOOL          fOK       TRUE           - success         |
//|                                           FALSE          - error           |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
BOOL ExactCompare
(
   PSZ_W           pSeg,
   PSZ_W           pSearch
)
{
   BOOL            fSkip = FALSE;

   if ( (UTF16strcmp( pSeg, pSearch ) == 0) )
   {
     fSkip = FALSE;
   }
   else
   {
     fSkip = TRUE;
   } /* endif */

   return( fSkip );

} /* end of function ExactCompare */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     WildCompare                                              |
//+----------------------------------------------------------------------------+
//|Function call:     fSkip = WildCompare( pSeg, pSearch )                     |
//+----------------------------------------------------------------------------+
//|Description:       compares if search string is equal to segment            |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ           pSeg                                       |
//|                   PSZ           pSearch                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
BOOL WildCompareW
(
   PSZ_W           pSeg,
   PSZ_W           pSearch
)
{
   USHORT          i = 0;

   if ( (*pSearch == EOS) && (*pSeg == EOS) )
   {
     return( 1 );
   } /* endif */

   if ( *pSearch == EOS )
   {
     return( 0 );
   } /* endif */

   if ( *pSearch == '*' )
   {
     if ( *(pSearch + 1) == EOS )
     {
       return( 1 ) ;
     } /* endif */

     for( i=0 ; i <= UTF16strlenCHAR(pSeg); i++ )
     {
       if ( (*(pSeg+i) == *(pSearch+1)) || (*(pSearch+1) == '?') )
       {
         if ( WildCompareW( pSeg+i+1, pSearch+2 ) == 1 )
         {
           return( 1 );
         } /* endif */
       } /* endif */
     } /* endfor */
   }
   else
   {
     if (*pSeg == '\0')
     {
       return( 0 );
     } /* endif */

     if ((*pSearch == '?') || (*pSearch == *pSeg))
     {
       if (WildCompareW( pSeg+1, pSearch+1 ) == 1)
       {
         return( 1 );
       } /* endif */
     } /* endif */
   } /* endif */

   return( 0 ) ;

} /* end of function WildCompare */

BOOL WildCompare
(
   PSZ           pSeg,
   PSZ           pSearch
)
{
   USHORT          i = 0;

   if ( (*pSearch == EOS) && (*pSeg == EOS) )
   {
     return( 1 );
   } /* endif */

   if ( *pSearch == EOS )
   {
     return( 0 );
   } /* endif */

   if ( *pSearch == '*' )
   {
     if ( *(pSearch + 1) == EOS )
     {
       return( 1 ) ;
     } /* endif */

     for( i=0 ; i <= strlen(pSeg); i++ )
     {
       if ( (*(pSeg+i) == *(pSearch+1)) || (*(pSearch+1) == '?') )
       {
         if ( WildCompare( pSeg+i+1, pSearch+2 ) == 1 )
         {
           return( 1 );
         } /* endif */
       } /* endif */
     } /* endfor */
   }
   else
   {
     if (*pSeg == '\0')
     {
       return( 0 );
     } /* endif */

     if ((*pSearch == '?') || (*pSearch == *pSeg))
     {
       if (WildCompare( pSeg+1, pSearch+1 ) == 1)
       {
         return( 1 );
       } /* endif */
     } /* endif */
   } /* endif */

   return( 0 ) ;

} /* end of function WildCompare */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FreeTMIda                                                |
//+----------------------------------------------------------------------------+
//|Function call:     FreeTMIda( pTMIda )                                      |
//+----------------------------------------------------------------------------+
//|Description:       free memory that was allocate in TMIda                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
VOID FreeTMIda
(
  PTMIDA           pTMIda
)
{
   if ( pTMIda->pAddr ) UtlAlloc( (PVOID *)&pTMIda->pAddr, 0L, 0L, NOMSG );

   if ( pTMIda->pPool ) PoolDestroy( pTMIda->pPool );
   if ( pTMIda->ppLongNames ) UtlAlloc( (PVOID *)&pTMIda->ppLongNames, 0L, 0L, NOMSG );
   if ( pTMIda->ppTagTables ) UtlAlloc( (PVOID *)&pTMIda->ppTagTables, 0L, 0L, NOMSG );
   if ( pTMIda->ppDocuments ) UtlAlloc( (PVOID *)&pTMIda->ppDocuments, 0L, 0L, NOMSG );
   if ( pTMIda->ppLanguages ) UtlAlloc( (PVOID *)&pTMIda->ppLanguages, 0L, 0L, NOMSG );
   if (pTMIda -> pszDoc2SearchUprList) UtlAlloc((PVOID *) &(pTMIda -> pszDoc2SearchUprList), 0, 0, NOMSG);

   // close TM in any case
   if ( pTMIda->pMemory != NULL )
   {
     MemoryFactory *pFactory = MemoryFactory::getInstance();
     pFactory->closeMemory( pTMIda->pMemory );
     pTMIda->pMemory = NULL;
     pMemoryDB = NULL;
   } /* endif */

   return;
} /* end of function FreeTMIda */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FreeTMIdaForSave                                         |
//+----------------------------------------------------------------------------+
//|Function call:     FreeTMIdaForSave( pTMIda );                              |
//+----------------------------------------------------------------------------+
//|Description:       free some memory and set some settings to 0              |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMIDA        pTMIda            ida of query             |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
VOID FreeTMIdaForSave
(
  PTMIDA           pTMIda
)
{
   // free ptr containing addresses
   if ( pTMIda->pAddr ) UtlAlloc( (PVOID *)&(pTMIda->pAddr), 0L, 0L, NOMSG );

   memset( &(pTMIda->NextSegAddr), 0, sizeof(TM_ADDRESS) );

   pTMIda->ulSegNum = 0;
   pTMIda->ulRunNum = 0;
   pTMIda->ulExtracted = 0;
   pTMIda->ulAddrCount = 0;
   pTMIda->ulAddrAlloc = 0;
   pTMIda->ulPrevPos = 0;
   pTMIda->lPercent = 0L;
   pTMIda->fPrevious = FALSE;
   pTMIda->fFound = FALSE;
   pTMIda->fNorm = FALSE;
   pTMIda->fStop = FALSE;
   pTMIda->fRun = FALSE;
   memset( pTMIda->szPrevSourceSeg, 0, sizeof(pTMIda->szPrevSourceSeg) );
   memset( pTMIda->szPrevTargetSeg, 0, sizeof(pTMIda->szPrevTargetSeg) );

   return;

} /* end of function FreeTMIdaForSave */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CutCR_LF                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     CutCR_LF( pString, fCR );                                |
//+----------------------------------------------------------------------------+
//|Description:       get rid of CR LF in string                               |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ           pString                                    |
//|                   BOOL          fCR                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
VOID CutCR_LF
(
   PSZ_W           pString,
   BOOL            fCR,
   ULONG           ulCP
)
{
   USHORT          i = 0;
   PSZ_W           pTemp;
   CHAR_W          c;
   CHAR_W          d;

   pTemp = pString;

   while ( (c = *pTemp) != EOS )
   {
     d = *(pTemp + 1);

     if ( fCR && (c == CR) )
     {
       pTemp+=1;
     }
     else if ( (c == LF) && (d == EOS) )
     {
       pString[i] = EOS;
       pTemp+=1;
     }
     else if ( (c == LF) && (d != EOS) )
     {
       if ( (IsDBCS_CP(ulCP)) &&
            (i>0) && EQFIsDBCSChar(pString[i-1], ulCP) && EQFIsDBCSChar(pString[i+1], ulCP))
       {
         // ignore LF in DBCS case..
         pTemp += 1;
       }
       else
       {
         pString[i++] = BLANK;
         pTemp+=1;
       }
     }
     else
     {
       pString[i++] = *pTemp;
       pTemp+=1;
     } /* endif */
   } /* endwhile */

   pString[i] = EOS;

   return;

} /* end of function CutCR_LF */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsterixCR_LF                                             |
//+----------------------------------------------------------------------------+
//|Function call:     AsterixCR_LF( pString );                                 |
//+----------------------------------------------------------------------------+
//|Description:       get rid of CR LF and "**" in search string               |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ           pString                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
VOID AsterixCR_LF
(
   PSZ_W           pString
)
{
   USHORT          i = 0;
   PSZ_W           pTemp;
   CHAR_W          c;
   CHAR_W          d;
   BOOL            fCRLF = TRUE;

   pTemp = pString;

   if ( *pTemp != '*' )
   {
     memmove( (PBYTE)(pString + 1), (PBYTE)pTemp, (UTF16strlenCHAR(pTemp) + 1)*sizeof(CHAR_W) );
     pTemp+=1;
     pString[i++] = '*';
   } /* endif */

   while ( (c = *pTemp) != EOS )
   {
     d = *(pTemp + 1);

     if ( c == CR )
     {
       pTemp+=1;
     }
     else if ( (c == LF) && (d == EOS) )
     {
       pString[i] = EOS;
       pTemp+=1;
     }
     else if ( (c == LF) && (d != EOS) )
     {
       pString[i++] = BLANK;
       pTemp+=1;
     }
     else if ( (c == '*') && (d == '*') )
     {
       pTemp+=1;
     }
     else
     {
       if ( c != BLANK )
       {
         fCRLF = FALSE;
       } /* endif */
       pString[i++] = c;
       pTemp+=1;
     } /* endif */
   } /* endwhile */

   if ( pString[i - 1] != '*' )
   {
     // if last char not already '*'
     pString[i++] = '*';
   } /* endif */
   pString[i] = EOS;

   if ( fCRLF )
   {
     // string contains only CRLFs and/or blanks
     pString[0] = '*';
     pString[1] = EOS;
   } /* endif */

   return;

} /* end of function AsterixCR_LF */




VOID PASCAL FAR TMCleanUp( USHORT usTermCode )
{
  usTermCode;

  if ( pMemoryDB != NULL)
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory( pMemoryDB  );
	pMemoryDB = NULL;
  } /* endif */

  DosExitList(EXLST_EXIT, 0L);        /* termination complete     */
}


/**********************************************************************/
/* Get a list of documents, tagtables or languges from a TM           */
/**********************************************************************/
BOOL TMMGetListFromTM
(
  PTMIDA      pTMIda,                  // ptr ot TMM Ida
  TMMTABLE    Table                    // ID of table
)
{
  BOOL            fOK = TRUE;                // function return code
  PSZ             (*ppTable)[];       // ptr to ptr for table processing
  PSHORT          psElements;         // ptr to number of elements in the table
  CHAR            szTempName[MAX_LONGPATH+MAX_LONGFILESPEC];
  PSZ             pszString = NULL;
  int iElements = 0;                  // number of elements to extract from memory
   
  /********************************************************************/
  /* Set variables for given table                                    */
  /********************************************************************/
  switch ( Table )
  {
    case TMM_TAGTABLES :
      ppTable       = pTMIda->ppTagTables;
      psElements    = &pTMIda->sTagTables;
      iElements     = pTMIda->pMemory->getNumOfMarkupNames();
      break;
    case TMM_DOCUMENTS :
      ppTable       = pTMIda->ppDocuments;
      psElements    = &pTMIda->sDocuments;
      iElements     = pTMIda->pMemory->getNumOfDocumentShortNames();
      break;
    case TMM_LANGUAGES :
      ppTable       = pTMIda->ppLanguages;
      psElements    = &pTMIda->sLanguages;
      iElements     = pTMIda->pMemory->getNumOfLanguages();
      break;
    case TMM_LONGNAMES :
      ppTable       = pTMIda->ppLongNames;
      psElements    = &pTMIda->sLongNames;
      iElements     = pTMIda->pMemory->getNumOfDocumentNames(); // all document names are now long names
      break;

    default :
      // wrong or not-supported type of table
      return( FALSE );
      break;
  } /* endswitch */

  // allocate buffer for table pointers
  if ( iElements != 0 )
  {
    LONG lNewSize = (LONG)(iElements) * (LONG)sizeof(PSZ_W);
    fOK = UtlAlloc( (PVOID *)&ppTable, 0, max( MIN_ALLOC, lNewSize ), ERROR_STORAGE );
  } /* endif */

  // get the names from the memory 
  if ( fOK )
  {
    int i;

    for ( i = 0; i < iElements; i++ )
    {
      // get current element
      switch ( Table )
      {
        case TMM_TAGTABLES : pTMIda->pMemory->getMarkupName( i, szTempName, sizeof(szTempName) ); break;
        case TMM_DOCUMENTS : pTMIda->pMemory->getDocumentShortName( i, szTempName, sizeof(szTempName) ); break;
        case TMM_LONGNAMES : pTMIda->pMemory->getDocumentName( i, szTempName, sizeof(szTempName) ); break;
        case TMM_LANGUAGES : pTMIda->pMemory->getLanguage( i, szTempName, sizeof(szTempName) ); break;
      } /* endswitch */

      // add element to the table
      pszString = PoolAddString( pTMIda->pPool, szTempName );
      fOK = (pszString != NULL);
      (*ppTable)[i] = pszString;

    } /* endfor */
    *psElements = (SHORT)iElements;
  } /* endif */   

  // set table pointer in IDA
  if ( fOK )
  {
    switch ( Table )
    {
      case TMM_TAGTABLES : pTMIda->ppTagTables = ppTable; break;
      case TMM_DOCUMENTS : pTMIda->ppDocuments = ppTable; break;
      case TMM_LANGUAGES : pTMIda->ppLanguages = ppTable; break;
      case TMM_LONGNAMES : pTMIda->ppLongNames = ppTable; break;
    } /* endswitch */
  } /* endif */

  return( fOK );
} /* end of function TMMGetListFromTM */

/**********************************************************************/
/* Find a name in a name table and returns the index of the name      */
/**********************************************************************/
SHORT TMMFindInTable
(
  PTMIDA      pTMIda,                  // ptr ot TMM Ida
  PSZ         pszName,                 // name being looked up
  TMMTABLE    Table                    // ID of table
)
{
  PSZ         (*ppTable)[];            // ptr for table processing
  SHORT       sElements;               // number of elements in the table
  SHORT           sIndex;              // index of name in table

  /********************************************************************/
  /* Set variables for given table                                    */
  /********************************************************************/
  switch ( Table )
  {
    case TMM_TAGTABLES :
      ppTable  = pTMIda->ppTagTables;
      sElements = pTMIda->sTagTables;
      break;
    case TMM_DOCUMENTS :
      ppTable  = pTMIda->ppDocuments;
      sElements = pTMIda->sDocuments;
      break;
    case TMM_LANGUAGES :
      ppTable   = pTMIda->ppLanguages;
      sElements = pTMIda->sLanguages;
      break;
    case TMM_LONGNAMES :
      ppTable   = pTMIda->ppLongNames;
      sElements = pTMIda->sLongNames;
      break;
    default :
      // wrong or not-supported type of table
      return( NOSTRINGINDEX );
      break;
  } /* endswitch */

  /********************************************************************/
  /* Look for name in selected table                                  */
  /********************************************************************/
  sIndex = 0;
  while ( sIndex < sElements )
  {
    if ( strcmp( (*ppTable)[sIndex], pszName ) == 0 )
    {
      // O.K. we found our name in the tabl
      break;
    }
    else
    {
      sIndex++;
    } /* endif */
  } /* endwhile */

  // check for not-found condition
  if ( sIndex >= sElements )
  {
    sIndex = NOSTRINGINDEX;
  } /* endif */

  return( sIndex );
} /* end of function TMMFindInTable */

/**********************************************************************/
/* Get the name for a given table index                               */
/**********************************************************************/
BOOL TMMGetNameForInd
(
  PTMIDA      pTMIda,                  // ptr ot TMM Ida
  SHORT       sIndex,                  // index of name
  PSZ         pszName,                 // buffer for name
  TMMTABLE    Table                    // ID of table
)
{
  PSZ         (*ppTable)[] = NULL;       // ptr for table processing
  SHORT       sElements = 0;             // number of elements in the table
  BOOL        fOK = TRUE;                // function return code

  /********************************************************************/
  /* Preset caller's name buffer                                      */
  /********************************************************************/
  *pszName = EOS;

  /********************************************************************/
  /* Set variables for given table                                    */
  /********************************************************************/
  switch ( Table )
  {
    case TMM_TAGTABLES :
      ppTable  = pTMIda->ppTagTables;
      sElements = pTMIda->sTagTables;
      break;
    case TMM_DOCUMENTS :
      ppTable  = pTMIda->ppDocuments;
      sElements = pTMIda->sDocuments;
      break;
    case TMM_LANGUAGES :
      ppTable   = pTMIda->ppLanguages;
      sElements = pTMIda->sLanguages;
      break;
    case TMM_LONGNAMES :
      ppTable   = pTMIda->ppLongNames;
      sElements = pTMIda->sLongNames;
      break;
    default :
      // wrong or not-supported type of table
      fOK = FALSE;
      break;
  } /* endswitch */

  /********************************************************************/
  /* Check if given index is range                                    */
  /********************************************************************/
  if ( fOK )
  {
    if ( (sIndex < 0) || (sIndex >= sElements) )
    {
      // index is not valid for selected table
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Get name from selected table                                     */
  /********************************************************************/
  if ( fOK )
  {
    strcpy( pszName, (*ppTable)[sIndex] );
  } /* endif */

  return( fOK );
} /* end of function TMMGetNameForInd */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TMMCorrectLFCount                                        |
//+----------------------------------------------------------------------------+
//|Function call:     TMMCorrectLFCount( string, sLFCount );                   |
//+----------------------------------------------------------------------------+
//|Description:       Corrects the number of LF chars at the end of the data   |
//+----------------------------------------------------------------------------+
//|Parameters:        string   ptr to buffer containing the source string      |
//|                            must be MAX_SEGMENT size large                  |
//|                   sLFCount number of LFs at end of data required           |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
VOID TMMCorrectLFCount
(
  PSZ_W       pszString,               // points to string to be processed
  SHORT       sLFCount                 // number of LFs at end of data required
)
{
  LONG        lLF;                     // number of LFs actually at end of string
  LONG        lLen;                    // length of input string

  lLF = TMMCountLFAtEnd( pszString );
  lLen = UTF16strlenCHAR( pszString );

  if ( lLF < sLFCount )
  {
    // add missing LF characters
    while ( lLF < sLFCount )
    {
      pszString[lLen] = LF;
      lLen++;
      pszString[lLen] = EOS;
      lLF++;
    } /* endwhile */
  }
  else if ( lLF > sLFCount )
  {
    // remove superfluous LF characters
    while ( (lLF > sLFCount) && (lLen > 0) )
    {
      lLen--;
      lLF--;
      pszString[lLen] = EOS;
    } /* endwhile */
  } /* endif */
} /* end of function TMMCorrectLFCount */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TMMCountLFAtEnd                                          |
//+----------------------------------------------------------------------------+
//|Function call:     sLF = TMMCountLFAtEnd( string );                         |
//+----------------------------------------------------------------------------+
//|Description:       Counts the number of LFs at the end of the passed data   |
//+----------------------------------------------------------------------------+
//|Parameters:        string   ptr to buffer containing the source string      |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       number of LFs at end of data                             |
//+----------------------------------------------------------------------------+
LONG TMMCountLFAtEnd
(
  PSZ_W       pszString                // points to string to be processed
)
{
  LONG        lI;                      // loop index
  LONG        lLen;                    // length of string
  LONG        lLF = 0;                 // number of LFs found

  // count number of LFs at end of string
  lLen = UTF16strlenCHAR(pszString);
  lI = lLen - 1;
  while ( lI >= 0)
  {
    if ( pszString[lI] == LF )
    {
      lLF++;
    }
    else
    {
      break;
    } /* endif */
    lI--;
  } /* endwhile */
  return( lLF );
} /* end of function TMMCountLFAtEnd */


USHORT ProposalTypeToFlag(OtmProposal::eProposalType type)
{
  if (type == OtmProposal::eptManual)
     return TRANSLFLAG_NORMAL;
  
  if (type == OtmProposal::eptMachine)
     return TRANSLFLAG_MACHINE;
	 
  if (type == OtmProposal::eptGlobalMemory)
     return TRANSLFLAG_GLOBMEM;
	 
  if (type == OtmProposal::eptGlobalMemoryStar)
     return TRANSLFLAG_GLOBMEMSTAR;
	 
  // error  
  return(TRANSLFLAG_GLOBMEMSTAR + 1);
}