//------------------------------------------------------------------------------
//EQFMEMAddMatchSegID.C                                                                  
//------------------------------------------------------------------------------
//Copyright Notice:                                                           
//                                                                            
//      Copyright (C) 1990-2016, International Business Machines              
//      Corporation and others. All rights reserved                           
//------------------------------------------------------------------------------
//Description:                                                                
// Implements the processing for the API call EqfAddmatchSegID
//------------------------------------------------------------------------------
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "MemoryFactory.h"
#include "MemoryUtil.h"
#include <time.h>                 // C library for time functions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMEM.ID>              // Translation Memory IDs
#include "OTMFUNC.H"              // public defines for function call interface
#include "EQFFUNCI.H"             // private defines for function call interface

enum { MATCHID_START_TASK, MATCHID_WORK_TASK, MATCHID_END_TASK } MATCHUD_TASKS;

// add match segment ID data area
typedef struct _ADDMATCHSEGIDDATA
{
 OtmProposal   *pProposal;                       // buffer for memory proposal
 OtmMemory     *pMem;                            // Handle of transl. memory
 OtmMemory     *pMemTemp;                        // Handle of tmporary transl. memory
 ULONG         ulSegmentCounter;                 // Number of segments processed
 LONG          NextTask;                         // next task to be performed
 CHAR          szMemName[MAX_LONGFILESPEC];      // Translation memory name 
 CHAR          szTempMemName[MAX_LONGFILESPEC];  // name of temporary translation memory 
 CHAR          szPluginName[MAX_LONGFILESPEC];   // name of plugin used for the memory
 CHAR          szMatchIDPrefix[256];             // prefix to be used for the match segment ID
 CHAR_W        szMatchIDPrefixW[256];            // prefix to be used for the match segment ID (UTF16 version)
 BOOL          fFirstGet;                        // TRUE = this is the first get access
 BOOL          fSuccess;                         // TRUE = processing has completed successfully
 BOOL          fForceNewMatchID;                 // TRUE = force new match segment ID even if the smatch already has one
 LANGUAGE      szSourceLanguage;                 // source language of the memory
 CHAR          szBuffer[2048];                   // general purpose buffer
 CHAR_W        szAddDataBuffer[MAX_SEGMENT_SIZE+1];// buffer for additional data of the match
} ADDMATCHSEGIDDATA, *PADDMATCHSEGIDDATA;

static USHORT AddMatchSegIDPrepare( PFCTDATA pData, PSZ pszMemName, PSZ pszTM_ID, PSZ pszStoreID, LONG lOptions );
static USHORT AddMatchSegIDProcess( PFCTDATA pData );

static int AddMatchSegIDStart( PADDMATCHSEGIDDATA pMatchSegIDData );
static int AddMatchSegIDWork( PADDMATCHSEGIDDATA  pMatchSegIDData );
static int AddMatchSegIDEnd( PADDMATCHSEGIDDATA pMatchSegIDData );


// Do the processing for the EqfAddMatchSegID API call
USHORT MemFuncAddMatchSegID
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName,              // Translation Memory being processed
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  LONG        lOptions                 // options for the function
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new organize run or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new analysis run
    usRC = AddMatchSegIDPrepare( pData, pszMemName, pszTM_ID, pszStoreID, lOptions );
  }
  else
  {
    // continue current organize process
    usRC = AddMatchSegIDProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function MemFuncAddMatchSegID */

// Prepare the add of match segment IDs
USHORT AddMatchSegIDPrepare
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName,              // Translation Memory being deleted
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  LONG        lOptions                 // options for the function
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PADDMATCHSEGIDDATA pMatchSegIDData = NULL;      // pointer to instance area
  BOOL        fOK = TRUE;              // internal O.K. flag
  PSZ         pszParm;                 // error parameter pointer
  OtmMemory *pMem = NULL;       // TM handle
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  // check if a TM has been specified
  if ( fOK )
  {
    if ( (pszMemName == NULL) || (*pszMemName == EOS) )
    {
      fOK = FALSE;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if there is a TM with the given name                
  if ( fOK )
  {
    if ( !pFactory->exists( NULL, pszMemName ) )
    {
      fOK = FALSE;
      UtlErrorHwnd( ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1, &pszMemName, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if the parameters for the creation of the segment match ID have been specified
  if ( fOK )
  {
    if ( ((pszTM_ID == NULL) || (*pszTM_ID == EOS)) && ((pszStoreID == NULL) || (*pszStoreID == EOS)) )
    {
      fOK = FALSE;
      UtlErrorHwnd( FUNC_ADDMATCHSEGID_NOIDSPECS, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // opem the memory
  if ( fOK )
  {
    int iRC = 0;
    pMem = pFactory->openMemory( NULL, pszMemName, EXCLUSIVE, &iRC );
    if ( iRC != 0)
    {
      pFactory->showLastError( NULL, pszMemName, NULL, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */


   // prepare adding of match segment IDs
   if ( fOK )
   {
      // allocate storage for the ADDMATCHSEGIDDATA area
      fOK = UtlAllocHwnd( (PVOID *)&pMatchSegIDData, 0L, (LONG)sizeof(ADDMATCHSEGIDDATA), ERROR_STORAGE, HWND_FUNCIF );

      if ( fOK )
      {
        // Fill  IDA with necessary values
        pMatchSegIDData->pMem = pMem;
        strcpy( pMatchSegIDData->szMemName, pszMemName );
        pMatchSegIDData->NextTask = MATCHID_START_TASK;
        pMatchSegIDData->fForceNewMatchID = lOptions & FORCENEWMATCHID_OPT;

        MADPrepareMatchSegIDPrefix( pszTM_ID, pszStoreID, pMatchSegIDData->szMatchIDPrefix );
        if ( pMatchSegIDData->szMatchIDPrefix[0] ) MultiByteToWideChar( CP_ACP, 0, pMatchSegIDData->szMatchIDPrefix, -1, pMatchSegIDData->szMatchIDPrefixW, sizeof(pMatchSegIDData->szMatchIDPrefixW)/sizeof(CHAR_W) );

        if ( !fOK )
        {
          // free IDA, otherwise it will be freed in organize process
          UtlAlloc( (PVOID *)&pMatchSegIDData, 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */
   } /* endif */

   // enable add match segment ID process if OK
   if ( fOK )
   {
     pData->fComplete = FALSE;
     pData->sLastFunction = FCT_EQFADDMATCHSEGID;
     pData->pvAddMatchSegID = pMatchSegIDData;
    } /* endif */

   // cleanup in case of errors
   if ( !fOK )
   {
     usRC = UtlGetDDEErrorCode( HWND_FUNCIF );
     if ( pMem != NULL )
     {
       pFactory->closeMemory( pMem );
     } /* endif */
     pData->fComplete = TRUE;
   } /* endif */

  return( usRC );
} /* end of function AddMatchSegIDPrepare */

// Do the actual processing to add match IDs to the segments
USHORT AddMatchSegIDProcess
(
  PFCTDATA    pData                    // function I/F session data
)
{
  INT      iRC = 0;

  PADDMATCHSEGIDDATA pMatchSegIDData;             // pointer to instance area

  pMatchSegIDData = (PADDMATCHSEGIDDATA)pData->pvAddMatchSegID;

  switch ( pMatchSegIDData->NextTask )
  {
    case MATCHID_START_TASK:
      iRC = AddMatchSegIDStart( pMatchSegIDData );
      break;

    case MATCHID_WORK_TASK:
      iRC = AddMatchSegIDWork( pMatchSegIDData );
      break;

    case MATCHID_END_TASK:
      pData->fComplete = TRUE;
      iRC = AddMatchSegIDEnd( pMatchSegIDData );
      if ( pMatchSegIDData ) UtlAlloc( (PVOID *)&pMatchSegIDData, 0L, 0L, NOMSG );
      pData->pvAddMatchSegID = NULL;
      break;
  } /* endswitch */

  return( (USHORT)iRC );
} /* end of function AddmatchSegID */


int AddMatchSegIDStart( PADDMATCHSEGIDDATA pMatchSegIDData )
{
  MemoryFactory *pFactory = MemoryFactory::getInstance();
  int               iRC = 0;

  // Address the organize IDA
  pMatchSegIDData->fFirstGet = TRUE;
  pMatchSegIDData->pProposal = new OtmProposal();

  // Create the Output translation memory
  if ( iRC == 0 )
  {
    OtmMemoryPlugin::MEMORYINFO MemInfo;
    OtmPlugin *pAnyPlugin = (OtmPlugin *)pMatchSegIDData->pMem->getPlugin();

    memset( &MemInfo, 0,sizeof(MemInfo) );

    if ( pAnyPlugin->getType() == OtmPlugin::eTranslationMemoryType )
    {
      OtmMemoryPlugin *pPlugin = (OtmMemoryPlugin *)pAnyPlugin;
      pMatchSegIDData->pMem->getSourceLanguage( pMatchSegIDData->szSourceLanguage, sizeof(pMatchSegIDData->szSourceLanguage) );
      pMatchSegIDData->pMem->getDescription( pMatchSegIDData->szBuffer, sizeof(pMatchSegIDData->szBuffer) );
      pPlugin->getMemoryInfo( pMatchSegIDData->szMemName, &MemInfo );
    }
    else if ( pAnyPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType )
    {
      OtmSharedMemoryPlugin *pPlugin = (OtmSharedMemoryPlugin *)pAnyPlugin;
      pMatchSegIDData->pMem->getSourceLanguage( pMatchSegIDData->szSourceLanguage, sizeof(pMatchSegIDData->szSourceLanguage) );
      pMatchSegIDData->pMem->getDescription( pMatchSegIDData->szBuffer, sizeof(pMatchSegIDData->szBuffer) );
      pPlugin->getMemoryInfo( pMatchSegIDData->szMemName, &MemInfo );
    }
    strcpy( pMatchSegIDData->szPluginName, pAnyPlugin->getName() );
    strcpy( pMatchSegIDData->szTempMemName, "$AddMatchID-" );
    strcat( pMatchSegIDData->szTempMemName, pMatchSegIDData->szMemName );

    // delete any memory left over from a previous call
    pFactory->deleteMemory( pMatchSegIDData->szPluginName, pMatchSegIDData->szTempMemName );

    if ( MemInfo.szFullPath[0] != '\0' )
    {
      pMatchSegIDData->pMemTemp = pFactory->createMemory( pMatchSegIDData->szPluginName, pMatchSegIDData->szTempMemName, pMatchSegIDData->szBuffer, pMatchSegIDData->szSourceLanguage, MemInfo.szFullPath[0], NULL, true, &iRC );
    }
    else
    {
      pMatchSegIDData->pMemTemp = pFactory->createMemory( pMatchSegIDData->szPluginName, pMatchSegIDData->szTempMemName, pMatchSegIDData->szBuffer, pMatchSegIDData->szSourceLanguage, '\0', NULL, true, &iRC );
    }

    if ( (iRC != 0) || (pMatchSegIDData->pMemTemp == NULL ) )
    {
      pFactory->showLastError( pMatchSegIDData->szPluginName, pMatchSegIDData->szTempMemName, NULL, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( iRC == 0 )
  {
    pMatchSegIDData->NextTask = MATCHID_WORK_TASK;
  }
  else
  {
    pMatchSegIDData->NextTask = MATCHID_END_TASK;
  } /* endif */

  return iRC;;
  } /* end of function AddMatchSegIDStart */


int AddMatchSegIDWork( PADDMATCHSEGIDDATA  pMatchSegIDData )
{
  int iRC = 0;
  PSZ               pReplAddr[3];   // Arrey of pointers to replacement strings

  if ( iRC == 0 )
  {
    int iProgress = 0;

    if ( pMatchSegIDData->fFirstGet )
    {
      iRC = pMatchSegIDData->pMem->getFirstProposal( *(pMatchSegIDData->pProposal), &iProgress );
      pMatchSegIDData->fFirstGet = FALSE;
    }
    else
    {
      iRC = pMatchSegIDData->pMem->getNextProposal( *(pMatchSegIDData->pProposal), &iProgress );
    } /* endif */         

    if ( iRC == 0 )
    {
      pMatchSegIDData->ulSegmentCounter++;
      pMatchSegIDData->pProposal->getAddInfo( pMatchSegIDData->szAddDataBuffer, sizeof(pMatchSegIDData->szAddDataBuffer)/sizeof(CHAR_W) );
      if ( MADAddMatchSegID( pMatchSegIDData->szAddDataBuffer, pMatchSegIDData->szMatchIDPrefixW, pMatchSegIDData->ulSegmentCounter, pMatchSegIDData->fForceNewMatchID ) )
      {
        pMatchSegIDData->pProposal->setAddInfo( pMatchSegIDData->szAddDataBuffer );
      } /* endif */

      // write proposal to output memory
      iRC = pMatchSegIDData->pMemTemp->putProposal( *(pMatchSegIDData->pProposal) );
    }
    else if ( iRC == OtmMemory::ERROR_ENTRYISCORRUPTED )
    {
      // ignore corrupted entry
    }
    else if ( iRC == OtmMemory::INFO_ENDREACHED )
    {
      pMatchSegIDData->fSuccess = TRUE;
      pMatchSegIDData->NextTask = MATCHID_END_TASK;
      iRC = 0;
    }
    else
    {
      OEMTOANSI( pMatchSegIDData->szMemName );
      pReplAddr[0] = pMatchSegIDData->szMemName;
      UtlErrorHwnd( ERROR_MEM_ORGANIZE_TERMINATED, MB_CANCEL, 1, &pReplAddr[0], EQF_ERROR, HWND_FUNCIF );
      ANSITOOEM( pMatchSegIDData->szMemName );

      pMatchSegIDData->NextTask = MATCHID_END_TASK;
    } /* endif */
  } /* endif */
  return( iRC );
} /* end of function AddMatchSegIDWork */


// do the end processing 
int AddMatchSegIDEnd( PADDMATCHSEGIDDATA pMatchSegIDData )
{
  int iRC = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( pMatchSegIDData->pMemTemp != NULL )
  {
    if ( pMatchSegIDData->fSuccess )
    {
      pMatchSegIDData->pMemTemp->rebuildIndex();
    }
    pFactory->closeMemory( pMatchSegIDData->pMemTemp );
    pMatchSegIDData->pMemTemp = NULL;
  }

  if ( pMatchSegIDData->pMem != NULL )
  {
    pFactory->closeMemory( pMatchSegIDData->pMem );
    pMatchSegIDData->pMem = NULL;
  } /* endif */

  // in case of successful processing replace the original memory with the temporary memory containing the segment match IDs otherwise delete the temporary memory
  if ( pMatchSegIDData->fSuccess )
  {
    iRC = pFactory->replaceMemory( pMatchSegIDData->szPluginName, pMatchSegIDData->szMemName, pMatchSegIDData->szTempMemName );
  }
  else
  {
    pFactory->deleteMemory( pMatchSegIDData->szPluginName, pMatchSegIDData->szTempMemName );
  } /* endif */

  if ( pMatchSegIDData->pProposal != NULL )
  {
    delete pMatchSegIDData->pProposal;
    pMatchSegIDData->pProposal = NULL;
  } /* endif */

 return iRC;
} /* end of function AddMatchSegIDCLose */
