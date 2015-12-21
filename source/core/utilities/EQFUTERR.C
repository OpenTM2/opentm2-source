/*! \file
	Description: EQF general services

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

/**********************************************************************/
/* Include files                                                      */
/**********************************************************************/
#define INCL_DEV
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_MORPH            // morph return codes
#include <eqf.h>                  // General Translation Manager include file

#include <time.h>                 // C time related functions


#include "EQFUTPRI.H"                  // private utility header file
#include <eqfutils.id>            // IDs used by dialog utilities

//#define DEF_EVENT_ENUMS
#include "EQFEVENT.H"                  // event logging functions

//static PEVENTTABLE pEventTable;        // ptr to event logging table
static SHORT  asLogGroups[30] = { -1 };// list of active debug log groups
extern PEVENTTABLE pEventTable;        // ptr to event logging table

//default variable used in error messages (will be changed to "" later on)
#define UTLERR_DEFVAR        "\nERROR: VARIABLE NOT SUPPLIED\n"

INT_PTR CALLBACK UTLYESTOALLDLG( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );

#if defined(UTL_LOGGING)
   #define UTLLOGPATH "\\EQFD\\BIN\\EQFUTL.LOG"
#endif

#define MAXNUMOFDDEMESSAGES  24


#define NO_ERRINFO_TXT       "NO SYSTEM ERROR INFO AVAILABLE"
#define NOMSG_TXT            "NO ERROR MESSAGE AVAILABLE"

/**********************************************************************/
/* Global variables                                                   */
/**********************************************************************/
       ERRDATA ErrData[MAX_TASK];      // data required for error handler
static CHAR chMsgBuffer[MSGBUFFERLENGTH]; // buffer for message text
static CHAR chDDEMsgBuffer[MSGBUFFERLENGTH]; // buffer for DDE messages
static CHAR   szDosError[80];          // buffer for DosErrorMessage
static DDEMSGBUFFER DDEMessages[MAXNUMOFDDEMESSAGES+1] = { { (HWND)0, 0, "" } } ;


CHAR  SYSTEM_ERROR_TXT [30];       // "SYSTEM ERROR"
CHAR  QDAM_ERROR_TXT [30]  ;       // "EQF ERROR"
CHAR  QLDB_ERROR_TXT [30]  ;       // "EQF ERROR"
CHAR  PROP_ERROR_TXT [30]  ;       // "EQF ERROR"
CHAR  EQF_ERROR_TXT [30]   ;       // "EQF ERROR"
CHAR  EQF_CMD_ERROR_TXT [30];      // "CMD ERROR"
CHAR  EQF_WARNING_TXT [30]  ;      // "EQF Warning"
CHAR  EQF_QUERY_TXT [30]    ;      // "EQF Query"
CHAR  EQF_INFO_TXT [30]     ;      // "EQF Information"
CHAR  INTERNAL_ERROR_TXT [30];     // "INTERNAL ERROR"


/**********************************************************************/
/* Protoypes for internal functions                                   */
/**********************************************************************/
SHORT  UtlQdamMsgTxt ( SHORT );
static SHORT UtlDosMsgTxt ( SHORT, PSZ, PUSHORT );
static SHORT UtlQldbMsgTxt ( SHORT );
static SHORT UtlPropMsgTxt ( SHORT );


//------------------------------------------------------------------------------
// External Function
//------------------------------------------------------------------------------
// Function Name:     UtlError               Report an error condition
//------------------------------------------------------------------------------
// Description:       Displays error message and handles link to help
//                    function (IPF).
//------------------------------------------------------------------------------
// Function Call:     USHORT UtlError( USHORT  usErrorID,
//                                     USHORT  usType,
//                                     USHORT  usNoOfParms,
//                                     PSZ     *pParmTable,
//                                     ERRTYPE ErrorType );
//------------------------------------------------------------------------------
// Parameters:        - usErrorID is the number of the error message to be
//                      displayed.
//                    - usType specifies the buttons to be contained in the
//                      message box (e.g. MB_OK for OK button, MB_YESNOCANCEL
//                      for YES, NO and CANCEL button).
//                      additional the default button has to be defined here
//                      (e.g. MB_YESNO | MB_DEFBUTTON?  ? will become default,
//                      where ? may be 1 2 or 3. )
//                    - usNoOfParms specifies the numbers of parameters (= size
//                      of parameter table) which are to be replaced in the
//                      message text
//                    - pParmTable is the pointer to pointer table for the
//                      paramter strings
//                    - ErrorType is the type of the error. Valid are:
//
//                      SYSTEM_ERROR   (errors in WINxxx and GPIxxx calls;
//                                      message text is retrieved by
//                                      WinQueryLastError )
//                      DOS_ERROR      (errors in DosXXX calls )
//                      EQF_ERROR      (user errors)
//                      EQF_INFO       (information message)
//                      EQF_WARNING    (warnings)
//                      INTERNAL_ERROR (internal processing errors)
//------------------------------------------------------------------------------
// Prerequesits:      The error handler has to be initialized during startup
//                    using the init call UtlInitError.
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       return code of WinMessageBox call indicating the selected
//                            button (e.g. MBID_OK = OK button)
//------------------------------------------------------------------------------
// Function flow:    Call UtlErrorHwnd with a NULL window handle
//------------------------------------------------------------------------------
USHORT UtlErrorW
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ_W   *pParmTableW,               // pointer to parameter table
   ERRTYPE ErrorType,                  // type of error
   BOOL    fUnicode                    // parameter table contains unicode strings
)
{
  return UtlErrorHwndW( sErrorNumber,               // number of message
                          usMsgType,                  // type of message
                          usNoOfParms,                // number of message parameters
                          pParmTableW,                // pointer to parameter table
                          ErrorType,                  // type of error
                          NULLHANDLE,                 // window handle
                          fUnicode );                 // parameter table contains unicode strings
}

USHORT UtlErrorHwndW
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ_W   *pParmTableW,               // pointer to parameter table
   ERRTYPE ErrorType,                  // type of error
   HWND    hwnd,                       // window handle
   BOOL    fUnicode                    // parameter table contains unicode strings
)
{
	USHORT usRc = 0;
	PSZ    pParmTable[5];
	int    i = 0;


	if (!fUnicode)
	{
		for ( i=0; i<usNoOfParms; i++ )
		{
			pParmTable[i] = (PSZ)pParmTableW[i];
		}

		usRc =  UtlErrorHwnd( sErrorNumber,
							usMsgType,
							usNoOfParms,
							pParmTable,
							ErrorType,
							hwnd );
	}
	else
	{
		PSZ pData = NULL;
		UtlAlloc((PVOID *) &pData, 0L, usNoOfParms * 256, NOMSG );
		if (pData || (usNoOfParms == 0))
		{
			for ( i=0; i<usNoOfParms; i++ )
			{
				pParmTable[i] = pData + i*256;
				if (UTF16strlenCHAR(pParmTableW[i]) >= 128)
				{
					CHAR_W  chW = pParmTableW[i][128];
					pParmTableW[i][128] = EOS;
					UtlDirectUnicode2Ansi( pParmTableW[i], pParmTable[i], 0L );
					pParmTableW[i][128] = chW;
				}
				else
				{
					UtlDirectUnicode2Ansi( pParmTableW[i], pParmTable[i], 0L );
				}
			}

			usRc =  UtlErrorHwnd( sErrorNumber,
								usMsgType,
								usNoOfParms,
								pParmTable,
								ErrorType,
								hwnd );
			UtlAlloc((PVOID *) &pData, 0L, 0L, NOMSG );
		}

	}
	return( usRc );
}




USHORT UtlError
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ     *pParmTable,                // pointer to parameter table
   ERRTYPE ErrorType                   // type of error
)
{
   return( UtlErrorHwnd( sErrorNumber,
                         usMsgType,
                         usNoOfParms,
                         pParmTable,
                         ErrorType,
                         NULLHANDLE ) );
}


//------------------------------------------------------------------------------
// External Function
//------------------------------------------------------------------------------
// Function Name:     UtlErrorHwnd           Report an error condition
//------------------------------------------------------------------------------
// Description:       Displays error message and handles link to help
//                    function (IPF).
//------------------------------------------------------------------------------
// Function Call:     USHORT UtlErrorHwnd( USHORT  usErrorID,
//                                         USHORT usType,
//                                         USHORT usNoOfParms,
//                                         PSZ *pParmTable,
//                                         ERRTYPE ErrorType,
//                                         HWND  hwndParent );
//------------------------------------------------------------------------------
// Parameters:        - usErrorID is the number of the error message to be
//                      displayed.
//                    - usType specifies the buttons to be contained in the
//                      message box (e.g. MB_OK for OK button, MB_YESNOCANCEL
//                      for YES, NO and CANCEL button).
//                      additional the default button has to be defined here
//                      (e.g. MB_YESNO | MB_DEFBUTTON?  ? will become default,
//                      where ? may be 1 2 or 3. )
//                    - usNoOfParms specifies the numbers of parameters (= size
//                      of parameter table) which are to be replaced in the
//                      message text
//                    - pParmTable is the pointer to pointer table for the
//                      paramter strings
//                    - ErrorType is the type of the error. Valid are:
//
//                      SYSTEM_ERROR   (errors in WINxxx and GPIxxx calls;
//                                      message text is retrieved by
//                                      WinQueryLastError )
//                      DOS_ERROR      (errors in DosXXX calls )
//                      EQF_ERROR      (user errors)
//                      EQF_INFO       (information message)
//                      EQF_WARNING    (warnings)
//                      EQF_QUERY      (queries)
//                      QDAM_ERROR
//                      QLDB_ERROR
//                      PROP_ERROR
//                      INTERNAL_ERROR (internal processing errors)
//
//                    - hwndParent is the parent window for the error
//                      message box
//------------------------------------------------------------------------------
// Prerequesits:      The error handler has to be initialized during startup
//                    using the init call UtlInitError.
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       return code of WinMessageBox call indicating the selected
//                            button (e.g. MBID_OK = OK button)
//------------------------------------------------------------------------------
// Function flow:     mask given message type to strip off invalid flags
//                    switch error type
//                      case EQF_ERROR:
//                        MB style = MB_ERROR
//                        title    = EQF_ERROR_TEXT
//                        call UtlGetMsgTxt to get text of message
//                      case EQF_WARNING:
//                        MB style = MB_WARNING
//                        title    = EQF_WARNING_TEXT
//                        call UtlGetMsgTxt to get text of message
//                      case EQF_INFO:
//                        MB style = MB_INFORMATION
//                        title    = EQF_INFOR_TEXT
//                        call UtlGetMsgTxt to get text of message
//                      case EQF_QUERY:
//                        MB style = MB_QUERY
//                        title    = EQF_QUERY_TEXT
//                        call UtlGetMsgTxt to get text of message
//                      case SYSTEM_ERROR:
//                        MB style = MB_ERROR
//                        title    = SYSTEM_ERROR_TEXT
//                        call WinGetErrorInfo to get latest error info
//                         and use the supplied data
//                      case DOS_ERROR:
//                        MB style = MB_ERROR
//                        title    = SYSTEM_ERROR_TEXT
//                        call UtlDosMsgTxt to get message for DOS error
//                        call UtlGetMsgTxt to get text of message
//                      case QDAM_ERROR:
//                        MB style = MB_ERROR
//                        title    = QDAM_ERROR_TEXT
//                        call UtlQdamMsgTxt to get message for QDAM error
//                        call UtlGetMsgTxt to get text of message
//                      case QLDB_ERROR:
//                        MB style = MB_ERROR
//                        title    = QLDB_ERROR_TEXT
//                        call UtlQldbMsgTxt to get message for QLDB error
//                        call UtlGetMsgTxt to get text of message
//                      case PROP_ERROR:
//                        MB style = MB_ERROR
//                        title    = PROP_ERROR_TEXT
//                        call UtlPropMsgTxt to get message for PROP error
//                        call UtlGetMsgTxt to get text of message
//                      case INTERNAL_ERROR:
//                        MB style = MB_ERROR
//                        title    = INTERNAL_ERROR_TEXT
//                        call UtlGetMsgTxt to get internal error message text
//                    endswitch
//                    search parent handle for message box or use supplied
//                     handle
//                    add help flag to message box style if help instance exist
//                    if process has a message queue then
//                      call WinMessageBox to show the error message
//                    else
//                      issue a series of beeps as no message box can be shown
//                    endif
//                    free any error info
//------------------------------------------------------------------------------
USHORT UtlErrorHwnd
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ     *pParmTable,                // pointer to parameter table
   ERRTYPE ErrorType,                  // type of error
   HWND    hwndMsgBoxParent            // window which should be msgbox parent
)
{
   unsigned int  usMsgboxStyle;               // style for WinMessageBox call
   unsigned int  usMsgboxRet = 0;             // msg box return value
   PSZ    pMsgboxTitle;                // title text of message box
   PSZ    pErrorText;                  // pointer to system error text
   PSZ    pDosError;                   // pointer to DOS return code string
   PSZ    pQdamError[2];               // pointer to QDAM return code string
   CHAR   chNum[10];                   // itoa string buffer for message number
   PSZ    pQldbError;                  // pointer to QLDB return code string
   PSZ    pPropError;                  // pointer to PROP return code string
   HWND   hwndFrame;                   // handle of troja main window
   HWND    hwndParent;                 // parent window for message box call
   CHAR   szError[80];                 // buffer for Messages
   usMsgType &= ( 0x0F | MB_DEFBUTTON1 | MB_DEFBUTTON2 | MB_DEFBUTTON3 );

   switch ( ErrorType )
   {
      case EQF_ERROR :
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = EQF_ERROR_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case CMD_ERROR :
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = EQF_CMD_ERROR_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case CMD_INFO :
         usMsgboxStyle = MB_INFORMATION;
         pMsgboxTitle = EQF_CMD_ERROR_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case EQF_WARNING:
         usMsgboxStyle = MB_WARNING;
         pMsgboxTitle = EQF_WARNING_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case EQF_INFO:
         usMsgboxStyle = MB_INFORMATION;
         pMsgboxTitle = EQF_INFO_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case EQF_QUERY:
         usMsgboxStyle = MB_QUERY;
         pMsgboxTitle = EQF_QUERY_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case SYSTEM_ERROR :
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = SYSTEM_ERROR_TXT;
         pErrorText = NO_ERRINFO_TXT;
         strncpy( chMsgBuffer, pErrorText, sizeof(chMsgBuffer) );
         break;

      case DOS_ERROR :
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = SYSTEM_ERROR_TXT;
         if ( usNoOfParms && pParmTable && *pParmTable )
         {
            strncpy( szDosError, *pParmTable, sizeof(szDosError) );
            szDosError[sizeof(szDosError)-1] = EOS;
         }
         else
         {
            szDosError[0] = EOS;
         } /* endif */
         sErrorNumber = UtlDosMsgTxt ( sErrorNumber, szDosError,
                                       &usMsgType );
         pDosError = szDosError;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, 1, &pDosError );
//         strcpy(chMsgBuffer, "Ebbi's special ");
         break;

      case QDAM_ERROR :
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = QDAM_ERROR_TXT;
         if ( usNoOfParms && pParmTable && *pParmTable )
         {
            strncpy( szError, *pParmTable, sizeof(szError) );
            szError[sizeof(szError)-1] = EOS;
         }
         else
         {
            szError[0] = EOS;
         } /* endif */
         pQdamError[0] = szError;
     pQdamError[1] = _itoa( sErrorNumber, chNum, 10 );
     sErrorNumber = UtlQdamMsgTxt ( sErrorNumber );
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, 2, &pQdamError[0] );
         break;

      case QLDB_ERROR :
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = QLDB_ERROR_TXT;
         if ( usNoOfParms && pParmTable && *pParmTable )
         {
            strncpy( szError, *pParmTable, sizeof(szError) );
            szError[sizeof(szError)-1] = EOS;
         }
         else
         {
            szError[0] = EOS;
         } /* endif */
         sErrorNumber = UtlQldbMsgTxt ( sErrorNumber );
         pQldbError = szError;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, 1, &pQldbError );
         break;

      case PROP_ERROR :
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = PROP_ERROR_TXT;
         if ( usNoOfParms && pParmTable && *pParmTable )
         {
            strncpy( szError, *pParmTable, sizeof(szError) );
            szError[sizeof(szError)-1] = EOS;
         }
         else
         {
            szError[0] = EOS;
         } /* endif */
         sErrorNumber = UtlPropMsgTxt ( sErrorNumber ) ;
         pPropError = szError;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, 1, &pPropError );
         break;

      case SHOW_ERROR :
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = EQF_ERROR_TXT;
         strcpy( chMsgBuffer, *pParmTable );
         break;

      case INTERNAL_ERROR :
      default :                        // unknown error type ==> internal error!
         usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = INTERNAL_ERROR_TXT;
         UtlGetMsgTxt( ERROR_INTERNAL, chMsgBuffer, usNoOfParms, pParmTable );
         break;

   } /* endswitch */

   switch (usMsgboxStyle)
   {
     case MB_ERROR:
        ERREVENT2( UTLERROR_LOC, MSGBOX_EVENT, sErrorNumber, GENERAL_GROUP, NULL );
        break;
     default:
        INFOEVENT2( UTLERROR_LOC, MSGBOX_EVENT, sErrorNumber, GENERAL_GROUP, NULL );
        break;
   } /* endswitch */

   usMsgboxStyle |= MB_DEFBUTTON1 | MB_MOVEABLE | usMsgType;
   usMsgboxStyle |= MB_APPLMODAL;

   // GQ 2015/10/13 Set parent handle of message box to HWND_FUNCIF when running in API mode and no parent handle has been specififed
   if ( !hwndMsgBoxParent && (UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE ) )
   {
     hwndMsgBoxParent = HWND_FUNCIF;
   }
  
   hwndParent = hwndMsgBoxParent;      // use supplied window handle

   if ( !hwndParent )
   {
      hwndParent     = QUERYACTIVEWINDOW();
      hwndFrame      = ErrData[UtlGetTask()].hwndFrame;
      if ( (GETPARENT( hwndParent ) == hwndFrame) ||
            (GETOWNER( hwndParent ) == hwndFrame) )
      {
         // ok, this window belongs to Troja
      }
      else
      {
         // focus seems to be on another application
         hwndParent     = hwndFrame;
      } /* endif */
   } /* endif */

   usMsgboxStyle |= ( ErrData[UtlGetTask()].hwndHelpInstance ) ? MB_HELP : 0;

   /*******************************************************************/
   /* Set last used message number not for ERROR_CMD_ERROR to avoid   */
   /* overwrite of actual error code                                  */
   /*******************************************************************/
   if ( sErrorNumber != ERROR_CMD_ERROR )
   {
     UtlSetUShort( QS_LASTERRORMSGID, sErrorNumber );
   } /* endif */
   UtlSetUShort( QS_CURMSGID, (USHORT)sErrorNumber );

   if ( hwndMsgBoxParent == HWND_FUNCIF )
   {
     PDDEMSGBUFFER pMsgBuffer = ErrData[UtlGetTask()].pLastMessage;
     if ( pMsgBuffer != NULL )
     {
       pMsgBuffer->sErrNumber  = sErrorNumber;
       strcpy( pMsgBuffer->chMsgText, chMsgBuffer );
     } /* endif */
   }
   else
   {
	   // display message box
	   if ( (usMsgboxStyle & 0x000F) == MB_EQF_YESTOALL )
	   {
		 YESTOALL_IDA IDA;
		 HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

		 IDA.pszTitle = pMsgboxTitle;
		 IDA.pszText  = chMsgBuffer;
		 IDA.sID      = sErrorNumber;
		 IDA.usStyle  = (USHORT)usMsgboxStyle;

		 DIALOGBOX( hwndParent, UTLYESTOALLDLG, hResMod,
					ID_UTL_YESTOALL_DLG,
					&IDA, usMsgboxRet );
	   }
	   else
	   { 
		   //   No RightStyle for message boxes
		   //     usMsgboxStyle |= MB_RTLREADING | MB_RIGHT;
		   usMsgboxRet = WinMessageBox ( HWND_DESKTOP,
									     hwndParent,
									     chMsgBuffer,
									     pMsgboxTitle,
									     sErrorNumber,
									     usMsgboxStyle );
	   } /* endif */
   }
   UtlSetUShort( QS_CURMSGID, 0 );

   return ( (USHORT)usMsgboxRet );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlGetMsgTxt           Get message text
//------------------------------------------------------------------------------
// Function call:     UtlGetMsgTxt( SHORT msgNo, PSZ pBuffer,
//                                  USHORT usNoOfParms, PSZ *pParmTable );
//------------------------------------------------------------------------------
// Description:       Get the text for a given message number
//------------------------------------------------------------------------------
// Input parameter:   SHORT    msgNo          message number = ID in MSG file
//                    PSZ      pBuffer        buffer for message text
//                    USHORT   usNoOfParms    number of parameters in table
//                    PSZ      *pParmTable    pointer to parameter table
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Prerequesits:      Name of message file must be stored in the ErrData
//                    structure
//------------------------------------------------------------------------------
// Function flow:     if no parameter supplied then
//                      set parameter pointer to VARIABLE_NOT_SUPPLIED text
//                    endif
//                    get message text using DosGetMessage
//                    remove CR/LF from message text, replace \n with LF
//------------------------------------------------------------------------------
VOID UtlGetMsgTxt
(
   SHORT    msgNo,                     // message number = ID in MSG file
   PSZ      pBuffer,                   // buffer for message text
   USHORT   usNoOfParms,               // number of parameters in table
   PSZ      *pParmTable                // pointer to parameter table
)
{
   ULONG  ulLength = 0;                // length of message buffer
   PSZ    pMsgText;                    // pointer for message text processing
   PSZ    pDefVar;                     // pointer for default insert variable

   //--- switch to default insert variable if no variable is supplied ---
   if ( usNoOfParms == 0 )
   {
      usNoOfParms = 1;
      pDefVar = UTLERR_DEFVAR;
      pParmTable = &pDefVar;
   } /* endif */

   DosError(0);
   DosGetMessage( pParmTable, usNoOfParms, pBuffer, sizeof(chMsgBuffer), msgNo,
                  ErrData[UtlGetTask()].chMsgFile, &ulLength );
   DosError(1);
   *(pBuffer+ulLength) = '\0';         // force end of message

   // remove CR/LF, replace C linefeed escape character (backslash-n) with LF
   pMsgText = pBuffer;
   while ( *pMsgText != '\0' )
   {
      switch (*pMsgText)
      {
        case BACKSLASH:
           *pBuffer++ = *pMsgText++;   // move character to target
           // if backslash is followed by 'n' and
           if ( *pMsgText == 'n' && (*(pMsgText+1) == ' ' ||  // blank    /*@N4C*/
                                     *(pMsgText+1) == CR  ||  // car ret  /*@N4A*/
                                     *(pMsgText+1) == LF  ) ) // linefeed /*@N4A*/
           {
              pMsgText++;              // ... skip it and
              *(pBuffer-1) = LF;       // replace backslash with linefeed
           } /* endif */
           break;
        case CR:
           pMsgText++;                 // ignore carriage return
           break;
        case LF:
           *pBuffer++ = ' ';           // replace linefeed with space
           pMsgText++;
           break;
        default:
           *pBuffer++ = *pMsgText++;   // copy character to output buffer
           break;
      } /* endswitch */

   } /* endwhile */

   *pBuffer = '\0';                    // terminate string

} /* end of function UtlGetMsgTxt */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlDosMsgTxt           Get the message text for DOS_ERROR
//------------------------------------------------------------------------------
// Function call:     UtlDosMsgTxt( SHORT sRC, PSZ pParm, PUSHORT pusMBStyle );
//------------------------------------------------------------------------------
// Description:       Get the message number, the parameter string and the
//                    message box style for errors of type DOS_ERROR.
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sRC           Error code of a DOS function
//                    PSZ      pParm         pointer to optional parameter
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusMBStyle    style of message box
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       new message number
//------------------------------------------------------------------------------
// Function flow:     switch sRC
//                      case ERROR_NOT_READY:
//                        message = ERROR_NOT_READY_MSG
//                        style   = MB_RETRYCANCEL
//                        parm    = drive letter
//                      case ERROR_DISK_FULL:
//                      case ERROR_WRITE_FAULT:
//                        message = ERROR_DISK_FULL_MSG
//                        style   = MB_CANCEL
//                        parm    = drive letter
//                      case ERROR_SHARING_VIOLATION:
//                      case ERROR_ACCESS_DENIED:
//                      case ERROR_NETWORK_ACCESS_DENIED:
//                      case ERROR_WRITE_PROTECT:
//                        message = ERROR_ACCESS_DENIED_MSG
//                        style   = MB_RETRYCANCEL
//                        parm    = supplied parameter
//                      case ERROR_DISK_CHANGE:
//                        message = ERROR_DISK_CHANGE_MSG
//                        style   = MB_RETRYCANCEL
//                        parm    = supplied parameter
//                      case ERROR_SECTOR_NOT_FOUND:
//                        message = ERROR_SECTOR_NOT_FOUND_MSG
//                        style   = MB_CANCEL
//                        parm    = supplied parameter
//                      case ERROR_PATH_NOT_FOUND:
//                        message = ERROR_PATH_NOT_FOUND_MSG
//                        style   = MB_CANCEL
//                        parm    = supplied parameter
//                      default:
//                        message = ERROR_GENERAL_DOS_ERROR_MSG
//                        style   = MB_CANCEL
//                        parm    = return code string
//                    endswitch
//------------------------------------------------------------------------------
static SHORT UtlDosMsgTxt
(
   SHORT sRC,                          // return code from Dos call
   PSZ   pParm,                        // parameter string
   PUSHORT pusMBStyle                  // style of message box
)
{
   SHORT  sMsg;                        // new message number

   switch (sRC)
   {
      case ERROR_NOT_READY:
         sMsg = ERROR_NOT_READY_MSG;
         *pusMBStyle = MB_RETRYCANCEL;
         // if full path name given, leave drive letter only
         if ( pParm[0] && (pParm[1] == ':') )
         {
            pParm[2] = EOS;
         } /* endif */
         break;
      case ERROR_DISK_FULL:
         sMsg = ERROR_DISK_FULL_MSG;
         *pusMBStyle = MB_RETRYCANCEL;
         // if full path name given, leave drive letter only
         if ( pParm[0] && (pParm[1] == ':') )
         {
            pParm[2] = EOS;
         } /* endif */
         break;
      case ERROR_WRITE_FAULT:
         sMsg = ERROR_DISK_FULL_MSG;
         *pusMBStyle = MB_CANCEL;
         // if full path name given, leave drive letter only
         if ( pParm[0] && (pParm[1] == ':') )
         {
            pParm[2] = EOS;
         } /* endif */
         break;
      case ERROR_NET_WRITE_FAULT :
         sMsg = ERROR_NETWORK_WRITE_FAULT;
         *pusMBStyle = MB_CANCEL;
         // if full path name given, leave drive letter only
         if ( pParm[0] && (pParm[1] == ':') )
         {
            pParm[2] = EOS;
         } /* endif */
         break;
      case ERROR_INVALID_DRIVE :
      case ERROR_BAD_NETPATH :
      case ERROR_BAD_NET_NAME :
      case ERROR_DEV_NOT_EXIST :
         sMsg = ERROR_DRIVE_NOT_VALID;
         *pusMBStyle = MB_CANCEL;
         // if full path name given, leave drive letter only
         if ( pParm[0] && (pParm[1] == ':') )
         {
            pParm[2] = EOS;
         } /* endif */
         break;
      case ERROR_SHARING_VIOLATION:
      case ERROR_ACCESS_DENIED:
      case ERROR_NETWORK_ACCESS_DENIED:
      case ERROR_WRITE_PROTECT:
      case ERROR_INVALID_PASSWORD:
         sMsg = ERROR_ACCESS_DENIED_MSG;
         *pusMBStyle = MB_RETRYCANCEL;
         break;
      case ERROR_DISK_CHANGE:
         sMsg = ERROR_DISK_CHANGE_MSG;
         // if full path name given, leave drive letter only
         if ( pParm[0] && (pParm[1] == ':') )
         {
            pParm[2] = EOS;
         } /* endif */
         *pusMBStyle = MB_RETRYCANCEL;
         break;
      case ERROR_SECTOR_NOT_FOUND:
         sMsg = ERROR_SECTOR_NOT_FOUND_MSG;
         *pusMBStyle = MB_CANCEL;
         break;
      case ERROR_PATH_NOT_FOUND:
         sMsg = ERROR_PATH_NOT_FOUND_MSG;
         *pusMBStyle = MB_CANCEL;
         sMsg = ERROR_PATH_NOT_FOUND_MSG;
         break;
      case  ERROR_DIRECTORY:
         sMsg = PATH_NOTEXIST_CREATEIT;
         *pusMBStyle = MB_CANCEL;
         break;
      case  ERROR_FILE_EXISTS:
         sMsg = FILE_EXISTS;
         *pusMBStyle = MB_CANCEL;
         break;
      default:
        sMsg = ERROR_GENERAL_DOS_ERROR_MSG;
         *pusMBStyle = MB_CANCEL;
        sprintf( pParm, "%u\n",(USHORT) sRC );
        break;
   } /* endswitch */

   return( sMsg );

} /* end of function UtlDosMsgTxt */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlQdamMsgText        Get the message text for QDAM_ERROR
//------------------------------------------------------------------------------
// Function call:     UtlQdamMsgText  ( USHORT usErr )
//------------------------------------------------------------------------------
// Description:       Get the message number, the parameter string and the
//                    message box style for errors of type QDAM_ERROR.
//------------------------------------------------------------------------------
// Input parameter:   SHORT    usErr         Error code of a NLP function
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       message number
//------------------------------------------------------------------------------
// Function flow:     determine type of error ( >0x8000 then Nlp )
//                    if Nlp-error then
//                      get rid of highbit nibble
//                      switch usErr
//                        case LX_BAD_DICT:
//                           message = EQFS_BAD_DICT;
//                        case LX_BAD_DICT_NME:
//                           message = EQFS_BAD_DICT_NAME;
//                        case LX_FL_ACC_DEND:
//                        case LX_FL_OP_FAILD:
//                           message = EQFS_FILE_ACC_DENIED;
//                        case LX_FL_NO_FND:
//                           message = EQFS_FILE_NOT_FOUND;
//                        case LX_FL_SHR_VIOLTN:
//                           message = EQFS_FILE_SHARING_VIOL;
//                        case LX_MEM_ALLOC_ERR:
//                           message = EQFS_DICT_MEM_ERR;
//                        case TMERR_TOO_MANY_QUERIES:
//                           message = ERROR_TOO_MANY_QUERIES;
//                        case TMERR_PROP_WRITE_ERROR:
//                           message = ERROR_DICT_PROP_WRITE;
//                        case TMERR_PROP_NOT_FOUND:
//                           message = ERROR_DICT_PROP_NOT_FOUND;
//                        case TMERR_PROP_READ_ERROR:
//                           message = ERROR_DICT_PROP_READ;
//                        case TMERR_PROP_EXIST:
//                           message = ERROR_DICT_PROP_EXIST;
//                        case TMERR_SERVER_NOT_STARTED:
//                           message = ERROR_SERVER_NOT_STARTED;
//                        case TMERR_SERVERCODE_NOT_STARTED:
//                           message = ERROR_SERVERCODE_NOT_STARTED;
//                        case TMERR_COMMUNICATION_FAILURE:
//                           message = ERROR_COMMUNICATION_FAILURE;
//                        case TMERR_SERVER_ABOUT_TO_EXIT:
//                           message = ERROR_SERVER_ABOUT_TO_EXIT;
//                        case LANUID_NO_LAN:
//                           message = ERROR_LANUID_NO_LAN;
//                        case LANUID_REQ_NOT_STARTED:
//                           message = ERROR_LANUID_REQ_NOT_STARTED;
//                        case LANUID_USER_NOT_LOG_ON:
//                           message = ERROR_LANUID_USER_NOT_LOG_ON;
//                        case LX_BAD_USR_AREA:
//                        case LX_REPLY_FULL:
//                        case LX_BAD_WORD:
//                        case LX_NO_AID_AVAIL:
//                        default:
//                            message = EQFS_DA_ERROR;
//                           break;
//                      endswitch
//                    else
//                      switch usErr
//                        case LX_DICT_WRT_ASD:
//                          message = EQFS_DICT_WRITE_ERR;
//                        case LX_OTHER_USER_ASD:
//                          message = EQF_DICT_ENTRY_LOCKED;
//                        case LX_PROTECTED_ASD:
//                          message = EQF_DICT_LOCKED;
//                        case LX_INSUF_STOR_ASD:
//                        case LX_MEM_ALLOC_ASD:
//                          message = EQFS_DICT_MEM_ERR;
//                        case LX_MAX_OPEN_ASD:
//                          message = EQFS_TOO_MANY_DICTS;
//                        case LX_OPEN_FLD_ASD:
//                          message = EQFS_FILE_OPEN_FAILED;
//                        case LX_INCOMP_SIG_ASD:
//                          message = EQFS_DICT_ASSOC;
//                        case LX_WRD_NT_FND_ASD:
//                          message = ERROR_SEARCH_ENTRY;
//                        case LX_RENUM_RQD_ASD:
//                          message = ERROR_RENUMBER;
//                        case TMERR_TOO_MANY_QUERIES:
//                          message = ERROR_TOO_MANY_QUERIES;
//                        case TMERR_PROP_WRITE_ERROR:
//                          message = ERROR_DICT_PROP_WRITE;
//                        case TMERR_PROP_NOT_FOUND:
//                          message = ERROR_DICT_PROP_NOT_FOUND;
//                        case TMERR_PROP_READ_ERROR:
//                          message = ERROR_DICT_PROP_READ;
//                        case TMERR_PROP_EXIST:
//                          message = ERROR_DICT_PROP_EXIST;
//                        case TMERR_SERVER_NOT_STARTED:
//                          message = ERROR_SERVER_NOT_STARTED;
//                        case TMERR_SERVERCODE_NOT_STARTED:
//                          message = ERROR_SERVERCODE_NOT_STARTED;
//                        case TMERR_COMMUNICATION_FAILURE:
//                          message = ERROR_COMMUNICATION_FAILURE;
//                        case TMERR_SERVER_ABOUT_TO_EXIT:
//                          message = ERROR_SERVER_ABOUT_TO_EXIT;
//                        case  TMERR_TOO_MANY_OPEN_DATABASES:
//                          message = ERROR_TOO_MANY_OPEN_TMS;
//                        case  TMERR_TOO_MANY_USERS_CONNECTED:
//                          message = ERROR_TOO_MANY_USERS_CONNECTED;
//                        case LANUID_NO_LAN:
//                          message = ERROR_LANUID_NO_LAN;
//                        case LANUID_REQ_NOT_STARTED:
//                          message = ERROR_LANUID_REQ_NOT_STARTED;
//                        case LANUID_USER_NOT_LOG_ON:
//                          message = ERROR_LANUID_USER_NOT_LOG_ON;
//                        case  BTREE_FILE_NOTFOUND:
//                          message = ERROR_DICT_NOT_FOUND;
//                        case  BTREE_INVALID_DRIVE:
//                          message = ERROR_EQF_DRIVE_NOT_ACCESSIBLE;
//                        case  BTREE_OPEN_FAILED:
//                          message = EQFS_FILE_OPEN_FAILED;
//                        case  BTREE_NETWORK_ACCESS_DENIED:
//                          message = ERROR_WRITE_DENIED_MSG;
//                        case LX_BAD_LANG_CODE:
//                           message = EQFRS_NOMORPH_DICT;
//                        case LX_IDX_NT_OPEN_ASD:
//                        case LX_ASC_NT_ALLWD_ASD:
//                        case LX_DATA_2_LRG_ASD:
//                        case LX_UNEXPECTED_ASD:
//                        case LX_UNINIT_PRM_ASD:
//                        case LX_WRD_EXISTS_ASD:
//                        case LX_FLE_OPEN_ASD:
//                        default:
//                          message = EQFS_DICT_GENERAL_ERROR;
//                      endswitch
//                    endif
//                    return error id
//------------------------------------------------------------------------------
SHORT  UtlQdamMsgTxt
(
   SHORT usErr                        // return code from Nlp call
)
{
  USHORT        usErrID;

  /********************************************************************/
  /* convert MORPH errors to our LX errors                            */
  /********************************************************************/
  if ((usErr >= ERR_MORPH_BASE) && (usErr <= ERR_MORPH_END) )
  {
    switch ( usErr )
    {
      case MORPH_INV_LANG_ID :        usErr = LX_BAD_LANG_CODE;  break;
      case MORPH_NO_MEMORY :          usErr = LX_MEM_ALLOC_ASD;  break;
      case MORPH_NO_LANG_PROPS :      usErr = LX_BAD_LANG_CODE;  break;
      case MORPH_ACT_DICT_FAILED :    usErr = LX_BAD_LANG_CODE;  break;
    } /* endswitch */
  } /* endif */

  if ( usErr & 0x8000 )        // it's an NLP rc
  {
    usErrID = EQFS_DA_ERROR;
  }
  else
  {
     switch (usErr)
     {
        case LX_IDX_FN_ERR:
           usErrID = ERROR_DISK_FULL_MSG;
           break;
        case LX_DICT_WRT_ASD:
           usErrID = EQFS_DICT_WRITE_ERR;
           break;
        case LX_OTHER_USER_ASD:             // entry is locked
           usErrID = EQF_DICT_ENTRY_LOCKED;
           break;
        case LX_PROTECTED_ASD:              // dictionary locked
           usErrID = EQF_DICT_LOCKED;
           break;
        case LX_INSUF_STOR_ASD:
        case LX_MEM_ALLOC_ASD:
           usErrID = ERROR_STORAGE;
           break;
        case LX_MAX_OPEN_ASD:
           usErrID = EQFS_TOO_MANY_DICTS;
           break;
        case LX_OPEN_FLD_ASD:
           usErrID = EQFS_FILE_OPEN_FAILED;
           break;
        case LX_INCOMP_SIG_ASD:
           usErrID = EQFS_DICT_ASSOC;
           break;
        case LX_WRD_NT_FND_ASD:
        case LX_WRD_EXISTS_ASD:
           usErrID = ERROR_SEARCH_ENTRY;
           break;
        case LX_RENUM_RQD_ASD:
           usErrID = ERROR_RENUMBER;
           break;
        case  BTREE_FILE_NOTFOUND:
          usErrID = ERROR_DICT_NOT_FOUND;
          break;
        case  BTREE_INVALID_DRIVE:
          usErrID = ERROR_EQF_DRIVE_NOT_ACCESSIBLE;
          break;
        case  BTREE_OPEN_FAILED:
          usErrID = EQFS_FILE_OPEN_FAILED;
          break;
        case  BTREE_NETWORK_ACCESS_DENIED:
          usErrID = ERROR_ACCESS_DENIED_MSG;
          break;
        case  BTREE_ACCESS_ERROR:
          usErrID = ERROR_WRITE_DENIED_MSG;
          break;
        case LX_BAD_LANG_CODE:
           usErrID = EQFRS_NOMORPH_DICT;
           break;
        case BTREE_IN_USE:
           usErrID = ERROR_DICT_LOCKED;
           break;
        case LX_IDX_NT_OPEN_ASD:
        case LX_ASC_NT_ALLWD_ASD:
        case LX_DATA_2_LRG_ASD:
        case LX_UNEXPECTED_ASD:
        case LX_UNINIT_PRM_ASD:
        case LX_FLE_OPEN_ASD:
        default:
           usErrID = EQFS_DICT_GENERAL_ERROR;
           break;
     } // endswitch
  } /* endif */

  return (usErrID);
} // end 'UtlQdamMsgText'


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlQldbMsgText        Get the message text for QLDB_ERROR
//------------------------------------------------------------------------------
// Function call:     UtlQldbMsgText  ( USHORT usErr )
//------------------------------------------------------------------------------
// Description:       Get the message number, the parameter string and the
//                    message box style for errors of type QLDB_ERROR.
//------------------------------------------------------------------------------
// Input parameter:   SHORT    usErr         Error code of a     function
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       message number
//------------------------------------------------------------------------------
// Function flow: switch usErr
//                  case QLDB_NO_MEMORY:
//                     usErrID = 1;
//                     break;
//                  case QLDB_ERR_DEALLOC:
//                     usErrID = 1;
//                     break;
//                  case QLDB_NO_TREEHANDLE:
//                     usErrID = EQFS_FILE_SHARING_VIOL;
//                     break;
//                  case QLDB_TOO_MANY_FIELDS:
//                     usErrID = EQFS_FILE_OPEN_FAILED;
//                     break;
//                  case QLDB_NO_VALID_DATA:
//                     usErrID = EQFS_FILE_OPEN_FAILED;
//                     break;
//                  case QLDB_INVALID_LEVEL:
//                     usErrID = EQFS_FILE_OPEN_FAILED;
//                     break;
//                  case QLDB_NO_NODE:
//                     usErrID = EQFS_FILE_OPEN_FAILED;
//                     break;
//                  case QLDB_INVALID_FIELDNO:
//                     usErrID = EQFS_FILE_OPEN_FAILED;
//                     break;
//                  case QLDB_ERROR_IN_TREE:
//                     usErrID = EQFS_FILE_OPEN_FAILED;
//                     break;
//                  case QLDB_REC_TOO_LONG:
//                     usErrID = EQFS_FILE_OPEN_FAILED;
//                     break;
//                  default:
//                     usErrID = EQFS_DA_ERROR;
//                endswitch
//                return error id
//------------------------------------------------------------------------------
static SHORT  UtlQldbMsgTxt
(
   SHORT usErr                        // return code from Nlp call
)
{
  USHORT        usErrID;

     switch (usErr)
     {
        case QLDB_NO_MEMORY:
           usErrID = 1;
           break;
        case QLDB_ERR_DEALLOC:
           usErrID = 1;
           break;
        case QLDB_NO_TREEHANDLE:
           usErrID = EQFS_FILE_SHARING_VIOL;
           break;
        case QLDB_TOO_MANY_FIELDS:
           usErrID = EQFS_FILE_OPEN_FAILED;
           break;
        case QLDB_NO_VALID_DATA:
           usErrID = EQFS_FILE_OPEN_FAILED;
           break;
        case QLDB_INVALID_LEVEL:
           usErrID = EQFS_FILE_OPEN_FAILED;
           break;
        case QLDB_NO_NODE:
           usErrID = EQFS_FILE_OPEN_FAILED;
           break;
        case QLDB_INVALID_FIELDNO:
           usErrID = EQFS_FILE_OPEN_FAILED;
           break;
        case QLDB_ERROR_IN_TREE:
           usErrID = EQFS_FILE_OPEN_FAILED;
           break;
        case QLDB_REC_TOO_LONG:
           usErrID = EQFS_FILE_OPEN_FAILED;
           break;
        default:
           usErrID = EQFS_DA_ERROR;
           break;
     }                                 // endswitch

  return (usErrID);
}                                      // end 'UtlQldbMsgText'

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlPropMsgText        Get the message text for PROP_ERROR
//------------------------------------------------------------------------------
// Function call:     UtlPropMsgText  ( USHORT usErr )
//------------------------------------------------------------------------------
// Description:       Get the message number, the parameter string and the
//                    message box style for errors of type PROP_ERROR.
//------------------------------------------------------------------------------
// Input parameter:   SHORT    usErr         Error code of a     function
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       message number
//------------------------------------------------------------------------------
// Function flow: switch usErr
//                  case ErrProp_AccessDenied:
//                     usErrID = ERROR_PROPERTY_ACCESS;
//                  case ErrProp_AlreadyExists:
//                     usErrID = ERROR_PROP_ALREADYEXISTS;
//                  case ErrProp_InvalidClass:
//                     usErrID = ERROR_PROP_INVALIDCLASS;
//                  case ErrProp_InvalidFile:
//                     usErrID = ERROR_PROP_INVALIDFILE;
//                  case ErrProp_ObjectBusy:
//                     usErrID = ERROR_PROP_OBJECTBUSY;
//                   case  Err_NoStorage:
//                     usErrID = ERROR_STORAGE;
//                   case Err_ReadFile:
//                   case Err_OpenFile :
//                     usErrID = ERROR_GENERAL_DOS_ERROR_MSG;
//                  case ErrProp_AlreadyDefined:
//                  case ErrProp_InvalidEntry:
//                  case ErrProp_InvalidName:
//                  case ErrProp_InvalidObjectName:
//                  case ErrProp_InvalidType:
//                  case ErrProp_NoEnvironment:
//                  case ErrProp_NoName:
//                  case ErrProp_NoSystem:
//                  case ErrProp_NotAuthorized:
//                  case ErrProp_NotFound:
//                  case ErrProp_SystemInvalid:
//                  case ErrProp_InvalidAccess:
//                  case ErrProp_MissingBuffer:
//                  case ErrProp_NotLoaded:
//                  case ErrProp_InvalidParms:
//                  case ErrProp_InvalidHandle:
//                  case ErrProp_InvalidData:
//                  default:
//                     usErrID = ERROR_PROP_DEFAULT;
//                endswitch
//                return error id
//------------------------------------------------------------------------------
static SHORT  UtlPropMsgTxt
(
   SHORT usErr                        // return code from Nlp call
)
{
  USHORT        usErrID;

     switch (usErr)
     {
        case ErrProp_AccessDenied:
           usErrID = ERROR_PROPERTY_ACCESS;
           break;
        case ErrProp_AlreadyExists:
           usErrID = ERROR_PROP_ALREADYEXISTS;
           break;
        case ErrProp_InvalidClass:
           usErrID = ERROR_PROP_INVALIDCLASS;
           break;
        case ErrProp_InvalidFile:
           usErrID = ERROR_PROP_INVALIDFILE;
           break;
        case ErrProp_ObjectBusy:
           usErrID = ERROR_PROP_OBJECTBUSY;
           break;
         case  Err_NoStorage:
           usErrID = ERROR_STORAGE;
           break;
         case Err_ReadFile:
         case Err_OpenFile :
           usErrID = ERROR_GENERAL_PROP_ERROR_MSG;
           break;
        case ErrProp_AlreadyDefined:
        case ErrProp_InvalidEntry:
        case ErrProp_InvalidName:
        case ErrProp_InvalidObjectName:
        case ErrProp_InvalidType:
        case ErrProp_NoEnvironment:
        case ErrProp_NoName:
        case ErrProp_NoSystem:
        case ErrProp_NotAuthorized:
        case ErrProp_NotFound:
        case ErrProp_SystemInvalid:
        case ErrProp_InvalidAccess:
        case ErrProp_MissingBuffer:
        case ErrProp_NotLoaded:
        case ErrProp_InvalidParms:
        case ErrProp_InvalidHandle:
        case ErrProp_InvalidData:
        default:
           usErrID = ERROR_PROP_DEFAULT;
           break;
     }                                 // endswitch

  return (usErrID);
}                                      // end 'UtlPropMsgText'

//------------------------------------------------------------------------------
// External Function
//------------------------------------------------------------------------------
// Function Name:     UtlInitError           Initialization for UtlError calls
//------------------------------------------------------------------------------
// Description:       Initializes error handler, This function MUST be called
//                    before any call to UtlError is done!
//------------------------------------------------------------------------------
// Function Call:     VOID UtlInitError( HAB     hab,
//                                       HWND    hwndFrame,
//                                       HWND    hwndHelpInstance,
//                                       PSZ     pMsgFile );
//------------------------------------------------------------------------------
// Parameters:        - hab is the process anchor block handle
//                    - hwndFrame is the frame window handle of the main window
//                    - hwndHelpInstance is the window handle of the
//                      help instance. Use NULL if no help has been associated
//                      yet.
//                    - pMsgFile points to the fully qualified name of the
//                      message file
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     Store supplied values in global ErrData structure
//------------------------------------------------------------------------------
VOID UtlInitError
(
   HAB  hab,                           // anchor block handle
   HWND hwndFrame,                     // handle of main window frame
   HWND hwndHelpInstance,              // help instance handle
   PSZ  pMsgFile                       // name of message file
)
{
   // check for function mode I/F (in this mode we use the help instance
   // handle as pointer to the last message area
   if ( hwndFrame == HWND_FUNCIF )
   {
     ErrData[UtlGetTask()].hab              = hab;
     ErrData[UtlGetTask()].hwndFrame        = hwndFrame;
     ErrData[UtlGetTask()].hwndHelpInstance = hwndHelpInstance;
     strcpy( ErrData[UtlGetTask()].chMsgFile, pMsgFile );
     ErrData[UtlGetTask()].pLastMessage     = (PDDEMSGBUFFER)hwndHelpInstance;
   }
   else
   {
     ErrData[UtlGetTask()].hab              = hab;
     ErrData[UtlGetTask()].hwndFrame        = hwndFrame;
     ErrData[UtlGetTask()].hwndHelpInstance = hwndHelpInstance;
     strcpy( ErrData[UtlGetTask()].chMsgFile, pMsgFile );
     ErrData[UtlGetTask()].pLastMessage     = NULL;
   } /* endif */


      strcpy(SYSTEM_ERROR_TXT   , "OpenTM2");
      strcpy(QDAM_ERROR_TXT     , "OpenTM2");
      strcpy(QLDB_ERROR_TXT     , "OpenTM2");
      strcpy(PROP_ERROR_TXT     , "OpenTM2");
      strcpy(EQF_ERROR_TXT      , "OpenTM2");
      strcpy(EQF_CMD_ERROR_TXT  , "OpenTM2 Batch" );
      strcpy(EQF_WARNING_TXT    , "OpenTM2");
      strcpy(EQF_QUERY_TXT      , "OpenTM2");
      strcpy(EQF_INFO_TXT       , "OpenTM2");
      strcpy(INTERNAL_ERROR_TXT , "OpenTM2");

   if ( ErrData[UtlGetTask()].hMsgFile )
   {

     UtlClose( ErrData[UtlGetTask()].hMsgFile, FALSE );
     ErrData[UtlGetTask()].hMsgFile = 0;
   } /* endif */
   {
     USHORT  usDummy;
     UtlOpen(pMsgFile, &ErrData[UtlGetTask()].hMsgFile , &usDummy,
              0L,                            // Create file size
              FILE_NORMAL,                   // Normal attribute
              OPEN_ACTION_OPEN_IF_EXISTS,    // Open if exists
              OPEN_SHARE_DENYWRITE,          // Deny Write access
              0L,                            // Reserved
              FALSE );                           // no error handling
   }
}

//------------------------------------------------------------------------------
// External Function
//------------------------------------------------------------------------------
// Function Name:     UtlTerminateError     Terminate UtlError
//------------------------------------------------------------------------------
// Description:       closes any open file handles ...
//------------------------------------------------------------------------------
// Function Call:     VOID UtlTerminateError()
//------------------------------------------------------------------------------
// Parameters:        VOID
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     close error file and init the structure
//------------------------------------------------------------------------------
VOID UtlTerminateError ( )
{
   if ( ! ErrData[UtlGetTask()].hMsgFile )
   {
     UtlClose( ErrData[UtlGetTask()].hMsgFile, FALSE );
   } /* endif */
   memset(&ErrData[UtlGetTask()], 0, sizeof(ERRDATA));
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlGetLastError
//------------------------------------------------------------------------------
// Function call:     UtlGetLastError ( PSZ )
//------------------------------------------------------------------------------
// Description:
//------------------------------------------------------------------------------
// Returncode type:
//------------------------------------------------------------------------------
// Function flow:
//
//------------------------------------------------------------------------------
VOID UtlGetLastError( HWND hwndDDE, PSZ pszMsgText)
{
  // look for given handle in our DDE message table
  PDDEMSGBUFFER pBuf = DDEMessages;
  int i;
  for ( i = 0; i < MAXNUMOFDDEMESSAGES; i++, pBuf++ )
  {
   if ( pBuf->hwndDDE == hwndDDE )
   {
     break;                        // found the handle in our table
   } /* endif */
  } /* endfor */
  if ( i < MAXNUMOFDDEMESSAGES )    // found the handle
  {
    pBuf->hwndDDE     = (HWND)0;
    pBuf->sErrNumber  = 0;
    strcpy( pszMsgText, pBuf->chMsgText );
    pBuf->chMsgText[0] = EOS;
  }
  else                             // handle not found, use default buffer
  {
    strcpy (pszMsgText, chDDEMsgBuffer);
    chDDEMsgBuffer[0] = EOS;
  } /* endif */
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlGetLastError
//------------------------------------------------------------------------------
// Function call:     UtlGetLastError ( PSZ )
//------------------------------------------------------------------------------
// Description:
//------------------------------------------------------------------------------
// Returncode type:
//------------------------------------------------------------------------------
// Function flow:
//
//------------------------------------------------------------------------------
USHORT UtlGetDDEErrorCode( HWND hwndDDE )
{
  USHORT usRC = NO_ERROR;

  // look for given handle in our DDE message table
  if ( hwndDDE == HWND_FUNCIF )
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
  }
  else
  {
      PDDEMSGBUFFER pBuf = DDEMessages;
      int i;
      for ( i = 0; i < MAXNUMOFDDEMESSAGES; i++, pBuf++ )
      {
       if ( pBuf->hwndDDE == hwndDDE )
       {
         break;                        // found the handle in our table
       } /* endif */
      } /* endfor */
      if ( i < MAXNUMOFDDEMESSAGES )    // found the handle
      {
        usRC = pBuf->sErrNumber;
      } /* endif */
  } /* endif */
  return( usRC );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlGetWinError
//------------------------------------------------------------------------------
// Function call:     UtlGetWinError( HRESULT, PSZ )
//------------------------------------------------------------------------------
// Description:  Changes the HRESULT error code into a readable tag
//               (for some returncodes only),
//               especially for OLE/RPC Codes, it may be extended
//------------------------------------------------------------------------------
// Returncode type:  VOID
//------------------------------------------------------------------------------
// Function flow:
//
//------------------------------------------------------------------------------
VOID UtlGetWinError( HRESULT hResult, PSZ pszErrString )
{
  typedef struct _WINERRTAGPARRAY
  {
     HRESULT ErrCode;
     CHAR  ErrTag[50];
  } WINERRTAGPARRAY;

  WINERRTAGPARRAY WinErrTagArray[] =
  {
      { 0x80010101L, "RPC_E_OUT_OF_RESOURCES" },
      { 0x80010102L, "RPC_E_ATTEMPTED_MULTITHREAD" },
      { 0x80010103L, "RPC_E_NOT_REGISTERED" },
      { 0x80010104L, "RPC_E_FAULT" },
      { 0x80010105L, "RPC_E_SERVERFAULT" },
      { 0x80010106L, "RPC_E_CHANGED_MODE" },
      { 0x80010107L, "RPC_E_INVALIDMETHOD" },
      { 0x80010108L, "RPC_E_DISCONNECTED" },
      { 0x80010109L, "RPC_E_RETRY" },
      { 0x8001010AL, "RPC_E_SERVERCALL_RETRYLATER" },
      { 0x8001010BL, "RPC_E_SERVERCALL_REJECTED" },
      { 0x8001010CL, "RPC_E_INVALID_CALLDATA" },
      { 0x8001010DL, "RPC_E_CANTCALLOUT_ININPUTSYNCCALL" },
      { 0x8001010EL, "RPC_E_WRONG_THREAD" },
      { 0x8001010FL, "RPC_E_THREAD_NOT_INIT" },
      { 0x0L, "" }
   };

   short sI;

   for (sI = 0; WinErrTagArray[sI].ErrCode != 0; sI ++)
   {
       if (WinErrTagArray[sI].ErrCode == hResult)
       {
            sprintf(pszErrString, "%s (0x%08lx)", WinErrTagArray[sI].ErrTag,
                WinErrTagArray[sI].ErrCode);
            break;
       }
   }

   if (WinErrTagArray[sI].ErrCode == 0)
   {
        // sorry we have no error text
        sprintf(pszErrString, "0x%08lx", WinErrTagArray[sI].ErrCode);
   }
}

BOOL UtlEvent
(
  SHORT sEventType,                    // type of the event
  SHORT sEventID,                      // the event identifier
  SHORT sEventClass,                   // the event class
  SHORT sEventRC                       // the event return code or error code
)
{
  return( UtlEvent2( sEventType, sEventID, sEventClass, sEventRC, NONE_GROUP,
                     NULL ) );
}
BOOL UtlEvent2
(
  SHORT sEventType,                    // type of the event
  SHORT sEventID,                      // the event identifier
  SHORT sEventClass,                   // the event class
  SHORT sEventRC,                      // the event return code or error code
  SHORT sGroup,                        // the event group
  PSZ   pszAddData                     // additional data for event (string)
)
{
  PEVENTENTRY2 pEvent;
  BOOL fLogGroup = FALSE;              // TRUE = logging for group is active

  //     increment a event position and wrap around at end of table
  #define INCREVENTPOS( pTable, pos ) \
     { pos++; if ( pos >= pTable->sListSize ) pos = 0; }


  // check if debug group of event is currently logged
  if ( sGroup == GENERAL_GROUP)
  {
    fLogGroup = TRUE;
  }
  else
  {
    PSHORT psLogGroup = asLogGroups;
    while ( (*psLogGroup != -1 ) && (*psLogGroup != sGroup ) )
    {
      psLogGroup++;
    } /* end while */
    if ( *psLogGroup == sGroup )
    {
      fLogGroup = TRUE;
    } /* end if */
  }

  // Setup pointer to current entry in event list
  if ( pEventTable && fLogGroup )
  {
    if ( (pEventTable->sFirstEvent == -1) && (pEventTable->sLastEvent == -1) )
    {
      // list is empty
      pEventTable->sFirstEvent = 0;
      pEventTable->sLastEvent  = 0;
    }
    else
    {
      INCREVENTPOS( pEventTable, pEventTable->sLastEvent );
      if ( pEventTable->sLastEvent == pEventTable->sFirstEvent )
      {
        INCREVENTPOS( pEventTable, pEventTable->sFirstEvent );
      } /* endif */
    } /* endif */
    pEvent = &(pEventTable->Event[pEventTable->sLastEvent]);

    // Fill current event
    time( &(pEvent->lTime) );
    pEvent->cType    = (CHAR)sEventType;
    pEvent->sEventId = sEventID;
    pEvent->sClass   = sEventClass;
    pEvent->sGroup   = sGroup;
    pEvent->sRC      = sEventRC;
    if ( (pszAddData != NULL) && (*pszAddData != EOS) )
    {
      pEvent->cRecType = EVENTRECEXTENDED;

      // position to next event entry
      INCREVENTPOS( pEventTable, pEventTable->sLastEvent );
      if ( pEventTable->sLastEvent == pEventTable->sFirstEvent )
      {
        INCREVENTPOS( pEventTable, pEventTable->sFirstEvent );
      } /* endif */

      // add string data in next event entry
      pEvent = &(pEventTable->Event[pEventTable->sLastEvent]);
      {
        PSZ pszData = (PSZ)pEvent;

        pEvent->cRecType = EVENTRECSTRING;
        strncpy( pszData+1, pszAddData, sizeof(EVENTENTRY2) - 1 );
        pszData[sizeof(EVENTENTRY2)-1] = EOS;
      }

    }
    else
    {
      pEvent->cRecType = EVENTRECNORMAL;
    } /* endif */

    // for specifie events: write event list anyway
    if ( (sEventID == TWBSTART_LOC )    ||
         (sEventID == TWBEXCEPTION_LOC) ||
         (sEventType == CHECKSUM_EVENT) )
    {
      CHAR szLogFile[MAX_EQF_PATH];
      GetStringFromRegistry( APPL_Name, KEY_Drive, szLogFile, sizeof( szLogFile ), "" );
      strcat( szLogFile, "\\OTM\\PROPERTY\\" );
      strcat( szLogFile, EVENTLOGFILENAME );


      // Write event log file
      UtlWriteFile( szLogFile, sizeof(EVENTTABLE), pEventTable, FALSE );
    } /* endif */
  } /* endif */

  return( TRUE );
} /* end of function UtlEvent */

/*! UtlAddEventGroup     Adds a group to be logged
	Description:       Adds a group to the list of events being logged
	Function Call:     UtlAddEventGroup( SHORT sGroup );
	Parameters:        - sGroup is the event group to be added
	Prerequesits:      UtlInitUtils must have been called before
	Returncode type:   BOOL
	Returncodes:       TRUE (always)
*/
BOOL UtlAddEventGroup
(
  SHORT sGroup                       // the event group
)
{
  PSHORT psLogGroup = asLogGroups;
  int    iMaxGroups = (sizeof(asLogGroups) / sizeof(SHORT)) - 1;

  // look for end of current event group list
  while ( (iMaxGroups > 0) && (*psLogGroup != -1 ) )
  {
    psLogGroup++;
    iMaxGroups--;
  } /* end while */

  // add new group at end of list
  if ( iMaxGroups > 0 )
  {
    *psLogGroup = sGroup;
  } /* end if */

  // terminate group list
  if ( iMaxGroups > 0 )
  {
    psLogGroup++;
    *psLogGroup = -1;
  } /* end if */

  // return to caller
  return( TRUE );
} /* end of function UtlAddEventGroup */

//+----------------------------------------------------------------------------+
// Internal function                                                            
//+----------------------------------------------------------------------------+
// Function name: UTLYESTOALLDLG                                                
//+----------------------------------------------------------------------------+
// Function call:                                                               
//   MRESULT EXPENTRY UTLYESTOALLDLG( hwnd, msg, mp1, mp2 )                     
//+----------------------------------------------------------------------------+
// Description:                                                                 
//   This dialog procedure simulates a yes-to-all message box                   
//+----------------------------------------------------------------------------+
// Input parameter:                                                             
//   HWND    hwnd     handle of window                                          
//   USHORT  msg      type of message                                           
//   MPARAM  mp1      first message parameter                                   
//   MPARAM  mp2      second message parameter                                  
//+----------------------------------------------------------------------------+
// Output parameter:                                                            
//   none                                                                       
//+----------------------------------------------------------------------------+
// Returncode type: MRESULT                                                     
//+----------------------------------------------------------------------------+
// Returncodes:                                                                 
//   TRUE                                                                       
//   FALSE                                                                      
//+----------------------------------------------------------------------------+
// Prerequesits:                                                                
//   - the function expects to receive in the WM_INITDLG message the            
//     pointer to the dialog IDA                                                
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK UTLYESTOALLDLG
(
HWND   hwnd,                        // handle of window
WINMSG msg,                         // type of message
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/

  MRESULT              mResult = FALSE; // dlg procedure return value
  PYESTOALL_IDA        pIDA;            // dialog IDA

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIDA = ACCESSDLGIDA( hwnd, PYESTOALL_IDA );
      HANDLEQUERYID( pIDA->sID, mp2 );
      break;

    case WM_INITDLG :
      {
        /************************************************************/
        /* save the pointer to the IDA in the dialog reserved memory*/
        /************************************************************/
        pIDA = (PYESTOALL_IDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pIDA );

        SETTEXT( hwnd, ID_UTL_YESTOALL_MSG_TXT, pIDA->pszText );
        SETTEXTHWND( hwnd, pIDA->pszTitle );
        SETWINDOWID( hwnd, pIDA->sID );

        /**********************************************************/
        /* Keep dialog within TWB                                 */
        /**********************************************************/
        {
          SWP  swpDlg, swpTWB;

          /********************************************************/
          /* Get dialog size/position                             */
          /********************************************************/
          WinQueryWindowPos( hwnd, &swpDlg );

          /********************************************************/
          /* Center dialog within TWB                             */
          /********************************************************/
          WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
          if ( (swpDlg.x > 0) && ((swpDlg.x + swpDlg.cx) < swpTWB.cx) )
          {
            swpDlg.x = (swpTWB.cx - swpDlg.cx) / 2;
          } /* endif */
          if ( (swpDlg.y > 0) && ((swpDlg.y + swpDlg.cy) < swpTWB.cy) )
          {
            swpDlg.y = (swpTWB.cy - swpDlg.cy) / 2;
          } /* endif */

          WinSetWindowPos( hwnd, HWND_TOP,
                           swpDlg.x, swpDlg.y, swpDlg.cx, swpDlg.cy,
                           EQF_SWP_MOVE |
                           EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
        }

        /**************************************************************/
        /* Set focus to default button                                */
        /**************************************************************/
        if ( pIDA->usStyle & MB_DEFBUTTON2 )
        {
          SETFOCUS( hwnd, ID_UTL_YESTOALL_YESTOALL_PB );
          SendDlgItemMessage( hwnd, ID_UTL_YESTOALL_YESTOALL_PB, BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON,
                              MAKELPARAM( TRUE, 0 ) );
        }
        else if ( pIDA->usStyle & MB_DEFBUTTON3 )
        {
          SETFOCUS( hwnd, ID_UTL_YESTOALL_NO_PB );
          SendDlgItemMessage( hwnd, ID_UTL_YESTOALL_NO_PB, BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON,
                              MAKELPARAM( TRUE, 0 ) );
       }
        else
        {
          SETFOCUS( hwnd, ID_UTL_YESTOALL_YES_PB );
          SendDlgItemMessage( hwnd, ID_UTL_YESTOALL_YES_PB, BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON,
                              MAKELPARAM( TRUE, 0 ) );
        } /* endif */

        // leave the focus where we put it
        mResult = MRFROMSHORT(DIALOGINITRETURN(TRUE));
      }
      break;

    case WM_COMMAND :
      {
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_UTL_YESTOALL_YES_PB :
            WinDismissDlg( hwnd, MBID_YES );
            break;

          case ID_UTL_YESTOALL_YESTOALL_PB :
            WinDismissDlg( hwnd, MBID_EQF_YESTOALL );
            break;

          case ID_UTL_YESTOALL_NO_PB :
            WinDismissDlg( hwnd, MBID_NO );
            break;

          case ID_UTL_YESTOALL_CANCEL_PB :
          case DID_CANCEL :
            WinDismissDlg( hwnd, MBID_CANCEL );
            break;
          case ID_UTL_YESTOALL_HELP_PB:
            UtlInvokeHelp();
            break;
        } /* endswitch */
      }
      break;

    default :
      {
        mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      }
      break;
  } /* endswitch */

  return( mResult );

} /* end of function UTLYESTOALLDLG */
