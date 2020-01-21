//+----------------------------------------------------------------------------+
//| EQFAPROF.C                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|   Analysis profile related functions                                       |
//|                                                                            |
//|    AnaProfileDialog                                                        |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_FOLDER           // folder related stuff
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqftai.h"               // Private include file for Text Analysis
#include <eqfiana1.id>            // dialog control IDs
#include <eqfaprof.h>             // analysis profile functions

//#include "EQFDDE.H"               // Batch mode definitions
extern HELPSUBTABLE hlpsubtblAnaProfDlg[];

// max. number of markup table groups
// (do not change without supplying a conversion function for existing profiles)
#define MAX_MU_GROUPS 10

// size of ID buffer
#define ANA_PROF_ID_SIZE 20

// analysis profile identifier
#define ANAPROFILEIDENTIFIER "EQFAPROF\x23\x11"

// name of trigger file activating the protect button
#define ANAPROF_PROTECT_TRIGGER "APRPROT.TRG"

// settings for a markup table group
typedef struct _MUANASETTINGS
{
  BOOL        fInUse;                            // TRUE = this group is in use
  BOOL        fAutoSubst;                        // auto substitution
  BOOL        fSNOMATCH;                         // SNOMATCH creation
  BOOL        fMatchInfo;                        // count TM match info
  BOOL        fRedundancy;                       // prepare redundancy report
  BOOL        fAdjustLeadingWS;                  // adjust leading whitespace
  BOOL        fAdjustTrailingWS;                 // adjust trailing whitespace
  BOOL        fRespectCRLF;                      // respect CRLF
  BOOL        fAutoJoin;                         // automatic segment joining
  BOOL        fNoBlank;                          // no blank at segment end option
  BYTE        bFreeArea[96];                     // free area for enhancements
} MUANASETTINGS, *PMUANASETTINGS;

// table entry for markup table list
typedef struct _MUTLISTENTRY
{
  CHAR        szMUT[MAX_FILESPEC];               // markup table name
  int         iGroup;                            // number of markup table group
} MUTLISTENTRY, *PMUTLISTENTRY;

// analysis profile 
typedef struct _ANAPROFILE
{
  CHAR        szID[ANA_PROF_ID_SIZE];            // profile identifier (mst be the first field in the struture
  BOOL        fProtected;                        // TRUE = profile is protected
  int         iMutTableSize;                     // number of entries in markup table list 
  BYTE        bFreeArea[200];                    // free area for enhancements
  MUANASETTINGS MutSettings[MAX_MU_GROUPS];      // table with markup table group settings
                                                 // the first entry (index 0) is used to store the analysis settings
                                                 // whih are in effect for markup tables not listed in this profile
  MUTLISTENTRY MutEntries[1];                    // array with markup tables
} ANAPROFILE, *PANAPROFILE;

// analysis profile dialog IDA
typedef struct _ANAPROFIDA
{
  CHAR        szProfile[MAX_LONGFILESPEC];      // profile name
  CHAR        szProfPath[MAX_LONGFILESPEC];     // fully qualified profile file
  CHAR        szBuffer[2048];                   // general purpose buffer
  PANAPROFILE pProf;                            // points to loaded profile
  int         iCurGroup;                        // currently active markup group
  BOOL        fInit;                            // TRUE = initalization in progress
} ANAPROFIDA, *PANAPROFIDA;


// internal prototypes
INT_PTR CALLBACK ANAPROFILEDLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );


// 
// Externally called functions
//



// apply the settings of a profile
BOOL AnaProfApplyProfile( LONG hProfile, PVOID pvTAIn, PSZ pszMarkup, BOOL fCombineFlags )
{
  BOOL fOK = TRUE;
  PTAINPUT pTAIn = (PTAINPUT)pvTAIn;
  int iGroup = 0;

  PANAPROFILE pProf = (PANAPROFILE)hProfile;

  // return when no profile is active
  if ( hProfile == 0 ) 
    return( TRUE );

  // check profile
  if ( strcmp( pProf->szID, ANAPROFILEIDENTIFIER ) != 0 )
  {
    fOK = FALSE;
  } /* endif */

  // save active analysis options in entry 0 of the MUT settings
  if ( fOK )
  {
    PMUANASETTINGS pEntry = &(pProf->MutSettings[0]);
    if ( !pEntry->fInUse )
    {
      pEntry->fInUse = TRUE;
      pEntry->fAutoSubst        = pTAIn->fInsertTMMatches;
      pEntry->fAdjustLeadingWS  = pTAIn->fLeadingWS;
      pEntry->fAdjustTrailingWS = pTAIn->fTrailingWS;
      pEntry->fAutoJoin         = pTAIn->fAutoJoin;
      pEntry->fMatchInfo        = pTAIn->fTMMatch;
      pEntry->fRedundancy       = pTAIn->fRedundCount;
      pEntry->fRespectCRLF      = pTAIn->fRespectCRLF;
      pEntry->fSNOMATCH         = pTAIn->fInsertNewMatch;
      pEntry->fNoBlank          = pTAIn->fNoAddBlank;
    } /* endif */
  } /* endif */

  // search markup table in markup list
  if ( fOK )
  {
    int i = 0;
    while ( i < pProf->iMutTableSize )
    {
      if ( strcmp( pProf->MutEntries[i].szMUT, pszMarkup ) == 0 )
      {
        iGroup = pProf->MutEntries[i].iGroup;
        break; 
      } /* endif */
      i++;
    } /*endwhile */
  } /* endif */

  // apply group settings if found or restore old settings from group 0 when not found
  if ( fOK )
  {
    PMUANASETTINGS pEntry = &(pProf->MutSettings[iGroup]);

    if ( fCombineFlags )
    {
      pTAIn->fInsertTMMatches |= pEntry->fAutoSubst;
      pTAIn->fLeadingWS       |= pEntry->fAdjustLeadingWS;
      pTAIn->fTrailingWS      |= pEntry->fAdjustTrailingWS;
      pTAIn->fAutoJoin        |= pEntry->fAutoJoin;
      pTAIn->fTMMatch         |= pEntry->fMatchInfo;
      pTAIn->fRedundCount     |= pEntry->fRedundancy;
      pTAIn->fRespectCRLF     |= (EQF_BOOL)pEntry->fRespectCRLF;
      pTAIn->fInsertNewMatch  |= pEntry->fSNOMATCH;
      pTAIn->fNoAddBlank      |= (EQF_BOOL)pEntry->fNoBlank;
    }
    else
    {
      pTAIn->fInsertTMMatches = pEntry->fAutoSubst;
      pTAIn->fLeadingWS       = pEntry->fAdjustLeadingWS;
      pTAIn->fTrailingWS      = pEntry->fAdjustTrailingWS;
      pTAIn->fAutoJoin        = pEntry->fAutoJoin;
      pTAIn->fTMMatch         = pEntry->fMatchInfo;
      pTAIn->fRedundCount     = pEntry->fRedundancy;
      pTAIn->fRespectCRLF     = (EQF_BOOL)pEntry->fRespectCRLF;
      pTAIn->fInsertNewMatch  = pEntry->fSNOMATCH;
      pTAIn->fNoAddBlank      = (EQF_BOOL)pEntry->fNoBlank;
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function AnaProfApplyProfile */

// activate analysis profile dialog
BOOL AnaProfileDialog( PSZ pszProfile, HWND hwndParent )
{
  int   iRC = 0;
  BOOL  fOK = FALSE;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  DIALOGBOX( hwndParent, ANAPROFILEDLGPROC, hResMod, ID_ANAPROF_DLG, pszProfile, iRC );

  if ( iRC == 1 ) 
  {
    fOK = TRUE;
  } /* endif */

  return( fOK );
} /* end of function AnaProfileDialog */

// check if the supplied name is a valid profile name
BOOL AnaProfileCheckName( PSZ pszName, HWND hwndParent, BOOL fShowMessage  )
{
  BOOL  fOK = TRUE;
  PSZ pszTest = pszName;

  UtlStripBlanks( pszName );
  while ( *pszTest && (isalnum(*pszTest) || (*pszTest == '-') || (*pszTest == '_') ) )
  {
    pszTest++;
  } /*endwhile */

  if ( (*pszTest != EOS) || (*pszName == EOS) || (strlen(pszName) > MAX_PROFILENAME_LEN) )
  {
    fOK = FALSE;
    if ( fShowMessage )
    {
      UtlErrorHwnd( ERROR_TA_PROFILE_INVNAME, MB_CANCEL, 1, &pszName, EQF_ERROR, hwndParent );
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function AnaProfileCheckName */

// list all analysis profiles
BOOL AnaProfListProfiles( HWND hwndLB, BOOL fIsCombo )
{
  BOOL  fOK = TRUE;
  CHAR  szSearchPath[MAX_EQF_PATH];
  CHAR  szProfile[MAX_LONGPATH];
  WIN32_FIND_DATA FileFindData;
  HANDLE hDir;
  BOOL fMoreFiles = FALSE;
  CHAR  szID[ANA_PROF_ID_SIZE];
  int   iProfiles = 0;

  AnaProfBuildFullProfileName( "*", szSearchPath );

  hDir = FindFirstFile( szSearchPath, &FileFindData );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    do
    {
      if ( (FileFindData.nFileSizeHigh != 0) || (FileFindData.nFileSizeLow != 0) )
      {
        FILE *hf = NULL;
        AnaProfBuildFullProfileName( FileFindData.cFileName , szProfile );

        // read first bytes of profile to check the ID
        hf = fopen( szProfile, "r" );
        if ( hf )
        {
          memset( szID, 0, sizeof(szID) );
          fread( szID, ANA_PROF_ID_SIZE, 1, hf );
          if ( strcmp( szID, ANAPROFILEIDENTIFIER ) == 0 )
          {
            Utlstrccpy( szProfile, FileFindData.cFileName, DOT );
            if ( fIsCombo )
            {
              CBINSERTITEMHWND( hwndLB, szProfile );
            }
            else
            {
              INSERTITEMHWND( hwndLB, szProfile );
            } /* endif */

            iProfiles++;
          } /* endif */

          fclose( hf );
        } /* endif */

        // continue with next file
        fMoreFiles = FindNextFile( hDir, &FileFindData );
      } /* endif */
    } while ( fMoreFiles );
    FindClose( hDir );
  } /* endif */


  return( fOK );
} /* end of function AnaProfListProfiles */

// load a analysis profile into memory
BOOL AnaProfLoadProfile( PSZ pszProfile, PLONG plProfileHandle, HWND hwndErrMsg )
{
  BOOL  fOK = TRUE;
  PANAPROFILE pProf = NULL;
  CHAR  szProfile[MAX_LONGPATH];
  USHORT usSize = 0;          // size of profile

  AnaProfBuildFullProfileName( pszProfile, szProfile );


  if ( UtlLoadFile( szProfile,(PVOID *) &pProf, &usSize, FALSE, FALSE ) )
  {
    if ( strcmp( pProf->szID, ANAPROFILEIDENTIFIER ) != 0 )
    {
      // ID is incorrect
      fOK = FALSE;
    }
    else if ( usSize != (sizeof(ANAPROFILE) + (pProf->iMutTableSize * sizeof(MUTLISTENTRY))) )
    {
      // size is incorrect
      fOK = FALSE;
    }
    else
    {
      *plProfileHandle = (LONG)pProf;
    } /* endif */
  }
  else
  {
    fOK = FALSE;
  } /* endif */

  if ( !fOK )
  {
    UtlErrorHwnd( ERROR_TA_PROFILE_LOAD, MB_CANCEL, 1, &pszProfile, EQF_ERROR, hwndErrMsg );
  } /* endif */

  return( fOK );
} /* end of function AnaProfLoadProfile */

// free a previously loaded analysis profile into memory
BOOL AnaProfFreeProfile( LONG lProfileHandle )
{
  BOOL  fOK = TRUE;
  PANAPROFILE pProf = (PANAPROFILE)lProfileHandle; 

  if ( pProf != NULL )
  {
    if ( strcmp( pProf->szID, ANAPROFILEIDENTIFIER ) != 0 )
    {
      // ID is incorrect
      fOK = FALSE;
    }
    else
    {
      UtlAlloc( (PVOID *)&pProf, 0, 0, NOMSG );
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function AnaProfFreeProfile */


// build fully qualified file name for a profile
BOOL AnaProfBuildFullProfileName( PSZ pszProfile, PSZ pszFullPathName )
{
  BOOL  fOK = TRUE;

  UtlMakeEQFPath( pszFullPathName, NULC, PROPERTY_PATH, NULL );
  strcat( pszFullPathName, BACKSLASH_STR );
  strcat( pszFullPathName, pszProfile );
  if ( strchr( pszProfile, DOT ) == NULL )
  {
    strcat( pszFullPathName, EXT_OF_ANAPROFILE );
  } /* endif */

  return( fOK );
} /* end of function AnaProfBuildFullProfileName */



// 
// Internally called functions
//

// save settings of currently active group to profile
VOID AnaProfSaveGroupSetings( PANAPROFIDA pIda, HWND hwnd, int iGroup )
{
  PMUANASETTINGS pEntry = &(pIda->pProf->MutSettings[iGroup]);

  memset( pEntry, 0, sizeof(MUANASETTINGS) );
  pEntry->fAutoSubst         = QUERYCHECK( hwnd, ID_ANAPROF_AUTOSUBST_CHK );
  pEntry->fSNOMATCH          = QUERYCHECK( hwnd, ID_ANAPROF_NOMATCHSEGS_CHK );
  pEntry->fMatchInfo         = QUERYCHECK( hwnd, ID_ANAPROF_MEMMATCH_CHK );
  pEntry->fRedundancy        = QUERYCHECK( hwnd, ID_ANAPROF_REDUND_CHK );
  pEntry->fAdjustLeadingWS   = QUERYCHECK( hwnd, ID_ANAPROF_LEADINGWS_CHK );
  pEntry->fAdjustTrailingWS  = QUERYCHECK( hwnd, ID_ANAPROF_TRAILINGWS_CHK );
  pEntry->fRespectCRLF       = QUERYCHECK( hwnd, ID_ANAPROF_RESPECTCRLF_CHK );
  pEntry->fAutoJoin          = QUERYCHECK( hwnd, ID_ANAPROF_AUTOJOIN_CHK );
  pEntry->fNoBlank           = QUERYCHECK( hwnd, ID_ANAPROF_NOBLANK_CHK );
}

// show settings for given group
VOID AnaProfShowGroupSettings( PANAPROFIDA pIda, HWND hwnd, int iGroup )
{
  PMUANASETTINGS pEntry = &(pIda->pProf->MutSettings[iGroup]);
  int i = 0;

  // set analysis flags
  SETCHECK( hwnd, ID_ANAPROF_AUTOSUBST_CHK, pEntry->fAutoSubst );
  SETCHECK( hwnd, ID_ANAPROF_NOMATCHSEGS_CHK, pEntry->fSNOMATCH );
  SETCHECK( hwnd, ID_ANAPROF_MEMMATCH_CHK, pEntry->fMatchInfo );
  SETCHECK( hwnd, ID_ANAPROF_REDUND_CHK, pEntry->fRedundancy );
  SETCHECK( hwnd, ID_ANAPROF_LEADINGWS_CHK, pEntry->fAdjustLeadingWS );
  SETCHECK( hwnd, ID_ANAPROF_TRAILINGWS_CHK, pEntry->fAdjustTrailingWS );
  SETCHECK( hwnd, ID_ANAPROF_RESPECTCRLF_CHK, pEntry->fRespectCRLF );
  SETCHECK( hwnd, ID_ANAPROF_AUTOJOIN_CHK, pEntry->fAutoJoin );
  SETCHECK( hwnd, ID_ANAPROF_NOBLANK_CHK, pEntry->fNoBlank );

  // fill selected markup table listbox
  DELETEALL( hwnd, ID_ANAPROF_SELECTED_LB );
  for( i = 0; i < pIda->pProf->iMutTableSize; i++ )
  {
    if ( (pIda->pProf->MutEntries[i].szMUT[0] != EOS) && (pIda->pProf->MutEntries[i].iGroup == iGroup) )
    {
      INSERTITEM( hwnd, ID_ANAPROF_SELECTED_LB, pIda->pProf->MutEntries[i].szMUT );
    } /* endif */
  } /* endfor */

  pIda->iCurGroup = iGroup;
}

// add a markup to the markup table list of the profile
VOID AnaProfAddMarkup( PANAPROFIDA pIda, HWND hwnd, PSZ pszMarkup, int iGroup )
{
  int i = 0;

  hwnd; 

  // find a free slot in the markup table list
  while ( i < pIda->pProf->iMutTableSize )
  {
    if ( pIda->pProf->MutEntries[i].szMUT[0] == EOS )
    {
      // found a free slot
      strcpy( pIda->pProf->MutEntries[i].szMUT, pszMarkup );
      pIda->pProf->MutEntries[i].iGroup = iGroup;
      break; 
    } /* endif */
    i++;
  } /*endwhile */

  if ( i >= pIda->pProf->iMutTableSize )
  {
    // enlarge profile and markup table list to add this markup
    int iOldSize = sizeof(ANAPROFILE) + (pIda->pProf->iMutTableSize * sizeof(MUTLISTENTRY));
    int iNewSize = sizeof(ANAPROFILE) + ((pIda->pProf->iMutTableSize + 10) * sizeof(MUTLISTENTRY));
    if ( UtlAlloc( (PVOID *)&(pIda->pProf), iOldSize, iNewSize, ERROR_STORAGE ) )
    {
      i = pIda->pProf->iMutTableSize;
      pIda->pProf->iMutTableSize += + 10;
      strcpy( pIda->pProf->MutEntries[i].szMUT, pszMarkup );
      pIda->pProf->MutEntries[i].iGroup = iGroup;
    } /* endif */
  } /* endif */
}

// remove a markup from the markup table list of the profile
VOID AnaProfRemoveMarkup( PANAPROFIDA pIda, HWND hwnd, PSZ pszMarkup )
{
  int i = 0;

  hwnd;

  // find markup in the markup table list and remove it
  while ( i < pIda->pProf->iMutTableSize )
  {
    if ( strcmp( pIda->pProf->MutEntries[i].szMUT, pszMarkup ) == 0 )
    {
      memset( &(pIda->pProf->MutEntries[i]), 0, sizeof(MUTLISTENTRY) );
      break; 
    } /* endif */
    i++;
  } /*endwhile */
}

// save analysis profile using the supplied fully qualified name
BOOL AnaProfSaveProfile( PANAPROFIDA pIda, HWND hwnd, PSZ pszProfile )
{
  ULONG ulSize = (ULONG)(sizeof(ANAPROFILE) + (pIda->pProf->iMutTableSize * sizeof(MUTLISTENTRY)));
  USHORT usRC = UtlWriteFileL( pszProfile, ulSize, pIda->pProf, TRUE ); 

  hwnd;

  return( usRC == 0 );
}

// analysis profile dialog 
INT_PTR CALLBACK ANAPROFILEDLGPROC
(
   HWND hwnd,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
   PANAPROFIDA    pIda;                      // dialog instance data area
   BOOL     fOK = TRUE;                // internal O.K. flag

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_ANAPROF_DLG, mp2 ); break;

      case WM_INITDLG:
        // allocate and anchor our dialog IDA
        pIda = NULL;
        fOK = UtlAlloc( (PVOID *) &pIda, 0L, (ULONG)sizeof(ANAPROFIDA), ERROR_STORAGE );
        if ( fOK )
        {
          fOK = ANCHORDLGIDA( hwnd, pIda );
          if ( !fOK )                           //no access to ida
          {
            UtlErrorHwnd( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR, hwnd );
          } /* endif */
        } /* endif */

        // prepare profile area 
        if ( fOK )
        {
          // setup profile name
          AnaProfBuildFullProfileName( (PSZ)mp2 , pIda->szProfPath );

          // load existing profile or create a new one
          if ( UtlFileExist(pIda->szProfPath) )
          {
            LONG lHandle = 0;
            fOK = AnaProfLoadProfile( (PSZ)mp2, &lHandle, hwnd );
            if ( fOK )
            {
              pIda->pProf = (PANAPROFILE)lHandle;
            } /* endif */
          }
          else
          {
            fOK = UtlAlloc( (PVOID*)&(pIda->pProf), 0, (sizeof(ANAPROFILE) + sizeof(MUTLISTENTRY)), ERROR_STORAGE );
            if ( fOK )
            {
              strcpy( pIda->pProf->szID, ANAPROFILEIDENTIFIER );
            } /* endif */
          } /* endif */
        } /* endif */

        // fill dialog fields
        if ( fOK )
        {
          pIda->fInit = TRUE;

          // update titlebar text
          sprintf( pIda->szBuffer, "Analysis profile %s", (PSZ)mp2 );
          if ( pIda->pProf->fProtected )
          {
            strcat( pIda->szBuffer, "  [Protected]" );
          } /* endif */
          SETTEXTHWND( hwnd, pIda->szBuffer );

          // hide or show protect button / disable fields for protected profiles
          UtlMakeEQFPath( pIda->szBuffer, NULC, SYSTEM_PATH,NULL );
          strcat( pIda->szBuffer, BACKSLASH_STR );
          strcat( pIda->szBuffer, ANAPROF_PROTECT_TRIGGER );
          if ( UtlFileExist( pIda->szBuffer ) )
          {
            SHOWCONTROL( hwnd,ID_ANAPROF_PROTECT_PB );
            if ( pIda->pProf->fProtected )
            {
              SETTEXT( hwnd, ID_ANAPROF_PROTECT_PB, "Unprotect" );
            } /* endif */
          }
          else
          {
            HIDECONTROL( hwnd,ID_ANAPROF_PROTECT_PB );
            if ( pIda->pProf->fProtected )
            {
              ENABLECTRL( hwnd, ID_ANAPROF_AVAIL_LB, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_SELECTED_LB, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_SELECT_PB, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_DESELECT_PB, FALSE );
              ENABLECTRL( hwnd, PID_PB_OK, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_AUTOSUBST_CHK, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_NOMATCHSEGS_CHK, FALSE  );
              ENABLECTRL( hwnd, ID_ANAPROF_MEMMATCH_CHK, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_REDUND_CHK, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_LEADINGWS_CHK, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_TRAILINGWS_CHK, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_RESPECTCRLF_CHK, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_AUTOJOIN_CHK, FALSE );
              ENABLECTRL( hwnd, ID_ANAPROF_NOBLANK_CHK, FALSE );
            } /* endif */
          } /* endif */

          // fill markup group number combobox
          {
            int i;
            for( i = 1; i < MAX_MU_GROUPS; i++ )
            {
              CHAR szNumber[3];
              sprintf( szNumber, "%d", i );
              CBINSERTITEMEND( hwnd, ID_ANAPROF_GROUP_CB, szNumber );
              CBSELECTITEM( hwnd, ID_ANAPROF_GROUP_CB, 0 );
            } /* endfor */
          }

          // fill available markup tables combobox
          EqfSend2Handler( TAGTABLEHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND( WinWindowFromID( hwnd, ID_ANAPROF_AVAIL_LB) ), 0L );

          // remove all markups used in the profile
          {
            int i = 0;
            while ( i < pIda->pProf->iMutTableSize )
            {
              if ( pIda->pProf->MutEntries[i].szMUT[0] != EOS )
              {
                int iItem = SEARCHITEM( hwnd, ID_ANAPROF_AVAIL_LB, pIda->pProf->MutEntries[i].szMUT );
                if ( iItem != LIT_NONE )
                {
                  DELETEITEM( hwnd, ID_ANAPROF_AVAIL_LB, iItem );
                } /* endif */
              } /* endif */
              i++;
            } /*endwhile */
          }

          // force a refresh of the remaining fields
          //PostMessage( hwnd, WM_COMMAND, MP1FROMSHORT(ID_ANAPROF_GROUP_CB), MP2FROM2SHORT( 0, CBN_SELCHANGE ) );

          pIda->fInit = FALSE;
        } /* endif */
        if ( !fOK )
        {
           //--- close analysis dialog, FALSE means: - do not start analysis instance
           DISMISSDLG( hwnd, SHORT1FROMMP1( mp1 ) );
        } /* endif */
        mResult = DIALOGINITRETURN( mResult );
        break;


      case WM_COMMAND:
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
	        case PID_PB_HELP:
	          mResult = UtlInvokeHelp();
	          break;

          case PID_PB_OK:
            {
              pIda = ACCESSDLGIDA( hwnd, PANAPROFIDA );
              if ( pIda->iCurGroup != 0 )
              {
                AnaProfSaveGroupSetings( pIda, hwnd, pIda->iCurGroup );
              } /* endif */
              if ( AnaProfSaveProfile( pIda, hwnd, pIda->szProfPath )  )
              {
                DISMISSDLG( hwnd, TRUE );
              } /* endif */
            }
	          break;

          case PID_PB_CANCEL:
          case DID_CANCEL:
            DISMISSDLG( hwnd, FALSE );
            break;

          case ID_ANAPROF_SELECT_PB:
            PostMessage( hwnd, WM_COMMAND, MAKEWPARAM( ID_ANAPROF_AVAIL_LB, LN_ENTER), 0L );
            break;

          case ID_ANAPROF_DESELECT_PB:
            PostMessage( hwnd, WM_COMMAND, MAKEWPARAM( ID_ANAPROF_SELECTED_LB, LN_ENTER), 0L );
            break;

          case ID_ANAPROF_AVAIL_LB:
            if ( HIWORD( mp1 ) == LN_ENTER )
            {
              SHORT sItem = QUERYSELECTION( hwnd, ID_ANAPROF_AVAIL_LB );
              pIda = ACCESSDLGIDA( hwnd, PANAPROFIDA );

              if ( sItem != LIT_NONE )
              {
                int iNumOfItems = 0;
                QUERYITEMTEXT( hwnd, ID_ANAPROF_AVAIL_LB, sItem, pIda->szBuffer );
                INSERTITEM( hwnd, ID_ANAPROF_SELECTED_LB, pIda->szBuffer );
                DELETEITEM( hwnd, ID_ANAPROF_AVAIL_LB, sItem );
                iNumOfItems = QUERYITEMCOUNT( hwnd,  ID_ANAPROF_AVAIL_LB );
                if ( iNumOfItems  )
                {
                  if ( sItem >= iNumOfItems )
                  {
                    sItem--;
                  } /* endif */
                  SELECTITEM( hwnd, ID_ANAPROF_AVAIL_LB, sItem );
                } /* endif */
                AnaProfAddMarkup( pIda, hwnd, pIda->szBuffer, pIda->iCurGroup );
              } /* endif */
            } /* endif */
            break;

          case ID_ANAPROF_SELECTED_LB:
            if ( HIWORD( mp1 ) == LN_ENTER )
            {
              SHORT sItem = QUERYSELECTION( hwnd, ID_ANAPROF_SELECTED_LB );
              if ( sItem != LIT_NONE )
              {
                int iNumOfItems = 0;
                pIda = ACCESSDLGIDA( hwnd, PANAPROFIDA );
                QUERYITEMTEXT( hwnd, ID_ANAPROF_SELECTED_LB, sItem, pIda->szBuffer );
                DELETEITEM( hwnd, ID_ANAPROF_SELECTED_LB, sItem );
                INSERTITEM( hwnd, ID_ANAPROF_AVAIL_LB, pIda->szBuffer );
                AnaProfRemoveMarkup( pIda, hwnd, pIda->szBuffer );
                iNumOfItems = QUERYITEMCOUNT( hwnd,  ID_ANAPROF_SELECTED_LB );
                if ( iNumOfItems  )
                {
                  if ( sItem >= iNumOfItems )
                  {
                    sItem--;
                  } /* endif */
                  SELECTITEM( hwnd, ID_ANAPROF_SELECTED_LB, sItem );
                } /* endif */
              } /* endif */
            } /* endif */
            break;

          case ID_ANAPROF_GROUP_CB:
            pIda = ACCESSDLGIDA( hwnd, PANAPROFIDA );
            if ( !pIda->fInit )
            {
              SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );
              if (sNotification == CBN_SELCHANGE )
              {
                SHORT sItem = 0;
                int iGroup = 0;
                CBQUERYSELECTEDITEMTEXT( sItem, hwnd, ID_ANAPROF_GROUP_CB, pIda->szBuffer );
                iGroup = atol( pIda->szBuffer );

                if ( pIda->iCurGroup != iGroup )
                {
                  if ( pIda->iCurGroup != 0  )
                  {
                    AnaProfSaveGroupSetings( pIda, hwnd, pIda->iCurGroup );
                  } /* endif */
                  AnaProfShowGroupSettings( pIda, hwnd, iGroup );
                } /* endif */
              } /* endif */
            } /* endif */
            break;

          case ID_ANAPROF_PROTECT_PB:
            pIda = ACCESSDLGIDA( hwnd, PANAPROFIDA );
            if ( pIda->pProf->fProtected )
            {
              pIda->pProf->fProtected = FALSE;
              SETTEXT( hwnd, ID_ANAPROF_PROTECT_PB, "Protect" );
            }
            else
            {
              pIda->pProf->fProtected = TRUE;
              SETTEXT( hwnd, ID_ANAPROF_PROTECT_PB, "Unprotect" );
            } /* endif */
         } /*endswitch */
         break;

      case WM_NOTIFY:
         break;

      case WM_HELP:
         EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblAnaProfDlg[0] );
         mResult = TRUE; 
         break;


      case WM_EQF_CLOSE:
         DISMISSDLG( hwnd, mp1 );
         break;

      case WM_DESTROY:
         pIda = ACCESSDLGIDA( hwnd, PANAPROFIDA );
         if ( pIda )
         {
           if ( pIda->pProf )
           {
              UtlAlloc( (PVOID *)&(pIda->pProf), 0L, 0, NOMSG );
           } /* endif */
           UtlAlloc( (PVOID *) &pIda, 0L, 0, NOMSG );
         } /* endif */
         break;

      case DM_GETDEFID:
        // check if user pressed the ENTER key, but wants only to select/deselect an item of the listbox 
        // via a simulated (keystroke) double click.  
        if ( GetKeyState(VK_RETURN) & 0x8000 )
        {
          HWND hwndFocus = GetFocus();
          if ( hwndFocus == GetDlgItem( hwnd, ID_ANAPROF_AVAIL_LB ) )
          {
            PostMessage( hwnd, WM_COMMAND, MAKEWPARAM( ID_ANAPROF_AVAIL_LB, LN_ENTER), 0L );
          }
          else if ( hwndFocus == GetDlgItem( hwnd, ID_ANAPROF_SELECTED_LB ) )
          {
            PostMessage( hwnd, WM_COMMAND, MAKEWPARAM( ID_ANAPROF_SELECTED_LB, LN_ENTER), 0L );
          } /* endif */
        } /* endif */
        mResult = TRUE;
        break;

      default:
         mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of ANAPROFILEDLGPROC */

