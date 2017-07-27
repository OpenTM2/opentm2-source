//+----------------------------------------------------------------------------+
//| EQFCHKPII.CPP                                                              |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: David Walters                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Interface to CHKPII                                           |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#include <eqf.h>                  // General Translation Manager include file

#include <stdio.h>                      /* Standard C I/O functions     */
#include <stdlib.h>                     /* Commonly used C functions    */
#include <direct.h>
#include <string.h>                     /* C/2 string functions         */
#include <sys\stat.h>
#include <wingdi.h>



 typedef int (* __cdecl OTMCHKPII_DCL)( PSZ, HWND, PSZ, PSZ, BOOL ) ;
 OTMCHKPII_DCL  OTMCHKPII_addr ;

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolChkpii                                                |
//+----------------------------------------------------------------------------+
BOOL FolChkpiiActive( void )      
{
char        szChkpiiDLL[256] ;
BOOL        bFound = FALSE ;

   UtlMakeEQFPath( szChkpiiDLL, NULC, PLUGIN_PATH, NULL ) ;
   if ( szChkpiiDLL[0] ) {
      strcat( szChkpiiDLL, "\\chkpii\\otmchkpii.dll" ) ;
      if ( UtlFileExist( szChkpiiDLL ) ) {
         bFound = TRUE ;
      }
   }

   return( bFound ) ;
}



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolChkpii                                                |
//+----------------------------------------------------------------------------+
BOOL FolChkpii
(
  HWND             hwndCur,
  PSZ              pszFolObjName,      // folder object name
  PSZ              pszDocObjName,      // document object name
  int              iItems              // Number of selected items
)
{
HINSTANCE   hInst_DLL;
char        szChkpiiDLL[256] ;
BOOL        bReturn = FALSE ;

   UtlMakeEQFPath( szChkpiiDLL, NULC, PLUGIN_PATH, NULL ) ;
   if ( szChkpiiDLL[0] ) {
      strcat( szChkpiiDLL, "\\chkpii\\otmchkpii.dll" ) ;

      hInst_DLL = LoadLibraryA( szChkpiiDLL );
      if ( hInst_DLL != NULL ) {

         OTMCHKPII_addr = (OTMCHKPII_DCL)GetProcAddress((HMODULE)hInst_DLL, "OtmChkpii");
         if ( OTMCHKPII_addr ) {
            int rc = (*OTMCHKPII_addr)( szChkpiiDLL, hwndCur, pszFolObjName, pszDocObjName, iItems ) ;
            if ( rc == 0 ) {
               bReturn = TRUE ;
            }
         }

         FreeLibrary( hInst_DLL ) ;
      }
   }

   return( bReturn ) ;
}
