/*! EQFMORPH.H                                                                   
	Description:    Include file for the Morphological Functions Module          
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
*/

/**********************************************************************/
/* Return codes of Morph... functions                                 */
/**********************************************************************/
#define MORPH_OK                  0                  // O.K.
#define MORPH_INV_LANG_ID         ERR_MORPH_BASE + 1 // specified language ID is invalid
#define MORPH_NO_MEMORY           ERR_MORPH_BASE + 2 // memory allocation failed
#define MORPH_NO_LANG_PROPS       ERR_MORPH_BASE + 3 // no properties for requested language
                                                     // (= no entry in PROPERTY.LNG)
#define MORPH_BEGIN_SERV_FAILED   ERR_MORPH_BASE + 4 // begin of morph services failed
#define MORPH_SET_CODEPAGE_FAILED ERR_MORPH_BASE + 5 // set of morph code page failed
#define MORPH_ACT_DICT_FAILED     ERR_MORPH_BASE + 6 // activation of morph dict failed
#define MORPH_TOO_MUCH_LANGUAGES  ERR_MORPH_BASE + 7 // too much (>256) languages are active
#define MORPH_NOT_FOUND           ERR_MORPH_BASE + 8 // term not found
#define MORPH_INV_PARMS           ERR_MORPH_BASE + 9 // input parameters are invalid
#define MORPH_NOADDENDA_DICT      ERR_MORPH_BASE +10 // addenda dict failed to open/create
#define MORPH_INV_TERM            ERR_MORPH_BASE +11 // term is invalid for this function
#define MORPH_DUP_TERM            ERR_MORPH_BASE +12 // term is already in dictionary
#define MORPH_FUNC_FAILED         ERR_MORPH_BASE +13 // function failed with unexpected RC
#define MORPH_FUNC_NOT_SUPPORTED  ERR_MORPH_BASE +14 // requested function is not supported
#define MORPH_EXIT_LOAD_ERROR     ERR_MORPH_BASE +15 // language exit could not be loaded
#define MORPH_EXIT_PROCADDR_ERROR ERR_MORPH_BASE +16 // language exit entry point not found
                                                                /* 2@KITxxxxA */
#define MORPH_ASD_ERROR           ERR_MORPH_BASE +17 // An ASD error occured
#define MORPH_BUFFER_OVERFLOW     ERR_MORPH_BASE +18 // buffer overflow

//+----------------------------------------------------------------------------+
//   Defines for the POS info encoding                                          
//+----------------------------------------------------------------------------+
#define MORPH_NO_POS_INFO  0x0001      // no POS info available flag
#define MORPH_PRONOUN      0x0002      // pronoun identifier
#define MORPH_VERB         0x0004      // verb identifier
#define MORPH_NOUN         0x0010      // noun identifier
#define MORPH_ADJECTIVE    0x0020      // adjective identifier
#define MORPH_ADVERB       0x0040      // adverb identifier
#define MORPH_PREPOSITION  0x0080      // preposition identifier
#define MORPH_INTERJECTION 0x0100      // interjection identifier
#define MORPH_CONJUNCTION  0x0200      // conjunction identifier

//+----------------------------------------------------------------------------+
//   Defines for the type of requested term list                                
//                                                                              
//   MORPH_ZTERMLIST: The term list consists of zero terminated strings and     
//                    is terminated by a zero value                             
//                    Sample:  term1\0term2\0term3\0\0                          
//                                                                              
//   MORPH_OFFSLIST : The term list contains the offset and the length of the   
//                    terms. The offset values are relative to the input        
//                    segment. The list is terminated by a pair of zeroes.      
//                    Sample: a input segment of "term1 term2 term3" would      
//                            result in the following term list:                
//                            0000 0005 0006 0005 000C 0005 0000 0000           
//                                                                              
//   MORPH_FLAG_ZTERMLIST: The term list consists of zero terminated strings    
//                    which are preceeded with a ULONG flag value               
//                    and is terminated by a zero flag plus a zero value        
//                    Sample:  (ULONG)term1\0(ULONG)term2\0(ULONG)\0            
//                                                                              
//   MORPH_FLAG_OFFSLIST : The term list contains the flags, offsest and the    
//                    length of the terms.                                      
//                    The offset values are relative to the input               
//                    segment. The list is terminated by a four zero values     
//                    Sample: a input segment of "term1 term2 term3" would      
//                            result in the following term list:                
//                            00000000 0005 0006 00000000 0005 000C 00000000    
//                            0000 0000                                         
//                                                                              
//   MORPH_LARGE_ZTERMLIST The term list is a list of zero terminated strings   
//                    and is terminated by a zero value                         
//                    The first 4 bytes of the list build a pointer to the      
//                    next block of the list. If this pointer is NULL there     
//                    are no following blocks.                                  
//                    Sample:  \0\0\0\0term1\0term2\0term3\0\0                  
//                             ----?--                                         
//                                 +--> pointer to next block (here NULL)       
//                    Note: 1) you have to free ALL blocks of the list not only 
//                             the first one when destroying the list...        
//                          2) The length variable passed to and returned from  
//                             MorphAddTermToList is the length of the last     
//                             block of a MORPH_LARGE_ZTERMLIST                 
//+----------------------------------------------------------------------------+

#define MORPH_ZTERMLIST       0x0001     // zero-terminated term list
#define MORPH_OFFSLIST        0x0002     // offset/length term list
#define MORPH_FLAG_ZTERMLIST  0x0003     // zero-terminated flagged term list
#define MORPH_FLAG_OFFSLIST   0x0004     // flagged offset/length term list
#define MORPH_LARGE_ZTERMLIST 0x0005     // zero-terminated large term list

#define MORPH_LARGE_ZTERMLIST_LEN 32000L         // max block size of larger ZTERM lists

/**********************************************************************/
/* Structure to access MORPH_FLAG_OFFSLIST type term lists            */
/**********************************************************************/
typedef struct _FLAGOFFSLIST
{
  LONG             lFlags;             // flags of term
  USHORT           usLen;              // length of term
  USHORT           usOffs;             // offset of term
} FLAGOFFSLIST, *PFLAGOFFSLIST;

//+----------------------------------------------------------------------------+
//   Flags for flagged term lists                                               
//+----------------------------------------------------------------------------+
#define TF_NOLOOKUP          0x00000001L // term is no lookup term
#define TF_NUMBER            0x00000002L // term is date/number/time value
#define TF_ABBR              0x00000004L // term is an abbreviation
#define TF_ALLCAPS           0x00000008L // term consists of capital letters only
#define TF_NEWSENTENCE       0x00000010L // term is a dummy term for the
                                         // start of a new sentence
#define TF_CONTRACT          0x00000020L // term is contracted verb or negation
                                         // e.g. can't or we'll (we will)
#define TF_PREFIX            0x00000040L // term is a prefix, e.g. c' (in c'est)
#define TF_COMPLEX           0x00000080L // term is complex token, e.g. con-taining
#define TF_INITCAP           0x00000100L // first letter of term is uppercase
#define TF_DBCS              0x00000200L // token string contains DBCS characters

//+----------------------------------------------------------------------------+
//   Prototypes for entry points in language exit                               
//+----------------------------------------------------------------------------+
typedef USHORT (*PFN_LANGINIT) ( PVOID pCB );
typedef USHORT (*PFN_LANGTERMINATE) ( PVOID pCB );
typedef USHORT (*PFN_LANGGETLEMMA) ( PVOID pCB );

/**********************************************************************/
/* Function prototypes                                                */
/**********************************************************************/
__declspec(dllexport)
USHORT MorphGetLanguageID
(
  PSZ     pszLanguage,                 // name of language
  PSHORT  psLanguageID                 // address of variable receiving the
                                       //    language ID
);

__declspec(dllexport)
USHORT MorphGetLanguageIDByPlugin
(
 PSZ     pszSpellPlugin,               // name of the spell plug-in
 PSZ     pszMorphPlugin,              // name of the morph plug-in
 PSZ     pszLanguage,                 // name of language
 PSHORT  psLanguageID                 // address of variable receiving the
 //    language ID
 );

__declspec(dllexport)
USHORT MorphGetStemForm
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInTerm,                 // pointer to input term
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PSZ_W    *ppTermList,                // address of caller's term list pointer
   ULONG    ulOemCP
);


USHORT MorphGetPOSInfo
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInTerm,                 // pointer to input term
   PULONG   pulPOSInfo,                 // address of variable receiving the
                                          //    POS info
   ULONG    ulOemCP
);

__declspec(dllexport)
USHORT MorphTokenize
(
   SHORT    sLanguageID,              // language ID
   PSZ      pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType                 // type of requested term list
);

__declspec(dllexport)
USHORT MorphTokenizeW
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                 // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   ULONG    ulOemCP
);


#define ABBREV_DICT   1                // dictionary type: user abbreviation dict.
#define ADDENDA_DICT  2                // dictionary type: addenda dictionary
#define ABBREV_SYSTEM_DICT   3         // dictionary type: system abbreviation dict.

USHORT MorphListDict
(
   SHORT    sLanguageID,              // language ID
   USHORT   usDictType,                // type of dictionary
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType                 // type of requested term list
);

__declspec(dllexport)
USHORT MorphWordRecognition
(
   SHORT    sLanguageID,               // language ID
   PSZ      pszInData,                 // pointer to input segment
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PUSHORT  pusOrgBufferSize,          // address of variable containing size of
                                       //    term list buffer
   PSZ      *ppOrgTermList,            // address of caller's term list pointer
                                       //    for original terms
   PUSHORT  pusDictBufferSize,         // address of variable containing size of
                                       //    term list buffer
   PSZ      *ppDictTermList,           // address of caller's term list pointer
                                       //    for dictionary terms
   BOOL     fInclNotFoundTerms,        // TRUE = include not found terms
                                                                /* 2@KITxxxxA */
   PUSHORT  pusOrgRC,                  // ptr to buffer for original return code
   USHORT   eqfMWTMode            // Multi Word Termns
);

__declspec(dllexport)
USHORT MorphWordRecognitionW
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PULONG   pusOrgBufferSize,          // address of variable containing size of
                                       //    term list buffer
   PSZ_W    *ppOrgTermList,            // address of caller's term list pointer
                                       //    for original terms
   PULONG   pusDictBufferSize,         // address of variable containing size of
                                       //    term list buffer
   PSZ_W    *ppDictTermList,           // address of caller's term list pointer
                                       //    for dictionary terms
   BOOL     fInclNotFoundTerms,        // TRUE = include not found terms
                                                                /* 2@KITxxxxA */
   PUSHORT  pusOrgRC,                  // ptr to buffer for original return code
   USHORT   eqfMWTMode            // Multi Word Termns
);



__declspec(dllexport)
USHORT MorphTerminate( VOID );

USHORT MorphVerify
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                 // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   ULONG    ulOemCP
);

USHORT MorphSpellAid
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszTerm,                   // pointer to input term
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,                // address of caller's term list pointer
   ULONG     ulOemCP
);

USHORT MorphAddToAddenda
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszTerm,                    // pointer to term being added
   ULONG    ulOemCP
);

USHORT MorphAddToAbbrev
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszTerm,                    // pointer to term being added
   ULONG    ulOemCP
);

__declspec(dllexport)
USHORT MorphAddTermToList
(
  PSZ      *ppList,                    // ptr to term list pointer
  PUSHORT  pusSize,                    // ptr to allocated size of term list
  PUSHORT  pusUsed,                    // ptr to used bytes in term list
  PSZ      pszTerm,                    // ptr to new term being added to list
  USHORT   usLength,                   // length of term
  USHORT   usOffs,                     // offset of term
  ULONG    ulFlags,                    // flags for the term
  USHORT   usListType                  // type of list MORPH_ZTERMLIST or
);

USHORT MorphAddTermToList2
(
  PSZ      *ppList,                    // ptr to term list pointer
  PULONG   pulSize,                    // ptr to allocated size of term list
  PULONG   pulUsed,                    // ptr to used bytes in term list
  PSZ      pszTerm,                    // ptr to new term being added to list
  USHORT   usLength,                   // length of term
  USHORT   usOffs,                     // offset of term
  ULONG    ulFlags,                    // flags for the term
  USHORT   usListType                  // type of list MORPH_ZTERMLIST or
);

__declspec(dllexport)
USHORT MorphAddTermToList2W
(
  PSZ_W    *ppList,                    // ptr to term list pointer
  PULONG   pulSize,                    // ptr to allocated size of term list
  PULONG   pulUsed,                    // ptr to used bytes in term list
  PSZ_W    pszTerm,                    // ptr to new term being added to list
  USHORT   usLength,                   // length of term
  USHORT   usOffs,                     // offset of term
  ULONG    ulFlags,                    // flags for the term
  USHORT   usListType                  // type of list MORPH_ZTERMLIST or
);

USHORT MorphCopyTermListASCII2Unicode
(
  PSZ      pList,                      // ptr to ASCII termlist
  USHORT   usBufferSize,               // size of allocated buffer
  PSZ_W    *ppListW,                   // ptr to term list pointer
  PULONG   pulSize,                    // ptr to allocated size of term list
  PULONG   pulUsed,                    // ptr to used bytes in term list
  USHORT   usListType,                  // type of list
  ULONG    ulOemCP
);

USHORT MorphCopyTermListUnicode2ASCII
(
  PSZ_W    pListW,                      // inputptr to unicode termlist
  USHORT   usListSize,                 // size of List
  PSZ     *ppList,                     // output to ASCII term list pointer
  PULONG   pulSize,                    // ptr to allocated size of term list
  PULONG   pulUsed,                    // ptr to used bytes in term list
  USHORT   usListType,                  // type of list
  ULONG    ulOemCP
);

USHORT MorphCopyStringListASCII2Unicode
(
  PUCHAR   pList,                    // ptr to ASCII termlist
  LONG     lListSize,                // size of List
  PSZ_W    *ppListW,                 // ptr to term list pointer
  PLONG    plSize,                   // ptr to allocated size of term list
  PLONG    plUsed ,                   // ptr to used bytes in term list
  ULONG    ulOemCP
);

USHORT MorphCopyStringListUnicode2ASCII
(
  PSZ_W    pList,                    // ptr to ASCII termlist
  LONG     lListSize,                // size of List
  PSZ     *ppList,                   // ptr to term list pointer
  PLONG    plSize,                   // ptr to allocated size of term list
  PLONG    plUsed,                    // ptr to used bytes in term list
  ULONG    ulOemCP
);




USHORT MorphExtractKeywords
(
   SHORT    sLanguageID,               // language ID
   PSZ_W      pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                 // type of term list MORPH_ZTERMLIST or
                                          //    MORPH_OFFSLIST
   ULONG    ulOemCP
);

__declspec(dllexport)
USHORT MorphCompIsolation
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszTerm,                   // pointer to input term
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,                // address of caller's term list pointer
   ULONG    ulOemCP
);

__declspec(dllexport)
USHORT MorphFreeLanguageID
(
  SHORT    sLanguageID                 // language ID being freed
);

BOOL MorphCheckLanguage
(
  PSZ     pszLanguage,                 // name of language
  PSHORT  psLanguageID                 // variable receiving the language ID
);

BOOL MorphCheckLanguageByPlugin
(
 PSZ     pszSpellPlugin,              // name of the spell check plugin
 PSZ     pszMorphPlugin,              // name of the morph plugin
 PSZ     pszLanguage,                 // name of language
 PSHORT  psLanguageID                 // variable receiving the language ID
 );

__declspec(dllexport)
USHORT MorphMultStemForm
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   USHORT   usBufferSize,              // size of buffer for stem form term
   PSZ_W    pBuffer,                    // address of output buffer
   ULONG    ulOemCP
);

USHORT MorphBuildDict
(
   SHORT    sLanguageID,               // language ID
   USHORT   usDictType,                // IN : type of dictionary
                                       //      ADDENDA_DICT or
                                       //      ABBREV_DICT
   ULONG    ulTermListSize,            // IN :  address of variable
                                       //    containing size of term list buffer
   PVOID    pTermList,                 // IN : address of term list pointer
   USHORT   usListType                 // IN : type of term list :
                                       //      MORPH_ZTERMLIST
);

USHORT MorphGetLanguageString
(
  SHORT   sLanguageID,                 // language id
  PSZ     pszLanguage                  // name of language
);


// defines for function MorphGetLanguageType:
#define MORPH_UNDEFINED_LANGTYPE  0
#define MORPH_DBCS_LANGTYPE       1
#define MORPH_THAI_LANGTYPE       4
#define MORPH_BIDI_A_LANGTYPE     8
#define MORPH_BIDI_H_LANGTYPE    16

__declspec(dllexport)
SHORT MorphGetLanguageType
(
  PSZ              pszLanguage         // language to be checked
);

USHORT MorphFreeTermList
(
  PVOID       pTermList,               // term list pointer
  USHORT      usListType               // type of term list
);


USHORT MorphThreadEnds
(
   SHORT    sLanguageID                // language ID
);


//+----------------------------------------------------------------------------+
//   Defines for MWTs                                                           
//+----------------------------------------------------------------------------+


typedef enum
{
  EQF_MWT_EMPTY,
  EQF_MWT_START_NOUN,
  EQF_MWT_START_ADJ,
  EQF_MWT_START_NOUN_ADJ
}  EQFMWTMODE ;


