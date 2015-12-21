//+----------------------------------------------------------------------------+
//|EQFXDOC.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:   G.Jornitz                                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.2 $ ----------- 26 Feb 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
// 
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 4 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFXDOC.CV_   1.1   10 Jan 1996 18:40:12   BUILD  $
 *
 * $Log:   J:\DATA\EQFXDOC.CV_  $
 *
 *    Rev 1.1   10 Jan 1996 18:40:12   BUILD
 * - adapted to new DocumentDelete
 *
 *    Rev 1.0   09 Jan 1996 09:03:20   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+


#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_ANALYSIS         // analysis functions
#include <eqf.h>                  // General Translation Manager include file

//f1////////////////////////////////////////////////////////////////////////////
// function DocumentLoad, will be included into document object handler       //
////////////////////////////////////////////////////////////////////////////////
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocLoad                                                  |
//+----------------------------------------------------------------------------+
//|Function call:     DocLoad( PSZ, PSZ )                                      |
//+----------------------------------------------------------------------------+
//|Description:       This function tries to load the specified document       |
//|                   into the specified folder                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ      folder name                                     |
//|                   PSZ      document name fully qualified                   |
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

BOOL DocLoad
(
  PSZ   pFolder,
  PSZ   pInFile
)
{
   //---------------------------------------------------------------------------
   // load a specified file into the source directory of a specified folder   --
   //---------------------------------------------------------------------------
   //declare variables

   HPROP        hpropRc;                    //return from create properties
   HPROP        hpropFolder;                //return from folder properties
   PPROPFOLDER  pProp;                      // pointer to folder properties
   USHORT       usDosRc;                    //Return code from Dos(Utl) operations
   ULONG        ulErrorInfo;                //error indicator from PRHA
   PSZ          pszReplace;                 //pointer to replace string UtlError
   PSZ          pFile;                      // pointer to file name
   BOOL         fOK = TRUE;                  //error flag
   CHAR         szBuf[ MAX_EQF_PATH ];
   CHAR         szFolderBuf[ MAX_EQF_PATH ];
   CHAR         szSourceFile[ MAX_EQF_PATH ];

   pFile = UtlGetFnameFromPath( pInFile );

   /*******************************************************************/
   /* open folder properties to find folder path, but add EXT first   */
   /*******************************************************************/
   strcat( pFolder, EXT_FOLDER_MAIN );
   UtlMakeEQFPath( szFolderBuf, NULC, SYSTEM_PATH, pFolder );

   if( (hpropFolder = OpenProperties( szFolderBuf, NULL,
                                 PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
   {
      fOK = FALSE;
      // display error message if not already displayed
      if ( ulErrorInfo != Err_NoStorage )
      {
         UtlError( ERROR_XLATE_FOLDER_NOT_EXIST, MB_CANCEL, 1,
                   &pFolder, EQF_ERROR);
      } /* endif */
   }
   else
   {
     pProp = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
     szFolderBuf[0] = pProp->PropHead.szPath[0];
     CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
   }/*endif*/

   /*******************************************************************/
   /* try to delete the doc. and the properties and create it afterw. */
   /*******************************************************************/
   if ( fOK )
   {
     USHORT usDummy = 0;
     UtlMakeFullPath( szBuf, (PSZ)NULP, szFolderBuf, pFile, (PSZ)NULP );
     DocumentDelete(szBuf, FALSE, &usDummy);

     hpropRc = CreateProperties( szBuf, NULL,
                                 PROP_CLASS_DOCUMENT, &ulErrorInfo );

     if ( !hpropRc )               //error from property handler
     {
       //display message create property fails
       fOK = FALSE;
       pszReplace = szBuf;
       UtlError( ERROR_CREATE_PROP, MB_CANCEL, 1, &pszReplace, EQF_ERROR );
     }
     else
     {
        CloseProperties( hpropRc, PROP_FILE, &ulErrorInfo );
        //call UtlCopy to copy source file to target folder
        UtlMakeEQFPath( szBuf, szFolderBuf[0],
                        DIRSOURCEDOC_PATH, pFolder );
        sprintf( szSourceFile, "%s\\%s", szBuf, pFile );
        usDosRc = UtlCopy( pInFile, szSourceFile, 1, 0L, TRUE );
        if ( usDosRc )
        {
           // delete properties since copy failed
           fOK = FALSE;
           DeleteProperties( szBuf, NULL, &ulErrorInfo );
        }
        else
        {
           /**********************************************************/
           /* reset any read-only flags of loaded document           */
           /**********************************************************/
           UtlSetFileMode( szSourceFile, FILE_NORMAL, 0L, TRUE );
           /*******************************************************************/
           /* build object name and copy it into pInFile -- space large enough*/
           /* copy folder object name, too.                                   */
           /*******************************************************************/
           UtlMakeFullPath( szBuf, (PSZ)NULP, szFolderBuf, pFile, (PSZ)NULP );
           strcpy(pInFile, szBuf );

           strcpy(pFolder, szFolderBuf );
        }/*end if*/
     }/*end if*/
   } /* endif */

   return fOK;

}/*end DocumentLoad*/

/**********************************************************************/
/* Document Unload                                                    */
/**********************************************************************/
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocUnSegm                                                |
//+----------------------------------------------------------------------------+
//|Function call:     DocUnSegm( PSZ, PSZ, PSZ );                              |
//+----------------------------------------------------------------------------+
//|Description:       Unsegment the target document and copy it into the       |
//|                   specified place, delete the document from the folder     |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         folder name                                  |
//|                   PSZ         input document name                          |
//|                   PSZ         output document name                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE        every thing okay                             |
//|                   FALSE       else                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     construct filename for segmented target file             |
//|                   if file exists then                                      |
//|                     call the unsegment utility                             |
//|                   else                                                     |
//|                     fOk = FALSE;                                           |
//|                   endif                                                    |
//|                   if okay then                                             |
//|                     delete the document                                    |
//|                   else                                                     |
//|                     display warning message and leave the doc in the folder|
//|                   endif                                                    |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+

BOOL  DocUnSegm
(
  PSZ pszFolderName,                   // pointer to folder name
  PSZ pszDocumentName,                 // document name
  PSZ pszOutName                       // output name
)
{
  BOOL fOK = TRUE;                     // success indicator
  BOOL fGoOn = FALSE;                  // issue message not to go on
  CHAR chFileName[ MAX_EQF_PATH ];     // file name

  /********************************************************************/
  /* construct filename for segmented target                          */
  /********************************************************************/
  UtlMakeEQFPath( chFileName, pszDocumentName[0], DIRSEGTARGETDOC_PATH,
                  UtlGetFnameFromPath( pszFolderName ));
  strcat( chFileName, BACKSLASH_STR );
  strcat( chFileName, UtlGetFnameFromPath(pszDocumentName));

  if ( UtlFileExist( chFileName ))
  {
    fOK = EQFUnSeg( chFileName, pszOutName, TADummyTag, &fGoOn,
                    UtlGetFnameFromPath( pszFolderName ), 0 );      //DAW  TODO...Add Track info.
                                                                    //DAW  DocUnSegm() is never called.
  }
  else
  {
    fOK = FALSE;
  } /* endif */
// 8@KIT1147D    -- following 8 lines deleted, 3 added
//if ( !fOK  )
//{
//    UtlError( ERROR_NOTARGETFILE, MB_CANCEL, 1, &pszDocumentName, EQF_ERROR  );
//}
//else
//{
//   DocumentDelete(pszDocumentName, FALSE);
//} /* endif */
  if ( fOK )
  {
     USHORT usDummy = 0;
     DocumentDelete(pszDocumentName, FALSE, &usDummy);
  } /* endif */
  return fOK;
}

