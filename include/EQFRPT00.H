/*! EQFRPT00.H
	Description: Private include file for Counting Report
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
*/

// defines
#define MAX_REPORTS                         6  // #reports
#define MAX_OPTIONS                        13  // #options (kind of report)
#define MAX_OPTIONS1                        4  // #options
#define MAX_OPTIONS2                        2  // #options
#define MAX_OPTIONS3                        3  // #options
#define MAX_OPTIONS4                       2   // #options for statistics
#define MAX_OPTIONS5                       2   // #options for statistics
#define MAX2_OPTIONS1                      10  // #options
#define MAX2_OPTIONS2                       3  // #options
#define MAX2_OPTIONS3                       3  // #options
#define MAX_FORMATS                         3  // #formats
#define MAX_REPORT_OPTIONS                  6  // #options pro report
#define RPT_HISTLOG_DIRECTORY  "\\PROPERTY\\"  // directory of histlogrecord
#define RPT_OUTPUT_EXTENSION            "RPT"  // output file extension    ASCII
#define RTF_OUTPUT_EXTENSION            "RTF"  // output file extension    RTF
#define MAX_OBJECTS                        10  // max number of selected documents
#define MAX_OUTPUT                       4096  // max size of output Strings  (4096)
#define INIT_SIZE_A                        10  // init and increase value of PALLINFO array
#define INIT_SIZE_C                         5  // init and increase value of PCALCINFO array
#define MAX_O_LINES                        50  // max output lines
//#define MAX_O_LENGTH                     78  // max output line length
#define MAX_O_LENGTH                       1000 // max output line length (400)
#define MAX_MRI_LINES                      83   // 25.01.02; 81 //79  // max mris
#define MAX_MRI_LENGTH                     100 // max mri length  old value :32/50
#define REPORT_2_OFFSET                     4  // depends on number of options
#define REPORT_3_OFFSET                     6  // depends on number of options

#define FUZZY_THRESHOLD_0                  0.5 // Theshold for different fuzzy classes
#define FUZZY_THRESHOLD_1                  0.7 // Theshold for different fuzzy classes
#define FUZZY_THRESHOLD_2                  0.9 // Theshold for different fuzzy classes

#define Standard_Words_per_Line            10. // Standard words per line
#define Standard_Words_per_Page            250.// Standard words per page


// Length of an entry field
//*************************

# define ROW_FIELD_LENGTH 7


// TABLE TO RTF definitions
//*************************

# define COLUMN_DELIMITER_STR "|"

// numerical output
#define STR_RPT3_NA              "-"
#define STR_RPT3_NULL            "0"


// Header Delimiter
#define STR_RPT3_HEADER          "="
#define STR_RPT3_LINE            "-"


# define RTF_Line_Begin   "{\\f2\\fs16"
# define RTF_Line_End     "}\\line\n"
# define RTF_Document_End "\\par \\pard\\plain }\\par}"
# define RTF_Page_Break   "\\par \\pard\\plain \\par \\page \\pard\\plain \\s1\\fs12\\cf0 {\\f2\\fs12\\cf0 }\\pard\\plain \\s1\\cf0 {\\f2\\fs12\\cf0 }\n"
# define RTF_Header_1     "{\\rtf1\\pc \\deff0{\\fonttbl{\\f0\\fs12\\fswiss Times New Roman;}\
{\\f1\\fswiss Helvetica;}{\\f2\\fnil Courier;}}{\\colortbl \\red0\\green0\\blue0;}{\\stylesheet{\\s0\\f0\\fs12\\cf0\\sl0 Default Text;}\
{\\s1\\cf0 Body Text;}{\\s2\\cf0 Footer;}{\\s3\\cf0 Header;}{\\s4\\f1 \\fs36\\cf0\\b\\qc\\sb144\\sa72\\keep\\keepn Title;}\
{\\s5\\cf0\\b\\i\\caps\\sb72\\sa72 Subhead;}{\\s6\\cf0 \\li720 Number List;}{\\s7\\cf0 \\li576 Bullet 1;}\
{\\s8\\cf0 \\li288 Bullet;}{\\s9\\cf0 Body Single;}}\\ftnbj \\landscape \\paperw16838\\paperh11905 \\margl1440 \\margr1440 \\sectd \\pard\\plain \\s1\\cf0 \n"
# define RTF_BOLD         "{\\b"
# define RTF_UNMAKE       "}"
# define ASCII_Page_Break "\x0C\n"


// Number of columns in Report
//****************************


#define MAX_REPORT_COLUMNS                 31 // max number of cols in  23
                                              // summary counting report

#define MAX_PAYREPORT_COLUMNS              10 // max number of cols in
                                              // summary counting report for pay in Final Fact Sheet

#define CNT_REPORT_COLUMN                  17 // Column where statistics starts



// list of columns of summary counting report
//*******************************************

typedef enum
{
    // Data
    //-----

    NumberShip,            // 0  : Number of shipments
    FuzzyLevel,            // 1   : adjusted fuzzy level
    AutoSubst,             // 2  : Summary for Automatic Substitution
    AnalAutoSubst,         // 3    - Analysis Automatic Substitution                  (p)
    AnalAutoSubst2,        // 4    - Analysis Automatic Substitution after Edit       (p)
    EditAutoSubst,         // 5    - Automatic Substitution while edit                (p)
    ManualMach,            // 6  : Manual Exact Matches
    ExactExist,            // 7    - Exact matches                                    (p)
    ReplExist,             // 8    - Replace Matches                                  (p)
    FuzzyExist,            // 9  : Fuzzy matches
    FuzzyExist_1,          // 10    - Fuzzy matches 1                                  (p)
    FuzzyExist_2,          // 11   - Fuzzy matches 2                                  (p)
    FuzzyExist_3,          // 12   - Fuzzy matches 3                                  (p)
    ChangedFuzzy,          // 13 : Number of changed  Words in Fuzzy (only Source)
    MachExist,             // 14 : Machine Matches                                    (p)
    NoneExist,             // 15 : Manually Translated                                (p)
    NotXlated,             // 16 : Not Translated

    // Statistics
    //-----------

    PercentExactUsed,      // 17
    PercentReplUsed,       // 18
    PercentFuzzyUsed,      // 19
    PercentFuzzyUsed_1,    // 20
    PercentFuzzyUsed_2,    // 21
    PercentFuzzyUsed_3,    // 22
    PercentMachUsed,       // 23

    // Additional Information for Statistics
    //--------------------------------------

    ExactUsed,             // 24
    ReplUsed,              // 25
    FuzzyUsed,             // 26
    FuzzyUsed_1,           // 27
    FuzzyUsed_2,           // 28
    FuzzyUsed_3,           // 29
    MachUsed               // 30


} Count_Columns;




// list of columns for pay factors
//*******************************************

typedef enum
{
    PayAnalAutoSubst,         // 0  - Analysis Automatic Substitution                  (p)
    PayAnalAutoSubst2,        // 1  - Analysis Automatic Substitution after Edit       (p)
    PayEditAutoSubst,         // 2  - Automatic Substitution while edit                (p)
    PayExactExist,            // 3  - Exact matches                                    (p)
    PayReplExist,             // 4  - Replace Matches                                  (p)
    PayFuzzyExist_1,          // 5  - Fuzzy matches 1                                  (p)
    PayFuzzyExist_2,          // 6  - Fuzzy matches 2                                  (p)
    PayFuzzyExist_3,          // 7  - Fuzzy matches 3                                  (p)
    PayMachExist,             // 8  - Machine Matches                                  (p)
    PayNoneExist              // 9  - Manually Translated                              (p)
} PayCount_Columns;


typedef enum
{
   Pay_Factor, Complexity_Factor
} Report_Pay_Factors;


typedef enum
{
   simple, medium, complex
} Report_Category;


typedef enum
{
    Standard, Standard_and_Group_Summary, Shrinked_in_Groups
} Report_Mode ;


typedef enum
{
    Details,
    No_Details
} Row_Mode;


typedef enum
{
    Alphabetically,
    Optimized
} Order_Mode;

typedef enum
{
      Source, Target,Segments, Modified
} Target_Mode;


typedef enum
{
      Used, Exists
} Count_Mode;



typedef enum
{
      TABLE,SUMMARY_TABLE,FACT_SHEET
} Table_Type;


typedef enum
{
      RPT_UNITY, RPT_LINE, RPT_PAGE
} Standard_Pay_Type;


typedef enum
{
      STANDARD_STAT, ADVANCED_STAT
} Statistics_Type;


// Structure for HTML output
// *************************

typedef struct _RPT_HEADER
{
    char szText[20];      // Header text infomation
    int  iColSpan;        // html colspan information

} RPT_HEADER, *PRPT_HEADER;


// Structure for one Row
//**********************

typedef char ONE_ID[256];

typedef struct _CntRow
{
     float            iRow[4][MAX_REPORT_COLUMNS];    // row 1..3 details, row 4 summary
     struct _CntRow*  PNext;                          // next shipment
     ONE_ID           szID[4];                        // Comment/Desciption of the row
     BOOL             fHistError;
     BOOL             fHistInconsistency;
     LONG             lRunningNo;                     // No for document id.
     CHAR             szShipment[MAX_DESCRIPTION];  // shipment string

}CntRow, *PCntRow;


// SUMMARY COUNTING DATA STRUCTURE
// for one file with different shipments
//**************************************

typedef struct _SumCntFile
{
    CHAR        szFolder[256];     // foldername
    CHAR        szName[256];       // filename
    CHAR        szLongName[256];   // long filename
    Table_Type  iTable;            // Type of the Table
    BOOL        fHistError;        // Hist Log Error
    BOOL        fHistInconsistency;// Hist Log Inconsistency
    LONG        lRunningNo;        // No for document id.
    LONG        docFuzzyLevel[3];  // adjusted category fuzzy level for analysis


    // pointer to array with counting information
    PCntRow PRow;

    // pointer to summary information (only one row)
    // for all shipments of the file
    PCntRow PSum;

    // pointer to next file
    struct _SumCntFile* PNext;

}SumCntFile, *PSumCntFile;





// type of reports
typedef enum
{
  HISTORY_REPORT,              // History Report
  CALCULATION_REPORT,          // Calculation Report
  SUMMARY_COUNTING_REPORT,     // Summary Calculating Report
  PRE_ANALYSIS_REPORT,         // Source/Source Source/NLV Analysis
  REDUNDANCY_REPORT,           // REDUNDANCY checking
  COMBINED_REPORT              // REDUNDANCY + Memory Match
} RPTTYPE;

// type of option
typedef enum
{
  BRIEF_SORT_BY_DATE,
  BRIEF_SORT_BY_DOCUMENT,
  DETAIL,
  VERSION
} OPTION_REPORT_1;

typedef enum
{
  WITH_TOTALS,
  WITHOUT_TOTALS
} OPTION_REPORT_2;

typedef enum
{
  BASE_LIST,
  DETAILED_LIST
} OPTION_REDUNDANT_SEGMENT_LIST;


typedef enum
{
  PLAIN_REPORT,
  REPORT_TOTALS,
  REPORT_TOTALS_FINAL_FACT_SHEET,
  ONLY_TOTALS_FINAL_FACT_SHEET,
  ONLY_FINAL_FACT_SHEET,
  ONLY_SUMMARY
} OPTION_REPORT_3;


typedef enum
{
  ASCII,
  RTF,
  HTML,
  TABBED,                 // database format
  XMLFILEFORMAT,
  UTF16_FILEFORMAT
} OPTION_FILE_FORMAT;



// type of report status
typedef enum
{
  RPT_READY,   // report ready
  RPT_ACTIVE,  // report active
  RPT_KILL     // kill report
} RPTSTATUS;


// type of output file actions
typedef enum
{
  OPEN_OUTPUT_FILE = 1,   // open output file
  WRITE_OUTPUT_FILE,      // write to output file
  CLOSE_OUTPUT_FILE,      // close output file
  ERROR_OUTPUT_FILE       // an error occured
} FILEACTION;

// structure of output strings
typedef struct _output
{
  USHORT usNum;
  CHAR   szFeld[MAX_O_LINES+1][MAX_O_LENGTH];
} OUTPUT, *POUTPUT;


// structure of output mris
typedef struct _outmri
{
  CHAR   szFeld[MAX_MRI_LINES][MAX_MRI_LENGTH];
} OUTMRI, *POUTMRI;

// union of variable parts of histlog record
typedef union _varpart
{
  DOCIMPORTHIST  DocImport;
  DOCIMPORTHIST2 DocImport2;
  ANALYSISHIST   Analysis;
  DOCSAVEHIST    DocSave;
  DOCSAVEHIST2   DocSave2;
  DOCSAVEHIST3   DocSave3;
  DOCEXPORTHIST  DocExport;
  DOCPROPHIST    DocProp;
  FOLPROPHIST    FolProp;
  FOLPROPHIST2   FolProp2;
  FOLPROPHISTSHIPMENT   FolPropShipment; //SHIPMENT_HANDLER
  VERSIONHIST    VersionHist;
  BYTE           FillArea[10000];  
} VARPART, *PVARPART;


// structure of time stamped and variable parts
typedef struct _allInfo
{
  ULONG         ulRecord;       // number of record
  HISTLOGRECORD histLogRecord;  // HISTLOGRECORD
  CHAR          szLongName[MAX_LONGFILESPEC]; // document long name
  VARPART       variablePart;   // variable part
} ALLINFO, *PALLINFO, **PPALLINFO;


// extended structure of words per class
#pragma pack(1)
typedef struct _sumPerClassEx
{
  ULONG  ulNumSegs;   // number of segments in this class
  ULONG  ulSrcWords;  // sum of all source words
  ULONG  ulModWords;  // sum of all modified words
  ULONG  ulTgtWords;  // sum of all target words
} SUMPERCLASSEX, *PSUMPERCLASSEX;

// extended structure of criteria
#pragma pack(1)
typedef struct _criteriaSumEx
{
   SUMPERCLASSEX  SimpleSum;     // number of segs in this class
   SUMPERCLASSEX  MediumSum;     // number of segs in this class
   SUMPERCLASSEX  ComplexSum;    // number of segs in this class
}  CRITERIASUMEX, *PCRITERIASUMEX;

// extended structure of docsavehist
////////////////////////////////////

#pragma pack(1)
typedef struct _docSaveHistEx
{
   CRITERIASUMEX  AnalAutoSubst;   // sums for segs transl. by analauto before edit
   CRITERIASUMEX  AnalAutoSubst2;  // sums for segs transl. by analauto after edit
   CRITERIASUMEX  EditAutoSubst;   // sums for segs transl. by editauto
   CRITERIASUMEX  ExactExist;      // sums for segs with exact prop
   CRITERIASUMEX  ExactUsed;       // sums for segs with exact prop
   CRITERIASUMEX  ReplExist;       // sums for segs with repl.prop
   CRITERIASUMEX  ReplUsed;        // sums for segs with repl.prop
   CRITERIASUMEX  FuzzyExist;      // sums for segs with fuzzy prop
   CRITERIASUMEX  FuzzyUsed;       // sums for segs with fuzzy prop
   CRITERIASUMEX  FuzzyExist_1;    // sums for segs with fuzzy prop
   CRITERIASUMEX  FuzzyUsed_1;     // sums for segs with fuzzy prop
   CRITERIASUMEX  FuzzyExist_2;    // sums for segs with fuzzy prop
   CRITERIASUMEX  FuzzyUsed_2;     // sums for segs with fuzzy prop
   CRITERIASUMEX  FuzzyExist_3;    // sums for segs with fuzzy prop
   CRITERIASUMEX  FuzzyUsed_3;     // sums for segs with fuzzy prop
   CRITERIASUMEX  MachExist;       // sums for segs with machine prop.
   CRITERIASUMEX  MachUsed;        // sums for segs with machine prop.
   CRITERIASUMEX  NoneExist;       // sums for segs with no proposal  (based on exists)
   CRITERIASUMEX  NoneExist2;      // sums for segs with no proposal  (based on used)
   CRITERIASUMEX  NotXlated;       // sums for TOBE, ATTR, CURRENT
}  DOCSAVEHISTEX, *PDOCSAVEHISTEX;


// structure of calculation info
typedef struct _calcInfo
{
  CHAR           szDocument[MAX_FILESPEC];  // document name
  CHAR           szLongName[MAX_LONGFILESPEC];  // document long name
  CHAR           szShipment[MAX_DESCRIPTION];  // shipment string
  int            Task;                      // ID of task
  LONG           lTime;                     // date and time of last action
  LONG           docFuzzyLevel[3];          // adjusted category fuzzy level for analysis
  DOCSAVEHISTEX  docSaveHistSum;            // DOCSAVEHISTEX summary of document
  CRITERIASUMEX  docCriteriaSum;            // CRITERIASUMEX summary of document
} CALCINFO, *PCALCINFO, **PPCALCINFO;


// structure of report
typedef struct _rpt
{
  OBJNAME       szFolderObjName;            // foldername + path
  CHAR          szFolderName[MAX_FILESPEC]; // foldername with extension
  CHAR          szFolder[MAX_FILESPEC];     // foldername
  OBJNAME       szRptInstanceObjName;       // report instance object name
  CHAR          szHistLogFile[MAX_PATH144]; // history log file + path


  BOOL          fBatch;                     // Batch job from eqfcmd

  USHORT        usFillMode;                 // folder or selected documents
  CHAR          szOptions[MAX_OPTIONS][40]; // report options
  PSZ           pReportOptions[MAX_REPORT_OPTIONS]; // pointerarray to options

  USHORT        usRptStatus;                // report status
  CHAR          szWorkString[MAX_O_LENGTH]; // string buffer
  HWND          hwndRptHandlerLB;           // handle of invisible LB containing selected documents
  HWND          hwndErrMsg;                 // handle of invisible LB containing selected documents

  USHORT        usReport;                   // selected report
  USHORT        usFormat;                   // selected format
  USHORT        usOptions;                  // last selected option
  USHORT        usOption[6];                // selected option with history
  USHORT        usOption1;                  // selected Options no 1 for summary counting report
  USHORT        usOption2;                  // selected Options no 2 for summary counting report
  USHORT        usOption3;                  // selected Options no 3 for summary counting report
  USHORT        usOption4;                  // selected Options no 4 for summary counting report
  USHORT        usOption5;                  // selected Options no 5 for summary counting report
  USHORT        usOption21;                 // selected Options no 1 for final fact sheet
  USHORT        usOption22;                 // selected Options no 2 for final fact sheet
  USHORT        usColumns[4];               // check boxes for summary counting report
  USHORT        usColumns4[5];               // check boxes for summary counting report
  USHORT        usShipmentChk;
  USHORT        usCurrency;                 // check boxes for summary counting report
  CHAR          szCurrency[5];                 // check boxes for summary counting report

  CHAR          szShipmentChk[MAX_DESCRIPTION];         // shipment string

  float         Complexity_Factor[MAX_PAYREPORT_COLUMNS][3];    // factor for final fact sheet
  float         Pay_Factor[MAX_PAYREPORT_COLUMNS][3];           // factor for final fact sheet
  float         Pay_per_Standard;           // Pay factor per standard page rep line
  USHORT        usStandard;                 // Standard page or Standard Line

  CHAR          szReport[40];               // selected report
  CHAR          szFormat[40];               // selected format
  CHAR          szProfile[40];              // selected format
  CHAR          szOption[40];               // selected option
  CHAR          szOption1[40];              // selected option no 1 for summary counting report
  CHAR          szOption2[40];              // selected option no 2 for summary counting report
  CHAR          szOption3[40];              // selected option no 3 for summary counting report
  CHAR          szOption4[40];              // selected option no 4 for summary counting report
  CHAR          szOption21[40];             // selected option no 1 for final fact sheet
  CHAR          szOption22[40];             // selected option no 2 for final fact sheet

  BOOL          fRptFile;                   // TRUE = output to file
  CHAR          szRptOutputFile[MAX_PATH144+MAX_FILESPEC];
                                            // output filename + path
  CHAR          szRptDescription[MAX_DESCRIPTION];
                                            // description field
  USHORT        usOutputFileProcess;        // process for output file
  HFILE         hfOutputFileHandle;         // output file handle
  BOOL          fRptLB;                     // TRUE = output to listbox
  HWND          hwndLB;                     // handle of listbox for listbox output
  CHAR          szRptDriveLetter[MAX_DRIVE];  // drives

  BOOL          fFolderSelected;            // folder selected or not
  USHORT        usAllDocuments;             // number of all docs in folder
  USHORT        usSelectedDocuments;        // number of selected docs in folder

  PPALLINFO     ppAllInfoField;             // pointer to array of PALLINFOs
  ULONG         ulAllInfoRecords;           // ALLINFO records

  PPCALCINFO    ppCalcInfoField;            // pointer to array of PCALCINFOs
  ULONG         ulCalcInfoRecords;          // CALCINFO records
  PCALCINFO     pCalcInfoSum;               // pointer to CALCINFO for summary

  PSZ           pszActualDocument;          // non-dde buffer for filelist
  CHAR          szShipment[MAX_DESCRIPTION];         // shipment string
  CHAR          szLongActualDocument[MAX_LONGFILESPEC]; // actual document in
  CHAR          szFileName[MAX_FILESPEC];   // active File
  OBJNAME       szFileObjName;              // filename + path
  ULONG         ulActualRecord;             // index of actual record in process

  POUTMRI       pOutputMris;                // pointer to OUTMRI
  POUTPUT       pOutputField;               // pointer to OUTPUT
  USHORT        usActualLine;               // index of actual output line
  USHORT        usStringIndex;              // current string index
  HFILE         hHTMLControl;               // html control
  CHAR          chHTMLControl[MAX_EQF_PATH]; // history log file + path
  PDDECNTRPT    pDDECntRpt;                    // Ida
  BOOL          fErrorPosted;               // ERROR Message posted
  CHAR          szLongFolderName[MAX_LONGFILESPEC]; // long folder name
  //HWND          hwndShipmentLB;         // listbox containing translator list
  CHAR          aszShipments[100][MAX_DESCRIPTION]; // array of shipments
  int           iShipmentsUsed;        // number of used entries in aszShipment array
  OBJNAME       szParentObjName;            // parent (sub)folder object name
  LONG          lSmallFuzzLevel;       // fuzzines level for small segments
  LONG          lMediumFuzzLevel;      // fuzzines level for medium sized segments
  LONG          lLargeFuzzLevel;       // fuzzines level for large segments
  BOOL          fProtectedProfile;     // TRUE = current profile is a protected one
  BOOL          fInitialLoad;          // TRUE = this is the initial load of the active profile
  BOOL          fKill;                 // TRUE = terminate processing asap
  USHORT        usProcessedDocuments;  // number of actally processed docs
  CHAR          szIntProfileName[40];  // internal profile name (if any)
  CHAR_W        szWorkStringW[MAX_O_LENGTH]; // string buffer (UTF16)
  CHAR          szLongNameBuffer[MAX_LONGFILESPEC];
  CALCREPORTBINDATA CalcRepBinData;    // area to collect calulation report data for our binary file
  CHAR          szDisplayProfileName[42]; // profile name as displayed in the report
  CHAR          szTargetLang[MAX_LANG_LENGTH]; // folder target language
} RPT, *PRPT;


// structure of report ida
typedef struct _rptida
{
  PRPT         pRpt;                   // pointer to RPT structure
  HPROP        hpropFolder;            // handle of folder properties
  PPROPFOLDER  ppropFolder;            // pointer to folder properties
  CHAR         szFolderPropName[MAX_PATH144+MAX_FILESPEC];
  CHAR         szDrives[MAX_DRIVELIST];// list of avaliable drives
  CHAR         szSysPath[MAX_EQF_PATH];// MAT system path
  CHAR         szWorkString[1000];     // string buffer
  CHAR         szDocObjName[MAX_EQF_PATH]; // document object name
  CHAR         szLongName[MAX_LONGFILESPEC];// buffer for document long name
  HWND         hwndTabCtrl;            // handle of tab control
  BOOL         fLeadingWS;             // setting of leading WS handling flag
  BOOL         fTrailingWS;            // setting of leading WS handling flag
  HWND         hwndPages[20];          // array with property pages
  HWND         hwndDocLB;              // handle of document listbox in general page

} RPTIDA, *PRPTIDA;




// structure of redundancy information
typedef struct _rpt_redundancy
{
  CHAR      szShortName[MAX_FILESPEC];           // short filename
  COUNTSUMS DocuRed;                             // Inner Document Redundancies
  COUNTSUMS OtherRed;                                      // Outer Document Redundancies
  LONG      OtherRedWords;
  BOOL      fUsed;                               // document namose flag
} RPT_REDUND, *PRPT_REDUND;



// set list window title
static CHAR szListWindowTitle[80];

// set view
static SHORT sRptView[] = {0, CLBLISTEND};


// set CLBCOLDATA
static CLBCOLDATA RptCLBCOLData[] = {
{ szListWindowTitle, MAX_O_LENGTH, TEXT_DATA, DT_LEFT},  // column title text, width, type, format flag
{ NULL,       0,                    TEXT_DATA,              0}};



// set CLBCTLDATA
static CLBCTLDATA RptCLBCTLData =
{
  sizeof(CLBCTLDATA),        // size of structure
  1,                         // 2 listbox column
  1,                         // distance between columns
  SYSCLR_WINDOWSTATICTEXT,   // color of title text
  SYSCLR_WINDOW,             // color of title text background
  SYSCLR_WINDOWTEXT,         // color of listbox items
  SYSCLR_WINDOW,             // color of listbox items background
  X15,                       // data seperator
  sRptView,                  // last used view list
  sRptView,                  // default view list for details
  sRptView,                  // default view list set by user
  sRptView,                  // view list for name view option
  sRptView,                  // list of sort criteria columns
  RptCLBCOLData              // pointer to column table
};


// prototypes for eqfrpt01.c
BOOL RptMain                   (BOOL, HWND, PRPT);  // main function of building counting report
BOOL RptGetRecords             (BOOL, PRPT);        // get histlog records of selected documents
BOOL RptSortRecords            (PRPT);        // sort histlog records in document order
BOOL RptLoadOutputMris         (PRPT);        // load output mris from resource dll
BOOL RptMakeCalculationRecords (PRPT);        // make calculating records
BOOL RptBuildOutputString      (BOOL, HWND, PRPT);  // build output string
BOOL RptOutputToFileWork       (PRPT);        // output to file
BOOL RptOutputToRTFFile        (PRPT);        // output to file
BOOL RptOutputToFile           (PRPT);        // output to file
BOOL RptOutputToWindow         (HWND, PRPT);  // output to list window
VOID RptFreeMemory             (PRPT);        // free allocated memory used by counting report
BOOL ShortenLongName           (int, PSZ);    // shorten long file names
BOOL RptReport1                (HWND, PRPT);  // counting report 1
BOOL RptReport2                (HWND, PRPT);  // counting report 2
BOOL RptReport3                (HWND, PRPT);  // counting report 3
BOOL RptReport4                (HWND, PRPT);  // counting report 4

BOOL RptSumOut
(
  PRPT        pRpt,                    // pointer to report instance data
  PCRITERIASUM pSum,                   // pointer to counts
  PSZ         pszFirstLine,            // string to be used in first line
  PSZ         pszSecondLine,           // string to be used for second line
  BOOL        fIfData                  // display sums only if data available
);

BOOL RptSumOut2
(
  PRPT        pRpt,                    // pointer to report instance data
  PCRITERIASUMEX pSum,                 // pointer to counts
  PSZ         pszFirstLine,            // string to be used in first line
  PSZ         pszSecondLine            // string to be used for second line
);


BOOL RPTPrepareMemoryMatchCount(BOOL, PRPT);

BOOL RptOutputToHTMLFile (PRPT pRpt, HFILE hFile, BOOL fView );
__declspec(dllexport)
BOOL RptOutputToHTMLControlOpen( PRPT pRPT );
__declspec(dllexport)
VOID RptOutputToHTMLControlClose( PRPT pRPT );
VOID RptTestApi();

#define RPT_PROTECTSTAMP "eP1_TZ82X"
BOOL RPTIsProtectedProfile( PSZ pszProfile  );
BOOL RptCheckProfileStamp( PPROPFOLDER pProp );
LONG RptCalcProfileCheckSum( PPROPFOLDER pProp );







