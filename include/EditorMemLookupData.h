/*! \file
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _EDITORMEMLOOKUPDATA_H_
#define _EDITORMEMLOOKUPDATA_H_

#include <core/pluginmanager/OtmMemory.h>
#include <vector>
#include <string>

// data area for memory lookup related data and objects
typedef struct _TPMEMLOOKUPDATA
{
  std::vector<OtmMemory *>MemoryList;            // list of memories for the lookup
  std::vector<OtmProposal *>FoundProposals;      // list of found proposals
  std::vector<OtmProposal *>BestProposals;       // list of best proposals founds
  OtmProposal SearchKey;                         // proposal used as search key
  CHAR_W szSource[MAX_SEGMENT_SIZE+1];           // buffer for proposal source text
  CHAR_W szTarget[MAX_SEGMENT_SIZE+1];           // buffer for proposal target text
  char szNameBuffer[MAX_LONGFILESPEC+1];         // buffer for document names and other stuff
  CHAR_W szDocSource[MAX_SEGMENT_SIZE+1];        // buffer for source segment text
} TPMEMLOOKUPDATA, *PTPMEMLOOKUPDATA;

#endif // #ifndef _EDITORMEMLOOKUPDATA_H_