//------------------------------------------------------------------------------
// EQFNTM.C
//------------------------------------------------------------------------------
// Description: TranslationMemory interface functions.
//------------------------------------------------------------------------------
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
// Entry Points:
// C TmCreate
// C TmOpen
// C TmClose
// C TmReplace
// C TmGet
// C TmExtract
// C TmInfo
// C TmDeleteTm
// C TmDeleteSegment
//------------------------------------------------------------------------------
// Externals:
//------------------------------------------------------------------------------
// Internals:
// C NTMGetThresholdFromProperties
// C NTMFillCreateInStruct
//------------------------------------------------------------------------------

/**********************************************************************/
/* needed header files                                                */
/**********************************************************************/
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#include <eqf.h>                  // General Translation Manager include file

#include <EQFSETUP.H>             // Directory name defines and other
#include "EQFDDE.H"               // Batch mode definitions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFEVENT.H>             // Event logging

#ifdef _DEBUG
  //#define SESSIONLOG
#endif
/**********************************************************************/
/* prototypes for internal functions                                  */
/**********************************************************************/
USHORT
C_TmCreate( PSZ         pszPathMem,      //(in)  full TM name x:\eqf\mem\mem.tmd
          HTM         *htm,            //(out) TM handle
          HTM         hModel,          //(in)  model handle
          PSZ         pszServer,       //(in)  server name or empty string
          PSZ         pszUserID,       //(in)  LAN USERID or empty string
          PSZ         pszSourceLang,   //(in)  source language or empty string
          PSZ         pszDescription,  //(in)  TM description or empty string
          USHORT      usMsgHandling,   //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
          HWND        hwnd );           //(in)  handle for error messages
USHORT
C_TmOpen( PSZ        szMemFullPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
        HTM        *phtm,              //(out) TM handle
        USHORT     usAccess,           //(in)  access mode: NON_EXCLUSIVE
                                       //                   EXCLUSIVE
                                       //                   FOR_ORGANIZE
        USHORT     usLocation,         //(in)  location:    TM_LOCAL
                                       //                   TM_REMOTE
                                       //                   TM_LOCAL_REMOTE
        USHORT     usMsgHandling,      //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
        HWND       hwnd );              //(in)  handle for error messages
USHORT
C_TmClose( HTM        htm,               //(in) TM handle returned from open
         PSZ        szMemPath,         //(in) full TM name x:\eqf\mem\mem.tmd
         USHORT     usMsgHandling,     //(in) message handling parameter
                                   //     TRUE:  display error message
                                   //     FALSE: display no error message
         HWND       hwnd );         //(in) handle for error messages
USHORT
C_TmReplaceHwnd( HTM,           //(in)  TM handle
           PSZ,           //(in)  full TM name x:\eqf\mem\mem.mem
           PTMX_PUT_IN,   //(in)  pointer to put input structure
           PTMX_PUT_OUT,  //(out) pointer to put output structure
           USHORT,        //(in)  message handling parameter
                          //      TRUE:  display error message
                          //      FALSE: display no error message
           HWND );                     //(in) handle for error messages

USHORT
C_TmReplaceHwndW( HTM       htm,            //(in)  TM handle
           PSZ            szMemPath,      //(in)  full TM name x:\eqf\mem\mem
           PTMX_PUT_IN_W  pstPutIn,       //(in)  pointer to put input structure
           PTMX_PUT_OUT_W pstPutOut,      //(out) pointer to put output structure
           USHORT         usMsgHandling,  //(in)  message handling parameter
                                          //      TRUE:  display error message
                                          //      FALSE: display no error message
           HWND           hwnd );          //(in)  handle for error messages

USHORT
C_TmExtractHwnd( HTM,
           PSZ,
           PTMX_EXT_IN,
           PTMX_EXT_OUT,
           USHORT,
           HWND );                     //(in) handle for error messages

USHORT
C_TmExtractHwndW
  ( HTM          htm,              //(in)  TM handle
    PSZ          szMemPath,        //(in)  full TM name x:\eqf\mem\mem.tmd
    PTMX_EXT_IN_W  pstExtIn,       //(in)  pointer to extract input structure
    PTMX_EXT_OUT_W pstExtOut,      //(out) pointer to extract output structure
    USHORT       usMsgHandling,    //(in)  message handling parameter
                                   //      TRUE:  display error message
                                   //      FALSE: display no error message
    HWND         hwnd );            //(in)  handle for error messages

USHORT
C_TmGetW(HTM            htm,             //(in)  TM handle
       PSZ            szMemPath,       //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN_W  pstGetIn,        //(in)  pointer to get input structure
       PTMX_GET_OUT_W pstGetOut,       //(out) pointer to get output structure
       USHORT         usMsgHandling );  //(in)  message handling parameter

USHORT C_TmInfoHwnd( HTM, PSZ, USHORT, PTMX_INFO_IN, PTMX_INFO_OUT, USHORT, HWND );

USHORT C_TmUpdSegHwnd
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling,             //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
  HWND        hwnd                       //(in)  handle for error messages
);

USHORT C_TmUpdSegHwndW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling,             //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
  HWND        hwnd                       //(in)  handle for error messages
);

USHORT
NTMFillCreateInStruct( HTM,
                       PSZ,
                       PSZ,
                       PSZ,
                       PSZ,
                       PSZ,
                       PTMX_CREATE_IN,
                       USHORT, HWND );
USHORT
NTMGetThresholdFromProperties( PSZ,
                               PUSHORT,
                               USHORT );



// utility to get the property file extension
char *GetPropFileExtension( char *pszMemPath )
{
  char *pszExt = strrchr( pszMemPath, '.' );
  if ( pszExt != NULL )
  {
    if ( strcmp( pszExt, EXT_OF_SHARED_MEM ) == 0 )
    {
      return( LANSHARED_MEM_PROP );
    }
    else
    {
      return( EXT_OF_MEM );
    }
  }
  else
  {
    return( EXT_OF_MEM );
  }
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmCreate
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmCreate( PSZ         pszPathMem,
//                              HTM         *htm,
//                              HTM         hModel,
//                              PSZ         pszServer,
//                              PSZ         pszUserID,
//                              PSZ         pszSourceLang,
//                              PSZ         pszDescription,
//                              USHORT      usMsgHandling  )
//------------------------------------------------------------------------------
// Description:       TM interface function to create a TM
//------------------------------------------------------------------------------
// Parameters:        pszPathMem     - (in)  full TM name x:\eqf\mem\mem.tmd
//                    *htm           - (out) TM handle
//                    hModel         - (in)  model handle
//                    pszServer      - (in)  server name or empty string
//                    pszUserID      - (in)  LAN USERID or empty string
//                    pszSourceLang  - (in)  source language or empty string
//                    pszDescription - (in)  TM description or empty string
//                    usMsgHandling  - (in)  message handling parameter
//                                           TRUE:  display error message
//                                           FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
TmCreate( PSZ         pszPathMem,      //(in)  full TM name x:\eqf\mem\mem.tmd
          HTM         *htm,            //(out) TM handle
          HTM         hModel,          //(in)  model handle
          PSZ         pszServer,       //(in)  server name or empty string
          PSZ         pszUserID,       //(in)  LAN USERID or empty string
          PSZ         pszSourceLang,   //(in)  source language or empty string
          PSZ         pszDescription,  //(in)  TM description or empty string
          USHORT      usMsgHandling,   //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
          HWND        hwnd )           //(in)  handle for error messages
{
	return C_TmCreate(pszPathMem, htm, hModel, pszServer, pszUserID, pszSourceLang, pszDescription,
							usMsgHandling, hwnd);
}
USHORT
C_TmCreate( PSZ         pszPathMem,      //(in)  full TM name x:\eqf\mem\mem.tmd
          HTM         *htm,            //(out) TM handle
          HTM         hModel,          //(in)  model handle
          PSZ         pszServer,       //(in)  server name or empty string
          PSZ         pszUserID,       //(in)  LAN USERID or empty string
          PSZ         pszSourceLang,   //(in)  source language or empty string
          PSZ         pszDescription,  //(in)  TM description or empty string
          USHORT      usMsgHandling,   //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
          HWND        hwnd )           //(in)  handle for error messages
{
  PTMX_CREATE_IN   pstCreateIn = NULL;    //pointer to create input structure
  PTMX_CREATE_OUT  pstCreateOut = NULL;   //pointer to create output structure
  BOOL             fOk = TRUE;            //processing flag
  USHORT           usRc = ERROR_NOT_ENOUGH_MEMORY; //function return code

  /********************************************************************/
  /* initialize TM handle                                             */
  /********************************************************************/
  *htm = NULLHANDLE;

  /********************************************************************/
  /* allocate storage for TMX_CREATE_IN and TMX_CREATE_OUT            */
  /********************************************************************/
  fOk = UtlAlloc( (PVOID *) &pstCreateIn, 0L,
                  (LONG)( sizeof( TMX_CREATE_IN ) +
                          sizeof( TMX_CREATE_OUT ) ),
                  FALSE );

  if ( fOk )
  {
    /******************************************************************/
    /* assign memory to pointer pstCreateOut                          */
    /******************************************************************/
    pstCreateOut = (PTMX_CREATE_OUT)(pstCreateIn + 1);

    /******************************************************************/
    /* call function to fill TMC_CREATE_IN structure                  */
    /******************************************************************/
    usRc = NTMFillCreateInStruct( hModel,
                                  pszPathMem,
                                  pszServer,
                                  pszUserID,
                                  pszSourceLang,
                                  pszDescription,
                                  pstCreateIn,
                                  usMsgHandling,
                                  hwnd );
    if ( usRc )
    {
      /****************************************************************/
      /* error from NTMFillCreateInStruct                             */
      /* stop further processing                                      */
      /* set usMsgHandling  to FALSE because error message is already */
      /* displayed by NTMFillCreateInStruct                           */
      /****************************************************************/
      fOk = FALSE;
      usMsgHandling = FALSE;
    } /* endif */

    if ( fOk )
    {
      /****************************************************************/
      /* call U code to pass TM command to server or handle it local  */
      /****************************************************************/
      usRc = TmtXCreate( pstCreateIn, pstCreateOut );
//    usRc = U( htmDummy,
//             (PXIN) pstCreateIn,
//             (PXOUT) pstCreateOut,
//             NEW_TM );

      if ( !usRc )
      {
        /**************************************************************/
        /* no error,  return pointer to TM CLB as handle              */
        /**************************************************************/
        *htm = (HTM)pstCreateOut->pstTmClb;
      }
      else
      {
        /**************************************************************/
        /* error stop further processing                              */
        /**************************************************************/
        fOk = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if an error occured call MemRcHandling in dependency  of the     */
  /* mesage flag to display an error message                          */
  /********************************************************************/
  if ( !fOk )
  {
    if ( usMsgHandling )
    {
      usRc = MemRcHandlingHwnd( usRc, pszPathMem, htm, pszServer, hwnd );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if memory for TMX_CREATE_IN and TMX_CREATE_OUT was allocated     */
  /* free the memory                                                  */
  /********************************************************************/
  if ( pstCreateIn )
  {
    UtlAlloc( (PVOID *) &pstCreateIn, 0L, 0L, NOMSG );
  } /* endif */

  return usRc;
} /* End of function TmCreate */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmOpen
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmOpen( PSZ        szMemFullPath,
//                            HTM        *phtm,
//                            USHORT     usAccess,
//                            USHORT     usLocation,
//                            USHORT     usMsgHandling )
//------------------------------------------------------------------------------
// Description:       TM interface function to open a TM
//------------------------------------------------------------------------------
// Parameters:        szMemFullPath  - (in)  full TM name x:\eqf\mem\mem.tmd
//                    *phtm          - (out) TM handle
//                    usAccess       - (in)  access mode: NON_EXCLUSIVE
//                                                        EXCLUSIVE
//                                                        FOR_ORGANIZE
//                    usLocation     - (in)  location:    TM_LOCAL
//                                                        TM_REMOTE
//                                                        TM_LOCAL_REMOTE
//                    usMsgHandling  - (in)  message handling parameter
//                                           TRUE:  display error message
//                                           FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
TmOpen( PSZ        szMemFullPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
        HTM        *phtm,              //(out) TM handle
        USHORT     usAccess,           //(in)  access mode: NON_EXCLUSIVE
                                       //                   EXCLUSIVE
                                       //                   FOR_ORGANIZE
        USHORT     usLocation,         //(in)  location:    TM_LOCAL
                                       //                   TM_REMOTE
                                       //                   TM_LOCAL_REMOTE
        USHORT     usMsgHandling,      //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
        HWND       hwnd )              //(in)  handle for error messages
{
	return C_TmOpen(szMemFullPath,
        phtm,
        usAccess,
        usLocation,
        usMsgHandling,
        hwnd );
}

USHORT
C_TmOpen( PSZ        szMemFullPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
        HTM        *phtm,              //(out) TM handle
        USHORT     usAccess,           //(in)  access mode: NON_EXCLUSIVE
                                       //                   EXCLUSIVE
                                       //                   FOR_ORGANIZE
        USHORT     usLocation,         //(in)  location:    TM_LOCAL
                                       //                   TM_REMOTE
                                       //                   TM_LOCAL_REMOTE
        USHORT     usMsgHandling,      //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
        HWND       hwnd )              //(in)  handle for error messages
{
  USHORT            usRc;              //function return code
  BOOL              fOk;               //process flag
  PTMX_OPEN_IN      pstOpenIn  = NULL; //open input structure
  PTMX_OPEN_OUT     pstOpenOut = NULL; //open output structure
  USHORT            usUserPriviliges;  //returned from UtlGetLANUserID
  PSZ               pszTemp;           //temp ptr for UtlGetFnameFromPath

  DEBUGEVENT( TMOPEN_LOC, FUNCENTRY_EVENT, 0 );

  /********************************************************************/
  /* initialize function return code                                  */
  /********************************************************************/
  usRc = ERROR_NOT_ENOUGH_MEMORY;

  /********************************************************************/
  /* initialize TM handle                                             */
  /********************************************************************/
  *phtm = NULLHANDLE;

  /********************************************************************/
  /* allocate storage for TMX_OPEN_IN and TMX_OPEN_OUT                */
  /********************************************************************/
  fOk = UtlAlloc( (PVOID *) &pstOpenIn, 0L,
                  (LONG)( sizeof( TMX_OPEN_IN ) +
                          sizeof( TMX_OPEN_OUT ) ),
                  FALSE );
  if ( fOk )
  {
    /******************************************************************/
    /* set usRc to NO_ERROR                                           */
    /******************************************************************/
    usRc = NO_ERROR;

    /******************************************************************/
    /* assign memory to pointer pstOpenOut                           */
    /******************************************************************/
    pstOpenOut = (PTMX_OPEN_OUT)(pstOpenIn + 1);

    if ( usLocation != TM_LOCAL )
    {
      /****************************************************************/
      /* TM may be LOCAL or LOCALREMOTE                               */
      /* it must be determined if it is a local or remoted TM         */
      /* for this a query is send to the TM handler                   */
      /****************************************************************/
      /****************************************************************/
      /* split the TM name form the path                              */
      /****************************************************************/
      pszTemp = UtlGetFnameFromPath( szMemFullPath );

      if ( pszTemp )
      {
        /**************************************************************/
        /* file name found in full TM path                            */
        /* get the server of the TM                                   */
        /**************************************************************/
        Utlstrccpy( pstOpenIn->stTmOpen.szServer,
                    pszTemp,
                    DOT );
        if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
        {
          // no server in function call IF
          pstOpenIn->stTmOpen.szServer[0] = EOS;
        }
        else
        {
          EqfSend2Handler ( MEMORYHANDLER,
                            WM_EQF_PROCESSTASK,
                            MP1FROMSHORT( TM_QUERY_SERVER_TASK ),
                            MP2FROMP( pstOpenIn->stTmOpen.szServer ) );
        } /* endif */

        /**************************************************************/
        /* because usLocation mybe TM_LOCALREMOTE, check return from  */
        /* previous message to see if it is really a remote TM        */
        /**************************************************************/
        if ( pstOpenIn->stTmOpen.szServer[0] != EOS )
        {
          /****************************************************************/
          /* remote processing currently not implemented for the new TM   */
          /* approach !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!                      */
          /****************************************************************/
          usRc = ERROR_REMOTE_TM_NOT_SUPPORTED;
          fOk  = FALSE;
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* NULL pointer in pszTemp, means internal programming error  */
        /* stop further processing and set usRc                       */
        /**************************************************************/
        fOk = FALSE;
        usRc = ERROR_INTERNAL;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* set server string to EOS for correct processing of local TM  */
      /****************************************************************/
      pstOpenIn->stTmOpen.szServer[0] = EOS;
    } /* endif */

    if ( fOk )
    {
      /****************************************************************/
      /* fill the TMX_OPEN_IN structure                               */
      /* stPrefixIn.usLengthInput                                     */
      /* stPrefixIn.usTmCommand                                       */
      /* stTmOpen.szDataName                                          */
      /* stTmOpen.szIndexName                                         */
      /* stTmOpen.szServer                                            */
      /* stTmOpen.szUserID                                            */
      /* stTmOpen.usAccess                                            */
      /* stTmOpen.usThreshold                                         */
      /****************************************************************/
      pstOpenIn->stPrefixIn.usLengthInput = sizeof( TMX_OPEN_IN );
      pstOpenIn->stPrefixIn.usTmCommand = TMC_OPEN;
      strcpy( pstOpenIn->stTmOpen.szDataName, szMemFullPath );
      Utlstrccpy( pstOpenIn->stTmOpen.szIndexName, szMemFullPath, DOT );
      INDEXNAMEFROMMEMPATH( szMemFullPath, pstOpenIn->stTmOpen.szIndexName );
      pstOpenIn->stTmOpen.szServer[0] = EOS;
      UtlGetLANUserID( pstOpenIn->stTmOpen.szUserid, &usUserPriviliges, FALSE );
      pstOpenIn->stTmOpen.usAccess = usAccess;

      /****************************************************************/
      /* if a source TM should be opened during folder import and it  */
      /* is the source TM which contains the IMPORT path it is not    */
      /* needed to get the threshold from the properties because      */
      /* for this TM only TmExtract calls are used.                   */
      /* For this the threshold is not needed.                        */
      /****************************************************************/
      if ( strstr( szMemFullPath, IMPORTDIR ) == NULL )
      {
        /**************************************************************/
        /* IMPORTPATH not found                                       */
        /* call function to get threshold valur from the TM properties  */
        /****************************************************************/
        if ( (usRc = NTMGetThresholdFromProperties
                       ( szMemFullPath,
                        &pstOpenIn->stTmOpen.usThreshold,
                        usMsgHandling)) != NO_ERROR )
        {
          /**************************************************************/
          /* error from NTMGetThresholdFromProperties                   */
          /* stop further processing                                    */
          /* set usMsgHandling to FALSE because in case of an error     */
          /* the message is displayed by NTMGetThresholdFromProperties  */
          /**************************************************************/
          fOk           = FALSE;
          usMsgHandling = FALSE;
          DEBUGEVENT( TMOPEN_LOC, STATE_EVENT, 1 );
        } /* endif */
      } /* endif */

      if ( fOk )
      {
        /**************************************************************/
        /* call U code to pass TM command to server or handle it local*/
        /**************************************************************/
        usRc = TmtXOpen ( pstOpenIn, pstOpenOut );
//      usRc = U( *phtm,
//                (PXIN)pstOpenIn,
//                (PXOUT)pstOpenOut,
//                NEW_TM );

        /**************************************************************/
        /* return pointer to TM CLB as handle                         */
        /* pstOpenOut->pstTmClb is a NULL pointer in error case       */
        /**************************************************************/
        *phtm = pstOpenOut->pstTmClb;
        switch ( usRc )
        {
          //-------------------------------------------------------------------
          case NO_ERROR:
            break;
          //-------------------------------------------------------------------
          case BTREE_CORRUPTED:
          case VERSION_MISMATCH :
            break;
          //-------------------------------------------------------------------
          default:
            /**********************************************************/
            /* set TM handle to NULL means that the TM is closed      */
            /* and stop further processing                            */
            /**********************************************************/
            *phtm = NULLHANDLE;
             fOk = FALSE;
            break;
        } /* endswitch */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* if memory for TMX_OPEN_IN and TMX_OPEN_OUT was allocated       */
    /* free the memory                                                */
    /******************************************************************/
    if ( pstOpenIn )
    {
      UtlAlloc( (PVOID *) &pstOpenIn, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if an error occured and message handling flag is set             */
  /* call MemRcHandling to display error message                      */
  /********************************************************************/
  if ( usMsgHandling && usRc )
  {
    /******************************************************************/
    /* display error message                                          */
    /******************************************************************/
    DEBUGEVENT( TMOPEN_LOC, STATE_EVENT, 2 );
    DEBUGEVENT( TMOPEN_LOC, ERROR_EVENT, usRc );
    usRc = MemRcHandlingHwnd( usRc, szMemFullPath, phtm, NULL, hwnd );
    switch ( usRc )
    {
        case BTREE_CORRUPTED:
        case VERSION_MISMATCH :
          if ( *phtm )
          {
             TmClose( *phtm, szMemFullPath, FALSE, 0 );
          } /* endif */
          *phtm = NULLHANDLE;
          break;
      default:
        break;
    } /* endswitch */
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMOPEN_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* end of function TmOpen */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmClose
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmClose( HTM        htm,
//                             PSZ        szMemPath,
//                             USHORT     usMsgHandling )
//------------------------------------------------------------------------------
// Description:       TM interface function to close a TM
//------------------------------------------------------------------------------
// Parameters:        htm           - (in) TM handle returned from open
//                    szMemPath     - (in) full TM name x:\eqf\mem\mem.tmd
//                    usMsgHandling - (in) message handling parameter
//                                         TRUE:  display error message
//                                         FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
TmClose( HTM        htm,               //(in) TM handle returned from open
         PSZ        szMemPath,         //(in) full TM name x:\eqf\mem\mem.tmd
         USHORT     usMsgHandling,     //(in) message handling parameter
                                   //     TRUE:  display error message
                                   //     FALSE: display no error message
         HWND       hwnd )         //(in) handle for error messages
{
	return C_TmClose(htm, szMemPath, usMsgHandling, hwnd);
}
USHORT
C_TmClose( HTM        htm,               //(in) TM handle returned from open
         PSZ        szMemPath,         //(in) full TM name x:\eqf\mem\mem.tmd
         USHORT     usMsgHandling,     //(in) message handling parameter
                                   //     TRUE:  display error message
                                   //     FALSE: display no error message
         HWND       hwnd )         //(in) handle for error messages
{
  USHORT            usRc;               //function return code
  PTMX_CLOSE_IN     pstCloseIn  = NULL; //close input structure
  PTMX_CLOSE_OUT    pstCloseOut = NULL; //close output structure
  BOOL              fOk;                //process flag
  USHORT            usQRc;              //rc from EqfSend2Handler
  SERVERNAME        szServer;           //local var for server name

  ERREVENT( TMCLOSE_LOC, ERROR_EVENT, 0 );

  /********************************************************************/
  /* initialze function return code                                   */
  /********************************************************************/
  usRc = ERROR_NOT_ENOUGH_MEMORY;

  /********************************************************************/
  /* do processing only when a valid TM handle is passed              */
  /********************************************************************/
  if ( htm != NULLHANDLE )
  {
    /******************************************************************/
    /* allocate storage for TMX_CLOSE_IN and TMX_CLOSE_OUT            */
    /******************************************************************/
    fOk = UtlAlloc( (PVOID *) &pstCloseIn, 0L,
                    (LONG)( sizeof( TMX_CLOSE_IN ) +
                            sizeof( TMX_CLOSE_OUT ) ),
                    FALSE );
    if ( fOk )
    {
      /****************************************************************/
      /* assign memory to pointer pstCloseOut                         */
      /****************************************************************/
      pstCloseOut = (PTMX_CLOSE_OUT)(pstCloseIn + 1);

      /****************************************************************/
      /* fill the TMX_CLOSE_IN structure                              */
      /* stPrefixIn.usLengthInput                                     */
      /* stPrefixIn.usTmCommand                                       */
      /* stTmClb                                                      */
      /****************************************************************/
      pstCloseIn->stPrefixIn.usLengthInput = sizeof( TMX_CLOSE_IN );
      pstCloseIn->stPrefixIn.usTmCommand   = TMC_CLOSE;

      /****************************************************************/
      /* call U code to pass TM command to server or handle it local  */
      /****************************************************************/
      usRc = TmtXClose ( (PTMX_CLB)htm, pstCloseIn, pstCloseOut );

      /****************************************************************/
      /* if an error occured call MemRcHandling in dependency of      */
      /* the message flag to display error message                    */
      /****************************************************************/
      if ( usMsgHandling && usRc )
      {
        usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, NULL, hwnd );
      } /* endif */

      // cleanup
      UtlAlloc( (PVOID *) &pstCloseIn, 0L, 0L, NOMSG );
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* no valid TM handle was passed (handle is NULL)                 */
    /* handle this as no error                                        */
    /******************************************************************/
    usRc = NO_ERROR;
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMCLOSE_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* end of function TmClose */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmReplace
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmReplace( HTM              htm,
//                               PSZ              szMemPath,
//                               PTMX_PUT_IN      pstPutIn,
//                               PTMX_PUT_OUT     pstPutOut,
//                               USHORT           usMsgHandling )
//------------------------------------------------------------------------------
// Description:       TM interface function to put a segment to TM
//------------------------------------------------------------------------------
// Parameters:        htm           - (in)  TM handle
//                    szMemPath     - (in)  full TM name x:\eqf\mem\mem.tmd
//                    pstPutIn      - (in)  pointer to put input structure
//                    pstPutOut     - (out) pointer to put output structure
//                    usMsgHandling - (in)  message handling parameter
//                                          TRUE:  display error message
//                                          FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
C_TmReplace( HTM           htm,            //(in)  TM handle
           PSZ           szMemPath,      //(in)  full TM name x:\eqf\mem\mem
           PTMX_PUT_IN   pstPutIn,       //(in)  pointer to put input structure
           PTMX_PUT_OUT  pstPutOut,      //(out) pointer to put output structure
           USHORT        usMsgHandling ) //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
{
  return( C_TmReplaceHwnd( htm, szMemPath, pstPutIn, pstPutOut, usMsgHandling,
                         NULLHANDLE ) );
} /* end of function TmReplace */
USHORT
TmReplace( HTM           htm,            //(in)  TM handle
           PSZ           szMemPath,      //(in)  full TM name x:\eqf\mem\mem
           PTMX_PUT_IN   pstPutIn,       //(in)  pointer to put input structure
           PTMX_PUT_OUT  pstPutOut,      //(out) pointer to put output structure
           USHORT        usMsgHandling ) //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
{
  return( C_TmReplace( htm, szMemPath, pstPutIn, pstPutOut, usMsgHandling ) );
} /* end of function TmReplace */



USHORT
C_TmReplaceW( HTM           htm,            //(in)  TM handle
           PSZ            szMemPath,      //(in)  full TM name x:\eqf\mem\mem
           PTMX_PUT_IN_W  pstPutInW,      //(in)  pointer to put input structure
           PTMX_PUT_OUT_W pstPutOutW,     //(out) pointer to put output structure
           USHORT         usMsgHandling ) //(in)  message handling parameter
                                          //      TRUE:  display error message
                                          //      FALSE: display no error message
{
  return( C_TmReplaceHwndW( htm, szMemPath, pstPutInW, pstPutOutW, usMsgHandling,
                         NULLHANDLE ) );
} /* end of function TmReplace */
USHORT
TmReplaceW( HTM           htm,            //(in)  TM handle
           PSZ            szMemPath,      //(in)  full TM name x:\eqf\mem\mem
           PTMX_PUT_IN_W  pstPutInW,      //(in)  pointer to put input structure
           PTMX_PUT_OUT_W pstPutOutW,     //(out) pointer to put output structure
           USHORT         usMsgHandling ) //(in)  message handling parameter
                                          //      TRUE:  display error message
                                          //      FALSE: display no error message
{
  return( C_TmReplaceW( htm, szMemPath, pstPutInW, pstPutOutW, usMsgHandling ) );
} /* end of function TmReplace */

USHORT
C_TmReplaceHwnd( HTM      htm,            //(in)  TM handle
           PSZ          szMemPath,      //(in)  full TM name x:\eqf\mem\mem
           PTMX_PUT_IN  pstPutIn,       //(in)  pointer to put input structure
           PTMX_PUT_OUT pstPutOut,      //(out) pointer to put output structure
           USHORT       usMsgHandling,  //(in)  message handling parameter
                                        //      TRUE:  display error message
                                        //      FALSE: display no error message
           HWND         hwnd )          //(in)  handle for error messages
{
  USHORT usRc;
  PTMX_PUT_IN_W  pstPutInW;
  PTMX_PUT_OUT_W pstPutOutW;
  ULONG   ulCP = 0L;

  ulCP = GetLangOEMCP( NULL);

  UtlAlloc( (PVOID *) &pstPutInW,  0L, sizeof( TMX_PUT_IN_W ), NOMSG );
  UtlAlloc( (PVOID *) &pstPutOutW, 0L, sizeof( TMX_PUT_OUT_W ), NOMSG );

  // copy structures
  TMX_PUT_IN_ASCII2Unicode( pstPutIn, pstPutInW, ulCP );


  usRc = TmReplaceHwndW( htm, szMemPath,  pstPutInW, pstPutOutW, usMsgHandling, hwnd );
  // copy output structures back
  TMX_PUT_OUT_Unicode2ASCII( pstPutOutW, pstPutOut );

  UtlAlloc( (PVOID *) &pstPutInW,  0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pstPutOutW, 0L, 0L, NOMSG );

  return usRc;
}

USHORT
TmReplaceHwndW( HTM       htm,            //(in)  TM handle
           PSZ            szMemPath,      //(in)  full TM name x:\eqf\mem\mem
           PTMX_PUT_IN_W  pstPutIn,       //(in)  pointer to put input structure
           PTMX_PUT_OUT_W pstPutOut,      //(out) pointer to put output structure
           USHORT         usMsgHandling,  //(in)  message handling parameter
                                          //      TRUE:  display error message
                                          //      FALSE: display no error message
           HWND           hwnd )          //(in)  handle for error messages
{
	return C_TmReplaceHwndW(htm,
           szMemPath,
           pstPutIn,
           pstPutOut,
           usMsgHandling,
           hwnd );
}
USHORT
C_TmReplaceHwndW( HTM       htm,            //(in)  TM handle
           PSZ            szMemPath,      //(in)  full TM name x:\eqf\mem\mem
           PTMX_PUT_IN_W  pstPutIn,       //(in)  pointer to put input structure
           PTMX_PUT_OUT_W pstPutOut,      //(out) pointer to put output structure
           USHORT         usMsgHandling,  //(in)  message handling parameter
                                          //      TRUE:  display error message
                                          //      FALSE: display no error message
           HWND           hwnd )          //(in)  handle for error messages
{
  USHORT     usRc;                       //function return code
  USHORT     usQRc;                      //rc from EqfSend2Handler
  SERVERNAME szServer;                   //local var for server name

  DEBUGEVENT( TMREPLACE_LOC, FUNCENTRY_EVENT, 0 );

  /********************************************************************/
  /* fill the TMX_PUT_IN prefix structure                             */
  /* stPrefixIn.usLengthInput                                         */
  /* stPrefixIn.usTmCommand                                           */
  /* the TMX_PUT_IN structure must not be filled it is provided       */
  /* by the caller                                                    */
  /********************************************************************/
  pstPutIn->stPrefixIn.usLengthInput = sizeof( TMX_PUT_IN_W );
  pstPutIn->stPrefixIn.usTmCommand   = TMC_REPLACE;

  /********************************************************************/
  /* call TmtXReplace                                                 */
  /********************************************************************/
  usRc = TmtXReplace ( (PTMX_CLB)htm, pstPutIn, pstPutOut );

  /********************************************************************/
  /* if an error occured call MemRcHandling in dependency of          */
  /* the message flag to display error message                        */
  /********************************************************************/
  if ( usMsgHandling && usRc )
  {
    /**************************************************************/
    /* get either server of TM for the error message or pass on   */
    /* the tagtable name (depending on usRc)                      */
    /**************************************************************/
    if ( usRc == ERROR_TA_ACC_TAGTABLE )
    {
      CHAR szTagTable[MAX_EQF_PATH];
      strcpy(szTagTable, pstPutIn->stTmPut.szTagTable);
      strcat( szTagTable, EXT_OF_FORMAT );
      usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, szTagTable, hwnd );
    }
    else
    {
     usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, NULL, hwnd );
    } /* endif */
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMREPLACE_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* End of function TmReplace */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmUpdSegm    Update segment data
//------------------------------------------------------------------------------
// Description:       Updates the segment data of a specific segment
//------------------------------------------------------------------------------
// Function call:     TmUpdSign( HTM              htm,
//                               PSZ              szMemPath,
//                               PTMX_PUT_IN      pstPutIn,
//                               ULONG            ulUpdKey,
//                               USHORT           usUpdTarget,
//                               USHORT           usFlags
//                               USHORT           usMsgHandling )
//------------------------------------------------------------------------------
// Description:       TM interface function to put a segment to TM
//------------------------------------------------------------------------------
// Parameters:        htm           - (in)  TM handle
//                    szMemPath     - (in)  full TM name x:\eqf\mem\mem.tmd
//                    pstPutIn      - (in)  pointer to put input structure
//                    pstPutOut     - (out) pointer to put output structure
//                    usMsgHandling - (in)  message handling parameter
//                                          TRUE:  display error message
//                                          FALSE: display no error message
//------------------------------------------------------------------------------
USHORT C_TmUpdSeg
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
)
{
  return( C_TmUpdSegHwnd( htm, szMemPath, pstPutIn, ulUpdKey, usUpdTarget,
                        usFlags, usMsgHandling, NULLHANDLE ) );
} /* end of function TmUpdSeg */
USHORT TmUpdSeg
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
)
{
  return( C_TmUpdSeg( htm, szMemPath, pstPutIn, ulUpdKey, usUpdTarget,
                        usFlags, usMsgHandling ) );
} /* end of function TmUpdSeg */

USHORT C_TmUpdSegW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
)
{
  return( C_TmUpdSegHwndW( htm, szMemPath, pstPutIn, ulUpdKey, usUpdTarget,
                        usFlags, usMsgHandling, NULLHANDLE ) );
} /* end of function TmUpdSegW */
USHORT TmUpdSegW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
)
{
  return( C_TmUpdSegW( htm, szMemPath, pstPutIn, ulUpdKey, usUpdTarget,
                        usFlags, usMsgHandling) );
} /* end of function TmUpdSegW */

USHORT C_TmUpdSegHwndW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN_W pstPutInW,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling,             //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
  HWND        hwnd                       //(in)  handle for error messages
)
{
  USHORT usRc;
  PTMX_PUT_IN   pstPutIn;
  ULONG         ulCP = 0L;

  ulCP = GetLangOEMCP( NULL);

  UtlAlloc( (PVOID *) &pstPutIn,  0L, sizeof( TMX_PUT_IN ), NOMSG );

  // copy structures
  TMX_PUT_IN_Unicode2ASCII( pstPutInW, pstPutIn, ulCP );

  usRc = C_TmUpdSegHwnd( htm, szMemPath,  pstPutIn, ulUpdKey, usUpdTarget,
                            usFlags, usMsgHandling,hwnd );

  UtlAlloc( (PVOID *) &pstPutIn,  0L, 0L, NOMSG );

  return usRc;
}


USHORT C_TmUpdSegHwnd
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling,             //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
  HWND        hwnd                       //(in)  handle for error messages
)
{
  USHORT     usRc;                       //function return code

  DEBUGEVENT( TMUPDSEG_LOC, FUNCENTRY_EVENT, 0 );

  usRc = TmtXUpdSeg( (PTMX_CLB)htm, pstPutIn, ulUpdKey, usUpdTarget,
                     usFlags );

  // do error handling
  if ( usMsgHandling && usRc )
  {
    if ( usRc == ERROR_TA_ACC_TAGTABLE )
    {
      CHAR szTagTable[MAX_EQF_PATH];
      strcpy(szTagTable, pstPutIn->stTmPut.szTagTable);
      strcat( szTagTable, EXT_OF_FORMAT );
      usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, szTagTable, hwnd );
    }
    else
    {
       usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, NULL, hwnd );
    } /* endif */
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMUPDSEG_LOC, ERROR_EVENT, usRc );
  } /* endif */


  return usRc;
} /* End of function TmUpdSeg */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmGet
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmGet( HTM          htm,
//                           PSZ          szMemPath,
//                           PTMX_GET_IN  pstGetIn,
//                           PTMX_GET_OUT pstGetOut,
//                           USHORT       usMsgHandling )
//------------------------------------------------------------------------------
// Description:       Retrives a match from the a TM
//------------------------------------------------------------------------------
// Parameters:        htm           - (in)  TM handle
//                    szMemPath     - (in)  full TM name x:\eqf\mem\mem.tmd
//                    pstGetIn      - (in)  pointer to get input structure
//                    pstGetOut     - (out) pointer to get output structure
//                    usMsgHandling - (in)  message handling parameter
//                                          TRUE:  display error message
//                                          FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
C_TmGet (HTM            htm,           //(in)  TM handle
       PSZ            pszMemPath,    //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN    pstGetIn,      //(in)  pointer to get input structure
       PTMX_GET_OUT   pstGetOut,     //(out) pointer to get output structure
       USHORT         usMsgHandling )//(in)  message handling parameter
                                     //      TRUE:  display error message
                                     //      FALSE: display no error message
{
  USHORT usRc = 0;
  PTMX_GET_IN_W pstGetInW = NULL;
  PTMX_GET_OUT_W pstGetOutW = NULL;

  UtlAlloc( (PVOID *) &pstGetInW,  0L, sizeof( TMX_GET_IN_W ), NOMSG );
  UtlAlloc( (PVOID *) &pstGetOutW, 0L, sizeof( TMX_GET_OUT_W ), NOMSG );

  // copy structures
  TMX_GET_IN_ASCII2Unicode( pstGetIn, pstGetInW, 0 );

  usRc = C_TmGetW( htm, pszMemPath, pstGetInW, pstGetOutW, usMsgHandling );

  // copy output structures back
  TMX_GET_OUT_Unicode2ASCII( pstGetOutW, pstGetOut, 0 );

  UtlAlloc( (PVOID *) &pstGetInW,  0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pstGetOutW, 0L, 0L, NOMSG );

  return usRc;
}
USHORT
TmGet (HTM            htm,           //(in)  TM handle
       PSZ            pszMemPath,    //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN    pstGetIn,      //(in)  pointer to get input structure
       PTMX_GET_OUT   pstGetOut,     //(out) pointer to get output structure
       USHORT         usMsgHandling )//(in)  message handling parameter
                                     //      TRUE:  display error message
                                     //      FALSE: display no error message
{
	return C_TmGet(htm,
       pszMemPath,
       pstGetIn,
       pstGetOut,
       usMsgHandling );
}

USHORT
C_TmGetW(HTM            htm,             //(in)  TM handle
       PSZ            szMemPath,       //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN_W  pstGetIn,        //(in)  pointer to get input structure
       PTMX_GET_OUT_W pstGetOut,       //(out) pointer to get output structure
       USHORT         usMsgHandling )  //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
{
  USHORT      usRc;                    //U code rc
  USHORT      usQRc;                   //EqfSend2Handler
  SERVERNAME  szServer;                //var for server name
  BOOL        fOk;                     //rc from UtlAlloc
  PSZ_W       pszTempString;           //temp string for conversion of CRLF
  USHORT      usI;                     //index var for for loop

  DEBUGEVENT( TMGET_LOC, FUNCENTRY_EVENT, 0 );

  /********************************************************************/
  /* fill the TMX_GET_IN structure                                    */
  /* stPrefixIn.usLengthInput                                         */
  /* stPrefixIn.usTmCommand                                           */
  /* the TMX_GET_IN structure must not be filled it is provided       */
  /* by the caller                                                    */
  /********************************************************************/
  pstGetIn->stPrefixIn.usLengthInput = sizeof( TMX_GET_IN_W );
  pstGetIn->stPrefixIn.usTmCommand   = TMC_GET;

  /********************************************************************/
  /* call U code to pass TM command to server or handle it local      */
  /********************************************************************/
  usRc = TmtXGet( (PTMX_CLB)htm, pstGetIn, pstGetOut );

  if ( (usRc == NO_ERROR) && pstGetOut->usNumMatchesFound )
  {
    /******************************************************************/
    /* convert the output according to convert flag                   */
    /******************************************************************/
    if ( pstGetIn->stTmGet.usConvert != MEM_OUTPUT_ASIS )
    {
      /****************************************************************/
      /* allocate storage for temp string                             */
      /****************************************************************/
      fOk = UtlAlloc( (PVOID *) &pszTempString, 0L,
                      (ULONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W),
                      NOMSG );
      if ( fOk )
      {
        /**************************************************************/
        /* loop over all found matches returned in get out struct     */
        /**************************************************************/
        for ( usI=0 ; usI < pstGetOut->usNumMatchesFound; usI++ )
        {
          /************************************************************/
          /* convert source string                                    */
          /************************************************************/
          NTMConvertCRLFW( pstGetOut->stMatchTable[usI].szSource,
                          pszTempString,
                          pstGetIn->stTmGet.usConvert );

          /************************************************************/
          /* convert target string                                    */
          /************************************************************/
          NTMConvertCRLFW( pstGetOut->stMatchTable[usI].szTarget,
                          pszTempString,
                          pstGetIn->stTmGet.usConvert );
        } /* endfor */
        /**************************************************************/
        /* free storage for temp string                               */
        /**************************************************************/
        UtlAlloc( (PVOID *) &pszTempString, 0L, 0L, NOMSG );
      }
      else
      {
        usRc = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if an error occured call MemRcHandling in dependency of          */
  /* the message flag to display error message                        */
  /********************************************************************/
  if ( usMsgHandling && usRc )
  {
    usRc = MemRcHandling( usRc, szMemPath, &htm, NULL );
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMGET_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* End of function TmGet */
USHORT
TmGetW (HTM            htm,           //(in)  TM handle
       PSZ            pszMemPath,    //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN_W  pstGetIn,      //(in)  pointer to get input structure
       PTMX_GET_OUT_W pstGetOut,     //(out) pointer to get output structure
       USHORT         usMsgHandling )//(in)  message handling parameter
                                     //      TRUE:  display error message
                                     //      FALSE: display no error message
{
	return C_TmGetW(htm,
       pszMemPath,
       pstGetIn,
       pstGetOut,
       usMsgHandling );
}



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmExtract
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmExtract( HTM           htm,
//                               PSZ           szMemPath,
//                               PTMX_EXT_IN   pstExtIn,
//                               PTMX_EXT_OUT  pstExtOut,
//                               USHORT        usMsgHandling )
//------------------------------------------------------------------------------
// Description:       Extract a segment from a TM
//------------------------------------------------------------------------------
// Parameters:        htm           - (in)  TM handle
//                    szMemPath     - (in)  full TM name x:\eqf\mem\mem.tmd
//                    pstExtIn      - (in)  pointer to extract input structure
//                    pstExtOut     - (out) pointer to extract output structure
//                    usMsgHandling - (in)  message handling parameter
//                                          TRUE:  display error message
//                                          FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
C_TmExtract( HTM           htm,            //(in)  TM handle
           PSZ           szMemPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
           PTMX_EXT_IN   pstExtIn,       //(in)  pointer to extract input structure
           PTMX_EXT_OUT  pstExtOut,      //(out) pointer to extract output structure
           USHORT        usMsgHandling ) //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
{
  return( C_TmExtractHwnd( htm, szMemPath, pstExtIn, pstExtOut, usMsgHandling,
                         NULLHANDLE ) );
} /* end of function TmExtract */
USHORT
TmExtract( HTM           htm,            //(in)  TM handle
           PSZ           szMemPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
           PTMX_EXT_IN   pstExtIn,       //(in)  pointer to extract input structure
           PTMX_EXT_OUT  pstExtOut,      //(out) pointer to extract output structure
           USHORT        usMsgHandling ) //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
{
  return( C_TmExtract( htm, szMemPath, pstExtIn, pstExtOut, usMsgHandling ) );
} /* end of function TmExtract */

USHORT
C_TmExtractW( HTM           htm,            //(in)  TM handle
           PSZ            szMemPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
           PTMX_EXT_IN_W  pstExtInW,      //(in)  pointer to extract input structure
           PTMX_EXT_OUT_W pstExtOutW,     //(out) pointer to extract output structure
           USHORT         usMsgHandling ) //(in)  message handling parameter
                                          //      TRUE:  display error message
                                          //      FALSE: display no error message
{
  return( C_TmExtractHwndW( htm, szMemPath, pstExtInW, pstExtOutW, usMsgHandling,
                         NULLHANDLE ) );
} /* end of function TmExtract */
USHORT
TmExtractW( HTM           htm,            //(in)  TM handle
           PSZ            szMemPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
           PTMX_EXT_IN_W  pstExtInW,      //(in)  pointer to extract input structure
           PTMX_EXT_OUT_W pstExtOutW,     //(out) pointer to extract output structure
           USHORT         usMsgHandling ) //(in)  message handling parameter
                                          //      TRUE:  display error message
                                          //      FALSE: display no error message
{
  return( C_TmExtractW( htm, szMemPath, pstExtInW, pstExtOutW, usMsgHandling ) );
} /* end of function TmExtract */


USHORT
C_TmExtractHwnd
         ( HTM            htm,            //(in)  TM handle
           PSZ            szMemPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
           PTMX_EXT_IN    pstExtIn,       //(in)  pointer to extract input structure
           PTMX_EXT_OUT   pstExtOut,      //(out) pointer to extract output structure
           USHORT         usMsgHandling,  //(in)  message handling parameter
                                          //      TRUE:  display error message
                                          //      FALSE: display no error message
           HWND           hwnd )          //(in)  handle for error messages
{
  USHORT usRc;
  PTMX_EXT_IN_W  pstExtInW;      //(in)  pointer to extract input structure
  PTMX_EXT_OUT_W pstExtOutW;     //(out) pointer to extract output structure
  ULONG          ulCP = 0L;

  ulCP = GetLangOEMCP(NULL);

  UtlAlloc( (PVOID *) &pstExtInW, 0L, sizeof(TMX_EXT_IN_W), NOMSG );
  UtlAlloc( (PVOID *) &pstExtOutW, 0L, sizeof(TMX_EXT_OUT_W), NOMSG );

  TMX_EXT_IN_ASCII2Unicode( pstExtIn, pstExtInW );

  usRc = C_TmExtractHwndW( htm, szMemPath, pstExtInW, pstExtOutW, usMsgHandling, hwnd );

  TMX_EXT_OUT_Unicode2ASCII( pstExtOutW, pstExtOut, ulCP );

  UtlAlloc( (PVOID *) &pstExtInW, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pstExtOutW, 0L, 0L, NOMSG );


  return usRc;
}

USHORT
TmExtractHwndW
  ( HTM          htm,              //(in)  TM handle
    PSZ          szMemPath,        //(in)  full TM name x:\eqf\mem\mem.tmd
    PTMX_EXT_IN_W  pstExtIn,       //(in)  pointer to extract input structure
    PTMX_EXT_OUT_W pstExtOut,      //(out) pointer to extract output structure
    USHORT       usMsgHandling,    //(in)  message handling parameter
                                   //      TRUE:  display error message
                                   //      FALSE: display no error message
    HWND         hwnd )            //(in)  handle for error messages
{
	return C_TmExtractHwndW
  ( htm,
    szMemPath,
    pstExtIn,
    pstExtOut,
    usMsgHandling,
    hwnd );
}

USHORT
C_TmExtractHwndW
  ( HTM          htm,              //(in)  TM handle
    PSZ          szMemPath,        //(in)  full TM name x:\eqf\mem\mem.tmd
    PTMX_EXT_IN_W  pstExtIn,       //(in)  pointer to extract input structure
    PTMX_EXT_OUT_W pstExtOut,      //(out) pointer to extract output structure
    USHORT       usMsgHandling,    //(in)  message handling parameter
                                   //      TRUE:  display error message
                                   //      FALSE: display no error message
    HWND         hwnd )            //(in)  handle for error messages
{
  USHORT      usRc;                      //U code rc
  USHORT      usQRc;                     //EqfSend2Handler
  SERVERNAME  szServer;                  //var for server name
  PSZ_W       pszTempString = NULL;      //temp string for conversion of CRLF
  BOOL        fOk;                       //rc from UtlAlloc

  /********************************************************************/
  /* fill the TMX_EXTRACT_IN structure                                */
  /* stPrefixIn.usLengthInput                                         */
  /* stPrefixIn.usTmCommand                                           */
  /* the TMX_EXTRACT_IN structure must not be filled it is provided   */
  /* by the caller                                                    */
  /********************************************************************/
  pstExtIn->stPrefixIn.usLengthInput = sizeof( TMX_EXT_IN_W );
  pstExtIn->stPrefixIn.usTmCommand   = TMC_EXTRACT;

  /********************************************************************/
  /* call U code to pass TM command to server or handle it local      */
  /********************************************************************/
  usRc = TmtXExtract( (PTMX_CLB)htm, pstExtIn, pstExtOut );

  if ( usRc == NO_ERROR )
  {
    /******************************************************************/
    /* convert the output according to convert flag                   */
    /******************************************************************/
    if ( (pstExtIn->usConvert == MEM_OUTPUT_CRLF) ||
         (pstExtIn->usConvert == MEM_OUTPUT_LF) )
    {

      /****************************************************************/
      /* allocate storage for temp string                             */
      /****************************************************************/
      fOk = UtlAlloc( (PVOID *) &pszTempString, 0L,
                      (ULONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W),
                      NOMSG );
      if ( fOk )
      {
        /**************************************************************/
        /* convert source string                                      */
        /**************************************************************/
        NTMConvertCRLFW( pstExtOut->stTmExt.szSource,
                        pszTempString,
                        pstExtIn->usConvert );

        /**************************************************************/
        /* convert target string                                      */
        /**************************************************************/
        NTMConvertCRLFW( pstExtOut->stTmExt.szTarget,
                        pszTempString,
                        pstExtIn->usConvert );

        /**************************************************************/
        /* free storage for temp string                               */
        /**************************************************************/
        UtlAlloc( (PVOID *) &pszTempString, 0L, 0L, NOMSG );
      }
      else
      {
        usRc = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if an error occured call MemRcHandling in dependency of          */
  /* the message flag to display error message                        */
  /********************************************************************/
  if ( usMsgHandling && usRc )
  {
    switch ( usRc )
    {
    //-----------------------------------------------------------------------
    case BTREE_EOF_REACHED:
       break;
    //-----------------------------------------------------------------------
    default:
      usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, NULL, hwnd );
      break;
    } /* endswitch */
  } /* endif */
  return usRc;
} /* End of function TmExtract */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmInfo
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmInfo( HTM           htm,
//                            PSZ           szMemPath,
//                            USHORT        usInfoLevel,
//                            PTMX_INFO_IN  pstInfoIn,
//                            PTMX_INFO_OUT pstInfoOut,
//                            USHORT        usMsgHandling )
//------------------------------------------------------------------------------
// Description:       Returns the signature record of the TM referenced
//                    by the passed handle.
//------------------------------------------------------------------------------
// Parameters:        htm           - (in)  TM handle
//                    szMemPath     - (in)  full TM name x:\eqf\mem\mem.tmd
//                    usInfoLevel   - (in)  information level
//                                            TMINFO_SIGNATURE
//                    pstInfoIn     - (in)  pointer to info input structure
//                    pstInfoOut    - (out) pointer to info output structure
//                    usMsgHandling - (in)  message handling parameter
//                                          TRUE:  display error message
//                                          FALSE: display no error message
//------------------------------------------------------------------------------
// Prerequesites:     The passed handle (htm) must be a valid TM handle
//                    returned from the prior TmOpen call.
//                    The TM must be opened befor calling TmInfo
//                    At this point in time only info level TMINFO_SIGNATURE
//                    is supported.
//------------------------------------------------------------------------------
USHORT
C_TmInfo( HTM           htm,            //(in)  TM handle
        PSZ           szMemPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
        USHORT        usInfoLevel,    //(in)  information level
                                      //        TMINFO_SIGNATURE
        PTMX_INFO_IN  pstInfoIn,      //(in)  pointer to info input structure
        PTMX_INFO_OUT pstInfoOut,     //(out) pointer to info output structure
        USHORT        usMsgHandling ) //(in)  message handling parameter
                                      //      TRUE:  display error message
{
  return( C_TmInfoHwnd( htm, szMemPath, usInfoLevel, pstInfoIn, pstInfoOut,
                      usMsgHandling, NULLHANDLE ) );
}

USHORT
C_TmInfoHwnd( HTM           htm,            //(in)  TM handle
        PSZ           szMemPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
        USHORT        usInfoLevel,    //(in)  information level
                                      //        TMINFO_SIGNATURE
        PTMX_INFO_IN  pstInfoIn,      //(in)  pointer to info input structure
        PTMX_INFO_OUT pstInfoOut,     //(out) pointer to info output structure
        USHORT        usMsgHandling,  //(in)  message handling parameter
                                      //      TRUE:  display error message
                                      //      FALSE: display no error message
        HWND          hwnd )          //(in)  handle for error messages
{
  USHORT      usRc;                   //function return code
  USHORT      usQRc;                  //rc from EqfSend2Handler
  SERVERNAME  szServer;               //local var for server name

  /********************************************************************/
  /* fill the TMX_INFO_IN structure                                   */
  /* stPrefixIn.usLengthInput                                         */
  /* stPrefixIn.usTmCommand                                           */
  /********************************************************************/
  pstInfoIn->stPrefixIn.usLengthInput = sizeof( TMX_INFO_IN );
  pstInfoIn->stPrefixIn.usTmCommand   = TMC_INFO;
  pstInfoIn->usInfoLevel = usInfoLevel;

  /********************************************************************/
  /* call U code to pass TM command to server or handle it local      */
  /********************************************************************/
  usRc = TmtXInfo( (PTMX_CLB)htm, pstInfoOut );
//usRc = U( htm,
//          (PXIN)pstInfoIn,               // Pointer to input structure
//          (PXOUT)pstInfoOut,
//          NEW_TM );

  if ( usMsgHandling && usRc )
  {
    usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, NULL, hwnd );
  } /* endif */
  return usRc;
} /* End of function TmInfo */

/**********************************************************************/
/**********************************************************************/
/* Internal functions                                                 */
/**********************************************************************/
/**********************************************************************/
//------------------------------------------------------------------------------
// Function name:     NTMFillCreateInStruct
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    NTMFillCreateInStruct( HTM             hModel,
//                                           PSZ             pszPathMem,
//                                           PSZ             pszServer,
//                                           PSZ             pszUserID,
//                                           PSZ             pszSourceLang,
//                                           PSZ             pszDescription,
//                                           PTMX_CREATE_IN  pstCreateIn,
//                                           USHORT          usMsgHandling )
//                                           HWND            hwnd )
//------------------------------------------------------------------------------
// Description:       Fills the TMX_CREATE_IN structure from the passed
//                    parameters or from a model handle.
//------------------------------------------------------------------------------
// Parameters:
//  hModel         - (in)  model handle
//  pszPathMem     - (in)  full TM name x:\eqf\mem\mem.tmd
//  pszServer      - (in)  model handle
//  pszUserID      - (in)  server name or empty str (when no model handle)
//  pszSourceLang  - (in)  LAN USERID or empty str  (when no model handle)
//  pszDescription - (in)  source language or empty str (when no model handle)
//  pstCreateIn    - (in)  TM description or empty str (when no model handle)
//  usMsgHandling  - (in)  message handling parameter
//                         TRUE:  display error message
//                         FALSE: display no error message
//------------------------------------------------------------------------------
// Prerequesits:      htm must be a valid TM handle returned from TmOpen
//                    The TM referenced by htm must be open.
//------------------------------------------------------------------------------
USHORT
NTMFillCreateInStruct( HTM             hModel,
                       PSZ             pszPathMem,
                       PSZ             pszServer,
                       PSZ             pszUserID,
                       PSZ             pszSourceLang,
                       PSZ             pszDescription,
                       PTMX_CREATE_IN  pstCreateIn,
                       USHORT          usMsgHandling,
                       HWND            hwnd )
{
  USHORT        usRc=NO_ERROR;
  USHORT        fOk;
  PTMX_INFO_IN  pstInfoIn;
  PTMX_INFO_OUT pstInfoOut;

  /********************************************************************/
  /* fill prefix of the TMX_CREATE_IN                                 */
  /* prefin.usLenIn     - length of the structure (TMX_CREATE_IN)     */
  /* prefin.usCommand   - the Tm command, here TMC_CREATE             */
  /********************************************************************/
  pstCreateIn->stPrefixIn.usLengthInput = sizeof( TMX_CREATE_IN );
  pstCreateIn->stPrefixIn.usTmCommand  = TMC_CREATE;

  if ( hModel )
  {
    /******************************************************************/
    /* model handle,get data for TMX_CREATE_IN stuc from model handle */
    /******************************************************************/
    /******************************************************************/
    /* initialize rc                                                  */
    /******************************************************************/
     usRc = ERROR_NOT_ENOUGH_MEMORY;

    /******************************************************************/
    /* allocate storage for TMX_INFO_IN and TMX_INFO_OUT              */
    /******************************************************************/
    fOk = (USHORT)UtlAlloc( (PVOID *) &pstInfoIn, 0L,
                    (LONG)( sizeof( TMX_INFO_IN ) +
                            sizeof( TMX_INFO_OUT ) ),
                            FALSE );
    if ( fOk )
    {
      /****************************************************************/
      /* assign memory to pointer pstInfoOut                          */
      /****************************************************************/
      pstInfoOut = (PTMX_INFO_OUT)(pstInfoIn + 1);

      /****************************************************************/
      /* fill prefix of TMX_INFO_IN structure                         */
      /****************************************************************/
      pstInfoIn->stPrefixIn.usLengthInput = sizeof(TMX_INFO_IN);
      pstInfoIn->stPrefixIn.usTmCommand = TMC_INFO;

      /****************************************************************/
      /* call function to get signature record of TM from model handle*/
      /****************************************************************/
      usRc = C_TmInfoHwnd( hModel,
                         pszPathMem,
                         TMINFO_SIGNATURE,
                         pstInfoIn,
                         pstInfoOut,
                         usMsgHandling,
                         hwnd );

      if ( !usRc )
      {
        /****************************************************************/
        /* fill the TMX_CREATE_IN structure from TmInfo ouput           */
        /* szDataName         - TM path and name X:\EQFD\MEM\MEM.TMD    */
        /* sIndexName         - TM path and name X:\EQFD\MEM\MEM.TMI    */
        /* szServer           - server name                             */
        /* szUserID           - LAN userid                              */
        /* szSourceLanguage   - source language name                    */
        /* szDescription      - description text                        */
        /* usThreshold        - threshold for return of fuzzy matches   */
        /* bLangTable         - language table                          */
        /****************************************************************/
        strcpy( pstCreateIn->stTmCreate.szDataName, pszPathMem );

        /**************************************************************/
        /* check if the extension of the passed TM name is a extension*/
        /* for a temporary TM used  by TM organize or the extension   */
        /* of a shared TM                                             */
        /**************************************************************/
        if ( strcmp( strrchr( pszPathMem, '.'), EXT_OF_SHARED_MEM ) == 0 )
        {
          /************************************************************/
          /* TM is a shared one, use "shared" index name              */
          /************************************************************/
          Utlstrccpy ( pstCreateIn->stTmCreate.szIndexName, pszPathMem, DOT );
          strcat( pstCreateIn->stTmCreate.szIndexName, EXT_OF_SHARED_MEMINDEX );
        }
        else if ( !strcmp( strrchr( pszPathMem, '.'), EXT_OF_TMDATA ) )
        {
          /************************************************************/
          /* TM is no temporaray TM, use "normal" index name          */
          /************************************************************/
          Utlstrccpy ( pstCreateIn->stTmCreate.szIndexName, pszPathMem, DOT );
          strcat( pstCreateIn->stTmCreate.szIndexName, EXT_OF_TMINDEX );
        }
        else
        {
          /************************************************************/
          /* TM is a temporary TM, use temporary index name           */
          /************************************************************/
          Utlstrccpy ( pstCreateIn->stTmCreate.szIndexName, pszPathMem, DOT );
          strcat( pstCreateIn->stTmCreate.szIndexName, EXT_OF_TEMP_TMINDEX );
        } /* endif */

        strcpy( pstCreateIn->stTmCreate.szServer,
                pstInfoOut->stTmSign.szServer );
        strcpy( pstCreateIn->stTmCreate.szUserid,
                pstInfoOut->stTmSign.szUserid );
        strcpy( pstCreateIn->stTmCreate.szSourceLanguage,
                pstInfoOut->stTmSign.szSourceLanguage );
        strcpy( pstCreateIn->stTmCreate.szDescription,
                pstInfoOut->stTmSign.szDescription );
        pstCreateIn->stTmCreate.usThreshold = pstInfoOut->usThreshold;
      } /* endif */
      /**************************************************************/
      /* free TMX_INFO_IN and TMX_INFO_OUT structure                */
      /**************************************************************/
      UtlAlloc( (PVOID *) &pstInfoIn, 0L, 0L, NOMSG );
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* no model handle get data for TMX_CREATE_IN stucture from input */
    /******************************************************************/
    /******************************************************************/
    /* fill the TMX_CREATE_IN structure                               */
    /* szDataName         - TM path and name X:\EQFD\MEM\MEM.TMD      */
    /* szIndexName        - TM path and name X:\EQFD\MEM\MEM.TMI      */
    /* szServer           - server name                               */
    /* szUserID           - LAN userid                                */
    /* szSourceLanguage   - source language name                      */
    /* szDescription      - description text                          */
    /* usThreshold        - threshold for return of fuzzy matches     */
    /* bLangTable         - language table                            */
    /******************************************************************/
    strcpy( pstCreateIn->stTmCreate.szDataName,       pszPathMem      );

    /**************************************************************/
    /* check if the extension of the passed TM name is a extension*/
    /* for a temporary TM used  by TM organize                    */
    /**************************************************************/
    if ( strcmp( strrchr( pszPathMem, '.'), EXT_OF_SHARED_MEM ) == 0 )
    {
      /************************************************************/
      /* TM is a shared one, use "shared" index name              */
      /************************************************************/
      Utlstrccpy ( pstCreateIn->stTmCreate.szIndexName, pszPathMem, DOT );
      strcat( pstCreateIn->stTmCreate.szIndexName, EXT_OF_SHARED_MEMINDEX );
    }
    else if ( !strcmp( strrchr( pszPathMem, '.'), EXT_OF_TMDATA ) )
    {
      /************************************************************/
      /* TM is no temporaray TM, use "normal" index name          */
      /************************************************************/
      Utlstrccpy ( pstCreateIn->stTmCreate.szIndexName, pszPathMem, DOT );
      strcat( pstCreateIn->stTmCreate.szIndexName, EXT_OF_TMINDEX );
    }
    else
    {
      /************************************************************/
      /* TM is a temporary TM, use temporary index name           */
      /************************************************************/
      Utlstrccpy ( pstCreateIn->stTmCreate.szIndexName, pszPathMem, DOT );
      strcat( pstCreateIn->stTmCreate.szIndexName, EXT_OF_TEMP_TMINDEX );
    } /* endif */

    strcpy( pstCreateIn->stTmCreate.szServer,         pszServer       );
    strcpy( pstCreateIn->stTmCreate.szUserid,         pszUserID       );
    strcpy( pstCreateIn->stTmCreate.szSourceLanguage, pszSourceLang   );
    strcpy( pstCreateIn->stTmCreate.szDescription,    pszDescription  );
    pstCreateIn->stTmCreate.usThreshold = TM_DEFAULT_THRESHOLD;
  } /* endif */

  return usRc;
} /* end of function NTMFillCreateInStruct */

//------------------------------------------------------------------------------
// Function name:     NTMGetThresholdFromProperties
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    NTMGetThresholdFromProperties( PSZ     pszMemFullPath,
//                                                   PUSHORT pusThreshold,
//                                                   USHORT  usMsgHandling )
//------------------------------------------------------------------------------
// Description:       retrieves the threshold from the TM property file
//------------------------------------------------------------------------------
// Parameters:        pszMemFullPath - (in)  full TM name x:\eqf\mem\mem.tmd
//                    pusThreshold   - (out) threshold retrieved from property
//                    usMsgHandling  - (in)  message handling parameter
//                                            TRUE:  display error message
//                                            FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
NTMGetThresholdFromProperties
  ( PSZ       pszMemFullPath,  //(in)  full TM name x:\eqf\mem\mem.tmd
    PUSHORT   pusThreshold,    //(out) threshold retrieved from property
    USHORT    usMsgHandling )  //(in)  message handling parameter
                               //TRUE:  display error message
                               //FALSE: display no error message
{
  CHAR      szSysPath[MAX_EQF_PATH];      //EQF system path X:\EQF
  PSZ       pszTemp;                      //temp ptr for property name
  CHAR      szPropertyName[MAX_FILESPEC]; //property name TMTEST.MEM
  HPROP     hProp;                        //handle of TM properties
  PPROP_NTM pProp;                        //pointer to TM properties
  EQFINFO   ErrorInfo;                    //rc from OpenProperties
  USHORT    usRc;                         //funtion rc

  /********************************************************************/
  /* get the TM name (w/o ext) from the the full TM path and append   */
  /* the TM property extension                                        */
  /********************************************************************/
  pszTemp = UtlGetFnameFromPath( pszMemFullPath);
  Utlstrccpy( szPropertyName, pszTemp, DOT );
  strcat( szPropertyName, GetPropFileExtension(pszMemFullPath));

  /********************************************************************/
  /* get the EQF system path  X:\EQF                                  */
  /* NTMOpenProperties and therefore OpenProperties needs only the    */
  /* system path and the TM property name with ext                    */
  /********************************************************************/
  UtlMakeEQFPath ( szSysPath, NULC, SYSTEM_PATH, NULL );

  /********************************************************************/
  /* open the properties of the TM                                    */
  /********************************************************************/
  usRc = NTMOpenProperties( &hProp,
                            (PVOID *)&pProp,
                            szPropertyName,
                            szSysPath,
                            PROP_ACCESS_READ,
                            usMsgHandling );
  if ( usRc == NO_ERROR || usRc == ERROR_OLD_PROPERTY_FILE )
  {
    /******************************************************************/
    /* if no error, return the threshold from the TM properties       */
    /******************************************************************/
    *pusThreshold = pProp->usThreshold;
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo);
  } /* endif */

  return usRc;
} /* end of function NTMGetThresholdFromProperties */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmDeleteTM
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmDeleteTM( PSZ     pszMemName,
//                                USHORT  usMsgHandling,
//                                HWND    hwndOwner )
//------------------------------------------------------------------------------
// Description:       Deletes a TM and its index file
//------------------------------------------------------------------------------
// Parameters:        pszMemName    - (in)  TM name MEM.TMD
//                    usMsgHandling - (in)  message handling parameter
//                                           TRUE:  display error message
//                                           FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
C_TmDeleteTM( PSZ     pszMemName,            //in)  TM name MEM.TMD
            USHORT  usMsgHandling,         //(in)  message handling parameter
                                           //      TRUE:  display error message
                                           //      FALSE: display no error message
            HWND    hwndOwner,
            PUSHORT pusYesToAllMode )      //(in/out) Yes-to-all mode
{
  BOOL       fDeleteGrayedOut = FALSE;     //delete greyed out flag
  CHAR       szSysPath[MAX_EQF_PATH];      //system path for NTMOpenProperties
  CHAR       szPropFileName[MAX_FILESPEC]; //property filename for NTMOpenProperties
  PSZ        pReplAddr[2];                 //replace UtlError
  HTM        htm = NULLHANDLE;             //TM handle
  HPROP      hProp = NULL;                 //property handle
  PPROP_NTM  pProp;                        //pointer to TM properties
  USHORT     usReply = MBID_YES;           //reply from query message
  BOOL       fOk;                          //processing flag
  USHORT     usRc = TM_FUNCTION_FAILED;    //function return code
  PTMX_DELTM_IN  pDelTmIn  = NULL;         //ptr to del in structure
  PTMX_DELTM_OUT pDelTmOut = NULL;         //ptr to del out structure
  EQFINFO    ErrorInfo;                    //rc from OpenProperties
  CHAR       szUserID[MAX_USERID];         //buffer for user ID
  USHORT     usUserLevel;
  BOOL       fShared = FALSE;              // = TRUE for shared TMs
  CHAR       szMemShortName[MAX_FILESPEC]; // short name of TM

  fDeleteGrayedOut = (usMsgHandling == DELETE_GREYEDOUT);

  // get user ID
  if ( UtlGetLANUserID( szUserID, &usUserLevel, FALSE ) != NO_ERROR )
  {
    usUserLevel = 0;
    szUserID[0] = NULC;
  } /* endif */


  /********************************************************************/
  /* allocat storage for delete in and out structure                  */
  /********************************************************************/
  fOk= UtlAlloc( (PVOID *) &pDelTmIn, 0L,
                 (LONG)( sizeof( TMX_DELTM_IN ) +
                         sizeof( TMX_DELTM_OUT ) ),
                         FALSE );
  if ( fOk)
  {
    BOOL fIsNew = FALSE;         // is-new flag

    pDelTmOut = (PTMX_DELTM_OUT)( pDelTmIn + 1 );

    ObjLongToShortName( pszMemName, szMemShortName, TM_OBJECT, &fIsNew );

    /******************************************************************/
    /* Create the full translation memory path+name+ext               */
    /******************************************************************/
    strcpy( pDelTmIn->szFullTmName, szMemShortName );
    if ( MemCreatePath( pDelTmIn->szFullTmName ) )
    {
      /****************************************************************/
      /* create the full index filename                               */
      /****************************************************************/
      INDEXNAMEFROMMEMPATH( pDelTmIn->szFullTmName, pDelTmIn->szFullIndexName );

      /****************************************************************/
      /* Create system path (used in MemOpenProp)                     */
      /* Make property file name                                      */
      /****************************************************************/
      UtlMakeEQFPath( szSysPath, NULC, SYSTEM_PATH, NULL );
      sprintf( szPropFileName, "%s%s", szMemShortName, GetPropFileExtension(pDelTmIn->szFullTmName) );

      /****************************************************************/
      /* create full property name                                    */
      /****************************************************************/
      UtlMakeEQFPath( pDelTmIn->szFullPropName, NULC, PROPERTY_PATH, NULL );
      strcat( pDelTmIn->szFullPropName, BACKSLASH_STR );
      strcat( pDelTmIn->szFullPropName, szPropFileName );

      if ( !MemOpenProp( &hProp,         // pointer to property handle
                         (PVOID *)&pProp,// pointer to property pointer
                         szPropFileName, // property file name
                         szSysPath,      // pointer to system path
                         PROP_ACCESS_READ, // open mode either PROP_READ or PROP_WRITE
                         FALSE ) )       // Message flag
      {
        /**************************************************************/
        /* open failed                                                */
        /* try to delete properties also when props cannot be opened  */
        /**************************************************************/
        UtlDelete( szPropFileName, 0L, FALSE );
        fOk = FALSE;
        /**************************************************************/
        /* update TM list box because the property file does not exist*/
        /* or is corrupted. The TM could not be used any longer,      */
        /* so remove it from the TM list                              */
        /**************************************************************/
        if ( hwndOwner != HWND_FUNCIF )
        {

          CHAR    szObjName[MAX_EQF_PATH];

          sprintf( szObjName, "%s\\%s", szSysPath, szPropFileName );
          EqfSend2Handler( MEMORYHANDLER, WM_EQFN_DELETED,
                           MP1FROMSHORT( clsMEMORYDB ),
                           MP2FROMP( szObjName ));
        } /* endif */
        usMsgHandling = FALSE;
      } /* endif */

      /****************************************************************/
      /* for shared TMs check if user is the owner of the TM and      */
      /* therefore is allowed to delete the TM physically             */
      /****************************************************************/
      if ( fOk )
      {
        fShared = (pProp->usLocation == TM_SHARED);

        if ( fShared )
        {
          if ( _stricmp( szUserID, pProp->stTMSignature.szUserid ) != 0 )
          {
            if ( hProp ) CloseProperties( hProp, PROP_QUIT, &ErrorInfo );

            /**********************************************************/
            /* user is not the owner of the TM, so allow undermine of */
            /* access only...                                         */
            /**********************************************************/
            if ( (hwndOwner != HWND_FUNCIF) && !ISBATCHHWND(hwndOwner) )
            {
              OEMTOANSI( pszMemName );
              pReplAddr[0] = pszMemName;
              if ( *pusYesToAllMode == MB_EQF_YESTOALL )
              {
                usReply = UtlError( WARNING_DELETE_REM_INC_MEMORY,
                                    MB_EQF_YESTOALL | MB_DEFBUTTON2, 1,
                                    &pReplAddr[0], EQF_QUERY );
                if ( usReply == MBID_EQF_YESTOALL )
                {
                  *pusYesToAllMode = usReply = MBID_YES;
                } /* endif */
              }
              else if ( *pusYesToAllMode == MBID_YES )
              {
                // user has choosen yes-to-all already
                usReply = MBID_YES;
              }
              else
              {
                usReply = UtlError( WARNING_DELETE_REM_INC_MEMORY,
                                    MB_YESNO | MB_DEFBUTTON2, 1,
                                    &pReplAddr[0], EQF_QUERY );
              } /* endif */
              ANSITOOEM( pszMemName );
            }
            else
            {
              usReply = MBID_YES;
            } /* endif */

            /**********************************************************/
            /* Disconnect TM if user response is YES                  */
            /**********************************************************/
            if ( usReply == MBID_YES )
            {
              BOOL fOK = TRUE;                  // internal O.K. flag
              CHAR szProp[MAX_EQF_PATH];

              /*******************************************************/
              /* Setup object name of TM                             */
              /*******************************************************/
              UtlMakeEQFPath( szProp, NULC, SYSTEM_PATH, NULL );
              strcat( szProp, BACKSLASH_STR );
              strcat( szProp, szMemShortName );
              strcat( szProp, GetPropFileExtension(pDelTmIn->szFullTmName) );

              /*******************************************************/
              /* Check if TM is locked                               */
              /*******************************************************/
              {
                SHORT sRC;

                sRC = QUERYSYMBOL( szProp );
                if ( sRC != -1 )
                {
                  PSZ pszErrParm = pszMemName;
                  OEMTOANSI( pszMemName );
                  UtlErrorHwnd( ERROR_MEM_NOT_DELETED,
                                MB_OK, 1, &pszErrParm,
                                EQF_ERROR, hwndOwner );
                  ANSITOOEM( pszMemName );
                  fOK = FALSE;
                } /* endif */
              }

              /*******************************************************/
              /* Setup property file name                            */
              /*******************************************************/
              UtlMakeEQFPath( szProp, NULC, PROPERTY_PATH, NULL );
              strcat( szProp, BACKSLASH_STR );
              strcat( szProp, szMemShortName );
              strcat( szProp, GetPropFileExtension(pDelTmIn->szFullTmName) );

              /*****************************************************/
              /* Delete local property file                        */
              /*****************************************************/
              if ( fOK )
              {
                USHORT usRC = UtlDelete( szProp, 0L, FALSE );
                fOK = ( usRC == NO_ERROR );
              } /* endif */

              /*****************************************************/
              /* Broadcast the EQFN_DELETED message                */
              /*****************************************************/
              if ( fOK )
              {
                UtlMakeEQFPath( szProp, NULC, SYSTEM_PATH, NULL );
                strcat( szProp, BACKSLASH_STR );
                strcat( szProp, szMemShortName );
                strcat( szProp, GetPropFileExtension(pDelTmIn->szFullTmName) );

                if ( hwndOwner == HWND_FUNCIF )
                {
                  ObjBroadcast( WM_EQFN_DELETED, clsMEMORYDB, szProp );
                }
                else
                {
                  EqfSend2AllHandlers( WM_EQFN_DELETED,
                                       MP1FROMSHORT( clsMEMORYDB ),
                                       MP2FROMP( szProp ) );
                } /* endif */
              } /* endif */
            } /* endif */

            /**********************************************************/
            /* Avoid normal delete processing                         */
            /* = immediately leave function                           */
            /**********************************************************/
            return( NO_ERROR );
          } /* endif */
        } /* endif */
      } /* endif */


      if ( fOk )
      {
        /**************************************************************/
        /* open the TM exclusively                                    */
        /**************************************************************/
        usRc = C_TmOpen( pDelTmIn->szFullTmName, &htm,
                        EXCLUSIVE, TM_LOCAL, FALSE, 0 );
        switch ( usRc )
        {
          //-------------------------------------------------------------------
          case NO_ERROR:
          case FILE_MIGHT_BE_CORRUPTED :
          case VERSION_MISMATCH :
          case CORRUPT_VERSION_MISMATCH :
          case TM_FILE_SCREWED_UP :
          case NOT_A_MEMORY_DATABASE :
          case BTREE_CORRUPTED:
            /********************************************************/
            /* Let the user confirm about the delete                */
            /********************************************************/
            OEMTOANSI( pszMemName );
            pReplAddr[0] = pszMemName;
            if ( (hwndOwner != HWND_FUNCIF) && !ISBATCHHWND(hwndOwner) )
            {
              if ( *pusYesToAllMode == MB_EQF_YESTOALL )
              {
                usReply = UtlError( (SHORT)(fShared ? WARNING_DEL_OWN_REM_TM  : WARNING_DELETE_MEMORY),
                                    MB_EQF_YESTOALL | MB_DEFBUTTON2, 1,
                                    &pReplAddr[0], EQF_QUERY );
                if ( usReply == MBID_EQF_YESTOALL )
                {
                  *pusYesToAllMode = usReply = MBID_YES;
                } /* endif */
              }
              else if ( *pusYesToAllMode == MBID_YES )
              {
                // user has choosen yes-to-all already
                usReply = MBID_YES;
              }
              else
              {
                usReply = UtlError( (SHORT)(fShared ? WARNING_DEL_OWN_REM_TM : WARNING_DELETE_MEMORY),
                                    MB_YESNO | MB_DEFBUTTON2, 1,
                                    &pReplAddr[0], EQF_QUERY );
              } /* endif */
            }
            else
            {
              usReply = MBID_YES;
            } /* endif */
            ANSITOOEM( pszMemName );
            break;
          case BTREE_IN_USE:
          case BTREE_DICT_LOCKED:
            /**********************************************************/
            /* display message, but do nothing else...                */
            /**********************************************************/
            OEMTOANSI( pszMemName );
            usRc = MemRcHandlingHwnd( usRc, pszMemName, &htm, NULL, hwndOwner );
            ANSITOOEM( pszMemName );
            usMsgHandling = FALSE;
            usReply = MBID_NO;
            break;
          //-------------------------------------------------------------------
          default:
            if ( fDeleteGrayedOut )
            {
              /******************************************************/
              /* Tm is greyed out, delete TM anyway, do not display */
              /* error messages                                     */
              /******************************************************/
              usReply = MBID_YES;
              usMsgHandling = FALSE;
            }
            else
            {
              /******************************************************/
              /* TM is not grayed - ask user if he wants to delete  */
              /* the tm                                             */
              /******************************************************/
              pReplAddr[0] = pszMemName;
              if ( (hwndOwner != HWND_FUNCIF) && !ISBATCHHWND(hwndOwner) )
              {
                if ( *pusYesToAllMode == MB_EQF_YESTOALL )
                {
                  usReply = UtlError( QUERY_MEM_DELETE_GREYEDOUT,
                                      MB_EQF_YESTOALL, 1,
                                      &pReplAddr[0], EQF_QUERY );
                  if ( usReply == MBID_EQF_YESTOALL )
                  {
                    *pusYesToAllMode = usReply = MBID_YES;
                  } /* endif */
                }
                else if ( *pusYesToAllMode == MBID_YES )
                {
                  // user has choosen yes-to-all already
                  usReply = MBID_YES;
                }
                else
                {
                  usReply = UtlError( QUERY_MEM_DELETE_GREYEDOUT,
                                      MB_YESNO, 1, &pReplAddr[0], EQF_QUERY );
                } /* endif */
              }
              else
              {
                usReply = MBID_YES;
              } /* endif */
              if ( usReply == MBID_NO )
              {
                usMsgHandling = FALSE;    // stop further messages
              } /* endif */

            } /* endif */
            fOk = FALSE;
            break;
        } /* endswitch */

        switch ( usReply )
        {
          //-----------------------------------------------------------------
          case MBID_YES :
            /**********************************************************/
            /* Close the local TM property file if it was open        */
            /**********************************************************/
            if ( hProp )
            {
              CloseProperties( hProp, PROP_QUIT, &ErrorInfo);
              hProp = NULL;
            } /* endif */

            /********************************************************/
            /* fill the rest of delete input structure              */
            /********************************************************/
            pDelTmIn->stPrefixIn.usLengthInput   = sizeof( TMX_DELTM_IN );
            pDelTmIn->stPrefixIn.usTmCommand     = TMC_DELETE_TM;

            /************************************************************/
            /* call U code to pass TM command to server or handle it    */
            /* the TM and its property file is deleted and TM is closed */
            /************************************************************/
            usRc = TmtXDeleteTM( htm, pDelTmIn, pDelTmOut );
//          usRc = U( htm, (PXIN)pDelTmIn, (PXOUT)pDelTmOut, NEW_TM );
            if ( usRc != NO_ERROR )
            {
              /********************************************************/
              /* reset error condition (File_NOT_FOUND) if we know,   */
              /* that there are files missing (fOk = FALSE)           */
              /********************************************************/
              if (!fOk && (usRc == ERROR_FILE_NOT_FOUND))
              {
                usRc = 0;
              } /* endif */
              fOk = FALSE;
            } /* endif */

            /**********************************************************/
            /* For shared TMs there is one more file which has to be  */
            /* deleted: the shared property file located in the MEM   */
            /* directory on the TM target drive                       */
            /**********************************************************/
            if ( fShared )
            {
              CHAR szProp[MAX_EQF_PATH];

              UtlMakeEQFPath( szProp, pDelTmIn->szFullTmName[0], MEM_PATH,
                              NULL );
              strcat( szProp, BACKSLASH_STR );
              strcat( szProp, szMemShortName );
              strcat( szProp, EXT_OF_SHARED_MEMPROP );

              UtlDelete( szProp, 0L, FALSE );
            } /* endif */

            break;
          //-----------------------------------------------------------------
          case MBID_NO :
            /********************************************************/
            /* close the TM if it was open                          */
            /********************************************************/
            if ( htm )
            {
              usRc = C_TmClose( htm, pDelTmIn->szFullTmName, FALSE, 0 );

              if ( usRc != NO_ERROR )
              {
                fOk = FALSE;
              } /* endif */
            } /* endif */
            break;
          case MBID_CANCEL :
            if ( htm )
            {
              usRc = C_TmClose( htm, pDelTmIn->szFullTmName, FALSE, 0 );

              if ( usRc != NO_ERROR )
              {
                fOk = FALSE;
              } /* endif */
              if ( pusYesToAllMode != NULL  )
              {
                *pusYesToAllMode = MBID_CANCEL;
              } /* endif */
            } /* endif */
            break;
          //-----------------------------------------------------------------
          default :
            break;
        } /* endswitch */

        /**************************************************************/
        /* Close the local TM property file if open                   */
        /**************************************************************/
        if ( hProp )
        {
          CloseProperties( hProp, PROP_QUIT, &ErrorInfo);
          hProp = NULL;
        } /* endif */

        /******************************************************************/
        /* Send a message to all handlers, that TM was deleted and        */
        /* to refresh the TM list window                                  */
        /******************************************************************/
        if ( usReply == MBID_YES )
        {
          CHAR    szObjName[MAX_EQF_PATH];

          sprintf( szObjName, "%s\\%s", szSysPath, szPropFileName );
          if ( hwndOwner == HWND_FUNCIF )
          {
            ObjBroadcast( WM_EQFN_DELETED, clsMEMORYDB, szObjName );
          }
          else
          {
            EqfSend2Handler( MEMORYHANDLER, WM_EQFN_DELETED,
                             MP1FROMSHORT( clsMEMORYDB ),
                             MP2FROMP( szObjName ));
          } /* endif */
        } /* endif */
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* creation of TM name via send2handler failed                  */
      /****************************************************************/
      fOk = FALSE;
    } /* endif */

    /******************************************************************/
    /* Free allocated storage                                         */
    /******************************************************************/
    UtlAlloc( (PVOID *) &pDelTmIn, 0L, 0L, NOMSG );
  }
  else
  {
    /******************************************************************/
    /* error from UtlAlloc set rc                                     */
    /******************************************************************/
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  if ( !fOk && usMsgHandling && (usRc != NO_ERROR) )
  {
    usRc = MemRcHandlingHwnd( usRc, pszMemName, &htm, NULL, hwndOwner );
  } /* endif */

  return usRc;
} /* end of function TmDeleteTM */
USHORT
TmDeleteTM( PSZ     pszMemName,            //in)  TM name MEM.TMD
            USHORT  usMsgHandling,         //(in)  message handling parameter
                                           //      TRUE:  display error message
                                           //      FALSE: display no error message
            HWND    hwndOwner,
            PUSHORT pusYesToAllMode )      //(in/out) Yes-to-all mode
{
	return C_TmDeleteTM(pszMemName, usMsgHandling, hwndOwner, pusYesToAllMode);
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmDeleteSegment
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmDeleteSegment( HTM           htm,
//                                     PSZ           szMemPath,
//                                     PTMX_PUT_IN   pDelIn,
//                                     PTMX_PUT_OUT  pDelOut,
//                                     USHORT        usMsgHandling )
//------------------------------------------------------------------------------
// Description:       Deletes a segment in a TM
//------------------------------------------------------------------------------
// Parameters:        htm           - (in)  TM handle
//                    szMemPath     - (in)  full TM name x:\eqf\mem\mem.tmd
//                    pDelIn        - (in)  pointer to delete input structure
//                    pDelOut       - (in)  pointer to delete output structure
//                    usMsgHandling - (in)  message handling parameter
//                                          TRUE:  display error message
//                                          FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
C_TmDeleteSegmentW(HTM            htm,
                 PSZ            szMemPath,
                 PTMX_PUT_IN_W  pDelInW,
                 PTMX_PUT_OUT_W pDelOutW,
                 USHORT         usMsgHandling )
{
  USHORT usTmtRc;
  pDelInW->stPrefixIn.usLengthInput = sizeof( TMX_PUT_IN_W );
  pDelInW->stPrefixIn.usTmCommand   = TMC_DELETE;

  usTmtRc = TmtXDelSegm ( (PTMX_CLB)htm, pDelInW, pDelOutW );
  /********************************************************************/
  /* Perform appropriate error handling if requested and required     */
  /********************************************************************/
  if ( usMsgHandling && usTmtRc )
    usTmtRc = MemRcHandling( usTmtRc, szMemPath, &htm, NULL );

  return usTmtRc;

}

USHORT
TmDeleteSegmentW(HTM            htm,
                 PSZ            szMemPath,
                 PTMX_PUT_IN_W  pDelInW,
                 PTMX_PUT_OUT_W pDelOutW,
                 USHORT         usMsgHandling )
{
	return C_TmDeleteSegmentW(htm,
                 szMemPath,
                 pDelInW,
                 pDelOutW,
                 usMsgHandling );
}
USHORT
C_TmDeleteSegment( HTM           htm,
                 PSZ           szMemPath,
                 PTMX_PUT_IN   pDelIn,
                 PTMX_PUT_OUT  pDelOut,
                 USHORT        usMsgHandling )
{
  USHORT            usRc = TRUE;             // Process return code
  ULONG             ulCP = 0L;

  PTMX_PUT_IN_W  pDelInW = NULL;
  PTMX_PUT_OUT_W pDelOutW = NULL;

  ulCP = GetLangOEMCP( NULL);
  UtlAlloc( (PVOID *) &pDelInW, 0L, sizeof( TMX_PUT_IN_W ), NOMSG );
  UtlAlloc( (PVOID *) &pDelOutW, 0L, sizeof( TMX_PUT_OUT_W), NOMSG );

  TMX_PUT_IN_ASCII2Unicode( pDelIn, pDelInW, ulCP );

  usRc = C_TmDeleteSegmentW( htm, szMemPath, pDelInW, pDelOutW, usMsgHandling );

  TMX_PUT_OUT_Unicode2ASCII( pDelOutW, pDelOut );

  UtlAlloc( (PVOID *) &pDelInW, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pDelOutW, 0L, 0L, NOMSG );


  return usRc;
} /* End of function TmDeleteSegment */

USHORT
TmDeleteSegment( HTM           htm,
                 PSZ           szMemPath,
                 PTMX_PUT_IN   pDelIn,
                 PTMX_PUT_OUT  pDelOut,
                 USHORT        usMsgHandling )
{
	return C_TmDeleteSegment(htm,
                 szMemPath,
                 pDelIn,
                 pDelOut,
                 usMsgHandling );
}


// Unicode additions
// I cannot find any call to TMX_GET_IN_Unicode2ASCII??


VOID TMX_GET_IN_Unicode2ASCII( PTMX_GET_IN_W pstGetInW, PTMX_GET_IN pstGetIn, ULONG cp )
{
  PTMX_GET   pstTmGet = &pstGetIn->stTmGet;
  PTMX_GET_W pstTmGetW= &pstGetInW->stTmGet;

  Unicode2ASCII( pstTmGetW->szSource, pstTmGet->szSource, cp );
    strcpy( pstTmGet->szTagTable, pstTmGetW->szTagTable );
    strcpy( pstTmGet->szSourceLanguage, pstTmGetW->szSourceLanguage );
    strcpy( pstTmGet->szFileName, pstTmGetW->szFileName );
    memcpy( &pstTmGet->szLongName, &pstTmGetW->szLongName, sizeof( LONG_FN ));
    pstTmGet->usSegmentId        = (USHORT)pstTmGetW->ulSegmentId;
    strcpy( pstTmGet->szAuthorName, pstTmGetW->szAuthorName );
    strcpy( pstTmGet->szTargetLanguage, pstTmGetW->szTargetLanguage );
    pstTmGet->usRequestedMatches = pstTmGetW->usRequestedMatches;
    pstTmGet->usMatchThreshold   = pstTmGetW->usMatchThreshold;
    pstTmGet->usConvert          = pstTmGetW->usConvert;
    pstTmGet->usParm             = (USHORT)pstTmGetW->ulParm;

    memcpy( &pstGetIn->stPrefixIn, &pstGetInW->stPrefixIn, sizeof( TMX_PREFIX_IN ));

}

VOID TMX_GET_IN_ASCII2Unicode( PTMX_GET_IN pstGetIn, PTMX_GET_IN_W pstGetInW, ULONG cp )
{

  PTMX_GET   pstTmGet = &pstGetIn->stTmGet;
  PTMX_GET_W pstTmGetW= &pstGetInW->stTmGet;

  ASCII2Unicode( pstTmGet->szSource, pstTmGetW->szSource , cp);
    strcpy( pstTmGetW->szTagTable, pstTmGet->szTagTable );
    strcpy( pstTmGetW->szSourceLanguage, pstTmGet->szSourceLanguage );
    strcpy( pstTmGetW->szFileName, pstTmGet->szFileName );
    memcpy( &pstTmGetW->szLongName, &pstTmGet->szLongName, sizeof( LONG_FN ));
    pstTmGetW->ulSegmentId        = pstTmGet->usSegmentId;
    strcpy( pstTmGetW->szAuthorName, pstTmGet->szAuthorName );
    strcpy( pstTmGetW->szTargetLanguage, pstTmGet->szTargetLanguage );
    pstTmGetW->usRequestedMatches = pstTmGet->usRequestedMatches;
    pstTmGetW->usMatchThreshold   = pstTmGet->usMatchThreshold;
    pstTmGetW->usConvert          = pstTmGet->usConvert;
    pstTmGetW->ulParm             = pstTmGet->usParm;

    memcpy( &pstGetInW->stPrefixIn, &pstGetIn->stPrefixIn, sizeof( TMX_PREFIX_IN ));

}

VOID TMX_GET_OUT_Unicode2ASCII( PTMX_GET_OUT_W pstGetOutW, PTMX_GET_OUT pstGetOut, ULONG cp )
{
  int i;

  PTMX_MATCH_TABLE pstMatchTable = &pstGetOut->stMatchTable[0];
  PTMX_MATCH_TABLE_W pstMatchTableW = &pstGetOutW->stMatchTable[0];
    for (i=0; i<pstGetOutW->usNumMatchesFound; i++)
    {
    Unicode2ASCII( pstMatchTableW->szSource, pstMatchTable->szSource, cp );
    Unicode2ASCII( pstMatchTableW->szTarget, pstMatchTable->szTarget, cp );

    strcpy( pstMatchTable->szFileName, pstMatchTableW->szFileName );
    strcpy( pstMatchTable->szTargetLanguage, pstMatchTableW->szTargetLanguage );
    strcpy( pstMatchTable->szTargetAuthor, pstMatchTableW->szTargetAuthor );
    strcpy( pstMatchTable->szTagTable,   pstMatchTableW->szTagTable );
    strcpy( pstMatchTable->szLongName,   pstMatchTableW->szLongName );
        pstMatchTable->usSegmentId  = (USHORT)pstMatchTableW->ulSegmentId;
        pstMatchTable->usTranslationFlag = pstMatchTableW->usTranslationFlag;
        pstMatchTable->lTargetTime  = pstMatchTableW->lTargetTime;
        pstMatchTable->usMatchLevel = pstMatchTableW->usMatchLevel;
        pstMatchTable->usOverlaps   = pstMatchTableW->usOverlaps;
        pstMatchTable->ulKey        = pstMatchTableW->ulKey;
        pstMatchTable->usTargetNum  = pstMatchTableW->usTargetNum;
        pstMatchTable->usDBIndex    = pstMatchTableW->usDBIndex;

        pstMatchTable++; pstMatchTableW++;
  }
    memcpy( &pstGetOut->stPrefixOut, &pstGetOutW->stPrefixOut, sizeof(TMX_PREFIX_OUT) );
  pstGetOut->usNumMatchesFound = pstGetOutW->usNumMatchesFound;
  pstGetOut->fsAvailFlags      = pstGetOutW->fsAvailFlags;
}

VOID TMX_GET_OUT_ASCII2Unicode( PTMX_GET_OUT pstGetOut, PTMX_GET_OUT_W pstGetOutW, ULONG cp )
{
  int i;

  PTMX_MATCH_TABLE pstMatchTable = &pstGetOut->stMatchTable[0];
  PTMX_MATCH_TABLE_W pstMatchTableW = &pstGetOutW->stMatchTable[0];
    for (i=0; i<pstGetOut->usNumMatchesFound; i++)
    {
    ASCII2Unicode( pstMatchTable->szSource,         pstMatchTableW->szSource, cp );
    ASCII2Unicode( pstMatchTable->szTarget,         pstMatchTableW->szTarget, cp );

    strcpy( pstMatchTableW->szFileName,       pstMatchTable->szFileName );
    strcpy( pstMatchTableW->szTargetLanguage, pstMatchTable->szTargetLanguage );
    strcpy( pstMatchTableW->szTargetAuthor,   pstMatchTable->szTargetAuthor );
    strcpy( pstMatchTableW->szTagTable,       pstMatchTable->szTagTable );

    strcpy( pstMatchTableW->szLongName,       pstMatchTable->szLongName );
        pstMatchTableW->ulSegmentId  = pstMatchTable->usSegmentId;
         pstMatchTableW->usTranslationFlag = pstMatchTable->usTranslationFlag;
        pstMatchTableW->lTargetTime  = pstMatchTable->lTargetTime;
        pstMatchTableW->usMatchLevel = pstMatchTable->usMatchLevel;
        pstMatchTableW->usOverlaps   = pstMatchTable->usOverlaps;
        pstMatchTableW->ulKey        = pstMatchTable->ulKey;
        pstMatchTableW->usTargetNum  = pstMatchTable->usTargetNum;
        pstMatchTableW->usDBIndex    = pstMatchTable->usDBIndex;

        pstMatchTable++; pstMatchTableW++;
    }
    memcpy( &pstGetOutW->stPrefixOut, &pstGetOut->stPrefixOut, sizeof(TMX_PREFIX_OUT) );
    pstGetOutW->usNumMatchesFound = pstGetOut->usNumMatchesFound;
  pstGetOutW->fsAvailFlags      = pstGetOut->fsAvailFlags;

}


VOID  TMX_PUT_IN_Unicode2ASCII( PTMX_PUT_IN_W pstPutInW, PTMX_PUT_IN pstPutIn, ULONG cp )
{
  PTMX_PUT   pstTmPut = &pstPutIn->stTmPut;
  PTMX_PUT_W pstTmPutW= &pstPutInW->stTmPut;

  Unicode2ASCII( pstTmPutW->szSource, pstTmPut->szSource, cp );
  Unicode2ASCII( pstTmPutW->szTarget, pstTmPut->szTarget, cp );

  strcpy( pstTmPut->szSourceLanguage, pstTmPutW->szSourceLanguage );
  strcpy( pstTmPut->szTargetLanguage, pstTmPutW->szTargetLanguage );
  strcpy( pstTmPut->szAuthorName, pstTmPutW->szAuthorName );
  pstTmPut->usTranslationFlag = pstTmPutW->usTranslationFlag;
  strcpy( pstTmPut->szFileName, pstTmPutW->szFileName );
  strcpy( pstTmPut->szLongName, pstTmPutW->szLongName );
  pstTmPut->usSourceSegmentId = (USHORT)pstTmPutW->ulSourceSegmentId;
  strcpy( pstTmPut->szTagTable, pstTmPutW->szTagTable );
  pstTmPut->lTime = pstTmPutW->lTime;

  memcpy( &pstPutIn->stPrefixIn, &pstPutInW->stPrefixIn, sizeof( TMX_PREFIX_IN ));
}

VOID  TMX_PUT_IN_ASCII2Unicode( PTMX_PUT_IN pstPutIn, PTMX_PUT_IN_W pstPutInW, ULONG cp )
{
  PTMX_PUT   pstTmPut = &pstPutIn->stTmPut;
  PTMX_PUT_W pstTmPutW= &pstPutInW->stTmPut;

  ASCII2Unicode( pstTmPut->szSource, pstTmPutW->szSource, cp );
  ASCII2Unicode( pstTmPut->szTarget, pstTmPutW->szTarget, cp );

  strcpy( pstTmPutW->szSourceLanguage, pstTmPut->szSourceLanguage );
  strcpy( pstTmPutW->szTargetLanguage, pstTmPut->szTargetLanguage );
  strcpy( pstTmPutW->szAuthorName, pstTmPut->szAuthorName );
  pstTmPutW->usTranslationFlag = pstTmPut->usTranslationFlag;
  strcpy( pstTmPutW->szFileName, pstTmPut->szFileName );
  strcpy( pstTmPutW->szLongName, pstTmPut->szLongName );
  pstTmPutW->ulSourceSegmentId = pstTmPut->usSourceSegmentId;
  strcpy( pstTmPutW->szTagTable, pstTmPut->szTagTable );
  pstTmPutW->lTime = pstTmPut->lTime;

  memcpy( &pstPutInW->stPrefixIn, &pstPutIn->stPrefixIn, sizeof( TMX_PREFIX_IN ));
}


VOID  TMX_PUT_OUT_ASCII2Unicode( PTMX_PUT_OUT pstPutOut, PTMX_PUT_OUT_W pstPutOutW )
{
  memcpy( &pstPutOutW->stPrefixOut, &pstPutOut->stPrefixOut, sizeof( TMX_PREFIX_OUT ));
}

VOID  TMX_PUT_OUT_Unicode2ASCII( PTMX_PUT_OUT_W pstPutOutW, PTMX_PUT_OUT pstPutOut )
{
  memcpy( &pstPutOut->stPrefixOut, &pstPutOutW->stPrefixOut, sizeof( TMX_PREFIX_OUT ));
}


VOID  TMX_EXT_IN_Unicode2ASCII( PTMX_EXT_IN_W pstExtInW, PTMX_EXT_IN pstExtIn )
{
  // PTMX_EXT_IN_W same as PTMX_EXT_IN
  memcpy( pstExtIn, pstExtInW, sizeof( TMX_EXT_IN ) );
}

VOID  TMX_EXT_IN_ASCII2Unicode( PTMX_EXT_IN pstExtIn, PTMX_EXT_IN_W pstExtInW )
{
  // PTMX_EXT_IN_W same as PTMX_EXT_IN
  memcpy( pstExtInW, pstExtIn, sizeof( TMX_EXT_IN ) );
}



VOID  TMX_EXT_OUT_ASCII2Unicode( PTMX_EXT_OUT pstExtOut, PTMX_EXT_OUT_W pstExtOutW, ULONG cp )
{
  PTMX_EXT   pstTmExt = &pstExtOut->stTmExt;
  PTMX_EXT_W pstTmExtW= &pstExtOutW->stTmExt;

  ASCII2Unicode( pstTmExt->szSource, pstTmExtW->szSource, cp );
  ASCII2Unicode( pstTmExt->szTarget, pstTmExtW->szTarget, cp );
  strcpy( pstTmExtW->szTagTable, pstTmExt->szTagTable );
  strcpy( pstTmExtW->szTargetLanguage, pstTmExt->szTargetLanguage );
  strcpy( pstTmExtW->szAuthorName, pstTmExt->szAuthorName );
  pstTmExtW->usTranslationFlag = pstTmExt->usTranslationFlag;
  strcpy( pstTmExtW->szFileName, pstTmExt->szFileName );
  strcpy( pstTmExtW->szLongName, pstTmExt->szLongName );
  pstTmExtW->lTargetTime = pstTmExt->lTargetTime;
  memcpy( &pstExtOutW->stPrefixOut, &pstExtOut->stPrefixOut, sizeof( TMX_PREFIX_OUT ));
  pstExtOutW->usNextTarget = pstExtOut->usNextTarget;
  pstExtOutW->ulTmKey      = pstExtOut->ulTmKey;
  pstExtOutW->ulMaxEntries = pstExtOut->ulMaxEntries;
  pstExtOutW->usNextTarget = pstExtOut->usNextTarget;

}

VOID  TMX_EXT_OUT_Unicode2ASCII( PTMX_EXT_OUT_W pstExtOutW, PTMX_EXT_OUT pstExtOut, ULONG cp )
{
  PTMX_EXT   pstTmExt = &pstExtOut->stTmExt;
  PTMX_EXT_W pstTmExtW= &pstExtOutW->stTmExt;

  Unicode2ASCII( pstTmExtW->szSource, pstTmExt->szSource, cp );
  Unicode2ASCII( pstTmExtW->szTarget, pstTmExt->szTarget, cp );
  strcpy( pstTmExt->szTagTable, pstTmExtW->szTagTable );
  strcpy( pstTmExt->szTargetLanguage, pstTmExtW->szTargetLanguage );
  strcpy( pstTmExt->szAuthorName, pstTmExtW->szAuthorName );
   pstTmExt->usTranslationFlag = pstTmExtW->usTranslationFlag;
  pstTmExt->usSourceSegmentId = (USHORT)pstTmExtW->ulSourceSegmentId;
  strcpy( pstTmExt->szFileName, pstTmExtW->szFileName );
  strcpy( pstTmExt->szLongName, pstTmExtW->szLongName );
  pstTmExt->lTargetTime = pstTmExtW->lTargetTime;
  memcpy( &pstExtOut->stPrefixOut, &pstExtOutW->stPrefixOut, sizeof( TMX_PREFIX_OUT ));
  pstExtOut->ulTmKey = pstExtOutW->ulTmKey;
  pstExtOut->ulMaxEntries = pstExtOutW->ulMaxEntries;
  pstExtOut->usNextTarget = pstExtOutW->usNextTarget;
}


// do a cleanup of temporary memories
void TMCleanupTempMem
(
  PSZ         pszPrefix                                     // ptr to memory prefix
)
{
  static char szSearchPath[MAX_EQF_PATH];
  static WIN32_FIND_DATA FindData;                         // file find structure
  HANDLE hFind;
  PSZ    pszName;
  BOOL   fMoreFiles = TRUE;

  // setup compare date (3 days before today)
  SYSTEMTIME st;
  FILETIME ft;
  LARGE_INTEGER liCompare;
  GetSystemTime( &st );
  SystemTimeToFileTime( &st, &ft );
  liCompare.LowPart = ft.dwLowDateTime;
  liCompare.HighPart = ft.dwHighDateTime;
  liCompare.QuadPart = liCompare.QuadPart - 2592000000000; // subtract 3 days

  // setup search path
  UtlMakeEQFPath( szSearchPath, NULC, MEM_PATH, NULL );
  strcat( szSearchPath, BACKSLASH_STR );
  pszName = szSearchPath + strlen( szSearchPath );
  strcat( szSearchPath, pszPrefix );
  strcat( szSearchPath, "*.*" );

  // loop over temporary memory files
  hFind = FindFirstFile( szSearchPath, &FindData );
  if ( hFind != INVALID_HANDLE_VALUE )
  {
    do
    {
      LARGE_INTEGER liFile;
      if ( (FindData.ftLastWriteTime.dwLowDateTime != 0) || (FindData.ftLastWriteTime.dwHighDateTime != 0) )
      {
        liFile.LowPart = FindData.ftLastWriteTime.dwLowDateTime;
        liFile.HighPart = FindData.ftLastWriteTime.dwHighDateTime;
      }
      else if ( (FindData.ftCreationTime.dwLowDateTime != 0) || (FindData.ftCreationTime.dwHighDateTime != 0) )
      {
        liFile.LowPart = FindData.ftCreationTime.dwLowDateTime;
        liFile.HighPart = FindData.ftCreationTime.dwHighDateTime;
      }
      else
      {
        liFile.LowPart = FindData.ftLastAccessTime.dwLowDateTime;
        liFile.HighPart = FindData.ftLastAccessTime.dwHighDateTime;
      } /* endif */

      {
        FILETIME ftCompare, ftFile;
        SYSTEMTIME stCompare, stFile;

        ftCompare.dwLowDateTime = liCompare.LowPart;
        ftCompare.dwHighDateTime = liCompare.HighPart;
        ftFile.dwLowDateTime = liFile.LowPart;
        ftFile.dwHighDateTime = liFile.HighPart;
        FileTimeToSystemTime( &ftCompare, &stCompare );
        FileTimeToSystemTime( &ftFile, &stFile );
        FileTimeToSystemTime( &ftFile, &stFile );
      }

      // delete file if it is older than our reference data
      if ( liFile.QuadPart < liCompare.QuadPart )
      {
        strcpy( pszName, FindData.cFileName );
        UtlDelete( szSearchPath, 0L, FALSE );
      } /* endif */

      // continue with next file
      fMoreFiles = FindNextFile( hFind, &FindData );
    } while ( fMoreFiles );
    FindClose( hFind );
  } /* endif */
} /* end of function TMCleanupTempMem */

// create a temporary memory
USHORT TMCreateTempMem
(
  PSZ         pszPrefix,                                    // short prefix to be used for memory name (should start with a dollar sign)
  PSZ         pszMemName,                                   // ptr to buffer for memory name
  HTM         *pHtm,                                        // ptr to buffer for memory handle
  HTM         htm,                                          // ptr to HTM of similar memory
  PSZ         pszSourceLang,                                // memory source language
  HWND        hwnd                                          // window handle for error messages
)
{
  char        szSemName[20];                                // name of semaphore
  char        szTempName[20];                               // name of temporary memory
  char        szMemPath[MAX_EQF_PATH];                       // name of temporary memory
  HANDLE      hMutexSem = NULLHANDLE;                       // handle of semaphore
  int         i = 0;
  BOOL        fCleanupTempMems = FALSE;                     // TRUE = start automatic memory name cleanup
  USHORT      usRC = 0;                                     // function return code


  // add a dollar sign if prefix does not start with one
  if ( *pszPrefix != '$' ) szTempName[i++] = '$';

  // add up to three characters of supplied prefix to temporary memory name
  while ( (i < 3) && (*pszPrefix != EOS) ) szTempName[i++] = *pszPrefix++;
  while ( i < 3 ) szTempName[i++] = 'M';
  szTempName[3] = EOS;


  // setup semaphore name
  sprintf( szSemName, "EQF-%s-MUTEX", szTempName );

  // ensure that no other process is currently looking for memory names
  hMutexSem = OpenMutex( MUTEX_ALL_ACCESS, TRUE, szSemName );
#ifdef SESSIONLOG
  if ( hMutexSem == NULL )
  {
    UtlLogWriteString( "TMCreateTempMemHwnd: Mutex %s did not exist", szSemName );
  }
  else
  {
    UtlLogWriteString( "TMCreateTempMemHwnd: Mutex %s exists", szSemName );
  } /* endif */
#endif
  if ( hMutexSem == NULL)
  {
    hMutexSem = CreateMutex( NULL, FALSE, szSemName );
#ifdef SESSIONLOG
    if ( hMutexSem == NULL )
    {
      int iRC = GetLastError();
      CHAR szRC[10];

      sprintf( szRC, "%ld", iRC );
      UtlLogWriteString2( "TMCreateTempMemHwnd: Creation of Mutex %s failed, RC=%s", szSemName, szRC );
    }
    else
    {
      UtlLogWriteString( "TMCreateTempMemHwnd: Mutex %s created successfully", szSemName );
    } /* endif */
#endif
  } /* endif */
  if ( hMutexSem )
  {
#ifdef SESSIONLOG
    UtlLogWriteString( "TMCreateTempMemHwnd: Waiting for Mutex %s", szSemName );
#endif
    WaitForSingleObject( hMutexSem, INFINITE );
#ifdef SESSIONLOG
    UtlLogWrite( "TMCreateTempMemHwnd: Wait complete" );
#endif
  } /* endif */

  // look for a free memory name reserve file
  {
    PSZ pszName;
    int i;             // loop counter

    UtlMakeEQFPath( szMemPath, NULC, MEM_PATH, NULL );
    strcat( szMemPath, BACKSLASH_STR );
    pszName = szMemPath + strlen( szMemPath );

    // find a not existing memory name reserve file
    i = 0;
    do
    {
      sprintf( pszName, "%s%5.5d.TXX", szTempName, i++ );
    } while ( UtlFileExist( szMemPath ) && (i < 1000) ); /* enddo */

    // check if a cleanup is necessary
    if ( i >= 10 )
    {
      fCleanupTempMems = TRUE;
    } /* endif */
  }

  // reserve current name
  {
    FILE *hf = fopen( szMemPath, "w" );
    if ( hf )
    {
      fwrite( "TXX", 1, 3, hf );
      fclose( hf );
    }
    else
    {
      usRC = ERROR_TA_TMERROR;
    } /* endif */
  }

  // allow other processes to look for free memory names
  if ( hMutexSem )
  {
#ifdef SESSIONLOG
    UtlLogWriteString( "TMCreateTempMemHwnd: Releasing Mutex %s", szSemName );
#endif
    ReleaseMutex( hMutexSem );
    CloseHandle( hMutexSem );
  } /* endif */

  // create memory
  if ( usRC == 0 )
  {
    // delete any existing memory files
    strcpy( szMemPath + (strlen(szMemPath)-4), EXT_OF_TMINDEX );
    UtlDelete( szMemPath, 0L, FALSE );
    strcpy( szMemPath + (strlen(szMemPath)-4), EXT_OF_TMDATA );
    UtlDelete( szMemPath, 0L, FALSE );

    // create new memory
    usRC = TmCreate( szMemPath, pHtm, htm, "", "", pszSourceLang, "", TRUE, hwnd );

    // store memory name in callers buffer
    Utlstrccpy( pszMemName, UtlGetFnameFromPath(szMemPath), '.' );
  } /* endif */

  // do a cleanup of older temporary memories
  if (  fCleanupTempMems )
  {
    TMCleanupTempMem( szTempName );
  } /* endif */

  return( usRC );
} /* end of function TMCreateTempMemHwnd */

// delete a temporary memory
void TMDeleteTempMem
(
  PSZ         pszMemName                                    // ptr to memory name
)
{
  char        szMemPath[MAX_EQF_PATH];                       // name of temporary memory

  UtlMakeEQFPath( szMemPath, NULC, MEM_PATH, NULL );
  strcat( szMemPath, BACKSLASH_STR );
  strcat( szMemPath, pszMemName );
  strcat( szMemPath, EXT_OF_TMDATA);
  UtlDelete( szMemPath, 0L, FALSE );
  strcpy( szMemPath + (strlen(szMemPath)-4), EXT_OF_TMINDEX );
  UtlDelete( szMemPath, 0L, FALSE );
  strcpy( szMemPath + (strlen(szMemPath)-4), ".TXX" );
  UtlDelete( szMemPath, 0L, FALSE );
} /* end of function TMDeleteTempMemHwnd */
