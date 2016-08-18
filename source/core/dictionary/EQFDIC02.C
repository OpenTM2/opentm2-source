//************************ Prolog *********************************
//
//               Copyright (C) 1990-2012, International Business Machines      
//               Corporation and others. All rights reserved         
//
//  Short description: Dialog for querying dictionary password.
//
//  Author:
//
//  Description:       The user is asked to enter the password associated with
//                     a protected dictionary.
//
//  Calling Hierarchy:
//
//  Entry point:  Dict1PasswordDlg()
//
// *********************** End Prolog *********************************

#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdimp.id"
#include "eqfdtag.h"
#include "eqfdde.h"
#include "eqfdicti.h"             // Private include file of dictionary handler

#define       TXT_SIZE      300

extern HELPSUBTABLE hlpsubtblDictPasswordDlg[];
extern HELPSUBTABLE hlpsubtblDict2PasswordDlg[];


INT_PTR CALLBACK DICT1PASSWORDDLG
(
HWND   hwndDlg,        //handle of dialog window
WINMSG message,        //message id
WPARAM mp1,            //message parameter
LPARAM mp2             //message parameter
)
{
  MRESULT      mResult = FALSE;             //return code
  BOOL         fOK = TRUE;                  //success indicator
  CHAR         szName[MAX_LONGFILESPEC];
  CHAR         chBuffer1[TXT_SIZE];         //general work buffer
  CHAR         chBuffer2[TXT_SIZE];         //general work buffer
  PSZ          pName;                       //pointer to string
  PSZ          pDot;                        //pointer to string
  ULONG        ulLength;                    //length of returned string
  PPROPDICTIONARY pDictProp;                //pointer to dict props
  PSZ          pszMsgTable[1];              //error msg parameter list

  switch ( message)
  {
    case ( WM_INITDLG ):
      //anchor dict props
      pDictProp = (PPROPDICTIONARY) mp2;
      ANCHORDLGIDA( hwndDlg, pDictProp );

      if ( pDictProp != NULC )
      {
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        if ( pDictProp->szLongName[0] != EOS )
        {
          strcpy( szName, pDictProp->szLongName );
          OEMTOANSI( szName );
        }
        else
        {
          Utlstrccpy( szName,
                      pDictProp->PropHead.szName,
                      DOT );
        }
        pName = szName;
        pszMsgTable[0] = pName;

        //fill in the password text on the dialog panel
        LOADSTRING( NULLHANDLE, hResMod, IDS_PASSWORD_TEXT, chBuffer1 );
        DosInsMessage( pszMsgTable,
                       1,
                       chBuffer1,
                       strlen( chBuffer1 ),
                       chBuffer2,
                       (sizeof ( chBuffer2 ) - 1),
                       &ulLength );

        chBuffer2[ulLength] = EOS;

        //add text to dialog
        SETTEXT( hwndDlg, ID_DICTPASSWORD_STATIC_TEXT, chBuffer2 );
      } /* endif */

      //length of password entry field is 8
      SETTEXTLIMIT( hwndDlg, ID_DICTPASSWORD_PSW_EF, MAX_FNAME - 1 );
      break;

    case WM_HELP:
/*************************************************************/
/* pass on a HELP_WM_HELP request                            */
/*************************************************************/
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblDictPasswordDlg[0] );
      mResult = TRUE;  // message processed
      break;




    case ( WM_CLOSE ):
      //distroy dialog
      WinDismissDlg( hwndDlg, SHORT1FROMMP1( mp1 ) );
      break;

    case WM_CONTROL:
      break;

    case ( WM_COMMAND ):
      mResult = MRFROMSHORT( TRUE);

      switch (WMCOMMANDID( mp1, mp2 ))
      {
		case ID_DICTPASSWORD_HELP_PB:
		  mResult = UtlInvokeHelp();
	      break;
        case DID_CANCEL:
        case ID_DICTPASSWORD_CANCEL_PB:
          WinPostMsg( hwndDlg, WM_CLOSE, NULL, NULL );
          break;

        case ID_DICTPASSWORD_ENTER_PB:
          //get access to property path
          pDictProp = ACCESSDLGIDA( hwndDlg, PPROPDICTIONARY );

          //query password sle
          QUERYTEXT( hwndDlg, ID_DICTPASSWORD_PSW_EF, szName );
          UtlStripBlanks( szName );
          if ( szName[0] != EOS )
          {
            if ( !DicGetCheckPassword( szName, &pDictProp->ulPassWord, TRUE ) )
            {
              pDot = strrchr( pDictProp->PropHead.szName, DOT );
              if ( pDictProp->szLongName[0] != EOS )
              {
                strcpy( szName, pDictProp->szLongName );
                OEMTOANSI( szName );
                pName = szName;
              }
              else
              if ( pDot )
              {
                Utlstrccpy( szName,
                            pDictProp->PropHead.szName,
                            DOT );
                pName = szName;
              }
              else
              {
                pName = pDictProp->PropHead.szName ;
              } /* endif */

              UtlErrorHwnd( ERRMSG_WRONG_PASSWORD, MB_CANCEL, 1, &pName,
                            EQF_ERROR, hwndDlg );
              fOK = FALSE;
              SETFOCUS( hwndDlg, ID_DICTPASSWORD_PSW_EF );
            }
            else
            {
              //password correct, remove protected flag
              pDictProp->ulPassWord = 0L;
              pDictProp->fProtected = FALSE;
            } /* endif */
          }
          else
          {
            UtlErrorHwnd( ERROR_NO_PASSWORD_ENTERED, MB_CANCEL,
                          0, NULL, EQF_ERROR, hwndDlg );
            SETFOCUS( hwndDlg, ID_DICTPASSWORD_PSW_EF );
            fOK = FALSE;
          } /* endif */

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
}

INT_PTR CALLBACK DICT2PASSWORDDLG
(
HWND   hwndDlg,        //handle of dialog window
WINMSG message,        //message id
WPARAM mp1,            //message parameter
LPARAM mp2             //message parameter
)
{
  MRESULT      mResult = FALSE;             //return code
  BOOL         fOK = TRUE;                  //success indicator
  CHAR         szName1[MAX_FNAME];          //buffer eight chars long
  CHAR         szName2[MAX_FNAME];          //buffer 8 chars long
  CHAR         chBuffer[TXT_SIZE];          //general work buffer
  PULONG       pulPassword;
  HMODULE hResMod;
  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  switch ( message)
  {
    case ( WM_INITDLG ):
      pulPassword = (PULONG) mp2;
      ANCHORDLGIDA( hwndDlg, pulPassword );

      //fill in the password text on the dialog panel
      LOADSTRING( NULLHANDLE, hResMod, IDS_2PASSWORDS_TEXT, chBuffer );

      //add text to dialog
      SETTEXT( hwndDlg, ID_DICT2PASSWORD_STATIC_TEXT, chBuffer );

      //length of password entry field is 8
      SETTEXTLIMIT( hwndDlg, ID_DICT2PASSWORD_PSW1_EF, MAX_FNAME - 1 );
      SETTEXTLIMIT( hwndDlg, ID_DICT2PASSWORD_PSW2_EF, MAX_FNAME - 1 );
      break;

    case WM_HELP:
/*************************************************************/
/* pass on a HELP_WM_HELP request                            */
/*************************************************************/
      EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblDict2PasswordDlg[0] );
      mResult = TRUE;  // message processed
      break;



    case ( WM_CLOSE ):
      //distroy dialog
      WinDismissDlg( hwndDlg, SHORT1FROMMP1( mp1 ) );
      break;

    case ( WM_COMMAND ):
      mResult = MRFROMSHORT( TRUE);

      switch (WMCOMMANDID( mp1, mp2 ))
      {
		case ID_DICT2PASSWORD_HELP_PB:
		  mResult = UtlInvokeHelp();
	      break;
        case DID_CANCEL:
        case ID_DICT2PASSWORD_CANCEL_PB:
          WinPostMsg( hwndDlg, WM_CLOSE, NULL, NULL );
          break;

        case ID_DICT2PASSWORD_ENTER_PB:
          pulPassword = ACCESSDLGIDA( hwndDlg, PULONG );

          //query password sles
          QUERYTEXT( hwndDlg, ID_DICT2PASSWORD_PSW1_EF, szName1 );
          UtlStripBlanks( szName1 );
          QUERYTEXT( hwndDlg, ID_DICT2PASSWORD_PSW2_EF, szName2 );
          UtlStripBlanks( szName2 );

          if ( strcmp( szName1, szName2 ) != 0 )
          {
            //error entering password
            UtlErrorHwnd( ERROR_PASSWORDS_NOT_INDENTICAL,
                          MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
            fOK = FALSE;
            SETFOCUS( hwndDlg, ID_DICT2PASSWORD_PSW1_EF );
          }
          else
          {
            //change password in chars to encoded ulong
            if ( szName1[0] != EOS )
            {
              if ( !DicGetCheckPassword( szName1, pulPassword, FALSE ) )
              {
                UtlError( ERROR_INV_PASSWORD, MB_CANCEL,
                          0, NULL, EQF_ERROR );
                fOK = FALSE;
                SETTEXT( hwndDlg, ID_DICT2PASSWORD_PSW1_EF, EMPTY_STRING );
                SETTEXT( hwndDlg, ID_DICT2PASSWORD_PSW2_EF, EMPTY_STRING );
                SETFOCUS( hwndDlg, ID_DICT2PASSWORD_PSW1_EF );
              } /* endif */
            }
            else
            {
              UtlErrorHwnd( ERROR_NO_PASSWORD_ENTERED, MB_CANCEL,
                            0, NULL, EQF_ERROR, hwndDlg );
              fOK = FALSE;
              SETFOCUS( hwndDlg, ID_DICT2PASSWORD_PSW1_EF );
            } /* endif */
          } /* endif */
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
}

BOOL DicGetCheckPassword               // compute a password value or check
// the given password against password
// value
// return value for fCheckPassword = TRUE:
//   TRUE  = password matchs password value
//   FALSE = password does not match
// return value for fCheckPassword = FALSE:
//   TRUE  = password has correct syntax
//   FALSE = password is invalid

(
PSZ              pszPassword,        // password string
PULONG           pulPassword,        // computed password value
BOOL             fCheckPassword      // flag: TRUE  = check given password
//       FALSE = compute password value
)
{
  ULONG            ulValue;            // computed password value
  BOOL             fOK = TRUE;         // function return code

  /********************************************************************/
  /* Compute the password value using the corrected algorythm         */
  /********************************************************************/
  {
    USHORT usI, usResult, usFactor;

    usI = 0;
    usFactor = 1;
    ulValue = 0L;
    while ( pszPassword[usI] != EOS )
    {
      usResult = (USHORT)toupper(pszPassword[usI]) * usFactor;
      ulValue += (ULONG)usResult;
      usI++;
      usFactor++;
    } /* endwhile */

    /******************************************************************/
    /* ensure that the lowest bit of the computed value is set        */
    /* (in order to distinguish it from the values computed using the */
    /* wrong algorythm)                                               */
    /******************************************************************/
    ulValue |= 1L;
  }


  if ( fCheckPassword )
  {
    /******************************************************************/
    /* Get type of algorythm to be used for password value            */
    /******************************************************************/
    if ( (*pulPassword & 1L) == 0L )
    {
      /****************************************************************/
      /* Use old (=wrong implemented) algorythm                       */
      /****************************************************************/
      USHORT usResult;
      USHORT usI = 0;
      ulValue = 0L;
      while ( pszPassword[usI] != EOS )
      {
        usResult = (USHORT)pszPassword[usI] * 4;
        ulValue  += (ULONG)usResult;
        usI++;
      } /* endwhile */

    } /* endif */

    fOK = ( ulValue == *pulPassword );
  }
  else
  {
    /******************************************************************/
    /* Check syntax of new password (all characters must be           */
    /* alphanumeric and it has to have at least a length of 4 chars)  */
    /******************************************************************/
    if ( strlen(pszPassword) >= 4 )
    {
      while ( *pszPassword && isalnum(*pszPassword) )
      {
        pszPassword++;
      } /* endwhile */
      if ( *pszPassword == EOS )
      {
        fOK = TRUE;
        *pulPassword = ulValue;
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      fOK = FALSE;                     // password length incorrect
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function DicGetCheckPassword */
