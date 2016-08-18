//+----------------------------------------------------------------------------+
//|EQFFOLGM.H    Global Memory defines and structure definitions               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

// length of some character fields
#define PROJECTID_SIZE  5
#define DIVISION_SIZE   5
#define ARRDATE_SIZE   20
#define HEADERINFO_SIZE 40

// CTID memory handling options
typedef enum _GMMEMOPT { GM_SUBSTITUTE_OPT, GM_HFLAG_OPT, GM_HFLAGSTAR_OPT, GM_EXCLUDE_OPT } GMMEMOPT;


// entry in charge-to-id memory options table
typedef struct _GM_CTID_OPTIONS
{
  CHAR_W szProjectID[PROJECTID_SIZE+1];          // project ID
  CHAR_W szDivision[DIVISION_SIZE+1];            // division
  CHAR_W chICU;                                  // ICU
  LONG   lWords;                                 // number of words
  LONG   lSegments;                              // number of segments
  CHAR_W   szArrivalDate[ARRDATE_SIZE];          // arrival date
  BOOL    fTmbArchived;                          // archived by TMB flag
  GMMEMOPT Option;                               // processing option
} GM_CTID_OPTIONS, *PGM_CTID_OPTIONS;

// in-memory representation of a loaded global memory option XML file
typedef struct _GMOPTIONFILE
{
  // data from XML file header
  CHAR_W szGenerator[HEADERINFO_SIZE+1];          // generator of XML filter file
  CHAR_W szGeneratorVersion[HEADERINFO_SIZE+1];          // generator of XML filter file
  CHAR_W szCreationDate[HEADERINFO_SIZE+1];          // creation date
  CHAR_W szMetadataFileName[HEADERINFO_SIZE+1];          // original XML filter file name
  CHAR_W szInputSpecs[HEADERINFO_SIZE+1];          // input specification
  CHAR_W szFilterName[HEADERINFO_SIZE+1];          // filter name
  BOOL   fGlobMemFolder;                           // TRUE = this is a global memory folder

  // status information
  BOOL    fErrorInList;                          // read of list failed
  CHAR    szErrorText[512];                      // error message text

  // multi purpose buffers
  CHAR_W  szBufferW[128];                        

  // option table
  LONG    lNumOfEntries;                         // number of entries in CTID options table
  LONG    lMaxNumOfEntries;                      // size of CTID options table (in number of entries)
  GM_CTID_OPTIONS Options[1];                    // dynamically allocated CTID option table
} GMOPTIONFILE, *PGMOPTIONFILE;

// loads a global memory option XML file into memory
__declspec(dllexport)
BOOL GlobMemLoadOptionFile( PSZ pszFileName, PVOID *ppvCTIDList );

// free all resource asscociated with a loaded global memory XML file
__declspec(dllexport)
BOOL GlobMemFreeOptionFile( PVOID *ppvCTIDList );

// saves a global memory option file to disk
__declspec(dllexport)
BOOL GlobMemSaveOptionFile( PSZ pszFileName, PVOID pvLoadedOptionFile );

// get option for a proposal
__declspec(dllexport)
GMMEMOPT GlobMemGetFlagForProposal( PVOID pCTIDList, PSZ_W pszAddData );
