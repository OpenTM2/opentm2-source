//-----------------------------------------------------------------------------+
//|EQFDDED.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016 International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:   R.Jornitz, QSoft Quality Software GmbH                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:   Document  Load and Unload for DDE                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_FOLDER           // folder list and document list functions

#include <eqf.h>                  // General Translation Manager include file

#include "eqfdde.h"               // for the DDE I/F
#include "otmfunc.h"              // API definitions
  #include "EQFHLOG.H"            // defines for history log processing

#define MAX_DDE_POOL     1000           // size of buffer
#define DDEREALLOCSIZE   8096          // re-allocation size for file buffer


USHORT DDEScanDir
(
  PSZ    pszStartDir,                  // pointer to start directory
  PSZ    pszPattern,                   // search pattern
  SHORT  sStartPathLen,                // length of original start path
  PSZ    *ppszFileBuffer,              // ptr to file list buffer pointer
  PULONG pulFileBufferSize,            // ptr to file buffer size variable
  PULONG pulFileBufferUsed,            // ptr to file buffer used variable,
  BOOL   fNonRecursive                 // true = non-recursive; i.e. do not scan sub-directories
);
USHORT DDECopyToTarget
(
  PDOCIMPEXP  pDocExpIda,              // pointer to ida
  PSZ         pszTarget,               // file to be exported
  PSZ         pszFinalTarget,          // final target of file
  PSZ         pActDocName              // ptr to actual dcument name

);

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEDocumentLoad                                          |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = DocLoad( pDocImpIda );                             |
//+----------------------------------------------------------------------------+
//|Description:       This function tries to load the specified document       |
//|                   into the specified folder                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PDOCIMPEXP  pDocImpIda       document import ida         |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     load of document okay                           |
//|                   FALSE    document could not be loaded                    |
//+----------------------------------------------------------------------------+
//|Function flow:     resolve single/multiple substitution                     |
//|                   loop through list of files or single file and do the     |
//|                     document load                                          |
//|                   return success indication                                |
//+----------------------------------------------------------------------------+

BOOL DDEDocLoad
(
  PDOCIMPEXP  pDocImpIda               // document import ida
)
{
  BOOL         fOK = TRUE;                      // error flag
  PSZ          pInFile;                         // input file name
  PSZ          pFileList = NULL;                 // pointer to file list
  PSZ          pFile;
  CHAR         szOutBuf[ MAX_FULLPATH ];
  PDDERETURN   pDDEReturn = &pDocImpIda->DDEReturn;


  pInFile = *(pDocImpIda->ppFileArray + pDocImpIda->usActFile);
  /*******************************************************************/
  /* extract filename if it is fully qualified...                    */
  /*******************************************************************/
  pFile = UtlGetFnameFromPath( pInFile );

  // fully qualify file name if it has no path information an no
  // startpath has been specified
  if ( !pFile && (pDocImpIda->szStartPath[0] == EOS) )
  {
    fOK = UtlInsertCurDir(pInFile, &(pDocImpIda->ppCurDirArray),szOutBuf);
    if ( fOK )
    {
      pInFile = szOutBuf;
    }
    else
    {
      fOK = FALSE;                               // name is not valid
      pDDEReturn->usRc = UTL_PARAM_TOO_LONG;
      UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1,
                    &pInFile, EQF_ERROR, pDocImpIda->hwndErrMsg );
    } /* endif */
  } /* endif */
  if ( fOK )
  {
    fOK = ResolveMultFileNames( pInFile, pDocImpIda->szStartPath, &pFileList );
  } /* endif */

  if ( fOK )
  {
    if ( pFileList )
    {
      pInFile = pFileList;
      while ( fOK && *pInFile )
      {
        fOK = DDEDocLoadWork( pDocImpIda, pInFile );
        pInFile += strlen(pInFile) + 1;
        /**************************************************************/
        /* let other processes work                                   */
        /**************************************************************/
        UtlDispatch();
      } /* endwhile */
      /****************************************************************/
      /* free resource ...                                            */
      /****************************************************************/
      UtlAlloc( (PVOID *) &pFileList, 0L, 0L, NOMSG );
    }
    else
    {
      fOK = DDEDocLoadWork( pDocImpIda, pInFile );
    } /* endif */
  } /* endif */

  return fOK;

}/* end DDEDocLoad */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEDocLoadWork                                           |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = DDEDocLoadWork( pDocImpIda, pInFile );             |
//+----------------------------------------------------------------------------+
//|Description:       This function tries to load the specified document       |
//|                   into the specified folder                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PDOCIMPEXP  pDocImpIda       document import ida         |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     load of document okay                           |
//|                   FALSE    document could not be loaded                    |
//+----------------------------------------------------------------------------+
//|Function flow:     try to open the folder properties                        |
//|                   if not okay then                                         |
//|                     display message that folder does not exist             |
//|                   else                                                     |
//|                     get drive identifier of folder                         |
//|                   endif                                                    |
//|                   if okay so far then                                      |
//|                     delete the document in the folder (just in case)       |
//|                     create the properties                                  |
//|                     if okay load in the document, else displ. message      |
//|                   endif                                                    |
//|                   return fOk                                               |
//+----------------------------------------------------------------------------+
BOOL DDEDocLoadWork
(
  PDOCIMPEXP  pDocImpIda,              // document import ida
  PSZ         pInFile                  // pointer to active file
)
{
   HPROP        hpropRc;                         // return from create props
   HPROP        hpropFolder;                     // return from folder props
   PPROPFOLDER  pProp;                           // pointer to folder props
   USHORT       usDosRc;                         // Return code from Dos(Utl)
   ULONG        ulErrorInfo;                     // error indicator from PRHA
   PSZ          pszReplace;                      // pointer to replace string
   BOOL         fOK = TRUE;                      // error flag
   CHAR         szBuf[ MAX_FULLPATH ];
   CHAR         szInFile[ MAX_FULLPATH ];        // contains normalized infile
   CHAR         szFolder[ MAX_EQF_PATH ];
   CHAR         szFolderBuf[ MAX_EQF_PATH ];
   CHAR         szSourceFile[ MAX_EQF_PATH ];
   PSZ          pFile;
   PDDERETURN   pDDEReturn = &pDocImpIda->DDEReturn;
   CHAR         szOutBuf[ MAX_FULLPATH ];
   BOOL         isLongFileName = FALSE;
   CHAR         pGet[ MAX_FULLPATH ];            // internal use
   CHAR         LongNameSave[ MAX_FULLPATH ];    // Save Long Filename
                                                 // for properties
   PSZ          pszCopyFile = NULL;              // Buffer for pInFile
   BOOL         fIsNew;                          // is shortversion new?
   USHORT       usRC = NO_ERROR;                 // function return code
   PSZ          pszBuffer = NULL;                // allocated buffer for file names
   ULONG        ulParentFolder = 0;              // ID of document parent folder
   CHAR         szShipment[MAX_DESCRIPTION];     // shipment string
   BOOL         fDocExists = FALSE;              // TRUE = document exists

   // initialize shipment string
   szShipment[0] = EOS;
   LongNameSave[0] = 0;

   // copy pInFile
   fOK=UtlAlloc((PVOID *) &pszBuffer,0L,
                (LONG) MAX_LONGPATH,ERROR_STORAGE);
   if (fOK)
   {
      pszCopyFile = pszBuffer;
      strcpy(pszCopyFile,pInFile);
   }

   /****************************************************************/
   /* if file is not fully qualified -- qualify it with current    */
   /* directory of requesting application (DDE Client)             */
   /* (only if not in relative path mode)                          */
   /****************************************************************/
   pFile = UtlGetFnameFromPath( pszCopyFile );
   if ( pFile )
   {
     strcpy(LongNameSave,pFile);
   }
   else
   {
     strcpy(LongNameSave,pszCopyFile);
   } /* endif */
   if ( (pDocImpIda->szStartPath[0] == EOS) &&
        !pFile && (pDocImpIda->ppCurDirArray != NULL) )
   {
    fOK = UtlInsertCurDir(pszCopyFile, &(pDocImpIda->ppCurDirArray), szOutBuf);
    if ( fOK )
    {
      pszCopyFile = szOutBuf;
      pFile = UtlGetFnameFromPath( pszCopyFile );
      if (!pFile )
      {
        pFile = pszCopyFile;
      } /* endif */
    } /* endif */
   } /* endif */

   if (!pFile )
   {
     pFile = pszCopyFile;
   } /* endif */

   // prefix file name with start path when in relative path mode
   if ( pDocImpIda->szStartPath[0] != EOS )
   {
     strcpy( szOutBuf, pDocImpIda->szStartPath );
     if ( szOutBuf[strlen(szOutBuf)-1] != BACKSLASH )
     {
       strcat( szOutBuf, BACKSLASH_STR );
     } /* endif */
     pFile = szOutBuf + strlen(szOutBuf);
     strcat( szOutBuf, pszCopyFile );
     strcpy(LongNameSave,pFile);
     pszCopyFile = szOutBuf;
   } /* endif */

   if ( fOK && (strlen( pszCopyFile ) < sizeof( szInFile ) ))
   {
     strcpy( szInFile, pszCopyFile );
   }
   else
   {
     /************************************************************/
     /* passed name is not valid                                 */
     /************************************************************/
     fOK = FALSE;
     usRC = UTL_PARAM_TOO_LONG;
     UtlErrorHwnd( usRC, MB_CANCEL, 1,
                   &pszCopyFile, EQF_ERROR, pDocImpIda->hwndErrMsg );
   } /* endif */

   // check folder existence and get folder short name
   if ( fOK )
   {
       BOOL fIsNew;

       // get any subfolder ID and split subfolder part from folder name
       fIsNew = !SubFolNameToObjectName( pDocImpIda->chFldName, pDocImpIda->szBuffer );
       if ( !fIsNew )
       {
         if ( FolIsSubFolderObject( pDocImpIda->szBuffer ) )
         {
           PSZ pszDelimiter;
           ulParentFolder = FolGetSubFolderIdFromObjName( pDocImpIda->szBuffer );
           strcpy( pDocImpIda->szBuffer, pDocImpIda->chFldName );
           pszDelimiter = strchr( pDocImpIda->szBuffer, BACKSLASH );
           if ( pszDelimiter ) *pszDelimiter = EOS;
         }
         else
         {
           ulParentFolder = 0L;
           strcpy( pDocImpIda->szBuffer, pDocImpIda->chFldName );
         } /* endif */
       } /* endif */

       if ( !fIsNew )
       {
         ObjLongToShortName( pDocImpIda->szBuffer, szFolder, FOLDER_OBJECT, &fIsNew );
       } /* endif */

       if ( fIsNew )
       {
         PSZ pszParm = pDocImpIda->chFldName;
         fOK = FALSE;
         usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
         UtlErrorHwnd( usRC, MB_CANCEL, 1,
                       &pszParm, EQF_ERROR, pDocImpIda->hwndErrMsg );
       } /* endif */
   } /* endif */


   if ( fOK )
   {
     // create full path of folder
     strcat( szFolder, EXT_FOLDER_MAIN );
     UtlMakeEQFPath( szFolderBuf, NULC, SYSTEM_PATH, szFolder );

     /*******************************************************************/
     /* open folder properties to find folder path, but add EXT first   */
     /*******************************************************************/
     if( (hpropFolder = OpenProperties( szFolderBuf, NULL,
                                   PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
     {
        fOK = FALSE;
        // display error message if not already displayed
        if ( ulErrorInfo != Err_NoStorage )
        {
         PSZ pszParm = pDocImpIda->chFldName;
           usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
           UtlErrorHwnd( usRC, MB_CANCEL, 1,
                         &pszParm, EQF_ERROR, pDocImpIda->hwndErrMsg );
        } /* endif */
     }
     else
     {
       pProp = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
       szFolderBuf[0] = pProp->chDrive;

       // remember folder shipment
       strcpy( szShipment, pProp->szShipment );

       CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
     }/*endif*/
   }/*endif*/

   if ( fOK )
   {
     // get Short version of Filename if pFile is Long Filename
     isLongFileName = UtlIsLongFileName(pFile);
     if (isLongFileName)
     {
       FolLongToShortDocName( szFolderBuf,
                              pFile,
                              pGet,
                              &fIsNew);
       strcpy(pFile,pGet);
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* check if document to be imported exists and is accessible ..    */
   /*******************************************************************/
   if ( fOK )
   {
     if ( !UtlFileExistLong( szInFile ) )
     {
       PSZ pszParm;
       fOK = FALSE;
       usRC = FILE_NOT_EXISTS;
       pszParm = LongNameSave;
       UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR,
                     pDocImpIda->hwndErrMsg );
     } /* endif */
   } /* endif */
   /*******************************************************************/
   /* check if we should delete the document first, i.e. if user wants*/
   /* that the document is deleted if it exists                       */
   /* else display an appropriate error message if document already   */
   /*      exist                                                      */
   /*******************************************************************/
   if ( fOK )
   {
     UtlMakeEQFPath( szBuf, szFolderBuf[0],
                     DIRSOURCEDOC_PATH, szFolder );
     sprintf( szSourceFile, "%s\\%s", szBuf, pFile );
     if ( UtlFileExist( szSourceFile ) )
     {
       fDocExists = TRUE;
       if ( pDocImpIda->fOverwrite )                              /* KIT1288M */
       {
         USHORT usDummy = 0;
         UtlMakeFullPath( szBuf, (PSZ)NULP, szFolderBuf, pFile, (PSZ)NULP );
         DocumentDelete(szBuf, FALSE, &usDummy );
       }
       else
       {
         fOK = FALSE;
         usRC = DDE_FILE_NO_OVERWRITE;
         UtlErrorHwnd( usRC, MB_CANCEL, 1,
                       &pFile, EQF_ERROR, pDocImpIda->hwndErrMsg );
       } /* endif */
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* create document properties                                      */
   /*******************************************************************/
   if ( fOK )
   {
     UtlMakeFullPath( szBuf, (PSZ)NULP, szFolderBuf, pFile, (PSZ)NULP );

     hpropRc = CreateProperties( szBuf, NULL,
                                 PROP_CLASS_DOCUMENT, &ulErrorInfo );

     if ( !hpropRc )               //error from property handler
     {
       //display message create property fails
       fOK = FALSE;
       pszReplace = szBuf;
       usRC = ERROR_CREATE_PROP;
       UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszReplace, EQF_ERROR, pDocImpIda->hwndErrMsg );
     }
     else
     {

        /***********************************/
        /* check if long filename          */
        /* if yes write to property list   */
        /***********************************/
        if (isLongFileName){

             PPROPDOCUMENT pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropRc );
             strcpy(pProp->szLongName, LongNameSave);

        }
        /**************************************************************/
        /* Update properties with values specified                    */
        /**************************************************************/
        {
          PPROPDOCUMENT pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropRc );

          strcpy( pProp->szFormat,     pDocImpIda->szFormat );
          strcpy( pProp->szMemory,     pDocImpIda->szMem );
          strcpy( pProp->szSourceLang, pDocImpIda->szSourceLang );
          strcpy( pProp->szTargetLang, pDocImpIda->szTargetLang );
          strcpy( pProp->szEditor,     pDocImpIda->szEdit );
          strcpy( pProp->szAlias,      pDocImpIda->szAlias );
          strcpy( pProp->szConversion, pDocImpIda->szConversion );

          // set document shipment number
          strcpy( pProp->szShipment, szShipment );

          // set document imported stamp
          UtlTime( (PLONG)&pProp->ulImp );

          // store parent folder ID
          pProp->ulParentFolder = ulParentFolder;
                    //
          // set document source stamp
          // added bt 19062001
          //

        {
          ULONG ulTime;

          ulTime = UtlFileTimeToLong(szInFile);
          if (ulTime)
          {
            pProp->ulSrc = ulTime;
          } else
          {
            pProp->ulSrc = 0L;
            fOK = FALSE;
          }
        }

          SaveProperties( hpropRc, &ulErrorInfo);
        }

        CloseProperties( hpropRc, PROP_FILE, &ulErrorInfo );
        //call UtlCopy to copy source file to target folder
        UtlMakeEQFPath( szBuf, szFolderBuf[0],
                        DIRSOURCEDOC_PATH, szFolder );
        sprintf( szSourceFile, "%s\\%s", szBuf, pFile );
        /**************************************************************/
        /* copy the input file into the source file directory ..      */
        /**************************************************************/
        usDosRc = UtlCopyHwnd( szInFile, szSourceFile, 1, 0L, TRUE, pDocImpIda->hwndErrMsg );
        if ( usDosRc )
        {
           // delete properties since copy failed
           fOK = FALSE;
           DeleteProperties( szSourceFile, NULL, &ulErrorInfo );
           usRC = ERROR_GENERAL_DOS_ERROR_MSG;
        }
        else
        {
           /**********************************************************/
           /* reset any read-only flags of loaded document           */
           /**********************************************************/
           usRC = UtlSetFileModeHwnd( szSourceFile, FILE_NORMAL, 0L, TRUE, pDocImpIda->hwndErrMsg );
           /*******************************************************************/
           /* build object name and copy it into pszCopyFile -- space large enough*/
           /* copy folder object name, too.                                   */
           /*******************************************************************/
           UtlMakeFullPath( szBuf, (PSZ)NULP, szFolderBuf, pFile, (PSZ)NULP );
           //inform folderhandler to update documentlist
           if ( pDocImpIda->hwndErrMsg == HWND_FUNCIF )
           {
             ObjBroadcast( WM_EQFN_CREATED, clsDOCUMENT, szBuf );
           }
           else
           {
             EqfSend2AllHandlers ( WM_EQFN_CREATED, MP1FROMSHORT( clsDOCUMENT ), MP2FROMP( szBuf ));
           } /* endif */

           // write import record to history log 
           { 
              PDOCIMPORTHIST2 pDocImpHist;  // history record for document import

              fOK = UtlAllocHwnd( (PVOID *)&pDocImpHist, 0L, (ULONG)sizeof(DOCIMPORTHIST2), ERROR_STORAGE, pDocImpIda->hwndErrMsg );

              if ( fOK )
              {
                pDocImpHist->sType               = EXTERN_SUBTYPE;
                strcpy( pDocImpHist->szPath,       pDocImpIda->szStartPath );
                strcpy( pDocImpHist->szFileName,   szInFile );
                strcpy( pDocImpHist->szMarkup,     pDocImpIda->szFormat );
                strcpy( pDocImpHist->szMemory,     pDocImpIda->szMem );
                strcpy( pDocImpHist->szSourceLang, pDocImpIda->szSourceLang );
                strcpy( pDocImpHist->szTargetLang, pDocImpIda->szTargetLang );
                pDocImpHist->fSourceDocReplaced =  fDocExists;
                pDocImpHist->fTargetDocReplaced =  FALSE;
                strcpy( pDocImpHist->szShipment,   szShipment );

                EQFBWriteHistLog2( szFolderBuf, pFile, DOCIMPORT_LOGTASK2, sizeof(DOCIMPORTHIST2), 
                                   (PVOID)pDocImpHist, TRUE, pDocImpIda->hwndErrMsg, LongNameSave );

                UtlAlloc( (PVOID *)&pDocImpHist, 0L, 0L, NOMSG );
             } /* endif */
           }

        }/*end if*/
     }/*end if*/
   } /* endif */

   if ( pDDEReturn )
   {
     pDDEReturn->usRc = usRC;
   } /* endif */

   UtlAlloc((PVOID *) &pszBuffer,0L, 0L, NOMSG);

   return fOK;

}/* End DDEDocLoadWork */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEDocUnLoad                                             |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = DDEDocUnLoad( pDocExpIda );                        |
//+----------------------------------------------------------------------------+
//|Description:       This function will export the specified document         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDOCIMPEXP  pDocExpIda                  pointer to ida   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     everything okay                                 |
//|                   FALSE    something went wrong                            |
//+----------------------------------------------------------------------------+
//|Function flow:     extract filename if it is fully qualified                |
//|                   create folder name                                       |
//|                   resolve any multiple substitions (if necessary)          |
//|                   call DDEDocUnLoadWork on the single file or the list     |
//|                     of files                                               |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+

BOOL  DDEDocUnLoad
(
  PDOCIMPEXP  pDocExpIda               // pointer to ida
)
{
  BOOL fOK = TRUE;                     // success indicator
  CHAR         szBuf[ MAX_FULLPATH ];
  CHAR         szFolderBuf[ MAX_EQF_PATH ];
  PSZ          pExpFile;               // pointer to export file name (full)
  PSZ          pTempFile;
  PSZ          pFile;                  // pointer to export file name
  PSZ          pFileList = NULL;       // pointer to file list
  HPROP        hpropFolder;            // return from folder props
  PPROPFOLDER  pProp;                  // pointer to folder props
  ULONG        ulErrorInfo;            // error indicator from PRHA
  PDDERETURN   pDDEReturn = &pDocExpIda->DDEReturn;
  CHAR         szOutBuf[ MAX_FULLPATH ];
  CHAR         szFolder[MAX_FILESPEC]; // buffer for folder short name

  pExpFile = *(pDocExpIda->ppFileArray + pDocExpIda->usActFile);

  /*******************************************************************/
  /* extract filename if it is fully qualified...                    */
  /*******************************************************************/
  pFile = UtlGetFnameFromPath( pExpFile );
  if ( !pFile && (pDocExpIda->szStartPath[0] == EOS) )
  {
    fOK = UtlInsertCurDir(pExpFile, &(pDocExpIda->ppCurDirArray),szOutBuf);
    if ( fOK )
    {
      pExpFile = szOutBuf;
      pFile = UtlGetFnameFromPath( pExpFile );
      if (!pFile )
      {
        pFile = pExpFile;
      } /* endif */
    }
    else
    {
      fOK = FALSE;                               // name is not valid
      pDDEReturn->usRc = UTL_PARAM_TOO_LONG;
      UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1,
                    &pExpFile, EQF_ERROR, pDocExpIda->hwndErrMsg );
    } /* endif */
  } /* endif */

  //  check folder existence and get folder short name
  if ( fOK )
  {
     BOOL fIsNew;
     ObjLongToShortName( pDocExpIda->chFldName, szFolder, FOLDER_OBJECT, &fIsNew );
     if ( fIsNew )
     {
        PSZ pszParm = pDocExpIda->chFldName;                 // set folder name
        pDDEReturn->usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, pDocExpIda->hwndErrMsg );
     } /* endif */
  } /* endif */

  if ( fOK )
  {
    /*******************************************************************/
    /* create folder name ...                                          */
    /* open folder properties to find folder path ...                  */
    /*******************************************************************/
    UtlMakeEQFPath( szFolderBuf, NULC, SYSTEM_PATH, szFolder );
    strcat( szFolderBuf, EXT_FOLDER_MAIN );

    if( (hpropFolder = OpenProperties( szFolderBuf, NULL,
                                  PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
    {
       fOK = FALSE;
       // display error message if not already displayed
       if ( ulErrorInfo != Err_NoStorage )
       {
          PSZ pszParm = pDocExpIda->chFldName;                 // set folder name
          pDDEReturn->usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
          UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1,
                        &pszParm, EQF_ERROR, pDocExpIda->hwndErrMsg );
       } /* endif */
    }
    else
    {
      pProp = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
      szFolderBuf[0] = pProp->chDrive;
      CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
      strcat( szFolder, EXT_FOLDER_MAIN );
    }/*endif*/
  } /* endif */

  /********************************************************************/
  /* construct filename depending on requested file type and export   */
  /* the file.                                                        */
  /* Exporting is either copying the file or using the EQFUnSeg       */
  /* utility.                                                         */
  /********************************************************************/
  if ( fOK )
  {
    if ( pDocExpIda->fSource )
    {
      UtlMakeEQFPath( szBuf, szFolderBuf[0],
                      DIRSOURCEDOC_PATH, szFolder );
    }
    else
    if ( pDocExpIda->fSNOMATCH )
    {
      UtlMakeEQFPath( szBuf, szFolderBuf[0],
                      DIRSEGNOMATCH_PATH, szFolder );
    }
    else
    {
      UtlMakeEQFPath( szBuf, szFolderBuf[0],
                      DIRSEGTARGETDOC_PATH, szFolder );
    } /* endif */
    strcat( szBuf, BACKSLASH_STR );
    if ( pFile == NULL )
    {
      strcat( szBuf, pExpFile );
    }
    else
    {
      strcat( szBuf, pFile );
    } /* endif */

    if ( pDocExpIda->szStartPath[0] == EOS )
    {
      fOK = ResolveLongMultFileNames( szBuf,
                                      UtlGetFnameFromPath(szBuf),
                                      &pFileList, 0L );
    }
    else
    {
      fOK = ResolveLongMultFileNames( szBuf,
                                      pExpFile,
                                      &pFileList, 0L );
    } /* endif */
    if (!fOK)
    {
       PSZ apszErrParm[2];

       apszErrParm[0] = pFileList;
       apszErrParm[1] = pDocExpIda->chFldName;
       pDDEReturn->usRc = DDE_DOC_NOT_IN_FOLDR;
       UtlErrorHwnd( DDE_DOC_NOT_IN_FOLDR, MB_CANCEL, 2, apszErrParm,
                    EQF_ERROR, pDocExpIda->hwndErrMsg );
    }/*endif*/
  } /* endif */

  if ( fOK )
  {
    /****************************************************************/
    /* if file is not fully qualified -- qualify it with current    */
    /* directory of requesting application (DDE Client)             */
    /****************************************************************/
    pFile = UtlGetFnameFromPath( pExpFile );
    if ( pDocExpIda->szStartPath[0] != EOS)
    {
      // use start path to fully qualifiy file name
      if ( (strlen(pExpFile) + strlen(pDocExpIda->szStartPath)) < sizeof( szBuf ) )
      {
        strcpy( szBuf, pDocExpIda->szStartPath );
        if ( szBuf[strlen(szBuf)-1] != BACKSLASH )
        {
          strcat( szBuf, BACKSLASH_STR );
        } /* endif */
        strcat( szBuf, pExpFile );
      }
      else
      {
        // passed name is not valid
        fOK = FALSE;
        pDDEReturn->usRc = UTL_PARAM_TOO_LONG;
        UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1,
                      &pExpFile, EQF_ERROR, pDocExpIda->hwndErrMsg );
      } /* endif */
    }
    else if ( !pFile )
    {
      /****************************************************************/
      /* should not happen because pExpFile was already checked above */
      /****************************************************************/
      fOK = FALSE;                            //passed name is not valid
      pDDEReturn->usRc = UTL_PARAM_TOO_LONG;
      UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1,
                    &pExpFile, EQF_ERROR, pDocExpIda->hwndErrMsg );
    }
    else
    {
      if ( strlen( pExpFile ) < sizeof( szBuf ) )
      {
        strcpy( szBuf, pExpFile );
      }
      else
      {
        /************************************************************/
        /* passed name is not valid                                 */
        /************************************************************/
        fOK = FALSE;
        pDDEReturn->usRc = UTL_PARAM_TOO_LONG;
        UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1,
                      &pExpFile, EQF_ERROR, pDocExpIda->hwndErrMsg );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    /******************************************************************/
    /* pFile now points to start of file name in szBuf...             */
    /******************************************************************/
    pFile = UtlGetFnameFromPath( szBuf );

    if ( pFileList )
    {
      pExpFile = pFileList;
      while ( fOK && *pExpFile )
      {
        /**************************************************************/
        /* tempfile always set because files of the folder are fully  */
        /* qualified                                                  */
        /**************************************************************/

        // skip EQF directory, folder directory and document directory
        // do not use UtlGetFnameFromPath as this will skip the relative
        // path part of the document name
        pTempFile = strchr( pExpFile, BACKSLASH );                       // \EQF\FOLDER.F00\STARGET\file
        if ( pTempFile ) pTempFile = strchr( pTempFile + 1, BACKSLASH ); // \FOLDER.F00\STARGET\file
        if ( pTempFile ) pTempFile = strchr( pTempFile + 1, BACKSLASH ); // \STARGET\file
        if ( pTempFile ) pTempFile = strchr( pTempFile + 1, BACKSLASH ); // \file
        if ( pTempFile ) pTempFile++;

        strcpy( pFile, (pTempFile) ? pTempFile : pExpFile );

        fOK = DDEDocUnLoadWork( pDocExpIda, szFolderBuf, szBuf, pTempFile );
        pExpFile += strlen(pExpFile) + 1;

        /**************************************************************/
        /* let other processes work                                   */
        /**************************************************************/
        UtlDispatch();
      } /* endwhile */
      /****************************************************************/
      /* free resource                                                */
      /****************************************************************/
      UtlAlloc( (PVOID *) &pFileList, 0L, 0L, NOMSG );
    }
    else
    {
      fOK = DDEDocUnLoadWork( pDocExpIda, szFolderBuf, szBuf, pFile );
    } /* endif */
  } /* endif */

  return fOK;
} /* end of function DDEDocUnLoad */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEDocUnLoadWork                                         |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = DDEDocUnLoad( pDocExpIda );                        |
//+----------------------------------------------------------------------------+
//|Description:       This function will export the specified document         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDOCIMPEXP  pDocExpIda                  pointer to ida   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     everything okay                                 |
//|                   FALSE    something went wrong                            |
//+----------------------------------------------------------------------------+
//|Function flow:     extract filename if it is fully qualified                |
//|                   create folder name                                       |
//|                   if necessary check if output file already exist          |
//|                   if okay then                                             |
//|                     construct filename depending on requested file type    |
//|                     and export the file.                                   |
//|                     Exporting is either copying the file or using the      |
//|                     EQFUnSeg utility                                       |
//|                   endif                                                    |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
BOOL  DDEDocUnLoadWork
(
  PDOCIMPEXP  pDocExpIda,              // pointer to ida
  PSZ         pFolderObj,              // pointer to full folder name
  PSZ         pExpFile,                // file to be exported
  PSZ         pActDocName              // ptr to actual dcument name
)
{
  BOOL fOK = TRUE;                     // success indicator
  BOOL fGoOn = FALSE;                  // issue message not to go on
  CHAR         szBuf[ MAX_FULLPATH ];
  PSZ          pFile;                  // pointer to export file name
  PSZ          pFolder;                // pointer to folder name
  USHORT       usRC = NO_ERROR;
  PDDERETURN   pDDEReturn = &pDocExpIda->DDEReturn;
  CHAR         szOutBuf[ MAX_FULLPATH ];
  CHAR         szDocShortName[MAX_FILESPEC]; // buffer for document short name
  //static CHAR  szLongName[MAX_LONGFILESPEC]; // buffer for document long name

  pFolder = UtlGetFnameFromPath( pFolderObj );

  /*******************************************************************/
  /* extract filename if it is fully qualified...                    */
  /*******************************************************************/
  if ( pDocExpIda->szStartPath[0] == EOS )
  {
    if ( pDocExpIda->lOptions & WITHOUTRELATIVEPATH_OPT )
    {
       pFile = pExpFile + (strlen(pExpFile) - strlen(pActDocName));
    }
    else
    {
       pFile = UtlGetFnameFromPath( pExpFile );
    }
  }
  else
  {
    // only skip startpath if document name is really prefixed with one ...
    if ( strncmp( pDocExpIda->szStartPath, pExpFile, strlen(pDocExpIda->szStartPath) ) == 0 )
    {
      pFile = pExpFile + strlen(pDocExpIda->szStartPath);
      if ( *pFile == BACKSLASH )
      {
        pFile++;
      } /* endif */
    }
    else
    {
      pFile = pExpFile;
    } /* endif */
  } /* endif */

  if ( (pDocExpIda->szStartPath[0] == EOS) && !pFile && pDocExpIda->ppCurDirArray )
  {
    fOK = UtlInsertCurDir(pExpFile, &(pDocExpIda->ppCurDirArray),szOutBuf);
    if ( !fOK )
    {
      /************************************************************/
      /* passed name is not valid                                 */
      /************************************************************/
      fOK = FALSE;
      usRC = UTL_PARAM_TOO_LONG;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pExpFile, EQF_ERROR, pDocExpIda->hwndErrMsg );
    }
    else
    {
      pExpFile = szOutBuf;
      pFile = UtlGetFnameFromPath( pExpFile );
      if (!pFile )
      {
        pFile = pExpFile;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if necessary check if output file already exist                  */
  /********************************************************************/
  if ( fOK && (! pDocExpIda->fOverwrite ) )
  {
    if ( UtlFileExist( pExpFile ) )
    {
      fOK = FALSE;
      usRC = DDE_FILE_NO_OVERWRITE;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pExpFile, EQF_ERROR, pDocExpIda->hwndErrMsg  );
    } /* endif */
  } /* endif */


  /********************************************************************/
  /* Check if document exists ...                                     */
  /********************************************************************/
  if ( fOK )
  {
    BOOL fIsNew = FALSE;
    FolLongToShortDocName( pFolderObj, pActDocName, szDocShortName, &fIsNew );
    if ( fIsNew )
    {
      PSZ apszErrParm[2];

      apszErrParm[0] = pFile;
      apszErrParm[1] = pDocExpIda->chFldName;
      fOK = FALSE;
      usRC = DDE_DOC_NOT_IN_FOLDR;
      UtlErrorHwnd( DDE_DOC_NOT_IN_FOLDR, MB_CANCEL, 2, apszErrParm,
                    EQF_ERROR, pDocExpIda->hwndErrMsg );
    }
    else
    {
      // get actual document long name (may have been corrupted by EQFCMD)
      // GQ 2016/04/26: disabled the code below as the szLongName variable is never used
      //szLongName[0] = EOS;
      //UtlMakeEQFPath( szBuf, *pFolderObj, SYSTEM_PATH, pFolder );
      //strcat( szBuf, BACKSLASH_STR );
      //strcat( szBuf, szDocShortName );
      //DocQueryInfo2( szBuf, NULL, NULL, NULL, NULL, szLongName, NULL, NULL, FALSE );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* construct filename depending on requested file type and export   */
  /* the file.                                                        */
  /* Exporting is either copying the file or using the EQFUnSeg       */
  /* utility.                                                         */
  /********************************************************************/
  if ( fOK )
  {
    if ( pDocExpIda->fSource )
    {
      UtlMakeEQFPath( szBuf, *pFolderObj, DIRSOURCEDOC_PATH, pFolder );
      strcat( szBuf, BACKSLASH_STR );
      strcat( szBuf, szDocShortName );
      if ( UtlFileExist( szBuf ))
      {
        usRC = DDECopyToTarget( pDocExpIda, szBuf, pExpFile, pActDocName );
        fOK = !usRC;
      }
      else
      {
        fOK = FALSE;
        usRC = FILE_NOT_EXISTS;
        UtlErrorHwnd( FILE_NOT_EXISTS, MB_CANCEL, 1, &pFile, EQF_ERROR, pDocExpIda->hwndErrMsg );
      } /* endif */
    }
    else
    if ( pDocExpIda->fSNOMATCH )
    {
      UtlMakeEQFPath( szBuf, *pFolderObj, DIRSEGNOMATCH_PATH, pFolder );
      strcat( szBuf, BACKSLASH_STR );
      strcat( szBuf, szDocShortName );
      if ( UtlFileExist( szBuf ))
      {
        usRC = DDECopyToTarget( pDocExpIda, szBuf, pExpFile, pActDocName );
        fOK = ! usRC;
      }
      else
      {
        fOK = FALSE;
        usRC = ERROR_NO_SNOMATCH_FOR_DOC;
        UtlErrorHwnd( ERROR_NO_SNOMATCH_FOR_DOC, MB_CANCEL, 1, &pFile,
                      EQF_ERROR, pDocExpIda->hwndErrMsg );
      } /* endif */
    }
    else
    {
      CHAR szTarget[MAX_EQF_PATH];

      UtlMakeEQFPath( szBuf, *pFolderObj,
                      DIRSEGTARGETDOC_PATH, pFolder );
      strcat( szBuf, BACKSLASH_STR );
      strcat( szBuf, szDocShortName );
      UtlMakeEQFPath( szTarget, *pFolderObj,
                      DIRTARGETDOC_PATH, pFolder );
      strcat( szTarget, BACKSLASH_STR );
      strcat( szTarget, szDocShortName );
      if ( UtlFileExist( szBuf ))
      {

        HPROP       hpropDocument;       // document properties
        ULONG       ulErrorInfo;         // error indicator from PRHA
        CHAR        szDocProp[MAX_EQF_PATH] ;
        USHORT      usTrackDocNum = 0 ;

        UtlMakeEQFPath( szDocProp, *pFolderObj, SYSTEM_PATH, pFolder );
        strcat( szDocProp, BACKSLASH_STR );
        strcat( szDocProp, szDocShortName );
        if ( ( pDocExpIda->fWithTrackID ) &&
             ( (hpropDocument = OpenProperties (szDocProp, NULL, PROP_ACCESS_READ, &ulErrorInfo))!= NULL) ) 
        {
          PPROPDOCUMENT ppropDocument;        // pointer to document properties
          EQFINFO       ErrorInfo;            // error code of property handler calls
     
          ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );
          usTrackDocNum = ppropDocument->usTrackDocNum ;
          CloseProperties(hpropDocument, PROP_QUIT, &ErrorInfo );
        }

        fOK = EQFUnSeg( szBuf, szTarget, TADummyTag, &fGoOn, pFolder, usTrackDocNum );
        if ( fOK )
        {
          usRC = DDECopyToTarget( pDocExpIda, szTarget, pExpFile, pActDocName );
          fOK = ! usRC;
        }
        else
        {
          usRC = ERROR_FILE_INVALID_DATA;
          UtlErrorHwnd( ERROR_FILE_INVALID_DATA, MB_CANCEL, 1, &pFile,
                        EQF_ERROR, pDocExpIda->hwndErrMsg );
        } /* endif */
      }
      else
      {
        fOK = FALSE;
        usRC = ERROR_NOTARGETFILE;
        UtlErrorHwnd( ERROR_NOTARGETFILE, MB_CANCEL, 1, &pFile,
                      EQF_ERROR, pDocExpIda->hwndErrMsg );
      } /* endif */
    } /* endif  source/snomatch/target */


    // Update Document properties with date
    // ------------------------------------

    if ( fOK)
    {

     ULONG         ulErrorInfo;            // error code of property handler
     PPROPDOCUMENT ppropDoc;             // pointer to document properties
     HPROP         hpropDoc;             // handle of document properties
     CHAR          szDocObjName[255];
     PSZ           pszReplace;

     // build object name of document properties
     UtlMakeEQFPath( szDocObjName,
                     pFolderObj[0], SYSTEM_PATH,
                     pFolder );
     strcat(szDocObjName, BACKSLASH_STR );
     strcat(szDocObjName, szDocShortName );

     // open document properties
     if( ( hpropDoc = OpenProperties
                             ( szDocObjName, NULL,
                               PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
     {
        fOK = FALSE;
        pszReplace = szDocObjName;
        UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1,
                  &pszReplace, EQF_ERROR);
     }
     else
     {
                    //--- open was ok, now update the properties ---
                    SetPropAccess( hpropDoc, PROP_ACCESS_WRITE);
                    ppropDoc = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDoc );
                    UtlTime( (PLONG)&ppropDoc->ulExp );
                    SaveProperties( hpropDoc, &ulErrorInfo );
                    ResetPropAccess( hpropDoc, PROP_ACCESS_WRITE);
                    CloseProperties( hpropDoc, PROP_FILE, &ulErrorInfo);

     } /*endif*/
    } /* endif */
  } /* endif */

  // get error code which was used in message box by UtlErrorHwnd
  if ( pDDEReturn )
  {
    pDDEReturn->usRc =  UtlGetDDEErrorCode(pDocExpIda->hwndErrMsg);
  } /* endif */

  return fOK;
} /* end of function DDEDocUnLoad */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ResolveMultFileNames                                     |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = ResolveMultFileNames( pInFile, &ppOutFiles );      |
//+----------------------------------------------------------------------------+
//|Description:       This function will resolve any single or multiple subst. |
//|                   symbols and returns a list of files.                     |
//|                   If no single or multiple substitution is nec. NO output  |
//|                   file list will be allocated                              |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ   pInFile     input file name                        |
//|                   PSZ  *ppOutFiles  output file list                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE    problems in allocating memory                   |
//|                   TRUE     everything okay                                 |
//+----------------------------------------------------------------------------+
//|Function flow:     check if single or multiple substitution available       |
//|                   allocate pool for filename strings                       |
//|                   if okay                                                  |
//|                     call UtlFindFirst to setup the loop                    |
//|                     while files found                                      |
//|                       store the filename; increase pool if necessary       |
//|                       call UtlFindNext                                     |
//|                     wend                                                   |
//|                     set ppOutFile pointer                                  |
//|                   endif                                                    |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
BOOL ResolveMultFileNames
(
  PSZ  pInFile,                        // input file name
  PSZ  pszStartPath,                   // start path or NULL
  PSZ  *ppOutFiles                     // pointer to list array
)
{
  return( ResolveMultFileNamesEx( pInFile, pszStartPath, ppOutFiles, FALSE ) );
}

BOOL ResolveMultFileNamesEx
(
  PSZ  pInFile,                        // input file name
  PSZ  pszStartPath,                   // start path or NULL
  PSZ  *ppOutFiles,                    // pointer to list array
  BOOL   fNonRecursive                 // true = non-recursive; i.e. do not scan sub-directories
)
{
  BOOL fOK = TRUE;
  BOOL fMult = FALSE;                  // multiple or single substitution
  PSZ  pFileName;                      // pointer to FileName
  CHAR   c;                            // active character
  PCHAR pStrPool;                      // string pool
  PCHAR pStartPool;                    // string pool start
  HDIR   hDir = HDIR_CREATE;
  LONGFILEFIND   stResultBuf;
  USHORT        usCount;
  ULONG   ulLen, ulLeft;               // length of strings
  ULONG   ulPathLen;                   // path length of mult/single subst.
  ULONG   ulAllocLen;                  // allocated length

  /********************************************************************/
  /* check if some of the list files are not explicitly specified,    */
  /* i.e. if they contain single or multiple substitution characters  */
  /* if so allocate new space and use UtlFindFirst and UtlFindNext    */
  /* to resolve any multiple file names                               */
  /********************************************************************/
  pFileName = pInFile;
  while ( ((c = *pFileName++)!= NULC) && !fMult )
  {
    fMult = (c == MULTIPLE_SUBSTITUTION) || (c == SINGLE_SUBSTITUTION);
  } /* endwhile */

  if ( (pszStartPath != NULL) && (*pszStartPath != EOS) )
  {
    ULONG ulBufSize = 0L;
    ULONG ulBufUsed = 0L;

    // only use scan dir if pInFile contains substitution characters or
    //     pInFile does not contain path information
    if ( fMult || (strchr( pInFile, '\\') == NULL))
    {
      // scan all files in start path
      DDEScanDir( pszStartPath, pInFile, (SHORT)strlen(pszStartPath),
                  ppOutFiles, &ulBufSize, &ulBufUsed, fNonRecursive );
    }
    else
    {
      // assume startpath and pInFile fully qualify the document
      if ( UtlAlloc( (PVOID *)ppOutFiles, 0L, (LONG)(MAX_LONGPATH + MAX_LONGPATH), ERROR_STORAGE ) )
      {
        PSZ pszBuffer = *ppOutFiles;
        strcpy( pszBuffer, pInFile );
        // no additional EOS delimiter required as buffer has been set to zero by UtlAlloc
       } /* endif */
    } /* endif */
  }
  else if ( fMult )
  {
    /******************************************************************/
    /* resolve it, i.e.                                               */
    /*   allocate memory                                              */
    /*   loop thru list                                               */
    /*     either copy entries without multi/single substitutions     */
    /*     or run resolution of single/multiple                       */
    /*   endwhile                                                     */
    /******************************************************************/
    fOK = UtlAlloc( (PVOID *) &pStartPool, 0L, (LONG) MAX_DDE_POOL, ERROR_STORAGE );

    if ( fOK )
    {
      ulAllocLen = ulLeft = MAX_DDE_POOL;
      pStrPool = pStartPool;
      /**************************************************************/
      /* Search for filenames                                       */
      /**************************************************************/
      usCount = 1;
      hDir = HDIR_CREATE;
      if ( UtlFindFirstLong(pInFile,&hDir,FILE_NORMAL,&stResultBuf,0))
      {
        usCount = 0;  // no files as return code is set
      } /* endif */
      /************************************************************/
      /* extract the path info (if any )...                       */
      /************************************************************/
      pFileName = UtlGetFnameFromPath( pInFile );
      if ( pFileName )
      {
        *pFileName = EOS;            // extract the path info
        ulPathLen = strlen( pInFile );
      }
      else
      {
        pFileName = pInFile;
        *pFileName = EOS;
        ulPathLen = 0;
      } /* endif */

      while ( usCount && fOK )
      {
        /************************************************************/
        /* Add items to our list                                    */
        /* do a realloc if necessary                                */
        /************************************************************/
        ulLen = strlen(stResultBuf.achName) + ulPathLen + 1;
        if ( ulLen >= ulLeft )
        {
          fOK = UtlAlloc( (PVOID *) &pStartPool, (LONG) ulAllocLen,
                                       (LONG) ulAllocLen + MAX_DDE_POOL,
                                       ERROR_STORAGE );
          if ( fOK )
          {
            pStrPool = pStartPool + ulAllocLen - ulLeft;
            ulLeft += MAX_DDE_POOL;
            ulAllocLen += MAX_DDE_POOL;
          } /* endif */
        } /* endif */

        if ( ulLen < ulLeft )
        {
          strcpy(pStrPool, pInFile );
          strcat(pStrPool, stResultBuf.achName);
          pStrPool += ulLen;
          ulLeft -= ulLen;
          /************************************************************/
          /* Find next list                                           */
          /************************************************************/
          if (UtlFindNextLong(hDir, &stResultBuf,0))
          {
             usCount=0;
          }/*endif*/
        } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* set the pointer to the string pool - will be freed by caller */
      /****************************************************************/
      *ppOutFiles = pStartPool;
    } /* endif */
  } /* endif */

  // close search file handle
  if ( hDir != HDIR_CREATE ) UtlFindClose( hDir, FALSE );


  return fOK;
} /* end of function ResolveMultFileNames */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     WildCompares                                             |
//+----------------------------------------------------------------------------+
//|Function call:     fSkip = WildCompares( pSeg, pSearch )                    |
//+----------------------------------------------------------------------------+
//|Description:       compares if search string is equal to segment            |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ           pSeg                                       |
//|                   PSZ           pSearch                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
BOOL WildCompares
(
   PSZ             pSeg,
   PSZ             pSearch
)
{
   USHORT          i = 0;

   if ( (*pSearch == EOS) && (*pSeg == EOS) )
   {
     return( 1 );
   } /* endif */

   if ( *pSearch == EOS )
   {
     return( 0 );
   } /* endif */

   if ( *pSearch == '*' )
   {
     if ( *(pSearch + 1) == EOS )
     {
       return( 1 ) ;
     } /* endif */

     for( i=0 ; i <= strlen(pSeg); i++ )
     {
       if ( (toupper(*(pSeg+i)) == (toupper(*(pSearch+1)))) || (*(pSearch+1) == '?') )
       {
         if ( WildCompares( pSeg+i+1, pSearch+2 ) == 1 )
         {
           return( 1 );
         } /* endif */
       } /* endif */
     } /* endfor */
   }
   else
   {
     if (*pSeg == '\0')
     {
       return( 0 );
     } /* endif */

     if ((*pSearch == '?') || (*pSearch == *pSeg))
     {
       if (WildCompares( pSeg+1, pSearch+1 ) == 1)
       {
         return( 1 );
       } /* endif */
     } /* endif */
   } /* endif */

   return( 0 ) ;

} /* end of function WildCompares */






typedef struct _DDESCANDIRDATA
{
  LONGFILEFIND ResultBuf;              // Buffer for long file names
  HDIR    hDir;                        // file search handle
  CHAR    szPath[MAX_LONGPATH];        // buffer for search path
  CHAR    szFile[MAX_LONGPATH];        // buffer for file name incl. path
} DDESCANDIRDATA, *PDDESCANDIRDATA;

// Recursive function to scan all files of a directory
// sub-directories are processed recursively
//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEScanDir                                               |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//| Recursive function to scan all files of a directory                        |
//| sub-directories are processed recursively                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        x                                                        |
//|                   x                                                        |
//+----------------------------------------------------------------------------+
//|Returns:           x                                                        |
//|                   x                                                        |
//+----------------------------------------------------------------------------+
USHORT DDEScanDir
(
  PSZ    pszStartDir,                  // pointer to start directory
  PSZ    pszPattern,                   // search pattern
  SHORT  sStartPathLen,                // length of original start path
  PSZ    *ppszFileBuffer,              // ptr to file list buffer pointer
  PULONG pulFileBufferSize,            // ptr to file buffer size variable
  PULONG pulFileBufferUsed,            // ptr to file buffer used variable
  BOOL   fNonRecursive                 // true = non-recursive; i.e. do not scan sub-directories
)
{
 PDDESCANDIRDATA pData = NULL;      // ptr to private data area
 USHORT usRC = NO_ERROR;

 if ( UtlAlloc( (PVOID *)&pData, 0L, sizeof(DDESCANDIRDATA), NOMSG ) )
 {
   // initialize data area
   pData->hDir = HDIR_CREATE;

   // setup search path
   strcpy( pData->szPath, pszStartDir );
   if ( pszStartDir[strlen(pszStartDir)-1] != BACKSLASH )
   {
     strcat( pData->szPath, BACKSLASH_STR );
   } /* endif */
   strcat( pData->szPath, pszPattern );

   // scan for files
   usRC = UtlFindFirstLong( pData->szPath, &(pData->hDir),
                            FILE_NORMAL,
                            &(pData->ResultBuf), FALSE );

   while ( !usRC )
   {
     // build complete file name

     strcpy( pData->szFile, pszStartDir );
     if ( pData->szFile[strlen(pData->szFile)-1] != BACKSLASH )
     {
       strcat( pData->szFile, BACKSLASH_STR );
     } /* endif */

     // add any path information of search pattern
     {
       PSZ pszPathInfo;                // path infor tpo be added
       PSZ pszFileName;                // start of file name in search pattern

       // position to start of path information
       pszPathInfo = pszPattern;
       if ( *pszPathInfo == BACKSLASH )
       {
         pszPathInfo++;
       } /* endif */

       // find end of path information and add path for complete file name
       pszFileName = strrchr( pszPathInfo, BACKSLASH );
       if ( pszFileName )
       {
         CHAR chTemp;
         pszFileName++;
         chTemp = *pszFileName;
         *pszFileName = EOS;
         strcat( pData->szFile, pszPathInfo );
         *pszFileName = chTemp;
       } /* endif */
     }

     // add file name
     strcat( pData->szFile, pData->ResultBuf.achName );


     // add file to list of files being imported
     {
       ULONG  ulAddLen;                // length of data being added
       PSZ    pszAddFile;              // points to file name being added

       pszAddFile = pData->szFile + sStartPathLen;
       if ( *pszAddFile == BACKSLASH )
       {
         pszAddFile++;
       } /* endif */

       //  leave at least one byte empty in buffer as buffer end
       //  delimiter
       ulAddLen = strlen( pszAddFile ) + 2;

       // enlarge file name buffer if too small
       if ( (ulAddLen + *pulFileBufferUsed) >= *pulFileBufferSize )
       {

          if ( UtlAlloc( (PVOID *)ppszFileBuffer,
                         (LONG)*pulFileBufferSize,
                         (LONG)*pulFileBufferSize + DDEREALLOCSIZE,
                         ERROR_STORAGE ) )
          {
            *pulFileBufferSize += DDEREALLOCSIZE;
          }
          else
          {
            usRC = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
       } /* endif */

       // add file to file name buffer
       if ( !usRC )
       {
         PSZ pszTarget = *ppszFileBuffer + *pulFileBufferUsed;

         strcpy( pszTarget, pszAddFile );
         *pulFileBufferUsed += strlen(pszAddFile) + 1;
       } /* endif */
     }

     // continue with next file
     usRC = UtlFindNextLong( pData->hDir, &(pData->ResultBuf), FALSE );
   } /* endwhile */


   // cleanup
   if ( pData->hDir != HDIR_CREATE )
   {
     UtlFindCloseLong( pData->hDir, FALSE );
     pData->hDir = HDIR_CREATE;
   } /* endif */


   // scan for directories
   if ( !fNonRecursive )
   {
	   usRC = NO_ERROR;
	   strcpy( pData->szPath, pszStartDir );
	   if ( pszStartDir[strlen(pszStartDir)-1] != BACKSLASH )
	   {
		 strcat( pData->szPath, BACKSLASH_STR );
	   } /* endif */
	   strcat( pData->szPath, "*.*" );

	   if ( !usRC )
	   {
		 usRC = UtlFindFirstLong( pData->szPath, &(pData->hDir),
								  FILE_DIRECTORY,
								  &(pData->ResultBuf), FALSE );
	   } /* endif */

	   while ( !usRC )
	   {
		 // build complete file name
		 strcpy( pData->szFile, pszStartDir );
		 if ( pData->szFile[strlen(pData->szFile)-1] != BACKSLASH )
		 {
		   strcat( pData->szFile, BACKSLASH_STR );
		 } /* endif */
		 strcat( pData->szFile, pData->ResultBuf.achName );


		 // handle directory
		 {
		   // scan the subdirectory
		  if( (strcmp(pData->ResultBuf.achName, CURRENT_DIR_NAME ) != 0) &&
			  (strcmp(pData->ResultBuf.achName, PARENT_DIR_NAME ) != 0) )
		  {
			usRC = DDEScanDir( pData->szFile, pszPattern, sStartPathLen,
							   ppszFileBuffer, pulFileBufferSize,
							   pulFileBufferUsed, fNonRecursive );

		  } /* endif */
		 }

		 // continue with next directory
		 usRC = UtlFindNextLong( pData->hDir, &(pData->ResultBuf), FALSE );
	   } /* endwhile */
   } /* endif */      

   // cleanup
   if ( pData->hDir != HDIR_CREATE )
   {
     UtlFindCloseLong( pData->hDir, FALSE );
   } /* endif */


   UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG  );
 } /* endif */
 return( usRC );
} /* end of function DDEScanDir */


//
// Function DDECopyToTarget
//
// Copies the unsegmented document to its final location and
// creates directories as desired
//
USHORT DDECopyToTarget
(
  PDOCIMPEXP  pDocExpIda,              // pointer to ida
  PSZ         pszTarget,               // file to be exported
  PSZ         pszFinalTarget,          // final target of file
  PSZ         pActDocName              // ptr to actual dcument name

)
{
  static CHAR szPathBuffer[MAX_LONGPATH];
  USHORT usRC = NO_ERROR;              // function return code

  // setup full path name of target file, if not in relative
  // path mode any relative path information of the document
  // name will be ignored
  if ( (pDocExpIda->szStartPath[0] == EOS) &&    // not in relpath mode ?
       (strchr( pActDocName, BACKSLASH)) )       // name contains relpath info ?
  {
    CHAR chTemp;
    PSZ pszTemp = pszFinalTarget +
                  (strlen(pszFinalTarget) - strlen(pActDocName) - 1) ;
    chTemp = *pszTemp;
    *pszTemp = EOS;
    strcpy( szPathBuffer, pszFinalTarget );
    *pszTemp = chTemp;
    strcat( szPathBuffer, strrchr( pActDocName, BACKSLASH ) );
  }
  else
  {
    strcpy( szPathBuffer, pszFinalTarget );
  } /* endif */

  // ensure that target directory exists
  {
    PSZ pszTemp;

    pszTemp = strrchr( szPathBuffer, BACKSLASH );

    if ( pszTemp )
    {
      *pszTemp = EOS;
      UtlMkMultDir( szPathBuffer, FALSE );
      *pszTemp = BACKSLASH;
    } /* endif */
  }

  usRC = UtlCopyHwnd( pszTarget, szPathBuffer, 1, 0L,
                      TRUE, pDocExpIda->hwndErrMsg );
  return( usRC );
} /* end of function DDECopyTotarget */
