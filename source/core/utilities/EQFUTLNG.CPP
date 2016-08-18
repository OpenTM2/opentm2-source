//+----------------------------------------------------------------------------+
//|EQFUTLNG.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_MORPH            // general morph functions
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                  // General Translation Manager include file
#include <eqflng00.h>             // specific defines for Language properties
#include <eqfsetup.h>             // specific defines for Language properties

#include "LanguageFactory.H"


// callback function to add languages to a combobox
int AddLangToComboBox( PVOID pvData, char *pszName, LanguageFactory::PLANGUAGEINFO pInfo  ){ pInfo; CBINSERTITEMHWND( (HWND)pvData, pszName ); return( 0 ); }

// callback function to add languages to a listbox
int AddLangToListBox( PVOID pvData, char *pszName, LanguageFactory::PLANGUAGEINFO pInfo  ){ pInfo; INSERTITEMHWND( (HWND)pvData, pszName ); return( 0 ); }


/*+---------------------------------------------------------------------------+
  | Function Name:  UtlFillTableLB                                            |
  |                                                                           |
  | Description:    This function fills a list boxes with all available       |
  |                 Markup languages, Noise lists, Source- or Target-         |
  |                 languages.                                                |
  |                                                                           |
  | Call:           USHORT UtlFillTableLB                                     |
  |                 (                                                         |
  |                   HWND   hListBox,                                        |
  |                   USHORT usBoxType                                        |
  |                 )                                                         |
  |                                                                           |
  | Where:          - hListBox is the window handle of the list box           |
  |                   to be filled.                                           |
  |                 - usBoxType specifies the type of table to be searched    |
  |                   and type of listbox to be filled.                       |
  |                   The following table types can be specified:             |
  |                    FORMAT_TABLE         (Markup language)                 |
  |                    EXCLUSION_LIST       (Word list, Noise list)           |
  |                    SOURCE_LANGUAGES     (active source languages)         |
  |                    TARGET_LANGUAGES     (active target languages)         |
  |                    ALL_SOURCE_LANGUAGES (all possible source languages)   |
  |                    ALL_TARGET_LANGUAGES (all possible target languages)   |
  |                                                                           |
  | Returns:        Number of entries found. 0 means no entry found or error. |
  |                                                                           |
  | Example:        The following example fills a list box with the ID        |
  |                 DID_CD_FORMAT. The list box is part of a dialog box.      |
  |                 The list box will contain all available markup            |
  |                 language tables available in the system. The properties   |
  |                 szLastUsed will be searched for last used values.         |
  |                                                                           |
  |                 UtlFillTableLB( WinWindowFromID(hDlg, DID_CD_FORMAT),     |
  |                                 FORMAT_TABLE );                           |
  +---------------------------------------------------------------------------+
*/

  USHORT UtlFillTableLB( HWND   hListBox,
                         USHORT usBoxType
                       )
{
  HDIR             hSearch = HDIR_CREATE; // Directory handle for DosFindxxx calls
  FILEFINDBUF      stFile;                // Output buffer of DosFindxxx
  USHORT           usDosRc;               // function return code of Dos calls
  USHORT           usNumbOfItems;         // Number of Items found
  USHORT           usCount;               // number of entries to be searched
  CHAR             szName[MAX_LANG_LENGTH]; // buffer for items being inserted
  CHAR             szPath[MAX_EQF_PATH];    // buffer for path names
  BOOL             fCombo;                  // listbox-is-a-combobox flag

  ENABLEUPDATEHWND_FALSE( hListBox );
  ISCOMBOBOX( hListBox, fCombo );


  switch ( usBoxType )
  {
    //-----------------------------------------------------------------------
    case FORMAT_TABLE:
    case EXCLUSION_LIST:
     UtlMakeEQFPath( szPath, NULC, TABLE_PATH, NULL );
     strcat( szPath, BACKSLASH_STR );
     strcat( szPath, DEFAULT_PATTERN_NAME );
     strcat( szPath, (usBoxType == FORMAT_TABLE) ? EXT_OF_FORMAT :
                                                   EXT_OF_EXCLUSION );

     // Search the first entry
     usCount = 1;
     usDosRc = UtlFindFirst( szPath, &hSearch, 0, &stFile,
                        sizeof(stFile), &usCount, 0L, FALSE );

     // Fill the list box with entries
     while ( usCount == 1 )
     {
       Utlstrccpy( szName, RESBUFNAME(stFile), DOT );
       if ( fCombo )
       {
         CBINSERTITEMHWND( hListBox, szName );
       }
       else
       {
         INSERTITEMHWND( hListBox, szName );
       } /* endif */
       usDosRc = UtlFindNext( hSearch, &stFile, sizeof(stFile), &usCount, FALSE );
     } /* endwhile */
     // close file search handle
     if ( hSearch != HDIR_CREATE ) UtlFindClose( hSearch, FALSE );


     break;

    case SOURCE_LANGUAGES     :
    case ALL_SOURCE_LANGUAGES :
      {
        LanguageFactory *pLangFactory = LanguageFactory::getInstance();
        pLangFactory->listLanguages( LanguageFactory::SOURCE_LANGUAGE_TYPE, fCombo ? AddLangToComboBox : AddLangToListBox, (PVOID)hListBox, FALSE );
      }
      break;

    case TARGET_LANGUAGES     :
    case ALL_TARGET_LANGUAGES :
      {
        LanguageFactory *pLangFactory = LanguageFactory::getInstance();
        pLangFactory->listLanguages( LanguageFactory::TARGET_LANGUAGE_TYPE, fCombo ? AddLangToComboBox : AddLangToListBox, (PVOID)hListBox, FALSE );
      }
      break;
  } /* endswitch */

  if ( fCombo )
  {
     usNumbOfItems = CBQUERYITEMCOUNTHWND( hListBox );
  }
  else
  {
     usNumbOfItems = QUERYITEMCOUNTHWND( hListBox );
  } /* endif */


  ENABLEUPDATEHWND_TRUE( hListBox );

  return ( usNumbOfItems );
} /* end of function UtlFillTableLB */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlDeleteLanguage                                        |
//+----------------------------------------------------------------------------+
//|Function call:     usRc=UtlDeleteLanguage(PSZ, BOOL)                        |
//+----------------------------------------------------------------------------+
//|Description:       This function deletes a language support file            |
//|                   NOTE: No NOISELIST will be deleted any more - because    |
//|                         we do not use them                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ   pFile   name of dict file to be deleted            |
//|                   BOOL  fMsg    Message flag                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       DOS Rc                                                   |
//|                   0      file deleted                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     make EQF-path                                            |
//|                   return (fOk)                                             |
//+----------------------------------------------------------------------------+
USHORT UtlDeleteLanguage
(
  PSZ  pszObjName,                     // language property to be deleted..
  BOOL fMsg                            // message flag
)
{
  USHORT usRc = 0;

  // GQ: This function is not supported anymore, languages are now handled by the spell and morph plugins
  pszObjName; fMsg;

  return (usRc) ;
}/* end of UtlDeleteLanguage */




/*
 ** UtlQuerySysLangFile
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *    Scans the system language file for the given language and,
 *    if found, set the names for the resource, the name of the
 *    help file and the name of the message file or returns FALSE
 *    if the language was not found. The returned names are fully
 *    qualified path names except for the name of the resource
 *    file.
 *
 *  RETURNS:
 *        BOOL  fOK   TRUE = language was found and file names have been
 *                                               set
 *                FALSE = language not found in file, file names have
 *                                                not been set
 */

BOOL UtlQuerySysLangFile
(
  PSZ              pszLanguage,        // language to search in file
  PSZ              pszResFile,         // buffer for name of resource file
  PSZ              pszHelpFile,        // buffer for name of help file
  PSZ              pszMsgFile          // buffer for name of message file
)
{
  CHAR     szFileName[MAX_EQF_PATH];   // buffer for file name
  BOOL     fOK = TRUE;                 // function return code
  PSZ      pszLangFileBuffer = NULL;   // buffer for in-memory copy of language file
  PSZ      pszCurLine;                 // ptr to current line in file
  BOOL     fFound = FALSE;
  ULONG    ulLength;                   // length of language file
  USHORT   usI;                        // loop index
  CHAR     szDrive[MAX_DRIVE];         // buffer for drive list
  CHAR     szLanDrive[MAX_DRIVE];      // buffer for drive list
  CHAR     szSysPath[MAX_EQF_PATH];    // global system path
  CHAR     EqfSystemPath[MAX_EQF_PATH];// global system path

  /********************************************************************/
  /* initialize result fields                                         */
  /********************************************************************/
  *pszResFile  = EOS;
  *pszHelpFile = EOS;
  *pszMsgFile  = EOS;

  /********************************************************************/
  /* build name of system language file                               */
  /********************************************************************/
  GetStringFromRegistry( APPL_Name, KEY_Drive, szDrive, sizeof( szDrive  ), "" );
  GetStringFromRegistry( APPL_Name, KEY_LanDrive, szLanDrive, sizeof( szLanDrive  ), "" );
  GetStringFromRegistry( APPL_Name, KEY_Path, szSysPath, sizeof( szSysPath), "" );
  if ( !szLanDrive[0] )
  {
    strcpy(szLanDrive, szDrive);
  } /* endif */
  sprintf( EqfSystemPath, "%s\\%s",  szLanDrive, szSysPath );
  strcpy( szFileName, EqfSystemPath );
  strcat( szFileName, BACKSLASH_STR );
  strcat( szFileName, TABLEDIR );
  strcat( szFileName, BACKSLASH_STR );
  strcat( szFileName, SYSLANGFILE );

  /********************************************************************/
  /* load system language file into memory                            */
  /********************************************************************/
  fOK = UtlLoadFileL( szFileName, (PVOID *)&pszLangFileBuffer,
                     &ulLength, FALSE, FALSE );

  /********************************************************************/
  /* preprocess data in language file: replace CR by space            */
  /********************************************************************/
  if ( fOK )
  {
    for ( usI = 0 ; usI < ulLength; usI++ )
    {
      if ( pszLangFileBuffer[usI] == CR )
      {
        pszLangFileBuffer[usI] = SPACE;
      } /* endif */
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* search for given language in file                                */
  /********************************************************************/
  if ( fOK )
  {
    pszCurLine = pszLangFileBuffer;

    pszLangFileBuffer[ulLength-1] = EOS; // force end string delimiter

    while ( !fFound && (*pszCurLine != EOS) )
    {
      if ( *pszCurLine == '*' )
      {
        /**************************************************************/
        /* ignore comment line                                        */
        /**************************************************************/
      }
      else
      {
        /**************************************************************/
        /* check language                                             */
        /**************************************************************/
        PSZ      pszStart;

        /**************************************************************/
        /* handle language names enclosed in double quotes            */
        /**************************************************************/
        if ( *pszCurLine == '\"' )
        {
          pszStart = pszCurLine + 1;
        }
        else
        {
          pszStart = pszCurLine;
        } /* endif */

        /**************************************************************/
        /* compare the language name                                  */
        /**************************************************************/
        if ( _strnicmp( pszStart, pszLanguage, strlen(pszLanguage) ) == 0 )
        {
          PSZ pszTarget;

          /************************************************************/
          /* Language has been found, now extract file names          */
          /************************************************************/

          /************************************************************/
          /* first of all skip language                               */
          /************************************************************/
          if ( *pszCurLine == DOUBLEQUOTE )
          {
            /**********************************************************/
            /* Skip language enclosed in double quotes                */
            /**********************************************************/
            pszCurLine++;
            while ( (*pszCurLine != LF ) &&
                    (*pszCurLine != EOS) &&
                    (*pszCurLine != DOUBLEQUOTE) )
            {
              pszCurLine++;
            } /* endwhile */
            if ( *pszCurLine == DOUBLEQUOTE )
            {
              pszCurLine++;
            } /* endif */
          }
          else
          {
            /**********************************************************/
            /* Skip language delimited by comma                       */
            /**********************************************************/
            while ( (*pszCurLine != LF ) &&
                    (*pszCurLine != EOS) &&
                    (*pszCurLine != COMMA) )
            {
              pszCurLine++;
            } /* endwhile */
          } /* endif */
          if ( *pszCurLine == COMMA ) pszCurLine++;

          /************************************************************/
          /* now extract message file name                            */
          /************************************************************/
          pszTarget = szFileName;
          while ( (*pszCurLine != LF ) &&
                  (*pszCurLine != EOS) &&
                  (*pszCurLine != COMMA) )
          {
            *pszTarget++ = *pszCurLine++;
          } /* endwhile */
          *pszTarget = EOS;
          UtlStripBlanks( szFileName );
          strcpy( pszMsgFile, EqfSystemPath );
          strcat( pszMsgFile, BACKSLASH_STR );
          strcat( pszMsgFile, MSGDIR );
          strcat( pszMsgFile, BACKSLASH_STR );
          strcat( pszMsgFile, szFileName );
          if ( *pszCurLine == COMMA ) pszCurLine++;

          /************************************************************/
          /* now extract help file name                               */
          /************************************************************/
          pszTarget = szFileName;
          while ( (*pszCurLine != LF ) &&
                  (*pszCurLine != EOS) &&
                  (*pszCurLine != COMMA) )
          {
            *pszTarget++ = *pszCurLine++;
          } /* endwhile */
          *pszTarget = EOS;
          UtlStripBlanks( szFileName );
          strcpy( pszHelpFile, EqfSystemPath );
          strcat( pszHelpFile, BACKSLASH_STR );
          strcat( pszHelpFile, MSGDIR );
          strcat( pszHelpFile, BACKSLASH_STR );
          strcat( pszHelpFile, szFileName );
          if ( *pszCurLine == COMMA ) pszCurLine++;


          /************************************************************/
          /* now extract resource file name                           */
          /************************************************************/
          pszTarget = szFileName;
          while ( (*pszCurLine != LF ) &&
                  (*pszCurLine != EOS) &&
                  (*pszCurLine != COMMA) )
          {
            *pszTarget++ = *pszCurLine++;
          } /* endwhile */
          *pszTarget = EOS;
          UtlStripBlanks( szFileName );
          strcpy( pszResFile, szFileName );
          if ( *pszCurLine == COMMA ) pszCurLine++;

          fFound = TRUE;
          fOK = (*pszResFile != EOS) &&
                (*pszMsgFile != EOS) &&
                (*pszHelpFile != EOS);
        }
        else
        {
          /************************************************************/
          /* languages do not match, ignore current line              */
          /************************************************************/
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* skip current line                                            */
      /****************************************************************/
      while ( (*pszCurLine != LF ) &&   (*pszCurLine != EOS) )
      {
        pszCurLine++;
      } /* endwhile */
      if ( *pszCurLine == LF )
      {
        pszCurLine++;
      } /* endif */
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* cleanup                                                          */
  /********************************************************************/
  if ( pszLangFileBuffer ) UtlAlloc( (PVOID *)&pszLangFileBuffer, 0L, 0L, NOMSG );

  return( fOK && fFound );
} /* end of function TWBQuerySysLangFile */

/*
 ** UtlFillSysLangPopup
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *    Adds the languages listed in the system language file
 *    to the menu hMenu, The items are inserted before item
 *    sDummy which is deleted after the items have been inserted.
 *    The ID used for the new items is sID + n (where n is the
 *    relative index of the new menu items). The menu item with
 *    the value pszCurLanguage receives a checkmark.
 *
 *  RETURNS:
 *        SHORT   number of inserted items
 */

SHORT UtlFillSysLangPopUp
(
  HMENU            hMenu,              // handle of menu
  SHORT            sDummy,             // ID of dummy item
  SHORT            sID,                // ID to be used for menu items
  PSZ              pszCurLanguage      // name of currently active language
)
{
  CHAR     szFileName[MAX_EQF_PATH];   // buffer for file name
  BOOL     fOK = TRUE;                 // function return code
  PSZ      pszLangFileBuffer = NULL;   // buffer for in-memory copy of language file
  PSZ      pszCurLine;                 // ptr to current line in file
  SHORT    sItems = 0;                 // number of items inserted
  ULONG    ulLength;                   // length of language file
  USHORT   usI;                        // loop index
  CHAR     szDrive[MAX_DRIVE];         // buffer for drive list
  CHAR     szLanDrive[MAX_DRIVE];      // buffer for drive list
  CHAR     szSysPath[MAX_EQF_PATH];    // global system path
  CHAR     szLanguage[MAX_EQF_PATH];   // current language
  CHAR     EqfSystemPath[MAX_EQF_PATH];// global system path
  PSZ      pszTarget;

  /********************************************************************/
  /* build name of system language file                               */
  /********************************************************************/
  GetStringFromRegistry( APPL_Name, KEY_Drive, szDrive, sizeof( szDrive  ), "" );
  GetStringFromRegistry( APPL_Name, KEY_LanDrive, szLanDrive, sizeof( szLanDrive  ), "" );
  GetStringFromRegistry( APPL_Name, KEY_Path, szSysPath, sizeof( szSysPath), "" );
  if ( !szLanDrive[0] )
  {
    strcpy(szLanDrive, szDrive);
  } /* endif */
  sprintf( EqfSystemPath, "%s\\%s",  szLanDrive, szSysPath );
  strcpy( szFileName, EqfSystemPath );
  strcat( szFileName, BACKSLASH_STR );
  strcat( szFileName, TABLEDIR );
  strcat( szFileName, BACKSLASH_STR );
  strcat( szFileName, SYSLANGFILE );

  /********************************************************************/
  /* load system language file into memory                            */
  /********************************************************************/
  fOK = UtlLoadFileL( szFileName, (PVOID *)&pszLangFileBuffer,
                     &ulLength, FALSE, FALSE );

  /********************************************************************/
  /* preprocess data in language file: replace CR by space            */
  /********************************************************************/
  if ( fOK )
  {
    for ( usI = 0 ; usI < ulLength; usI++ )
    {
      if ( pszLangFileBuffer[usI] == CR )
      {
        pszLangFileBuffer[usI] = SPACE;
      } /* endif */
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* search for given language in file                                */
  /********************************************************************/
  if ( fOK )
  {
    pszCurLine = pszLangFileBuffer;

    pszLangFileBuffer[ulLength-1] = EOS; // force end string delimiter

    while ( *pszCurLine != EOS )
    {
      if ( *pszCurLine == '*' )
      {
        /**************************************************************/
        /* ignore comment line                                        */
        /**************************************************************/
      }
      else
      {
        /**************************************************************/
        /* check language                                             */
        /**************************************************************/
        PSZ      pszStart, pszEnd;
        CHAR     chTemp;
        BOOL     fOK = TRUE;

        /**************************************************************/
        /* handle language names enclosed in double quotes            */
        /**************************************************************/
        if ( *pszCurLine == '\"' )
        {
          pszStart = pszCurLine + 1;
        }
        else
        {
          pszStart = pszCurLine;
        } /* endif */

        /**************************************************************/
        /* Isolate language name                                      */
        /**************************************************************/
        pszEnd = pszCurLine;
        if ( *pszEnd == DOUBLEQUOTE )
        {
          /**********************************************************/
          /* Skip language enclosed in double quotes                */
          /**********************************************************/
          pszEnd++;
          while ( (*pszEnd != LF ) &&
                  (*pszEnd != EOS) &&
                  (*pszEnd != DOUBLEQUOTE) )
          {
            pszEnd++;
          } /* endwhile */
          pszCurLine = pszEnd;
          if ( *pszCurLine == DOUBLEQUOTE )
          {
            pszCurLine++;
          } /* endif */

        }
        else
        {
          /**********************************************************/
          /* Skip language delimited by comma                       */
          /**********************************************************/
          while ( (*pszEnd != LF ) &&
                  (*pszEnd != EOS) &&
                  (*pszEnd != COMMA) )
          {
            pszEnd++;
          } /* endwhile */
          pszCurLine = pszEnd;
          if ( *pszCurLine == COMMA )
          {
            pszCurLine++;
          } /* endif */
        } /* endif */
        chTemp = *pszEnd;
        *pszEnd = EOS;
        strcpy( szLanguage, pszStart );
        *pszEnd = chTemp;

        /************************************************************/
        /* now extract message file name                            */
        /************************************************************/
        pszTarget = szFileName;
        while ( (*pszCurLine != LF ) &&
                (*pszCurLine != EOS) &&
                (*pszCurLine != COMMA) )
        {
          *pszTarget++ = *pszCurLine++;
        } /* endwhile */
        *pszTarget = EOS;
        UtlStripBlanks( szFileName );
        strcpy( szSysPath, EqfSystemPath );
        strcat( szSysPath, BACKSLASH_STR );
        strcat( szSysPath, MSGDIR );
        strcat( szSysPath, BACKSLASH_STR );
        strcat( szSysPath, szFileName );
        if ( *pszCurLine == COMMA ) pszCurLine++;
        fOK = UtlFileExist(szSysPath );

        /************************************************************/
        /* now extract help file name                               */
        /************************************************************/
        if ( fOK )
        {
          pszTarget = szFileName;
          while ( (*pszCurLine != LF ) &&
                  (*pszCurLine != EOS) &&
                  (*pszCurLine != COMMA) )
          {
            *pszTarget++ = *pszCurLine++;
          } /* endwhile */
          *pszTarget = EOS;
          UtlStripBlanks( szFileName );
          strcpy( szSysPath, EqfSystemPath );
          strcat( szSysPath, BACKSLASH_STR );
          strcat( szSysPath, MSGDIR );
          strcat( szSysPath, BACKSLASH_STR );
          strcat( szSysPath, szFileName );
          if ( *pszCurLine == COMMA ) pszCurLine++;
          fOK = UtlFileExist(szSysPath );
        } /* endif */


        /************************************************************/
        /* now extract resource file name                           */
        /************************************************************/
        if ( fOK )
        {
          pszTarget = szFileName;
          while ( (*pszCurLine != LF ) &&
                  (*pszCurLine != EOS) &&
                  (*pszCurLine != COMMA) )
          {
            *pszTarget++ = *pszCurLine++;
          } /* endwhile */
          *pszTarget = EOS;
          UtlStripBlanks( szFileName );
          strcpy( szSysPath, EqfSystemPath );
          strcat( szSysPath, BACKSLASH_STR );
          strcat( szSysPath, WINDIR );
          strcat( szSysPath, BACKSLASH_STR );
          strcat( szSysPath, szFileName );
          if ( *pszCurLine == COMMA ) pszCurLine++;
          fOK = UtlFileExist(szSysPath );
        } /* endif */

        /**************************************************************/
        /* Insert language menu item                                  */
        /**************************************************************/
        if ( fOK )
        {
          BOOL fChecked = _stricmp( szLanguage, pszCurLanguage ) == 0;
          InsertMenu( hMenu, sDummy,
                        (( fChecked ) ? MF_CHECKED : MF_UNCHECKED ) |
                        MF_BYCOMMAND,
                        sID++, szLanguage );
          sItems++;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* skip current line                                            */
      /****************************************************************/
      while ( (*pszCurLine != LF ) &&   (*pszCurLine != EOS) )
      {
        pszCurLine++;
      } /* endwhile */
      if ( *pszCurLine == LF )
      {
        pszCurLine++;
      } /* endif */
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Remove dummy menu item                                           */
  /********************************************************************/
  DeleteMenu( hMenu, sDummy, MF_BYCOMMAND );
  /********************************************************************/
  /* cleanup                                                          */
  /********************************************************************/
  if ( pszLangFileBuffer ) UtlAlloc( (PVOID *)&pszLangFileBuffer, 0L, 0L, NOMSG );

  return( sItems );
} /* end of function UtlFillSysLangPopUp */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlEditorLangSupport                                     |
//+----------------------------------------------------------------------------+
//|Function call:     pFail =  UtlEditorLangSupport( pEditor, pSrcLng, pTgtLng |
//+----------------------------------------------------------------------------+
//|Description:       check if the passed editor will support the selected     |
//|                   source and target language                               |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ   pEditor  -- ptr to editor                          |
//|                   PSZ   pSrcLng  -- ptr to source language                 |
//|                   PSZ   pTgtLng  -- ptr to target language                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL  if everything ok                                   |
//|                   PSZ   ptr to language not supported by Editor            |
//+----------------------------------------------------------------------------+
//|Function flow:     if specified Editor == STANDARD                          |
//|                     -- if srclang or tgtlang == THAI -- indicate not supp. |
//+----------------------------------------------------------------------------+
PSZ UtlEditorLangSupport( PSZ pszEditor, PSZ pszSourceLang, PSZ pszTargetLang )
{
  PSZ pFail = NULL;
  pszEditor;
  pszSourceLang;
  pszTargetLang;
  /********************************************************************/
  /* Current restriction only on STANDARD editor -- more to follow... */
  /********************************************************************/
 // if ( strcmp( pszEditor, "STANDARD" ) == 0 )
 // {
 //   if ( MorphGetLanguageType( pszSourceLang ) == MORPH_THAI_LANGTYPE )
 //   {
 //     pFail = pszSourceLang;
 //   }
 //   else
 //   if ( MorphGetLanguageType( pszTargetLang ) == MORPH_THAI_LANGTYPE )
 //   {
 //     pFail = pszTargetLang;
 //   } /* endif */
 // }
 // else
 // {
 // } /* endif */

  return pFail;
}


////////////////////////////////////////////////////////////////
// Get Codepage appropriate for given language ( not OS-dependent!)
///////////////////////////////////////////////////////////////////
ULONG GetLangAnsiCP
(
 PSZ pLanguage
)
{
  ULONG   ulCP = 0L;
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();

  if (pLanguage)
  {
    ulCP = pLangFactory->getAnsiCP( pLanguage );
    ulCP = ADJUSTCP( ulCP );
  }

  if ( !ulCP)
  {
    PPROPSYSTEM pPropSys= NULL;
    PSZ         pSystemPrefLang = NULL;

    pPropSys = GetSystemPropPtr();
    ulCP = pLangFactory->getAnsiCP( pPropSys->szSystemPrefLang );
    ulCP = ADJUSTCP( ulCP );
  }

  if (!ulCP)
  {
    ulCP = (ULONG)GetCodePage( ANSI_CP );
  }
  return(ulCP);
}

// same as GetLangxxxCP but returns unadjusted codepage
ULONG GetOrgLangCP
(
 PSZ pLanguage,
 BOOL  fASCII
)
{
  ULONG   ulCP = 0L;
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();

  if (pLanguage)
  {
    ulCP = fASCII ? pLangFactory->getOEMCP( pLanguage ) : pLangFactory->getAnsiCP( pLanguage );
  }

  if ( !ulCP)
  {
    PPROPSYSTEM pPropSys= NULL;

    pPropSys = GetSystemPropPtr();
    ulCP = fASCII ? pLangFactory->getOEMCP( pPropSys->szSystemPrefLang ) : pLangFactory->getAnsiCP( pPropSys->szSystemPrefLang );
  }

  if (!ulCP)
  {
    ulCP = (ULONG)GetCodePage( fASCII ? OEM_CP : ANSI_CP );
  }
  return(ulCP);
}

////////////////////////////////////////////////////////////////
// Get Codepage appropriate for given language ( not OS-dependent!)
///////////////////////////////////////////////////////////////////
typedef struct LANGUAGE_CACHE
{
  CHAR        szLanguage[MAX_LANG_LENGTH];
  ULONG       ulCodePage;
  unsigned int uiCounter;
} LANGUAGE_CACHE, *PLANGUAGE_CACHE;

#define MAX_LANGCACHE 10

static LANGUAGE_CACHE LangCache[MAX_LANGCACHE];
static int iLangCacheUsed = 0;                   // number of used entries in laguage cache
static unsigned int uiLangCacheCounter = 1;      // update counter for language cache entries
static char szChachedLang1[MAX_LANG_LENGTH] = "";
static char szChachedLang2[MAX_LANG_LENGTH] = "";
static ULONG ulCachedCP1 = 0;
static ULONG ulCachedCP2 = 0;

static ULONG InternalGetLangOEMCP
(
 PSZ pszLanguage
)
{
  BOOL        fFound = FALSE;                    // true if language found in cache
  CHAR        szUprLanguage[MAX_LANG_LENGTH];    // uppercased language name
  int         i = 0;                             // index for language cache table
  ULONG       ulCP = 0L;                         // evaluated OEM codepage
  int         iCachePos = -1;                    // position of language within cache, -1 = not in cache

  // first check our static buffers (in most cases only two languages are needed
  if ( szChachedLang1[0] && (strcmp( szChachedLang1, pszLanguage) == 0 ) )
  {
    return( ulCachedCP1 );
  }
  else if ( szChachedLang2[0] && (strcmp( szChachedLang2, pszLanguage) == 0 ) )
  {
    return( ulCachedCP2 );
  } /* endif */

  // check if language is already in our cache
  strcpy( szUprLanguage, pszLanguage );
  _strupr( szUprLanguage );
  while ( (i < iLangCacheUsed) && !fFound )
  {
    if ( strcmp( szUprLanguage, LangCache[i].szLanguage ) == 0 )
    {
      iCachePos = i;
      fFound = TRUE;
      ulCP = LangCache[i].ulCodePage;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */

  // if not contained in cache, get codepage from the language factory
  if ( !fFound )
  {
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    ulCP = pLangFactory->getOEMCP( pszLanguage );

    if ( ulCP != 0 )
    {
      int iAddPos = -1;

      ulCP = ADJUSTCP( ulCP );

      // add to cache if codepage has been retrieved
      if ( ulCP )
      {
        if ( iLangCacheUsed < MAX_LANGCACHE)
        {
          // use free slot
          iCachePos = iAddPos = iLangCacheUsed;
          iLangCacheUsed++;
        }
        else
        {
          int j = 0;

          // find latest used slot
          iAddPos = 0;
          for( j = 0; j < iLangCacheUsed; j++ )
          {
            if ( LangCache[j].uiCounter < LangCache[iAddPos].uiCounter)
            {
              iAddPos = j;
            } /* endif */
          } /* endfor */
        } /* endif */

        // add at found slot
        strcpy( LangCache[iAddPos].szLanguage, szUprLanguage );
        LangCache[iAddPos].ulCodePage = ulCP;
        iCachePos = iAddPos;

        // fill static buffers as well
        if ( !szChachedLang1[0]  )
        {
          strcpy( szChachedLang1, pszLanguage );
          ulCachedCP1 = ulCP;
        }
        else if ( !szChachedLang2[0]  )
        {
          strcpy( szChachedLang2, pszLanguage );
          ulCachedCP2 = ulCP;
        } /* endif */

      } /* endif */
    } /* endif */
  } /* endif */

  // update cache update counter
  if ( iCachePos >= 0 )
  {
    uiLangCacheCounter++;
    if ( !uiLangCacheCounter )
    {
      // overflow in language cache counter, restart at 1
      int j= 0;
      for ( j= 0; j < iLangCacheUsed; j++ )
      {
        LangCache[j].uiCounter = 0;
      } /* endfor */
      uiLangCacheCounter = 1;
    } /* endif */
    LangCache[iCachePos].uiCounter = uiLangCacheCounter;
  } /* endif */

  return( ulCP );
} /* end of function InternalGetLangOEMCP */

ULONG GetLangOEMCP
(
 PSZ pLanguage
)
{
  ULONG  ulCP = 0L;
  if (pLanguage)
  {
    ulCP = InternalGetLangOEMCP( pLanguage );
  }

  if (!ulCP)
  {
    PPROPSYSTEM pPropSys= NULL;
    PSZ         pSystemPrefLang = NULL;

    pPropSys = GetSystemPropPtr();
    pSystemPrefLang = &pPropSys->szSystemPrefLang[0];         // must be filled!!
    ulCP = pPropSys->ulSystemPrefCP;
    if (!ulCP)
    {
      pPropSys->ulSystemPrefCP = ulCP = InternalGetLangOEMCP( pSystemPrefLang );
    }
  }
  if (!ulCP)
  {
    ulCP = (ULONG)GetCodePage( OEM_CP );
  }
  return(ulCP);
}


// Set System Language or return an error
USHORT SetSystemLanguage( PSZ pLanguage )
{
    USHORT      usRC = 0;
    PPROPSYSTEM pPropSys = GetSystemPropPtr();

    // check if this is a valid language ...
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    ULONG ulCP = pLangFactory->getOEMCP( pLanguage );

    //    if so store it and save it to the system properties
    if ( ulCP != 0 )
    {
      // before we can use this language we have to check if the language s supported
      // by the operating system (i.e. the approbriate support has been installed)
      if ( UtlIsLanguageSupported( pLanguage, TRUE, HWND_FUNCIF ) )
      {
        HPROP hPropSys = EqfQuerySystemPropHnd();

        pPropSys->ulSystemPrefCP = ulCP;
        pPropSys->ulSystemPrefCP = ADJUSTCP( pPropSys->ulSystemPrefCP );
        strcpy( pPropSys->szSystemPrefLang, pLanguage );

        if( SetPropAccess( hPropSys, PROP_ACCESS_WRITE) )
        {
          ULONG ulErrorInfo = 0;
          SaveProperties( hPropSys, &ulErrorInfo );
          ResetPropAccess( hPropSys, PROP_ACCESS_WRITE);
        } /* endif */
      }
      else
      {
        usRC = ERROR_LANG_NOTSUPPORTED;
      } /* endif */
    }
    else
    {
      PSZ   pReplAddr[2];      // Pointer to an address list of replacement strings

      usRC = ERROR_PROPERTY_LANG_DATA;

      pReplAddr[0] = pLanguage;
      pReplAddr[1] = "";
      UtlError( usRC, MB_CANCEL, 1, &pReplAddr[0], EQF_ERROR );

    }
    return usRC;
}

// Get the currently set codepage.
PSZ GetSystemLanguage( VOID )
{
    PPROPSYSTEM pPropSys = GetSystemPropPtr();

    return &pPropSys->szSystemPrefLang[0];
}

// check if given language name is valid
BOOL isValidLanguage( PSZ pszLanguage, BOOL fAdjustLangName )
{
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();
  return( pLangFactory->isValidLanguage( pszLanguage, fAdjustLangName ) );
}

// get the group a given language belongs to
BOOL GetLanguageGroup( PSZ pszLanguage, PSZ pszGroup )
{
  BOOL fOK = TRUE;
  LanguageFactory::LANGUAGEINFO LangInfo;
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();

  fOK = pLangFactory->getLanguageInfo( pszLanguage, &LangInfo );
  if ( fOK ) strcpy( pszGroup, LangInfo.szLangGroup );
  return( fOK );
}

