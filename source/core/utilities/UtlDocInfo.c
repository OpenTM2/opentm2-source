/*! \file
	Description: Document info functions 
	
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqf.h"
#include "eqffol.h"
#include "eqfsetup.h"

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocQueryInfo(2)                                          |
//+----------------------------------------------------------------------------+
//|Description:       Queries information from document properties, if         |
//|                   document can not supply all requested information the    |
//|                   corresponding function FolQueryInfo is used to get       |
//|                   the missing data.                                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ     pszDocName  object name of document              |
//|                   PSZ     pszMemory   buffer for translation memory or NULL|
//|                   PSZ     pszFormat   buffer for document format or NULL   |
//|                   PSZ     pszSrcLng   buffer for source language or NULL   |
//|                   PSZ     pszTrgLng   buffer for target language or NULL   |
//|                   PSZ     pszLongName buffer for long document name or NULL|
//|                   PSZ     pszAlias    buffer for document alias or NULL    |
//|                   BOOL    fMsg        do-message-handling flag             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR          function completed successfully        |
//|                   other             TM/2 error code                        |
//+----------------------------------------------------------------------------+
USHORT DocQueryInfo
(
PSZ              pszDocName,         // object name of document
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for document format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
BOOL             fMsg                // do-message-handling flag
)
{
  return( DocQueryInfoEx( pszDocName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, NULL, NULL, NULL, NULL, NULL, NULL,
                          fMsg, NULLHANDLE ) );
}

USHORT DocQueryInfo2
(
PSZ              pszDocName,         // object name of document
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for document format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszLongName,        // buffer for document long name or NULL
PSZ              pszAlias,           // buffer for document alias or NULL
PSZ              pszEditor,          // buffer for editor or NULL
BOOL             fMsg                // do-message-handling flag
)
{
  return( DocQueryInfoEx( pszDocName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, pszLongName, pszAlias, pszEditor,
                          NULL, NULL, NULL, fMsg,
                          NULLHANDLE ) );
}

USHORT DocQueryInfoHwnd
(
PSZ              pszDocName,         // object name of document
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for document format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // window handle for error messages
)
{
  return( DocQueryInfoEx( pszDocName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, NULL, NULL, NULL,
                          NULL, NULL, NULL, fMsg, hwnd ) );
}

USHORT DocQueryInfo2Hwnd
(
PSZ              pszDocName,         // object name of document
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for document format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszLongName,        // buffer for document long name or NULL
PSZ              pszAlias,           // buffer for document alias or NULL
PSZ              pszEditor,          // buffer for editor or NULL
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // window handle for error messages
)
{
  return( DocQueryInfoEx( pszDocName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, pszLongName, pszAlias, pszEditor,
                          NULL, NULL, NULL, fMsg, hwnd ) );
}

USHORT DocQueryInfoEx
(
PSZ              pszDocName,         // object name of document
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for document format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszLongName,        // buffer for document long name or NULL
PSZ              pszAlias,           // buffer for document alias or NULL
PSZ              pszEditor,          // buffer for editor or NULL
PSZ              pszConversion,      // buffer for conversion or NULL
PSZ              pszUnUsed1,         // not in use yet, always NULL
PSZ              pszUnUsed2,         // not in use yet, always NULL
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // window handle for error messages
)
{
  PPROPDOCUMENT    pProp = NULL;       // pointer to document properties
  USHORT           usRC = NO_ERROR;    // function return code
  PSZ              pszFileName = NULL; // ptr to filename
  BOOL             fOK;                // success indicator

  pszUnUsed1;
  pszUnUsed2;
  /********************************************************************/
  /* Preset caller's buffers                                          */
  /********************************************************************/
  if ( pszMemory != NULL )      *pszMemory = EOS;
  if ( pszFormat != NULL )      *pszFormat = EOS;
  if ( pszSrcLng != NULL )      *pszSrcLng = EOS;
  if ( pszTrgLng != NULL )      *pszTrgLng = EOS;
  if ( pszLongName != NULL )    *pszLongName = EOS;
  if ( pszAlias != NULL )       *pszAlias = EOS;
  if ( pszEditor != NULL )      *pszEditor = EOS;
  if ( pszConversion != NULL )  *pszConversion = EOS;

  fOK = UtlAlloc( (PVOID *) &pszFileName, 0L, (LONG) MAX_LONGPATH, NOMSG );
  if ( fOK )
  {
    PSZ pszName = UtlGetFnameFromPath( pszDocName );
    if ( pszName )
    {
      ULONG ulBytesRead;
      memcpy( pszFileName, pszDocName, pszName - pszDocName );
      pszFileName[ pszName - pszDocName ] = EOS;
      strcat( pszFileName, PROPDIR );
      strcat( pszFileName, BACKSLASH_STR );
      strcat( pszFileName, pszName );

      fOK = UtlLoadFileL( pszFileName,            // name of file to be loaded
                         (PVOID *)&pProp,        // return pointer to loaded file
                         &ulBytesRead,           // length of loaded file
                         FALSE,
                         FALSE );
    }
    else
    {
      fOK = FALSE;
    } /* endif */
    if ( !fOK )
    {
      /************************************************************/
      /* Error accessing properties of document                   */
      /************************************************************/
      if ( fMsg )
      {
        UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszDocName,
                      EQF_ERROR, hwnd );
      } /* endif */
      usRC = ERROR_PROPERTY_ACCESS;
    } /* endif */
  }
  else
  {
    /************************************************************/
    /* Error accessing properties of document                   */
    /************************************************************/
    if ( fMsg )
    {
      UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
    } /* endif */
    usRC = ERROR_STORAGE;
  } /* endif */

  /********************************************************************/
  /* Supply data available in document properties                     */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    /******************************************************************/
    /* Copy not empty data from document properties to caller's buffer*/
    /* and set the corresponding ptr to NULL so that the data from    */
    /* the document will not be overwritten with the data from the    */
    /* folder                                                         */
    /******************************************************************/
    if ( (pszMemory != NULL) && (pProp->szLongMemory[0] != EOS) )
    {
      strcpy( pszMemory, pProp->szLongMemory );
      pszMemory = NULL;
    }
    else
    if ( (pszMemory != NULL) && (pProp->szMemory[0] != EOS) )
    {
      strcpy( pszMemory, pProp->szMemory );
      pszMemory = NULL;
    } /* endif */

    if ( (pszFormat != NULL) && (pProp->szFormat[0] != EOS) )
    {
      strcpy( pszFormat, pProp->szFormat );
      pszFormat = NULL;
    } /* endif */

    if ( (pszSrcLng != NULL) && (pProp->szSourceLang[0] != EOS) )
    {
      strcpy( pszSrcLng, pProp->szSourceLang );
      pszSrcLng = NULL;
    } /* endif */

    if ( (pszTrgLng != NULL) && (pProp->szTargetLang[0] != EOS) )
    {
      strcpy( pszTrgLng, pProp->szTargetLang );
      pszTrgLng = NULL;
    } /* endif */

    if ( pszLongName != NULL )
    {
      strcpy( pszLongName, pProp->szLongName );
      if ( *pszLongName == EOS )
      {
        // no long name, use short name instead
        strcpy( pszLongName, UtlGetFnameFromPath( pszDocName ) );
      } /* end */         
 
    } /* endif */

    if ( pszAlias != NULL )
    {
      strcpy( pszAlias, pProp->szAlias );
    } /* endif */

    if ( (pszEditor != NULL) && (pProp->szEditor[0] != EOS) )
    {
      strcpy( pszEditor, pProp->szEditor );
      pszEditor = NULL;
    } /* endif */

    if ( (pszConversion != NULL) && (pProp->szConversion[0] != EOS) )
    {
      strcpy( pszConversion, pProp->szConversion );
      pszConversion = NULL;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Complete with data from folder                                   */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    /******************************************************************/
    /* Call FolQueryInfo function if one of the data pointer is      */
    /* not NULL                                                       */
    /******************************************************************/
    if ( (pszMemory != NULL) ||
         (pszFormat != NULL) ||
         (pszSrcLng != NULL) ||
         (pszTrgLng != NULL) ||
         (pszEditor != NULL) ||
         (pszConversion != NULL) )
    {
      OBJNAME szParentObjName;         // object name of parent (sub)folder
      if ( pProp->ulParentFolder == 0 )
      {
        // parent is the main folder
        strcpy( szParentObjName, pProp->PropHead.szPath );
      }
      else
      {
        // parent is a subfolder
        SubFolIdToObjectName( pProp->PropHead.szPath, pProp->ulParentFolder, szParentObjName );
      } /* endif */
      usRC = FolQueryInfoEx( szParentObjName, pszMemory, pszFormat,
                             pszSrcLng, pszTrgLng, pszEditor,
                              pszConversion, NULL, NULL, NULL, NULL, NULL, fMsg, hwnd );
     } /* endif */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  UtlAlloc( (PVOID *) &pszFileName, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function DocQueryInfo */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlQueryCharTableForDocConv                              |
//+----------------------------------------------------------------------------+
//|Description:       Gets the conversion table for the ASCII->ANSI conversion |
//|                   using the document conversion or the active code-page    |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ  the full document file name                         |
//|                   PCHAR * pointer to the conversio table pointer           |
//+----------------------------------------------------------------------------+
USHORT UtlQueryCharTableForDocConv
(
  PSZ         pszDocFullName,          // full file name of segmented document
  PCHAR       *ppConvTable,             // ptr to conversion table pointer
  ULONG       ulOemCP
)
{
  CHAR        szDocObjName[MAX_EQF_PATH]; // buffer for document object name
  CHAR        szConversion[MAX_DESCRIPTION]; // buffer for document conversion
  USHORT      usRC = NO_ERROR;         // function return code
  USHORT      usCodePage = 0;          // buffer for code-page
  // setup document object name
  {
    PSZ  pszDocPos, pszLastDirPos;

    strcpy( szDocObjName, pszDocFullName );
    pszDocPos = strrchr( szDocObjName, BACKSLASH );
    if ( pszDocPos )
    {
      *pszDocPos = EOS;
      pszLastDirPos = strrchr( szDocObjName, BACKSLASH );
      if ( pszLastDirPos )
      {
        pszDocPos++;
        pszLastDirPos++;
        while ( *pszDocPos )
        {
          *pszLastDirPos++ = *pszDocPos++;
        } /* endwhile */
        *pszLastDirPos = EOS;
      } /* endif */
    } /* endif */
  }

#ifdef _TQM
  szConversion[0] = EOS;
#else
  // get document conversion
  usRC =  DocQueryInfoEx( szDocObjName,
                          NULL, NULL, NULL, NULL,
                          NULL, NULL, NULL,
                          szConversion, NULL, NULL,
                          FALSE, NULLHANDLE );
#endif

  // get code-page for document conversion
  if ( (usRC == NO_ERROR) && (szConversion[0] != EOS) )
  {
    usRC = UtlHandleConversionStrings( CONVCHECK_MODE, NULLHANDLE,
                                       szConversion, &usCodePage, NULL );
  } /* endif */

  // get conversion table for code-page or use default conversion table
  if ( (usRC == NO_ERROR) &&
       (szConversion[0] != EOS) &&
       (usCodePage != 0) )
  {
    UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, (PUCHAR * )ppConvTable, usCodePage );
  }
  else
  {
    UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, (PUCHAR *) ppConvTable, (USHORT)ulOemCP );
  } /* endif */

  return( usRC );
} /* end of function UtlQueryCharTableForDocConv */
