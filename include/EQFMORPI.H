//+----------------------------------------------------------------------------+
//|  EQFDMORPI.H  - Internal header file for Morphological Functions Module    |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2017, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+

//+----------------------------------------------------------------------------+
//|  General constants                                                         |
//+----------------------------------------------------------------------------+
#define BUFFER_SIZE        8192        // size of general use buffer(s)
#define DICTHANDLE         USHORT      // map to Nlp dictionary handles
#define PDICTHANDLE        PUSHORT     // map to Nlp dictionary handles
#define MORPHCODEPAGE      850         // default codepage
#define END_OF_ELEMENTS    0xFFFFFFFF  // end delimiter for element chain
#define INITIAL_POOL_SIZE  20000       // initial size of string pools
#define MAX_SEG_SIZE       0xFF00      // max size for a single segment
#define TERMLIST_INCR      1024        // increment for term list buffer size
#define TERM_SIZE          1024        // increment for term list buffer size

//+----------------------------------------------------------------------------+
//|  Language exit values                                                      |
//+----------------------------------------------------------------------------+
//#define MORPH_DEF_LANG_EXIT "EQFMOTOL" // name of default language exit
#define MORPH_DEF_LANG_EXIT "EQFMONUL" // name of default language exit
#define LANGEXIT_INIT       "INIT"     // name of exit INIT function
#define LANGEXIT_TOKENIZE   "TOKENIZE" // name of exit TOKENIZE function
#define LANGEXIT_VERIFY     "VERIFY"   // name of exit VERIFY function
#define LANGEXIT_SPELLAID   "SPELLAID" // name of exit SPELLAID function
#define LANGEXIT_TERMADD    "TERMADD"  // name of exit TERMADD function
#define LANGEXIT_LEMMA      "LEMMA"    // name of exit LEMMA function
#define LANGEXIT_ACTADD     "ACTADD"   // name of exit ACTADD function
#define LANGEXIT_TERMINATE  "TERMINATE"// name of exit TERMINATE function
#define LANGEXIT_GETPOS     "GETPOS"   // name of exit GETPOS function
#define LANGEXIT_COMPISOL   "COMPISOL" // name of exit COMPISOL function
#define LANGEXIT_ABBREVADD  "ABBREVADD" // name of exit ABBREVADD function
#define LANGEXIT_LISTDICT   "LISTDICT"  // name of exit LISTDICT function
#define LANGEXIT_BUILDDICT  "BUILDDICT" // name of exit BUILDDICT function
#define LANGEXIT_VERIFYBLOCK "VERIFYBLOCK"   // name of exit VERIFYBLOCK function
#define LANGEXIT_SPELLAIDW   "SPELLAIDW" // name of exit SPELLAIDW function
#define LANGEXIT_TOKENIZEW  "TOKENIZEW" // name of exit TOKENIZE function
#define LANGEXIT_TERMADDW   "TERMADDW"  // name of exit TERMADD function
#define LANGEXIT_LEMMAW     "LEMMAW"    // name of exit LEMMA function
#define LANGEXIT_GETPOSW    "GETPOSW"   // name of exit GETPOS function
#define LANGEXIT_COMPISOLW  "COMPISOLW" // name of exit COMPISOL function
#define LANGEXIT_ABBREVADDW "ABBREVADDW" // name of exit ABBREVADD function
#define LANGEXIT_THREADENDS "THREADENDS" // name of exit THREADENDS function

//+----------------------------------------------------------------------------+
//|  Defines for the Nlp POS info encoding                                     |
//+----------------------------------------------------------------------------+
#define NLP_PRONOUN        0           // pronoun identifier
#define NLP_VERB           1           // verb identifier
#define NLP_NOUN           2           // noun identifier
#define NLP_ADJECTIVE      3           // adjective identifier
#define NLP_ADVERB         4           // adverb identifier
#define NLP_PREPOSITION    5           // preposition identifier
#define NLP_INTERJECTION   6           // interjection identifier
#define NLP_CONJUNCTION    7           // conjunction identifier

//+----------------------------------------------------------------------------+
//|  Hash element as used by the MorphHash interface functions                 |
//+----------------------------------------------------------------------------+
typedef struct _HASHENTRY
{
  HASHLINK HashLink;                   // link used by low level hash routines
  PSZ_W    pszTerm;                    // pointer to term (= key of entry)
  PSZ_W    pLemmaList;                 // pointer to lemma list for term
  ULONG    ulKey;                      // key (= index) of this element
  ULONG    ulNext;                     // next element in recently used chain
                                       // (is also used by free element chain)
  ULONG    ulPrev;                     // previous element in recently used chain
  ULONG    ulListLength;               // length of lemma list for term
} HASHENTRY, *PHASHENTRY;

//+----------------------------------------------------------------------------+
//|  Hash Table Structure used by MorphHash functions                          |
//+----------------------------------------------------------------------------+
typedef struct _MORPHHASH
{
  PHASH            pHash;              // hash of low level hash routines
  ULONG            ulElements;         // number of elements in hash
  ULONG            ulMaxElements;      // max number of elements
  ULONG            ulFirstFree;        // index of first element in free list
  ULONG            ulFirstUsed;        // index of first elment in used chain
  ULONG            ulLastUsed;         // index of last elment in used chain
  PHASHENTRY       pElements;          // ptr to allocated element table
  PPOOL            StringPool;         // string pool
} MORPHHASH, *PMORPHHASH;

//+----------------------------------------------------------------------------+
//|  Language Control block (LCB)                                              |
//|                                                                            |
//|  contains all data necessary for morphologic language processing           |
//+----------------------------------------------------------------------------+
  typedef USHORT (*PFN_INIT)( PVOID *, PSZ, USHORT, USHORT );
  typedef USHORT (*PFN_ACTADD)( PVOID, PSZ );
  typedef USHORT (*PFN_TERMINATE)( PVOID );
  typedef USHORT (*PFN_TOKENIZE)( PVOID, PSZ, PUSHORT, PVOID *, USHORT );
  typedef USHORT (*PFN_VERIFY)( PVOID, PSZ );
  typedef USHORT (*PFN_SPELLAID)( PVOID, PSZ, PUSHORT, PVOID *, USHORT );
  typedef USHORT (*PFN_TERMADD)( PVOID, PSZ );
  typedef USHORT (*PFN_LEMMA)( PVOID, PSZ, PUSHORT, PVOID *, USHORT );
  typedef USHORT (*PFN_GETPOS)( PVOID, PSZ, PUSHORT );
  typedef USHORT (*PFN_COMPISOL)( PVOID, PSZ, PUSHORT, PVOID *, USHORT );
  typedef USHORT (*PFN_ABBREVADD)( PVOID, PSZ );
  typedef USHORT (*PFN_LISTDICT)( PVOID, USHORT, PUSHORT, PVOID *, USHORT );
  typedef USHORT (*PFN_BUILDDICT)( PVOID, USHORT, USHORT, PVOID, USHORT );
  typedef USHORT (*PFN_VERIFYBLOCK)( PVOID, PSZ_W, PUSHORT, PVOID *, USHORT );
  typedef USHORT (*PFN_SPELLAIDW)( PVOID, PSZ_W, PULONG, PVOID *, USHORT );
  typedef USHORT (*PFN_TOKENIZEW)( PVOID, PSZ_W, PULONG, PVOID *, USHORT );
  typedef USHORT (*PFN_TERMADDW)( PVOID, PSZ_W );
  typedef USHORT (*PFN_LEMMAW)( PVOID, PSZ_W, PULONG, PVOID *, USHORT );
  typedef USHORT (*PFN_GETPOSW)( PVOID, PSZ_W, PUSHORT );
  typedef USHORT (*PFN_COMPISOLW)( PVOID, PSZ_W, PULONG, PVOID *, USHORT );
  typedef USHORT (*PFN_ABBREVADDW)( PVOID, PSZ_W );
  typedef USHORT (*PFN_THREADENDS)( PVOID );


typedef struct _LCB                    // language control block
{
   TID      tidThread;                 // ID of thread calling morph functions
   CHAR     szLanguage[MAX_LANG_LENGTH+1]; // name of langage of this LCB
   CHAR     szLangExit[MAX_LONGFILESPEC];  // name of language exit
   CHAR     szSpellPlugin[MAX_LONGFILESPEC];    // name of the Spell plug-in
   CHAR     szMorphPlugin[MAX_LONGFILESPEC];    // name of the Morph plug-in
   BOOL     fNoCaching;                // do not use stem form cache flag
   BOOL     fNoDoNotLookup;            // do not use don't-lookup table flag
   BOOL     fCompSep;                  // language-supports-compound-seperation
   //-------------- compound list for use by stem form reduction only ------
   USHORT   usCompListSize;            // current size of compound list buffer
   PSZ_W    pCompList;                 // buffer for compound lists
   //---------------------- part for caching           ---------------------
   PMORPHHASH  HashCache;
   PMORPHHASH  HashNoLookup;
   PMORPHHASH  POSHashCache;
   PMORPHHASH  POSHashNoLookup;
   //---------------------- part for dictionaries      ---------------------
   CHAR     szMorphDict[MAX_LONGFILESPEC]; // name of morphologic dictionary
   CHAR     szAddendaDict[MAX_LONGFILESPEC];// name of addenda dictionary
   //---------------------- part for language exit     ---------------------
   PVOID           pvLangCB;           // control block pointer of language exit
   BOOL            fAddendaDirty;      // addenda-dict-requires-refresh flag
   BOOL            fAbbrevDirty;       // abbrev.-dict-requires-refresh flag

   //---- part for performance measurements in stem form area ----
   ULONG    ulStemAtomTime;            // time in atom manager calls
   ULONG    ulStemCacheTime;           // time in cache functions
   ULONG    ulStemNlpTime;             // time in Nlp calls or in user exit
   ULONG    ulStemOtherTime;           // time in remaining code of function
   ULONG    ulStemRequests;            // # of stem from reduction requests
   ULONG    ulStemFound;               // # of stem forms found
   ULONG    ulAtomFound;               // # of terms found in atom table
   ULONG    ulAtomRequests;            // # of lookups in atom table
   ULONG    ulCacheRequests;           // # of lookups in cache
   ULONG    ulCacheFound;              // # of terms found in cache
   ULONG    ulCacheTerms;              // # of terms stored in cache
   ULONG    ulNlpLookups;              // # of terms looked up using Nlp service
   ULONG    ulNlpLookupsLwr;           // # of Nlp lookups in lowercase
   ULONG    ulNlpFound;                // # of terms found using Nlp service
   ULONG    ulNlpFoundLwr;             // # of lowercase terms found using Nlp
   //---------------------- general use data areas ----------------------------
   CHAR     szBuffer[BUFFER_SIZE];     // general use buffer
   USHORT   usCodePage;                // codepage to be used for POE
} LCB, *PLCB;

//+----------------------------------------------------------------------------+
//|  Structure to process offset/length type term lists                        |
//+----------------------------------------------------------------------------+
typedef union _LISTTERM
{
  ULONG   ul;
  struct
  {
    USHORT   usLength;
    USHORT   usOffs;
  } OffsLen;
} LISTTERM, *PLISTTERM;

//+----------------------------------------------------------------------------+
//|  Prototypes for internal functions of the morphologic functions module     |
//+----------------------------------------------------------------------------+
__declspec(dllexport) USHORT MorphGetLCB( SHORT sLanguageID, PLCB *ppLCB );
USHORT MorphRefreshDicts( PLCB pLCB, USHORT usDictType );
__declspec(dllexport) USHORT MorphCheck4Refresh( PLCB pLCB );


//+----------------------------------------------------------------------------+
//|  Prototypes for morph hash functions                                       |
//+----------------------------------------------------------------------------+
PMORPHHASH MorphHashCreate( ULONG ulMaxElements );
BOOL       MorphHashDestroy( PMORPHHASH pMorphHash );
BOOL       MorphHashReset( PMORPHHASH pMorphHash );
BOOL       MorphHashAdd( PMORPHHASH, PSZ_W, ULONG, PSZ_W );
BOOL       MorphHashDelete( PMORPHHASH, PHASHENTRY pEntry );
BOOL       MorphHashSearch( PMORPHHASH, PSZ_W, PULONG, PSZ_W * );
BOOL       MorphHashWrite( PMORPHHASH pMorphHash, PSZ pszFileName );
BOOL       MorphHashRead( PMORPHHASH, PSZ, BOOL );

//+----------------------------------------------------------------------------+
//|  Prototypes for the user functions called by the low level hash routines   |
//+----------------------------------------------------------------------------+
SHORT  MorphHashCompare( PHASHENTRY pEntry, PHASHENTRY pEntry2, PVOID pUserPtr );
SHORT  MorphHashFree( PHASHENTRY pEntry, PVOID pUserPtr  );
USHORT MorphHashKeyValue( ULONG, PHASHENTRY );
SHORT  MorphHashWriteElement( PHASHENTRY pEntry, PBUFCB pBufCB  );


