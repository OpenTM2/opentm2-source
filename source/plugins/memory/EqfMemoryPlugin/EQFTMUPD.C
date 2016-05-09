//+----------------------------------------------------------------------------+
//|EQFTMUPD.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: G. Queck                                                            |
//|        Stefan Doersam                                                      |
//+----------------------------------------------------------------------------+
//|Description: Translation Memory update functions: Add, Delete, Replace      |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|  TmtAdd                                                                    |
//|  TmtDelete                                                                 |
//|  TmtReplace                                                                |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|  AddSegToCluster                                                           |
//|  FindFreshAddrInCluster                                                    |
//|  TakeBlockFromSpool                                                        |
//|  WriteStringToDisk                                                         |
//|  WriteBlock                                                                |
//|  FindMatchSegInCluster                                                     |
//|  CheckMatchForDelete                                                       |
//|  DeleteSegment                                                             |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#include <eqf.h>                  // General Translation Manager include file

#include <EQFTMI.H>               // Private header file of Translation Memory

/*---------------------------------------------------------------------*\
|                       Tmt Add                                         |
+-----------------------------------------------------------------------+
|  Function name      : TmtAdd                                          |
|  Description        : Adds a segment to a TM.                         |
|  Function Prototype : TmtAdd (ptmtg, pAddIn, pAddOut)                 |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  Add adds a segment to a TM database.                                 |
|                                                                       |
|  The source is fetched from the AddIn buffer, and two parsing tables  |
|  are created as the result of "cleaning" the source sentence.         |
|  TextTable identifies tags and text within the source, and WordsTable |
|  identifies words, noise words and useless words (like all caps       |
|  words).  These tables enable to build the primary key.  This key     |
|  enables thru chaining process, to find the last block of the cluster |
|  to which the new segment is added.  if the cluster is still empty, a |
|  new block is fetched from the preformatted blocks, Key directory and |
|  Tm Header are updated.  Otherwise, a search for the first available  |
|  byte in the last cluster is performed.                               |
|  Finally, the segment is written into the TM file.                    |
|                                                                       |
|  The AddIn buffer has the same structure as SEGEMENT, except for      |
|  some fields in its beginning, and is written directly into           |
|  achBlockImage.                                                       |
|  Finally AddOut is assigned with the output of the Add command        |
|                                                                       |
|  Two internal rc-s are used:                                          |
|  rcAdd1 indicates problems while accessing the first block of the     |
|          cluster, if the cluster was empty; or problems during        |
|          tracking until the last block in the cluster is reached.     |
|  rcAdd2 indicates problems during writing of added segment to TM      |
|                                                                       |
|  The final value returned is in rcAdd.                                |
|                                                                       |
|  The Address field of AddOut is meaningless unless the return code    |
|  is OK.                                                               |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - A Dos operation like write/close failed.              |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  GetFirstSegAddress                                                   |
|  FindFreshAddrInCluster                                               |
|  AddSegToCluster                                                      |
\*---------------------------------------------------------------------*/
USHORT
TmtAdd (PTMT_GLOBALS ptmtg,              /* Pointer to Tmt Globals.....*/
        PADD_IN      pAddIn,             /* Pointer to input buffer....*/
        PADD_OUT     pAddOut) {          /* Pointer to output buffer...*/
  /*-------------------------------------------------------------------*/
  USHORT     rc,                         /* Returned rc................*/
             rcAdd;                      /* Problems in adding segment.*/
  TM_ADDRESS   addr;                     /* Pointer to an address......*/
  PSEGMENT     pseg = &(pAddIn->segIn);  /* Temporary segment pointer..*/
  /*-------------------------------------------------------------------*/

  ptmtg->pPrefixOut = (PPREFIX_OUT) &(pAddOut->prefout);
  /* Get address of first segment in cluster in pAddr..................*/
  GetFirstSegAddress (ptmtg,
                      pseg->bufData + pseg->usDispSource,
                      &addr);

  /* Get the first free address in the cluster.........................*/
  rc = FindFreshAddrInCluster (ptmtg, &addr, TRUE);

  /* Check if there are any problems...................................*/
  if (rc == OK) {
    /* Build AddOut and add the segment to the cluster.................*/
    pAddOut->addr = addr;
    rcAdd = AddSegToCluster(ptmtg, &addr, pseg);
    if (rcAdd != OK) rc = rcAdd;
  } /* Endif Find Fresh was OK */

  pAddOut->prefout.usLenOut = sizeof(ADD_OUT);

  return (rc);
} /* End of TmtAdd */

/*---------------------------------------------------------------------*\
|                       Tmt Delete                                      |
+-----------------------------------------------------------------------+
|  Function name      : TmtDelete                                       |
|  Description        : Deletes a segment from the TM.                  |
|  Function Prototype : TmtDelete (ptmtg,pDeleteIn , pDeleteOut)        |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  Delete is used to delete a segment from the TM file. The segment to  |
|  be deleted is given as input. It will be deleted only if an exact    |
|  match is found and the control information also matches perfectly.   |
|  As soon as the first match is found, the segment is deleted and no   |
|  other matches are looked for.                                        |
|  A Delete consists of three steps:                                    |
|  1. Find the cluster from the input segment. (GetFirstSegAddress)     |
|  2. Scan the cluster to see if there is any matching segment.         |
|     The scan terminates when the first match is found.                |
|     (FindMatchSegInCluster).                                          |
|  3. Delete the matching segment from the TM file.                     |
|  A Physical delete is performed if the segment is the last one in its |
|  cluster. Otherwise, a logical delete, that only raises the fDelFlag  |
|  in the segment to TRUE, is performed.                                |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK            - The command ended successfully.                      |
|  SEG_NOT_FOUND - The segment to be deleted is not in the TM.          |
|  CLUSTER_EMPTY - The cluster to which the segment belongs is empty.   |
|  Other values  - A Dos operation like write/close failed.             |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  GetFirstSegAddress,                                                  |
|  FindMatchSegInCluster,                                               |
|  DeleteSegment                                                        |
\*---------------------------------------------------------------------*/
USHORT
TmtDelete (PTMT_GLOBALS ptmtg,            /* Pointer to Tmt Globals....*/
           PDEL_IN      pDeleteIn,        /* Pointer to input buffer...*/
           PDEL_OUT     pDeleteOut) {     /* Pointer to output buffer..*/
  /*-------------------------------------------------------------------*/
  USHORT    rc;                      /* Returned rc....................*/
  BOOL      fLastSeg;                /* Indicates if last seg reached. */
  PSEGMENT  psegDel = &(ptmtg->seg), /* The segment to be deleted......*/
            psegIn = &(pDeleteIn->segIn); /* Input segment.............*/
  PSZ       pszSource = &(psegIn->bufData[psegIn->usDispSource]);
                                     /* Source of segment..............*/
  TM_ADDRESS  addr;                  /* Address to be returned.........*/
  /*-------------------------------------------------------------------*/

  ptmtg->pPrefixOut = (PPREFIX_OUT) &(pDeleteOut->prefout);

  /* Put address of first segment in cluster into addr.................*/
  GetFirstSegAddress (ptmtg, pszSource, &addr);

  /* pAddr points initially to the first segment address.  On exit.....*/
  /* from the function it points to the deleted segment.  fLastSeg.....*/
  /* indicates if the segment found is the last one....................*/
  rc = FindMatchSegInCluster (ptmtg, &addr, &fLastSeg, psegIn, psegDel);

  // Convert the cluster empty return code into segment not found
  if (rc == CLUSTER_EMPTY) {
    rc = SEG_NOT_FOUND;
  } /* endif */

  /* Possible return codes OK, SEG_NOT_FOUND etc.......................*/
  if (rc == OK) {
    /* The segment was found, delete physically or logically...........*/
    pDeleteOut->addr = addr;
    rc = DeleteSegment (ptmtg, &addr, fLastSeg, psegDel);
  }

  pDeleteOut->prefout.usLenOut = sizeof(DEL_OUT);
  return (rc);
} /* End of TmtDelete */

/*---------------------------------------------------------------------*\
|                       Tmt Replace                                     |
+-----------------------------------------------------------------------+
|  Function name      : TmtReplace                                      |
|  Description        : Replaces a segment in TM.                       |
|  Function Prototype : TmtReplace (ptmtg, pReplaceIn, pReplaceOut)     |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - A Dos operation like write/close failed.              |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  GetFirstSegAddress                                                   |
|  FindMatchSegInCluster                                                |
|  DeleteSegment                                                        |
|  FindFreshAddrInCluster                                               |
|  AddSegToCluster                                                      |
\*---------------------------------------------------------------------*/
USHORT
TmtReplace (PTMT_GLOBALS ptmtg,           /* Pointer to TmtGlobals     */
           PREP_IN      pReplaceIn,       /* Pointer to input buffer   */
           PREP_OUT     pReplaceOut) {    /* Pointer to output buffer  */
  /*-------------------------------------------------------------------*/
  USHORT     rc,                      /* Returned rc...................*/
             rcAdd;                   /* Problems during add...........*/
  BOOL       fLastSeg;                /* Indicates if last seg reached.*/
  PSEGMENT   psegDel = &(ptmtg->seg), /* Temporary segment.............*/
             psegIn = &(pReplaceIn->segIn);/* Input segment............*/
  TM_ADDRESS  addr;                   /* Saves address to return.......*/
  PSZ       pszSource = &(psegIn->bufData[psegIn->usDispSource]);
                                      /* Source of segment.............*/
  /*-------------------------------------------------------------------*/


  ptmtg->pPrefixOut = (PPREFIX_OUT) &(pReplaceOut->prefout);

  pReplaceOut->addrDel.usEntryInDir   = 0;
  pReplaceOut->addrDel.usBlockNumber  = 0;
  pReplaceOut->addrDel.usDispBlockPtr = 0;

  /* Put address of first segment in cluster into addrDel..............*/
  GetFirstSegAddress (ptmtg, pszSource, &addr);

  /* addrDel initially points to the first segment address. On exit....*/
  /* from the function it points to the segment to be deleted..........*/
  rc = FindMatchSegInCluster (ptmtg, &addr, &fLastSeg, psegIn, psegDel);

  /********************************************************************/  /*@G7A*/
  /* FindMatchSegInCluster returns only with rc == OK when psegIn and */  /*@G7A*/
  /* pSegDel matches "exact exact". This means that in both segments  */  /*@G7A*/
  /* the filenames, segment numbers (==position in file), industry    */  /*@G7A*/
  /* codes and the source text (ignoring CRLF) are exactly the same   */  /*@G7A*/
  /* and psegIn is younger than pSegDel. If psegIn is older than      */  /*@G7A*/
  /* pSegDel rc = NOT_REPLACED_OLD_SEGMENT is returned. If the        */  /*@G7A*/
  /* cluster is empty rc = CLUSTER_EMPTY is returned. If no "exact    */  /*@G7A*/
  /* exact" segment is found rc = SEG_NOT_FOUND is returned.          */  /*@G7A*/
  /*                                                                  */  /*@G7A*/
  /* REPLACE TABLE                                                    */  /*@G7A*/
  /* pDelSeg will be replaced only if pInsSeg is younger. The         */  /*@G7A*/
  /* following table shows how human and machine translations are     */  /*@G7A*/
  /* treated. Replace means DeleteSegment and AddSegToCluster.        */  /*@G7A*/
  /* +-------------Â----------------Â---------+                       */  /*@G7A*/
  /* | pDelSeg     | pInSeg         |         |                       */  /*@G7A*/
  /* |             | younger transl.|         |                       */  /*@G7A*/
  /* +-------------Å----------------Å---------+                       */  /*@G7A*/
  /* | human       | human          | Replace |                       */  /*@G7A*/
  /* +-------------Å----------------Å---------+                       */  /*@G7A*/
  /* | machine     | machine        | Replace |                       */  /*@G7A*/
  /* +-------------Å----------------Å---------+                       */  /*@G7A*/
  /* | human       | machine        | No      |                       */  /*@G7A*/
  /* +-------------Å----------------Å---------+                       */  /*@G7A*/
  /* | machine     | human          | Replace |                       */  /*@G7A*/
  /*  -------------Á----------------Á---------+                       */  /*@G7A*/
  /********************************************************************/  /*@G7A*/
  /********************************************************************/  /*@G7A*/
  /* if a "exact exact" match is found and psegIn is younger than     */  /*@G7A*/
  /* pSegDel than delete pSegDel that psegIn can be added (== replace)*/  /*@G7A*/
  /********************************************************************/  /*@G7A*/
  if (rc == OK && psegDel->tStamp != psegIn->tStamp)                      /*@G7A*/
  {                                                                       /*@G7A*/
    /* A match was found which is older, delete it but only if input...*/ /*@G7C*/
    /* segment is no machine translation and found segment is a........*/ /*@G7A*/
    /* human translation ..............................................*/ /*@G7A*/
    if ( (psegIn->usTranslationFlag == TRANSLFLAG_NORMAL) ||                             
         (psegDel->usTranslationFlag == psegIn->usTranslationFlag) )
    {                                                                     /*@G7A*/
      /* Assign the addr before the delete ..............................*/
      /* because the delete might update the addr........................*/
      pReplaceOut->addrDel = addr;
      rc = DeleteSegment (ptmtg, &addr, fLastSeg, psegDel);
      if (rc == OK) addr = pReplaceOut->addrDel;
    } /* endif */                                                         /*@G7A*/
  }/* Endif (rc==OK && not the same date ) ..............................*/

  /********************************************************************/  /*@G7A*/
  /* if no segment or no "exact exact" one is found (SEG_NOT_FOUND || */  /*@G7A*/
  /* CLUSTER_EMPTY ) add segment anyway. See REPLACE TABLE above to   */  /*@G7A*/
  /* see how machine-, human translation and age is treated.          */  /*@G7A*/
  /********************************************************************/  /*@G7A*/
  if ( ( (rc == OK) && (psegDel->tStamp != psegIn->tStamp)                /*@G7C*/
       && !(!(psegDel->usTranslationFlag != TRANSLFLAG_NORMAL) && (psegIn->usTranslationFlag != TRANSLFLAG_NORMAL) ) )        
       || (rc == SEG_NOT_FOUND) || (rc == CLUSTER_EMPTY) )               

  {                                                                       /*@G7C*/
    /* Start add operation with the addr found by the delete...........*/
    /* Because a physical delete might have caused the block image to..*/
    /* be inconsistant with the addrDel, we must reload it, thus, we...*/
    /* pass TRUE as the third parameter................................*/
    rc = FindFreshAddrInCluster (ptmtg, &addr, TRUE);
    /* At this point achBlockImage is prepared in memory...............*/
    if (rc == OK) {
      pReplaceOut->addrAdd = addr;

      /* AddSegToCluster fills in missing fields of the segment and...*/
      /* writes it in address pAddr...................................*/
      rcAdd = AddSegToCluster(ptmtg, &addr, psegIn);

      if (rcAdd != OK) rc = rcAdd;
    } /* Endif (rc was OK or warning) */
  } /* Endif Delete was OK */

  /* Build the ReplaceOut Buffer and exit................................*/
  pReplaceOut->prefout.usLenOut = sizeof(REP_OUT);
  return (rc);

} /* End of TmtReplace */

/*---------------------------------------------------------------------*\
|                       Add Seg To Cluster                              |
+-----------------------------------------------------------------------+
|  Function name      : AddSegToCluster                                 |
|  Description        : This function appends a segment to the end of   |
|                       a cluster.                                      |
|  Function prototype : AddSegToCluster(ptmtg, pAddr, pseg)             |
+-----------------------------------------------------------------------+
|  Implementation Remarks                                               |
|  ----------------------                                               |
|  This function adds a segment to the end of its cluster. It is given  |
|  an initial address and it starts scanning from this address, in      |
|  order to find the cluster's end. Once the appending location is      |
|  found, it prepares the segment for storage by computing the data     |
|  which is missing in the input form of the segment. Then it writes    |
|  the segment to the disk and handles all possible return codes.       |
|                                                                       |
|  Return Codes                                                         |
|  ------------                                                         |
|  0 - OK                                                               |
|  Other values - A Dos operation like write/close failed.              |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  CalcSecondaryKey                                                     |
|  WriteStringToDisk                                                    |
\*---------------------------------------------------------------------*/
USHORT
AddSegToCluster (PTMT_GLOBALS  ptmtg,      /* Pointer to globals area..*/
                 PTM_ADDRESS   pAddr,      /* Ptr to initial address...*/
                 PSEGMENT      pseg) {     /* Pointer to a segment.....*/
  /*-------------------------------------------------------------------*/
  SZSECKEY     szSecKey;                   /* The secondary key........*/
  USHORT       rc;                         /* returned rc..............*/
  static OLDSEGMENT OldSeg;                // buffer for V1.0 format segment
  /*-------------------------------------------------------------------*/
  /* Fill in the missing fields in the segment....................,....*/
  strncpy((PSZ)(pseg->achSegMarker), SEGMARKER, SEG_MARKER_LENGTH);
  pseg->fLogicalDel = FALSE;

  // build V1 segment in our static segment buffer
  {
    memcpy( &OldSeg, pseg, FIELDOFFSET(SEGMENT, szLongName[0]) );
    memcpy( (PUCHAR)&OldSeg + FIELDOFFSET(OLDSEGMENT, usSegNumber),
            (PUCHAR)pseg + FIELDOFFSET(SEGMENT, usSegNumber ),
            pseg->usLenSegment - FIELDOFFSET(SEGMENT, usSegNumber) );
    OldSeg.usLenSegment    -= sizeof(pseg->szLongName);
  }

  /* The secondary key, after the reserved area........................*/
  CalcSecondaryKey (ptmtg, (PSZ)szSecKey);
  OldSeg.usDispSecKey = OldSeg.usLenSegment -
                             FIELDOFFSET(OLDSEGMENT, bufData[0]);
  OldSeg.usLenSecKey  = (USHORT)(strlen((PSZ)szSecKey) + 1);                        /*@1108M*/

  //--- check if the the data buffer of SEGMENT structure overflowes /*@1108A*/
  //--- This happens when the source & translation & the secondary   /*@1108A*/
  //--- key exceeds the maximum size of the data buffer. See the     /*@1108A*/
  //--- definition of SEGMENT in EQFTMDEF.H                          /*@1108A*/
  if ( OldSeg.usLenSegment > sizeof(BUFFERIN) - MAX_SEC_LENGTH )      /*@1108A*/
  {                                                                  /*@1108A*/
    //--- buffer is full set rc                                      /*@1108A*/
    rc =  SEGMENT_BUFFER_FULL;                                       /*@1108A*/
  }                                                                  /*@1108A*/
  else                                                               /*@1108A*/
  {                                                                  /*@1108A*/
    strcpy( OldSeg.bufData + OldSeg.usDispSecKey, (PSZ)szSecKey);

    /* Updating lenSegment in SEGMENT structure..........................*/
    OldSeg.usLenSegment = OldSeg.usLenSegment + OldSeg.usLenSecKey;

    rc = WriteStringToDisk (ptmtg, (PCHAR)&OldSeg,
                            OldSeg.usLenSegment, pAddr);
  } /* endif */                                                      /*@1108A*/
  return (rc);
} /* End of AddSegToCluster */

/*---------------------------------------------------------------------*\
|                       Find Fresh Addr In Cluster                      |
+-----------------------------------------------------------------------+
|  Function name      : FindFreshAddrInCluster                          |
|  Description        : Finds the first address to which a segment can  |
|                       be written.                                     |
|  Function Prototype : FindFreshAddrInCluster ( ptmtg , pAddr,         |
|                                                fReadFirstBlock)       |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function finds the address where a new segment should be added. |
|  The search for the address is done within one cluster, and starts    |
|  with the block in pAddr. Upon exit pAddr points to the fresh address.|
|  The cluster is scanned until the last block associated with it is    |
|  read. If the block is full, a new block is brought from the spool.   |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - A Dos operation like write/close failed.              |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  ReadBlock                                                            |
|  TakeBlockFromSpool                                                   |
\*---------------------------------------------------------------------*/
USHORT
FindFreshAddrInCluster (PTMT_GLOBALS ptmtg,  /* Pointer to Tmt globals.*/
                        PTM_ADDRESS  pAddr,  /* Pointer to address.....*/
                        BOOL fReadFirstBlock) { /* Read block flag.....*/
  /*-------------------------------------------------------------------*/
  USHORT   rc = OK,                     /* returned rc.................*/
           usBlockSize;                 /* Block Size in bytes.........*/
  PBLOCK_HEADER pbh = (PBLOCK_HEADER) ptmtg->pchBlockImage;
                                        /* Pointer to block header.....*/
  PTM_HEADER ptmh = ptmtg->pTmHeader;   /* A fine car..................*/
  /*-------------------------------------------------------------------*/
  /* If the cluster is empty: add the first block to the cluster.......*/
  /* otherwise read until last block of cluster........................*/
  if (pAddr->usBlockNumber == 0)
  {
    /* The cluster is empty, take the first available block and update.*/
    /* the key directory...............................................*/
    rc = TakeBlockFromSpool(ptmtg, OK, pAddr);
    if (rc == OK)
    {
      ptmh->ausKeyDirectory[pAddr->usEntryInDir] = pAddr->usBlockNumber;
      rc = WriteTmhToDisk(ptmtg, WF_KEYDIR);
    }
  }
  else /* cluster is not empty */ {
    /* if fReadFirstBlock is TRUE, read the block whose addr is in.....*/
    /* pAddr into block image (this happens in Add, not in Replace)....*/
    if (fReadFirstBlock) rc = ReadBlock(ptmtg, pAddr, TRUE);

    /* Scan the cluster until last block is found. ....................*/
    usBlockSize = ptmh->ctmh.usBlockSize;
    while ((pbh->usNextBlock != 0)  &&
           (pbh->usFirstAvailByte == usBlockSize) &&
           (rc == OK)) rc = ReadBlock(ptmtg, pAddr, FALSE);

    if (rc == OK) {
      /* The Block Image conatins the last block in the cluster........*/
      if (pbh->usFirstAvailByte < usBlockSize) {
        /* The current block is not full, Update pAddr.................*/
        pAddr->usBlockNumber = pbh->usBlockNum;
        pAddr->usDispBlockPtr = pbh->usFirstAvailByte;
      }
      else /* blockHeader->usFirstAvailByte = usBlockSize */ {
        /* Update the current block header with the next block's......*/
        /* number and write to disk...................................*/
        pbh->usNextBlock = ptmh->usFirstAvailBlock;
        rc = WriteBlock(ptmtg);
        if (rc == OK) rc = TakeBlockFromSpool(ptmtg, OK, pAddr) ;

      } /* Endif The block is not full */
    } /* Endif we got to the last block safely */
  } /* Endif cluster is empty */

  return (rc);
}  /* end of FindFreshAddrInCluster */

/*---------------------------------------------------------------------*\
|                       Take Block From Spool                           |
+-----------------------------------------------------------------------+
|  Function name      : TakeBlockFromSpool                              |
|  Description        : Updates the number of preformatted blocks and   |
|                       the number of the first available block. If     |
|                       necessary, it formats more blocks.              |
|  Function Prototype : TakeBlockFromSpool (ptmtg, rcPrevious, pAddr)   |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function updates the number of free pre-formatted blocks        |
|  numFreeBlocks, and the number of first available block               |
|  usFirstAvailBlock.  Then the new TM header is written to TM          |
|  database, including the KeyDirectory which could have been updated   |
|  prior to the call of this function.  (For example by Add, if the     |
|  cluster was empty and first block is added to this cluster).         |
|  The number of pre formatted blocks is tested and if it is below a    |
|  threshold, the function FormatMore is called.                        |
|  If a previous call to FormatMore returned !OK or fDBfull or fDiskFull|
|  is TRUE there is no point in calling FormatMore again.               |
|  The second parameter indicates if FormatMore should be called,       |
|  (only when it is 0)                                                  |
|                                                                       |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - A Dos operation like write/close failed.              |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  FormatMore                                                           |
\*---------------------------------------------------------------------*/
USHORT
TakeBlockFromSpool (PTMT_GLOBALS  ptmtg, /* pointer to TmtGloabls......*/
                    USHORT rcPrevious,   /* rc of previous call........*/
                    PTM_ADDRESS pAddr) { /* Pointer to an address......*/
  /*-------------------------------------------------------------------*/
  PTM_HEADER ptmh = ptmtg->pTmHeader; /* Pointer to TM header..........*/
  USHORT    rc,               /* Returned value........................*/
            usBlockSize = ptmh->ctmh.usBlockSize;
                              /* Block Size in bytes...................*/
  PBLOCK_HEADER pbh = (PBLOCK_HEADER)ptmtg->pchBlockImage;
                              /* Pointer to block Header...............*/
  /*-------------------------------------------------------------------*/

  /* Prepare the Block Header and the address structure................*/
  pAddr->usBlockNumber = ptmh->usFirstAvailBlock;
  pAddr->usDispBlockPtr = BLOCK_HEADER_SIZE;

  pbh->usBlockNum = ptmh->usFirstAvailBlock++;
  pbh->usNextBlock = 0;
  pbh->usPrevBlock = 0 ;
  pbh->usFirstAvailByte = BLOCK_HEADER_SIZE;
  /* Initialize the block to contain Nulls.............................*/
  memset (pbh + BLOCK_HEADER_SIZE, 0, usBlockSize - BLOCK_HEADER_SIZE);

  /* We have one less free block now...................................*/
  ptmh->usNumFreeBlocks--;

  rc = WriteTmhToDisk(ptmtg, WF_FREEBLOCKS | WF_FIRSTAVAIL);
  if (rc == OK) {
    /* The write succeeded, check if more formatting is needed.........*/
    if (rcPrevious != OK) {
      /* The previous formatting attempt failed, don't try again.......*/
      rc = rcPrevious;
    } else {
      if (((LONG)ptmh->ctmh.usBlockSize * (LONG)ptmh->usNumFreeBlocks) <
          (LONG)MIN_SPOOL_SIZE) {
        /* More blocks need to be reformatted..........................*/
        rc = FormatMore (ptmtg);
        if (rc == ERROR_DISK_FULL) rc = OK;
      }
    } /* endif (rcPrevious != OK) */
  } /* endif rc == OK */
  return (rc);
} /* End of TakeBlockFromSpool */

/*---------------------------------------------------------------------*\
|                           WriteStringToDisk                           |
+-----------------------------------------------------------------------+
|  Function name      : WriteStringToDisk                               |
|  Description        : Writes a string StringToWrite with a variable   |
|                       size of StringLength bytes.                     |
|  Function Prototype : WriteStringToDisk (ptmtg, pchStringToWrite,     |
|                                          lenString , pAddr)           |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function is called by many commands in order to write a segment |
|  into the TM database.                                                |
|                                                                       |
|  The string may span over more than one block. Current BlockImage     |
|  is the first block that is written into TM. It was initialized by    |
|  the calling function. If the space on this block is not enough, then |
|  more block images are filled and written to TM db.                   |
|                                                                       |
|  pchStringToWrite is a pointer to the string to be written.           |
|  lenString is the number of bytes to write.                           |
|  pAddr is a pointer to the address where the string should be written.|
|                                                                       |
|  The function returns the return code in rcWriteString. Return codes  |
|  other then OK or(fDBfull or fDiskFull is TRUE) cause immediate return|
|  from the function.                                                   |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - A Dos operation like write/close failed.              |
|                                                                       |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  TakeBlockFromSpool                                                   |
|  ReadBlock                                                            |
|  WriteBlock                                                           |
\*---------------------------------------------------------------------*/
USHORT
WriteStringToDisk (PTMT_GLOBALS ptmtg,   /* pointer to Tmt globals.....*/
                   PCHAR   bufWrite,     /* Write buffer...............*/
                   USHORT  usLenWrite,   /* String length..............*/
                   PTM_ADDRESS  pAddr) { /* pointer to strings address.*/
  /*-------------------------------------------------------------------*/
  USHORT     rcFormat = 0,            /* Rc returned by the FormatMore.*/
             rc = 0,                  /* Rc returned by the UtlWrite...*/
             usBlockNumber,           /* block number..................*/
             usBlockSize =            /* block size....................*/
                           ptmtg->pTmHeader->ctmh.usBlockSize,
             usBytesToWrite;          /* Number of bytes to write in...*/
                                      /*   the block image.............*/
  PBLOCK_HEADER pbh = (PBLOCK_HEADER)ptmtg->pchBlockImage;
                                      /* block header................. */
  /*-------------------------------------------------------------------*/
  /* Loop as long as there are more bytes to write and there are no....*/
  /* Dos or serious block format errors................................*/
  while ((rc == OK) &&
         (usLenWrite > 0)) {
    /* Check if the current block is full..............................*/
    if (pAddr->usDispBlockPtr == usBlockSize) {
      /* If the pointer to the next block is not NULL then read it.....*/
      /* from disk. (the only important info is the pointer to next....*/
      /* block which it may have.).....................................*/

      if (pbh->usNextBlock != 0) {
        /* Block number is read from header of block image.............*/
        /* Second parm pAddr is not used - no address must be set......*/
        rc = ReadBlock(ptmtg, pAddr, FALSE);
        if (rc == OK) {
          pAddr->usDispBlockPtr = BLOCK_HEADER_SIZE;
          pAddr->usBlockNumber = pbh->usBlockNum;
        }
      } /* Endif pointer is not NULL */
    } /* Endif Block is full */

    /* If everything is OK set and update various values...............*/
    if (rc == OK) {
      usBytesToWrite=min(usBlockSize-pAddr->usDispBlockPtr, usLenWrite);

      /* Move usBytesToWrite bytes starting from the string to write...*/
      /* into the Block Image, starting at pAddr->dispBlockPtr.........*/
      memcpy( ptmtg->pchBlockImage + pAddr->usDispBlockPtr,
              bufWrite,
              usBytesToWrite);

      /* Update the various pointers...................................*/
      bufWrite += usBytesToWrite;
      usLenWrite = usLenWrite - usBytesToWrite;
      pAddr->usDispBlockPtr = pAddr->usDispBlockPtr + usBytesToWrite;
      /* The update of usFirstAvailByte takes care of two cases in.....*/
      /* which WriteString is applied. The first - in adding new.......*/
      /* segments and the second in raising del flags where the........*/
      /* writing is done into the middle of the block..................*/
      pbh->usFirstAvailByte = max (pAddr->usDispBlockPtr,
                                   pbh->usFirstAvailByte) ;

      /* fetch pointer to next block from header of currnet BlockImage.*/

      if ((pbh->usNextBlock == 0) &&
          (usLenWrite > 0) &&
          (pAddr->usDispBlockPtr == usBlockSize)) {
        /* The pointer to the next block is NULL and there are more....*/
        /* bytes to write, and current block image is full.............*/
        usBlockNumber = ptmtg->pTmHeader->usFirstAvailBlock;
        pbh->usNextBlock = usBlockNumber;

        /* Write block must be performed before Take Block From Spool, */
        /* because that function overwrites the Block Header in the....*/
        /* Block image.................................................*/
        rc = WriteBlock(ptmtg);

        if (rc == OK) {
          rc = TakeBlockFromSpool (ptmtg, rcFormat, pAddr);
          if ((ptmtg->pPrefixOut->fDiskFull == TRUE) ||
              (ptmtg->pPrefixOut->fDBfull == TRUE)) {
            rcFormat = !OK;
            rc = OK;
          }
        }
      } else /* No need for a block from the spool */ {
        rc = WriteBlock(ptmtg);
      } /* Endif need to get block from spool */
    } /* Endif ((rc == OK) and ...) */
  } /* End while ((rc == OK) and ...) */

  return (rc);
} /* End of WriteStringToDisk */

/*---------------------------------------------------------------------*\
|                              WriteBlock                               |
+-----------------------------------------------------------------------+
|  Function name      : WriteBlock                                      |
|  Description        : Writes a block image into the TM on the disk.   |
|  Function Prototype : WriteBlock (ptmtg)                              |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  WriteBlock writes a block from the block image to the disk. It       |
|  takes the block number from the block number field in the block      |
|  header and therefore it never needs any input address. The block     |
|  to be written is always assumed to be in the block image.            |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - A Dos operation like write/close failed.              |
|                 The returned value is (RC_CONST + DosRc).             |
|  API calls:                                                           |
|  ----------                                                           |
|  UtlWrite                                                             |
\*---------------------------------------------------------------------*/
USHORT
WriteBlock (PTMT_GLOBALS ptmtg) {     /* pointer to Tmt Globals Struct */
  /*-------------------------------------------------------------------*/
  USHORT       rc,                        /* Returned rc...............*/
               usTemp,                    /* Temporary variable........*/
               usBlockNumber,             /* Block Number..............*/
               usBlockSize = ptmtg->pTmHeader->ctmh.usBlockSize;
                                          /* block size................*/
  ULONG         uldisp;                     /* displacement..............*/
  PUCHAR        pchBlockImage = ptmtg->pchBlockImage;
                                          /* block image...............*/
  /*-------------------------------------------------------------------*/
   /* Fetch the block number from the block header.....................*/
   usBlockNumber = ((PBLOCK_HEADER)pchBlockImage)->usBlockNum ;

   uldisp = ptmtg->pTmHeader->ldispFirstBlock +
   (LONG)(usBlockNumber-1) * (LONG)usBlockSize;/* no. of bytes to skip */

   rc = UtlChgFilePtr (ptmtg->hfTM,            /* file handle..........*/
                             uldisp,           /* displacement.........*/
                             FILE_BEGIN,       /* displacement from....*/
                             &uldisp,          /* new pointer..........*/
                             FALSE);

   if (rc == OK) {
     rc = UtlWrite(ptmtg->hfTM,                /* file handle..........*/
                   pchBlockImage,              /* pointer to block.....*/
                   usBlockSize,                /* block size...........*/
                   &usTemp,                    /* bytes written........*/
                   FALSE);
   }
   return (rc);
} /* End of WriteBlock */

/*---------------------------------------------------------------------*\
|                       Find Match Seg In Cluster                       |
+-----------------------------------------------------------------------+
|  Function name      : FindMatchSegInCluster                           |
|  Description        : Finds the first segment in the cluster that     |
|                       matches the input segment                       |
|  Function Prototype : FindMatchSegInCluster  (ptmtg, pAddr,           |
|                                               pfLastSeg,              |
|                                               pDeleteIn, pseg)        |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function finds the first segment in a given cluster that        |
|  matches perfectly the control info and source text of an input       |
|  segment.  It receives an input address from which the search should  |
|  start.  In the same variable (pAddr) it returns the address of the   |
|  segment that was found.  It uses a buffer to store the segments that |
|  are read from the disk.  It calls CheckMatchForDelete to check       |
|  matching segments.                                                   |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)        - The command ended successfully.                      |
|  CLUSTER EMPTY - There are no blocks in the cluster.                  |
|  SEG_NOT_FOUND - The segment to be deleted is not in the TM.          |
|  Other values  - A Dos operation like write/close failed.             |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  ReadSegFromDisk                                                      |
|  CheckMatchForDelete                                                  |
\*---------------------------------------------------------------------*/
USHORT
FindMatchSegInCluster (PTMT_GLOBALS ptmtg,     /* Ptr to Tmt globals...*/
                       PTM_ADDRESS  pAddr,     /* Pointer to address...*/
                       PBOOL        pfLastSeg, /* Last segment flag....*/
                       PSEGMENT     psegIn,    /* Ptr to input buffer..*/
                       PSEGMENT     pseg) {    /* Pointer to segment...*/
  /*-------------------------------------------------------------------*/
  TM_ADDRESS   AddrOld;              /* temporary address variable.....*/
  USHORT       rc = OK,              /* Returned rc....................*/
               usBlockSize =         /* The size of each TM block.....*/
                 ptmtg->pTmHeader->ctmh.usBlockSize;
  BOOL         fMatch = FALSE,       /* Match exists flag..............*/
               fFirstSeg = TRUE;     /* First segment read flag........*/
  /*-------------------------------------------------------------------*/
  *pfLastSeg = FALSE;

  memset(&AddrOld, 0, sizeof(AddrOld));
  /* scan the whole cluster for a matching segment.....................*/
  while (!*pfLastSeg && (rc == OK) && !fMatch) {

    AddrOld = *pAddr;
    /* The following "if" makes sure that an address at the end of a...*/
    /* block will be replaced by a true address at the beginning of the*/
    /* next block......................................................*/

    if ( pAddr->usDispBlockPtr == usBlockSize ) {
      AddrOld.usBlockNumber =
                   ((PBLOCK_HEADER)(ptmtg->pchBlockImage))->usNextBlock;
      AddrOld.usDispBlockPtr = BLOCK_HEADER_SIZE ;
    }
    /* fFirstSeg and *pfLastSeg are updated by ReadSegmnet.............*/
    /* pAddr points to the segment to read, and after the call points..*/
    /* to the next segment address.....................................*/
    rc = ReadSegmentFromDisk(ptmtg, pAddr, pseg, &fFirstSeg, pfLastSeg);

    if ((rc == OK) && (!pseg->fLogicalDel)){
      /* Segment was read with no problem..............................*/
      /* CheckMatchForDelete returns TRUE if match exists,.............*/
      fMatch = CheckMatchForDelete (psegIn, pseg);
    }
  } /* End while */

  /* compute the return code...........................................*/
  if (rc == OK) {
    /* No reading problem..............................................*/
    if (fMatch){
      *pAddr = AddrOld;
      /* If the time stamp of the segment in the TM is younger do not .*/
      /* replace ......................................................*/
      if (pseg->tStamp > psegIn->tStamp) rc = NOT_REPLACED_OLD_SEGMENT;
    }else{
      rc = SEG_NOT_FOUND;
    }
  }
  return (rc) ;
} /* End of FindMatchSegInCluster */

/*---------------------------------------------------------------------*\
|                       Check Match For Delete                          |
+-----------------------------------------------------------------------+
|  Function name      : CheckMatchForDelete                             |
|  Description        : Checks if the input segment perfectly matches a |
|                       TM segment                                      |
|  Function Prototype : CheckMatchForDelete (pDeleteIn, pseg)           |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function looks for a match between an input segment which lies  |
|  in the In structure and a segment from the TM file which is the      |
|  current segment to be checked. The match between the source parts    |
|  must be an exact byte by byte match but ignoring CRLF and LF         |
|  sequences, while no match is needed between the target parts.  The   |
|  source match is not sufficient however, and additional match is      |
|  required between the control parts.  The check starts with the       |
|  control parameters which are less costly to check.  The source check |
|  is done only if the control part matches perfectly.                  |
|                                                                       |
|  Returned Values:                                                     |
|  ----------------                                                     |
|  TRUE  - a match was found                                            |
|  FALSE - no match was found                                           |
\*---------------------------------------------------------------------*/
BOOL
CheckMatchForDelete (PSEGMENT psegIn,    /* pointer to input buffer....*/
                     PSEGMENT pseg) {    /* currently, output buffer...*/
   /*------------------------------------------------------------------*/
   BOOL        fMatch;               /* Match was found flag...........*/

   /*------------------------------------------------------------------*/

   /* fMatch = match of seg# in file.............................*/
   fMatch = (psegIn->usSegNumber == pseg->usSegNumber);
   if (fMatch) {
     /* fMatch = match of filenames....................................*/

     fMatch = NTMDocMatch( psegIn->szFileName, psegIn->szLongName,
                           pseg->szFileName, pseg->szLongName );

     if (fMatch) {
       /* fMatch = Industry codes match................................*/
       /* matching in terms of sets and NOT lists......................*/
       fMatch = (psegIn->usLenIndustry == pseg->usLenIndustry );
       if (fMatch) {
         fMatch = (memcmp(&psegIn->bufData[psegIn->usDispIndustry],
                      &pseg->bufData[pseg->usDispIndustry],
                      pseg->usLenIndustry) == 0);
         if (fMatch) {
           /* fMatch = exact match of sources while ignoring CRLF/LF...*/
           fMatch = (UtlCompIgnWhiteSpace(
                                 &psegIn->bufData[psegIn->usDispSource],
                                 &pseg->bufData[pseg->usDispSource],
                                 0) == 0);
         }
       }
     }
   }
  return (fMatch) ;
} /* end of CheckMatchForDelete */

/*---------------------------------------------------------------------*\
|                       Delete Segment                                  |
+-----------------------------------------------------------------------+
|  Function name      : DeleteSegment                                   |
|  Description        : Deletes a segment from the TM file.             |
|  Function Prototype : DeleteSegment (ptmtg, pAddr, fLastSeg, pseg)    |
+-----------------------------------------------------------------------|
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The function performs segment deletion from the TM file. The         |
|  deletion is either logical - when the segment is not the last one    |
|  in its cluster, or physical - if it is the last one. An input flag,  |
|  fLastSeg, indicates which of the cases occurs.                       |
|  Upon entry, the segment to be deleted lies in pseg, and the          |
|  address of the deleted segment is given in pAddr.                    |
|  In case of logical delete, The DelFlag is set in pseg and part       |
|  of the segment, containing the del flag is written back.             |
|  In case of physical delete, all the blocks in which the segment      |
|  resides are cleaned by a proper update of FirstAvailByte in their    |
|  headers.                                                             |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)        - The command ended successfully.                      |
|  Other values  - A Dos operation like write/close failed.             |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  WriteStringToDisk                                                    |
|  ReadBlock                                                            |
|  WriteBlock                                                           |
\*---------------------------------------------------------------------*/
USHORT
DeleteSegment (PTMT_GLOBALS ptmtg,    /* Pointer to Tmt globals........*/
              PTM_ADDRESS   pAddr,    /*  Address of segment to delete.*/
              BOOL          fLastSeg, /* Last segment in cluster flag..*/
              PSEGMENT      pseg) {   /* Pointer to the segment........*/
  /*-------------------------------------------------------------------*/
  SHORT    rc;                           /* Returned rc................*/
  USHORT   usLenString;                  /* Length of string to write..*/
  PBLOCK_HEADER pbh = (PBLOCK_HEADER) ptmtg->pchBlockImage;
                                        /* Pointer to block header.....*/
  /*-------------------------------------------------------------------*/

   /* Because the segment might span more then one block, we must read.*/
   /* the block containing the segment beginning.......................*/
   rc = ReadBlock (ptmtg, pAddr, TRUE);

   if (rc == OK) {
     if (fLastSeg) {
       /* Physical delete. Reset the first available byte to where the.*/
       /* segment begins, and write the block to disk..................*/
       pbh->usFirstAvailByte = pAddr->usDispBlockPtr;
       rc = WriteBlock (ptmtg)  ;

       /* The loop cleans all the other blocks in the cluster by.......*/
       /* assigning FirstAVailByte = BLOCK_HEADER_SIZE. The FALSE......*/
       /* indicates that the block number is in the block image........*/
       while ((rc == OK) && (pbh->usNextBlock != 0))
       {
         rc = ReadBlock (ptmtg, NULL, FALSE) ;
         if (rc == OK)
         {
           /* The following operation cleans the block.................*/
           pbh->usFirstAvailByte = BLOCK_HEADER_SIZE ;
           rc = WriteBlock (ptmtg) ;
         }
       } /* end while */
     } /* End of physical delete */
     else /* !fLastSeg */
     {
       static OLDSEGMENT OldSeg;               // buffer for V1.0 format segment

      // build V1 segment in our static segment buffer
      {
        memcpy( &OldSeg, pseg, FIELDOFFSET(SEGMENT, szLongName[0]) );
        memcpy( (PUCHAR)&OldSeg + FIELDOFFSET(OLDSEGMENT, usSegNumber),
                (PUCHAR)pseg + FIELDOFFSET(SEGMENT, usSegNumber ),
               pseg->usLenSegment - FIELDOFFSET(SEGMENT, usSegNumber) );
        OldSeg.usLenSegment   -= sizeof(pseg->szLongName);
      }
      /* Delete logically.............................................*/

       OldSeg.fLogicalDel = TRUE ;
       /* set lenString to the displacment (in BYTES) of...............*/
       /* fLogicalDel + the size of fLogicalDel........................*/
       usLenString = FIELDOFFSET(OLDSEGMENT, fLogicalDel) + sizeof(EQF_BOOL);

       /* Write string with this length to the disk....................*/
       rc = WriteStringToDisk (ptmtg, (PCHAR)&OldSeg, usLenString, pAddr);
     } /* Endif (fLastSeg) - End of Logical delete */
   } /* Endif (rc == OK) */

   return (rc) ;
} /* end of DeleteSegment */
