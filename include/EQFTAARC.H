//+----------------------------------------------------------------------------+
//|EQFTAARC.H                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:        G. Queck (QSoft)                                             |
//+----------------------------------------------------------------------------+
//|Description:   Private include file of archive TM function                  |
//+----------------------------------------------------------------------------+


#define BUILDARCHTM_TASK    USER_TASK + 1

// XLATE markups (markups which can't be handled within this function)
#define XLATEOS2_MARKUP "XLATEOS2"
#define XLATEWIN_MARKUP "XLATEWIN"

/**********************************************************************/
/* process step enumeration                                           */
/**********************************************************************/
typedef enum _ARCHTM_STEP
{
  PROCESS_INIT,                        // intialize the process
  NEXT_DOCUMENT,                       // start next document
  START_DOCUMENT,                      // prepare current document
  PROCESS_DOCUMENT,                    // process current document
  END_DOCUMENT,                        // cleanup current document
  PROCESS_COMPLETE,                    // complete the process
  END_PROCESS                          // end process loop
} ARCHTM_STEP;

/**********************************************************************/
/* Instance Data Area (IDA) for build archive TM function             */
/**********************************************************************/
typedef struct _ARCHTM_IDA
{
  CHAR        szArchTM[MAX_LONGFILESPEC];     // name of archive TM
  CHAR        szTempTM[MAX_LONGFILESPEC];     // name of temporary output TM
  CHAR        szShortArchTM[MAX_FILESPEC];     // name of archive TM
  CHAR        szMemPath[MAX_LONGPATH]; // fully qualified TM name
  CHAR        szOrgMemPath[MAX_LONGPATH]; // fully qualified TM name of original TM
  CHAR        szMemExt[10];            // buffer for TM base file extension
  CHAR        szIndexExt[10];          // buffer for TM index extension
  BOOL        fUseAsFolderTM;          // TRUE = use archive TM as folder TM
  BOOL        fClearTM;                // TRUE = clear TM before adding segments
  BOOL        fKill;                   // TRUE = end current process immediately
  BOOL        fTerminate;              // TRUE = termination processing active
  BOOL        fDisabled;               // TRUE = disable dialog
  HWND        hwndDocLB;               // listbox for document names
  OBJNAME     szParentObjName;         // object name of calling (sub)folder
  OBJNAME     szFolObjName;            // object name of calling main folder
  CHAR        szFolder[MAX_FILESPEC];  // main folder name (with folder extension)
  CHAR        szFolName[MAX_FILESPEC]; // main folder name (without folder extension)
  CHAR        szFolLongName[MAX_LONGFILESPEC]; // long folder name
  OBJNAME     szObjName;               // build archive TM object name
  CHAR        szBuffer[4096];          // general purpose buffer
  BOOL        fErrorStop;              // TRUE = process stopped by an error
  USHORT      usComplete;              // current completion rate
  USHORT      usLastComplete;          // completion rate as displayed by slider
  ARCHTM_STEP CurStep;                 // current processing step
  ULONG       ulSegments;              // number of segments added to archive TM
  SHORT       sCurDoc;                 // number of current document (within listbox)
  CHAR        szCurDoc[MAX_FILESPEC];  // name of current document
  CHAR        szLongName[MAX_LONGFILESPEC];  // name of current document
  CHAR        szAlias[MAX_LONGFILESPEC];// alias name of current document
  CHAR        szShortAlias[MAX_FILESPEC];// short alias name of current document
  OBJNAME     szDocObjName;            // object name of current document
  SHORT       sMaxDocs;                // number of documents to process
  OtmMemory   *pMem;                   // ptr to memory object 
  OtmMemory   *pMemOrg;                // ptr to original memory object 
  PLOADEDTABLE pLoadedQFTable;         // ptr to loaded QF TagTable
  CHAR        szDocFormat[MAX_FNAME];  // name of document markup table
  CHAR        szDocSourceLang[MAX_LANG_LENGTH]; // document source language
  CHAR        szDocTargetLang[MAX_LANG_LENGTH]; // document target language
  CHAR        szSourceDocName[MAX_EQF_PATH]; // buffer for source document name
  CHAR        szTargetDocName[MAX_EQF_PATH]; // buffer for target document name
  PTBDOCUMENT pSourceDoc;              // ptr to loaded source document
  PTBDOCUMENT pTargetDoc;              // ptr to loaded target document
  ULONG       ulSegNum;                // number of currently active segment
  BOOL        fXlateDocs;              // XLATE documents selected
  BOOL        fOtherDocs;              // non-XLATE documents selected
  HWND        hwndErrMsg;              // handle of parent window for error messages
  BOOL        fBatch;                  // TRUE = we are in batch mode
  PSZ         pszDocNames;             // list of document names
  PDDEARCHTM  pDDEArchTM;              // batch archive TM data structure
  ULONG       ulAddSegNum;             // segment number in additional table
  ULONG       ulActiveTable;           // active segment table
  BOOL        fSetMFlag;               // set machine translation flag in segs added to TM
  BOOL        fSourceSourceMem;        // create a source-source memory
  BOOL        fFolderLocked;           // folder has been locked flag
  CHAR        szFolMemory[MAX_LONGFILESPEC]; // name of folder TM
  CHAR        szMemDescription[512];   // buffer for memory description
  CHAR        szMemSourceLang[MAX_LANG_LENGTH]; // memory source language
  CHAR_W      szContext[MAX_SEGMENT_SIZE+1]; // buffer for segment context
  CHAR_W      szMetaData[MAX_SEGMENT_SIZE+1]; // buffer for segment metadata
  ULONG       ulMemTimeStamp;           // time stamp to be used for proposals written to the memory
} ARCHTM_IDA, *PARCHTM_IDA;

/**********************************************************************/
/* Prototypes for internal functions                                  */
/**********************************************************************/
MRESULT ArchTMInit( HWND, WPARAM, LPARAM );
MRESULT ArchTMCommand( HWND, SHORT, SHORT );
MRESULT ArchTMControl( HWND, SHORT, SHORT );
MRESULT ArchTMClose( HWND, WPARAM, LPARAM );
MRESULT TAArchTMProcCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

