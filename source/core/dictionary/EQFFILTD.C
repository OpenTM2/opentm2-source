//+----------------------------------------------------------------------------+
//|  EQFFILTD.C - EQF Filter Control                                           |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_FILT             // dictionary filter functions
#include <eqf.h>                  // General Translation Manager include file

#include "OtmDictionaryIF.H"
#include "eqffilti.h"                  // Filter private include file
#include "eqffilt.id"

#if !defined( ERROR_FILTER_DISCARDCHANGES )
  #define ERROR_FILTER_DISCARDCHANGES   4711
#endif

/**********************************************************************/
/* Convert a string to uppercase which is already converted to ANSI   */
/**********************************************************************/
#define ANSIUPPER( string ) AnsiUpper( string )

extern HELPSUBTABLE hlpsubtblFiltDlg[];
extern HELPSUBTABLE hlpsubtblFiltop1Dlg[];

FILTOPERATOR FiltOperatorTable[] =
{

  { OP_EQUAL,               "="},
  { OP_NOTEQUAL,            "<>"},
  { OP_SMALLER,             "<"},
  { OP_GREATER,             ">"},
  { OP_SMALLEREQUAL,        "<="},
  { OP_GREATEREQUAL,        ">="},
  { OP_LIKE,                "LIKE"},
  { OP_BETWEEN,             "BETWEEN"},
  { OP_IN,                  "IN"},
  { OP_AND,                 "AND"},
  { OP_OR,                  "OR"},
  { OP_NOT,                 "NOT"},
  { OP_OPENBRACKET,         "("},
  { OP_CLOSEDBRACKET,       ")"},
  { OP_END,                 ""},
};

/**********************************************************************/
/* Maximum number of bytes in dialog MLEs                             */
/**********************************************************************/
#define MAX_MLE_SIZE     4096

//Structure of filter dialog ida
typedef struct _FILTIDA
{
  HDCB             hDCB;               // handle of active dictionary
  USHORT           usOpDlg;                      //indicates operator dlg chosen
  CHAR             szDictName[MAX_EQF_PATH];     //dict name
  CHAR             szPathBuffer[MAX_EQF_PATH];   //filter path
  CHAR             szStartName[MAX_FNAME];       // initial filter name
  CHAR             szName[MAX_FNAME];            //filter name without path
  CHAR             szSaveName[MAX_FNAME];        // save to filter name
  CHAR             szString[16000];              //string buffer
  CHAR             szTemp[16000];                //string buffer
  CHAR             szValue[16000];               //string buffer
  CHAR             szOperator[OPERATOR_LEN];     //operator string
  CHAR             szFieldName[NAME_LEN];        //field name string
  CHAR             szOpFieldName[NAME_LEN];      //field name string
  USHORT           usLine;                       //number of lines in mle
  PUCHAR           pucLineStart;                 //pos in mle where line begins
  PUCHAR           pucErrorPos;                  //pos in mle where error is
  PPOLISHSTACK     pPosStack;                    //ptr to current pos in filter
  PUSHORT          pusSelPos;                    //ptr to sel names offset array
  PUSHORT          pusAllPos;                    //ptr to all names offset array
  FILTER           F;                            //filter
  HWND             hwndSelMLE;                   //handle of SELECT condition mle
  HWND             hwndWhereMLE;                 //handle of WHERE condition mle
  PPROPDICTIONARY  pDictProp;                    //ptr to dictionary properties
} FILTIDA , *PFILTIDA;

/**********************************************************************/
/* Function prototypes                                                */
/**********************************************************************/
INT_PTR CALLBACK DictFiltDlg( HWND, WINMSG, WPARAM, LPARAM );
MRESULT FiltInit( HWND, WPARAM, LPARAM );
MRESULT FiltCommand( HWND, SHORT, SHORT );
MRESULT FiltControl( HWND, SHORT, SHORT );
INT_PTR CALLBACK FiltOpDlg ( HWND, WINMSG, WPARAM, LPARAM );
static VOID FiltFillEntryLB( HWND, PPROPDICTIONARY );
static VOID FillFiltLB( HWND, PFILTIDA, USHORT );
static VOID FiltQuoteStringIdentification( PFILTIDA );
static BOOL FiltReadToken( PFILTIDA, PUCHAR * );
static VOID FiltSkipBlanks ( PUCHAR * );
static BOOL FiltIdentifier ( HWND, PFILTIDA, BOOL );
static BOOL FiltSelectTest( HWND, PFILTIDA );
static BOOL FiltExpression( HWND, PFILTIDA, PUCHAR * );
static BOOL FiltSimpleExpression( HWND, PFILTIDA, PUCHAR *  );
static BOOL FiltPushOnBuffer( PFILTIDA );
static BOOL FiltPushOnStack( PFILTIDA, SHORT, USHORT );
static BOOL FiltConstant ( HWND, PFILTIDA, PUCHAR *);
static SHORT FiltGetOperatorID( PSZ );
static BOOL FiltTerm ( HWND, PFILTIDA, PUCHAR * );
static BOOL FiltPushOnSelNames( PFILTIDA, USHORT );
static BOOL FiltPushOnAllNames( PFILTIDA, USHORT );
static BOOL FiltBetween( HWND, PFILTIDA, PUCHAR * );
VOID FiltPosToError( HWND hwndMLE, PUCHAR pucBuffer, PUCHAR pucStartPos, PUCHAR pucEndPos );
VOID FiltBlankToUS( PSZ pszFieldname );

/**********************************************************************/
/* Program body                                                       */
/**********************************************************************/
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DictionaryFilter                                         |
//+----------------------------------------------------------------------------+
//|Function call:     DictionaryFilter( hDCB, pFiltName, hwndDlg )             |
//+----------------------------------------------------------------------------+
//|Description:       Calls up the filter dialog                               |
//+----------------------------------------------------------------------------+
//|Parameters:        HDCB hDCB        - dictionary calling filter dialog      |
//|                   PSZ  pFiltName   - filter selected                       |
//|                   HWND hwndDlg     - handle of parent dialog               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     create global structure - FiltIda                        |
//|                   remember selected dictionary in global structure         |
//|                   remember selected filter in global structure             |
//|                   call filter dialog                                       |
//+----------------------------------------------------------------------------+

VOID DictionaryFilter( HDCB hDCB, PSZ pFiltName, HWND hwndDlg )
{
  PFILTIDA        pFiltIda;                    //global structure
  BOOL            fOK = TRUE;                   //return value
  INT_PTR         iRC;

  //create FiltIda
  fOK = (UtlAlloc( (PVOID *)&pFiltIda, 0L, sizeof(FILTIDA), ERROR_STORAGE ));

  if ( fOK )
  {
	HMODULE hResMod;
	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    //remember selected dictionary in Ida
    pFiltIda->hDCB = hDCB;

    //remember selected filter in Ida
    if ( pFiltName != NULL )
    {
      strcpy( pFiltIda->szStartName, pFiltName );
    } /* endif */

    DIALOGBOX( hwndDlg, DictFiltDlg, hResMod, ID_FILT_DLG, pFiltIda, iRC );
    if ( iRC == TRUE )
    {
      strcpy( pFiltName, pFiltIda->szSaveName );
    }
    else
    {
      *pFiltName = EOS;
    } /* endif */
    UtlAlloc( (PVOID *)&pFiltIda, 0L, 0L, NOMSG );
  } /* endif */
}  /* end DictionaryFilter */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DictFiltDlg                                              |
//+----------------------------------------------------------------------------+
//|Function call:     DictFiltDlg( hwndDlg, msg, mp1, mp2 )                    |
//+----------------------------------------------------------------------------+
//|Description:       Handles the filter dialog - initialization, controls     |
//|                                    and commands.                           |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   msg         - message parameter                          |
//|                   mp1         - first parameter                            |
//|                   mp2         - second parameter                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch message                                           |
//|                     case wm_initdlg:                                       |
//|                       call function FiltInit                               |
//|                     case wm_control:                                       |
//|                       call function FiltControl                            |
//|                     case wm_char:                                          |
//|                       if no enter key is pressed or no listbox is active   |
//|                         pass message to default dialog procedure           |
//|                     case wm_command:                                       |
//|                       call function FiltCommand                            |
//|                     case wm_close:                                         |
//|                       free allocated memory                                |
//|                       dismiss dialog                                       |
//|                     default:                                               |
//|                       default window processing                            |
//|                   endswitch                                                |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK DictFiltDlg
(
HWND hwndDlg,     // handle of dialog window
WINMSG msg,       // message id
WPARAM mp1,       // message parameter or NULL
LPARAM mp2        // message parameter or NULL
)
{
  MRESULT           mResult = 0;      // return value
  PFILTIDA          pIda;             // pointer to global structure

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FILT_DLG, mp2 ); break;

    case WM_INITDLG:
      mResult = FiltInit( hwndDlg, mp1, mp2 );
      break;

    case WM_HELP:
/*************************************************************/
/* pass on a HELP_WM_HELP request                            */
/*************************************************************/
      EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblFiltDlg[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_COMMAND:
      mResult = FiltCommand( hwndDlg, WMCOMMANDID( mp1, mp2 ),
                             WMCOMMANDCMD( mp1, mp2 ) );
      break;

    //case WM_CHAR:
    //  /**************************************************************/
    //  /* prevent selection of default pushbutton when enter is      */
    //  /* pressed in a listbox                                       */
    //  /**************************************************************/
    //  if ( ( SHORT1FROMMP1(mp1) == VK_ENTER ) ||
    //       ( SHORT1FROMMP1(mp1) == VK_NEWLINE ) )
    //  {
    //    /************************************************************/
    //    /* get ID of control having the focus                       */
    //    /* (loop is required for combo boxes which consist of       */
    //    /*  several individual controls)                            */
    //    /************************************************************/
    //    hwndFocus = GETFOCUS();
    //    while ( hwndFocus )
    //    {
    //      if ( GETPARENT( hwndFocus ) == hwndDlg )
    //      {
    //        break;               // leave loop, control is owned by dialog
    //      }
    //      else
    //      {
    //        hwndFocus = GETPARENT( hwndFocus );
    //      } /* endif */
    //    } /* endwhile */
    //    usFocusID = (USHORT)IDFROMWINDOW( hwndFocus );

    //    if ( (usFocusID == ID_FILT_OPERATORS_LB) ||
    //         (usFocusID == ID_FILT_AVAILFIELDS_LB) )
    //    {
    //      mResult = MRFROMSHORT( TRUE );
    //    }
    //    else
    //    {
    //      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
    //    } /* endif */
    //  }
    //  else
    //  {
    //    mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
    //  } /* endif */
    //  break;

    case DM_GETDEFID:
      {
        HWND   hwndFocus;          // handle of focus window

        if ( GetKeyState(VK_RETURN) & 0x8000 )
        {
          hwndFocus = GetFocus();
          if ( hwndFocus == GetDlgItem( hwndDlg, ID_FILT_OPERATORS_LB ) )
          {
            FiltControl( hwndDlg, ID_FILT_OPERATORS_LB, LN_ENTER );
            mResult = TRUE;
          }
          else if ( hwndFocus == GetDlgItem( hwndDlg, ID_FILT_AVAILFIELDS_LB ) )
          {
            FiltControl( hwndDlg, ID_FILT_AVAILFIELDS_LB, LN_ENTER );
            mResult = TRUE;
          } /* endif */
        } /* endif */
      }
      break;

    case WM_CLOSE:
      // Get access to IMPIDA
      pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );

      //free allocated memory
      FiltFree ( &pIda->F );
      DelCtrlFont (hwndDlg, ID_FILT_DESCR_EF);
      DelCtrlFont (hwndDlg, ID_FILT_SELECT_MLE);
      WinDismissDlg( hwndDlg, SHORT1FROMMP1(mp1) );
      mResult = MRFROMSHORT( FALSE );
      break;
    default:
      return( WinDefDlgProc( hwndDlg, msg, mp1, mp2 ) );
      break;
  } /* endswitch */

  return( mResult );
}  /* end DictFiltDlg */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltInit                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     FiltInit( hwndDlg, mp1, mp2 )                            |
//+----------------------------------------------------------------------------+
//|Description:       Handles initialization of filter dialog.                 |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   mp1         - first parameter                            |
//|                   mp2         - second parameter                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     fill Name listbox with filters                           |
//|                   add the empty/new filter - MODELFILT                     |
//|                   select filter chosen in dialog calling filter dialog     |
//|                   open dictionary properties - dictionary from the         |
//|                                                calling the filter dialog   |
//|                   fill available listbox with fieldnames in dictionary     |
//|                       filling balnks between strings with underscore       |
//|                   close dictionary properties                              |
//|                                                                            |
//|                   if all OK                                                |
//|                     fill operators listbox with available operators        |
//|                                                                            |
//|                   activate the dialog window                               |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+
MRESULT FiltInit
(
HWND hwndDlg,                       // handle of dialog window
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  PFILTIDA       pIda;                // ptr to IDA
  MRESULT        mResult = FALSE;     // return code of this function
  HWND           hwndOpLB;            // handle of entry listbox
  USHORT         usI;                 // loop counter
  SHORT          sItem;               // listbox item index
  USHORT         usRC;                // return code of called functions
  /* 3@52A */
  BOOL           fOK = TRUE;          // internal OK flag
  HDCB           ahDCB[MAX_DICTS+1];  // list of associated dictionaries
  USHORT         usNumOfDicts;        // number of dictionaries in association

  mp1;                                // suppress 'unreferenced parameter' msg

  /*******************************************************************/
  /* anchor IDA pointer                                              */
  /*******************************************************************/
  pIda = (PFILTIDA)mp2;
  ANCHORDLGIDA( hwndDlg, pIda );

  /*******************************************************************/
  /* Get MLE window handles                                          */
  /*******************************************************************/
  pIda->hwndWhereMLE = WinWindowFromID( hwndDlg, ID_FILT_WHERE_MLE );
  pIda->hwndSelMLE   = WinWindowFromID ( hwndDlg, ID_FILT_SELECT_MLE );

  /*******************************************************************/
  /* Set text limits and initial button states                       */
  /*******************************************************************/
  CBSETTEXTLIMIT( hwndDlg, ID_FILT_SAVETO_CB, MAX_FNAME - 1 );
  SETTEXTLIMIT( hwndDlg, ID_FILT_DESCR_EF, MAX_DESCRIPTION - 1 );

  MLESETTEXTLIMITHWND( pIda->hwndWhereMLE, MAX_MLE_SIZE );
  MLESETTEXTLIMITHWND( pIda->hwndSelMLE, MAX_MLE_SIZE );
  ENABLECTRL( hwndDlg, ID_FILT_DELETE_PB, FALSE );

  /*******************************************************************/
  /* fill Name and save to comboboxes with filter names              */
  /*******************************************************************/
  EqfSend2Handler( FILTERHANDLER, WM_EQF_INSERTNAMES,
                   MP1FROMHWND( WinWindowFromID( hwndDlg, ID_FILT_NAME_LB) ),
                   0L );
  UtlCopyListBox( WinWindowFromID( hwndDlg, ID_FILT_SAVETO_CB ),
                  WinWindowFromID( hwndDlg, ID_FILT_NAME_LB ) );

  /*******************************************************************/
  /* select chosen filter                                            */
  /*******************************************************************/
  CBSEARCHSELECT( sItem, hwndDlg, ID_FILT_NAME_LB, pIda->szStartName );
  SETTEXT( hwndDlg, ID_FILT_SAVETO_CB, pIda->szStartName );

  /*******************************************************************/
  /* Get dictionary properties (for associated dictionaries use      */
  /* properties of first dictionary in association)                  */
  /*******************************************************************/
  usRC = AsdRetDictList( pIda->hDCB, ahDCB, &usNumOfDicts );
  if ( usRC != LX_RC_OK_ASD )
  {
    /*****************************************************************/
    /* Error accessing dictionary                                    */
    /*****************************************************************/
    UtlError( usRC, MB_CANCEL, 0, NULL, QDAM_ERROR );
    fOK = FALSE;
  }
  else
  {
    /*****************************************************************/
    /* Use handle of first dictionary to get properties pointer      */
    /*****************************************************************/
    usRC = AsdRetPropPtr( NULL, ahDCB[0], &pIda->pDictProp );
    if ( usRC != LX_RC_OK_ASD )
    {
      /***************************************************************/
      /* Error accessing dictionary properties                       */
      /***************************************************************/
      UtlError( usRC, MB_CANCEL, 0, NULL, QDAM_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Fill available fields listbox with entry fields.                */
  /* Blanks between names are filled by '_'                          */
  /*******************************************************************/
  if ( fOK )
  {
    FiltFillEntryLB( hwndDlg, pIda->pDictProp );
  } /* endif */

  /****************************************************************/
  /* fill operators listbox with valid operators                  */
  /****************************************************************/
  if ( fOK )
  {
    hwndOpLB = WinWindowFromID( hwndDlg, ID_FILT_OPERATORS_LB );
    ENABLECTRLHWND( hwndOpLB, FALSE );
    usI = 0;
    while ( FiltOperatorTable[usI].sOperator != OP_END )
    {
      if ( FiltOperatorTable[usI].szName[0] != EOS )
        INSERTITEMENDHWND( hwndOpLB, FiltOperatorTable[usI].szName );
      usI++;
    } /* endwhile */
    ENABLECTRLHWND( hwndOpLB, TRUE );
  } /* endif */

  /*******************************************************************/
  /* Remove dialog if errors occurred                                */
  /*******************************************************************/
  if ( !fOK )
  {
    WinDismissDlg( hwndDlg, FALSE );
  } /* endif */

  /*******************************************************************/
  /* Set focus to description entry field of dialog                  */
  /*******************************************************************/
  if ( fOK )
  {
    SetCtrlFnt (hwndDlg, GetCharSet(),
                ID_FILT_DESCR_EF, ID_FILT_AVAILFIELDS_LB );
    SetCtrlFnt (hwndDlg, GetCharSet(),
                ID_FILT_SELECT_MLE, ID_FILT_WHERE_MLE );
    SETFOCUS( hwndDlg, ID_FILT_DESCR_EF );
    mResult = MRFROMSHORT(TRUE);
  } /* endif */

  return( mResult );
} /* end FiltInit */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltControl                                              |
//+----------------------------------------------------------------------------+
//|Function call:     FiltControl( hwndDlg, msg, mp1, mp2 )                    |
//+----------------------------------------------------------------------------+
//|Description:       Handles all of the filter dialog controls.               |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   msg         - message parameter                          |
//|                   mp1         - first parameter                            |
//|                   mp2         - second parameter                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch dialog control                                    |
//|                     case operators listbox:                                |
//|                     if double click or enter on an item in listbox         |
//|                     switch operator id                                     |
//|                       case between:                                        |
//|                         call up dialog for between handling                |
//|                       case in:                                             |
//|                         call up dialog for in handling                     |
//|                       case =:                                              |
//|                       case <:                                              |
//|                       case >:                                              |
//|                       case >=:                                             |
//|                       case <=:                                             |
//|                       case <>:                                             |
//|                       case like:                                           |
//|                         call up dialog for handling the above operators    |
//|                       case AND:                                            |
//|                       case OR:                                             |
//|                       case NOT:                                            |
//|                         add to where mle at mle cursor position            |
//|                     endswitch                                              |
//|                                                                            |
//|                   case available fields listbox:                           |
//|                     if double click or enter on item in listbox            |
//|                       get fieldname selected                               |
//|                       add blanks at end of string                          |
//|                       add to select mle at current select mle cursor pos   |
//|                     endif                                                  |
//|                   case either mle:                                         |
//|                     remember that changes were done - set flag             |
//|                   case name listbox:                                       |
//|                     if a filter is selected                                |
//|                       get selected filter                                  |
//|                       if new filter                                        |
//|                         add special text in description field              |
//|                         empty both mle                                     |
//|                         select on filter in save to combo listbox          |
//|                       else                                                 |
//|                         build property path name                           |
//|                         open property file                                 |
//|                         fill description field                             |
//|                         fill select mle                                    |
//|                         fill where mle                                     |
//|                         select same filter in save to combo-box            |
//|                       endif                                                |
//|                     endif                                                  |
//|                   endswitch                                                |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+

MRESULT FiltControl
(
HWND    hwndDlg,                     // handle of window
SHORT   sId,                         // id in action
SHORT   sNotification                // notification
)
{
  MRESULT        mResult = MRFROMSHORT(TRUE); // result of processing
  PFILTIDA       pIda;                 // ptr to IDA
  SHORT          sItem;                // listbox selection
  INT_PTR        iResult = 0;         // return value
  USHORT         usRC;                 // function return code
  USHORT         usNoOfNames;          // number of names in SELECT MLE
  USHORT         usI;                  // loop index
  BOOL           fChange;              // TRUE = MLE data has been changed
  USHORT         usMBCode;             // message box return code
  CHAR           szTempName[MAX_FNAME];// buffer for filter name
  USHORT         usAnchor, usCursor;
  USHORT         usLength;
  BOOL           fOK = TRUE;

  switch ( sId )
  {
    case ID_FILT_OPERATORS_LB:
      if ( sNotification == LN_ENTER )
      {
        pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );
        sItem = QUERYSELECTION( hwndDlg, ID_FILT_OPERATORS_LB );
        if ( sItem != LIT_NONE )
        {
          /**********************************************************/
          /* Get selected operator                                  */
          /**********************************************************/
          strcpy( pIda->szOperator, FiltOperatorTable[sItem].szName );

          /**********************************************************/
          /* Get currently selected dictionary field                */
          /**********************************************************/
          {
            SHORT sFieldItem = QUERYSELECTION( hwndDlg, ID_FILT_AVAILFIELDS_LB );
            if ( sFieldItem != LIT_NONE )
            {
              //load name of selected item in listbox into szString
              QUERYITEMTEXT( hwndDlg, ID_FILT_AVAILFIELDS_LB,
                             sFieldItem, pIda->szOpFieldName );
            } /* endif */
          }


          if ( pIda->szOpFieldName[0] != NULC )
          {
			HMODULE hResMod;
			hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            switch ( FiltOperatorTable[sItem].sOperator )
            {
              case OP_BETWEEN :
                pIda->usOpDlg = 2;
                DIALOGBOX( hwndDlg, FiltOpDlg, hResMod, ID_FILTOP2_DLG, pIda, iResult );
                break;
              case OP_IN :
                pIda->usOpDlg = 3;
                DIALOGBOX( hwndDlg, FiltOpDlg, hResMod, ID_FILTOP3_DLG, pIda, iResult );
                break;
              case OP_EQUAL :
              case OP_NOTEQUAL :
              case OP_SMALLER :
              case OP_GREATER :
              case OP_SMALLEREQUAL :
              case OP_GREATEREQUAL :
              case OP_LIKE :
                pIda->usOpDlg = 1;
                DIALOGBOX( hwndDlg, FiltOpDlg, hResMod, ID_FILTOP1_DLG, pIda, iResult );
                break;
              case OP_OPENBRACKET :
              case OP_CLOSEDBRACKET :
              case OP_AND :
              case OP_OR :
              case OP_NOT :
                //add to szString plus blank
                strcpy( pIda->szString, pIda->szOperator );
                strcat( pIda->szString, "  " );

                //add to mle at cursor position
                MLEINSERTHWND( pIda->hwndWhereMLE, pIda->szString );
                MLEQUERYSELHWND( pIda->hwndWhereMLE, usAnchor, usCursor );
                MLESETSELHWND( pIda->hwndWhereMLE, usAnchor, usCursor );
                break;
            } /* endswitch */

            if ( iResult )
            {
              //add to mle at cursor position
              MLEINSERTHWND( pIda->hwndWhereMLE, pIda->szString );
              MLEQUERYSELHWND( pIda->hwndWhereMLE, usAnchor, usCursor );
              MLESETSELHWND( pIda->hwndWhereMLE, usAnchor, usCursor );
            } /* endif */
          }
          else
          {
            //no field name selected
            UtlErrorHwnd( ERROR_NO_ENTRYFIELD, MB_CANCEL,
                          0, NULL, EQF_ERROR, hwndDlg );

          } /* endif */
        } /* endif */
      } /* endif */
      break;

    case ID_FILT_AVAILFIELDS_LB:
      //single click or space bar
      if ( sNotification == LN_SELECT )
      {
        pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );

        //get selected item index of selected item in available fields listbox
        sItem = QUERYSELECTION( hwndDlg, ID_FILT_AVAILFIELDS_LB );
        if ( sItem != LIT_NONE )
        {
          //load name of selected item in listbox into szString
          QUERYITEMTEXT( hwndDlg, ID_FILT_AVAILFIELDS_LB,
                         sItem, pIda->szOpFieldName );
        } /* endif */
      }
      else
      {
        pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );

        //double click on a field, add field to mle at cursor position
        if ( sNotification == LN_ENTER )
        {
          //get selected item index of selected item in available fields
          //listbox
          sItem = QUERYSELECTION( hwndDlg,
                                  ID_FILT_AVAILFIELDS_LB );
          if ( sItem != LIT_NONE )
          {
            //load name of selected item in listbox into szString
            QUERYITEMTEXT( hwndDlg, ID_FILT_AVAILFIELDS_LB,
                           sItem, pIda->szFieldName );

            //add fieldname to SELECT section in filter structure

            /*******************************************************/
            /* check if field is already in MLE                    */
            /*******************************************************/
            MLEQUERYTEXTLENGTHHWND( pIda->hwndSelMLE, usLength );
            if ( pIda->F.usSelMLESize < (usLength + 100) )
            {
              fOK = UtlAlloc( (PVOID *)&pIda->F.pucSelMLE,
                              (LONG)pIda->F.usSelMLESize,
                              (LONG)max( usLength + 100, MIN_ALLOC ),
                              ERROR_STORAGE );
              if ( fOK )
              {
                pIda->F.usSelMLESize = max( usLength + 100, MIN_ALLOC );
              } /* endif */
            } /* endif */

            if ( fOK )
            {
              LONG                  lLength;

              pIda->F.usSelMLEUsed = usLength;
              memset( pIda->F.pucSelMLE, 0, pIda->F.usSelMLESize );
              lLength = (LONG)usLength;
              MLEEXPORTHWND( pIda->hwndSelMLE, (PSZ)pIda->F.pucSelMLE, lLength );
              usLength = (USHORT)lLength;
            } /* endif */
            if ( fOK )
            {
              ANSIUPPER( (PSZ)pIda->F.pucSelMLE );
              ANSIUPPER( pIda->szFieldName );
              if ( strstr( (PSZ)pIda->F.pucSelMLE, pIda->szFieldName ) != NULL )
              {
                WinAlarm( HWND_DESKTOP, WA_WARNING );
              }
              else
              {
                // get not-uppercased field name
                QUERYITEMTEXT( hwndDlg, ID_FILT_AVAILFIELDS_LB,
                               sItem, pIda->szFieldName );
                //add a blank after field name
                strcat( pIda->szFieldName, "  " );

                //add to mle at cursor position
                MLEINSERTHWND( pIda->hwndSelMLE, pIda->szFieldName );
                MLEQUERYSELHWND( pIda->hwndSelMLE, usAnchor, usCursor );
                MLESETSELHWND( pIda->hwndSelMLE, usAnchor, usCursor );
              } /* endif */
            } /* endif */


          } /* endif */
        } /* endif */
      } /* endif */
      break;

    case ID_FILT_NAME_LB:
      //if a filter is selected then update description, mle and
      //save to combo box respectively
      if ( sNotification == CBN_SELCHANGE )
      {
        pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );

        /**************************************************************/
        /* Get filter name from combobox                              */
        /**************************************************************/
        sItem = CBQUERYSELECTION( hwndDlg, ID_FILT_NAME_LB );
        if ( sItem != LIT_NONE )
        {
          CBQUERYITEMTEXT( hwndDlg, ID_FILT_NAME_LB, sItem, szTempName );
        }
        else
        {
          szTempName[0] = EOS;         // no name is selected
        } /* endif */
        UtlStripBlanks( szTempName );

        if ( szTempName[0] == EOS )
        {
          SETTEXT( hwndDlg, ID_FILT_DESCR_EF, EMPTY_STRING );
          SETTEXTHWND( pIda->hwndSelMLE, EMPTY_STRING );
          SETTEXTHWND( pIda->hwndWhereMLE, EMPTY_STRING );
          SETTEXT( hwndDlg, ID_FILT_SAVETO_CB, EMPTY_STRING );
          pIda->szName[0] = EOS;
          ENABLECTRL( hwndDlg, ID_FILT_DELETE_PB, FALSE );
        }
        else if ( strcmp( pIda->szName, szTempName ) != 0 )
        {
          ENABLECTRL( hwndDlg, ID_FILT_DELETE_PB, TRUE );

          fChange  = MLECHANGEDHWND( pIda->hwndWhereMLE );
          fChange |= MLECHANGEDHWND( pIda->hwndSelMLE );
          fChange |= SLECHANGED( hwndDlg, ID_FILT_DESCR_EF );

          if ( fChange && pIda->szName[0] )
          {
            usMBCode = UtlErrorHwnd( ERROR_FILTER_DISCARDCHANGES, MB_YESNO,
                                     0, NULL, EQF_QUERY, hwndDlg );
          }
          else
          {
            usMBCode = MBID_YES;
          } /* endif */

          if ( usMBCode == MBID_YES )
          {
            /******************************************************/
            /* build fully qualified path name                    */
            /******************************************************/
            strcpy( pIda->szName, szTempName );
            UtlMakeEQFPath( pIda->szPathBuffer, NULC, PROPERTY_PATH, NULL );
            strcat( pIda->szPathBuffer, BACKSLASH_STR );
            strcat( pIda->szPathBuffer, pIda->szName );
            strcat( pIda->szPathBuffer, EXT_OF_FILTPROP );

            /******************************************************/
            /* read filter into memory                            */
            /******************************************************/
            usRC = FiltRead( pIda->szPathBuffer, &pIda->F );

            if ( usRC == NO_ERROR )
            {
              PUCHAR   pszTemp, pucEndOfBuffer;

              /********************************************************/
              /* Convert all filter strings to ANSI                   */
              /********************************************************/
              OEMTOANSI( pIda->F.Prop.szDescription );
              OEMTOANSI( (PSZ)pIda->F.pucWhereMLE );
              pszTemp = pIda->F.pucBuffer;
              pucEndOfBuffer = pIda->F.pucBuffer + pIda->F.usBufferUsed;
              while ( pszTemp < pucEndOfBuffer )
              {
                OEMTOANSI( (PSZ)pszTemp );
                pszTemp += strlen((PSZ)pszTemp) + 1;
              } /* endwhile */

              //fill description field
              SETTEXT( hwndDlg, ID_FILT_DESCR_EF, pIda->F.Prop.szDescription );

              /**********************************************************/
              /* fill SELECT MLE                                        */
              /**********************************************************/
              SETTEXTHWND( pIda->hwndSelMLE, EMPTY_STRING );
              MLEDELETEHWND( pIda->hwndSelMLE );
              usNoOfNames = pIda->F.usSelNameUsed / sizeof(USHORT);
              if ( usNoOfNames == 0 )    // = use all avalaible fields
              {
                strcpy( pIda->szString, "*" );
                MLEINSERTHWND( pIda->hwndSelMLE, pIda->szString  );
              }
              else
              {
                for ( usI = 0; usI < usNoOfNames; usI++ )
                {
                  strcpy( pIda->szString, (PSZ)(pIda->F.pucBuffer +
                          pIda->F.pusSelNames[usI]) );
                  FiltBlankToUS( pIda->szString );
                  strcat( pIda->szString, " " );

                  MLEINSERTHWND( pIda->hwndSelMLE, pIda->szString );
                } /* endfor */

              } /* endif */
              MLESETSELHWND( pIda->hwndSelMLE, -1, -1 );

              //fill where mle
              SETTEXTHWND( pIda->hwndWhereMLE, (PSZ)pIda->F.pucWhereMLE );
              MLESETSELHWND( pIda->hwndWhereMLE, -1, -1 );


              //fill save to combo box
              SETTEXT( hwndDlg, ID_FILT_SAVETO_CB, pIda->szName );

              /****************************************************/
              /* Reset EF and MLE change flags                    */
              /****************************************************/
              MLESETCHANGEDHWND( pIda->hwndWhereMLE, FALSE );
              MLESETCHANGEDHWND( pIda->hwndSelMLE, FALSE );
              SLESETCHANGED( hwndDlg, ID_FILT_DESCR_EF, FALSE );
            } /* endif */
          }
          else
          {
            SETTEXT( hwndDlg, ID_FILT_NAME_LB, pIda->szName );
          } /* endif */
        } /* endif */

      } /* endif */
      break;

    case ID_FILT_SAVETO_CB:
      if ( sNotification == CBN_EFCHANGE )
      {
        pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );
        QUERYTEXT( hwndDlg, ID_FILT_SAVETO_CB, pIda->szSaveName );
        UtlStripBlanks( pIda->szSaveName );
        ENABLECTRL( hwndDlg, ID_FILT_SAVE_PB, (pIda->szSaveName[0] != EOS) );
      } /* endif */
      break;

  } /* endswitch */
  return( mResult );
} /* end FiltControl */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltCommand                                              |
//+----------------------------------------------------------------------------+
//|Function call:     FiltCommand( hwndDlg, msg, mp1, mp2 )                    |
//+----------------------------------------------------------------------------+
//|Description:       Handles dialog cancel and filter save which checks       |
//|                   the all the entries made by the user, i.e. triggers      |
//|                   syntax checkers for the condition mles.                  |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   msg         - message parameter                          |
//|                   mp1         - first parameter                            |
//|                   mp2         - second parameter                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     query combo-box for filter name                          |
//|                   if no name entered                                       |
//|                     issue warning                                          |
//|                     focus on combo-box                                     |
//|                     leave command loop                                     |
//|                   endif                                                    |
//|                   if changes made to either mle of an existing filter      |
//|                     issue warning                                          |
//|                     if user wants to change name                           |
//|                       focus on combo-box                                   |
//|                       leave command loop                                   |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   allocate memory for select mle                           |
//|                   if allocation successful                                 |
//|                     initialize filter buffer, filter stack, selected       |
//|                       fieldnames stack and all fieldnames stack            |
//|                     do syntax test on select mle                           |
//|                     if fOK                                                 |
//|                     allocate memory for where mle                          |
//|                     if allocation successful                               |
//|                       do syntax test on where mle                          |
//|                   endif                                                    |
//|                   if fOK                                                   |
//|                     write to filter properties file                        |
//|                   dismiss dialog                                           |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+
MRESULT FiltCommand
(
HWND    hwndDlg,         //handle of change/add dialog window
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  PFILTIDA  pIda;                     // ptr to IDA
  MRESULT mResult;                    // result of processing
  BOOL    fOK = TRUE;                 // success indicator
  USHORT  usRC;                       // return code
  USHORT  usLength;                   // mle length
  PUCHAR  pucMLE;                     // pointer for MLE processing
  BOOL    fFilterExists = FALSE;      // TRUE = filter with this name exists
  PSZ     pszErrParm;                 // pointer to UtlError parameter

  sNotification;                      // get rid of compiler warnings
  mResult = MRFROMSHORT(TRUE);

  switch ( sId )
  {
    case ID_FILT_HELP_PB:
      UtlInvokeHelp();
      break;

    case ID_FILT_CANCEL_PB:
    case DID_CANCEL:
      WinPostMsg ( hwndDlg, WM_CLOSE, MP1FROMSHORT( FALSE ), 0l );
      mResult = (MRESULT) FALSE;
      break;

    case ID_FILT_DELETE_PB:
      pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );
      //get name of current filter from combo box
      QUERYTEXT( hwndDlg, ID_FILT_NAME_LB, pIda->szName );
      UtlStripBlanks( pIda->szName );
      if ( pIda->szName[0] != NULC )
      {
        /************************************************************/
        /* get user confirmation for delete                         */
        /************************************************************/
        pszErrParm = pIda->szName;
        if ( UtlErrorHwnd( WARNING_DELETE_FILTER, MB_YESNO,
                           1, &pszErrParm, EQF_QUERY, hwndDlg ) == MBID_YES )
        {
          /******************************************************/
          /* build fully qualified filter name                  */
          /******************************************************/
          UtlMakeEQFPath( pIda->szPathBuffer, NULC, PROPERTY_PATH, NULL );
          strcat( pIda->szPathBuffer, BACKSLASH_STR );
          strcat( pIda->szPathBuffer, pIda->szName );
          strcat( pIda->szPathBuffer, EXT_OF_FILTPROP );

          /**********************************************************/
          /* Delete the filter file, delete filter from filter      */
          /* listboxes, broadcast delete message, refresh MLEs      */
          /* and filter description                                 */
          /**********************************************************/
          if ( UtlDelete( pIda->szPathBuffer, 0L, TRUE ) == NO_ERROR )
          {
            /********************************************************/
            /* broadcast delete notification message                */
            /* and close dialog                                     */
            /********************************************************/
            EqfSend2AllHandlers( WM_EQFN_DELETED,
                                 MP1FROMSHORT( clsFILTER ),
                                 MP2FROMP( pIda->szName ) );
            pIda->szSaveName[0] = EOS;
            WinPostMsg ( hwndDlg, WM_CLOSE, MP1FROMSHORT( TRUE ), 0l );
          } /* endif */
        } /* endif */
      } /* endif */
      break;

    case ID_FILT_SAVE_PB:
      pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );

      //get save_to filter name from combo box
      QUERYTEXT( hwndDlg, ID_FILT_SAVETO_CB, pIda->szSaveName );
      UtlStripBlanks( pIda->szSaveName );
      if ( pIda->szSaveName[0] == NULC )
      {
        //issue warning that no filter for save was selected
        UtlErrorHwnd( ERROR_NO_FILTER_SELECTED, MB_CANCEL,
                      0, NULL, EQF_ERROR, hwndDlg );

        //focus on combo box
        fOK = FALSE;
        SETFOCUS( hwndDlg, ID_FILT_SAVETO_CB );
      } /* endif */

      /************************************************************/
      /* check if filter name is valid                           */
      /************************************************************/
      if ( fOK )
      {
        pszErrParm = pIda->szSaveName;
        while ( *pszErrParm && isalnum(*pszErrParm) )
        {
          pszErrParm++;
        } /* endwhile */
        if ( *pszErrParm != EOS )
        {
          /**********************************************************/
          /* invalid character(s) in filter name                    */
          /**********************************************************/
          fOK = FALSE;
          pszErrParm = pIda->szSaveName;
          UtlErrorHwnd( ERROR_FILTERNAME_INVALID, MB_CANCEL,
                        1, &pszErrParm, EQF_ERROR, hwndDlg );
          SETFOCUS( hwndDlg, ID_FILT_SAVETO_CB );
        } /* endif */
      } /* endif */


      if ( fOK )
      {
        //export SELECT mle into allocated area
        MLEQUERYTEXTLENGTHHWND( pIda->hwndSelMLE, usLength );
        if ( pIda->F.usSelMLESize < (usLength + 100) )
        {

          fOK = UtlAlloc( (PVOID *)&pIda->F.pucSelMLE,
                          (LONG)pIda->F.usSelMLESize,
                          (LONG)max( usLength + 100, MIN_ALLOC ),
                          ERROR_STORAGE );
          if ( fOK )
          {
            pIda->F.usSelMLESize = max( usLength + 100, MIN_ALLOC );
          } /* endif */
        } /* endif */

        if ( fOK )
        {
          ULONG ulLength;

          pIda->F.usSelMLEUsed = usLength;
          memset( pIda->F.pucSelMLE, 0, pIda->F.usSelMLESize );
          ulLength = (ULONG)usLength;
          MLEEXPORTHWND( pIda->hwndSelMLE, (PSZ)pIda->F.pucSelMLE, ulLength );
          usLength = (USHORT)ulLength;
          while ( pIda->F.usSelMLEUsed &&
                  ( pIda->F.pucSelMLE[pIda->F.usSelMLEUsed-1] == BLANK) )
          {
            pIda->F.pucSelMLE[pIda->F.usSelMLEUsed] = EOS;
            pIda->F.usSelMLEUsed--;
          } /* endwhile */
          pIda->F.usSelMLEUsed++;

          //initialize filter memory
          pIda->F.usStackUsed = 0;
          pIda->F.usBufferUsed = 0;
          pIda->F.usSelNameUsed = 0;
          pIda->F.usAllNameUsed = 0;
          pIda->pPosStack = pIda->F.pStack;
          pIda->pusSelPos = pIda->F.pusSelNames;
          pIda->pusAllPos = pIda->F.pusAllNames;

          //do syntax test on select part
          if ( !FiltSelectTest( hwndDlg, pIda ))
            fOK = FALSE;
        } /* endif */

        //export WHERE mle into allocated area
        if ( fOK )
        {
          MLEQUERYTEXTLENGTHHWND( pIda->hwndWhereMLE, usLength );
          if ( pIda->F.usWhereMLESize < (usLength + 100) )
          {

            fOK = UtlAlloc( (PVOID *)&pIda->F.pucWhereMLE,
                            (LONG)pIda->F.usWhereMLESize,
                            (LONG)max( usLength + 100, MIN_ALLOC ),
                            ERROR_STORAGE );
            if ( fOK )
            {
              pIda->F.usWhereMLESize = max( usLength + 100, MIN_ALLOC );
            } /* endif */
          } /* endif */

          if ( fOK )
          {
            ULONG ulLength;

            memset( pIda->F.pucWhereMLE, 0, pIda->F.usWhereMLESize );
            pIda->F.usWhereMLEUsed = usLength + 1;
            ulLength = (ULONG)usLength;
            MLEEXPORTHWND( pIda->hwndWhereMLE, (PSZ)pIda->F.pucWhereMLE, ulLength );
            usLength = (USHORT)ulLength;
            while ( pIda->F.usWhereMLEUsed &&
                    ( pIda->F.pucWhereMLE[pIda->F.usWhereMLEUsed-1] == BLANK) )
            {
              pIda->F.pucWhereMLE[pIda->F.usWhereMLEUsed] = EOS;
              pIda->F.usWhereMLEUsed--;
            } /* endwhile */
            pIda->F.usWhereMLEUsed++;

            //do syntax test on WHERE part
            pucMLE = pIda->F.pucWhereMLE;
            FiltSkipBlanks( &pucMLE );
            if ( *pucMLE )
            {
              fOK = FiltExpression( hwndDlg, pIda, &pucMLE );
            }
            else
            {
              /***************************************************/
              /* Where MLE is empty, issue error message if      */
              /* select MLE is empty also                        */
              /***************************************************/
              if ( !pIda->F.usSelNameUsed )
              {
                UtlErrorHwnd( ERROR_NO_FILTER_STATEMENTS, MB_CANCEL,
                              0, NULL, EQF_ERROR, hwndDlg );
                FiltPosToError( pIda->hwndWhereMLE,
                                pIda->F.pucWhereMLE, pucMLE, pucMLE );
                fOK = FALSE;
              } /* endif */
            } /* endif */
            FiltSkipBlanks( &pucMLE );

            /*****************************************************/
            /* Issue error message if there is still MLE data    */
            /* waiting for processing                            */
            /*****************************************************/
            if ( fOK && *pucMLE )
            {
              UtlErrorHwnd( ERROR_SYNTAX_INCORRECT, MB_CANCEL,
                            0, NULL, EQF_ERROR, hwndDlg );
              FiltPosToError( pIda->hwndWhereMLE,
                              pIda->F.pucWhereMLE, pucMLE, pucMLE );
              fOK = FALSE;
            } /* endif */
          } /* endif */
        } /* endif */

        if ( fOK )
        {
          //if ok add to all to filter props
          //build filter path and add to filter props
          strupr( pIda->szSaveName );
          UtlMakeEQFPath( pIda->szPathBuffer, NULC, PROPERTY_PATH, NULL );
          strcat( pIda->szPathBuffer, BACKSLASH_STR );
          strcat( pIda->szPathBuffer, pIda->szSaveName );
          strcat( pIda->szPathBuffer, EXT_OF_FILTPROP );
          strcpy( pIda->F.Prop.szFilterPath, pIda->szPathBuffer );

          //get description from entry field and add to filter props
          QUERYTEXT( hwndDlg, ID_FILT_DESCR_EF, pIda->F.Prop.szDescription );
          fFilterExists = UtlFileExist( pIda->szPathBuffer );
        } /* endif */

        /***********************************************************/
        /* Get user confirmation if filter is saved under a        */
        /* new name and a filter with the given name exists        */
        /***********************************************************/
        if ( fOK )
        {
          if ( fFilterExists &&
               (stricmp( pIda->szName, pIda->szSaveName ) != 0)  )
          {
            pszErrParm = pIda->szSaveName;
            usRC = UtlErrorHwnd( WARNING_FILTER_EXISTS, MB_YESNO,
                                 1, &pszErrParm, EQF_QUERY, hwndDlg );
            fOK = ( usRC != MBID_NO );
            if ( !fOK )
            {
              SETFOCUS( hwndDlg, ID_FILT_SAVETO_CB );
            } /* endif */
          } /* endif */
        } /* endif */

        /***********************************************************/
        /* Write filter property file                              */
        /***********************************************************/
        if ( fOK )
        {
          PUCHAR   pszTemp, pucEndOfBuffer;

          /********************************************************/
          /* Convert all filter strings to ANSI                   */
          /********************************************************/
          ANSITOOEM( pIda->F.Prop.szDescription );
          ANSITOOEM( (PSZ)pIda->F.pucWhereMLE );

          pszTemp = pIda->F.pucBuffer;
          pucEndOfBuffer = pIda->F.pucBuffer + pIda->F.usBufferUsed;
          while ( pszTemp < pucEndOfBuffer )
          {
            ANSITOOEM( (PSZ)pszTemp );
            pszTemp += strlen((PSZ)pszTemp) + 1;
          } /* endwhile */

          usRC = FiltWrite( pIda->szPathBuffer, &pIda->F );
          fOK = (usRC == NO_ERROR);
        } /* endif */

        /***********************************************************/
        /* Inform all handlers about new or changed filter         */
        /***********************************************************/
        if ( fOK )
        {
          if ( fFilterExists )
          {
            EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                 MP1FROMSHORT( clsFILTER ),
                                 MP2FROMP( pIda->szSaveName ) );
          }
          else
          {
            EqfSend2AllHandlers( WM_EQFN_CREATED,
                                 MP1FROMSHORT( clsFILTER ),
                                 MP2FROMP( pIda->szSaveName ) );
          } /* endif */
        } /* endif */

        //dismiss dialog
        if ( fOK )
        {
          WinPostMsg ( hwndDlg, WM_CLOSE, MP1FROMSHORT( TRUE ), 0l );
        } /* endif */

      } /* endif */
      break;

    case ID_FILT_OPERATORS_LB:
    case ID_FILT_AVAILFIELDS_LB:
    case ID_FILT_NAME_LB:
    case ID_FILT_SAVETO_CB:
      mResult = FiltControl( hwndDlg, sId, sNotification );
      break;
    case ID_FILT_DESCR_EF:
    case ID_FILT_SELECT_MLE:
      if ( sNotification == EN_KILLFOCUS )
      {
        ClearIME( hwndDlg );
      } /* endif */
      break;
  } /* endswitch */
  return( mResult );
} /* end FiltCommand */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltSelectTest                                           |
//+----------------------------------------------------------------------------+
//|Function call:     FiltSelectTest( hwndDlg, pIda )                          |
//+----------------------------------------------------------------------------+
//|Description:       collects the entry field names that appear in the        |
//|                   SELECT condition mle.                                    |
//+----------------------------------------------------------------------------+
//|Parameters:        pIda        - pointer to global structure                |
//|                   hwndDlg     - dialog handle                              |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOLEAN                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE OR FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     while fOK and SELECT mle not empty                       |
//|                   FiltReadToken                                            |
//|                     if TempBuffer is a valid field name then               |
//|                       fOK = TRUE                                           |
//|                     else                                                   |
//|                       break off                                            |
//|                   endwhile                                                 |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltSelectTest( HWND hwndDlg, PFILTIDA pIda )
{
  BOOL       fOK = TRUE;              //success indicator
  PUCHAR     pucStartPos;             // start position of current token
  PUCHAR     pucMLE;                  // position within SELECT MLE

  pucMLE = pIda->F.pucSelMLE;

  /*******************************************************************/
  /* Check for all fields identifier                                */
  /*******************************************************************/
  while ( *pucMLE == BLANK )
  {
    pucMLE++;
  } /* endwhile */
  if ( (*pucMLE == '*') && (*(pucMLE+1) == EOS) )
  {
    pIda->F.usSelMLEUsed = 1;
    pIda->F.pucSelMLE[0] = EOS;
  }
  else
  {
    pucMLE = (PUCHAR)pIda->F.pucSelMLE;
    FiltSkipBlanks( &pucMLE );

    while ( fOK && *pucMLE )
    {
      //read successive entry field names
      pucStartPos = pucMLE;

      FiltReadToken( pIda, &pucMLE );

      //test if szTemp is a valid dictionary field name
      fOK = FiltIdentifier( hwndDlg, pIda, FALSE );

      if ( !fOK )
      {
        FiltPosToError( pIda->hwndSelMLE, pIda->F.pucSelMLE,
                        pucStartPos, pucStartPos + strlen(pIda->szTemp) - 1 );
      } /* endif */
    } /* endwhile */
  } /* endif */


  return( fOK );
} /* end FiltSelectTest */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltExpression                                           |
//+----------------------------------------------------------------------------+
//|Function call:     FiltExpression ( hwndDlg, pIda, ppucMLE )                |
//+----------------------------------------------------------------------------+
//|Description:       handles the Where filter condition at the upper most     |
//|                   level. That is, it deals with the operators AND and OR   |
//|                   combind with  so-called tuple (operand operator operand) |
//|                   occurrences (embeddings and/or NOT constructs).          |
//|                   E.g. Author = 'John' AND (subject_code = 'IBM' OR        |
//|                        Project_Nr = 4) AND                                 |
//|                        (NOT Creation_date BETWEEN 88/01/20 AND 90/01/30 OR |
//|                         Language IN ('English', 'French'))                 |
//|                   The functions store the information in reverse Polish    |
//|                   notation, i.e. first the operands then the operators.    |
//|                                                                            |
//|  FiltExpression                                                            |
//|                                                                            |
//|                        +--------------------+                              |
//|            --------Â--+FiltSimpleExpression+--------Â----Â----          |
//|                       +--------------------+                            |
//|                     |                                 AND  OR              |
//|                     |                                 |    |               |
//|                     +---------------------------------Á----+               |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg - dialog handle                                  |
//|                   pIda    - pointer to global structure                    |
//|                   ppucMLE - pointer to current position in mle             |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOLEAN                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE OR FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     while fOK and mle not empty                              |
//|                     FiltReadToken                                          |
//|                     switch TempBuffer                                      |
//|                       case AND:                                            |
//|                       case OR:                                             |
//|                         store operator temporarily                         |
//|                       default:                                             |
//|                         if FiltSimpleExpression then                       |
//|                           if saved operator                                |
//|                             save operator identifier to filter stack       |
//|                         else                                               |
//|                           fOK = FALSE                                      |
//|                     endswitch                                              |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
static
BOOL FiltExpression
(
HWND         hwndDlg,                //dialog handle
PFILTIDA     pIda,                   //pointer to Ida
PUCHAR       *ppucMLE                //pointer to mle
)
{
  BOOL     fOK = TRUE;                //success indicator
  PUCHAR   pucStartPos;               //pointer to token start position
  PUCHAR   pucWhereMLE = *ppucMLE;    //pointer to mle
  SHORT    sOperatorID = OP_AND;      //operator string id

  FiltSkipBlanks( &pucWhereMLE );

  fOK = FiltSimpleExpression( hwndDlg, pIda, &pucWhereMLE );

  while ( fOK && *pucWhereMLE && (sOperatorID != OP_END) )
  {
    // get next token from input
    pucStartPos = pucWhereMLE;
    FiltReadToken( pIda, &pucWhereMLE );

    if ( (stricmp( pIda->szTemp, AND_STR ) == 0) ||
         (stricmp( pIda->szTemp, OR_STR ) == 0) )
    {
      sOperatorID = FiltGetOperatorID( pIda->szTemp );

      fOK = FiltSimpleExpression( hwndDlg, pIda, &pucWhereMLE );

      if ( fOK )
      {
        fOK = FiltPushOnStack( pIda, sOperatorID, 0 );
      } /* endif */
    }
    else
    {
      /**************************************************************/
      /* not valid token for an expression, restore old position    */
      /**************************************************************/
      pucWhereMLE = pucStartPos;
      sOperatorID = OP_END;
    } /* endif */
  } /* endwhile */

  *ppucMLE = pucWhereMLE;
  return( fOK );
} /* end FiltExpression */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltSimpleExpression                                     |
//+----------------------------------------------------------------------------+
//|Function call:     FiltSimpleExpression( hwndDlg, pIda, ppucMLE )           |
//+----------------------------------------------------------------------------+
//|Description:       FiltSimpleExpression handles the so-called tuple (operand|
//|                   operator operand) constructs that can occur in three     |
//|                   forms, namely between brackets, preceeded by NOT (with   |
//|                   and without brackets) or nacked.                         |
//|                                                                            |
//| FiltSimpleExpression                                                       |
//|                                                                            |
//|                       +----------------+                                   |
//|       +----- ( ------+ FiltExpression +---------- ) ------------+        |
//|       |               +----------------+                          |        |
//|       |               +----------------------+                            |
//|   ---Å ---- NOT ----+ FiltSimpleExpression +-------------------Å---    |
//|       |               +----------------------+                    |        |
//|       |       +--------------+                                    |        |
//|       +------+FiltIdentifier+--------+                                  |
//|               +--------------+                                   |        |
//|                                        |                          |        |
//|        +---Â---Â---Â---Â--Â---Â----Â---Á---+                      |        |
//|                                                          |        |
//|        =   <   >  =<  =>  <>  IN  BETWEEN  LIKE                   |        |
//|                                             +--------+   |        |
//|        +---Á---Á---Á---Á--Á---Á----Á-------Á--------+FiltTerm+---+        |
//|                                                      +--------+            |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   pIda        - pointer to global structure                |
//|                   ppucMLE     - pointer to current position in WHERE mle   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOLEAN                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     if TempBuffer not empty                                  |
//|                     switch TempBuffer                                      |
//|                       case NOT:                                            |
//|                         set NOT boolean flag to TRUE                       |
//|                         if not FiltSimpleExpression then                   |
//|                           break off - error msg issued elsewhere           |
//|                         else if NOT boolean flag on                        |
//|                           save NOT identifier to filter stack              |
//|                         endif                                              |
//|                       default:                                             |
//|                         if FiltIdentifier then                             |
//|                           remember ptr pos for possible error msg to mle   |
//|                           switch pointer to input string                   |
//|                              case '=':                                     |
//|                                save operator temporarily                   |
//|                                increment pointer to mle                    |
//|                                FiltSkipBlanks                              |
//|                              case '<':                                     |
//|                              case '>':                                     |
//|                                save operator temporarily                   |
//|                                increment pointer to mle                    |
//|                                switch pointer to mle                       |
//|                                  case '=':                                 |
//|                                    concat with saved operator              |
//|                                    get operator id of saved operator string|
//|                                    increment pointer to mle                |
//|                                    FiltSkipBlanks                          |
//|                                  case '>':                                 |
//|                                    concat with saved operator              |
//|                                    get operator id of saved operator string|
//|                                    if operator string not valid            |
//|                                      issue warning                         |
//|                                      set fOK to false                      |
//|                                    else                                    |
//|                                      increment pointer to mle              |
//|                                    endif                                   |
//|                                    FiltSkipBlanks                          |
//|                                  default                                   |
//|                                    FiltSkipBlanks                          |
//|                                endswitch                                   |
//|                              default:                                      |
//|                                FiltReadToken                               |
//|                                switch TempBuffer                           |
//|                                  case LIKE:                                |
//|                                  case IN:                                  |
//|                                    save operator temporarily               |
//|                                  case BETWEEN:                             |
//|                                    save operator temporarily               |
//|                                    call function FiltBetween               |
//|                                  default:                                  |
//|                                    error message - operator missing or     |
//|                                                    invalid                 |
//|                                    set fOK to false                        |
//|                                endswitch                                   |
//|                           endswitch                                        |
//|                         endif                                              |
//|                         if fOK                                             |
//|                           if function FiltTerm then                        |
//|                             save operator identifier to filter stack       |
//|                             put operator on filter stack                   |
//|                           else                                             |
//|                             break off - error message issued elsewhere     |
//|                         else                                               |
//|                           break off - error message issued elsewhere       |
//|                     endswitch                                              |
//|                   else TempBuffer is empty                                 |
//|                     if pointer to input string from mle = '(' then         |
//|                       increment input pointer                              |
//|                       FiltSkipBlanks                                       |
//|                       while not ')' and not EOS mle                        |
//|                         if not FiltExpression then                         |
//|                           break off - error msg issued elsewhere           |
//|                       endwhile                                             |
//|                       if EOS mle then                                      |
//|                         error message - closing bracket missing            |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltSimpleExpression( HWND hwndDlg,         //dialog handle
                           PFILTIDA pIda,        //pointer to ida
                           PUCHAR *ppucMLE )     //pointer to mle
{
  BOOL     fOK = TRUE;                          //success indicator
  CHAR     szOperator[OPERATOR_LEN] ;           //operator string
  SHORT    sOperatorID = 0;                     //operator string id
  PUCHAR   pucWhereMLE = *ppucMLE;              //pointer to mle
  PUCHAR   pucStartPos;               //pointer to token start position

  FiltSkipBlanks( &pucWhereMLE );

  if ( *pucWhereMLE == OPEN_BRACKET )
  {
    /*****************************************************************/
    /* Process bracketed expression                                  */
    /*****************************************************************/

    //increment pointer
    pucWhereMLE++;

    FiltSkipBlanks( &pucWhereMLE );

    fOK = FiltExpression( hwndDlg, pIda, &pucWhereMLE );

    FiltSkipBlanks( &pucWhereMLE );

    if ( *pucWhereMLE != CLOSED_BRACKET )
    {
      //error message because of missing closing bracket
      UtlErrorHwnd( ERROR_BRACKET_MISSING, MB_CANCEL,
                    0, NULL, EQF_ERROR, hwndDlg );

      //focus on mle at error position
      FiltPosToError( pIda->hwndWhereMLE, pIda->F.pucWhereMLE,
                      pucWhereMLE, pucWhereMLE );

      fOK = FALSE;
    }
    else
    {
      /***************************************************************/
      /* Skip closing bracket                                        */
      /***************************************************************/
      pucWhereMLE++;

      /***************************************************************/
      /* Skip blanks                                                 */
      /***************************************************************/
      FiltSkipBlanks( &pucWhereMLE );
    } /* endif */
  }
  else
  {
    // get next token from input
    pucStartPos = pucWhereMLE;
    FiltReadToken( pIda, &pucWhereMLE );

    /*****************************************************************/
    /* Check for NOT operator                                        */
    /*****************************************************************/
    if ( (stricmp( pIda->szTemp, NOT_STR ) == 0) )
    {
      /***************************************************************/
      /* Process following expression                                */
      /***************************************************************/
      fOK = FiltSimpleExpression( hwndDlg, pIda, &pucWhereMLE );

      /***************************************************************/
      /* Push NOT operator on stack                                  */
      /***************************************************************/
      if ( fOK )
      {
        fOK = FiltPushOnStack( pIda, OP_NOT, 0 );
      } /* endif */
    }
    else
    {
      /***************************************************************/
      /* Process simple expression                                   */
      /***************************************************************/

      /***************************************************************/
      /* Process identifier (= field name) which is alread in        */
      /* pIda->szTemp                                                */
      /***************************************************************/
      fOK = FiltIdentifier( hwndDlg, pIda, TRUE );
      if ( !fOK )
      {
        FiltPosToError( pIda->hwndWhereMLE, pIda->F.pucWhereMLE,
                        pucStartPos, pucWhereMLE );
      } /* endif */

      /***************************************************************/
      /* Process operator                                            */
      /***************************************************************/
      if ( fOK )
      {
        pucStartPos = pucWhereMLE;

        switch ( *pucWhereMLE )
        {
          case EQUAL:
            //save operator temporarily
            szOperator[0] = *pucWhereMLE;

            //save operator id
            sOperatorID = OP_EQUAL;

            //increment mle ptr
            pucWhereMLE++;

            FiltSkipBlanks( &pucWhereMLE );
            break;
          case SMALLER_THAN :
          case BIGGER_THAN :
            //save operator temporarily
            szOperator[0] = *pucWhereMLE;

            //increment mle ptr and check if another operator follows
            pucWhereMLE++;
            switch ( *pucWhereMLE )
            {
              case EQUAL:
                //save operator temporarily
                szOperator[1] = *pucWhereMLE;
                szOperator[2] = EOS;

                //get operator id
                sOperatorID = FiltGetOperatorID( szOperator );

                //increment mle ptr
                pucWhereMLE++;

                FiltSkipBlanks( &pucWhereMLE );
                break;
              case BIGGER_THAN:
                //save operator temporarily
                szOperator[1] = *pucWhereMLE;
                szOperator[2] = EOS;

                //get operator id
                sOperatorID = FiltGetOperatorID( szOperator );

                //if returned sOperatorID = -1 then operator error
                //operator should be <> and not >< for instance
                if ( sOperatorID == -1 )
                {
                  //issue warning

                  fOK = FALSE;

                  //focus on mle
                }
                else
                {
                  //increment mle ptr
                  pucWhereMLE++;
                } /* endif */

                if ( fOK )
                  FiltSkipBlanks( &pucWhereMLE );
                break;
              default:
                szOperator[1] = EOS;
                //get operator id
                sOperatorID = FiltGetOperatorID( szOperator );

                //position on next non blank character in mle
                FiltSkipBlanks( &pucWhereMLE );
                break;
            } /* endswitch */
            break;
          default:
            //read input from WHERE mle
            FiltReadToken( pIda, &pucWhereMLE );

            if ( (stricmp( pIda->szTemp, LIKE_STR ) == 0 ) ||
                 (stricmp( pIda->szTemp, IN_STR ) == 0  ) )
            {
              strcpy( szOperator, pIda->szTemp );

              //get operator id
              sOperatorID = FiltGetOperatorID( szOperator );
            }
            else if ( (strcmp( pIda->szTemp, BETWEEN_STR ) == 0) )
            {
              strcpy( szOperator, pIda->szTemp );

              //save operator id
              sOperatorID = OP_BETWEEN;

              fOK = FiltBetween( hwndDlg, pIda, &pucWhereMLE );
            }
            else
            {
              //invalid operator - issue warning
              UtlErrorHwnd( ERROR_INVALID_OP, MB_CANCEL,
                            0, NULL, EQF_ERROR, hwndDlg );

              //focus on mle at error position
              FiltPosToError( pIda->hwndWhereMLE, pIda->F.pucWhereMLE,
                              pucStartPos, pucWhereMLE );

              fOK = FALSE;
            } /* endif */
            break;
        } /* endswitch */
      } /* endif */

      //the between case has already been handled separately in the
      //function FiltBetween
      if ( fOK && sOperatorID != OP_BETWEEN )
        fOK = FiltTerm( hwndDlg, pIda, &pucWhereMLE );

      if ( fOK )
      {
        //save operator on stack
        fOK = FiltPushOnStack( pIda, sOperatorID, 0 );
      } /* endif */
    } /* endif */
  } /* endif */

  *ppucMLE = pucWhereMLE;

  return( fOK );
} /* FiltSimpleExpression */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltBetween                                              |
//+----------------------------------------------------------------------------+
//|Function call:     FiltBetween( hwndDlg, pIda, ppucMLE )                    |
//+----------------------------------------------------------------------------+
//|Description:       FiltBetween checks the syntax for the operator BETWEEN   |
//|                   which indicates a range whereby the operator AND acts as |
//|                   the separator between starting range and ending range,   |
//|                   e.g. BETWEEN 15/01/91 AND 30/03/91.                      |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   pIda        - pointer to global structure                |
//|                   ppucMLE     - pointer to current position in mle         |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOLEAN                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     if FiltConstant then                                     |
//|                     FiltReadToken                                          |
//|                     if TempBuffer = 'AND' then                             |
//|                       if FiltConstant then                                 |
//|                         fOk is TRUE                                        |
//|                       else                                                 |
//|                         break off - error message issued elsewhere         |
//|                     else                                                   |
//|                       error message - incorrect operator or missing        |
//|                       break off                                            |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltBetween( HWND   hwndDlg,       //dialog handle
                  PFILTIDA pIda,        //pointer to ida
                  PUCHAR *ppucMLE )     //pointer to mle
{
  BOOL     fOK = TRUE;                          //success indicator
  PUCHAR   pucWhereMLE = *ppucMLE;              //pointer to mle

  fOK = FiltConstant( hwndDlg, pIda, &pucWhereMLE );

  if ( fOK )
  {
    //read input from WHERE mle
    FiltReadToken( pIda, &pucWhereMLE );

    if ( (stricmp( pIda->szTemp, AND_STR ) == 0) )
    {
      fOK = FiltConstant( hwndDlg, pIda, &pucWhereMLE );
    }
    else
    {
      //incorrect operator or operator missing
      UtlErrorHwnd( ERROR_BETWEEN_OPERATOR, MB_CANCEL,
                    0, NULL, EQF_ERROR, hwndDlg );

      //focus on mle at error position stored in ida
      FiltPosToError( pIda->hwndWhereMLE, pIda->F.pucWhereMLE,
                      pucWhereMLE, pucWhereMLE );

      fOK = FALSE;
    } /* endif */
  } /* endif */

  *ppucMLE = pucWhereMLE;

  return( fOK );
} /* end FiltBetween */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltGetOperatorID                                        |
//+----------------------------------------------------------------------------+
//|Function call:     FiltGetOperatorID( pszOperatorName )                     |
//+----------------------------------------------------------------------------+
//|Description:       This function returns the operator id belonging to the   |
//|                   operator string inputted to the function.                |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ           -  pointer to operator string              |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       the operator id or -1 if an id cannot be found           |
//+----------------------------------------------------------------------------+
//|Function flow:     while enum table containing operator strings plus ids    |
//|                   not empty                                                |
//|                      get id belonging to operator string                   |
//|                   endwhile                                                 |
//|                   if not id found                                          |
//|                     set id to -1                                           |
//|                   return operator id                                       |
//+----------------------------------------------------------------------------+

static
SHORT FiltGetOperatorID( PSZ pszOperatorName )
{
  USHORT     usI = 0;                 //table counter
  BOOL       fOK = FALSE;             //success indicator
  SHORT      sOperatorID = 0;         //filter operator enum id

  while ( FiltOperatorTable[usI].sOperator != OP_END && !fOK )
  {
    if ( stricmp( FiltOperatorTable[usI].szName, pszOperatorName ) == 0 )
    {
      //operator found, remember id and leave while loop
      sOperatorID =(SHORT) FiltOperatorTable[usI].sOperator;
      fOK = TRUE;
    }
    else
      usI++;
  } /* endwhile */

  //if operator cannot be found return -1 to indicate operator error
  if ( !fOK )
  {
    sOperatorID = -1;
  } /* endif */

  return( sOperatorID );
} /* FiltGetOperatorID */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltPushOnBuffer                                         |
//+----------------------------------------------------------------------------+
//|Function call:     FiltPushOnBuffer( pIda )                                 |
//+----------------------------------------------------------------------------+
//|Description:       This function stores the data string associated with the |
//|                   filter, e.g. the dictionary field names and values       |
//+----------------------------------------------------------------------------+
//|Parameters:        PFILTIDA     - pointer to filter global structure        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     if necessary allocate memory for buffer and remember     |
//|                   size of buffer allocated                                 |
//|                   if allocation successful ( fOK is TRUE )                 |
//|                     put string on buffer                                   |
//|                     remember how much of the buffer has been used          |
//|                   endif                                                    |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltPushOnBuffer( PFILTIDA pIda )
{
  BOOL fOK = TRUE;                    //success indicator

  if ((strlen(pIda->szTemp) + 1 + pIda->F.usBufferUsed) > pIda->F.usBufferSize)
  {
    //allocate more space
    fOK = ( UtlAlloc( (PVOID *)&pIda->F.pucBuffer,
                      (LONG)pIda->F.usBufferSize,
                      (LONG)pIda->F.usBufferSize + BUFFER_SIZE,
                      ERROR_STORAGE ));
    pIda->F.usBufferSize += BUFFER_SIZE;
  } /* endif */

  if ( fOK )
  {
    strcpy( (PSZ)(pIda->F.pucBuffer + pIda->F.usBufferUsed), pIda->szTemp );
    pIda->F.usBufferUsed = pIda->F.usBufferUsed + (USHORT) (strlen( pIda->szTemp)+1);
  } /* endif */

  return( fOK );
} /* end FiltPushOnBuffer */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltPushOnStack                                          |
//+----------------------------------------------------------------------------+
//|Function call:     FiltPushOnStack( pIda, sOperatorID, usOffset )           |
//+----------------------------------------------------------------------------+
//|Description:       This function pushes the operator or string id plus      |
//|                   offset where data string on buffer begins onto the       |
//|                   filter stack                                             |
//+----------------------------------------------------------------------------+
//|Parameters:        PFILTIDA     - pointer to global structure               |
//|                   SHORT        - operator id                               |
//|                   USHORT       - offset                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     if necessary allocate memory for filter stack and        |
//|                   remember size of stack allocated                         |
//|                   if allocation successful ( fOK is TRUE )                 |
//|                     put operator id on stack                               |
//|                     put offset of where data begins in buffer on stack     |
//|                   endif                                                    |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltPushOnStack( PFILTIDA pIda, SHORT sOperatorId, USHORT usOffset )
{
  BOOL fOK = TRUE;                    //success indicator
  ULONG ulCurOffset;                 //current offset into stack

  //if not enough space allocate more and position stack pointer
  if ( (pIda->F.usStackUsed + sizeof(POLISHSTACK)) > pIda->F.usStackSize )
  {
    ulCurOffset = pIda->pPosStack - pIda->F.pStack;

    //allocate more space
    fOK = ( UtlAlloc( (PVOID *)&pIda->F.pStack,
                      (LONG)pIda->F.usStackSize,
                      (LONG)pIda->F.usStackSize + BUFFER_SIZE,
                      ERROR_STORAGE ));
    pIda->F.usStackSize += BUFFER_SIZE;

    pIda->pPosStack = pIda->F.pStack + ulCurOffset;
  } /* endif */

  if ( fOK )
  {
    pIda->F.usStackUsed += sizeof(POLISHSTACK);
    pIda->pPosStack->sOperatorId = sOperatorId;
    pIda->pPosStack->usOffset = usOffset;
    pIda->pPosStack++;
  } /* endif */

  return( fOK );
} /* end FiltPushOnStack */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltPushOnSelNames                                       |
//+----------------------------------------------------------------------------+
//|Function call:     FiltPushOnSelNames( pIda, usOffset )                     |
//+----------------------------------------------------------------------------+
//|Description:       This function pushes the offset of the field names stored|
//|                   on the buffer onto the selected names stack              |
//+----------------------------------------------------------------------------+
//|Parameters:        PFILTIDA     - pointer to global structure               |
//|                   USHORT       - offset                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     if necessary allocate memory for SelNames stack and      |
//|                   remember size of stack allocated                         |
//|                   if allocation successful ( fOK is TRUE )                 |
//|                     put offset of where data begins in buffer on stack     |
//|                   endif                                                    |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltPushOnSelNames( PFILTIDA pIda, USHORT usOffset )
{
  BOOL fOK = TRUE;                    //success indicator
  ULONG ulCurOffset;                 //current offset into stack

  //if not enough space allocate more and position stack pointer
  if ( (pIda->F.usSelNameUsed + sizeof(USHORT)) > pIda->F.usSelNameSize )
  {
    ulCurOffset = pIda->pusSelPos - pIda->F.pusSelNames;

    //allocate more space
    fOK = ( UtlAlloc( (PVOID *)&pIda->F.pusSelNames,
                      (LONG)pIda->F.usSelNameSize,
                      (LONG)pIda->F.usSelNameSize + BUFFER_SIZE,
                      ERROR_STORAGE ));
    pIda->F.usSelNameSize += BUFFER_SIZE;

    pIda->pusSelPos = pIda->F.pusSelNames + ulCurOffset;
  } /* endif */

  if ( fOK )
  {
    pIda->F.usSelNameUsed += sizeof(USHORT);
    *pIda->pusSelPos = usOffset;
    pIda->pusSelPos++;
  } /* endif */

  return( fOK );
} /* end FiltPushOnSelNames */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltPushOnAllNames                                       |
//+----------------------------------------------------------------------------+
//|Function call:     FiltPushOnAllNames( pIda, usOffset )                     |
//+----------------------------------------------------------------------------+
//|Description:       This function pushes the offset of the field names stored|
//|                   on the buffer onto the all names stack where field names |
//|                   from both SELECT and WHERE are stored                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PFILTIDA     - pointer to global structure               |
//|                   USHORT       - offset                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     if necessary allocate memory for AllNames stack and      |
//|                   remember size of stack allocated                         |
//|                   if allocation successful ( fOK is TRUE )                 |
//|                     put offset of where data begins in buffer on stack     |
//|                   endif                                                    |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltPushOnAllNames( PFILTIDA pIda, USHORT usOffset )
{
  BOOL fOK = TRUE;                    //success indicator
  ULONG ulCurOffset;                 //current offset into stack

  //if not enough space allocate more and position stack pointer
  if ( (pIda->F.usAllNameUsed + sizeof(USHORT)) > pIda->F.usAllNameSize )
  {
    ulCurOffset = pIda->pusAllPos - pIda->F.pusAllNames;

    //allocate more space
    fOK = ( UtlAlloc( (PVOID *)&pIda->F.pusAllNames,
                      (LONG)pIda->F.usAllNameSize,
                      (LONG)pIda->F.usAllNameSize + BUFFER_SIZE,
                      ERROR_STORAGE ));
    pIda->F.usAllNameSize += BUFFER_SIZE;

    pIda->pusAllPos = pIda->F.pusAllNames + ulCurOffset;
  } /* endif */

  if ( fOK )
  {
    pIda->F.usAllNameUsed += sizeof(USHORT);
    *pIda->pusAllPos = usOffset;
    pIda->pusAllPos++;
  } /* endif */

  return( fOK );
} /* end FiltPushOnAllNames */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltReadToken                                            |
//+----------------------------------------------------------------------------+
//|Function call:     FiltReadToken( pIda, ppucMLE )                           |
//+----------------------------------------------------------------------------+
//|Description:       FiltReadToken reads characters from the mle input string |
//|                   and puts them on TempBuffer in pIda. FiltReadToken stops |
//|                   at the first occurrence of one of the following:         |
//|                     ''' or ' ' or '(' or '=' or '<' or '>'                 |
//+----------------------------------------------------------------------------+
//|Parameters:        pIda        - pointer to global structure                |
//|                   ppucMLE     - pointer to current position in mle         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     remember pointer pos for possible error msg to mle       |
//|                   while not ''' or ' ' or '(' or '=' or '<' or '>'         |
//|                     increment mle input pointer                            |
//|                     put on TempBuffer                                      |
//|                   endwhile                                                 |
//|                   FiltSkipBlanks                                           |
//+----------------------------------------------------------------------------+

static
BOOL FiltReadToken( PFILTIDA pIda, PUCHAR *ppucMLE )
{
  BOOL     fOK = TRUE;                    //success indicator
  UCHAR    uChar;                         //character variable
  USHORT   usI = 0;                       //counter
  PUCHAR   pucMLE = *ppucMLE;                   //pointer to mle
  USHORT   usFreeSpace;

  usFreeSpace = sizeof(pIda->szTemp) - 1;
  if ( (*pucMLE == OPEN_BRACKET) || (*pucMLE == CLOSED_BRACKET) )
  {
    pIda->szTemp[usI++] = *pucMLE++;
    usFreeSpace--;
  }
  else
  {
    while ( fOK && usFreeSpace && (( uChar = *pucMLE )!= NULC) )
    {
      switch ( uChar )
      {
//         case QUOTE:
        case BLANK:
        case CR:
        case LF:
        case OPEN_BRACKET:
        case CLOSED_BRACKET:
        case EQUAL:
        case SMALLER_THAN:
        case BIGGER_THAN:
          fOK = FALSE;                //stop criterion
          break;

        default :
          pIda->szTemp[usI++] = uChar;
          usFreeSpace--;
          pucMLE++;
          break;
      } /* end switch */
    } /* endwhile */
  } /* endif */

  *ppucMLE = pucMLE;

  //add end of string indicator
  pIda->szTemp[usI] = EOS;

  //skip blanks
  FiltSkipBlanks( ppucMLE );

  return( fOK );
} /* end FiltReadToken */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltTerm                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     FiltTerm( hwndDlg, pIda, ppucMLE )                       |
//+----------------------------------------------------------------------------+
//|Description:       FiltTerm handles the last operand in a so-called tuple.  |
//|                   E.g.the value 'John' in author = 'John' or ('IBM','AEG') |
//|                   in subj_code IN ('IBM','AEG').                           |
//|                                                                            |
//|  FiltTerm                                                                  |
//|                                                                            |
//|                  +------------+                                            |
//|    Â-----------+FiltConstant+---------------------Â---                 |
//|                 +------------+                                           |
//|     |                     +------------+             |                     |
//|     +----------- ( --Â--+FiltConstant+--Â-- ) ---+                     |
//|                          +------------+   |                               |
//|                       +------- , ----------+                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   pIda        - pointer to global structure                |
//|                   ppucMLE     - pointer to current position in mle         |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOLEAN                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch pointer to input string                           |
//|                     case '(':                                              |
//|                       increment input pointer                              |
//|                       FiltSkipBlanks                                       |
//|                       remember pointer pos for possible error msg to mle   |
//|                       while not ')' and not EOS mle                        |
//|                          if function FiltConstant then                     |
//|                            skip comma                                      |
//|                            FiltSkipBlanks                                  |
//|                          else break off - msg issued elsewhere             |
//|                       endwhile                                             |
//|                       if EOS mle then                                      |
//|                         error message - closing bracket missing            |
//|                     default:                                               |
//|                       if not function FiltConstant                         |
//|                         break off - error msg issued elsewhere             |
//|                   endswitch                                                |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltTerm
(
HWND         hwndDlg,                         //dialog handle
PFILTIDA     pIda,                            //filter ida
PUCHAR       *ppucMLE                         //pointer to mle
)
{
  BOOL     fOK = TRUE;                          //success indicator
  UCHAR    uChar;                               //character variable
  PUCHAR   pucWhereMLE = *ppucMLE;              //pointer to mle


  if ( (uChar = *pucWhereMLE) != NULC )
  {
    //remember current pos in mle for possible error positioning
    //use pIda->pucErrorPos

    switch ( uChar )
    {
      case OPEN_BRACKET:
        //increment mle pointer
        pucWhereMLE++;

        //skip blanks
        FiltSkipBlanks( &pucWhereMLE );

        /***********************************************************/
        /* push a start list operator on stack                     */
        /***********************************************************/
        fOK = FiltPushOnStack( pIda, OP_STARTOFLIST, 0 );

        while ( fOK && *pucWhereMLE && *pucWhereMLE != CLOSED_BRACKET )
        {
          //handle the actual data strings
          fOK = FiltConstant( hwndDlg, pIda, &pucWhereMLE );
          if ( fOK )
          {
            /*****************************************************/
            /* Skip seperator (COMMA)                            */
            /*****************************************************/
            if ( *pucWhereMLE && (*pucWhereMLE == COMMA) )
              pucWhereMLE++;

            //skip blanks
            FiltSkipBlanks( &pucWhereMLE );
          } /* endif */
        } /* endwhile */

        if ( fOK )
        {
          if ( !*pucWhereMLE )
          {
            //error message because of missing closing bracket
            UtlErrorHwnd( ERROR_BRACKET_MISSING, MB_CANCEL,
                          0, NULL, EQF_ERROR, hwndDlg );

            //focus on mle at error position stored in ida
            FiltPosToError( pIda->hwndWhereMLE, pIda->F.pucWhereMLE,
                            pucWhereMLE, pucWhereMLE );

            fOK = FALSE;
          }
          else
          {
            /******************************************************/
            /* skip closing brakcet and following white space     */
            /******************************************************/
            pucWhereMLE++;
            FiltSkipBlanks( &pucWhereMLE );
          } /* endif */
        } /* endif */
        break;
      default :
        fOK = FiltConstant( hwndDlg, pIda, &pucWhereMLE );
        break;
    } /* endswitch */
  }
  else
  {
    //error message because of missing data
    UtlErrorHwnd( ERROR_SYNTAX_INCORRECT, MB_CANCEL,
                  0, NULL, EQF_ERROR, hwndDlg );

    //focus on mle at error position stored in ida
    FiltPosToError( pIda->hwndWhereMLE, pIda->F.pucWhereMLE,
                    pucWhereMLE, pucWhereMLE );

    fOK = FALSE;
  } /* endif */
  *ppucMLE = pucWhereMLE;
  return( fOK );
} /* end FiltTerm */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltConstant                                             |
//+----------------------------------------------------------------------------+
//|Function call:     FiltConstant( hwndDlg, pIda, ppucMLE )                   |
//+----------------------------------------------------------------------------+
//|Description:       FiltConstant detects strings, integers or field names    |
//|                   and stores these values on buffer                        |
//|                   E.g.the value 'John' in author = 'John'is a string or    |
//|                   create_date in update_date = create_date which is a field|
//|                   name.                                                    |
//|                                                                            |
//|  FiltConstant                                                              |
//|                                                                            |
//|                  +--------------+                                          |
//|    Â-----------+FiltIdentifier+--------------------------Â---          |
//|     |            +--------------+                                         |
//|                 +-----------+                              |              |
//|     +-----------+FiltNumber +-----------------------------+              |
//|     |            +-----------+                              |              |
//|     |                                                       |              |
//|     +---- '  --Â---------------------Â------  ' ------------+              |
//|                      +---------+                                         |
//|                +------+character+----+                                     |
//|                       +---------+                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   pIda        - pointer to global structure                |
//|                   ppucMLE     - pointer to current position in mle         |
//|                   fPush       - string is complete and may be pushed on    |
//|                                 stack                                      |
//|                   fAnd        - the last string after the AND operator of  |
//|                                 BETWEEN AND has been obtained from mle     |
//|                                 and the complete string may be concatenated|
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOLEAN                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch pointer to input string                           |
//|                     case ''':                                              |
//|                       remember pointer pos for possible error msg to mle   |
//|                       increment input string pointer                       |
//|                       if pointer = ''' and pointer+1 = blank then          |
//|                         mark empty string on filter stack                  |
//|                         FiltSkipBlanks                                     |
//|                       endif                                                |
//|                                                                            |
//|                       while not ''' and not EOS mle input                  |
//|                         put character on temp buffer                       |
//|                         increment input string pointer                     |
//|                         if string pointer = ''' and the next one also      |
//|                           put on filter stack                              |
//|                           increment pointer by two                         |
//|                         endif                                              |
//|                       endwhile                                             |
//|                                                                            |
//|                       if EOS mle input                                     |
//|                         error msg - quote missing                          |
//|                         break off - fOK = FALSE                            |
//|                       endif                                                |
//|                                                                            |
//|                         FiltPushOnStack                                    |
//|                         FiltPushOnBuffer                                   |
//|                                                                            |
//|                       increment mle pointer                                |
//|                       FiltSkipBlanks                                       |
//|                     default:                                               |
//|                       FiltReadToken                                        |
//|                       if not function FiltIdentifier then                  |
//|                         break off - error message issued elsewhere         |
//|                   endswitch                                                |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltConstant
(
HWND         hwndDlg,               //dialog handle
PFILTIDA     pIda,                  //filter ida
PUCHAR       *ppucMLE               //pointer to mle
)
{
  BOOL     fOK = TRUE;                //success indicator
  UCHAR    uChar;                     //character variable
  USHORT   usI = 0;                   //array counter
  PUCHAR   pucWhereMLE = *ppucMLE;    //pointer to mle
  PUCHAR   pucStartPos;               // start position of constant


  if ( (uChar = *pucWhereMLE ) != NULC )
  {
    pucStartPos = pucWhereMLE;

    switch ( uChar )
    {
      case QUOTE:
        pucWhereMLE++;              // skip opening quote

        while ( fOK && *pucWhereMLE )
        {
          //handle two quotes in a row to indicate a quote in string
          //e.g. 'Mother''s Day greetings'
          if ( *pucWhereMLE == QUOTE )
          {
            pucWhereMLE++;
            if ( *pucWhereMLE == QUOTE )
            {
              pIda->szTemp[usI] = *pucWhereMLE;
              pucWhereMLE++;
              usI++;
            }
            else
            {
              fOK = FALSE;   //end of string quote so leave while loop
            } /* endif */
          }
          else
          {
            pIda->szTemp[usI] = *pucWhereMLE;
            pucWhereMLE++;
            usI++;
          } /* endif */
        } /* endwhile */

        //if mle ended and fOK is still true end quote was missing
        if ( fOK && !*pucWhereMLE )
        {
          //error message that quote is missing
          UtlErrorHwnd( ERROR_QUOTE_MISSING, MB_CANCEL,
                        0, NULL, EQF_ERROR, hwndDlg );

          //focus on mle at error position stored in ida
          FiltPosToError( pIda->hwndWhereMLE, pIda->F.pucWhereMLE,
                          pucStartPos, pucWhereMLE );


          fOK = FALSE;
        }
        else
        {
          //indicate end of string
          pIda->szTemp[usI] = EOS;

          fOK = TRUE;   //string end found
        } /* endif */

        if ( fOK )
        {
          //save operator id and offset on stack
          fOK = FiltPushOnStack( pIda, OP_STRING, pIda->F.usBufferUsed );

          //save data in data buffer area
          fOK = FiltPushOnBuffer( pIda );

          //skip blanks
          FiltSkipBlanks( &pucWhereMLE );
        } /* endif */
        break;
      case '0' :
      case '1' :
      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' :
      case '-' :
      case '+' :
        /***********************************************************/
        /* Process numeric constant                                */
        /***********************************************************/
        do
        {
          pIda->szTemp[usI++] = *pucWhereMLE++;
        } while ( (*pucWhereMLE >= '0') && (*pucWhereMLE <= '9') );
        pIda->szTemp[usI] = EOS;
        fOK = FiltPushOnStack( pIda, OP_STRING, pIda->F.usBufferUsed );

        //save data in data buffer area
        if ( fOK )
        {
          fOK = FiltPushOnBuffer( pIda );
        } /* endif */

        //skip blanks
        if ( fOK )
        {
          FiltSkipBlanks( &pucWhereMLE );
        } /* endif */
        break;

      default:
        //read input from mle
        pucStartPos = pucWhereMLE;
        FiltReadToken( pIda, &pucWhereMLE );

        //test if a valid dictionary field name
        fOK = FiltIdentifier( hwndDlg, pIda, TRUE );
        if ( !fOK )
        {
          FiltPosToError( pIda->hwndWhereMLE, pIda->F.pucWhereMLE,
                          pucStartPos, pucWhereMLE );
        } /* endif */
        break;
    } /* endswitch */
  } /* endif */
  *ppucMLE = pucWhereMLE;
  return( fOK );
} /* end FiltConstant */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltIdentifier                                           |
//+----------------------------------------------------------------------------+
//|Function call:     FiltIdentifier( hwnd, pIda, fWhere )                     |
//+----------------------------------------------------------------------------+
//|Description:       FiltIdentifier checks dictionary entry field names.      |
//|                   E.g.the author in author = 'John' needs to be a valid    |
//|                   entry field name in the dictionary for which the filter  |
//|                   is being set. Needs to be in the list of available fields|
//+----------------------------------------------------------------------------+
//|Parameters:        hwnd        - dialog handle                              |
//|                   pIda        - pointer to global structure                |
//|                   fWhere      - fWhere true then fiel name in WHERE mle    |
//|                               - else field name in SELECT mle              |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOLEAN                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     if TempBuffer not in list of available fields then       |
//|                     issue error message that field name is invalid         |
//|                     fOK = FALSE                                            |
//|                   else                                                     |
//|                     if fWhere                                              |
//|                       save field name offset to filter stack               |
//|                       save TempBuffer to filter stack                      |
//|                     else                                                   |
//|                       save field name offset to select stack               |
//|                       save TempBuffer to filter stack                      |
//|                     endif                                                  |
//|                     save all field names in all field names stack          |
//|                   endif                                                    |
//|                   return fOK                                               |
//+----------------------------------------------------------------------------+

static
BOOL FiltIdentifier
(
HWND         hwndDlg,                         //dialog handle
PFILTIDA     pIda,                            //filter ida
BOOL         fWhere                           //save to which part in filter
)
{
  BOOL        fOK = TRUE;             // success indicator
  SHORT       sItem;                  // listbox counter
  PSZ         pError;                 // pointer to error string
  PPROFENTRY  pEntry;                 // ptr to dictionary profile entry

  //see if szTemp is in the listbox with all available entry field names
  sItem = SEARCHITEM( hwndDlg, ID_FILT_AVAILFIELDS_LB, pIda->szTemp );

  //if entryfield name not in listbox issue warning
  if ( sItem == LIT_NONE || sItem == LIT_ERROR )
  {
    if ( pIda->szTemp[0] == EOS )
    {
      UtlErrorHwnd( ERROR_INCOMPLETE_CONDITION, MB_CANCEL, 0,
                    NULL, EQF_ERROR, hwndDlg );
    }
    else
    {
      pError = pIda->szTemp;
      UtlErrorHwnd( ERROR_FIELDNAME_INVALID, MB_CANCEL, 1,
                    &pError, EQF_ERROR, hwndDlg );
    } /* endif */

    fOK = FALSE;     //break off
  } /* endif */

  //store in filterstack only if reading from WHERE mle
  if ( fOK )
  {
    /*****************************************************************/
    /* Get pointer to dictionary profile entry                       */
    /*****************************************************************/
    pEntry = (PPROFENTRY)QUERYITEMHANDLE( hwndDlg, ID_FILT_AVAILFIELDS_LB,
                                          sItem );

    /*****************************************************************/
    /* Replace user's field name with user name from dictionary      */
    /*****************************************************************/
    strcpy( pIda->szTemp, pEntry->chUserName );
    OEMTOANSI( pIda->szTemp );

    /*****************************************************************/
    /* Add field to approbriate stack                                */
    /*****************************************************************/
    if ( fWhere )
    {
      //save operator id and fiekdname offset on stack
      fOK = FiltPushOnStack( pIda, OP_FIELDNAME, pIda->F.usBufferUsed );

      //save field name in data buffer area
      fOK = FiltPushOnBuffer( pIda );
    }
    else
    {
      //store fields from SELECT mle
      //save offset on selnames
      fOK = FiltPushOnSelNames( pIda, pIda->F.usBufferUsed );

      //save field name in data buffer area
      fOK = FiltPushOnBuffer( pIda );
    } /* endif */
  } /* endif */

  //store all field names in where and select
  if ( fOK )
  {
    //save offset on AllNames
    fOK = FiltPushOnAllNames( pIda, pIda->F.usBufferUsed );

    //save field name in data buffer area
    if ( fOK )
      fOK = FiltPushOnBuffer( pIda );
  } /* endif */

  return( fOK );
} /* end FiltIdentifier */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltSkipBlanks                                           |
//+----------------------------------------------------------------------------+
//|Function call:     FiltSkipBlanks( PUCHAR *ppCurPos )                       |
//+----------------------------------------------------------------------------+
//|Description:       FiltSkipBlanks reads blanks and white spaces from mle input |
//|                   string                                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        pIda        - pointer to global structure                |
//|                   ppucMLE     - pointer to current position in mle         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     while input string pointer = blank or LF or NL           |
//|                     increment mle input pointer                            |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
static
VOID FiltSkipBlanks
(
PUCHAR    *ppucCurPos                   //pointer to current position
)
{
  BOOL     fOK = TRUE;                    //success indicator
  UCHAR    uChar;                         //character variable
  PUCHAR   pucCurPos = *ppucCurPos;             //pointer to mle

  while ( fOK && ( uChar = *pucCurPos ) != NULC )
  {
    switch ( uChar )
    {
      case BLANK:
      case CR:
      case LF:
        pucCurPos++;                // increment current position pointer
        break;

      default :
        fOK = FALSE;                // not blank or white space
        break;
    } /* end switch */
  } /* endwhile */
  *ppucCurPos = pucCurPos;
} /* end FiltSkipBlanks */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltOpDlg                                                |
//+----------------------------------------------------------------------------+
//|Function call:     FiltOpDlg( hwndDlg, msg, mp1, mp2 )                      |
//+----------------------------------------------------------------------------+
//|Description:       Handles the filter operator dialogs for the novice user  |
//|                   not as yet acquainted with the filter condition syntax.  |
//|                   The operator dialogs are triggered by double clicking    |
//|                   or pressing enter on an operator in the listbox with     |
//|                   all available operators. There are three dialogs in all  |
//|                   and the dialog that appears is dependent on the operator |
//|                   selected.                                                |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   msg         - message parameter                          |
//|                   mp1         - first parameter                            |
//|                   mp2         - second parameter                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE or FALSE                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch message                                           |
//|                     case wm_initdlg:                                       |
//|                       call up an operator dialog dependent on the operator |
//|                       selected filling all text statics on the dialog      |
//|                       panels in accordance with the operator selected.     |
//|                     case wm_command:                                       |
//|                       if cancel                                            |
//|                         dismiss dialog                                     |
//|                       else if ok button pressed on any of the three dialogs|
//|                         remember the value(s) to be associated with the    |
//|                           selected entry field adding quotes               |
//|                       special handling for like, between and in            |
//|                     case wm_close:                                         |
//|                       dismiss dialog                                       |
//|                     default:                                               |
//|                       default window processing                            |
//|                   endswitch                                                |
//|                   return mResult                                           |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK FiltOpDlg
(
HWND   hwndDlg,
WINMSG message,
WPARAM mp1,
LPARAM mp2
)
{
  PFILTIDA    pIda;                       //ptr to instance data area
  MRESULT     mResult = FALSE;            //return value of procedure
  CHAR        chBuffer[TXT_SIZE];         //general work buffer
  USHORT      usI;                        //counter
  BOOL        fOK = TRUE;                 //success indicator
  BOOL        fFirstElement;          // value is first element of IN list

  switch ( message)
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );
      switch ( pIda->usOpDlg )
      {
        case 1 : HANDLEQUERYID( ID_FILTOP1_DLG, mp2 ); break;
        case 2 : HANDLEQUERYID( ID_FILTOP2_DLG, mp2 ); break;
        case 3 : HANDLEQUERYID( ID_FILTOP2_DLG, mp2 ); break;
      } /* endswitch */
      break;


    case WM_HELP:
/*************************************************************/
/* pass on a HELP_WM_HELP request                            */
/*************************************************************/
      EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblFiltop1Dlg[0] );
      mResult = TRUE;  // message processed
      break;


    case WM_INITDLG:
	{
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      //anchor IDA
      pIda = (PFILTIDA) mp2;
      ANCHORDLGIDA( hwndDlg, pIda);

      //fill in the general text on the dialog panel
      if ( pIda->usOpDlg == 1 )
      {
        LOADSTRING( (HAB)UtlQueryULong( QL_HAB ), hResMod,
                    SID_FILTOP1_TEXT, chBuffer );

        //add text to dialog
        SETTEXT( hwndDlg, ID_FILTOP1_INFO_TEXT, chBuffer );

        //add field name and operator to dialog
        strcpy( pIda->szString, pIda->szOpFieldName );
        strcat( pIda->szString, "  ");
        strcat( pIda->szString, pIda->szOperator );
        SETTEXT( hwndDlg, ID_FILTOP1_FIELD_TEXT, pIda->szString );
        strcat( pIda->szString, "  ");

      }
      else if ( pIda->usOpDlg == 2 )
      {
        LOADSTRING( (HAB)UtlQueryULong( QL_HAB ), hResMod,
                    SID_FILTOP2_TEXT, chBuffer );

        //add text to dialog
        SETTEXT( hwndDlg, ID_FILTOP2_INFO_TEXT, chBuffer );

        //add field name and operator to dialog
        strcpy( pIda->szString, pIda->szOpFieldName );
        strcat( pIda->szString, "  ");
        strcat( pIda->szString, pIda->szOperator );
        SETTEXT( hwndDlg, ID_FILTOP2_FIELD1_TEXT, pIda->szString );
        strcat( pIda->szString, "  ");
      }
      else
      {
        LOADSTRING( (HAB)UtlQueryULong( QL_HAB ), hResMod,
                    SID_FILTOP3_TEXT, chBuffer );

        //add text to dialog
        SETTEXT( hwndDlg, ID_FILTOP3_INFO_TEXT, chBuffer );

        //add field name and operator to dialog
        strcpy( pIda->szString, pIda->szOpFieldName );
        strcat( pIda->szString, "  ");
        strcat( pIda->szString, pIda->szOperator );
        SETTEXT( hwndDlg, ID_FILTOP3_FIELD1_TEXT, pIda->szString );
        strcat( pIda->szString, "  ");
      }
      {
        USHORT  usCharSet;

        usCharSet = GetCharSet();
        //fill in the general text on the dialog panel
        if ( pIda->usOpDlg == 1 )
        {
          SetCtrlFnt( hwndDlg, usCharSet,
                      ID_FILTOP1_FIELD_TEXT,  ID_FILTOP1_VALUE_EF );
        }
        else if ( pIda->usOpDlg == 2 )
        {
          SetCtrlFnt( hwndDlg, usCharSet,
                      ID_FILTOP2_FIELD1_TEXT,  ID_FILTOP2_VALUE1_EF );
          SetCtrlFnt( hwndDlg, usCharSet,
                      ID_FILTOP2_FIELD2_TEXT,  ID_FILTOP2_VALUE2_EF );
        }
        else
        {
          SetCtrlFnt( hwndDlg, usCharSet,
                      ID_FILTOP3_FIELD1_TEXT, 0 );
          SetCtrlFnt( hwndDlg, usCharSet,
                      ID_FILTOP3_VALUE1_EF,  ID_FILTOP3_VALUE2_EF );
          SetCtrlFnt( hwndDlg, usCharSet,
                      ID_FILTOP3_VALUE3_EF,  ID_FILTOP3_VALUE4_EF );
          SetCtrlFnt( hwndDlg, usCharSet,
                      ID_FILTOP3_VALUE5_EF,  ID_FILTOP3_VALUE6_EF );
        } /* endif */
      }
	  }
      break;

    case WM_CLOSE:
      pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );
      // delete the created fonts
      if ( pIda->usOpDlg == 1 )
      {
        DelCtrlFont( hwndDlg, ID_FILTOP1_FIELD_TEXT );
      }
      else if ( pIda->usOpDlg == 2 )
      {
        DelCtrlFont( hwndDlg, ID_FILTOP2_FIELD1_TEXT );
        DelCtrlFont( hwndDlg, ID_FILTOP2_FIELD2_TEXT );
      }
      else
      {
        DelCtrlFont( hwndDlg, ID_FILTOP3_FIELD1_TEXT );
        DelCtrlFont( hwndDlg, ID_FILTOP3_VALUE1_EF );
        DelCtrlFont( hwndDlg, ID_FILTOP3_VALUE3_EF );
        DelCtrlFont( hwndDlg, ID_FILTOP3_VALUE5_EF );
      }
      WinDismissDlg( hwndDlg, SHORT1FROMMP1(mp1) );
      mResult = MRFROMSHORT( FALSE );
      break;

    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwndDlg, PFILTIDA );
      mResult = MRFROMSHORT( TRUE);

      switch (WMCOMMANDID( mp1, mp2 ))
      {
        case ID_FILTOP1_HELP_PB:
        case ID_FILTOP2_HELP_PB:
        case ID_FILTOP3_HELP_PB:
          UtlInvokeHelp();
          break;
        case DID_CANCEL:
        case ID_FILTOP1_CANCEL_PB:
//            WinDismissDlg( hwndDlg, FALSE );
          WinPostMsg ( hwndDlg, WM_CLOSE, MP1FROMSHORT(FALSE), NULL );
          break;

        case ID_FILTOP1_OK_PB:
          switch ( pIda->usOpDlg )
          {
            case 1 :
              //remember value for entry field
              QUERYTEXT( hwndDlg, ID_FILTOP1_VALUE_EF, pIda->szTemp );

              //if operator is LIKE then check if asterisk or question mark
              //present, else add asterisk
              if ( !stricmp( pIda->szOperator, LIKE_STR) )
              {
                if ( !strchr( pIda->szTemp,
                              SINGLE_SUBSTITUTION )  &&
                     !strchr( pIda->szTemp,
                              MULTIPLE_SUBSTITUTION ) )
                  strcat( pIda->szTemp, MULT_SUB_STR );
              } /* endif */

              //if quotes included in value then add preceding quote so that
              //quote is treated correctly and if string add quotes
              FiltQuoteStringIdentification( pIda );

              //add value to string
              strcat( pIda->szString, pIda->szValue );
              //add blanks for better spacing in mle
              strcat( pIda->szString, "  ");
              break;

            case 2 :
              //remember value of first entry field
              QUERYTEXT( hwndDlg, ID_FILTOP2_VALUE1_EF, pIda->szTemp );

              //if quotes included in value then add preceding quote so that
              //quote is treated correctly and if string add quotes
              FiltQuoteStringIdentification( pIda );

              //add value to string
              strcat( pIda->szString, pIda->szValue );
              //add blanks for better spacing in mle
              strcat( pIda->szString, "  ");

              //remember value of second entry field
              QUERYTEXT( hwndDlg, ID_FILTOP2_VALUE2_EF, pIda->szTemp );

              //if quotes included in value then add preceding quote so that
              //quote is treated correctly and if string add quotes
              FiltQuoteStringIdentification( pIda );

              //add AND and last value of between sequence
              strcat( pIda->szString, "AND  " );
              strcat( pIda->szString, pIda->szValue );
              //add blanks for better spacing in mle
              strcat( pIda->szString, "  ");
              break;

            case 3 :
              usI = 0;
              fFirstElement = TRUE;
              while ( usI < 6 && fOK )
              {
                QUERYTEXT( hwndDlg, ID_FILTOP3_VALUE1_EF + usI, pIda->szTemp );
                if ( fFirstElement )
                {
                  if ( !pIda->szTemp[0] )
                  {
                    UtlErrorHwnd( ERROR_EF_EMPTY, MB_CANCEL, 0,
                                  NULL, EQF_ERROR, hwndDlg );
                    fOK = FALSE;
                    pIda->szString[0] = NULC;
                  }
                  else
                  {
                    strcat( pIda->szString, "( " );
                    FiltQuoteStringIdentification( pIda );
                    strcat( pIda->szString, pIda->szValue );
                    fFirstElement = FALSE;
                  } /* endif */
                }
                else
                {
                  if ( pIda->szTemp[0] )
                  {
                    strcat( pIda->szString, ", " );
                    FiltQuoteStringIdentification( pIda );
                    strcat( pIda->szString, pIda->szValue );
                  } /* endif */
                } /* endif */
                usI++;
              } /* endwhile */

              //add closing bracket if all well
              if ( fOK )
                strcat( pIda->szString, ")  " );

              break;

            default :
              break;
          } /* endswitch */

          //close dialog
          WinPostMsg ( hwndDlg, WM_CLOSE, MP1FROMSHORT(fOK), NULL );
//            WinDismissDlg( hwndDlg, fOK );
          break;

        case ID_FILTOP3_VALUE1_EF:
        case ID_FILTOP3_VALUE2_EF:
        case ID_FILTOP3_VALUE5_EF:
          if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
          {
            ClearIME( hwndDlg );
          } /* endif */
          break;
      } /* endswitch */
      break;

    default:
      mResult = WinDefDlgProc (hwndDlg, message, mp1, mp2 );
  }
  return( mResult );
} /* end FiltOpDlg */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltQuoteStringIdentification                            |
//+----------------------------------------------------------------------------+
//|Function call:     FiltQuoteStringIdentification( pIda )                    |
//+----------------------------------------------------------------------------+
//|Description:       This function adds quotes to a string - if the string    |
//|                   is empty it forms the string '' and if the string        |
//|                   contains a quotes somewhere in the middle it indicates   |
//|                   that the quote is to stay by adding another quote, e.g.  |
//|                   'mother''s day'                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        pIda        - pointer to global structure                |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if string is empty                                       |
//|                     indicate using two quotes - ''                         |
//|                   else                                                     |
//|                     add quote at beginning and end of string               |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

static
VOID FiltQuoteStringIdentification( PFILTIDA pIda )
{
  USHORT usI = 0;      //counter
  USHORT usJ = 0;      //counter

  //if empty add quotes
  if ( pIda->szTemp[usI] == NULC )
  {
    pIda->szValue[usI] = QUOTE;                               /* 3@KIT1002C */
    pIda->szValue[1] = QUOTE;
    pIda->szValue[2] = EOS;
  }
  else
  {
    //if quotes included in value then add preceding quote so that
    //quote is treated correctly
    while ( pIda->szTemp[usI] != EOS )
    {
      if ( pIda->szTemp[usI] != QUOTE )
      {
        pIda->szValue[usJ] = pIda->szTemp[usI];
        usI++;
        usJ++;
      }
      else
      {
        pIda->szValue[usJ] = pIda->szTemp[usI];
        usJ++;
        pIda->szValue[usJ] = pIda->szTemp[usI];
        usI++;
        usJ++;
      } /* endif */
    } /* endwhile */
    pIda->szValue[usJ] = EOS;
    strcpy( pIda->szTemp, pIda->szValue );

    // add start and end quote
    strcpy( pIda->szValue, "'" );
    strcat( pIda->szValue, pIda->szTemp );
    strcat( pIda->szValue, "'" );
  } /* endif */
} /* end FiltQuoteStringIdentification */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltFillEntryLB                                          |
//+----------------------------------------------------------------------------+
//|Function call:     FiltFillEntryLB( hwndDlg, pDictProp )                    |
//+----------------------------------------------------------------------------+
//|Description:       This function takes the entry fields belonging to        |
//|                   the dictionary that's calling the filter function and    |
//|                   fill thae available entry fields listbox on the          |
//|                   filter dialog with the entry fields filling              |
//|                   all blanks between words that make up an entry field     |
//|                   name with underscore.                                    |
//+----------------------------------------------------------------------------+
//|Parameters:        hwndDlg     - dialog handle                              |
//|                   pDictprop   - pointer to dictionary properties           |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get all entry fields in dict property file               |
//|                   and add to available entry fields listbox on             |
//|                   the filter dialog fill the blank between the words       |
//|                   that go to make up an entry field name with an           |
//+----------------------------------------------------------------------------+

static
VOID FiltFillEntryLB( HWND hwndDlg, PPROPDICTIONARY pDictProp )
{
  PPROFENTRY pEntry;                  //pointer for profile entry processing
  HWND       hwndLB;                  //handle of entry listbox
  USHORT     usI;                     //loop counter
  SHORT      sItem;                   //item index
  CHAR        szBuffer[DICTENTRYLENGTH]; // buffer for field names

  pEntry = pDictProp->ProfEntry;
  hwndLB = WinWindowFromID( hwndDlg, ID_FILT_AVAILFIELDS_LB );

  ENABLEUPDATEHWND_FALSE( hwndLB );       // avoid flickering
  DELETEALLHWND( hwndLB );                 // clear listbox

  // fill listbox with profile entries
  for ( usI = 0; usI < pDictProp->usLength; usI++, pEntry++ )
  {
    /*****************************************************************/
    /* convert blanks to underscores                                 */
    /*****************************************************************/
    strcpy( szBuffer, pEntry->chUserName );
    FiltBlankToUS( szBuffer );

    /*****************************************************************/
    /* Add field name to listbox                                     */
    /*****************************************************************/
    OEMTOANSI( szBuffer );
    sItem = INSERTITEMENDHWND( hwndLB, szBuffer );
    if ( sItem != LIT_NONE )
    {
      SETITEMHANDLEHWND( hwndLB, sItem, pEntry );
    } /* endif */
  } /* endfor */


  ENABLEUPDATEHWND_TRUE( hwndLB );        // show changed listbox
  SELECTITEMHWND( hwndLB, 0 );             // select first listbox item
} /* end of FiltFillEntryLB */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltPosToError                                           |
//+----------------------------------------------------------------------------+
//|Function call:     FiltPosToError( hwndMLE, pucBuffer, pucStartPos,         |
//|                                   pucEndPos );                             |
//+----------------------------------------------------------------------------+
//|Description:       Set MLE cursor to position of error                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND     hwndMLE         handle of MLE                   |
//|                   PUCHAR   pucBuffer       pointer to buffer address       |
//|                   PUCHAR   pucStartPos     ptr to start position           |
//|                   PUCHAR   pucEndPos       ptr to end position             |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     Get line number of from position                         |
//|                   Get offset of from position                              |
//|                   Get from insertion point                                 |
//|                   if start and end pos are equal                           |
//|                     to insertion point := from insertion point             |
//|                   else                                                     |
//|                     Get line number of to position                         |
//|                     Get offset of to position                              |
//|                     Get from insertion point                               |
//|                   endif                                                    |
//|                  set MLE cursor position                                   |
//|                  set focus on MLE window                                   |
//+----------------------------------------------------------------------------+
VOID FiltPosToError
(
HWND     hwndMLE,                    // handle of MLE
PUCHAR   pucBuffer,                  // pointer to buffer address
PUCHAR   pucStartPos,                // ptr to start position
PUCHAR   pucEndPos                   // ptr to end position
)
{
  ULONG   ulFrom, ulTo;               // selection points
  ULONG   ulLineFrom, ulLineTo;       // line position of error
  ULONG   ulOffsFrom, ulOffsTo;       // offset within line of error
  PUCHAR   pucTemp;                    // work pointer

  /********************************************************************/
  /* Get line number of from position                                 */
  /********************************************************************/
  ulLineFrom  = 0;
  pucTemp = pucStartPos;
  while ( pucTemp >= pucBuffer )
  {
    if ( *pucTemp-- == LF )
    {
      ulLineFrom++;
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* Get offset of from position                                      */
  /********************************************************************/
  ulOffsFrom  = 0;
  pucTemp = pucStartPos;
  while ( (pucTemp >= pucBuffer ) && (*pucTemp != LF) )
  {
    pucTemp--;
    ulOffsFrom++;
  } /* endwhile */

  /********************************************************************/
  /* Get from insertion point                                         */
  /********************************************************************/
  MLECHARFROMLINEHWND( hwndMLE, ulLineFrom, ulFrom );
  ulFrom += ulOffsFrom;

  /********************************************************************/
  /* if start and end pos are equal use iptFrom for iptTo else        */
  /* compute to insertion point                                       */
  /********************************************************************/
  if ( pucStartPos == pucEndPos )
  {
    ulTo = ulFrom;
  }
  else
  {
    /********************************************************************/
    /* Get line number of to position                                   */
    /********************************************************************/
    ulLineTo  = 0;
    pucTemp = pucEndPos;
    while ( pucTemp >= pucBuffer )
    {
      if ( *pucTemp-- == LF )
      {
        ulLineTo++;
      } /* endif */
    } /* endwhile */

    /********************************************************************/
    /* Get offset of to position                                        */
    /********************************************************************/
    ulOffsTo  = 0;
    pucTemp = pucStartPos;
    while ( (pucTemp >= pucBuffer) && (*pucTemp != LF) )
    {
      pucTemp--;
      ulOffsTo++;
    } /* endwhile */

    /********************************************************************/
    /* Get to insertion point                                           */
    /********************************************************************/
    MLECHARFROMLINEHWND( hwndMLE, ulLineTo, ulTo );
    ulTo += ulOffsTo;

  } /* endif */

  MLESETSELHWND( hwndMLE, ulFrom, ulTo );

  SETFOCUSHWND( hwndMLE );

} /* end of function FiltPosToError */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltBlankToUS                                            |
//+----------------------------------------------------------------------------+
//|Function call:     FiltBlankToUS( PSZ pszFieldName );                       |
//+----------------------------------------------------------------------------+
//|Description:       Converts blank characters in field name to underscore    |
//|                   characters.                                              |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ     pszFieldName     ptr to field name               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     while not end of field                                   |
//|                     if current character is blank                          |
//|                       replace character with undrscore                     |
//|                     endif                                                  |
//|                     next character                                         |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
VOID FiltBlankToUS
(
PSZ     pszFieldName                 // ptr to field name
)
{
  while ( *pszFieldName )
  {
    if ( *pszFieldName == BLANK )
    {
      *pszFieldName++ = UNDERSCORE;
    }
    else
    {
      pszFieldName++;
    } /* endif */
  } /* endwhile */
} /* end of function FiltBlankToUS */
