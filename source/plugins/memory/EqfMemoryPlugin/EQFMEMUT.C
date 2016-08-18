//------------------------------------------------------------------------------
//EQFMEMUT.C
//------------------------------------------------------------------------------
//Copyright Notice:
//
//      Copyright (C) 1990-2015, International Business Machines
//      Corporation and others. All rights reserved
//------------------------------------------------------------------------------
//Description: Utility programs for the memory database
//             administrative system
//------------------------------------------------------------------------------

#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_LIST             // terminology list functions
#define INCL_EQF_DAM
#define INCL_EQF_ASD
#include <eqf.h>                  // General Translation Manager include file

#include "EQFDDE.H"               // Batch mode definitions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFQDAMI.H>

//#include <eqflstex.h>

// ============  Get a part from the translation memory database=====================
// ------------------------------------------------------------------------
//                             TmGetTMPart
//
//  USHORT TmGetTMPart( HTM          htm,
//                      PSZ          pszMemPath,
//                      PGETPART_IN  pGetPartIn,
//                      PGETPART_OUT pGetPartOut,
//                      USHORT       usMsgHandling )
//
//  htm           : handle of TM
//  pszMemPath    : full qualified TM name ( X:\EQF\MEM\TM.MEM )
//  pGetPartIn    : pointer to input structure
//  pGetPartOut   : pointer to output structure
//  usMsgHandling : message handling flag
//                  TRUE: the function handles error and displayes
//                        an error message
//                  FALSE: do no error processing
//
//  returncode : NO_ERROR in case of no error
//               other values: return codes form U code
//
//  This function allows to get a part of the translation memory file
//  starting on any position in the file with a specified length. The
//  desired block of the translation memory file and the bytes read are
//  returned. Before calling this function the TM has to opened with
//  usMode EXCLUSIVE_FOR_GET_PART_TM
//  The structures PGETPART_IN and PGETPART_OUT are described in
//  EQFTMDEF.H
//  If the number of bytes to read set in input structure
//  pGetPartIn->ulBytesToRead is higher than the defined maximum
//  GETPART_BUFFER_SIZE, then pGetPartIn->ulBytesToRead is set to
//  GETPART_BUFFER_SIZE.
//
//  Example how to use TmGetTMPart :
//
//  PGETPART_IN  pGetPartIn;
//  PGETPART_OUT pGetPartOut;
//  USHORT       usTestRc;
//  USHORT       usBytesWritten;
//  HFILE        hf
//  USHORT       usAction;
//  CHAR         szTestMem[40];
//
//  strcpy(szTestMem,"D:\\EQF\\MEM\\T3.MEM");
//
//  //--- allocate storage for input structure
//  UtlAlloc( (PVOID *) &pGetPartIn, 0L, (LONG)( sizeof( GETPART_IN )),
//            ERROR_STORAGE);
//  //--- allocate storage for output structure
//  UtlAlloc( (PVOID *) &pGetPartOut, 0L, (LONG)( sizeof( GETPART_OUT )),
//            ERROR_STORAGE);
//  //--- start getting at file position 0
//  pGetPartIn->ulFilePos     = 0;
//  //--- bytes to read
//  pGetPartIn->ulBytesToRead = GETPART_BUFFER_SIZE;
//  //--- open translation memory
//  usTestRc = TmOpen( szTestMem,
//                     &htm,
//                     EXCLUSIVE_FOR_GET_PART,
//                     FALSE );
//  //--- open output file
//  usTestRc = UtlOpen ( "D:\\TESTPART.MEX",
//                       &hf,
//                       &usAction,
//                       0L,
//                       FILE_NORMAL,
//                       FILE_CREATE,
//                       OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE
//                       0L,
//                       FALSE );
//  do
//  {
//     //--- call to get part function
//     TmGetTMPart( htm, szTestMem, pGetPartIn, pGetPartOut, TRUE);
//     //--- write output to file
//     UtlWrite( hf, pGetPartOut->aucOutBuffer,
//               (USHORT)pGetPartOut->ulBytesRead,
//               &usBytesWritten, TRUE );
//     //--- set new read position
//     pGetPartIn->ulFilePos = pGetPartOut->ulNextFilePos;
//
//  } while ( pGetPartIn->ulBytesToRead == pGetPartOut->ulBytesRead)
//  //--- close TM and output file
//  TmClose( htm, szTestMem, TRUE );
//  usTestRc = UtlClose( hf, TRUE );
//
//  ----------------------------------------------------------------------+

USHORT TmGetTMPart( HTM      htm,              //Memory database handle
                    PSZ      pszMemPath,       //Full translation memory path
                    PGETPART_IN  pGetPartIn,   //Pointer to the input structure
                    PGETPART_OUT pGetPartOut,  //Pointer to the output structure
                    USHORT   usMsgHandling )   //Message handling parameter
{
   USHORT usURc = NO_ERROR;      // U function return code


   //--- Assign the get part command in prefix of input structure
   pGetPartIn->prefin.idCommand = TMC_GET_PART_OF_TM_FILE;

   //--- Assign the length of the input structure in prefix of input structure
   pGetPartIn->prefin.usLenIn   = sizeof( GETPART_IN);

   //--- call to U code
   //usURc = ServerInterface( htm,
   //           (PXIN)pGetPartIn,         // Pointer to input structure
   //           (PXOUT)pGetPartOut,       // Pointer to output structure
   //           OLD_TM );                 // should work for old and new TMs
   //
   //--- Perform appropriate error handling if requested and required
  if ( usMsgHandling && ( usURc != NO_ERROR) )
  {
     usURc = MemRcHandling( usURc, pszMemPath, &htm, NULL );
  } /* endif */

    return usURc;

} /* End of function TmGetTMPart */

// ================ Handle the memory database return codes ===================

USHORT MemRcHandling
(
 USHORT           usMemRc,         // Return code of memory database call
 PSZ              pszMemPath,      // Pointer to full translation memory path
 HTM          *   htm,             // address of TM handle (may be undefined or NULL)
 PSZ              pszServer        // Pointer to servername
)
{
  return( MemRcHandlingHwnd( usMemRc, pszMemPath, htm, pszServer, NULLHANDLE ) );
}

USHORT MemRcHandlingHwnd
(
 USHORT           usMemRc,         // Return code of memory database call
 PSZ              pszMemPath,      // Pointer to full translation memory path
 HTM          *   htm,             // address of TM handle (may be undefined or NULL)
 PSZ              pszServer,       // Pointer to servername
 HWND             hwnd             // Owner widow handle for error messages
)
/* This function handles all global error of memory database calls.
   The function does not change the memory database return code
   unless recovery took successfully place which means that NO_ERROR
   will be returned. For a list of return codes see EQFTMDEF.H.
   If a storage error occurred in this function the return code will be set to 1. */
{

 MRESULT      mrOrganizeRc = FALSE;     // rc form EqfSend2Handler for organizing
 CHAR         chString[16];             // Work string
 PSZ          pReplAddr[2];             // Arrey of pointers to replacement strings
 CHAR         szMemName[MAX_FILESPEC];  // Memory database name
 USHORT       Response;                 // Response from message box
 USHORT       usErrorMessage = 0;       // Error message number
 HTM          htmWork;                  // A work handle for intermediate storage

 // Isolate the translation memory name from the full path string
 // but first check if it is a full path string
 if ( pszMemPath != NULL )
 {
   CHAR szLongName[MAX_LONGFILESPEC];

   if ( strchr( pszMemPath,'.' ) && strchr( pszMemPath,'\\' ))
   {
     // Copy the translation memory name from the full path
     // to the name szMemName
     Utlstrccpy( szMemName,
                 UtlGetFnameFromPath( pszMemPath ),
                 DOT );
     ObjShortToLongName( szMemName, szLongName, TM_OBJECT );
     OEMTOANSI( szLongName );

     pReplAddr[0] = szLongName;
   }
   else if ( strchr( pszMemPath,'.' ) )
   {
     Utlstrccpy( szMemName, pszMemPath, DOT );
     ObjShortToLongName( szMemName, szLongName, TM_OBJECT );

     OEMTOANSI( szLongName );
     pReplAddr[0] = szLongName;
   }
   else
   {
     // It is not a full path string but a user error
     // szMemName is set to an empty string in order to avoid Trap D's
     strcpy( szMemName, "" );
     pReplAddr[0] = pszMemPath;
   } /* endif */
 } /* endif */
 else
 {
   // if the name is NULL copy an empty string to the name
   strcpy( szMemName, "" );
   pReplAddr[0] = szMemName;
 } /* endelse */

 switch ( usMemRc )
 {
   //-----------------------------------------------------------------
   case ERROR_SHARING_VIOLATION:
   case BTREE_DICT_LOCKED:
   case BTREE_ENTRY_LOCKED:
   case BTREE_ACCESS_ERROR:
     // Issue message: Memory database %1 is currently not accessible for
     //                the function which is requested
     UtlErrorHwnd( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL, 1,
               &pReplAddr[0], EQF_ERROR, hwnd );
     break;

   //---------------------------------------------------------------------
   case ERROR_PATH_NOT_FOUND:
   case ERROR_INVALID_DRIVE:
   case BTREE_INVALID_DRIVE :
     if ( pszMemPath != NULL )
     {
       // Issue the message: Troja drive %1 is not accessible.
       memcpy( chString, pszMemPath, 2 );
       chString[2] = NULC;
       pReplAddr[0] = chString;
       UtlErrorHwnd( ERROR_EQF_DRIVE_NOT_ACCESSIBLE, MB_CANCEL, 1,
                 &pReplAddr[0], EQF_ERROR, hwnd );
     }
     else
     {
       // Issue message for undefined error
       MemRcHandlingErrorUndefinedHwnd( usMemRc, pReplAddr[0], hwnd );
     } /* endif */
     break;

   //---------------------------------------------------------------------
   case ERROR_NOT_ENOUGH_MEMORY:
   case BTREE_NO_ROOM:
   case BTREE_NO_BUFFER:
     // Issue the message: Short on system storage.
     UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0,
               NULL, EQF_ERROR, hwnd );
     break;

   //---------------------------------------------------------------------

   case ERROR_VERSION_NOT_SUPPORTED:
     // Issue the message: Need new Translationmanager version
     UtlErrorHwnd( ERROR_MEM_VERSION_NOT_SUPPORTED, MB_CANCEL, 1,
                   &pReplAddr[0], EQF_ERROR, hwnd );
     break;


   //---------------------------------------------------------------------
   case VERSION_MISMATCH:
   case CORRUPT_VERSION_MISMATCH:
   case FILE_MIGHT_BE_CORRUPTED:
   case BTREE_CORRUPTED:
   case BTREE_USERDATA:
     if ( ( *htm != NULLHANDLE ) && ( pszMemPath != NULL ) )
     {
       // Handle must be saved because an other task may
       // overwrite it
       htmWork = *htm;
       switch ( usMemRc )
       {
         // Set the variable to issue the proper error message
         case FILE_MIGHT_BE_CORRUPTED:
         case BTREE_CORRUPTED:
         case BTREE_USERDATA:
           usErrorMessage = ERROR_MEM_MIGHT_BE_CORRUPTED;
           break;
         case VERSION_MISMATCH:
         case CORRUPT_VERSION_MISMATCH:
           usErrorMessage = ERROR_MEM_VERSION_MISMATCH;
           break;
       } /* endswitch */

       // Issue the message: "The translation memory %1 is corrupted/backlevel"
       //                    "\n Do you want to start the organize process of"
       //                    "the translation memory %1 now ?"
       Response = UtlErrorHwnd( usErrorMessage, MB_YESNO, 1,
                 &pReplAddr[0], EQF_ERROR, hwnd );
       if ( Response == MBID_YES )   //--- user wants to organize TM now
       {
         // Close the open TM and set TM handle to NULL
         TmClose( htmWork, pszMemPath, FALSE, 0 );
         *htm = NULLHANDLE;

         //--- send TM handler message for organizing TM
         mrOrganizeRc = EqfSend2Handler ( MEMORYHANDLER,
                                          WM_EQF_PROCESSTASK,
                                          MP1FROMSHORT( PID_FILE_MI_ORGANIZE ),
                                          MP2FROMP( pReplAddr[0] ));
         if ( mrOrganizeRc )   //--- organize was successfull
         {
            //--- set return code so that caller know that TM was corrupted
            //--- and was successfully organized
            usMemRc = TM_WAS_CORRUPTED_IS_ORGANIZED;
         }/*endif*/
       }
       else                          //--- user do not want to organize TM now
       {
         // Restore the TM handle in case it was destroyed by an other task
         *htm = htmWork;
       } /* endif */
     }
     else
     {
       // Issue message for corrupted TM
       UtlErrorHwnd( ERROR_RENUMBER, MB_CANCEL, 1, pReplAddr, EQF_ERROR, hwnd );
     } /* endif */
     break;
   //---------------------------------------------------------------------
   case DISK_FULL:
     // Issue the message: "The disk on which memory database %1 is located is full"
     UtlErrorHwnd( ERROR_MEM_DISK_FULL, MB_CANCEL, 1,
               &pReplAddr[0], EQF_ERROR, hwnd );
     break;
   case BTREE_DISK_FULL:
   case BTREE_WRITE_ERROR   :
     // Issue the message: "Disk xxx is full"
     memcpy( chString, pszMemPath, 2 );
     chString[2] = NULC;
     pReplAddr[0] = chString;
     UtlErrorHwnd( ERROR_DISK_FULL_MSG, MB_CANCEL, 1,
               &pReplAddr[0], EQF_ERROR, hwnd );
     break;
   //---------------------------------------------------------------------
   case DB_FULL:
   case BTREE_LOOKUPTABLE_TOO_SMALL:
     // Issue the message: "Memory database %1 is full"
     UtlErrorHwnd( ERROR_MEM_DB_FULL, MB_CANCEL, 1,
               &pReplAddr[0], EQF_ERROR, hwnd );
     break;
   //--------------------------------------------------------------------
   case ERROR_OLD_PROPERTY_FILE:
     UtlErrorHwnd( ITM_TM_NEEDS_ORGANIZE, MB_CANCEL, 1,
               &pReplAddr[0], EQF_ERROR, hwnd );
     break;
   //---------------------------------------------------------------------
   case FILE_ALREADY_EXISTS:
     // Issue the message: The name  %1  is invalid or a memory database with this name
     //                    exists already or a flat file with this name exists alredy.
     UtlErrorHwnd( ERROR_MEM_NAME_INVALID, MB_CANCEL, 1,
               &pReplAddr[0], EQF_ERROR, hwnd );
     break;
   //---------------------------------------------------------------------
   case NOT_REPLACED_OLD_SEGMENT:
     // Issue the message: In the translation memory %1 a function requested to replace
     //                    a segment but the corresponding segment alredy in the translation memory
     //                    is more recent. The replace is refused.
     UtlErrorHwnd( ERROR_MEM_NOT_REPLACED, MB_CANCEL, 1,
               &pReplAddr[0], EQF_ERROR, hwnd );
     break;
   //---------------------------------------------------------------------
   case TM_FILE_SCREWED_UP:
   case NOT_A_MEMORY_DATABASE:
   case BTREE_ILLEGAL_FILE:
     // Issue the message: The translation memory %1 is seriously damaged. It can not
     //                    be recovered.
     UtlErrorHwnd( ERROR_MEM_DESTROYED,MB_CANCEL, 1,
               &pReplAddr[0], EQF_ERROR, hwnd );
     break;
   //---------------------------------------------------------------------
   case TM_FILE_NOT_FOUND:
   case BTREE_FILE_NOTFOUND:
                                                                     /*30@SAD*/
     // If the path name is available
     if ( pszMemPath != NULL )
     {
       //--- issue an error message else
       UtlErrorHwnd( ERROR_TM_FILE_NOT_FOUND, MB_CANCEL, 1,
                 &pReplAddr[0], EQF_ERROR, hwnd );

       //--- if local TM
       if ( pszServer )
       {
         if ( pszServer[0] == EOS )
         {
           CHAR   szObjName[MAX_EQF_PATH];

           //--- send message to TM list handler to force update of TM
           //--- listbox, so that the local TM is grayed out
           UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
           strcat( szObjName, BACKSLASH_STR );
           strcat( szObjName, szMemName );
           strcat( szObjName, EXT_OF_MEM );
           EqfSend2Handler( MEMORYHANDLER,
                            WM_EQFN_PROPERTIESCHANGED,
                            MP1FROMSHORT( PROP_CLASS_MEMORY ),
                            MP2FROMP( szObjName ) );
                                                                      /*3@SAD*/
         } /* endif */
       } /* endif */
                                                                      /*3@SAD*/
     }
     else
     {
       // issue undefined error message
       MemRcHandlingErrorUndefinedHwnd( usMemRc, pReplAddr[0], hwnd );
     } /* endelse */
     break;
   //---------------------------------------------------------------------
   case TMERR_TM_OPENED_EXCLUSIVELY:
     // If the path name is there issue an error message else
     // issue undefined error message

     if ( pszMemPath != NULL )
     {
       UtlErrorHwnd( ERROR_TM_OPENED_EXCLUSIVELY, MB_CANCEL, 1,
                 &pReplAddr[0], EQF_ERROR, hwnd );
     } /* endif */
     else
     {
       MemRcHandlingErrorUndefinedHwnd( usMemRc, pReplAddr[0], hwnd );
     } /* endelse */
     break;
   //---------------------------------------------------------------------
   case TMERR_TM_OPENED_SHARED:
     // If the path name is there issue an error message else
     // issue undefined error message
     if ( pszMemPath != NULL )
     {
       UtlErrorHwnd( ERROR_TM_OPENED_SHARED, MB_CANCEL, 1,
                 &pReplAddr[0], EQF_ERROR, hwnd );
     } /* endif */
     else
     {
       MemRcHandlingErrorUndefinedHwnd( usMemRc, pReplAddr[0], hwnd );
     } /* endelse */
     break;
   //---------------------------------------------------------------------
   case TMERR_NO_MORE_MEMORY_AVAILABLE:
       MemRcHandlingHwnd( ERROR_NOT_ENOUGH_MEMORY, pszMemPath, htm, pszServer, hwnd );
     break;
   //---------------------------------------------------------------------
   case TMERR_PROP_EXIST:
     // If the path name is there issue an error message else
     // issue undefined error message
     if ( pszMemPath != NULL )
     {
       UtlErrorHwnd( ERROR_PROP_EXIST, MB_CANCEL, 1,
                 &pReplAddr[0], EQF_ERROR, hwnd );
     } /* endif */
     else
     {
       MemRcHandlingErrorUndefinedHwnd( usMemRc, pReplAddr[0], hwnd );
     } /* endelse */
     break;
   //---------------------------------------------------------------------
   case TMERR_PROP_WRITE_ERROR:
     // If the path name is there issue an error message else
     // issue undefined error message
     if ( pszMemPath != NULL )
     {
       UtlErrorHwnd( ERROR_PROP_WRITE, MB_CANCEL, 1,
                 &pReplAddr[0], EQF_ERROR, hwnd );
     } /* endif */
     else
     {
       MemRcHandlingErrorUndefinedHwnd( usMemRc, pReplAddr[0], hwnd );
     } /* endelse */
     break;
   //---------------------------------------------------------------------
   case TM_PROPERTIES_NOT_OPENED:                                    /*@1170A*/
     // If the path name is there issue an error message else        /*@1170A*/
     // issue undefined error message                                /*@1170A*/
     if ( pszMemPath != NULL )                                       /*@1170A*/
     {                                                               /*@1170A*/
       UtlErrorHwnd( ERROR_OPEN_TM_PROPERTIES, MB_CANCEL, 1,             /*@1170A*/
                 &pszMemPath, EQF_ERROR, hwnd );                         /*@1170A*/
     } /* endif */                                                   /*@1170A*/
     else                                                            /*@1170A*/
     {                                                               /*@1170A*/
       MemRcHandlingErrorUndefinedHwnd( usMemRc, pReplAddr[0], hwnd );         /*@1170A*/
     } /* endelse */                                                 /*@1170A*/
     break;                                                          /*@1170A*/
   //---------------------------------------------------------------------
   case TM_PROPERTIES_DIFFERENT:
     //--- This rc is returned from TmOpen when the local and remote
     //--- properties of a remote TM are different. This happens when a
     //--- TM owner deletes a remote TM and creates it new with other
     //--- properties (source/target language, markup. location)
     //--- A message is displayed in any case even when TmOpen was called
     //--- with usMesgHandling = FALSE. This case prevents that two messages
     //---  are displayed if the TmOpen is called with usMsgHandling FALSE
     break;
   //---------------------------------------------------------------------
   case ERROR_NETWORK_ACCESS_DENIED :                                  /*@S6A*/
   case BTREE_NETWORK_ACCESS_DENIED:
     // Issue message: Access to %1 failed                             /*@S6A*/
     memcpy( chString, pszMemPath, 1 );                                /*@S6A*/
     UtlErrorHwnd( ERROR_ACCESS_DENIED_MSG, MB_CANCEL, 1,                  /*@S6A*/
               &pReplAddr[0], EQF_ERROR, hwnd );                             /*@S6A*/
     break;                                                            /*@S6A*/
   //---------------------------------------------------------------------
   case SEGMENT_BUFFER_FULL :                                        /*@1108A*/
   case BTREE_BUFFER_SMALL:
     // Issue message: The segment is too large.                     /*@1108A*/
     UtlErrorHwnd( ERROR_MEM_SEGMENT_TOO_LARGE, MB_CANCEL, 0,            /*@1108A*/
               NULL, EQF_ERROR, hwnd );                                    /*@1108A*/
     break;                                                          /*@1108A*/
   //---------------------------------------------------------------------
   case ERROR_INTERNAL :                                             /*@1116A*/
     // Issue message: An internal error or program error occurred.  /*@1116A*/
     UtlErrorHwnd( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR, hwnd ); /*@1116A*/
     break;                                                          /*@1116A*/
   //---------------------------------------------------------------------
   case ERROR_INTERNAL_PARM :                                        /*@1116A*/
     // Issue message: An internal error or program error occurred   /*@1116A*/
     // in file  xxx.xx (line xxx). pReplAddr[0] contains name & line/*@1116A*/
     UtlErrorHwnd( ERROR_INTERNAL_PARM, MB_CANCEL, 1, &pReplAddr[0],     /*@1116A*/
               EQF_ERROR, hwnd );                                          /*@1116A*/
     break;                                                          /*@1116A*/
   //---------------------------------------------------------------------
   case BTREE_DUPLICATE_KEY :
   case BTREE_NOT_FOUND     :
   case BTREE_INVALID       :
   case BTREE_READ_ERROR    :
   case BTREE_EOF_REACHED   :
   case BTREE_EMPTY         :
   case BTREE_OPEN_ERROR    :
   case BTREE_CLOSE_ERROR   :
   case BTREE_NUMBER_RANGE  :
   case BTREE_INV_SERVER    :
   case BTREE_READONLY      :
   case BTREE_DATA_RANGE    :
   case BTREE_MAX_DICTS     :
   case BTREE_NO_EXCLUSIVE  :
   case BTREE_OPEN_FAILED   :
     // Pass on error to generic error handling module
     UtlErrorHwnd( usMemRc, MB_CANCEL, 1, &pReplAddr[0], QDAM_ERROR, hwnd );
     break;
   case ERROR_TA_ACC_TAGTABLE:
     if ( pszServer )
     {
       pReplAddr[0] = pszServer;
     }
     else
     {
       pReplAddr[0] = "";
     } /* endif */
     UtlErrorHwnd( usMemRc, MB_CANCEL, 1, &pReplAddr[0], EQF_ERROR, hwnd );
     break;
   case  BTREE_IN_USE:
     UtlErrorHwnd( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL, 1, &pReplAddr[0],
                   EQF_ERROR, hwnd );
     break;
   //---------------------------------------------------------------------
   default:
       MemRcHandlingErrorUndefinedHwnd( usMemRc, pReplAddr[0], hwnd );
    break;
 } /* endswitch */

 return usMemRc;
} /* end of function MemRcHandling  */

// ======================================================================

VOID MemRcHandlingErrorUndefined( USHORT usMemRc,        // Error code
                                  PSZ    pszMemName )    // Memory name
{
  MemRcHandlingErrorUndefinedHwnd( usMemRc, pszMemName, NULLHANDLE );
}

VOID MemRcHandlingErrorUndefinedHwnd( USHORT usMemRc,        // Error code
                                  PSZ    pszMemName,     // Memory name
                                  HWND   hwnd )

{
  CHAR         chString[50];        // work string
  PSZ          pReplAddr[2];        // array of pointers to replacement strings

  // set the first reply parameter
  pReplAddr[0] = pszMemName;

  //--- dezimal error code is displayed
  sprintf( chString, "%u", usMemRc );

  pReplAddr[1] = chString;
  /********************************************************************/
  /* check if we deal with Morph problems...                          */
  /********************************************************************/
  if ( (usMemRc >= ERR_MORPH_BASE) && (usMemRc <= ERR_MORPH_END)  )
  {
    UtlErrorHwnd( usMemRc, MB_CANCEL, 2, &pReplAddr[0], QDAM_ERROR, hwnd );
  }
  else
  if ( (usMemRc >= ERR_BTREE_BASE) && (usMemRc <= ERR_BTREE_END)  )
  {
    UtlErrorHwnd( usMemRc, MB_CANCEL, 2, &pReplAddr[0], QDAM_ERROR, hwnd );
  }
  else
  {
    UtlErrorHwnd( ERROR_MEM_UNDEFINED, MB_CANCEL, 2,
              &pReplAddr[0], EQF_ERROR, hwnd );
  } /* endif */
} /* end of function MemRcHandlingErrorUndefined */

// ======================================================================



/*---------------------------------------------------------------------*\
                           Read AB Grouping
+-----------------------------------------------------------------------+
  Function name      : ReadABGrouping
  Description        : Builds the AB grouping array
  Function Prototype : ReadABGrouping(pszFileName, pszLanguage,
                                                   abABGrouping)
+-----------------------------------------------------------------------+
  Implementation remarks
  ----------------------
  This function reads the language data from the language file and
  builds the AB Grouping array.

  The language file is in the following format:
  Lines beginning with an asterix are comment lines and are ignored.
  Blank lines are ignored.
  Other lines are considered as language information, where the
  information is grouped as follows:
  First line is the language name in English (With no imbedded blanks)
  The language name case is ignored.
  Subsequent lines begin with a number followed by a colon, blanks,
  and a string of characters belonging to the group.
  The groups can appear in any order. And more then once.
  It is an error to have a group number larger than GROUP_NUM-1, or
  to have a letter appearing more than once.
  Characters in the codepage, not appearing in the language data are
  not part of the alphabet and are assigned GROUP_NUM.
  A group is terminated by the line  99: end of group
  e.g.
  English.
  0: abBC
  1: eE
  2: gqQt
  .
  * Ye comment.
  1: Ac
  7: LmMn
  99: end of group

  Return Codes:
  -------------
  OK (0)       - The command ended successfully.
  LANG_FILE_NOT_FOUND - The language file was not found.
  LANGUAGE_NOT_FOUND - The source language does not exist in the
                       language file.
  LANG_FILE_LINE_TOO_LONG - A file line exceeded the allowed length.
  LANG_GROUP_NUM_TOO_LARGE - A non-existing group number in file.
  LANG_CHAR_APPEARS_TWICE - A character was found in 2 group lines.
  Other values - An API call failed.

  Function Calls
  --------------
  GetLangFileLine

  API calls:
  ----------

  C function calls:
  -----------------
\*---------------------------------------------------------------------*/
//USHORT
//ReadABGrouping (PSZ pszFileName,        /* The language file name......*/
//                PSZ pszLanguage,        /* The language name...........*/
//                ABGROUP abABGrouping)   /* AB grouping array...........*/
//{
//  /*-------------------------------------------------------------------*/
//  USHORT      rc;                       /* Returned Rc.....................*/
//  USHORT      usGroup = 0;              /* The group number................*/
//  LANG_LINE   szLine,                   /* Buffer for file reads...........*/
//              szToken;                  /* Buffer for tokens. .............*/
//  PCHAR       pch;                      /* An index variable...............*/
//  BOOL        fMore = TRUE;             /* A GP flag.......................*/
//  PBYTE       pFileData = NULL;         /* pointer to file data            */
//  PBYTE       pLngData;                 /* temp pointer to file data       */
//  ULONG       ulLength;
//  PSZ         pszToken;
//
//  /*-------------------------------------------------------------------*/
//  /* Load the language file. ..........................................*/
//  rc = (USHORT)UtlLoadFileL( pszFileName, (PVOID *)&pFileData, &ulLength, FALSE, FALSE);
//
//  if (rc == TRUE)
//  {
//    pLngData = pFileData;
//
//    /* Read through the file until you find the language name. ........*/
//    do
//    {
//      rc = UtlGetLangFileLine( &pLngData, szLine, MAX_LINE_LENGTH);
//      if (rc == OK)
//      {
//        /* A line was read, compare the language name. ................*/
//        /* Extract the name from the line, and compare to input........*/
//        UtlStripBlanks( szLine );
//        if (!_strcmpi(szLine, pszLanguage))
//        {
//          /* We've found the language tag. ............................*/
//          /* Initialize the abGrouping array...........................*/
//          memset(abABGrouping, (BYTE)GROUP_NUM, CODEPAGE_SIZE-1);
//
//          /* Now read the group data and assign to array...............*/
//          do
//          {
//            rc = UtlGetLangFileLine( &pLngData, szLine, MAX_LINE_LENGTH);
//            if (rc == OK)
//            {
//              /* we've read a line, parse it to get group data. .......*/
//              /* at first isolate group number */
//              pszToken = strtok( szLine, ": " );
//              if ( pszToken )
//              {
//                usGroup =  (USHORT)atoi( pszToken );
//              } /* endif */
//
//              /* then read the grouping data */
//              pszToken = strtok( NULL, ": \r\n" );
//              if ( pszToken )
//              {
//                strcpy( szToken, pszToken );
//              }
//              else
//              {
//                szToken[0] = EOS;
//              } /* endif */
//
//              /* now check that no more tokens are on the line */
//              pszToken = strtok( NULL, ": \r\n" );
//
//              /* if more or less then 2 tokens read stop processing ...*/
//              if ( (pszToken == NULL) && (usGroup > 0) &&
//                   (strlen( szToken ) > 0) )
//              {
//                /* Check that the group value is within the limits.....*/
//                if (usGroup >= GROUP_NUM)
//                {
//                  rc = LANG_GROUP_NUM_TOO_LARGE;
//                }
//                else
//                {
//                  /* The line was parsed, set the array values.........*/
//                  for (pch = szToken; (*pch != NULC); pch++)
//                  {
//                     if (abABGrouping[*pch] != (BYTE)GROUP_NUM)
//                     {
//                       /* The character appeared already...............*/
//                       *pch = NULC;
//                       rc = LANG_CHAR_APPEARS_TWICE;
//                     }
//                     else
//                       abABGrouping[*pch] = (BYTE)usGroup;
//                  }
//                }
//                /* If we had a LANG_GROUP or a LANG_CHAR error, leave..*/
//                fMore = (rc == OK);
//              }
//              else
//                fMore = FALSE; /* finito :-) */
//            } /* Endif UtlGetLangFileLine was OK */
//            else /* An error in UtlGetLangFileLine */
//            {
//              if (rc == TMERR_EOF)
//              {
//                /* No error, it's just the EOF. .......................*/
//                 rc = OK;
//              }
//              fMore = FALSE;
//            } /* Endelse UtlGetLangFileLine not OK */
//          } while (fMore);
//        } /* Endif found language */
//      } /* Endif UtlGetLangFileLine was OK - to find Language */
//      else /* rc <> OK */
//      {
//        /* An error or EOF. ...........................................*/
//        rc = (rc == TMERR_EOF) ? LANGUAGE_NOT_FOUND : rc;
//        fMore = FALSE;
//      } /* Endelse not OK */
//    } while (fMore);
//  } /* Endif UtlLoadFile was OK */
//
//  /* free memory used for file data */
//  UtlAlloc( (PVOID *) &pFileData, 0L, 0L, NOMSG );
//
//  /* Change the return code for file_not_found.........................*/
//  if (rc == ERROR_OPEN_FAILED)
//    rc = LANG_FILE_NOT_FOUND;
//  return (rc);
//} /* End of ReadABGrouping */


typedef struct _CONVERSIONAREA
{
  BTREE BtreeIn;                      // structure for input BTREE
  BTREE BtreeOut;                     // structure for output BTREE
  CHAR_W  szKey[1024];
  BYTE    bData[0xFFFFFF];
  TMX_SIGN TMXSign;                    // signature record
  char szOutFile[MAX_LONGFILESPEC];
  CHAR szBackupName[MAX_EQF_PATH];
} CONVERSIONAREA, *PCONVERSIONAREA;

// convert single memory file
USHORT MemConvertMemFile( PSZ pszMem, BOOL fDataFile, FILE *hfLog  )
{
  PCONVERSIONAREA pData = NULL;
  SHORT sRc = 0;

  fDataFile;

  if ( UtlAlloc( (PVOID *)&pData, 0, sizeof(CONVERSIONAREA), ERROR_STORAGE ) )
  {
    PBTREE pbTree = &pData->BtreeIn;
    PBTREE pbNewTree = &pData->BtreeOut;

    sRc = QDAMDictOpenLocal( pszMem, 20, ASD_ORGANIZE | ASD_GUARDED, &pbTree );

    // get user data
    if ( !sRc )
    {
      USHORT usLen = sizeof(pData->TMXSign);
      sRc = QDAMDictSignLocal( pbTree, (PCHAR)&pData->TMXSign, &usLen );
    } /* endif */

    if ( !sRc )
    {
      NTMVITALINFO NtmVitalInfo;          // structure to contain vital info for TM

      memset( &(pData->BtreeOut), 0, sizeof(pData->BtreeOut) );

      memcpy( &NtmVitalInfo, pbTree->pBTree->chCollate, sizeof(NTMVITALINFO));

      strcpy( pData->szOutFile, pszMem );
      strcat( pData->szOutFile, ".OUT" );
      pData->TMXSign.bMajorVersion = TM_MAJ_VERSION_7;

      sRc = QDAMDictCreateLocal( pData->szOutFile, 20, (PCHAR)&(pData->TMXSign), sizeof(TMX_SIGN),
                                 NULL, NULL, NULL, &pbNewTree, &NtmVitalInfo );
    } /* endif */

    while ( !sRc )
    {
      ULONG ulDataLen = sizeof(pData->bData);
      ULONG ulKeyLen  = sizeof(pData->szKey) / sizeof(CHAR_W);

      sRc = QDAMDictNextLocal( pbTree, pData->szKey, &ulKeyLen, pData->bData, &ulDataLen );

      if ( !sRc )
      {
        sRc = QDAMDictInsertLocal( pbNewTree, pData->szKey, pData->bData, ulDataLen );
      } /* endif */

    } /*endwhile */

    if ( sRc == BTREE_EOF_REACHED )
    {
      sRc = 0;
    } /* endif */

    QDAMDictCloseLocal( pbTree );
    QDAMDictCloseLocal( pbNewTree );

    if ( !sRc  )
    {
      UtlMakeEQFPath( pData->szBackupName, NULC, SYSTEM_PATH, NULL );
      strcat( pData->szBackupName, "\\MEM_TM611_VERSION\\" );
      strcat( pData->szBackupName, UtlGetFnameFromPath( pszMem ) );

      if ( UtlFileExist( pData->szBackupName ) )
      {
        if ( hfLog ) printf( "backup file %s exist already, ", pData->szBackupName );
        sRc = BTREE_ILLEGAL_FILE;
      }
      else
      {
        UtlDelete( pData->szBackupName, 0L, FALSE );
        UtlMove( pszMem, pData->szBackupName, 0L, FALSE );
        UtlMove( pData->szOutFile, pszMem, 0L, FALSE );
      } /* endif */
   } /* endif */

    UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );
  } /* endif */

  return( sRc );
}

// convert a memory to the new record format
USHORT MemConvertMem( PSZ pszFullMemName )
{
  CHAR szBackupName[MAX_EQF_PATH];
  CHAR szPropName[MAX_EQF_PATH];
  CHAR szIndexName[MAX_EQF_PATH];
  BOOL           fPropFile = FALSE;
  PPROP_NTM      pProp = NULL;
  USHORT         usBytesRead = 0;
  USHORT         usRC = 0;
  FILE           *hfLog = NULL;

  // open log file
  {
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkMultDir( szLogFile, FALSE );
    strcat( szLogFile, "\\MEMCONV.LOG" );

    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "=================================================================\n" );
      fprintf( hfLog, "converting memory file %s...\n", pszFullMemName  );
    } /* endif */
  }

  // create backup directory
  UtlMakeEQFPath( szBackupName, NULC, SYSTEM_PATH, NULL );
  strcat( szBackupName, "\\MEM_TM611_VERSION" );
  UtlMkMultDir( szBackupName, FALSE );

  // complete backup file name
  Utlstrccpy( szBackupName + strlen(szBackupName), UtlGetFnameFromPath( pszFullMemName ), DOT );
  strcat( szBackupName, BACKSLASH_STR );
  strcat( szBackupName, EXT_OF_MEM );

  // load memory property file
  UtlMakeEQFPath( szPropName, NULC, PROPERTY_PATH, NULL );
  strcat( szPropName, BACKSLASH_STR );
  Utlstrccpy( szPropName + strlen(szPropName), UtlGetFnameFromPath( pszFullMemName ), DOT );
  strcat( szPropName, EXT_OF_MEM );
  fPropFile = UtlLoadFile( szPropName, (PVOID *)&pProp, &usBytesRead, FALSE, FALSE );

  if ( fPropFile )
  {
    if ( pProp->stTMSignature.bMajorVersion == TM_MAJ_VERSION_6 )
    {
      if ( hfLog )
      {
        fprintf( hfLog, "   converting memory base file %s... ", pszFullMemName );
      } /* endif */
      usRC = MemConvertMemFile( pszFullMemName, TRUE, hfLog );
      if ( usRC )
      {
        if ( hfLog )
        {
          fprintf( hfLog, "   ... conversion failed with RC=%u\n", usRC );
        } /* endif */
      }
      else
      {
        if ( hfLog )
        {
          fprintf( hfLog, "   ... done\n" );
        } /* endif */
      } /* endif */

      if ( !usRC )
      {
        Utlstrccpy ( szIndexName, pszFullMemName, DOT );
        if ( strcmp( strrchr( pszFullMemName, '.'), EXT_OF_SHARED_MEM ) == 0 )
        {
          strcat( szIndexName, EXT_OF_SHARED_MEMINDEX );
        }
        else
        {
          strcat( szIndexName, EXT_OF_TMINDEX );
        } /* endif */

        if ( hfLog )
        {
          fprintf( hfLog, "   converting memory index file %s... ", szIndexName );
        } /* endif */
        usRC = MemConvertMemFile( szIndexName, FALSE, hfLog );
        if ( usRC )
        {
          if ( hfLog )
          {
            fprintf( hfLog, "   ... conversion failed with RC=%u\n", usRC );
          } /* endif */
        }
        else
        {
          if ( hfLog )
          {
            fprintf( hfLog, "   ... done\n" );
          } /* endif */
        } /* endif */
      } /* endif */

      if ( !usRC )
      {
        pProp->stTMSignature.bMajorVersion = TM_MAJ_VERSION_7;

        if ( UtlFileExist(szBackupName) )
        {
          if ( hfLog )
          {
            fprintf( hfLog, "backup of property file %s exists already as %s, no backup performed\n", szPropName, szBackupName );
          } /* endif */
        }
        else
        {
          UtlMove( szPropName, szBackupName, 0L, FALSE );
          UtlWriteFile( szPropName, usBytesRead, pProp, FALSE );
          if ( hfLog )
          {
            fprintf( hfLog, "updated property file, conversion complete\n" );
          } /* endif */
        } /* endif */
      } /* endif */

    }
    else if ( pProp->stTMSignature.bMajorVersion == TM_MAJ_VERSION_7 )
    {
      usRC = MEM_CONVERTMEM_ALREADYNEWFORMAT;
      if ( hfLog )
      {
        fprintf( hfLog, "   no conversion required, Properties indicate Version 7 memory\n"  );
      } /* endif */
    } /* endif */
    UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );
  } /* endif */

  if ( hfLog ) fclose (hfLog );

  return( usRC  );
}
