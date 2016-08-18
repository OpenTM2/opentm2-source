/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _ANALYSISMEMLOOKUPDATA_H_
#define _ANALYSISMEMLOOKUPDATA_H_

#include <core/pluginmanager/OtmMemory.h>
#include <vector>
#include <string>

/*! \brief Data area for the memory lookup in the analysis
 * 
 */
typedef struct _ANALYSISMEMLOOKUPDATA
{
  wchar_t szSegmentBuffer[MAX_SEGMENT_SIZE+1];    // buffer for segment data
  char szNameBuffer[MAX_LONGFILESPEC];           // buffer for document names and other
  OtmProposal      SearchKey;                    // search key for memory lookup
  std::vector<OtmMemory *> InputMemoryDBs;       // list of input memory databases
  std::vector<OtmMemory *> ROMemoryDBs;          // list of read only memory databases
  std::vector<OtmProposal *> FoundProposals;     // list of found proposals
  std::vector<OtmProposal *> BestProposals;      // list of best proposals
  std::vector<OtmMemory *> SearchMemoryDBs;      // dynamically created list of search memory databases
  OtmMemory *pDocMem;                            // document memory database handle
  wchar_t szAddInfoBuffer[MAX_SEGMENT_SIZE+1];   // buffer for additional segment information
  std::vector<OtmProposal *>DupSearchProposals;  // list of found proposals for duplicate search
  OtmMemory *pDupMem;                            // memory for duplicate search
  OtmMemory *pRedundCountMem;                    // memory for redundancy counting
} ANALYSISMEMLOOKUPDATA, *PANALYSISMEMLOOKUPDATA;


#endif // #ifndef _ANALYSISMEMLOOKUPDATA_H_
 