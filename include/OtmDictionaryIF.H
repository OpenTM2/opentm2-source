/*! OtmDictionaryIF.H  - Public header file for Dictionary Interface
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

#if !defined(_OtmDictionaryIF_H_)

#define _OtmDictionaryIF_H_

//-----------------------------------------------------------------------------
//  AsdBegin            Begin/Initialize ASD services (NlpBegAsd+NlpBegService)
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdBegin
(
   USHORT usMaxDicts,                  // maximum number of dictionaries
   PHUCB  phUCB                        // ptr to user control block handle
);

//-----------------------------------------------------------------------------
//  AsdEnd              Terminate ASD services (NlpEndAsd)
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdEnd
(
   HUCB  hUCB                          // user control block handle
);

//-----------------------------------------------------------------------------
//  AsdOpen             Open a dictionary and its associated dictionaries and
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdOpen
(
   HUCB     hUCB,                      // user control block handle
   USHORT   usOpenFlags,               // open flags, see defines below
   USHORT   usNumDicts,                // number of dictioanries in ppszDicts
   PSZ      *ppszDicts,                // ptr array with dictionary properties
   PHDCB    phDCB,                     // dictionary control block handle
   PUSHORT  pusErrDict                 // number of failing dictionary
                                       // usNumDicts + 1 = association failed
);

//-----------------------------------------------------------------------------
//  AsdClose            Close dictionaries opened by AsdOpen (NlpCloseAsd)
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdClose
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB                       // dictionary control block handle
);


//-----------------------------------------------------------------------------
//  AsdInsEntry         Insert a new entry into a dictionary (NlpInsEntryAsd)
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdInsEntry
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm,                   // term to be inserted
   PSZ_W    pucData,                   // data for term
   ULONG    ulLength,                  // length of term data
   PULONG   pulTermNo                  // number of inserted term
);

__declspec(dllexport) USHORT AsdInsEntryNonUnicode
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm,                   // term to be inserted
   PUCHAR   pucData,                   // data for term
   ULONG    ulLength,                  // length of term data
   PULONG   pulTermNo                  // number of inserted term
);


//-----------------------------------------------------------------------------
//  AsdRepEntry         Replace a dictionary entry (NlpUpdEntryAsd)
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRepEntry
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm,                   // term to be deleted
   PSZ_W    pucData,                   // data for term
   ULONG    ulLength,                  // length of term data
   PULONG   pulTermNo                  // number of inserted term
);


//-----------------------------------------------------------------------------
//  AsdDelEntry         Delete a dictionary entry (NlpDelEntryAsd)
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdDelEntry
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm                    // term to be deleted
);


//-----------------------------------------------------------------------------
//  AsdBuild            Create a new dictionary
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdBuild
(
   HUCB     hUCB,                      // user control block handle
   BOOL     fGuarded,                  // TRUE = open in guarded mode
   PHDCB    phDCB,                     // dictionary control block handle
   PSZ      pszDictName                // name of dictionary property file
);

//-----------------------------------------------------------------------------
//  AsdTranslate        Lookup all terms for a segment and return the terms
//                      or the translation depending on your request or
//                      all translations and other dictionary fields marked
//                      for automatic lookup.
//                      the mode flag controls the way AsdTranslate works:
//
//                        ASDT_TERMS_ONLY   only terms are added to the term
//                                          list
//                        ASDT_TRANSLATIONS terms and translations are added
//                                          the list
//                        ASDT_MARKED_DATA  terms, translations and marked
//                                          dictioanry fields are added to
//                                          the list. Data of dictionary fields
//                                          other than the translation is
//                                          prefixed by ASDT_DICT_DATA_CHAR
//                                          which is 0x01.
//                        ASDT_SINGLECOMP_LOOKUP lookup componenets of compound
//                                          as well
//                        (these flags are mutual exclusive!)
//                        ASDT_SINGLECOMP_LOOKUP lookup componenets of compound
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdTranslate
(
   HUCB     hUCB,                      //     user control block handle
   HDCB     hDCB,                      //     dictionary control block handle
   PUCHAR   pucSegment,                //     ptr to input segment (segment
                                       //     must be terminated by a null
                                       //     character)
   USHORT   usOutBufSize,              //     size of outbuf
   PUCHAR   pucOutBuf,                 //     buffer for terms and translations
   USHORT   usMode                     //     mode for AsdTranslate
);

__declspec(dllexport) USHORT AsdTranslateW
(
   HUCB     hUCB,                      //     user control block handle
   HDCB     hDCB,                      //     dictionary control block handle
   PSZ_W    pucSegment,                //     ptr to input segment (segment
                                       //     must be terminated by a null
                                       //     character)
   USHORT   usOutBufSize,              //     size of outbuf
   PSZ_W    pucOutBuf,                 //     buffer for terms and translations
   USHORT   usMode                     //     mode for AsdTranslate
);

//-----------------------------------------------------------------------------
//  AsdFndBegin         Asd interface for NlpFndBeginAsd call
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdFndBegin
(
   PSZ_W    pucSubString,              // desired substring
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
);

//-----------------------------------------------------------------------------
//   AsdFndEquiv      - Asd interface for NlpFndEquivAsd
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdFndEquiv
(
   PUCHAR   pucTermIn,                 // desired term
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PUCHAR   pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
);

__declspec(dllexport) USHORT AsdFndEquivW
(
   PSZ_W    pucTermIn,                 // desired term
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
);

//-----------------------------------------------------------------------------
//  AsdFndMatch       - Asd interface for NlpFndMatchAsd
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdFndMatch
(
   PSZ_W    pucTermIn,                 // desired term
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
);
//-----------------------------------------------------------------------------
//   AsdFndNumber     - Asd interface for NlpFndNumberAsd
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdFndNumber
(
   ULONG    ulTermNumber,              // desired term number
   HDCB     hDCB,                      // dictionary control block handle
   USHORT   usRelocation,              // relocation flag
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
);

//-----------------------------------------------------------------------------
//  AsdNxtTerm        - Asd interface for NlpNxtTermAsd
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdNxtTermW
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
);

__declspec(dllexport) USHORT AsdNxtTerm
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PUCHAR   pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
);

//-----------------------------------------------------------------------------
//   AsdPrvTerm       - Asd interface for NlpPrvTermAsd
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdPrvTerm
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // matching term found
   PULONG   pulTermNumber,             // term number
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of match
);

//-----------------------------------------------------------------------------
//   AsdRenumber      - Asd interface for NlpRenumberAsd
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRenumber
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB                       // user control block handle
);

//-----------------------------------------------------------------------------
//   AsdRetEntry      - Asd interface for NlpRetEntryAsd
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRetEntry
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PUCHAR   pucTerm,                   // term of this entry
   PULONG   pulTermNumber,             // term number
   PUCHAR   pucEntryData,              // data of term entry
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of term
);

__declspec(dllexport) USHORT AsdRetEntryW
(
   HDCB     hDCB,                      // dictionary control block handle
   HUCB     hUCB,                      // user control block handle
   PSZ_W    pucTerm,                   // term of this entry
   PULONG   pulTermNumber,             // term number
   PSZ_W    pucEntryData,              // data of term entry
   PULONG   pulDataLength,             // entry data length
   PHDCB    phDCB                      // dictionary of term
);

//-----------------------------------------------------------------------------
//   AsdListIndex     - Get a list of terms from the index
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdListIndex
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
);

//-----------------------------------------------------------------------------
//  AsdDelete         - Delete a dictionary (dictionary must be closed!)
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdDelete
(
   PSZ      pszPropName                // name of dictionary property file
);

//-----------------------------------------------------------------------------
//  AsdRename         - Rename a dictionary (dictionary must be closed!)
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRename
(
   PSZ      pszOldPropName,            // name of old dictionary property file
   PSZ      pszNewPropName             // name of new dictionary property file
);

//-----------------------------------------------------------------------------
//   AsdRetPropPtr    - Get pointer to properties for an open dictionary
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRetPropPtr
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PPROPDICTIONARY *ppProp             // ptr to dictioanry properties
);

//-----------------------------------------------------------------------------
//  AsdGetStemForm         - get the stem form of a term
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdGetStemForm
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszInTerm,                 // input term
   PSZ_W    pszOutTerm                 // output term (reduced to stem form)
);

//-----------------------------------------------------------------------------
//   AsdEntryChange - Correct internal lookup tables after entry has changed.
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdEntryChange
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm                    // term which has been changed or added
);

//-----------------------------------------------------------------------------
//   AsdIsUcbOK     - Check if an UCB is OK
//----------------------------------------------------------------------------
__declspec(dllexport) BOOL  AsdIsUcbOK
(
   HUCB     hUCB                       // user control block handle
);

//-----------------------------------------------------------------------------
//   AsdIsDcbOK     - Check if an DCB is OK
//----------------------------------------------------------------------------
__declspec(dllexport) BOOL  AsdIsDcbOK
(
   HDCB     hDCB                       // dictionary control block handle
);

//-----------------------------------------------------------------------------
//   AsdQueryDictName    - Return (long) name of dictionary
//----------------------------------------------------------------------------
__declspec(dllexport) BOOL  AsdQueryDictName
(
   HDCB     hDCB,                      // dictionary control block handle
   PSZ      pszDictName                // ptr to buffer for dictionary name
);

//-----------------------------------------------------------------------------
//   AsdQueryDictName    - Return the short name of a dictionary (w/o path
//                         and extension
//----------------------------------------------------------------------------
__declspec(dllexport) BOOL  AsdQueryDictShortName
(
   HDCB     hDCB,                      // dictionary control block handle
   PSZ      pszDictName                // ptr to buffer for dictionary name
);

//-----------------------------------------------------------------------------
//   AsdRetDictList    - Return list of associated dictionaries
//-----------------------------------------------------------------------------+
//  Description:
//     Return a list of the handles of all dictionaries associated to the
//     given dictionary handle.
//-----------------------------------------------------------------------------+
//  Flow:
//-----------------------------------------------------------------------------+
//  Arguments:
//     HDCB     hDCB                IN     dictionary control block handle
//     HDCB     *ahDCB              OUT    ptr to array for dictionary handles
//                                         (size should be MAX_DICTS)
//     PUSHORT  pusNumOfDicts       OUT    number of dictionaries associated
//-----------------------------------------------------------------------------+
//  Returns:
//     USHORT   usRC        function return code:
//                          LX_RC_OK_ASD        =  OK
//                          LX_...              =  Nlp errors
//-----------------------------------------------------------------------------+
//  Prereqs:
//     none
//-----------------------------------------------------------------------------+
//  SideEffects:
//     none
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRetDictList
(
   HDCB     hDCB,                      // dictionary control block handle
   HDCB     *ahDCB,                    // ptr to array for dictionary handles
   PUSHORT  pusNumOfDicts              // ptr to variable receiving number of
                                       // associated dictionaries
);

//-----------------------------------------------------------------------------
//   AsdRetUserHandle  - Return the user handle supplied by NlpBegin
//-----------------------------------------------------------------------------+
//  Description:
//     Return the ASD functions user handle which is has been created using
//     NlpBegin.
//-----------------------------------------------------------------------------+
//  Flow:
//-----------------------------------------------------------------------------+
//  Arguments:
//     HUCB     hUCB                IN     user control block handle
//     PUSHORT  pusUserHandle       OUT    Nlp user handle
//-----------------------------------------------------------------------------+
//  Returns:
//     USHORT   usRC        function return code:
//                          LX_RC_OK_ASD        =  OK
//                          LX_...              =  Nlp errors
//-----------------------------------------------------------------------------+
//  Prereqs:
//     hUCB must have been created using AsdBegin.
//-----------------------------------------------------------------------------+
//  SideEffects:
//     none
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRetUserHandle
(
   HUCB     hUCB,                      // user control block handle
   PUSHORT  pusUserHandle              // Nlp user handle
);

//-----------------------------------------------------------------------------
//   AsdRetServiceHandle  - Return the service handle supplied by NlpBegService
//-----------------------------------------------------------------------------+
//  Description:
//     Return the ASD functions user handle which is has been created using
//     NlpBegService.
//-----------------------------------------------------------------------------+
//  Flow:
//-----------------------------------------------------------------------------+
//  Arguments:
//     HUCB     hUCB                IN     user control block handle
//     PUSHORT  pusServiceHandle    OUT    Nlp service handle
//-----------------------------------------------------------------------------+
//  Returns:
//     USHORT   usRC        function return code:
//                          LX_RC_OK_ASD        =  OK
//                          LX_...              =  Nlp errors
//-----------------------------------------------------------------------------+
//  Prereqs:
//     hUCB must have been created using AsdBegin.
//-----------------------------------------------------------------------------+
//  SideEffects:
//     none
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRetServiceHandle
(
   HUCB     hUCB,                      // user control block handle
   PUSHORT  pusServiceHandle           // Nlp service handle
);

//-----------------------------------------------------------------------------
//   AsdRetMorphHandle  - Return the handle of a morphology dictionary
//-----------------------------------------------------------------------------+
//  Description:
//     Return the handle of the morphology dictionary which has been opened
//     implicitely by AsdOpen.
//-----------------------------------------------------------------------------+
//  Flow:
//-----------------------------------------------------------------------------+
//  Arguments:
//     HDCB     hDCB                IN     dictionary control block handle
//     PUSHORT  pusMorphHandle      OUT    NlpActDict dictionary handle
//-----------------------------------------------------------------------------+
//  Returns:
//     USHORT   usRC        function return code:
//                          LX_RC_OK_ASD        =  OK
//                          LX_...              =  Nlp errors
//-----------------------------------------------------------------------------+
//  Prereqs:
//     hDCB must have been created using AsdOpen.
//-----------------------------------------------------------------------------+
//  SideEffects:
//     none
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRetMorphHandle
(
   HDCB     hDCB,                      // dictionary control block handle
   PUSHORT  pusMorphHandle             // NlpActDict dictionary handle
);

//-----------------------------------------------------------------------------
//   AsdMergeEntry      - Merge a dictionary entry into another dictionary
//-----------------------------------------------------------------------------+
//  Description:
//     Merge a dictionary entry into another dictionary. If the dictionary
//     entry exists in the target dictionary, the new entry is merged
//     into the existing entry, the new entry is ignored or the existing entry
//     is replaced with the new one depending on the control flag setting and
//     the user selection.
//-----------------------------------------------------------------------------+
//  Flow:
//-----------------------------------------------------------------------------+
//  Arguments:
//     HUCB     hUCB                IN     handle of user control block
//     HDCB     hDCBSource          IN     handle of source dictionary
//     PUCHAR   pucTerm             IN     headword of new term
//     ULONG    ulDataLength        IN     length of new term data
//     PUCHAR   pucData             IN     pointer to new term data
//     HDCB     hDCBTarget          IN     handle of target dictionary
//     PUSHORT  pusControlFlags     IN/OUT a combination of control flags:
//
//        type of merge flags, these flags are mutually exclusive
//        MERGE_REPLACE       replace existing entries
//        MERGE_NOREPLACE     do not replace existing entries
//        MERGE_ADD           add data of new entry to data of existing entry
//
//        if the following flag is specified one of the merge type flags
//        has to be specified also
//        MERGE_NOUSERPROMPT  do not ask user for type of merge
//        MERGE_USERPROMPT    ask user for type of merge
//
//        the default is MERGE_ADD | MERGE_NOUSERPROMPT
//
//        the control flags are changed on return to reflect the user's
//        type of merge selection. Also the flag MERGE_NOUSERPROMPT may be
//        set, if the user has choosen to merge without being prompted anymore.
//-----------------------------------------------------------------------------+
//  Returns:
//     USHORT   usRC        function return code:
//                          LX_RC_OK_ASD        =  OK
//                          LX_...              =  Nlp errors
//-----------------------------------------------------------------------------+
//  Prereqs:
//     HUCB mus have been created using AsdBegin.
//     hDCBSource must have been created using AsdOpen.
//     hDCBTarget must have been created using AsdOpen.
//-----------------------------------------------------------------------------+
//  SideEffects:
//     The target dictionary is updated.
//----------------------------------------------------------------------------

__declspec(dllexport) USHORT AsdMergeEntry
(
   HUCB     hUCB,                      // handle of user control block
   HDCB     hDCBSource,                // handle of source dictionary
   PSZ_W    pucTerm,                   // headword of new term
   ULONG    ulDataLength,              // length of new term data
   PSZ_W    pucData,                   // pointer to new term data
   HDCB     hDCBTarget,                // handle of target dictionary
   PUSHORT  pusControlFlags            // pointer to control flags variable
);

//-----------------------------------------------------------------------------
//   AsdNumEntries    - get number of entries in a dictionary
//-----------------------------------------------------------------------------+
//  Description:
//     Call fastdam function QDAMDictNumentries to return number of entries.
//-----------------------------------------------------------------------------+
//  Flow:
//-----------------------------------------------------------------------------+
//  Arguments:
//     HDCB     hDCB                IN     dictionary control block handle
//     PULONG   pulNumber           IN     ptr to buffer for number of entries
//-----------------------------------------------------------------------------+
//  Returns:
//     USHORT   usRC        function return code:
//                          LX_RC_OK_ASD        =  OK
//                          LX_...              =  Nlp errors
//-----------------------------------------------------------------------------+
//  Prereqs:
//     hDCB must have been created using AsdOpen.
//-----------------------------------------------------------------------------+
//  SideEffects:
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdNumEntries
(
   HDCB     hDCB,                      // dictionary control block handle
   PULONG   pulNumber                  // ptr to buffer for number of entries
);

__declspec(dllexport) USHORT AsdDictVersion
(
   HDCB     hDCB,                      // dictionary control block handle
   PUSHORT  pusVersion                 // ptr to version of dictionary
);

__declspec(dllexport) USHORT AsdDictSetOldVersion
(
    HDCB hDCB                          // dictionary control block handle
);

//-----------------------------------------------------------------------------
// Function name:     AsdLockDict      Lock or unlock complete dictionary
//-----------------------------------------------------------------------------+
// Description:       Locks or unlocks complete dictionary.
//                    :p.
//                    While the dictionary is locked no other function using
//                    Asd dictionary calls may write to the dictionary.
//-----------------------------------------------------------------------------+
// Input parameter:   HDCB     hDCB   dictionary control block handle
//                    BOOL     fLock  lock flag (TRUE = lock, FALSE = unlock)
//-----------------------------------------------------------------------------+
// Returncode type:   USHORT
//-----------------------------------------------------------------------------+
// Returncodes:       LX_RC_OK_ASD        =  OK
//                    other               =  error code
//-----------------------------------------------------------------------------+
// Prerequesits:      hDCB must be a valid dictionary handle which refers to
//                    a single dictionary. The handle for associated
//                    dictionaries may not be used.
//-----------------------------------------------------------------------------+
// Samples:           _
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdLockDict
(
  HDCB     hDCB,                       // dictionary control block handle
  BOOL     fLock                       // lock flag (TRUE = lock,
                                       //            FALSE = unlock)
);

//-----------------------------------------------------------------------------
// Function name:     AsdLockEntry     Lock or unlock a single dictionary entry
//-----------------------------------------------------------------------------+
// Function call:     _
//-----------------------------------------------------------------------------+
// Description:       Locks or unlocks a single dictionary term.
//                    :p.
//                    While the dictionary term is locked no other function
//                    using Asd dictionary calls may write to the locked
//                    term.
//-----------------------------------------------------------------------------+
// Input parameter:   HDCB     hDCB    dictionary control block handle
//                    PSZ      pszTerm ptr to term being locked/unlocked
//                    BOOL     fLock   lock flag (TRUE = lock, FALSE = unlock)
//-----------------------------------------------------------------------------+
// Returncode type:   USHORT
//-----------------------------------------------------------------------------+
// Returncodes:       LX_RC_OK_ASD        =  OK
//                    other               =  error code
//-----------------------------------------------------------------------------+
// Prerequesits:      hDCB must be a valid dictionary handle which refers to
//                    a single dictionary. The handle for associated
//                    dictionaries may not be used.
//-----------------------------------------------------------------------------+
// Samples:           _
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdLockEntry
(
  HDCB     hDCB,                       // dictionary control block handle
  PSZ_W    pszTerm,                    // ptr to term being locked/unlocked
  BOOL     fLock                       // lock flag (TRUE = lock,
                                       //            FALSE = unlock)
);

//-----------------------------------------------------------------------------
//  AsdDeleteRemote   - Delete a dictionary (dictionary must be closed!)
//-----------------------------------------------------------------------------+
//  Description:
//     Physically deletes all files belonging to a dictionary: base dictionary,
//     index dictionary and dictionary property file on the server.
//     The dictionary should be closed..
//-----------------------------------------------------------------------------+
//  Flow:
//-----------------------------------------------------------------------------+
//  Arguments:
//     PSZ      pszPropName         IN     name of dictionary property file
//-----------------------------------------------------------------------------+
//  Returns:
//     USHORT   usRC        function return code:
//                          LX_RC_OK_ASD        =  OK
//                          LX_OPEN_FLD_ASD     =  error deleting dictionary
//                          LX_IDX_NT_OPEN_ASD  =  error deleting index
//                          LX_UNEXPECTED_ASD   =  parameters invalid
//                                                 or property file not found
//-----------------------------------------------------------------------------+
//  Prereqs:
//     None.
//-----------------------------------------------------------------------------+
//  SideEffects:
//     None.
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdDeleteRemote
(
   PSZ      pszPropName                // name of dictionary property file
);

//-----------------------------------------------------------------------------
//  AsdGetHandle        Get dictionary user handle from HDCB
//-----------------------------------------------------------------------------+
//  Description:
//-----------------------------------------------------------------------------+
//  Flow:
//-----------------------------------------------------------------------------+
//  Arguments:
//-----------------------------------------------------------------------------+
//  Returns:
//-----------------------------------------------------------------------------+
//  Prereqs:
//     hDCB must have been created using AsdOpen or AsdBuild.
//-----------------------------------------------------------------------------+
//  SideEffects:
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdHandleFromDCB
(
   HDCB     hDCB,                      //     dictionary control block handle
   PUSHORT  pusDictHandle,
   PUSHORT  pusIndexHandle,
   PVOID    * ppBTreeDict,
   PVOID    * ppBTreeIndex,
   PLONG    pLHandle
);



__declspec(dllexport) USHORT AsdCloseOrganize
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ      pszDictPath,               // full asd dict path of original
   CHAR     chPrimDrive,               // remote primary drive
   USHORT   usRc                       // return code from temp file close
);

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------+
// Function name:     AsdRCHandling
//-----------------------------------------------------------------------------+
// Function call:     AsdRCHandling( USHORT usAsdRC, HDCB hDCB, PSZ pszDict,
//                                   PSZ pszServer, PSZ pszTerm );
//-----------------------------------------------------------------------------+
// Description:       Handles errors which occured during Asd calls. The
//                    function calls UtlError with the correct parameters
//                    depending on the error condition.
//                    Either hDCB or pszDict and pszServer must be set.
//-----------------------------------------------------------------------------+
// Input parameter:   USHORT    usAsdRC        return code of Asd function
//                    HDCB      hDCB           dictionary used during function
//                                             or NULL if n/a
//                    PSZ       pszDict        Name of dictionary or NULL if
//                                             hDCB is supplied
//                    PSZ       pszServer      Server name of NULL if hDCB is
//                                             supplied
//                    PSZ       pszTerm        current term or NULL if n/a
//                    HWND      hwnd           caller's window handle or NULL
//                                             if n/a
//-----------------------------------------------------------------------------+
// Returncode type:   USHORT
//-----------------------------------------------------------------------------+
// Returncodes:       return code of UtlError
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdRCHandling
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
);

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------+
// Function name:     AsdTermList
//-----------------------------------------------------------------------------+
// Function call:     _
//-----------------------------------------------------------------------------+
// Description:       will return a list with the number of requested terms
//                    (if available) for single and associated dictionary
//                    handles.
//                    The returned list is an ASCII-Z list.
//                    The list won't be sorted, because this will be done
//                    automatically by PM.
//-----------------------------------------------------------------------------+
// Parameters:
//     HDCB     hDCB            IN          dictionary control block handle
//     HUCB     hUCB            IN          user control block handle
//     PSZ      pszTerm         IN          term to start with
//     USHORT  usNumTerms       IN          number of terms to be looked up
//     USHORT  usAction         IN          required action
//     PUCHAR  pucBuffer        IN/OUT      pointer to buffer
//     USHORT  usLen            IN          length of provided buffer
//-----------------------------------------------------------------------------+
// Returncode type:   USHORT
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdTermList
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszTerm,                   // term to be started with
   USHORT   usNumTerms,                // number of terms to be looked up
   USHORT   usAction,                  // requested action
   PSZ_W    pucBuffer,                 // pointer to buffer for term list
   USHORT   usLen                      // length of term list buffer
);

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------+
// Function name:     AsdWildCardList
//-----------------------------------------------------------------------------+
// Function call:     _
//-----------------------------------------------------------------------------+
// Description:       will return a list with the number of terms following the
//                    given pattern or a list of terms having the given
//                    compound (type of search is controlled by fCompound flag)
//                    The returned list is an ASCII-Z list.
//                    The list won't be sorted, because this will be done
//                    automatically by PM.
//-----------------------------------------------------------------------------+
// Parameters:
//     HDCB     hDCB            IN          dictionary control block handle
//     HUCB     hUCB            IN          user control block handle
//     PSZ      pszStartTerm    IN          term to be started with
//     PSZ      pszPattern      IN          search pattern or compound
//     USHORT   usNumTerms      IN          number of terms to be looked up
//     BOOL     fCompound       IN          search-for-compounds flag
//     PUCHAR   pucBuffer       IN/OUT      pointer to buffer
//     USHORT   usLen           IN          length of provided buffer
//-----------------------------------------------------------------------------+
// Returncode type:   USHORT
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdWildCardList
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszStartTerm,              // term to be started with
   PSZ_W    pszPattern,                // search pattern or compound
   USHORT   usNumTerms,                // number of terms to be looked up
   BOOL     fCompound,                 // search-for-compounds flag
   PSZ_W    pucBuffer,                 // pointer to buffer for term list
   USHORT   usLen                      // length of term list buffer
);

__declspec(dllexport) USHORT AsdGetTranslation
(
   HUCB     hUCB,                      //     user control block handle
   HDCB     hDCB,                      //     dictionary control block handle
   PUCHAR   pucDictData,               //     ptr to term data
   USHORT   usOutBufSize,              //     size of outbuf
   PUCHAR   pucOutBuf,                 //     buffer for translations
   SHORT    sListType                  //     type of list requested
);

__declspec(dllexport) USHORT AsdGetTranslationW
(
   HUCB     hUCB,                      //     user control block handle
   HDCB     hDCB,                      //     dictionary control block handle
   PSZ_W    pucDictData,               //     ptr to term data
   USHORT   usOutBufSize,              //     size of outbuf
   PSZ_W    pucOutBuf,                 //     buffer for translations
   SHORT    sListType                  //     type of list requested
);


//-----------------------------------------------------------------------------
//  AsdUpdTime          Get time of last update of a dictionary (works also
//                      for associated dictionaries)
//-----------------------------------------------------------------------------+
//  Description:
//     Returns the last update time of a dictionary.
//-----------------------------------------------------------------------------+
//  Arguments:
//     HDCB     hDCB            IN          dictionary control block handle
//     PLONG    plUpdTime       OUT         last update time of dictionary
//-----------------------------------------------------------------------------+
//  Returns:
//     USHORT   usRC        function return code
//                          0               if OK
//                          LX_xxxx         Nlp errors
//-----------------------------------------------------------------------------+
//  Prereqs:
//     hDCB must have been created using AsdOpen or AsdBuild.
//-----------------------------------------------------------------------------+
//  SideEffects:
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdUpdTime
(
   HDCB     hDCB,                      // dictionary control block handle
   PLONG    plUpdTime                  // ptr to buffer for dict update time
);

//-----------------------------------------------------------------------------
//  AsdLoadDictProperties  - load dictionary propery file
//----------------------------------------------------------------------------
__declspec(dllexport) USHORT AsdLoadDictProperties
(
   PSZ             pszPropFile,        // fully qualified property file name
   PPROPDICTIONARY pDictProp           // buffer for property data

);

__declspec(dllexport) USHORT AsdResynch
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB                       // dictionary control block handle
);

__declspec(dllexport) ULONG GetCPFromDCB
(
  HDCB     hDCB
);

__declspec(dllexport) BOOL DicRenameDict( PSZ pszOldAndNewName, BOOL fMsg );
__declspec(dllexport) VOID DicBatchImp( HWND, PVOID );
__declspec(dllexport) VOID DicBatchExp( HWND, PVOID );
__declspec(dllexport) VOID DictionaryExport( HWND, PSZ, PVOID );
__declspec(dllexport) VOID DictionaryOrganize( HWND, PSZ, PVOID );
__declspec(dllexport) VOID DictionaryProp( HWND, PSZ );
__declspec(dllexport) VOID DictionaryRemote( HWND );



#endif
