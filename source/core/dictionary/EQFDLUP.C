//+----------------------------------------------------------------------------+
//|  EQFDLUP.C  - Dictionary lookup and edit services                          |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                             |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//|     LupBegin      - Begin/Initialize Dictionary Lookup Services            |
//|     LupEnd        - Terminate Dictionary Lookup Services                   |
//|     LupLookup     - Lookup a term                                          |
//|     LupEdit       - Edit a term                                            |
//|     LupSelTerm    - Term selection dialog                                  |
//|                                                                           |
//|  +-- status ('H'= Header, 'D'=Design, 'C'=Code, 'T'=Test, ' '=complete,    |
//|              'Q'=Quick-and-dirty )                                         |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//|     UtlAlloc                - General memory allocation routine            |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//|     ---------------------------------------------------------------------  |
//|  C  Lupxxxxxx               - add a new term to a term list                |
//|                                                                           |
//|  +-- status ('H'= Header, 'D'=Design, 'C'=Code, 'T'=Test, ' '=complete,    |
//|              'Q'=Quick-and-dirty )                                         |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFDASDI.H"             // private ASD+LUP services header file
#include "OtmDictionaryIF.H"
#include "EQFUTMDI.H"             // support for MDI dialogs

static USHORT usCurrentID = 1;    // ID to be used for next lookup CB

//+----------------------------------------------------------------------------+
//| LupBegin            Begin/Initialize Dictionary Lookup Services            |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    This function initializes the lookup services: loads resource module,   |
//|    allocates memory and preloads the dialogs.                              |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB    hUCB,          IN    Asd services user control block            |
//|    HDCB    hDCB,          IN    Asd services dictioanry control block      |
//|    HWND    hwndParent,    IN    parent handle for dialogs and messages     |
//|    USHORT  usNotifyMsg,   IN    message used for notifications             |
//|    PRECTL  prclDisplay,   IN    size/position of display dialog or NULL    |
//|    PRECTL  prclEdit,      IN    size/position of edit dialog or NULL       |
//|    PHLUPCB phLUPCB        OUT   Lookup services control block              |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LUP_OK              if OK                          |
//|                         LUP_MEM_ALLOC_ERR   if memory allocation failed    |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
USHORT LupBegin
(
   HUCB    hUCB,                       // Asd services user control block
   HDCB    hDCB,                       // Asd services dictioanry control block
   HWND    hwndParent,                 // parent handle for dialogs and messages
   USHORT  usNotifyMsg,                // message used for notifications
   PRECTL  prclDisplay,                // size/position of display dialog or NULL
   PRECTL  prclEdit,                   // size/position of edit dialog or NULL
   PHLUPCB phLUPCB,                    // Lookup services control block
   PUSHORT pusID                       // identifier for Lookup CB
)
{
  return( LupBegin2( hUCB, hDCB, hwndParent, usNotifyMsg, prclDisplay,
                     prclEdit, phLUPCB, pusID,
                     LUP_MDIDLG_MODE ) );
} /* end of function LupBegin */

USHORT LupBegin2
(
   HUCB    hUCB,                       // Asd services user control block
   HDCB    hDCB,                       // Asd services dictioanry control block
   HWND    hwndParent,                 // parent handle for dialogs and messages
   USHORT  usNotifyMsg,                // message used for notifications
   PRECTL  prclDisplay,                // size/position of display dialog or NULL
   PRECTL  prclEdit,                   // size/position of edit dialog or NULL
   PHLUPCB phLUPCB,                    // Lookup services control block
   PUSHORT pusID,                      // identifier for Lookup CB
   ULONG   ulFlags                     // flags for kind of lookup
)
{
   USHORT    usRC = LUP_OK;            // function return code
   PLUPCB    pLUPCB = NULL;            // pointer to lookup control block
   BOOL      fOK;                      // ok flag set by UtlAlloc call

   prclEdit;                           // avoid compiler warning

   //
   // check input parameters
   //
   if ( !AsdIsUcbOK( hUCB ) )
   {
      usRC = LUP_BAD_ASD_UCB;
   }
   else if ( !AsdIsDcbOK( hDCB ) )
   {
      usRC = LUP_BAD_ASD_DCB;
   } /* endif */

   //
   // allocate lookup control block
   //
   if ( usRC == LUP_OK )
   {
      fOK = UtlAlloc( (PVOID *)&pLUPCB, 0L, (LONG) sizeof(LUPCB), NOMSG );
      if ( !fOK )
      {
         usRC = LUP_MEM_ALLOC_ERR;
      } /* endif */
   } /* endif */

   //
   // fill lookup control block
   //
   if ( usRC == LUP_OK )
   {
      pLUPCB->lSignature  = LUPCB_SIGNATURE;
      pLUPCB->usID        = usCurrentID++;
      pLUPCB->hUCB        = hUCB;
      pLUPCB->hDCB        = hDCB;
      pLUPCB->hwndParent  = hwndParent;
      pLUPCB->usNotifyMsg = usNotifyMsg;
      pLUPCB->ulFlags     = ulFlags;
      if ( prclDisplay )
      {
         memcpy( &pLUPCB->rclDisp, prclDisplay, sizeof(RECTL) );
      } /* endif */
   } /* endif */

   //
   // cleanup
   //
   if ( usRC != LUP_OK )
   {
      if ( pLUPCB )
      {
         UtlAlloc( (PVOID *)&pLUPCB, 0L, 0L, NOMSG );
         pLUPCB = NULL;
      } /* endif */
   } /* endif */

   *phLUPCB = pLUPCB;                  // set caller's lookup control handle
   if ( pLUPCB != NULL )
   {
     *pusID = pLUPCB->usID;
   } /* endif */

   return( usRC );                     // return any error code to caller

} /* end of LupBegin */

//+----------------------------------------------------------------------------+
//| LupEnd              Terminate lookup services                              |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HLUPCB   hLUPCB        IN       lookup control block handle             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD    if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hLUPCB must have been created using LupBegin.                           |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT LupEnd
(
   HLUPCB hLUPCB                       // Lookup services control block
)
{
   USHORT    usRC = LUP_OK;            // function return code
   PLUPCB    pLUPCB;                   // pointer to lookup control block
   USHORT    usI;                      // loop counter

   pLUPCB = (PLUPCB) hLUPCB;           // convert handle to pointer

   if ( pLUPCB && (pLUPCB->lSignature == LUPCB_SIGNATURE) )
   {
      /****************************************************************/
      /* Terminate the lookup dialog if active                        */
      /****************************************************************/
      if ( WinIsWindow( (HAB)UtlQueryULong( QL_HAB ), pLUPCB->hwndLook ) )
      {
         SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIDESTROY,
                      MP1FROMHWND(pLUPCB->hwndLook), 0L ) ;
      } /* endif */

      /****************************************************************/
      /* Terminate any active display dialog                          */
      /* (Copy handles and number of display dialogs to a private     */
      /*  area as the number of dialogs will change and the dialogs   */
      /*  will be removed from the array when destroyed)              */
      /****************************************************************/
      {
        HWND     *pahwnd;              // ptr to buffer for display dlg HWNDs
        USHORT   usDlgs;               // number of active display dialogs

        usDlgs = pLUPCB->usDisplayDlgs;
        if ( UtlAlloc( (PVOID *)&pahwnd, 0L, sizeof(HWND) * MAX_DISP_DLGS,
                       ERROR_STORAGE ) )
        {
          memcpy( (PVOID)pahwnd, (PVOID)pLUPCB->ahwndDisplay,
                  sizeof(HWND) * MAX_DISP_DLGS );
          for ( usI = 0; usI < usDlgs; usI++ )
          {
            if ( WinIsWindow( (HAB)UtlQueryULong( QL_HAB ), pahwnd[usI] ) )
            {
               SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIDESTROY,
                            MP1FROMHWND(pahwnd[usI]), 0 ) ;
            } /* endif */
          } /* endfor */
          UtlAlloc( (PVOID *)&pahwnd, 0L, 0L, NOMSG );
        } /* endif */
      }

      if ( pLUPCB->lpfnDispProc != NULL )
      {
        FreeProcInstance( pLUPCB->lpfnDispProc );
      } /* endif */
      if ( pLUPCB->lpfnLookupProc != NULL )
      {
        FreeProcInstance( pLUPCB->lpfnLookupProc );
      } /* endif */
      UtlAlloc( (PVOID *)&pLUPCB, 0L, 0L, NOMSG );
   }
   else
   {
      usRC = LUP_BAD_LUPCB_ERR;
   } /* endif */

   return( usRC );                     // return Nlp RC to caller

} /* end of LupEnd */

//+----------------------------------------------------------------------------+
//| LupActivate         Activate lookup dialog or set focus to it              |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HLUPCB   hLUPCB        IN       lookup control block handle             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD    if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hLUPCB must have been created using LupBegin.                           |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT LupActivate
(
   HLUPCB hLUPCB                       // Lookup services control block
)
{
   USHORT    usRC = LUP_OK;            // function return code
   PLUPCB    pLUPCB;                   // pointer to lookup control block

   pLUPCB = (PLUPCB) hLUPCB;           // convert handle to pointer

   if ( pLUPCB && (pLUPCB->lSignature == LUPCB_SIGNATURE) )
   {
      /****************************************************************/
      /* Set focus to lookup dialog if window still exists            */
      /****************************************************************/
      if ( WinIsWindow( (HAB)UtlQueryULong( QL_HAB ), pLUPCB->hwndLook ) )
      {
           SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIACTIVATE,
                        MP1FROMHWND(pLUPCB->hwndLook), 0L ) ;
      }
      else
      {
        /**************************************************************/
        /* Lookup dialog has been closed, start it again              */
        /**************************************************************/
        LupLookup( hLUPCB, EMPTY_STRINGW );
      } /* endif */
   }
   else
   {
      usRC = LUP_BAD_LUPCB_ERR;
   } /* endif */

   return( usRC );                     // return Nlp RC to caller

} /* end of LupActivate */

//+----------------------------------------------------------------------------+
//| LupLookup           Look up a term and display it in the display dialog    |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HLUPCB   hLUPCB        IN       lookup control block handle             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD    if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hLUPCB must have been created using LupBegin.                           |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT LupLookup
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PCHAR_W  pucTerm                    // term being looked-up
)
{
   PLUPCB    pLUPCB = NULL;            // pointer to lookup control block
   USHORT    usRC = LUP_OK;            // function return code

   pLUPCB = (PLUPCB) hLUPCB;           // convert handle to pointer

   if ( !pLUPCB || (pLUPCB->lSignature != LUPCB_SIGNATURE) )
   {
     usRC = LUP_BAD_LUPCB_ERR;
   }
   else
   {
     usRC = LupLookup2( hLUPCB, pucTerm, NULLHANDLE );
   } /* endif */
  return( usRC );
}

USHORT LupLookup2
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PCHAR_W  pucTermW,                  // term being looked-up
   HWND     hwnd                       // parent handle for dialog box
)
{
   PLUPCB    pLUPCB = NULL;            // pointer to lookup control block
   USHORT    usRC = LUP_OK;            // function return code

   pLUPCB = (PLUPCB) hLUPCB;           // convert handle to pointer

   if ( !pLUPCB || (pLUPCB->lSignature != LUPCB_SIGNATURE) )
   {
      usRC = LUP_BAD_LUPCB_ERR;
   } /* endif */

   // load lookup dialog
   if ( usRC == LUP_OK )
   {
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      // load lookup dialogs
      if ( hwnd == NULLHANDLE )
      {
        hwnd = (HWND)(SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                         WM_MDIGETACTIVE, 0, 0L ) );
      } /* endif */
      TermLookup( pLUPCB, pucTermW, hResMod, hwnd );
   } /* endif */

   return( usRC );                     // return RC to caller

} /* endif */

//+----------------------------------------------------------------------------+
//| LupEdit             Edit a term                                            |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HLUPCB   hLUPCB        IN       lookup control block handle             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD    if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hLUPCB must have been created using LupBegin.                           |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT LupEdit
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PCHAR_W  pucTerm                    // term being edited
)
{
   PLUPCB    pLUPCB = NULL;            // pointer to lookup control block
   USHORT    usRC = LUP_OK;            // function return code

   pLUPCB = (PLUPCB) hLUPCB;           // convert handle to pointer

   if ( !pLUPCB || (pLUPCB->lSignature != LUPCB_SIGNATURE) )
   {
     usRC = LUP_BAD_LUPCB_ERR;
   }
   else
   {
     usRC = LupEdit2( hLUPCB, pucTerm, pLUPCB->hwndParent );
   } /* endif */
  return( usRC );
}

USHORT LupEdit2
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PCHAR_W  pucTermW,                  // term being edited
   HWND     hwnd                       // parent handle for dialog box
)
{
   PLUPCB    pLUPCB = NULL;            // pointer to lookup control block
   USHORT    usRC = LUP_OK;            // function return code
   HDCB      hdcbEdit = NULL;
   ULONG     ulTermNum, ulDataLen;     // fields required for AsdFndMatch

   //CHAR_W    chTerm[256];
   //PSZ_W     pucTerm = &chTerm[0];

   //Unicode2ASCII( pucTermW, pucTerm );


   pLUPCB = (PLUPCB) hLUPCB;           // convert handle to pointer


   if ( !pLUPCB || (pLUPCB->lSignature != LUPCB_SIGNATURE) )
   {
      usRC = LUP_BAD_LUPCB_ERR;
   } /* endif */

  // do lookup
  if ( usRC == LUP_OK )
  {
	 HMODULE hResMod;
	 hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
     if ( pucTermW && *pucTermW != EOS )
     {
        if ( AsdFndMatch( pucTermW,
                          pLUPCB->hDCB,
                          pLUPCB->hUCB,
                          pLUPCB->ucTerm,
                          &ulTermNum,
                          &ulDataLen,
                          &hdcbEdit ) != LX_RC_OK_ASD )
        {
          hdcbEdit = NULL;
        } /* endif */
        //strcpy( pLUPCB->ucTerm, pucTerm );
     }
     else
     {
        pLUPCB->ucTerm[0] = EOS;
     } /* endif */

     SearchAndEdit( hwnd, pLUPCB, pucTermW, hResMod,
                    hdcbEdit );
  } /* endif */

   return( usRC );                     // return RC to caller

} /* endif */

BOOL LupRegisterDisplayDlg
(
   PLUPCB    pLUPCB,                   // pointer to lookup control block
   HWND      hwndDlg                   // handle of display dialog
)
{
  BOOL             fOK = TRUE;         // function return value

  if ( pLUPCB->usDisplayDlgs < MAX_DISP_DLGS )
  {
    pLUPCB->ahwndDisplay[pLUPCB->usDisplayDlgs] = hwndDlg;
    pLUPCB->usDisplayDlgs++;
  }
  else
  {
    fOK = FALSE;                       // no more room left in hwnd array
  } /* endif */

  return( fOK );
} /* end of function LupRegisterDisplayDlg */

BOOL LupUnregisterDisplayDlg
(
   PLUPCB    pLUPCB,                   // pointer to lookup control block
   HWND      hwndDlg                   // handle of display dialog
)
{
  BOOL             fOK = TRUE;         // function return value

  USHORT           usIndex;            // loop index

  /********************************************************************/
  /* Search dialog in array of registered dialogs                     */
  /********************************************************************/
  usIndex = 0;
  while ( usIndex < pLUPCB->usDisplayDlgs )
  {
    if ( pLUPCB->ahwndDisplay[usIndex] == hwndDlg )
    {
      break;                           // gotcha!, leave loop
    }
    else
    {
      usIndex++;                       // try next one
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* Remove dialog or set return code                                 */
  /********************************************************************/
  if ( usIndex < pLUPCB->usDisplayDlgs )
  {
    if ( (usIndex < (pLUPCB->usDisplayDlgs - 1)) &&
         (pLUPCB->usDisplayDlgs > 1) )
    {
      /****************************************************************/
      /* Shift all following window handles to avoid gaps in the      */
      /* array                                                        */
      /****************************************************************/
      memmove( &(pLUPCB->ahwndDisplay[usIndex]),
               &(pLUPCB->ahwndDisplay[usIndex+1]),
               (pLUPCB->usDisplayDlgs - usIndex - 1) * sizeof(HWND) );
    } /* endif */
    pLUPCB->usDisplayDlgs--;


    /*************************************************************/
    /* Post Dialog hidden notification if no lookup dialog is    */
    /* active and no more display dialog are active for this     */
    /* lookup session                                            */
    /*************************************************************/
    if ( (pLUPCB->usDisplayDlgs == 0) && (pLUPCB->hwndLook == NULLHANDLE) )
    {
      WinPostMsg( pLUPCB->hwndParent,
                  pLUPCB->usNotifyMsg,
                  MP1FROMSHORT( pLUPCB->usID ),
                  MP2FROM2SHORT( DLG_HIDDEN, LOOKUP_DLG ) );
    } /* endif */
  }
  else
  {
    fOK = FALSE;                       // dialog is not in our hwnd array
  } /* endif */

  return( fOK );
} /* end of function LupUnregisterDisplayDlg */


//+----------------------------------------------------------------------------+
//| LupGetLookupHandle  Get window handle for lookup                           |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HLUPCB   hLUPCB        IN       lookup control block handle             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    HWND     hwnd        handle of lookup dialog or null                    |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hLUPCB must have been created using LupBegin.                           |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
HWND LupGetLookupHandle
(
   HLUPCB   hLUPCB                     // Lookup services control block
)
{
  HWND hwndLookUp = NULL;
  PLUPCB    pLUPCB = NULL;            // pointer to lookup control block

  pLUPCB = (PLUPCB) hLUPCB;           // convert handle to pointer

  if ( pLUPCB && (pLUPCB->lSignature == LUPCB_SIGNATURE) )
  {
    hwndLookUp = pLUPCB->hwndLook;
  } /* endif */
  return( hwndLookUp );
}

USHORT LupGetDispHandles
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PHWND *  pphwndDisp
)
{
  USHORT    usNumHandles = 0;         // number of open handles
  PLUPCB    pLUPCB = NULL;            // pointer to lookup control block

  pLUPCB = (PLUPCB) hLUPCB;           // convert handle to pointer

  if ( pLUPCB && (pLUPCB->lSignature == LUPCB_SIGNATURE) )
  {
    *pphwndDisp  = &pLUPCB->ahwndDisplay[0];
    usNumHandles = pLUPCB->usDisplayDlgs;
  } /* endif */
  return( usNumHandles );
}
