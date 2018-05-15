/*! \file
	Description: This module contains the routines that work on documents and lines.

	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_TM               // public translation memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_FOLDER           // folder related functions
//#define INCL_EQF_ANALYSIS         // analysis functions
//#define INCL_EQF_TAGTABLE         // tagtable defines and functions
//#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file

#include "OtmProposal.h"
//#include "core\pluginmanager\OtmMorph.h"
#include "core\memory\MemoryFactory.h"
#include "EQFHLOG.H"              // defines for history log processing

//#include "vector"

// type of memory matches
typedef enum _MATCHTYPE
{
  NO_MATCH, FUZZY_MATCH, EXACTMORE_MATCH, EXACTONE_MATCH, EXACTEXACT_MATCH,
  REPL_MATCH, FUZZYREPL_MATCH, MACHINE_MATCH, GLOBMEM_MATCH
} MATCHTYPE, *PMATCHTYPE;

// data structure for adjust count information
typedef struct _ADJUSTCOUNTINFODATA
{
  // count fields
  ULONG ulNoProps;
  ULONG ulFuzzy;
  ULONG ulExactExact;
  ULONG ulExactOne;
  ULONG ulExactMore;
  ULONG ulRepl;
  ULONG ulFuzzyRepl;
  ULONG ulMachineMatch;
  ULONG ulTotal;
  ULONG ulSegNoProps;
  ULONG ulSegFuzzy;
  ULONG ulSegExactExact;
  ULONG ulSegExactOne;
  ULONG ulSegExactMore;
  ULONG ulSegTotal;
  ULONG ulSegRepl;
  ULONG ulSegFuzzyRepl;
  ULONG ulSegMachineMatch;
  COUNTSUMS Total;
  COUNTSUMS ExactExact;
  COUNTSUMS ExactOne;
  COUNTSUMS Fuzzy1;
  COUNTSUMS Fuzzy2;
  COUNTSUMS Fuzzy3;
  COUNTSUMS NoProps;
  COUNTSUMS MTProps;
  COUNTSUMS Repl;
  COUNTSUMS ExactMore;
  // memory lookup fields
  OtmProposal      SearchKey;                  // search key for memory lookup
  std::vector<OtmMemory *> MemoryDBs;            // list of memory databases
  std::vector<OtmProposal *> FoundProposals;     // list of found proposals
  std::vector<OtmProposal *> BestProposals;      // list of best proposals
  CHAR   szMemNames[MAX_NUM_OF_FOLDER_MDB+1][MAX_LONGFILESPEC]; // name of memories 
  CHAR   szBuffer[2048];                        // general purpose buffer
  CHAR   szTagTable[MAX_EQF_PATH];              // document format
  CHAR   szSourceLanguage[MAX_EQF_PATH];        // source language
  CHAR   szTargetLanguage[MAX_EQF_PATH];        // target language
  CHAR   szLongName[MAX_LONGFILESPEC];          // LongName,

} ADJUSTCOUNTINFODATA, *PADJUSTCOUNTINFODATA;

// prototypes for helper functions
USHORT ACI_UpdateDocProps( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjectName );
USHORT ACI_CloseMemory( PADJUSTCOUNTINFODATA pData );
USHORT ACI_OpenMemory( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName );
USHORT ACI_PrepareSearchKey( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName );
USHORT ACI_GetmatchType( PADJUSTCOUNTINFODATA pData, PTBSEGMENT pSeg, PMATCHTYPE pMatchType );
USHORT ACI_AddCounts( PCOUNTSUMS pSum, LONG lSegs, LONG lWords );

// Adjust word count information in document properties
USHORT EQFBAdjustCountInfo( PTBDOCUMENT pDoc, PSZ pszDocObjName )
{
  PADJUSTCOUNTINFODATA pData = NULL;
  USHORT usRC = 0;

  // allocate our data area
  pData = new(ADJUSTCOUNTINFODATA);

  if ( pData == NULL )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;  
  } /* endif */     

  // clear all counters in the data area
  pData->ulNoProps = 0;
  pData->ulFuzzy = 0;
  pData->ulExactExact = 0;
  pData->ulExactOne = 0;
  pData->ulExactMore = 0;
  pData->ulRepl = 0;
  pData->ulFuzzyRepl = 0;
  pData->ulMachineMatch = 0;
  pData->ulTotal = 0;
  pData->ulSegNoProps = 0;
  pData->ulSegFuzzy = 0;
  pData->ulSegExactExact = 0;
  pData->ulSegExactOne = 0;
  pData->ulSegExactMore = 0;
  pData->ulSegTotal = 0;
  pData->ulSegRepl = 0;
  pData->ulSegFuzzyRepl = 0;
  pData->ulSegMachineMatch = 0;
  memset( &(pData->Total), 0, sizeof(pData->Total) );
  memset( &(pData->ExactExact), 0, sizeof(pData->ExactExact) );
  memset( &(pData->ExactOne), 0, sizeof(pData->ExactOne) );
  memset( &(pData->Fuzzy1), 0, sizeof(pData->Fuzzy1) );
  memset( &(pData->Fuzzy2), 0, sizeof(pData->Fuzzy2) );
  memset( &(pData->Fuzzy3), 0, sizeof(pData->Fuzzy3) );
  memset( &(pData->NoProps), 0, sizeof(pData->NoProps) );
  memset( &(pData->MTProps), 0, sizeof(pData->MTProps) );
  memset( &(pData->Repl), 0, sizeof(pData->Repl) );
  memset( &(pData->ExactMore), 0, sizeof(pData->ExactMore) );

  // open document and folder memories
  if ( usRC == 0 ) ACI_OpenMemory( pData, pszDocObjName );

  // loop over all segments and get count information
  if ( usRC == 0 )
  {
    ULONG       ulSegNum = 1;
    PTBSEGMENT pSeg = EQFBGetSegW( pDoc, ulSegNum );

    ACI_PrepareSearchKey( pData, pszDocObjName );

    while ( pSeg && (ulSegNum < pDoc->ulMaxSeg) )
    {
      // for all segments with translatable data which are not joined...
      if ( ((pSeg->qStatus == QF_XLATED) || (pSeg->qStatus == QF_TOBE ) || (pSeg->qStatus == QF_ATTR ) || (pSeg->qStatus == QF_CURRENT)) &&  
            !pSeg->SegFlags.Joined )
      {
        MATCHTYPE MatchType = NO_MATCH;
        if (pSeg->qStatus == QF_XLATED )
        { 
          MatchType = EXACTONE_MATCH;
        }
        else
        {
          ACI_GetmatchType( pData, pSeg, &MatchType );
        } /* endif */           

        switch ( MatchType )
        {
          case NO_MATCH         : 
            pData->ulNoProps += pSeg->usSrcWords; 
            pData->ulSegNoProps++; 
            ACI_AddCounts( &(pData->NoProps), 1, pSeg->usSrcWords );
            break;
          case REPL_MATCH       :
          case FUZZYREPL_MATCH  :
          case FUZZY_MATCH      : 
            {
              int iFuzziness = pData->BestProposals[0]->getFuzziness();
              pData->ulFuzzy += pSeg->usSrcWords; 
              pData->ulSegFuzzy++; 
              if( iFuzziness >= (USHORT)(FUZZY_THRESHOLD_0 * 100))
              {
                if ( iFuzziness <= (USHORT)(FUZZY_THRESHOLD_1 * 100))
                {
                  ACI_AddCounts( &(pData->Fuzzy1), 1, pSeg->usSrcWords );
                }
                else if ( iFuzziness <= (USHORT)(FUZZY_THRESHOLD_2 * 100))
                {
                  ACI_AddCounts( &(pData->Fuzzy2), 1, pSeg->usSrcWords );
                }
                else
                {
                  ACI_AddCounts( &(pData->Fuzzy3), 1, pSeg->usSrcWords );
                }  /* end if fuzzyness */
              }
              else
              {
                // treat as no match
                ACI_AddCounts( &(pData->NoProps), 1, pSeg->usSrcWords );
              } /* endif */
            }
            break;
          case EXACTMORE_MATCH  : 
            pData->ulExactMore += pSeg->usSrcWords; 
            pData->ulSegExactMore++; 
            ACI_AddCounts( &(pData->ExactMore), 1, pSeg->usSrcWords );
            break;
          case EXACTONE_MATCH   : 
            pData->ulExactOne += pSeg->usSrcWords; 
            pData->ulSegExactOne++; 
            ACI_AddCounts( &(pData->ExactOne), 1, pSeg->usSrcWords );
            break;
          case EXACTEXACT_MATCH : 
            pData->ulExactExact += pSeg->usSrcWords; 
            pData->ulSegExactExact++; 
            ACI_AddCounts( &(pData->ExactExact), 1, pSeg->usSrcWords );
            break;
          case MACHINE_MATCH    : 
            pData->ulMachineMatch += pSeg->usSrcWords; 
            pData->ulSegMachineMatch++; 
            ACI_AddCounts( &(pData->MTProps), 1, pSeg->usSrcWords );
            break;
          case GLOBMEM_MATCH    : 
            pData->ulExactOne += pSeg->usSrcWords; 
            pData->ulSegExactOne++; 
            ACI_AddCounts( &(pData->ExactMore), 1, pSeg->usSrcWords );
            break;
          default               : 
            pData->ulNoProps += pSeg->usSrcWords; 
            pData->ulSegNoProps++; 
            ACI_AddCounts( &(pData->NoProps), 1, pSeg->usSrcWords );
            break;
        } /* endswitch */           
        pData->ulTotal += pSeg->usSrcWords;
        pData->ulSegTotal++;
       } /* endif */

      ulSegNum++;
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
    } /* endwhile */
  } /* endif */           

  // close all memories
  if ( pData != NULL ) ACI_CloseMemory( pData );

  // update document properties
  if ( usRC == 0 ) ACI_UpdateDocProps( pData, pszDocObjName );

  // cleanup
  if ( pData != NULL )  
  {
     for ( int i=0; i < (int)pData->FoundProposals.size(); i++ ) delete( pData->FoundProposals[i] );
     for ( int i=0; i < (int)pData->BestProposals.size(); i++ ) delete( pData->BestProposals[i] );

    delete( pData );
  }

  return( usRC );
} /* end of function AdjustCountInfo */

//
// helper functions for AdjustCountInfo
//
//

// update document properties with count info
USHORT ACI_UpdateDocProps( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName )
{
  HPROP           hPropDocument;            // handle to document properties
  PPROPDOCUMENT   pPropDocument = NULL;     // pointer to document properties
  ULONG           ulErrorInfo;              // error indicator from PRHA
  USHORT usRC = 0;

  if( (hPropDocument = OpenProperties( pszDocObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo)) != NULL )
  {
    pPropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hPropDocument );
    SetPropAccess( hPropDocument, PROP_ACCESS_WRITE );

    pPropDocument->ulNoProps    = pData->ulNoProps;
    pPropDocument->ulFuzzy      = pData->ulFuzzy;
    pPropDocument->ulExactExact = pData->ulExactExact;
    pPropDocument->ulExactOne   = pData->ulExactOne;
    pPropDocument->ulExactMore  = pData->ulExactMore;
    pPropDocument->ulRepl       = pData->ulRepl;
    pPropDocument->ulFuzzyRepl  = pData->ulFuzzyRepl;
    pPropDocument->ulMachineMatch = pData->ulMachineMatch;
    pPropDocument->ulTotal      = pData->ulTotal;
    pPropDocument->ulSegNoProps    = pData->ulSegNoProps;
    pPropDocument->ulSegFuzzy      = pData->ulSegFuzzy;
    pPropDocument->ulSegExactExact = pData->ulSegExactExact;
    pPropDocument->ulSegExactOne   = pData->ulSegExactOne;
    pPropDocument->ulSegExactMore  = pData->ulSegExactMore;
    pPropDocument->ulSegTotal      = pData->ulSegTotal;
    pPropDocument->ulSegRepl       = pData->ulSegRepl;
    pPropDocument->ulSegFuzzyRepl  = pData->ulSegFuzzyRepl;
    pPropDocument->ulSegMachineMatch = pData->ulSegMachineMatch;

    memcpy( &pPropDocument->Total,      &pData->Total,    sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->ExactExact, &pData->ExactExact, sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->ExactOne,   &pData->ExactOne, sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->Fuzzy1,     &pData->Fuzzy1,   sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->Fuzzy2,     &pData->Fuzzy2,   sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->Fuzzy3,     &pData->Fuzzy3,   sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->NoProps,    &pData->NoProps,  sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->MTProps,    &pData->MTProps,  sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->Repl,       &pData->Repl,     sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->ExactMore,  &pData->ExactMore, sizeof(COUNTSUMS) );

    // save and close properties
    SaveProperties( hPropDocument, &ulErrorInfo );
    ResetPropAccess( hPropDocument, PROP_ACCESS_WRITE);
    CloseProperties( hPropDocument, PROP_FILE, &ulErrorInfo);
  } /* endif */
  return( usRC );
} /* end of function ACI_UpdateDocProps */


// open the document memory and the list of folder read-only memories
USHORT ACI_OpenMemory( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName )
{
  USHORT      usRc = 0;
  HPROP       hProp;                   // handle to document properties
  ULONG       ulErrorInfo;             // error indicator from PRHA
  int         iNumOfMems = 0;
  BOOL        fOK = TRUE;

  DocQueryInfo2( pszDocObjName, pData->szMemNames[0], NULL, NULL, NULL, NULL, NULL, NULL, TRUE );
  iNumOfMems++;

  // prepare folder object name using pData->szMemNames[1] as name buffer
  {
    CHAR szDrive[3];
    strcpy( pData->szMemNames[1], pszDocObjName );
    UtlSplitFnameFromPath( pData->szMemNames[1] );
    UtlQueryString( QST_PRIMARYDRIVE, szDrive, sizeof(szDrive) );
    pData->szMemNames[1][0] = szDrive[0];
  }

  // get list of folder read-only memories
  hProp = OpenProperties( pData->szMemNames[1], NULL, PROP_ACCESS_READ, &ulErrorInfo );
  if ( hProp )
  {
    PPROPFOLDER pProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp );

    // add folder R/O memories to the list
    if ( pProp->aLongMemTbl[0][0] != EOS )
    {
      int i = 0;
      while ( (i < MAX_NUM_OF_READONLY_MDB) && (pProp->aLongMemTbl[i][0] != EOS) )
      {
        strcpy( pData->szMemNames[iNumOfMems], pProp->aLongMemTbl[i] );
        iNumOfMems++;
        i++;
      } /*endwhile */
    }
    else
    {
      PSZ pszToken;

      //get first R/O memory from list
      strcpy( pData->szBuffer, pProp->MemTbl );
      pszToken = strtok( pData->szBuffer, X15_STR );
      while ( (pszToken != NULL) && fOK )
      {
        strcpy( pData->szMemNames[iNumOfMems], pszToken );
        iNumOfMems++;
        pszToken = strtok( NULL, X15_STR );
      } /* endwhile */
    } /* endif */
    CloseProperties( hProp, PROP_QUIT, &ulErrorInfo );
  } /* endif */

  // open the memories
  if ( !usRc && iNumOfMems )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    int i = 0;
    while ( !usRc && (i < iNumOfMems) )
    {
      int iRC = 0;
      OtmMemory *pMem = pFactory->openMemory( NULL, pData->szMemNames[i], READONLYACCESS, &iRC );
      if ( pMem != NULL )
      {
        pData->MemoryDBs.push_back( pMem );
      } /* endif */        
      usRc = (USHORT)iRC;
      i++;
    } /*endwhile */

    // prepare proposal vectors
    for ( int i = 0; i < 3; i++ ) pData->FoundProposals.push_back( new(OtmProposal) );
    for ( int i = 0; i < 3; i++ ) pData->BestProposals.push_back( new(OtmProposal) );

  } /* endif */

  return( usRc );
} /* end of function ACI_OpenMemory */

//  close the memory DBs associated with the folder
USHORT ACI_CloseMemory( PADJUSTCOUNTINFODATA pData )
{
  USHORT       usRc = 0;
  int i = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  for (i = 0; i < (int)pData->MemoryDBs.size(); i++ )
  {
    pFactory->closeMemory( pData->MemoryDBs[i] );
  }
  pData->MemoryDBs.clear(); 

  return( usRc );
} /* end of function ACI_CloseMemory */

// get document info and store it in search key
USHORT ACI_PrepareSearchKey( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName )
{
  DocQueryInfo2( pszDocObjName,
                  NULL,                        // memory
                  pData->szTagTable,           // document format
                  pData->szSourceLanguage,     // source language
                  pData->szTargetLanguage,     // target language
                  pData->szLongName,           // LongName,
                  NULL,                        // Alias
                  NULL,                        // editor
                  TRUE );

  pData->SearchKey.clear();
  pData->SearchKey.setMarkup( pData->szTagTable ); 
  pData->SearchKey.setSourceLanguage( pData->szSourceLanguage ); 
  pData->SearchKey.setTargetLanguage( pData->szTargetLanguage ); 
  pData->SearchKey.setDocName( pData->szLongName ); 
  pData->SearchKey.setDocShortName( UtlGetFnameFromPath( pszDocObjName ) ); 

  return( 0 );
}

// get the available proposals for the segment and evaluate the type of matches
USHORT ACI_GetmatchType( PADJUSTCOUNTINFODATA pData, PTBSEGMENT pSeg, PMATCHTYPE pMatchType )
{
  USHORT       usRC = 0;
  int i = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  *pMatchType = NO_MATCH;

  pData->SearchKey.setSource( pSeg->pDataW );

  OtmProposal::clearAllProposals( pData->BestProposals );
  while ( !usRC && (i < (int)pData->MemoryDBs.size()) && (pData->MemoryDBs[i] != NULL) )
  {
    OtmProposal::clearAllProposals( pData->FoundProposals );
    pData->MemoryDBs[i]->searchProposal( pData->SearchKey, pData->FoundProposals, GET_EXACT );
    pFactory->copyBestMatches( pData->FoundProposals, pData->BestProposals, 3 );
    i++;
  } /* endwhile */
  
  // evaluate match type
  int iProposals = OtmProposal::getNumOfProposals( pData->BestProposals );
  if ( iProposals == 0  )
  {
    *pMatchType = NO_MATCH;
  }
  else if ( pData->BestProposals[0]->getType() == OtmProposal::eptMachine )
  {
    *pMatchType = MACHINE_MATCH;
  }
  else if ( (pData->BestProposals[0]->getType() == OtmProposal::eptGlobalMemory) ||
            (pData->BestProposals[0]->getType() == OtmProposal::eptGlobalMemoryStar) )
  {
    *pMatchType = GLOBMEM_MATCH;
  }
  else if ( !pData->BestProposals[0]->isExactMatch() )
  {
    *pMatchType = FUZZY_MATCH;
  }
  else if ( iProposals == 1 ) 
  {
    // only one exact proposal
    if ( pData->BestProposals[0]->getMatchType() == OtmProposal::emtExactExact )
    {
      *pMatchType = EXACTEXACT_MATCH;
    }
    else
    {
      *pMatchType = EXACTONE_MATCH;
    } /* endif */       
  }
  else
  {
    // several exact proposals
    if ( pData->BestProposals[0]->getMatchType() == OtmProposal::emtExactExact )
    {
      *pMatchType = EXACTEXACT_MATCH;
    }
    else
    {
      *pMatchType = EXACTMORE_MATCH;
    } /* endif */       
  } /* endif */     
  return( usRC );
} /* end of function ACI_GetmatchType */

USHORT ACI_AddCounts( PCOUNTSUMS pSum, LONG lSegs, LONG lWords )
{
  LONG lAbsWords = abs( lWords );
  if ( lAbsWords < SIMPLE_SENT_BOUND )
  {
    pSum->ulSimpleSegs  += lSegs;
    pSum->ulSimpleWords += lWords;
  }
  else if ( lAbsWords < MEDIUM_SENT_BOUND )
  {
    pSum->ulMediumSegs  += lSegs;
    pSum->ulMediumWords += lWords;
  }
  else
  {
    pSum->ulComplexSegs  += lSegs;
    pSum->ulComplexWords += lWords;
  } /* endif */
  return( NO_ERROR );
} /* end of function TAAddCounts */
