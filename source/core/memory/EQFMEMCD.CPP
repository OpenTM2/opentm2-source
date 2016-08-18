//+----------------------------------------------------------------------------+
//|  EQFMEMCD.C - TM create dialog and TM properties dialog                    |
//+----------------------------------------------------------------------------+
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Author                                   :   Markus Conrad                |
//|  Code for creation of remote TMs added by :   Stefan Doersam               |
//+----------------------------------------------------------------------------+
//|  Description  : This program provides the end user dialog                  |
//|                 and the function to create a translation memory            |
//|                 and the new TM property dialog                             |
//+----------------------------------------------------------------------------+
//|  Entry Points : MEMCREATEDLG( HWND, USHORT, MPARAM, MPARAM )               |
//|                 MEMPROPDLG( HWND, USHORT, MPARAM, MPARAM )                 |
//+----------------------------------------------------------------------------+
//|  Externals    :                                                            |
//+----------------------------------------------------------------------------+
//|  Internals    : MemCreateCommand                                           |
//|                 MemCreateControl                                           |
//|                 UpdateLastUsedProp                                         |
//|                 PrepTmCreate                                               |
//|                 HandleOtherControlMsgs                                     |
//|                 SetupDriveIcons                                            |
//|                 DeleteDriveIcons                                           |
//|                 DisplayDriveIcons                                          |
//|                 DisplayServerDriveIcons                                    |
//|                 ProcessWM_COMMAND                                          |
//|                 InitMemCreateDlg                                           |
//|                 SelectLocalRB                                              |
//|                 iCompare                                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "MemoryFactory.h"


#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMEM.ID>              // Translation Memory IDs
// #include <TIME.H>                 // C time functions
#include <EQFQDAM.H>            // Low level TM access functions
#include <OTMFUNC.H>              // function call I/F

// the name of the default memory plugin
#define DEFAULT_MEMORY_PLUGIN "EqfMemoryPlugin" 

/**********************************************************************/
/* IDA structure for TM Create Dialog                                 */
/**********************************************************************/
typedef struct _TMX_CREATE_IDA
{
  PSZ    pszParms;                         // ptr to input parameter, will receive new memory name on exit
  CHAR   szMemName[MAX_LONGFILESPEC];      // TM name as entered by user
  CHAR   szSourceLang[MAX_LANG_LENGTH];    //language name
  CHAR   szPlugin[MAX_LONGFILESPEC];       // name of selected plugin
  std::vector<OtmMemoryPlugin *> vMemoryPlugins; // list of available memory plugins
  std::vector<OtmSharedMemoryPlugin *> vSharedMemoryPlugins; // list of available memory plugins for shared memories
  CHAR   szUserid[MAX_USERID];             //LAN Userid of TM: if local '\0'
  OtmPlugin *pSelectedPlugin;              // pointer to selected plugin
  CHAR   szDriveList[MAX_DRIVELIST];       // list of drives to be displayed 
  CHAR   szLastDriveList[MAX_DRIVELIST];   // list of currently displayed drives

  CHAR   szMemPath[MAX_EQF_PATH];          //D:\EQF\MEM\, TM path
  CHAR   szMemShortName[MAX_LONGFILESPEC]; // TM short name
//CHAR   szMemPathMemName[MAX_EQF_PATH];   //D:\EQF\MEM\TESTTM concat of szMemPath,szMemName
  CHAR   szFullMemName[MAX_EQF_PATH];      //D:\EQF\MEM\TESTTM.TMD
  CHAR   szMemPropPath[MAX_EQF_PATH];      //D:\EQF
  CHAR   szMemPropName[MAX_FILESPEC];      //TESTTM.MEM, TM property name
  CHAR   szServer[MAX_SERVER_NAME];        //server Name of TM or \0 if TM is local
  CHAR   szMemDesc[MAX_MEM_DESCRIPTION];   //description of memory database
  CHAR   szTemp[MAX_PATH144];              //temp work string
  CHAR   szTemp2[MAX_PATH144];             //temp work string
  USHORT usCreate;                         //if FALSE the TM create dialog failed
  USHORT usLocation;                       //TM_LOCAL,TM_REMOTE, TM_LOCALREMOTE,
} TMX_CREATE_IDA, * PTMX_CREATE_IDA;


VOID    MemCreateCommand( HWND hDlg );
VOID    MemCreateControl( HWND hDlg, USHORT usId );
VOID    UpdateLastUsedProp( PTMX_CREATE_IDA );
static VOID HandleOtherControlMsgs( HWND, USHORT, SHORT, SHORT );
static VOID SetupDriveIcons( HWND, USHORT, PSZ );
static VOID DeleteDriveIcons( HWND, PSZ );
static VOID DisplayDriveIcons( HWND, PSZ );
static VOID ProcessWM_COMMAND( HWND, WPARAM, LPARAM );
static VOID InitMemCreateDlg( HWND );
static VOID SelectLocalRB( HWND, PTMX_CREATE_IDA );
static VOID SelectSharedRB( HWND, PTMX_CREATE_IDA );
static VOID SortString( PSZ );
static int iCompare( const void *, const void *);

BOOL MemCreatePropsFromDlgData
(
PTMX_CREATE_IDA pCIDA,               // Pointer to create IDA
HWND            hwnd                 // window handle for error messages
);

extern HELPSUBTABLE pid_memcr_subtable[];
extern HELPSUBTABLE hlpsubtblTMPropDlg[];

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MEMCREATEDLG                                             |
//+----------------------------------------------------------------------------+
//|Function call:     MRESULT                                                  |
//|                   EXPENTRY MEMCREATEDLG( HWND      hDlg,                   |
//|                                          USHORT    usMessage,              |
//|                                          WPARAM    mp1,                    |
//|                                          LPARAM    mp2 )                   |
//+----------------------------------------------------------------------------+
//|Description:       TM Create dialog procedure                               |
//+----------------------------------------------------------------------------+
//|Parameters:        hDlg      -  dialog handle                               |
//|                   usMessage -  PM message                                  |
//|                   mp1       -  message parameter 1                         |
//|                   mp2       -  message parameter 1                         |
//|                                contains the pointer to TMX_CREATE_IDA      |
//|                                the TMX_CREATE_IDA is allocated and freed   |
//|                                by the TM handler (EQFMEM.C)                |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      - TMX_CREATE_IDA may contain a TM name in szMemName      |
//|                     which will be diaplyed in the TM name entry field      |
//|                   - TMX_CREATE_IDA contains teh parameter usCreate         |
//|                     which indicates if the TM is created (TRUE)            |
//|                     or nor (FALSE), so that the TM handler can handle      |
//|                     this correctly after return of the dialog procedure.   |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK MEMCREATEDLG( HWND      hDlg,
                               WINMSG    usMessage,
                               WPARAM    mp1,
                               LPARAM    mp2 )

//   This function indicates in usCreate if the translation memory was created
//   sucessfully (TRUE) ur unsucessfully (FALSE)
//
//   The address of PROPTRANSLMEM must be stored in mp2.
//
//   PROPTRANSLMEM may contain a translation memory name in
//   szMemName.
{
  /* Definition of local variables */
  MRESULT        mResult = (MRESULT)FALSE;  //Function return code
  BOOL fOK = TRUE;               //Process control return code
  PTMX_CREATE_IDA pCIDA;                    //Pointer to the create IDA

  // handle the different PM messages
  switch ( usMessage )
  {
    // ---------------------------------------------------------------------
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_MEMCREATE_DLG, mp2 ); break;

    case WM_INITDLG :
      SETWINDOWID( hDlg, ID_MEMCREATE_DLG );

      // allocate and intialize the memory create instance data area
      pCIDA = new(TMX_CREATE_IDA);
      if ( pCIDA == NULL )
      {
        UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
        fOK = FALSE;
      }
      else
      {
        memset( pCIDA->szMemName, 0, sizeof(pCIDA->szMemName) );
        memset( pCIDA->szSourceLang, 0, sizeof(pCIDA->szSourceLang) );
        memset( pCIDA->szLastDriveList, 0, sizeof(pCIDA->szLastDriveList) );
      }

      // anchor our IDA
      if ( fOK )
      {
        if ( ANCHORDLGIDA( hDlg, pCIDA ) == FALSE )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */
      }

      // process supplied parameters
      if ( fOK )
      {
        pCIDA->pszParms = (PSZ)PVOIDFROMMP2( mp2 );
        if ( pCIDA->pszParms != NULL)
        {
          /* mp2 contains the new TM name or a X15 string */
          PSZ pszTMName = (PSZ)PVOIDFROMMP2(mp2);

          if ( strchr (pszTMName, X15) )
          {
            /* this is a X15 string, get single contents */
            strcpy( pCIDA->szMemName, UtlParseX15 (pszTMName, MEM_CREATE_NAME_IND) );
            strcpy( pCIDA->szSourceLang, UtlParseX15 (pszTMName, MEM_CREATE_SOURCELANG_IND) );
          }
          else
          {
            /* mp2 contains only the new TM name */
            strcpy( pCIDA->szMemName, pszTMName );
          } /* endif */

          /* strip leading and trailing blanks from TM name */
          UtlStripBlanks( pCIDA->szMemName );
        } /* endif */
      }


      if ( fOK )
      {
        InitMemCreateDlg( hDlg );
      }/*endif*/

      /*************************************************************/
      /* Disable shared RB if no sharable/remote drives are        */
      /* available                                                 */
      /*************************************************************/

      //--- set focus to Name entry field
      WinSetFocus( HWND_DESKTOP, WinWindowFromID( hDlg, DID_CD_NAME ) );
      mResult = MRFROMSHORT(DIALOGINITRETURN( TRUE ));
      break;
      // ---------------------------------------------------------------------
   case WM_COMMAND:
      //--- call function to process all command messages
      ProcessWM_COMMAND( hDlg, mp1, mp2 );
      break;
      // ---------------------------------------------------------------------

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &pid_memcr_subtable[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_DESTROY:
      pCIDA = ACCESSDLGIDA( hDlg, PTMX_CREATE_IDA );
      if ( pCIDA ) delete( pCIDA );
      break;


    default:
      // Call the default Dialog procedure
      mResult = UTLDEFDIALOGPROC( hDlg, usMessage, mp1, mp2);
      break;
  } /* end switch usMessage */

  return mResult;
} /* end of function MEMCREATEDLG */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemCreateControl                                         |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
VOID MemCreateControl( HWND hDlg, USHORT usId  )
{
  // If a drive is selected store it in szMemPath
  PTMX_CREATE_IDA pCIDA;       // Pointer create IDA
  USHORT          usLast;      // last selected drive ID

  // Get address of dialog procedure data area
  pCIDA = ACCESSDLGIDA(hDlg, PTMX_CREATE_IDA);

  if (pCIDA != NULL)
  {
    // Get id of previous selected drive button
    usLast = ( PID_DRIVEBUTTON_A ) + pCIDA->szMemPath[0] - 'A';

    // Deselect  previous selected drive button
    WinSendMsg( WinWindowFromID( hDlg, usLast ),
                WM_DRIVEBUTTON_SELECT, 0, NULL);

    // Save selected drive character
    pCIDA->szMemPath[0] = (CHAR)( usId - ( PID_DRIVEBUTTON_A ) + (USHORT)'A');
    // Select selected drive button
    WinSendMsg( WinWindowFromID( hDlg, usId ), WM_DRIVEBUTTON_SELECT,
                MP1FROMSHORT( TRUE ), NULL);
  } /* endif */
} /* end of function MemCreateControl */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemCreateCommand                                         |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Handle the OK Command message                            |
//+----------------------------------------------------------------------------+
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
VOID MemCreateCommand( HWND hDlg )
{
  // Handle the OK Command messages
  BOOL            fOK = TRUE;       // Process OK flag
  PTMX_CREATE_IDA pCIDA;             // Pointer to create IDA
  OtmMemory       *pMemory = NULL;   // handle of memory database
  BOOL            fMemExists = TRUE; // Flag which indicates the existance of a memory database
  PSZ             pReplAddr;         // Pointer to an address list of replacement strings
  BOOL fShared = FALSE;
  BOOL fLocalMemoryNeeded = FALSE;

  MemoryFactory *pFactory = MemoryFactory::getInstance();


  //--- Get address of dialog procedure data area
  pCIDA = ACCESSDLGIDA(hDlg, PTMX_CREATE_IDA);

  //--- if no access to IDA stop further processing
  if ( pCIDA == NULL ) fOK = FALSE;

  if ( fOK )
  {
    // Disable the Create and Cancel button
    EnableOKAndCANCELButton( hDlg, FALSE );

    // Create memory database name with extension
    QUERYTEXT( hDlg, DID_CD_NAME, pCIDA->szMemName );
    ANSITOOEM( pCIDA->szMemName );

    //--- remove leading and trailing blanks
    UtlStripBlanks( pCIDA->szMemName );
    if ( pCIDA->szMemName[0] == NULC )
    {
      fOK = FALSE;
      UtlError(  ERROR_MEM_CREATE_NO_NAME, MB_CANCEL, 0,
                 NULL, EQF_ERROR );
      WinSetFocus( HWND_DESKTOP, WinWindowFromID( hDlg, DID_CD_NAME ));
      fMemExists = FALSE;
    } /* endif */
   
    // get selected plugin
    SHORT  sItem = CBQUERYSELECTION( hDlg, DID_CD_TYPE_CB );
    if ( sItem != LIT_NONE )
    {
      pCIDA->pSelectedPlugin = (OtmPlugin *) CBQUERYITEMHANDLE( hDlg, DID_CD_TYPE_CB, sItem );
    }

    // '-' should not be included in memname, because mysql don't allow this
    if ( fOK && (pCIDA->usLocation == TM_SHARED) && strchr(pCIDA->szMemName,'-')!=NULL )
    {
      fOK = FALSE;
      MessageBox( hDlg, "'-' isn't allowed in shared memory name,\nplease replace it with underline or other character.", "Error", MB_OK );
      fMemExists = FALSE;
    }


    if ( fOK )
    {
      if ( !UtlCheckLongName( pCIDA->szMemName ) )
      {
        fOK = FALSE;
        pReplAddr = pCIDA->szMemName;
        OEMTOANSI( pCIDA->szMemName );
        UtlError(  ERROR_INV_LONGNAME, MB_CANCEL, 1,
                   &pReplAddr, EQF_ERROR );
        WinSetFocus( HWND_DESKTOP, WinWindowFromID( hDlg, DID_CD_NAME ));
        fMemExists = FALSE;
      } /* endif */
    } /* endif */

    // check if there is already a memory with the given name
    if ( fOK )
    {
      if ( pFactory->exists( NULL, pCIDA->szMemName ) )
      {
          pReplAddr = pCIDA->szMemName;
          OEMTOANSI( pCIDA->szMemName );
          UtlError( ERROR_MEM_NAME_INVALID, MB_CANCEL, 1, &pReplAddr, EQF_ERROR );

          // Set cursor to the incorrect field the name field
          WinSetFocus( HWND_DESKTOP, WinWindowFromID( hDlg, DID_CD_NAME ));
          fMemExists = FALSE;
          fOK = FALSE;
      } /* end */         
    }/*endif*/

  }/*endif*/


  if ( fOK )
  {
    SHORT sSelectedItem;

    CBQUERYSELECTEDITEMTEXT( sSelectedItem, hDlg, DID_CD_SOURCE, pCIDA->szSourceLang );
    QUERYTEXT( hDlg, DID_CD_DESCRIPTION, pCIDA->szMemDesc );
    ANSITOOEM( pCIDA->szMemDesc );

    if ( !pCIDA->szSourceLang[0]  )
    {
      //--- display warning 'No source language selected'
      UtlError(  ERROR_NO_SOURCE_LANGUAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
      fOK = 0;
      fMemExists = FALSE;
    } /* endif */
  } /* endif */


  //==================================================================
  // - Create memory database. If an error occured
  // -  issue appropriate error messages and set fOK to FALSE
  //==================================================================
  if ( fOK )
  {
    int iRC;
    fShared = (pCIDA->pSelectedPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType);
    fLocalMemoryNeeded = FALSE;

    if ( fShared && ((OtmSharedMemoryPlugin *)pCIDA->pSelectedPlugin)->isLocalMemoryUsed() )
    {
      fLocalMemoryNeeded = TRUE;
    }


    // for shared memories using a local memory as copy...
    if ( fLocalMemoryNeeded )
    {
      // ... use default memory plugin to create the local memory
      pMemory = pFactory->createMemory( (char *)pFactory->getDefaultMemoryPlugin(), pCIDA->szMemName, pCIDA->szMemDesc, pCIDA->szSourceLang, pCIDA->szMemPath[0], &iRC );

      // create the shared component
      if ( pMemory != NULL )
      {
        OtmMemory *pSharedMem = pFactory->createSharedMemory( hDlg, (char *)pCIDA->pSelectedPlugin->getName(), pCIDA->szMemName, pMemory );
        if ( pSharedMem != NULL )
        {
          // use shared memory from now on
          pMemory = pSharedMem;
        }
        else
        {
          MessageBox( hDlg, "fail to create shared memory", "Error", MB_OK );
          fOK = FALSE;
        } /* end */       
      }
    }
    else
    {
      // otherwise use selected plugin
      pMemory = pFactory->createMemory( (char *)pCIDA->pSelectedPlugin->getName(), pCIDA->szMemName, pCIDA->szMemDesc, pCIDA->szSourceLang, pCIDA->szMemPath[0], &iRC );
      if ( pMemory == NULL )
      {
        pFactory->showLastError( (char *)pCIDA->pSelectedPlugin->getName(), pCIDA->szMemName, NULL, hDlg );
      } /* endif */
    }

    if ( pMemory == NULL )
    {
      //--- An error occurred in the create
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // get memory object name
  if ( fOK )
  {
     pFactory->getObjectName( pMemory, pCIDA->szTemp, sizeof(pCIDA->szTemp) );
  } /* endif */     

  //--- Tm is created. Close it. 
  if ( pMemory != NULL )
  {

    pFactory->closeMemory( pMemory );
    pMemory = NULL;
  }

  //=================================================================
  // - Save the last used values
  // - Post a message to update the main memory database listbox.
  // - Issue a message that create was OK. Set usCreate to TRUE
  // - Dismiss the dialog
  //=================================================================
  if ( fOK )
  {
    UpdateLastUsedProp( pCIDA );

    //EqfSend2Handler( MEMORYHANDLER, WM_EQFN_CREATED, MP1FROMSHORT( clsMEMORYDB ), MP2FROMP( pCIDA->szTemp ));

    //--- Issue the message that the memory database create was OK
    OEMTOANSI( pCIDA->szMemName );
    pReplAddr = pCIDA->szMemName;
    UtlError( MESSAGE_MEM_CREATE_COMPLETED , MB_OK, 1, &pReplAddr, EQF_INFO );
    ANSITOOEM( pCIDA->szMemName );
    pCIDA->usCreate = TRUE;

    DelCtrlFont (hDlg, DID_CD_DESCRIPTION);

    // pass name of created memory to calling function if requested
    if ( pCIDA->pszParms != NULL ) strcpy( pCIDA->pszParms, pCIDA->szMemName );

    DISMISSDLG( hDlg, fOK );
  } /* endif */

  //=================================================================
  // Delete eventually allocated properties and translation memory
  // if the create failed and the translation
  //  memory has not properly been created
  //=================================================================
  if ( !pCIDA->usCreate )
  {
    // Enable the Create and Cancel button
    EnableOKAndCANCELButton( hDlg, TRUE );
  } /* endif */

} /* end of function MemCreateCommand */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UpdateLastUsedProp                                       |
//+----------------------------------------------------------------------------+
//|Function call:     Update TM last used values                               |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
VOID UpdateLastUsedProp( PTMX_CREATE_IDA pCIDA )
{
  PMEM_LAST_USED       pLastUsed = NULL;          // Pointer to the default values
  USHORT    usSize = 0;
  if ( UtlLoadFile( pCIDA->szMemPropPath, (PVOID *)&pLastUsed, &usSize, TRUE, FALSE ) )
  {
    // Move actual values to MEM_LAST_USED
    strcpy( pLastUsed->szSourceLang,     pCIDA->szSourceLang );
    strcpy( pLastUsed->szCreateServer,   pCIDA->szServer     );
    pLastUsed->usCreateLocation = pCIDA->usLocation;

    UtlWriteFile( pCIDA->szMemPropPath, usSize, (PVOID)pLastUsed, FALSE );
    UtlAlloc( (PVOID *)&pLastUsed, 0, 0, NOMSG );
  } /* endif */
} /* end of function UpdateLastUsedProp */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HandleOtherControlMsgs                                   |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       handles all WM_CONTROL messages except                   |
//|                   the messages for the drive buttons                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
static
VOID HandleOtherControlMsgs( HWND   hDlg,
                             USHORT usMessage,
                             SHORT  sId,
                             SHORT  sNotification )
{
  PTMX_CREATE_IDA pCIDA = NULL;             //Pointer to the create IDA

  usMessage;                          // get rid of compiler warnings
  sNotification;

  switch ( sId )                           // selected control
  {
    //--------------------------------------------------------------------
    case DID_CD_TYPE_CB:                
      {
        //--- Get address of dialog procedure data area
        pCIDA = ACCESSDLGIDA(hDlg, PTMX_CREATE_IDA);

        if ( sNotification == CBN_SELCHANGE )
        {
          // get plugin for selected memory type
          SHORT  sItem = CBQUERYSELECTION( hDlg, DID_CD_TYPE_CB );

          if ( sItem != LIT_NONE )
          {
            pCIDA->pSelectedPlugin = (OtmPlugin *) CBQUERYITEMHANDLE( hDlg, DID_CD_TYPE_CB, sItem );

            // get list of supported drives from memory plugin
            if ( pCIDA->pSelectedPlugin->getType() == OtmPlugin::eTranslationMemoryType )
            {
              ((OtmMemoryPlugin *)pCIDA->pSelectedPlugin)->getListOfSupportedDrives( pCIDA->szDriveList );
            }
            else if ( pCIDA->pSelectedPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType )
            {
              ((OtmSharedMemoryPlugin *)pCIDA->pSelectedPlugin)->getListOfSupportedDrives( pCIDA->szDriveList );
            }

            //--- call function to setup and display the drive icons
            SetupDriveIcons( hDlg, TM_LOCAL, NULL );

            //--- delete server name in IDA/properties
            memset( pCIDA->szServer, NULC, sizeof( pCIDA->szServer ) );
            //--- delete userid in IDA/properties
            memset( pCIDA->szUserid, NULC, sizeof( pCIDA->szUserid ) );
            pCIDA->usLocation = TM_LOCAL;
          } /* endif */
        }
      }/*end if*/
      break;
      //------------------------------------------------------------------
    default :
      break;
  }/*end switch*/
}/* end HandleOtherControlMsgs */
/*
+------------------------------------------------------------------------------+
| SetupDriveIcons( HWND   hDlg,                                                |
|                  USHORT usLocation,                                          |
|                  PSZ    pszServer  )                                         |
|    hDlg       :  Dialog Box Handle                                           |
|    usLocation :  Indicates if local or remote drives should be displayed.    |
|                  Possible values: TM_REMOTE, TM_LOCAL, TM_SHARED             |
|    pszServer  :  name of server to get the drives                            |
|                  possible values: server name: get disks of server           |
|                                   \0         : get local/shared disks        |
|    get access to IDA and system properties                                   |
|    if previous displayed drive icons should be deleted                       |
|       switch old TM location                                                 |
|        case local RB was pevious selected                                    |
|          delete local drive icons                                            |
|        case remote RB was pevious selected                                   |
|          delete remote drive icons                                           |
|        case shared RB was pevious selected                                   |
|          delete shared drive icons                                           |
|       endswitch                                                              |
|    else                                                                      |
|       set flag that from now previous displayed drive icons sould be         |
|       deleted                                                                |
|    end                                                                       |
|    switch new location                                                       |
|     case TM_LOCAL                                                            |
|      display and position local drive icons                                  |
|          set location flag to TM_LOCAL                                       |
|     case TM_SHARED                                                           |
|      display and position shared drive icons                                 |
|          set location flag to TM_SHARED                                      |
|    endswitch                                                                 |
+------------------------------------------------------------------------------+
*/
static
VOID SetupDriveIcons( HWND   hDlg,           //dialog handle
                      USHORT usLocation,     //drive location
                      PSZ    pszServer )     //selected server in server LB
{
  PTMX_CREATE_IDA pCIDA = ACCESSDLGIDA(hDlg, PTMX_CREATE_IDA);

  pszServer; usLocation;

  if ( pCIDA->szLastDriveList[0] != EOS )          
  {
    DeleteDriveIcons( hDlg, pCIDA->szLastDriveList );
  }/*endif*/

  DisplayDriveIcons( hDlg, pCIDA->szDriveList );
  strcpy( pCIDA->szLastDriveList, pCIDA->szDriveList );
}/* end SetupDriveIcons */

/*
+------------------------------------------------------------------------------+
| DeleteDriveIcons( HWND hDlg, PSZ pszDriveList )                              |
|    hDlg        :  Dialog Box Handle                                          |
|    pszDriveList : drive letters of icons to be deleted                       |
|                                                                              |
|    Get address of dialog procedure data area                                 |
|    loop thru pszDriveList and destroy appropriate drive icons                |
+------------------------------------------------------------------------------+
*/
static
VOID DeleteDriveIcons( HWND  hDlg, PSZ pszDriveList )
{

  USHORT usIndex;                            // Index variable
  USHORT usDrive;                            // id for a drive icon

  //--- sort drivelist
  SortString( pszDriveList );

  usIndex = 0;
  while ( pszDriveList[usIndex] != EOS )
  {
    usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A, pszDriveList[usIndex] );
    WinDestroyWindow( WinWindowFromID( hDlg, usDrive ) );
    usIndex++;
  }/*endwhile*/
}/* end DeleteDriveIcons */
/*
+------------------------------------------------------------------------------+
| DisplayDriveIcons( HWND hDlg, PSZ pszDriveList )                             |
|                                                                              |
|     hDlg        :  Dialog Box Handle                                         |
|     pszDriveList : drive letters of icons to be displayed                    |
|                                                                              |
|     Get address of dialog procedure data area                                |
|     Get position of dummy control and group box                              |
|     Display and position drive icons                                         |
|     Select EQF system drive as default drive                                 |
|     !!!!! add usage of last used values for drive                            |
|                                                                              |
+------------------------------------------------------------------------------+
*/
static
VOID DisplayDriveIcons( HWND hDlg, PSZ pszDriveList )  
{
  SWP            swpDrive;                  // output from WinQueryWindowPos
  SWP            swpBox;                    // size/position of drive groupbox
  PTMX_CREATE_IDA pCIDA = NULL;             // Pointer to the create IDA

  //--- Get address of dialog procedure data area
  pCIDA = ACCESSDLGIDA(hDlg, PTMX_CREATE_IDA);

  //--- sort drivelist
  SortString( pszDriveList );

  // Get position of dummy control and group box
  WinQueryWindowPos( WinWindowFromID( hDlg, DID_CD_DUMMY ), &swpDrive );
  WinQueryWindowPos( WinWindowFromID( hDlg, DID_CD_TMLOCATION_GB ), &swpBox );

  // Display available drives as icons
  if ( *pszDriveList != EOS ) UtlCreateDriveButtons( hDlg, pszDriveList, PID_DRIVEBUTTON_A, WS_GROUP | WS_TABSTOP, WS_VISIBLE );

  // Hide dummy button for Windows (is hidden in OS/2)
  ShowWindow( WinWindowFromID( hDlg, DID_CD_DUMMY ), SW_HIDE );

  // Set position of drive buttons depending on dummy button
  if ( *pszDriveList != EOS ) UtlSetPosDriveButtons2( hDlg,      PID_DRIVEBUTTON_A,
                                swpDrive.x,   swpDrive.y,
                                swpDrive.cx,  swpDrive.cy,
                                (swpBox.x + swpBox.cx),
                                swpBox.y,
                                6, // leave 6 pels in x direction and
                                5, // 5 pels in y direction between buttons
                                WinWindowFromID( hDlg, DID_CD_TXT_DRIVES ) );

  //--- Select the EQF system drive as default drive if it is contained in the drive list
  if ( *pszDriveList != EOS ) 
  {
    char szDrive[4];
    USHORT usIndex = 0;
    UtlQueryString( QST_PRIMARYDRIVE, szDrive, sizeof(szDrive) ); 
    while ( pszDriveList[usIndex] != szDrive[0] ) usIndex++;
    if ( pszDriveList[usIndex] != szDrive[0] ) usIndex = 0;
    usIndex = pszDriveList[usIndex] - 'A';
    SETCHECK_TRUE( hDlg, (PID_DRIVEBUTTON_A + usIndex) );
  }
}/* end DisplayDriveIcons */

/*
+------------------------------------------------------------------------------+
| ProcessWM_COMMAND                                                            |
|    processes all WM_COMMAND messages of MemCreateDlg                         |
+------------------------------------------------------------------------------+
*/
static
VOID ProcessWM_COMMAND( HWND   hDlg,
                        WPARAM mp1 ,
                        LPARAM mp2 )
{
  PTMX_CREATE_IDA pCIDA;                    //Pointer to the create IDA
  USHORT         usUserPriv;                //LAN user privilege class
  mp2;

  switch ( WMCOMMANDID( mp1, mp2 ) )             //selected control
  {
    case PID_PB_HELP:
     UtlInvokeHelp();
     break;
    // -----------------------------------------------------------------------
    case PID_PB_OK:                        //Create button selected
      EnableOKAndCANCELButton( hDlg, FALSE );

      // Get address of dialog procedure data area
      pCIDA = ACCESSDLGIDA(hDlg, PTMX_CREATE_IDA);

      //--- Try to get the LAN userid also for local TMs. If the /*@U1A*/
      //--- user is currently logged on thes userid should be saved/*@U1A*/
      //--- also for local TMs.                                  /*@U1A*/
      UtlGetLANUserID( pCIDA->szUserid, &usUserPriv, FALSE );    /*@U1A*/

      MemCreateCommand( hDlg );

      EnableOKAndCANCELButton( hDlg, TRUE );
      break;
      // -----------------------------------------------------------------------
    case PID_PB_CANCEL:
    case DID_CANCEL:                      //ESC key selected
      DelCtrlFont (hDlg, DID_CD_DESCRIPTION);
      DISMISSDLG( hDlg, TRUE );
      break;
      // -----------------------------------------------------------------------
    case DID_CD_TYPE_CB :                
      HandleOtherControlMsgs( hDlg, WM_COMMAND, WMCOMMANDID( mp1, mp2 ), WMCOMMANDCMD( mp1, mp2 ) );
      break;

    case DID_CD_DESCRIPTION:
      if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
      {
        ClearIME( hDlg );
      } /* endif */
      break;

    default :
      // Process the routine MemCreateControl if a drive control is selected
      if ( ( WMCOMMANDCMD( mp1, mp2 ) == BN_CLICKED ||
             WMCOMMANDCMD( mp1, mp2 ) == EQF_CD_BM_CLICK )   &&
           (WMCOMMANDID( mp1, mp2 ) >= PID_DRIVEBUTTON_A ) &&
           (WMCOMMANDID( mp1, mp2 ) <= PID_DRIVEBUTTON_Z))
      {
        MemCreateControl( hDlg, SHORTFROMMP1( mp1 ) );
      }
      else
      {
        WinDefDlgProc( hDlg, WM_COMMAND, mp1, mp2 );
      } /* endif */
      break;
  }/* end switch */
  // ---------------------------------------------------------------------
}/* end ProcessWM_COMMAND */

//+----------------------------------------------------------------------------+
//|Function name:     InitMemCreateDlg                                         |
static VOID InitMemCreateDlg( HWND   hDlg )
{
  USHORT         usRc = TRUE;               //Process control return code
  PTMX_CREATE_IDA pCIDA;                    //Pointer to the create IDA
  HWND           hName;                     //Handle of the name field
  CHAR           szLastUsed[MAX_FILESPEC];  //Name of object containing last used values
  PMEM_LAST_USED pLastUsed = NULL;          //Pointer to the last used values
  USHORT         usUserPriv;                //LAN user privilege class
  SHORT          sItem;               // listbox item index
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  // Get address of dialog procedure data area
  pCIDA = ACCESSDLGIDA(hDlg, PTMX_CREATE_IDA);

  // set default memory plugin
  strcpy( pCIDA->szPlugin, DEFAULT_MEMORY_PLUGIN  );

  hName = WinWindowFromID( hDlg, DID_CD_NAME );
  SETTEXTLIMITHWND( hName, (MAX_LONGFILESPEC - 1) );

  SETTEXTLIMIT( hDlg, DID_CD_DESCRIPTION, MAX_MEM_DESCRIPTION - 1 );

  SetCtrlFnt (hDlg, GetCharSet(), DID_CD_DESCRIPTION, DID_CD_NAME );

  // get list of available memory plugins

  pFactory->getMemoryPlugins( pCIDA->vMemoryPlugins, pCIDA->vSharedMemoryPlugins );

  // fill memory type combobox and select last-used / default plugin
  for( int i=0; i < (int)pCIDA->vMemoryPlugins.size(); i++ )
  {
    const char *pszType = pCIDA->vMemoryPlugins[i]->getDescriptiveMemType();
    SHORT sItem = CBINSERTITEM( hDlg, DID_CD_TYPE_CB, pszType );
    CBSETITEMHANDLE( hDlg, DID_CD_TYPE_CB, sItem, (pCIDA->vMemoryPlugins[i]) );
    if ( stricmp( pCIDA->szPlugin, pCIDA->vMemoryPlugins[i]->getName() ) == 0 )
    {
      CBSELECTITEM( hDlg, DID_CD_TYPE_CB, sItem );
    }
  }
  for( int i=0; i < (int)pCIDA->vSharedMemoryPlugins.size(); i++ )
  {
    const char *pszType = pCIDA->vSharedMemoryPlugins[i]->getDescriptiveMemType();
    SHORT sItem = CBINSERTITEM( hDlg, DID_CD_TYPE_CB, pszType );
    CBSETITEMHANDLE( hDlg, DID_CD_TYPE_CB, sItem, (pCIDA->vSharedMemoryPlugins[i]) );
    if ( stricmp( pCIDA->szPlugin, pCIDA->vSharedMemoryPlugins[i]->getName() ) == 0 )
    {
      CBSELECTITEM( hDlg, DID_CD_TYPE_CB, sItem );
    }
  }

  // get last-used values
  if ( usRc )
  {
    USHORT usSize = 0;
    strcpy( szLastUsed, MEMORYLU_PROPERTIES_NAME );

    UtlMakeEQFPath( pCIDA->szMemPropPath, NULC, PROPERTY_PATH, NULL );
    strcat( pCIDA->szMemPropPath, "\\" );
    strcat( pCIDA->szMemPropPath, MEMORYLU_PROPERTIES_NAME );
    usRc = (USHORT)UtlLoadFile( pCIDA->szMemPropPath, (PVOID *)&pLastUsed, &usSize, TRUE, FALSE );
  } /* endif usRc */

  if ( usRc )
  {
    //  If a translation memory name is present move it to the
    //  translation memory entry field.
    if ( pCIDA->szMemName[0] != NULC )
    {
      OEMTOANSI( pCIDA->szMemName );
      SETTEXTHWND( hName , pCIDA->szMemName );
      ANSITOOEM( pCIDA->szMemName );
    } /* endif */

    if ( usRc )
    {
      // Fill the source language drop down list box by scanning all
      // language entries in the file SOURCE.LNG and select if
      // available the last used entry
      MemFillTableLB( WinWindowFromID(hDlg, DID_CD_SOURCE), SOURCE_LANGUAGES, szLastUsed);
    } /* endif */

    if ( usRc )
    {
      //--- if a source language is provided by caller, call function to select the language in the combobox
      if ( pCIDA->szSourceLang[0] != NULC ) CBSEARCHSELECT( sItem, hDlg, DID_CD_SOURCE, pCIDA->szSourceLang );
    } /* endif */
  } /* endif */

  if ( usRc )
  {
    memset( pCIDA->szUserid, NULC, sizeof( pCIDA->szUserid ) );
    UtlGetLANUserID( pCIDA->szUserid, &usUserPriv, FALSE );
  }/*endif usRc*/

  //--- Close the last uses TM properties if open
  if ( pLastUsed != NULL )
  {
    UtlAlloc( (PVOID *)&pLastUsed, 0, 0, NOMSG );
  } /* endif */

  if ( !usRc )
  {
    DelCtrlFont (hDlg, DID_CD_DESCRIPTION);
    DISMISSDLG( hDlg, TRUE );
  } /* endif */
}/* end InitMemCreateDlg */


/*
+------------------------------------------------------------------------------+
| SortString                                                                   |
|    sorts the passed string                                                   |
+------------------------------------------------------------------------------+
*/
static
VOID SortString( PSZ pszString )
{
  qsort( (void *)pszString, strlen( pszString ),
         (size_t)sizeof( CHAR ), iCompare );
}/* end SortString*/

/*
+---------------------------------------------------------------------------+
| iCompare                                                                  |
|    local function to compare characters, used by qsort                    |
|    if element1 less    than element2 => return value less than 0          |
|    if element1 equal   to   element2 => return value 0                    |
|    if element1 greater than element2 => return value greater than 0       |
+---------------------------------------------------------------------------+
*/
int iCompare( const void *arg1, const void *arg2 )
{
  int intRc;

  if ( *((PUCHAR)arg1) < *((PUCHAR)arg2) )
  {
    intRc = -1;
  }
  else
  {
    if ( *((PUCHAR)arg1) == *((PUCHAR)arg2) )
    {
      intRc = 0;
    }
    else
    {
      intRc = 1;
    }/*endif*/
  }/*endif*/
  return( intRc );
}/* end iCompare */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemBatchTMCreate                                         |
//+----------------------------------------------------------------------------+
//|Function call:     MemBatchTMCreate( pMemCrt );                             |
//+----------------------------------------------------------------------------+
//|Description:       Creates a folder in batch mode.                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PMEMCRT    pMemCrt     TM create data structure          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       function completed successfully                  |
//|                   other   number of error message for given error condition|
//+----------------------------------------------------------------------------+
//|Function flow:     create invisible listbox for names of folder/memories/...|
//|                   check folder name syntax                                 |
//|                   check if folder name is unique                           |
//|                   check if folder target drive is valid                    |
//|                   check if translation memory exists                       |
//|                   check if format is valid                                 |
//|                   check if editor is valid                                 |
//|                   check if dictionary/ies exist(s)                         |
//|                   allocate folder property memory and fill-in values       |
//|                   create folder properties                                 |
//|                   create folder directory                                  |
//|                   broadcast folder-created message                         |
//|                   destroy invisible listbox                                |
//|                   cleanup                                                  |
//+----------------------------------------------------------------------------+
//USHORT MemBatchTMCreate
//(
//HWND             hwndParent,         // handle of memory list window
//PDDEMEMCRT       pMemCrt             // TM create data structure
//)
//{
//  HWND             hwndLB = NULLHANDLE;// handle of invisible listbox
//  PSZ              pszParm;            // pointer for error parameters
//  BOOL             fOK = TRUE;         // internal O.K. flag
//  PTMX_CREATE_IDA  pCIDA = NULL;       //Pointer to the create IDA
//  HTM              htm = NULLHANDLE;   // handle of memory database
//
//  /********************************************************************/
//  /* Create invisible listbox for names of folder/memories/...        */
//  /********************************************************************/
//  hwndLB = WinCreateWindow( hwndParent, WC_LISTBOX, "", 0L, 0, 0, 0, 0,
//                            hwndParent, HWND_TOP, 1, NULL, NULL );
//
//  /********************************************************************/
//  /* Allocate storage for create IDA                                  */
//  /********************************************************************/
//  if ( fOK )
//  {
//    fOK = UtlAllocHwnd( (PVOID *)&pCIDA, 0L, (LONG)sizeof(PROPTRANSLMEM),
//                        ERROR_STORAGE, pMemCrt->hwndErrMsg );
//  } /* endif */
//
//  /********************************************************************/
//  /* Check memory name syntax                                         */
//  /********************************************************************/
//  if ( fOK )
//  {
//    if ( !UtlCheckLongName( pMemCrt->szName ) )
//    {
//      pszParm = pMemCrt->szName;
//      UtlErrorHwnd(  ERROR_INV_LONGNAME , MB_CANCEL, 1,
//                     &pszParm, EQF_ERROR, pMemCrt->hwndErrMsg );
//      fOK = FALSE;
//    } /* endif */
//  } /* endif */
//
//  /********************************************************************/
//  /* Check if source language is valid                                */
//  /********************************************************************/
//  if ( fOK )
//  {
//    SHORT sItem;                       // listbox item index
//
//    DELETEALLHWND( hwndLB );
//    UtlFillTableLB( hwndLB, SOURCE_LANGUAGES );
//    sItem = ISEARCHITEMHWND( hwndLB, pMemCrt->szSourceLang );
//    if ( sItem < 0 )
//    {
//      pszParm = pMemCrt->szSourceLang;
//      pMemCrt->DDEReturn.usRc = ERROR_PROPERTY_LANG_DATA;
//      UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
//                    &pszParm, EQF_ERROR, pMemCrt->hwndErrMsg );
//      fOK = FALSE;
//    }
//    else
//    {
//      QUERYITEMTEXTHWND( hwndLB, sItem, pMemCrt->szSourceLang );
//    } /* endif */
//  } /* endif */
//
//  /********************************************************************/
//  /* Check if target drive is valid                                   */
//  /********************************************************************/
//  if ( fOK && (pMemCrt->chToDrive != EOS) )
//  {
//    if ( pMemCrt->fShared )
//    {
//      CHAR szLANDrives[MAX_DRIVELIST];   // buffer for LAN drive letters
//
//      /******************************************************************/
//      /* Get LAN drives                                                 */
//      /******************************************************************/
//      UtlGetLANDriveList( (PBYTE)szLANDrives );
//
//      /******************************************************************/
//      /* Check if specified target drive is in list of valid drives     */
//      /******************************************************************/
//      if ( strchr( szLANDrives, toupper(pMemCrt->chToDrive) ) == NULL )
//      {
//        CHAR szDrive[MAX_DRIVE];
//
//        fOK = FALSE;
//        szDrive[0] = pMemCrt->chToDrive;
//        szDrive[1] = COLON;
//        szDrive[2] = EOS;
//        pszParm = szDrive;
//        pMemCrt->DDEReturn.usRc = ERROR_LAN_DRIVE_NOT_VALID;
//        UtlErrorHwnd( pMemCrt->DDEReturn.usRc, MB_CANCEL, 1,
//                      &pszParm, EQF_ERROR, pMemCrt->hwndErrMsg );
//      } /* endif */
//    }
//    else
//    {
//      CHAR szEqfDrives[MAX_DRIVELIST];   // buffer for EQF drive letters
//
//      /******************************************************************/
//      /* Get local EQF drives                                           */
//      /******************************************************************/
//      UtlGetCheckedEqfDrives( szEqfDrives );
//
//      /******************************************************************/
//      /* Check if specified target drive is in list of valid drives     */
//      /******************************************************************/
//      if ( strchr( szEqfDrives, toupper(pMemCrt->chToDrive) ) == NULL )
//      {
//        CHAR szDrive[MAX_DRIVE];
//
//        fOK = FALSE;
//        szDrive[0] = pMemCrt->chToDrive;
//        szDrive[1] = COLON;
//        szDrive[2] = EOS;
//        pszParm = szDrive;
//        pMemCrt->DDEReturn.usRc = ERROR_EQF_DRIVE_NOT_VALID;
//        UtlErrorHwnd( pMemCrt->DDEReturn.usRc, MB_CANCEL, 1,
//                      &pszParm, EQF_ERROR, pMemCrt->hwndErrMsg );
//      } /* endif */
//    } /* endif */
//  } /* endif */
//
//  // check if TM exists already
//  if ( fOK )
//  {
//      OBJLONGTOSHORTSTATE ObjState;
//
//      // checks also for shared objects on the supplied target drive
//      ObjLongToShortNameEx( pMemCrt->szName , pMemCrt->chToDrive,
//                            pCIDA->szMemShortName,
//                            TM_OBJECT, &ObjState );
//
//      switch ( ObjState )
//      {
//        case OBJ_EXISTS_ALREADY:
//          {
//            PSZ pszErrParm = pMemCrt->szName ;
//            UtlErrorHwnd( ERROR_MEM_NAME_INVALID, MB_CANCEL, 1, &pszErrParm, EQF_ERROR, pMemCrt->hwndErrMsg );
//            fOK = FALSE;
//            pMemCrt->DDEReturn.usRc = ERROR_MEM_NAME_INVALID;
//          }
//          break;
//
//        case SHARED_OBJ_EXISTS:
//          {
//            CHAR szDrive[3];             // buffer for drive string
//            PSZ  apszErrParm[2];
//
//            // there is already a shared object with the given name but it has
//            // not been connected
//            apszErrParm[0] = pMemCrt->szName;
//            sprintf( szDrive, "%c:", pMemCrt->chToDrive );
//            apszErrParm[1] = szDrive;
//            UtlErrorHwnd( ERROR_SHAREDMEM_EXISTS, MB_CANCEL, 2, apszErrParm, EQF_ERROR, pMemCrt->hwndErrMsg );
//            fOK = FALSE;
//            pMemCrt->DDEReturn.usRc = ERROR_SHAREDMEM_EXISTS;
//          }
//          break;
//
//        default:
//        case OBJ_IS_NEW:
//          break;
//      } /* endswitch */
//  } /* endif */
//
//
//
//  /********************************************************************/
//  /* Fill create IDA fields                                           */
//  /********************************************************************/
//  if ( fOK )
//  {
//    USHORT         usUserPriv;        // LAN user privilege class
//
//    strcpy( pCIDA->szSourceLang, pMemCrt->szSourceLang );
//    strcpy( pCIDA->szMemDesc, pMemCrt->szDescr );
//    strcpy( pCIDA->szMemName, pMemCrt->szName );
//
//    pCIDA->usLocation = ( pMemCrt->fShared ) ? TM_SHARED : TM_LOCAL;
//
//    UtlMakeEQFPath( pCIDA->szMemPath, NULC, MEM_PATH, NULL );
//    strcat( pCIDA->szMemPath, "\\" );
//
//    pCIDA->szMemPath[0] = pMemCrt->chToDrive;
//
//    UtlMakeEQFPath( pCIDA->szMemPropPath, NULC, SYSTEM_PATH, NULL );
//
//    if ( UtlGetLANUserID( pCIDA->szUserid, &usUserPriv, FALSE ) != NO_ERROR )
//    {
//      memset( pCIDA->szUserid, NULC, sizeof( pCIDA->szUserid ) );
//    } /* endif */
//
//    fOK = MemCreatePropsFromDlgData( pCIDA, pMemCrt->hwndErrMsg );
//  } /* endif */
//
//  /*******************************************************************/
//  /* In case of shared TMs create the EQF directories if necessary   */
//  /*******************************************************************/
//  if ( fOK && (pCIDA->usLocation == TM_SHARED) )
//  {
//    UtlMakeEQFPath( pCIDA->szTemp, pCIDA->szFullMemName[0], MEM_PATH, NULL );
//    UtlMkMultDir( pCIDA->szTemp, FALSE );
//  } /* endif */
//
//  /********************************************************************/
//  /* Create memory database                                           */
//  /********************************************************************/
//  if ( fOK )
//  {
//    USHORT usRC;
//
//    usRC = TmCreate( pCIDA->szFullMemName,   // Name of TM to be created
//                     &htm,                   // Handle of TM (output)
//                     (HTM)0,                 // Handle of model TM or 0
//                     pCIDA->szServer,        // Server name
//                     pCIDA->szUserid,
//                     pCIDA->szSourceLang,    // Source language or null string
//                     pCIDA->szMemDesc,
//                     TRUE,
//					 pMemCrt->hwndErrMsg );
//    if ( usRC == NO_ERROR )
//    {
//      //--- Tm is created. Close it.
//      TmClose( htm, pCIDA->szFullMemName, FALSE, 0 );
//    }
//    else
//    {
//      fOK = FALSE;
//    } /* endif */
//  } /* endif */
//
//  /*******************************************************************/
//  /* For shared TMs only: Create copy of property file in MEM        */
//  /* directory on target drive                                       */
//  /*******************************************************************/
//  if ( fOK && (pCIDA->usLocation == TM_SHARED) )
//  {
//    /*****************************************************************/
//    /* Setup source: fully qualified path of local property file     */
//    /*****************************************************************/
//    UtlMakeEQFPath( pCIDA->szTemp, NULC, PROPERTY_PATH, NULL );
//    strcat( pCIDA->szTemp, BACKSLASH_STR );
//    strcat( pCIDA->szTemp, pCIDA->szMemPropName );
//
//    /*****************************************************************/
//    /* Setup target: fully qualified path for copy of property file  */
//    /*****************************************************************/
//    strcpy( pCIDA->szTemp2, pCIDA->szMemPath );
//    strcat( pCIDA->szTemp2, pCIDA->szMemShortName );
//    strcat( pCIDA->szTemp2, EXT_OF_SHARED_MEMPROP );
//
//    /*****************************************************************/
//    /* Copy the property file                                        */
//    /*****************************************************************/
//    fOK = ( UtlCopy( pCIDA->szTemp, pCIDA->szTemp2, 1, 0L, TRUE ) ==
//            NO_ERROR );
//  } /* endif */
//
//  //=================================================================
//  // - Post a message to update the main memory database listbox.
//  //=================================================================
//  if ( fOK )
//  {
//    sprintf( pCIDA->szTemp, "%s\\%s", pCIDA->szMemPropPath,
//             pCIDA->szMemPropName );
//    EqfSend2Handler( MEMORYHANDLER, WM_EQFN_CREATED,
//                     MP1FROMSHORT( clsMEMORYDB ), MP2FROMP( pCIDA->szTemp ));
//  } /* endif */
//
//  /********************************************************************/
//  /* Destroy invisible listbox                                        */
//  /********************************************************************/
//  if ( hwndLB )  WinDestroyWindow( hwndLB );
//
//  /********************************************************************/
//  /* Cleanup                                                          */
//  /********************************************************************/
//  if ( pCIDA ) UtlAlloc( (PVOID *) &pCIDA, 0L, 0L, NOMSG );
//
//  /********************************************************************/
//  /* Tell DDE handler that task has been completed                    */
//  /********************************************************************/
//  if ( !fOK )
//  {
//    pMemCrt->DDEReturn.usRc = UtlGetDDEErrorCode( pMemCrt->hwndErrMsg );
//  } /* endif */
//  WinPostMsg( pMemCrt->hwndOwner, WM_EQF_DDE_ANSWER,
//              NULL, MP2FROMP(&pMemCrt->DDEReturn) );
//
//  return( pMemCrt->DDEReturn.usRc );
//} /* end of function MemBatchTMCreate */
//
//
BOOL MemCreatePropsFromDlgData
(
PTMX_CREATE_IDA pCIDA,               // Pointer to create IDA
HWND            hwnd                 // window handle for error messages
)
{
  BOOL            fOK = TRUE;          // Process OK flag
  PPROP_NTM       pProp;               // Pointer to TM properties
  PSZ             pReplAddr;           // Pointer to an address list of replacement strings
  EQFINFO         ErrorInfo;           // Property handler error info
  HPROP           hProp = NULL;      // Property handle

  if ( fOK)
  {
    strcpy( pCIDA->szMemPropName, strupr(pCIDA->szMemShortName));
    strcat( pCIDA->szMemPropName, EXT_OF_TMPROP );

    // Create the properties for the translation memory
    hProp = CreatePropertiesEx( pCIDA->szMemPropName,
                                pCIDA->szMemPropPath,
                                PROP_CLASS_MEMORYDB,
                                &ErrorInfo, TRUE );
    if ( !hProp )
    {
      //--- The properties have not been created.Issue the appropriate message
      if ( ErrorInfo == Err_NoDiskSpace )
      {
        strncpy( pCIDA->szTemp, pCIDA->szMemPropPath, 1 );
        pReplAddr = pCIDA->szTemp;
        UtlErrorHwnd(  ERROR_DISK_IS_FULL, MB_CANCEL, 1,
                       &pReplAddr, EQF_ERROR, hwnd );
      }
      else
      {
        if ( ErrorInfo != Err_NoStorage )
        {
          OEMTOANSI( pCIDA->szMemName );
          pReplAddr = pCIDA->szMemName;
          UtlErrorHwnd( ERROR_MEM_NAME_INVALID , MB_CANCEL, 1,
                        &pReplAddr, EQF_ERROR, hwnd );
          ANSITOOEM( pCIDA->szMemName );
        }/*endif*/
      }/*endif*/

      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //--- Address the property area
    pProp = (PPROP_NTM)MakePropPtrFromHnd( hProp );

    /****************************************************************/
    /* fill szFullMemName d:\eqf\mem\tmname.tmd                     */
    /****************************************************************/
    strcpy( pCIDA->szFullMemName, pCIDA->szMemPath );
    strcat( pCIDA->szFullMemName, pCIDA->szMemShortName );
    if ( pCIDA->usLocation == TM_SHARED )
    {
      strcat( pCIDA->szFullMemName, EXT_OF_SHARED_MEM );
    }
    else
    {
      strcat( pCIDA->szFullMemName, EXT_OF_TMDATA );
    } /* endif */

    /****************************************************************/
    /* fill the TM properties structure  as properties structure    */
    /* the TM signature record structure TMX_SIGN is used           */
    /* in addition to the generl property header                    */
    /****************************************************************/
    strcpy( pProp->szNTMMarker, NTM_MARKER );
    strcpy( pProp->stTMSignature.szName,   pCIDA->szMemShortName );
    strcpy( pProp->szLongName,             pCIDA->szMemName );
    strcpy( pProp->stTMSignature.szServer, pCIDA->szServer  );
    strcpy( pProp->stTMSignature.szUserid, pCIDA->szUserid  );
    pProp->usLocation = pCIDA->usLocation;
    UtlTime( &pProp->stTMSignature.lTime );
    strcpy( pProp->stTMSignature.szSourceLanguage, pCIDA->szSourceLang );
    pProp->stTMSignature.bMajorVersion = TM_MAJ_VERSION;
    pProp->stTMSignature.bMinorVersion = TM_MIN_VERSION;
    strcpy( pProp->stTMSignature.szDescription, pCIDA->szMemDesc );
    strcpy( pProp->szFullMemName,          pCIDA->szFullMemName );
    pProp->usThreshold = TM_DEFAULT_THRESHOLD;

    if ( SaveProperties( hProp, &ErrorInfo) )
    {
      pReplAddr = pCIDA->szMemName;
      UtlErrorHwnd( ERROR_WRITE_PROPERTIES, MB_CANCEL, 1, &pReplAddr,
                    EQF_ERROR, hwnd );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( hProp != NULL )
  {
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo);
  } /* endif */
  return( fOK );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MEMPROPDLG                                               |
//+----------------------------------------------------------------------------+
//|Function call:     MRESULT                                                  |
//|                   EXPENTRY MEMPROPDLG( HWND      hDlg,                     |
//|                                          USHORT    usMessage,              |
//|                                          WPARAM    mp1,                    |
//|                                          LPARAM    mp2 )                   |
//+----------------------------------------------------------------------------+
//|Description:       TM property dialog procedure                             |
//+----------------------------------------------------------------------------+
//|Parameters:        hDlg      -  dialog handle                               |
//|                   usMessage -  PM message                                  |
//|                   mp1       -  message parameter 1                         |
//|                   mp2       -  message parameter 1                         |
//|                                contains the pointer to MEM_PROP_IDA        |
//|                                the MEM_PROP_IDA is allocated and freed     |
//|                                by the TM handler (EQFMEM00.C)              |
//+----------------------------------------------------------------------------+
//|Prerequesits:      - the MEM_PROP_IDA must have been allocated by the       |
//|                     calling function                                       |
//|                   - the IDA member contains the handle of an open TM       |
//|                   - the IDA member szMemPath contains the fully qualified  |
//|                     path of the TM                                         |
//|                   - the IDA member szMemName contains the name of the TM   |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK MEMPROPDLG
(
HWND      hDlg,
WINMSG    usMessage,
WPARAM    mp1,
LPARAM    mp2
)
{
  MRESULT    mResult = (MRESULT)FALSE;// function return code
  BOOL       fOK;                     // internal processing O.K. flag
  PMEM_PROP_IDA  pIDA;                // pointer to the dialog IDA

  switch ( usMessage )
  {
    // ---------------------------------------------------------------------
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_TM_PROP_DLG, mp2 ); break;
    case WM_HELP:
    /*************************************************************/
    /* pass on a HELP_WM_HELP request                            */
    /*************************************************************/
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblTMPropDlg[0] );
      mResult = TRUE;  // message processed
      break;

      // ---------------------------------------------------------------------
    case WM_INITDLG :
      {
        fOK = TRUE;
        SETWINDOWID( hDlg, ID_TM_PROP_DLG );

        // anchor IDA pointer (passed to dialog via mp2)
        pIDA = (PMEM_PROP_IDA)PVOIDFROMMP2( mp2 );
        if ( ANCHORDLGIDA( hDlg, pIDA ) == FALSE )
        {
          // Issue the system error message
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */

        // prepare dialog controls
        SetCtrlFnt (hDlg, GetCharSet(), ID_TM_PROP_DESCR_EF, 0 );
        SETTEXTLIMIT( hDlg, ID_TM_PROP_DESCR_EF, MAX_MEM_DESCRIPTION - 1 );

        // fill dialog controls
        if ( fOK )
        {
          OtmMemoryPlugin::PMEMORYINFO pInfo = NULL; 
          MemoryFactory *pFactory = MemoryFactory::getInstance();

          // set TM name
          OEMTOANSI( pIDA->szMemName );
          SETTEXT( hDlg, ID_TM_PROP_NAME_EF, pIDA->szMemName );
          ANSITOOEM( pIDA->szMemName );

          // get memory info
          pInfo = (OtmMemoryPlugin::PMEMORYINFO) malloc( sizeof(OtmMemoryPlugin::MEMORYINFO) );
          memset( pInfo, 0, sizeof(OtmMemoryPlugin::MEMORYINFO) );
          OtmMemoryPlugin *pPlugin = (OtmMemoryPlugin *)pIDA->pMem->getPlugin();
          pFactory->getMemoryInfo( (PSZ)pPlugin->getName(), pIDA->szMemName, pInfo );

          // set TM description entry field
          {
            CHAR szDescr[MAX_MEM_DESCRIPTION];  // buffer for description
            pIDA->pMem->getDescription( szDescr, sizeof(szDescr) );
            OEMTOANSI( szDescr );
            SETTEXT( hDlg, ID_TM_PROP_DESCR_EF, szDescr );
          }

          // show memory short name
          {
            CHAR szShortName[MAX_FILESPEC];
            if ( pInfo->szFullPath[0] != '\0' )
            {
              Utlstrccpy( szShortName, UtlGetFnameFromPath( pInfo->szFullPath ), '.' );
            }
            else
            {
              strcpy( szShortName, "n/a" );
            }
            SETTEXT( hDlg, ID_TM_PROP_SHORTNAME_EF, szShortName );
          }
          // set location static
          {
            CHAR szLocation[80];      // Buffer for location string

            memset( szLocation, 0, sizeof(szLocation) );
            if ( pPlugin->getType() == OtmPlugin::eTranslationMemoryType )
            {
              strcpy( szLocation, pPlugin->getDescriptiveMemType() );
            }
            else if ( pPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType )
            {
              strcpy( szLocation, ((OtmSharedMemoryPlugin *)pPlugin)->getDescriptiveMemType() );
            }
            SETTEXT( hDlg, ID_TM_PROP_LOC_EF, szLocation );
          }

          // set source language static
          {
            char szLang[40];
            pIDA->pMem->getSourceLanguage( szLang, sizeof(szLang) );
            SETTEXT( hDlg, ID_TM_PROP_SOURCELANG_EF, szLang );
          }

          // set drive static
          {
            CHAR szDrive[4];
            if ( pInfo->szFullPath[0] != '\0' )
            {
              szDrive[0] = pInfo->szFullPath[0];
              szDrive[1] = ':';
              szDrive[2] = '\0';
            }
            else
            {
              strcpy( szDrive, "n/a" );
            }
            SETTEXT( hDlg, ID_TM_PROP_DRIVE_EF, szDrive );
          }

          // set size static 
          {
            ULONG ulSize = pIDA->pMem->getFileSize();
            char szSize[20];
            sprintf( szSize, "%lu", ulSize );
            SETTEXT( hDlg, ID_TM_PROP_SIZE_EF, szSize );
          }

          // set last update static 
          {
            char szLastUpdate[30];

            if ( pInfo->szFullPath[0] != '\0' )
            {
              WIN32_FIND_DATA FileInfo;
              HANDLE hDir;

              // use last update date of base file for last update field
              hDir = FindFirstFile( pInfo->szFullPath, &FileInfo );
              if ( hDir != INVALID_HANDLE_VALUE )
              {
                FILETIME ftLocalTime;
                WORD wDate, wTime;

                FileTimeToLocalFileTime( &(FileInfo.ftLastWriteTime), &ftLocalTime );
                FileTimeToDosDateTime( &ftLocalTime, &wDate, &wTime );

                UtlFDateToDateString( (FDATE *)&wDate, szLastUpdate, sizeof(szLastUpdate)-1 );
                strcat( szLastUpdate, " " );
                UtlFTimeToTimeString( (FTIME *)&wTime, szLastUpdate + strlen(szLastUpdate), sizeof(szLastUpdate) - strlen(szLastUpdate) );

                FindClose( hDir );
              } 
              else
              {
                strcpy( szLastUpdate, "n/a" );
              }
            }
            else
            {
              strcpy( szLastUpdate, "n/a" );
            }

            SETTEXT( hDlg, ID_TM_PROP_UPD_EF, szLastUpdate );
          }

          // set number of segments static
          {
            ULONG ulProposals = pIDA->pMem->getProposalNum();
            char szProposals[20];
            sprintf( szProposals, "%lu", ulProposals );
            SETTEXT( hDlg, ID_TM_PROP_SEGS_EF, szProposals );
          }

          // fill document listbox
          {
            int iDocuments = pIDA->pMem->getNumOfDocumentNames();
            int i;

            for ( i = 0; i < iDocuments; i++ )
            {
              char szDocument[MAX_LONGFILESPEC];
              pIDA->pMem->getDocumentName( i, szDocument, sizeof(szDocument) );
              INSERTITEM( hDlg, ID_TM_PROP_DOCS_LB, szDocument );
            } /* endfor */
          }        

          // fill markup listbox
          {
            int iMarkups = pIDA->pMem->getNumOfMarkupNames();
            int i;

            for ( i = 0; i < iMarkups; i++ )
            {
              char szMarkup[MAX_FILESPEC];
              pIDA->pMem->getMarkupName( i, szMarkup, sizeof(szMarkup) );
              INSERTITEM( hDlg, ID_TM_PROP_MARKUP_LB, szMarkup );
            } /* endfor */
          }

          if ( pInfo != NULL ) free ( pInfo );
        } /* endif */

        // set focus to description entry field or dismiss dialog
        if ( fOK )
        {
          SETFOCUS( hDlg, ID_TM_PROP_DESCR_EF );
        }
        else
        {
          DelCtrlFont( hDlg, ID_TM_PROP_DESCR_EF );
          DISMISSDLG( hDlg, FALSE );
        } /* endif */
        mResult = MRFROMSHORT(DIALOGINITRETURN( TRUE ));
      }
      break;

      // ---------------------------------------------------------------------
    case WM_COMMAND:
      // handle the pushbuttons
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
    		case  ID_TM_PROP_HELP_PB:
		      mResult = UtlInvokeHelp();
		   break;
        case ID_TM_PROP_CHANGE_PB:
          {
            BOOL fOK      = TRUE;

            pIDA = ACCESSDLGIDA( hDlg, PMEM_PROP_IDA );

            // get new description text
            if ( fOK )
            {
              CHAR szDescr[MAX_MEM_DESCRIPTION];  // buffer for description

              QUERYTEXT( hDlg, ID_TM_PROP_DESCR_EF, szDescr );
              ANSITOOEM( szDescr );
              pIDA->pMem->setDescription( szDescr );
            } /* endif */

            DISMISSDLG( hDlg, TRUE );
          }
          break;

        case ID_TM_PROP_CANCEL_PB:
        case DID_CANCEL:                      // ESC key selected
          DelCtrlFont( hDlg, ID_TM_PROP_DESCR_EF );
          DISMISSDLG( hDlg, FALSE );
          break;
        case ID_TM_PROP_DESCR_EF:
          if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
          {
            ClearIME( hDlg );
          } /* endif */
          break;
      } /* end switch */
      break;
      // ---------------------------------------------------------------------
    default:
      // Call the default Dialog procedure
      mResult = UTLDEFDIALOGPROC( hDlg, usMessage, mp1, mp2);
      break;
  } /* end switch usMessage */

  return mResult;
} /* end of function MEMPROPDLG */


// create a new Translation Memory using the function I/F
USHORT MemFuncCreateMem
(
PSZ         pszMemName,              // name of new Translation Memory
PSZ         pszDescription,          // description for new Translation Memory or NULL
CHAR        chToDrive,               // target drive for new Translation Memory
PSZ         pszSourceLanguage,       // Translation Memory source language
LONG        lOptions                 // type of new Translation Memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pszParm;                 // pointer for error parameters
  char        szEmpty[2];
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  szEmpty[0] = EOS;

  // check required parameters
  if ( usRC == NO_ERROR )
  {
    if ( (pszMemName == NULL) || (*pszMemName == EOS) )
    {
      usRC = TMT_MANDCMDLINE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    if ( (pszSourceLanguage == NULL) || (*pszSourceLanguage == EOS) )
    {
      usRC = ERROR_NO_SOURCELANG;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check memory name syntax
  if ( usRC == NO_ERROR )
  {
    if ( !UtlCheckLongName( pszMemName ) )
    {
      pszParm = pszMemName;
      UtlErrorHwnd(  ERROR_INV_LONGNAME, MB_CANCEL, 1,
                     &pszParm, EQF_ERROR, HWND_FUNCIF );
      usRC = ERROR_MEM_NAME_INVALID;
    } /* endif */
  } /* endif */

  // Check if target drive is valid
  if ( (usRC == NO_ERROR) && (chToDrive != EOS) )
  {
    if ( lOptions & SHARED_OPT )
    {
      CHAR szLANDrives[MAX_DRIVELIST];   // buffer for LAN drive letters

      // Get LAN drives
      UtlGetLANDriveList( (PBYTE)szLANDrives );

      // Check if specified target drive is in list of valid drives
      if ( strchr( szLANDrives, toupper(chToDrive) ) == NULL )
      {
        CHAR szDrive[MAX_DRIVE];

        szDrive[0] = chToDrive;
        szDrive[1] = COLON;
        szDrive[2] = EOS;
        pszParm = szDrive;
        usRC = ERROR_LAN_DRIVE_NOT_VALID;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }
    else
    {
      CHAR szEqfDrives[MAX_DRIVELIST];   // buffer for EQF drive letters

      // Get local EQF drives
      UtlGetCheckedEqfDrives( szEqfDrives );

      // Check if specified target drive is in list of valid drives
      if ( strchr( szEqfDrives, toupper(chToDrive) ) == NULL )
      {
        CHAR szDrive[MAX_DRIVE];

        szDrive[0] = chToDrive;
        szDrive[1] = COLON;
        szDrive[2] = EOS;
        pszParm = szDrive;
        usRC = ERROR_EQF_DRIVE_NOT_VALID;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF  );
      } /* endif */
    } /* endif */
  } /* endif */

  // check if TM exists already
  if ( usRC == NO_ERROR )
  {
    if ( pFactory->exists( NULL, pszMemName ) )
    {
      UtlErrorHwnd( ERROR_MEM_NAME_INVALID, MB_CANCEL, 1, &pszMemName, EQF_ERROR, HWND_FUNCIF );
      usRC = ERROR_MEM_NAME_INVALID;
    }
  } /* endif */

  // check if source language is valid
  if ( usRC == NO_ERROR )
  {
    if ( !UtlCheckIfExist( pszSourceLanguage, SOURCE_LANGUAGE_OBJECT ) )
    {
      usRC = ERROR_PROPERTY_LANG_DATA;
      UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                    &pszSourceLanguage, EQF_ERROR, HWND_FUNCIF  );
    } /* endif */
  } /* endif */


  // Check specified options
  if ( usRC == NO_ERROR)
  {
    if ( lOptions == 0L )
    {
      lOptions = LOCAL_OPT;
    }
    else if ( lOptions == LOCAL_OPT )
    {
      // O.K. option is valid
    }
    else if ( lOptions == SHARED_OPT )
    {
      // O.K. option is valid
    }
    else
    {
      usRC = WRONG_OPTIONS_RC;
      UtlErrorHwnd( ERROR_INTERNAL, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF  );
    } /* endif */
  } /* endif */


  // create memory database
  BOOL fOK = (usRC==NO_ERROR ? TRUE : FALSE);
  OtmMemory     *pMemory  = NULL; 

  if (fOK)
  {
    int iRC;
    char szPlugin[256];

    // select the correct memory plugin
    if ( lOptions == SHARED_OPT )
    {
      // using this interface we can only use the LAN-based shared memory plugin...
      strcpy( szPlugin, "EqfSharedOnLanMemoryPlugin" );
    }
    else
    {
      // use the default memory plugin
      strcpy( szPlugin, pFactory->getDefaultMemoryPlugin() );
    }


    // create the memory
    pMemory = pFactory->createMemory( szPlugin, pszMemName, ( pszDescription == NULL ) ? szEmpty : pszDescription, pszSourceLanguage, chToDrive, &iRC );
    if ( pMemory == NULL )
    {
      fOK = FALSE;
    } 
  } 

  // get memory object name
  if ( fOK )
  {
    char szObjName[MAX_LONGFILESPEC];

     pFactory->getObjectName( pMemory, szObjName, sizeof(szObjName) );

      // notify creation success
      HWND hwnd = FindWindow( TWBMAIN ,NULL);
      if(hwnd != NULL)
      {
        COPYDATASTRUCT cds;
        cds.dwData = 1;
        cds.cbData = sizeof( szObjName );
        cds.lpData =  szObjName;
        SendMessage(hwnd,WM_COPYDATA, MP1FROMCHAR(chToDrive),(LPARAM)(LPVOID)&cds);
      }
  } 

  //--- Tm is created. Close it. 
  if ( pMemory != NULL )
  {

    pFactory->closeMemory( pMemory );
    pMemory = NULL;
  }
  
  usRC = (fOK ? 0 : 1);

  return( usRC );
} /* end of function MemFuncCreateMem */
