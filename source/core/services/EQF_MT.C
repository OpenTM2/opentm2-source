//+----------------------------------------------------------------------------+
//|EQF_MT.C                                                                    |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:   G.Jornitz                                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  MT accessing thread for MAT Tools                             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|    EQFMT     --  mt thread                                                 |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|    ActivateMT     -- initialise dicitonary                                 |
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
// $Revision: 1.2 $ ----------- 29 Jul 2002
// --RJ: R07197: use system pref. lang
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: Unicode enablement
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
 * $Header:   J:\DATA\EQF_MT.CV_   1.3   14 Jan 1998 15:12:56   BUILD  $
 *
 * $Log:   J:\DATA\EQF_MT.CV_  $
 *
 *    Rev 1.3   14 Jan 1998 15:12:56   BUILD
 * - changed MAX_PATH to MAX_PATH144 or MAX_LONGPATH
 *
 *    Rev 1.2   26 Feb 1997 17:15:52   BUILD
 * -- Compiler defines for _POE22, _TKT21, and NEWTCSTUFF eliminated
 *
 *    Rev 1.1   28 Mar 1996 10:06:00   BUILD
 * - removed unused preprocessor directives
 *
 *    Rev 1.0   09 Jan 1996 09:18:22   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis defines (required for EQFTAI.H)
#include <eqf.h>                  // General Translation Manager include file

#include <eqftai.h>               // private analysis include file for EQFDOC00.H

#include <eqfdoc00.h>             // for document handler defines
#include <PROCESS.H>              // C process functions

static USHORT  ActivateMT      ( PDOCUMENT_IDA );

static  HMODULE  hmodMTDll = NULLHANDLE;     // handle of MT DLL
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFMT                                                    |
//+----------------------------------------------------------------------------+
//|Function call:     EQFMT                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       MT access thread                                         |
//|                   opens the connection to MT,                              |
//|                   sends the requests to the MT subsystem                   |
//|                   closes the connection                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameter:         PVOID     -- pointer to PDOC structure                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      fSemMTProc will be used for controling access            |
//|                    fSemMTProc = TRUE: thread is working                    |
//|                               = FALSE: thread waits for work               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     activate the MT subsystem                                |
//|                   set fSemMTProc to FALSE                                  |
//|                   while pDoc->fRunMt                                       |
//|                     while !fSemMTProc                                      |
//|                        sleep a short amount                                |
//|                     endwhile                                               |
//|                     call EQFMTTrans indirectly                             |
//|                     set fMTReady for this segment                          |
//|                     indicate waiting for further work (fSemMTProc = FALSE) |
//|                   endwhile                                                 |
//|                   close MT subsystem                                       |
//|                   indicate finished                                        |
//|                   return                                                   |
//+----------------------------------------------------------------------------+

VOID  EQFMT
(
   PVOID pInstance                     // pointer to request structure
)
{
   BOOL          fOK = TRUE;           // success indicator
   PSTEQFSAB     pstQReq;              // request queue
   PSTEQFGEN     pstEQFGen;            // pointer to generic structure
   PDOCUMENT_IDA pDoc;

   pDoc     = (PDOCUMENT_IDA) pInstance;
   pstEQFGen = pDoc->pstEQFGen;

   ActivateMT( pDoc );                           // activate the MT subsystem

   pDoc->fSemMTProc = FALSE;

   while (pDoc->fRunMT)
   {
      if ( pDoc->fRunMT )
      {
        pstQReq  = pDoc->pstEQFQMT;        // get pointer to new buffer

        /**************************************************************/
        /*  if nothing in segment we are done, else sent request      */
        /*  to MT subsystem ...                                       */
        /**************************************************************/
        if ( *(pstQReq->pucSourceSeg) )
        {
      CHAR szSourceSeg[ MAX_SEGMENT_SIZE ];
      CHAR szMTSeg[ MAX_SEGMENT_SIZE ];

      Unicode2ASCII( pstQReq->pucSourceSeg, szSourceSeg, 0L );

          // find MT translation for segment
          fOK = pDoc->pfnEQFMTTrans( pDoc->szDocName,           // document name
                                     (USHORT) pstQReq->ulParm1, // segment number
                                     szSourceSeg,               // source segment
                                     szMTSeg,                   // machine transl.
                                     (PSZ)pDoc->szMTBuffer );
          if ( !fOK )
          {
      ASCII2Unicode( szMTSeg, pstQReq->pucMTSeg, 0L );
      ASCII2Unicode( (PSZ)pDoc->szMTBuffer, pstEQFGen->szMsgBuffer, 0L  );

            pstEQFGen->fsConfiguration &= ~EQFF_MT_CONF;   // no MT configured
            pDoc->fRunMT = FALSE;
          } /* endif */
        }
        else
        {
          memset(pstQReq->pucMTSeg, 0, 8 * sizeof( CHAR_W) );
        } /* endif */
        pstQReq->fMTReady   = TRUE;              // data processed for segment.
        pDoc->fSemMTProc = FALSE;
      } /* endif */
   } // endwhile (running)

   pDoc->fSemMTProc = TRUE;
   fOK = pDoc->pfnEQFMTClose( (PSZ)pDoc->szMTBuffer );
   if ( !fOK )
   {
     UTF16strcpy( pstEQFGen->szMsgBuffer, pDoc->szMTBuffer );
     pstEQFGen->usRC = EQFS_MTERROR;
   } /* endif */

   DosFreeModule( hmodMTDll );
} // end thread 'EQFMT'


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: ActivateMT                                                   |
//+----------------------------------------------------------------------------+
//|Function call:     ActivateMT ( PDOCUMENT_IDA )                             |
//+----------------------------------------------------------------------------+
//|Description:       open the requested MT interface                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PDOCUMENT_IDA  pointer to document ida                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE                 every thing okay                    |
//|                   FALSE                error happened activating the conn. |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     init the message buffer                                  |
//|                   call the EQFMTInit indirectly and set return code        |
//|                   if !fOK then                                             |
//|                     copy return message and set return code                |
//|                   endif                                                    |
//|                   set pDoc->fRunMT depending on results so far             |
//|                   return returncode                                        |
//+----------------------------------------------------------------------------+
static
USHORT  ActivateMT
(
    PDOCUMENT_IDA pDoc                          // pointer to document
)
{
  BOOL          fOK = TRUE;                     // success indicator
  PSTEQFGEN     pstEQFGen;                       // pointer to generic struct
  CHAR          szFileName[ MAX_PATH144 ];

  pstEQFGen = pDoc->pstEQFGen;                  // get pointer to generic struct
  pDoc->szMTBuffer[0] = EOS;                     // init return buffer
  fOK = pDoc->pfnEQFMTInit( pDoc->szFolderName,  // name of the folder
                            pDoc->szDocName,     // document name
                            pDoc->usSrcLang,     // source language
                            pDoc->usTgtLang,     // target language
                            (PSZ)pDoc->szMTBuffer );  // message buffer

  if ( !fOK )
  {
    UTF16strcpy (pstEQFGen->szMsgBuffer, pDoc->szMTBuffer );
    pstEQFGen->usRC = EQFS_MTERROR;
    pstEQFGen->fsConfiguration &= ~EQFF_MT_CONF;           // no MT configured
  }
  else
  {
    /**********************************************************************/
    /* set MT display flags depending on existance of file for the moment */
    /**********************************************************************/
    UtlMakeEQFPath( szFileName, NULC, PROPERTY_PATH, NULL );
    strcat( szFileName, BACKSLASH_STR );
    strcat( szFileName, pDoc->szMTDll );
    pDoc->fNoMTProp = UtlFileExist( szFileName );

  } /* endif */
  pDoc->fRunMT = (USHORT)fOK ;                           // set global run flag

  return ((USHORT)fOK);
} // end 'ActivateMT'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFMTInit                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFMTInit( PDOCUMENT_IDA );                              |
//+----------------------------------------------------------------------------+
//|Description:       check if the MT DLL is okay, then start the thread       |
//+----------------------------------------------------------------------------+
//|Parameters:        PDOCUMENT_IDA       pointer to document ida              |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     load the specified DLL                                   |
//|                   check if supplied DLL contains all entry points          |
//|                   if not all entry points or loading error then            |
//|                     issue error message                                    |
//|                   else                                                     |
//|                     allocate memory                                        |
//|                   endif                                                    |
//|                   if okay then                                             |
//|                     start thread                                           |
//|                   endif                                                    |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
VOID EQFMTInit
(
   PDOCUMENT_IDA pDoc                           // pointer to document ida
)
{
   USHORT usRc;
   BOOL   fOK = TRUE;                            // success indicator
   PSZ    pszInsert;                             // dummy variable for UtlError
   CHAR   szTemp[100];                          // string for language property
   CHAR   szTemp1[100];                         // string for language property
   PSZ    pMem;                                 // pointer to memory

  /********************************************************************/
  /* hardcode the name of the TTMT DLL for the moment                 */
  /* should be moved in system properties                             */
  /********************************************************************/
   strcpy( pDoc->szMTDll, "EQFTTMT" );


   pDoc->pstEQFQMT  = pDoc->stEQFSab;
   /*******************************************************************/
   /* check if supplied DLL contains all entry points                 */
   /*******************************************************************/
   usRc = DosLoadModule( NULL, 0 ,
                         pDoc->szMTDll ,
                         &(hmodMTDll));

   if ( usRc == NO_ERROR )
   {
     // load addresses of start and stop functions
     usRc = DosGetProcAddr( hmodMTDll,
                            EQFMTINIT,
                            (PFN*)(&(pDoc->pfnEQFMTInit)));
     if ( usRc == NO_ERROR )
     {
        usRc = DosGetProcAddr( hmodMTDll,
                               EQFMTTRANS,
                               (PFN*) (&(pDoc->pfnEQFMTTrans)));
     } /* endif */
     if ( usRc == NO_ERROR )
     {
        usRc = DosGetProcAddr( hmodMTDll,
                               EQFMTCLOSE,
                               (PFN*) (&(pDoc->pfnEQFMTClose)));
     } /* endif */
     if ( usRc != NO_ERROR )
     {
       //set MT subsystem name and display error message
       pszInsert = pDoc->szMTDll;
       usRc = UtlError( ERROR_START_MT, MB_YESNO, 1, &pszInsert, EQF_QUERY);
       if ( usRc != MBID_YES )
       {
         fOK = FALSE;
         pDoc->pstEQFGen->usRC = ERROR_MSG_HANDLED;
       } /* endif */
       pDoc->pstEQFGen->fsConfiguration &= ~EQFF_MT_CONF;  // no MT configured
     }
     else
     {
       /***************************************************************/
       /* fill in the source and target languages                     */
       /***************************************************************/
       pMem =  pDoc->szMemory[0];          // pointer to active transl. memory
       pMem = UtlGetFnameFromPath( pMem );
       Utlstrccpy( szTemp, pMem, DOT);            // get filename without extension
       fOK  = (BOOL) EqfSend2Handler( MEMORYHANDLER,
                                      WM_EQF_PROCESSTASK,
                                      MP1FROMSHORT( QUERY_LANG_PAIR_INPUT_MEM ),
                                      MP2FROMP( szTemp ));
       if ( fOK )
       {
          strcpy(szTemp1, UtlParseX15( szTemp,TARGET_LANGUAGE_TM_IND));
          fOK  = (BOOL) EqfSend2Handler( MEMORYHANDLER,
                                         WM_EQF_PROCESSTASK,
                                         MP1FROMSHORT( QUERY_LANG_PROP_INPUT_LANG),
                                         MP2FROMP( szTemp1));
          if ( fOK )
          {
             pDoc->usTgtLang = (USHORT)atoi(UtlParseX15( szTemp1,LANG_LANG_CODE_IND));
          } /* endif */
       }
       if ( fOK )
       {
          strcpy(szTemp1, UtlParseX15( szTemp,SOURCE_LANGUAGE_TM_IND));
          fOK  = (BOOL) EqfSend2Handler( MEMORYHANDLER,
                                         WM_EQF_PROCESSTASK,
                                         MP1FROMSHORT( QUERY_LANG_PROP_INPUT_LANG),
                                         MP2FROMP( szTemp1));
          if ( fOK )
          {
             pDoc->usSrcLang = (USHORT)atoi(UtlParseX15( szTemp1,LANG_LANG_CODE_IND));
          } /* endif */
       }
     } /* endif */
   }
   else
   {
     pDoc->pstEQFGen->fsConfiguration &= ~EQFF_MT_CONF;    // no MT configured
     fOK = FALSE;
   } /* endif */


   /*******************************************************************/
   /* allocate the stack memory                                       */
   /*******************************************************************/
   if ( fOK )
   {
     fOK = UtlAlloc( &(pDoc->pStackMT), 0L, (ULONG) REQ_STACKSIZE,
                     ERROR_STORAGE);
     if ( !fOK )
     {
       pDoc->pstEQFGen->usRC = ERROR_MSG_HANDLED;
     } /* endif */
   } /* endif */
   /*******************************************************************/
   /* start the thread                                                */
   /*******************************************************************/
   if ( fOK )
   {
     if ( pDoc->tidMT == -1)                       // thread successfully started?
     {
        pDoc->pstEQFGen->usRC = ERROR_STORAGE;
        pDoc->fSemMTProc = FALSE;
     }
     else
     {
        // let thread some time to process initialisation
        pDoc->fSemMTProc = TRUE;
     } /* endif */
   } /* endif */
   return;
}

