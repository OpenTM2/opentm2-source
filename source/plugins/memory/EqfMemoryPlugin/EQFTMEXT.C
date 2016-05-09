//+----------------------------------------------------------------------------+
//|EQFTMEXT.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:      Christian Michel, Marc Hoffmann, Stefan Doersam                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Extension for TMT code for COMM code                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|  TmtDeleteTM                                                               |
//|  TmtCloseOrganize                                                          |
//|  TmtDeleteFile                                                             |
//|  RenameFile                                                                |
//|  GetFileInfo                                                               |
//|  fCheckFileClosed                                                          |
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
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: get rid of compiler warnings
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.2 $ ----------- 4 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFTMEXT.CV_   1.2   26 Oct 1998 18:43:42   BUILD  $
 *
 * $Log:   K:\DATA\EQFTMEXT.CV_  $
 *
 *    Rev 1.2   26 Oct 1998 18:43:42   BUILD
 * - disabled DosSleep in Windows environment
 *
 *    Rev 1.1   14 Jan 1998 14:20:56   BUILD
 * - renamed DeleteFile to TmtDeleteFile
 *
 *    Rev 1.0   09 Jan 1996 09:15:30   BUILD
 * Initial revision.
*/
// ----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#include <eqf.h>                  // General Translation Manager include file

#include <EQFTMI.H>               // Private header file of Translation Memory

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtDeleteTM                                              |
//+----------------------------------------------------------------------------+
//|Author:            Marc Hoffmann                                            |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT TmtDeleteTM( HTM        hMem,                     |
//|                                       PDELTM_IN  pDelTmIn,                 |
//|                                       PDELTM_OUT pDelTmOut )               |
//+----------------------------------------------------------------------------+
//|Description:       Deletes a TM and its property file                       |
//+----------------------------------------------------------------------------+
//|Input Parameters:  HTM         hMem       TM handle                         |
//|                   PDELTM_IN   pDelTmIn   pointer to delete in structure    |
//|                   PDELTM_OUT  pDelTmOut  pointer to delete out structure   |
//+----------------------------------------------------------------------------+
//|Output parameter:  PDELTM_OUT  pDelTmOut  pointer to delete out structure   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK (0)       - The command ended successfully.           |
//|                   Other values - Dos return codes                          |
//|                                                                            |
// ----------------------------------------------------------------------------+
USHORT TmtDeleteTM( HTM        hMem,
                    PDELTM_IN  pDelTmIn,
                    PDELTM_OUT pDelTmOut )
{
  USHORT     usDosRc = TM_FUNCTION_FAILED; // function returncode
// !!!! CHM 07/06/93  PCLOSE_OUT pCloseOut = NULL;             // Pointer to CLOSE_OUT structure
// !!!! CHM 07/06/93  PCLOSE_IN  pCloseIn  = NULL;             // Pointer to CLOSE_IN structure
  CLOSE_OUT  stCloseOut;                   // Close output structure
  CLOSE_IN   stCloseIn;                    // Close input structure

  // Security check
  if ( pDelTmIn != NULL )
  {
    // close the TM
    if ( hMem != NULLHANDLE )
    {
// !!!! CHM 07/06/93      pCloseOut = &stCloseOut;
// !!!! CHM 07/06/93      pCloseIn  = &stCloseIn;
      memset( &stCloseOut, NULC, sizeof(CLOSE_OUT));  // !!!! CHM 07/06/93
      memset( &stCloseIn,  NULC, sizeof(CLOSE_IN));   // !!!! CHM 07/06/93

      // Assign values to the appropriate fields
      stCloseIn.prefin.usLenIn     = sizeof( CLOSE_IN ); // !!!! CHM 07/06/93
      stCloseIn.prefin.idCommand = TMC_CLOSE;            // !!!! CHM 07/06/93

      usDosRc = Tmt( hMem,                 // --- TM handle
                     (PIN) &stCloseIn,     // Pointer to input structure   // !!!! CHM 07/06/93
                     (POUT) &stCloseOut ); // Pointer to output structure  // !!!! CHM 07/06/93
    } /* endif */

    if ( ( pDelTmIn->szTMPathFileName != NULL ) &&
         ( pDelTmIn->szPropPathFileName != NULL ) )
    {
      // Delete the TM
      usDosRc = UtlDelete( pDelTmIn->szTMPathFileName, 0L, FALSE );
      switch ( usDosRc )                                               /*@SAC*/
      {                                                                /*@SAC*/
        case NO_ERROR :                                                /*@SAC*/
        case ERROR_FILE_NOT_FOUND :                                    /*@SAC*/
        case ERROR_PATH_NOT_FOUND :                                    /*@SAC*/
        case ERROR_INVALID_DRIVE  :                                    /*@SAC*/
        case ERROR_ACCESS_DENIED  :                                    /*@SAC*/
        // Delete the property file
        usDosRc = UtlDelete( pDelTmIn->szPropPathFileName, 0L, FALSE );
        break;                                                         /*@SAC*/
      } /* endswitch */                                                /*@SAC*/
    } /* endif */
  } /* endif */

  // set the returncode and the output structure size
  if ( pDelTmOut != NULL )
  {
    pDelTmOut->prefout.rcTmt = usDosRc;
    pDelTmOut->prefout.usLenOut = sizeof( DELTM_OUT );
  } /* endif */

  return usDosRc;
} /* end of function TmtDeleteTM */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtCloseOrganize                                         |
//+----------------------------------------------------------------------------+
//|Author:            Marc Hoffmann                                            |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT TmtCloseOrganize( HTM         hMem,               |
//|                                            PENDORG_IN  pEndOrgIn,          |
//|                                            PENDORG_OUT pEndOrgOut )        |
//+----------------------------------------------------------------------------+
//|Description:       Finishes the organize process in the proper way          |
//+----------------------------------------------------------------------------+
//|Input parameter:   HTM         hMem        TM handle                        |
//|                   PENDORG_IN  pEndOrgIn   pointer to organize in structure |
//+----------------------------------------------------------------------------+
//|Output parameter:  PENDORG_OUT pEndOrgOut  pointer to organize out structure|
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       returncodes from Dos function                            |
// ----------------------------------------------------------------------------+
USHORT TmtCloseOrganize( HTM         hMem,
                         PENDORG_IN  pEndOrgIn,
                         PENDORG_OUT pEndOrgOut )
{
  USHORT     usDosRc = TM_FUNCTION_FAILED; // function returncode
// !!!! CHM 07/06/93  PCLOSE_OUT pCloseOut = NULL;             // Pointer to CLOSE_OUT structure
// !!!! CHM 07/06/93  PCLOSE_IN  pCloseIn  = NULL;             // Pointer to CLOSE_IN structure
  CLOSE_OUT  stCloseOut;                   // Close output structure
  CLOSE_IN   stCloseIn;                    // Close input structure
  CHAR       szRenamedMem[MAX_EQF_PATH];   // String name for renamed TM path

  // Security check
  if ( pEndOrgIn != NULL )
  {
    // close the original TM
    if ( hMem != NULLHANDLE )
    {
// !!!! CHM 07/06/93      pCloseOut = &stCloseOut;
// !!!! CHM 07/06/93      pCloseIn  = &stCloseIn;
      memset( &stCloseOut, NULC, sizeof(CLOSE_OUT) ); // !!!! CHM 07/06/93
      memset( &stCloseIn, NULC, sizeof(CLOSE_IN) );   // !!!! CHM 07/06/93

      // Assign values to the appropriate fields
      stCloseIn.prefin.usLenIn   = sizeof( CLOSE_IN );  // !!!! CHM 07/06/93
      stCloseIn.prefin.idCommand = TMC_CLOSE;           // !!!! CHM 07/06/93

      usDosRc = Tmt( hMem,
                     (PIN) &stCloseIn,      // Pointer to input structure    // !!!! CHM 07/06/93
                     (POUT) &stCloseOut );  // Pointer to output structure   // !!!! CHM 07/06/93
    } /* endif */

    if ( ( *(pEndOrgIn->szOrgTM) != '\0' ) &&    // !!!! CHM 07/06/93
         ( *(pEndOrgIn->szTmpTM) != '\0' ) &&    // !!!! CHM 07/06/93
         ( usDosRc == NO_ERROR ))
    {
      // Build the rename path
      Utlstrccpy( szRenamedMem, pEndOrgIn->szOrgTM, DOT );
      strcat( szRenamedMem, EXT_OF_RENAMED_MEM );

      //--- call function to assure that TM is really closed
      //--- this have to be done because the DosClose request is not
      //--- executed immediatly, it is queued.
      //--- This might be an HPFS effect ?
      fCheckFileClosed( pEndOrgIn->szOrgTM );                        /* @01A */

      // Rename the input TM into xxx.MER
      if ( ( usDosRc = UtlMove( pEndOrgIn->szOrgTM,
                                szRenamedMem, 0L, FALSE ) )
           == NO_ERROR )
      {

        //--- call function to assure that TM is really closed
        //--- this have to be done because the DosClose request is not
        //--- executed immediatly, it is queued.
        //--- This might be an HPFS effect ?
        fCheckFileClosed( pEndOrgIn->szTmpTM );                      /* @01A */
// !!!! CHM 07/06/93        fCheckFileClosed( pEndOrgIn->szOrgTM );                      /* @01A */

        // Move the temporary TM into the input TM
        if ( ( usDosRc = UtlMove( pEndOrgIn->szTmpTM,
                                  pEndOrgIn->szOrgTM, 0L, FALSE ) )
             == NO_ERROR )
        {
          // Everything successful renamed and moved, now delete
          // the temporary TM
          usDosRc = UtlDelete( szRenamedMem, 0L, FALSE );
        } /* endif */
        else
        {
          // An error occurred move the xxx.MER back to xxx.MEM
          UtlMove( szRenamedMem, pEndOrgIn->szOrgTM, 0L, FALSE );
        } /* endelse */
      } /* endif */
    } /* endif */
  } /* endif */

  // set the returncode and the output structure size
  if ( pEndOrgOut != NULL )
  {
    pEndOrgOut->prefout.rcTmt    = usDosRc;
    pEndOrgOut->prefout.usLenOut = sizeof( ENDORG_OUT );
  } /* endif */

  return usDosRc;
} /* end of function TmtCloseOrganize */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtDeleteFile                                            |
//+----------------------------------------------------------------------------+
//|Function call:     VOID TmtDeleteFile ( PDELFILE_IN pDelFileIn,             |
//|                                     PDELFILE_OUT pDelFileOut)              |
//+----------------------------------------------------------------------------+
//|Description:       delete a file located on the server                      |
//|                   This function tries to delete the file specified in the  |
//|                   input structure.                                         |
//+----------------------------------------------------------------------------+
//|Author:            Christian Michel                                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   pDelFileIn:  Pointer to the file input structure         |
//+----------------------------------------------------------------------------+
//|Output parameter:  pDelFileOut: Pointer to the pipe output structure        |
//|                   pDelFileOut->prefout.rcTmt : rc from Utl(Dos)Delete      |
//|                   pDelFileOut->prefout.usLenOut = sizeof (DELFILE_OUT)     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
// ----------------------------------------------------------------------------+
VOID TmtDeleteFile (PDELFILE_IN pDelFileIn, PDELFILE_OUT pDelFileOut)
{
   pDelFileOut->prefout.rcTmt = UtlDelete (pDelFileIn->szFileName, 0L, FALSE);

   pDelFileOut->prefout.usLenOut = sizeof (DELFILE_OUT);
} /* end of TmtDeleteFile */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     RenameFile                                               |
//+----------------------------------------------------------------------------+
//|Author:            Christian Michel                                         |
//+----------------------------------------------------------------------------+
//|Function call:     VOID RenameFile ( PRENFILE_IN pRenFileIn,                |
//|                                     PRENFILE_OUT pRenFileOut )             |
//+----------------------------------------------------------------------------+
//|Description:       This function tries to rename the file specified in the  |
//|                   input structure.                                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   pRenFileIn : pointer to rename in structure              |
//+----------------------------------------------------------------------------+
//|Output parameter:  pRenFileOut: pointer to rename out structure             |
//|                   pDelRenOut->prefout.rcTmt : rc from Utl(Dos)Delete       |
//|                   pDelRenOut->prefout.usLenOut = sizeof (DELFILE_OUT)      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
// ----------------------------------------------------------------------------+
VOID RenameFile (PRENFILE_IN pRenFileIn, PRENFILE_OUT pRenFileOut)
{
   pRenFileOut->prefout.rcTmt = UtlMove (pRenFileIn->szOldFile,
                                         pRenFileIn->szNewFile, 0L, FALSE);

   pRenFileOut->prefout.usLenOut = sizeof (RENFILE_OUT);
} /* end of DeleteFile */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetFileInfo                                              |
//+----------------------------------------------------------------------------+
//|Author:            Christian Michel                                         |
//+----------------------------------------------------------------------------+
//|Function call:     VOID GetFileInfo ( PFILEINFO_IN pFileInfoIn,             |
//|                                      PFILEINFO_OUT pFileInfoOut )          |
//+----------------------------------------------------------------------------+
//|Description:       This function queries the file info for the specified    |
//|                   file with the DosFindFirst function and returns the      |
//|                   complete FILEFINDBUF structure. The return code of the   |
//|                   DosFindFirst function is copied to the rcTmt element of  |
//|                   the output structure.                                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   pFileInfoIn : pointer to fime info in structure          |
//+----------------------------------------------------------------------------+
//|Output parameter:  pFileInfoOut : pointer to fime info out structure        |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
// ----------------------------------------------------------------------------+
VOID GetFileInfo (PFILEINFO_IN pFileInfoIn, PFILEINFO_OUT pFileInfoOut)
{
   HDIR              hSearch;             /* Dir handle for DosFindxxx calls */
   USHORT            usSearchCount,       /* number of entries to find       */
                     usRc;

   /* first look for the file specified in the input structure with a        */
   /* DosFindFirst call; use the stFile element in the return structure for  */
   /* this call; the Dos return code of the DosFindFirst function will be    */
   /* copied to the rcTmt element in the output data structure               */
   usSearchCount = 1;                                  /* find only one file */
   hSearch = HDIR_CREATE;                   /* Allocate a new, unused handle */

   usRc = UtlFindFirst (pFileInfoIn->szFileName, &hSearch, 0,
                        &(pFileInfoOut->stFile), sizeof (pFileInfoOut->stFile),
                        &usSearchCount, 0L, FALSE);
   if (hSearch)
   {
      /* close the directory handle only if it is a valid handle             */
      UtlFindClose (hSearch, FALSE);
   } /* endif */

   pFileInfoOut->prefout.rcTmt = usRc;
   pFileInfoOut->prefout.usLenOut = sizeof (FILEINFO_OUT);
} /* end of GetFileInfo */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fCheckFileClosed                                         |
//+----------------------------------------------------------------------------+
//|Function call:     BOOL fCheckFileClosed (PSZ pszFileName)                  |
//+----------------------------------------------------------------------------+
//|Author: Christian Michel                                                    |
//+----------------------------------------------------------------------------+
//|Description:       Tries to open the specified file exclusively to check    |
//|                   whether the file was closed correctly before exclusive   |
//|                   operations, such as delete or move, can be performed on  |
//|                   the file. If the file open fails, it tries up to 10 times|
//|                   to open it again after waiting a short time.             |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ pszFileName - Name of file to check                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE  - file was closed                                  |
//|                   FALSE - file is still open                               |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
// ----------------------------------------------------------------------------+
BOOL fCheckFileClosed (PSZ pszFileName)                             /* 14@01A */
{
   USHORT   usCount = 10,  /* max. number of open calls */
            usRc,
            usAction;
   HFILE    hfOpen;
   BOOL     fFileClosed = FALSE;

   do
   {
      usRc = UtlOpen (pszFileName,
                      &hfOpen,     /* file handle */
                      &usAction,   /* action taken after open */
                      0L,          /* Filesize is irrelevant */
                      FILE_NORMAL, /* attributes */
                      FILE_OPEN,   /* open flag, open it, don't create it */
                      (USHORT)(OPEN_ACCESS_READWRITE    | /* sharing mode flags */
                      OPEN_SHARE_DENYREADWRITE |
                      OPEN_FLAGS_NO_CACHE      |
                      OPEN_FLAGS_WRITE_THROUGH |
                      OPEN_FLAGS_FAIL_ON_ERROR),
                      0L,           /* reserved value, must be 0 */
                      FALSE);       /* no error handling */

      if (usRc == NO_ERROR)
      {
         /* file was opened successfully, so it was closed before */
         /* close the file again (maybe in critical section) */
         usRc = UtlClose (hfOpen, FALSE);
         fFileClosed = TRUE;
      } /* endif */
   } while (usCount-- && !fFileClosed); /* enddo */

   return (fFileClosed);
} /* end of function fCheckFileClosed */
