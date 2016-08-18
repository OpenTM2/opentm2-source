//+----------------------------------------------------------------------------+
//|  EQFDIC03.C - EQF Dictionary Create dialog procedures                      |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|      all places marked with 'TBD' need rework and or new code !!!          |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM              // QDAM files
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdtag.h"              // tag include file
#include "eqfdde.h"
#include "eqfdasdi.h"             // internal ASD file (for field system names)
#include "OtmDictionaryIF.H"
#include "eqfdicti.h"             // Private include file of dictionary handler

#include "eqfrdics.h"                  // dict structures for u-code
#include "eqfdic00.id"                 // dialog IDs
#include "eqfdimp.id"                  // dialog IDs
#include "eqfdprop.id"                 // dialog IDs


extern HELPSUBTABLE hlpsubtblDicNewDlg[];
extern HELPSUBTABLE hlpsubtblDicModelDlg[];
extern HELPSUBTABLE hlpsubtblDicNameDlg[];
extern HELPSUBTABLE hlpsubtblDicAddFieldDlg[];

/**********************************************************************/
/* The second string of each table entry is normally the user name    */
/* for the field. In this table the username is replaced by the ID    */
/* of the string in the resource file. The name is then loaded        */
/* dynamically from the resource.                                     */
/**********************************************************************/
PROFENTRY DefaultProfEntries[] =   {
  { "Headword","21710",0,-1,1,RW_STATUS,RW_STATUS,FIRST_DISPLAY,1,TRUE,
    TRUE,TRUE,FALSE},
  { "Author","21711",0,-1,1,RW_STATUS,RW_STATUS,NO_DISPLAY,1,FALSE,
    FALSE,FALSE,FALSE},
  { "Creation Date","21712",0,-1,1,RO_STATUS,RO_STATUS,NO_DISPLAY,
    1,FALSE,FALSE,FALSE,FALSE},
  { "Status Code","21713",0,-1,1,RW_STATUS,RW_STATUS,NO_DISPLAY,1,
    FALSE,FALSE,FALSE,FALSE},
  { "Part of Speech","21714",0,-1,2,RW_STATUS,RW_STATUS,
    THIRD_DISPLAY, 1,FALSE,FALSE,FALSE,FALSE},
  { "Source of Headword","21715",0,-1,3,RW_STATUS,RW_STATUS,
    NO_DISPLAY, 1,FALSE,FALSE,FALSE,FALSE},
  { "Abbrev./Fullform","21716",0,-1,3,RW_STATUS,
    RW_STATUS, THIRD_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Author of Update","21717",0,-1,3,RW_STATUS,RW_STATUS,
    NO_DISPLAY, 1,FALSE,FALSE,FALSE,FALSE},
  { "Last Update","21718",0,-1,3,RO_STATUS,RO_STATUS,NO_DISPLAY,1,
    FALSE,FALSE,FALSE,FALSE},
  { "Definition","21719",0,-1,3,RW_STATUS,RW_STATUS,SECOND_DISPLAY,
    2,FALSE,FALSE,FALSE,FALSE},
  { "Source of Definition","21720",0,-1,3,RW_STATUS,
    RW_STATUS, NO_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Synonym","21721",0,-1,3,RW_STATUS,RW_STATUS,THIRD_DISPLAY,1,FALSE,
    FALSE,FALSE,FALSE},
  { "Other Related Terms","21722",0,-1,3,RW_STATUS,RW_STATUS,
    THIRD_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Context","21723",0,-1,3,RW_STATUS,RW_STATUS,SECOND_DISPLAY,2,FALSE,
    FALSE,FALSE,FALSE},
  { "Source of Context","21724",0,-1,3,RW_STATUS,RW_STATUS,
    NO_DISPLAY, 1,FALSE,FALSE,FALSE,FALSE},
  { "Comments","21725",0,-1,3,RW_STATUS,RW_STATUS,NO_DISPLAY,1,FALSE,
    FALSE,FALSE,FALSE},
  { "Note on Usage","21726",0,-1,3,RW_STATUS,RW_STATUS,NO_DISPLAY,2,
    FALSE,FALSE,FALSE,FALSE},
  { "Style","Style",0,-1,3,RW_STATUS,RW_STATUS,SECOND_DISPLAY,1,
    FALSE,FALSE,FALSE,FALSE},
  { "Language","21727",0,-1,4,RW_STATUS,RW_STATUS,NO_DISPLAY,1,FALSE,
    FALSE,FALSE,FALSE},
  { "Translation","21728",0,-1,4,RW_STATUS,RW_STATUS,FIRST_DISPLAY,1,
    TRUE,TRUE,TRUE,FALSE},
  { "Company/Subject Code","21729",0,-1,4,RW_STATUS,RW_STATUS,
    FIRST_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Source of Translation","21730",0,-1,4,RW_STATUS,
    RW_STATUS,NO_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Abbrev./Fullform","21731",0,-1,4,RW_STATUS,
    RW_STATUS,NO_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Author","21732",0,-1,4,RW_STATUS,RW_STATUS,NO_DISPLAY,1,FALSE,
    FALSE,FALSE,FALSE},
  { "Trans Author of Update","21733",0,-1,4,RW_STATUS,RW_STATUS,
    NO_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Creation Date","21734",0,-1,4,RO_STATUS,RO_STATUS,
    NO_DISPLAY, 1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Last Update","21735",0,-1,4,RO_STATUS,RO_STATUS,NO_DISPLAY,
    1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Status Code","21736",0,-1,4,RW_STATUS,RW_STATUS,NO_DISPLAY,
    1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Part of Speech","21737",0,-1,4,RW_STATUS,RW_STATUS,
    NO_DISPLAY, 1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Definition","21738",0,-1,4,RW_STATUS,RW_STATUS,NO_DISPLAY,2,
    FALSE,FALSE,FALSE,FALSE},
  { "Trans Source of Definition","21730",0,-1,4,RW_STATUS,
    RW_STATUS, NO_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Synonym","21740",0,-1,4,RW_STATUS,RW_STATUS,NO_DISPLAY,1,
    FALSE,FALSE,FALSE,FALSE},
  { "Trans Other Related Terms","21741",0,-1,4,RW_STATUS,
    RW_STATUS, NO_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Context","21742",0,-1,4,RW_STATUS,RW_STATUS,NO_DISPLAY,2,FALSE,
    FALSE,FALSE,FALSE},
  { "Trans Source of Context","21743",0,-1,4,RW_STATUS,RW_STATUS,
    NO_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Trans Comments","21744",0,-1,4,RW_STATUS,RW_STATUS,NO_DISPLAY,1,
    FALSE,FALSE,FALSE,FALSE},
  { "Trans Style","Trans Style",0,-1,4,RW_STATUS,RW_STATUS,NO_DISPLAY,1,FALSE,
    FALSE,FALSE,FALSE},
};

PROFENTRY MinProfEntries[] =   {
  { "Headword","21710",0,-1,1,RW_STATUS,RW_STATUS,FIRST_DISPLAY,1,TRUE,
    TRUE,TRUE,FALSE},
  { "Part of Speech","21714",0,-1,2,RW_STATUS,RW_STATUS,
    THIRD_DISPLAY, 1,FALSE,FALSE,FALSE,FALSE},
  { "Abbrev./Fullform","21716",0,-1,3,RW_STATUS,
    RW_STATUS, THIRD_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Definition","21719",0,-1,3,RW_STATUS,RW_STATUS,SECOND_DISPLAY,
    2,FALSE,FALSE,FALSE,FALSE},
  { "Synonym","21721",0,-1,3,RW_STATUS,RW_STATUS,THIRD_DISPLAY,1,FALSE,
    FALSE,FALSE,FALSE},
  { "Other Related Terms","21722",0,-1,3,RW_STATUS,RW_STATUS,
    THIRD_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
  { "Context","21723",0,-1,3,RW_STATUS,RW_STATUS,SECOND_DISPLAY,2,FALSE,
    FALSE,FALSE,FALSE},
  { "Translation","21728",0,-1,4,RW_STATUS,RW_STATUS,FIRST_DISPLAY,1,
    TRUE,TRUE,TRUE,FALSE},
  { "Company/Subject Code","21729",0,-1,4,RW_STATUS,RW_STATUS,
    THIRD_DISPLAY,1,FALSE,FALSE,FALSE,FALSE},
};

//+----------------------------------------------------------------------------+
//|  Create Dictionary                                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
BOOL DictionaryNew( HWND hwndParent, PDICTPARMS pDictParms )
{
  BOOL    fOK;
  HMODULE hResMod;
  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  //prevent warning during compile
  hwndParent = hwndParent;

  DIALOGBOX( EqfQueryTwbClient(), DICCREATEDICTIONARYDLG,
             hResMod, ID_DICNEW_DLG, pDictParms, fOK );
  return( fOK );
}

//+----------------------------------------------------------------------------+
//|  New Dictionary Dialog / Dictionary Property Dialog                        |
//|                                                                            |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK DICCREATEDICTIONARYDLG
(
HWND hwndDlg,
WINMSG message,
WPARAM mp1,
LPARAM mp2
)
{
  PDICCREATEIDA   pIda;               //ptr to instance data area
  EQFINFO         ErrorInfo;          // error returned by property handler
  MRESULT         mResult = FALSE;    //dialog procedure return value
  USHORT          usUserPriv;         //LAN user privilege class
  USHORT          usRc;               //return code
  SHORT           sAllItems;          //total number of items in listbox

  switch ( message)
  {

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp((HWND)  ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblDicNewDlg[0] );
      mResult = TRUE;  // message processed
      break;




    case WM_INITDLG:
      //allocate memory
      UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof(DICCREATEIDA), ERROR_STORAGE );

      if ( !pIda )
      {
        WinDismissDlg( hwndDlg, FALSE );
        return( MRFROMSHORT(FALSE) );
      } /* endif */

      ANCHORDLGIDA( hwndDlg, pIda);

      if ( mp2 )       //create dictionary called up in import dialog
      {
        //process dictionary property dialog
        pIda->pDictParms = (PDICTPARMS)mp2;

        //copy structure read from sgml maptable
        memcpy( &pIda->Prop, pIda->pDictParms->pszData,
                sizeof(PROPDICTIONARY) );
        //make second copy as backup in case of cancel after entry field
        //manipulation
        memcpy( &pIda->PropCopy, &pIda->Prop, sizeof(PROPDICTIONARY) );

        // fill in version of new dictionary -- if not folder import
        if (pIda->pDictParms->usType != NEWFOLIMP)
        {
          // indicate we are building current level
          pIda->Prop.usVersion = BTREE_VERSION3;
          pIda->PropCopy.usVersion = BTREE_VERSION3;
        }

        //name of dictionary
        Utlstrccpy( pIda->szDicName, pIda->Prop.PropHead.szName, DOT );
        if ( pIda->Prop.szLongName[0] != EOS )
        {
          strcpy( pIda->szLongName, pIda->Prop.szLongName );
          OEMTOANSI( pIda->szLongName );
        }
        else
        {
          Utlstrccpy( pIda->szLongName, pIda->Prop.PropHead.szName, DOT );
        } /* endif */

        //remove ext from szName as fname without ext used, ext added at end
        strcpy( pIda->Prop.PropHead.szName, pIda->szDicName );
        SETTEXT( hwndDlg, ID_DICNEW_NAME_EF, pIda->szLongName );

        if ( pIda->pDictParms->usType == NEWFOLIMP )
        {
          ENABLECTRL( hwndDlg, ID_DICNEW_NAME_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_DICNEW_PROTECTED_PB, FALSE );
          ENABLECTRL( hwndDlg, ID_DICNEW_SOURCELANG_CB, FALSE );
          ENABLECTRL( hwndDlg, ID_DICNEW_YES_PB, FALSE );
          ENABLECTRL( hwndDlg, ID_DICNEW_SET_PB, FALSE );
          if(pIda->Prop.szLongDesc[0] == EOS)
          {
            OEMTOANSI( pIda->Prop.szDescription );
            SETTEXT( hwndDlg, ID_DICNEW_DESCR_EF, pIda->Prop.szDescription );
            ANSITOOEM( pIda->Prop.szDescription );
          }
          else
          {
            OEMTOANSI( pIda->Prop.szLongDesc );
            SETTEXT( hwndDlg, ID_DICNEW_DESCR_EF, pIda->Prop.szLongDesc );
            ANSITOOEM( pIda->Prop.szLongDesc );
          }
          SETFOCUS( hwndDlg, ID_DICNEW_DESCR_EF );
        } /* endif */
      }
      else   //create a new dictionary
      {
        //initialize dictionary properties
        memset( &pIda->Prop, NULC, sizeof(PROPDICTIONARY ) );
        pIda->Prop.PropHead.usClass = PROP_CLASS_DICTIONARY;
        pIda->Prop.PropHead.chType  = PROP_TYPE_INSTANCE;
        pIda->Prop.fProtected = FALSE;
        memcpy( pIda->Prop.ProfEntry, MinProfEntries,
                sizeof( MinProfEntries ) );
        pIda->Prop.usLength = sizeof( MinProfEntries ) /
                              sizeof(PROFENTRY);
        DicLoadFieldNames( &(pIda->Prop) );

        //make second copy as backup in case of cancel after entry field
        //manipulation
        memcpy( &pIda->PropCopy, &pIda->Prop, sizeof(PROPDICTIONARY) );
        pIda->hDicProp = NULL;
      } /* endif */

      //get pointer to system properties
      pIda->pSysProp =(PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd() );

      usRc = UtlGetLANUserID( pIda->Prop.szUserid, &usUserPriv, FALSE );

      if ( usRc != NO_ERROR )         //LAN, Requester not started
      {
        ENABLECTRL( hwndDlg, ID_DICNEW_SHARED_RB, FALSE );
      }
      else
      {
        /****************************************************************/
        /* Disable shared RB if no LAN or shared drives are available   */
        /****************************************************************/
        UtlGetLANDriveList( (PBYTE)pIda->szLANDriveList );
        if ( pIda->szLANDriveList[0] == EOS )
        {
          ENABLECTRL( hwndDlg, ID_DICNEW_SHARED_RB, FALSE );
        }
        else
        {
          ENABLECTRL( hwndDlg, ID_DICNEW_SHARED_RB, TRUE );
        } /* endif */
      } /* endif */

      //select local radio button and set focus on button
      SETCHECK_TRUE( hwndDlg, ID_DICNEW_LOCAL_RB );

      //post message to proceed for local radio button
      WinPostMsg( hwndDlg, WM_COMMAND,
                  MP1FROMSHORT( ID_DICNEW_LOCAL_RB ),
                  MP2FROM2SHORT( 0, BN_CLICKED ) );

      if(pIda->Prop.szDescription[0] != EOS)
      {
        OEMTOANSI( pIda->Prop.szDescription );
        SETTEXT( hwndDlg, ID_DICNEW_DESCR_EF, pIda->Prop.szDescription );
        ANSITOOEM( pIda->Prop.szDescription );
      }
      else
      {
        OEMTOANSI( pIda->Prop.szLongDesc );
        SETTEXT( hwndDlg, ID_DICNEW_DESCR_EF, pIda->Prop.szLongDesc );
        ANSITOOEM( pIda->Prop.szLongDesc );
      }


      UtlFillTableLB( WinWindowFromID( hwndDlg, ID_DICNEW_SOURCELANG_CB ),
                      SOURCE_LANGUAGES );

      //query number of languages available
      sAllItems = CBQUERYITEMCOUNT( hwndDlg, ID_DICNEW_SOURCELANG_CB );

      if ( sAllItems )
      {
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        //select source language as in sgml file
        if ( pIda->Prop.szSourceLang[0] != NULC )
        {
          int sItem = CBSEARCHITEM( hwndDlg, ID_DICNEW_SOURCELANG_CB,
                                    pIda->Prop.szSourceLang );
          if ( sItem != LIT_NONE )
          {
            CBSELECTITEM( hwndDlg, ID_DICNEW_SOURCELANG_CB, sItem );
          }
          else
          {
            PSZ pszErrParm[2];

            pszErrParm[0] = pIda->Prop.szSourceLang;
            pszErrParm[1] = pIda->szDicName;

            UtlError( ERROR_LANG_NOTINSTALLED, MB_OK,
                      2, pszErrParm, EQF_WARNING );
          } /* endif */
        }
        else
        {
          //select first in listbox
          CBSELECTITEM( hwndDlg, ID_DICNEW_SOURCELANG_CB, 0 );
        } /* endif */

        //if dictionary listbox empty disable yes button
        //create invisible listbox for available folders ---
        pIda->hwndDictLB = WinCreateWindow( hwndDlg, WC_LISTBOX,
                                            "",
                                            WS_CHILD | LBS_STANDARD,
                                            0, 0, 5, 5,
                                            hwndDlg, HWND_TOP,
                                            1,
                                            NULL, NULL );

        //fill available dictionaries listbox
        EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES,
                         MP1FROMHWND( pIda->hwndDictLB ), 0L );

        //set focus to first entry field
        SETFOCUS( hwndDlg, ID_DICNEW_NAME_EF );

        //set text limits for entry fields and combo boxes
        SETTEXTLIMIT( hwndDlg, ID_DICNEW_NAME_EF, MAX_LONGFILESPEC - 1 );
        SETTEXTLIMIT( hwndDlg, ID_DICNEW_DESCR_EF,
                      sizeof( pIda->Prop.szLongDesc) - 1);

        SetCtrlFnt (hwndDlg, GetCharSet(),
                    ID_DICNEW_DESCR_EF, ID_DICNEW_NAME_EF );

        //name of second pushbutton
        if ( pIda->Prop.fProtected )
        {
           LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_UNPROTECT_TEXT, pIda->szBuffer1 );
        }
        else
        {
           LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_PROTECTED_TEXT, pIda->szBuffer1 );
        } /* endif */
        SETTEXT( hwndDlg, ID_DICNEW_PROTECTED_PB, pIda->szBuffer1);

        //grey out protect button until at least the dict name has
        //been entered
        if ( !mp2 )       //create dictionary called up in import dialog
        {
          ENABLECTRL( hwndDlg, ID_DICNEW_PROTECTED_PB, FALSE );
        } /* endif */

        //position dialog window
        WinSetWindowPos( hwndDlg, HWND_TOP, 200,200,0,0,
                         EQF_SWP_MOVE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE );


        if ( (pIda->pDictParms != NULL) &&
             (pIda->pDictParms->usType == NEWFOLIMP) )
        {
          SETFOCUS( hwndDlg, ID_DICNEW_DESCR_EF );
        } /* endif */

        mResult = DIALOGINITRETURN(TRUE);   // leave the focus where we put it
      }
      else
      {
        //no language support files loaded
        UtlError( ERROR_NO_LANG_SUPPORT_INSTALLED, MB_CANCEL, 0,
                  NULL, EQF_ERROR );
        WinDismissDlg( hwndDlg, FALSE );
        return( MRFROMSHORT(FALSE) );
      } /* endif */
      break;

/*-------------------------------------------------------------------------*/
    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );
      if ( pIda )
      {
        UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
        ANCHORDLGIDA( hwndDlg, NULL );
      } /* endif */
      break;

/*-------------------------------------------------------------------------*/

    case WM_CLOSE:
      pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );
      if ( pIda && pIda->hDicProp )
      {
        CloseProperties( pIda->hDicProp, PROP_QUIT, &ErrorInfo );
        pIda->hDicProp = NULL;
      } /* endif */
      DelCtrlFont (hwndDlg, ID_DICNEW_DESCR_EF);
      WinDismissDlg( hwndDlg, SHORT1FROMMP1(mp1) );
      break;

/*-------------------------------------------------------------------------*/
    case WM_COMMAND:
      mResult = DictCreateCommand( hwndDlg, WMCOMMANDID( mp1, mp2 ),
                                   WMCOMMANDCMD( mp1, mp2 ) );
      break;

/*-------------------------------------------------------------------------*/
    default:
      mResult = WinDefDlgProc( hwndDlg, message, mp1, mp2 );
/*-------------------------------------------------------------------------*/
  } /* endswitch */
  return( mResult );
}


MRESULT DictCreateControl
(
HWND   hwndDlg,                     // dialog handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PDICCREATEIDA   pIda;               //ptr to instance data area
  MRESULT         mResult = FALSE;    //dialog procedure return value
  USHORT          usId;               //counter

  sNotification;

  mResult = MRFROMSHORT( TRUE );      //TRUE is the default return value

  pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );

  switch ( sId )
  {
    case ID_DICNEW_NAME_EF:
      //allow password to be entered, i.e. activate protect button
      if ( (pIda->pDictParms == NULL) || (pIda->pDictParms->usType != NEWFOLIMP))
      {
        ENABLECTRL( hwndDlg, ID_DICNEW_PROTECTED_PB, TRUE );
      } /* endif */

      //get name of dictionary from dialog
      QUERYTEXT( hwndDlg, ID_DICNEW_NAME_EF, pIda->szLongName );

      break;
    case ID_DICNEW_LOCAL_RB:
      //enable creation of a dictionary
      ENABLECTRL( hwndDlg, ID_DICNEW_CREATE_PB, TRUE );

      //empty server name strings
      pIda->Prop.szServer[0] = NULC;

      // set dictionary location
      pIda->Prop.usLocation = LOC_LOCAL;

      //delete current drive icons
      DeleteDriveIcons( hwndDlg, pIda->szDriveList );

      //get local drives
      strcpy( pIda->szDriveList, pIda->pSysProp->szDriveList );
      //sort drive letters in alphabetical order
      UtlSortString( pIda->szDriveList );

      //create drive icon buttons
      UtlDriveButtons( hwndDlg, pIda->szDriveList, PID_DRIVEBUTTON_A,
                       WS_GROUP | WS_TABSTOP,
                       WS_VISIBLE,
                       WinWindowFromID( hwndDlg, ID_DICNEW_LOCATION_GB ),
                       WinWindowFromID( hwndDlg, ID_DICNEW_DRIVES_PB ),
                       NULLHANDLE );

      //set chdrive to default value (= EQF system drive)
      pIda->chDrive = pIda->pSysProp->szPrimaryDrive[0];

      //display selected drive
      usId = IDFROMDRIVE( PID_DRIVEBUTTON_A, pIda->chDrive );
      SETDRIVE( hwndDlg, usId, TRUE );

      break;

    case ID_DICNEW_SHARED_RB:
      //empty server name strings
      pIda->Prop.szServer[0] = NULC;

      // set dictionary location
      pIda->Prop.usLocation = LOC_SHARED;

      //delete current drive
      DeleteDriveIcons( hwndDlg, pIda->szDriveList );

      //get LAN drives
      strcpy( pIda->szDriveList, pIda->szLANDriveList );

      //sort drive letters in alphabetical order
      UtlSortString( pIda->szDriveList );

      //create drive icon buttons
      UtlDriveButtons( hwndDlg, pIda->szDriveList, PID_DRIVEBUTTON_A,
                       WS_GROUP | WS_TABSTOP,
                       WS_VISIBLE,
                       WinWindowFromID( hwndDlg, ID_DICNEW_LOCATION_GB ),
                       WinWindowFromID( hwndDlg, ID_DICNEW_DRIVES_PB ),
                       NULLHANDLE );

      /**************************************************************/
      /* Select default drive = first drive of LAN drive list       */
      /**************************************************************/
      usId = IDFROMDRIVE( PID_DRIVEBUTTON_A, pIda->szLANDriveList[0] );
      SETDRIVE( hwndDlg, usId, TRUE );
      pIda->chDrive = pIda->szLANDriveList[0];
      break;

  } /* endswitch */
  return( mResult );
} /* end DictCreateControl */


MRESULT DictCreateCommand
(
HWND hwndDlg,                       // dialog handle
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  PDICCREATEIDA   pIda;               //ptr to instance data area
  EQFINFO         ErrorInfo;          // error returned by property handler
  MRESULT         mResult = FALSE;    //dialog procedure return value
  USHORT          usRc;               //return code from other function calls
  BOOL            fOK = TRUE;         //internal ok flag
  BOOL            fChanged = TRUE;    //flag if entry field contents changed
  PSZ             pError;             //ptr to error string
  ULONG64         ulSpaceFree;        //ammount of free space

  HMODULE hResMod;
  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  sNotification;

  pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );

  mResult = MRFROMSHORT( TRUE );      // TRUE is the default return value

  switch ( sId )
  {
	case ID_DICNEW_HELP_PB:
	  mResult = UtlInvokeHelp();
	  break;
    case DID_CANCEL:
    case ID_DICNEW_CANCEL_PB:
      WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(FALSE), 0L );
      break;

    case ID_DICNEW_YES_PB:
      DIALOGBOX( hwndDlg, DICMODELDLGPROC, hResMod,
                 ID_DICMODEL_DLG, pIda, fOK ) ;
      break;

    case ID_DICNEW_PROTECTED_PB:
      if ( !pIda->Prop.fProtected )
      {
        pIda->ulPassword = 0L;
        DIALOGBOX( hwndDlg, DICT2PASSWORDDLG, hResMod,
                   ID_DICT2PASSWORD_DLG, &pIda->ulPassword, fOK );
        if ( fOK )
        {
          pIda->Prop.ulPassWord = pIda->ulPassword;
          pIda->Prop.fProtected = TRUE;
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_UNPROTECT_TEXT,
                      pIda->szBuffer1 );
          SETTEXT( hwndDlg, ID_DICNEW_PROTECTED_PB, pIda->szBuffer1);
        } /* endif */
      }
      else
      {
        DIALOGBOX( hwndDlg, DICT1PASSWORDDLG, hResMod,
                   ID_DICTPASSWORD_DLG, &pIda->Prop, fOK );
        if ( fOK )
        {
          pIda->Prop.ulPassWord = 0L;
          pIda->Prop.fProtected = FALSE;
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_PROTECTED_TEXT,
                      pIda->szBuffer1 );
          SETTEXT( hwndDlg, ID_DICNEW_PROTECTED_PB, pIda->szBuffer1);
        } /* endif */
      } /* endif */
      break;

    case ID_DICNEW_SET_PB:
      //set dict entry structure dialog
      DIALOGBOX( hwndDlg, SETENTRYSTRUCTDLG, hResMod, ID_DICNAME_DLG,
                 pIda, fOK );
      break;

    case ID_DICNEW_CREATE_PB:
      fOK = TRUE;
      //clear set buffer
      pIda->Prop.PropHead.szName[0] = NULC;

      //check if enough space on disc to create new dictionary
      ulSpaceFree = UtlQueryFreeSpace( pIda->chDrive, FALSE );

      if ( ulSpaceFree < MIN_DICT_SPACE )
      {
        CHAR  szDrive[2];           //drive array

        szDrive[0] = pIda->chDrive;
        szDrive[1] = EOS;
        pError = szDrive;
        UtlErrorHwnd( NO_SPACE_FOR_CREATION, MB_CANCEL,
                      1, &pError, EQF_ERROR, hwndDlg );
        fOK = FALSE;
      } /* endif */

      //get name of dictionary from dialog
      QUERYTEXT( hwndDlg, ID_DICNEW_NAME_EF, pIda->szBuffer1 );
      //remove leading and trailing blanks
      UtlStripBlanks( pIda->szBuffer1 );

      //get dict fname without extension from prop file if props
      //exist
      if ( pIda->Prop.PropHead.szName[0] != NULC )
      {
        Utlstrccpy( pIda->szBuffer2,
                    UtlGetFnameFromPath( pIda->Prop.szDictPath ), DOT );
        if ( strcmp( pIda->szBuffer1, pIda->szBuffer2 ) == 0 )
          fChanged = FALSE; //name has not been changed
      } /* endif */

      //test if name has been altered if it is valid
      if ( fChanged && fOK )
      {
        //name has been changed
        //name empty?
        if ( pIda->szBuffer1[0] == NULC && fOK )
        {
          UtlErrorHwnd( ERROR_NO_FILE_NAME, MB_CANCEL, 0, NULL,
                        EQF_WARNING, hwndDlg );
          SETFOCUS( hwndDlg, ID_DICNEW_NAME_EF );
          fOK = FALSE;
        }
        else
        {
          if ( !UtlCheckLongName( pIda->szBuffer1 ) )
          {
            PSZ pszParm = pIda->szBuffer1;
            UtlErrorHwnd( ERROR_INV_LONGNAME, MB_CANCEL, 1, &pszParm, EQF_WARNING, hwndDlg );
            fOK = FALSE;
            SETFOCUS( hwndDlg, ID_DICNEW_NAME_EF );
            //clear set buffer
            pIda->Prop.PropHead.szName[0] = NULC;
          }
          else
          {
            BOOL fIsNew = FALSE;         // folder-is-new flag
            strcpy( pIda->szLongName, pIda->szBuffer1 );
            ObjLongToShortName( pIda->szLongName, pIda->Prop.PropHead.szName,
                                DICT_OBJECT, &fIsNew );
          } /* endif */
        } /*endif*/
      } /*endif*/

      if ( fOK )
      {
        UtlMakeEQFPath( pIda->szDicName, pIda->chDrive, DIC_PATH, NULL );
        UtlMakeEQFPath( pIda->szPropName, NULC, PROPERTY_PATH, NULL );
        strcat( pIda->szDicName, BACKSLASH_STR );
        strcat( pIda->szPropName, BACKSLASH_STR );
        strupr( pIda->Prop.PropHead.szName );
        strcat( pIda->szDicName, pIda->Prop.PropHead.szName );
        strcat( pIda->szPropName, pIda->Prop.PropHead.szName );
        switch ( pIda->Prop.usLocation )
        {
          case LOC_LOCAL  : strcat( pIda->szDicName, EXT_OF_DIC ); break;
          case LOC_REMOTE : strcat( pIda->szDicName, EXT_OF_DIC ); break;
          case LOC_SHARED : strcat( pIda->szDicName, EXT_OF_SHARED_DIC ); break;
        } /* endswitch */
        strcat( pIda->szPropName, EXT_OF_DICPROP );
      } /* endif */

      if ( fOK )
      {
        //check if prop file or asd file exists -
        //this could be the case with LAN drives to which many
        //user have access

        //if local properties don't exist
        if ( !UtlFileExist( pIda->szPropName ) )
        {
          if ( UtlFileExist( pIda->szDicName ) ) //if asd file exists
            fOK = FALSE;
        }
        else
          fOK = FALSE; //file exists

        if ( !fOK )
        {
          pError = pIda->szLongName;

          //error msg that file already exists
          UtlErrorHwnd( ERROR_DICTIONARY_EXISTS, MB_CANCEL, 1,
                        &pError, EQF_WARNING, hwndDlg );
          SETTEXT( hwndDlg, ID_DICNEW_NAME_EF, "" );
          SETFOCUS( hwndDlg, ID_DICNEW_NAME_EF );

          //clear set buffer
          pIda->Prop.PropHead.szName[0] = NULC;
        } /* endif */

        if ( fOK )
        {
          //dict name does not exist locally
          if ( fOK )
          {
            strcat( pIda->Prop.PropHead.szName, EXT_OF_DICPROP );

            UtlMakeEQFPath( pIda->Prop.PropHead.szPath, NULC,
                            SYSTEM_PATH, NULL );

            //build base part of dictionary name (w/o extension)
            UtlMakeEQFPath( pIda->Prop.szDictPath,
                            pIda->chDrive,
                            DIC_PATH, NULL );

            strcat( pIda->Prop.szDictPath, BACKSLASH_STR );
            Utlstrccpy( pIda->Prop.szDictPath +
                        strlen( pIda->Prop.szDictPath ),
                        pIda->Prop.PropHead.szName, DOT );
            strcpy( pIda->Prop.szIndexPath, pIda->Prop.szDictPath );

            //add dictionary extension to dictionary path and index path
            switch ( pIda->Prop.usLocation )
            {
              case LOC_SHARED :
                strcat( pIda->Prop.szDictPath, EXT_OF_SHARED_DIC );
                strcat( pIda->Prop.szIndexPath, EXT_OF_SHARED_DICTINDEX );
                break;

              case LOC_LOCAL :
              case LOC_REMOTE:
              default :
                strcat( pIda->Prop.szDictPath, EXT_OF_DIC );
                strcat( pIda->Prop.szIndexPath, EXT_OF_DICTINDEX );
                break;
            } /* endswitch */

            // store dict long name
            strcpy( pIda->Prop.szLongName, pIda->szLongName );
            ANSITOOEM( pIda->Prop.szLongName );

            //query description and add to prop struct
             pIda->Prop.szDescription[0] = EOS;
             QUERYTEXT( hwndDlg, ID_DICNEW_DESCR_EF, pIda->Prop.szLongDesc);
             ANSITOOEM( pIda->Prop.szLongDesc );


            //query source language and add to prop struct
            QUERYTEXT( hwndDlg, ID_DICNEW_SOURCELANG_CB,
                       pIda->Prop.szSourceLang );
            if ( pIda->Prop.szSourceLang[0] == NULC )
            {
              UtlErrorHwnd( ERROR_NO_SOURCE_LANG_SELECTED, MB_CANCEL,
                            0, NULL, EQF_ERROR, hwndDlg );
              fOK = FALSE;
              SETFOCUS( hwndDlg, ID_DICNEW_SOURCELANG_CB );

              //clear set buffer in case user also changes name of dict
              pIda->Prop.PropHead.szName[0] = NULC;
            } /* endif */
          }
          else
          {
            //clear set buffer in case user also changes name of dict
            pIda->Prop.PropHead.szName[0] = NULC;

            //focus on entry field
            SETFOCUS( hwndDlg, ID_DICNEW_NAME_EF );
          } /* endif */

          if ( fOK )
          {
            SETCURSOR( SPTR_WAIT );

            Utlstrccpy( pIda->szBuffer2,
                        UtlGetFnameFromPath( pIda->szDicName ),
                        DOT );
            PROPNAME( pIda->szBuffer1, pIda->szBuffer2 );
            pIda->hDicProp = OpenProperties( pIda->szBuffer1, NULL,
                                             PROP_ACCESS_WRITE, &ErrorInfo);

            if ( pIda->hDicProp == NULL )
            {
              //create dictionary properties
              pIda->hDicProp = CreateProperties(
                                               pIda->szBuffer1, NULL, PROP_CLASS_DICTIONARY,
                                               &ErrorInfo);
            } /* endif */

            //if ok, copy allocated property structure to created one
            if ( pIda->hDicProp )
            {
              fOK = (PutAllProperties( pIda->hDicProp, &pIda->Prop,
                                       &ErrorInfo ) == 0);
            }
            else
            {
              if ( ErrorInfo == Err_NoStorage )
              {
                //delete dict properties
                UtlDelete( pIda->szPropName, 0l, FALSE );

                WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(TRUE), 0L );
                fOK = FALSE;
              }
              else
              {
                pError = pIda->szLongName;
                UtlErrorHwnd( ERROR_CREATING_PROPS, MB_CANCEL,
                              1, &pError, EQF_ERROR, hwndDlg );
                fOK = FALSE;

                //delete dict properties
                UtlDelete( pIda->szPropName, 0l, FALSE );

                //clear set buffer in case user changes name of dict
                pIda->Prop.PropHead.szName[0] = NULC;
              }
            } /* endif */
          } /* endif */

          // if ok, save properties
          if ( fOK )
          {
            fOK = ( SaveProperties( pIda->hDicProp, &ErrorInfo ) == 0);
            if ( !fOK )
            {
              if ( ErrorInfo == Err_NoStorage )
              {
                //delete dict properties
                UtlDelete( pIda->szPropName, 0l, FALSE );

                WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(TRUE), 0L );
              }
              else
              {
                pError = pIda->szLongName;
                UtlErrorHwnd( ERROR_SAVING_PROPS, MB_CANCEL,
                              1, &pError, EQF_ERROR, hwndDlg );
                //clear set buffer in case user changes name of dict
                pIda->Prop.PropHead.szName[0] = NULC;

                //delete dict properties
                UtlDelete( pIda->szPropName, 0l, FALSE );
              } /* endif */
            } /* endif */
          } /* endif */

          //if remote send copy of properties to lan
          //update dict and index paths update PropHead.szPath
          if ( fOK )
          {
            switch ( pIda->Prop.usLocation )
            {
              case LOC_LOCAL  :
                break;

              case LOC_SHARED :
                /*************************************************/
                /* Create copy of property file on target drive  */
                /*************************************************/
                {
                  CHAR szTarget[MAX_EQF_PATH];

                  /***********************************************/
                  /* Set up path to properties on target drive   */
                  /***********************************************/
                  UtlMakeEQFPath( szTarget, pIda->chDrive, DIC_PATH, NULL );

                  /***********************************************/
                  /* Create directory if it does not exist       */
                  /***********************************************/
                  UtlMkMultDir( szTarget, FALSE );

                  /***********************************************/
                  /* Complete target property file name          */
                  /***********************************************/
                  strcat( szTarget, BACKSLASH_STR );
                  Utlstrccpy( szTarget + strlen(szTarget),
                              pIda->Prop.PropHead.szName, DOT );
                  strcat( szTarget, EXT_OF_SHARED_DICTPROP );

                  /***********************************************/
                  /* Write property file on target drive         */
                  /***********************************************/
                  usRc = UtlWriteFile( szTarget, sizeof(pIda->Prop),
                                       &pIda->Prop, TRUE );

                  /***********************************************/
                  /* Delete local property file if creation      */
                  /* of shared property file failed              */
                  /***********************************************/
                  if (usRc != NO_ERROR )
                  {
                    fOK = FALSE;
                    CloseProperties( pIda->hDicProp,
                                     PROP_QUIT, &ErrorInfo );
                    pIda->hDicProp = NULL;
                    UtlDelete( pIda->szPropName, 0l, FALSE );
                  } /* endif */
                }
                break;
            } /* endswitch */

            if ( fOK )
            {
              //fill msg parameter array for QDAM error messages
              pIda->pszMsgError[0] = pIda->szLongName;
              //current entry
              pIda->pszMsgError[1] = EMPTY_STRING;
              //dictionary drive
              pIda->szDrive[0] = pIda->Prop.szDictPath[0];
              pIda->szDrive[1] = EOS;
              pIda->pszMsgError[2] = pIda->szDrive;
              pIda->pszMsgError[3] = EMPTY_STRING;
              //dict source lang
              pIda->pszMsgError[4] = pIda->Prop.szSourceLang;

              //init DAM/TOLSTOY
              usRc = AsdBegin( 2, &pIda->hUser );

              if ( usRc != LX_RC_OK_ASD )
              {
                pError =  QDAMErrorString( usRc, pIda->pszMsgError );
                UtlErrorHwnd( usRc, MB_CANCEL, 1, &pError,
                              QDAM_ERROR, hwndDlg );
                fOK = FALSE;

                //close dict properties
                CloseProperties( pIda->hDicProp,
                                 PROP_QUIT, &ErrorInfo );
                pIda->hDicProp = NULL;

                //delete dict properties
                UtlDelete( pIda->szPropName, 0l, TRUE );
              }
              else
              {
                //create dictionary
                if ( usRc == LX_RC_OK_ASD )
                {
                  usRc = AsdBuild ( pIda->hUser, //user handle
                                    FALSE,              //guarded mode
                                    &pIda->hDict,       //dict handle
                                    pIda->szPropName ); //dict prop file

                  if ( usRc != LX_RC_OK_ASD )
                  {
                    pError =  QDAMErrorString( usRc, pIda->pszMsgError );
                    UtlErrorHwnd( usRc, MB_CANCEL, 1, &pError,
                                  QDAM_ERROR, hwndDlg );
                    fOK = FALSE;

                    //close dict properties
                    CloseProperties( pIda->hDicProp, PROP_QUIT,
                                     &ErrorInfo );

                    pIda->hDicProp = NULL;

                    //close dictionary
                    AsdClose( pIda->hUser, pIda->hDict );
                    AsdEnd( pIda->hUser );

                    //delete all files created
                    AsdDelete( pIda->szPropName );

                  }
                  else
                  {
                    AsdClose( pIda->hUser, pIda->hDict );
                    AsdEnd( pIda->hUser );
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */

      // inform the rest of Troja that we have a new dictioanry ...
      if ( fOK)
      {
        //make property path
        PROPNAME( pIda->szPropName, pIda->Prop.PropHead.szName );

        EqfSend2AllHandlers( WM_EQFN_CREATED,
                             MP1FROMSHORT( clsDICTIONARY ),
                             MP2FROMP(pIda->szPropName) );
      } /* endif */

      //if all okay close dialog
      if ( fOK )
      {
        //new dict dialog called up in import program
        if ( pIda->pDictParms )
        {
          Utlstrccpy( pIda->szBuffer1,
                      UtlGetFnameFromPath( pIda->szDicName ),
                      DOT );
          strcpy(pIda->pDictParms->pszData, pIda->szBuffer1);
        }
        else
        {
          //inform user that all went well
          pError = pIda->szLongName;

          //mention that dict was successfully created
          UtlError( MESSAGE_DIC_CREATE_COMPLETED,
                    MB_OK, 1, &pError, EQF_INFO );
        } /* endif */

        SETCURSOR( SPTR_ARROW );
        WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(TRUE), 0L );
      } /* endif */
      break;

    case ID_DICNEW_NAME_EF:
    case ID_DICNEW_LOCAL_RB:
    case ID_DICNEW_SHARED_RB:
      mResult = DictCreateControl( hwndDlg, sId, sNotification );
      break;

    case ID_DICNEW_DESCR_EF:
      if ( sNotification == EN_KILLFOCUS )
      {
        ClearIME( hwndDlg );
      } /* endif */
      break;

    default:
      //check for drive buttons
      if ( ( sId >= PID_DRIVEBUTTON_A )  && ( sId <= PID_DRIVEBUTTON_Z )  )
      {
        //deselect any previously selected drive button
        if ( pIda->chDrive != ' ' )
        {
          SETDRIVE( hwndDlg,
                    IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                 pIda->chDrive ),
                    FALSE );
        } /* endif */
        pIda->chDrive = DRIVEFROMID( PID_DRIVEBUTTON_A, sId );
        SETDRIVE( hwndDlg, sId, TRUE );
      } /* endif */
  } /* endswitch */
  return mResult;
} /* end DictCreateCommand */


static
VOID DeleteDriveIcons( HWND  hDlg,
                       PSZ   pszDriveList )
{
  USHORT usIndex;                            // Index variable
  USHORT usDrive;                            // id for a drive icon

  //--- sort drivelist
  UtlSortString( pszDriveList );

  usIndex = 0;
  while ( pszDriveList[usIndex] != EOS )
  {
    usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                           pszDriveList[usIndex] );
    WinDestroyWindow( WinWindowFromID( hDlg, usDrive ) );
    usIndex++;
  }/*endwhile*/
}/* end DeleteDriveIcons */

//dialog with list of dictionaries from which the user may select one for
//use as model
INT_PTR CALLBACK DICMODELDLGPROC
(
HWND   hwndDlg,
WINMSG message,
WPARAM mp1,
LPARAM mp2
)
{
  EQFINFO           ErrorInfo;            // error returned by property handler
  PDICCREATEIDA     pIda;                 // ptr to instance data area
  PPROPDICTIONARY   pModelProp;           // ptr to model dictionary props
  HPROP             hModelProp;           // handle of model dictionary props
  SHORT             sItem;                // index of selected listbox item
  BOOL              fOK;                 // ok flag returned to caller
  MRESULT           mResult = FALSE;

  switch ( message)
  {
    case WM_INITDLG:
      //anchor IDA
      pIda = (PDICCREATEIDA) mp2;
      ANCHORDLGIDA( hwndDlg, pIda);

      EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES,
                       MP1FROMHWND( WinWindowFromID( hwndDlg, ID_DICMODEL_DICT_LB ) ),
                       0L );
      INSERTITEM( hwndDlg, ID_DICMODEL_DICT_LB, MODELDICT );
      SELECTITEM( hwndDlg, ID_DICMODEL_DICT_LB, 0 );

      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblDicModelDlg[0] );
      mResult = TRUE;  // message processed
      break;



    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );
      mResult = MRFROMSHORT( TRUE);
      fOK = TRUE;                    // assume everything is ok

      switch (WMCOMMANDID( mp1, mp2 ))
      {
		case ID_DICMODEL_HELP_PB:
		  mResult = UtlInvokeHelp();
		  break;
        case DID_CANCEL:
        case ID_DICMODEL_CANCEL_PB:
          WinDismissDlg( hwndDlg, FALSE );
          break;

        case ID_DICMODEL_OK_PB:
          sItem = QUERYSELECTION( hwndDlg, ID_DICMODEL_DICT_LB );

          if ( sItem != LIT_NONE )
          {
            QUERYITEMTEXT( hwndDlg, ID_DICMODEL_DICT_LB, sItem,
                           pIda->szModelDictLongName );
            if ( strcmp( pIda->szModelDictLongName , MODELDICT ) == 0 )
            {
              memcpy( pIda->Prop.ProfEntry, DefaultProfEntries,
                      sizeof( DefaultProfEntries ) );
              pIda->Prop.usLength = sizeof( DefaultProfEntries ) /
                                    sizeof(PROFENTRY);
              pIda->Prop.usUserNameCount = 0;

              DicLoadFieldNames( &(pIda->Prop) );
            }
            else
            {
              BOOL fIsNew = FALSE;         // is-new flag
              ANSITOOEM( pIda->szModelDictLongName );
              ObjLongToShortName( pIda->szModelDictLongName, pIda->szModelDictShortName,
                                  DICT_OBJECT, &fIsNew );
              PROPNAME( pIda->szBuffer1, pIda->szModelDictShortName );
              hModelProp = OpenProperties( pIda->szBuffer1, NULL,
                                           PROP_ACCESS_READ, &ErrorInfo);
              if ( hModelProp )
              {
                pModelProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hModelProp );
                memcpy( pIda->Prop.ProfEntry, pModelProp->ProfEntry,
                        sizeof( pIda->Prop.ProfEntry ) );
                pIda->Prop.usLength = pModelProp->usLength;
                pIda->Prop.usUserNameCount = pModelProp->usUserNameCount;
                strcpy( pIda->Prop.szSourceLang,
                        pModelProp->szSourceLang );
                memcpy( &pIda->Prop.ColFontDictEntry,
                        &pModelProp->ColFontDictEntry,
                        sizeof( pIda->Prop.ColFontDictEntry ) );
                memcpy( &pIda->Prop.ColFontEntryVal,
                        &pModelProp->ColFontEntryVal,
                        sizeof( pIda->Prop.ColFontEntryVal) );
                CloseProperties( hModelProp, PROP_QUIT, &ErrorInfo );
              } /* endif */
            } /* endif */

            //update propcopy
            pIda->PropCopy = pIda->Prop;

            WinDismissDlg( hwndDlg, TRUE );
          } /* endif */
          break;
      } /* endswitch */
      break;

    default:
      mResult = WinDefDlgProc (hwndDlg, message, mp1, mp2 );
  } /* endswitch */
  return( mResult );
}


//+----------------------------------------------------------------------------+
//|  Set Entry Structure Dialog - user can determine fields, field size,       |
//|  field name, display level.                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK SETENTRYSTRUCTDLG
(
HWND hwndDlg,
WINMSG message,
WPARAM mp1,
LPARAM mp2
)
{
  PDICCREATEIDA   pIda;               // ptr to instance data area
  MRESULT         mResult = FALSE;    // dialog procedure return value

  switch ( message)
  {
/*--------------------------------------------------------------------------*/


    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp((HWND)  ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblDicNameDlg[0] );
      mResult = TRUE;  // message processed
      break;


    case WM_INITDLG:
      //anchor IDA
      pIda = (PDICCREATEIDA) mp2;
      ANCHORDLGIDA( hwndDlg, pIda);

      if ( !pIda )
      {
        WinDismissDlg( hwndDlg, FALSE );
        return( MRFROMSHORT(FALSE) );
      } /* endif */

      //set text colour in level group box to black
      SETCOLOR( hwndDlg, ID_DICNAME_LEV1_TEXT, CLR_BLACK );
      SETCOLOR( hwndDlg, ID_DICNAME_LEV2_TEXT, CLR_BLACK );
      SETCOLOR( hwndDlg, ID_DICNAME_LEV3_TEXT, CLR_BLACK );
      SETCOLOR( hwndDlg, ID_DICNAME_LEV4_TEXT, CLR_BLACK );
      DicFillEntryLB( hwndDlg, pIda, ID_DICNAME_ENTRYFIELDS_LB );

      // disable add pushbutton if maximum of dictionary fields has been
      // exceeded
      if ( pIda->Prop.usLength >= MAX_FIELDS_FOR_DICT )
      {
        ENABLECTRL( hwndDlg, ID_DICNAME_ADDFIELD_PB, FALSE );
      } /* endif */

      /*************************************************************/
      /* Force processing of listbox selection (not required for   */
      /* OS/2)                                                     */
      /*************************************************************/
      SetCtrlFnt (hwndDlg, GetCharSet(), ID_DICNAME_ENTRYFIELDS_LB, 0 );
      SetEntryStructControl( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB, LN_SELECT );

      //position dialog window
      WinSetWindowPos( hwndDlg, HWND_TOP, 20,20,0,0,
                       EQF_SWP_MOVE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE );


      mResult = MRFROMSHORT(TRUE);   // leave the focus where we put it
      break;

    case WM_CLOSE:
      DelCtrlFont (hwndDlg, ID_DICNAME_ENTRYFIELDS_LB);
      WinDismissDlg( hwndDlg, SHORT1FROMMP1(mp1) );
      mResult = MRFROMSHORT( FALSE );
      break;

    case WM_COMMAND:
      mResult = SetEntryStructCommand( hwndDlg, WMCOMMANDID( mp1, mp2 ),
                                       WMCOMMANDCMD( mp1, mp2 ) );
      break;
/*--------------------------------------------------------------------------*/
    default:
      mResult = WinDefDlgProc( hwndDlg, message, mp1, mp2 );
/*--------------------------------------------------------------------------*/
  } /* endswitch */
  return( mResult );
}


MRESULT SetEntryStructControl
(
HWND   hwndDlg,                     // dialog handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PDICCREATEIDA   pIda;               // ptr to instance data area
  MRESULT         mResult = FALSE;    // dialog procedure return value

  pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );

  switch ( sId )
  {
    case ID_DICNAME_ENTRYFIELDS_LB:
      if ( sNotification == LN_SELECT )
      {
        SHORT sItem;

        sItem = QUERYSELECTION( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB );
        if ( sItem != LIT_NONE )
        {
          pIda->pEntry = (PPROFENTRY) QUERYITEMHANDLE( hwndDlg,
                                                       ID_DICNAME_ENTRYFIELDS_LB, sItem );

          switch ( pIda->pEntry->usDisplay )
          {
            case 0:
              SETCHECK_TRUE( hwndDlg, ID_DICNAME_OMIT_RB );
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_L1_RB);
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_L2_RB );
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_L3_RB );
              break;
            case 1:
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_OMIT_RB );
              SETCHECK_TRUE ( hwndDlg, ID_DICNAME_L1_RB);
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_L2_RB );
              SETCHECK_FALSE ( hwndDlg, ID_DICNAME_L3_RB);
              break;
            case 2:
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_OMIT_RB );
              SETCHECK_FALSE ( hwndDlg, ID_DICNAME_L1_RB);
              SETCHECK_TRUE( hwndDlg, ID_DICNAME_L2_RB );
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_L3_RB );
              break;
            case 3:
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_OMIT_RB );
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_L1_RB );
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_L2_RB );
              SETCHECK_TRUE ( hwndDlg, ID_DICNAME_L3_RB);
              break;
          } /* endswitch */

          switch ( pIda->pEntry->usLevel )
          {
            case 1:
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV1_TEXT, TRUE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV2_TEXT, FALSE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV3_TEXT, FALSE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV4_TEXT, FALSE );
              break;
            case 2:
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV2_TEXT, TRUE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV1_TEXT, FALSE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV3_TEXT, FALSE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV4_TEXT, FALSE );
              break;
            case 3:
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV3_TEXT, TRUE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV1_TEXT, FALSE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV2_TEXT, FALSE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV4_TEXT, FALSE );
              break;
            case 4:
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV4_TEXT, TRUE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV1_TEXT, FALSE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV2_TEXT, FALSE );
              ENABLECTRL( hwndDlg, ID_DICNAME_LEV3_TEXT, FALSE );
              break;
          } /* endswitch */

          switch ( pIda->pEntry->usEntryFieldType )
          {
            case 1:
              SETCHECK_TRUE( hwndDlg, ID_DICNAME_SMALL_RB );
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_LARGE_RB );
              break;
            case 2:
              SETCHECK_TRUE( hwndDlg, ID_DICNAME_LARGE_RB );
              SETCHECK_FALSE( hwndDlg, ID_DICNAME_SMALL_RB );
              break;
          } /* endswitch */

          /********************************************************/
          /* Handle automatic lookup checkbox                     */
          /********************************************************/
          if ( (strcmp( pIda->pEntry->chSystName, HEADWORD_SYST_NAME) == 0)
               ||
               (strcmp( pIda->pEntry->chSystName, SYSNAME_TRANSLATION) == 0) )
          {
            /******************************************************/
            /* Headword and translation have always the           */
            /* checkbox switched on, set the checkbox on and      */
            /* disable the checkbox to avoid tampering with it    */
            /******************************************************/
            ENABLECTRL( hwndDlg, ID_DICNAME_AUTLOOKUP_CB, FALSE );
            SETCHECK_TRUE( hwndDlg, ID_DICNAME_AUTLOOKUP_CB );
          }
          else
          {
            ENABLECTRL( hwndDlg, ID_DICNAME_AUTLOOKUP_CB, TRUE );
            SETCHECK( hwndDlg, ID_DICNAME_AUTLOOKUP_CB,
                      pIda->pEntry->fAutLookup );
          } /* endif */
        } /* endif */
      } /* endif */
      break;

    case ID_DICNAME_OMIT_RB:  pIda->pEntry->usDisplay = 0; break;

    case ID_DICNAME_L1_RB:    pIda->pEntry->usDisplay = 1; break;

    case ID_DICNAME_L2_RB:    pIda->pEntry->usDisplay = 2; break;

    case ID_DICNAME_L3_RB:    pIda->pEntry->usDisplay = 3; break;

    case ID_DICNAME_SMALL_RB: pIda->pEntry->usEntryFieldType = 1; break;

    case ID_DICNAME_LARGE_RB: pIda->pEntry->usEntryFieldType = 2; break;

    case ID_DICNAME_AUTLOOKUP_CB:
      pIda->pEntry->fAutLookup = QUERYCHECK( hwndDlg,
                                             ID_DICNAME_AUTLOOKUP_CB );
      break;

  } /* endswitch */
  return( mResult );
}

MRESULT SetEntryStructCommand
(
HWND hwndDlg,                       // dialog handle
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  PDICCREATEIDA   pIda;                          // ptr to instance data area
  MRESULT         mResult = MRFROMSHORT(TRUE);   // function return value

  sNotification;

  pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );

  switch ( sId )
  { case ID_DICNAME_HELP_PB:
      mResult = UtlInvokeHelp();
      break;
    case DID_CANCEL:
    case ID_DICNAME_CANCEL_PB:
      pIda->Prop = pIda->PropCopy;
      DicFillEntryLB( hwndDlg, pIda,  ID_DICNAME_ENTRYFIELDS_LB );
      WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(FALSE), 0L );
      mResult = (MRESULT) FALSE;
      break;

    case ID_DICNAME_DELFIELD_PB:
      {
        SHORT           sItem;                  // index of listbox items
        ULONG           ulArrayPos;             // counter

        // check if listbox item is selected
        sItem = QUERYSELECTION( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB );
        if ( sItem == LIT_NONE )
        {
          UtlErrorHwnd( ERROR_ENTRYFIELD_NOT_SELECTED,
                        MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
        }
        else
        {
          pIda->pEntry = (PPROFENTRY)QUERYITEMHANDLE( hwndDlg,
                                                      ID_DICNAME_ENTRYFIELDS_LB,
                                                      sItem );

          if ( ( strcmp( pIda->pEntry->chSystName, "Headword" ) == 0) ||
               ( strcmp( pIda->pEntry->chSystName, "Translation" ) == 0) )
          {
            //error msg that these two fields cannot be deleted
            UtlErrorHwnd( ERROR_ENTRYFIELD_NOT_DELETED,
                          MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );

          }
          else
          {
            //remove entry from array
            ulArrayPos = ( pIda->Prop.usLength -
                           (pIda->pEntry - pIda->Prop.ProfEntry)
                           - 1 );
            if ( ulArrayPos > 0 )
            {
              memmove( pIda->pEntry, pIda->pEntry+1,
                       ulArrayPos * sizeof(PROFENTRY) );
            } /* endif */

            //set new number of prof entries
            pIda->Prop.usLength--;

            //fill entry listbox
            DicFillEntryLB( hwndDlg, pIda, ID_DICNAME_ENTRYFIELDS_LB );

            if ( sItem < (SHORT)pIda->Prop.usLength )
            {
              //position in list box where delete was done
              SELECTITEM( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB, sItem );
            }
            else if ( sItem == (SHORT)pIda->Prop.usLength )
            {
              //position in list box where delete was done
              SELECTITEM( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB, sItem - 1 );
            } /* endif */
          } /* endif */
        } /* endif */
        //focus on list box item
        SETFOCUS( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB );

        /*************************************************************/
        /* ensable ADD pushbutton if less than maximum number of     */
        /* dictionary fields have been defined                       */
        /*************************************************************/
        if ( pIda->Prop.usLength < MAX_FIELDS_FOR_DICT )
        {
          ENABLECTRL( hwndDlg, ID_DICNAME_ADDFIELD_PB, TRUE );
        } /* endif */
      }
      break;

    case ID_DICNAME_CHANGENAME_PB:
      {
        SHORT           sItem;                  // index of listbox items
        BOOL            fOK;                    // O.K. flag ...

        // check if listbox item is selected
        sItem = QUERYSELECTION( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB );
        if ( sItem == LIT_NONE )
        {
          UtlErrorHwnd( ERROR_ENTRYFIELD_NOT_SELECTED,
                        MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
        }
        else
        {
		  HMODULE hResMod;
		  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          pIda->pEntry = (PPROFENTRY) QUERYITEMHANDLE ( hwndDlg,
                                                        ID_DICNAME_ENTRYFIELDS_LB,
                                                        sItem );
          pIda->fChangeName = TRUE;
          DIALOGBOX( hwndDlg, PCH_DLGPROC, hResMod, ID_PCH_DLG, pIda, fOK );

          // if new term entered, refill the entry fields listbox
          // with new value
          if ( fOK == TRUE )
          {
            DicFillEntryLB( hwndDlg, pIda,  ID_DICNAME_ENTRYFIELDS_LB );

            //position on changed name
            SELECTITEM( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB, sItem );
          } /* endif */
          //focus on list box item
          SETFOCUS( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB );
        } /* endif */
      }
      break;

    case ID_DICNAME_ADDFIELD_PB:
      {
        SHORT           sItem;                  // index of listbox items
        BOOL            fOK = TRUE;             // O.K. flag ...
        ULONG          ulEnd = 0;

        //build new entry for 'Add Field' function
        memset( &pIda->NewEntry, 0, sizeof(PPROFENTRY) );
        sprintf( pIda->NewEntry.chSystName, "Username%d",
                 pIda->Prop.usUserNameCount );
        sprintf( pIda->NewEntry.chUserName,  "Username%d",
                 pIda->Prop.usUserNameCount );

        sItem = QUERYSELECTION( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB );
        if ( sItem != LIT_NONE )
        {
		  HMODULE hResMod;
		  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          pIda->pEntry = (PPROFENTRY) QUERYITEMHANDLE ( hwndDlg,
                                                        ID_DICNAME_ENTRYFIELDS_LB,
                                                        sItem );
          pIda->NewEntry.usLevel = pIda->pEntry->usLevel;
          pIda->NewEntry.usStatus =  RW_STATUS;
          pIda->NewEntry.usSystStatus =  RW_STATUS;
          pIda->NewEntry.usDisplay = NO_DISPLAY;
          pIda->NewEntry.usEntryFieldType = TYPE_SMALL;
          pIda->NewEntry.fVital = FALSE;
          pIda->NewEntry.fSysVital = FALSE;
          pIda->NewEntry.sId = pIda->Prop.usUserNameCount+1;
          if ( pIda->pEntry->usLevel == 4 )
          {
            pIda->NewEntry.sTokenId = TUSER_TOKEN;
          }
          else
          {
            pIda->NewEntry.sTokenId = EUSER_TOKEN;
          } /* endif */

          //remember position in array
          ulEnd = ( pIda->Prop.usLength -
                    (pIda->pEntry - pIda->Prop.ProfEntry)
                    - 1 );

          pIda->fChangeName = FALSE;
          DIALOGBOX( hwndDlg, PCH_DLGPROC, hResMod, ID_PCH_DLG, pIda, fOK );
        }
        else
        {
          UtlErrorHwnd( ERROR_ENTRYFIELD_NOT_SELECTED,
                        MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
        } /* endif */

        //if new field entered, add field to profile ...
        if ( fOK == TRUE )
        {
          if ( ulEnd > 0 )
          {
            memmove( pIda->pEntry+2, pIda->pEntry+1, ulEnd * sizeof(PROFENTRY) );
          } /* endif */
          memcpy( pIda->pEntry+1, &pIda->NewEntry, sizeof(PROFENTRY) );
          pIda->Prop.usLength++;
          pIda->Prop.usUserNameCount++;

          // if new term entered, refill the entry fields listbox
          // with new value
          if ( fOK == TRUE )
          {
            DicFillEntryLB( hwndDlg, pIda, ID_DICNAME_ENTRYFIELDS_LB );
            //position on added field
            SELECTITEM( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB, sItem + 1 );
          } /* endif */
        } /* endif */

        //focus on list box item
        SETFOCUS( hwndDlg, ID_DICNAME_ENTRYFIELDS_LB );

        /*************************************************************/
        /* disable ADD pushbutton if maximum number of fields has    */
        /* been exceeded                                             */
        /*************************************************************/
        if ( pIda->Prop.usLength >= MAX_FIELDS_FOR_DICT )
        {
          ENABLECTRL( hwndDlg, ID_DICNAME_ADDFIELD_PB, FALSE );
        } /* endif */
      }
      break;

    case ID_DICNAME_SET_PB:
      {
        USHORT usI;

        //update propcopy
        pIda->PropCopy = pIda->Prop;

        //set system vital and status values to newly set values
        for ( usI = 0; usI < pIda->Prop.usLength; usI++ )
        {
          pIda->Prop.ProfEntry[usI].usSystStatus =
          pIda->Prop.ProfEntry[usI].usStatus;
          pIda->Prop.ProfEntry[usI].fSysVital =
          pIda->Prop.ProfEntry[usI].fVital;
        } /* endfor */

        //if all okay close dialog
        WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(TRUE), 0L );
      }
      break;
    case ID_DICNAME_ENTRYFIELDS_LB:
    case ID_DICNAME_OMIT_RB:
    case ID_DICNAME_L1_RB:
    case ID_DICNAME_L2_RB:
    case ID_DICNAME_L3_RB:
    case ID_DICNAME_SMALL_RB:
    case ID_DICNAME_LARGE_RB:
    case ID_DICNAME_AUTLOOKUP_CB:
      mResult = SetEntryStructControl( hwndDlg, sId, sNotification );
      break;
  } /* endswitch */
  return( mResult );
}

//dialog to either add a new entry field to a level or to change the name of
//an entry field.

INT_PTR CALLBACK PCH_DLGPROC
(
HWND hwndDlg,     // handle of dialog window
WINMSG msg,       // message id
WPARAM mp1,       // message parameter or NULL
LPARAM mp2        // message parameter or NULL
)
{
  PDICCREATEIDA pIda;   // ida
  MRESULT mResult;       // return value

  switch ( msg )
  {
    case WM_INITDLG:
      mResult = PCH_InitDlg( hwndDlg, mp1, mp2 );
      break;

    case WM_HELP:
  /*************************************************************/
  /* pass on a HELP_WM_HELP request                            */
  /*************************************************************/
  EqfDisplayContextHelp((HWND)  ((LPHELPINFO) mp2)->hItemHandle,
                         &hlpsubtblDicAddFieldDlg[0] );
  mResult = TRUE;  // message processed
  break;


    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );
      mResult = PCH_Command ( hwndDlg, mp1, mp2, pIda );
      break;
    case WM_CLOSE:
      DelCtrlFont(hwndDlg, ID_PCH_DISPLAYED_EF);
      WinDismissDlg( hwndDlg, SHORT1FROMMP1(mp1) );
      mResult = MRFROMSHORT( FALSE );
      break;
    default:
      return( WinDefDlgProc( hwndDlg, msg, mp1, mp2 ) );
      break;
  } /* endswitch */

  return( mResult );
}

MRESULT PCH_InitDlg
(
HWND hwndDlg,       // handle of dialog window
WPARAM mp1,         // first message parameter
LPARAM mp2          // second message parameter
)
{
  PDICCREATEIDA    pIda;           // ptr to IDA of profile dialog
  HMODULE hResMod;
  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  mp1 = mp1;                      // suppress 'unreferenced parameter' msg

  pIda = (PDICCREATEIDA)mp2;

  if ( pIda->fChangeName )
  {
    LOADSTRING( NULLHANDLE, hResMod, IDS_PCH_CHANGE_TEXT, pIda->szBuffer1);
    SETTEXTHWND( hwndDlg, pIda->szBuffer1 );
    strcpy( pIda->szBuffer1, pIda->pEntry->chUserName );
    OEMTOANSI( pIda->szBuffer1 );
    SETTEXT( hwndDlg, ID_PCH_SYSTNAME_EF, pIda->szBuffer1 );
    LOADSTRING( NULLHANDLE, hResMod, IDS_PCH_CHANGE_PB, pIda->szBuffer1 );
    SETTEXT( hwndDlg, ID_PCH_OK_PB, pIda->szBuffer1);
  }
  else
  {
    SETWINDOWID( hwndDlg, ID_DICADDFIELD_DLG );
    LOADSTRING( NULLHANDLE, hResMod, IDS_PCH_ADD_TEXT, pIda->szBuffer1);
    SETTEXTHWND( hwndDlg, pIda->szBuffer1 );

    SETTEXT( hwndDlg, ID_PCH_SYSTNAME_EF, pIda->NewEntry.chSystName );
    if ( strcmp( pIda->NewEntry.chSystName,
                 pIda->NewEntry.chUserName ) != 0 )
    {
      strcpy( pIda->szBuffer1, pIda->NewEntry.chUserName );
      OEMTOANSI( pIda->szBuffer1 );
      SETTEXT( hwndDlg, ID_PCH_DISPLAYED_EF, pIda->szBuffer1 );
    } /* endif */

    LOADSTRING( NULLHANDLE, hResMod, IDS_PCH_ADD_PB, pIda->szBuffer1 );
    SETTEXT( hwndDlg, ID_PCH_OK_PB, pIda->szBuffer1);
  } /* endif */
  SETTEXTLIMIT( hwndDlg, ID_PCH_DISPLAYED_EF, DICTENTRYLENGTH - 1);

  // remember address of IDA
  ANCHORDLGIDA( hwndDlg, pIda );

  SETFOCUS( hwndDlg, ID_PCH_DISPLAYED_EF );
  SetCtrlFnt(hwndDlg, GetCharSet(), ID_PCH_DISPLAYED_EF, ID_PCH_SYSTNAME_EF);

  // activate the dialog window
  WinSetWindowPos( hwndDlg, HWND_TOP, 0, 0, 0, 0, EQF_SWP_SHOW | EQF_SWP_ACTIVATE );

  // leave the focus where we put it
  return( MRFROMSHORT(DIALOGINITRETURN(TRUE)) );
}

MRESULT PCH_Command
(
HWND    hwndDlg,         // handle of change/add dialog window
WPARAM  mp1,             // first parameter of WM_CONTROL
LPARAM  mp2,             // second parameter of WM_CONTROL
PDICCREATEIDA pIda       // pointer to ida
)
{
  MRESULT mResult;         // result of processing
  BOOL    fOK = TRUE;
  LONG    lLen;

  mp2 = mp2;               // suppress unref. par. msg
  mResult = MRFROMSHORT(TRUE);

  switch ( WMCOMMANDID( mp1, mp2 ) )
  {
	 case ID_PCH_HELP_PB:
       mResult = UtlInvokeHelp();
       break;
    case ID_PCH_CANCEL_PB:
    case DID_CANCEL:
      WinPostMsg ( hwndDlg, WM_CLOSE, MP1FROMSHORT( FALSE ), 0l );
      mResult = (MRESULT) FALSE;
      break;
    case ID_PCH_OK_PB:
      lLen = QUERYTEXT( hwndDlg, ID_PCH_DISPLAYED_EF, pIda->szBuffer1 );
      UtlStripBlanks( pIda->szBuffer1 );
      ANSITOOEM( pIda->szBuffer1 );

      if ( !pIda->fChangeName )
      {
        strcpy( pIda->NewEntry.chUserName, pIda->szBuffer1 );
      }
      else
      {
        strcpy( pIda->pEntry->chUserName, pIda->szBuffer1 );
      } /* endif */

      if ( lLen == 0 || ( pIda->szBuffer1[0] == EOS ))
      {
        UtlErrorHwnd( ERROR_FIELDNAME_EMPTY, MB_CANCEL, 0, NULL,
                      EQF_ERROR, hwndDlg );
        SETFOCUS( hwndDlg, ID_PCH_DISPLAYED_EF );
        fOK = FALSE;
      }
      else
      {
        if ( lLen >= DICTENTRYLENGTH )
        {
          UtlErrorHwnd( ERROR_FIELDNAME_TOO_LONG, MB_CANCEL, 0, NULL,
                        EQF_ERROR, hwndDlg );
          SETFOCUS( hwndDlg, ID_PCH_DISPLAYED_EF );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      if ( fOK )
      {
        WinPostMsg ( hwndDlg, WM_CLOSE, MP1FROMSHORT( TRUE ), 0l );
      } /* endif */
      break;

    case ID_PCH_DISPLAYED_EF:
      if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
      {
        ClearIME( hwndDlg );
      } /* endif */
      break;
  } /* endswitch */
  return( mResult );
}

//+----------------------------------------------------------------------------+
//|  Fill entry listbox with data from properties                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
VOID DicFillEntryLB( HWND hwndDlg, PDICCREATEIDA pIda, USHORT idListBox )
{
  PPROFENTRY pEntry;                  // pointer for profile entry processing
  HWND       hwndLB;                  // handle of entry listbox
  USHORT     usI;                     // loop counter
  SHORT      sItem;                   // item index
  CHAR       szAnsiBuffer[DICTENTRYLENGTH]; // buffer for OEM->ANSI conversion

  pEntry = pIda->Prop.ProfEntry;
  hwndLB = WinWindowFromID( hwndDlg, idListBox );

  ENABLEUPDATEHWND_FALSE( hwndLB );           // avoid flickering
  DELETEALLHWND( hwndLB );                    // clear listbox

  // fill listbox with profile entries
  for ( usI = 0; usI < pIda->Prop.usLength; usI++, pEntry++ )
  {
    strcpy( szAnsiBuffer, pEntry->chUserName );
    OEMTOANSI( szAnsiBuffer );
    sItem = INSERTITEMENDHWND( hwndLB, szAnsiBuffer );
    SETITEMHANDLEHWND( hwndLB, sItem, pEntry );
  } /* endfor */

  ENABLEUPDATEHWND_TRUE( hwndLB );             // show changed listbox
  SELECTITEMHWND( hwndLB, 0 );                  // select first listbox item
} /* end of DicFillEntryLB */


//+----------------------------------------------------------------------------+
//|  Load the strings for the dictionary field user names from the resource    |
//|  file                                                                      |
//|  It is assumed that the user names contain the string ID of the string     |
//|  to load                                                                   |
//+----------------------------------------------------------------------------+
BOOL DicLoadFieldNames
(
PPROPDICTIONARY pProp                // dictionary to be processed
)
{
  BOOL fOK = TRUE;                     // function return code
  USHORT usI;                          // loop index
  SHORT sStringID;                     // ID of string
  CHAR  chUserName[DICTENTRYLENGTH];   // buffer for user names
  HMODULE hResMod;
  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  for ( usI = 0; usI < pProp->usLength; usI++ )
  {
    sStringID = (SHORT)atoi( pProp->ProfEntry[usI].chUserName );
    if ( sStringID )
    {
      chUserName[0] = EOS;
      LOADSTRING( NULLHANDLE, hResMod, sStringID, chUserName );
      if ( chUserName[0] != EOS  )
      {
        strcpy( pProp->ProfEntry[usI].chUserName, chUserName );
      } /* endif */
    } /* endif */
  } /* endfor */

  return( fOK );
} /* end of DicLoadFieldNames */
