//------------------------------------------------------------------------------
//EQFMEMIP.C                                                                  
//------------------------------------------------------------------------------
//Copyright Notice:                                                           
//                                                                            
//      Copyright (C) 1990-2015, International Business Machines              
//      Corporation and others. All rights reserved                           
//------------------------------------------------------------------------------
//Description: Import of Translation Memory in Troja Format                   
// The function EQFMemImportTrojaStart checks the case of import              
// witch has to be performed and its feasibility. If no merge of              
// translation memories is required then the import is performed              
// immediately. If a merge is required a WM_EQF_MEMMERGE_START                
// message is posted to initiate the merge process.                           
// The function EQFMemImportTrojaEnd performs the cleanup of the              
// import which means that the imported files will be deleted.                
//------------------------------------------------------------------------------

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "MemoryFactory.h"

#include "EQFDDE.H"               // Batch mode definitions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFSETUP.H>             // directory names

//  Import logic:
//              Check whether properties of TM to be imported exist
//                  no                                   yes
//  ------------------------------------Å-------------------------------------------
//  Check if a TM with equal name and              Check if TM-ID's match
//  equal ID exists on the same drive as        no                   yes
//  the TM to be imported or user want  +---------------------Å---------------------
//  to import(merge) anyway              display question if 
//           ja                no       merge anyway.        Check space on disk
//  -----------------------Å------------+ if answer is         and issue message if
//   Check space on disk                YES      NO         necessary.
//   and issue message if             +---------Å-----------+  Abort        Go
//   necessary.                       post msg End with:  +----------Å------------
//    Abort        Go                to start TM_IMPORT_ End with: Post message
//  ----------Å------------+           merge    FAILED     TM_IMPORT_to start
//  End with: Create                                     FAILED    merge
//  TM_IMPORT_properties                                 +----------Á------------
//  FAILED    and correct             +---------Á-----------+
//  ----------+them.        Check if a TM with equal name and
//            Post message equal ID exists on a other drive as
//            to start     the TM to be imported or user want to be anyway
//            merge                 ja                no
//            +------------+ -----------------------Å--------------------------
//                            Check space on disk    Create properties
//                            and issue message if   and correct them.
//                            necessary.            
//                             Abort        Go      Rename translation memory
//                           ----------Å------------+   did an error occure ?
//                           End with: Create           no           yes
//                           TM_IMPORT_properties  +-------------Å-------------
//                           FAILED    and correct  End with:    Issue error.
//                           ----------+them.        TM_IMPOR_OK  End with:
//                                     Post message              TM_IMPORT_
//                                     to start                  FAILED
//                                     merge                    
//                                     +------------Á-------------Á-------------

USHORT CheckSpaceStartMerge( HWND hWnd, PMEM_MERGE_IDA pMIDA );
USHORT SearchAndOpenEqualMem( PMEM_MERGE_IDA pMIDA, PSZ szDriveList, USHORT usLocation );
USHORT CreatePropFromImportProp( PMEM_MERGE_IDA pMIDA, BOOL fUpdateTMList );

BOOL NTMCreatePropandIndexFromImport ( PMEM_MERGE_IDA, BOOL );
BOOL NTMGetOldImportInfo( PMEM_MERGE_IDA pMIDA );

// ================ Check the case of Import and handle it ========================

USHORT EQFMemImportTrojaStart
(
  HWND       hWnd,
  LPARAM     mp2,               // mp2 points to a string containing the name of the
                                // invoking handler and the full
                                // path of the translation memory to be imported
  HWND       hwndErrMsg )       // parent for error message boxes
{
    PMEM_MERGE_IDA    pMIDA;          // Pointer to the merge IDA
    USHORT            usRc = TRUE;    // Return code to control the process
    USHORT            usDosRc;        // Return code from Dos functions
    PSZ               pReplAddr[1];   // Pointer to an address list of replacement strings
    EQFINFO           ErrorInfo;      // Property handler error info
    CHAR *            pszTemp;        // Temporary character pointer
    USHORT            usLocation;     // TM location
    BOOL              fPropExist = FALSE;
    
    // Allocate storage for the merge IDA
    usRc = (USHORT)UtlAllocHwnd( (PVOID *)&pMIDA, 0L, (LONG)sizeof(MEM_MERGE_IDA),
                         ERROR_STORAGE, hwndErrMsg );

    OtmMemoryPlugin::PMEMORYINFO pInfo = new( OtmMemoryPlugin::MEMORYINFO );
    if(pInfo==NULL)
		usRc = !usRc;

    // --------------------------------------------------------------------
    // Check if properties with the same name exist
    if ( usRc )
    {
      // Indicate in the MIDA that it is created via an import process
      pMIDA->fImport = TRUE;

      // Remember parent handler for error message box
      pMIDA->hwndErrMsg = hwndErrMsg;

      // Copy the full path of the translation memory to be merged
      // into the merge IDA and uppercase it
      strcpy( (PSZ)pMIDA->szPathMergeMem,
              UtlParseX15( (PSZ)PVOIDFROMMP2(mp2), 1 ) );
      strupr( pMIDA->szPathMergeMem );
      /******************************************************************/
      /* copy the index file from import directory                      */
      /******************************************************************/
      Utlstrccpy( pMIDA->szPathMergeIndex, pMIDA->szPathMergeMem, DOT );
      strcat( pMIDA->szPathMergeIndex, EXT_OF_TMINDEX );

      // Copy the invoking handler name to the merge IDA. The handler name will be
      // used to send the WM_EQFN_TASKDONE notification message at the end of the process
      strcpy( (PSZ)pMIDA->szInvokingHandler,
              UtlParseX15( (PSZ)PVOIDFROMMP2(mp2), 0 ) );

      // Create the full path to the translation memory property
      // to be merged
      {
        strcpy( pMIDA->szPathMergeProp, pMIDA->szPathMergeMem );

        // search for MEM and replace it with
        // PROPERTY
        sprintf( pMIDA->szTemp, "\\%s\\", MEMDIR );
        pszTemp = strstr( pMIDA->szPathMergeProp, pMIDA->szTemp );
        pszTemp++;
        strcpy( pszTemp, PROPDIR );
      } /* endif */

      if (usRc)
      {
        // Prepare the data to open the properties
        _splitpath( pMIDA->szPathMergeMem,
                    pMIDA->szDriveMergeMem,
                    pMIDA->szDirMergeMem,
                    pMIDA->szNameMergeMem,
                    pMIDA->szExtMergeMem );
        pMIDA->szDriveMergeMem[1] = NULC;
        strcpy( pMIDA->szNameMem, pMIDA->szNameMergeMem );

        // Complete the property path by adding the \ and the name + ext
        strcat( pMIDA->szPathMergeProp, BACKSLASH_STR );
        strcat( pMIDA->szPathMergeProp, pMIDA->szNameMergeMem );
        strcat( pMIDA->szPathMergeProp, EXT_OF_TMPROP );

        // load the properties of the TM being imported to get access to any long TM name
        {
          PPROP_NTM pMemProp = NULL;
          ULONG  ulBytesRead;
          if( UtlLoadFileL( pMIDA->szPathMergeProp, (PVOID *)&pMemProp,
                           &ulBytesRead, FALSE, FALSE ) )
          {
            if ( pMemProp->szLongName[0] != EOS )
            {
              BOOL fIsNew = FALSE;         // new flag

              ObjLongToShortName( pMemProp->szLongName, pMIDA->szNameMem, TM_OBJECT, &fIsNew );
              strcpy( pMIDA->szShortNameMem, pMIDA->szNameMem );
              strcpy( pMIDA->szLongName, pMemProp->szLongName );
            } /* endif */
            UtlAlloc( (PVOID *)&pMemProp, 0L, 0L, NOMSG );
          } /* endif */
        }

        // Prepare the data to open the properties
        strcpy( pMIDA->szPropName, pMIDA->szNameMem );
        strcat( pMIDA->szPropName, EXT_OF_TMPROP );
        UtlMakeEQFPath( pMIDA->szSystemPath, NULC, SYSTEM_PATH, NULL );

        // Open the properties
		// an empty function, replace it with getMemoryInfo
        // usRc = NTMOpenProperties( &hProp,
        //                            (PVOID *)&pProp,
        //                            pMIDA->szPropName,
        //                            pMIDA->szSystemPath,
        //                            PROP_ACCESS_READ,
        //                            FALSE );
		MemoryFactory *pFactory = MemoryFactory::getInstance();
        int iRC = pFactory->getMemoryInfo( NULL, pMIDA->szNameMem, pInfo );
		usRc = iRC==0?0:1;

        if ( usRc == NO_ERROR || usRc == ERROR_OLD_PROPERTY_FILE )
        {
          /************************************************************/
          /* set property exist and old property flag                 */
          /************************************************************/
          fPropExist = pMIDA->fPropExist = TRUE;
          if ( usRc == ERROR_OLD_PROPERTY_FILE )
          {
            usRc = FALSE;
            pMIDA->fOldPropFile = TRUE;
            // Prepare the path of the TM
            UtlMakeEQFPath( pMIDA->szPathMem, NULC, MEM_PATH, NULL );
            sprintf( pMIDA->szPathMem, "%s\\%s%s", pMIDA->szPathMem,
                                                   pMIDA->szNameMergeMem,
                                                   EXT_OF_MEM );
          }
          else if ( pFactory->isSharedMemory(pMIDA->szNameMem))
          {
            // Prepare the path of the TM
            UtlMakeEQFPath( pMIDA->szPathMem, NULC, MEM_PATH, NULL );
            sprintf( pMIDA->szPathMem, "%s\\%s%s", pMIDA->szPathMem,
                                                   pMIDA->szNameMem,
                                                   EXT_OF_SHARED_MEM );
            pMIDA->fOldPropFile = FALSE;
            usRc = TRUE;
          }
          else
          {
            // Prepare the path of the TM
            UtlMakeEQFPath( pMIDA->szPathMem, NULC, MEM_PATH, NULL );
            sprintf( pMIDA->szPathMem, "%s\\%s%s", pMIDA->szPathMem,
/* GQ Fix for KBT1080 */                           pMIDA->szNameMem, // pMIDA->szNameMergeMem,
                                                   EXT_OF_TMDATA );
            pMIDA->fOldPropFile = FALSE;
            usRc = TRUE;
          } /* endif */
        }
        else
        {
          fPropExist = pMIDA->fPropExist = FALSE;
        } /* endif */

        if ( fPropExist )
        {
          // get the drive letter and the server of the TM from the properties
          // and close the properties
          if ( pMIDA->fOldPropFile )
          {
            pMIDA->szPathMem[0]  = pInfo->szFullPath[0];
            pMIDA->szDriveMem[0] = pInfo->szFullPath[0];
            strcpy( pMIDA->szServer, EMPTY_STRING );
          }
          else
          {
            pMIDA->szPathMem[0]  = pInfo->szFullPath[0];
            pMIDA->szDriveMem[0] = pInfo->szFullPath[0];
            strcpy( pMIDA->szServer, EMPTY_STRING );
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    // ----------------------------------------------------------------------
    // If the properties for the TM to be imported exists
    if ( fPropExist && usRc )
    {
      usLocation = TM_LOCAL;
      // Invoke the utility which checks the existence of the same TM.
      // If a TM with the same name equal id is found, the TM's will
      // be opened and the handles will be returned.
      // If a TM with the same name but different id is found, the user is
      // prompted for confirmation of merge.
      // if merge should be done anyway, it is handled like an equal TM

      usRc = SearchAndOpenEqualMem( pMIDA,                   // Address of merge IDA
                                    pMIDA->szDriveMem,       // Drive to be searched
                                    usLocation );
      if ( usRc )
      {
        if ( pMIDA->pOutputMem != NULL )
        {
          // --- An TM with same id exists, or TMs should be merged anyway
          // --- Check if enough space for the merge and start merge if OK
          // --- else return FALSE.
          // ask user if TMs should be merged
          if ( !ISBATCHHWND(hwndErrMsg) )
          {
            USHORT usMBCode;           // return value from message box

            pReplAddr[0] = ( pMIDA->szLongName[0] ? pMIDA->szLongName : pMIDA->szNameMergeMem );
            if ( (pMIDA->hwndErrMsg != HWND_FUNCIF) && !ISBATCHHWND(pMIDA->hwndErrMsg) )
            {
              usMBCode = UtlErrorHwnd( QUERY_MEM_FOLIMP_MERGE,
                                       MB_YESNO | MB_DEFBUTTON2,
                                       1,
                                       &pReplAddr[0], EQF_QUERY,
                                       pMIDA->hwndErrMsg );
            }
            else
            {
              usMBCode = MBID_YES;
            } /* endif */

            if ( usMBCode == MBID_NO )
            {
              usRc = FALSE;                      // do not import TM
              pMIDA->fMsg = TRUE;                // message has been shown
              pMIDA->usImportRc = TM_IMPORT_OK;  // avoid error return code
              EQFMemImportTrojaEnd( pMIDA );     // clean up
              return ( FALSE );                  // return immediately
            }
            else
            {
              strcpy( pMIDA->szNameMem, pMIDA->szLongName );
            } /* endif */
          }
          if ( usRc )
          {
            usRc = CheckSpaceStartMerge( hWnd, pMIDA );
            if ( usRc && (pMIDA->hwndErrMsg == HWND_FUNCIF) )
            {
              // call merge process directly!
              MemFuncMergeTM( pMIDA );
              pMIDA = NULL; // pMID has been freed in the meantime ...
            } /* endif */
          } /* endif */
        }
        else
        {
          // No Tm with same ID exists
          // Issue the message: Translation Memory %1 can not be imported.
          // A translation memory with the same name exists already but
          // it is reserved for other purposes.
          pReplAddr[0] = pMIDA->szLongName;
          UtlErrorHwnd( ERROR_MEMIMPORT_TM_EXISTS, MB_CANCEL, 1,
                        &pReplAddr[0], EQF_ERROR, pMIDA->hwndErrMsg );
          usRc = FALSE;
          pMIDA->fMsg = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */

    // ----------------------------------------------------------------------
    // If the properties for the TM  do NOT exist
    if ( !fPropExist && usRc )
    {
      /****************************************************************/
      /* The same TM does not exists                                  */
      /* It is agreed that the TM in such a case is                   */
      /* installed on the same drive the TM was initially imported    */
      /* Prepare the path of the TM                                   */
      /****************************************************************/
      UtlMakeEQFPath( pMIDA->szPathMem, NULC, MEM_PATH, NULL );

      /****************************************************************/
      /* check if the TM to be imported is a new one                  */
      /****************************************************************/
      if ( !strcmp( strrchr( pMIDA->szPathMergeMem, DOT), EXT_OF_TMDATA ) )
      {
        /**************************************************************/
        /* new TM, complete  target TM name and call function to      */
        /* create index and prop file                                 */
        /**************************************************************/
        sprintf( pMIDA->szPathMem, "%s\\%s%s", pMIDA->szPathMem,
                                               pMIDA->szNameMem,
                                               EXT_OF_TMDATA );
        usRc = (USHORT)NTMCreatePropandIndexFromImport( pMIDA, TRUE );
      }
      else if ( !strcmp( strrchr( pMIDA->szPathMergeMem, DOT), EXT_OF_SHARED_MEM ) )
      {
        /**************************************************************/
        /* new TM, complete  target TM name and call function to      */
        /* create index and prop file                                 */
        /**************************************************************/
        sprintf( pMIDA->szPathMem, "%s\\%s%s", pMIDA->szPathMem,
                                               pMIDA->szNameMem,
                                               EXT_OF_SHARED_MEM );
        usRc = (USHORT)NTMCreatePropandIndexFromImport( pMIDA, TRUE );
      }
      else
      {
        /**************************************************************/
        /* old TM, complete  target TM name and call function to      */
        /* create properties from imported properties                 */
        /**************************************************************/
        sprintf( pMIDA->szPathMem, "%s\\%s%s", pMIDA->szPathMem,
                                               pMIDA->szNameMergeMem,
                                               EXT_OF_MEM );
        usRc = CreatePropFromImportProp( pMIDA, FALSE );
      } /* endif */

      // If usRc Rename TM
      if (usRc)
      {
        usDosRc = UtlSmartMove( pMIDA->szPathMergeMem,
                                pMIDA->szPathMem, FALSE);

        // Handle the usDosRc return code
        if ( usDosRc == NO_ERROR )
        {
          // Tm has been moved correctly
          EQFMemImportTrojaEnd( pMIDA );
        }
        else
        {
          if (usDosRc == ERROR_ACCESS_DENIED )
          {
            // Issue the message: Translation Memory %1 can not be imported.
            // A translation memory with the same name exists already but
            // it is reserved for other purposes.
            pReplAddr[0] = pMIDA->szNameMergeMem;
            UtlErrorHwnd( ERROR_MEMIMPORT_TM_EXISTS, MB_CANCEL, 1,
                          &pReplAddr[0], EQF_ERROR, pMIDA->hwndErrMsg );
            usRc = FALSE;
            pMIDA->fMsg = TRUE;
          }
          else
          {
            // Let the utility UtlMove issue the error message
            usDosRc = UtlSmartMoveHwnd( pMIDA->szPathMergeMem,
                                        pMIDA->szPathMem, TRUE,
                                        pMIDA->hwndErrMsg );
            if (usDosRc)
            {
              usRc = FALSE;
              pMIDA->fMsg = TRUE;
            }
            else
            {
              // Tm has been moved correctly
              EQFMemImportTrojaEnd( pMIDA );
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    // If usRc == FALSE then invoke the TROJA Import end procedure
    // and indicate that the Import failed.
    if ( !usRc && pMIDA )
    {
        pMIDA->usImportRc = TM_IMPORT_FAILED;

      EQFMemImportTrojaEnd( pMIDA );
    } /* endif */

    if(pInfo != NULL)
	   delete pInfo;
  return( usRc );
} /* End of function EQFMemImportTrojaStart */

// ======= Terminate the import and post a message to the caller ======

VOID  EQFMemImportTrojaEnd
(
    PMEM_MERGE_IDA pMIDA              // Address of merge IDA
)
{
    USHORT       usDosRc;             // Dos function return code
    PSZ          pReplAddr[1];        // Pointer to an address list of replacement strings

    //  Close the translation memory to be imported and close the Tm
    CloseMergeTmAndTm( pMIDA, TRUE );                                /*@1139C*/

    // Erase the TM to be imported. Return code for tests only
    usDosRc = UtlDelete( pMIDA->szPathMergeMem, 0L, FALSE );

    /******************************************************************/
    /* delete the Index file ...                                      */
    /******************************************************************/
    UtlDelete( pMIDA->szPathMergeIndex, 0L, FALSE );

    // Erase the Properties to be imported. Return code for tests only
    usDosRc = UtlDelete( pMIDA->szPathMergeProp, 0L, FALSE );

    // If the Import terminated unsuccessfully
    if ( pMIDA->usImportRc != TM_IMPORT_OK )
    {
      // If no significant message has been issued yet and an Organize
      // process has not been invoked either then issue an appropriate
      // error message
      if( !pMIDA->fMsg && !pMIDA->fOrganizeInvoked )
      {
        // Set the address of the replacement string for the messages
        pReplAddr[0] = pMIDA->szNameMergeMem;
        switch (pMIDA->usImportRc)
        {
         case TM_IMPORT_FAILED:
           if ( !pMIDA->fImportAnyway )  // TM should not be imported anyway
           {
              pMIDA->usImportRc = TM_IMPORT_OK;
           }
           else
           {
                                                                      /*4@V8D*/
              pMIDA->fMsg = TRUE;
           }/*endif*/
           break;

         case TM_IMPORT_FORCED:

           // Issue the message:
           // The import of translation memory %1 was forced before completion.
           if ( !ISBATCHHWND(pMIDA->hwndErrMsg) )
           {
              UtlError( ERROR_MEM_IMPORT_TERM_FORCED, MB_CANCEL, 1,
                        &pReplAddr[0], EQF_ERROR );
              pMIDA->fMsg = TRUE;
           } /* endif */
           break;
         default:
          break;
        } /* endswitch */
      } /* endif */

      // Delete the properties if they had been created
      // Return code for test purposes only
      if (pMIDA->fPropCreated)
      {
        usDosRc = UtlDelete( pMIDA->szPathProp, 0L, TRUE );
      } /* endif */
    } /* endif */

    // Post a message to the caller with the proper return code mp1.
    // TRUE if the import was OK else FALSE
    if ( pMIDA->hwndErrMsg != HWND_FUNCIF )
    {
      EqfSend2Handler( pMIDA->szInvokingHandler, WM_EQFN_TASKDONE,
                       MP1FROMSHORT( pMIDA->usImportRc == TM_IMPORT_OK ), 0L);

      // Refresh the translation memory list. This is not needed if an organize
      // process has been started. The organize itself will update the TM list
      if ( !pMIDA->fOrganizeInvoked )
      {
        // build object name
        UtlMakeEQFPath( pMIDA->szSystemPath, NULC, SYSTEM_PATH, NULL );
        sprintf( pMIDA->szPathMem, "%s\\%s%s", pMIDA->szSystemPath,
                 pMIDA->szNameMem, EXT_OF_MEM );
        if ( pMIDA->hwndErrMsg != HWND_FUNCIF )
        {
          if ( pMIDA->fPropCreated )
          {
            EqfSend2Handler( MEMORYHANDLER, WM_EQFN_CREATED,
                             MP1FROMSHORT(  clsMEMORYDB ),
                             MP2FROMP( pMIDA->szPathMem ));
          }
          else
          {
            EqfSend2Handler( MEMORYHANDLER,
                             WM_EQFN_PROPERTIESCHANGED,
                             MP1FROMSHORT( PROP_CLASS_MEMORY ),
                             MP2FROMP( pMIDA->szPathMem ) );
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    // Free the Merge IDA
    UtlAlloc( (PVOID *) &pMIDA, 0L, 0L, NOMSG );

} /* End of function EQFMemImportTrojaEnd */

// ====== If enough space is available the merge process is started ==========

  USHORT CheckSpaceStartMerge
  (
    HWND            hWnd,                    // Window handle
    PMEM_MERGE_IDA  pMIDA                    // Address of merge IDA
  )
  {
    USHORT          usRc = TRUE;             // Return code to control the process
    USHORT          usResponse = MBID_RETRY; // Response from message box
    LONG            lBytesShort;             // Number of bytes short on the disk
    CHAR            szNumber[10];            // Working field to display numbers
    PSZ             pReplAddr[3];            // Pointer to an address list of replacement strings
    CHAR *          pszTemp;                 // Temporary character pointer
    USHORT          usDummy;                                           /*@RCA*/

    hWnd;                                   // get rid of compiler warnings
    // Return code TRUE means that merge has been started
    // If any error condition occurs the return code will be FALSE

    usResponse = MBID_OK;

    // If the properties do not exist they will be created
    if ( usRc && !pMIDA->fPropExist )
    {
      usRc = CreatePropFromImportProp( pMIDA, TRUE );
    } /* endif */

    // If everything OK post a message to start the merge process
    if (usRc)
    {
        BOOL    fOK;
        CHAR    szBuffer[80];

        // build object name
        strcpy( szBuffer, MEMMERGE_PROC_OBJPREFIX );
        strcat( szBuffer, pMIDA->szNameMergeMem );
        if ( pMIDA->hwndErrMsg != HWND_FUNCIF )
        {
          fOK = CreateProcessWindow( szBuffer, MemMergeCallBack, pMIDA );
        }
        else
        {
          fOK = TRUE;
        } /* endif */

        if ( !fOK )
        {
          /* free IDA, otherwise it will be freed in CallBack */
          UtlAlloc( (PVOID *) &pMIDA, 0L, 0L, NOMSG );
        } /* endif */
    } /* endif */
    return usRc;
  } /* End of function CheckSpaceStartMerge */
/*
+------------------------------------------------------------------------------+
 Function Name : SearchAndOpenEqualMem                                        
                                                                              
 Description   : Search a specific TM, open it and compare ID's               
                 If a TM with the same name and the same ID is found then     
                 a equal TM is found. If the IDs do not match a message       
                 is displayed if the TMs should be merged anyway. If the      
                 answer is Yes, it is treated as equal.                       
                                                                              
 Call          :  USHORT SearchAndOpenEqualMem                                
                  (                                                           
                    PMEM_MERGE_IDA pMIDA,                                     
                    PSZ            pszDriveMem,                               
                    USHORT         usLocation                                 
                  )                                                           
                                                                              
 Where:          - pMIDA is a pointer to the merge IDA                        
                 - pszDriveMem pointer to the drive list to be searched       
                 - usLocation is the location of the TM                       
                   The following locations are valid:                         
                     TM_LOCAL       search only for local TMs                 
                     TM_REMOTE      search only for remote TMs                
                     TM_LOCALREMOTE search for local and remote TMs           
                                                                              
 Returns       : TRUE - no error occured.                                     
                   This means not! that a equal TM is found. This has to be   
                   checked via the TM handles. If no TM is found, the handles 
                   returned are NULL.                                         
                 FALSE - an error occured.                                    
                   No equal TMs found, the returned TM handles are NULL       
                                                                              
 Known problems: if a remote TM exists, but there is no property file         
                 to it, it will be ignored, because there is no chance        
                 to create the remote property file                           
+------------------------------------------------------------------------------+
*/
USHORT SearchAndOpenEqualMem
(
  PMEM_MERGE_IDA pMIDA,             // Address of merge IDA
  PSZ            pszDriveMem,       // Pointer to a drive list to be searched
  USHORT         usLocation         // Location of the TM
)
{
  BOOL         fTMFound = FALSE;   // Translation memory found indicator
  USHORT       usRc = TRUE;        // Function return code
  USHORT       usTmRc;             // TM function return code
  USHORT       usTmRc2;            // TM function return code
  USHORT       usIndex = 0;        // Loop index
  USHORT       Response;           // Response from message box
  CHAR         szDrive[3];         // Drive string to display
  PSZ          pReplAddr[2];       // Pointer to an address list of replacement strings
  USHORT       usResponse;         //return from UtlError
  PSZ          pszReplace[4];      // pointer arry for replace strings
  MemoryFactory *pFactory = MemoryFactory::getInstance();
  int iRC = 0;

  //--- Set both TM handles to NULL
  pMIDA->pOutputMem = NULL;
  pMIDA->pMergeMem = NULL;

  //--- set merge forced flag
  pMIDA->fImportAnyway = TRUE;

  // Open TM to be imported
  pMIDA->pMergeMem = pFactory->openMemory( NULL, pMIDA->szNameMergeMem, EXCLUSIVE, &iRC );
  if ( pMIDA->pMergeMem == NULL )
  {
    usRc = FALSE;
    pFactory->showLastError( NULL, pMIDA->szNameMergeMem, NULL, pMIDA->hwndErrMsg );
  } /* endif */       


  if ( usRc )
  {
    pMIDA->pOutputMem = pFactory->openMemory( NULL, pMIDA->szNameMem, NONEXCLUSIVE, &iRC );
    if ( pMIDA->pOutputMem == NULL )
    {
      usRc = FALSE;
      pFactory->showLastError( NULL, pMIDA->szNameMem, NULL, pMIDA->hwndErrMsg );
    } /* endif */       

    if ( usRc )
    {
      pMIDA->pOutputMem->getSourceLanguage( pMIDA->szTargetMemSourceLang, sizeof(pMIDA->szTargetMemSourceLang) );
      pMIDA->pMergeMem->getSourceLanguage( pMIDA->szSourceMemSourceLang, sizeof(pMIDA->szSourceMemSourceLang) );

      if ( strcmp( pMIDA->szSourceMemSourceLang, pMIDA->szTargetMemSourceLang ) )
      {
          /*******************************************************/
          /* display warning message that source languages are   */
          /* not identical and query for continue                */
          /*******************************************************/
          pszReplace[0] = pMIDA->szNameMergeMem;        //source TM
          pszReplace[1] = pMIDA->szNameMem;             //target TM
          pszReplace[2] = pMIDA->szSourceMemSourceLang;
          pszReplace[3] = pMIDA->szTargetMemSourceLang;
          if ( !ISBATCHHWND(pMIDA->hwndErrMsg) )
          {
            usResponse = MBID_YES;
          }
          else
          {
            usResponse = UtlError(  ERROR_MEM_DIFFERENT_SOURCE_LANG,
                                    MB_YESNO | MB_DEFBUTTON2, 4,
                                    pszReplace, EQF_QUERY );
          } /* endif */
          if ( usResponse == MBID_NO )   //--- do not merge TMs
          {
            //--- stop further processing processing
            usRc = FALSE;
            pFactory->closeMemory( pMIDA->pOutputMem );
            pMIDA->pOutputMem = NULL;
            pMIDA->fImportAnyway = FALSE;
          }/*endif*/
        }/*endif*/
      } /* endif */
    } /* endif */

  // If an error occurred or no equal TM was found close the TM's and set handles to NULL
  if ( !usRc )
  {
    //  Close the translation memory to be imported and close the Tm
    CloseMergeTmAndTm( pMIDA, FALSE );                             
  } /* endif */
  return usRc;
} /* End of function SearchAndOpenEqualMem */

// ======= Create properties from the properties to be imported =======

  USHORT CreatePropFromImportProp
  (
    PMEM_MERGE_IDA pMIDA,             // Address of merge IDA
    BOOL           fUpdateTMList      // If TRUE then update the TM List else do not update
  )
  // The function returns TRUE if no error occurred else it returns FALSE.
  {
    USHORT            usRc = TRUE;    // Function return code
    USHORT            usDosRc;        // DOS function return code
    HPROP             hProp = NULL;   // Property handle
    PPROPTRANSLMEM    pProp = NULL;   // Property pointer
    EQFINFO           ErrorInfo;      // Property handler error info
    PSZ               pReplAddr[1];   // Pointer to an address list of replacement strings

    if ( !pMIDA->fPropExist )
    {
      // Copy the properties to be imported to the system drive
      // but prepare the property path first
      UtlMakeEQFPath( pMIDA->szPathProp, NULC, PROPERTY_PATH, NULL );
      sprintf(pMIDA->szPathProp, "%s\\%s%s", pMIDA->szPathProp,
                                             pMIDA->szNameMergeMem,
                                             EXT_OF_MEM );

      usDosRc = UtlCopyHwnd( pMIDA->szPathMergeProp, pMIDA->szPathProp, 0, 0L,
                             TRUE, pMIDA->hwndErrMsg );

      if (usDosRc)
      {
        // An error occurred while copying
        usRc = FALSE;
      }
      else
      {
        // The copy operation was OK
        // Set the flag that properties have been created
        pMIDA->fPropCreated = TRUE;
      } /* endif */

      if (usRc)
      {
        // Get the properties for write
        usRc = MemOpenProp(&(hProp),            // address of property handle
                           (PVOID *)&(pProp),   // address of property pointer
                           pMIDA->szPropName,   // Property name
                           pMIDA->szSystemPath, // Property path
                           PROP_ACCESS_WRITE,   // write access only
                           TRUE );              // Issue messages
      } /* endif */

      if (usRc)
      {
        // Change the drive letter in the properties
        pProp->stPropHead.szPath[0] = pMIDA->szSystemPath[0]; // Property path of memory database in property header /*@KFA*/
        pProp->szPath[0]          = pMIDA->szSystemPath[0];   // Property path of memory database
        memset( pProp->szFullMemName, NULC, MAX_EQF_PATH );
        strcpy( pProp->szFullMemName, pMIDA->szPathMem );     // Full name and path to the TM
                                                              // with correct drive, name and ext.
        pProp->szXTagsListFile[0] = pMIDA->szSystemPath[0];   // Full path + name + ext to format table
        pProp->szXWordsListFile[0]= pMIDA->szSystemPath[0];   // Full path + name + ext to noise list
        pProp->szLanguageFile[0]  = pMIDA->szSystemPath[0];   // Full path + name + ext to the language file
        pProp->szFullFtabPath[0]  = pMIDA->szSystemPath[0];   // Full path to tag tables and exclusion lists

        // Clear last used values of exported file in ASCII format
        memset( pProp->szPathLastExported, NULC, sizeof(pProp->szPathLastExported) );

        // Clear UserID and Servername
        memset( pProp->szUserid, NULC, MAX_USERID );
        memset( pProp->szServer, NULC, MAX_SERVER_NAME );

        // Save the properties
        if ( SaveProperties( hProp, &ErrorInfo) )
        {
          usRc = FALSE;
        } /* endif */

        // Send a message that the properties were changed and that
        // they now exist. The message should only be sent if requested because there
        // are cases the Translation Memory does not exist yet.
        if ( usRc )
        {
          if (fUpdateTMList)
          {
            // Send a message that properties have changed
            // assemble object name
            sprintf( pProp->szTemp, "%s\\%s", pProp->szPath, pProp->szName );
            EqfSend2Handler( MEMORYHANDLER,
                             WM_EQFN_CREATED,
                             MP1FROMSHORT( clsMEMORYDB ),
                             MP2FROMP( pProp->szTemp ) );
          } /* endif */

          pMIDA->fPropExist = TRUE;
        } /* endif */
      } /* endif */
    }
    else
    {
      // The properties do exist already
      usRc = FALSE;
    } /* endif */

    if (!usRc)
    {
      // Issue error message:The properties for the new object %1 could not be created.
      pReplAddr[0] = pMIDA->szNameMergeMem;
      UtlErrorHwnd( ERROR_CREATE_PROPERTIES, MB_CANCEL, 1,
                &pReplAddr[0], EQF_ERROR, pMIDA->hwndErrMsg );
    } /* endif */

    // Close the properties if open and set property handle to NULL
    if (hProp)
    {
      CloseProperties( hProp, PROP_QUIT, &ErrorInfo);
      hProp = NULL;
    } /* endif */

    return usRc;
  } /* End of function CreatePropFromImportProp */

BOOL
NTMGetOldImportInfo( PMEM_MERGE_IDA pMIDA )
{
  PPROPTRANSLMEM pOldProps = NULL;
  ULONG          ulBytesRead;
  BOOL           fOk;

  fOk = UtlLoadFileL( pMIDA->szPathMergeProp, (PVOID *)&pOldProps, &ulBytesRead,
                     FALSE, FALSE);
  if ( fOk )
  {
    strcpy( pMIDA->szSourceMemSourceLang, pOldProps->szSourceLang );
    strcpy( pMIDA->szSourceMemTargetLang, pOldProps->szTargetLang );
    strcpy( pMIDA->szTagTable,            pOldProps->szFormat     );
    UtlAlloc( (PVOID *) &pOldProps, 0L, 0L, NOMSG );
  } /* endif */

  return fOk;
} /* end of function NTMGetOldImportInfo */


BOOL
NTMCreatePropandIndexFromImport
(
  PMEM_MERGE_IDA pMIDA,             // Address of merge IDA
  BOOL           fUpdateTMList      // If TRUE then update the TM List else do not update
)
{
  EQFINFO           ErrorInfo;     //rc propert functions
  PSZ               pReplAddr[1];  //replace for error message
  USHORT            usRc;          //rc from Utl functions and NTMOpenProperties
  //PPROP_NTM         pProp = NULL;  //pointer to properties
  //HPROP             hProp = NULL;  //property handle
  BOOL              fOk = TRUE;    //function rc

  

  if ( !pMIDA->fPropExist )
  {
    /******************************************************************/
    /* Copy the properties to be imported to the system drive         */
    /* but prepare the property path first                            */
    /******************************************************************/
    UtlMakeEQFPath( pMIDA->szPathProp, NULC, PROPERTY_PATH, NULL );
    sprintf(pMIDA->szPathProp, "%s\\%s%s", pMIDA->szPathProp,
                                           pMIDA->szNameMem,
                                           EXT_OF_TMPROP );

    usRc = UtlCopyHwnd( pMIDA->szPathMergeProp, pMIDA->szPathProp, 0, 0L,
                        TRUE, pMIDA->hwndErrMsg );

    if ( !usRc )
    {
      // correct property header as new short name may be different from
      // short name of imported TM
      PPROP_NTM pMemProp = NULL;
      ULONG ulBytesRead;

      if( UtlLoadFileL( pMIDA->szPathProp, (PVOID *)&pMemProp,
                       &ulBytesRead, FALSE, FALSE ) )
      {
		// the following information need to change
		strcpy( pMemProp->stPropHead.szPath,  pMIDA->szSystemPath);
		strcpy( pMemProp->stPropHead.szName, pMIDA->szPropName);
		strcpy( pMemProp->szFullMemName, pMIDA->szPathMem);
		//write to disk
        UtlWriteFileL( pMIDA->szPathProp, ulBytesRead, pMemProp, FALSE );
        UtlAlloc( (PVOID *)&pMemProp, 0L, 0L, NOMSG );
      } /* endif */
    } /* endif */

    if (usRc)
    {
      /****************************************************************/
      /* error during copy stop further processing                    */
      /****************************************************************/
      fOk = FALSE;
    }
    else
    {
      /****************************************************************/
      /* The copy operation was OK                                    */
      /* Set the flag that properties have been created               */
      /****************************************************************/
      pMIDA->fPropCreated = TRUE;
    } /* endif */

    if ( fOk )
    {
        if (fUpdateTMList)
        {
			MemoryFactory *pFactory = MemoryFactory::getInstance();
			OtmMemoryPlugin::PMEMORYINFO pInfo = new( OtmMemoryPlugin::MEMORYINFO );
			if(pInfo!=NULL)
			{
				CHAR szCompleteName[MAX_LONGPATH]={0};
				if ( pFactory->getMemoryInfo( NULL, pMIDA->szNameMem, pInfo ) != 0 )
				{
					usRc = 1;
					fOk =  FALSE;
				}	

				sprintf(szCompleteName,"%s:%s",pInfo->szPlugin,pInfo->szName);
		      
				// fill memories vector
				EqfSend2Handler( MEMORYHANDLER,
								WM_EQF_MEMFILL_LISTBOX,
								MP1FROMSHORT( clsMEMORYDB ),
								MP2FROMP( pInfo->szName ) );

				// let the memory show in memory list window
				EqfSend2AllHandlers( WM_EQFN_CREATED,
								MP1FROMSHORT(clsMEMORYDB),
								MP2FROMP(szCompleteName));


				if(pInfo != NULL)
					delete pInfo;
			}

        }// end if (fUpdateTMList)
       
        pMIDA->fPropExist = TRUE;

    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* the properties do exist already                                */
    /******************************************************************/
    usRc = FALSE;
  } /* endif */

  if ( !fOk )
  {
    /******************************************************************/
    /* Issue error message:                                           */
    /* The properties for the new object %1 could not be created.     */
    /******************************************************************/
    pReplAddr[0] = pMIDA->szNameMergeMem;
    UtlErrorHwnd( ERROR_CREATE_PROPERTIES, MB_CANCEL, 1,
                  &pReplAddr[0], EQF_ERROR, pMIDA->hwndErrMsg );
  } /* endif */

  if ( fOk )
  {
    /******************************************************************/
    /* copy the index file from import directory                      */
    /******************************************************************/
    PSZ pszExt = strrchr( pMIDA->szPathMergeMem, DOT );
    if ( pszExt && (strcmp( pszExt, EXT_OF_SHARED_MEM ) == 0 ) )
    {
      Utlstrccpy( pMIDA->szPathMergeIndex, pMIDA->szPathMergeMem, DOT );
      strcat( pMIDA->szPathMergeIndex, EXT_OF_SHARED_MEMINDEX );
      Utlstrccpy( pMIDA->szPathIndex, pMIDA->szPathMem, DOT );
      strcat( pMIDA->szPathIndex, EXT_OF_SHARED_MEMINDEX );
    }
    else
    {
      Utlstrccpy( pMIDA->szPathMergeIndex, pMIDA->szPathMergeMem, DOT );
      strcat( pMIDA->szPathMergeIndex, EXT_OF_TMINDEX );
      Utlstrccpy( pMIDA->szPathIndex, pMIDA->szPathMem, DOT );
      strcat( pMIDA->szPathIndex, EXT_OF_TMINDEX );
    } /* endif */

    usRc = UtlCopyHwnd( pMIDA->szPathMergeIndex, pMIDA->szPathIndex, 0, 0L,
                        TRUE, pMIDA->hwndErrMsg );

    if (usRc)
    {
      /****************************************************************/
      /* error during copy stop further processing                    */
      /****************************************************************/
      fOk = FALSE;
      /****************************************************************/
      /* delete the above create property file                        */
      /****************************************************************/
      UtlDelete( pMIDA->szPathProp, 0L, FALSE );
    } /* endif */

  } /* endif */

  return fOk;
} /* End of function NTMCreatePropandIndexFromImport  */
