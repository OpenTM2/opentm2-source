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
PULONG           pulDocTime,         // buffer for document last update time 
PSZ              pszUnUsed2,         // not in use yet, always NULL
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // window handle for error messages
)
{
  PPROPDOCUMENT    pProp = NULL;       // pointer to document properties
  USHORT           usRC = NO_ERROR;    // function return code
  PSZ              pszFileName = NULL; // ptr to filename
  BOOL             fOK;                // success indicator

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
  if ( pulDocTime != NULL )      *pulDocTime = 0;

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

    if ( pulDocTime != NULL )
    {
      *pulDocTime = pProp->ulTouched;
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

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolQueryInfo                                             |
//+----------------------------------------------------------------------------+
//|Description:       Queries information from folder properties.              |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ     pszFolName  object name of folder                |
//|                   PSZ     pszMemory   buffer for translation memory or NULL|
//|                   PSZ     pszFormat   buffer for document format or NULL   |
//|                   PSZ     pszSrcLng   buffer for source language or NULL   |
//|                   PSZ     pszTrgLng   buffer for target language or NULL   |
//|                   BOOL    fMsg        do-message-handling flag             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR          function completed successfully        |
//|                   other             TM/2 error code                        |
//+----------------------------------------------------------------------------+
USHORT FolQueryInfo
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
BOOL             fMsg                // do-message-handling flag
)
{
  return( FolQueryInfo2Hwnd( pszFolName, pszMemory, pszFormat, pszSrcLng,
                             pszTrgLng, NULL, fMsg, NULLHANDLE ) );
}

USHORT FolQueryInfo2
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszEditor,          // buffer for editor or NULL
BOOL             fMsg                // do-message-handling flag
)
{
  return( FolQueryInfoEx( pszFolName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, pszEditor,
                          NULL, NULL, NULL, NULL, NULL, NULL,
                          fMsg, NULLHANDLE ) );
}

USHORT FolQueryInfoHwnd
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // owner handle for error messages
)
{
  return( FolQueryInfoEx( pszFolName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, NULL, NULL, NULL, NULL, NULL, NULL, NULL, fMsg, hwnd ) );
}


USHORT FolQueryInfo2Hwnd
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszEditor,          // buffer for editor or NULL
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // owner handle for error messages
)
{
  return( FolQueryInfoEx( pszFolName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, pszEditor,
                          NULL, NULL, NULL, NULL, NULL, NULL,
                          fMsg, hwnd ) );
}

USHORT FolQueryInfoEx
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszEditor,          // buffer for editor or NULL
PSZ              pszConversion,      // buffer for conversion or NULL
PSZ              pszVendor,          // buffer for translator name or NULL
PSZ              pszVendorEMail,     // buffer for eMail address or NULL
PBOOL            pfDisabled,         // folder disabled flag
PVOID            pvUnused1,
PVOID            pvUnused2,
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // owner handle for error messages
)
{
  PPROPFOLDER      pProp = NULL;       // pointer to folder properties
  USHORT           usRC = NO_ERROR;    // function return code
  CHAR             szSysDrive[MAX_DRIVE]; // buffer for system drive
  PSZ              pszPropName = NULL; // ptr to property file name
  PSZ              pszObjName = NULL;  // ptr to object name
  BOOL             fOK = TRUE;         // success indicator
  BOOL             fSubFolder = FALSE; // TRUE = object is a subfolder
  BOOL             fDone = FALSE;      // completed flag

  pvUnused1; pvUnused2;

  /********************************************************************/
  /* Preset caller's buffers                                          */
  /********************************************************************/
  if ( pszMemory != NULL )      *pszMemory = EOS;
  if ( pszFormat != NULL )      *pszFormat = EOS;
  if ( pszSrcLng != NULL )      *pszSrcLng = EOS;
  if ( pszTrgLng != NULL )      *pszTrgLng = EOS;
  if ( pszEditor != NULL )      *pszEditor = EOS;
  if ( pszConversion != NULL )  *pszConversion = EOS;
  if ( pszVendor != NULL )      *pszVendor = EOS;
  if ( pszVendorEMail != NULL ) *pszVendorEMail = EOS;
  if ( pfDisabled!= NULL )      *pfDisabled = FALSE;

  // allocate buffer for property file names
  fOK = UtlAlloc( (PVOID *) &pszObjName, 0L, (LONG)(2 * MAX_LONGPATH), NOMSG );
  if ( !fOK )
  {
    if ( fMsg )
    {
      UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
    } /* endif */
    usRC = ERROR_STORAGE;
  } /* endif */

  // loop over parent subfolder/folder chain until all required data can be supplied
  if ( fOK )
  {
    // start with supplied folder/subfolder object name
    pszPropName = pszObjName + MAX_LONGPATH;
    strcpy( pszObjName, pszFolName );

    do
    {
      fSubFolder = FolIsSubFolderObject( pszObjName );

      if ( fSubFolder )
      {
        // object name is already the fully qualified property file name
        strcpy( pszPropName, pszObjName );
      }
      else
      {
        // setup folder property file name
        PSZ pszName = strrchr( pszObjName, BACKSLASH );
        if ( pszName )
        {
          *pszName = EOS;
          UtlQueryString( QST_PRIMARYDRIVE, szSysDrive, sizeof(szSysDrive) );
          sprintf( pszPropName, "%c%s\\%s\\%s", szSysDrive[0], pszObjName + 1, PROPDIR, pszName + 1 );
          *pszName = BACKSLASH;
        }
        else
        {
          fOK = FALSE;
        } /* endif */
      } /* endif */

      // load property file
      if ( fOK )
      {
        ULONG ulLen;
        fOK = UtlLoadFileL( pszPropName, (PVOID *)&pProp, &ulLen, FALSE, FALSE );
      } /* endif */

      // do any error handling
      if ( !fOK )
      {
        // Error accessing properties
        if ( fMsg )
        {
          UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszFolName,
                        EQF_ERROR, hwnd );
        } /* endif */
        usRC = ERROR_PROPERTY_ACCESS;
      } /* endif */

      // complete caller data with data from properties
      if ( fOK )
      {
        fDone = TRUE;                // assume all info can be provided ...

        if ( pszMemory != NULL )
        {
          if ( pProp->szLongMemory[0] != EOS )
            strcpy( pszMemory, pProp->szLongMemory );
          else if ( pProp->szMemory[0] != EOS )
            strcpy( pszMemory, pProp->szMemory );

          if ( *pszMemory != EOS )
            pszMemory = NULL;       // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszFormat != NULL )
        {
          strcpy( pszFormat, pProp->szFormat );
          if ( *pszFormat != EOS )
            pszFormat = NULL;       // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszSrcLng != NULL )
        {
          strcpy( pszSrcLng, pProp->szSourceLang );
          if ( *pszSrcLng != EOS )
            pszSrcLng = NULL;      // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszTrgLng != NULL )
        {
          strcpy( pszTrgLng, pProp->szTargetLang);
          if ( *pszTrgLng != EOS )
            pszTrgLng = NULL;      // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszEditor != NULL )
        {
          strcpy( pszEditor, pProp->szEditor );
          if ( *pszEditor != EOS )
            pszEditor = NULL;      // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszConversion != NULL )
        {
          strcpy( pszConversion, pProp->szConversion );
          if ( *pszConversion != EOS )
            pszConversion = NULL;  // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszVendor != NULL )
        {
          strcpy( pszVendor, pProp->szVendor );
          if ( *pszVendor != EOS )
            pszVendor = NULL;      // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszVendorEMail != NULL )
        {
          strcpy( pszVendorEMail, pProp->szVendorEMail );
          if ( *pszVendorEMail != EOS )
            pszVendorEMail = NULL;       // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */
      } /* endif */


      if ( (pProp != NULL) && (pfDisabled != NULL) )
      {
        *pfDisabled = pProp->fDisabled_UserExitRefresh;
          fDone = FALSE;          // stay in loop until folder is reached
      } /* endif */



      // prepare data lookup from parent if necessary
      if ( fOK && !fDone )
      {
        if ( !fSubFolder )
        {
          fDone = TRUE;                // no parent for folders available
        }
        else
        {
          if ( pProp->ulParentFolder == 0 )
          {
            // next parent is the main folder, cut off subfolder name and property
            // directory to create the main folder object name
            PSZ pszName = strrchr( pszObjName, BACKSLASH );
            if ( pszName )
            {
              *pszName = EOS;
              pszName = strrchr( pszObjName, BACKSLASH );
            } /* endif */
            if ( pszName )
            {
              *pszName = EOS;
            }
            else
            {
              // subfolder object name is damaged
              fOK = FALSE;
            } /* endif */
          }
          else
          {
            // next parent is a subfolder, replace name part of object name
            // with name of parent folder
            PSZ pszName = strrchr( pszObjName, BACKSLASH );
            if ( pszName )
            {
              pszName++;               // position to name part
              sprintf( pszName, "%8.8ld%s", pProp->ulParentFolder, EXT_OF_SUBFOLDER );
            }
            else
            {
              // subfolder object name is damaged
              fOK = FALSE;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */

      if ( pProp )
      {
        UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        pProp = NULL;
      } /* endif */
    } while ( fOK && !fDone );
  } /* endif */

  // Cleanup
  if ( pszObjName )  UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function FolQueryInfoEx */

// convert a main folder object name and a subfolder ID to a subfolder object name
BOOL SubFolIdToObjectName( PSZ pszMainFolderObjName, ULONG ulID, PSZ pszObjName )
{
  BOOL        fOK = TRUE;              // function return code

  sprintf( pszObjName, "%s\\%s\\%8.8ld%s", pszMainFolderObjName, PROPDIR, ulID, EXT_OF_SUBFOLDER );

  return( fOK );
} /* end of function SubFolIdToObjectName */


// check if the given object name is the object name of a subfolder
BOOL FolIsSubFolderObject( PSZ pszObjName )
{
  // subfolders object names have an extention of .F01 thereas folders have an
  // exten of .F00, subfolders have the property directory between the folder
  // directory and the subfolder name
  BOOL fSubFolder = FALSE;
  PSZ pszSubFolExt = EXT_OF_SUBFOLDER;
  PSZ pszPropDir = PROPDIR;
  int iLen= strlen(pszSubFolExt );

  if ( strcmp( pszObjName + strlen(pszObjName) - iLen, pszSubFolExt ) == 0 )
  {
    // extention is O.K. now check the property directory name
    PSZ pszPropDirPos = strchr( pszObjName, BACKSLASH );
    if ( pszPropDirPos ) pszPropDirPos = strchr( pszPropDirPos+1, BACKSLASH );
    if ( pszPropDirPos ) pszPropDirPos = strchr( pszPropDirPos+1, BACKSLASH );
    if ( pszPropDirPos )
    {
      iLen = strlen(pszPropDir);
      if ( (strncmp( pszPropDirPos+1, pszPropDir, iLen ) == 0) &&
           (pszPropDirPos[iLen+1] == BACKSLASH) )
      {
        fSubFolder = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */
  return( fSubFolder );
} /* end of function FolIsSubFolderObject */

// Private data structure used by function SubFolNameToObjectName
typedef struct _SUBFOLNAMETOOBJECTNAMEDATA
{
  PROPFOLDER  Prop;                    // buffer for folder properties
  CHAR        szObjName[MAX_EQF_PATH]; // buffer for object names
  CHAR        szPropName[MAX_EQF_PATH];// buffer for property file names
  CHAR        szShortName[MAX_FILESPEC]; // buffer for folder short name
  CHAR        szFolPropPath[MAX_EQF_PATH]; // buffer for folder property directory path
  CHAR        szSearchPath[MAX_EQF_PATH]; // buffer for subfolder search path
  FILEFINDBUF stFileFindBuf;           // file find buffer
} SUBFOLNAMETOOBJECTNAMEDATA, *PSUBFOLNAMETOOBJECTNAMEDATA;

// convert a folder/subFolder name to a subfolder object name
BOOL SubFolNameToObjectName( PSZ pszName, PSZ pszObjName )
{
  BOOL        fOK = TRUE;              // function return code
  PSZ         pszEndOfName = NULL;     // points to end of currently processed name
  PSUBFOLNAMETOOBJECTNAMEDATA pData = NULL; // ptr to private data area
  ULONG       ulCurParentID = 0;       // ID of current parent
  CHAR        chDrive = EOS;

  // allocate private data area
  fOK = UtlAlloc( (PVOID *)&pData, 0L, sizeof(SUBFOLNAMETOOBJECTNAMEDATA), NOMSG );

  // start with folder name ...
  if ( fOK )
  {
    BOOL fIsNew;

    // isolate folder name
    pszEndOfName = strchr( pszName, BACKSLASH );
    if ( pszEndOfName ) *pszEndOfName =EOS;

    // get folder short name
    ObjLongToShortName( pszName, pData->szShortName, FOLDER_OBJECT, &fIsNew );
    if ( fIsNew )
    {
      fOK = FALSE;                     // folder does not exist
    } /* endif */

    // build folder property file name
    if ( fOK )
    {
      strcat( pData->szShortName, EXT_FOLDER_MAIN );
      UtlMakeEQFPath( pData->szObjName, SYSTEM_PATH, NULC, NULL );
      strcat( pData->szObjName, BACKSLASH_STR );
      strcat( pData->szObjName, pData->szShortName );
    } /* endif */

    // load folder properties to get correct drive letter
    if ( fOK )
    {
//      PVOID pvTemp = &pData->Prop;
//      ULONG ulLen = sizeof(pData->Prop);
//      fOK = UtlLoadFileL( pData->szObjName, &pvTemp, &ulLen, FALSE, TRUE );
        HPROP   hFolProp;
        EQFINFO ErrorInfo;
        hFolProp = OpenProperties( pData->szObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
        if (hFolProp)
        {
			PPROPFOLDER pProp;
			pProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
			chDrive = pProp->chDrive;
			CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo);
	    }
	    else
	    {
			fOK = FALSE;
	    }

    } /* endif */

    // build folder object and path name of folders property directory
    if ( fOK )
    {
      UtlMakeEQFPath( pData->szObjName, chDrive /*pData->Prop.chDrive*/, SYSTEM_PATH, pData->szShortName );
      UtlMakeEQFPath( pData->szPropName, chDrive /*pData->Prop.chDrive*/, PROPERTY_PATH, pData->szShortName );
      strcpy( pData->szFolPropPath, pData->szPropName );
    } /* endif */
  } /* endif */

  // look for current subfolder
  if ( fOK )
  {
    while ( fOK && pszEndOfName )      // while OK and miore subfolders to follow ...
    {
      // isolate next subfolder name
      PSZ pszSubFolder = pszEndOfName + 1;
      *pszEndOfName = BACKSLASH;
      pszEndOfName = strchr( pszSubFolder, BACKSLASH );
      if ( pszEndOfName ) *pszEndOfName = EOS;

      // search all subfolders for given name
      {
        HDIR hdir = HDIR_CREATE;
        USHORT usCount = 1;
        USHORT usRC;
        BOOL   fFound = FALSE;

        // setup subfolder search path
        sprintf( pData->szSearchPath, "%s\\*%s", pData->szFolPropPath, EXT_OF_SUBFOLDER );

        // loop over all subfolders
        usRC = UtlFindFirst( pData->szSearchPath, &hdir, FILE_NORMAL, &(pData->stFileFindBuf),
                             sizeof(pData->stFileFindBuf), &usCount, 0L, FALSE );
        while ( !fFound && fOK && (usRC == NO_ERROR) && usCount )
        {
          // load subfolders property file and check parent ID and name
          {
            PVOID pvTemp = &pData->Prop;
            ULONG ulLen = sizeof(pData->Prop);

            sprintf( pData->szObjName, "%s\\%s", pData->szFolPropPath, pData->stFileFindBuf.cFileName );

            fOK = UtlLoadFileL( pData->szObjName, &pvTemp, &ulLen, FALSE, TRUE );
            if ( fOK )
            {
              if ( (pData->Prop.ulParentFolder == ulCurParentID) &&
                   (strcmp( pData->Prop.szLongName, pszSubFolder ) == 0) )
              {
                fFound = TRUE;
                ulCurParentID = pData->Prop.ulSubFolderID;
              } /* endif */
            } /* endif */
          }

          // try next subfolder
          if ( !fFound )
          {
            usRC = UtlFindNext( hdir, &(pData->stFileFindBuf), sizeof(pData->stFileFindBuf),
                                &usCount, 0);
          } /* endif */
        } /* endwhile */

        // close search file handle
        if ( hdir != HDIR_CREATE ) UtlFindClose( hdir, FALSE );

        // end loop if subfolder was not found
        if ( !fFound ) fOK = FALSE;
      }
    } /* endwhile */
  } /* endif */

  // cleanup
  if ( fOK ) strcpy( pszObjName, pData->szObjName );
  if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function SubFolNameToObjectName */


// convert a folder or subfolder object name to a folder/subfolder name
BOOL SubFolObjectNameToName( PSZ pszObjName, PSZ pszName )
{
  BOOL        fOK = TRUE;              // function return code
  PSZ         pszExtention;            // points to extention of object name
  PPROPFOLDER pProp = NULL;            // loaded property file
  PSZ         pszObjNameBuf = NULL;    // buffer for object name

  // handle folder and subfolder object name differently:
  // the subfolder object name is the fully qualified property file path
  // the folder object name is the path name of the property file w/o the
  // property directory
  pszExtention = strchr( pszObjName, DOT );
  if ( pszExtention == NULL )
  {
    // passed name is no valid object name
    fOK = FALSE;
  }
  else
  {
    // allocate buffer for object names of parent (sub)folders
    fOK = UtlAlloc( (PVOID *)&pszObjNameBuf, 0L, MAX_EQF_PATH, NOMSG );

    // handle folder or subfolder object name
    if ( fOK )
    {
      if ( strcmp( pszExtention, EXT_FOLDER_MAIN ) == 0 )
      {
        ULONG ulLen = 0;

        // process folder

        // insert property directory into object name
        {
          PSZ pszName = strrchr( pszObjName, BACKSLASH );
          if ( pszName )
          {
            *pszName = EOS;
            strcpy( pszObjNameBuf, pszObjName );
            strcat( pszObjNameBuf, BACKSLASH_STR );
            strcat( pszObjNameBuf, PROPDIR );
            *pszName = BACKSLASH;
            strcat( pszObjNameBuf, pszName );

            // replace drive letter with system drive letter
            {
              CHAR szPrimaryDrive[20];
              UtlQueryString( QST_PRIMARYDRIVE, szPrimaryDrive, sizeof(szPrimaryDrive) );
              *pszObjNameBuf = szPrimaryDrive[0];
            }
          }
          else
          {
            // supplied object name is invalid
            fOK = FALSE;
          } /* endif */
        }

        // load folder property file
        if ( fOK )
        {
          fOK = UtlLoadFileL( pszObjNameBuf, (PVOID *)&pProp, &ulLen, FALSE, FALSE );
        } /* endif */

        // extract folder name
        if ( fOK )
        {
          if ( pProp->szLongName[0] )
          {
            strcpy( pszName, pProp->szLongName );
          }
          else
          {
            Utlstrccpy( pszName, pProp->PropHead.szName, DOT );
          } /* endif */
        } /* endif */

      }
      else
      {
        // process subfolder

        // load subfolder property file and get parent ID
        if ( fOK )
        {
          ULONG ulLen;
          fOK = UtlLoadFileL( pszObjName, (PVOID *)&pProp, &ulLen, FALSE, FALSE );
        } /* endif */

        // recursively call function again to get the name of the parent folder
        if ( fOK )
        {
          // build main folder object name
          {
            PSZ pszEndOfFolder;

            strcpy( pszObjNameBuf, pszObjName );
            pszEndOfFolder = strrchr( pszObjNameBuf, BACKSLASH );
            if ( pszEndOfFolder )
            {
              *pszEndOfFolder = EOS;
              pszEndOfFolder = strrchr( pszObjNameBuf, BACKSLASH );
            } /* endif */
            if ( pszEndOfFolder )
            {
              *pszEndOfFolder = EOS;
            }
            else
            {
              fOK = FALSE;           // invalid subfolder object name
            } /* endif */
          }


          if ( pProp->ulParentFolder )
          {
            // there are parent subfolders, call function to get parent subfolder name

            // build name of parent subfolder (main folder name already in buffer)
            sprintf( pszObjNameBuf + strlen(pszObjNameBuf), "\\%s\\%8.8ld%s",
                     PROPDIR, pProp->ulParentFolder, EXT_OF_SUBFOLDER );

            // recursively call for parent subfolder name
            if ( fOK )
            {
              fOK =  SubFolObjectNameToName( pszObjNameBuf, pszName );
            } /* endif */
          }
          else
          {
            // no more parent folder, call function to get main folder name

            // recursively call for parent folder name
            if ( fOK )
            {
              fOK =  SubFolObjectNameToName( pszObjNameBuf, pszName );
            } /* endif */
          } /* endif */
        } /* endif */

        // concatenate name of this subfolder to name already in buffer
        if ( fOK )
        {
          strcat( pszName, BACKSLASH_STR );
          strcat( pszName, pProp->szLongName );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // cleanup
  if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
  if ( pszObjNameBuf ) UtlAlloc( (PVOID *)&pszObjNameBuf, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function SubFolObjectNameToName*/

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolLongToShortDocName                                    |
//+----------------------------------------------------------------------------+
//|Description:       Converts a long document name to a short one and         |
//|                   ensures that the short names are unique within the       |
//|                   folder.                                                  |
//|                   Do not use UtlMakeEQFPath and OpenProperties because     |
//|                   this function is called from our API, too.               |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ     pszFolObjName  object name of folder             |
//|                   PSZ     pszLongName    ptr to long file name of document |
//|                   PSZ     pszShortName   buffer for document short name    |
//|                   PBOOL   pfIsNew        TRUE if document is a new one     |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE              function completed successfully        |
//|                   other             TM/2 error code                        |
//+----------------------------------------------------------------------------+
USHORT FolLongToShortDocName
(
PSZ              pszFolObjName,      // object name of folder
PSZ              pszLongName,        // long document name
PSZ              pszShortName,       // buffer for short document name
PBOOL            pfIsNew             // TRUE if document is a new one
)
{
  // our private data area
  typedef struct _L2SDATA
  {
    CHAR      szShortName[MAX_FILESPEC];    // buffer for short document name
    CHAR      szFolder[MAX_FILESPEC];       // buffer for folder name
    CHAR      szSearchPath[MAX_EQF_PATH];   // document search path
    CHAR      szDocFullPath[MAX_EQF_PATH];  // fully qualified document name
    FILEFINDBUF stResultBuf;                // DOS file find structure
  } L2SDATA, *PL2SDATA;

  // local variables
  PSZ         pszFileName = NULL;      // ptr to private filename area
  PL2SDATA    pData = NULL;            // ptr to private data area
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fIsLongName = UtlIsLongFileName( pszLongName );

  // preset callers's variables
  *pfIsNew = TRUE;
  pszShortName[0] = EOS;

  // allocate our private data area
  if ( !UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(L2SDATA), ERROR_STORAGE ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // first try: for short names and mixed-cased names check if document
  // properties exist for the given name
  if ( usRC == NO_ERROR )
  {
    if ( (fIsLongName == 2) ||         // a normal but mixed-case file name
         (fIsLongName == FALSE) )      // or a uppercase short name???
    {
      // Check if we have a document with the given name

      strcpy( pszShortName, pszLongName );

      strcpy( pData->szFolder, UtlGetFnameFromPath( pszFolObjName ) );
      /******************************************************************/
      /* Build path ..                                                  */
      /******************************************************************/
      sprintf( pData->szSearchPath, "%c:\\%s\\%s\\%s", pszFolObjName[0], PATH,
               pData->szFolder, SOURCEDIR );

      strcat( pData->szSearchPath, BACKSLASH_STR );
      strcat( pData->szSearchPath, pszLongName );
      strupr( pData->szSearchPath );
      if ( UtlFileExist( pData->szSearchPath ) ) // is there a document with
      // this name???
      {
        // use the name of the existing document as short name
        strcpy( pszShortName, pszLongName );
        strupr( pszShortName );
        *pfIsNew = FALSE;

        // cleanup
        if ( pData != NULL ) UtlAlloc( (PVOID *)&pData, 0L, 0L,NOMSG );

        // return
        return( usRC );
      } /* endif */
    } /* endif */
  } /* endif */

  // get document short name and setup search path
  if ( usRC == NO_ERROR )
  {
    UtlLongToShortName( pszLongName, pData->szShortName );
    strcpy( pData->szFolder, UtlGetFnameFromPath( pszFolObjName ) );
    /******************************************************************/
    /* Build path ..                                                  */
    /******************************************************************/
    sprintf( pData->szSearchPath, "%c:\\%s\\%s\\%s", pszFolObjName[0], PATH,
             pData->szFolder, SOURCEDIR );

    strcat( pData->szSearchPath, BACKSLASH_STR );
    strcat( pData->szSearchPath, pData->szShortName );
    strcat( pData->szSearchPath, DEFAULT_PATTERN_EXT );
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    UtlAlloc( (PVOID *) &pszFileName, 0L, (LONG) MAX_LONGPATH, NOMSG );
    if ( !pszFileName )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // look for documents having the same short name
  if ( usRC == NO_ERROR )
  {
    USHORT usDosRC = NO_ERROR;         // return code of called DOS functions
    USHORT usCount = 1;                // number of files requested
    HDIR   hDir = HDIR_CREATE;         // file find handle
    BOOL   fOK;
    CHAR   szShortNameCaseMismatch[MAX_FILESPEC]; // buffer for short name which does not match case
    BOOL   fCaseMismatchFound = FALSE; // TRUE = found matching file with different casing

    usDosRC = UtlFindFirst( pData->szSearchPath, &hDir, FILE_NORMAL,
                            &(pData->stResultBuf), sizeof(pData->stResultBuf),
                            &usCount, 0L, FALSE );

    while ( *pfIsNew && (usDosRC == NO_ERROR) && usCount )
    {
      PPROPDOCUMENT pProp = NULL;      // ptr to document properties
      ULONG ulBytesRead;

      sprintf( pszFileName, "%c:\\%s\\%s\\%s\\%s", pszFolObjName[0], PATH,
               pData->szFolder, PROPDIR, RESBUFNAME( pData->stResultBuf ) );

      fOK = UtlLoadFileL( pszFileName,            // name of file to be loaded
                         (PVOID *)&pProp,        // return pointer to loaded file
                         &ulBytesRead,           // length of loaded file
                         FALSE,
                         FALSE );
      if ( !fOK )
      {
        usDosRC = ERROR_PROPERTY_ACCESS;
      } /* endif */

      if ( usDosRC == NO_ERROR )
      {
        BOOL fFound;

        // for real long names use case-sensitive compare else
        // use case-insensitive compare
        if ( fIsLongName == TRUE ) // compare with 1!!! (2 is mixed-case name!)
        {
          // the long name in the properties is stored in ASCII ...
          OEMTOANSI( pProp->szLongName );

          fFound = (strcmp( pszLongName, pProp->szLongName ) == 0);
          if ( !fFound && !fCaseMismatchFound )
          {
            fCaseMismatchFound = (stricmp( pszLongName, pProp->szLongName ) == 0);
            if ( fCaseMismatchFound )
            {
              strcpy( szShortNameCaseMismatch, RESBUFNAME( pData->stResultBuf ) );
            } /* endif */
          } /* endif */
        }
        else
        {
          fFound = (stricmp( pszLongName, pProp->szLongName ) == 0);
        } /* endif */


        if ( fFound )
        {
          *pfIsNew = FALSE;
          strcpy( pszShortName, RESBUFNAME( pData->stResultBuf ) );
        } /* endif */

        UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
      } /* endif */

      // try next document
      if ( *pfIsNew )
      {
        usCount = 1;
        usDosRC = UtlFindNext( hDir, &pData->stResultBuf,
                               sizeof(pData->stResultBuf), &usCount, FALSE );
      } /* endif */
    } /* endwhile */

    UtlFindClose( hDir, FALSE );

    if ( *pfIsNew && fCaseMismatchFound )
    {
      *pfIsNew = FALSE;
      strcpy( pszShortName, szShortNameCaseMismatch );
    } /* endif */

  } /* endif */

  // find a unique name if document is not contained in the folder
  if ( (usRC == NO_ERROR) && *pfIsNew )
  {
    SHORT i1 = 0;                      // counter for document extension 
    SHORT i2 = 0;                      // counter for document extension 
    SHORT i3 = 0;                      // counter for document extension 
    BOOL  fOverFlow = FALSE;
    CHAR chLetters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    CHAR szExt[4] = "000";

    sprintf( pData->szSearchPath, "%c:\\%s\\%s\\%s", pszFolObjName[0], PATH,
             pData->szFolder, SOURCEDIR );

    do
    {
      szExt[0] = chLetters[i3];
      szExt[1] = chLetters[i2];
      szExt[2] = chLetters[i1];

      sprintf( pData->szDocFullPath, "%s\\%s.%s", pData->szSearchPath, pData->szShortName, szExt );

      // get next number
      i1 += 1;
      if ( i1 >= 36 )
      {
        i1 = 0;
        i2 += 1;
        if ( i2 >= 36 )
        {
          i2 = 0;
          i3 += 1;
          if ( i3 >= 36 )
          {
            fOverFlow = TRUE;
          } /* endif */
        } /* endif */
      } /* endif */
    } while ( !fOverFlow && UtlFileExist( pData->szDocFullPath ) ); /* enddo */

    strcpy( pszShortName, UtlGetFnameFromPath( pData->szDocFullPath ) );
  } /* endif */

  // cleanup
  if ( pData != NULL ) UtlAlloc( (PVOID *)&pData, 0L, 0L,NOMSG );

  if ( pszFileName != NULL ) UtlAlloc( (PVOID *)&pszFileName, 0L, 0L,NOMSG );

  return( usRC );
} /* end of function FolLongToShortDocName */

//+----------------------------------------------------------------------------+
//|  Fill List Box with document names                                         |
//   Special mode if listbox handle is HWND_FUNCIF in this case
//   the pszBuffer is the address!!! of a buffer pointer and the area
//   pointed to by this pointer will contain the names of the documents on
//   return
//+----------------------------------------------------------------------------+
USHORT LoadDocumentNames( PSZ folder, HWND hlbox, LONG flg, PSZ pszBuffer )
{
  EQFINFO     ErrorInfo;               // error info from EQF API call
  PPROPSYSTEM pSysp;                   // EQF system properties
  FILEFINDBUF stResultBuf;             // DOS file find struct
  USHORT      usCount = 1;             // number of files requested
  USHORT      usRC;                    // return code of called functions
  HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle
  char       *ppath, *pform;
  PVOID           hProp;               // handle of document properties
  SHORT        sItem;
  CHAR szFormat[MAX_FILESPEC];         // folder format / Tag Table
  static CHAR szMemory[MAX_LONGFILESPEC]; // folder Translation Memory
  static CHAR szFolObjName[MAX_EQF_PATH]; // folder object name
  CHAR szSourceLang[MAX_LANG_LENGTH];  // folder source language
  CHAR szTargetLang[MAX_LANG_LENGTH];  // folder target language
  CHAR szEditor[MAX_FILESPEC];         // folder editor
  PSZ  pszName = RESBUFNAME(stResultBuf);     // address name field in result buffer
  USHORT      usDocs  = 0;             // number of documents found
  PSUBFOLINFO pInfo = NULL;            // subfolder information table

  // variables for document name buffer
  LONG        lBufferSize = 0L;        // size of document buffer
  LONG        lBufferUsed = 0L;        // used bytes in document buffer
  PSZ         pDocNameBuffer = NULL;   // ptr to document buffer
  ULONG       ulFolderID = 0L;         // ID of current folder/subfolder
  BOOL        fDisabled = FALSE;       // folder is disabled flag


#ifdef MEASURETIME
  LONG64 lDummy = 0;
  LONG64 lOther = 0;
  LONG64 lMakeDocListItem = 0;
  LONG64 lInsertItem = 0;
  LONG64 lSubFolCreateInfoTable = 9;
  LONG64 lFolQueryInfo = 0;
  LONG64 lFind = 0;
  LONG64 lPropUpdate = 0;
#endif

  // strip off with-documents-from-included-subfolders flag
  BOOL fWithSubFolderDocs = flg & LOADDOCNAMES_INCLSUBFOLDERS;
  flg &= ~LOADDOCNAMES_INCLSUBFOLDERS;

#ifdef MEASURETIME
  GetElapsedTime( &lDummy );
#endif


  /***********************************************************/
  /* Get settings from folder                                */
  /***********************************************************/
  strcpy( szFolObjName, folder );
  ulFolderID = FolGetSubFolderIdFromObjName( folder );

  // get subfolder information structure for current folder
  if ( fWithSubFolderDocs )
  {
#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif
    // build table containing info on all subfolders of active folder
    SubFolCreateInfoTable( folder, &pInfo );

#ifdef MEASURETIME
    GetElapsedTime( &lSubFolCreateInfoTable );
#endif

    // flag all subfolders belonging to current (sub)folder
    {
      PSUBFOLINFO pCurEntry = pInfo;
      while ( pCurEntry->szName[0] != EOS )
      {
        if ( pCurEntry->ulParentFolder == ulFolderID )
        {
          pCurEntry->fFlag = TRUE;
          pCurEntry->ulValue = TRUE; // we have to check the childs of this entry ...
        } /* endif */
        pCurEntry++;
      } /* endwhile */

      // now process all subfolders with ulValue set
      pCurEntry = pInfo;
      while ( pCurEntry->szName[0] != EOS )
      {
        if ( pCurEntry->ulValue )
        {
          PSUBFOLINFO pSubEntry = pInfo;
          while ( pSubEntry->szName[0] != EOS )
          {
            if ( pSubEntry->ulParentFolder == pCurEntry->ulID )
            {
              pSubEntry->fFlag = TRUE;
              pSubEntry->ulValue = TRUE; // we have to check the childs of this entry ...
            } /* endif */
            pSubEntry++;
          } /* endwhile */

          pCurEntry->ulValue = FALSE;// check for child subfolders has been done
          pCurEntry = pInfo;         // restart at the beginning
        }
        else
        {
          pCurEntry++;
        } /* endif */
      } /* endwhile */
    }
  } /* endif */

#ifdef MEASURETIME
    GetElapsedTime( &lDummy );
#endif

  FolQueryInfoEx( folder, szMemory, szFormat, szSourceLang, szTargetLang, szEditor, NULL, NULL, NULL, &fDisabled, NULL, NULL, FALSE, NULLHANDLE );

#ifdef MEASURETIME
    GetElapsedTime( &lFolQueryInfo );
#endif

  if ( hlbox != HWND_FUNCIF )
  {
    ENABLEUPDATEHWND_FALSE( hlbox );
    SETCURSOR( SPTR_WAIT );
  } /* endif */

  pSysp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
  UtlAlloc( (PVOID *)&ppath, 0L, (LONG) (2 * MAX_PATH144), ERROR_STORAGE );
  pform = ppath + MAX_PATH144;
  if ( FolIsSubFolderObject( folder ) )
  {
    strcpy( szFolObjName, folder );
    UtlSplitFnameFromPath( szFolObjName );  // cut off subfolder name
    UtlSplitFnameFromPath( szFolObjName );  // cut off property directory
  } /* endif */
  sprintf( ppath, "%s\\%s\\*%s", szFolObjName, pSysp->szDirSourceDoc, EXT_DOCUMENT);

#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif

  usRC = UtlFindFirst( ppath, &hDirHandle, FILE_NORMAL, &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);

#ifdef MEASURETIME
    GetElapsedTime( &lFind );
#endif

  while ( (usRC == NO_ERROR) && usCount )
  {
    //--- check properties of document ---
#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif

    hProp = OpenProperties( pszName, szFolObjName, PROP_ACCESS_READ, &ErrorInfo);

#ifdef MEASURETIME
    GetElapsedTime( &lFind );
#endif
    if ( hProp )
    {
      PPROPDOCUMENT pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );
      BOOL fContainedInFolder = FALSE;

      if ( pProp->ulParentFolder == ulFolderID )
      {
        // document belongs to our subfolder
        fContainedInFolder = TRUE;
      }
      else if ( fWithSubFolderDocs )
      {
        // loop through subfolder info table and check if document belongs
        // to an included subfolder
        PSUBFOLINFO pCurEntry = pInfo;
        while ( pCurEntry->szName[0] != EOS )
        {
          if ( pCurEntry->fFlag && (pCurEntry->ulID == pProp->ulParentFolder) )
          {
            fContainedInFolder = TRUE;
            break;
          }
          else
          {
            pCurEntry++;
          } /* endif */
        } /* endwhile */
      } /* endif */

      if ( !fContainedInFolder )
      {
        // ignore this document, it does not belong to our subfolder
      }
      else if ( (flg == LOADDOCNAMES_ITEMTEXT) && (hlbox != HWND_FUNCIF) )
      {
        // update document properties if one of the parent settings is the same
        // as the document settings
        BOOL fUpdate = FALSE;

        PSZ  pszDocMem = pProp->szLongMemory;
        if (*pszDocMem == EOS )
        {
          pszDocMem = pProp->szMemory;
        } /* endif */

        if ( strcmp( szMemory, pszDocMem ) == 0 )
        {
          // memory is identical to parent memory, blank it out
          pProp->szMemory[0] = EOS;
          pProp->szLongMemory[0] = EOS;
          fUpdate = TRUE;
        } /* endif */

        // fix for S613005 do not update markup table automatically!!!
        //if ( strcmp( szFormat, pProp->szFormat ) == 0 )
        //{
        //  // markup is identical to parent markup, blank it out
        //  pProp->szFormat[0] = EOS;
        //  fUpdate = TRUE;
        //} /* endif */

        if ( strcmp( szSourceLang, pProp->szSourceLang ) == 0 )
        {
          // source language is identical to parent source language, blank it out
          pProp->szSourceLang[0] = EOS;
          fUpdate = TRUE;
        } /* endif */

        if ( strcmp( szTargetLang, pProp->szTargetLang ) == 0 )
        {
          // Target language is identical to parent target language, blank it out
          pProp->szTargetLang[0] = EOS;
          fUpdate = TRUE;
        } /* endif */

        // update document properties if required
        if ( fUpdate )
        {
#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif
          if ( SetPropAccess( hProp, PROP_ACCESS_WRITE) )
          {
            SaveProperties( hProp, &ErrorInfo);
            ResetPropAccess( hProp, PROP_ACCESS_WRITE );
          } /* endif */
#ifdef MEASURETIME
    GetElapsedTime( &lPropUpdate );
#endif

        } /* endif */

        // create listbox item
#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif

        FOLMakeDocListItem ( pProp, szFormat, szMemory, szSourceLang, szTargetLang, szEditor, pszBuffer );

#ifdef MEASURETIME
    GetElapsedTime( &lMakeDocListItem );
#endif

        sItem = INSERTITEMHWND( hlbox, pszBuffer );
        if ( fDisabled && (sItem != LIT_NONE) )
        {
          WinSendMsg( hlbox, LM_EQF_SETITEMSTATE, MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
        } /* endif */


#ifdef MEASURETIME
    GetElapsedTime( &lInsertItem );
#endif

        usDocs++;
      }
      else
      {
        if ( hlbox != HWND_FUNCIF )
        {
          if ( flg == LOADDOCNAMES_OBJNAME )
          {
            OBJNAME szDocObjName;
            strcpy( szDocObjName, pProp->PropHead.szPath );
            strcat( szDocObjName, BACKSLASH_STR );
            strcat( szDocObjName, pProp->PropHead.szName );
            INSERTITEMHWND( hlbox, szDocObjName );
          }
          else if ( flg == LOADDOCNAMES_LONGNAME )
          {
            PSZ pszLongName = pProp->szLongName;
            if ( *pszLongName == EOS ) pszLongName = pszName;

            INSERTITEMHWND( hlbox, pszLongName );
          }
          else
          {
            INSERTITEMHWND( hlbox, pszName );
          } /* endif */
        }
        else
        {
          // document name buffer mode of function
          OBJNAME szDocObjName;
          PSZ pszInsertName;
          LONG lAddLen; 
          
          // get/build document name
          if ( flg == LOADDOCNAMES_OBJNAME )
          {
            strcpy( szDocObjName, pProp->PropHead.szPath );
            strcat( szDocObjName, BACKSLASH_STR );
            strcat( szDocObjName, pProp->PropHead.szName );
            pszInsertName = szDocObjName;
          }
          else if ( flg == LOADDOCNAMES_LONGNAME )
          {
            pszInsertName = pProp->szLongName;
            if ( *pszInsertName == EOS ) pszInsertName = pszName;
          }
          else
          {
            pszInsertName = pszName;
          } /* endif */

          // add document name to document name buffer
          lAddLen = strlen(pszInsertName) + 1;
          if ( lBufferSize < (lBufferUsed + lAddLen) )
          {
            UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                          lBufferSize + 8096L, ERROR_STORAGE, HWND_FUNCIF );
            lBufferSize += 8096L;
          } /* endif */

          if ( pDocNameBuffer != NULL )
          {
            strcpy( pDocNameBuffer + lBufferUsed, pszInsertName );
            lBufferUsed += lAddLen;
          } /* endif */
        } /* endif */
        usDocs++;
      } /* endif */

      CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
    }
    else
    {
      /*************************************************************/
      /* Insert as disabled item                                  */
      /*************************************************************/
      if ( (flg == LOADDOCNAMES_ITEMTEXT) && (hlbox != HWND_FUNCIF) )
      {
        sprintf( pszBuffer,
                 "%s\\%s\x15%s\x15 \x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15 \x15 \x15 \x15 \x15 \x15",
                 szFolObjName,              // FOL_OBJECT_IND
                 pszName,
                 pszName,                   // FOL_NAME_IND
                 0L,                        // FOL_TRANSLATED_IND
                 0L,                        // FOL_SEGMENTED_IND
                 0L,                        // FOL_EXPORTED_IND
                 0L,                        // FOL_IMPORTED_IND
                 0L,                        // FOL_TOUCHED_IND
                 0L                         // FOL_TOUCHEDTIME_IND
               );
        sItem = INSERTITEMHWND( hlbox, pszBuffer );
        usDocs++;
        if ( sItem != LIT_NONE )
        {
          WinSendMsg( hlbox, LM_EQF_SETITEMSTATE, MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
        } /* endif */
      } /* endif */
    } /* endif */

#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif
    usRC = UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf),
                        &usCount, 0);
#ifdef MEASURETIME
    GetElapsedTime( &lFind );
#endif
  } /* endwhile */
  // close search file handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );


  UtlAlloc( (PVOID *)&ppath, 0L, 0L, NOMSG );


  if ( hlbox != HWND_FUNCIF )
  {
    ENABLEUPDATEHWND_TRUE( hlbox);
    SETCURSOR( SPTR_ARROW );
  }
  else
  {
    if ( pDocNameBuffer == NULL )
    {
      UtlAllocHwnd( (PVOID *)&pDocNameBuffer, 0L, 8096L, ERROR_STORAGE, HWND_FUNCIF );
    } /* endif */

    if ( pDocNameBuffer != NULL )
    {
      // terminate document buffer and return pointer to it
      // it is up to the caller to free the document buffer
      *(pDocNameBuffer + lBufferUsed) = EOS;
    } /* endif */
    *((PSZ *)pszBuffer) = pDocNameBuffer;
  } /* endif */

  if ( pInfo ) UtlAlloc( (PVOID *)&pInfo, 0L, 0L, NOMSG );

#ifdef MEASURETIME
  GetElapsedTime( &lOther );

  {
    FILE *hfLog = NULL;
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    strcat( szLogFile, "\\FOLLIST.LOG" );

    hfLog = fopen( szLogFile, "a" );

    if ( hfLog )
    {
      fprintf( hfLog, "**** INSERTNAMES for %s ********\n", folder );

      fprintf( hfLog, "Time for MakeDocListItem   : %10I64d ms\n", lMakeDocListItem );
      fprintf( hfLog, "INSERTITEM in listbox time : %10I64d ms\n", lInsertItem );
      fprintf( hfLog, "SubFolCreateInfoTable time : %10I64d ms\n", lSubFolCreateInfoTable );
      fprintf( hfLog, "FolQueryInfo time          : %10I64d ms\n", lFolQueryInfo );
      fprintf( hfLog, "FindFirst/FindNext time    : %10I64d ms\n", lFind );
      fprintf( hfLog, "DocPropUpdate time         : %10I64d ms\n", lPropUpdate );
      fprintf( hfLog, "Other times                : %10I64d ms\n", lOther );
 
      fclose( hfLog );
    } /* endif */


  }
#endif

  return( usDocs );
}


//------------------------------------------------------------------------------
// Create a document list listbox item
//------------------------------------------------------------------------------
BOOL FOLMakeDocListItem
(
PPROPDOCUMENT  pProp,               // property pointer
PSZ       pszFormat,                // name of folder format table
PSZ       pszMemory,                // name of folder translation memory
PSZ       pszSourceLang,            // name of folder source language
PSZ       pszTargetLang,            // name of folder target language
PSZ       pszEditor,                // name of folder editor
PSZ       pszBuffer                 // buffer to fill with folderlist item
)
{
  CHAR            szDocPath[MAX_EQF_PATH]; // path to source document
  USHORT           usCount = 1;        // For UtlFindFirst
  HDIR             hSearch = HDIR_CREATE; // Directory handle for UtlFindFirst
  FILEFINDBUF      stFile;             // Output buffer of UtlFindFirst
  PSZ              pszExtension;       // points to file name extension
  PSZ              pszDocName;         // points to document name
  PSZ              pszName;            // points to file name without path info

  /*******************************************************************/
  /* Build document path                                             */
  /*******************************************************************/
  strcpy( szDocPath, pProp->PropHead.szPath );
  strcat( szDocPath, BACKSLASH_STR );
  UtlQueryString( QST_SOURCEDIR, szDocPath + strlen(szDocPath), MAX_FILESPEC );
  strcat( szDocPath, BACKSLASH_STR );
  strcat( szDocPath, pProp->PropHead.szName );

  /*******************************************************************/
  /* Get document size                                               */
  /*******************************************************************/
  memset( &stFile, 0, sizeof(stFile) );
  UtlFindFirst( szDocPath, &hSearch, 0, &stFile, sizeof(stFile), &usCount,
                0L, FALSE );
  UtlFindClose( hSearch, FALSE );

  /*******************************************************************/
  /* Use document specific memory, format and languages              */
  /*******************************************************************/
  if ( pProp->szLongMemory[0] != EOS )
    pszMemory = pProp->szLongMemory;
  else
    if ( pProp->szMemory[0] != EOS )     pszMemory = pProp->szMemory;
  if ( pProp->szFormat[0] != EOS )     pszFormat = pProp->szFormat;
  if ( pProp->szSourceLang[0] != EOS ) pszSourceLang = pProp->szSourceLang;
  if ( pProp->szTargetLang[0] != EOS ) pszTargetLang = pProp->szTargetLang;
  if ( pProp->szEditor[0] != EOS )     pszEditor = pProp->szEditor;
  /*******************************************************************/
  /* build complete item string                                      */
  /*******************************************************************/

  OEMTOANSI( pProp->szLongName );
  OEMTOANSI( pProp->szVendor );

  // get file name without path part
  pszDocName = ( pProp->szLongName[0] != EOS ) ? pProp->szLongName : pProp->PropHead.szName;
  pszName = strrchr( pszDocName, BACKSLASH );
  if ( pszName )
  {
    pszName = pszName + 1;
  }
  else
  {
    pszName = pszDocName;
  } /* endif */

  // get file name extension (if any)
  pszExtension = strrchr( pszName, '.' );
  if ( pszExtension )
  {
    pszExtension = pszExtension + 1;
  }
  else
  {
    pszExtension = pszName + strlen( pszName ); // position to string end character
  } /* endif */

  sprintf( pszBuffer,
//           "%s\\%s\x15%s\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%.1lu\x15%d\x15%d\x15%d\x15%d\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%lu\x15" ,
           "%s\\%s\x15%s\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%s\x15%lu\x15%.1lu\x15%d\x15%d\x15%s\x15%d\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%lu\x15%s\x15%s\x15" ,
           pProp->PropHead.szPath,    // FOL_OBJECT_IND
           pProp->PropHead.szName,
           ( pProp->szLongName[0] == EOS ) ?  // FOL_NAME_IND
           pProp->PropHead.szName : pProp->szLongName,
           pProp->Format,             // FOL_FORMAT_IND
           pProp->ulXLated,           // FOL_TRANSLATED_IND
           pProp->ulSeg,              // FOL_SEGMENTED_IND
           pProp->ulExp,              // FOL_EXPORTED_IND
           pProp->ulImp,              // FOL_IMPORTED_IND
//           pProp->ulTouched,          // FOL_TOUCHED_IND
           pProp->szVendor,           // FOL_TRANSLATOR_IND
           pProp->ulTouched,          // FOL_TOUCHEDTIME_IND
           RESBUFSIZE(stFile),        // FOL_SIZE_IND
           pProp->usComplete,         // FOL_COMPLETE_IND
           pProp->usModified,         // FOL_MODIFIED_IND
           pProp->szShipment,         //                    (was pProp->usScratch),  FOL_SCRATCH_IND
           0,                         // currently not used (was pProp->usCopied),   FOL_COPIED_IND
           pszFormat,
           pszMemory,
           pszSourceLang,
           pszTargetLang,
           pszEditor,
           pProp->szAlias,
           pProp->ulSrc,
           pszName,
           pszExtension
         );
  ANSITOOEM( pProp->szLongName );
  ANSITOOEM( pProp->szVendor );
  return( TRUE );
}

// create the subfolder information table for the given folder
BOOL SubFolCreateInfoTable( PSZ pszFolderObjName, PSUBFOLINFO *ppInfo )
{
  FILEFINDBUF stResultBuf;             // DOS file find struct
  USHORT      usCount = 1;             // number of files requested
  USHORT      usRC;                    // return code of called functions
  HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle

  PSZ          pszPath;
  PSZ          pszFileName = NULL;     // subfolder properties file name
  PSZ          pszSearchName = NULL;   // subfolder properties search path
  PPROPFOLDER  pProp = NULL;           // subfolder properties
  PSZ  pszName = RESBUFNAME(stResultBuf);// address name field in result buffer
  BOOL        fOK = TRUE;              // function return code
  int         iCurEntry = 0;           // current entry in info table
  int         iTableSize = 0;          // current size (elements) of info table
  PSUBFOLINFO pInfo = NULL;            // ptr to subfolder info table



  fOK = UtlAlloc( (PVOID *)&pszPath, 0L, (LONG) (3 * MAX_PATH144), ERROR_STORAGE );
  if ( fOK )
  {
    pszFileName = pszPath + MAX_PATH144;
    pszSearchName = pszFileName + MAX_PATH144;
  } /* endif */

  // allocate initial subfolder info table with minimal size (for folders which have no subfolders)
  if ( fOK )
  {
    int iNewSize = 1;
    fOK = UtlAlloc( (PVOID *)&pInfo, (iTableSize * sizeof(SUBFOLINFO)),
                    (iNewSize * sizeof(SUBFOLINFO)), NOMSG );
    if ( fOK ) iTableSize = iNewSize;
  } /* endif */

  // build subfolder search path
  if ( fOK )
  {
    strcpy( pszSearchName, pszFolderObjName );
    if ( FolIsSubFolderObject( pszFolderObjName ) )
    {
      UtlSplitFnameFromPath( pszSearchName );  // cut off subfolder name
      UtlSplitFnameFromPath( pszSearchName );  // cut off property directory
    } /* endif */
    strcpy( pszPath, pszSearchName );
    strcat( pszSearchName, BACKSLASH_STR );
    strcat( pszSearchName, PROPDIR );
    strcat( pszSearchName, "\\*" );
    strcat( pszSearchName, EXT_OF_SUBFOLDER );
  } /* endif */

  if ( fOK )
  {
    usRC = UtlFindFirst( pszSearchName, &hDirHandle, FILE_NORMAL, &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);
    while ( (usRC == NO_ERROR) && usCount )
    {
      ULONG ulLen;

      // load subfolder properties
      sprintf( pszFileName, "%s\\%s\\%s", pszPath, PROPDIR, pszName );
      if ( UtlLoadFileL( pszFileName, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
      {
        // enlarge info table if necessary
        if ( (iCurEntry + 1) >= iTableSize )
        {
          int iNewSize = iTableSize + 20;

          fOK = UtlAlloc( (PVOID *)&pInfo, (iTableSize * sizeof(SUBFOLINFO)),
                          (iNewSize * sizeof(SUBFOLINFO)), NOMSG );
          if ( fOK ) iTableSize = iNewSize;
        } /* endif */

        // add current subfolder to table
        if ( fOK )
        {
          strcpy( pInfo[iCurEntry].szName,  pProp->szLongName );
          pInfo[iCurEntry].ulParentFolder = pProp->ulParentFolder;
          pInfo[iCurEntry].ulID           = pProp->ulSubFolderID;
          iCurEntry++;
        } /* endif */
        UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        pProp = NULL;
      } /* endif */

      usRC = UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf),
                          &usCount, 0);
    } /* endwhile */
  } /* endif */

  // close search file handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

  UtlAlloc( (PVOID *)&pszPath, 0L, 0L, NOMSG );

  if ( fOK )
  {
    *ppInfo = pInfo;
  }
  else
  {
    *ppInfo = NULL;
    if ( pInfo ) UtlAlloc( (PVOID *)&pInfo, 0L, 0L, NOMSG );
  } /* endif */

  return( fOK );

} /* end of function SubFolCreateInfoTable */

// function to get the ID of a folder or subfolder using the object name
// for folders an ID of 0 is returned
ULONG FolGetSubFolderIdFromObjName( PSZ pszObjName )
{
  ULONG ulID = 0L;                     // subfolder ID, default is zero

  if ( FolIsSubFolderObject( pszObjName ) )
  {
    // position to subfolder name within object name
    PSZ pszName = strrchr( pszObjName, BACKSLASH );
    if ( pszName )
    {
      PSZ pszExtention = strchr( pszName, DOT );
      if ( pszExtention )
      {
        *pszExtention = EOS;
        ulID = atol( pszName + 1 );
        *pszExtention = DOT;
      } /* endif */
    } /* endif */
  } /* endif */

  return( ulID );
} /* end of function FolGetSubFolderIdFromObjName */

