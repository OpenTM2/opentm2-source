//+----------------------------------------------------------------------------+
//|EQFCNT00.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Stefan Doersam                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Object handler and instance for word count                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|T CountHandler_WP                                                           |
//|T COUNT_WP                                                                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals: Wct                                                              |
//|           UtlAlloc                                                         |
//|           UtlCreateChildResources                                          |
//|           UtlCreateChildSysMenu                                            |
//|           UtlOpen                                                          |
//|           UtlClose                                                         |
//|           UtlDelete                                                        |
//|           UtlWrite                                                         |
//|           UtlDispatch                                                      |
//|           UtlDefInstanceProc                                               |
//|           UtlTransAccel                                                    |
//|           EqfSend2AllObjects                                               |
//|           EqfSend2Handler                                                  |
//|           EqfQueryObject                                                   |
//|           EqfQueryActiveFolderHwnd                                         |
//|           EqfQueryObjectStatus                                             |
//|           EqfRemoveObject                                                  |
//|           EqfActivateInstance                                              |
//|           EqfQueryMenuTbl                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|  CountFrameSubProc                                                         |
//|  CreateWindow                                                              |
//|  CountOriginal                                                             |
//|  CountTranslation                                                          |
//|  WriteToFile                                                               |
//|  CreateColums                                                              |
//|  Handle_WM_EQF_COUNT                                                       |
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
// $Revision: 1.2 $ ----------- 29 Mar 2010
// GQ: - added handling for FUZZYMATCH_OPT in EqfCountWords
// 
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.2 $ ----------- 20 Jul 2009
// GQ: - support HTML output in wordcount API call
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.2 $ ----------- 1 Apr 2009
// GQ: - fixed P400172 Support "Count replace matches seperately"
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
// $Revision: 1.2 $ ----------- 31 Jul 2008
// GQ: - added handling for DUPMEMMATCH_OPT option
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
// $Revision: 1.3 $ ----------- 7 Nov 2006
// GQ: - added define INCL_EQF_TM
// 
// 
// $Revision: 1.2 $ ----------- 14 Aug 2006
// GQ: - added handling for XML output option
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
// $Revision: 1.2 $ ----------- 13 Jun 2005
// GQ: - fixed P021941: Launching word count twice does not cause error
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
// $Revision: 1.2 $ ----------- 12 Mar 2004
// GQ: - nonDDE WordCount: store parent object name and parent long name in CNT data structure
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
// $Revision: 1.3 $ ----------- 12 Mar 2003
// GQ: - fixed P016647: EQFCountWords: documents are not sorted correctly in word count file
// 
// 
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
// 
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.2 $ ----------- 5 Dec 2002
// GQ: - fixed P015011 nonDDE cannot handle file names containing the character comma
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.2 $ ----------- 6 Feb 2002
// RJ: KBT1120: del. redundant comp.def( _MATCHCOUNT, _LONGNAMES)
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.7 $ ----------- 2 Aug 2001
// GQ: Enable non-DDE word count for subfolders
//
//
// $Revision: 1.6 $ ----------- 17 Apr 2001
// GQ: Fixed PTM KBT1037 Non-DDE and additional drives
//
//
// $Revision: 1.5 $ ----------- 5 Oct 2000
// - enabled coded for subfolders
//
//
// $Revision: 1.4 $ ----------- 7 Feb 2000
// - added handling for long folder names
//
//
// $Revision: 1.3 $ ----------- 24 Jan 2000
// - fixed PTM KBT0642: Batch WordCount returns error 351
//
//
//
// $Revision: 1.2 $ ----------- 3 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFCNT00.CV_   1.16   29 Nov 1999 14:14:20   BUILD  $
 *
 * $Log:   K:\DATA\EQFCNT00.CV_  $
 *
 *    Rev 1.16   29 Nov 1999 14:14:20   BUILD
 * - added handling for long folder names in batch IF
 *
 *    Rev 1.15   01 Mar 1999 10:08:52   BUILD
 * - fixed some minor bugs in non-DDE batch mode word count
 *
 *    Rev 1.14   11 Jan 1999 16:07:40   BUILD
 * unlock files
 *
 *    Rev 1.13   30 Nov 1998 16:38:06   BUILD
 * enable for funccall if
 *
 *    Rev 1.12   30 Nov 1998 14:54:46   BUILD
 * - added code for function call interface word counting
 *
 *    Rev 1.11   13 Mar 1998 11:28:12   BUILD
 * - use new function UtlGetDDEErrorCode to get batch error code
 *
 *    Rev 1.10   09 Mar 1998 14:54:20   BUILD
 * - DDE: support multiple batch sessions
 *
 *    Rev 1.9   23 Feb 1998 11:56:38   BUILD
 * - use QS_LASTDDEMSGID instead of QS_LASTERRORMESSAGEID
 *
 *    Rev 1.8   14 Jan 1998 15:33:28   BUILD
 * - support Win32bit compile
 *
 *    Rev 1.7   10 Jul 1997 12:45:56   BUILD
 * ttr jpn077me wordcount fixed
 *
 *
 *    Rev 1.6   04 Jul 1997 09:12:38   BUILD
 * wordcount fix in batchmode: overwrite=no
 * and file already exists now unlocks all counting-files
 *
 *    Rev 1.5   23 Jun 1997 10:01:26   BUILD
 * - fixed PTM KBT0026: Wordcount wrong file exists message in batch mode
 *
 *    Rev 1.4   11 Jun 1997 17:53:16   BUILD
 * - implemented TM match counting
 * - fixed PTM KBT0019 EQFCMD problem with file names
 *
 *    Rev 1.3   11 Jun 1997 17:20:16   BUILD
 * - implemented TM match counting
 *
 *    Rev 1.2   23 Jul 1996 17:57:24   BUILD
 * - fixed PTM KAT0086: EQFCMD for wordcount does not work correctly
 *
 *    Rev 1.1   19 Feb 1996 09:14:42   BUILD
 * - fixed PTM KWT0301: Word count windows are not minimized/restored (problem
 *   was caused by using the wrong object name for the word count instance)
 *
 *    Rev 1.0   09 Jan 1996 09:04:02   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_WCOUNT           // word count functions
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdde.h"               // batch mode definitions
#ifdef FUNCCALLIF
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines
#endif
#include "eqfwcnti.h"             // Private include file for wordcount
#include <eqfcnt01.id>            //word count dialog id file
#include <eqffol00.h>             //subfolder stuff

USHORT CntBatchCount
(
  HWND             hwndParent,         // handle of count handler window
  PDDEWRDCNT       pWrdCnt             // wordcount data structure
);

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     WCntListHandlerCallBack                                  |
//+----------------------------------------------------------------------------+
//|Description  : Object handler and instance window procedure                 |
//|               for language update.                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Function flow: see documentation for Generic Handler layout                 |
//+----------------------------------------------------------------------------+
MRESULT WCntListHandlerCallBack
(
  PHANDLERCOMMAREA pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  hwnd; mp1;                           // get rid of compiler warnings

  switch ( msg )
  {
    /******************************************************************/
    /* WM_CREATE: fill variables of communication area                */
    /******************************************************************/
    case WM_CREATE :
      pCommArea->pfnCallBack          = WCntListCallBack;
      strcpy( pCommArea->szHandlerName, COUNTHANDLER );
      pCommArea->sBaseClass           = clsWORDCOUNT;
      pCommArea->sListWindowID        = ID_WORDCNT_WINDOW;
      pCommArea->sListboxID           = DID_CNT00_COUNTHANDLER_LB;

      /****************************************************************/
      /* Define object classes to be notified for EQFN messages       */
      /****************************************************************/
      pCommArea->asNotifyClassList[0] = clsWORDCOUNT;
      pCommArea->asNotifyClassList[1] = 0;       // end of list

      /****************************************************************/
      /* Define additional messages processed by the callback function*/
      /****************************************************************/
      pCommArea->asMsgsWanted[0]      = 0;       // end of list
      break;


    case WM_EQF_CREATE:
    {
      BOOL fOK;                        // success indicator
      HWND hFrame;                     // frame handle
      PCNT pCnt = NULL;                // pointer to count ida

      //--- mp1: SHORT1 = open flag  SHORT2 = NULL
      //--- mp2: object name

      //--- allocate storage for count structure passed to count dialog
      //--- and count instance, if fails display error message
      fOK =  UtlAlloc( (PVOID *) &pCnt, 0L, (ULONG)sizeof( CNT), ERROR_STORAGE ) ;

      if ( fOK )   //--- processing ok so far
      {
         //--- save folder object name to count structure
         strcpy( pCnt->szFolderObjName, (PSZ) mp2 );
         strcpy( pCnt->szParentObjName, (PSZ) mp2 );

         if ( FolIsSubFolderObject( pCnt->szFolderObjName ) )
         {
           // cut off subfolder name and property directory to form folder object name
           UtlSplitFnameFromPath( pCnt->szFolderObjName );
           UtlSplitFnameFromPath( pCnt->szFolderObjName );
         } /* endif */
         strcpy( pCnt->szFolderName, UtlGetFnameFromPath( pCnt->szFolderObjName ) );
         SubFolObjectNameToName( pCnt->szParentObjName, pCnt->szLongFolderName );

         //--- save count instance object name to count structure, that is :
         //--- COUNTHANDLER (#define) concatenated with folder objectname
         sprintf( pCnt->szCountInstanceObjName, "%s%s",
                  COUNTHANDLER, pCnt->szFolderObjName );

         /*********************************************************/
         /* Initialize morphologic language IDs                   */
         /*********************************************************/
         pCnt->sTgtLanguage = -1;
         pCnt->sSrcLanguage = -1;

         //--- if object with this object name and class count exists
         hFrame = EqfQueryObject( pCnt->szCountInstanceObjName, clsWORDCOUNT, 0 );
         if( hFrame )
         {
           // issue error message "count window already open"
           UtlError( ERROR_COUNT_WINDOW_OPEN, MB_CANCEL, 0, NULL, EQF_WARNING );

           //--- set focus to existing object
           ActivateMDIChild( hFrame );

           //--- set fOk to FALSE to stop processing
           fOK = FALSE;
         }/*endif*/
      }/*endif fOK*/

      if ( fOK )   //--- processing ok so far
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        INT_PTR iRc;

        DIALOGBOX( EqfQueryTwbClient(), WORDCOUNTDLGPROC,
                   hResMod, ID_WORDCNT_DLG, pCnt, iRc );

        if ( iRc > 0 )
        {
           fOK = CreateListWindow( pCnt->szCountInstanceObjName,
                                   WCntListCallBack, (PVOID) pCnt, FALSE );
        }
        else
        {
          /************************************************************/
          /* error happened or user canceled                          */
          /************************************************************/
          fOK = FALSE;
        } /* endif */
       }/*endif fOK*/


       if (!fOK )
       {
         //---- free count structure
         UtlAlloc( (PVOID *) &pCnt, 0L, 0L, NOMSG);
         mResult = MRFROMSHORT(TRUE);
       } /* endif */
      }
      break;

    case  WM_EQF_DDE_REQUEST:
      /************************************************************/
      /*     mp1:  (DDETASK) Task                                 */
      /*     mp2:  (PVOID) pTaskIda                               */
      /************************************************************/
      switch ( SHORT1FROMMP1( mp1 ) )
      {
        case  TASK_WORDCNT:
          {
            PDDEWRDCNT pWrdCnt = (PDDEWRDCNT)PVOIDFROMMP2(mp2);
            CntBatchCount( hwnd, pWrdCnt );
          }
          break;
        default :
          break;
      } /* endswitch */
      break;

    case WM_DESTROY:
      /****************************************************************/
      /* Nothing to do, as nothing has been allocated by the language */
      /* handler callback function                                    */
      /****************************************************************/
      break;


    default:
      break;
  } /* endswitch */
  return( mResult );
}

USHORT CntBatchCount
(
  HWND             hwndParent,         // handle of count handler window
  PDDEWRDCNT       pWrdCnt             // wordcount data structure
)
{
  BOOL             fIsNew = TRUE;
  CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
   HWND             hwndLB = NULLHANDLE;// handle of invisible listbox
   PSZ              pszParm;           // pointer for error parameters
   BOOL             fOK = TRUE;        // internal O.K. flag
   PCNT             pCnt;              // pointer to count structure
   OBJNAME          szFolObjName;      // buffer for folder object name
   SHORT  sIndex;                      // index of listbox items
   SHORT  sItemCount = 0;              // number of listbox items
   SHORT  sRc;                         //rc from WM_EQF_QUERYSYMBOL
   SHORT sItem;                        // index of listbox items
   USHORT   i;                         // loop index
   PPROPFOLDER  ppropFolder;           // pointer to folder properties
   HPROP        hpropFolder;           // folder properties handle
   ULONG        ulErrorInfo;           // error indicator from property handler
   OBJNAME      szFolObject;           // folder object name
   USHORT usRc;                        // retrun code from Utl... functions
   USHORT usAction;                    // action performed by UtlOpen
   PSZ    pszReplace;                  // error message parameter pointer
   HFILE  hfFile;                      // file handle

   /********************************************************************/
   /* Create invisible listbox for names of folder/documents/...       */
   /********************************************************************/
   hwndLB = WinCreateWindow( hwndParent, WC_LISTBOX, "",
                             LBS_MULTIPLESEL,
                             0, 0, 0, 0,
                             hwndParent, HWND_TOP,
                             DID_CNT00_COUNTHANDLER_LB,
                             NULL, NULL );

   /*******************************************************************/
   /* Allocate wordcount data area                                    */
   /*******************************************************************/
   fOK = UtlAllocHwnd( (PVOID *)&pCnt, 0L, (ULONG)sizeof(CNT), ERROR_STORAGE,
                       pWrdCnt->hwndErrMsg );

   /*******************************************************************/
   /* Check if folder exists                                          */
   /*******************************************************************/
   if ( fOK )
   {
     ObjLongToShortName( pWrdCnt->szFolder, szShortName, FOLDER_OBJECT, &fIsNew );
     if ( fIsNew )
     {
       fOK = FALSE;
       pszParm = pWrdCnt->szFolder;
       pWrdCnt->DDEReturn.usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
       UtlErrorHwnd( pWrdCnt->DDEReturn.usRc, MB_CANCEL, 1,
                     &pszParm, EQF_ERROR, pWrdCnt->hwndErrMsg );
     } /* endif */
   } /* endif */


   /*******************************************************************/
   /* Check if documents exist                                        */
   /*******************************************************************/
   if ( fOK )
   {
     /*****************************************************************/
     /* Build folder object name (access to folder properties is      */
     /* required to correct folder drive letter)                      */
     /*****************************************************************/
     {
       UtlMakeEQFPath( szFolObjName, NULC, SYSTEM_PATH, NULL );
       strcat( szFolObjName, BACKSLASH_STR );
       strcat( szFolObjName, szShortName );

       strcat( szFolObjName, EXT_FOLDER_MAIN );
       hpropFolder = OpenProperties( szFolObjName, NULL,
                                     PROP_ACCESS_READ, &ulErrorInfo);
       if( hpropFolder )
       {
         ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
         if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
         {
           szFolObjName[0] = ppropFolder->chDrive;
         } /* endif */
         CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
       } /* endif */
     }

     /******************************************************************/
     /* Fill our listbox with the names of the documents of the        */
     /* selected folder                                               */
     /******************************************************************/
     DELETEALLHWND( hwndLB );
     EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                      MP1FROMHWND( hwndLB ), MP2FROMP(szFolObjName)  );

     /******************************************************************/
     /* Search document names in listbox                               */
     /******************************************************************/
     i = 0;
     while ( fOK && (i < pWrdCnt->usFileNums) )
     {
       CHAR szDocShortName[MAX_FILESPEC];        // buffer for short doc name
       BOOL fIsNew = FALSE;

       FolLongToShortDocName( szFolObjName, pWrdCnt->ppFileArray[i],
                                szDocShortName, &fIsNew );

       if ( fIsNew )
       {
         sItem = -1;                   // trigger not-found condition
       }
       else
       {
         sItem = 1;                    // a dummy value ...
       } /* endif */

       if ( sItem < 0 )
       {
         fOK = FALSE;
         pszParm = pWrdCnt->ppFileArray[i];
         pWrdCnt->DDEReturn.usRc = ERROR_TA_SOURCEFILE;
         UtlErrorHwnd( pWrdCnt->DDEReturn.usRc, MB_CANCEL, 1,
                       &pszParm, EQF_ERROR, pWrdCnt->hwndErrMsg );
       } /* endif */
       i++;
     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* Fill count area data fields                                     */
   /*******************************************************************/
   if ( fOK )
   {
     pCnt->fBatch = TRUE;
     pCnt->hwndErrMsg = pWrdCnt->hwndErrMsg;
     pCnt->pDDEWrdCnt = pWrdCnt;

     strcpy( pCnt->szFolderObjName, szFolObjName );   // folder object name
     strcpy( pCnt->szFolderName, szShortName ); // folder name
     strcat( pCnt->szFolderName, EXT_FOLDER_MAIN );
     Utlstrccpy( pCnt->szLongFolderName, pCnt->szFolderName, DOT );
     ObjShortToLongName( pCnt->szLongFolderName, pCnt->szLongFolderName,
                         FOLDER_OBJECT );

     sprintf( pCnt->szCountInstanceObjName, "%s%s",   // count instance name
              COUNTHANDLER, pCnt->szFolderObjName );
     pCnt->hwndCountHandlerLB = hwndLB;
     strcpy( pCnt->szTitle, COUNTHANDLER );
     if ( pWrdCnt->fTMMatch )
     {
       pCnt->fOrg = TMMATCHES_STATE;
       pCnt->fTran = 0;
     }
     else
     {
       pCnt->fOrg = !pWrdCnt->fTarget;
       pCnt->fTran = pWrdCnt->fTarget;
     } /* endif */
     pCnt->fFile = TRUE;
   } /* endif */

   /*******************************************************************/
   /* Retrieve values from folder properties                          */
   /*******************************************************************/
   if ( fOK )
   {
      UtlMakeEQFPath( szFolObject, NULC, SYSTEM_PATH, NULL );
      strcat( szFolObject, BACKSLASH_STR );
      strcat( szFolObject, szShortName ); // folder name
      strcat( szFolObject, EXT_FOLDER_MAIN );

      fOK = FolQueryInfoHwnd( szFolObject, pCnt->szMemory, pCnt->szFormat, NULL,
                          NULL, TRUE, pWrdCnt->hwndErrMsg ) == NO_ERROR;
   } /* endif */

   /*******************************************************************/
   /* Insert documents into listbox                                   */
   /*******************************************************************/
   if ( fOK )
   {
     DELETEALLHWND( hwndLB );
     if ( pWrdCnt->usFileNums )
     {
       /***************************************************************/
       /* insert specified documents                                  */
       /***************************************************************/

       for ( i = 0; i < pWrdCnt->usFileNums; i++ )
       {
         CHAR szDocShortName[MAX_FILESPEC];        // buffer for short doc name
         BOOL fIsNew = FALSE;

         FolLongToShortDocName( szFolObjName, pWrdCnt->ppFileArray[i],
                                szDocShortName, &fIsNew );

         INSERTITEMHWND( hwndLB, szDocShortName );
       } /* endwhile */
     }
     else
     {
       /***************************************************************/
       /* insert all documents of folder                              */
       /***************************************************************/
       EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                        MP1FROMHWND( hwndLB ), MP2FROMP(pCnt->szFolderObjName) );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check if count files are in use                                 */
   /* Note: Files which are in use are skipped without further        */
   /*       notice                                                    */
   /*******************************************************************/
   if ( fOK )
   {


     sIndex = 0;
     sItemCount = QUERYITEMCOUNTHWND( hwndLB );

     while ( sIndex < sItemCount )
     {
       /***************************************************************/
       /* Get current item from listbox                               */
       /***************************************************************/
       QUERYITEMTEXTHWND( hwndLB, sIndex, pCnt->szFileName );

       /*****************************************************************/
       /* build object name (folder name concatenated with filename )   */
       /*****************************************************************/
       sprintf( pCnt->szFileObjName, "%s\\%s",
                pCnt->szFolderObjName, pCnt->szFileName );

       /*****************************************************************/
       /* check if symbol for this file already exists                  */
       /* i.e. file is in use                                           */
       /*****************************************************************/
       sRc = QUERYSYMBOL( pCnt->szFileObjName );
       if ( sRc != -1 )
       {
           /*************************************************************/
           /* flag file as locked in count handler listbox.             */
           /*************************************************************/
           SETITEMHANDLEHWND( hwndLB, sIndex, MP2FROMLONG(ITEM_LOCKED) );
       }
       else
       {
           /***************************************************************/
           /* file is not in use, lock it !                               */
           /***************************************************************/
           SETSYMBOL( pCnt->szFileObjName );

           /***************************************************************/
           /* flag file as unlocked in count handler listbox              */
           /***************************************************************/
           SETITEMHANDLEHWND( hwndLB, sIndex, NULL );
       } /* endif */

       sIndex++;                       // continue with next item
     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* Check output file name                                          */
   /*******************************************************************/
  if ( fOK && (UtlGetFnameFromPath( pWrdCnt->szOutFile ) == NULL) )
  {
    fOK = UtlInsertCurDir( pWrdCnt->szOutFile,
                           &(pWrdCnt->ppCurDirArray),pCnt->szOutputFile);
    if ( !fOK )
    {
      PSZ pszFileName = pWrdCnt->szOutFile;

      fOK = FALSE;                               // name is not valid
      UtlErrorHwnd( UTL_PARAM_TOO_LONG, MB_CANCEL, 1,
                    &pszFileName, EQF_ERROR, pWrdCnt->hwndErrMsg );
    } /* endif */
  }
  else
  {
    strcpy( pCnt->szOutputFile, pWrdCnt->szOutFile );
  } /* endif */
  strupr( pCnt->szOutputFile );

  if ( fOK )
  {
    CHAR szSysPath[MAX_EQF_PATH];

    //--- get system path
    UtlMakeEQFPath( szSysPath, NULC, SYSTEM_PATH, NULL );
    strcat( szSysPath, "\\" );

    //--- compare if target path contains as first directory the
    //--- system path
    if ( (strnicmp( pCnt->szOutputFile + 2, szSysPath + 2,
                    strlen(szSysPath) - 2 ) == 0) ||
         (strnicmp( pCnt->szOutputFile + 3, szSysPath + 3,
                    strlen(szSysPath) - 2 ) == 0) )
    {
       //--- display error message that this is not allowed
       PSZ pszReplace = szSysPath;
       UtlErrorHwnd( ERROR_EQF_PATH_INVALID, MB_CANCEL,
                     1, &pszReplace, EQF_ERROR, pWrdCnt->hwndErrMsg );
       //--- stop further processing
       fOK = FALSE;
    } /* endif */

    if ( fOK )
    {


       //--- check filename
       usRc = UtlOpen( pCnt->szOutputFile,      //filename
                       &hfFile,                       //file handle
                       &usAction,                     //action taken by Open
                       0L,                            //file size
                       FILE_NORMAL,                   //attribute  read/write
                       OPEN_ACTION_FAIL_IF_EXISTS |   //fail if exist
                       OPEN_ACTION_CREATE_IF_NEW,
                       OPEN_ACCESS_READONLY |         //open for read only
                       OPEN_SHARE_DENYREADWRITE,      //deny any other access
                       0L,                            //reserved, must be 0
                       FALSE );                       //do no error handling
       switch ( usRc )   //--- rc from UtlOpen
       {
          //-----------------------------------------------------------
          case ( ERROR_FILENAME_EXCED_RANGE ) :   //--- no valid filename
             //--- display error message that filename is no valid
             pszReplace = pCnt->szOutputFile;
             UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1,
                           &pszReplace, EQF_WARNING, pWrdCnt->hwndErrMsg );

             fOK = FALSE;
             break;
          //-----------------------------------------------------------
          case ( ERROR_PATH_NOT_FOUND ) :   //--- path does not exist
             //--- display error message path not exist
             UtlErrorHwnd( ERROR_PATH_NOT_EXIST, MB_CANCEL, 0, (PSZ *) NULP,
                           EQF_WARNING, pWrdCnt->hwndErrMsg );
             //--- stop further processing
             fOK = FALSE;
             break;
          //-----------------------------------------------------------
          case ( ERROR_NOT_READY ) :   //--- disk not ready
             //--- display error message disk not ready
             pszReplace = pCnt->szDriveLetter;
             UtlErrorHwnd( ERROR_NOT_READY_MSG, MB_CANCEL, 1,
                           &pszReplace, EQF_ERROR, pWrdCnt->hwndErrMsg );
             //--- stop further processing
             fOK = FALSE;
             break;
          //-----------------------------------------------------------
          case ( NO_ERROR ) :   //--- filename ok
             //--- close file, do not handle  error
             UtlClose( hfFile, FALSE );
             //--- delete file, do not handle  error
             UtlDelete( pCnt->szOutputFile, 0L, FALSE );
             break;
          //-----------------------------------------------------------
          case ( ERROR_FILE_EXISTS ) :   //--- file exists
          case ( ERROR_OPEN_FAILED ) :
            //--- display error message that file exists
            pszReplace = pCnt->szOutputFile;
            if ( pWrdCnt->fOverWrite )
            {
              if ( UtlDeleteHwnd( pCnt->szOutputFile, 0L, TRUE, pWrdCnt->hwndErrMsg ) )
              {
                fOK = FALSE;
              } /* endif */
            }
            else
            {
              UtlErrorHwnd( ERROR_FILE_EXISTS_ALREADY, MB_CANCEL,
                            1, &pszReplace, EQF_QUERY, pWrdCnt->hwndErrMsg );

              sIndex=0;
              while ( sIndex < sItemCount )
              {
                /*******************************************************/
                /* Get current item from listbox                       */
                /*******************************************************/
                QUERYITEMTEXTHWND( hwndLB, sIndex, pCnt->szFileName );

                /*******************************************************/
                /* build object name                                   */
                /*******************************************************/
                sprintf( pCnt->szFileObjName, "%s\\%s",
                pCnt->szFolderObjName, pCnt->szFileName );

                /*******************************************************/
                /* unlock file                                         */
                /*******************************************************/

                if(!(SHORT)QUERYITEMHANDLEHWND(hwndLB,sIndex))
                {
                    REMOVESYMBOL( pCnt->szFileObjName );
                }

                sIndex++;
              }/* end while */
              fOK = FALSE;
            } /* endif */
            break;
       } /* endswitch */
    } /* endif */
  } /* endif */

   /*******************************************************************/
   /* Start count process                                             */
   /*******************************************************************/
   if ( fOK )
   {
     fOK = CreateListWindow( pCnt->szCountInstanceObjName, WCntListCallBack,
                             (PVOID)pCnt, FALSE );
   } /* endif */

   // unlock files
   // ------------

   if ( fOK )
   {

     sIndex=0;
     while ( sIndex < sItemCount )
     {
       /*******************************************************/
       /* Get current item from listbox                       */
       /*******************************************************/
       QUERYITEMTEXTHWND( hwndLB, sIndex, pCnt->szFileName );

       /*******************************************************/
       /* build object name                                   */
       /*******************************************************/
       sprintf( pCnt->szFileObjName, "%s\\%s",
       pCnt->szFolderObjName, pCnt->szFileName );

       /*******************************************************/
       /* unlock file                                         */
       /*******************************************************/

       if(!(SHORT)QUERYITEMHANDLEHWND(hwndLB,sIndex))
       {
           REMOVESYMBOL( pCnt->szFileObjName );
       }

       sIndex++;
     }/* end while */

   } /* endif */


   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( !fOK )
   {
      if ( pCnt   != NULL ) UtlAlloc( (PVOID *) &pCnt, 0L, 0L, NOMSG) ;

      /****************************************************************/
      /* report end of task to DDE handler                            */
      /****************************************************************/
      WinPostMsg( pWrdCnt->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                  &pWrdCnt->DDEReturn );
   } /* endif */

   if ( !fOK ) pWrdCnt->DDEReturn.usRc = UtlGetDDEErrorCode( pWrdCnt->hwndErrMsg );

   return( pWrdCnt->DDEReturn.usRc );

} /* end of function CntBatchCount */

// perform the function call interface word count
USHORT CntFuncCountWords
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  LONG        lOptions,                // options for word count
  PSZ         pszOutFile               // name of output file
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new word count or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new word count run
    usRC = CntFuncPrepCount( pData, pszFolderName, pszDocuments,
                             lOptions, pszOutFile );
  }
  else
  {
    // continue current word count process
    usRC = CntFuncCountProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function CntFuncCount */

// compare two strings (used by qsort)
int CntDocNameCompare( const void *pElem1, const void *pElem2 )
{
  int iResult = 0;
  PSZ pszName1 = (PSZ)*((PSZ *)pElem1);
  PSZ pszName2 = (PSZ)*((PSZ *)pElem2);
  PSZ pszExt1, pszExt2, pszShort1, pszShort2;

  // remove document short name
  pszShort1 = strrchr( pszName1, X15 );
  pszShort2 = strrchr( pszName2, X15 );
  if ( pszShort1 ) *pszShort1 = EOS;
  if ( pszShort2 ) *pszShort2 = EOS;

  // remove any file extension first (GUI sorts 1st on file name, 2nd on extension)
  pszExt1 = strrchr( pszName1, DOT );
  pszExt2 = strrchr( pszName2, DOT );
  if ( pszExt1 ) *pszExt1 = EOS;
  if ( pszExt2 ) *pszExt2 = EOS;

  // compare file name part first
  iResult = _stricoll( pszName1, pszName2 );

  // if same, use extension for compare
  if ( iResult == 0 )
  {
    if ( pszExt1 == NULL )
    {
      iResult = -1;
    }
    else if ( pszExt2 == NULL )
    {
      iResult = 1;
    } 
    else
    {
      iResult = stricmp( pszExt1 + 1, pszExt2 + 1);
    } /* endif */
  } /* endif */

  // restore file extensions and short names
  if ( pszExt1 ) *pszExt1 = DOT;
  if ( pszExt2 ) *pszExt2 = DOT;
  if ( pszShort1 ) *pszShort1 = X15;
  if ( pszShort2 ) *pszShort2 = X15;
  
  return( iResult );
} /* end of function CntDocNameCompare */

// prepare the function I/F word count
USHORT CntFuncPrepCount
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  LONG        lOptions,                // options for word count
  PSZ         pszOutFile               // name of output file
)
{
  PSZ         pszParm;                 // pointer for error parameters
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  USHORT      usDocuments = 0;         // number of documents being analyzed
  PSZ         pDocNameBuffer = NULL;   // document name buffer
  LONG        lBufferSize = 0L;        // size of document buffer
  LONG        lBufferUsed = 0L;        // used bytes in document buffer
  OBJNAME     szFolObject;             // folder object name
  PCNT        pCnt = NULL;             // pointer to count structure
  PSZ         pszReplace;              // ptr to error message parameter
  PCNTIDA     pCNTIda = NULL;
  BOOL             fIsNew = TRUE;
  CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
  PSZ         pDocumentListBuffer = NULL; // buffer for list of documents

   // allocate storage for word count IDA and count data area
   if ( fOK )
   {
     fOK = UtlAlloc( (PVOID *)&pCnt, 0L, (LONG)sizeof(*pCnt), NOMSG );
     if ( fOK )
     {
       fOK = UtlAlloc( (PVOID *)&pCNTIda, 0L, (ULONG)sizeof(CNTIDA), NOMSG );
     } /* endif */

     if ( fOK )
     {
        pCNTIda->pCnt = pCnt;
        pCNTIda->pCnt->sTgtLanguage = -1;
        pCNTIda->pCnt->sSrcLanguage = -1;
     }
     else
     {
       usRC = ERROR_STORAGE;
       UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // check if folder exists
   if ( fOK )
   {
     if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
     {
       fOK = FALSE;
       usRC = TA_MANDFOLDER;
       UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
     }
     else
     {
       fIsNew = !SubFolNameToObjectName( pszFolderName,  pData->szObjName );
       strcpy( pCnt->szParentObjName, pData->szObjName );
       strcpy( pCnt->szDisplayName, pszFolderName );

       if ( !fIsNew )
       {
         PSZ pszDelim;
         strcpy( pData->szObjName, pszFolderName );
         pszDelim = strchr( pData->szObjName, BACKSLASH );
         if ( pszDelim ) *pszDelim = EOS;
         ObjLongToShortName( pData->szObjName, szShortName, FOLDER_OBJECT, &fIsNew );

         // store main folder object name in count area
         if ( !fIsNew ) 
         { 
           SubFolNameToObjectName( pData->szObjName, pCnt->szFolderObjName );
         } /* endi f*/
       } /* endif */

       if ( fIsNew )
       {
        fOK = FALSE;
        pszParm = pszFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
       } /* endif */
     } /* endif */
   } /* endif */

   // check if documents exist
   if ( fOK && (pszDocuments != NULL) && (*pszDocuments != EOS))
   {
     // build folder object name (access to folder properties is
     // required to correct folder drive letter)
     {
       PPROPFOLDER  ppropFolder;        // pointer to folder properties
       HPROP        hpropFolder;        // folder properties handle
       ULONG        ulErrorInfo;        // error indicator from property handler

       UtlMakeEQFPath( pData->szObjName, NULC, SYSTEM_PATH, NULL );
       strcat( pData->szObjName, BACKSLASH_STR );
       strcat( pData->szObjName, szShortName );
       strcat( pData->szObjName, EXT_FOLDER_MAIN );
       hpropFolder = OpenProperties( pData->szObjName, NULL,
                                     PROP_ACCESS_READ, &ulErrorInfo);
       if( hpropFolder )
       {
         ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
         if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
         {
           pData->szObjName[0] = ppropFolder->chDrive;
         } /* endif */
         CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
       } /* endif */
     }

     // allocate buffer for specified documents names as the list is modified
     // during processing
     if ( fOK && (*pszDocuments != EOS))
     {
       int iLen = max( (strlen(pszDocuments)+2), 512 );

       fOK = UtlAllocHwnd( (PVOID *)&pDocumentListBuffer , 0L, iLen, ERROR_STORAGE, HWND_FUNCIF );
     } /* endif */

     if ( fOK && (*pszDocuments != EOS))
     {
       strcpy( pDocumentListBuffer, pszDocuments );
     } /* endif */

     if ( fOK && (*pszDocuments != EOS))
     {
       PSZ pszCurrent = pDocumentListBuffer;
       PSZ pszNext = NULL;
       BOOL fIsNew = FALSE;

       while ( UtlGetNextFileFromCommaList( &pszCurrent, &pszNext ) )
       {
         CHAR szDocShortName[MAX_FILESPEC];

         FolLongToShortDocName( pData->szObjName, pszCurrent,
                                szDocShortName, &fIsNew );

         if ( fIsNew )
         {
           fOK = FALSE;
           pszParm = pszCurrent;
           usRC = ERROR_TA_SOURCEFILE;
           UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
         }
         else
         {
           // add document short name to document name buffer
           LONG lAddLen = strlen(szDocShortName) + 1;
           if ( lBufferSize < (lBufferUsed + lAddLen) )
           {
             UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                           lBufferSize + 8096L, ERROR_STORAGE, HWND_FUNCIF );
             lBufferSize += 8096L;
           } /* endif */

           if ( pDocNameBuffer != NULL )
           {
             strcpy( pDocNameBuffer + lBufferUsed, szDocShortName );
             lBufferUsed += lAddLen;
           } /* endif */
           usDocuments++;
         } /* endif */
       } /* endwhile */
     } /* endif */
   } /* endif */

   // process specified options
   if ( fOK )
   {
     if ( lOptions & DUPMEMMATCH_OPT )
     {
       pCnt->fOrg = DUPLICATE_STATE;
       pCnt->fInclMemMatch = TRUE;
       pCnt->fTran = FALSE;
     }
     else if ( lOptions & FUZZYMATCH_OPT )
     {
       pCnt->fOrg = FUZZYMATCH_STATE;
       pCnt->fTran = FALSE;
     }
     else if ( lOptions & DUPLICATE_OPT )
     {
       pCnt->fOrg = DUPLICATE_STATE;
       pCnt->fInclMemMatch = FALSE;
       pCnt->fTran = FALSE;
     }
     else if ( lOptions & TMMATCH_OPT )
     {
       pCnt->fOrg = TMMATCHES_STATE;
       pCnt->fReplMatch = (lOptions & SEPERATEREPLMATCH_OPT) != 0;
       pCnt->fTran = FALSE;
     }
     else
     {
       pCnt->fOrg = ((lOptions & SOURCE_OPT) != 0L);
       pCnt->fTran = ((lOptions & TARGET_OPT) != 0L);
     } /* endif */
     pCnt->fFile = TRUE;
     if ( lOptions & XML_OUTPUT_OPT)
     {
       pCnt->fXMLOutput = TRUE;
       pCnt->usFormat = XML_FORMAT;
     }
     else if ( lOptions & HTML_OUTPUT_OPT)
     {
       pCnt->fXMLOutput = TRUE;
       pCnt->usFormat = HTML_FORMAT;
     }
     else 
     {
       pCnt->fXMLOutput = FALSE;
       pCnt->usFormat = ASCII_FORMAT;
     } /* endif */
   } /* endif */

   // get number of documents if no specific documents have been specified
   if ( fOK )
   {
     // get folder object name
     SubFolNameToObjectName( pszFolderName, szFolObject );
     if ( usDocuments == 0 )
     {
       usDocuments = LoadDocumentNames( szFolObject, HWND_FUNCIF,
                                        LOADDOCNAMES_INCLSUBFOLDERS,
                                        (PSZ)&pDocNameBuffer );
     } /* endif */
   } /* endif */

  if ( fOK )
  {
    if ( (pszOutFile == NULL) || (*pszOutFile == EOS) )
    {
      fOK = FALSE;                               // name is not valid
      UtlErrorHwnd( ERROR_NO_EXPORT_NAME, MB_CANCEL, 0, NULL,
                    EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      strcpy( pCnt->szOutputFile, pszOutFile );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    CHAR szSysPath[MAX_EQF_PATH];

    //--- get system path
    UtlMakeEQFPath( szSysPath, NULC, SYSTEM_PATH, NULL );
    strcat( szSysPath, "\\" );

    //--- compare if target path contains as first directory the
    //--- system path
    if ( (strnicmp( pCnt->szOutputFile + 2, szSysPath + 2,
                    strlen(szSysPath) - 2 ) == 0) ||
         (strnicmp( pCnt->szOutputFile + 3, szSysPath + 3,
                    strlen(szSysPath) - 2 ) == 0) )
    {
       //--- display error message that this is not allowed
       PSZ pszReplace = szSysPath;
       UtlErrorHwnd( ERROR_EQF_PATH_INVALID, MB_CANCEL,
                     1, &pszReplace, EQF_ERROR, HWND_FUNCIF );
       //--- stop further processing
       fOK = FALSE;
    } /* endif */

    if ( fOK )
    {
       //--- check filename
       HFILE  hfFile = NULLHANDLE;
       USHORT usAction;
       usRC = UtlOpen( pCnt->szOutputFile,      //filename
                       &hfFile,                       //file handle
                       &usAction,                     //action taken by Open
                       0L,                            //file size
                       FILE_NORMAL,                   //attribute  read/write
                       OPEN_ACTION_FAIL_IF_EXISTS |   //fail if exist
                       OPEN_ACTION_CREATE_IF_NEW,
                       OPEN_ACCESS_READONLY |         //open for read only
                       OPEN_SHARE_DENYREADWRITE,      //deny any other access
                       0L,                            //reserved, must be 0
                       FALSE );                       //do no error handling
       switch ( usRC )   //--- rc from UtlOpen
       {
          //-----------------------------------------------------------
          case ( ERROR_FILENAME_EXCED_RANGE ) :   //--- no valid filename
             //--- display error message that filename is no valid
             pszReplace = pCnt->szOutputFile;
             UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1,
                           &pszReplace, EQF_WARNING, HWND_FUNCIF );
             fOK = FALSE;
             break;
          //-----------------------------------------------------------
          case ( ERROR_PATH_NOT_FOUND ) :   //--- path does not exist
             //--- display error message path not exist
             UtlErrorHwnd( ERROR_PATH_NOT_EXIST, MB_CANCEL, 0, (PSZ *) NULP,
                           EQF_WARNING, HWND_FUNCIF );
             //--- stop further processing
             fOK = FALSE;
             break;
          //-----------------------------------------------------------
          case ( ERROR_NOT_READY ) :   //--- disk not ready
             //--- display error message disk not ready
             pszReplace = pCnt->szDriveLetter;
             UtlErrorHwnd( ERROR_NOT_READY_MSG, MB_CANCEL, 1,
                           &pszReplace, EQF_ERROR, HWND_FUNCIF );
             //--- stop further processing
             fOK = FALSE;
             break;
          //-----------------------------------------------------------
          case ( NO_ERROR ) :   //--- filename ok
             //--- close file, do not handle  error
             UtlClose( hfFile, FALSE );
             //--- delete file, do not handle  error
             UtlDelete( pCnt->szOutputFile, 0L, FALSE );
             break;
          //-----------------------------------------------------------
          case ( ERROR_FILE_EXISTS ) :   //--- file exists
          case ( ERROR_OPEN_FAILED ) :
            //--- display error message that file exists
            pszReplace = pCnt->szOutputFile;
            usRC = NO_ERROR;
            if ( lOptions & OVERWRITE_OPT )
            {
              if ( UtlDeleteHwnd( pCnt->szOutputFile, 0L, TRUE, HWND_FUNCIF ) )
              {
                fOK = FALSE;
              } /* endif */
            }
            else
            {
              UtlErrorHwnd( ERROR_FILE_EXISTS_ALREADY, MB_CANCEL,
                            1, &pszReplace, EQF_QUERY, HWND_FUNCIF );
              fOK = FALSE;
            } /* endif */
            break;
       } /* endswitch */
    } /* endif */
  } /* endif */

   // fill word count data structure
   if ( fOK )
   {
     pData->pvWordCountData           = pCNTIda;
   } /* endif */

   // Fill count area data fields
   if ( fOK )
   {
     pCnt->fBatch = TRUE;
     pCnt->hwndErrMsg = HWND_FUNCIF;
     pCnt->pDDEWrdCnt = NULL;

     strcpy( pCnt->szDisplayName, pszFolderName ); // folder name for display
     strcpy( pCnt->szFolderName, szShortName ); // folder name
     strcat( pCnt->szFolderName, EXT_FOLDER_MAIN );
     Utlstrccpy( pCnt->szLongFolderName, pCnt->szFolderName, DOT );
     ObjShortToLongName( pCnt->szLongFolderName, pCnt->szLongFolderName,
                         FOLDER_OBJECT );
     sprintf( pCnt->szCountInstanceObjName, "%s%s",   // count instance name
              COUNTHANDLER, pCnt->szFolderObjName );
     strcpy( pCnt->szTitle, COUNTHANDLER );
     pCnt->usNoOfDocs     = usDocuments;
     pCnt->pDocNameBuffer = pDocNameBuffer;
   } /* endif */

   // ensure that document names are in the same sort order 
   // as in the GUI (fix for P016647)
   // In the GUI the documents are sorted using their long names
   // in the nonDDE-IF the documents are sorted using the short names
   // (same order as returned by DIR command)
   if ( fOK )
   {
     CHAR szLongAndShortName[MAX_LONGFILESPEC+MAX_FILESPEC+10];
     PSZ  *apszName = NULL;
     PSZ  pszDoc = pCnt->pDocNameBuffer;
     int  iDoc = 0;

     // create string pool for document long and short names
     PPOOL pPool = PoolCreate( 10000 );
     if ( pPool == NULL )
     {
       UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
       fOK = FALSE;
     } /* endif */

     // allocate array for document long name pointers and references to
     // document short names
     fOK = UtlAllocHwnd( (PVOID *)&apszName, 0L, sizeof(PSZ)*pCnt->usNoOfDocs, 
                         ERROR_STORAGE, HWND_FUNCIF );
     
     while ( fOK && (*pszDoc != EOS) )
     {
       // get document long names and add to table
       CHAR szDocObjName[MAX_EQF_PATH];

       // get document long name
       UtlMakeEQFPath( szDocObjName, pCnt->szFolderObjName[0], SYSTEM_PATH, pCnt->szFolderName );
       strcat( szDocObjName, BACKSLASH_STR );
       strcat( szDocObjName, pszDoc ); 
       DocQueryInfoEx( szDocObjName, NULL, NULL, NULL, NULL, szLongAndShortName,
                       NULL, NULL, NULL, NULL, NULL, FALSE, HWND_FUNCIF );
       if ( szLongAndShortName[0] == EOS )
       {
         strcpy( szLongAndShortName, pszDoc );
       }
       else
       {
         OEMTOANSI( szLongAndShortName );
       } /* endif */

       strcat( szLongAndShortName, X15_STR );
       strcat( szLongAndShortName, pszDoc );

       // add long and short name to string pool and remember ptr to it
       apszName[iDoc++] = PoolAddString( pPool, szLongAndShortName );

       // continue with next document
       pszDoc += strlen(pszDoc) + 1;
     } /* endwhile */

     // sort long name table
     if ( fOK )
     {
       qsort( apszName, iDoc, sizeof(PSZ), CntDocNameCompare );
     } /* endif */

     // fill short name buffer in sort order of document long names
     if ( fOK )
     {
       PSZ pszDoc = pCnt->pDocNameBuffer;

       int i = 0;
       for ( i = 0; i < iDoc; i++ )
       {
         PSZ pszShortName = strrchr( apszName[i], X15 ) + 1;
         strcpy( pszDoc, pszShortName );
         pszDoc += strlen(pszDoc) + 1;
       } /* endfor */
       *pszDoc = EOS;
     } /* endif */

     // cleanup
     if ( pPool ) PoolDestroy( pPool ); 
     if ( apszName ) UtlAlloc( (PVOID *)&apszName, 0L, 0L, NOMSG );
   } /* endif */

   // Retrieve values from folder properties
   if ( fOK )
   {
      UtlMakeEQFPath( szFolObject, NULC, SYSTEM_PATH, NULL );
      strcat( szFolObject, BACKSLASH_STR );
      strcat( szFolObject, szShortName );
      strcat( szFolObject, EXT_FOLDER_MAIN );

      fOK = FolQueryInfoHwnd( szFolObject, pCnt->szMemory, pCnt->szFormat, NULL,
                          NULL, TRUE, HWND_FUNCIF ) == NO_ERROR;
   } /* endif */

   // cleanup
   if ( !fOK )
   {
      if ( pDocNameBuffer ) UtlAlloc( (PVOID *)&pDocNameBuffer, 0L, 0L, NOMSG );
      if ( pCnt )           UtlAlloc( (PVOID *)&pCnt, 0L, 0L, NOMSG );
      if ( pCNTIda )        UtlAlloc( (PVOID *)&pCNTIda, 0L, 0L, NOMSG );
   } /* endif */


   if ( fOK )
   {
     pData->fComplete = FALSE;
     pData->usWordCountPhase = FCTPHASE_INIT;
   }
   else
   {
     usRC = UtlQueryUShort( QS_LASTERRORMSGID );
   } /* endif */

   if ( pDocumentListBuffer ) UtlAlloc( (PVOID *)&pDocumentListBuffer, 0L, 0L, NOMSG );
   return( usRC );

} /* end of function CntFuncCount */
