/*! \file
	Copyright Notice:
	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved

	This module contains the routines that makes the connection between
	the MAT Tools and the RTF Editor
	
	The following public variables and routines are defined:
	
	EQF_XStart()    - start the translation processor
	EQF_XStop()     - stop the translation processor
	EQF_XStartEx()  - start the translation processor in extended mode
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqfdoc00.h"             // private document handler include file

#include <eqftpi.h>               // internal TPRO file

BOOL  fFirstDocLoad = TRUE;


/*/////////////////////////////////////////////////////////////////////
:h3.EQF_XSTART - start the translation browser
*//////////////////////////////////////////////////////////////////////
// Description:
//  This function will start the browser  and load the passed document
// Flow:
//      - call EQFBFuncStart
//
//  Arguments:
//
//   pEQFProgPath: Program directory of troja
//   pSegSource  : segmented Source filename (full qualified)
//   pSegTarget  : segmented Target filename (full qualified)
//   pstEQFGen   : pointer to generic editor structure
//
//  Output:
//     fOK       : success indicator  TRUE/FALSE
///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ 
EQF_XSTART(  PSZ           pEQFProgPath,   // Program directory of troja
             PSZ           pSegSource  ,   //segmented Source file:
             PSZ           pSegTarget  ,   // segmented Target file:
             PSTEQFGEN     pstEQFGen )     // pointer to generic edit structure
{
   BOOL    fOk;                         //error flag
   BOOL    fReflow = TRUE;              // reflow allowed
   PTBDOCUMENT *ppDoc = (PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0]);
   pEQFProgPath;                        // not used in Translation Browser


   /*******************************************************************/
   /* consistency check...                                            */
   /*******************************************************************/
   if ( fFirstDocLoad ||
        (*ppDoc && (*ppDoc)->ulEyeCatcher != TPRO_EYECATCHER) )
   {
     *ppDoc = NULL;
   } /* endif */

   fOk = EQFBFuncRTFStart( pSegSource, pSegTarget, pstEQFGen,
                        fReflow, *ppDoc);     // do init of editor
  if ( fOk && fFirstDocLoad)
  {
    fFirstDocLoad = FALSE;
  } /* endif */
  return ( (EQF_BOOL)fOk );
}/*end StartMFE*/






/*/////////////////////////////////////////////////////////////////////
:h3.EQF_XSTOP - stop the translation browser
*//////////////////////////////////////////////////////////////////////
// Description:
//  This function will save the document or stop the editor
//
//  Flow:
//     - call EQFBFuncClose
//
//  Arguments:
//
//   pEQFProgPath: Program directory of troja
//   pstEQFGen   : pointer to generic editor structure
//   fKill       : boolean indication
//                 TRUE:  shut down the editor
//                 FALSE: save document only
//
//  Output:
//     fOK       : success indicator  TRUE/FALSE
///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ 
EQF_XSTOP( PSZ       pEQFProgPath,              // Program directory of troja                                 //
           PSTEQFGEN pstEQFGen,                 // generic editor structure                        //
           EQF_BOOL  fKill)                     // shut down requested??
{

   pEQFProgPath;                                // get rid of compiler warning

   if ( !fKill )                                // save ???
   {
      EQFBFuncClose( pstEQFGen );
      fFirstDocLoad = TRUE;
   }
   else                                         // shutdown editor ....
   {
      // do nothing
   } /* endif */
   return ( TRUE );
}




/*/////////////////////////////////////////////////////////////////////
:h3.EQF_XSTARTEX - start the translation browser with more than 1 document
*//////////////////////////////////////////////////////////////////////
// Description:
//  This function will start the browser  and load the passed document
// Flow:
//      - if first invocation or try to activate passed
//            then call EQF_XSTART
//            else try to activate passed on document
//                 if not already loaded, load via EQF_XSTART
//
//
//  Arguments:
//
//   pEQFProgPath: Program directory of troja
//   pDocObjName : ptr array of selected document objects (fully qualified
//   pstEQFGen   : pointer to generic editor structure
//
//  Output:
//     fOK       : success indicator  TRUE/FALSE
///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/
EQF_XSTARTEX( PSZ         pEQFProgPath,   // Program directory of troja
              PSZ         *ppObjNames ,   //segmented Source file:
              PSTEQFGEN   pstEQFGen )     // pointer to generic edit structure
{
   BOOL    fOK = TRUE;                                     //error flag
   PSZ     pszSegSource = NULL;
   PSZ     pszSegTarget = NULL;
   PSZ     pTemp = NULL;
   fOK = UtlAlloc( (PVOID *)&pTemp, 0L, (LONG) (2*MAX_LONGPATH), ERROR_STORAGE );
   if ( fOK )
   {
     pszSegSource = pTemp;
     pszSegTarget = pTemp + MAX_LONGPATH;
   } /* endif */

   if ( fOK && ppObjNames && !EQF_XDOCGETPATHS( ppObjNames[0], pszSegSource, pszSegTarget ))
   {
     /*****************************************************************/
     /* check if we have already a session running...                 */
     /*****************************************************************/
     PTBDOCUMENT *ppDoc = (PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0]);
     if ( fFirstDocLoad ||
          !(*ppDoc && (*ppDoc)->ulEyeCatcher == TPRO_EYECATCHER ))
     {
       fOK = EQF_XSTART(  pEQFProgPath,    // Program directory of troja
                          pszSegSource  ,   //segmented Source file:
                          pszSegTarget  ,   // segmented Target file:
                          pstEQFGen );     // pointer to generic edit structure
       /*****************************************************************/
       /* add all other docs to our TBDOC structure...                  */
       /*****************************************************************/
     }
     else
     {
       /***************************************************************/
       /* add new documents to list of loaded/loadable docs...        */
       /* (will be done implicitly through access to PDOCUMENT_IDA    */
       /* via pstEQFGen)                                              */
       /* activate TMs and dicts for current document                 */
       /* Activate the current document of the pool array             */
       /* if it is not in our ring of documents right now -- load it. */
       /***************************************************************/
       USHORT usRc;
       usRc = EQF_XDOCACT( pstEQFGen,
                           ((PDOCUMENT_IDA)pstEQFGen->pDoc)->apszDocNames[0]);
       if ( !usRc )
       {
         BOOL fFound = FALSE;
         PTBDOCUMENT pDoc;
         PTBDOCUMENT pDocStart = *ppDoc;
         pDoc = pDocStart;
         do
         {
           if (pDoc->docType == STARGET_DOC)
           {
             /*********************************************************/
             /* check if it is the document we are looking for..      */
             /*********************************************************/
             fFound = (strcmp( pDoc->szDocName, pszSegTarget ) == 0);
           } /* endif */
           pDoc = pDoc->next;
         } while ( (pDoc != pDocStart) && !fFound ); /* enddo */

         if ( !fFound )
         {
           /***********************************************************/
           /* load document                                           */
           /***********************************************************/
           fOK = EQF_XSTART(  pEQFProgPath,    // Program directory of troja
                              pszSegSource  ,   //segmented Source file:
                              pszSegTarget  ,   // segmented Target file:
                              pstEQFGen );     // pointer to generic edit structure
         } /* endif */
         /***********************************************************/
         /* activate already loaded document                        */
         /***********************************************************/
         if ( fOK )
         {
           BOOL fFound = FALSE;
           PTBDOCUMENT pDoc;
           PTBDOCUMENT pDocStart = *ppDoc;
           pDoc = pDocStart;
           do
           {
             if (pDoc->docType == STARGET_DOC)
             {
               /*********************************************************/
               /* check if it is the document we are looking for..      */
               /*********************************************************/
               fFound = (strcmp( pDoc->szDocName, pszSegTarget ) == 0);
               if ( !fFound )
               {
                 pDoc = pDoc->next;
               } /* endif */
             }
             else
             {
               pDoc = pDoc->next;
             } /* endif */
           } while ( (pDoc != pDocStart) && !fFound  ); /* enddo */
           *ppDoc = pDoc;       // anchor currently active document
           #ifndef _WINDOWS
             WinSetActiveWindow( HWND_DESKTOP, pDoc->hwndFrame );
           #else
             SendMessage( pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                          0, MP2FROMHWND( pDoc->hwndFrame ));
           #endif
         } /* endif */
       }
       else
       {
         fOK = FALSE;
       } /* endif */
     } /* endif */
   }
   else
   {
     /*****************************************************************/
     /* display error message                                         */
     /*****************************************************************/
     fOK = FALSE;
   } /* endif */
   /*******************************************************************/
   /* free allocated resource                                         */
   /*******************************************************************/
   UtlAlloc( (PVOID *)&pTemp, 0L, 0L, NOMSG );

  return ( (EQF_BOOL)fOK );
}/*end EQF_XSTARTEX*/

