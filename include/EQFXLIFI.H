//+----------------------------------------------------------------------------+
//| EQFXLIFI.H                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: XLIFF import / export interface                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#ifndef EQFXLIFI_H_INCLUDED

#define EQFXLIFI_H_INCLUDED

// size of token buffer
#define TMXTOKBUFSIZE 32000

// name to use for undefined languages
#define OTHERLANGUAGES "OtherLanguages"

// name of TM specific prop elements
#define DESCRIPTION_PROP      "tmgr-description"
#define DESCRIPTION_PROP_W    L"tmgr-description"

#define MARKUP_PROP      "tmgr-markup"
#define MARKUP_PROP_W    L"tmgr-markup"

#define LANGUAGE_PROP    "tmgr-language"
#define LANGUAGE_PROP_W  L"tmgr-language"

#define DOCNAME_PROP    "tmgr-docname"
#define DOCNAME_PROP_W  L"tmgr-docname"

#define MACHFLAG_PROP    "tmgr-MTflag"
#define MACHFLAG_PROP_W  L"tmgr-MTflag"


// tmgr names used in xliff folder export/
#define TM_PROPERTIES      "tmgr:properties"
#define TM_MARKUP          "tmgr:markup"
#define TM_MEMORY          "tmgr:memory"
#define TM_SOURCELANG      "tmgr:sourcelang"
#define TM_TARGETLANG      "tmgr:targetlang"
#define TM_SHORTNAME       "tmgr:shortname"
#define TM_ALIAS           "tmgr:alias"
#define TM_EDITOR          "tmgr:editor"
#define TM_CONVERSION      "tmgr:conversion"
#define TM_VENDOR          "tmgr:vendor"
#define TM_VENDOREMAIL     "tmgr:vendoremail"
#define TM_SHIPMENT        "tmgr:shipment"

#define TM_FOLDER          "tmgr:folder"
#define TM_DESCRIPTION     "tmgr:description"
#define TM_ORGNAME         "tmgr:orgname"
#define TM_ORGLONGNAME     "tmgr:orglongname"
#define TM_LONGNAME        "tmgr:longname"
#define TM_COORDINATOR     "tmgr:coordinator"
#define TM_COORDINATOREMAIL  "tmgr:coordinatoremail"
#define TM_PARENT          "tmgr:parent"
#define TM_PARENTEMAIL     "tmgr:parentemail"
#define TM_EXPORIGINATOR   "tmgr:exporigionator"
#define TM_EXPEMAIL        "tmgr:expemail"
#define TM_VENDOR          "tmgr:vendor"
#define TM_VENDOREMAIL     "tmgr:vendoremail"
#define TM_PRODUCT         "tmgr:product"
#define TM_PRODUCTFAMILY   "tmgr:productfamily"
#define TM_SIMILARPRODUCT  "tmgr:similarproduct"
#define TM_SUBJECTDICT     "tmgr:subjectdict"
#define TM_SUBJECTMEM      "tmgr:subjectmem"
#define TM_PREVVERSION     "tmgr:prevversion"
#define TM_VERSION         "tmgr:version"
#define TM_ROMEM           "tmgr:romem"
#define TM_DICT            "tmgr:dict"
#define TM_SEGSTATUS       "tmgr:segstatus"
#define TM_MACHMATCH       "tmgr:machinematch"
#define TM_SEGNUM          "tmgr:segnumber"
#define TM_DATETIME        "tmgr:datetime"
#define TM_MATCHINFO       "tmgr:matchinfo"
#define TM_MATCHRATIO      "tmgr:matchratio"

#define TM_SEGSTATUS_L     L"tmgr:segstatus"

// attribute types
typedef enum { MODIFIED_ON_ATTR, MODIFIED_BY_ATTR, CREATED_ON_ATTR, CREATED_BY_ATTR, IMPORT_JOB_ID , UNKNOWN_ATTR } ATTRIBUTEID;

// IDs of XLIFF elelements
typedef enum { XLIFF_ELEMENT, IWS_TM_ATTRIBUTE_METADATA_ELEMENT, IWS_PROJECT_ATTRIBUTE_METADATA_ELEMENT,
               IWS_PROJECT_ATTRIBUTES, IWS_SCOPING_CONFIGURATION_ELEMENT, IWS_DWB_PROJECT_INFO_ELEMENT, IWS_EXTENSION_LEVEL_ELEMENT, 
               FILE_ELEMENT, HEADER_ELEMENT, BODY_ELEMENT, TRANS_UNIT_ELEMENT, SOURCE_ELEMENT, TARGET_ELEMENT, ALT_TRANS_ELEMENT, 
               IWS_ATTRIBUTE_ELEMENT, IWS_SEGMENT_METADATA_ELEMENT, IWS_BOUNDARY_SEG_ELEMENT, IWS_MARKUP_SEG_ELEMENT, PH_ELEMENT,
               IWS_STATUS_ELEMENT, NOTE_ELEMENT, TM_PROPERTIES_ELEMENT, TM_MARKUP_ELEMENT, TM_MEMORY_ELEMENT, TM_SOURCELANG_ELEMENT,
               TM_TARGETLANG_ELEMENT, TM_SHORTNAME_ELEMENT, TM_ALIAS_ELEMENT, TM_EDITOR_ELEMENT, TM_CONVERSION_ELEMENT,
               TM_VENDOR_ELEMENT, TM_VENDOREMAIL_ELEMENT, TM_SHIPMENT_ELEMENT, TM_FOLDER_ELEMENT, TM_DESCRIPTION_ELEMENT,
               TM_ORGNAME_ELEMENT, TM_ORGLONGNAME_ELEMENT, TM_LONGNAME_ELEMENT, TM_COORDINATOR_ELEMENT, TM_COORDINATOEMAIL_ELEMENT,
               TM_PARENT_ELEMENT, TM_PARENTEMAIL_ELEMENT, TM_EXPORIGIONATOR_ELEMENT, TM_EXPEMAIL_ELEMENT, 
               TM_PRODUCT_ELEMENT, TM_PRODUCTFAMILY_ELEMENT, TM_SIMILARPRODUCT_ELEMENT, TM_SUBJECTDICT_ELEMENT,
               TM_SUBJECTMEM_ELEMENT, TM_PREVVERSION_ELEMENT, TM_VERSION_ELEMENT, TM_ROMEM_ELEMENT, TM_DICT_ELEMENT,
               TM_MACHMATCH_ELEMENT, TM_SEGNUM__ELEMENT, TM_DATETIME_ELEMENT, TM_SEGSTATUS_ELEMENT, UNKNOWN_ELEMENT, SUB_ELEMENT } ELEMENTID;

// stack elements
typedef struct _XLIFFELEMENT
{
  ELEMENTID ID;                        // ID of element 
  ATTRIBUTEID AttrID;             // ID of current attribute
  BOOL      fInlineTagging;            // inline tag processing
  BOOL      fProposal;                 // proposal processing
} XLIFFELEMENT, *PXLIFFELEMENT;


//+----------------------------------------------------------------------------+
//| Class for name lists                                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
class CNameList
{
  public:
	  CNameList();
	  ~CNameList();
    BOOL AddName( const char *pszName, const char *pszValue );  // add a name/value pair to the list
    BOOL FindName( const char *pszName, char *pszValue, int iBufSize );  // find a name and returns its value
  protected:
    PSZ m_pList;                       // points to list data buffer
    int m_iListSize;                   // size of list in # of bytes
};

//+----------------------------------------------------------------------------+
//| Class for tagging lists                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
class CTagList
{
  public:
	  CTagList();
	  ~CTagList();
    void clear();
    int add( int id, PSZ_W pszTagging );
    PSZ_W get( int id );
  protected:
    struct _TAGGING
    {
      int id;                                    // ID of tag
      int iOffset;                               // offset of tag withing buffer
    } TAGGING, *PTAGGING;

    int m_iTaggingUsed;                          // used elements in tagging table 
    int m_iTaggingSize;                          // size of tagging table in # of elements
    struct _TAGGING *m_pTagging;                 // pointer to tagging table 
    int m_iBufSize;                              // size of data buffer in # of characters
    int m_iBufUsed;                              // used # of characters in buffer
    PSZ_W    m_pBuffer;                          // points to data buffer
};

//
// class for our XLIFF format related utilities
//
class CXLIFFUtil
{
  public:
  CXLIFFUtil();
  ~CXLIFFUtil();
  BOOL TMXLanguage2TMLanguage( PSZ pszTMLanguage, PSZ pszTMXLanguage, PSZ pszResultingLanguage );
  void SetNameLists( CNameList *pLangTmx2Tmgr, CNameList *pType2Markup );

  // date conversion help functions
  BOOL IsLeapYear( const int iYear );
  int GetDaysOfMonth( const int iMonth, const int iYear );
  int GetDaysOfYear( const int iYear );
  int GetYearDay( const int iDay, const int iMonth, const int iYear );

  // file related methods
  USHORT BuildTMFileName( PSZ pszFileName, CHAR chDrive, USHORT usPathID, PSZ pszFolder, PSZ pszDocument );
  int OpenOutputFile( PSZ pszFile, FILE **phfFile );
  int CloseFile( FILE *hfFile );
  int WriteBOM( FILE *hfFile );

  // segmented file functions
  int LoadSegmentedFile( PSZ pszFileName, PTBDOCUMENT *ppDoc, BOOL fTargetDoc );
  int FreeSegmentedFile( PTBDOCUMENT *ppDoc );
  int SaveSegmentedFile( PSZ pszFile, PTBDOCUMENT pDoc );
  int UpdateSegment( PTBDOCUMENT pDoc, ULONG ulSeg, PSZ_W pszData );
  int SetDocCompletion( PSZ pszFolObjName, PSZ pszDocShortName, PTBDOCUMENT pDoc );

  private:

  // name list for name conversions
  CNameList *pLangTmx2Tmgr;
  CNameList *pType2Markup;

};

//
// class for our Idiom XLIFF SAX handler
//
class IdiomXLIFFParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  IdiomXLIFFParseHandler();
  virtual ~IdiomXLIFFParseHandler();

  // processing modes
  typedef enum _XLIFFPROCESSMODE { EXPORT_MODE, IMPORT_MODE } XLIFFPROCESSMODE;

  // setter functions for import info
  void SetFile( USHORT usFileID, FILE *hf );
  void SetNameLists( CNameList *pLangTmx2Tmgr, CNameList *pType2Markup );
  void SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle ); 
  void SetSourceLanguage( char *pszSourceLang );
  void SetXMLWriter( CXmlWriter *pxw );
  void SetMode( XLIFFPROCESSMODE mode );
  void SetLoadedDocs( PTBDOCUMENT pTargetDoc, PTBDOCUMENT pSourceDoc );
  void SetLanguagePointer( PSZ pszInSourceLanguage, PSZ pszInTargetLanguage );
  void SetSourceFileName( PSZ pszSourceFile );

  // getter functions 
  void GetDescription( char *pszDescription, int iBufSize );
  void GetSourceLanguage( char *pszSourceLang, int iBufSize );
  BOOL IsHeaderDone( void );
  LONG GetTotalSegs(){ return(lTotalSegs); } ;
  LONG GetTranslatedSegs(){ return( lTranslatedSegs ); } ;

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t /*unsigned int*/ length);
  //void ignorableWhitespace(const XMLCh* const chars, const unsigned int length);
  //void resetDocument();


  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);
  //void resetErrors();

private:
  ELEMENTID GetElementID( PSZ_W pszName );
  void Push( PXLIFFELEMENT pElement );
  void Pop( PXLIFFELEMENT pElement );
  BOOL GetValue( PSZ pszString, int iLen, int *piResult );

  // mem import interface data
  PFN_MEMINSERTSEGMENT pfnInsertSegment;
  LONG lMemHandle;

  // other helper functions
  int WriteTransUnitFromSegmentData( PSZ_W pszFileName, LONG lTranslUnit );
  int WriteSegmentText( PSZ_W pszText );
  int WriteDateAsNumber( LONG lDate );
  BOOL GetAttrValue( PSZ_W pszString, PSZ_W pszAttrName, PSZ_W pszBuffer );
  PSZ_W FindTag( PSZ_W pszText, PSZ_W pszTag );

  // processing mode
  XLIFFPROCESSMODE ProcessingMode;

  // processing flags
  BOOL fSource;                        // TRUE = source data collected  
  BOOL fTarget;                        // TRUE = target data collected
  BOOL fCatchData;                     // TRUE = catch data
  BOOL fWithTagging;                   // TRUE = add tagging to data
  BOOL fHeaderDone;                    // TRUE = header has been processed
  BOOL fPassThru;                      // TRUE = write all elements and attributes directly to XML writer

  // segment data
  ULONG ulSegNo;                       // segmet number
  LONG  lTime;                         // segment date/time
  BOOL  fMachineTranslation;           // machine translation flag
  ULONG m_ulSegNum;                    // current segment number

  // file data area
  typedef struct _FILEINFO
  {
    CHAR_W   szSourceLanguage[40];               // buffer for ISO source language
    CHAR_W   szTargetLanguage[40];               // buffer for ISO target language
    CHAR_W   szContent[256];                     // buffer for content info
    CHAR_W   szDataType[256];                    // buffer for data type
    CHAR_W   szTool[256];                        // buffer for tool info
    CHAR_W   szBuild[256];                       // buffer for build info
    CHAR_W   szFile[MAX_LONGFILESPEC];           // buffer for file name
  } FILEINFO, *PFILEINFO;


  // translation unit data area
  typedef struct _TRANSLUNIT
  {
    ULONG    ulNum;                              // number of this translation unit
    CHAR_W   szSource[MAX_SEGMENT_SIZE];         // buffer for source text
    CHAR_W   szTarget[MAX_SEGMENT_SIZE];         // buffer for target text
    PMD_METADATA pMetaData;                      // metadata of this translation unit
    BOOL     fPlaceHoldersOnly;                  // TRUE = target contains placeholders only
    BOOL     fApproved;                          // TRUE = translation unit has been approved
    CHAR_W   szState[50];                        // state of translation unit target
    PMD_NOTELIST_DATA pNoteList;                 // ptr to current notelist
    LONG     lSourceWords;                       // number of source words
  } TRANSLUNIT, *PTRANSLUNIT;

  // proposal data area
  typedef struct _PROPOSAL
  {
    SHORT    sQuality;                           // quality of this proposal
    CHAR_W   szSource[MAX_SEGMENT_SIZE];         // buffer for source text
    CHAR_W   szTarget[MAX_SEGMENT_SIZE];         // buffer for target text
    CHAR_W   szDocName[MAX_LONGFILESPEC];        // buffer for proposal document
    CHAR_W   szAuthor[40];                       // modifier of proposal
    LONG     lCreationDate;                      // creation date
    CHAR_W   szModifier[40];                     // modifier of proposal
    LONG     lModificationDate;                  // modification date
  } PROPOSAL, *PPROPOSAL;


  // tagging of current unit
  CTagList *m_pTagging;

  // instance of our XLIFF utilities
  CXLIFFUtil *m_Util;

  // buffers
  #define DATABUFFERSIZE 4098

  typedef struct _BUFFERAREAS
  {
    CHAR_W   szData[DATABUFFERSIZE];  // buffer for collected data
    CHAR_W   szTagging[DATABUFFERSIZE];  // buffer for tagging
    CHAR_W   szProp[DATABUFFERSIZE];  // buffer for collected prop values
    CHAR     szLang[50];              // buffer for language
    CHAR     szDocument[EQF_DOCNAMELEN];// buffer for document name
    MEMEXPIMPSEG SegmentData;         // buffer for segment data
    CHAR     szDescription[1024];     // buffer for memory descripion
    CHAR     szMemSourceLang[50];     // buffer for memory source language
    TRANSLUNIT TranslUnit;            // buffer for current translation unit
    PROPOSAL  Proposal;               // buffer for current proposal 
    FILEINFO  FileInfo;               // buffer for file info 
    BYTE      PropMetaData[16000];    // buffer to prepare proposal metadata
    CHAR_W    szNoteAuthor[40];       // buffer for note author
  } BUFFERAREAS, *PBUFFERAREAS;

  PBUFFERAREAS pBuf; 

  int iCurTuv;                        // current TUV index
  int iTuvArraySize;                  // current size of TUV array

  // element stack
  int iStackSize;
  int iCurElement;
  XLIFFELEMENT CurElement;
  PXLIFFELEMENT pStack;

  bool            fSawErrors;

  // file handles
  FILE        *hfLog;                            // log file handle
  FILE        *hfSource;                         // source file handle
  FILE        *hfSegSource;                      // segmented source file handle
  FILE        *hfSegTarget;                      // segmented target file handle
  FILE        *hfMetaData;                       // metadata file handle
  CXmlWriter  *pxw;                              // ptr to active CXmlWriter object

  PTBDOCUMENT  pTargetDoc;                       // loaded target document
  PTBDOCUMENT  pSourceDoc;                       // loaded source document
  ULONG        ulSegNum;                         // current segment number
  PSZ          pszSourceLanguage;                // ptr to buffer for source language 
  PSZ          pszTargetLanguage;                // ptr to buffer for target language
  LONG         lTranslatedSegs;                  // number of translated segments
  LONG         lTotalSegs;                       // total number of translatable segments 
  CHAR_W       szSourceFile[MAX_LONGFILESPEC];   // name of source file (w7o path info)
};

//
// class for our FolderXLIFF SAX handler
//
class FolXliffParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  FolXliffParseHandler();
  virtual ~FolXliffParseHandler();

  // setter functions for import info
  void SetFile( USHORT usFileID, FILE *hf );
  void SetNameLists( CNameList *pLangTmx2Tmgr, CNameList *pType2Markup );
  void SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle ); 
  void SetSourceLanguage( char *pszSourceLang );
  void SetXMLWriter( CXmlWriter *pxw );
  void SetSourceFileName( PSZ pszSourceFile );
  void SetImpExpData( PFOLIMPEXPXLIFF pData);

  // getter functions 
  void GetDescription( char *pszDescription, int iBufSize );
  void GetSourceLanguage( char *pszSourceLang, int iBufSize );
  BOOL IsHeaderDone( void );
  LONG GetTotalSegs(){ return(lTotalSegs); } ;
  LONG GetTranslatedSegs(){ return( lTranslatedSegs ); } ;
  BOOL ErrorOccured( void );
  void GetErrorText( char *pszTextBuffer, int iBufSize );

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t /*unsigned int*/ length);
  //void ignorableWhitespace(const XMLCh* const chars, const unsigned int length);
  //void resetDocument();


  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);
  //void resetErrors();

private:
  ELEMENTID GetElementID( PSZ_W pszName );
  void Push( PXLIFFELEMENT pElement );
  void Pop( PXLIFFELEMENT pElement );
  ELEMENTID GetParentID();
  BOOL GetValue( PSZ pszString, int iLen, int *piResult );

  // creation functions
  int CheckFolder();
  int CheckAndPrepareDocument();

  // mem import interface data
  PFN_MEMINSERTSEGMENT pfnInsertSegment;
  LONG lMemHandle;

  // memory interface function
  int OpenMem( PSZ pszMemory );
  int CloseMem();
  int UpdateMemProposal( ULONG ulSegNum, PSZ_W pszSource, PSZ_W pszTarget, PSZ pszMarkup, PSZ pszSourceLang, 
                         PSZ pszTargetLang, PSZ pszShortName, PSZ pszLongName, LONG lTime );

  // other helper functions
  int WriteTransUnitFromSegmentData( PSZ_W pszFileName, LONG lTranslUnit );
  int WriteSegmentText( PSZ_W pszText );
  int WriteDateAsNumber( LONG lDate );
  BOOL GetAttrValue( PSZ_W pszString, PSZ_W pszAttrName, PSZ_W pszBuffer );
  PSZ_W FindTag( PSZ_W pszText, PSZ_W pszTag );
  void CopyWhenFilled( PSZ pszFrom, PSZ pszTo );

  // processing flags
  BOOL fSource;                        // TRUE = source data collected  
  BOOL fTarget;                        // TRUE = target data collected
  BOOL fCatchData;                     // TRUE = catch data
  BOOL fWithTagging;                   // TRUE = add tagging to data
  BOOL fHeaderDone;                    // TRUE = header has been processed
  BOOL fPassThru;                      // TRUE = write all elements and attributes directly to XML writer

  // segment data
  ULONG ulSegNo;                       // segmet number
  LONG  lTime;                         // segment date/time
  BOOL  fMachineTranslation;           // machine translation flag
  ULONG m_ulSegNum;                    // current segment number

  // file data area
  typedef struct _FILEINFO
  {
    CHAR_W   szSourceLanguage[40];               // buffer for ISO source language
    CHAR_W   szTargetLanguage[40];               // buffer for ISO target language
    CHAR_W   szContent[256];                     // buffer for content info
    CHAR_W   szDataType[256];                    // buffer for data type
    CHAR_W   szTool[256];                        // buffer for tool info
    CHAR_W   szBuild[256];                       // buffer for build info
    CHAR     szFile[MAX_LONGFILESPEC];           // buffer for file name
    CHAR     szTMSourceLanguage[40];             // buffer for TM source language
    CHAR     szTMTargetLanguage[40];             // buffer for TM target language
    // info from tmgr_properties
    CHAR     szMarkup[MAX_FILESPEC];             // buffer for TM markup
    CHAR     szMemory[MAX_LONGFILESPEC];         // buffer for TM language
    CHAR     szShortName[MAX_FILESPEC];          // buffer for document short name
    CHAR     szAlias[MAX_LONGFILESPEC];          // buffer for document alias
    CHAR     szEditor[MAX_FILESPEC];             // buffer for document alias
    CHAR     szConversion[MAX_DESCRIPTION];      // buffer for document conversion 
    CHAR     szVendor[MAX_DESCRIPTION];          // buffer for vendor Name
    CHAR     szVendorEMail[MAX_DESCRIPTION];     // buffer for vendor e-mail
    CHAR     szShipment[MAX_DESCRIPTION];        // buffer for shipment string
    CHAR     szFolLongName[MAX_LONGFILESPEC];    // buffer for folder long name
    CHAR     szFolShortName[MAX_FILESPEC];       // buffer for folder short name
    CHAR     szFolObjName[MAX_EQF_PATH];         // buffer for folder object name
    CHAR     szDocObjName[MAX_EQF_PATH];         // buffer for documentobject name
  } FILEINFO, *PFILEINFO;

  // folder data area
  typedef struct _FOLDERINFO
  {
   char      szDescription[ 41];     // folder description
   char      szFormat[MAX_FILESPEC];         // folder's documents format table
   char      szMemory[MAX_FILESPEC];         // folder's memory db
   CHAR      szEditor[MAX_FILESPEC];     // name of editor
   CHAR      szSourceLang[MAX_LANG_LENGTH]; // FOLDER SOURCE LANGUAGE
   CHAR      szTargetLang[MAX_LANG_LENGTH]; // FOLDER TARGET LANGUAGE
   CHAR      szOrgName[MAX_FNAME];          // original folder name
   CHAR      szConversion[MAX_DESCRIPTION]; // document export conversion / CP
   CHAR      szLongName[MAX_LONGFILESPEC];  // folder long (descriptive) name
   char      szLongMemory[MAX_LONGFILESPEC];     // long name of folder's memory db
   CHAR      aLongMemTbl[MAX_NUM_OF_READONLY_MDB][MAX_LONGFILESPEC];   // folder's list of R/O mems
   CHAR      aLongDicTbl[MAX_NUM_OF_FOLDER_DICS][MAX_LONGFILESPEC];  // folder's list of dictionaries
   CHAR      szOrgLongName[MAX_LONGFILESPEC];  // original folder long (descriptive) name
   EQF_BOOL  fAFCFolder;                          // Mark the folder as AFC folder
   EQF_BOOL  fTCMasterFolder;                     // Mark TC Master folder
   CHAR      szCoordinator[MAX_DESCRIPTION];      // Project coordinator
   CHAR      szCoordinatorEMail[MAX_DESCRIPTION]; // Project coordinator e-mail
   CHAR      szParent[MAX_DESCRIPTION];           // Parent Name
   CHAR      szParentEMail[MAX_DESCRIPTION];      // Parent Name e-mail
   CHAR      szExpOriginator[MAX_DESCRIPTION];    // Originator defined during export
   CHAR      szExpEMail[MAX_DESCRIPTION];         // Originator Name e-mail
   CHAR      szVendor[MAX_DESCRIPTION];           // Vendor Name (Translator)
   CHAR      szVendorEMail[MAX_DESCRIPTION];      // Vendor e-mail
   CHAR      szProduct[MAX_DESCRIPTION];          // Product Name
   CHAR      szProductFamily[MAX_DESCRIPTION];    // Product Family
   CHAR      szSimilarProduct[MAX_DESCRIPTION];   // Similar Product Family
   CHAR      szSubjectDict[MAX_DESCRIPTION];      // Product subject area dictionary
   CHAR      szSubjectMem[MAX_DESCRIPTION];       // Product subject area memory
   CHAR      szPrevVersion[MAX_DESCRIPTION];      // previous version of the product
   CHAR      szVersion[MAX_DESCRIPTION];          // version of the product
   CHAR      szShipment[MAX_DESCRIPTION];         // shipment string
   CHAR      szLogShipment[MAX_DESCRIPTION];      // last shipment string in history log
  } FOLDERINFO, *PFOLDERINFO;


  // translation unit data area
  typedef struct _TRANSLUNIT
  {
    ULONG    ulNum;                              // number of this translation unit
    CHAR_W   szSource[MAX_SEGMENT_SIZE];         // buffer for source text
    CHAR_W   szTarget[MAX_SEGMENT_SIZE];         // buffer for target text
    PMD_METADATA pMetaData;                      // metadata of this translation unit
    BOOL     fPlaceHoldersOnly;                  // TRUE = target contains placeholders only
    BOOL     fApproved;                          // TRUE = translation unit has been approved
    CHAR_W   szState[50];                        // state of translation unit target
    PMD_NOTELIST_DATA pNoteList;                 // ptr to current notelist
    LONG     lSourceWords;                       // number of source words
    USHORT   qStatus;                            // status of segment  (XLATED, TOBE, etc..)
    LONG     lDate;                              // update date/time
  } TRANSLUNIT, *PTRANSLUNIT;

  // proposal data area
  typedef struct _PROPOSAL
  {
    SHORT    sQuality;                           // quality of this proposal
    CHAR_W   szSource[MAX_SEGMENT_SIZE];         // buffer for source text
    CHAR_W   szTarget[MAX_SEGMENT_SIZE];         // buffer for target text
    CHAR_W   szDocName[MAX_LONGFILESPEC];        // buffer for proposal document
    CHAR_W   szAuthor[40];                       // modifier of proposal
    LONG     lCreationDate;                      // creation date
    CHAR_W   szModifier[40];                     // modifier of proposal
    LONG     lModificationDate;                  // modification date
  } PROPOSAL, *PPROPOSAL;


  // tagging of current unit
  CTagList *m_pTagging;

  // instance of our XLIFF utilities
  CXLIFFUtil *m_Util;

  // buffers
  #define DATABUFFERSIZE 4098

  typedef struct _BUFFERAREAS
  {
    CHAR_W   szData[DATABUFFERSIZE];  // buffer for collected data
    CHAR_W   szTagging[DATABUFFERSIZE];  // buffer for tagging
    CHAR_W   szProp[DATABUFFERSIZE];  // buffer for collected prop values
    CHAR     szLang[50];              // buffer for language
    CHAR     szDocument[EQF_DOCNAMELEN];// buffer for document name
    MEMEXPIMPSEG SegmentData;         // buffer for segment data
    CHAR     szDescription[1024];     // buffer for memory descripion
    CHAR     szMemSourceLang[50];     // buffer for memory source language
    TRANSLUNIT TranslUnit;            // buffer for current translation unit
    PROPOSAL  Proposal;               // buffer for current proposal 
    FILEINFO  FileInfo;               // buffer for file info 
    FOLDERINFO  FolderInfo;           // buffer for folder info 
    BYTE      PropMetaData[16000];    // buffer to prepare proposal metadata
    CHAR_W    szNoteAuthor[40];       // buffer for note author
    CHAR      szErrorMessage[1024];   // buffer for error message text
    CHAR      szSegTargetFile[MAX_EQF_PATH];   // buffer for segmented target file
    CHAR      szCurMemory[MAX_LONGFILESPEC]; // name of current memory
    CHAR      szCurMemPath[MAX_LONGFILESPEC]; // path name of current memory
    OtmMemory *pMem;                    // handle of current memory 
  } BUFFERAREAS, *PBUFFERAREAS;

  PBUFFERAREAS pBuf; 

  int iCurTuv;                        // current TUV index
  int iTuvArraySize;                  // current size of TUV array

  // element stack
  int iStackSize;
  int iCurElement;
  XLIFFELEMENT CurElement;
  PXLIFFELEMENT pStack;

  bool            fSawErrors;
  BOOL fError;                         // TRUE = parsing ended with error

  // file handles
  FILE        *hfLog;                            // log file handle
  FILE        *hfSource;                         // source file handle
  FILE        *hfSegSource;                      // segmented source file handle
  FILE        *hfSegTarget;                      // segmented target file handle
  FILE        *hfMetaData;                       // metadata file handle
  CXmlWriter  *pxw;                              // ptr to active CXmlWriter object

  PTBDOCUMENT  pTargetDoc;                       // loaded target document
  PTBDOCUMENT  pSourceDoc;                       // loaded source document
  ULONG        ulSegNum;                         // current segment number
  LONG         lTranslatedSegs;                  // number of translated segments
  LONG         lTotalSegs;                       // total number of translatable segments 
  CHAR_W       szSourceFile[MAX_LONGFILESPEC];   // name of source file (w/o path info)

  BOOL         fFolderCreated;                   // TRUE = folder has been crreated
  BOOL         fDocumentCreated;                 // TRUE = document has been crreated
  BOOL         m_fDocChecked;                    // TRUE = document has ben checked and prepared
  BOOL         m_fSkipDoc;                       // TRUE = document does not exist and has to be skipped

  PFOLIMPEXPXLIFF pImpExpData;                   // pointer to impor/expor data area
};

#endif
