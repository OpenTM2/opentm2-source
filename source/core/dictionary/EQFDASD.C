   #define ASD_USE_COMPOUNDS_IN_AUT_LOOKUP   // activates compound handling
// #define USE_COMPOUND_LIST           // use index compound list
// #define MEASURETIME                 // if defined time measurements are taken
//+----------------------------------------------------------------------------+
//|  EQFDASD.C  - ASD layer of Dictionary Services                             |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//|     --------- Nlp interface functions with index processing -------------  |
//|     AsdBegin      - Begin/Initialize ASD services (NlpBegAsd)              |
//|     AsdEnd        - Terminate ASD services (NlpEndAsd)                     |
//|     AsdOpen       - Open a dictionary and its associated dictionaries and  |
//|                     index files (NlpOpenAsd)                               |
//|     AsdClose      - Close dictionaries opened by AsdOpen (NlpCloseAsd)     |
//|     AsdInsEntry   - Insert a new entry into a dictionary (NlpInsEntryAsd)  |
//|     AsdRepEntry   - Replace a dictionary entry (NlpUpdEntryAsd)            |
//|     AsdDelEntry   - Delete a dictionary entry (NlpDelEntryAsd)             |
//|     --------------- 1:1 Nlp interface calls -----------------------------  |
//|     AsdFndBegin   - Asd interface for NlpFndBeginAsd                       |
//|     AsdFndEquiv   - Asd interface for NlpFndEquivAsd                       |
//|     AsdFndMatch   - Asd interface for NlpFndMatchAsd                       |
//|     AsdFndNumber  - Asd interface for NlpFndNumberAsd                      |
//|     AsdNxtTerm    - Asd interface for NlpNxtTermAsd                        |
//|     AsdPrvTerm    - Asd interface for NlpPrvTermAsd                        |
//|     AsdRenumber   - Asd interface for NlpRenumberAsd                       |
//|     AsdRetEntry   - Asd interface for NlpRetEntryAsd                       |
//|     ---------------------------------------------------------------------  |
//|     AsdBuild      - Create a new dictionary                                |
//|     AsdDelete     - Delete a dictionary (dictionary must be closed!)       |
//|     AsdDeleteRemote-Delete a remote dictionary                             |
//|     AsdRename     - Rename a dictionary (dictionary must be closed!)       |
//|     AsdRenameRemote-Rename a remote dictionary                             |
//|     AsdRetPropPtr - Get pointer to properties for an open dictionary       |
//|     AsdRetSigSize - Return size of signature record                        |
//|     AsdRetSigPtr  - Return ptr to signature record                         |
//|     AsdRetSigRec  - Return signature record                                |
//|     AsdHandleFromDCB    - Get dictionary handle from a DCB                 |
//|     AsdIsUcbOK          - Check if an UCB is OK                            |
//|     AsdIsDcbOK          - Check if an DCB is OK                            |
//|     AsdQueryDictName    - Return name of dictioanry (w/o path and extension|
//|     AsdRetDictList      - Return list of associated dictionaries           |
//|     AsdRetUserHandle    - Return the user handle supplied by NlpBegin      |
//|     AsdRetServiceHandle - Return the handle supplied by NlpBegService      |
//|     AsdRetMorphHandle   - Return the handle of a morphology dictionary     |
//|     ---------------------------------------------------------------------  |
//|     AsdTranslate  - Lookup all translations for all terms in a segment.    |
//|     AsdEntryChange - Correct internal lookup tables after entry changed.   |
//|                                                                           |
//|  +-- status ('H'= Header, 'D'=Design, 'C'=Code, 'T'=Test, ' '=complete,    |
//|              'Q'=Quick-and-dirty )                                         |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//|     UtlAlloc                - General memory allocation routine            |
//|     NlpBegService           - Initialize Nlp services                      |
//|     NlpEndService           - Terminate Nlp services                       |
//|     NlpActDict              - activate a dictionary                        |
//|     NlpDeactDict            - deactivate a dictionary                      |
//|     NlpMorphID              - morphological identification of a term       |
//|     NlpBegAsd               - Initialize an ASD user                       |
//|     NlpEndAsd               - Terminate an ASD user                        |
//|     NlpOpenAsd              - Open an ASD dictionary                       |
//|     NlpCloseAsd             - Close an ASD dictionary                      |
//|     NlpRetSignature         - Read signature record of an ASD dictionary   |
//|     NlpSetCodePage          - Set Nlp services code page                   |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//|     ---------------------------------------------------------------------  |
//|  Q  AsdLoadDictProperties   - load dictionary property file                |
//|     AsdGetStemForm          - get the stem form of a term                  |
//|     AsdAddToIndexData       - add a term to the data part of an index entry|
//|     AsdDelFromIndexData     - delete a term from the data part of an index |
//|     AsdLocInIndex           - locate a term in the data part of an index   |
//|     AsdExtractTerms         - extract terms from an dictionary data        |
//|     AsdGetIndexEntry        - read index entry for a given term            |
//|     AsdInsIndexEntry        - insert a new entry into the index dictionary |
//|     AsdPutIndexEntry        - rewrite an index entry for a given term      |
//|     AsdDelIndexEntry        - delete an index entry                        |
//|     AsdIsIndexEmpty         - checks if an index entry is empty            |
//|     AsdInsIndexEntry        - insert a new entry into the index dictionary |
//|     AsdUpdateIndex          - update the index dictionary                  |
//|     AsdMakeIndexEntry       - create an empty index entry                  |
//|     AsdAddToTaskList        - add a new task to a task list                |
//|  Q  AsdIsMWT                - check if a given term is a multi word term   |
//|     AsdGetNextCode          - gets the next node code                      |
//|     AsdHandleToDCB          - convert a Nlp dictionary handle to a DCB     |
//|     AsdAddToTermList        - add a new term to a term list                    |
//|                                                                           |
//|  +-- status ('H'= Header, 'D'=Design, 'C'=Code, 'T'=Test, ' '=complete,    |
//|              'Q'=Quick-and-dirty )                                         |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|     AsdOpen: name of morphological dictionary is hardcoded to US.DC2.      |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFQDAMI.H"             // fastdam header file
#include "EQFRDICS.H"             // for CheckPropCompatibility function

#define ASDDEFAULTS               // tells EQFDASDI.H to add defaults
#include "EQFDASDI.H"             // internal ASD services header file
#include "OtmDictionaryIF.H"
#include "eqfevent.h"             // event logging facility

// undefine the Control Block for communications -- we might have used
// the same typedef for dictionary control block
   #undef DCB


void DAMLINK
QDamOpenAsd( PUCHAR,                /* in  - name of dictionary  */
             PUCHAR,                /* server name                    */
             USHORT,                /* in  - number of pages     */
             USHORT,                /* in  - immediate writing?  */
             USHORT,                /* in  - user handle         */
             PUSHORT,               /* out - dictionary handles  */
             PUSHORT,               /* out - length of user area */
             PUSHORT);              /* out - return code         */

//extern DAM2QDAM DamRec[];              // DAM to QDAM table of DAMQDAM.C
USHORT DamBTreeRc ( SHORT sRc );       // QDAM to DAM rc conversion

#define DEF_LANG_PROP "english\x015US.DC2\x0156011\x15 \x15"

// convert old type index records if necessary and get record size
USHORT AsdPrepareIndexRecord
( 
  PBYTE       *ppucData,               // ptr to index data buffer pointer
  PULONG      pulSize,                 // ptr to size of index data buffer
  PULONG      pulUsed                  // ptr to # of bytes used in index data buffer
);

// compute checksum of USHORT offset array
LONG AsdBuildHeaderCheckSum
(
  PASDINDEXENTRYHEADER pHeader
);

// make index entry compatible to older versions of Tmgr
USHORT AsdMakeCompatibleRecord
( 
  PBYTE       pucData,                 // ptr to index data buffer
  ULONG       ulUsed                   // # of bytes used in index data buffer
);


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdTermList                                              |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       will return a list with the number of requested terms    |
//|                   (if available) for single and associated dictionary      |
//|                   handles.                                                 |
//|                   The returned list is an ASCII-Z list.                    |
//|                   The list won't be sorted, because this will be done      |
//|                   automatically by PM.                                     |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//|    HUCB     hUCB            IN          user control block handle          |
//|    PSZ      pszTerm         IN          term to start with                 |
//|    USHORT  usNumTerms       IN          number of terms to be looked up    |
//|    USHORT  usAction         IN          required action                    |
//|    PUCHAR  pucBuffer        IN/OUT      pointer to buffer                  |
//|    USHORT  usLen            IN          length of provided buffer          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
USHORT AsdTermList
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm,                   // term to be started with
   USHORT   usNumTerms,                // number of terms to be looked up
   USHORT   usAction,                  // requested action
   PSZ_W    pucBuffer,                 // pointer to buffer for term list
   USHORT   usLen                      // len of term list buffer (# charw's)
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // Get term list
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      NlpTermListW( pUCB->usUser,       // ASD user handle
                   pDCB->usDictHandle, // dictionary handle
                   pszTerm,            // term to be deleted
                   usNumTerms,         // number of terms to be looked up
                   usAction,           // requested action
                   pucBuffer,          // buffer for term list
                   usLen,              // size of buffer (# charw's)
                   &usNlpRC );
   } /* endif */

   return( usNlpRC );

} /* end of AsdTermList */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdWildCardList                                          |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       will return a list with the number of terms following the|
//|                   given pattern or a list of terms having the given        |
//|                   compound (type of search is controlled by fCompound flag)|
//|                   The returned list is an ASCII-Z list.                    |
//|                   The list won't be sorted, because this will be done      |
//|                   automatically by PM.                                     |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//|    HUCB     hUCB            IN          user control block handle          |
//|    PSZ      pszStartTerm    IN          term to be started with            |
//|    PSZ      pszPattern      IN          search pattern or compound         |
//|    USHORT   usNumTerms      IN          number of terms to be looked up    |
//|    BOOL     fCompound       IN          search-for-compounds flag          |
//|    PUCHAR   pucBuffer       IN/OUT      pointer to buffer                  |
//|    USHORT   usLen           IN          length of provided buffer          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
USHORT AsdWildCardList
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszStartTerm,              // term to be started with
   PSZ_W    pszPattern,                // search pattern or compound
   USHORT   usNumTerms,                // number of terms to be looked up
   BOOL     fCompound,                 // search-for-compounds flag
   PSZ_W    pucBuffer,                 // pointer to buffer for term list
   USHORT   usLen                      // length of term list buffer
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // Get matching terms
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usNlpRC = QDamWildCardList(
                        pUCB->usUser,       // ASD user handle
                        pDCB->usDictHandle, // dictionary handle
                        pszStartTerm,       // term to start with
                        pszPattern,         // search pattern
                        usNumTerms,         // number of terms to be looked up
                        fCompound,          // compound search flag
                        pucBuffer,          // buffer for term list
                        usLen );            // size of buffer
   } /* endif */

   return( usNlpRC );

} /* end of AsdWildCardList */

//+----------------------------------------------------------------------------+
//| AsdBegin            Begin/Initialize ASD services (NlpBegAsd+NlpBegService)|
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    This function initializes ASD dictionary services (and Nlp services).   |
//|    It must be called before any other ASD functions is called.             |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    Allocate user control block and services reply area;                    |
//|    if ok then                                                              |
//|       begin Asd services using NlpBegAsd                                   |
//|    endif;                                                                  |
//|    if ok then                                                              |
//|       begin Nlp services using NlpBegService                               |
//|    endif;                                                                  |
//|    if ok then                                                              |
//|       complete user control block data                                     |
//|    endif;                                                                  |
//|    if not ok then;                                                         |
//|       end all pending services; free all allocated memory;                 |
//|    endif;                                                                  |
//|    set caller's user control block handle;                                 |
//|    return RC;                                                              |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    USHORT usMaxDicts      IN    maximum number of dictionaries (w/o index!)|
//|    PHUCB  phUCB           OUT   user control block handle                  |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD    if OK                              |
//|                         LX_xxx          Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
USHORT AsdBegin
(
   USHORT usMaxDicts,                  // maximum number of dictionaries
   PHUCB  phUCB                        // ptr to user control block handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call
   PUCB      pUCB = NULL;              // pointer to user control block
   BOOL      fOK;                      // ok flag set by UtlAlloc call
   #if defined(MEASURETIME)
      SEL    selGlobSeg, selLocalSeg;  // selectors returned by DosGetInfoSeg
   #endif

   //
   // allocate user control block and services reply area
   //
   fOK = UtlAlloc( (PVOID *)&pUCB, 0L, (LONG) max(sizeof(UCB),MIN_ALLOC), NOMSG );

   //
   // begin ASD services
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      NlpBegAsd( (USHORT)((usMaxDicts+1) * 2),   // max = dicts plus index plus associates
                 0,                    // no Nlp index dictionaries are used
                 2,                    // only two associations per user
                 usMaxDicts,           // association depth = number of dicts
                 &pUCB->usUser,        // address of user handle
                 &usNlpRC );           // address of return code variable
   } /* endif */

   //
   // add DBCS info - not used any more (RJ02-07-25)
   //
   //if ( usNlpRC == LX_RC_OK_ASD )
   //{
   //  pUCB->fDBCS = AsdFillDBCSTable( pUCB->fisDBCS1 );
   //} /* endif */

   //
   // complete UCB
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      pUCB->usMaxDicts = usMaxDicts;
      pUCB->lSignature = UCB_SIGNATURE;
      #if defined(MEASURETIME)
         DosGetInfoSeg( &selGlobSeg, &selLocalSeg );
         pUCB->pInfoSeg = MAKEPGINFOSEG(selGlobSeg);
      #endif
   } /* endif */

   //
   // cleanup
   //
   if ( usNlpRC != LX_RC_OK_ASD )
   {
      if ( pUCB )
      {
         UtlAlloc( (PVOID *)&pUCB, 0L, 0L, NOMSG );
      } /* endif */
   } /* endif */

   *phUCB = (HUCB) pUCB;               // set caller's user control block handle

   return( usNlpRC );                  // return Nlp RC to caller

} /* end of AsdBegin */

//+----------------------------------------------------------------------------+
//| AsdEnd              Terminate ASD services (NlpEndAsd)                     |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    if user control block is ok then                                        |
//|       call NlpEndAsd;                                                      |
//|       if ok then                                                           |
//|          call NlpEndService;                                               |
//|       endif;                                                               |
//|       if ok then                                                           |
//|          free user control block and replay area;                          |
//|       endif;                                                               |
//|    else                                                                    |
//|       set RC to LX_BAD_USR_AREA;                                           |
//|    endif;                                                                  |
//|    return RC;                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB        IN       user control block handle                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD    if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdEnd
(
   HUCB  hUCB                          // user control block handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call
   PUCB      pUCB;                     // pointer to user control block
   #if defined(MEASURETIME)
      FILE *hTimeLog;                  // handle of time log
   #endif

   pUCB = (PUCB) hUCB;                 // convert handle to pointer

   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   #if defined(MEASURETIME)
      if ( usNlpRC == LX_RC_OK_ASD )
      {
         hTimeLog = fopen( "\\TIME.LOG", "a" );
         fprintf( hTimeLog, "==========================================\n");
         fprintf( hTimeLog, "AsdInsEntry\AsdRepEntry/AsdDelEntry times:\n");
         fprintf( hTimeLog, "%10ld ms for base dictionary updates\n",
                  pUCB->ulBaseUpdTime );
         fprintf( hTimeLog, "%10ld ms for index updates\n",
                  pUCB->ulIdxUpdTime );
         fprintf( hTimeLog, "%10ld ms for index update preparation\n\n",
                  pUCB->ulPrepUpdTime );
         fprintf( hTimeLog, "AsdTranslate times:\n");
         fprintf( hTimeLog, "%10ld ms for atom manager calls\n",
                  pUCB->ulTransAtomTime );
         fprintf( hTimeLog, "          (%d terms looked up)\n",
                  pUCB->ulTransTerms );
         fprintf( hTimeLog, "%10ld ms for stem form reductions\n",
                  pUCB->ulTransStemRedTime );;
         fprintf( hTimeLog, "          (%d stem reductions)\n",
                  pUCB->ulTransStemRed );;
         fprintf( hTimeLog, "%10ld ms for list index function\n",
                  pUCB->ulTransListIndexTime );
         fprintf( hTimeLog, "          (%d list index calls)\n",
                  pUCB->ulTransListIndex );
         fprintf( hTimeLog, "%10ld ms for create term list\n",
                  pUCB->ulTransListTime );
         fprintf( hTimeLog, "%10ld ms for NlpFindxxx in base dictionary\n",
                  pUCB->ulTransNlpFindTime );
         fprintf( hTimeLog, "          (%d find calls)\n",
                  pUCB->ulTransNlpFind );
         fprintf( hTimeLog, "%10ld ms for NlpRetEntry for base dictionary\n",
                  pUCB->ulTransNlpRetTime );
         fprintf( hTimeLog, "          (%d ret calls)\n",
                  pUCB->ulTransNlpRet );
         fprintf( hTimeLog, "%10ld ms for search for MWTs\n",
                  pUCB->ulTransMWTTime );
         fprintf( hTimeLog, "%10ld ms for normalize input segment\n",
                  pUCB->ulTransNormTime );
         fprintf( hTimeLog, "%10ld ms for AsdTokenize\n",
                  pUCB->ulTransTokTime );
         fprintf( hTimeLog, "%10ld ms for other times in AsdTranslate\n",
                  pUCB->ulTransOtherTime );
         fprintf( hTimeLog, "\nActual data length is        %10ld\n",
                  pUCB->ulPayload );
         fprintf( hTimeLog, "\nLDB data length is           %10ld\n",
                  pUCB->ulDataLen );
         fprintf( hTimeLog, "\nNew LDB data length would be %10ld\n",
                  pUCB->ulNewDataLen );
         fprintf( hTimeLog, "\nAverage payload per entry is %10ld %%\n",
                  ( pUCB->ulDataLen ) ?
                     (pUCB->ulPayload * 100L / pUCB->ulDataLen ) : 0 );

         fclose( hTimeLog );
      } /* endif */
   #endif

   if ( ASDOK(usNlpRC) )
   {
      NlpEndAsd( pUCB->usUser,         // user handle
                 &usNlpRC );           // address of return code variable

      if ( usNlpRC == LX_RC_OK_ASD )
      {
         if ( pUCB->fTransUsed )
         {
            WinDestroyAtomTable( pUCB->hNotInBaseAtoms );
         } /* endif */
      } /* endif */

      if ( usNlpRC == LX_RC_OK_ASD )
      {
        if ( pUCB->pszTermList )
        {
          UtlAlloc( (PVOID *)&pUCB->pszTermList, 0L, 0L, NOMSG );
        } /* endif */
        if ( pUCB->pszDictTermList )
        {
          UtlAlloc( (PVOID *)&pUCB->pszDictTermList, 0L, 0L, NOMSG );
        } /* endif */
        if ( pUCB->pszOrgTermList )
        {
          UtlAlloc( (PVOID *)&pUCB->pszOrgTermList, 0L, 0L, NOMSG );
        } /* endif */
        if ( pUCB->pszCompList )
        {
          UtlAlloc( (PVOID *)&pUCB->pszCompList, 0L, 0L, NOMSG );
        } /* endif */
        UtlAlloc( (PVOID *)&pUCB, 0L, 0L, NOMSG);                // free UCB
      } /* endif */
   } /* endif */

   return( usNlpRC );                  // return Nlp RC to caller

} /* end of AsdEnd */

//+----------------------------------------------------------------------------+
//| AsdOpen             Open a dictionary and its associated dictionaries and  |
//|                     index files (NlpOpenAsd)                               |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    AsdOpen opens one or more dictionaries. For each of the dictionaries    |
//|    the name of the corresponding dictionary property file has to be        |
//|    specified. During the open the property file is loaded, the base        |
//|    and index dictionaries are openend and a dictionary control block (DCB) |
//|    is created, The returned DCB handle must be used in all calls referring |
//|    to this dictionary.                                                     |
//|    If more than one dictioary is specified, all specified dictionaries     |
//|    are associated and a DCB handle for the associated dictionary is        |
//|    returned.                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    while ok and not all dictioaries passed as arguments are processed;     |
//|       get next dictionary property name;                                   |
//|       load dictionary profile/properties;                                  |
//|       if ok then                                                           |
//|         synchronize local properties with remote ones                      |
//|       endif;                                                               |
//|       if ok then                                                           |
//|          open dictionary using NlpOpenAsd and get signature record;        |
//|       endif;                                                               |
//|       if ok then                                                           |
//|          open dictionary index using NlpOpenAsd;                           |
//|       endif;                                                               |
//|       if ok then                                                           |
//|          if first dictionary then                                          |
//|             open morphological dictionary;                                 |
//|          else                                                              |
//|            copy dictionary handle from previous DCB;                       |
//|          endif;                                                            |
//|       endif;                                                               |
//|    endwhile;                                                               |
//|    if ok then                                                              |
//|       if more than one dictionary was opended then                         |
//|          associate dictionaries using NlpAssocAsd;                         |
//|          if ok then;                                                       |
//|             associate index dictionaries using NlpAssocAsd;                |
//|          end;                                                              |
//|       endif;                                                               |
//|    endif;                                                                  |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB            IN          user control block handle          |
//|    USHORT   usOpenFlags     IN          open flags see EQFDASD.H           |
//|    USHORT   usNumDicts      IN          number of dictionaries in ppszDicts|
//|    PSZ      *ppszDicts      IN          ptr array with dictionary propertie|
//|    PHDCB    phDCB           OUT         dictionary control block handle    |
//|    PUSHORT  pusErrDict      OUT         number of failing dictionary       |
//|                                         usNumDicts + 1 = association failed|
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdOpen
(
   HUCB     hUCB,                      // user control block handle
   USHORT   usOpenFlags,               // open flags
   USHORT   usNumDicts,                // number of dictionaries in ppszDicts
   PSZ      *ppszDicts,                // ptr array with dictionary properties
   PHDCB    phDCB,                     // dictionary control block handle
   PUSHORT  pusErrDict                 // number of failing dictionary
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   USHORT    usDummyRC;                // used for 'don't care' return codes
   PUCB      pUCB;                     // pointer to user control block
   PDCB      pLastDCB = NULL;          // pointer to previous DCB
   PDCB      pFirstDCB = NULL;         // pointer to first DCB of DCB chain
   PDCB      pDCB = NULL;              // pointer to currently active DCB
   BOOL      fOK;                      // ok flag set by UtlAlloc call
   PSZ       pszCurDict;               // name of currently processed
                                       // dictionary property file
   DICTHANDLE ausDictHandle[2];         // buffer for returned dictionary handles
   USHORT    usOpenDicts = 0;           // number of openend dictionaries
   USHORT    usNumPages = 0;            // number of dictionary pages
   DICTHANDLE ausDictHandles[MAX_DICTS*2];  // list of dictionary handles
   DICTHANDLE ausIndexHandles[MAX_DICTS*2];                // list of index handles
   USHORT    usIndexDataLen;           // length of index signature record
   USHORT     usOpenRC = LX_RC_OK_ASD; // return code from NlpOpenAsd
                                                                /* 2@KIT1073A */
   BOOL       fPropsChanged = FALSE;   // 'remote props have been changed' flag
   CHAR       szDictName[MAX_FNAME];   // buffer for dictionary name

   pUCB = (PUCB) hUCB;                 // convert handle to pointer
   *pusErrDict = 0;                    // initialize caller's error dict

   DEBUGEVENT( ASDOPEN_LOC, FUNCENTRY_EVENT, usNumDicts );

   if ( pUCB && (pUCB->lSignature == UCB_SIGNATURE) ) // if UCB is ok ...
   {
      while ( usNumDicts &&                      // while ditionary name given
              (usNlpRC == LX_RC_OK_ASD) )        // and no error occured ...
      {
         pszCurDict = *ppszDicts;                          // get ptr to current name

         //
         //  create dictionary control block
         //
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            fOK = UtlAlloc( (PVOID *)&pDCB, 0L, (LONG) sizeof(DICTCB), NOMSG );
            if ( fOK )
            {
               pDCB->lSignature = DCB_SIGNATURE;        // set DCB identifier
               pDCB->usOpenFlags = usOpenFlags;         // remember open flags
               strcpy( pDCB->szPropName, pszCurDict );  // remember prop name
            }
            else
            {
               usNlpRC = LX_MEM_ALLOC_ASD;       // set short on memory RC
            } /* endif */
         } /* endif */

         //
         //  load dictionary property file
         //
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            usNlpRC = AsdLoadDictProperties( pszCurDict,   // load dictionary
                                             &pDCB->Prop );// property file
         } /* endif */

                                                               /* 38@KIT1073A */
         /*************************************************************/
         /* Synchronize local properties with remote ones             */
         /*************************************************************/
         if ( usNlpRC == LX_RC_OK_ASD )                         /* 2@KIT1183A */
         {
           pDCB->usLocation = pDCB->Prop.usLocation;

           switch ( pDCB->usLocation )
           {
             case LOC_LOCAL :
               /*******************************************************/
               /* Nothing to do                                       */
               /*******************************************************/
               fPropsChanged = FALSE;
               break;

             case LOC_SHARED :
               /*******************************************************/
               /* Check dictionary properties on LAN drive            */
               /*******************************************************/
               {
                 CHAR szLANProps[MAX_EQF_PATH];  // buffer for prop file name
                 PPROPDICTIONARY pRemProps = NULL;//buffer for remote props.

                 /*****************************************************/
                 /* setup path of dictionary properties               */
                 /* (the property file is located in the dictionary   */
                 /*  directory!)                                      */
                 /*****************************************************/
                 UtlMakeEQFPath( szLANProps, pDCB->Prop.szDictPath[0],
                                 DIC_PATH, NULL );
                 strcat( szLANProps, BACKSLASH_STR );
                 Utlstrccpy( szLANProps + strlen(szLANProps),
                             UtlGetFnameFromPath( pDCB->Prop.szDictPath ),
                             DOT );
                 strcat( szLANProps, EXT_OF_SHARED_DICTPROP );

                 /*****************************************************/
                 /* Allocate buffer for remote properties             */
                 /*****************************************************/
                 if ( UtlAlloc( (PVOID *)&pRemProps, 0L,
                                (LONG)sizeof(PROPDICTIONARY), ERROR_STORAGE ) )
                 {
                   /*****************************************************/
                   /* Load remote properties                            */
                   /*****************************************************/
                   usNlpRC = AsdLoadDictProperties( szLANProps, pRemProps );

                   /*****************************************************/
                   /* Compare properties and update local properties    */
                   /* if necessary                                      */
                   /*****************************************************/
                   if ( usNlpRC == LX_RC_OK_ASD )
                   {
                     if ( fCompDictProps( pRemProps, &pDCB->Prop ) )
                     {
                       pRemProps->szDictPath[0] = pDCB->Prop.szDictPath[0];
                       pRemProps->szIndexPath[0] = pDCB->Prop.szIndexPath[0];
                       memcpy( &pDCB->Prop, pRemProps, sizeof(pDCB->Prop) );
                       UtlMakeEQFPath( pDCB->Prop.PropHead.szPath, NULC,
                                       SYSTEM_PATH, NULL );
                       fPropsChanged = TRUE;
                     } /* endif */
                   } /* endif */
                   UtlAlloc( (PVOID *)&pRemProps, 0L, 0L, NOMSG );
                 }
                 else
                 {
                   usNlpRC = LX_MEM_ALLOC_ASD;       // set short on memory RC
                 } /* endif */
               }
               break;
           } /* endswitch */
         } /* endif */                                          /* 1@KIT1183A */

         /*************************************************************/
         /* If the remote properties have been changed, update the    */
         /* local ones                                                */
         /*************************************************************/
         if ( usNlpRC == LX_RC_OK_ASD )
         {
           if ( fPropsChanged )
           {
             if ( UtlWriteFile( pszCurDict, sizeof(PROPDICTIONARY),
                                (PVOID)&pDCB->Prop, TRUE ) != NO_ERROR )
             {
               usNlpRC = LX_OPEN_FLD_ASD;
             }
             else
             {
               /*******************************************************/
               /* Notify all handlers of property change              */
               /*******************************************************/
               UtlMakeEQFPath( (PCHAR)pDCB->aucDummy, NULC, SYSTEM_PATH, NULL );
               strcat( (PCHAR)pDCB->aucDummy, BACKSLASH_STR );
               strcat((PCHAR) pDCB->aucDummy, szDictName );
               strcat( (PCHAR)pDCB->aucDummy, EXT_OF_DICTPROP );

               EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                    MP1FROMSHORT( PROP_CLASS_DICTIONARY ),
                                    MP2FROMP(pDCB->aucDummy) );
             } /* endif */
           } /* endif */
         } /* endif */

         //
         // open dictionary
         //
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            /**********************************************************/
            /* Reset guarded flag if dictionary is opened without     */
            /* index                                                  */
            /**********************************************************/
            if ( usOpenFlags & ASD_NOINDEX )
            {
              usOpenFlags &= ~ASD_GUARDED;
            }
            else
            {
              usOpenFlags |=  ASD_GUARDED;
            } /* endif */

            /**********************************************************/
            /* Automatically set ASD_SHARED flag for shared           */
            /* dictionaries not opened in ASD_LOCKED mode             */
            /**********************************************************/
            if ( (pDCB->Prop.usLocation == LOC_SHARED) &&
                 !(usOpenFlags & ASD_LOCKED) )
            {
              usOpenFlags |= ASD_SHARED;
            } /* endif */

            usNumPages = (usOpenFlags & ASD_GUARDED) ? ASD_GUARDED_NUM_PAGES :
                                      ASD_UNGUARDED_NUM_PAGES;
            QDamOpenAsd( (PUCHAR)pDCB->Prop.szDictPath,
                         (PUCHAR)pDCB->Prop.szServer,           // server name
                         usNumPages,
                        usOpenFlags,
                        pUCB->usUser,
                        ausDictHandle,
                        &pDCB->usUserDataLen,
                        &usNlpRC );

            /**********************************************************/
            /* handle corrupted return code                           */
            /**********************************************************/
            if ( usNlpRC == LX_RENUM_RQD_ASD )
            {
              usOpenRC = usNlpRC;      // save return code
              usNlpRC = LX_RC_OK_ASD;  // reset flow control return code
              if ( *pusErrDict == 0 )  // no corrupted dict yet ???
              {
                *pusErrDict = usOpenDicts + 1; // set index for dict.
              } /* endif */
            } /* endif */

            if ( usNlpRC == LX_RC_OK_ASD )
            {
               pDCB->usDictHandle = ausDictHandle[0];
               DEBUGEVENT( ASDOPEN_LOC, STATE_EVENT, pDCB->usDictHandle );
            } /* endif */
         } /* endif */

         //
         // check dictionary signature record
         //
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            // get field level/numbers for specific fields
            AsdGetField( &pDCB->TransField, SYSNAME_TRANSLATION, &pDCB->Prop );
            AsdGetField( &pDCB->SynField,   SYSNAME_SYNONYM, &pDCB->Prop );
            AsdGetField( &pDCB->AbbrField,  SYSNAME_ABBREVIATION, &pDCB->Prop );
            AsdGetField( &pDCB->RelField,   SYSNAME_RELATED, &pDCB->Prop );
            AsdGetField( &pDCB->CreateDateField, SYSNAME_CREATEDATE,
                         &pDCB->Prop );
            AsdGetField( &pDCB->UpdateField, SYSNAME_LASTUPDATE,
                         &pDCB->Prop );
            AsdGetField( &pDCB->TargCreateDateField, SYSNAME_TARGCREATEDATE,
                         &pDCB->Prop );
            AsdGetField( &pDCB->TargUpdateField, SYSNAME_TARGLASTUPDATE,
                         &pDCB->Prop );
            AsdGetField( &pDCB->ContextField, SYSNAME_CONTEXT, &pDCB->Prop );
            AsdGetField( &pDCB->AuthorField, SYSNAME_AUTHOR, &pDCB->Prop );
            AsdGetField( &pDCB->AuthorOfUpdateField, SYSNAME_AUTHOROFUPDATE,
                         &pDCB->Prop );
            AsdGetField( &pDCB->TransAuthorField, SYSNAME_TRANSAUTHOR,
                         &pDCB->Prop );
            AsdGetField( &pDCB->TransAuthorOfUpdateField,
                         SYSNAME_TRANSAUTHOROFUPDATE,
                         &pDCB->Prop );
            AsdGetField( &pDCB->EntryStyle, SYSNAME_ENTRYSTYLE, &pDCB->Prop );
            AsdGetField( &pDCB->TargetStyle, SYSNAME_TARGETSTYLE, &pDCB->Prop );

            // initializatation for LDB processing
            QLDBFillFieldTables( &pDCB->Prop, pDCB->ausNoOfFields,
                                 pDCB->ausFirstField );
         } /* endif */

         //
         // open index dictionary
         //
         if ( ASDOK(usNlpRC) && !(usOpenFlags & ASD_NOINDEX) )
         {
            if ( pDCB->Prop.szIndexPath[0] != EOS )
            {
//             NlpOpenAsd( pDCB->Prop.szIndexPath, usNumPages,
               QDamOpenAsd( (PUCHAR)pDCB->Prop.szIndexPath,
                            (PUCHAR)pDCB->Prop.szServer,           // server name
                            usNumPages,
                           usOpenFlags,
                           pUCB->usUser,
                           ausDictHandle,
                           &usIndexDataLen,
                           &usNlpRC );

               /**********************************************************/
               /* handle corrupted return code                           */
               /**********************************************************/
               if ( usNlpRC == LX_RENUM_RQD_ASD )
               {
                 usOpenRC = usNlpRC;      // save return code
                 usNlpRC = LX_RC_OK_ASD;  // reset flow control return code
               } /* endif */
               if ( usNlpRC == LX_RC_OK_ASD )
               {
                  pDCB->usIndexHandle = ausDictHandle[0];
               }
               else
               {
                  usNlpRC = LX_IDX_NT_OPEN_ASD;
               } /* endif */
            }
            else
            {
               usNlpRC = LX_IDX_NT_OPEN_ASD;
            } /* endif */
         } /* endif */

         if ( usNlpRC == LX_RC_OK_ASD )
         {
           if ( pDCB->Prop.szSourceLang[0] != EOS )
           {
              strcpy( pUCB->cLangProp, pDCB->Prop.szSourceLang );
           }
           else
           {
              strcpy( pUCB->cLangProp, ASD_DEFAULT_LANGUAGE );
           } /* endif */

         } /* endif */

         if ( usNlpRC == LX_RC_OK_ASD )
         {
           if ( usOpenDicts != 0 )
           {
              pDCB->sLangID = pFirstDCB->sLangID;
              pDCB->ulOemCP = pFirstDCB->ulOemCP;
           }
           else
           {
              usNlpRC = MorphGetLanguageID( pUCB->cLangProp,
                                            &pDCB->sLangID );
              usNlpRC = AsdMorphRCToNlp( usNlpRC );
              pDCB->ulOemCP = GetLangOEMCP(NULL);
           } /* endif */
         } /* endif */

         if ( usNlpRC == LX_RC_OK_ASD )
         {
            ppszDicts++;
            usNumDicts--;
            ausDictHandles[usOpenDicts] = pDCB->usDictHandle;
            ausIndexHandles[usOpenDicts] = pDCB->usIndexHandle;
            usOpenDicts++;
            pDCB->usOpenNum = usOpenDicts;// we start with numbering at 1
            if ( pLastDCB )
            {
               pLastDCB->pNextDCB = pDCB; // anchor current DCB;
            }
            else
            {
               pFirstDCB = pDCB;       // remember the very first DCB
            } /* endif */
            pLastDCB = pDCB;           // current DCB gets last one
         } /* endif */

      } /* endwhile */

      //
      // associate dictionaries if more than one given
      //
      if ( (usNlpRC == LX_RC_OK_ASD) && (usOpenDicts > 1) )
      {
         //
         //  create dictionary control block for associated dictionary
         //
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            fOK = UtlAlloc( (PVOID *)&pDCB, 0L, (LONG) sizeof(DICTCB), NOMSG );
            if ( !fOK )
            {
               usNlpRC = LX_MEM_ALLOC_ASD;       // set short on memory RC
            } /* endif */
         } /* endif */

         //
         //  associate base dictionaries
         //
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            pDCB->lSignature = DCB_SIGNATURE;        // set DCB identifier
            pDCB->usOpenFlags = usOpenFlags;         // remember open flags
            strcpy( pDCB->szPropName, "ASSOCIATION" );// remember prop name
            ausDictHandles[usOpenDicts] = 0;      // terminate handle list
            NlpAssocAsd( ausDictHandles,
                         pUCB->usUser,
                         &pDCB->usDictHandle,
                         &usNlpRC );
         } /* endif */


         //
         //  associate index dictionaries
         //
         if ( ASDOK(usNlpRC) && !(usOpenFlags & ASD_NOINDEX) )
         {
            ausIndexHandles[usOpenDicts] = 0;      // terminate handle list
            NlpAssocAsd( ausIndexHandles,
                         pUCB->usUser,
                         &pDCB->usIndexHandle,
                         &usNlpRC );
         } /* endif */

         //
         //  append dictionary DCBs to associated DCB and copy morphdicthandle
         //
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            pDCB->fAssoc   = TRUE;     // identify as associated dictionary
            pDCB->pNextDCB = pFirstDCB;// append associated dictionaries
            pFirstDCB = pDCB;          // make association DCB the first one
            pDCB->sLangID = pDCB->pNextDCB->sLangID;
            pDCB->ulOemCP = pDCB->pNextDCB->ulOemCP;
         } /* endif */

      } /* endif */
   }
   else
   {
      usNlpRC = LX_BAD_USR_AREA;
   } /* endif */

                                                               /* 10@KITxxxxA */
   /*******************************************************************/
   /* Initialize dictionary update time                               */
   /*******************************************************************/
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     NlpDictUpdTime( pFirstDCB->usDictHandle,
                     &pFirstDCB->lUpdTime,
                     &usNlpRC );
   } /* endif */

   // set caller's DCB handle or do a cleanup in case of errors
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      *phDCB = (HDCB) pFirstDCB;
   }
   else
   {
      // close all successful opened dictionaries and free DCBs
      *pusErrDict = usOpenDicts + 1;

      usDummyRC = AsdClose( (HUCB) pUCB,
                            (HDCB) (( pFirstDCB ) ? pFirstDCB : pDCB ) );
      *phDCB = NULL;
   } /* endif */

   if ( usNlpRC != LX_RC_OK_ASD )
   {
     ERREVENT( ASDOPEN_LOC, INTFUNCFAILED_EVENT, usNlpRC );
   } /* endif */
   DEBUGEVENT( ASDOPEN_LOC, FUNCEXIT_EVENT, 0 );

   return( (usNlpRC == LX_RC_OK_ASD) ? usOpenRC : usNlpRC );

} /* endof AsdOpen */

//+----------------------------------------------------------------------------+
//| AsdUpdTime          Get time of last update of a dictionary (works also    |
//|                     for associated dictionaries)                           |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Returns the last update time of a dictionary.                           |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//|    PLONG    plUpdTime       OUT         last update time of dictionary     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hDCB must have been created using AsdOpen or AsdBuild.                  |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdUpdTime
(
   HDCB     hDCB,                      // dictionary control block handle
   PLONG    plUpdTime                  // ptr to buffer for dict update time
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PDCB      pDCB;                     // pointer to currently active DCB

   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
     NlpDictUpdTime( pDCB->usDictHandle,
                     plUpdTime,
                     &usNlpRC );
   } /* endif */

   return( usNlpRC );

} /* endof AsdUpdTime */

//+----------------------------------------------------------------------------+
//| AsdClose            Close dictionaries opened by AsdOpen (NlpCloseAsd)     |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    close morphological dictionary;                                         |
//|    if associated dictionaries then;                                        |
//|       close associated dictionary handle;                                  |
//|       close associated index dictionary handle;                            |
//|    endif;                                                                  |
//|    for all dictionaries belonging to DCB chain;                            |
//|       close dictionary handle;                                             |
//|       close index dictionary handle;                                       |
//|       free dictionary profile;                                             |
//|       free DCB;                                                            |
//|    endfor;                                                                 |
//|    if associated dictionary DCB then                                       |
//|       free DCB;                                                            |
//|    endif;                                                                  |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB            IN          user control block handle          |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen or AsdBuild.                  |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdClose
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB                       // dictionary control block handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   USHORT    usCloseRC = LX_RC_OK_ASD; // first erraneous return code
   PUCB      pUCB;                     // pointer to user control block
   PDCB      pLastDCB = NULL;          // pointer to previous DCB
   PDCB      pDCB;                     // pointer to currently active DCB

   DEBUGEVENT( ASDCLOSE_LOC, FUNCENTRY_EVENT, 0 );

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */


   if ( usNlpRC == LX_RC_OK_ASD )
   {
      while ( pDCB &&                            // while not end of DCB chain
              (usNlpRC == LX_RC_OK_ASD) )        // and no error occured ...
      {
         //
         // close base dictionary
         //
         if ( pDCB->usDictHandle )
         {
            DEBUGEVENT( ASDCLOSE_LOC, STATE_EVENT, pDCB->usDictHandle );
            NlpCloseAsd( pDCB->usDictHandle, pUCB->usUser, &usNlpRC );
            if ( usNlpRC != LX_RC_OK_ASD )
            {
              /********************************************************/
              /* An error occured ==> remember return code if it is   */
              /* the first one, reset return code to LX_RC_OK_ASD to  */
              /* allow normal close of remaining dictionaries         */
              /********************************************************/
              if ( usCloseRC == LX_RC_OK_ASD )
              {
                usCloseRC = usNlpRC;
              } /* endif */
              usNlpRC = LX_RC_OK_ASD;
            } /* endif */
         } /* endif */

         //
         //  close index dictionary
         //
         if ( (usNlpRC == LX_RC_OK_ASD) && pDCB->usIndexHandle )
         {
            NlpCloseAsd( pDCB->usIndexHandle, pUCB->usUser, &usNlpRC );
            if ( usNlpRC != LX_RC_OK_ASD )
            {
              /********************************************************/
              /* An error occured ==> remember return code if it is   */
              /* the first one, reset return code to LX_RC_OK_ASD to  */
              /* allow normal close of remaining dictionaries         */
              /********************************************************/
              if ( usCloseRC == LX_RC_OK_ASD )
              {
                usCloseRC = usNlpRC;
              } /* endif */
              usNlpRC = LX_RC_OK_ASD;
            } /* endif */
         } /* endif */

         //
         //  remember next DCB, free current one
         //
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            pLastDCB = pDCB;           // remember current DCB
            pDCB = pDCB->pNextDCB;     // go to next DCB
            UtlAlloc( (PVOID *)&pLastDCB, 0L, 0L, NOMSG );

         } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* Set return code to close return code                         */
      /****************************************************************/
      usNlpRC = usCloseRC;
   }
   else
   {
      usNlpRC = LX_BAD_USR_AREA;
   } /* endif */

   if ( usNlpRC != LX_RC_OK_ASD )
   {
     ERREVENT( ASDCLOSE_LOC, INTFUNCFAILED_EVENT, usNlpRC );
   } /* endif */
   DEBUGEVENT( ASDCLOSE_LOC, FUNCEXIT_EVENT, usNlpRC );


   return( usNlpRC );                  // return Nlp RC to caller

} /* endof AsdClose */


//+----------------------------------------------------------------------------+
//| AsdInsEntry         Insert a new entry into a dictionary (NlpInsEntryAsd)  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    add entry to dictionary using NlpInsEntryAsd;                           |
//|    if successful then                                                      |
//|       StemTerm = reduce term to stem form;                                 |
//|       get index entry for StemTerm or create empty one if new;             |
//|       if term is a MWT then                                                |
//|          add term to index entry StemTerm;                                 |
//|       endif;                                                               |
//|       write/update index entry StemTerm;                                   |
//|       if entry has synonyms then                                           |
//|          for all synonyms do;                                              |
//|             StemSyn = reduce synonym to stem form;                         |
//|             if StemSyn entry exists in index then                          |
//|                add term to list of synonyms;                               |
//|             else                                                           |
//|                create new synonym entry in index;                          |
//|             endif;                                                         |
//|          endfor;                                                           |
//|       endif;                                                               |
//|       if entry has an abbreviation then                                    |
//|          create or update abbreviation entry in dictionary index;          |
//|       endif;                                                               |
//|    endif;                                                                  |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//|    HUCB     hUCB            IN          user control block handle          |
//|    PSZ      pszTerm         IN          term to be deleted                 |
//|    PUCHAR   pucData         IN          data for term                      |
//|    ULONG    ulLength        IN          length of term data                |
//|    PULONG   pulTermNo       OUT         number of inserted term            |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen or AsdBuild.                  |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdInsEntry
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm,                   // term to be inserted
   PSZ_W    pucData,                   // data for term
   ULONG    ulLength,                  // length of term data in charw's
   PULONG   pulTermNo                  // number of inserted term
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   USHORT    usTerms     = 0;          // number of terms in index process list
   PSZ_W     pucTaskList = NULL;       // index process list
   ULONG     ulListSize  = 0;          // current size of task list
   ULONG     ulListUsed  = 0;          // used bytes in task list
   BOOL      fMWT;                     // TRUE = term is a multi word term
   USHORT    usFirstWordLength;        // length of the first word of a MWT

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // insert the term into the base dictionary
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      #if defined(MEASURETIME)
         INITTIME( pUCB );
      #endif

     NlpInsEntryAsdW( pszTerm,                   // term to be deleted
                      (PBYTE)pucData,                   // entry data
                      ulLength * sizeof(CHAR_W),  // length of entry data in bytes
                      pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      pulTermNo,                 // number of inserted term
                      &usNlpRC );
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulBaseUpdTime );
      #endif
   } /* endif */

   //
   // if MWT add first MWT word to insert-into-index list else add dummy
   // action BUILD_ACTION to list to force the creation of an index entry
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      fMWT = AsdIsMWT( pUCB, pDCB, pszTerm, &usFirstWordLength );
      if ( fMWT )
      {
         usNlpRC = AsdAddToTaskList( MWT_TYPE,
                                     INSERT_ACTION,
                                     pszTerm,
                                     usFirstWordLength,
                                     &usTerms,
                                     (PBYTE *) &pucTaskList,
                                     &ulListSize,
                                     &ulListUsed );

      }
      else
      {
         usNlpRC = AsdAddToTaskList( MWT_TYPE,
                                     BUILD_ACTION,
                                     pszTerm,
                                     (SHORT)(UTF16strlenCHAR(pszTerm)),
                                     &usTerms,
                                     (PBYTE *) &pucTaskList,
                                     &ulListSize,
                                     &ulListUsed );
      } /* endif */
   } /* endif */

   //
   // add special terms (e.g. synonyms) to insert-into-index list
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usNlpRC = AsdExtractTerms( hDCB, INSERT_ACTION,
                                 pucData,                // _w's
                                 ulLength,       // # of w's
                                 &usTerms,
                                 &pucTaskList, &ulListSize, &ulListUsed,
                                 pszTerm );
   } /* endif */

   //
   // process the returned task list and update index
   //
   if ( ASDOK(usNlpRC) )
   {
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulPrepUpdTime );
      #endif
      usNlpRC = AsdUpdateIndex( hUCB, hDCB, pszTerm, usTerms, pucTaskList );
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulIdxUpdTime );
      #endif
   } /* endif */

   //
   // cleanup
   //
   if ( pucTaskList )
   {
      UtlAlloc( (PVOID *)&pucTaskList, 0L, 0L, NOMSG );
   } /* endif */

   return( usNlpRC );

} /* end of AsdInsEntry */

USHORT AsdInsEntryNonUnicode
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm,                   // term to be inserted
   PUCHAR   pucData,                   // data for term
   ULONG    ulLength,                  // length of term data in bytes
   PULONG   pulTermNo                  // number of inserted term
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   USHORT    usTerms     = 0;          // number of terms in index process list
   PSZ_W     pucTaskList = NULL;       // index process list
   ULONG     ulListSize  = 0;          // current size of task list
   ULONG     ulListUsed  = 0;          // used bytes in task list
   BOOL      fMWT;                     // TRUE = term is a multi word term
   USHORT    usFirstWordLength;        // length of the first word of a MWT

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // insert the term into the base dictionary
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     NlpInsEntryAsdW( pszTerm,                   // term to be deleted
                      (PBYTE)pucData,            // entry data
                      ulLength,                  // length of entry data in bytes
                      pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      pulTermNo,                 // number of inserted term
                      &usNlpRC );
   } /* endif */

   //
   // if MWT add first MWT word to insert-into-index list else add dummy
   // action BUILD_ACTION to list to force the creation of an index entry
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      fMWT = AsdIsMWT( pUCB, pDCB, pszTerm, &usFirstWordLength );
      if ( fMWT )
      {
         usNlpRC = AsdAddToTaskList( MWT_TYPE,
                                     INSERT_ACTION,
                                     pszTerm,
                                     usFirstWordLength,
                                     &usTerms,
                                     (PBYTE *) &pucTaskList,
                                     &ulListSize,
                                     &ulListUsed );

      }
      else
      {
         usNlpRC = AsdAddToTaskList( MWT_TYPE,
                                     BUILD_ACTION,
                                     pszTerm,
                                     (SHORT)(UTF16strlenCHAR(pszTerm)),
                                     &usTerms,
                                     (PBYTE *) &pucTaskList,
                                     &ulListSize,
                                     &ulListUsed );
      } /* endif */
   } /* endif */

   //
   // add special terms (e.g. synonyms) to insert-into-index list
   //
//   if ( usNlpRC == LX_RC_OK_ASD )
//   {
//      usNlpRC = AsdExtractTerms( hDCB, INSERT_ACTION,
//                                 pucData,                // _w's
//                                 (USHORT)ulLength,       // # of w's
//                                 &usTerms,
//                                 &pucTaskList, &usListSize, &usListUsed,
//                                 pszTerm );
//   } /* endif */

   //
   // process the returned task list and update index
   //
   if ( ASDOK(usNlpRC) )
   {
//      usNlpRC = AsdUpdateIndex( hUCB, hDCB, pszTerm, usTerms, pucTaskList );
   } /* endif */

   //
   // cleanup
   //
   if ( pucTaskList )
   {
      UtlAlloc( (PVOID *)&pucTaskList, 0L, 0L, NOMSG );
   } /* endif */
   return usNlpRC;
} /* end of AsdInsEntry */


//+----------------------------------------------------------------------------+
//| AsdRepEntry         Replace a dictionary entry (NlpUpdEntryAsd)            |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    get dictionary entry for the term;                                      |
//|    if successful then                                                      |
//|       replace dictionary entry for the term using NlpUpdEntryAsd;          |
//|    endif;                                                                  |
//|    if successful then                                                      |
//|       build synonym change list;                                           |
//|       for all entries in synonyms change list do;                          |
//|             reduce synonym to stem form;                                   |
//|             if synonym was deleted then                                    |
//|                get index entry for synonym stem form;                      |
//|                remove term from synonym list;                              |
//|                update index entry;                                         |
//|             else if synonym was added then                                 |
//|                if synonym entry exists in index then                       |
//|                   add term to list of synonyms;                            |
//|                else                                                        |
//|                   create new synonym entry in index;                       |
//|                endif;                                                      |
//|             endif;                                                         |
//|       endfor;                                                              |
//|       same as above for abbreviations etc.;                                |
//|    endif;                                                                  |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//|    HUCB     hUCB            IN          user control block handle          |
//|    PSZ      pszTerm         IN          term to be deleted                 |
//|    PUCHAR   pucData         IN          new data for term                  |
//|    ULONG    ulLength        IN          length of term data                |
//|    PULONG   pulTermNo       OUT         number of term being updated        |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen or AsdBuild.                  |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdRepEntry
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm,                   // term to be deleted
   PSZ_W    pucData,                   // data for term
   ULONG    ulLength,                  // length of term data in char_ws
   PULONG   pulTermNo                  // number of inserted term
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   ULONG     ulTermNumber;             // number of term
   PBYTE     pucOldData = NULL;        // old data of dictionary entry
   ULONG     ulDataLength = 0;         // data length
   DICTHANDLE usDictHandle;            // dictionary of term
   USHORT    usTerms     = 0;          // number of terms in index process list
   PSZ_W     pucTaskList = NULL;       // index process list
   ULONG     ulListSize  = 0;          // current size of task list
   ULONG     ulListUsed  = 0;          // used bytes in task list

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // lookup old term data
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      #if defined(MEASURETIME)
         INITTIME( pUCB );
      #endif
      NlpFndMatchAsdW( pszTerm,                   // term we are looking for
                      pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      (PBYTE)pDCB->aucDummy,            // matching term found
                      &ulTermNumber,             // number of term found
                      &ulDataLength,             // length of entry data in bytes
                      &usDictHandle,             // dictionary of match
                      &usNlpRC );
   } /* endif */

   //
   // retrieve data for term
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      UtlAlloc( (PVOID *)&pucOldData, 0L, ulDataLength , NOMSG );
      if ( pucOldData )
      {
         NlpRetEntryAsdW( usDictHandle,           // dictionary handle
                         pUCB->usUser,           // ASD user handle
                         pDCB->aucDummy,         // term for this entry
                         &ulTermNumber,          // number of term
                         pucOldData,             // entry data
                         &ulDataLength,          // data length in # of BYTES
                         &usDictHandle,          // dictionary of term
                         &usNlpRC );
      }
      else
      {
         usNlpRC = LX_MEM_ALLOC_ASD;
      } /* endif */
   } /* endif */

   //
   // update the term data in the base dictionary
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      NlpUpdEntryAsdW( pszTerm,                   // term to be deleted
                      (PBYTE)pucData,                   // entry data
                      ulLength * sizeof(CHAR_W), // length of entry data in bytes
                      pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      pulTermNo,                 // number of inserted term
                      &usNlpRC );
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulBaseUpdTime );
      #endif
   } /* endif */

   //
   // MWT processing
   //
   // Note: no MWT processing required as the headword of the
   //       dictionary entry remains unchanged

   //
   // add special terms from old entry data to update-index list
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usNlpRC = AsdExtractTerms( hDCB, DELETE_ACTION, (PSZ_W)pucOldData,
                                 (ulDataLength / sizeof(CHAR_W)),  // # of w's
                                 &usTerms,
                                 &pucTaskList, &ulListSize, &ulListUsed,
                                 pszTerm );
   } /* endif */

   //
   // add special terms from new entry data to update-index list
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usNlpRC = AsdExtractTerms( hDCB, INSERT_ACTION, pucData,
                                 ulLength,   // # of charw's
                                 &usTerms,
                                 &pucTaskList, &ulListSize, &ulListUsed,
                                 pszTerm );
   } /* endif */

   //
   // process the returned task list and update index
   //
   if ( ASDOK(usNlpRC) )
   {
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulPrepUpdTime );
      #endif
      usNlpRC = AsdUpdateIndex( hUCB, hDCB, pszTerm, usTerms, pucTaskList );
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulIdxUpdTime );
      #endif
   } /* endif */

   //
   // cleanup
   //
   if ( pucOldData )
   {
      UtlAlloc( (PVOID *)&pucOldData, 0L, 0L, NOMSG );
   } /* endif */
   if ( pucTaskList )
   {
      UtlAlloc( (PVOID *)&pucTaskList, 0L, 0L, NOMSG );
   } /* endif */

   return( usNlpRC );

} /* end of AsdRepEntry */


//+----------------------------------------------------------------------------+
//| AsdDelEntry         Delete a dictionary entry (NlpDelEntryAsd)             |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    get dictionary entry for the term;                                      |
//|    if successful then                                                      |
//|       delete dictionary entry for the term using NlpDelEntryAsd;           |
//|    endif;                                                                  |
//|    if successful then                                                      |
//|       StemTerm = reduce term to stem form;                                 |
//|       get index entry for StemTerm;                                        |
//|       if term is contained in MWT list of StemTerm index entry then        |
//|          remove term from list;                                            |
//|          update index entry;                                               |
//|       endif;                                                               |
//|       if entry has synonyms then                                           |
//|          for all synonyms do;                                              |
//|             reduce synonym to stem form;                                   |
//|             get index entry for synonym stem form;                         |
//|             remove term from synonym list;                                 |
//|             update index entry;                                            |
//|          endfor;                                                           |
//|       endif;                                                               |
//|       if entry has an abbreviation then                                    |
//|          get index entry for abbreviation;                                 |
//|          remove term from abbreviation list of index entry;                |
//|          update index entry;                                               |
//|       endif;                                                               |
//|    endif;                                                                  |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//|    HUCB     hUCB            IN          user control block handle          |
//|    PSZ      pszTerm         IN          term to be deleted                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen or AsdBuild.                  |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdDelEntry
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm                    // term to be deleted
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   ULONG     ulTermNumber;             // number of term
   PSZ_W     pucDictData = NULL;       // base dictionary entry data
   ULONG     ulDataLength = 0;         // data length
   DICTHANDLE usDictHandle;            // dictionary of term
   USHORT    usTerms     = 0;          // number of terms in index process list
   PSZ_W     pucTaskList = NULL;       // index process list
   ULONG     ulListSize  = 0;          // current size of task list
   ULONG     ulListUsed  = 0;          // used bytes in task list
   BOOL      fMWT;                     // TRUE = term is a multi word term
   USHORT    usFirstWordLength;        // length of the first word of a MWT

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // lookup term to be deleted
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      #if defined(MEASURETIME)
         INITTIME( pUCB );
      #endif
      NlpFndMatchAsdW( pszTerm,                   // term we are looking for
                      pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      (PBYTE)pDCB->aucDummy,            // matching term found
                      &ulTermNumber,             // number of term found
                      &ulDataLength,             // length of entry data # of bytes
                      &usDictHandle,             // dictionary of match
                      &usNlpRC );
   } /* endif */

   //
   // retrieve data for term
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      UtlAlloc( (PVOID *)&pucDictData, 0L,  max(ulDataLength, MIN_ALLOC), NOMSG );
      if ( pucDictData )
      {
         NlpRetEntryAsdW( usDictHandle,           // dictionary handle
                         pUCB->usUser,           // ASD user handle
                         pDCB->aucDummy,         // term for this entry
                         &ulTermNumber,          // number of term
                         (PBYTE)pucDictData,     // entry data
                         &ulDataLength,          // data length in # of bytes
                         &usDictHandle,          // dictionary of term
                         &usNlpRC );
      }
      else
      {
         usNlpRC = LX_MEM_ALLOC_ASD;
      } /* endif */
   } /* endif */

   //
   // delete the term from the base dictionary
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      NlpDelEntryAsdW( pszTerm,                   // term to be deleted
                      pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      &usNlpRC );
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulBaseUpdTime );
      #endif
   } /* endif */

   //
   // if MWT add first MWT word to delete-from-index list
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      fMWT = AsdIsMWT( pUCB, pDCB, pszTerm, &usFirstWordLength );
      if ( fMWT )
      {
         usNlpRC = AsdAddToTaskList( MWT_TYPE,
                                     DELETE_ACTION,
                                     pszTerm,
                                     usFirstWordLength,
                                     &usTerms,
                                     (PBYTE *) &pucTaskList,
                                     &ulListSize,
                                     &ulListUsed );

      } /* endif */
   } /* endif */

   //
   // add special terms (e.g. synonyms) to delete-from-index list
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usNlpRC = AsdExtractTerms( hDCB, DELETE_ACTION, pucDictData,
                                (ulDataLength / sizeof(CHAR_W)),           // # of w'S
                                 &usTerms,
                                 &pucTaskList, &ulListSize, &ulListUsed,
                                 pszTerm );
   } /* endif */

   //
   // process the returned task list and update index
   //
   if ( ASDOK(usNlpRC) )
   {
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulPrepUpdTime );
      #endif
      usNlpRC = AsdUpdateIndex( hUCB, hDCB, pszTerm, usTerms, pucTaskList );
      #if defined(MEASURETIME)
         GETTIME( pUCB, pUCB->ulIdxUpdTime );
      #endif
   } /* endif */

   //
   // cleanup
   //
   if ( pucDictData )
   {
      UtlAlloc( (PVOID *)&pucDictData, 0L, 0L, NOMSG );
   } /* endif */
   if ( pucTaskList )
   {
      UtlAlloc( (PVOID *)&pucTaskList, 0L, 0L, NOMSG );
   } /* endif */

   return( usNlpRC );
} /* end of AsdDelEntry */


//+----------------------------------------------------------------------------+
//| AsdBuild            Create a new dictionary                                |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    create DCB;                                                             |
//|    if ok then                                                              |
//|       load dictionary profile/properties;                                  |
//|    endif;                                                                  |
//|    if ok then                                                              |
//|       create user part of signature record;                                |
//|    endif;                                                                  |
//|    if ok then                                                              |
//|       create dictionary using NlpBuildAsd;                                 |
//|    endif;                                                                  |
//|    if ok then                                                              |
//|       create index dictionary using NlpBuildAsd;                           |
//|    endif;                                                                  |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                  IN      user control block handle        |
//|    BOOL     fGuarded              IN      TRUE = open in guarded mode      |
//|    PSZ      pszDictName           IN      name of dictionary property file |
//|    PHDCB    phDCB                 OUT     dictionary control block handle  |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    Created dictionary is open.                                             |
//+----------------------------------------------------------------------------+
USHORT AsdBuild
(
   HUCB     hUCB,                      // user control block handle
   BOOL     fGuarded,                  // TRUE = open in guarded mode
   PHDCB    phDCB,                     // dictionary control block handle
   PSZ      pszDictName                // name of dictionary property file
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // pointer to user control block
   PDCB      pDCB = NULL;              // pointer to DCB
   BOOL      fOK;                      // ok flag set by UtlAlloc call
   USHORT     usIndexLength;           // length of user data in index dictionary
   USHORT     usFeatMask;
   USHORT     usLangCode = 0;          // language code of source language
   PSZ        pszLangCode;             // ptr to language code string

   pUCB = (PUCB) hUCB;                 // convert handle to pointer

   if ( !pUCB || (pUCB->lSignature != UCB_SIGNATURE))  // check UCB
   {
      usNlpRC = LX_BAD_USR_AREA;
   } /* endif */

   //
   //  create dictionary control block
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      fOK = UtlAlloc( (PVOID *)&pDCB, 0L, (LONG) sizeof(DICTCB), NOMSG );
      if ( fOK )
      {
         pDCB->lSignature = DCB_SIGNATURE;       // set DCB identifier
         pDCB->usAsdType  = usAsdType;           // default type of dictionary
         pDCB->usFeatMask = usAsdFeatMask;       // default feature mask
         pDCB->usCodepage = usAsdCodepage;       // default code page
         strcpy( pDCB->szPropName, pszDictName ); // remember prop name
      }
      else
      {
         usNlpRC = LX_MEM_ALLOC_ASD;       // set short on memory RC
      } /* endif */
   } /* endif */

   //
   //  load dictionary property file
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usNlpRC = AsdLoadDictProperties( pszDictName,  // load dictionary
                                       &pDCB->Prop );// property file
   } /* endif */

   //
   // get language properties
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      if ( pDCB->Prop.szSourceLang[0] != EOS )
      {
         strcpy( pUCB->cLangProp, pDCB->Prop.szSourceLang );
      }
      else
      {
         strcpy( pUCB->cLangProp, ASD_DEFAULT_LANGUAGE );
      } /* endif */
   } /* endif */


   //
   //  create dictionary
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     // get field level/numbers for specific fields
     AsdGetField( &pDCB->TransField, SYSNAME_TRANSLATION, &pDCB->Prop );
     AsdGetField( &pDCB->SynField,   SYSNAME_SYNONYM, &pDCB->Prop );
     AsdGetField( &pDCB->AbbrField,  SYSNAME_ABBREVIATION, &pDCB->Prop );
     AsdGetField( &pDCB->RelField,   SYSNAME_RELATED, &pDCB->Prop );
     AsdGetField( &pDCB->CreateDateField, SYSNAME_CREATEDATE, &pDCB->Prop );
     AsdGetField( &pDCB->UpdateField, SYSNAME_LASTUPDATE, &pDCB->Prop );
     AsdGetField( &pDCB->TargCreateDateField, SYSNAME_TARGCREATEDATE, &pDCB->Prop );
     AsdGetField( &pDCB->TargUpdateField, SYSNAME_TARGLASTUPDATE, &pDCB->Prop );
     AsdGetField( &pDCB->ContextField, SYSNAME_CONTEXT, &pDCB->Prop ); 
     AsdGetField( &pDCB->AuthorField, SYSNAME_AUTHOR, &pDCB->Prop );
     AsdGetField( &pDCB->AuthorOfUpdateField, SYSNAME_AUTHOROFUPDATE, &pDCB->Prop );
     AsdGetField( &pDCB->TransAuthorField, SYSNAME_TRANSAUTHOR, &pDCB->Prop );
     AsdGetField( &pDCB->TransAuthorOfUpdateField, SYSNAME_TRANSAUTHOROFUPDATE, &pDCB->Prop );
     AsdGetField( &pDCB->EntryStyle, SYSNAME_ENTRYSTYLE, &pDCB->Prop );
     AsdGetField( &pDCB->TargetStyle, SYSNAME_TARGETSTYLE, &pDCB->Prop );

     // initializatation for LDB processing
     QLDBFillFieldTables( &pDCB->Prop, pDCB->ausNoOfFields,
                          pDCB->ausFirstField );

     usFeatMask = usAsdFeatMask;
     if ( ucbAsdEditCmd[0] == EOS )
     {
        usFeatMask &= ~0x02;           // remove term editing
     } /* endif */

     pDCB->usUserDataLen = 0;

     NlpBuildAsd( (PUCHAR)pDCB->Prop.szDictPath,
                  usAsdType,                     // ASD type
                  2,                             // number of pages
                  (USHORT)fGuarded,                      // guarded flag
                  usLangCode,                    // language code
                  usAsdCodepage,                 // code page
                  usFeatMask,                    // feature mask
                  ucbAsdPrimCol,                 // primary collating sequence
                  (PUCHAR)pDCB->Prop.szServer,           // server name
                  ucbAsdEditCmd,                 // term edit command
                  ucbAsdTermTbl,                 // term encoding table
                  ucbAsdEntryTbl,                // entry encoding table
                  &(pDCB->usUserDataLen),       // length of user data
                  NULL,                         // user data structure
                  ucbAsdIndexLst,                // list of index dictionaries
                  pUCB->usUser,                  // ASD user handle
                  &(pDCB->usDictHandle),         // returned dictionary handle
                  &usNlpRC );                    // function return code

   } /* endif */

   //
   //  create index dictionary
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usIndexLength = sizeof(IndexSigData);

      NlpBuildAsd( (PUCHAR)pDCB->Prop.szIndexPath,
                   usAsdType,
                   2,                             // ASD type
                   (USHORT)fGuarded,                      // guarded flag
                   usLangCode,                    // language code
                   usAsdCodepage,                 // code page
                   0x00,                          // feature mask
                   ucbAsdPrimCol,                 // primary collating sequence
                   (PUCHAR)pDCB->Prop.szServer,   // server name
                   ucbAsdEditCmd,                 // term edit command
                   ucbAsdTermTbl,                 // term encoding table
                   ucbAsdEntryTbl,                // entry encoding table
                   &(usIndexLength),              // length of user data
                   (PUCHAR)&IndexSigData,         // user data structure
                   ucbAsdIndexLst,                // list of index dictionaries
                   pUCB->usUser,                  // ASD user handle
                   &(pDCB->usIndexHandle),        // returned dictionary handle
                   &usNlpRC );                    // function return code
   } /* endif */

   //
   // open morphological dictionary
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usNlpRC = MorphGetLanguageID( ( pDCB->Prop.szSourceLang[0] ) ?
                                    pDCB->Prop.szSourceLang :
                                    ASD_DEFAULT_LANGUAGE,
                                    &pDCB->sLangID );
      usNlpRC = AsdMorphRCToNlp( usNlpRC );
      pDCB->ulOemCP = GetLangOEMCP(( pDCB->Prop.szSourceLang[0] ) ?
                                    pDCB->Prop.szSourceLang :
                                    ASD_DEFAULT_LANGUAGE);
   } /* endif */


   *phDCB = (HDCB) pDCB;               // set caller's dictionary handle

   return( usNlpRC );
} /* end of AsdBuild */

//+----------------------------------------------------------------------------+
//| AsdGetHandle        Get dictionary user handle from HDCB                   |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hDCB must have been created using AsdOpen or AsdBuild.                  |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdHandleFromDCB
(
   HDCB     hDCB,                      //     dictionary control block handle
   PUSHORT  pusDictHandle,
   PUSHORT  pusIndexHandle,
   PVOID    * ppBTreeDict,
   PVOID    * ppBTreeIndex,
   PLONG    pLHandle
)
{
  PDCB  pDCB = (PDCB) hDCB;

  *pusDictHandle = pDCB->usDictHandle;
  *pusIndexHandle = pDCB->usIndexHandle;

  NlpRetBTree ( pDCB->usDictHandle, ppBTreeDict, pLHandle );
  NlpRetBTree ( pDCB->usIndexHandle,  ppBTreeIndex, pLHandle );

   return( LX_RC_OK_ASD );
} /* end of AsdHandleFromDCB */


//+----------------------------------------------------------------------------+
//| AsdFndBegin         Asd interface for NlpFndBeginAsd call                  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpFndBegAsd.       |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PUCHAR   pucSubString        IN     desired substring                   |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUCHAR   pucTerm             OUT    matching term found                 |
//|    PULONG   pulTermNumber       OUT    term number                         |
//|    PULONG   pulDataLength       OUT    entry data length                   |
//|    PHDCB    phDCB               OUT    dictionary of match                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdFndBegin
(
   PSZ_W    pucSubString,              // desired substring
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;        // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      NlpFndBeginAsdW( pucSubString,              // desired substring
                      pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // user handle
                      pucTerm,                   // term for this entry
                      pulTermNumber,             // term number
                      pulDataLength,             // entry data length
                      &usDictHandle,             // dictionary of match
                      &usNlpRC );                // return code
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdFndBegin */


//+----------------------------------------------------------------------------+
//|  AsdFndEquiv      - Asd interface for NlpFndEquivAsd                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpFndEquivAsd.     |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PUCHAR   pucTermIn           IN     desired term                        |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUCHAR   pucTerm             OUT    matching term found                 |
//|    PULONG   pulTermNumber       OUT    term number                         |
//|    PULONG   pulDataLength       OUT    entry data length                   |
//|    PHDCB    phDCB               OUT    dictionary of match                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdFndEquivW
(
   PSZ_W    pucTermIn,                 // desired term
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;        // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      NlpFndEquivAsdW( pucTermIn,                // desired term
                      pDCB->usDictHandle,       // dictionary handle
                      pUCB->usUser,             // user handle
                      pucTerm,                  // term found
                      pulTermNumber,            // term number
                      pulDataLength,            // entry data length
                      &usDictHandle,            // dictionary of match
                      &usNlpRC );               // return code
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdFndEquiv */

//+----------------------------------------------------------------------------+
//| AsdFndMatch       - Asd interface for NlpFndMatchAsd                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpFndMatch.        |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PUCHAR   pucTermIn           IN     desired term                        |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUCHAR   pucTerm             OUT    matching term found                 |
//|    PULONG   pulTermNumber       OUT    term number                         |
//|    PULONG   pulDataLength       OUT    entry data length                   |
//|    PHDCB    phDCB               OUT    dictionary of match                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+

USHORT AsdFndMatch
(
   PSZ_W    pucTermIn,                 // desired term
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length in # of w's
   PHDCB    phDCB                      // dictionary of match
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;        // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
     ULONG  ulDataLenBytes = 0;
      NlpFndMatchAsdW( pucTermIn,                // desired term
                      pDCB->usDictHandle,       // dictionary handle
                      pUCB->usUser,             // user handle
                      (PBYTE)pucTerm,                  // term found
                      pulTermNumber,            // term number
                      &ulDataLenBytes,          // entry data length in # of bytes
                      &usDictHandle,            // dictionary of match
                      &usNlpRC );               // return code
     * pulDataLength = ulDataLenBytes / sizeof(CHAR_W);
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdFndMatch */

//+----------------------------------------------------------------------------+
//|  AsdFndNumber     - Asd interface for NlpFndNumberAsd                      |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpFndNumberAsd     |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    ULONG    ulTermNumber        IN     desired term number                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    USHORT   usRelocation        IN     relocation flag                     |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUCHAR   pucTerm             OUT    matching term found                 |
//|    PULONG   pulDataLength       OUT    entry data length                   |
//|    PHDCB    phDCB               OUT    dictionary of match                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdFndNumber
(
   ULONG    ulTermNumber,              // desired term number
   HDCB     hDCB,                      // dictionary control block handle
   USHORT   usRelocation,              // relocation flag
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;        // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      NlpFndNumberAsdW( ulTermNumber,             // desired term number
                       pDCB->usDictHandle,       // dictionary handle
                       usRelocation,             // relocation flag
                       pUCB->usUser,             // user handle
                       pucTerm,                  // term found
                       pulDataLength,            // entry data length
                       &usDictHandle,            // dictionary of match
                       &usNlpRC );               // return code
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdFndNumber */

//+----------------------------------------------------------------------------+
//| AsdNxtTerm        - Asd interface for NlpNxtTermAsd                        |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpNxtTermAsd.      |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUCHAR   pucTerm             OUT    matching term found                 |
//|    PULONG   pulTermNumber       OUT    term number                         |
//|    PULONG   pulDataLength       OUT    entry data length                   |
//|    PHDCB    phDCB               OUT    dictionary of match                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdNxtTermW
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length in w's
   PHDCB    phDCB                      // dictionary of match
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;        // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      NlpNxtTermAsdW( pDCB->usDictHandle,       // dictionary handle
                     pUCB->usUser,             // user handle
                     pucTerm,                  // term found
                     pulTermNumber,            // term number
                     pulDataLength,            // entry data length in w's
                     &usDictHandle,            // dictionary of match
                     &usNlpRC );               // return code
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdNxtTermW */

//+----------------------------------------------------------------------------+
//|  AsdPrvTerm       - Asd interface for NlpPrvTermAsd                        |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpPrvTerm.         |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUCHAR   pucTerm             OUT    matching term found                 |
//|    PULONG   pulTermNumber       OUT    term number                         |
//|    PULONG   pulDataLength       OUT    entry data length                   |
//|    PHDCB    phDCB               OUT    dictionary of match                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdPrvTerm
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;        // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      NlpPrvTermAsdW( pDCB->usDictHandle,       // dictionary handle
                     pUCB->usUser,             // user handle
                     pucTerm,                  // term found
                     pulTermNumber,            // term number
                     pulDataLength,            // entry data length
                     &usDictHandle,            // dictionary of match
                     &usNlpRC );               // return code
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdPrvTerm */

//+----------------------------------------------------------------------------+
//|  AsdRenumber      - Asd interface for NlpRenumberAsd                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpRenumberAsd      |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HUCB     hUCB                IN     user control block handle           |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdRenumber
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB                       // user control block handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      NlpRenumberAsd( pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // user handle
                      &usNlpRC );                // return code
   } /* endif */

   return( usNlpRC );
} /* end of AsdPrvTerm */

//+----------------------------------------------------------------------------+
//|  AsdNumEntries    - get number of entries in a dictionary                  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Call fastdam function QDAMDictNumentries to return number of entries.   |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PULONG   pulNumber           IN     ptr to buffer for number of entries |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdNumEntries
(
   HDCB     hDCB,                      // dictionary control block handle
   PULONG   pulNumber                  // ptr to buffer for number of entries
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PDCB      pDCB;                     // dictionary control block pointer

   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKDCB( pDCB, usNlpRC );          // check dictionary control block pointer

   //
   // call QDAM function
   //
   if ( ASDOK(usNlpRC) )
   {
//   usNlpRC = DamBTreeRc( QDAMDictNumEntriesLocal( DamRec[pDCB->usDictHandle].pDamBTree,
//                                             pulNumber ) );
     usNlpRC = DamBTreeRc( QDAMDictNumEntriesLocal( DamGetBTreeFromDamRec(pDCB->usDictHandle),
                                               pulNumber ) );
   } /* endif */

   return( usNlpRC );
} /* end of AsdNumEntries */

//+----------------------------------------------------------------------------+
//|  AsdRetEntry      - Asd interface for NlpRetEntryAsd                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpRetEntryAsd.     |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUCHAR   pucTerm             OUT    term of this entry                  |
//|    PULONG   pulTermNumber       OUT    term number                         |
//|    PUCHAR   pucEntryData        OUT    data of term entry                  |
//|    PULONG   pulDataLength       OUT    entry data length                   |
//|    PHDCB    phDCB               OUT    dictionary of term                  |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdRetEntryW
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                    // term of this entry
   PULONG   pulTermNumber,             // term number
   PSZ_W    pucEntryData,               // data of term entry
   PULONG   pulDataLengthW,             // entry data length in # of w'ss
   PHDCB    phDCB                      // dictionary of term
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;        // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      ULONG ulDataLenBytes = *pulDataLengthW * sizeof(CHAR_W);
      NlpRetEntryAsdW( pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // user handle
                      pucTerm,                   // term for this entry
                      pulTermNumber,             // term number
                      (PBYTE)pucEntryData,              // data for term entry
                      &ulDataLenBytes,           // entry data length in bytes
                      &usDictHandle,             // dictionary of term
                      &usNlpRC );                // return code
      * pulDataLengthW = (ulDataLenBytes) / sizeof(CHAR_W);
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdRetEntry */


//+----------------------------------------------------------------------------+
//|  AsdListIndex     - Get a list of terms from the index                     |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Builds a list of specific terms (e.g. MWTs) for a given term.           |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                IN     user control block handle           |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PUCHAR   pucTerm             IN     term to lookup                      |
//|    PUSHORT  usTermType          IN     type of requested list              |
//|    PUCHAR   *ppucTermList       IN/OUT buffer for returned terms           |
//|    PUSHORT  pusUsed             OUT    used size of buffer                 |
//|    PUSHORT  pusSize             IN/OUT actual size of buffer               |
//|    PUSHORT  pusTerms            OUT    number of terms in buffer           |
//|    BOOL     fTermInStemForm     IN     TRUE = term is in stem form already |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+


USHORT AsdListIndex
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pucTerm,                   // term to lookup
   USHORT   usTermType,                // type of requested list
   PSZ_W   *ppucTermList,             // buffer for returned terms
   PLONG    plUsed,                    // used size of buffer
   PLONG    plSize,                    // actual size of buffer
   PUSHORT  pusTerms,                  // number of terms in buffer
   BOOL     fTermInStemForm            // TRUE = term is in stem form already
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   PSZ_W     pucStemTerm = NULL;       // pointer to stem form reduced term
   PBYTE     pucTemp;                  // pointer for index data processing
   PBYTE     pucEnd;                   // pointer to begin of next term list
   PBYTE     pucIndexData = NULL;      // buffer for index data
   ULONG     ulIndexSize  = 0;         // size of index data buffer
   ULONG     ulIndexUsed  = 0;         // number of bytes used in index buffer
   HDCB      hDictFoundDCB = 0;        // handle of dictionary containing term
   USHORT    usNumDictHits = 0;        // number of dictionaries containing hits

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;
   *pusTerms = 0;                      // no terms found so far

   DEBUGEVENT2( ASDLISTINDEX_LOC, FUNCENTRY_EVENT, 0, DICT_GROUP, NULL );

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // build stem form of term if not done yet
   //
   if ( ASDOK(usNlpRC) )
   {
      if ( fTermInStemForm )
      {
         pucStemTerm = pucTerm;
      }
      else
      {
         #if defined(MEASURETIME)
            GETTIME( pUCB, pUCB->ulTransListIndexTime );
         #endif
         usNlpRC = AsdGetStemForm( hUCB, hDCB, pucTerm, pUCB->ucStemTerm );
         #if defined(MEASURETIME)
            GETTIME( pUCB, pUCB->ulTransStemRedTime );
            pUCB->ulTransStemRed++;
         #endif
         pucStemTerm = pUCB->ucStemTerm;
      } /* endif */
   } /* endif */

   //
   // lookup term in index for all given dictionaries
   //
   while ( ASDOK(usNlpRC) && pDCB )
   {
      if ( ASDOK(usNlpRC) )
      {  // usIndexSize + usIndexUsed in number of bytes
         usNlpRC = AsdGetIndexEntry( hUCB, hDCB, pucStemTerm,
                      &pucIndexData, &ulIndexSize, &ulIndexUsed,
                      &hDictFoundDCB );
      } /* endif */

      //
      // build requested term list
      //
      if ( ASDOK(usNlpRC) )
      {
         PASDINDEXENTRYHEADER pHeader = (PASDINDEXENTRYHEADER)pucIndexData;
         // get pointer to start of index data
         pucTemp      = pucIndexData;

         // get pointer to end of list
         pucEnd       = pucTemp + pHeader->ulTermOffs[usTermType+1];

         // get pointer to start of term list
         pucTemp      = pucTemp + pHeader->ulTermOffs[usTermType];

         if ( ASDOK(usNlpRC) ) usNumDictHits++;

         // loop through term list and look for term
         while ( ASDOK(usNlpRC) && (pucTemp < pucEnd) )
         {
            USHORT usTermLength = *(PUSHORT)pucTemp;
            PSZ_W  pszIndexTerm = (PSZ_W)(pucTemp + sizeof(USHORT));
            usNlpRC =  AsdAddToTermList( pszIndexTerm,
                                         (USHORT)(usTermLength / sizeof(CHAR_W)),  // length in CHAR_Ws
                                         pusTerms,
                                         ppucTermList,
                                         plSize,
                                         plUsed,
                                         TRUE,
                                         0 );
            pucTemp += usTermLength + sizeof(USHORT);   // go to next entry in list
         } /* endwhile */
      } /* endif */

      //
      // continue with next dictionary if this is not the last one
      // and a term has been found
      //
      if ( pDCB->fAssoc )
      {
         if ( ASDOK(usNlpRC) )
         {
            pDCB = ((PDCB)hDictFoundDCB)->pNextDCB;
            hDCB = (HDCB) pDCB;
         } /* endif */
      }
      else
      {
         // loop thru remaining dictionaries and reset return code
         pDCB = pDCB->pNextDCB;
         hDCB = (HDCB) pDCB;
         usNlpRC = (USHORT)(( usNlpRC == LX_WRD_NT_FND_ASD ) ? LX_RC_OK_ASD : usNlpRC);
      } /* endif */
   } /* endwhile */

   // if the entries in the term list are from different dictionaries and a 
   // MWT list has been requested, we have to sort the combined term list
   // depending on the term size (longest entries first)
   if ( (*pusTerms > 1) && (usNumDictHits > 1) && (usTermType == MWT_TYPE) )
   {
     BOOL fTermsSwitched = FALSE;
 
     // repeat until no adjacent terms had to be exchanged in the list
     do
     {
       PSZ_W pszCurrent = (PSZ_W)*ppucTermList;
       USHORT usRemainingTerms = *pusTerms;

       fTermsSwitched = FALSE;

       // loop over all terms and exchange adjacent terms if their length is in worn order
       while ( usRemainingTerms > 1 )
       {
         ULONG ulCurrentLen = UTF16strlenCHAR( pszCurrent );
         PSZ_W pszNext = pszCurrent + ulCurrentLen + 1;
         ULONG ulNextLen = UTF16strlenCHAR( pszNext  );

         if ( ulNextLen > ulCurrentLen )
         {
           // we have to switch these two terms
           CHAR_W szTermBuf[MAX_TERM_LEN+2];
           UTF16strcpy( szTermBuf, pszCurrent );
           memmove( pszCurrent, pszNext, (ulNextLen + 1) * sizeof(CHAR_W) );
           pszNext = pszCurrent + ulNextLen + 1;
           UTF16strcpy( pszNext, szTermBuf );
           fTermsSwitched = TRUE;
         } /* endif */
         usRemainingTerms--;
         pszCurrent = pszNext;

       } /* endif */
     } while ( fTermsSwitched );
   } /* endif */

   //
   // cleanup
   //
   if ( pucIndexData )
   {
      UtlAlloc( (PVOID *)&pucIndexData, 0L, 0L, NOMSG );
   } /* endif */

   usNlpRC = (USHORT)(( usNlpRC == LX_WRD_NT_FND_ASD ) ? LX_RC_OK_ASD : usNlpRC);

   DEBUGEVENT2( ASDLISTINDEX_LOC, FUNCEXIT_EVENT, usNlpRC, DICT_GROUP, NULL );

   return( usNlpRC );

} /* end of AsdListIndex */


//+----------------------------------------------------------------------------+
//| AsdDelete         - Delete a dictionary (dictionary must be closed!)       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Physically deletes all files belonging to a dictionary: base dictionary,|
//|    index dictionary and dictionary property file. The dictionary must      |
//|    closed.                                                                 |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PSZ      pszPropName         IN     name of dictionary property file    |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_OPEN_FLD_ASD     =  error deleting dictionary   |
//|                         LX_IDX_NT_OPEN_ASD  =  error deleting index        |
//|                         LX_UNEXPECTED_ASD   =  parameters invalid          |
//|                                                or property file not found  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
USHORT AsdDelete
(
   PSZ      pszPropName                // name of dictionary property file
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PPROPDICTIONARY pProp;              // pointer to dictionary properties
   USHORT    usDelDict;                // return code of UtlDelete on dictionary
   USHORT    usDelIndex;               // return code of UtlDelete on index
   USHORT    usDelProp;                // return code of UtlDelete on properties
   BOOL      fOK;                      // ok flag set by UtlAlloc call

   //
   // allocate buffer for dictionary properties
   //
   fOK = UtlAlloc( (PVOID *)&pProp, 0L, (LONG) sizeof(PROPDICTIONARY), NOMSG );
   if ( !fOK )
   {
      usNlpRC = LX_MEM_ALLOC_ASD;
   } /* endif */

   //
   // load dictionary properties
   //
   if ( ASDOK(usNlpRC) )
   {
      usNlpRC = AsdLoadDictProperties( pszPropName, pProp );
      if ( !ASDOK(usNlpRC) )
      {
         usNlpRC = LX_UNEXPECTED_ASD;
      } /* endif */
   } /* endif */

   //
   // delete dictionary files
   //
   if ( ASDOK(usNlpRC) )
   {
      usDelDict  = UtlDelete( pProp->szDictPath, 0L, FALSE );
      if ( pProp->szIndexPath[0] != EOS )
      {
         usDelIndex = UtlDelete( pProp->szIndexPath, 0L, FALSE );
      }
      else
      {
         usDelIndex = 0;
      } /* endif */

      usDelProp = UtlDelete( pszPropName, 0L, FALSE );

      if ( pProp->usLocation == LOC_SHARED )
      {
        /**************************************************************/
        /* Delete copy of property file on shared drive               */
        /**************************************************************/
        CHAR       szLANProp[MAX_EQF_PATH];
        USHORT     usDelLANProp;
        PSZ        pszExt;

        // don't delete RPR file if this is no regular shared dict!
        pszExt = strrchr( pProp->szDictPath, DOT );
        if ( pszExt && (stricmp( pszExt, EXT_OF_SHARED_DIC ) == 0 ) )
        {
           Utlstrccpy( szLANProp, pProp->szDictPath, DOT );
           strcat( szLANProp, EXT_OF_SHARED_DICTPROP );
           usDelLANProp = UtlDelete( szLANProp, 0L, FALSE );
           usDelProp = ( usDelProp ) ? usDelProp : usDelLANProp;

           /**************************************************************/
           /* Delete any dummy lock files                                */
           /**************************************************************/
           strcpy( szLANProp, pProp->szDictPath );
           pszExt = strrchr( szLANProp, DOT );
           if ( pszExt ) pszExt[2] = '-';
           UtlDelete( szLANProp, 0L, FALSE );

           strcpy( szLANProp, pProp->szIndexPath );
           pszExt = strrchr( szLANProp, DOT );
           if ( pszExt ) pszExt[2] = '-';
           UtlDelete( szLANProp, 0L, FALSE );
        } /* endif */
      } /* endif */

      if ( usDelDict )
      {
         usNlpRC = LX_OPEN_FLD_ASD;
      }
      else if ( usDelProp )
      {
         usNlpRC = LX_UNEXPECTED_ASD;
      }
      else if ( usDelIndex )
      {
         usNlpRC = LX_IDX_NT_OPEN_ASD;
      } /* endif */
   } /* endif */

   //
   // cleanup
   //
   if ( pProp )
   {
      UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
   } /* endif */

   return( usNlpRC );

} /* end of AsdDelete */

//+----------------------------------------------------------------------------+
//| AsdRename         - Rename a dictionary (dictionary must be closed!)       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Renames all files of a dictionary: base dictionary, index dictionary    |
//|    and dictionary property file. In additon the property file is changed   |
//|    using the new names. The dictiomary must closed.                        |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PSZ      pszOldPropName      IN     name of old dictionary property file|
//|    PSZ      pszNewPropName      IN     name of new dictionary property file|
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_MEM_ALLOC_ASD    =  memory allocation failed    |
//|                         LX_OPEN_FLD_ASD     =  error renaming dictionary   |
//|                         LX_IDX_NT_OPEN_ASD  =  error renaming index        |
//|                         LX_UNEXPECTED_ASD   =  parameters invalid          |
//|                                                or property file not found  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
USHORT AsdRename
(
   PSZ      pszOldPropName,            // name of old dictionary property file
   PSZ      pszNewPropName             // name of new dictionary property file
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PPROPDICTIONARY pProp;              // pointer to dictionary properties
   BOOL      fOK;                      // ok flag set by UtlAlloc call
   CHAR      szNewName[MAX_FILESPEC+1];// buffer for new dictionary name
   CHAR      szNewPath[MAX_EQF_PATH+1];// buffer for new path names
   USHORT    usRC;                     // return code of Dosxxxx functions

   //
   // allocate buffer for dictionary properties
   //
   fOK = UtlAlloc( (PVOID *)&pProp, 0L, (LONG) sizeof(PROPDICTIONARY), NOMSG );
   if ( !fOK )
   {
      usNlpRC = LX_MEM_ALLOC_ASD;
   } /* endif */

   //
   // load dictionary properties
   //
   if ( ASDOK(usNlpRC) )
   {
      usNlpRC = AsdLoadDictProperties( pszOldPropName, pProp );
      if ( !ASDOK(usNlpRC) )
      {
         usNlpRC = LX_UNEXPECTED_ASD;
      } /* endif */
   } /* endif */

   //
   // rename dictionary file
   //
   if ( ASDOK(usNlpRC) )
   {
      // build new dictionary name
      Utlstrccpy( szNewName, UtlGetFnameFromPath( pszNewPropName ), '.' );
      UtlMakeEQFPath( szNewPath, pProp->szDictPath[0], DIC_PATH, NULL );
      strcat( szNewPath, "\\" );
      strcat( szNewPath, szNewName );
      strcat( szNewPath, EXT_OF_DIC );

      // rename the file
      usRC = UtlMove( pProp->szDictPath, szNewPath, 0L, FALSE );
      if ( usRC )
      {
         usNlpRC = LX_OPEN_FLD_ASD;
      }
      else
      {
         strcpy( pProp->szDictPath, szNewPath ); // set new dictionary name
      } /* endif */
   } /* endif */

   //
   // rename dictionary index file
   //
   if ( ASDOK(usNlpRC) )
   {
      // build new dictionary index name
      UtlMakeEQFPath( szNewPath, pProp->szIndexPath[0], DIC_PATH, NULL );
      strcat( szNewPath, "\\" );
      strcat( szNewPath, szNewName );
      strcat( szNewPath, EXT_OF_DICTINDEX );

      // rename the file
      usRC = UtlMove( pProp->szIndexPath, szNewPath, 0L, FALSE );
      if ( usRC )
      {
         usNlpRC = LX_OPEN_FLD_ASD;
      }
      else
      {
         strcpy( pProp->szIndexPath, szNewPath ); // set new dictionary name
      } /* endif */
   } /* endif */

   //
   // change property file and rewrite it to disk
   //
   if ( ASDOK(usNlpRC) )
   {
      // correct property head
      strcpy( pProp->PropHead.szName, UtlGetFnameFromPath( pszNewPropName ) );

      // rewrite property file
      usRC = UtlWriteFile( pszNewPropName, sizeof(PROPDICTIONARY), pProp,
                              FALSE );

      // delete old property file
      if ( !usRC )
      {
         usRC = UtlDelete( pszOldPropName, 0L, FALSE );
      } /* endif */

      if ( usRC )
      {
         usNlpRC = LX_UNEXPECTED_ASD;
      } /* endif */
   } /* endif */

   //
   // cleanup
   //
   if ( pProp )
   {
      UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
   } /* endif */

   return( usNlpRC );

} /* end of AsdRename */


//+----------------------------------------------------------------------------+
//|  AsdRetPropPtr    - Get pointer to properties for an open dictionary       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Returns a pointer to the properties of a prviously openened dictionary. |
//|    Associated dictionaries can not be used with this call.                 |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                IN     user control block handle           |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PPROPDICtionaRY *ppProp      OUT    ptr to dictionary properties        |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen or AsdBuild                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdRetPropPtr
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PPROPDICTIONARY *ppProp             // ptr to dictionary properties
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   if ( ASDOK(usNlpRC) )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */
   if ( ASDOK(usNlpRC) )
   {
      if ( pDCB->fAssoc )              // check associated dictionary flag
      {
         usNlpRC = LX_ASC_NT_ALLWD_ASD;
      } /* endif */
   } /* endif */

   //
   // set caller's dictionary property pointer
   //
   if ( ASDOK(usNlpRC) )
   {
      *ppProp = &pDCB->Prop;
   } /* endif */

   return( usNlpRC );
} /* end of AsdRetPropPtr */

//+----------------------------------------------------------------------------+
//|  AsdEntryChange - Correct internal lookup tables after entry has changed.  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Correct the internal atom tables after a dictionary entry has been added|
//|    or changed.                                                             |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                IN     user control block handle           |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PSZ      pszTerm             IN     term which has been changed         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    The term-not-found atom tables are corrected.                           |
//+----------------------------------------------------------------------------+
USHORT AsdEntryChange
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm                    // term which has been changed or added
)
{
   EQF_ATOM  TermAtom;                  // atom of saved term
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   USHORT    usWordLength;             // length of first word of a MWT
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   PSZ_W     pucTermBuf1 = NULL;       // buffer for term processing
   PSZ_W     pucTermBuf2 = NULL;       // buffer for term processing
   ULONG     ulOemCp;

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   ulOemCp  = GetLangOEMCP(NULL);      // get system pref. lang.
   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( ASDOK(usNlpRC) )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // get buffer for term processing
   //
   if ( ASDOK(usNlpRC) )
   {
      UtlAlloc( (PVOID *)&pucTermBuf1, 0L, (LONG)(sizeof(CHAR_W)*((MAX_TERM_LEN * 2) + 2)), NOMSG );
      if ( !pucTermBuf1 )
      {
         usNlpRC = LX_MEM_ALLOC_ASD;
      }
      else
      {
         pucTermBuf2 = pucTermBuf1 + (MAX_TERM_LEN + 1);
      } /* endif */
   } /* endif */

   //
   // get stem form of term
   //
   if ( ASDOK(usNlpRC) && pUCB->fTransUsed )
   {
      usNlpRC = AsdGetStemForm( hUCB, hDCB, pszTerm, pucTermBuf1 );
   } /* endif */

   //
   // correct translation atom tables
   //
   if ( ASDOK(usNlpRC) && pUCB->fTransUsed )
   {
      // delete term as-is from atom tables
      DELETEATOMW( pUCB->hNotInBaseAtoms, pszTerm, TermAtom, ulOemCp  );

      // delete stem form of term from atom tables
      DELETEATOMW( pUCB->hNotInBaseAtoms, pucTermBuf1, TermAtom, ulOemCp  );

      // if term is a multi-word-term, delete first word of MWT too
      if ( AsdIsMWT( pUCB, pDCB, pszTerm, &usWordLength ) )
      {
         UTF16strncpy( pucTermBuf1, pszTerm, usWordLength );
         *(pucTermBuf1 + usWordLength) = EOS;
         AsdGetStemForm( hUCB, hDCB, pucTermBuf1, pucTermBuf2 );

         // delete term as-is from atom tables
         DELETEATOMW( pUCB->hNotInBaseAtoms, pucTermBuf1, TermAtom, ulOemCp  );

         // delete stem form of term from atom tables
         DELETEATOMW( pUCB->hNotInBaseAtoms, pucTermBuf2, TermAtom, ulOemCp  );
      } /* endif */
   } /* endif */

   return( usNlpRC );

} /* end of AsdEntryChange */

//+----------------------------------------------------------------------------+
//|  AsdIsUcbOK     - Check if an UCB is OK                                    |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Check if user control block handle points to a valid user control block.|
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                IN     user control block handle           |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL     fOK         function return code:                              |
//|                         TRUE                =  HUCB is OK                  |
//|                         FALSE               =  HUCB is no valid UCB        |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
BOOL  AsdIsUcbOK
(
   HUCB     hUCB                       // user control block handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)

   CHECKUCB( ((PUCB)hUCB), usNlpRC );  // check user control block pointer

   return ( ASDOK(usNlpRC) );

} /* end of AsdIsUcbOK */

//+----------------------------------------------------------------------------+
//|  AsdIsDcbOK     - Check if an DCB is OK                                    |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Check if user control block handle points to a valid user control block.|
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL     fOK         function return code:                              |
//|                         TRUE                =  HDCB is OK                  |
//|                         FALSE               =  HDCB is no valid DCB        |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
BOOL  AsdIsDcbOK
(
   HDCB     hDCB                       // dictionary control block handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)

   CHECKDCB( ((PDCB)hDCB), usNlpRC );  // check dictionary control block pointer

   return ( ASDOK(usNlpRC) );

} /* end of AsdIsDcbOK */

//+----------------------------------------------------------------------------+
//|  AsdQueryDictName    - Return name of dictioanry (w/o path and extension)  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Return the name of a dictionary for a given dictionary handle.          |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PSZ      pszDictName         OUT    ptr to buffer for dictionary name   |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL     fOK         function return code:                              |
//|                         TRUE                =  function completed OK       |
//|                         FALSE               =  an error occured            |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
BOOL AsdQueryDictName
(
   HDCB     hDCB,                      // dictionary control block handle
   PSZ      pszDictName                // ptr to buffer for dictionary name
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)

   CHECKDCB( ((PDCB)hDCB), usNlpRC );  // check dictionary control block pointer


   if ( pszDictName != NULL )
   {
     *pszDictName = EOS;
   } /* endif */

   if ( ASDOK(usNlpRC) )
   {
      if ( ((PDCB)hDCB)->fAssoc )
      {
        usNlpRC = LX_ASC_NT_ALLWD_ASD;     // association not allowed
      }
      else
      {
        if ( ((PDCB)hDCB)->Prop.szLongName[0] != EOS )
        {
          strcpy( pszDictName, ((PDCB)hDCB)->Prop.szLongName );
        }
        else
        {
          Utlstrccpy( pszDictName,
                      UtlGetFnameFromPath( ((PDCB)hDCB)->Prop.szDictPath), '.' );
        } /* endif */
      } /* endif */
   } /* endif */

   return ( ASDOK(usNlpRC) );

} /* end of AsdQueryDictName */

//+----------------------------------------------------------------------------+
//|  AsdQueryDictName    - Return name of dictioanry (w/o path and extension)  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Return the name of a dictionary for a given dictionary handle.          |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PSZ      pszDictName         OUT    ptr to buffer for dictionary name   |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL     fOK         function return code:                              |
//|                         TRUE                =  function completed OK       |
//|                         FALSE               =  an error occured            |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
BOOL AsdQueryDictShortName
(
   HDCB     hDCB,                      // dictionary control block handle
   PSZ      pszDictName                // ptr to buffer for dictionary name
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)

   CHECKDCB( ((PDCB)hDCB), usNlpRC );  // check dictionary control block pointer


   if ( pszDictName != NULL )
   {
     *pszDictName = EOS;
   } /* endif */

   if ( ASDOK(usNlpRC) )
   {
      if ( ((PDCB)hDCB)->fAssoc )
      {
        usNlpRC = LX_ASC_NT_ALLWD_ASD;     // association not allowed
      }
      else
      {
        Utlstrccpy( pszDictName,
                    UtlGetFnameFromPath( ((PDCB)hDCB)->Prop.szDictPath), '.' );
      } /* endif */
   } /* endif */

   return ( ASDOK(usNlpRC) );

} /* end of AsdQueryDictShortName */


//+----------------------------------------------------------------------------+
//|  AsdRetDictList    - Return list of associated dictionaries                |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Return a list of the handles of all dictionaries associated to the      |
//|    given dictionary handle.                                                |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HDCB     *ahDCB              OUT    ptr to array for dictionary handles |
//|                                        (size should be MAX_DICTS)          |
//|    PUSHORT  pusNumOfDicts       OUT    number of dictionaries associated   |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdRetDictList
(
   HDCB     hDCB,                      // dictionary control block handle
   HDCB     *ahDCB,                    // ptr to array for dictionary handles
   PUSHORT  pusNumOfDicts              // ptr to variable receiving number of
                                       // associated dictionaries
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PDCB      pDCB;                     // ptr to dictionary control block

   pDCB = (PDCB) hDCB;                 // convert handle to pointer
   CHECKDCB( pDCB, usNlpRC );          // check dictionary control block pointer
   *pusNumOfDicts = 0;                 // reset number of dictionaries

   if ( ASDOK(usNlpRC) )
   {
      if ( pDCB->fAssoc )              // for associated dictionaries ...
      {
         pDCB = pDCB->pNextDCB;        // ... start with first associated dict.
      } /* endif */

      while ( pDCB )                   // while not at end of dictionary list
      {
         ahDCB[*pusNumOfDicts] = (HDCB) pDCB;   // fill in dictionary handle
         *pusNumOfDicts += 1;          // increment dictionary number
         pDCB = pDCB->pNextDCB;        // continue with next dictionary
      } /* endwhile */
   } /* endif */

   return ( usNlpRC );

} /* end of AsdRetDictList */

//+----------------------------------------------------------------------------+
//|  AsdRetUserHandle  - Return the user handle supplied by NlpBegin           |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Return the ASD functions user handle which is has been created using    |
//|    NlpBegin.                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUSHORT  pusUserHandle       OUT    Nlp user handle                     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdRetUserHandle
(
   HUCB     hUCB,                      // user control block handle
   PUSHORT  pusUserHandle              // Nlp user handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // ptr to user control block

   pUCB = (PUCB) hUCB;                 // convert handle to pointer
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( ASDOK(usNlpRC) )
   {
      if ( pusUserHandle )
      {
         *pusUserHandle = pUCB->usUser;
      }
      else
      {
         usNlpRC = LX_UNEXPECTED_ASD;
      } /* endif */
   } /* endif */

   return ( usNlpRC );

} /* end of AsdRetUserHandle */

//+----------------------------------------------------------------------------+
//|  AsdRetServiceHandle  - Return the service handle supplied by NlpBegService|
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Return the ASD functions user handle which is has been created using    |
//|    NlpBegService.                                                          |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUSHORT  pusServiceHandle    OUT    Nlp service handle                  |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdRetServiceHandle
(
   HUCB     hUCB,                      // user control block handle
   PUSHORT  pusServiceHandle           // Nlp service handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // ptr to user control block

   pUCB = (PUCB) hUCB;                 // convert handle to pointer
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( ASDOK(usNlpRC) )
   {
      *pusServiceHandle = 0;
      usNlpRC = LX_UNEXPECTED_ASD;
   } /* endif */

   return ( usNlpRC );

} /* end of AsdRetServiceHandle */

//+----------------------------------------------------------------------------+
//|  AsdRetMorphHandle  - Return the handle of a morphology dictionary         |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Return the handle of the morphology dictionary which has been opened    |
//|    implicitely by AsdOpen.                                                 |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PUSHORT  pusMorphHandle      OUT    NlpActDict dictionary handle        |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    none                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdRetMorphHandle
(
   HDCB     hDCB,                      // dictionary control block handle
   PUSHORT  pusMorphHandle             // NlpActDict dictionary handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PDCB      pDCB;                     // ptr to dictionary control block

   pDCB = (PDCB) hDCB;                 // convert handle to pointer
   CHECKDCB( pDCB, usNlpRC );          // check dictionary control block pointer

   if ( ASDOK(usNlpRC) )
   {
      *pusMorphHandle = 0;
      usNlpRC = LX_UNEXPECTED_ASD;
   } /* endif */

   return ( usNlpRC );

} /* end of AsdRetMorphHandle */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdLockDict      Lock or unlock complete dictionary      |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Locks or unlocks complete dictionary.                    |
//|                   :p.                                                      |
//|                   While the dictionary is locked no other function using   |
//|                   Asd dictionary calls may write to the dictionary.        |
//+----------------------------------------------------------------------------+
//|Input parameter:   HDCB     hDCB   dictionary control block handle          |
//|                   BOOL     fLock  lock flag (TRUE = lock, FALSE = unlock)  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       LX_RC_OK_ASD        =  OK                                |
//|                   other               =  error code                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      hDCB must be a valid dictionary handle which refers to   |
//|                   a single dictionary. The handle for associated           |
//|                   dictionaries may not be used.                            |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
USHORT AsdLockDict
(
  HDCB     hDCB,                       // dictionary control block handle
  BOOL     fLock                       // lock flag (TRUE = lock,
                                       //            FALSE = unlock)
)
{
  PDCB      pDCB;                      // ptr to dictionary control block
  USHORT    usNlpRC = LX_RC_OK_ASD;    // return code of Nlp call(s) ;

  pDCB = (PDCB) hDCB;

  //
  // check input data
  //
  CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer

  //
  // lock/unlock the term
  //
  if ( usNlpRC == LX_RC_OK_ASD )
  {
    NlpLockDict( pDCB->usDictHandle, fLock, &usNlpRC );
  } /* endif */

  return( usNlpRC );
} /* end of function AsdLockDict */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdLockEntry     Lock or unlock a single dictionary entry|
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Locks or unlocks a single dictionary term.               |
//|                   :p.                                                      |
//|                   While the dictionary term is locked no other function    |
//|                   using Asd dictionary calls may write to the locked       |
//|                   term.                                                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   HDCB     hDCB    dictionary control block handle         |
//|                   PSZ      pszTerm ptr to term being locked/unlocked       |
//|                   BOOL     fLock   lock flag (TRUE = lock, FALSE = unlock) |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       LX_RC_OK_ASD        =  OK                                |
//|                   other               =  error code                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      hDCB must be a valid dictionary handle which refers to   |
//|                   a single dictionary. The handle for associated           |
//|                   dictionaries may not be used.                            |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
USHORT AsdLockEntry
(
  HDCB     hDCB,                       // dictionary control block handle
  PSZ_W    pszTerm,                    // ptr to term being locked/unlocked
  BOOL     fLock                       // lock flag (TRUE = lock,
                                       //            FALSE = unlock)
)
{
  PDCB      pDCB;                      // ptr to dictionary control block
  USHORT    usNlpRC  = LX_RC_OK_ASD;   // return code of Nlp call(s)

  pDCB = (PDCB) hDCB;

  //
  // check input data
  //
  CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer

  //
  // lock/unlock the term
  //
  if ( usNlpRC == LX_RC_OK_ASD )
  {
    NlpLockEntryW( pDCB->usDictHandle, pszTerm, fLock, &usNlpRC );
  } /* endif */

  return( usNlpRC );
} /* end of function AsdLockEntry */

//+----------------------------------------------------------------------------+
//| AsdDeleteRemote   - Delete a dictionary (dictionary must be closed!)       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Physically deletes all files belonging to a dictionary: base dictionary,|
//|    index dictionary and dictionary property file on the server.            |
//|    The dictionary should be closed..                                       |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PSZ      pszPropName         IN     name of dictionary property file    |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_OPEN_FLD_ASD     =  error deleting dictionary   |
//|                         LX_IDX_NT_OPEN_ASD  =  error deleting index        |
//|                         LX_UNEXPECTED_ASD   =  parameters invalid          |
//|                                                or property file not found  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None.                                                                   |
//+----------------------------------------------------------------------------+
USHORT AsdDeleteRemote
(
   PSZ      pszPropName                // name of dictionary property file
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PPROPDICTIONARY pProp;              // pointer to dictionary properties
   USHORT    usDelDict;                // return code of UtlDelete on dictionary
   USHORT    usDelDict2;               // return code of UtlDelete on dictionary
   USHORT    usDelIndex;               // return code of UtlDelete on index
   USHORT    usDelProp;                // return code of UtlDelete on properties
   BOOL      fOK;                      // ok flag set by UtlAlloc call
   CHAR      chPropName[ MAX_EQF_PATH ]; // property name path ...

   //
   // allocate buffer for dictionary properties
   //
   fOK = UtlAlloc( (PVOID *)&pProp, 0L, (LONG) sizeof(PROPDICTIONARY), NOMSG );
   if ( !fOK )
   {
      usNlpRC = LX_MEM_ALLOC_ASD;
   } /* endif */

   //
   // load dictionary properties
   //
   if ( ASDOK(usNlpRC) )
   {
      usNlpRC = AsdLoadDictProperties( pszPropName, pProp );
      if ( !ASDOK(usNlpRC) )
      {
         usNlpRC = LX_UNEXPECTED_ASD;
      } /* endif */
   } /* endif */

   //
   // delete dictionary files
   //
   if ( ASDOK(usNlpRC) )
   {
      usDelDict  = QDAMDeleteFile( pProp->szServer, pProp->szDictPath );
      /****************************************************************/
      /* in case of locked dictionary - do not proceed with deleting  */
      /****************************************************************/
      if ( usDelDict != BTREE_DICT_LOCKED   )
      {
        if ( pProp->szIndexPath[0] != EOS )
        {
           usDelIndex = QDAMDeleteFile( pProp->szServer, pProp->szIndexPath );
        }
        else
        {
           usDelIndex = 0;
        } /* endif */

        //delete remote property file
        UtlMakeEQFPath( chPropName, pProp->chRemPrimDrive, COMPROP_PATH, NULL );
        strcat( chPropName, BACKSLASH_STR );
        strcat( chPropName, pProp->PropHead.szName );
        usDelDict2 = QDAMDeleteFile( pProp->szServer, chPropName );

        //delete local property file
        usDelProp = UtlDelete( pszPropName, 0L, FALSE );
        if ( usDelDict )
        {
           usNlpRC = DamBTreeRc( usDelDict );
        }
        else if ( usDelProp )
        {
           usNlpRC = LX_UNEXPECTED_ASD;
        }
        else if ( usDelIndex )
        {
          usNlpRC = DamBTreeRc( usDelIndex );
        }
        else if ( usDelDict2 )
        {
          usNlpRC = DamBTreeRc( usDelDict2 );
        } /* endif */
      }
      else
      {
        usNlpRC = DamBTreeRc( usDelDict );
      } /* endif */
   } /* endif */

   //
   // cleanup
   //
   if ( pProp )
   {
      UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
   } /* endif */

   return( usNlpRC );

} /* end of AsdDeleteRemote */



//ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
//บ   Internal Routines                                                        บ
//ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ

//+----------------------------------------------------------------------------+
//| AsdLoadDictProperties  - load dictionary propery file                      |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdLoadDictProperties
(
   PSZ             pszPropFile,        // fully qualified property file name
   PPROPDICTIONARY pDictProp           // buffer for property data

)
{
   ULONG      ulBytesRead;             // number of bytes read from file
   BOOL       fOK;                     // internal OK flag
   USHORT     usDosRc;                 // Return code from Dos operations
   HFILE      hInputfile = NULLHANDLE; // File handle for input file
   USHORT     usAction;                // input file action

   usDosRc = UtlOpen( pszPropFile, &hInputfile , &usAction,
                      0L, FILE_NORMAL,
                      FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE,
                      0L, FALSE );

   fOK = ( usDosRc == 0 );

   if (fOK)
   {
      usDosRc = UtlReadL( hInputfile , pDictProp, sizeof(PROPDICTIONARY),
                         &ulBytesRead, FALSE );

      fOK = (usDosRc == 0 );

      // Check if the entire file was read
      if ( sizeof(PROPDICTIONARY) != ulBytesRead )
      {
        fOK = FALSE;
      } /* endif */
   } /* endif */

   if ( hInputfile )
   {
      UtlClose( hInputfile, FALSE );          // Close Input file if needed
   } /* endif */

   return ( (USHORT)((fOK) ? LX_RC_OK_ASD : LX_OPEN_FLD_ASD ));

} /* end of AsdLoadDictProperties */

//+----------------------------------------------------------------------------+
//| AsdAddToIndexData         - add a term to the data part of an index entry  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Adds a given term to one of the term lists in the data of an index      |
//|    entry. If necessary, the data area is enlarged using UtlAlloc.          |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    check input data;                                                       |
//|    if ok then                                                              |
//|       get length of new term;                                              |
//|       if index area has not enough room left then                          |
//|          enlarge index area and adjust new size;                           |
//|       endif;                                                               |
//|    endif;                                                                  |
//|    if ok then                                                              |
//|       insert new term at the end of the term list for the given term type; |
//|    endif;                                                                  |
//|    return Nlp return code to caller;                                       |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    USHORT   usTermType      IN          type of term (from TERMTYPE enum)  |
//|    PSZ      pszTerm         IN          term to be added                   |
//|    PUCHAR   *ppIndexData    IN/OUT      ptr to index data area             |
//|    PUSHORT  pusDataSize     IN/OUT      size of index data area            |
//|    PUSHORT  pusDataUsed     IN/OUT      number of bytes used in index data |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        successful completion          |
//|                         LX_...              Nlp return codes               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdAddToIndexData
(
   USHORT   usTermType,                // type of term (from TERMTYPE enum)
   PSZ_W    pszTerm,                   // term to be added
   PBYTE    *ppIndexData,              // ptr to index data area
   PULONG   pulDataSize,               // BYTES of index data area
   PULONG   pulDataUsed,               // BYTES  used in index data
   BOOL     fSort                      // TRUE = sort term into term list
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   ULONG     ulEndOffset;              // offsets in index data area
   ULONG     ulOffset;                 // offsets in index data area
   ULONG     ulTermLength = 0;         // length of term in BYTES
   PBYTE     pucTermListEnd;           // pointer to end of a term list
   PBYTE     pucTermList;              // pointer to start of a term list
   BOOL      fOK = TRUE;               // OK flag returned by UtlAlloc
   USHORT    usTempLen = 0;

   //
   // check input data
   //
   if ( !pszTerm || !*ppIndexData || (usTermType >= LASTTERMTYPE) )
   {
      usNlpRC = LX_UNEXPECTED_ASD;
   } /* endif */

   //
   // enlarge index data area if necessary and possible!
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      ulTermLength = UTF16strlenBYTE(pszTerm);  // get length of term to be inserted

      // no size restriction anymore...
      //if ( ((*pulDataUsed) + ulTermLength + sizeof(USHORT) ) >= MAXDATASIZE )
      //{
      //  /**************************************************************/
      //  /* Max. record size for QDAM records has been reached         */
      //  /**************************************************************/
      //  usNlpRC = LX_IDX_DAT_SIZE;
      //}
      //else
      {
        while ( fOK && ( (*pulDataUsed + ulTermLength + sizeof(USHORT)) >= *pulDataSize ) )
        {
           fOK = UtlAlloc( (PVOID *)ppIndexData, (*pulDataSize), (*pulDataSize + INDEX_SIZE_INCR), NOMSG );
           if ( fOK )
           {
              *pulDataSize += INDEX_SIZE_INCR;
           }
           else
           {
              usNlpRC = LX_MEM_ALLOC_ASD;
           } /* endif */
        } /* endwhile */
      } /* endif */
   } /* endif */

   //
   // insert the new term
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     PASDINDEXENTRYHEADER pHeader = (PASDINDEXENTRYHEADER)*ppIndexData;

      pucTermList  = *ppIndexData;          // get start of index data

      // get offset to end of data for given term type
      ulEndOffset     = pHeader->ulTermOffs[usTermType+1];
      pucTermListEnd  = pucTermList + ulEndOffset;  // position to end of list
      ulOffset     = pHeader->ulTermOffs[usTermType];
      pucTermList  = pucTermList + ulOffset;     // position to begin of list

      // make room for new term
      if ( *pulDataUsed != ulEndOffset )       // if not at end of data ...
      {
         memmove( pucTermListEnd + ulTermLength + sizeof(USHORT), pucTermListEnd, (*pulDataUsed - ulEndOffset) );
      } /* endif */

      if ( fSort )
      {
        // get insert position in term list
        usTempLen =  (*((PUSHORT)pucTermList) );
        while ( ( pucTermList < pucTermListEnd ) && (usTempLen > ulTermLength) )
        {
           pucTermList += usTempLen + sizeof(USHORT);   // go to next entry in list
           usTempLen =  (*((PUSHORT) pucTermList) );
        } /* endwhile */

        // add or insert new term
        if ( pucTermList < pucTermListEnd )
        {
           // insert new term at found location
           memmove( (pucTermList + ulTermLength + sizeof(USHORT)),
                    (pucTermList), pucTermListEnd - pucTermList );
           *((PUSHORT)pucTermList) =  (USHORT)ulTermLength;      // store term length information
           memcpy(  pucTermList + sizeof(USHORT), pszTerm, ulTermLength );        // store term data
        }
        else
        {
           // add term to end of list
           *((PUSHORT)pucTermListEnd) =  (USHORT)ulTermLength;    // store length and
           memcpy( pucTermListEnd + sizeof(USHORT), pszTerm, ulTermLength );                  // data of term
        } /* endif */
      }
      else
      {
         // add term to end of list
         *((PUSHORT)pucTermListEnd) =  (USHORT)ulTermLength;                 // store length and
         memcpy(  pucTermListEnd + sizeof(USHORT), pszTerm, ulTermLength);   // data of term
      } /* endif */


      *pulDataUsed += ulTermLength + sizeof(USHORT);          // adapt used size

      // adapt offsets of following data
      usTermType++;                              // start with next term type
      while ( usTermType <= LASTTERMTYPE )
      {
         pHeader->ulTermOffs[usTermType] += ulTermLength + sizeof(USHORT);
         usTermType++;                           // continue with next term type
      } /* endwhile */
   } /* endif */


   /*******************************************************************/
   /* Ignore index overflows ...                                      */
   /* (may be later on we should issue here a warning)                */
   /*******************************************************************/
   //if ( usNlpRC == LX_IDX_DAT_SIZE )
   //{
   //  usNlpRC = LX_RC_OK_ASD;
   //} /* endif */

   return( usNlpRC );

} /* endof AsdAddToIndexData */

//+----------------------------------------------------------------------------+
//| AsdDelFromIndexData       - delete a term from the data part of an index   |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Deletes/Removes a term from a specific term list of an index            |
//|    entry. If the term is not contained in the term list, no action         |
//|    is performed and no return code will be set.                            |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    check input data;                                                       |
//|    if ok then                                                              |
//|       locate term in term list using AsdLocInIndex;                        |
//|    endif;                                                                  |
//|    if ok then                                                              |
//|       if term found then                                                   |
//|          remove term from term list and adjust term offsets;               |
//|       endif;                                                               |
//|    endif;                                                                  |
//|    return Nlp return code to caller;                                       |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    USHORT   usTermType      IN          type of term (from TERMTYPE enum)  |
//|    PSZ      pszTerm         IN          term to be removed                 |
//|    USHORT   usTerms         IN          number of subsequent terms to      |
//|                                         remove from index data             |
//|    PUCHAR   pIndexData      IN          ptr to index data area             |
//|    PUSHORT  pusDataUsed     IN/OUT      number of bytes used in index data |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        successful completion          |
//|                         LX_...              Nlp return codes               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdDelFromIndexData
(
   USHORT   usTermType,                // type of term (from TERMTYPE enum)
   PSZ_W    pszTerm,                   // term to be removed
   USHORT   usTerms,                   // number of subsequent terms to remove
   PBYTE    pIndexData,                // ptr to index data area
   PULONG   pulDataUsed                // number of bytes used in index data
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   USHORT    usOffset;                 // offsets in index data area
   ULONG     ulTermLength;             // length of term
   PBYTE     pucTemp = NULL;           // pointer for index data processing

   //
   // check input data
   //
   if ( !pszTerm || !pIndexData || (usTermType >= LASTTERMTYPE) )
   {
      usNlpRC = LX_UNEXPECTED_ASD;
   } /* endif */

   //
   // locate term in term list
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      usNlpRC = AsdLocInIndex( usTermType, pszTerm, pIndexData, &pucTemp );
   } /* endif */

   //
   // if found remove the term
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      if ( pucTemp )                             // if term found ...
      {
         PBYTE  pucNextTerm;

         while ( usTerms )
         {
           // remember current position in term list
           pucNextTerm = pucTemp;

           // get end offset of found term
           ulTermLength = *(PUSHORT)pucTemp + sizeof(USHORT);
           usOffset     = (USHORT)(pucTemp - (PBYTE)pIndexData + ulTermLength);

           // remove term
           if ( *pulDataUsed != usOffset )       // if not at end of data ...
           {
              memmove( pucTemp, pucTemp + ulTermLength, *pulDataUsed - usOffset );
           } /* endif */
           *pulDataUsed = *pulDataUsed - ulTermLength;           // correct used size

           // adapt offsets of following data
           {
             PASDINDEXENTRYHEADER pHeader = (PASDINDEXENTRYHEADER)pIndexData;
             USHORT usType = usTermType + 1;       // start with next term type
             while ( usType <= LASTTERMTYPE )
             {
               pHeader->ulTermOffs[usType] = pHeader->ulTermOffs[usType] - ulTermLength;
               usType++;
             } /* endwhile */
           }

           // prepare removed of next term
           usTerms--;
           pucTemp = pucNextTerm;
         } /* endwhile */
      } /* endif */
   } /* endif */

   return( usNlpRC );

} /* endof AsdDelFromIndexData */

//+----------------------------------------------------------------------------+
//| AsdLocInIndex       - locate a term in the data part of an index           |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Locates a term from a specific term list of an index entry.             |
//|    If the term is not contained in the term list, the term pointer is      |
//|    set to NULL but no error return code is set.                            |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    check input data;                                                       |
//|    if ok then                                                              |
//|       compute start and end pointers for search;                           |
//|       while not end of TaskList and not found do                           |
//|          if current term is not the term we are looking for then           |
//|             go to next term in list                                        |
//|          endif;                                                            |
//|       endwhile;                                                            |
//|     endif;                                                                 |
//|     set caller's found term pointer to term if found or to NULL if term    |
//|        was not found;                                                      |
//|     return Nlp return code;                                                |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    USHORT   usTermType      IN          type of term (from TERMTYPE enum)  |
//|    PSZ      pszTerm         IN          term to locate in term list        |
//|    PUCHAR   pIndexData      IN          ptr to index data area             |
//|    PUCHAR   *ppTermLoc      OUT         location of term in index data     |
//|                                         or NULL if term is not in term list|
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        successful completion          |
//|                         LX_...              Nlp return codes               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdLocInIndex
(
   USHORT   usTermType,                // type of term (from TERMTYPE enum)
   PSZ_W    pszTerm,                   // term to be added
   PBYTE    pIndexData,                // ptr to index data area
   PBYTE    *ppTermLoc                 // location of term in index data
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   ULONG     ulOffset;                 // offsets in index data area
   USHORT    usTermLength;             // length of term
   PBYTE     pucTemp = NULL;           // pointer for index data processing
   PBYTE     pucEnd;                   // pointer to begin of next term list
   BOOL      fFound = FALSE;           // term found flag
   PASDINDEXENTRYHEADER pHeader = (PASDINDEXENTRYHEADER)pIndexData;

   //
   // check input data
   //
   if ( !pszTerm || !pIndexData || (usTermType >= LASTTERMTYPE) )
   {
      usNlpRC = LX_UNEXPECTED_ASD;
   } /* endif */

   //
   // locate term in term list
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      // get pointer to start of index data
      pucTemp      = pIndexData;

      // get pointer to end of list
      ulOffset     = pHeader->ulTermOffs[usTermType+1];
      pucEnd       = pucTemp + ulOffset;

      // get pointer to start of term list
      ulOffset     = pHeader->ulTermOffs[usTermType];
      pucTemp      = pucTemp + ulOffset;

      // get length of term
      usTermLength = (USHORT)UTF16strlenBYTE( pszTerm );

      // loop through term list and look for term
      while ( !fFound && (pucTemp < pucEnd) )
      {
         if ( (usTermLength == *((PUSHORT)pucTemp)) &&
              (memcmp( (PBYTE)pszTerm, pucTemp + sizeof(USHORT), usTermLength ) == 0 ) )
         {
            fFound = TRUE;
         }
         else
         {
            pucTemp += *((PUSHORT)pucTemp) + sizeof(USHORT);   // go to next entry in list
         } /* endif */
      } /* endwhile */

   } /* endif */

   // set caller's term found pointer
   *ppTermLoc = ( fFound ) ? pucTemp : NULL;

   return( usNlpRC );

} /* endof AsdLocInIndex */

//+----------------------------------------------------------------------------+
//| AsdExtractTerms         - extracts specific entries from dictionary data   |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Extracts entries from dictionary data (flat LDB format) and store       |
//|    the retrieved entries in a buffer area. The entries extracted are       |
//|    the entries for which the code names are stored in the DCB.             |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|   HDCB    hDCB                   IN      dictionary control block          |
//|   USHORT  usAction               IN      value to store in action field    |
//|   PUCHAR  pucDictData            IN      ptr to dictionary data            |
//|   USHORT  usDataLength           IN      length of dictionary data         |
//|   PUSHORT pusTerms               IN/OUT  number of terms extracted         |
//|   PUCHAR  **pucTerms             IN/OUT  ptr to extracted terms            |
//|   PUSHORT pusSize                IN/OUT  size of term list                 |
//|   PUSHORT pusUsed                IN/OUT  used bytes in term list           |
//|   PSZ     pszTerm                IN      currently processed term          |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_...              error(s) occured               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdExtractTerms
(
   HDCB    hDCB,                       // dictionary control block
   USHORT  usAction,                   // value to store in action field
   PSZ_W   pucDictData,                // ptr to dictionary data
   ULONG   ulDataLength,               // length of dictionary data in CHARW's
   PUSHORT pusTerms,                   // number of terms extracted
   PSZ_W   *ppucTerms,                 // ptr to task list
   PULONG  pulSize,                    // current size of task list
   PULONG  pulUsed,                    // bytes used in task list
   PSZ_W   pszTerm                     // currently processed term
)
{
   USHORT   usNlpRC = LX_RC_OK_ASD;    // return code of Nlp call
   PDCB     pDCB;                      // pointer to dictionary control block
   PSZ_W    pucEntry;                  // ptr to current entry
   USHORT   usTermType;                // type of term/entry found
   USHORT   usEntryLength;             // length of current entry
   USHORT   usTermLength;              // length of inserted entry
   PSZ_W    pucTermEnd;                // ptr to end of term
   USHORT   usLevel = 0;               // current level returned by QLDBNextNode
   USHORT   usLdbRC;                   // return code of QLDB functions
   USHORT   usField;                   // field number
   PVOID    hLdbTree;                  // QLDB tree handle

   pszTerm;                            // avoid compiler warning

   /*******************************************************************/
   /* Determine type of LDB record and perform the approbriate        */
   /* processing                                                      */
   /*******************************************************************/
   if ( *pucDictData == QLDB_FIRST_LEVEL )
   {
     /*****************************************************************/
     /* Record is in QLDB format                                      */
     /*****************************************************************/

     /*****************************************************************/
     /* Initialize variables                                          */
     /*****************************************************************/
     pDCB = (PDCB) hDCB;                         // convert handle to DCB pointer

     /*****************************************************************/
     /* Build node tree                                               */
     /*****************************************************************/
     hLdbTree = NULL;
     usLdbRC = QLDBRecordToTree( pDCB->ausNoOfFields, pucDictData,
                                 ulDataLength, &hLdbTree );

     /*****************************************************************/
     /* Loop through nodes of LDB tree                                */
     /*****************************************************************/
     if ( usLdbRC == QLDB_NO_ERROR )
     {
       usLdbRC = QLDBCurrNode( hLdbTree, pDCB->apszFields, &usLevel );
     } /* endif */

     while ( (usLdbRC == QLDB_NO_ERROR) && (usLevel != QLDB_END_OF_TREE))
     {
       /***************************************************************/
       /* extract terms from current node                             */
       /***************************************************************/
       usField = 0;
       while ( usField < pDCB->ausNoOfFields[usLevel-1] )
       {
         /*************************************************************/
         /* Look for special fields                                   */
         /*************************************************************/
         if ( *pDCB->apszFields[usField] )
         {
           if ( (usLevel == pDCB->SynField.usLevel) &&
                (usField == pDCB->SynField.usIndex) )
           {
              usTermType = SYNONYM_TYPE;
           }
           else if ( (usLevel == pDCB->AbbrField.usLevel) &&
                     (usField == pDCB->AbbrField.usIndex) )
           {
              usTermType = ABBREV_TYPE;
           }
           else if ( (usLevel == pDCB->RelField.usLevel) &&
                     (usField == pDCB->RelField.usIndex) )
           {
              usTermType = RELATED_TYPE;
           }
           else
           {
              usTermType = LASTTERMTYPE;
           } /* endif */

           /*************************************************************/
           /* add special terms to task list                            */
           /*************************************************************/
           if ( usTermType != LASTTERMTYPE )
           {
              pucEntry      = pDCB->apszFields[usField];
              usEntryLength = (USHORT)UTF16strlenCHAR(pucEntry);

              do {
                 // search end of first term
                 pucTermEnd = pucEntry;
                 while ( usEntryLength && (*pucTermEnd != ',') )
                 {
                    usEntryLength--;
                    pucTermEnd++;
                 } /* endwhile */

                 usTermLength = (USHORT)(pucTermEnd - pucEntry);

                 if ( usTermLength )                    // if term is not empty ...
                 {
                   PBYTE  pbTerms;
                   pbTerms = (PBYTE)(* ppucTerms);
                   usNlpRC = AsdAddToTaskList( usTermType,
                                                usAction,
                                                pucEntry,
                                                (USHORT)(min(usTermLength,MAX_TERM_LEN-1)),
                                                pusTerms,
                                                &pbTerms, /*ppucTerms,*/
                                                pulSize,
                                                pulUsed );
                   *ppucTerms = (PSZ_W)pbTerms;
                 } /* endif */

                 // skip to next term
                 while ( usEntryLength &&
                         ((*pucTermEnd == ',') || (*pucTermEnd == ' ') ) )
                 {
                    usEntryLength--;
                    pucTermEnd++;
                 } /* endwhile */
                 pucEntry = pucTermEnd;

              } while ( usEntryLength ); /* enddo */
           } /* endif */
         } /* endif */

         /*************************************************************/
         /* continue with next field                                  */
         /*************************************************************/
         usField++;
       } /* endwhile */

       /***************************************************************/
       /* Continue with next node                                     */
       /***************************************************************/
       usLdbRC = QLDBNextNode( hLdbTree, pDCB->apszFields, &usLevel );
     } /* endwhile */

     /*****************************************************************/
     /* Destroy node tree                                             */
     /*****************************************************************/
     QLDBDestroyTree( &hLdbTree );
   }
   else
   {
     /*****************************************************************/
     /* Record is in old LDB format                                   */
     /*****************************************************************/
     usNlpRC = LX_INCOMP_SIG_ASD;
   } /* endif */

#if defined(USE_COMPOUND_LIST)
   /*******************************************************************/
   /* Extract compounds of term                                       */
   /*******************************************************************/
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     do
     {
       /***************************************************************/
       /* look for end of current compound                            */
       /***************************************************************/
       pucData = pszTerm;
       while ( (*pucData != EOS) && (*pucData != BLANK) )
       {
         pucData++;
       } /* endwhile */

       /***************************************************************/
       /* Add current compound to task list                           */
       /***************************************************************/
       if ( (pucData - pszTerm) != 0 )
       {
         usNlpRC = AsdAddToTaskList( COMPOUND_TYPE,
                                     usAction,
                                     pszTerm,
                                     pucData - pszTerm,
                                     pusTerms,
                                     ppucTerms,
                                     pusSize,
                                     pusUsed );
       } /* endif */

       pszTerm = pucData;
       if ( *pszTerm == BLANK )
       {
         pszTerm++;
       } /* endif */
     } while ( (usNlpRC == LX_RC_OK_ASD) && (*pszTerm != EOS) ); /* enddo */
   } /* endif */
#endif

   return( usNlpRC );

} /* endof AsdExtractTerms */

//+----------------------------------------------------------------------------+
//| AsdGetIndexEntry        - read index entry for a given term                |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Read an index entry into the given buffer. If the buffer is to small,   |
//|    it is enlarged.                                                         |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|   HUCB    hUCB                   IN      user control block                |
//|   HDCB    hDCB                   IN      dictionary control block          |
//|   PUCHAR  pucTerm                IN      dictionary entry headword         |
//|   PUCHAR  *ppucData              IN/OUT  ptr to index data buffer          |
//|   PUSHORT pusSize                IN/OUT  size of index data buffer         |
//|   PUSHORT pusUsed                OUT     # of bytes used in index data buf |
//|   PHDCB   phDictFoundDCB         OUT     handle of dictionary containing   |
//|                                          term                              |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_...              error(s) occured               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdGetIndexEntry
(
   HUCB    hUCB,                       // user control block
   HDCB    hDCB,                       // dictionary control block
   PSZ_W   pucTerm,                    // index entry term
   PBYTE   *ppucData,                  // ptr to index data buffer
   PULONG  pulSize,                    // size of index data buffer
   PULONG  pulUsed,                    // # of bytes used in index data buffer
   PHDCB   phDictFoundDCB              // handle of dictionary containing term
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   ULONG     ulTermNumber;             // number of term
   ULONG     ulDataLength = 0;         // data length of index entry
   DICTHANDLE usDictHandle;            // dictionary of index term
   BOOL      fOK;                      // flag returned by UtlAlloc

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // lookup index entry
   //
   if ( ASDOK(usNlpRC) )
   {
      UTF16strcpy( pDCB->aucDummy, pucTerm );
      UtlLowerW( pDCB->aucDummy );
      NlpFndMatchAsdW( pDCB->aucDummy,            // term we are looking for
                      pDCB->usIndexHandle,       // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      (PBYTE) pDCB->aucDummy,            // matching term found
                      &ulTermNumber,             // number of term found
                      &ulDataLength,             // length of entry data in # of bytes
                      &usDictHandle,             // dictionary of match
                      &usNlpRC );
   } /* endif */

   //
   // adjust size of index data buffer
   //
   if ( ASDOK(usNlpRC) )
   {
      if ( ulDataLength > *pulSize )
      {
         fOK = UtlAlloc( (PVOID *)ppucData, *pulSize, ulDataLength, NOMSG );
         if ( fOK  )
         {
            *pulSize = ulDataLength;
         }
         else
         {
            usNlpRC = LX_MEM_ALLOC_ASD;
         } /* endif */
      } /* endif */
   } /* endif */

   //
   // get data of index entry
   //
   if ( ASDOK(usNlpRC) )
   {
         NlpRetEntryAsdW( usDictHandle,           // dictionary handle
                         pUCB->usUser,           // ASD user handle
                         pDCB->aucDummy,         // term for this entry
                         &ulTermNumber,          // number of term
                         (PBYTE)*ppucData,              // entry data
                         &ulDataLength,          // data length in # of bytes
                         &usDictHandle,          // dictionary of term
                         &usNlpRC );
         if ( ASDOK(usNlpRC) )
         {
            // convert old type index records if necessary and get record size
            AsdPrepareIndexRecord( ppucData, pulSize, pulUsed );

            if ( pDCB->fAssoc )
            {
               // search dictionary containing the term
               while ( pDCB && (pDCB->usIndexHandle != usDictHandle) )
               {
                  pDCB = pDCB->pNextDCB;
               } /* endwhile */
               *phDictFoundDCB = ( pDCB ) ? (HDCB) pDCB : hDCB;
            }
            else
            {
               *phDictFoundDCB = hDCB;
            } /* endif */
         } /* endif */
   } /* endif */

   return( usNlpRC );
} /* endof AsdGetIndexEntry */

//+----------------------------------------------------------------------------+
//| AsdPutIndexEntry        - rewrites an index entry                          |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Rewrites an index entry to the index dictionary.                        |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|   HUCB    hUCB                   IN      user control block                |
//|   HDCB    hDCB                   IN      dictionary control block          |
//|   PUCHAR  pucTerm                IN      dictionary entry headword         |
//|   PUCHAR  pucData                IN      ptr to index data buffer          |
//|   PUSHORT pusUsed                IN      # of charws used in index data buf |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_...              error(s) occured               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdPutIndexEntry
(
   HUCB    hUCB,                       // user control block
   HDCB    hDCB,                       // dictionary control block
   PSZ_W   pucTerm,                    // index entry term
   PBYTE   pucData,                    // ptr to index data buffer
   ULONG   ulUsed                      // # of bytes used in index data buffer
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   ULONG     ulTermNumber;             // number of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   // 
   // Create an index entry which is compatible to older versions
   // 
   usNlpRC = AsdMakeCompatibleRecord( pucData, ulUsed );

   //
   // replace data of index entry
   //
   if ( ASDOK(usNlpRC) )
   {
      UTF16strcpy( pDCB->aucDummy, pucTerm );
      UtlLowerW( pDCB->aucDummy );
      NlpUpdEntryAsdW( pDCB->aucDummy,            // term being replaced
                      (PBYTE)pucData,            // new data for term
                      ulUsed,                    // length of new data in BYTES
                      pDCB->usIndexHandle,       // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      &ulTermNumber,             // number of term found
                      &usNlpRC );
   } /* endif */

   return( usNlpRC );

} /* endof AsdPutIndexEntry */

//+----------------------------------------------------------------------------+
//| AsdInsIndexEntry        - insert a new index entry                         |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Insert a new entry into the index dictionary.                           |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|   HUCB    hUCB                   IN      user control block                |
//|   HDCB    hDCB                   IN      dictionary control block          |
//|   PUCHAR  pucTerm                IN      dictionary entry headword         |
//|   PUCHAR  pucData                IN      ptr to index data buffer          |
//|   PUSHORT pusUsed                IN      # of bytes used in index data buf |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_...              error(s) occured               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdInsIndexEntry
(
   HUCB    hUCB,                       // user control block
   HDCB    hDCB,                       // dictionary control block
   PSZ_W   pucTerm,                    // index entry term
   PBYTE   pucData,                    // ptr to index data buffer
   ULONG   ulUsed                      // # of BYTES used in index data buffer
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   ULONG     ulTermNumber;             // number of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   // 
   // Create an index entry which is compatible to older versions
   // 
   usNlpRC = AsdMakeCompatibleRecord( pucData, ulUsed );

   //
   // replace data of index entry
   //
   if ( ASDOK(usNlpRC) )
   {
      UTF16strcpy( pDCB->aucDummy, pucTerm );
      UtlLowerW( pDCB->aucDummy );
      NlpInsEntryAsdW( pDCB->aucDummy,            // term being replaced
                      (PBYTE)pucData,                   // new data for term
                      ulUsed,                    // length of new data in BYTES
                      pDCB->usIndexHandle,       // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      &ulTermNumber,             // number of term found
                      &usNlpRC );
   } /* endif */

   return( usNlpRC );

} /* endof AsdInsIndexEntry */

//+----------------------------------------------------------------------------+
//| AsdDelIndexEntry        - delete an index entry                            |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Delete an entry from the index dictionary.                              |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|   HUCB    hUCB                   IN      user control block                |
//|   HDCB    hDCB                   IN      dictionary control block          |
//|   PUCHAR  pucTerm                IN      dictionary entry headword         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_...              error(s) occured               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdDelIndexEntry
(
   HDCB    hUCB,                       // user control block
   HDCB    hDCB,                       // dictionary control block
   PSZ_W   pucTerm                     // index entry term
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // delete index entry
   //
   if ( ASDOK(usNlpRC) )
   {
      UTF16strcpy( pDCB->aucDummy, pucTerm );
      UtlLowerW( pDCB->aucDummy );
      NlpDelEntryAsdW( pDCB->aucDummy,            // term being deleted
                      pDCB->usIndexHandle,       // dictionary handle
                      pUCB->usUser,              // ASD user handle
                      &usNlpRC );
   } /* endif */

   return( usNlpRC );

} /* endof AsdDelIndexEntry */

//+----------------------------------------------------------------------------+
//| AsdIsIndexEmpty         - checks if an index entry contains no data        |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    check if an index entry has no more data.                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|   PUCHAR  pucData                IN      ptr to index data buffer          |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL     fIsEmpty    function return code                               |
//|                         TRUE                index entry is empty           |
//|                         FALSE               index entry still has data     |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
BOOL AsdIsIndexEmpty
(
   PSZ_W   pucData                     // index data buffer
)
{
   BOOL    fIsEmpty;                   // index data is empty flag

   // Note: An index entry is empty if the offset for the termtype
   //       LASTTERMTYPE is the same as the offset for the first
   //       termtype
   PASDINDEXENTRYHEADER pHeader = (PASDINDEXENTRYHEADER)pucData;

   fIsEmpty = ( pHeader->ulTermOffs[0] == pHeader->ulTermOffs[LASTTERMTYPE] );

   return( fIsEmpty );
} /* endof AsdIsIndexEmpty */

//+----------------------------------------------------------------------------+
//| AsdUpdateIndex      Update index dictionary                                |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Process a task list containing terms to be added to or to be deleted    |
//|    from the index dictionary.                                              |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//|    HUCB     hUCB            IN          user control block handle          |
//|    PUCHAR   pucTaskList     IN          terms to be added or deleted       |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         0               if OK                              |
//|                         LX_xxxx         Nlp errors                         |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen or AsdBuild.                  |
//|    pucTaskList must have been created using AsdExtractTerms or             |
//|    AsdAddToTaskList.                                                       |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdUpdateIndex
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pucHeadword,               // headword of dictionary entry
   USHORT   usTerms,                   // number of terms intask list
   PSZ_W    pucTaskList                // terms to be added or deleted
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   PBYTE     pucIndexData = NULL;      // index entry data
   ULONG     ulIndexSize = 0;          // current size of index entry data in BYTES!!
   ULONG     ulIndexUsed = 0;          // current used BYTES of index entry data
   PTERMBUFENTRY pTermEntry;           // pointer to entry in index process list
   BOOL      fNewEntry;                // TRUE = this is a new index entry
   BOOL      fChanged;                 // TRUE = index data has been changed
   HDCB      hDictFoundDCB;            // handle of dictionary containing term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // process the task list and update index dictionary
   //
   pTermEntry = (PTERMBUFENTRY) pucTaskList;
   while ( ASDOK(usNlpRC) && usTerms )
   {
      usNlpRC = AsdGetStemForm( hUCB, hDCB, (PSZ_W)pTermEntry->ucTerm,
                                pUCB->ucStemTerm );
      if ( ASDOK(usNlpRC) )
      {
         // get the index entry
         fChanged = FALSE;             // nothing changed yet
         usNlpRC = AsdGetIndexEntry( hUCB, hDCB, pUCB->ucStemTerm,
                      &pucIndexData, &ulIndexSize, &ulIndexUsed,
                      &hDictFoundDCB );

         if ( usNlpRC == LX_WRD_NT_FND_ASD )
         {
            usNlpRC = LX_RC_OK_ASD;    // reset Nlp return code
            fNewEntry = TRUE;          // we are working on a new entry
            usNlpRC = AsdMakeIndexEntry( &pucIndexData, &ulIndexSize, &ulIndexUsed );
           //usIndexSize + usIndexUsed is in bytes!!
         }
         else
         {
            fNewEntry = FALSE;         // we are working on an existing entry
         } /* endif */

         // perform the requested action
         switch ( pTermEntry->usAction )
         {
            case BUILD_ACTION:
               // used to create empty entries, do nothing here
               break;

            case DELETE_ACTION:
               // remove term from index data
               if ( ASDOK(usNlpRC) )
               {
                  usNlpRC = AsdDelFromIndexData( pTermEntry->usTermType, 
                               pucHeadword, 1, pucIndexData, &ulIndexUsed );
                  fChanged = TRUE;               // index data has been changed
               } /* endif */

               /*******************************************************/
               /* For MWT updates process the entry in MWTSTEM_TYPE   */
               /* list also                                           */
               /*******************************************************/
               if ( ASDOK(usNlpRC) && (pTermEntry->usTermType == MWT_TYPE) )
               {
                 /******************************************************/
                 /* Check if stem form of term differs from original term */
                 /******************************************************/
                 if ( ASDOK(usNlpRC) )
                 {
                     MorphMultStemForm( pDCB->sLangID,
                                      pucHeadword,
                                      sizeof(pUCB->ucTermBuf)/sizeof(CHAR_W),
                                      pUCB->ucTermBuf,
                                      pDCB->ulOemCP);

                   if ( UTF16strcmp( pucHeadword, pUCB->ucTermBuf ) != 0 )
                   {
                     usNlpRC = AsdDelFromIndexData( MWTSTEM_TYPE,
                                  pucHeadword, 2, pucIndexData, &ulIndexUsed );
                     fChanged = TRUE;               // index data has been changed
                   } /* endif */
                 } /* endif */
               } /* endif */
               break;

            case INSERT_ACTION:
               // add term from index data
               if ( ASDOK(usNlpRC) )
               {
                  usNlpRC = AsdAddToIndexData( pTermEntry->usTermType,
                               pucHeadword, &pucIndexData, &ulIndexSize,
                               &ulIndexUsed, TRUE );
                  fChanged = TRUE;              // index data has been changed
               } /* endif */

               if ( pTermEntry->usTermType == MWT_TYPE )
               {
                 /*************************************************************/
                 /* If stem form of term differs from original term, add      */
                 /* term to MWTSTEM_TYPE list                                 */
                 /*************************************************************/
                 if ( usNlpRC == LX_RC_OK_ASD )
                 {
                   MorphMultStemForm( pDCB->sLangID, pucHeadword,
                                      sizeof(pUCB->ucTermBuf)/sizeof(CHAR_W),
                                      pUCB->ucTermBuf,
                                      pDCB->ulOemCP);
                   if ( UTF16strcmp( pucHeadword, pUCB->ucTermBuf ) != 0 )
                   {
                     usNlpRC = AsdAddToIndexData( MWTSTEM_TYPE,
                                                  pucHeadword,
                                                  &pucIndexData, &ulIndexSize,
                                                  &ulIndexUsed, FALSE );
                     fChanged = TRUE;            // index data has been changed
                     if ( usNlpRC == LX_RC_OK_ASD )
                     {
                       usNlpRC = AsdAddToIndexData( MWTSTEM_TYPE,
                                                    pUCB->ucTermBuf,
                                                    &pucIndexData, &ulIndexSize,
                                                    &ulIndexUsed, FALSE );
                     } /* endif */
                   } /* endif */
                 } /* endif */
               } /* endif */
               break;
         } /* endswitch */

         // rewrite or insert index entry
         if ( ASDOK(usNlpRC) )
         {
            if ( fNewEntry )
            {//5th param = # of BYTES
               usNlpRC = AsdInsIndexEntry( hUCB, hDCB, pUCB->ucStemTerm,
                                           (PBYTE) pucIndexData, ulIndexUsed );
            }
            else if ( fChanged )
            {  // 5th param = # of BYTES
               usNlpRC = AsdPutIndexEntry( hUCB, hDCB, pUCB->ucStemTerm,
                                           (PBYTE)pucIndexData, ulIndexUsed );
            } /* endif */
         } /* endif */


      } /* endif */
      // usLength is number of bytes til next TermBufEntry
      pTermEntry = (PTERMBUFENTRY) ((PUCHAR)pTermEntry + pTermEntry->usLength);
      usTerms--;
   } /* endwhile */

   if ( pucIndexData )
   {
      UtlAlloc( (PVOID *)&pucIndexData, 0L, 0L, NOMSG );
   } /* endif */

   return( usNlpRC );
} /* end of AsdUpdateIndex */


//+----------------------------------------------------------------------------+
//| AsdMakeIndexEntry       - create an empty index dictionary record          |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Create the record for an empty index dictionary entry.                  |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|   PUCHAR  *ppucData              IN/OUT  ptr to index data buffer          |
//|   PUSHORT pusSize                IN/OUT  size of index data buffer         |
//|   PUSHORT pusUsed                OUT     # of bytes used in index data buf |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_...              error(s) occured               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdMakeIndexEntry
(
   PBYTE  *ppucData,                  // ptr to index data buffer
   PULONG  pulSize,                    // # of bytes of index data buffer
   PULONG  pulUsed                     // # of bytes used in index data buffer
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   ULONG     ulDataLength;             // data length of index entry
   USHORT    usI;                      // loop counter
   BOOL      fOK;                      // flag returned by UtlAlloc

   //
   // adjust size of index data buffer
   //
   if ( ASDOK(usNlpRC) )
   {
      ulDataLength = max( sizeof(ASDINDEXENTRYHEADER), INDEX_SIZE_INCR );
      if ( ulDataLength > *pulSize )
      {
         fOK = UtlAlloc( (PVOID *)ppucData, *pulSize, ulDataLength, NOMSG );
         if ( fOK  )
         {
            *pulSize = ulDataLength;
         }
         else
         {
            usNlpRC = LX_MEM_ALLOC_ASD;
         } /* endif */
      } /* endif */
   } /* endif */

   //
   // build empty index entry
   //
   if ( ASDOK(usNlpRC) )
   {
     PASDINDEXENTRYHEADER pHeader = (PASDINDEXENTRYHEADER)*ppucData;
     memset( pHeader, 0, sizeof(ASDINDEXENTRYHEADER) );
     *pulUsed = sizeof(ASDINDEXENTRYHEADER);
     for ( usI = 0; usI <= LASTTERMTYPE; usI++)
     {
       pHeader->ulTermOffs[usI] = (USHORT)*pulUsed;
     } /* endfor */
   } /* endif */

   return( usNlpRC );
} /* endof AsdMakeIndexEntry */

//+----------------------------------------------------------------------------+
//| AsdAddToTaskList        - add a new task to a task list                    |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Add a new task to a task list and enlarge task list if current task     |
//|    list size is exceeded.                                                  |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    USHORT    usTermType          IN      type of term                      |
//|    USHORT    usAction            IN      action to be performed on term    |
//|    PUCHAR    pucTerm             IN      term being added                  |
//|    USHORT    usTermLength        IN      length of term being added        |
//|    PUSHORT   pusTerms            IN/OUT  number of terms in task list      |
//|    PUCHAR    *ppucTaskList       IN/OUT  ptr to task list                  |
//|    PUSHORT   pusSize             IN/OUT  current size of task list         |
//|    PUSHORT   pusUsed             IN/OUT  used bytes in task list           |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_MEM_ALLOC_ASD    memory allocation failed       |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdAddToTaskList
(
   USHORT   usTermType,                // type of term
   USHORT   usAction,                  // action to be performed on term
   PSZ_W    pucTerm,                   // term being added
   USHORT   usTermLength,              // length of term in char_w's
   PUSHORT  pusTerms,                  // number of terms in task list
   PBYTE    *ppucTaskList,             // ptr to task list
   PULONG   pulSize,                   // current size of task list: must be in Bytes!!
   PULONG   pulUsed                    // used bytes in task list: must be in bytes!!
 )
{
   USHORT        usNlpRC = LX_RC_OK_ASD; // return code of Nlp call(s)
   USHORT        usEntryLength;        // length of current entry in bytes
   PSZ_W         pucTemp;
   PTERMBUFENTRY pTermEntry;           // ptr to current entry
   BOOL          fOK = TRUE;           // return code of UtlAlloc function

   usEntryLength = usTermLength * sizeof(CHAR_W) + sizeof(TERMBUFENTRY);

   //
   // enlarge task list if required
   //
   while ( fOK && ((*pulUsed + usEntryLength) >= *pulSize ))
   {
      fOK = UtlAlloc( (PVOID *)ppucTaskList, *pulSize, *pulSize + TERM_BUFFER_INCR, NOMSG );
      if ( fOK )
      {
         *pulSize += TERM_BUFFER_INCR;
      }
      else
      {
         usNlpRC = LX_MEM_ALLOC_ASD;
      } /* endif */
   } /* endwhile */

   //
   // add new entry to list
   //
   if ( ASDOK(usNlpRC) )
   {
      pucTemp = (PSZ_W)(*ppucTaskList + *pulUsed);
      pTermEntry = (PTERMBUFENTRY) pucTemp;
      pTermEntry->usLength   = usEntryLength;     // in number of bytes
      pTermEntry->usAction   = usAction;
      pTermEntry->usTermType = usTermType;
      memcpy( (PBYTE)pTermEntry->ucTerm, (PBYTE)pucTerm, usTermLength * sizeof(CHAR_W) );

      pTermEntry->ucTerm[usTermLength] = EOS;
      *pulUsed = *pulUsed + usEntryLength;
      *pusTerms += 1;
   } /* endif */

   return( usNlpRC );
} /* end of AsdAddToTaskList */


//+----------------------------------------------------------------------------+
//| AsdIsMWT                - check if a given term is a multi word term       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Checks if the given term is a multi word term. For MWTs the length      |
//|    of the first word of the term is calculated.                            |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PUCB      pUCB                IN      user control block pointer        |
//|    PDCB      pDCB                IN      dictionary control block pointer  |
//|    PUCHAR    pucTerm             IN      term being checked                |
//|    PUSHORT   pusFirstWordLength  OUT     length of first word of a MWT     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL     fMWT        function return code                               |
//|                         TRUE                term is a MWT                  |
//|                         FALSE               term is no MWT                 |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
BOOL AsdIsMWT
(
   PUCB     pUCB,                      // user control block pointer
   PDCB     pDCB,                      // dictionary control block pointer
   PSZ_W    pucTerm,                   // term being checked
   PUSHORT  pusFirstWordLength         // length of first word of the MWT in char_w's
)
{
   PSZ_W   pucTest;                    // ptr for term processing
   BOOL    fMWT;                       // term is MWT flag

   pucTest = pucTerm;                  // start at begin of term

   while ( *pucTest == SPACE )         // skip spaces at the begin of the term
   {
      pucTest++;
   } /* endwhile */
   /*******************************************************************/
   /* MWTs has to be determined differently in case of DBCS code page */
   /*******************************************************************/
 //  if ( _dbcs_cp != DBCS_CP )
 //  {
 //    while ( *pucTest && (*pucTest != SPACE) )  // skip non-space characters
 //    {
 //       pucTest++;
 //    } /* endwhile */
//
 //    if ( *pucTest )                     // not at end of term ???
 //    {
 //       // term could be a MWT
 //       fMWT = TRUE;
 //       *pusFirstWordLength = pucTest - pucTerm; // is in charw's!!
 //    }
 //    else
 //    {
 //       // the term is definitely NO MWT
 //       fMWT = FALSE;
 //       *pusFirstWordLength = 0;
 //    } /* endif */
 //  }
 //  else
   {
     // use AsdTokenize to determine if we are dealing with MWTs
     USHORT usRc;
     USHORT usNumTerms;   // number of retrieved terms
     PTERMLENOFFS pTermList = NULL;
     usRc = AsdTokenize( pUCB, pDCB, pucTest, &usNumTerms, &pTermList );
     if ( (usRc == 0) && (usNumTerms>1))
     {
        // term could be a MWT
        fMWT = TRUE;
        // WordLength=Length+Offset otherwise might be negative
        *pusFirstWordLength = pTermList->usLength + pTermList->usOffset;
     }
     else
     {
        // the term is definitely NO MWT
        fMWT = FALSE;
        *pusFirstWordLength = 0;
     }
     /*****************************************************************/
     /* free allocated resources                                      */
     /*****************************************************************/
     if ( pTermList )
     {
       UtlAlloc( (PVOID *)&pTermList, 0L, 0L, NOMSG );
     } /* endif */
   }
   return( fMWT );

} /* end of AsdIsMWT */


//+----------------------------------------------------------------------------+
//| AsdGetNextCode          - gets the next node code                          |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PUCHAR    pucCode             IN/OUT  node code                         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    Nothing                                                                 |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
VOID AsdNextCode
(
   PUCHAR pucCode                      // node code
)
{
  if (pucCode [1] < 'Z')   // code is a term code
    pucCode [1]++;
  else if (pucCode [1] == 'Z')
  {
    pucCode [0]++;
    pucCode [1] = 'A';
  }
  else if (pucCode [1] < 'z') // code is a non-term code
    pucCode [1]++;
  else if (pucCode [1] == 'z')
  {
    pucCode [0]++;
    pucCode [1] = 'a';
  }
} /* end of AsdNextCode */

//+----------------------------------------------------------------------------+
//| AsdHandleToDCB          - convert a Nlp dictionary handle to a DCB handle  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PDCB pDCB,                    IN      dictionary control block          |
//|    USHORT usDictHandle           IN      handle returned by Nlp calls      |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    HDCB   hDCB                           dictionary control block handle or|
//|                                          NULL in case of errors            |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
HDCB AsdHandleToDCB
(
   PDCB pDCB,                          // dictionary control block
   DICTHANDLE usDictHandle             // handle returned by Nlp calls
)
{
   while ( pDCB && (pDCB->usDictHandle != usDictHandle) )
   {
      pDCB = pDCB->pNextDCB;
   } /* endwhile */

   return( (HDCB)pDCB );
} /* end of AsdHandleToDCB */


//+----------------------------------------------------------------------------+
//| AsdTokenize             - tokenize a string using NlpComplex               |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Tokenize a string and store all terms found in a termlist               |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PUCB      pUCB                IN      user control block                |
//|    PDCB      pDCB                IN      dictionary control block          |
//|    PUCHAR    pucString           IN      string being tokenized            |
//|    PUSHORT   pusTerms            OUT     number of terms found             |
//|    PTERMLENOFFS *ppTermList      OUT     ptr to created term list          |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_...              other Nlp error codes          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdTokenize
(
   PUCB      pUCB,                     // user control block
   PDCB      pDCB,                     // dictionary control block
   PSZ_W     pucString,                // string being tokenized
   PUSHORT   pusTerms,                 // number of terms found
   PTERMLENOFFS *ppTermList            // ptr to created term list
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   USHORT    usTermListSize = 0;       // allocated size of term list
   PUSHORT   pusTermList;              // ptr for term list processing

   pUCB;
   /*******************************************************************/
   /* Use tokenize function of morphologic functions                  */
   /*******************************************************************/
   *ppTermList = NULL;
   usNlpRC = MorphTokenizeW( pDCB->sLangID, pucString,
                            &usTermListSize,
                            (PVOID *)ppTermList,
                            MORPH_OFFSLIST, pDCB->ulOemCP );

   /*******************************************************************/
   /* Count returned terms or adapt return code                       */
   /*******************************************************************/
   if ( usNlpRC == MORPH_OK )
   {
     *pusTerms = 0;
     pusTermList = (PUSHORT)*ppTermList;
     while ( *pusTermList )
     {
       *pusTerms += 1;
       pusTermList += 2;
     } /* endwhile */
   }
   else
   {
     usNlpRC = AsdMorphRCToNlp( usNlpRC );
   } /* endif */

   return( usNlpRC );

} /* endof AsdTokenize */

//+----------------------------------------------------------------------------+
//| AsdGetField            - get field number and field level                  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Gets the level and the field number for a specific dictionary field.    |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|   address first profile entry                                              |
//|   while not end of profile and system name not found                       |
//|      adjust field number                                                   |
//|      skip to next profile entry                                            |
//|   endwhile;                                                                |
//|   if found then                                                            |
//|      copy level and field number to field structure                        |
//|   else                                                                     |
//|      set field structure to zero                                           |
//|   endif;                                                                   |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|   PFIELDDATA pField,             OUT     ptr to field data buffer          |
//|   PSZ     pszSysName,            IN      system name of field              |
//|   PPROPDICTIONARY  pProfile      IN      ptr to dictionary profile         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        code name is stored in pucCode |
//|                         LX_WRD_NT_FND_ASD   no field with the given name   |
//|                                             found                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdGetField
(
   PFIELDDATA       pField,            // ptr to field data buffer
   PSZ              pszSysName,        // system name of field
   PPROPDICTIONARY  pProfile           // ptr to dictionary profile
)
{
   PPROFENTRY   pProfEntry;            // pointer to profile entry
   USHORT       usField = 0;               // field index
   USHORT       usI;                   // loop index
   USHORT       usLastLevel = 0;       // last level

   pProfEntry = pProfile->ProfEntry;
   usI             = 0;
   pField->fInDict = FALSE;
   pField->usField = 0;
   pField->usLevel = 0;
   pField->usIndex = 0;

   while ( (usI < pProfile->usLength) && !pField->usLevel )
   {
     /*****************************************************************/
     /* handle level changes                                          */
     /*****************************************************************/
     if ( pProfEntry->usLevel > usLastLevel )
     {
       usLastLevel = pProfEntry->usLevel;
       usField     = 0;
     } /* endif */

     /*****************************************************************/
     /* Check if field was found                                      */
     /*****************************************************************/
     if ( stricmp(pProfEntry->chSystName, pszSysName) == 0)
     {
       pField->fInDict = TRUE;
       pField->usField = usI;
       pField->usLevel = usLastLevel;
       pField->usIndex = usField;
     } /* endif */

     /*****************************************************************/
     /* next entry                                                    */
     /*****************************************************************/
     pProfEntry++;
     usI++;
     usField++;
   } /* endwhile */

   return( (USHORT)((pField->fInDict) ? LX_RC_OK_ASD : LX_WRD_NT_FND_ASD ));
} /* endof AsdGetField */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdFillDBCSTable                                         |
//+----------------------------------------------------------------------------+
//|Function call:     AsdFillDBCSTable( BOOL fisDBCS[]);                       |
//+----------------------------------------------------------------------------+
//|Description:       Fills DBCS information fields                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   BOOL    fisDBCS[]     pointer to character recognition   |
//|                                         array                              |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     we are in a DBCS environment                    |
//|                   FALSE    we are in a SBCS environment                    |
//+----------------------------------------------------------------------------+
//BOOL AsdFillDBCSTable
//(
//   BOOL     fisDBCS1[]                 // DBCS character flag array
//)
//{  unsigned int cp;
//  BOOL        fDBCS = FALSE;           // TRUE = we are in a DBCS environment
//  memset( fisDBCS1, 0, 256 * sizeof(BOOL) );
//
//  cp = GetKBCodePage();
//  switch(cp)
//  {
//    case 874:                          // Thai
//    case 932:                          // Japanese
//    case 936:                          // Simplified Chinese
//    case 949:                          // Korean
//    case 950:                          // Chinese (Traditional)
//    case 1351:                         //
//      fDBCS = TRUE;           // TRUE = we are in a DBCS environment
//      break;
//  } /* endswitch */
//  if ( fDBCS )
//  {
//    int i;
//
//    for( i = 0; i < 256; i++ )
//    {
//      fisDBCS1[i] = (isdbcs1( i ) == DBCS_1ST);
//    } /* endfor */
//  } /* endif */
//
//  return( fDBCS );
//} /* end of function AsdFillDBCSTable */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdMWTMatch         Test for MWT match                   |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Compare a MWT term with the data in the segment.         |
//|                   Handle plural inflection of MWT or term in segment.      |
//|                                                                            |
//|                   To reduce the amount of compare operations, first the    |
//|                   base part of the MWT is compared with the segment data.  |
//|                   The base part is for MWTs ending with 's' the MWT        |
//|                   without the last three character for other MWT endings   |
//|                   the MWT without the last character.                      |
//|                   e.g.  "data windows"     ==> "data wind"                 |
//|                         "data histories"   ==> "data histor"               |
//|                         "data window"      ==> "data windo"                |
//|                   If the compare of the base part fails no more compares   |
//|                   are necessary as the terms will not match.               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   USHORT usMWTLength   length of MWT                       |
//|                   PUCHAR pucMWT        data of MWT                         |
//|                   USHORT usSegLength   length of data segment              |
//|                   PUCHAR pucSeg        data of segment                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE      current term in sgement matched multiword term |
//|                   FALSE     no match                                       |
//+----------------------------------------------------------------------------+
//|Function flow:     Compare base part of MWT and segment                     |
//+----------------------------------------------------------------------------+
BOOL AsdMWTMatch
(
  USHORT    usMWTLength,               // length of MWT
  PSZ_W     pucMWT,                    // data of MWT
  USHORT    usSegLength,               // length of data segment
  PSZ_W     pucSeg                     // data of segment
)
{
  BOOL      fMWTMatch = FALSE;         // function return code
  BOOL      fFullMatch;                // all-characters-match flag

  if ( (usMWTLength <= usSegLength) &&
       !memicmp( pucSeg, pucMWT, usMWTLength - 1) )
  {
    fFullMatch = !memicmp( pucSeg+(usMWTLength-1), pucMWT+(usMWTLength-1), 1 );

    if ( fFullMatch )
    {
      if ( !isalpha(pucSeg[usMWTLength]) )
      {
        /****************************************************************/
        /* We have an exact match!                                      */
        /****************************************************************/
        fMWTMatch = TRUE;
      }
      else if ( (tolower(pucSeg[usMWTLength]) == 's') &&
                !isalpha(pucSeg[usMWTLength+1]) )
      {
        /****************************************************************/
        /* terms seems to be in plural form                             */
        /****************************************************************/
        fMWTMatch = TRUE;
      }
      else if ( (tolower(pucSeg[usMWTLength]) == 'e') &&
                (tolower(pucSeg[usMWTLength+1]) == 's') &&
                !isalpha(pucSeg[usMWTLength+2]) )
      {
        /****************************************************************/
        /* terms seems to be in plural form build with "es"             */
        /****************************************************************/
        fMWTMatch = TRUE;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* check for special plural forms                               */
      /****************************************************************/
      switch ( tolower(pucMWT[usMWTLength-1] ) )
      {
        case 'x' :
          if ( (tolower(pucSeg[usMWTLength-1]) == 'c') &&
               (tolower(pucSeg[usMWTLength])   == 'e') &&
               (tolower(pucSeg[usMWTLength+1]) == 's') &&
               !isalpha(pucSeg[usMWTLength+2]) )
          {
            fMWTMatch = TRUE;
          } /* endif */
          break;

        case 'y' :
          if ( (tolower(pucSeg[usMWTLength-1]) == 'i') &&
               (tolower(pucSeg[usMWTLength])   == 'e') &&
               (tolower(pucSeg[usMWTLength+1]) == 's') &&
               !isalpha(pucSeg[usMWTLength+2]) )
          {
            fMWTMatch = TRUE;
          } /* endif */
          break;

        case 'f' :
          if ( (tolower(pucSeg[usMWTLength-1])  == 'v') &&
               (tolower(pucSeg[usMWTLength])    == 'e') &&
               (tolower(pucSeg[usMWTLength+1])  == 's') &&
               !isalpha(pucSeg[usMWTLength+2]) )
          {
            fMWTMatch = TRUE;
          } /* endif */
          break;

      } /* endswitch */
    } /* endif */
  } /* endif */


  return( fMWTMatch );

} /* end of function AsdMWTMatch */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdMorphRcToNlp  Convert Morph return code Nlp RC        |
//+----------------------------------------------------------------------------+
//|Function call:     usNlpRC = AsdMorphRcToNlp( USHORT usMorphRC );           |
//+----------------------------------------------------------------------------+
//|Description:       Convert a return code from Morph... functions to         |
//|                   a Nlp return code (LX_...).                              |
//+----------------------------------------------------------------------------+
//|Input parameter:   USHORT    usMorphRC      return code of Morph function   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       LX_ return codes                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
USHORT AsdMorphRCToNlp
(
  USHORT   usMorphRC                   // return code from Morph functions
)
{
  USHORT   usNlpRC;

  switch ( usMorphRC )
  {
    case MORPH_OK :                 usNlpRC = LX_RC_OK_ASD;               break;
    case MORPH_INV_LANG_ID :        usNlpRC = LX_BAD_LANG_CODE;           break;
    case MORPH_NO_MEMORY :          usNlpRC = LX_MEM_ALLOC_ASD;           break;
    case MORPH_NO_LANG_PROPS :      usNlpRC = LX_BAD_LANG_CODE;           break;
                                                                 /* 2@KIT1081A */
    case MORPH_ACT_DICT_FAILED :    usNlpRC = LX_BAD_LANG_CODE;           break;
    default :                       usNlpRC = LX_UNEXPECTED_RC;           break;
  } /* endswitch */

  return ( usNlpRC );
} /* end of function AsdMorphRcToNlp */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdCloseOrganize                                         |
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
//+----------------------------------------------------------------------------+
USHORT AsdCloseOrganize
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ      pszDictPath,               // full asd dict path of original
   CHAR     chPrimDrive,               // remote primary drive
   USHORT   usRc                       // return code from temp file close
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   USHORT    usCloseRC = LX_RC_OK_ASD; // first erraneous return code
   PUCB      pUCB;                     // pointer to user control block
   PDCB      pLastDCB = NULL;          // pointer to previous DCB
   PDCB      pDCB;                     // pointer to currently active DCB

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   // check input data
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */


   if ( usNlpRC == LX_RC_OK_ASD )
   {
      while ( pDCB &&                            // while not end of DCB chain
              (usNlpRC == LX_RC_OK_ASD) )        // and no error occured ...
      {
         // close index dictionary
         if ( pDCB->usIndexHandle )
         {
            NlpCloseAsd( pDCB->usIndexHandle, pUCB->usUser, &usNlpRC );
            if ( usNlpRC != LX_RC_OK_ASD )
            {
              /********************************************************/
              /* An error occured ==> remember return code if it is   */
              /* the first one, reset return code to LX_RC_OK_ASD to  */
              /* allow normal close of remaining dictionaries         */
              /********************************************************/
              if ( usCloseRC == LX_RC_OK_ASD )
              {
                usCloseRC = usNlpRC;
              } /* endif */
              usNlpRC = LX_RC_OK_ASD;
            } /* endif */
         } /* endif */

         //  close base dictionary
         if ( (usNlpRC == LX_RC_OK_ASD) && pDCB->usDictHandle )
         {
            NlpCloseOrganize( pDCB->usDictHandle, pszDictPath,
                              chPrimDrive, &usNlpRC, usRc );
            if ( usNlpRC != LX_RC_OK_ASD )
            {
              /********************************************************/
              /* An error occured ==> remember return code if it is   */
              /* the first one, reset return code to LX_RC_OK_ASD to  */
              /* allow normal close of remaining dictionaries         */
              /********************************************************/
              if ( usCloseRC == LX_RC_OK_ASD )
              {
                usCloseRC = usNlpRC;
              } /* endif */
              usNlpRC = LX_RC_OK_ASD;
            } /* endif */
         } /* endif */

         //  remember next DCB, free current one
         if ( usNlpRC == LX_RC_OK_ASD )
         {
            pLastDCB = pDCB;           // remember current DCB
            pDCB = pDCB->pNextDCB;     // go to next DCB
            UtlAlloc( (PVOID *)&pLastDCB, 0L, 0L, NOMSG );
         } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* Set return code to close return code                         */
      /****************************************************************/
      usNlpRC = usCloseRC;
   }
   else
   {
      usNlpRC = LX_BAD_USR_AREA;
   } /* endif */

   return( usNlpRC );                  // return Nlp RC to caller
} /* end of function AsdCloseOrganize */


                                                              /* 186@KIT1082A */
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdRCHandling                                            |
//+----------------------------------------------------------------------------+
//|Function call:     AsdRCHandling( USHORT usAsdRC, HDCB hDCB, PSZ pszDict,   |
//|                                  PSZ pszServer, PSZ pszTerm );             |
//+----------------------------------------------------------------------------+
//|Description:       Handles errors which occured during Asd calls. The       |
//|                   function calls UtlError with the correct parameters      |
//|                   depending on the error condition.                        |
//|                   Either hDCB or pszDict and pszServer must be set.        |
//+----------------------------------------------------------------------------+
//|Input parameter:   USHORT    usAsdRC        return code of Asd function     |
//|                   HDCB      hDCB           dictionary used during function |
//|                                            or NULL if n/a                  |
//|                   PSZ       pszDict        Name of dictionary or NULL if   |
//|                                            hDCB is supplied                |
//|                   PSZ       pszServer      Server name of NULL if hDCB is  |
//|                                            supplied                        |
//|                   PSZ       pszTerm        current term or NULL if n/a     |
//|                   HWND      hwnd           caller's window handle or NULL  |
//|                                            if n/a                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of UtlError                                  |
//+----------------------------------------------------------------------------+
USHORT AsdRCHandling
(
  USHORT    usAsdRC,                   // return code of Asd function
  HDCB      hDCB,                      // dictionary used during function
                                       // or NULL if n/a
  PSZ       pszDict,                   // Name of dictionary or NULL if
                                       //   hDCB is supplied
  PSZ       pszServer,                 // Server name of NULL if hDCB is
                                       //   supplied
  PSZ_W     pszTerm,                   // current term or NULL if n/a
  HWND      hwnd                       // caller's window handle or NULL if n/a
)
{
  CHAR      szParm[MAX_EQF_PATH];      // buffer for parameters (dictionary name..)
  USHORT    usParms;                   // number of parameters
  ULONG     ulReturn;                  // return value from UtlError
  PSZ       pszParm;                   // pointer to parameter
  PPROPDICTIONARY pDictProp;           // ptr to dictionary properties
  BOOL      fUnicode = FALSE;          // true if parameters are unicode!
  PSZ_W     pszUniParm;

  /********************************************************************/
  /* Set defaults                                                     */
  /********************************************************************/
  pszParm = NULL;
  usParms = 0;
  pszUniParm = NULL;

  /********************************************************************/
  /* Get dictionary name                                              */
  /********************************************************************/
  if ( pszDict )
  {
    strncpy( szParm, pszDict, sizeof(szParm)-1 );
    szParm[sizeof(szParm)-1] = EOS;
  }
  else if ( hDCB != NULL )
  {
    AsdQueryDictName( hDCB, szParm );
  } /* endif */

  if ( usAsdRC > 0x8000 )              // it's an NLP rc
  {
    /******************************************************************/
    /* All Nlp calls require the dictionary name as paramaeter        */
    /******************************************************************/
    if ( pszDict || hDCB )
    {
      pszParm = szParm;
      usParms = 1;
    } /* endif */
  }
  else
  {
     switch ( usAsdRC )
     {
        case LX_INSUF_STOR_ASD:
        case LX_MEM_ALLOC_ASD:
           /***********************************************************/
           /* No parameter required                                   */
           /***********************************************************/
           break;

        case LX_WRD_NT_FND_ASD:
        case LX_WRD_EXISTS_ASD:
        case LX_OTHER_USER_ASD:                                 /* 1@KIT1285M */
           /***********************************************************/
           /* Use term as parameter                                   */
           /***********************************************************/
           if ( pszTerm )
           {
             pszUniParm = pszTerm;
             usParms = 1;
             fUnicode = TRUE;
           } /* endif */
           break;

        case TMERR_SERVER_NOT_STARTED:
        case TMERR_SERVERCODE_NOT_STARTED:
        case TMERR_COMMUNICATION_FAILURE:
        case TMERR_SERVER_ABOUT_TO_EXIT:
        case  TMERR_TOO_MANY_OPEN_DATABASES:
        case  TMERR_TOO_MANY_USERS_CONNECTED:
        case LANUID_NO_LAN:
        case LANUID_REQ_NOT_STARTED:
        case LANUID_USER_NOT_LOG_ON:
          /************************************************************/
          /* Server name is required as parameter                     */
          /************************************************************/
          if ( pszServer )
          {
            pszParm = pszServer;
            usParms = 1;
          }
          else if ( hDCB )
          {
            if ( AsdRetPropPtr( NULL, hDCB, &pDictProp ) == LX_RC_OK_ASD )
            {
              pszParm = pDictProp->szServer;
              usParms = 1;
            }
          }
          else if ( pszDict )
          {
            /**********************************************************/
            /* Try to get server name from properties                 */
            /**********************************************************/
            UtlMakeEQFPath( szParm, NULC, PROPERTY_PATH, NULL );
            strcat( szParm, BACKSLASH_STR );
            strcat( szParm, pszDict );
            strcat( szParm, EXT_OF_DICTPROP );
            pDictProp = NULL;
            if ( UtlLoadFileL( szParm, (PVOID *)&pDictProp, &ulReturn, FALSE, FALSE ) )
            {
              strcpy( szParm, pDictProp->szServer );
              pszParm = szParm;
              usParms = 1;
              UtlAlloc( (PVOID *)&pDictProp, 0L, 0L, NOMSG );
            }
          } /* endif */
          break;


        case LX_DICT_WRT_ASD:
        case LX_PROTECTED_ASD:
        case LX_MAX_OPEN_ASD:
        case LX_OPEN_FLD_ASD:
        case LX_INCOMP_SIG_ASD:
        case LX_RENUM_RQD_ASD:
        case  BTREE_FILE_NOTFOUND:
        case  BTREE_INVALID_DRIVE:
        case  BTREE_OPEN_FAILED:
        case  BTREE_NETWORK_ACCESS_DENIED:
        case  BTREE_ACCESS_ERROR:
        case LX_BAD_LANG_CODE:
        case LX_IDX_NT_OPEN_ASD:
        case LX_ASC_NT_ALLWD_ASD:
        case LX_DATA_2_LRG_ASD:
        case LX_UNEXPECTED_ASD:
        case LX_UNINIT_PRM_ASD:
        case TMERR_TOO_MANY_QUERIES:
        case TMERR_PROP_WRITE_ERROR:
        case TMERR_PROP_NOT_FOUND:
        case TMERR_PROP_READ_ERROR:
        case TMERR_PROP_EXIST:
        case LX_FLE_OPEN_ASD:
        default:
          /************************************************************/
          /* Dictionary name is required as parameter                 */
          /************************************************************/
          if ( pszDict || hDCB )
          {
            pszParm = szParm;
            usParms = 1;
          }
          else
          {
            pszParm = NULL;
            usParms = 0;
          } /* endif */
          break;

     } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* Call error handler                                               */
  /********************************************************************/
  if ( hwnd )
  {
    ulReturn = UtlErrorHwndW( usAsdRC, MB_CANCEL, usParms,
                              ( fUnicode) ? &pszUniParm : (PSZ_W *)&pszParm,
                              QDAM_ERROR, hwnd, fUnicode);
  }
  else
  {
    ulReturn = UtlErrorW( usAsdRC, MB_CANCEL, usParms,
                         ( fUnicode ) ? &pszUniParm : (PSZ_W *)&pszParm,
                         QDAM_ERROR, fUnicode );
  } /* endif */

  return( (USHORT)ulReturn );
} /* end of function AsdRCHandling */

// the following are old Asd functions which are still needed to make organize
// of an non-Unicode dict into an Unicode dict!!
//+----------------------------------------------------------------------------+
//| AsdNxtTerm        - Asd interface for NlpNxtTermAsd                        |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Converts Asd parameters to Nlp parameters and calls NlpNxtTermAsd.      |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    HUCB     hUCB                IN     user control block handle           |
//|    PUCHAR   pucTerm             OUT    matching term found                 |
//|    PULONG   pulTermNumber       OUT    term number                         |
//|    PULONG   pulDataLength       OUT    entry data length                   |
//|    PHDCB    phDCB               OUT    dictionary of match                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_...              =  Nlp errors                  |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+
USHORT AsdNxtTerm
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PUCHAR   pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;            // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      NlpNxtTermAsd( pDCB->usDictHandle,       // dictionary handle
                     pUCB->usUser,             // user handle
                     pucTerm,                  // term found:
                     pulTermNumber,            // term number
                     pulDataLength,            // entry data length
                     &usDictHandle,            // dictionary of match
                     &usNlpRC );               // return code
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdNxtTerm */

USHORT AsdRetEntry
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PUCHAR   pucTerm,                   // term of this entry
   PULONG   pulTermNumber,             // term number
   PUCHAR   pucEntryData,              // data of term entry
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of term
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   DICTHANDLE usDictHandle = 0;        // dictionary of term

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */

   //
   // call Nlp function
   //
   if ( ASDOK(usNlpRC) )
   {
      NlpRetEntryAsd( pDCB->usDictHandle,        // dictionary handle
                      pUCB->usUser,              // user handle
                      pucTerm,                   // term for this entry
                      pulTermNumber,             // term number
                      pucEntryData,              // data for term entry
                      pulDataLength,             // entry data length
                      &usDictHandle,             // dictionary of term
                      &usNlpRC );                // return code
   } /* endif */

   //
   // adapt dictionary handle
   //
   if ( ASDOK(usNlpRC) )
   {
      *phDCB = AsdHandleToDCB( pDCB, usDictHandle );
   } /* endif */

   return( usNlpRC );
} /* end of AsdRetEntry */

USHORT AsdDictVersion
(
    HDCB hDCB,
    PUSHORT pusVersion
)
{
  PDCB      pDCB;
  USHORT    usNlpRC = LX_RC_OK_ASD;

  pDCB = (PDCB) hDCB;
  *pusVersion = 0;

  CHECKDCB( pDCB, usNlpRC );
  if (ASDOK(usNlpRC) )
  {
//  *pusVersion = (DamRec[pDCB->usDictHandle].pDamBTree)->pBTree->usVersion;
    *pusVersion = (DamGetBTreeFromDamRec(pDCB->usDictHandle))->pBTree->usVersion;
  }
  return(usNlpRC);
} /* end of AsdDictVersion */


// This will set all necessary items to indicate we are dealing with an old non-unicode
// dictionary
USHORT AsdDictSetOldVersion
(
    HDCB hDCB
)
{
  PDCB      pDCB;
  USHORT    usNlpRC = LX_RC_OK_ASD;
  pDCB = (PDCB) hDCB;

  CHECKDCB( pDCB, usNlpRC );
  if (ASDOK(usNlpRC) )
  {
//  PBTREEGLOB pBT = (DamRec[pDCB->usDictHandle].pDamBTree)->pBTree;
    PBTREEGLOB pBT = (DamGetBTreeFromDamRec(pDCB->usDictHandle))->pBTree;

    pBT->usVersion = BTREE_VERSION;
    strcpy(pBT->chEQF,BTREE_HEADER_VALUE_V0);
    pBT->fTerse = FALSE;
    pBT->compare =  QDAMKeyCompareNonUnicode;

//  pBT = (DamRec[pDCB->usIndexHandle].pDamBTree)->pBTree;
    pBT = (DamGetBTreeFromDamRec(pDCB->usIndexHandle))->pBTree;
    pBT->usVersion = BTREE_VERSION;
    strcpy(pBT->chEQF,BTREE_HEADER_VALUE_V0);
    pBT->fTerse = FALSE;
    pBT->compare =  QDAMKeyCompareNonUnicode;
    pBT->fWriteHeaderPending = TRUE;


  }
  return(usNlpRC);
} /* end of AsdDictSetOldVersion */


// Synchronize memory and file ...
USHORT AsdResynch
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB                       // dictionary control block handle
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // pointer to user control block
   PDCB      pDCB;                     // pointer to currently active DCB

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );          // check user control block pointer

   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC );       // check dictionary control block pointer
   } /* endif */


   if ( usNlpRC == LX_RC_OK_ASD )
   {
     if ( pDCB->usDictHandle )
     {
        NlpResynchAsd( pDCB->usDictHandle, pUCB->usUser, &usNlpRC );
     }
     if ( pDCB->usIndexHandle && (usNlpRC == LX_RC_OK_ASD))
     {
        NlpResynchAsd( pDCB->usIndexHandle, pUCB->usUser, &usNlpRC );
     }
   }
   else
   {
      usNlpRC = LX_BAD_USR_AREA;
   } /* endif */

   return( usNlpRC );                  // return Nlp RC to caller

} /* endof AsdResynch */

ULONG GetCPFromDCB
(
  HDCB  hDCB
)
{
  PDCB  pDCB;
  ULONG  ulOemCP = 0L;

   pDCB = (PDCB) hDCB;
   if ( pDCB)
   {
     ulOemCP = pDCB->ulOemCP;             // fits to pDCB->sLangID which == sLanguageID
                                          // (HOPEFULLY!)

   }
   return (ulOemCP);
}

// convert old type index records if necessary and get record size
USHORT AsdPrepareIndexRecord
( 
  PBYTE       *ppucData,               // ptr to index data buffer pointer
  PULONG      pulSize,                 // ptr to size of index data buffer
  PULONG      pulUsed                  // ptr to # of bytes used in index data buffer
)
{
  PASDINDEXENTRYHEADER pHeader = (PASDINDEXENTRYHEADER)*ppucData;
  ULONG ulOldHeaderSize = sizeof(USHORT) * (LASTTERMTYPE + 1);
  USHORT      usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
  BOOL        fOK = TRUE;               // internal OK flag

  if ( pHeader->usTermOffs[0] == ulOldHeaderSize )
  {
    // data offset is right after USHORT array --> this is an old index record
    ULONG ulNewSize = 0;

    // get # of used bytes in record
    *pulUsed = pHeader->usTermOffs[LASTTERMTYPE];

    // enlarge index record buffer if necessary
    ulNewSize = *pulUsed + sizeof(ASDINDEXENTRYHEADER) - ulOldHeaderSize;
    if ( ulNewSize > *pulSize )
    {
      fOK = UtlAlloc( (PVOID *)ppucData, *pulSize, ulNewSize, NOMSG );
      if ( fOK  )
      {
        *pulSize = ulNewSize;
        pHeader = (PASDINDEXENTRYHEADER)*ppucData;
      }
      else
      {
        usNlpRC = LX_MEM_ALLOC_ASD;
      } /* endif */
    } /* endif */

    // make room for new header area and setup header data
    if ( fOK )
    {
      int i;
      ULONG ulDifference = sizeof(ASDINDEXENTRYHEADER) - ulOldHeaderSize;
   
      memmove( pHeader + 1, *ppucData + ulOldHeaderSize, *pulUsed - ulOldHeaderSize );
      *pulUsed = *pulUsed + ulDifference;

      // checksum mismatch, record may have been processed by older version of Tmgr
      for( i = 0; i <= LASTTERMTYPE; i++ )
      {
        pHeader->usTermOffs[i] = (USHORT)(pHeader->usTermOffs[i] + ulDifference);
        pHeader->ulTermOffs[i] = pHeader->usTermOffs[i];
      } /* endfor */
      pHeader->lCheckSum = AsdBuildHeaderCheckSum(pHeader);
    } /* endif */
  } /* endif */

  // adjust ULONG offset if necessary
  if ( fOK  && (pHeader->lCheckSum != AsdBuildHeaderCheckSum(pHeader) ) )
  {
    int i;

    // checksum mismatch, record may have been processed by older version of Tmgr
    for( i = 0; i <= LASTTERMTYPE; i++ )
    {
      pHeader->ulTermOffs[i] = pHeader->usTermOffs[i];
    } /* endfor */
    pHeader->lCheckSum = AsdBuildHeaderCheckSum(pHeader);
  } /* endif */

  // set # of used bytes in record
  if ( fOK )
  {
    *pulUsed = pHeader->ulTermOffs[LASTTERMTYPE];
  } /* endif */

  return( usNlpRC );

} /* end of function AsdPrepareIndexRecord */ 

// compute checksum of USHORT offset array
LONG AsdBuildHeaderCheckSum
(
  PASDINDEXENTRYHEADER pHeader
)
{
  LONG lCheckSum = 0;
  int i;

  // checksum mismatch, record may have been processed by older version of Tmgr
  for( i = 0; i < LASTTERMTYPE; i++ )
  {
    lCheckSum += pHeader->usTermOffs[i];
  } /* endfor */
  return( lCheckSum );
} /* end of function AsdBuildHeaderCheckSum */ 

// make index entry compatible to older versions of Tmgr
USHORT AsdMakeCompatibleRecord
( 
  PBYTE       pucData,                 // ptr to index data buffer
  ULONG       ulUsed                   // # of bytes used in index data buffer
)
{
  USHORT      usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
  PASDINDEXENTRYHEADER pHeader = (PASDINDEXENTRYHEADER)pucData;

  if ( ulUsed < MAXDATASIZE )
  {
    // set USHORT offsets to values in ULONG ofset array
    int i;
    for( i = 0; i <= LASTTERMTYPE; i++ )
    {
      pHeader->usTermOffs[i] = (USHORT)pHeader->ulTermOffs[i];
    } /* endfor */
  }
  else
  {
    // simulate an empty record as record data is too large for older versions
    int i;
    for( i = 0; i <= LASTTERMTYPE; i++ )
    {
      pHeader->usTermOffs[i] = sizeof(ASDINDEXENTRYHEADER);
    } /* endfor */
  } /* endif */
  pHeader->lCheckSum = AsdBuildHeaderCheckSum(pHeader);

  return( usNlpRC );
} /* end of function AsdMakeCompatibleRecord */ 
