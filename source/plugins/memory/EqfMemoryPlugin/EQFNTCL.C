//+----------------------------------------------------------------------------+
//|EQFNTCL.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
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
// $Revision: 1.2 $ ----------- 11 Apr 2006
// GQ: - added time measurement log
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
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
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
// $Revision: 1.3 $ ----------- 14 Feb 2000
// - cleanup language group tables
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFNTCL.CV_   1.2   01 Jul 1997 09:59:46   BUILD  $
 *
 * $Log:   J:\DATA\EQFNTCL.CV_  $
 *
 *    Rev 1.2   01 Jul 1997 09:59:46   BUILD
 * - added retry logic for in-use condition during TM access
 *
 *    Rev 1.1   24 Feb 1997 11:50:52   BUILD
 * - added support for long document names
 *
 *    Rev 1.0   09 Jan 1996 09:11:34   BUILD
 * Initial revision.
*/
// ----------------------------------------------------------------------------+
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#include <eqf.h>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMORPI.H>

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXClose    closes data and index files                 |
//+----------------------------------------------------------------------------+
//|Description:       closes qdam data and index files                         |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXClose( PTMX_CLOSE_IN pTmCloseIn, //input struct         |
//|                           PTMX_CLOSE_OUT pTmCloseOut ) //output struct     |
//+----------------------------------------------------------------------------+
//|Input parameter: PTMX_CLOSE_IN  pTmCloseIn     input structure              |
//+----------------------------------------------------------------------------+
//|Output parameter: PTMX_CLOSE_OUT pTmCloseOut   output structure             |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: identical to return code in close out structure                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//| copy compact area record to tm data file                                   |
//| close tm index file                                                        |
//| close tm data file                                                         |
//|                                                                            |
//| return close out structure                                                 |
// ----------------------------------------------------------------------------+

USHORT TmtXClose
(
  PTMX_CLB pTmClb,             //ptr to control block
  PTMX_CLOSE_IN pTmCloseIn,    //ptr to input struct
  PTMX_CLOSE_OUT pTmCloseOut   //ptr to output struct
)
{
  USHORT     usRc = NO_ERROR;          //return value
  USHORT     usRc1 = NO_ERROR;          //return value

  pTmCloseIn = pTmCloseIn;

  //insert compact area, language, file name and author from pTmClb to respective
  //record in tm data file
  if ( pTmClb->bCompactChanged )
  {
    SHORT sRetries = MAX_RETRY_COUNT;
    do
    {
      usRc = EQFNTMUpdate( pTmClb->pstTmBtree, COMPACT_KEY,
                           pTmClb->bCompact, MAX_COMPACT_SIZE-1 );
      if ( pTmClb->fShared && (usRc == BTREE_IN_USE) )
      {
        UtlWait( MAX_WAIT_TIME );
        sRetries--;
      } /* endif */
    }
    while( pTmClb->fShared && (usRc == BTREE_IN_USE) && (sRetries > 0));
  } /* endif */

  //close index file regardless of errors
  usRc1 = EQFNTMClose( &pTmClb->pstInBtree );
  if ( !usRc )
    usRc = usRc1;

  //close data file
  usRc1 = EQFNTMClose( &pTmClb->pstTmBtree );
  if ( !usRc )
    usRc = usRc1;

  UtlAlloc( (PVOID *) &(pTmClb->pLanguages), 0L, 0L, NOMSG );

  UtlAlloc( (PVOID *) &(pTmClb->pAuthors), 0L, 0L, NOMSG );

  UtlAlloc( (PVOID *) &(pTmClb->pTagTables), 0L, 0L, NOMSG );

  UtlAlloc( (PVOID *) &(pTmClb->pFileNames), 0L, 0L, NOMSG );

  UtlAlloc( (PVOID *) &(pTmClb->pLangGroups), 0L, 0L, NOMSG );

  UtlAlloc( (PVOID *) &(pTmClb->psLangIdToGroupTable), 0L, 0L, NOMSG );

  NTMDestroyLongNameTable( pTmClb );

  // free new structures allocated by sub functions
  if ( pTmClb->pvTempMatchList )      UtlAlloc( (PVOID *) &(pTmClb->pvTempMatchList), 0L, 0L, NOMSG );
  if ( pTmClb->pvIndexRecord )        UtlAlloc( (PVOID *) &(pTmClb->pvIndexRecord), 0L, 0L, NOMSG );
  if ( pTmClb->pvTmRecord )           UtlAlloc( (PVOID *) &(pTmClb->pvTmRecord), 0L, 0L, NOMSG );
  

  // write logging info if logginf is active
  if ( pTmClb->fTimeLogging )
  {
    FILE *hfLog = NULL;
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, NOMSG );
    strcat( szLogFile, "\\MEMTIME.LOG" );
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "**** Memory Time Measurement Log ****\n" );
      fprintf( hfLog, "Allocation time            %12I64d [ms]\n", pTmClb->lAllocTime );
      fprintf( hfLog, "Tokenization time          %12I64d [ms]\n", pTmClb->lTokenizeTime );
      fprintf( hfLog, "GetExact time              %12I64d [ms]\n", pTmClb->lGetExactTime );
      fprintf( hfLog, "GetFuzzy time              %12I64d [ms]\n", pTmClb->lGetFuzzyTime );
      fprintf( hfLog, "Other times                %12I64d [ms]\n", pTmClb->lOtherTime );
      fprintf( hfLog, "GetFuzzy time              %12I64d [ms]\n", pTmClb->lGetFuzzyTime );
      fprintf( hfLog, "GetFuzzy FuzzyTest time    %12I64d [ms]\n", pTmClb->lFuzzyTestTime );
      fprintf( hfLog, "GetFuzzy FillMatchEntry    %12I64d [ms]\n", pTmClb->lFuzzyFillMatchEntry );
      fprintf( hfLog, "GetFuzzy NTMGet time       %12I64d [ms]\n", pTmClb->lFuzzyGetTime );
      fprintf( hfLog, "GetFuzzy other times       %12I64d [ms]\n", pTmClb->lFuzzyOtherTime );
      fprintf( hfLog, "FillMatchEntry alloctime   %12I64d [ms]\n", pTmClb->lFillMatchAllocTime );
      fprintf( hfLog, "FillMatchEntry other time  %12I64d [ms]\n", pTmClb->lFillMatchOtherTime );
      fprintf( hfLog, "FillMatchEntry read time   %12I64d [ms]\n", pTmClb->lFillMatchReadTime );
      fprintf( hfLog, "FillMatchEntry fill time   %12I64d [ms]\n", pTmClb->lFillMatchFillTime );
      fprintf( hfLog, "FillMatchEntry fill1 time  %12I64d [ms]\n", pTmClb->lFillMatchFill1Time );
      fprintf( hfLog, "FillMatchEntry fill2 time  %12I64d [ms]\n", pTmClb->lFillMatchFill2Time );
      fprintf( hfLog, "FillMatchEntry fill3 time  %12I64d [ms]\n", pTmClb->lFillMatchFill3Time );
      fprintf( hfLog, "FillMatchEntry fill4 time  %12I64d [ms]\n", pTmClb->lFillMatchFill4Time );
      fprintf( hfLog, "FillMatchEntry clean time  %12I64d [ms]\n", pTmClb->lFillMatchCleanupTime );
      fclose( hfLog );
    } /* endif */

  } /* endif */

  //release control block memory
  UtlAlloc( (PVOID *) &pTmClb, 0L, 0L, NOMSG );

  pTmCloseOut->stPrefixOut.usLengthOutput = sizeof( TMX_CLOSE_OUT );
  pTmCloseOut->stPrefixOut.usTmtXRc = usRc;
  return( usRc );
}

