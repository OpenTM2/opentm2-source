//+----------------------------------------------------------------------------+
//|EQFPROPS.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: HTML Property sheets for folder/mem/dict/documents             |
//|                                                                            |
//+----------------------------------------------------------------------------+


#define INCL_EQF_TP
#define INCL_EQF_TM
#define INCL_EQF_EDITORAPI
#define INCL_EQF_TAGTABLE

#include "core\memory\MemoryFactory.h"

#include "eqfutdlg.h"       // utilities include AH


#include <eqfdtag.h>        // include tag definitions
#include "eqfdde.h"
#include "eqffol.h"         // long filenames
#include "eqfdoc00.h"


#include "eqfdoc01.h"       // include doc prop definitions AH


#include "eqffll.id"        // IDs for folder lists
#include "eqffll00.h"       // Folder List Handler defines
#include "eqffol00.h"             // Folder handler defines


#include "eqfmem.id"        // IDs for folder lists
#include <../source/core/pluginmanager/OtmMemory.h>
#include "eqftmi.h"         // Folder List Handler defines

#include <io.h>             // C library for I/O
#include <time.h>           // C library for time/date
#include <Windows.h>

#define MAX_WORKSEG   5000

static VOID
DisplayPropsAsHtml
(
  HWND  hwndParent,
  PSZ   pszWorkSeg,
  PSZ   pszFileName,
  PSZ   pszHtmlTitle
);

static BOOL
 EqfCopyFolderPropsToHtml
 (
   PPROPFOLDER pFolProp,
   PSZ         pszWorkSeg,
   PSZ         pszFileName,
   BOOL        fIsSubFolder
 );

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    EqfFolderPropsToHtml                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+


BOOL EqfFolderPropsToHtml
(
PSZ pszFolderObj,
HWND hwndParent
)
{
   BOOL   fOk = TRUE;

   fOk = SubFolderPropsToHtml ( pszFolderObj, hwndParent, FALSE );

   return fOk;

}// end of function EqfFolderPropsToHtml

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    EqfMemPropsToHtml                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


BOOL EqfMemPropsToHtml
(
HWND hwndParent,
PMEM_IDA  pIDA,
OtmMemory *pMem

)
{

    BOOL        fOk=TRUE;

    USHORT      usRc = 0;
    HFILE       hOutFile = NULLHANDLE;        // File handle for input file
    USHORT      usAction;                     // file action performed by DosOpen
    ULONG      ulBytesWritten;               // number of bytes written to file
    CHAR        chFileName[MAX_LONGPATH];
    PSZ         pszWorkSeg = NULL;

    ULONG       ulWorkSegLen = 0;    // current length of worksegment

    // allocate buffer for work segment
    if ( fOk )
    {
      fOk = UtlAlloc( (PVOID *)&pszWorkSeg, 0L, 0xFF00, ERROR_STORAGE);
      ulWorkSegLen = 0xFF00;
    } /* endif */

    //-----------------------------------------------------------------
    //  Write HTML file containing information
    //-----------------------------------------------------------------

    /******************************************************************/
    /* Open temp file name                                            */
    /******************************************************************/

    if (fOk)
    {

        UtlMakeEQFPath( chFileName, NULC, SYSTEM_PATH, NULL );
        strcat( chFileName, "\\property.htm" );

        usRc = UtlOpen( chFileName, &hOutFile, &usAction, 0L, FILE_NORMAL, FILE_CREATE | FILE_TRUNCATE,
                        OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, 0L, FALSE );

        if (usRc != 0)
            fOk = FALSE;

    } // end if


    if (fOk)
    {
        char szBuffer[256];
        char szDrive[4];
        char szLastUpdate[30];
        long lProposals = 0;
        OtmMemoryPlugin::PMEMORYINFO pInfo = NULL;

        pInfo = (OtmMemoryPlugin::PMEMORYINFO)malloc( sizeof(OtmMemoryPlugin::MEMORYINFO) );
        memset( pInfo, 0, sizeof(OtmMemoryPlugin::MEMORYINFO) );
        OtmPlugin *pPlugin =(OtmPlugin *)pMem->getPlugin();
        if ( pPlugin->getType() == OtmPlugin::eTranslationMemoryType )
        {
          ((OtmMemoryPlugin *)pPlugin)->getMemoryInfo( pIDA->szMemName, pInfo );
        }
        else if ( pPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType )
        {
          ((OtmSharedMemoryPlugin *)pPlugin)->getMemoryInfo( pIDA->szMemName, pInfo );
        }

        if ( pInfo->szFullPath[0] != '\0' )
        {
          WIN32_FIND_DATA FileInfo;
          HANDLE hDir;

          sprintf( szDrive, "%c:", pInfo->szFullPath[0] );

          // use last update date of base file for last update field
          hDir = FindFirstFile( pInfo->szFullPath, &FileInfo );
          if ( hDir != INVALID_HANDLE_VALUE )
          {
            FILETIME ftLocalTime;
            WORD wDate, wTime;

            FileTimeToLocalFileTime( &(FileInfo.ftLastWriteTime), &ftLocalTime );
            FileTimeToDosDateTime( &ftLocalTime, &wDate, &wTime );

            UtlFDateToDateString( (FDATE *)&wDate, szLastUpdate, sizeof(szLastUpdate)-1 );
            strcat( szLastUpdate, " " );
            UtlFTimeToTimeString( (FTIME *)&wTime, szLastUpdate + strlen(szLastUpdate), sizeof(szLastUpdate) - strlen(szLastUpdate) );

            FindClose( hDir );
          } 
          else
          {
            strcpy( szLastUpdate, "n/a" );
          }
          Utlstrccpy( szBuffer, UtlGetFnameFromPath( pInfo->szFullPath ), '.' );
        }
        else
        {
          strcpy( szDrive, "n/a" );
          strcpy( szLastUpdate, "n/a" );
          strcpy( szBuffer, "n/a" );
        }


        strcpy (pszWorkSeg,"<html><body><h2>Memory Properties</h2><ul>");

        strcat (pszWorkSeg, "<li><b>Memory Name:</b> ");
        strcat (pszWorkSeg, pIDA->szMemName );

        strcat (pszWorkSeg, "<li><b>Short Name:</b> ");
        strcat (pszWorkSeg, szBuffer );
        strcat (pszWorkSeg, "<li><b>Memory plugin:</b> ");
        strcat (pszWorkSeg, pPlugin->getName() );
        strcat (pszWorkSeg, "<li><b>Description:</b> ");
        pMem->getDescription( szBuffer, sizeof(szBuffer) );
        strcat (pszWorkSeg, szBuffer );
        strcat (pszWorkSeg, "<li><b>Source language:</b> ");
        pMem->getSourceLanguage( szBuffer, sizeof(szBuffer) );
        strcat (pszWorkSeg, szBuffer );
        strcat( pszWorkSeg, "<li><b>Drive:</b> " );
        strcat( pszWorkSeg, szDrive );
        strcat (pszWorkSeg, "<li><b>Last Update:</b> ");
        strcat( pszWorkSeg, szLastUpdate );
        ULONG ulSize = pMem->getFileSize();
        sprintf( pszWorkSeg+strlen(pszWorkSeg), "<li><b>Size of Memory:</b> %lu Bytes", ulSize );

        strcat (pszWorkSeg, "<li><b>Different segments:</b> ");
        lProposals = pMem->getProposalNum();
        ltoa( lProposals, szBuffer, 10 );
        strcat (pszWorkSeg, szBuffer );

        // add markup names
        {
          int iMarkups = pMem->getNumOfMarkupNames();
          strcat (pszWorkSeg, "<li><b>Markup:</b><br> ");
          for ( int i = 0; i < iMarkups; i++ )
          {
            pMem->getMarkupName( i, szBuffer, sizeof( szBuffer ) );
            strcat (pszWorkSeg, szBuffer );
            strcat (pszWorkSeg, "<br>");
          } /* end */             
        }

        strcat (pszWorkSeg,"</ul></body></html>");

         // take care of NLV characters ( internal all is stored in ASCII)
        OEMTOANSI( pszWorkSeg );
        usRc = UtlWriteL( hOutFile, pszWorkSeg, strlen( pszWorkSeg ), &ulBytesWritten, FALSE);

        if (usRc != 0) fOk = FALSE;

        if ( pInfo != NULL ) free( pInfo );
    } // end if



    if ( hOutFile )
    {
        UtlClose( hOutFile, FALSE );
    } /* endif */


    //-----------------------------------------------------------------
    // Open HTML control
    //-----------------------------------------------------------------

    /********************************************************************/
    /* force display in HTML control                                    */
    /********************************************************************/

    if (fOk)
    {

        WinSendMsg( GETPARENT(hwndParent), WM_EQF_SHOWHTML, "Memory Properties", chFileName );
    }

    if ( hOutFile )
    {
        UtlDelete( chFileName, 0L, FALSE );
    } /* endif */


    // Garbage collection

    if ( pszWorkSeg ) UtlAlloc( (PVOID *)&pszWorkSeg, 0L, 0L, NOMSG );

    return fOk;

}// end of function EqfFolderMemToHtml



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:  EqfDictPropsToHtml                                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function call: EqfDictPropsToHtml (PSZ pszDictionaryObj, HWND hwndParent)   |
//+----------------------------------------------------------------------------+
//|Description:  Shows dictionary properties in HTML                           |                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:   PSZ  pszDictionaryObj pointer to dictionary object            |
//|              HWND hwndParent dialog handle                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
BOOL EqfDictPropsToHtml
(
PSZ pszDictionaryObj,
HWND hwndParent
)
{
    BOOL            fOk=TRUE;
    HPROP           hDicProp;               // dictionary properties handle
    PPROPDICTIONARY pDicProp = NULL;        // ptr to dictionary properties
    EQFINFO         ErrorInfo;              // return code from prop. handler

    USHORT      usRc = 0;
    HFILE       hOutFile = NULLHANDLE;      // file handle for input file
    USHORT      usAction;                   // file action performed by DosOpen
    ULONG       ulBytesWritten;             // number of bytes written to file

    CHAR        chSystemPath[MAX_EQF_PATH];
    CHAR        chFileName[MAX_EQF_PATH];
  //  CHAR        szWorkSeg[MAX_WORKSEG];
    PSZ         pszWorkSeg;
    int         i;

    fOk = UtlAlloc( (PVOID *)&pszWorkSeg, 0L, 10000L, ERROR_STORAGE);
    //-----------------------------------------------------------------
    // Open dictionary properties
    //-----------------------------------------------------------------

    hDicProp = OpenProperties( pszDictionaryObj, NULL,
                               PROP_ACCESS_READ, &ErrorInfo);
    if ( hDicProp )
    {
        pDicProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hDicProp );
    }
    else
    {
        if ( ErrorInfo != Err_NoStorage )
        {
            UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszDictionaryObj,
                          EQF_ERROR, hwndParent);
        } /* endif */

        fOk = FALSE;

    }//end if

    /******************************************************************/
    /* Open temp file name                                            */
    /******************************************************************/

    if (fOk)
    {
        UtlMakeEQFPath( chSystemPath, NULC, SYSTEM_PATH, NULL );
        sprintf( chFileName, "%s\\%s", chSystemPath, "property.htm" );

        usRc = UtlOpen( chFileName, &hOutFile, &usAction,
                        0L,
                        FILE_NORMAL,
                        FILE_CREATE | FILE_TRUNCATE,
                        OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
                        0L,
                        FALSE );

        if (usRc != 0) fOk = FALSE;
    } // end if

   if (fOk)
   {
      CHAR szDrive[MAX_EQF_PATH];
      PSZ  pszName;
      strcpy (pszWorkSeg,"<html><body><h2>Dictionary Properties</h2>");

      strcat (pszWorkSeg, "<h3>General Properties</h3>");
      strcat (pszWorkSeg, "<TABLE WIDTH=\"80%\" BGCOLOR=\"#FFFFFF\">");

      strcat (pszWorkSeg, "<tr><td><b>Name: </b></td><td>");
      if (pDicProp->szLongName[0] != EOS)
      {
         strcat (pszWorkSeg, pDicProp->szLongName);
      }
      else
      {
         strcpy (szDrive, pDicProp->szDictPath);
         pszName =  UtlSplitFnameFromPath(szDrive);
         pszName[strlen(pszName)-4] = EOS;
         strcat (pszWorkSeg, pszName);
      } // end if
      strcat (pszWorkSeg, "</td></tr>");

      strcat (pszWorkSeg, "<tr><td><b>Short name: </b></td><td>");
      Utlstrccpy( pszWorkSeg + strlen(pszWorkSeg), UtlGetFnameFromPath( pDicProp->szDictPath ), DOT );
      strcat (pszWorkSeg, "</td></tr>");

      strcat (pszWorkSeg, "<tr><td><b>Source language: </b></td><td>");
      strcat (pszWorkSeg, pDicProp->szSourceLang);
      strcat (pszWorkSeg, "</td></tr>");

      strcat (pszWorkSeg, "<tr><td><b>Drive: </b></td><td>");
      strcpy (szDrive, pDicProp->szDictPath);
      UtlSplitFnameFromPath(szDrive);
      UtlSplitFnameFromPath(szDrive);
      UtlSplitFnameFromPath(szDrive);
      strcat (pszWorkSeg, szDrive);
      strcat (pszWorkSeg, "</td></tr>");

      strcat (pszWorkSeg, "<tr><td><b>Description: </b></td><td>");
      if(pDicProp->szDescription[0] != EOS)
     {
       strcat (pszWorkSeg, pDicProp->szDescription);
      }
     else
     {
       strcat (pszWorkSeg, pDicProp->szLongDesc);
      }

      strcat (pszWorkSeg, "</td></tr>");

      strcat (pszWorkSeg, "<tr><td><b>Location: </b></td><td>");
      switch (pDicProp->usLocation)
      {
         case LOC_LOCAL: strcat (pszWorkSeg, "Local"); break;
         case LOC_REMOTE: strcat (pszWorkSeg, "Remote"); break;
         case LOC_SHARED: strcat (pszWorkSeg, "Shared"); break;
      }/* endswitch */
      strcat (pszWorkSeg, "</td></tr>");

      strcat (pszWorkSeg, "<tr><td><b>Protected?: </b></td><td>");
      switch (pszWorkSeg, pDicProp->fProtected)
      {
         case 1: strcat (pszWorkSeg, "Yes"); break;
         case 0: strcat (pszWorkSeg, "No"); break;
      }/*endswitch*/
      strcat (pszWorkSeg, "</td></tr>");
      strcat (pszWorkSeg, "</TABLE>");


      strcat (pszWorkSeg, "<h3>Entry Structure</h3>");
//      strcat (pszWorkSeg, "<TABLE WIDTH=\"80%\" BGCOLOR=\"#FFFFFF\">");

//      strcat (pszWorkSeg, "<tr><td><b>Entry Fields: </b></td><td>");
      for (i = 0; i <= MAX_PROF_ENTRIES; i++)
      {
         if (pDicProp->ProfEntry[i].chSystName[0] != EOS)
         {
            strcat (pszWorkSeg, "<dl>");
            strcat (pszWorkSeg, "<li><b>Entry Field <i> ");
            strcat (pszWorkSeg, pDicProp->ProfEntry[i].chSystName);
            strcat (pszWorkSeg, ": </i></b><br> ");

            switch (pDicProp->ProfEntry[i].usDisplay)
            {
               case 0: strcat (pszWorkSeg, "<i> Omit,</i>"); break;
               case 1: strcat (pszWorkSeg, "<i> Panel 1,</i>"); break;
               case 2: strcat (pszWorkSeg, "<i> Panel 2,</i>"); break;
               case 3: strcat (pszWorkSeg, "<i> Panel 3,</i>"); break;
            } /* endswitch */

            switch (pDicProp->ProfEntry[i].usLevel)
            {
               case 1: strcat (pszWorkSeg, "<i> Entry Level,</i>"); break;
               case 2: strcat (pszWorkSeg, "<i> Homonym Level,</i>"); break;
               case 3: strcat (pszWorkSeg, "<i> Sense Level,</i>"); break;
               case 4: strcat (pszWorkSeg, "<i> Target Level,</i>"); break;
            } /* endswitch */

            switch (pDicProp->ProfEntry[i].usEntryFieldType)
            {
               case 1: strcat (pszWorkSeg, "<i> Small Size,</i>"); break;
               case 2: strcat (pszWorkSeg, "<i> Large Size,</i>"); break;
            } /* endswitch */

            if (strcmp(pDicProp->ProfEntry[i].chSystName, "Headword") == 0 ||
                strcmp(pDicProp->ProfEntry[i].chSystName, "Translation") == 0)
               strcat (pszWorkSeg, "<i> Default Aut. Lookup </i>");
            else
               switch (pDicProp->ProfEntry[i].fAutLookup)
               {
                  case 1: strcat (pszWorkSeg, "<i> Aut. Lookup </i>"); break;
                  case 0: strcat (pszWorkSeg, "<i> No Aut. Lookup  </i>"); break;
               } /* endswitch */
            strcat (pszWorkSeg, "</dl>");
         } /*endif*/
      } /*endfor*/


//      strcat (pszWorkSeg, "</td></tr>");
//      strcat (pszWorkSeg, "</TABLE>");

      strcat (pszWorkSeg,"</body></html>");
      // take care of NLV characters ( internal all is stored in ASCII)
     OEMTOANSI( pszWorkSeg );


      usRc = UtlWriteL( hOutFile,
                       pszWorkSeg, strlen( pszWorkSeg ),
                       &ulBytesWritten, FALSE);

      if (usRc != 0) fOk = FALSE;
   } /* end if */



   if ( hOutFile )
   {
      UtlClose( hOutFile, FALSE );
   } /* endif */


    //-----------------------------------------------------------------
    // Open HTML control
    //-----------------------------------------------------------------

    /********************************************************************/
    /* force display in HTML control                                    */
    /********************************************************************/

    if (fOk)
    {
        WinSendMsg( GETPARENT(hwndParent), WM_EQF_SHOWHTML, "Dictionary Properties", chFileName );
    }

    // Garbage collection


    if (hDicProp) CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo );
    UtlAlloc( (PVOID *)&pszWorkSeg, 0L, 0L, NOMSG );

    return fOk;

}// end of function EqfDictPropsToHtml

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:  EqfDocPropsToHtml                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function call: EqfDocPropsToHtml (PSZ pszDocumentObj, HWND hwndParent)      |
//+----------------------------------------------------------------------------+
//|Description:  Shows document properties in HTML                             |                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:   PSZ  pszDocumentObj pointer to document object                |
//|              HWND hwndParent dialog handle                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


BOOL EqfDocPropsToHtml
(
PSZ pszDocumentObj,
HWND hwndParent
)
{

    BOOL        fOk=TRUE;
    HPROP       hDocProp;                    // document properties handle
    PPROPDOCUMENT pDocProp = NULL;           // ptr to document properties
    EQFINFO     ErrorInfo;                   // return code from prop. handler

    USHORT      usRc = 0;
    HFILE       hOutFile = NULLHANDLE;       // File handle for input file
    USHORT      usAction;                    // file action performed by DosOpen
    ULONG       ulBytesWritten;              // number of bytes written to file

    CHAR        chSystemPath[MAX_EQF_PATH];
    CHAR        chFileName[MAX_EQF_PATH];
    CHAR        szWorkSeg[MAX_WORKSEG];
    PSZ         pszWorkSeg;
    HPROP       hFolProp;                    // folder properties handle
    PPROPFOLDER pFolProp= NULL;              // ptr to folder properties

    char        szBuffer[256];               // Output buffer for itoa and LongtoDateTime
    FILEFINDBUF stFile;                      // Output buffer of UtlFindFirst
    USHORT      usCount = 1;                 // For UtlFindFirst
    HDIR        hSearch = HDIR_CREATE;       // Dir handle for UtlFindFirst

    pszWorkSeg = &szWorkSeg[0];

    // Open document properties
    hDocProp = OpenProperties( pszDocumentObj, NULL,
                               PROP_ACCESS_READ, &ErrorInfo);

    if ( hDocProp )
    {
        pDocProp = (PPROPDOCUMENT) MakePropPtrFromHnd( hDocProp );
    }
    else
    {
        if ( ErrorInfo != Err_NoStorage )
        {
            UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszDocumentObj,
                          EQF_ERROR, hwndParent);
        } /* endif */

        fOk = FALSE;

    }//end if

    // Open Folder properties
    UtlSplitFnameFromPath(pszDocumentObj);
    UtlQueryString( QST_PRIMARYDRIVE, chSystemPath, sizeof(chSystemPath) );
    pszDocumentObj[0] = chSystemPath[0]; // use primary drive for folder props!

    hFolProp = OpenProperties( pszDocumentObj, NULL,
                               PROP_ACCESS_READ, &ErrorInfo);


    if ( hFolProp )
    {
        pFolProp = (PPROPFOLDER) MakePropPtrFromHnd( hFolProp );

    }
    else
    {
        if ( ErrorInfo != Err_NoStorage )
        {
            UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszDocumentObj,
                          EQF_ERROR, hwndParent);
        } /* endif */

        fOk = FALSE;

    }//end if

    //  Write HTML file containing information

    /******************************************************************/
    /* Open temp file name                                            */
    /******************************************************************/

    if (fOk)
    {

        UtlMakeEQFPath( chSystemPath, NULC, SYSTEM_PATH, NULL );
        sprintf( chFileName, "%s\\%s", chSystemPath, "property.htm" );

        usRc = UtlOpen( chFileName, &hOutFile, &usAction,
                        0L,
                        FILE_NORMAL,
                        FILE_CREATE | FILE_TRUNCATE,
                        OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
                        0L,
                        FALSE );

        if (usRc != 0) fOk = FALSE;
    } // end if

    //Adding Settings information to HTML document

    if (fOk)
    {
        PSZ pszMemory;

        strcpy (pszWorkSeg,"<html><body><h2>Document Properties</h2>");

        strcat (pszWorkSeg, "<h3>Settings</h3>");
        strcat (pszWorkSeg, "<TABLE WIDTH=\"70%\" BGCOLOR=\"#FFFFFF\">");

        strcat (pszWorkSeg, "<tr><td><b>Short name: </b></td><td>");
        strcat( pszWorkSeg, pDocProp->PropHead.szName );
        strcat (pszWorkSeg, "</td></tr>");

        strcat (pszWorkSeg, "<tr><td><b>Translation Memory: </b></td><td>");
        pszMemory = pFolProp->szMemory;
        if ( pFolProp->szLongMemory[0] != EOS ) pszMemory = pFolProp->szLongMemory;

        if ( pDocProp->szLongMemory[0] !=EOS )
        {
          pszMemory = pDocProp->szLongMemory;
        }
        else if ( pDocProp->szMemory[0] !=EOS )
        {
          pszMemory = pDocProp->szMemory;
        } /* endif */
        strcat( pszWorkSeg, pszMemory );
        strcat (pszWorkSeg, "</td></tr>");

        strcat (pszWorkSeg, "<tr><td><b>Source language: </b></td><td>");

        if (pDocProp->szSourceLang[0]==EOS)
        {
            strcat (pszWorkSeg, pFolProp->szSourceLang);
        }
        else
        {
            strcat (pszWorkSeg, pDocProp->szSourceLang);
        }//endfif

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Target language: </b></td><td>");
        if (pDocProp->szTargetLang[0]==EOS)
        {
            strcat (pszWorkSeg, pFolProp->szTargetLang);
        }
        else
        {
            strcat (pszWorkSeg, pDocProp->szTargetLang);
        }//endfif

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Editor: </b></td><td>");
        if (pDocProp->szEditor[0]==EOS)
        {
            strcat (pszWorkSeg, pFolProp->szEditor);
        }
        else
        {
            strcat (pszWorkSeg, pDocProp->szEditor);
        }//endfif

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>  Markup Table: </b></td><td>");
        if (pDocProp->szFormat[0]==EOS)
        {
            strcat (pszWorkSeg, pFolProp->szFormat);
        }
        else
        {
            strcat (pszWorkSeg, pDocProp->szFormat);
        }//endfif

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "</TABLE>");


        //Adding statistics information to HTML document


        strcat (pszWorkSeg, "<h3>Statistics</h3>");
        strcat (pszWorkSeg, "<TABLE WIDTH=\"70%\" BGCOLOR=\"#FFFFFF\">");

        /**********************************************************/
        /* Build document path                                    */
        /**********************************************************/
         strcpy( szBuffer, pDocProp->PropHead.szPath );
         strcat( szBuffer, BACKSLASH_STR );
         UtlQueryString( QST_SOURCEDIR,
                          szBuffer + strlen(szBuffer),
                          MAX_FILESPEC );
         strcat( szBuffer, BACKSLASH_STR );
         strcat( szBuffer, pDocProp->PropHead.szName );


        /**********************************************************/
        /* Get document size                                      */
        /**********************************************************/
        memset( &stFile, 0, sizeof(stFile) );
        UtlFindFirst( szBuffer, &hSearch, 0, &stFile, sizeof(stFile), &usCount,
                      0L, FALSE );
        UtlFindClose( hSearch, FALSE );



        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Size: </b></td><td>");
        ltoa( RESBUFSIZE(stFile), szBuffer, 10 );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Translated: </b></td><td>");
        LONGTODATETIME( pDocProp->ulXLated, szBuffer );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Analyzed: </b></td><td>");
        LONGTODATETIME( pDocProp->ulSeg, szBuffer );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Date of Export: </b></td><td>");
        LONGTODATETIME( pDocProp->ulExp, szBuffer );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Date of Import: </b></td><td>");
        LONGTODATETIME( pDocProp->ulImp, szBuffer );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");          // added bt 19062001
        strcat (pszWorkSeg, "<tr><td><b>Date of Source File: </b></td><td>");
        LONGTODATETIME( pDocProp->ulSrc, szBuffer );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Last Update: </b></td><td>");
        LONGTODATETIME( pDocProp->ulTouched, szBuffer );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Completion Rate: </b></td><td>");
        itoa( pDocProp->usComplete, szBuffer, 10 );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Changed Segments: </b></td><td>");
        itoa( pDocProp->usComplete, szBuffer, 10 );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>New Segments: </b></td><td>");
        itoa( pDocProp->usComplete, szBuffer, 10 );
        strcat (pszWorkSeg, szBuffer);

        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg, "<tr><td><b>Copied Segments: </b></td><td>");
        itoa( pDocProp->usCopied, szBuffer, 10 );
        strcat (pszWorkSeg, szBuffer);


        strcat (pszWorkSeg, "</td></tr>");
        strcat (pszWorkSeg,"</TABLE></body></html>");

         // take care of NLV characters ( internal all is stored in ASCII)
        OEMTOANSI( pszWorkSeg );
        usRc = UtlWriteL( hOutFile,
                         pszWorkSeg, strlen( pszWorkSeg ),
                         &ulBytesWritten, FALSE);

        if (usRc != 0) fOk = FALSE;
    } // end if



    if ( hOutFile )
    {
        UtlClose( hOutFile, FALSE );
    } /* endif */


    //-----------------------------------------------------------------
    // Open HTML control
    //-----------------------------------------------------------------

    /********************************************************************/
    /* force display in HTML control                                    */
    /********************************************************************/

    if (fOk)
    {
      DisplayPropsAsHtml( hwndParent, szWorkSeg, chFileName, "Document Properties");
    }

    // Garbage collection

    if (hDocProp) CloseProperties( hDocProp, PROP_QUIT, &ErrorInfo );
    if (hFolProp) CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );


    return fOk;


}// end of function EqfDocPropsToHtml



BOOL
SubFolderPropsToHtml
(
  PSZ     pszFolderObj,
  HWND    hwndParent,
  BOOL    fIsSubFolder
 )
 {

    BOOL        fOk=TRUE;
    HPROP       hFolProp = 0;                 // folder properties handle
    PPROPFOLDER pFolProp = NULL;              // ptr to folder properties
    EQFINFO     ErrorInfo;                    // return code from prop. handler
    CHAR        chSystemPath[MAX_EQF_PATH];
    CHAR        chFileName[MAX_EQF_PATH];
    CHAR        szWorkSeg[MAX_WORKSEG];
    PSZ         pszWorkSeg;

    pszWorkSeg = &szWorkSeg[0];


    /******************************************************************/
    /* Open temp file name                                            */
    /******************************************************************/
     if (fOk)
     {
         UtlMakeEQFPath( chSystemPath, NULC, SYSTEM_PATH, NULL );
         sprintf( chFileName, "%s\\%s", chSystemPath, "property.htm" );
     } // end if

     if (fIsSubFolder)
     {
       // copy properties of subfolder and folder together
       // Fill folder info fields
       fOk = UtlAlloc( (PVOID *)&pFolProp, 0L,
                            (LONG)sizeof( PROPFOLDER ), ERROR_STORAGE);

       FolQueryInfoEx( pszFolderObj,       // parent (sub)folder object name
                       pFolProp->szLongMemory,      // maybe long or short: use LongMemory to fit size
                       pFolProp->szFormat,
                       pFolProp->szSourceLang,
                       pFolProp->szTargetLang,
                       pFolProp->szEditor,
                       pFolProp->szConversion,
                       pFolProp->szVendor,
                       pFolProp->szVendorEMail, NULL, NULL, NULL, TRUE, hwndParent );
       SubFolObjectNameToName( pszFolderObj, pFolProp->szLongName );
       UtlSplitFnameFromPath(pFolProp->szLongName );
     }
     else
     {
       //-----------------------------------------------------------------
       // Open Folder properties
       //-----------------------------------------------------------------

       // replace drive letter with primary drive letter
       UtlQueryString( QST_PRIMARYDRIVE, chSystemPath, sizeof(chSystemPath) );
       strcpy( chSystemPath + 1, pszFolderObj + 1 );

       hFolProp = OpenProperties( chSystemPath, NULL,
                                  PROP_ACCESS_READ, &ErrorInfo);

       if ( hFolProp )
       {
           pFolProp = (PPROPFOLDER) MakePropPtrFromHnd( hFolProp );
       }
       else
       {
           if ( ErrorInfo != Err_NoStorage )
           {
               UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszFolderObj,
                             EQF_ERROR, hwndParent);
           } /* endif */

           fOk = FALSE;
       }//end if

     }
     if (fOk)
     {
        EqfCopyFolderPropsToHtml(pFolProp, szWorkSeg, chFileName, fIsSubFolder );
     }

    /********************************************************************/
    /* force display in HTML control                                    */
    /********************************************************************/
    if ( fOk)
    {
      if (fIsSubFolder)
      {
        DisplayPropsAsHtml( hwndParent, szWorkSeg, chFileName, "Subfolder Properties");
      }
      else
      {
        DisplayPropsAsHtml( hwndParent, szWorkSeg, chFileName, "Folder Properties");
      }
    }

    // Garbage collection
    if (fIsSubFolder)
    {
      UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
    }
    else
    {
      if (hFolProp) CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    }

   return fOk;
 } // end of function SubFolderPRopsToHtml

static VOID
DisplayPropsAsHtml
(
  HWND  hwndParent,
  PSZ   pszWorkSeg,
  PSZ   pszFileName,
  PSZ   pszHtmlTitle
)
{    // force display in HTML control
    // take care of odd parentship in case of IE-View

    HWND hwnd = GETPARENT( hwndParent );
    GetClassName( hwnd, pszWorkSeg, MAX_WORKSEG );
    if ( memcmp( pszWorkSeg, "AfxMDI", 6 ) == 0 )
    {
       hwnd = GETPARENT( hwnd );
    } /* endif */
    WinSendMsg( hwnd, WM_EQF_SHOWHTML, pszHtmlTitle, pszFileName );
    return;
} /* end of function DisplayPropsAsHtml */


static BOOL
 EqfCopyFolderPropsToHtml
 (
   PPROPFOLDER  pFolProp,
   PSZ          pszWorkSeg,
   PSZ          pszFileName,
   BOOL         fIsSubFolder
 )
 {
   BOOL        fOk = TRUE;
   CHAR        szDicts[1000];
   CHAR        szTMs[1000];
   USHORT      usRc = 0;
   HFILE       hOutFile = NULLHANDLE;        // File handle for input file
   USHORT      usAction;                     // file action performed by DosOpen
   ULONG       ulBytesWritten;               // number of bytes written to file

   if (fOk)
   {
      usRc = UtlOpen( pszFileName, &hOutFile, &usAction,
                          0L,
                          FILE_NORMAL,
                          FILE_CREATE | FILE_TRUNCATE,
                          OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
                          0L,
                          FALSE );
      if (usRc != 0) fOk = FALSE;
   }

    if (fOk)
    {
        strcpy (pszWorkSeg,"<html><body><h2>Folder Properties</h2>");

        strcat (pszWorkSeg, "<ul>");
        if (fIsSubFolder)
        {
          strcat (pszWorkSeg, "<li><b>Subfolder name: </b>");
        }
        else
        {
          strcat (pszWorkSeg, "<li><b>Folder name: </b>");
        }
        if (pFolProp->szLongName[0] != EOS)
        {
          if (fIsSubFolder)
          {
             strcat (pszWorkSeg, pFolProp->szLongName + strlen(pFolProp->szLongName)+1);
             strcat (pszWorkSeg, "<li><b>Parent (sub)folder name: </b>");
             strcat (pszWorkSeg, pFolProp->szLongName);
          }
          else
          {
             strcat( pszWorkSeg, pFolProp->szLongName );
          }
        }
        else
        {
            strcat( pszWorkSeg, pFolProp->PropHead.szName);
            pszWorkSeg[strlen(pszWorkSeg)-4]=EOS;
        } // end if
        if (!fIsSubFolder)
        {
          strcat (pszWorkSeg, "<li><b>Short name: </b>");
          Utlstrccpy( pszWorkSeg + strlen(pszWorkSeg), pFolProp->PropHead.szName, DOT );
        } // end if
        strcat (pszWorkSeg, "<li><b>Document format: </b>");
        strcat (pszWorkSeg, pFolProp->szFormat);
        strcat (pszWorkSeg, "<li><b>Source language: </b>");
        strcat (pszWorkSeg, pFolProp->szSourceLang);
        strcat (pszWorkSeg, "<li><b>Target language: </b>");
        strcat (pszWorkSeg, pFolProp->szTargetLang);
        if (!fIsSubFolder)
        {
          strcat (pszWorkSeg, "<li><b>Description: </b>");
          strcat (pszWorkSeg, pFolProp->szDescription);
        }
        strcat (pszWorkSeg, "<li><b>Editor: </b>");
        strcat (pszWorkSeg, pFolProp->szEditor);
        strcat (pszWorkSeg, "<li><b>Memory: </b>");
        if (pFolProp->szLongMemory[0] != EOS)
        {
           strcat (pszWorkSeg, pFolProp->szLongMemory);
        }
        else
        {
            strcat (pszWorkSeg, pFolProp->szMemory);
        } // end if

        if (!fIsSubFolder)
        {
          strcat (pszWorkSeg, "<li><b>Dictionary: </b>");
            if (pFolProp->DicTbl[0] != EOS)
            {
                strcpy( szDicts, pFolProp->DicTbl );
                {
                   PSZ pszTemp = szDicts;
                   while ( *pszTemp != EOS )
                   {
                     if ( *pszTemp == X15 )
                     {
                       if ( pszTemp[1] == EOS )      // last X15 in string ???
                       {
                         *pszTemp = EOS;             //   yes -> discard X15
                       }
                       else
                       {
                          *pszTemp = COMMA;           //   no -> replace by comma
                       } /* endif */
                     } /* endif */
                       pszTemp++;
                   } /* endwhile */
                }
            }
            else
            {
              int i = 0;
              szDicts[0] = EOS;
              while ( (i < MAX_NUM_OF_FOLDER_DICS) &&
                    (pFolProp->aLongDicTbl[i][0] != EOS) )
              {
                  if ( szDicts[0] != EOS )        // not first dict in buffer?
                  {
                      strcat( szDicts, "," );
                  } /* endif */
                  strcat( szDicts, pFolProp->aLongDicTbl[i] );
                  i++;
              }     /*end while */
            }
            strcat(pszWorkSeg, szDicts);
        }
        strcat (pszWorkSeg, "<li><b>Conversion: </b>");
        strcat (pszWorkSeg, pFolProp->szConversion);
        if (!fIsSubFolder && ((pFolProp->aLongMemTbl[0][0] != EOS) || (pFolProp->MemTbl[0] != EOS)) )
        {
          strcat (pszWorkSeg, "<li><b>Translation Memories to be searched: </b>");
          if ( pFolProp->aLongMemTbl[0][0] != EOS )
          {
              int i = 0;
              szTMs[0] = EOS;
              while ( (i < MAX_NUM_OF_READONLY_MDB) &&
                                (pFolProp->aLongMemTbl[i][0] != EOS) )
              {
                if ( szTMs[0] != EOS )        // not first dict in buffer?
                {
                        strcat( szTMs, "," );
                } /* endif */
                strcat( szTMs, pFolProp->aLongMemTbl[i] );
                i++;
              } /* endwhile */
          }
          else
          {
            strcpy( szTMs, pFolProp->MemTbl );
            {
                PSZ pszTemp = szTMs;
                while ( *pszTemp != EOS )
                {
                    if ( *pszTemp == X15 )
                    {
                        if ( pszTemp[1] == EOS )      // last X15 in string ???
                        {
                            *pszTemp = EOS;             //   yes -> discard X15
                            }
                            else
                            {
                                *pszTemp = COMMA;           //   no -> replace by comma
                            } /* endif */
                    } /* endif */
                    pszTemp++;
                } /* endwhile */
            }
          } /* endif */
          strcat(pszWorkSeg, szTMs);
        }
        if (fIsSubFolder)
        {
          strcat (pszWorkSeg, "<li><b>Translator: </b>");
          strcat (pszWorkSeg, pFolProp->szVendor);
          strcat (pszWorkSeg, "<li><b>Translator eMail: </b>");
          strcat (pszWorkSeg, pFolProp->szVendorEMail);
        }

        // MTI job data
        if ( !fIsSubFolder && pFolProp->fMTFieldsFilled )
        {
          sprintf( pszWorkSeg+strlen(pszWorkSeg), "<li><b>Total words processed for MT: %lu</b>",
                   pFolProp->ulMTTotalWords[0]+pFolProp->ulMTTotalWords[1]+pFolProp->ulMTTotalWords[2] );
          sprintf( pszWorkSeg+strlen(pszWorkSeg), "<li><b>Words send to MT server: %lu</b>",
            pFolProp->ulMTSendWords[0]+pFolProp->ulMTSendWords[1]+pFolProp->ulMTSendWords[2] );
          if ( pFolProp->fMTReceived )
          {
            sprintf( pszWorkSeg+strlen(pszWorkSeg), "<li><b>Words received from MT server: %lu</b>",
              pFolProp->ulMTReceivedWords[0]+pFolProp->ulMTReceivedWords[1]+pFolProp->ulMTReceivedWords[2] );
          } /* endif */
        } /* endif */

        strcat (pszWorkSeg,"</ul></body></html>");

        // ensure that national characters in names and description are
        // displayed correctly
        OEMTOANSI( pszWorkSeg);

        if (usRc != 0) fOk = FALSE;
    } // end if

   if (fOk)
   {
     usRc = UtlWriteL( hOutFile,
                       pszWorkSeg, strlen( pszWorkSeg ),
                       &ulBytesWritten, FALSE);
   }

  if ( hOutFile )
  {
      UtlClose( hOutFile, FALSE );
  } /* endif */

    return fOk;
 }  /* end of function EqfCopyFolderPropsToHtml  */
