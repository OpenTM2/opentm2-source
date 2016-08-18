/*------------------------------------------------------------------------+
|  EQFTALP2.C - Implementation file for List Processing                   |
+-------------------------------------------------------------------------+
|  Copyright Notice:                                                      |
|                                                                         |
|      Copyright (C) 1990-2013, International Business Machines           |
|      Corporation and others. All rights reserved                        |    
+-------------------------------------------------------------------------+
|  Author       : G. Queck                                                |
+-------------------------------------------------------------------------+
|  Description:                                                           |
|      A set of functions to build term (word) frequency tables during    |
|      text analysis.                                                     |
+-------------------------------------------------------------------------+
|  Include files:                                                         |
|       EQF.H          EQFTALP1.H      EQFTAFUN.H                         |
|       EQFDASD.H      EQFTALP0.H                                         |
|       EFZLTOL3.H     EQFTALP2.H                                         |
+-------------------------------------------------------------------------+
|  Entry Points:                                                          |
|      LPNAdd      LPFAdd       LPTLOutput      LPLoadTagTable            |
|      LPNCreate   LPFCreate    LPContextUpd                              |
|      LPNCompare  LPFCompare   LPTermSave                                |
|      LPNFree     LPFFree      LPTermFree                                |
+-------------------------------------------------------------------------+
|  Compile Notes                                                          |
|    Compile this file with the LOCAL_TEST symbol defined to create a     |
|    program to test this module.  When LOCAL_TEST is defined calls to    |
|    TAGiveCntrl are not called and a main() function is compiled.        |
+-------------------------------------------------------------------------+
*/
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file

#include "OtmDictionaryIF.H"
//#include "OtmProposal.h"
#include "EQFTAI.H"               // Analysis private include file
#include <time.h>

/**********************************************************************/
/*  global variables                                                  */
/**********************************************************************/
CHAR_W  szEndChar[2];                   // global string for end tag delimiter

#if defined(LOG_ALLOCS)
  LONG lAllocs    = 0L;                // number of allocations
  LONG lAllocSize = 0L;                // overall size of allocations
#endif

/*อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ                   Prototypes for Private Functions                         บ
ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ*/

/*---------------------------------------------------------------------------+
|  Name:         LocalTAGiveCntrl                                            |
|  Purpose:      Calls TAGiveCntrl & handles fKill/fTerminate results        |
|  Parameters:   1. PLPPROCDATA - ptr to all data required                   |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LocalTAGiveCntrl(pProcData);                          |
+---------------------------------------------------------------------------*/
static BOOL LocalTAGiveCntrl(PLPPROCDATA pProcData);

/*---------------------------------------------------------------------------+
|  Name:         LPOutTerm                                                   |
|  Purpose:      Output a single term to list file. (New or found terms)     |
|  Parameters:   1. PTANODE - pointer to an LPFENTRY or LPNENTRY node          |
|                2. PVOID - pr to a LPPROCDATA containing pointers to hwnd,  |
|                               TAINSTDATA ...                               |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:     This function is passed as a parameter to ListScan          |
|                fTerminate may be set (force shutdown request)              |
|  Samples:      fOK = LPOutTerm(&pnewnode, (PVOID) pprocData);              |
+---------------------------------------------------------------------------*/
static EQF_BOOL LPOutTerm(PTANODE pNode, PVOID pLPProcData);

/*---------------------------------------------------------------------------+
|  Name:         LPOutTermData                                               |
|  Purpose:      Output the data of a "new" or a "found" term (TEXT)         |
|  Parameters:   1. PLPPROCDATA - data for TAGiveCntrl                       |
|                2. PTANODE - pointer to "new" or "found" term node            |
|                3. PLPDATA - pointer local data                             |
|                4. PBOOL - Pointer to Terminate boolean variable            |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutTermData(pProcData, pNewNode, pLPData,           |
|                                    &fTerminate);                           |
+---------------------------------------------------------------------------*/
static BOOL LPOutTermData(PLPPROCDATA pProcData, PTANODE pNode, PLPDATA pLPData,
                          PBOOL pfTerminate);

/*---------------------------------------------------------------------------+
|  Name:         LPOutContext                                                |
|  Purpose:      Output context data or the reference to it                  |
|  Parameters:   1. PLPSEG - ptr to the context data to be output            |
|                2. PLPDATA - ptr to List Processing data record             |
|                3. PBOOL - Pointer to fTerminate variable                   |
|  Returns:      BOOL - True if successful, else FALSE (output errors)       |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutContext(pContext, pLPData, &fTerminate);         |
+---------------------------------------------------------------------------*/
static BOOL LPOutContext(PLPSEG pContext, PLPDATA pLPData, PBOOL pfTerminate);

/*---------------------------------------------------------------------------+
|  Name:         LPGetContext                                                |
|  Purpose:      Read from file the data for a context                       |
|  Parameters:   1. PLPSEG - ptr to the context data                         |
|                2. PSZ - pointer to the buffer area                         |
|                3. PLPFCB - pointer to list of open files CBs               |
|                4. PBOOL - pointer to the termination variable              |
|  Returns:      BOOL - True if successful, else FALSE                       |
|  Comments:                                                                 |
|  Samples:      fOK = LPGetContext(pLPSeg, szBuffer, pSourcePath,           |
|                                   pLstFCB, &fTerminate);                   |
+---------------------------------------------------------------------------*/
static BOOL LPGetContext(PLPSEG pLPSeg, PSZ_W pszBuffer, PSZ pSourcePath,
                         PLPFCB *ppLPFCB, PBOOL pfTerminate);

/*---------------------------------------------------------------------------+
|  Name:         LPContextUpd                                                |
|  Purpose:      If context not in list then add it                          |
|  Parameters:   1. PTAINPUT - ptr to TAINPUT struct (for fTerminate)        |
|                2. PLPCTXT * - ptr to ptr to the list of contexts           |
|                3. PLPSEG a context structure                               |
|  Returns:      BOOL - True if Error free                                   |
|  Comments:                                                                 |
|  Samples:      fOK = LPContextUpd(pTAInput, &pCntxtLst, pContext);         |
+---------------------------------------------------------------------------*/
static BOOL LPContextUpd(PTAINPUT pTAInput, PLPCTXT *ppCntxtLst,
                         PLPSEG pContext);

/*---------------------------------------------------------------------------+
|  Name:         LPOutContextReferences                                      |
|  Purpose:      Output a list of context references                         |
|  Parameters:   1. PLPDATA - Ptr to LP data area                            |
|                2. PBOOL - Ptr to termination variable                      |
|  Returns:      BOOL - TRUE if successful, FALSE otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutContextReferences(PlpData, pfTerminate);         |
+---------------------------------------------------------------------------*/
static BOOL LPOutContextReferences(PLPDATA pLPData, PBOOL pfTerminate);

/*---------------------------------------------------------------------------+
|  Name:         LPNewLine                                                   |
|  Purpose:      Print a newline to an already open output file              |
|  Parameters:   1. PBUFCB - ptr to an IO control block                    |
|  Returns:      BOOL - Return code from UtlBufWrite                         |
|  Comments:                                                                 |
|  Samples:      fOK = LPNewLine(pIOCB);                                     |
+---------------------------------------------------------------------------*/
// No longer needed - newline is in tag table ENDDELIMITER
static BOOL LPNewLine(PBUFCB pIOCB);

/*---------------------------------------------------------------------------+
|  Name:         LPIndent                                                    |
|  Purpose:      Print a number of spaces - intended for indenting           |
|  Parameters:   1. PBUFCB - ptr to an IO control block                    |
|                2. USHORT - the number of spaces to indent                  |
|  Returns:      BOOL - Return code from UtlBufWrite                         |
|  Comments:                                                                 |
|  Samples:      fOK = LPIndent(pIOCB, 2);                                   |
+---------------------------------------------------------------------------*/
static BOOL LPIndent(PBUFCB pIOCB, USHORT uSpaces);

/*---------------------------------------------------------------------------+
|  Name:         LPOutStatement                                              |
|  Purpose:      Output an SGML statement (starttag - value/attrs - endtag)  |
|  Parameters:   1. PBUFCB - Ptr to the output file IO control block       |
|                2. PTAGTABLE - the tag table to extact the tags from        |
|                3. LISTTAGS - the start tag index                           |
|                4. LISTTAGS - the end tag index                             |
|                5. PSZ - ptr to the tag value                               |
|                6. USHORT - number of attributes                            |
|                7. PSZ * - Ptr to ptr to list of attributes                 |
|                8. PBOOL - ptr to termination variable                      |
|                9. USHORT - the no. of characters to indent                 |
|  Returns:      BOOL - True if Error free                                   |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutStatement(pCBList, pLPData->pTagDef,             |
|                               CONTEXT_TAG, ECONTEXT_TAG, szWorkContext,    |
|                               0, NULL, pfTerminate, 2);                    |
+---------------------------------------------------------------------------*/
static BOOL LPOutStatement(
                     PBUFCB pIOCB,
                     PTAGTABLE pTagTable,
                     LISTTAGS openTag,
                     LISTTAGS closeTag,
                     PSZ_W    pszTagValue,
                     USHORT   usAttributeCount,
                     PSZ_W    *ppAttributeList,
                     PBOOL    pfTerminate,
                     USHORT   usIndent,
                     PSZ_W    pTagNamesW);


/*---------------------------------------------------------------------------+
|  Name:         LPOutTag                                                    |
|  Purpose:      Output the opening sequence of a tag with attributes/values |
|  Parameters:   1. PBUFCB - Ptr to the output file IO control block       |
|                2. PTAGTABLE - the tag table to extact the tags from        |
|                3. LISTTAGS - the index of the tag to output                |
|                4. USHORT - number of attributes                            |
|                5. PSZ * - Ptr to ptr to list of attributes                 |
|                6. PBOOL - ptr to termination variable                      |
|                7. USHORT - Indent level                                    |
|  Returns:      BOOL - True if Error free                                   |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutTag(pLPData->pCBList, pLPData->pTagDef,          |
|                  CONTEXT_TAG, 1, &pAttributeList, pfTerminate, 2);         |
+---------------------------------------------------------------------------*/
static BOOL LPOutTag(PBUFCB pIOCB,
                     PTAGTABLE pTagTable,
                     LISTTAGS tag,
                     USHORT   usAttributeCount,
                     PSZ_W     *ppAttributeList,
                     PBOOL    pfTerminate,
                     USHORT   usIndent,
                     PSZ_W    pTagNamesW);


/*---------------------------------------------------------------------------+
|  Name:         LPOutTagList                                                |
|  Purpose:      Output a list of tags (with no attributes) where the values |
|                are in a string buffer format                               |
|  Parameters:   1. PBUFCB - ptr to an IO control block                    |
|                2. PTAGTABLE - the tag table to extact the tags from        |
|                3. LISTTAGS - index of the start tag for each entry         |
|                4. LISTTAGS - index of the end tag for each entry           |
|                5. PBYTE - A reference point for calculating string offsets |
|                6. STRINGBUFFER - offset to a list of string values         |
|                7. PBOOL - ptr to the fTerminate variable                   |
|  Returns:      BOOL - Return code LPOutTag                                 |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutTagList(pIOCB, pLPData->pTagDef,                 |
|                                 DICTNAME_TAG, EDICTNAME_TAG,               |
|                                 pByte, pTAInput->stInputDict, pfTerminate);|
+---------------------------------------------------------------------------*/
static BOOL LPOutTagList(PBUFCB pIOCB,
                         PTAGTABLE pTagTable,
                         LISTTAGS openTag,
                         LISTTAGS closeTag,
                         PBYTE pReference,
                         STRINGBUFFER stValues,
                         PBOOL pfTerminate,
                         PSZ_W pTagNamesW,
                         PSZ_W pConvBuf,
                         ULONG  ulSrcOemCP);

/*---------------------------------------------------------------------------+
|  Name:         LPOutHeader                                                 |
|  Purpose:      Output a header (in SGML format) for a NTL or FTL term      |
|  Parameters:   1. PBUFCB - ptr to an IO control block                    |
|                2. PTAINPUT - ptr to TAINPUT for header info                |
|                3. PLPDATA - reference point for calculating string offsets |
|  Returns:      BOOL - Return TRUE if successful, FALSE otherwise           |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutHeader(pIOCB, pTAInput, pLPDAta);                |
+---------------------------------------------------------------------------*/
static BOOL LPOutHeader(PBUFCB pIOCB, PTAINPUT pTAInput, PLPDATA pLPData);

/*---------------------------------------------------------------------------+
|  Name:         LPTagTableExists                                            |
|  Purpose:      Check if the Tag Table for list processing exists           |
|  Parameters:   1. PLPDATA - pointer local data                             |
|  Returns:      BOOL - True if the table exists, False otherwise            |
|  Comments:                                                                 |
|  Samples:      fOK = LPTagTableExists(pLPData);                            |
+---------------------------------------------------------------------------*/
static BOOL LPTagTableExists(PLPDATA pLPData);

/*---------------------------------------------------------------------------+
|  Name:         LPGetTagString                                              |
|  Purpose:      Get a tag string from the LP tag table using the given tag  |
|  Parameters:   1. LISTTAGS - the tag to use as an index                    |
|                2. PTAGTABLE - Ptr to the tag table                         |
|  Returns:      PSZ - ptr to the tag string in the tagtable                 |
|  Comments:                                                                 |
|  Samples:      pszTag = LPGetTagString(CONTEXT_TAG, pLPData->pTagDef);     |
+---------------------------------------------------------------------------*/
static PSZ_W LPGetTagString(LISTTAGS tagIndex, PTAGTABLE pTagTable, PSZ_W pTagNamesW);

/*---------------------------------------------------------------------------+
|  Name:         LPGetTagEndDelim                                            |
|  Purpose:      Get a tag end delimiter from the LP tag table               |
|  Parameters:   1. LISTTAGS - the tag to use as an index                    |
|                2. PTAGTABLE - Ptr to the tag table                         |
|  Returns:      PSZ - ptr to the tag string in the tagtable                 |
|  Comments:                                                                 |
|  Samples:      pszTag = LPGetTagEndDelim(CONTEXT_TAG, pLPData->pTagDef);   |
+---------------------------------------------------------------------------*/
static PSZ_W LPGetTagEndDelim(LISTTAGS tagIndex, PTAGTABLE pTagTable,
                              PSZ_W pTagNamesW, ULONG ulCP);

/*---------------------------------------------------------------------------+
|  Name:         LPGetAttributeString                                        |
|  Purpose:      Get an attribute string from the LP tag table using the     |
|                given attribute index                                       |
|  Parameters:   1. LISTTAGS - the tag to use as an index                    |
|                2. PTAGTABLE - Ptr to the tag table                         |
|  Returns:      PSZ - ptr to the tag string in the tagtable                 |
|  Comments:                                                                 |
|  Samples:      pszAttr = LPGetAttributeString(ID_ATTR,                     |
|                                               pLPData->pTagDef);           |
+---------------------------------------------------------------------------*/
static PSZ LPGetAttributeString(LISTTAGS attributeIndex, PTAGTABLE pTagTable);

/*อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ                           Newest Functions                                 บ
ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ*/

/*---------------------------------------------------------------------------+
|  Name:         LPContextRefSearch                                          |
|  Purpose:      Search for a context ref in the ContextRef tree             |
|  Parameters:   1. PLPDATA - Ptr to LP local data                           |
|                2. PLPSEG - pt to segment descriptor                        |
|  Returns:      PLPCTXTREF - ptr to the found reference (or NULL)           |
|  Comments:                                                                 |
|  Samples:      pContextRef = LPContextRefSearch(pLPData, pContext);        |
+---------------------------------------------------------------------------*/
PLPCTXTREF LPContextRefSearch(PLPDATA pLPData, PLPSEG pContext);

/*---------------------------------------------------------------------------+
|  Name:         LPContextRefInsert                                          |
|  Purpose:      Insert a node into the context ref. tree                    |
|  Parameters:   1. PLPDATA - Ptr to LP local data                           |
|                2. PLPSEG - pt to segment descriptor                        |
|  Returns:      BOOL - TRUE if successful, FALSE otherwise                  |
|  Comments:     Assumes the node is not already in the tree                 |
|  Samples:      fOK = LPContextRefInsert(pLPData, pContext, pfTerminate);   |
+---------------------------------------------------------------------------*/
BOOL LPContextRefInsert(PLPDATA pLPData, PLPSEG pLPSeg, PBOOL pfTerminate);

/*---------------------------------------------------------------------------+
|  Name:         LPContextRefCompare                                         |
|  Purpose:      Compare two ContextRef node structures                      |
|  Parameters:   1. PLPCTXTREF - pointer to the first node                   |
|                2. PLPCTXTREF - pointer to the second node                  |
|  Returns:      SHORT - -1: node1<node2; 0: node1==node2; 1:node1>node2     |
|  Comments:                                                                 |
|  Samples:      s = LPContextRefCompare(&node1, &node2);                    |
+---------------------------------------------------------------------------*/
SHORT LPContextRefCompare(PLPCTXTREF pNewNode1, PLPCTXTREF pNewNode2);

/*---------------------------------------------------------------------------+
|  Name:         LPContextRefFree                                            |
|  Purpose:      Free a node of the ContextRefs tree                         |
|  Parameters:   1. PLPCTXTREF - pointer to the node to be freed             |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPContextRefFree(&node);                              |
+---------------------------------------------------------------------------*/
BOOL LPContextRefFree(PLPCTXTREF pNode);

/*---------------------------------------------------------------------------+
|  Name:         LPInitContextReferences                                     |
|  Purpose:      Initialises the context references tree                     |
|  Parameters:   1. PLPDATA - Pointer to LPDATA                              |
|  Returns:      BOOL - true if successful                                   |
|  Comments:                                                                 |
|  Samples:      fOK = LPInitContextReferences(pLPData);                     |
+---------------------------------------------------------------------------*/
BOOL LPInitContextReferences(PLPDATA pLPData);

/*อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ                 Implementation of Exported Functions                       บ
ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ*/

/*---------------------------------------------------------------------------+
|  Name:         LPNAdd                                                      |
|  Purpose:      Maintain entry for a word in the "New" table.               |
|  Parameters:   1. PTAINPUT - pt to TAINPUT struct                          |
|                2. PLPDATA - Pointer to local data                          |
|                3. BOOL * - set to true if term is found in table           |
|                4. PSZ - The term to be updated                             |
|                5. PLPSEG - pt to segment descriptor                        |
|                6. PUSHORT - Ptr to the frequency (if found)                |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPNAdd(pTAinput, &LPData, &Found, WordPtr, &SegDescr, |
|                             &usFrequency);                                 |
+---------------------------------------------------------------------------*/
BOOL LPNAdd(PTAINPUT pTAInput, PLPDATA pLPData, BOOL *pfFound,
            PSZ_W pszTerm, PLPSEG pLPSeg, PUSHORT pusReturnFrequency)
{
    LPNENTRY  searchNode;
    PLPNENTRY pNodeFound;
    BOOL fOK = TRUE;

    // Build NTL work node
    searchNode.pszTerm = pszTerm;

    // Call List Search with work node, Table pointer
    pNodeFound = (PLPNENTRY) ListSearch(pLPData->pNTL, (PTANODE) &searchNode);

    // Set fFound according to the result
    *pfFound = (pNodeFound != NULL);

    // if Found then
    if (*pfFound)
      {
      pNodeFound->usFrequency++;

      // The new frequency is returned
      *pusReturnFrequency = pNodeFound->usFrequency;

      // if fNTLContext is TRUE (contexts required)
      if (pTAInput->fNTLcontext)
         // Call LPContextUpd with: first Context pt., pTAINPUT & pLPSEG
         fOK = LPContextUpd(pTAInput, &(pNodeFound->pContext), pLPSeg);

      }

    return fOK;
}


/*---------------------------------------------------------------------------+
|  Name:         LPNCreate                                                   |
|  Purpose:      Create a new entry in the "New" table                       |
|  Parameters:   1. PTAINPUT - TAINPUT struct pointer                        |
|                2. PLPDATA - Pointer to local data                          |
|                3. PSZ - The term to be stored                              |
|                4. PLPSEG - pt to segment descriptor                        |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPNCreate(pTAInput, pLPData, WordPtr, &SegDescr);     |
+---------------------------------------------------------------------------*/
BOOL LPNCreate(PTAINPUT pTAInput, PLPDATA pLPData, PSZ_W pszTerm, PLPSEG pLPSeg)
{
    PLPNENTRY pNewNode;
    BOOL     fOK;

    // Allocate memory for node
    fOK = UtlAlloc( (PVOID *) &pNewNode, 0L,(LONG) sizeof(LPNENTRY), ERROR_STORAGE);
#if defined(LOG_ALLOCS)
    lAllocs++;
    lAllocSize += sizeof(LPNENTRY);
#endif
    if (!fOK)
      pTAInput->pInD->fTerminate = TRUE;

    if (fOK)
      // Allocate memory for word & save it
      fOK = LPTermSave(pTAInput, pLPData, (PSZ_W *) &(pNewNode->pszTerm), pszTerm);

    if (fOK)
      {
      // Fill fields of new node (pointer to word done above)
      // Dictionary handle not required at this stage
      pNewNode->usFrequency = 1;
      pNewNode->pContext = NULL;

      // if fNTLContext is TRUE (contexts required)
      if (pTAInput->fNTLcontext)
         // call LPContextUpd with first context pt., pTAINPUT & pLPSEG
         fOK = LPContextUpd(pTAInput, &(pNewNode->pContext), pLPSeg);
      }

    if (fOK)
       // Call ListInsert
       fOK = ListInsert(pLPData->pNTL,
                        (PTANODE *) &(pLPData->pNTL->root),
                        (PTANODE) pNewNode);

    return fOK;
}

/*---------------------------------------------------------------------------+
|  Name:         LPNCompare                                                  |
|  Purpose:      Compare two "New" node structures                           |
|  Parameters:   1. PLPNENTRY - pointer to the first node                    |
|                2. PLPNENTRY - pointer to the second node                   |
|  Returns:      SHORT - -1: node1<node2; 0: node1==node2; 1:node1>node2     |
|  Comments:     Future language oriented function will substitute stricmp   |
|  Samples:      s = LPNCompare(&node1, &node2);                             |
+---------------------------------------------------------------------------*/
SHORT LPNCompare(PLPNENTRY pNewNode1, PLPNENTRY pNewNode2)
{
     return (SHORT)(UTF16stricmp(pNewNode1->pszTerm, pNewNode2->pszTerm));

}

/*---------------------------------------------------------------------------+
|  Name:         LPNFree                                                     |
|  Purpose:      Free a node of the "New" terms list (and associated memory) |
|  Parameters:   1. PLPNENTRY - pointer to the node to be freed              |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPNFree(&node);                                       |
+---------------------------------------------------------------------------*/
BOOL LPNFree(PLPNENTRY pNode)
{
    BOOL fOK = TRUE;
    PLPCTXT pNext;

    // Free list of contexts
    while (pNode->pContext != NULL)
      {
      // Store next pointer in list
      pNext = pNode->pContext->pNext;

      //   free element
      fOK = UtlAlloc( (PVOID *) &(pNode->pContext), 0L, 0L, NOMSG) && fOK;
#if defined(LOG_ALLOCS)
        lAllocs--;
#endif

      //   retrieve next pointer in list
      pNode->pContext = pNext;
      }

    // free node memory (but not it's term)
    fOK = UtlAlloc( (PVOID *) &pNode, 0L, 0L, NOMSG) && fOK;
#if defined(LOG_ALLOCS)
        lAllocs--;
#endif

    // We do not check the value of fOK here. Deallocation should continue
    // even if errors occur.

    return fOK;
}

/*---------------------------------------------------------------------------+
|  Name:         LPFAdd                                                      |
|  Purpose:      Maintain entry for a word in the "Found" table.             |
|  Parameters:   1. PTAINPUT - pt to TAINPUT struct                          |
|                2. PLPDATA - Pointer to local data                          |
|                3. BOOL * - set to true if term is found in table           |
|                4. PSZ - The term to be updated                             |
|                5. PLPSEG - pt to segment descriptor                        |
|                6. PUSHORT - Ptr to the frequency (if found)                |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:     A new element is allocated and added to the table           |
|  Samples:      fOK = LPFAdd(pTAInput, &LPData, &Found, WordPtr, &SegDescr, |
|                             &usFrequency);                                 |
+---------------------------------------------------------------------------*/
BOOL LPFAdd(PTAINPUT pTAInput, PLPDATA pLPData, BOOL *pfFound,
            PSZ_W pszTerm, PLPSEG pLPSeg, PUSHORT pusReturnFrequency)
{
    LPFENTRY  searchNode;
    PLPFENTRY pNodeFound;
    BOOL fOK = TRUE;

    // Build FTL work node
    searchNode.pszTerm = pszTerm;

    // Call List Search with work node, Table pointer
    pNodeFound = (PLPFENTRY) ListSearch(pLPData->pFTL, (PTANODE) &searchNode);

    // Set fFound according to the result
    *pfFound = (pNodeFound != NULL);

    // if Found then
    if (*pfFound)
      {
      pNodeFound->usFrequency++;

      // The new frequency is returned
      *pusReturnFrequency = pNodeFound->usFrequency;

      // if fFTLContext is TRUE (contexts required)
      if (pTAInput->fFTLcontext)
         // Call LPContextUpd with: first Context pt., pTAINPUT & pLPSEG
         fOK = LPContextUpd(pTAInput, &(pNodeFound->pContext), pLPSeg);
      }

    return fOK;
}


/*---------------------------------------------------------------------------+
|  Name:         LPFCreate                                                   |
|  Purpose:      Create a new entry in the "Found" table                     |
|  Parameters:   1. PTAINPUT - TAINPUT struct                                |
|                2. PLPDATA - Pointer to local data                          |
|                3. PSZ - The term to be stored                              |
|                4. PLPSEG - pt to segment descriptor                        |
|                5. PSZ - pointer to the (adjacent strings) translations     |
|                6. HDCB - Dictionary handle for the found term              |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPFCreate(pTAInput, pLPData, WordPtr, &SegDescr,      |
|                                pszTranslation, hDictHandle);               |
+---------------------------------------------------------------------------*/
BOOL LPFCreate(PTAINPUT pTAInput, PLPDATA pLPData, PSZ_W pszTerm,
               PLPSEG pLPSeg, PSZ_W pszTranslation, HDCB hDict)
{
    PLPFENTRY pNewNode;
    BOOL     fOK;

    // Allocate memory for node
    fOK = UtlAlloc( (PVOID *) &pNewNode, 0L, (LONG) sizeof(LPFENTRY), ERROR_STORAGE);
#if defined(LOG_ALLOCS)
    lAllocs++;
    lAllocSize += sizeof(LPFENTRY);
#endif
    if (!fOK)
      pTAInput->pInD->fTerminate = TRUE;

    if (fOK)
      // Allocate memory for word & save it
      fOK = LPTermSave(pTAInput, pLPData, &(pNewNode->pszTerm), pszTerm);

    if (fOK && pszTranslation )
      // Allocate memory for translation & save it
      fOK = LPTermSave( pTAInput, pLPData, &(pNewNode->pszTranslations),
                        pszTranslation );

    if (fOK)
      {
      // Fill fields of new node (pointer to word & translation done above)
      pNewNode->usFrequency = 1;
      pNewNode->hDict = hDict;
      pNewNode->pContext = NULL;

      // if fFTLContext is TRUE (contexts required)
      if (pTAInput->fFTLcontext)
         // call LPContextUpd with pt to first context, PLPSEG
         fOK = LPContextUpd(pTAInput, &(pNewNode->pContext), pLPSeg);
      }

    if (fOK)
       // Call ListInsert
       fOK = ListInsert(pLPData->pFTL, &(pLPData->pFTL->root), (PTANODE) pNewNode);

    return fOK;
}

/*---------------------------------------------------------------------------+
|  Name:         LPFCompare                                                  |
|  Purpose:      Compare two "Found" node structures                         |
|  Parameters:   1. PLPFENTRY - pointer to the first node                    |
|                2. PLPFENTRY - pointer to the second node                   |
|  Returns:      SHORT - -1: node1<node2; 0: node1==node2; 1:node1>node2     |
|  Comments:     Future language oriented function will substitute stricmp   |
|  Samples:      s = LPFCompare(&node1, &node2);                             |
+---------------------------------------------------------------------------*/
SHORT LPFCompare(PLPFENTRY pNewNode1, PLPFENTRY pNewNode2)
{
    return (SHORT)(UTF16stricmp(pNewNode1->pszTerm, pNewNode2->pszTerm));

}


/*---------------------------------------------------------------------------+
|  Name:         LPFFree                                                     |
|  Purpose:      Free a node of the "Found" terms list (& associated memory) |
|  Parameters:   1. PLPFENTRY - pointer to the node to be freed              |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPFFree(&node);                                       |
+---------------------------------------------------------------------------*/
BOOL LPFFree(PLPFENTRY pNode)
{
    BOOL fOK = TRUE;
    PLPCTXT pNext;

    // Get first element of context list
    // repeat until next element pointer != NULL
    while (pNode->pContext != NULL)
      {
      pNext = pNode->pContext->pNext;

      //   free element
      fOK = UtlAlloc( (PVOID *) &(pNode->pContext), 0L, 0L, NOMSG) && fOK;
#if defined(LOG_ALLOCS)
        lAllocs--;
#endif

      //   get next element no.
      pNode->pContext = pNext;
      }

    // free node memory (but not it's term)
    fOK = UtlAlloc( (PVOID *) &pNode, 0L, 0L, NOMSG) && fOK;
#if defined(LOG_ALLOCS)
        lAllocs--;
#endif

    // We do not check the value of fOK here. Deallocation should continue
    // even if errors occur.

    return fOK;
}


/*---------------------------------------------------------------------------+
|  Name:         LPTLOutput                                                  |
|  Purpose:      Write the term list to disk                                 |
|  Parameters:   1. HWND - Window handle                                     |
|                2. PLPDATA - Pointer to LP data                             |
|                3. PTAINPUT - Input parameter struct                        |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPTLOutput(hwnd, &lpdata ,&inputRec);                 |
|   Writes files IN UNICODE only!! ( internal format!!)                      |
+---------------------------------------------------------------------------*/
BOOL LPTLOutput(HWND hwnd, PLPDATA pLPData, PTAINPUT pTAInput)
{
    BOOL  fOK = TRUE;                   // General status variable
    BOOL  fOK1 = TRUE;                  // Status variable for the UtlBuClose call
    PSZ   pszTLName = NULL;             // Name of output file
    PUNB_TREE pTermList;                // Used for casting from NTL or FTL
    LPPROCDATA ProcData;                // Gives LPOutTerm assess to HWND, LPDATA, TAINPUT
    PSZ   pData;                        // to output errormessage
    BOOL  fExisted = FALSE;             // list existed already flag


    PBOOL pfTerminate = &(pTAInput->pInD->fTerminate);   // Termination variable

    BOOL fNewList = (pLPData->ListType == NEW_LIST);
    LISTTAGS tableTypeTag;  // Used as a local ptr to the tag for NTLIST or FTLIST

    // Firstly, check to see if the output tag table exists
    fOK = LPTagTableExists(pLPData);

    if (!fOK)
       {
       pData = EMPTY_STRING; // no name for tagfile yet specified
       UtlError(ERROR_COULDNT_LOAD_TAGFILE,
                MB_CANCEL,
                1,
                &(pData),
                EQF_ERROR);
       *pfTerminate = TRUE;
       }

    if (fOK)                       // select TL name
       {
       pszTLName = (fNewList) ? pTAInput->szNTLname
                              : pTAInput->szFTLname;

       fExisted = UtlFileExist( pszTLName );

       // perform UtlBufOpen with TL Name
       fOK = ( UtlBufOpen( &(pLPData->pCBList), pszTLName,
                           BUFFERSIZE, FILE_CREATE, TRUE ) == NO_ERROR );

       if (!fOK)
          *pfTerminate = TRUE;
       }

    if (fOK)                        // print opening sequence: format is UNICODE!!
    {
        fOK = (UtlBufWrite(  pLPData->pCBList, UNICODEFILEPREFIX,
                               (SHORT)strlen(UNICODEFILEPREFIX), TRUE ) == NO_ERROR );

       tableTypeTag = (fNewList) ? NTLIST_TAG
                                 : FTLIST_TAG;

       // PERFORM LPOutTag with (N/F)TLIST
       fOK = LPOutTag(pLPData->pCBList,   // IOCB
                      pLPData->pTagDef,   // ptr to the tag table
                      tableTypeTag,       // Tag String
                      0,                  // No attributes
                      NULL,
                      pfTerminate,
                      0,                  // indent level
                      pLPData->pListFormatTable->pTagNamesW);
       if (fOK)
       {
          fOK = LPNewLine(pLPData->pCBList);     // get new line after opening
                                                 // sequence
       } // endif
    } // endif

    if (fOK)         // PERFORM LPOutHeader with TL Structure
       {
       fOK = LPOutHeader(pLPData->pCBList, pTAInput, pLPData);
       }

    if (fOK)
       {
       fOK = LPInitContextReferences(pLPData);

       if (!fOK)
          *pfTerminate = TRUE;  // Error mesage will be given by ListAlloc
       }

    if (fOK)     // prepare PROCDATA struct with hwnd, PTAINSTDATA, LPDATA
       {
       ProcData.hwnd = hwnd;
       ProcData.pTAInput = pTAInput;
       ProcData.pInD = pTAInput->pInD;
       ProcData.pLP = pLPData;

       // call ListScan with LPOutTerm, pt to LPPROCDATA
       pTermList = (fNewList) ? pLPData->pNTL
                              : pLPData->pFTL;

       fOK = ListScan(pTermList, pTermList->root, LPOutTerm, (PVOID) &ProcData);

       // Free the context references created by LPOutContext (EVEN fOK = FALSE)
       LPFreeContextReferences(pLPData);   // Next Context will restart at 1

       pLPData->szLastSegment[0] = NULC;   // Clear for next call to LPTLOutput
       }

    if (fOK)        // PERFORM LPOutTag with NTLIST or FTLIST
       {
       tableTypeTag = (fNewList) ? ENTLIST_TAG
                                 : EFTLIST_TAG;

       fOK = LPOutTag(pLPData->pCBList,   // IOCB
                      pLPData->pTagDef,   // ptr to the tag table
                      tableTypeTag,       // Calculated above
                      0,                  // No attributes
                      NULL,
                      pfTerminate,
                      0,                  // indent level
                      pLPData->pListFormatTable->pTagNamesW);
       }

    // PERFORM UtlBufOutClose with the output file control block
    // Note: using fOK1 because UtlBufOutClose shoule be called even if fOK
    //       is false at this stage.
    fOK1 = ( UtlBufClose( pLPData->pCBList, TRUE ) == NO_ERROR );


   /*******************************************************************/
   /* Broadcast notification message                                  */
   /*******************************************************************/
   if ( fOK && fOK1 )
   {
     if ( fExisted )
     {
       EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                            MP1FROMSHORT( PROP_CLASS_LIST ),
                            MP2FROMP(pszTLName) );
     }
     else
     {
       EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT(clsLIST),
                            MP2FROMP(pszTLName) );
     } /* endif */
   } /* endif */

    return (fOK && fOK1);
}


/*---------------------------------------------------------------------------+
|  Name:         LPTermSave                                                  |
|  Purpose:      Saves the term in a Term Save Area                          |
|  Parameters:   1. PTAINPUT - TAINPUT struct                                |
|                2. PLPDATA - Pointer to LPDATA                              |
|                3. PSZ * - pointer to pointer to the saved term             |
|                4. PSZ - pointer to the term being saved                    |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:     Sets fTerminate if allocate fails                           |
|  Samples:      fOK = LPTermSave(pTAInput, pLPData, &pszSavedTerm, pszTerm);|
+---------------------------------------------------------------------------*/
BOOL LPTermSave(PTAINPUT pTAInput, PLPDATA pLPData, PSZ_W * ppszSavedTerm,
                PSZ_W pszTerm)
{
    BOOL fOK = TRUE;
    // Get first TermArea from LPDATA
    PLPTERM_AREA pTermArea = pLPData->pTermArea;

    // IF No TermArea THEN
    if (pTermArea == NULL)
      {
      // allocate a new one and link it
      fOK = UtlAlloc( (PVOID *) &pTermArea, 0L, (LONG) sizeof(LPTERM_AREA), ERROR_STORAGE);
#if defined(LOG_ALLOCS)
      lAllocs++;
      lAllocSize += sizeof(LPTERM_AREA);
#endif
      if (!fOK)
        pTAInput->pInD->fTerminate = TRUE;
      else
        {
        // Initialise the TermArea
        pTermArea->pLink = NULL;
        pTermArea->pNext = pTermArea->Block;
        pLPData->pTermArea = pTermArea;
        }
      }

    if (fOK)
      {
      // IF space NOT available THEN
      // TERM_AREA_SIZE are number of CHAR_W's
        if ((TERM_AREA_SIZE - ((PCHAR_W)pTermArea->pNext - (PCHAR_W)pTermArea->Block))
            <
           (UTF16strlenCHAR(pszTerm) + 2))
          {
          // allocate another term area
          fOK = UtlAlloc( (PVOID *) &(pTermArea),
                         0L,
                         (LONG) sizeof(LPTERM_AREA) * sizeof(CHAR_W),
                         ERROR_STORAGE);
#if defined(LOG_ALLOCS)
          lAllocs++;
          lAllocSize += sizeof(LPTERM_AREA);
#endif
          // If allocation failed then set fTerminate
          if (!fOK)
            pTAInput->pInD->fTerminate = TRUE;
          else
            {
            // pTermArea is linked to the head of the TermArea list
            pTermArea->pLink = pLPData->pTermArea;

            // Initialise the new TermArea Block pointer
            pTermArea->pNext = pTermArea->Block;

            // Update pointer to head of list in pLPData
            pLPData->pTermArea = pTermArea;
            }
          }
      }

    if (fOK)
      {
      // Copy term to TermArea
      UTF16strcpy(pTermArea->pNext, pszTerm);

      // Set address of return pointer
      *ppszSavedTerm = pTermArea->pNext;

      // Set address of next available space
      pTermArea->pNext = pTermArea->pNext + UTF16strlenCHAR(pszTerm) + 1;
      }

    // return true if everything OK
    return fOK;
}


/*---------------------------------------------------------------------------+
|  Name:         LPTermFree                                                  |
|  Purpose:      Frees the memory in the Term Save Area                      |
|  Parameters:   1. PLPDATA - Pointer to LPDATA                              |
|  Returns:      BOOL - Always returns true                                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPTermFree(pLPData);                                  |
+---------------------------------------------------------------------------*/
BOOL LPTermFree(PLPDATA pLPData)
{
    // Get first pointer from LPDATA
    PLPTERM_AREA pTermArea = pLPData->pTermArea;
    PLPTERM_AREA pNextTermArea;

    // WHILE pointer != NULL
    while (pTermArea != NULL)
      {
      // save pointer to next
      pNextTermArea = pTermArea->pLink;

      // free current
      UtlAlloc( (PVOID *) &pTermArea, 0L, 0L, NOMSG);
#if defined(LOG_ALLOCS)
        lAllocs--;
#endif

      // current <= next
      pTermArea = pNextTermArea;
      }

    pLPData->pTermArea = NULL; // Set value in pLPData (Used local var for deallocation)

    return TRUE; // Always returns true
}


/*อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ                 Implementation of Private Functions                        บ
ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ*/


/*---------------------------------------------------------------------------+
|  Name:         LPContextRefSearch                                          |
|  Purpose:      Search for a context ref in the ContextRef tree             |
|  Parameters:   1. PLPDATA - Ptr to LP local data                           |
|                2. PLPSEG - pt to segment descriptor                        |
|  Returns:      PLPCTXTREF - ptr to the found reference (or NULL)           |
|  Comments:                                                                 |
|  Samples:      pContextRef = LPContextRefSearch(pLPData, pContext);        |
+---------------------------------------------------------------------------*/
PLPCTXTREF LPContextRefSearch(PLPDATA pLPData, PLPSEG pContext)
{
    LPCTXTREF SearchNode;

    memcpy(&(SearchNode.context), pContext, sizeof(LPSEG));

    // Call List Search with work node, Table pointer
    return (PLPCTXTREF) ListSearch(pLPData->pContextRefs, (PTANODE) &SearchNode);
}


/*---------------------------------------------------------------------------+
|  Name:         LPContextRefInsert                                          |
|  Purpose:      Insert a node into the context ref. tree                    |
|  Parameters:   1. PLPDATA - Ptr to LP local data                           |
|                2. PLPSEG - pt to segment descriptor                        |
|  Returns:      BOOL - TRUE if successful, FALSE otherwise                  |
|  Comments:     Assumes the node is not already in the tree                 |
|  Samples:      fOK = LPContextRefInsert(pLPData, pContext, pfTerminate);   |
+---------------------------------------------------------------------------*/
BOOL LPContextRefInsert(PLPDATA pLPData, PLPSEG pLPSeg, PBOOL pfTerminate)
{
    PLPCTXTREF pNewNode;
    BOOL     fOK;

    // Allocate memory for node
    fOK = UtlAlloc( (PVOID *) &pNewNode, 0L,(LONG) sizeof(LPCTXTREF), ERROR_STORAGE);
#if defined(LOG_ALLOCS)
    lAllocs++;
    lAllocSize += sizeof(LPCTXTREF);
#endif
    if (!fOK)
      *pfTerminate = TRUE;

    if (fOK)            // Fill fields of new node & insert
      {
      memcpy(&(pNewNode->context), pLPSeg, sizeof(LPSEG));
      pNewNode->usRef = (USHORT) pLPData->pContextRefs->ulCount + 1;

      fOK = ListInsert(pLPData->pContextRefs,
                       &(pLPData->pContextRefs->root),
                       (PTANODE) pNewNode);
      }

    return fOK;
}

/*---------------------------------------------------------------------------+
|  Name:         LPContextRefCompare                                         |
|  Purpose:      Compare two ContextRef node structures                      |
|  Parameters:   1. PLPCTXTREF - pointer to the first node                   |
|                2. PLPCTXTREF - pointer to the second node                  |
|  Returns:      SHORT - -1: node1<node2; 0: node1==node2; 1:node1>node2     |
|  Comments:                                                                 |
|  Samples:      s = LPContextRefCompare(&node1, &node2);                    |
+---------------------------------------------------------------------------*/
SHORT LPContextRefCompare(PLPCTXTREF pNewNode1, PLPCTXTREF pNewNode2)
{
    return (SHORT)(memcmp(&(pNewNode1->context), &(pNewNode2->context), sizeof(LPSEG)));
}

/*---------------------------------------------------------------------------+
|  Name:         LPContextRefFree                                            |
|  Purpose:      Free a node of the ContextRefs tree                         |
|  Parameters:   1. PLPCTXTREF - pointer to the node to be freed             |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPContextRefFree(&node);                              |
+---------------------------------------------------------------------------*/
BOOL LPContextRefFree(PLPCTXTREF pNode)
{
    BOOL fOK = TRUE;

    // free node memory
    fOK = UtlAlloc( (PVOID *) &pNode, 0L, 0L, NOMSG);
#if defined(LOG_ALLOCS)
        lAllocs--;
#endif

    return fOK;
}

/*--------------------- Macros -----------------------------------------------*/
#define CTXTREF_LINK_OFFSET ((char *)(&((LPCTXTREF *)0)->link) - (char *)0)

/*---------------------------------------------------------------------------+
|  Name:         LPInitContextReferences                                     |
|  Purpose:      Initialises the context references tree                     |
|  Parameters:   1. PLPDATA - Pointer to LPDATA                              |
|  Returns:      BOOL - true if successful                                   |
|  Comments:                                                                 |
|  Samples:      fOK = LPInitContextReferences(pLPData);                     |
+---------------------------------------------------------------------------*/
BOOL LPInitContextReferences(PLPDATA pLPData)
{
    // Create the tree for the ContextRefs
    pLPData->pContextRefs = ListAlloc(CTXTREF_LINK_OFFSET,
                                      (PFN_CMP) LPContextRefCompare,
                                      (PFN_FREE) LPContextRefFree);
#if defined(LOG_ALLOCS)
        lAllocs++;
        lAllocSize +=  (LONG)sizeof(UNB_TREE);
#endif

    return (pLPData->pContextRefs != NULL);  // errors in allocation?
}


/*---------------------------------------------------------------------------+
|  Name:         LPFreeContextReferences                                     |
|  Purpose:      Frees the context references created by a call to LPTLOutput|
|  Parameters:   1. PLPDATA - Pointer to LPDATA                              |
|  Returns:      BOOL - Always returns true                                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPFreeContextReferences(pLPData);                     |
+---------------------------------------------------------------------------*/
BOOL LPFreeContextReferences(PLPDATA pLPData)
{
    // Free the sort buffer used for sorting context reference numbers
    UtlAlloc( (PVOID *) &(pLPData->pusSortBuffer), 0L, 0L, NOMSG);
#if defined(LOG_ALLOCS)
        lAllocs--;
#endif
    pLPData->usSortBufferSize = 0;

    // Free the context reference tree
    ListFree(pLPData->pContextRefs);
#if defined(LOG_ALLOCS)
        lAllocs--;
#endif

    return TRUE; // Always returns true
}


/*---------------------------------------------------------------------------+
|  Name:         LPCloseFiles                                                |
|  Purpose:      Close all the files opened for reading contexts             |
|  Parameters:   1. PLPFCB * - ptr to last of open files CBs                 |
|  Returns:      BOOL - True if successful, else FALSE                       |
|  Comments:                                                                 |
|  Samples:      fOK = LPCloseFiles(&(pLPData->ppLPFCB);                     |
+---------------------------------------------------------------------------*/
BOOL LPCloseFiles(PLPFCB *ppLPFCB)
{
  BOOL fOK = TRUE;              // Error status variable
  USHORT usRC;                  // Utility fn. return code
  PLPFCB pFCBIndex = *ppLPFCB;

  while (pFCBIndex != NULL)
     {
     usRC = UtlClose(pFCBIndex->hFile,
                     FALSE);          // Do not handle errors in utility

     if (fOK)  // We will not abort on an error here but fOK will be returned
       fOK = (usRC == 0);

     *ppLPFCB = (*ppLPFCB)->pNext;
     UtlAlloc( (PVOID *) &pFCBIndex, 0L, 0L, NOMSG);
     pFCBIndex = *ppLPFCB;
     }

  return fOK;
  // Errors in this function do not produce error messages as fTerminate
  // may have been set already, and this is just cleaning up!
}


/*---------------------------------------------------------------------------+
|  Name:         LPLoadTagTable                                              |
|  Purpose:      Loads the tag table for NTL & FTL I/O                       |
|  Parameters:   1. PLPDATA - ptr to the LPDATA area                         |
|                2. PTAINPUT - ptr to TAINPUT                                |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:     Called by LPInit                                            |
|                Sets fTerminate if failure                                  |
|  Samples:      fOK = LPLoadTagTable(pLPData, pTAInput);                    |
+---------------------------------------------------------------------------*/
BOOL LPLoadTagTable(PLPDATA pLPData, PTAINPUT pTAInput)
{
   BOOL             fOK = TRUE;
   CHAR             szFileName[MAX_EQF_PATH];  // Filename for Tag Table
   PSZ_W            pszData;                        // pointer to tag end string
   USHORT           usRc;
   PSZ              pszFileName = szFileName;


   usRc = TALoadTagTable( LISTFORMATTABLE, &(pLPData->pListFormatTable) , TRUE, TRUE );
   pLPData->pTagDef = pLPData->pListFormatTable->pTagTable;

   if (usRc)
   {
     UtlMakeEQFPath(szFileName, NULC, TABLE_PATH, NULL);
     sprintf( szFileName, PATHCATFILE, szFileName, LISTFORMATTABLE );
     strcat( szFileName, EXT_OF_INTTABLE);

     UtlError(ERROR_COULDNT_LOAD_TAGFILE,
              MB_CANCEL,
              1,
              &pszFileName,  //Ptr to szFileName
              EQF_ERROR);
     pTAInput->pInD->fTerminate = TRUE;
   }
   else
   {
      fOK = (pLPData->pTagDef != NULL)
            &&
            (memcmp(pLPData->pTagDef->chId, FORMATID, sizeof(FORMATID) ) == 0);

      if (!fOK)
      {
        UtlMakeEQFPath(szFileName, NULC, TABLE_PATH, NULL);
        sprintf( szFileName, PATHCATFILE, szFileName, LISTFORMATTABLE );
        strcat( szFileName, EXT_OF_INTTABLE);

        UtlError(NO_VALID_FORMAT,
                 MB_CANCEL,
                 1,
                 &pszFileName,  //Ptr to szFileName
                 EQF_ERROR);
        pTAInput->pInD->fTerminate = TRUE;
      }
      else
      {
         pszData = LPGetTagEndDelim((LISTTAGS) 0, pLPData->pTagDef,
                            pLPData->pListFormatTable->pTagNamesW,
                            pLPData->ulSrcOemCP);  // get end tag
                                                           // delimiter
         while (*pszData == BLANK)                         // without blank
         {                                                 // and "end of
           pszData++;                                      // line"-string into
         } /* endwhile */                                  // szEndChar
         szEndChar[0] = *pszData;
         szEndChar[1] = EOS;
      } // endif
    } // endif
   return fOK;
}

/*---------------------------------------------------------------------------+
|  Name:         LPNewLine                                                   |
|  Purpose:      Print a newline to an already open output file              |
|  Parameters:   1. PBUFCB - ptr to an IO control block                    |
|  Returns:      BOOL - Return code from UtlBufWrite                         |
|  Comments:                                                                 |
|  Samples:      fOK = LPNewLine(pIOCB);                                     |
+---------------------------------------------------------------------------*/
static BOOL LPNewLine(PBUFCB pIOCB)
{
   return ( UtlBufWriteW(pIOCB, L"\r\n", 2 * sizeof(CHAR_W), TRUE ) == NO_ERROR );
}

/*---------------------------------------------------------------------------+
|  Name:         LPIndent                                                    |
|  Purpose:      Print a number of spaces - intended for indenting           |
|  Parameters:   1. PBUFCB - ptr to an IO control block                    |
|                2. USHORT - the number of spaces to indent                  |
|  Returns:      BOOL - Return code from UtlBufWrite                         |
|  Comments:                                                                 |
|  Samples:      fOK = LPIndent(pIOCB, 2);                                   |
+---------------------------------------------------------------------------*/
static BOOL LPIndent(PBUFCB pIOCB, USHORT uSpaces)
{
   if (uSpaces <= MAX_INDENT)
     return (UtlBufWriteW(pIOCB, INDENT_SPACES, (USHORT)(uSpaces * sizeof(CHAR_W)), TRUE ) == NO_ERROR);
   else
     return (UtlBufWriteW(pIOCB, INDENT_SPACES, MAX_INDENT * sizeof(CHAR_W), TRUE) == NO_ERROR);
}


/*---------------------------------------------------------------------------+
|  Name:         LPOutTagList                                                |
|  Purpose:      Output a list of tags (with no attributes) where the values |
|                are in a string buffer format                               |
|  Parameters:   1. PBUFCB - ptr to an IO control block                    |
|                2. PTAGTABLE - the tag table to extact the tags from        |
|                3. LISTTAGS - index of the start tag for each entry         |
|                4. LISTTAGS - index of the end tag for each entry           |
|                5. PBYTE - A reference point for calculating string offsets |
|                6. STRINGBUFFER - offset to a list of string values         |
|                7. PBOOL - ptr to the fTerminate variable                   |
|  Returns:      BOOL - Return code LPOutTag                                 |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutTagList(pIOCB, pLPData->pTagDef,                 |
|                                 DICTNAME_TAG, EDICTNAME_TAG,               |
|                                 pByte, pTAInput->stInputDict, pfTerminate);|
+---------------------------------------------------------------------------*/
static BOOL LPOutTagList(PBUFCB pIOCB,
                         PTAGTABLE pTagTable,
                         LISTTAGS openTag,
                         LISTTAGS closeTag,
                         PBYTE pReference,
                         STRINGBUFFER stValues,
                         PBOOL pfTerminate,
                         PSZ_W  pTagNamesW,
                         PSZ_W  pConvBufW,
                         ULONG  ulSrcOemCP)

{
   BOOL fOK = TRUE;
   USHORT i;
   PSZ pszValue;
   CHAR_W szTemp[256];
   pConvBufW = &szTemp[0];

   for (i=0, pszValue = (PSZ)(pReference + stValues.ulOffset);
        i < stValues.usNumber;
        i++, pszValue += strlen(pszValue) + 1)
   {

      ASCII2Unicode( pszValue, pConvBufW, ulSrcOemCP);

      fOK = LPOutStatement(
                     pIOCB,        // Output control block
                     pTagTable,    // ptr to the tag table
                     openTag,      // open tag index
                     closeTag,     // close tag index
                     pConvBufW,    // list name
                     0,            // No attributes
                     NULL,         // No attribute list
                     pfTerminate,  // ptr to termination variable
                     2, pTagNamesW);       // Indent level

      if (!fOK)
         break; // Abort loop
   }

   return fOK;
}


/*---------------------------------------------------------------------------+
|  Name:         LPOutHeader                                                 |
|  Purpose:      Output a header (in SGML format) for a NTL or FTL term      |
|  Parameters:   1. PBUFCB - ptr to an IO control block                    |
|                2. PTAINPUT - ptr to TAINPUT for header info                |
|                3. PLPDATA - reference point for calculating string offsets |
|  Returns:      BOOL - Return TRUE if successful, FALSE otherwise           |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutHeader(pIOCB, pTAInput, pLPDAta);                |
+---------------------------------------------------------------------------*/
static BOOL LPOutHeader(PBUFCB pIOCB, PTAINPUT pTAInput, PLPDATA pLPData)
{
   BOOL fOK = TRUE;
   PBOOL pfTerminate = &(pTAInput->pInD->fTerminate);
   TIME_L lTime;
   CHAR_W szTemp[MAX_EQF_PATH];          //  is 60; use for DATE_BUF_LEN = 50 too
   PSZ_W  pTagNamesW = pLPData->pListFormatTable->pTagNamesW;
   PSZ_W  pConvBuf = &szTemp[0];

   memset(&lTime, 0, sizeof(lTime));
   fOK = LPOutTag(pIOCB,
                  pLPData->pTagDef,      // ptr to the tag table
                  HEADER_TAG,            // tag to be output
                  0,                     // No attributes
                  NULL,
                  pfTerminate,
                  1, pTagNamesW);
   if ( fOK )
   {
     fOK = LPNewLine(pIOCB);
   } // endif

   if (fOK)              // Output the list input text filenames
      {
      fOK = LPOutTagList(pIOCB,
                         pLPData->pTagDef,        // ptr to the tag table
                         TEXTNAME_TAG,            // open Tag index
                         ETEXTNAME_TAG,           // close Tag index
                         (PBYTE) pTAInput,        // Reference for STRINGBUFFER
                         pTAInput->stSourcefiles, // STRINGBUFFER
                         pfTerminate,
                         pTagNamesW,
                         pConvBuf,
                         pLPData->ulSrcOemCP);
      }

   if (fOK)              // Output the list of dictionary names
      {
      fOK = LPOutTagList(pIOCB,
                         pLPData->pTagDef,      // ptr to the tag table
                         DICTNAME_TAG,          // open Tag index
                         EDICTNAME_TAG,         // close Tag index
                         (PBYTE) pTAInput,      // Reference for STRINGBUFFER
                         pTAInput->stInputDict, // STRINGBUFFER
                         pfTerminate,
                         pTagNamesW,
                         pConvBuf,
                         pLPData->ulSrcOemCP);

      }

    if (fOK)             // Output the Output dictionary name
     {
       ASCII2UnicodeBuf(pTAInput->szOutDictName, szTemp,
                        MAX_EQF_PATH, pLPData->ulSrcOemCP);

       fOK = LPOutStatement(
                      pLPData->pCBList,          // Output control block
                      pLPData->pTagDef,          // ptr to the tag table
                      OUTDICT_TAG,               // open Tag index
                      EOUTDICT_TAG,              // close Tag index
                      szTemp,                    // list name
                      0,                         // No attributes
                      NULL,                      // No attribute list
                      pfTerminate,               // ptr to termination variable
                      2, pTagNamesW);
     }

    if (fOK)             // Output the exclusion list name
       {
       ASCII2UnicodeBuf(pTAInput->szExclusionList, szTemp,
                        MAX_EQF_PATH, pLPData->ulSrcOemCP);
       fOK = LPOutStatement(
                      pLPData->pCBList,          // Output control block
                      pLPData->pTagDef,          // ptr to the tag table
                      EXCLLISTNAME_TAG,          // open Tag index
                      EEXCLLISTNAME_TAG,         // close Tag index
                      szTemp,                    // list name
                      0,                         // No attributes
                      NULL,                      // No attribute list
                      pfTerminate,               // ptr to termination variable
                      2, pTagNamesW);
       }

    if (fOK)            // Output the exclusion dictionary name
       {
       ASCII2UnicodeBuf(pTAInput->szExclDictname, szTemp,
                        MAX_EQF_PATH, pLPData->ulSrcOemCP);

       fOK = LPOutStatement(
                      pLPData->pCBList,          // Output control block
                      pLPData->pTagDef,          // ptr to the tag table
                      EXCLDICT_TAG,              // open Tag index
                      EEXCLDICT_TAG,             // close Tag index
                      szTemp,                    // list name
                      0,                         // No attributes
                      NULL,                      // No attribute list
                      pfTerminate,               // ptr to termination variable
                      2, pTagNamesW);
       }

    if (fOK)            // Output the DATE
       {
       UtlTime( &lTime);                 // Get current time
       UtlLongToDateStringW( (LONG) lTime, szTemp, MAX_EQF_PATH);
       fOK = LPOutStatement(
                      pLPData->pCBList,          // Output control block
                      pLPData->pTagDef,          // ptr to the tag table
                      CREATEDATE_TAG,            // open Tag index
                      ECREATEDATE_TAG,           // close Tag index
                      szTemp,                 // list name
                      0,                         // No attributes
                      NULL,                      // No attribute list
                      pfTerminate,               // ptr to termination variable
                      2, pTagNamesW);
       }

    if (fOK)            // Output the TIME - Use same lTime as above
       {
       UtlLongToTimeStringW( (LONG) lTime, szTemp, MAX_EQF_PATH);
       fOK = LPOutStatement(
                      pLPData->pCBList,          // Output control block
                      pLPData->pTagDef,          // ptr to the tag table
                      CREATETIME_TAG,            // open Tag index
                      ECREATETIME_TAG,           // close Tag index
                      szTemp,                 // list name
                      0,                         // No attributes
                      NULL,                      // No attribute list
                      pfTerminate,               // ptr to termination variable
                      2, pTagNamesW);
       }

    if (fOK)            // Output the close Header
    {
       fOK = LPOutTag(pIOCB,
                      pLPData->pTagDef,      // ptr to the tag table
                      EHEADER_TAG,           // Tag index
                      0,                     // No attributes
                      NULL,
                      pfTerminate,
                      1, pTagNamesW);
       if (fOK )
       {
         fOK = LPNewLine(pIOCB);       // new line after end header tag
       } /* endif */
    }

   return fOK;
}

/*---------------------------------------------------------------------------+
|  Name:         LPOutTerm                                                   |
|  Purpose:      Output a single term to list file. (New or found terms)     |
|  Parameters:   1. PTANODE - pointer to an LPFENTRY or LPNENTRY node          |
|                2. PVOID - pr to a LPPROCDATA containing pointers to hwnd,  |
|                               TAINSTDATA ...                               |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:     This function is passed as a parameter to ListScan          |
|                fTerminate may be set (force shutdown request)              |
|  Samples:      fOK = LPOutTerm(&pnewnode, (PVOID) pprocData);              |
+---------------------------------------------------------------------------*/
static EQF_BOOL LPOutTerm(PTANODE pNode, PVOID pLPProcData)
{
    BOOL fOK = TRUE;
    PLPPROCDATA pProcData =(PLPPROCDATA) pLPProcData; // Converted from a PVOID

    // Extract pointers from the PLPPROCDATA struct
    PTAINPUT pTAInput = pProcData->pTAInput;
    PLPDATA pLPData = pProcData->pLP;

    PLPNENTRY pLPNentry = (PLPNENTRY) pNode;  // Used if ListType = NEW_LIST
    PLPFENTRY pLPFentry = (PLPFENTRY) pNode;  // Used if ListType = FOUND_LIST

    // Get type of list from LPDATA
    BOOL fNewList = (pLPData->ListType == NEW_LIST);

    ULONG oldSliderPosition; // Dialog's new slider position
    ULONG newSliderPosition; // Dialog's old slider position

    //
    // Update the slider
    //

    if (pLPData->ulTotalTerms == 0)  // If this is the first term:
       {                             // then initialise variables
       pLPData->ulCurrentTerm = 1;

       if (*(pTAInput->szNTLname) != '\0') // New Terms will be output
          pLPData->ulTotalTerms += pLPData->pNTL->ulCount;

       if (*(pTAInput->szFTLname) != '\0') // Found Terms will be output
          pLPData->ulTotalTerms += pLPData->pFTL->ulCount;

       // First Time so Reset Slider
       UpdateSliderPosition( pTAInput->hwndProcWin, 1L );
       }
    else         // Update the term counter & Slider
       {
       oldSliderPosition = (ULONG)(pLPData->ulCurrentTerm   *
                                   ( SLIDER_INCREMENT - 1 ) /
                                   pLPData->ulTotalTerms);

       pLPData->ulCurrentTerm++;

       newSliderPosition = (ULONG)(pLPData->ulCurrentTerm   *
                                   ( SLIDER_INCREMENT - 1 ) /
                                   pLPData->ulTotalTerms);

       if (oldSliderPosition != newSliderPosition)
          {            //Update the slider only if position has changed
          UpdateSliderPosition( pTAInput->hwndProcWin, newSliderPosition);
          }
       }


    // if TL == NTL && frequency of Term >= Frequency for NTL or
    // if TL == FTL && frequency of Term >= Frequency for FTL then

    if ((fNewList &&
         (pLPNentry->usFrequency >= pTAInput->usNTLNumOccurences)) ||
        (!fNewList &&
         (pLPFentry->usFrequency >= pTAInput->usFTLNumOccurences)))
       {             // Perform LPOutTermData
       fOK = LPOutTermData(pProcData,
                           pNode,
                           pLPData,
                           &(pTAInput->pInD->fTerminate));
       }

    if (fOK)
      {
      fOK = LocalTAGiveCntrl(pProcData);
      }

    return ((EQF_BOOL)fOK);
}


/*---------------------------------------------------------------------------+
|  Name:         LPOutTermData                                               |
|  Purpose:      Output the data of a "new" or a "found" term (TEXT)         |
|  Parameters:   1. PLPPROCDATA - data for TAGiveCntrl                       |
|                2. PTANODE - pointer to "new" or "found" term node            |
|                3. PLPDATA - pointer local data                             |
|                4. PBOOL - Pointer to Terminate boolean variable            |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutTermData(pProcData, pNewNode, pLPData,           |
|                                    &fTerminate);                           |
+---------------------------------------------------------------------------*/
static BOOL LPOutTermData(PLPPROCDATA pProcData, PTANODE pNode, PLPDATA pLPData,
                          PBOOL pfTerminate)
{
    BOOL fOK = TRUE;                // Status variable
    PLPCTXT pLPCtxt;                // pointer for tracing context list
    CHAR    szDictName[MAX_LONGFILESPEC]; // Temp dictionary name
    CHAR_W  szTempW[MAX_LONGFILESPEC];

    // Get type of input node and create a pointer to a "new"
    // or a "found entry,
    PLPNENTRY pLPNentry = (PLPNENTRY) pNode;  // Used if ListType = NEW_LIST
    PLPFENTRY pLPFentry = (PLPFENTRY) pNode;  // Used if ListType = FOUND_LIST

    // Get type of list from LPDATA
    BOOL fNewList = (pLPData->ListType == NEW_LIST);

    // prepare n. of occurrences of usFrequency
    USHORT usFrequency = (fNewList) ? pLPNentry->usFrequency
                                    : pLPFentry->usFrequency;

    // Prepare the term to be output
    PSZ_W pszTerm = (fNewList) ? pLPNentry->pszTerm
                             : pLPFentry->pszTerm;

    if (fOK)              // Output the opening tag
       {
       fOK = LPOutTag(pLPData->pCBList,   // IOCB
                      pLPData->pTagDef,   // ptr to the tag table
                      TERM_TAG,           // Tag name
                      0,                  // No attributes
                      NULL,
                      pfTerminate,
                      1, pLPData->pListFormatTable->pTagNamesW);
       if (fOK)
          fOK = LPNewLine(pLPData->pCBList);
       }

    if (fOK)            // Output the string value
       {
       fOK = LPOutStatement(
                      pLPData->pCBList,        // Output control block
                      pLPData->pTagDef,        // ptr to the tag table
                      LEMMA_TAG,               // open Tag index
                      ELEMMA_TAG,              // close Tag index
                      pszTerm,                 // LEMMA Value
                      0,                       // No attributes
                      NULL,                    // No attribute list
                      pfTerminate,             // ptr to termination variable
                      2, pLPData->pListFormatTable->pTagNamesW);   // Indent level
       }

    if (fOK && (!fNewList) && (pLPFentry->pszTranslations != NULL))
       {
       fOK = LPOutStatement(
                      pLPData->pCBList,        // Output control block
                      pLPData->pTagDef,        // ptr to the tag table
                      TRANSLATION_TAG,         // open Tag index
                      ETRANSLATION_TAG,        // close Tag index
                      pLPFentry->pszTranslations, // LEMMA Value
                      0,                       // No attributes
                      NULL,                    // No attribute list
                      pfTerminate,             // ptr to termination variable
                      2, pLPData->pListFormatTable->pTagNamesW);        // Indent level
       }

    if (fOK && (!fNewList))   // Output the found term dictionary
       {
       AsdQueryDictName(pLPFentry->hDict, szDictName);
       ASCII2UnicodeBuf( szDictName, &szTempW[0], MAX_LONGFILESPEC, pLPData->ulSrcOemCP );

       fOK = LPOutStatement(
                      pLPData->pCBList,        // Output control block
                      pLPData->pTagDef,        // ptr to the tag table
                      DICTNAME_TAG,            // Open tag index
                      EDICTNAME_TAG,           // close tag index
                      szTempW,                 // Dictionary name
                      0,                       // No attributes
                      NULL,                    // No attribute list
                      pfTerminate,             // ptr to termination variable
                      2, pLPData->pListFormatTable->pTagNamesW);       // Indent level
       }

    if (fOK)               // Output the frequency
       {
         itoa((INT) usFrequency,  (PSZ)(pLPData->szPrBuf), RADIX_10);
         ASCII2UnicodeBuf( (PSZ)(pLPData->szPrBuf), &szTempW[0], MAX_LONGFILESPEC, pLPData->ulSrcOemCP );

       fOK = LPOutStatement(
                      pLPData->pCBList,        // Output control block
                      pLPData->pTagDef,        // ptr to the tag table
                      FREQUENCY_TAG,           // open Tag index
                      EFREQUENCY_TAG,          // close Tag index
                      szTempW,                 // Frequency value
                                               // Short converted to string
                      0,                       // No attributes
                      NULL,                    // No attribute list
                      pfTerminate,             // ptr to termination variable
                      2, pLPData->pListFormatTable->pTagNamesW);      // Indent level
       }

    if (fOK)           // Get first source context (from new node or found node)
      {
      pLPCtxt = (fNewList) ? pLPNentry->pContext
                           : pLPFentry->pContext;

      pLPData->usReferencesInBuffer = 0; // clear no. of context references

      while (fOK && pLPCtxt != NULL)    // If contexts exist then output them
         {

         fOK = LPOutContext(&(pLPCtxt->context), pLPData, pfTerminate);


         pLPCtxt = pLPCtxt->pNext;

         if (fOK)        // Dispatch messages
            {
            fOK = LocalTAGiveCntrl(pProcData);
            }
         }
       }

    if (fOK)                   // Output the context references now
       {
       fOK = LPOutContextReferences(pLPData, pfTerminate);
       }

    if (fOK)                   // Output the Closing Tag
       {
       fOK = LPOutTag(pLPData->pCBList,   // IOCB
                      pLPData->pTagDef,   // ptr to the tag table
                      ETERM_TAG,          // Tag name
                      0,                  // No attributes
                      NULL,
                      pfTerminate,
                      1,pLPData->pListFormatTable->pTagNamesW);
       if (fOK)
          fOK = LPNewLine(pLPData->pCBList);
       }

    return fOK;
}


/*---------------------------------------------------------------------------+
|  Name:         LocalTAGiveCntrl                                            |
|  Purpose:      Calls TAGiveCntrl & handles fKill/fTerminate results        |
|  Parameters:   1. PLPPROCDATA - ptr to all data required                   |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LocalTAGiveCntrl(pProcData);                          |
+---------------------------------------------------------------------------*/
static BOOL LocalTAGiveCntrl(PLPPROCDATA pProcData)
{
   BOOL fOK;

   fOK = TAGiveCntrl(pProcData->hwnd, (PTAINSTDATA) pProcData->pInD);

   if (fOK && pProcData->pTAInput->fKill)
   {                                                            /* 4@KIT1156D */
     pProcData->pTAInput->fKill = FALSE;
     pProcData->pTAInput->pInD->fTerminate = TRUE;
     fOK = FALSE;
   }

   return fOK;
}

/*---------------------------------------------------------------------------+
|  Name:         LPTagTableExists                                            |
|  Purpose:      Check if the Tag Table for list processing exists           |
|  Parameters:   1. PLPDATA - pointer local data                             |
|  Returns:      BOOL - True if the table exists, False otherwise            |
|  Comments:                                                                 |
|  Samples:      fOK = LPTagTableExists(pLPData);                            |
+---------------------------------------------------------------------------*/
static BOOL LPTagTableExists(PLPDATA pLPData)
{
    return (pLPData->pTagDef != NULL);
}


/*---------------------------------------------------------------------------+
|  Name:         LPGetTagString                                              |
|  Purpose:      Get a tag string from the LP tag table using the given tag  |
|  Parameters:   1. LISTTAGS - the tag to use as an index                    |
|                2. PTAGTABLE - Ptr to the tag table                         |
|  Returns:      PSZ - ptr to the tag string in the tagtable                 |
|  Comments:                                                                 |
|  Samples:      pszTag = LPGetTagString(CONTEXT_TAG, pLPData->pTagDef);     |
+---------------------------------------------------------------------------*/
static PSZ_W LPGetTagString(LISTTAGS tagIndex, PTAGTABLE pTagTable, PSZ_W pTagNamesW)
{
    //PSZ pTagNames = (PSZ) ((PBYTE) pTagTable + pTagTable->uTagNames);
    PTAG pTag    = (PTAG) ((PBYTE) pTagTable + pTagTable->stFixTag.uOffset);

    return pTag[tagIndex].uTagnameOffs + pTagNamesW;
}

/*---------------------------------------------------------------------------+
|  Name:         LPGetTagEndDelim                                            |
|  Purpose:      Get a tag end delimiter from the LP tag table               |
|  Parameters:   1. LISTTAGS - the tag to use as an index                    |
|                2. PTAGTABLE - Ptr to the tag table                         |
|  Returns:      PSZ - ptr to the tag string in the tagtable                 |
|  Comments:                                                                 |
|  Samples:      pszTag = LPGetTagEndDelim(CONTEXT_TAG, pLPData->pTagDef);   |
+---------------------------------------------------------------------------*/
static PSZ_W LPGetTagEndDelim(LISTTAGS tagIndex, PTAGTABLE pTagTable,
                              PSZ_W pTagNamesW, ULONG ulCP )
{


    PSZ pTagNames = (PSZ) ((PBYTE) pTagTable + pTagTable->uTagNames);
    PTAG pTag    = (PTAG) ((PBYTE) pTagTable + pTagTable->stFixTag.uOffset);
    static CHAR_W szEndTag[40];

    pTagNamesW;
    memset( &szEndTag[0], 0, sizeof( szEndTag ));
    ASCII2UnicodeBuf( pTag[tagIndex].uEndDelimOffs + pTagNames, &szEndTag[0], 40, ulCP );
    return szEndTag;


}

/*---------------------------------------------------------------------------+
|  Name:         LPGetAttributeString                                        |
|  Purpose:      Get an attribute string from the LP tag table using the     |
|                given attribute index                                       |
|  Parameters:   1. LISTTAGS - the tag to use as an index                    |
|                2. PTAGTABLE - Ptr to the tag table                         |
|  Returns:      PSZ - ptr to the tag string in the tagtable                 |
|  Comments:                                                                 |
|  Samples:      pszAttr = LPGetAttributeString(ID_ATTR,                     |
|                                               pLPData->pTagDef);           |
+---------------------------------------------------------------------------*/
static PSZ LPGetAttributeString(LISTTAGS attributeIndex, PTAGTABLE pTagTable)
{
PSZ pTagNames = (PSZ) ((PBYTE) pTagTable + pTagTable->uTagNames);
    PATTRIBUTE pAttr = (PATTRIBUTE) ((PBYTE) pTagTable +
                                     pTagTable->stAttribute.uOffset);

    return pAttr[attributeIndex - pTagTable->uNumTags].uStringOffs + pTagNames;
}

/*---------------------------------------------------------------------------+
|  Name:         LPGetAttributeEndDelim                                      |
|  Purpose:      Get an attribute End delimiter from the LP tag table using  |
|                the given attribute index                                   |
|  Parameters:   1. LISTTAGS - the tag to use as an index                    |
|                2. PTAGTABLE - Ptr to the tag table                         |
|  Returns:      PSZ - ptr to the tag string in the tagtable                 |
|  Comments:                                                                 |
|  Samples:      pszAttr = LPGetAttributeString(ID_ATTR,                     |
|                                               pLPData->pTagDef);           |
+---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
|  Name:         cmpUSHORT                                                   |
|  Purpose:      Compare two USHORTS - for the library sort routine          |
|  Parameters:   1. PUSHORT - first number to compare                        |
|                2. PUSHORT - second number to compare                       |
|  Returns:      int - negative if e1<e2, positive if e1>e2, 0 if e1=e2      |
|  Comments:                                                                 |
+---------------------------------------------------------------------------*/
static int cmpUSHORT( const void *e1, const void *e2)
{
   if ( (*(PUSHORT)e1) == (*(PUSHORT)e2) )
      return 0;
   else if ((*(PUSHORT)e1) > (*(PUSHORT)e2))
           return 1;
        else
           return -1;
}

/*---------------------------------------------------------------------------+
|  Name:         LPOutContextReferences                                      |
|  Purpose:      Output a list of context references                         |
|  Parameters:   1. PLPDATA - Ptr to LP data area                            |
|                2. PBOOL - Ptr to termination variable                      |
|  Returns:      BOOL - TRUE if successful, FALSE otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutContextReferences(PlpData, pfTerminate);         |
+---------------------------------------------------------------------------*/
static BOOL LPOutContextReferences(PLPDATA pLPData, PBOOL pfTerminate)
{
   BOOL fOK = TRUE;                          // Error Status variable
   CHAR szBuffer[USHORT_MAX_LEN];  // Buffer for converted reference number
   USHORT i;
   CHAR_W szTempW[USHORT_MAX_LEN];

   if (pLPData->usReferencesInBuffer != 0)
      {
      // Sort the buffer first
      qsort((PVOID) pLPData->pusSortBuffer,
            (size_t) pLPData->usReferencesInBuffer,
            sizeof(USHORT),
            cmpUSHORT);

      for (i=0; i<pLPData->usReferencesInBuffer; i++)
         {
           itoa(pLPData->pusSortBuffer[i], szBuffer, RADIX_10);
           ASCII2UnicodeBuf( szBuffer, &szTempW[0], USHORT_MAX_LEN, pLPData->ulSrcOemCP );


         fOK = LPOutStatement(
                        pLPData->pCBList,      // Output control block
                        pLPData->pTagDef,      // ptr to the tag table
                        CONREF_TAG,            // open Tag index
                        ECONREF_TAG,           // close Tag index
                        szTempW,               // Short converted to string
                        0,                     // No attributes
                        NULL,                  // No attribute list
                        pfTerminate,           // ptr to termination variable
                        2, pLPData->pListFormatTable->pTagNamesW);       // Indent level

         if (!fOK)
            break;
         }
      }

   return(fOK);
}

/*---------------------------------------------------------------------------+
|  Name:         LPOutContext                                                |
|  Purpose:      Output context data or the reference to it                  |
|  Parameters:   1. PLPSEG - ptr to the context data to be output            |
|                2. PLPDATA - ptr to List Processing data record             |
|                3. PBOOL - Pointer to fTerminate variable                   |
|  Returns:      BOOL - True if successful, else FALSE (output errors)       |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutContext(pContext, pLPData, &fTerminate);         |
+---------------------------------------------------------------------------*/
static BOOL LPOutContext(PLPSEG pContext, PLPDATA pLPData, PBOOL pfTerminate)
{
   BOOL fOK = TRUE;                          // Error Status variable
   BOOL fFound = FALSE;
   USHORT usContextRef = 0;        // Local Context Reference Number
   USHORT usRefCount;              // copied from pLPData->usReferencesInBuffer;
   USHORT usBufSize;               // copied from pLPData->usSortBufferSize
   CHAR szBuffer[USHORT_MAX_LEN];  // Buffer for converted reference number
   CHAR szBuffer_help[USHORT_MAX_LEN];  // Buffer for converted reference number
   CHAR_W szBufferW[USHORT_MAX_LEN];   // Unicode buffer
   PSZ_W pszBuffer = szBufferW;       // PSZ to buffer (needed for ptr to ptr to
                                   //                attribute list)
   PLPCTXTREF pContextRef;         // Search node

   // Scan already output contexts for new context
   pContextRef = LPContextRefSearch(pLPData, pContext);
   fFound = (pContextRef != NULL);

   if (fFound)
   {
      usContextRef = pContextRef->usRef;
   }
   else       // If not found then get the context and compare to last
   {
      // Get the context from the appropriate input file
      fOK = LPGetContext(pContext,                 // Context data ptr
                         pLPData->szWorkSegment,   // Output buffer for context
                         (PSZ)(pLPData->szSSourcePath),    // source path name
                         &(pLPData->pLstFCB),      // ptr to ptr to List of open files
                         pfTerminate);             // ptr to termination variable

      if (fOK)
      {
         if (UTF16stricmp(pLPData->szWorkSegment, pLPData->szLastSegment) == 0)

         {
            fFound = TRUE;
            usContextRef = (USHORT) pLPData->pContextRefs->ulCount; // Last ref. number
         }
         else
            UTF16strcpy(pLPData->szLastSegment, pLPData->szWorkSegment);
         }
      }

#define SORT_BUFFER_INCREMENT 100

   if (fOK && fFound)               // then Output the context reference
      {
      usRefCount = pLPData->usReferencesInBuffer;
      usBufSize = pLPData->usSortBufferSize;

      if (usRefCount == usBufSize) // Increase the size of the sort buffer
         {
         fOK = UtlAlloc( (PVOID *) &(pLPData->pusSortBuffer),
                        (LONG) usBufSize * sizeof(USHORT),
                        (LONG) (usBufSize + SORT_BUFFER_INCREMENT) *
                                sizeof(USHORT),
                        ERROR_STORAGE);
#if defined(LOG_ALLOCS)
        lAllocs++;
        lAllocSize += (usBufSize + SORT_BUFFER_INCREMENT) * sizeof(USHORT);
#endif

         pLPData->usSortBufferSize += SORT_BUFFER_INCREMENT;

         if (!fOK)
            *pfTerminate = TRUE;
         }

      if (fOK)      // Add the context reference to the sort buffer
      {
         pLPData->pusSortBuffer[usRefCount] = usContextRef;
         pLPData->usReferencesInBuffer ++;
      }
   }

   if (fOK && !fFound)              // else output a New context
      {                             // so Create a new context reference
      fOK = LPContextRefInsert(pLPData, pContext, pfTerminate);

      if (fOK)        // Create the new context's reference number attribute
         {
         sprintf(szBuffer,     // Add 'u' onto "ID=%"
                 "%su",
                 LPGetAttributeString(ID_ATTR, pLPData->pTagDef));

         sprintf(szBuffer_help,     // Update the %u with ulCount
                 szBuffer,
                 pLPData->pContextRefs->ulCount);
         strcpy(szBuffer, szBuffer_help);
         ASCII2UnicodeBuf(szBuffer, szBufferW, USHORT_MAX_LEN , pLPData->ulSrcOemCP);

         // Output the context
         fOK = LPOutStatement(
                        pLPData->pCBList,          // Output control block
                        pLPData->pTagDef,          // ptr to the tag table
                        CONTEXT_TAG,               // open Tag index
                        ECONTEXT_TAG,              // close Tag index
                        pLPData->szWorkSegment,    // Tag value (Context)
                        1,                         // 1 attribute
                        &pszBuffer,                // attribute list (pt to szBuffer)
                        pfTerminate,               // ptr to termination variable
                        2, pLPData->pListFormatTable->pTagNamesW);          // Indent level
         }
      }

   return fOK;
}


/*---------------------------------------------------------------------------+
|  Name:         LPGetContext                                                |
|  Purpose:      Read from file the data for a context                       |
|  Parameters:   1. PLPSEG - ptr to the context data                         |
|                2. PSZ - pointer to the buffer area                         |
|                3. PLPFCB - pointer to list of open files CBs               |
|                4. PBOOL - pointer to the termination variable              |
|  Returns:      BOOL - True if successful, else FALSE                       |
|  Comments:                                                                 |
|  Samples:      fOK = LPGetContext(pLPSeg, szBuffer, pSourcePath,           |
|                                   pLstFCB, &fTerminate);                   |
+---------------------------------------------------------------------------*/
static BOOL LPGetContext(PLPSEG pLPSeg, PSZ_W pszBuffer, PSZ pSourcePath,
                         PLPFCB *ppLPFCB, PBOOL pfTerminate)
{
   PLPFCB pFCBIndex = *ppLPFCB;     // ptr to first file in list of
                                    // open files
   PLPFCB pFCBLast = NULL;          // Used to point to last FCB
   USHORT usRC;                     // Return variable for Utl functions
   USHORT usActionTaken;            // Returned from UtlOpen
   BOOL fOK = TRUE;                 // error Status variable
   ULONG ulFilePos;                 // Needed for UtlChgFilePtr
   ULONG  ulBytesRead;              // Needed for UtlRead
   CHAR  szFileName[MAX_EQF_PATH];  // full name of file to be opended
   PSZ  pszFileName = szFileName;   // Ptr to full filename to be opened

   // search the file in the opened file list
   while (pFCBIndex != NULL)
      {
      // if pointers are equal then strings are equal
      if (pFCBIndex->pszFileName == pLPSeg->pszFileName)  // The file is open
         {
         break;
         }
      else                     // Not the correct file
         {
         pFCBLast = pFCBIndex;         // Save previous position in list -
                                       // in case it is the last
         pFCBIndex = pFCBIndex->pNext; // Get next in list
         }
      }

   if (pFCBIndex == NULL) // i.e. file not open already
     {
     fOK = UtlAlloc( (PVOID *) &pFCBIndex, 0L,(LONG) sizeof(LPFCB), ERROR_STORAGE);
#if defined(LOG_ALLOCS)
        lAllocs++;
        lAllocSize += sizeof(LPFCB);
#endif

     if (!fOK)
       *pfTerminate = TRUE;

     if (fOK)        // Allocation successful - Set the variables in the element
       {
       pFCBIndex->pszFileName = pLPSeg->pszFileName;
       pFCBIndex->pNext = NULL;

       // create complete filename
       sprintf( szFileName,
                PATHCATFILE,
                pSourcePath,
                pFCBIndex->pszFileName);

       // Open the file
       usRC = UtlOpen(szFileName,           // Full filename
                      &(pFCBIndex->hFile),  // new file handle
                      &usActionTaken,
                      0L,
                      FILE_NORMAL,          // Normal attribute
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_SHARE_DENYWRITE, // Deny Write access
                      0L,                   // Reserved
                      TRUE );              // no error handling

       fOK = (usRC == 0);

       if (!fOK)          // File open failed
          {
          // At this stage we try to close any currently open context files
          // and try again
          fOK = LPCloseFiles(ppLPFCB);

          if (fOK) // No errors closing files - try opening again
             {
             usRC = UtlOpen(szFileName,           // Full filename
                            &(pFCBIndex->hFile),  // new file handle
                            &usActionTaken,
                            0L,
                            FILE_NORMAL,          // Normal attribute
                            OPEN_ACTION_OPEN_IF_EXISTS,
                            OPEN_SHARE_DENYWRITE, // Deny Write access
                            0L,                   // Reserved
                            TRUE );              // no error handling

             fOK = (usRC == 0);
             }
          }

       // If UtlOpen still fails then set fTerminate
       if (!fOK)            // File open failed
         {
//       UtlError(ERROR_OPENING_CONTEXT_FILE,
//                MB_CANCEL,
//                1,
//                &pszFileName,
//                EQF_ERROR);
         *pfTerminate = TRUE;
         }
       }

     if (fOK)               // Link the element in the list
        {
        if (*ppLPFCB == NULL)          // If no entries in list already
           *ppLPFCB = pFCBIndex;       // Make this entry the first in the list
        else
          pFCBLast->pNext = pFCBIndex; // Link up with last in list
        }
     } /* ENDIF file not already open */

   //
   //  At this stage the file was found to be open or else was opened above.
   //  In any case: if (fOK) then pFCBIndex points to the file required.
   //

   if (fOK)              // Seek for the position of the context
     {
     usRC = UtlChgFilePtr(pFCBIndex->hFile,
                          pLPSeg->ulPosition, // Position to go to
                          SEEK_SET,           // Seek from start of file
                          &ulFilePos,         // New file position
                          FALSE);             // Do not handle errors

     fOK = ((usRC == 0) && (pLPSeg->ulPosition == ulFilePos));

     if (fOK && (pLPSeg->usLength > MAX_SEGMENT_SIZE))
        {  // Before reading, make sure segment length <= MAX_SEGMENT_SIZE
        fOK = FALSE;
        UtlError(ERROR_READING_CONTEXT,  // not readable file!!!!!
                 MB_CANCEL,
                 1,
                 &pszFileName,
                 EQF_ERROR);
        *pfTerminate = TRUE;
        }

     if (fOK)           // Read the context
     {
        usRC = UtlReadL(pFCBIndex->hFile,
                       pszBuffer,         // Buffer for context
                       (USHORT)(pLPSeg->usLength * sizeof(CHAR_W)),  // Bytes to read
                       &ulBytesRead,      // Actual bytes read
                       FALSE);            // Do not handle errors

        fOK = ((usRC == 0) && (pLPSeg->usLength * sizeof(CHAR_W) == ulBytesRead));

        if (fOK)        // Set null terminator
        {
           pszBuffer[ulBytesRead/sizeof(CHAR_W)] = '\0';
        }
     }

     if (!fOK && !(*pfTerminate))  // File seek or read error
       {
       UtlError(ERROR_READING_CONTEXT,     // not readable file!!!!
                     MB_CANCEL,
                     1,
                     &pszFileName,
                     EQF_ERROR);
       *pfTerminate = TRUE;
       }
     }

   return fOK;
}



/*---------------------------------------------------------------------------+
|  Name:         LPOutStatement                                              |
|  Purpose:      Output an SGML statement (starttag - value/attrs - endtag)  |
|  Parameters:   1. PBUFCB - Ptr to the output file IO control block       |
|                2. PTAGTABLE - the tag table to extact the tags from        |
|                3. LISTTAGS - the start tag index                           |
|                4. LISTTAGS - the end tag index                             |
|                5. PSZ - ptr to the tag value                               |
|                6. USHORT - number of attributes                            |
|                7. PSZ * - Ptr to ptr to list of attributes                 |
|                8. PBOOL - ptr to termination variable                      |
|                9. USHORT - the no. of characters to indent                 |
|  Returns:      BOOL - True if Error free                                   |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutStatement(pCBList, pLPData->pTagDef,             |
|                               CONTEXT_TAG, ECONTEXT_TAG, szWorkContext,    |
|                               0, NULL, pfTerminate, 2);                    |
+---------------------------------------------------------------------------*/
static BOOL LPOutStatement(
                     PBUFCB pIOCB,
                     PTAGTABLE pTagTable,
                     LISTTAGS openTag,
                     LISTTAGS closeTag,
                     PSZ_W    pszTagValue,
                     USHORT   usAttributeCount,
                     PSZ_W    *ppAttributeList,
                     PBOOL    pfTerminate,
                     USHORT   usIndent,
                     PSZ_W    pTagNamesW)
{
    BOOL fOK = TRUE;       // Error Status Variable
    BOOL fWriteOK = TRUE;  // UtlBufWrite Status Variable
//    PSZ pszOutputString;   // Only used if error (get rid of errormessage)

    // Output the start tag
    fOK = LPOutTag(pIOCB,
                   pTagTable,             // ptr to the tag table
                   openTag,               // Tag index
                   usAttributeCount,      // Number of attributes
                   ppAttributeList,
                   pfTerminate,
                   usIndent,
                   pTagNamesW);

    if (fOK)                // Write the tag value
       fOK = fWriteOK = (UtlBufWriteW(pIOCB, pszTagValue,
                                     (USHORT)UTF16strlenBYTE(pszTagValue), TRUE) == NO_ERROR);

   if (fOK)                 // Output the Close tag
      fOK = LPOutTag(pIOCB,
                     pTagTable,   // ptr to the tag table
                     closeTag,    // Tag index
                     0,           // No attributes
                     NULL,
                     pfTerminate,
                     0, pTagNamesW);               // No indent

   if (fOK)     // if no tag in list get new line
     fOK = fWriteOK = LPNewLine(pIOCB);

   // If any writes in this function failed
   if (!fWriteOK)
      {
//    pszOutputString = szEndChar;
//
//    UtlError(ERROR_WRITING_TAG_TO_LIST_FILE,
//             MB_CANCEL,
//             1,
//             &(pszOutputString),
//             EQF_ERROR);
      *pfTerminate = TRUE;
      }

   return fOK;
}


/*---------------------------------------------------------------------------+
|  Name:         LPOutTag                                                    |
|  Purpose:      Output a tag with it's attributes/values                    |
|  Parameters:   1. PBUFCB - Ptr to the output file IO control block       |
|                2. PTAGTABLE - the tag table to extact the tags from        |
|                3. LISTTAGS - the index of the tag to output                |
|                4. USHORT - number of attributes                            |
|                5. PSZ * - Ptr to ptr to list of attributes                 |
|                6. PBOOL - ptr to termination variable                      |
|                7. USHORT - Indent level                                    |
|  Returns:      BOOL - True if Error free                                   |
|  Comments:                                                                 |
|  Samples:      fOK = LPOutTag(pLPData->pCBList, pLPData->pTagDef,          |
|                  CONTEXT_TAG, 1, &pAttributeList, pfTerminate, 2);         |
+---------------------------------------------------------------------------*/
static BOOL LPOutTag(PBUFCB pIOCB,
                     PTAGTABLE pTagTable,
                     LISTTAGS tag,
                     USHORT   usAttributeCount,
                     PSZ_W    *ppAttributeList,
                     PBOOL    pfTerminate,
                     USHORT   usIndent,
                     PSZ_W    pTagNamesW)
{
   BOOL fOK;            // Error status variable
   USHORT i;            // for loop index
   PSZ_W pszOutputString; // General output string

   fOK = LPIndent(pIOCB, usIndent);

   if (fOK)               // Output the tag string
      {
      pszOutputString = LPGetTagString(tag, pTagTable, pTagNamesW);

      if (pszOutputString)
         fOK = (UtlBufWriteW(pIOCB, pszOutputString, (USHORT)UTF16strlenBYTE(pszOutputString),
                            TRUE ) == NO_ERROR );
      else
         fOK = FALSE;
      }

   if (fOK)               // Output each attribute
      {
      for (i = 0; (i < usAttributeCount) && fOK; i++)
          {
          fOK = (UtlBufWriteW(pIOCB, L" ", sizeof(CHAR_W), TRUE ) == NO_ERROR );

          if (fOK)
             {
             fOK = (UtlBufWriteW(pIOCB,
                               ppAttributeList[i],
                               (USHORT)UTF16strlenBYTE( ppAttributeList[i]),
                               TRUE ) == NO_ERROR );
             }
          }
      }

   if (fOK)               // Output the end delimiter
      {
      pszOutputString = szEndChar;

      if (pszOutputString)
         fOK = ( UtlBufWriteW(pIOCB, pszOutputString,
                 (USHORT)UTF16strlenBYTE(pszOutputString), TRUE) == NO_ERROR );
      else
         fOK = FALSE;
      }

   // manage errors in any of the calls above
   if (!fOK)
      {
         *pfTerminate = TRUE;
      }

   return fOK;
}



/*---------------------------------------------------------------------------+
|  Name:         LPContextUpd                                                |
|  Purpose:      If context not in list then add it                          |
|  Parameters:   1. PTAINPUT - ptr to TAINPUT struct (for fTerminate)        |
|                2. PLPCTXT * - ptr to ptr to the list of contexts           |
|                3. PLPSEG a context structure                               |
|  Returns:      BOOL - True if Error free                                   |
|  Comments:                                                                 |
|  Samples:      fOK = LPContextUpd(pTAInput, &pCntxtLst, pContext);         |
+---------------------------------------------------------------------------*/
static BOOL LPContextUpd(PTAINPUT pTAInput, PLPCTXT *ppCntxtLst,
                         PLPSEG pContext)
{
    PLPCTXT pOldHead = *ppCntxtLst; // Copy of ptr to head of context list
    BOOL fFound = FALSE;
    BOOL fOK = TRUE;

    // if No contexts in list already
    if (pOldHead != NULL)
      {
      // Check if the context is in the list already (last inserted context)
      fFound = (memcmp(pContext, &(pOldHead->context), sizeof(LPSEG)) == 0);
      }

    // If not found then
    if (!fFound)
      {
      // Allocate the new structure, putting it at the head of the list
      fOK = UtlAlloc( (PVOID *) ppCntxtLst,
                     0L,
                     (LONG) sizeof(LPCTXT),
                     ERROR_STORAGE);
#if defined(LOG_ALLOCS)
        lAllocs++;
        lAllocSize += sizeof(LPCTXT);
#endif

      if (!fOK)
         pTAInput->pInD->fTerminate = TRUE;

      if (fOK)
         {
         // Link to the old head of list - OK if NULL
         (*ppCntxtLst)->pNext = pOldHead;

         // fill the structure with the noe context details
         memcpy(&((*ppCntxtLst)->context), pContext, sizeof(LPSEG));
         }
      }

    return fOK;
}


