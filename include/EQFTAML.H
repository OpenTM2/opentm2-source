/*! match list processing functions
	Copyright (c) 1990-2017, International Business Machines Corporation and others. All rights reserved.
*/

__declspec(dllexport)
int TAMatchListInit
(
  PTAINPUT    pTAIn
);


__declspec(dllexport)
int TAMatchListTerminate
(
  PTAINPUT    pTAIn
);

__declspec(dllexport)
int TAMatchListProcess
(
  PTAINPUT    pTAIn,
  PSZ_W       pszSegment,
  PSZ_W       pszProposal,
  MATCHTYPE   MatchType,
  USHORT      usFuzzyness
);

__declspec(dllexport)
int TABalanceList
(
  PSZ         pszFolder,                         // folder object name
  PSZ         pszDocs,                           // ptr to list of documents
  USHORT      usFiles                            // number of source files
);

