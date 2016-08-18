//+----------------------------------------------------------------------------+
//|EQFTADIT.H   Header file for IBMIDDOC/IBMDITA processing functions          |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
typedef enum _DITAPROCESSINGMODES
{
  DITAWRITETOMEM,
  DITAPREPAREPROPOSAL
} DITAPROCESSINGMODES;

__declspec(dllexport)
BOOL DITASpecialProcessingRequired
(
  PSZ         pszDocMarkup,                      // markup of document
  PSZ         pszMemMarkup,                      // markup of memory proposal
  DITAPROCESSINGMODES mode                       // type of special processing
);


__declspec(dllexport)
BOOL DITAPreprocessProposals
(
  PSZ_W       pszSource,                         // source of memory proposal
  PSZ_W       pszTarget,                         // target of memory proposal
  PVOID       pvReplaceList                      // replacement list pointer or NULL
);

__declspec(dllexport)
BOOL DITAProcessProposal
(
  PSZ_W       pszDocSource,                      // source of document segment
  PSZ_W       pszSource,                         // source of memory proposal
  PSZ_W       pszTarget,                         // target of memory proposal
  PSZ         pszMarkup,                         // markup of memory proposal
  ULONG       ulOemCP                            // OEM CP for proposal source
);

__declspec(dllexport)
BOOL DITALoadReplacementList
(
  PSZ         pszReplaceList,                    // fully qualified replacement list name
  PVOID       *ppvReplaceList                    // ptr to callers replacement list pointer
);

__declspec(dllexport)
BOOL DITAFreeReplacementList
(
  PVOID       *ppvReplaceList                    // ptr to callers replacement list pointer
);


