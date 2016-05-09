//+----------------------------------------------------------------------------+
//|EQFNTTMT.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author: Stefan Doersam                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|C TmtX                                                                      |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//| TmtXCreate                                                                 |
//| TmtXOpen                                                                   |
//| TmtXClose                                                                  |
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
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
// 
// 
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
// 
// 
// $Revision: 1.2 $ ----------- 9 Nov 2001
// RJ: Re-Put NTTMT file
// 
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 3 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFNTTMT.CV_   1.0   09 Jan 1996 09:12:28   BUILD  $
 *
 * $Log:   J:\DATA\EQFNTTMT.CV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:12:28   BUILD
 * Initial revision.
*/
// ----------------------------------------------------------------------------+
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file

#include <EQFTMI.H>               // Private header file of Translation Memory

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtX                                                     |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT TmtX( HTM  htm,                                   |
//|                                PIN  pIn,                                   |
//|                                POUT pOut )                                 |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
// ----------------------------------------------------------------------------+
USHORT
TmtX( HTM  htm,
      PXIN  pIn,
      PXOUT pOut)
{
  USHORT   usRc;

  switch ( pIn->usTmCommand )
  {
    //-------------------------------------------------------------------------
    case (TMC_CREATE):
      usRc = TmtXCreate ( (PTMX_CREATE_IN)pIn, (PTMX_CREATE_OUT)pOut );
      break;
    //-------------------------------------------------------------------------
    case (TMC_OPEN):
      usRc = TmtXOpen ( (PTMX_OPEN_IN)pIn, (PTMX_OPEN_OUT)pOut );
      break;
    //-------------------------------------------------------------------------
    case (TMC_CLOSE):
      usRc = TmtXClose ( (PTMX_CLB)htm, (PTMX_CLOSE_IN)pIn, (PTMX_CLOSE_OUT)pOut );
      break;
    //-------------------------------------------------------------------------
    case (TMC_REPLACE):
      usRc = TmtXReplace ( (PTMX_CLB)htm, (PTMX_PUT_IN_W)pIn, (PTMX_PUT_OUT_W)pOut);
      break;
    //-------------------------------------------------------------------------
    case (TMC_GET):
      usRc = TmtXGet ( (PTMX_CLB)htm, (PTMX_GET_IN_W)pIn, (PTMX_GET_OUT_W)pOut);
      break;
    //-------------------------------------------------------------------------
    case ( TMC_EXTRACT):
      usRc = TmtXExtract ( (PTMX_CLB)htm, (PTMX_EXT_IN_W)pIn, (PTMX_EXT_OUT_W)pOut);
      break;
    //-------------------------------------------------------------------------
    case (TMC_INFO):
      usRc = TmtXInfo ( (PTMX_CLB)htm, (PTMX_INFO_OUT)pOut );
      break;
    //-------------------------------------------------------------------------
    case TMC_END_ORGANIZE:
      usRc = TmtXCloseOrganize( htm, (PTMX_ENDORG_IN)pIn, (PTMX_ENDORG_OUT)pOut);
      break;
    //-------------------------------------------------------------------------
    case TMC_DELETE_TM:
      usRc = TmtXDeleteTM( htm, (PTMX_DELTM_IN)pIn, (PTMX_DELTM_OUT)pOut );
      break;
    //-------------------------------------------------------------------------
    case (TMC_DELETE):
      usRc = TmtXDelSegm ( (PTMX_CLB)htm, (PTMX_PUT_IN_W)pIn, (PTMX_PUT_OUT_W)pOut);
      break;
//  //-------------------------------------------------------------------------
//  case (TMC_ADD):
//    usRc = TmtAdd (ptmtg, (PADD_IN)pIn, (PADD_OUT)pOut);
//    break;
//  //-------------------------------------------------------------------------
//  case TMC_GET_PART_OF_TM_FILE:
//    usRc = TmtGetTMPart( ptmtg, (PGETPART_IN)pIn, (PGETPART_OUT)pOut );
//    break;
    //-------------------------------------------------------------------------
    default:
      usRc = ILLEGAL_TM_COMMAND;
  } /* End switch (idCommand ) */

  return( usRc );
} /* end of function TmtX */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXInfo                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT                                                   |
//|                   TmtXInfo( PTMX_CLB       pstClb,                         |
//|                             PTMX_INFO_OUT  pstInfoOut )                    |
//+----------------------------------------------------------------------------+
//|Description:       function to retrieve the signature record                |
//+----------------------------------------------------------------------------+
//|Parameters:        pstClb     - (in)  pointer to TM CLB                     |
//|                   pstInfoOut - (out) pointer to info output structure      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
// ----------------------------------------------------------------------------+
USHORT
TmtXInfo( PTMX_CLB       pstClb,
          PTMX_INFO_OUT  pstInfoOut )
{
  USHORT usRc = NO_ERROR;
  /********************************************************************/
  /* copy the isgnature record from TM CLB to info out structure      */
  /********************************************************************/
  memcpy( &pstInfoOut->stTmSign, &pstClb->stTmSign, sizeof(TMX_SIGN) );

  /********************************************************************/
  /* get the threshold value from the TM CLB                          */
  /********************************************************************/
  pstInfoOut->usThreshold = pstClb->usThreshold;

  /********************************************************************/
  /* fill the prefix of the info output structure                     */
  /********************************************************************/
  pstInfoOut->stPrefixOut.usLengthOutput = sizeof( TMX_INFO_OUT );
  pstInfoOut->stPrefixOut.usTmtXRc = usRc;

  return usRc;
} /* end of function TmtXInfo */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXCloseOrganize                                        |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT                                                   |
//|                   TmtXCloseOrganize( HTM             hMem,                 |
//|                                      PTMX_ENDORG_IN  pstEndOrgIn,          |
//|                                      PTMX_ENDORG_OUT pstEndOrgOut )        |
//+----------------------------------------------------------------------------+
//|Description:       Closes a TM (old or new) after organization              |
//+----------------------------------------------------------------------------+
//|Parameters:        hMem         - (in) TM handle                            |
//|                   pstEndOrgIn  - (in) pointer to end organize in structure |
//|                   pstEndOrgOut - (in) pointer to end organize out structure|
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
// ----------------------------------------------------------------------------+
USHORT
TmtXCloseOrganize( HTM             hMem,          //(in) TM handle
                   PTMX_ENDORG_IN  pstEndOrgIn,   //(in) pointer to end organize in structure
                   PTMX_ENDORG_OUT pstEndOrgOut ) //(in) pointer to end organize out structure
{
  USHORT     usRc = TM_FUNCTION_FAILED;         //function returncode

  CLOSE_IN       stOldCloseIn;                 //old Close input structure
  CLOSE_OUT      stOldCloseOut;                //old Close output structure
  TMX_CLOSE_IN   stNewCloseIn;                 //new Close input structure
  TMX_CLOSE_OUT  stNewCloseOut;                //new Close output structure
  CHAR           szRenamedTM[MAX_EQF_PATH];    //temp TM name
  CHAR           szRenamedIndex[MAX_EQF_PATH]; //temp index name
  CHAR           szRenamedProp[MAX_EQF_PATH];  //temp property name

  if ( pstEndOrgIn != NULL )
  {
    /******************************************************************/
    /* close the original TM                                          */
    /******************************************************************/
    if ( hMem != NULLHANDLE )
    {
      if ( pstEndOrgIn->usOrgType == TM_CONVERT  )
      {
        /**************************************************************/
        /* an old TM was converted close the old TM                   */
        /**************************************************************/
        memset( &stOldCloseIn, NULC, sizeof(CLOSE_IN) );
        memset( &stOldCloseOut, NULC, sizeof(CLOSE_OUT) );

        /**************************************************************/
        /* fill the close input structure                             */
        /**************************************************************/
        stOldCloseIn.prefin.usLenIn   = sizeof( CLOSE_IN );
        stOldCloseIn.prefin.idCommand = TMC_CLOSE;

        /**************************************************************/
        /* call Tmt to close the old TM                               */
        /**************************************************************/
        usRc = Tmt( hMem,
                    (PIN)  &stOldCloseIn,
                    (POUT) &stOldCloseOut );

        /**************************************************************/
        /* call function to assure that TM is really closed           */
        /* this have to be done because the DosClose request is not   */
        /* executed immediatly, it is queued.                         */
        /* This might be an HPFS effect ?                             */
        /* fCheckFileClosed close th TM it is open                    */
        /**************************************************************/
        fCheckFileClosed( pstEndOrgIn->szOrgTM );

        if ( usRc == NO_ERROR )
        {
          /************************************************************/
          /* at this point in time the following TMs exists           */
          /* *.MEM  - this is the original TM                         */
          /* *.TMD  - the new TM (organized)                          */
          /* *.TMI  - the TM index file                               */
          /* *.MEM  - property file                                   */
          /* *.TPR  - new property file                               */
          /************************************************************/
          /* rename the property file                                 */
          /* *.MEM -> *.RPR                                           */
          /* *.TPR -> *.MEM                                           */
          /* *.RPR -> delete if all OK                                */
          /* when property file successfully renamed, delete org TM   */
          /************************************************************/
          Utlstrccpy( szRenamedProp, pstEndOrgIn->szTmpProp, DOT );
          strcat( szRenamedProp, EXT_OF_RENAMED_TMPROP );

          if ( ( usRc = UtlMove( pstEndOrgIn->szOrgProp,
                                 szRenamedProp, 0L, FALSE ) )
                 == NO_ERROR )
          {
            if ( ( usRc = UtlMove( pstEndOrgIn->szTmpProp,
                                   pstEndOrgIn->szOrgProp, 0L, FALSE ) )
                   == NO_ERROR )
            {
               UtlDelete( szRenamedProp, 0L, FALSE );
               UtlDelete( pstEndOrgIn->szOrgTM, 0L, FALSE );
            }
            else
            {
              UtlMove( szRenamedProp, pstEndOrgIn->szOrgProp, 0L, FALSE );
              usRc = UtlDelete( szRenamedProp, 0L, FALSE );
            } /* endif */
          } /* endif */
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* an new TM was organized close the new TM                   */
        /**************************************************************/
        memset( &stNewCloseIn, NULC, sizeof(TMX_CLOSE_IN) );
        memset( &stNewCloseOut, NULC, sizeof(TMX_CLOSE_OUT) );

        /**************************************************************/
        /* fill the close input structure                             */
        /**************************************************************/
        stNewCloseIn.stPrefixIn.usLengthInput = sizeof(TMX_CLOSE_IN);
        stNewCloseIn.stPrefixIn.usTmCommand   = TMC_CLOSE;

        /**************************************************************/
        /* call TmtX to close the TM                                  */
        /**************************************************************/
        usRc = TmtX( hMem,
                     (PXIN)  &stNewCloseIn,
                     (PXOUT) &stNewCloseOut );

        if ( ( *(pstEndOrgIn->szOrgTM) != '\0' ) &&
             ( *(pstEndOrgIn->szTmpTM) != '\0' ) &&
             ( (usRc == NO_ERROR ) || ( usRc == BTREE_CORRUPTED)))
        {
          /************************************************************/
          /* call function to assure that TM is really closed         */
          /* this have to be done because the DosClose request is not */
          /* executed immediatly, it is queued.                       */
          /* This might be an HPFS effect ?                           */
          /* fCheckFileClosed close th TM it is open                  */
          /************************************************************/
          fCheckFileClosed( pstEndOrgIn->szOrgTM );

          /************************************************************/
          /* at this point in time the following TMs exists           */
          /* *.TMD  - the original TM                                 */
          /* *.TTI  - the original index file                         */
          /* *.TTD  - the temporary TM (new organized)                */
          /* *.TTI  - the TM index file                               */
          /* the file are renamed (moved) in the following way        */
          /*                                                          */
          /* if (move *.TMD -> TRD == OK)                             */
          /*   if (move *.TTD -> TMD == OK)                           */
          /*     delete TRD                                           */
          /*   else                                                   */
          /*    (move *.TRD -> TMD)                                   */
          /*  end                                                     */
          /* end                                                      */
          /************************************************************/
          Utlstrccpy( szRenamedTM, pstEndOrgIn->szOrgTM, DOT );
          strcat( szRenamedTM, EXT_OF_RENAMED_TMDATA );

          if ( ( usRc = UtlMove( pstEndOrgIn->szOrgTM,
                                 szRenamedTM, 0L, FALSE ) )
                 == NO_ERROR )
          {
            if ( ( usRc = UtlMove( pstEndOrgIn->szTmpTM,
                                   pstEndOrgIn->szOrgTM, 0L, FALSE ) )
                   == NO_ERROR )
            {
              UtlDelete( szRenamedTM, 0L, FALSE );
            }
            else
            {
              UtlMove( szRenamedTM, pstEndOrgIn->szOrgTM, 0L, FALSE );
            } /* endif */
          } /* endif */

          if ( usRc == NO_ERROR )
          {
            /**********************************************************/
            /* if (move *.TMI -> TRI == OK)                           */
            /*   if (move *.TTI -> TMI == OK)                         */
            /*     delete TRI                                         */
            /*   else                                                 */
            /*    (move *.TRI -> TMI)                                 */
            /*  end                                                   */
            /* end                                                    */
            /**********************************************************/
            fCheckFileClosed( pstEndOrgIn->szOrgIndex );
            Utlstrccpy( szRenamedIndex, pstEndOrgIn->szOrgIndex, DOT );
            strcat( szRenamedIndex, EXT_OF_RENAMED_TMINDEX );

            if ( ( usRc = UtlMove( pstEndOrgIn->szOrgIndex,
                                   szRenamedIndex, 0L, FALSE ) )
                   == NO_ERROR )
            {
              if ( ( usRc = UtlMove( pstEndOrgIn->szTmpIndex,
                                     pstEndOrgIn->szOrgIndex, 0L, FALSE ) )
                     == NO_ERROR )
              {
                UtlDelete( szRenamedIndex, 0L, FALSE );
              }
              else
              {
                UtlMove( szRenamedIndex, pstEndOrgIn->szOrgIndex, 0L, FALSE );
              } /* endif */
            } /* endif */
          } /* endif */

          /************************************************************/
          /* Create copy of property file for shared TMs if the       */
          /* copy of the property file does not exist                 */
          /************************************************************/
          if ( usRc == NO_ERROR )
          {
            if ( strcmp( strrchr( pstEndOrgIn->szOrgTM, DOT ),
                         EXT_OF_SHARED_MEM ) == 0 )
            {
              /********************************************************/
              /* This is a shared TM; now check for the property file */
              /* copy                                                 */
              /********************************************************/
              CHAR szProp[MAX_EQF_PATH];         // local properties
              CHAR szSharedProp[MAX_EQF_PATH];   // properties on shared drive

              UtlMakeEQFPath( szProp, NULC, PROPERTY_PATH, NULL );
              strcat( szProp, BACKSLASH_STR );
              Utlstrccpy( szProp + strlen(szProp),
                          UtlGetFnameFromPath( pstEndOrgIn->szOrgTM ),
                          DOT );
              strcat( szProp, EXT_OF_MEM );

              Utlstrccpy( szSharedProp, pstEndOrgIn->szOrgTM, DOT );
              strcat( szSharedProp, EXT_OF_SHARED_MEMPROP );

              /***********************************************************/
              /* Copy the property file if does not exist on shared drive*/
              /***********************************************************/
              if ( !UtlFileExist(szSharedProp) )
              {
                usRc = UtlCopy( szProp, szSharedProp, 1, 0L, FALSE );
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* set rc and length in PTMX_ENDORG_OUT structure                   */
  /********************************************************************/
  if ( pstEndOrgOut != NULL )
  {
    pstEndOrgOut->stPrefixOut.usTmtXRc       = usRc;
    pstEndOrgOut->stPrefixOut.usLengthOutput = sizeof( TMX_ENDORG_OUT );
  } /* endif */

  return usRc;
} /* end of function TmXtCloseOrganize */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXDeleteTM                                             |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
// ----------------------------------------------------------------------------+
USHORT
TmtXDeleteTM( HTM            htm,
              PTMX_DELTM_IN  pDelTmIn,
              PTMX_DELTM_OUT pDelTmOut )
{
  USHORT         usRc = TM_FUNCTION_FAILED;
  TMX_CLOSE_IN   stCloseIn;
  TMX_CLOSE_OUT  stCloseOut;

  if ( pDelTmIn != NULL )
  {
    // close the TM
    if ( htm != NULLHANDLE )
    {
      memset( &stCloseIn,  NULC, sizeof( TMX_CLOSE_IN ) );
      memset( &stCloseOut, NULC, sizeof( TMX_CLOSE_OUT ) );

      // Assign values to the appropriate fields
      /****************************************************************/
      /* fill frefix of close in structure                            */
      /****************************************************************/
      stCloseIn.stPrefixIn.usLengthInput = sizeof( TMX_CLOSE_IN );
      stCloseIn.stPrefixIn.usTmCommand   = TMC_CLOSE;

      usRc = TmtX( htm,
                  (PXIN) &stCloseIn,
                  (PXOUT) &stCloseOut );
    } /* endif */

    if ( ( pDelTmIn->szFullTmName[0]    != EOS ) &&
         ( pDelTmIn->szFullIndexName[0] != EOS ) &&
         ( pDelTmIn->szFullPropName[0]  != EOS ) )
    {
      /****************************************************************/
      /* delete the TM                                                */
      /****************************************************************/
      usRc = UtlDelete( pDelTmIn->szFullTmName, 0L, FALSE );

      switch ( usRc )
      {
        case NO_ERROR :
        case ERROR_FILE_NOT_FOUND :
        case ERROR_PATH_NOT_FOUND :
        case ERROR_INVALID_DRIVE  :
        case ERROR_ACCESS_DENIED  :
        /**************************************************************/
        /* delete the property and ondex file                         */
        /**************************************************************/
        UtlDelete( pDelTmIn->szFullPropName, 0L, FALSE );
        UtlDelete( pDelTmIn->szFullIndexName, 0L, FALSE );
        break;
      } /* endswitch */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* set rc and size in del out structure                             */
  /********************************************************************/
  if ( pDelTmOut != NULL )
  {
    pDelTmOut->stPrefixOut.usTmtXRc       = usRc;
    pDelTmOut->stPrefixOut.usLengthOutput = sizeof( TMX_DELTM_OUT );
  } /* endif */

  return usRc;
} /* end of function TmtXDeleteTM */

