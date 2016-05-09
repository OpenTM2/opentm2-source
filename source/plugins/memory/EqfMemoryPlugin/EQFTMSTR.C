//+----------------------------------------------------------------------------+
//|EQFTMSTR.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Authors:      Stefan Doersam                                                |
//|              G. Queck                                                      |
//+----------------------------------------------------------------------------+
//|Description:  Tmt start/end functions for TMs                               |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|  TmtOpen                                                                   |
//|  TmtClose                                                                  |
//|  TmtCreate                                                                 |
//|  OpenTmFile                                                                |
//|  ReadTmHeader                                                              |
//|  AllocTmtGlobals                                                           |
//|  SetTmtWorkPointers                                                        |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
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
// $Revision: 1.2 $ ----------- 7 Nov 2008
// GQ: - code cleanup
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
// $Revision: 1.2 $ ----------- 4 Sep 2002
// --RJ: R907197: del. initdbcs()
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: get rid of compiler warnings
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFTMSTR.CV_   1.2   28 Apr 1997 11:01:30   BUILD  $
 *
 * $Log:   J:\DATA\EQFTMSTR.CV_  $
 *
 *    Rev 1.2   28 Apr 1997 11:01:30   BUILD
 * - allocate work areas in TMT_GLOBALS in a separate chunk of memory as
 *   the overall size of the global structure had exceeded the 64kb limit
 *
 *    Rev 1.1   26 Feb 1997 17:25:10   BUILD
 * -- Compiler defines for _POE22, _TKT21, and NEWTCSTUFF eliminated
 *
 *    Rev 1.0   09 Jan 1996 09:15:40   BUILD
 * Initial revision.
*/
// ----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#include <eqf.h>                  // General Translation Manager include file

#include <EQFTMI.H>               // Private header file of Translation Memory

VOID PASCAL FAR TmCleanUp( USHORT );
                                                                 /*14@AT17A*/
#define FREENODEAREA( pNodeArea )                   \
{                                                   \
  PNODEAREA  pArea, pRoot;                          \
  pRoot = pNodeArea;                                \
  if ( pRoot )                                      \
  {                                                 \
     do                                             \
     {                                              \
       pArea = pRoot;                               \
       pRoot = pArea->pNext;                        \
       UtlAlloc( (PVOID *) (PVOID *)&pArea, 0L, 0L, NOMSG );  \
     } while ( pRoot ); /* enddo */                 \
  } /* endif */                                     \
}

/*---------------------------------------------------------------------*\
|                               Tmt Open                                |
+-----------------------------------------------------------------------|
|  Function name      : TmtOpen                                         |
|  Description        : Open a specific TM file.                        |
|  Function Prototype : TmtOpen (pOpenIn,pOpenOut)                      |
|-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The Open opens a TM file. This function returns various rc values    |
|  according to the file status.                                        |
|                                                                       |
|  If the coruuption flag is up, a suitable returned code will relay    |
|  this information to the caller, the Open will be successful though.  |
|                                                                       |
|  Return Codes :                                                       |
|  -------------                                                        |
|  OK (0)          - The command ended successfully.                    |
|  TM_FILE_NOT_FOUND - The TM file of that name was not found.          |
|  FILE_ALREADY_OPEN - The TM is already open.                          |
|  FILE_MIGHT_BE_CORRUPTED - The TM opened might be corrupted.          |
|  VERSION_MISMATCH - The code version does not match DB version.       |
|  TM_FILE_SCREWED_UP - A discrepency between the Advertised (In the    |
|                       Tm File) TMH size, and the actual size read.    |
|  CORRUPT_VERSION_MISMATCH - DB_MIGHT_BE_CORRUPTED & VERSION_MISMATCH. |
|  Other values - An API call failed.                                   |
|                                                                       |
|  Function calls                                                       |
|  --------------                                                       |
|  OpenTmFile                                                           |
|  ReadTmHeader                                                         |
|  SetTmtWorkPointers                                                   |
|  AllocTmtGlobals                                                      |
|                                                                       |
|  API calls                                                            |
|  ---------                                                            |
\*---------------------------------------------------------------------*/
USHORT
TmtOpen (POPEN_IN  pOpenIn,         /* Pointer to input buffer.........*/
         POPEN_OUT pOpenOut) {      /* Pointer to output buffer........*/
  /*-------------------------------------------------------------------*/
  USHORT  rc,                      /* Returned Rc......................*/
          usFixedBytes;            /* The actual bytes read............*/
  PTM_HEADER   ptmh = NULL;        /* Pointer to TM Header.............*/
  PTMT_GLOBALS ptmtg = NULL;       /* Pointer to TMT Globals...........*/
  ULONG   ulFilePtr;               /* New file pointer.................*/
  UCHAR    achTmPrefix[TM_PREFIX_SIZE + 1];
                                   /* buffer to hold prefix read from .*/
                                   /* TM DB file ......................*/
  PLOADEDTABLE pLoadedTable;       /* temporary pointr for loaded tag table *//*@AT17A*/
  /*-------------------------------------------------------------------*/

  /* Calculate initial allocation size. ...............................*/
  rc = AllocTmtGlobals( (USHORT) (MAX_TMT_GLOBALS_SIZE), &ptmtg);

  if (rc == OK) {
    /* Allocation succeded. ...........................................*/
    ptmh = ptmtg->pTmHeader;
    rc = OpenTmFile(ptmtg, pOpenIn->szTmFileName, TMC_OPEN);

    if (rc == OK) {

      /* Check if a TM DB prefix exist at the beginig of the file......*/
      /* To be on the safe side, change the file pointer. .............*/
      rc = UtlChgFilePtr(ptmtg->hfTM,        /* File Handle............*/
                         0L,                 /* Beginning of file......*/
                         FILE_BEGIN,         /* Where from.............*/
                         &ulFilePtr,         /* New location (Out).....*/
                         FALSE);

      if (rc == OK) {
        /* The file pointer was moved, read the TM DB identifier ......*/
        rc = UtlRead(ptmtg->hfTM,            /* File Handle............*/
                     achTmPrefix,            /* Read Destination.......*/
                     TM_PREFIX_SIZE,         /* Number of bytes to read*/
                     &usFixedBytes,          /* Number read (Out)......*/
                     FALSE);

        if ( (rc == OK) && (usFixedBytes == TM_PREFIX_SIZE) ) {
          achTmPrefix[TM_PREFIX_SIZE] = NULC;

          if ( strcmp(TM_PREFIX, (PSZ)achTmPrefix) != 0) {
            rc = NOT_A_MEMORY_DATABASE ;
            UtlClose( ptmtg->hfTM, FALSE );
          }
        }
        else
        {
          if (rc == OK)
          {
            rc = NOT_A_MEMORY_DATABASE ;
            UtlClose( ptmtg->hfTM, FALSE );
          } /* endif */
        } /* end if (rc == OK) && (usFixedBytes == TM_PREFIX_SIZE) ....*/
      }/* end if (rc == OK) ...........................................*/

      if (rc == OK ) {
        /* Load Tm header..............................................*/
        rc = ReadTmHeader (ptmtg);

        if (rc == OK) {
          /* Set work space pointers: Block Image, Words table and Text*/
          /* Table, then free the unused space.........................*/
          rc = SetTmtWorkPointers(ptmtg);

          if ((rc == OK) && (!ptmh->fCorruption) &&
              (!pOpenIn->fOpenGetPart)) {
            /* Corruption flag is False. Set it to TRUE on the disk....*/
            ptmh->fCorruption = TRUE;
            rc = WriteTmhToDisk(ptmtg, WF_CORRUPT);
            ptmh->fCorruption = FALSE;

          } /* Endif SetTmtWorkPointers was OK */
        } /* Endif ReadTmHeader was OK */
      }/* end if(rc == OK ) it is  a TM DB file according to the prefix*/
    } /* Endif OpenTmFile was OK */

    /* If Something went wrong, free allocation. ......................*/
    if (rc != OK)
    {                                                                   /*@AT17A*/
      /****************************************************************//*@AT17A*/
      /* because now TATagTokenize function is used, clean allocated  *//*@AT17A*/
      /* for the structure for loaded tag table                       *//*@AT17A*/
      /****************************************************************//*@AT17A*/
      if ( (pLoadedTable = (PLOADEDTABLE)ptmtg->pstLoadedTagTable ) != NULL ) /*@AT17A*/
      {                                                                 /*@AT17A*/
        UtlAlloc( (PVOID *) &pLoadedTable->pTagTree,  0L,0L, NOMSG );            /*@AT17A*/
        UtlAlloc( (PVOID *) &pLoadedTable->pAttrTree, 0L,0L, NOMSG );            /*@AT17A*/
        FREENODEAREA( pLoadedTable->pNodeArea );                        /*@AT17A*/
        FREENODEAREA( pLoadedTable->pAttrNodeArea );                    /*@AT17A*/
        UtlAlloc( (PVOID *) &(ptmtg->pstLoadedTagTable), 0L,0L, NOMSG );         /*@AT17A*/
      } /* endif */                                                     /*@AT17A*/
      UtlAlloc( (PVOID *) &ptmtg, 0L, 0L, NOMSG );  /* free memory */
    }/* endif */
  } /* Endif memory allocation was OK */

  /* If rc == OK here then there was no FATAL error....................*/
  if (rc == OK) {
    /* No fatal error: check version and corruption flag ..............*/
    if (ptmh->fCorruption) {
       /* Corruption flag..............................................*/
       if (ptmh->usDbVersion != TMT_CODE_VERSION) {
         /* Code version mismatch and corruption flag .................*/
         rc = CORRUPT_VERSION_MISMATCH;
       } else {
         /* Just corruption flag.......................................*/
         rc = FILE_MIGHT_BE_CORRUPTED;
       }
     } else {
       if (ptmh->usDbVersion != TMT_CODE_VERSION) {
         /* Just Code version mismatch.................................*/
         rc = VERSION_MISMATCH;
       }
     } /* Endif corruption flag is ON */
     /* Set the Open Out structure ....................................*/
     pOpenOut->htm = (HTM) ptmtg;
   } /* Endif no fatal error */
   pOpenOut->prefout.usLenOut = sizeof(OPEN_OUT);

   return rc;
} /* End of TmtOpen  */

/*---------------------------------------------------------------------*\
|                              Tmt Create                               |
+-----------------------------------------------------------------------+
|  Function name      : TmtCreate                                       |
|  Description        : Creates a new TM DB.                            |
|  Function Prototype : TmtCreate (pCreateIn, pCreateOut)               |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The Create function creates a new TM file. It dynamically allocates  |
|  enough space to read the exclude and words tables (for cleaning the  |
|  source), and space for BlockImage ,TextTable and WordsTable which    |
|  are work tables needed for the cleaning.                             |
|  The function builds the TmtGlobals structure and fills in the        |
|  Dos file handle and the pointers to all 'global' tables:             |
|  BlockImage, TextTable, WordsTable, TmHeader.                         |
|  The exclude tag list and exclude word list are read into the file    |
|  TmHeader. Other values in the TmHeader are read from the language    |
|  or taken from the CreateIn buffer.                                   |
|  Once the Tm Header has been built the rest of the space is freed.    |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)             - The command ended successfully.                 |
|  FILE_ALREADY_EXISTS - A TM file of the same name already exists.     |
|  BLOCK_SIZE_TOO_SMALL - The block size passed is too small            |
|  Other values - The OS/2 return code of the failure.                  |
|                                                                       |
|  Function Calls                                                       |
|  --------------                                                       |
|  WriteTmhToDisk                                                       |
|  OpenTmFile                                                           |
|  FormatMore                                                           |
|  AllocateTmtGlobals                                                   |
|  SetTmtWorkPointers                                                   |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
|  UtlOpen                                                              |
|  UtlDelete                                                            |
|                                                                       |
|  C Function Calls                                                     |
|  ----------------                                                     |
|  time                                                                 |
|  strncpy                                                              |
\*---------------------------------------------------------------------*/
USHORT
TmtCreate (PCREATE_IN   pCreateIn,    /* Pointer to input structure....*/
           PCREATE_OUT  pCreateOut) { /* Pointer to output structure...*/
  /*-------------------------------------------------------------------*/
  USHORT     rc;                          /* Returned Rc......................*/
  PTM_HEADER ptmh = NULL;                 /* PTmHeader........................*/
  PTMT_GLOBALS ptmtg = NULL;              /* The pointer that will be the htm.*/
  PLOADEDTABLE pLoadedTable;              /* temporary pointr for loaded tag table *//*@AT17A*/

  /*-------------------------------------------------------------------*/
  /* Allocate initial size.............................................*/
  if (pCreateIn->ctmh.usBlockSize <= BLOCK_HEADER_SIZE) {
    rc = BLOCK_SIZE_TOO_SMALL;
  } else {
  rc =  AllocTmtGlobals((USHORT)(sizeof(TMT_GLOBALS) +
                                 MAX_TM_HEADER_SIZE +
                                 pCreateIn->ctmh.usBlockSize +
                                 MAX_TEXT_TAB_SIZE +
                                 MAX_WORDS_TAB_SIZE),
                        &ptmtg);
  }

  if (rc == OK) {
    /* The memory allocation succeeded. ...............................*/
    /* Set the pointer to TMH in the globals structure.................*/
    ptmh = ptmtg->pTmHeader;
    ptmh->usTmHeaderSize = sizeof(TM_HEADER);

    /* Build AB Grouping array in TM Header. ..........................*/

    memcpy(ptmh->abABGrouping, pCreateIn->abABGrouping, sizeof(ABGROUP));

    if (rc == OK)
    {
      /* Language file and language were found, ABGrouping built.......*/
      /* Read Exclude Tag file into Header.............................*/

      memcpy(((PCHAR)ptmh) + ptmh->usTmHeaderSize,
             &(pCreateIn->bufData[pCreateIn->usDispExclTagList]),
             pCreateIn->usLenExclTagList);

      ptmh->usDispExclTagList = ptmh->usTmHeaderSize;
      ptmh->usLenExclTagList  = pCreateIn->usLenExclTagList;;

      ptmh->usTmHeaderSize = ptmh->usTmHeaderSize + pCreateIn->usLenExclTagList;

      if (rc == OK) {
        /* Tag list loaded OK, read Words list. .......................*/

      memcpy(((PCHAR)ptmh) + ptmh->usTmHeaderSize,
                &(pCreateIn->bufData[pCreateIn->usDispExclWordList]),
                pCreateIn->usLenExclWordList);

      ptmh->usDispExclWordList = ptmh->usTmHeaderSize;
      ptmh->usLenExclWordList  = pCreateIn->usLenExclWordList;;

      ptmh->usTmHeaderSize = ptmh->usTmHeaderSize + pCreateIn->usLenExclWordList;

        if (rc == OK) {
          /* Words list loaded OK, Everything initialized OK. .........*/
          /* Build TM header in memory. ...............................*/
          ptmh->ctmh        = pCreateIn->ctmh;
          strncpy((PSZ)ptmh->achTmPrefix, TM_PREFIX, TM_PREFIX_SIZE);
          ptmh->usDbVersion = TMT_CODE_VERSION;
          ptmh->fCorruption = FALSE,
          ptmh->tCreate     = pCreateIn->tCreate;

          /* Fill parameters of Tm blocks:.............................*/
          /* First block aligned on SECTOR_SIZE boundary...............*/
          ptmh->ldispFirstBlock = ((ptmh->usTmHeaderSize /
                                    SECTOR_SIZE) + 1) * SECTOR_SIZE;
          /* Initialize block information..............................*/
          ptmh->usFirstAvailBlock = 1;
          ptmh->usNumFreeBlocks = 0;
          ptmh->usNumTMBlocks = 0;

          /* Initialize the Key Directory entries to 0.................*/
          memset((PBYTE)ptmh->ausKeyDirectory, NULC,
                 KEY_DIR_SIZE * sizeof(ptmh->ausKeyDirectory[0]));

         /* create The TM DB file (also check if it exists). ..........*/
         rc = OpenTmFile(ptmtg, ptmh->ctmh.szTmFileName, TMC_CREATE);
         if (rc == OK) {
           /* Reallocate the Tmt_Globals structure. ...................*/
           /* Assign values to the workspace pointers in the ptmtg.....*/
           rc = SetTmtWorkPointers(ptmtg);

           if (rc == OK) {
             /* Write Tm header into the TM DB. .......................*/
             rc = WriteTmhToDisk(ptmtg, WF_ALL);
             if (rc == OK) {
               /* Format numTMBlocks blocks. And Write them to disk. ..*/
               ptmtg->pPrefixOut = &(pCreateOut->prefout);
               rc = FormatMore(ptmtg);
               /* the disk is full and the TM cannot be created .......*/
               if (rc == OK) {
                 /* Alles gutte, set the Create Out structure. ........*/
                  pCreateOut->htm = (HTM)ptmtg;
               } /* Endif FormatMore was OK */
             } /* Endif WriteTMHeader was OK */
           } /* Endif memory realloc was OK */
           if (rc != OK) {
             /* We must erase the TM file that was created.............*/
             UtlClose((ptmtg)->hfTM, FALSE);
             UtlDelete(ptmh->ctmh.szTmFileName, 0L, FALSE);
           } /* Endif Need to close the file */
         } /* Endif OpenTmFile was OK */
       } /* Endif ReadExcludeWords was OK */
     } /* Endif ReadExcludeTags was OK */
   } /* Endif ReagABGrouping was OK */

   /* If something went wrong, free allocation. .....................*/
   if (rc != OK)
   {                                                                   /*@AT17A*/
     /****************************************************************//*@AT17A*/
     /* because now TATagTokenize function is used, clean allocated  *//*@AT17A*/
     /* for the structure for loaded tag table                       *//*@AT17A*/
     /****************************************************************//*@AT17A*/
      if ( (pLoadedTable = (PLOADEDTABLE)ptmtg->pstLoadedTagTable) != NULL ) /*@AT17A*/
      {                                                                 /*@AT17A*/
        UtlAlloc( (PVOID *) &pLoadedTable->pTagTree,  0L,0L, NOMSG );            /*@AT17A*/
        UtlAlloc( (PVOID *) &pLoadedTable->pAttrTree, 0L,0L, NOMSG );            /*@AT17A*/
        FREENODEAREA( pLoadedTable->pNodeArea );                        /*@AT17A*/
        FREENODEAREA( pLoadedTable->pAttrNodeArea );                    /*@AT17A*/
        UtlAlloc( (PVOID *) &(ptmtg->pstLoadedTagTable), 0L,0L, NOMSG );         /*@AT17A*/
      } /* endif */                                                     /*@AT17A*/
     UtlAlloc( (PVOID *) &ptmtg, 0L, 0L, NOMSG );
   }
 } /* Endif memory allocation was OK */

 pCreateOut->prefout.usLenOut = sizeof(CREATE_OUT);
 return (rc);
} /* end of TmtCreate */

/*---------------------------------------------------------------------*\
|                               Tmt Close                               |
+-----------------------------------------------------------------------+
|  Function name      : TmtClose                                        |
|  Description        : Closes a TM file                                |
|  Function Prototype : TmtClose (ptmtg, pCloseOut)                     |
| --------------------------------------------------------------------- |
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The Close command closes a TM file. The TM file is identified by its |
|  handle. It is assumed that when the TMT function is invoked with     |
|  the Close command, only one instance of this TM file is open.        |
|  It is the responsibility of MAT/Multi-User interface to ensure that  |
|  this is indeed the case.                                             |
|  if the file was originally opened as a valid TM file, the Corruption |
|  flag on the disk is turned off (set to FALSE).                       |
|  Otherwise (the file was opened as a corrupted file) it is unchanged. |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - A Dos operation like write/close failed.              |
|                                                                       |
|  Function Calls:                                                      |
|  ---------------                                                      |
|  WriteTmhToDisk                                                       |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
|  UtlClose                                                             |
\*---------------------------------------------------------------------*/
USHORT
TmtClose (PTMT_GLOBALS ptmtg,       /* Pointer to Globals structure....*/
          PCLOSE_OUT   pCloseOut) { /* Pointer to output buffer........*/
  /*-------------------------------------------------------------------*/
  USHORT  rc = OK;                  /* Returned Rc.....................*/
  PLOADEDTABLE pLoadedTable;       /* temporary pointr for loaded tag table *//*@AT17A*/
  /*-------------------------------------------------------------------*/
  if ( ptmtg->pTmHeader )
  {
    if (!ptmtg->pTmHeader->fCorruption) {
      /* The corruption flag in memory is false. Reset the flag on.......*/
      /* the disk by overwirting it......................................*/
      rc = WriteTmhToDisk(ptmtg, WF_CORRUPT);
    } /* Endif (fCorruption in memory is FALSE) */
  } /* endif */
  if (rc == OK) {
    /* We wrote the corruption flag. Now. close the file...............*/
    if ( ptmtg->hfTM )
    {
      rc = UtlClose (ptmtg->hfTM, FALSE);
    } /* endif */

    if (rc == OK)
    {                                                                   /*@AT17A*/
      /* The file was closed, free the memory..........................*/
      /****************************************************************//*@AT17A*/
      /* because now TATagTokenize function is used, clean allocated  *//*@AT17A*/
      /* for the structure for loaded tag table                       *//*@AT17A*/
      /****************************************************************//*@AT17A*/
      if ( (pLoadedTable = (PLOADEDTABLE)ptmtg->pstLoadedTagTable) != NULL ) /*@AT17A*/
      {                                                                 /*@AT17A*/
        UtlAlloc( (PVOID *) &pLoadedTable->pTagTree,  0L,0L, NOMSG );            /*@AT17A*/
        UtlAlloc( (PVOID *) &pLoadedTable->pAttrTree, 0L,0L, NOMSG );            /*@AT17A*/
        FREENODEAREA( pLoadedTable->pNodeArea );                        /*@AT17A*/
        FREENODEAREA( pLoadedTable->pAttrNodeArea );                    /*@AT17A*/
        UtlAlloc( (PVOID *) &(ptmtg->pstLoadedTagTable), 0L,0L, NOMSG );         /*@AT17A*/
      } /* endif */

      // free work areas (they are allocated in another data chunk)
      if ( ptmtg->pWorkArea1 != NULL )
      {
         UtlAlloc( (PVOID *)&(ptmtg->pWorkArea1), 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *) &ptmtg, 0L, 0L, NOMSG );
    } /* Endif UtlClose was OK */
  } /* rc == OK */

  pCloseOut->prefout.usLenOut = sizeof (CLOSE_OUT);
  return (rc);
} /* End of TmtClose */


/*---------------------------------------------------------------------*\
|                             Open TM File                              |
+-----------------------------------------------------------------------+
|  Function name      : OpenTmFile                                      |
|  Description        : Opens the TM file                               |
|  Function Prototype : OpenTmFile(ptmtg, pszFileName, idCommand)       |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The function opens the TM file and sets the various Tmt global       |
|  variables. If the command is CREATE then the TM is created. If the   |
|  command is OPEN, the TM is opened.                                   |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  FILE_ALREADY_EXIST - A TM file of the same name already exists.      |
|  TM_FILE_NOT_FOUND - The TM file of that name was not found.          |
|  OK (0)       - The command ended successfully.                       |
|  Other values - An API call failed.                                   |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
|  UtlOpen                                                              |
\*---------------------------------------------------------------------*/
USHORT
OpenTmFile (PTMT_GLOBALS ptmtg,       /* The TM Globals Area...........*/
            PSZ          pszFileName, /* The TM Full File Name.........*/
            USHORT       idCommand) { /* TMC_CREATE / TMC_OPEN.........*/
  /*-------------------------------------------------------------------*/
  USHORT  rc;                      /* Returned Rc......................*/
  USHORT  fsOpenAction;            /* Open Action Flags................*/
  USHORT  usActionTaken;           /* Output parameter.................*/
  ULONG   cbFile;                  /* File size for create.............*/
  USHORT  fsOpenMode =  (USHORT)( OPEN_ACCESS_READWRITE    |
                                  OPEN_SHARE_DENYREADWRITE |
                                  OPEN_FLAGS_FAIL_ON_ERROR |
                                  OPEN_FLAGS_SEQUENTIAL    |
                                  OPEN_FLAGS_WRITE_THROUGH);
  /*-------------------------------------------------------------------*/

  if (idCommand == TMC_CREATE) {
    fsOpenAction = OPEN_ACTION_FAIL_IF_EXISTS |
                   OPEN_ACTION_CREATE_IF_NEW;
    cbFile = ptmtg->pTmHeader->ldispFirstBlock - 1L;
  }
  else /* idCommand == TMC_OPEN */ {
    fsOpenAction = OPEN_ACTION_OPEN_IF_EXISTS |
                   OPEN_ACTION_FAIL_IF_NEW;
    cbFile = 0;
  }

  rc = UtlOpen(pszFileName,              /* File Name..................*/
               &(ptmtg->hfTM),           /* File Handle (Out)..........*/
               &usActionTaken,           /* File Handle (Out)..........*/
               cbFile,                   /* File size irrelevant.......*/
               FILE_NORMAL,              /* Open attributes............*/
               fsOpenAction,             /* Open action flags..........*/
               fsOpenMode,               /* Open Mode..................*/
               0L,                       /* Reserved...................*/
               FALSE);

  if (rc == ERROR_OPEN_FAILED) {
    /* The TM already exists...........................................*/
    rc = (idCommand == TMC_CREATE) ? FILE_ALREADY_EXISTS
                                   : TM_FILE_NOT_FOUND;
  }
  else
  {
    switch ( rc )
    {
      case ERROR_FILE_NOT_FOUND:
        rc = TM_FILE_NOT_FOUND;
        break;
      default:
        break;
    } /* endswitch */
  } /* endif */

  return (rc);
} /* End of OpenTmFile */

/*---------------------------------------------------------------------*\
|                            Read TM Header                             |
+-----------------------------------------------------------------------+
|  Function name      : ReadTmHeader                                    |
|  Description        : Reads a TM Header to memory.                    |
|  Function Prototype : ReadTmHeader(ptmtg)                             |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The function reads the TM Header for the TM file on disk. Firstly it |
|  allocates enough memory to read the fixed size of the header. Then   |
|  it reads the fixed part in. Next it uses the real size of the header |
|  (in usTmHeaderSize which is in the fixed part) to reallocte the      |
|  allocated memory. Then reads the rest of the TM Header in.           |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  TM_FILE_SCREWED_UP - A discrepency between the Advertised (In the    |
|                       Tm File) TMH size, and the actual size read.    |
|  OK (0)       - The command ended successfully.                       |
|  Other values - An API call failed.                                   |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
|  UtlChgFilePtr                                                        |
|  UtlRead                                                              |
\*---------------------------------------------------------------------*/
USHORT
ReadTmHeader (PTMT_GLOBALS ptmtg) {/* Tmt Globals pointer..............*/
  /*-------------------------------------------------------------------*/
  USHORT  rc,                       /* Returned Rc.....................*/
          usFixedBytes,             /* The actual bytes read...........*/
          usVariableBytes;          /* The bytes read in the UtlRead...*/
  ULONG   ulFilePtr;                /* New file pointer................*/
  PTM_HEADER ptmh = ptmtg->pTmHeader; /* Pointer to TM Header..........*/
  HFILE      hfTM = ptmtg->hfTM;    /* File Handle.....................*/
  /*-------------------------------------------------------------------*/
  /*-------------------------------------------------------------------*/
  /* To be on the safe side, change the file pointer. .................*/
  rc = UtlChgFilePtr(hfTM,               /* File Handle................*/
                     0L,                 /* Beginning of file..........*/
                     FILE_BEGIN,         /* Where from.................*/
                     &ulFilePtr,         /* New location (Out).........*/
                     FALSE);

  if (rc == OK) {
    /* The file pointer was moved, read the fixed part of the TMH......*/
    rc = UtlRead(hfTM,                   /* File Handle................*/
                 (PBYTE)(ptmh),          /* Read Destination...........*/
                 sizeof (TM_HEADER),     /* Number of bytes to read....*/
                 &usFixedBytes,          /* Number read (Out)..........*/
                 FALSE);

    if (rc == OK) {
      /* The Read succeeded, read the rest of the TM ..................*/
      rc = UtlRead(hfTM,
                   (PBYTE)(ptmh) + sizeof(TM_HEADER),
                   (USHORT) (ptmh->usTmHeaderSize - sizeof(TM_HEADER)),
                   &usVariableBytes, FALSE);

      /* If the read was OK, check that the TMH size is OK ............*/
      if ((rc == OK) &&
          (ptmh->usTmHeaderSize != usFixedBytes + usVariableBytes)) {
        /* The Tm file was corrupted!..................................*/
        rc = TM_FILE_SCREWED_UP;
        UtlClose( hfTM, FALSE );
      } /* Endif the Variable UtlRead was OK */
    } /* Endif Fixed UtlRead was OK */
  } /* Endif UtlChgFilePtr was OK */
  return rc;
} /* End of ReadTmHeader */

/*---------------------------------------------------------------------*\
|                         Set Tmt Work Pointers                         |
+-----------------------------------------------------------------------+
|  Function name      : SetTmtWorkPointers                              |
|  Description        : Sets The ptmtg pointers.                        |
|  Function Prototype : SetTmtWorkPointers(ptmtg, pchTmData)            |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The function assigns values to the various Tmt global workspace      |
|  pointers.                                                            |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
\*---------------------------------------------------------------------*/
USHORT
SetTmtWorkPointers (PTMT_GLOBALS ptmtg) {/* The TM Globals Area........*/
  /*-------------------------------------------------------------------*/
  USHORT  rc = OK;                    /* Returned Rc...................*/
//!!!  USHORT  usTotalSize;           /* Effective Storage to allocate */
  PTM_HEADER ptmh = ptmtg->pTmHeader; /* Pointer to Tm Header..........*/
  PLOADEDTABLE pLoadedTable;       /* temporary pointr for loaded tag table *//*@AT17A*/
  /*-------------------------------------------------------------------*/

  /* Assign the pointers...............................................*/
  ptmtg->pchBlockImage = ((PUCHAR)(ptmh)) + ptmh->usTmHeaderSize;
  ptmtg->pteTextTable  = (PTOKENENTRY)(ptmtg->pchBlockImage +
                                     ptmh->ctmh.usBlockSize);
  ptmtg->pteWordsTable = (PTOKENENTRY)(((PCHAR)ptmtg->pteTextTable) +
                                       MAX_TEXT_TAB_SIZE);
  ptmtg->pWorkArea2    = ptmtg->pWorkArea1 + MAX_TM_WORK_AREA;

  /*****************************************************************//*@AT17A*/
  /* set pointer to tag table located in TMT_GLOBALS structure into*//*@AT17A*/
  /* loaded tag table structure. Tag table name not needed => EOS  *//*@AT17A*/
  /*****************************************************************//*@AT17A*/
  pLoadedTable = (PLOADEDTABLE)ptmtg->pstLoadedTagTable;             /*@AT17A*/
  pLoadedTable->szName[0] = EOS;                                     /*@AT17A*/
  pLoadedTable->pTagTable =                                          /*@AT17A*/
    (PTAGTABLE)(((PCHAR) ptmh) + ptmh->usDispExclTagList);           /*@AT17A*/

// !!!  /* Free unused memory................................................*/
// !!!  usTotalSize =   sizeof(TMT_GLOBALS) + ptmh->usTmHeaderSize +
// !!!                  ptmh->ctmh.usBlockSize + MAX_TEXT_TAB_SIZE +
// !!!                  MAX_WORDS_TAB_SIZE +
// !!!                  (2 * MAX_TM_WORK_AREA); /* Size......................*/
// !!!
// !!!  rc = DosReallocSeg( usTotalSize, SELECTOROF(ptmtg));
  return rc;
} /* End of SetTmtWorkPointers */

/*---------------------------------------------------------------------*\
|                         Alloc Tmt Globals                             |
+-----------------------------------------------------------------------+
|  Function name      : AllocTmtGlobals                                 |
|  Description        : Allocates a Tmg Globals structure               |
|  Function Prototype : AllocTmtGlobals(usTmtGlobalsSize, pptmtg)       |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The function allocates the Tmt globals structure and assigns the     |
|  pTmHeader                                                            |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
\*---------------------------------------------------------------------*/
USHORT
AllocTmtGlobals (USHORT usTmtGlobalsSize, /* The allocateion size......*/
                 PPTMT_GLOBALS pptmtg) {  /* The TM Globals Area.......*/
  /*-------------------------------------------------------------------*/
  USHORT  rc = OK;                 /* Returned Rc......................*/
  /*-------------------------------------------------------------------*/

  /* Allocate the segment..............................................*/
  rc = (USHORT)(!UtlAlloc( (PVOID *) pptmtg, 0L, (ULONG) usTmtGlobalsSize, NOMSG ));
   if ( rc != OK )
   {
    rc = ERROR_NOT_ENOUGH_MEMORY;                                  /*@AT17A*/
   } /* endif */

  if (rc == OK)
  {
    /* Initialize the allocated memory to 0 ..........................*/
    memset ( *pptmtg, NULC, usTmtGlobalsSize );

    /* The memory allocation succeeded. ..............................*/
    /* Allocate set the pointert to the tm header ....................*/
    (*pptmtg)->pTmHeader = (PTM_HEADER)(((PCHAR)(*pptmtg)) +
                           sizeof(TMT_GLOBALS));
    /***************************************************************//*@AT17A*/
    /* allocate storage for loaded tagtable structure              *//*@AT17A*/
    /***************************************************************//*@AT17A*/
    rc = (USHORT)(!UtlAlloc( (PVOID *) &(*pptmtg)->pstLoadedTagTable, 0L,               /*@AT17A*/
                    (LONG)sizeof( LOADEDTABLE ), NOMSG ));            /*@AT17A*/
    if ( rc != OK )                                                  /*@AT17A*/
    {                                                                /*@AT17A*/
      UtlAlloc( (PVOID *) pptmtg, 0L, 0L, NOMSG );                             /*@AT17A*/
      rc = ERROR_NOT_ENOUGH_MEMORY;                                  /*@AT17A*/
    }
    else
    {
      //  allocate storage for work areas
     rc = (USHORT)(!UtlAlloc( (PVOID *) &(*pptmtg)->pWorkArea1, 0L,               /*@AT17A*/
                       (LONG)( 2 * MAX_TM_WORK_AREA), NOMSG ));            /*@AT17A*/
       if ( rc != OK )                                                  /*@AT17A*/
       {                                                                /*@AT17A*/
        UtlAlloc( (PVOID *) &(*pptmtg)->pstLoadedTagTable, 0L, 0L, NOMSG );
         UtlAlloc( (PVOID *) pptmtg, 0L, 0L, NOMSG );                             /*@AT17A*/
         rc = ERROR_NOT_ENOUGH_MEMORY;                                  /*@AT17A*/
       }
    } /* endif */                                                    /*@AT17A*/
  } /* endif */
  return rc;
} /* End of AllocTmtGlobals */


