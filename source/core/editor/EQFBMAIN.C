/*! \file
	Copyright Notice:
	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved

	This module contains the routines that makes the connection between
	the MAT Tools and the Translation Processor
	
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
#define INIT_TABLES               // forces EQFBCONF.H to initialize data
#include <eqftpi.h>               // internal TPRO file

USHORT usTables = 0;                   // number of tables in pLoadedTables


BOOL  fFirstDocLoad = TRUE;

// the following table contains a list of tagtables which should
// set the reflow flag to false

//static CHAR aszTagTables[][13] =
//   {
//     "EQFBMRI",                             // MRI tagtable
//     ""
//   };
static CHAR aszTagTables[][13] =
   {
     "EQFBMRI",                             // MRI tagtable
     "EQFAMRI",								// P017614, RJ added 03/06/20
     "EQFMRI",								// P017614, RJ added 03/06/20
     ""
   };

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

   fOk = EQFBFuncStart( pSegSource, pSegTarget, pstEQFGen,
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
            SendMessage( pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                          0, MP2FROMHWND( pDoc->hwndFrame ));
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

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBFreeTables
//-----------------------------------------------------------------------------
// Function call:     EQFBFreeTables()
//-----------------------------------------------------------------------------
// Description:       free space for previously allocated tag tables
//-----------------------------------------------------------------------------
// Parameters:        VOID
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     for all loaded tables
//                      - free the space for the table file
//                      - free the space for the tag tree
//                      - free the space for the attribute tree
//-----------------------------------------------------------------------------

CHARFORMAT2* get_aszFontExtSpecs()
{
	return(aszFontExtSpecs);
}

VIOFONTCELLSIZE* get_vioFontSize()
{
	return(vioFontSize);
}

USHORT* get_usRightMargin()
{
	return(&usRightMargin);
}

OPENDATA* get_OpenData()
{
	return(&OpenData);
}

TEXTTYPETABLE* get_DefTextTypeTable()
{
	return(DefTextTypeTable);
}

TEXTTYPETABLE* get_TextTypeTable()
{
	return(TextTypeTable);
}

KEYPROFTABLE* get_DefKeyTable()
{
	return(DefKeyTable);
}

KEYPROFTABLE* get_KeyTable()
{
	return(KeyTable);
}
CHAR* get_aszFontFacesGlobal()
{
	return(&aszFontFacesGlobal[0][0]);
}
USEROPT* get_EQFBUserOpt()
{
	return(&EQFBUserOpt);
}
PPROPSYSTEM get_pSysProp()
{
	return(pSysProp);
}
void set_pSysProp(PPROPSYSTEM pNewSysProp)
{
	pSysProp = pNewSysProp;
}

RESKEYTABLE* get_ResKeyTab()
{
	return(&ResKeyTab[0]);
}
EQFBBLOCK* get_EQFBBlockMark()
{
	return(&EQFBBlockMark);
}
FILE* get_hMTLog()
{
	return(hMTLog);
}
void set_hMTLog( FILE *hMTLogNew )
{
  hMTLog = hMTLogNew;
	return;
}

FUNCTIONTABLE* get_FuncTab()
{
	return(&FuncTab[0]);
}

