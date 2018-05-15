//+----------------------------------------------------------------------------+
//| EQFRPXML.CPP                                                            |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Create calcualtion reports in XML format                      |
//+----------------------------------------------------------------------------+

#ifndef CPPTEST
extern "C"
{
#endif
  #define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_TP               // public translation processor functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_ASD              // dictionary access functions (Asd...)
  #define INCL_EQF_EDITORAPI        // editor API
  #define INCL_EQF_FOLDER           // folder list and document list functions

#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF
  #include "eqfhlog.h"                   // defines and structures of history log
  #include "eqfdde.h"
  #include "eqfrpt00.h"   // private include file for report
  #include "eqfrpt01.h"   // private include file for report
  #include "eqfrpt.h"
  #include "eqfserno.h"
  #include "eqfrpt.mri"   // private mri's for report
  #include <time.h>
#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif

#include "eqfxsl.h"

//#define _DLL


// if IGNOREINTERNALIMPORTS is defined, internal document imports are ignored
#define IGNOREINTERNALIMPORTS

// if IGNOREIMPORTSAVERECORDS is defined IMPORT save records are ignored 
#define IGNOREIMPORTSAVERECORDS 

// if XSLTCONVERSIONLOG is defined, some logging is performed to log file \EQF\LOGS\XSLTCONV.LOG
//#define XSLTCONVERSIONLOG

#include "cxmlwriter.h"
#include "core\utilities\LanguageFactory.h"

//// includes for Xalan XSLT
//
//// the Win32 Xerces/Xalan build requires the default structure packing...
//#pragma pack( push )
//#pragma pack(8)
//
//#include <iostream>
//#include <xercesc/util/PlatformUtils.hpp>
//#include <xercesc/util/PlatformUtils.hpp>
//#include <xalanc/XalanTransformer/XalanTransformer.hpp>
//
//#pragma pack( pop )


typedef struct _DOCNAME
{
  char szName[MAX_LONGFILESPEC];
  char szShortName[MAX_FILESPEC];
  BOOL fUsed;
} DOCNAME, *PDOCNAME;


// variables, defines, and prototypes for logging for logging
#define RPTLOGFILE    "CALCRPT.LOG"
#define RPTLOGTRIGGER "RPTLOGIT.DAT"
FILE *hRptLog2 = NULLHANDLE;
BOOL fRptLogging2 = FALSE;

FILE *RptOpenLogFile( PSZ pszLogFile, PSZ pszLogName );
void RptLogStart2();
void RptLogEnd2();
void RptLogString2( PSZ pszString );
void RptLog2String2( PSZ pszString1, PSZ pszString2 );
void RptLogBuildSum2( PSZ pszBuffer, PSZ pszCol, PCRITERIASUM pSum );
void RptLogBuildSumEx2( PSZ pszBuffer, PSZ pszCol, PCRITERIASUMEX pSum );
void myRptLogDocSave( PSZ pszString, PDOCSAVEHIST pDocSave );
void myRptLogDocSave3( PSZ pszString, PDOCSAVEHIST3 pDocSave3 );
void RptLogCalcInfo2( PSZ pszString, PCALCINFO pCalcInfo );
void RptLogDocPropSums2( PSZ pszString, PPROPDOCUMENT pProp );
void RptLogBuildCountSum2( PSZ pszBuffer, PSZ pszCol, PCOUNTSUMS pSum );
void RptLogUShort( PSZ pszString, USHORT usValue );
void RptLogULong( PSZ pszString, ULONG ulValue );
BOOL RptGetDocLongName( PRPT pRpt, PSZ pszShortName, PSZ pszLongName );
BOOL RptCheckDocNames( PRPT pRpt, PRPT_REDUND pRPT_REDUND_START, PDOCNAME pDocNames, int iNumberOfDocuments );
VOID RptFreeMemoryX( PRPT pRpt );
BOOL RptMakeCalculationRecordsX( PRPT pRpt );
BOOL RPTPrepareMemoryMatchCountX(BOOL bDde, PRPT pRpt);
void HistLogCorrectRecSizesX( PHISTLOGRECORD pRecord );

// date / time formats (normally located in UtiVars)

typedef enum _DATEDISPLAYFORMAT
{
   MDY_DATE_FORMAT,                    // date format MM DD YY
   DMY_DATE_FORMAT,                    // date format DD MM YY
   YMD_DATE_FORMAT                     // date format YY MM DD
} DATEDISPLAYFORMAT;

// time display formats (must be in same order as OS2 setup formats)
typedef enum _TIMEDISPLAYFORMAT        // time display formats
{
   S_12_HOURS_TIME_FORMAT,             // time 0..11 format
   S_24_HOURS_TIME_FORMAT              // time 0..23 format
} TIMEDISPLAYFORMAT;

// type of report
typedef enum { HIST_REPTYPE, CALC_REPTYPE, REDUNDANCY_REPTYPE, REDSEGLIST_REPTYPE } CALCREPORTTYPE;


// histlog task ID to task name table
struct 
{
  LOGTASK TaskID;
  CHAR    szName[40];
} TaskIDtoName[] = {
{ DOCIMPORT_LOGTASK,          "Document import" },
{ DOCIMPNEWTARGET_LOGTASK,    "Document import / new target" },
{ ANALYSIS_LOGTASK,           "Analysis" },
{ AUTOMATICSUBST_LOGTASK,     "Automatic substitution" },
{ DOCDELETE_LOGTASK,          "Delete of document" },
{ DOCSAVE_LOGTASK,            "Save of document" },
{ DOCEXPORT_LOGTASK,          "Document export" },
{ DOCPROP_LOGTASK,            "Document properties changed" },
{ FOLPROP_LOGTASK,            "Folder properties changed" },
{ LONGNAME_LOGTASK,           "Document with long name imported" },
{ DOCSAVE_LOGTASK2,           "Save of document V2" },
{ DOCIMPNEWTARGET_LOGTASK2,   "Document import / new target V2" },
{ HISTDATA_INVALID_LOGTASK,   "Wordcount reset" },
{ DOCAPI_LOGTASK,             "API histlog entry" },
{ HISTDATA_INCONSISTENT_LOGTASK,  "Inconsistent histlory log" },
{ VERSION_LOGTASK,            "Version Info" },
{ FOLPROPSHIPMENT_LOGTASK,    "Folder shipment changed" },
{ DOCIMPORT_LOGTASK2,         "Document import V2" },
{ AUTOMATICSUBST_LOGTASK3,    "Automatic substitution V3" },
{ DOCSAVE_LOGTASK3,           "Save of document V3" },
{ ANALYSIS_LOGTASK3,          "Analysis of document V3" },
{ DOCAPI_LOGTASK3,            "API histlog entry V3" },
{ DOCIMPNEWTARGET_LOGTASK3,   "Document import/new target V3" },
{ HISTLOGDELETE_LOGTASK,      "Delete of history log file" },
{ HISTDATA_RESET_LOGTASK,     "Wordcount reset" },
{ FOLPROP_LOGTASK2,           "Folder properties changed" },
{ STARGET_CORRUPT_LOGTASK,    "Corrrupted data has been corrected" },
{ (LOGTASK)0,                 "" } };

#define DATEFORMATSTRING_YMD     "%4.4d%c%2.2d%c%2.2d"
#define DATEFORMATSTRING_DMY     "%2.2d%c%2.2d%c%4.4d"
#define DATEFORMATSTRING_MDY     "%2.2d%c%2.2d%c%4.4d"
#define TIMEFORMATSTRING         "%2.2d%c%2.2d%c%2.2d"

USHORT UtiVar_usDateFormat;
char   UtiVar_chDateSeperator;
USHORT UtiVar_usTimeFormat;
char   UtiVar_chTimeSeperator;
char   UtiVar_szTime1159[10];
char   UtiVar_szTime2359[10];

typedef char ONE_COMMENT[30];
ONE_COMMENT   XmlComment_2[MAX_REPORT_COLUMNS]=
{
  "Shipment", 
  "FuzzyLevel",
  "SumAnalyze",
  "AnalyzeAuto",
  "AnalyzePost",
  "AnalyzeEdit",
  "SumEdit",
  "EditExact",
  "EditReplace",
  "SumFuzzy",
  "Fuzzy5069",
  "Fuzzy7089",
  "Fuzzy9099",
  "ChangedFuzzy",
  "MachineMatches",
  "NoneMatches",
  "NotTranslated",

  // Statistics
  STR_RPT3_COLUMN2_18,
  STR_RPT3_COLUMN2_19,
  STR_RPT3_COLUMN2_20,
  STR_RPT3_COLUMN2_21,
  STR_RPT3_COLUMN2_22,
  STR_RPT3_COLUMN2_23,
  // Used for statistics
  STR_RPT3_COLUMN2_24,
  STR_RPT3_COLUMN2_25,
  STR_RPT3_COLUMN2_26,
  STR_RPT3_COLUMN2_27,
  STR_RPT3_COLUMN2_28,
  STR_RPT3_COLUMN2_29,
  STR_RPT3_COLUMN2_30,
  STR_RPT3_COLUMN2_31
};

// local currencies
typedef CHAR LOCAL_CURRENCY[5];
LOCAL_CURRENCY MyszRptCurrency2[] = {
  "ARP", "AUD", "ATS", "BSD", "BBD", "BEF", "BMD", "BRR", "BGL", "CAD", "CHF", "CLP", "CNY",
  "CYP", "CSK", "DEM", "DKK", "DZD", "EEK", "EGP", "EUR", "FJD", "FIM", "FRF", "GBP", "GRD",
  "HKD", "HUF", "ISK", "INR", "IDR", "IEP", "ILS", "ITL", "JMD", "JOD", "JPY", "LBP", "LTL",  
  "LUF", "LVL", "MYR", "MXP", "NLG", "NZD", "NOK", "PKR", "PHP", "PLZ", "PTE", "ROL", "SUR",
  "SAR", "SGD", "SKK", "ZAR", "KRW", "ESP", "SDD", "SEK", "TWD", "THB", "TTD", "TRL", "USD",
  "VEB", "ZMK", "XEC", "XEU", "XDR", "XAG", "XAU", "XPT" };

/**********************************************************************/
/* macro COPYCOUNTS                                                   */
/*                                                                    */
/* copy the values in the source structure to the target structure    */
/* if the number of segments in the source record is not zero         */
/**********************************************************************/

#define COPYCOUNTS( tgt, src ) \
    tgt.ulNumSegs = src.usNumSegs; \
    tgt.ulSrcWords = src.ulSrcWords; \
    tgt.ulModWords = src.ulModWords; \
    tgt.ulTgtWords = src.ulTgtWords; \

#define PCOPYCOUNTS( tgt, src ) \
    tgt->ulNumSegs = src.usNumSegs; \
    tgt->ulSrcWords = src.ulSrcWords; \
    tgt->ulModWords = src.ulModWords; \
    tgt->ulTgtWords = src.ulTgtWords; \


/**********************************************************************/
/* macro ADDCOUNTS                                                    */
/*                                                                    */
/* add the values in the source structure to the target structure     */
/* if the number of segments in the source record is not zero         */
/**********************************************************************/
#define ADDCOUNTS( tgt, src ) \
  if ( src.usNumSegs ) \
  { \
    tgt.ulNumSegs += src.usNumSegs; \
    tgt.ulSrcWords += src.ulSrcWords; \
    tgt.ulModWords += src.ulModWords; \
    tgt.ulTgtWords += src.ulTgtWords; \
  }

#define MERGECOUNTS( tgt, src ) \
  if ( src.ulNumSegs ) \
  { \
    tgt.ulNumSegs  += src.ulNumSegs; \
    tgt.ulSrcWords += src.ulSrcWords; \
    tgt.ulModWords += src.ulModWords; \
    tgt.ulTgtWords += src.ulTgtWords; \
  }

#define COPY2COUNTS( tgt, src ) \
    tgt.ulNumSegs  = src.ulNumSegs; \
    tgt.ulSrcWords = src.ulSrcWords; \
    tgt.ulModWords = src.ulModWords; \
    tgt.ulTgtWords = src.ulTgtWords; \


/**********************************************************************/
/* macro ADDCOUNTS2                                                   */
/*                                                                    */
/* same as macro ADDCOUNTS but for source structures with an ULONG    */
/* value for number of segments                                       */
/**********************************************************************/
#define ADDCOUNTS2( tgt, src ) \
  if ( src.ulNumSegs ) \
  { \
    tgt.ulNumSegs  += src.ulNumSegs; \
    tgt.ulSrcWords += src.ulSrcWords; \
    tgt.ulModWords += src.ulModWords; \
    tgt.ulTgtWords += src.ulTgtWords; \
  }

/**********************************************************************/
/* macro COPYALLCOUNTS                                                */
/*                                                                    */
/* Copy the three sums SimpleSum, MediumSum, and ComplexSum using the */
/* COPYCOUNTS macro                                                   */
/**********************************************************************/
#define COPYALLCOUNTS( tgt, src ) \
  { \
    COPYCOUNTS( tgt.SimpleSum, src.SimpleSum ); \
    COPYCOUNTS( tgt.MediumSum, src.MediumSum ); \
    COPYCOUNTS( tgt.ComplexSum, src.ComplexSum ); \
  }

#define PCOPYALLCOUNTS( tgt, src ) \
  { \
    COPYCOUNTS( tgt->SimpleSum, src.SimpleSum ); \
    COPYCOUNTS( tgt->MediumSum, src.MediumSum ); \
    COPYCOUNTS( tgt->ComplexSum, src.ComplexSum ); \
  }


/**********************************************************************/
/* macro ADDALLCOUNTS                                                 */
/*                                                                    */
/* Add the three sums SimpleSum, MediumSum, and ComplexSum using the  */
/* ADDCOUNTS macro                                                    */
/**********************************************************************/
#define ADDALLCOUNTS( tgt, src ) \
  { \
    ADDCOUNTS( tgt.SimpleSum, src.SimpleSum ); \
    ADDCOUNTS( tgt.MediumSum, src.MediumSum ); \
    ADDCOUNTS( tgt.ComplexSum, src.ComplexSum ); \
  }

#define MERGEALLCOUNTS( tgt, src ) \
  { \
    MERGECOUNTS( tgt.SimpleSum, src.SimpleSum ); \
    MERGECOUNTS( tgt.MediumSum, src.MediumSum ); \
    MERGECOUNTS( tgt.ComplexSum, src.ComplexSum ); \
  }

#define COPY2ALLCOUNTS( tgt, src ) \
  { \
    COPY2COUNTS( tgt.SimpleSum, src.SimpleSum ); \
    COPY2COUNTS( tgt.MediumSum, src.MediumSum ); \
    COPY2COUNTS( tgt.ComplexSum, src.ComplexSum ); \
  }



/**********************************************************************/
/* macro ADDALLCOUNTS2                                                */
/*                                                                    */
/* same as macro ADDALLCOUNTS but using ADDCOUNTS2 macro              */
/**********************************************************************/
#define ADDALLCOUNTS2( tgt, src ) \
  { \
    ADDCOUNTS2( tgt.SimpleSum, src.SimpleSum ); \
    ADDCOUNTS2( tgt.MediumSum, src.MediumSum ); \
    ADDCOUNTS2( tgt.ComplexSum, src.ComplexSum ); \
  }

// macro to subtract only if result will not be negative
#define SUBTRACTWITHCHECKING( a, b ) if ( a >= b ) a = a - b;

// disable this define to base the processing on the short names in the history log
#define LONG_NAME_IN_HISTLOG

int RptAllInfoCompare2( const void *pElement1, const void *pElement2 );

// main report functions 
BOOL RptCalcReport( PRPT pRpt, CXmlWriter *xw );
BOOL RptHistoryReport( PRPT pRpt, CXmlWriter *xw );
BOOL RptCountingReport( PRPT pRpt, CXmlWriter *xw );
BOOL RptRedundantSegmentList( PRPT pRpt, CXmlWriter *xw );

// static functions
static VOID MergeResults ( PDOCSAVEHISTEX pDoc1, PDOCSAVEHISTEX pDoc2, BOOL fmemreset );
static VOID CopyResults ( PDOCSAVEHISTEX pDoc1, PDOCSAVEHISTEX pDoc2);
BOOL RptMainProc(BOOL bDde, HWND hwnd, PRPT pRpt, CXmlWriter *xw );
void  RptCalcReportRow( PRPT pRpt, PCntRow pCntRow, Row_Mode R_Mode, int S_Mode, 
                     int (*pColumn_Switch)[MAX_REPORT_COLUMNS],  CXmlWriter *xw, Table_Type iTable_Type );
int CalcRepLoadProfile( PRPT pRpt, PSZ pszProfile, BOOL fMsg );
BOOL RptSingleCatOut( CXmlWriter *xw, PSUMPERCLASS pNumbers, PSZ pszCategory );
BOOL RptSingleCatOut2( CXmlWriter *xw, PSUMPERCLASSEX pNumbers, PSZ pszCategory );
BOOL RptSingleCountOut( CXmlWriter *xw, ULONG ulCount, PSZ pszCategory, PSZ pszType );
BOOL RptSumOut( PRPT pRpt, CXmlWriter *xw, PCRITERIASUM pSum, PSZ pszType );
BOOL RptSumOut2( PRPT pRpt, CXmlWriter *xw, PCRITERIASUMEX pSum, PSZ pszType );
void RPTTaskIdToTask( LOGTASK Task, PSZ pszBuffer  );
BOOL RptShowReport( PRPT pRpt, PSZ pszXMLFile, HWND hwnd  );
BOOL RptCreateXMLOutput( PRPT pRpt, PSZ pszOutFile );
BOOL RptMakeOutputFile( PRPT pRpt, PSZ pszXMLFile );
BOOL RptConvertXML( PRPT pRpt, PSZ pszXMLFile, PSZ pszOutFile, RPTTYPE Report, OPTION_FILE_FORMAT Format );
BOOL RptGetStyleSheetNameForReportFormat( RPTTYPE Report, OPTION_FILE_FORMAT Format, PSZ pszXSLFile );
BOOL RptCheckProfile( PRPT pRpt );
BOOL RptWriteFuzzyLevel( CXmlWriter *xw, PSZ pszType, LONG lLevel );
BOOL RptGetRecords2( BOOL bDde, PRPT pRpt, PBOOL pfCorrupted, PLONG pLastValidTime, PLONG pNextValidTime );
ULONG RptGetRecordSize( PALLINFO pAllInfo );


static VOID MergeResults
(
  PDOCSAVEHISTEX pDocSaveHistSumTmp,
  PDOCSAVEHISTEX pDocInfoAnalBase,
  BOOL           fmemreset
)
{
  if ( !pDocSaveHistSumTmp || !pDocInfoAnalBase )
  {
    return;
  } // end if

  MERGEALLCOUNTS( pDocSaveHistSumTmp->EditAutoSubst, pDocInfoAnalBase->EditAutoSubst );

  // ExactExist/Used, sum overall: segments, source, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->ExactExist,pDocInfoAnalBase->ExactExist );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->ExactUsed,pDocInfoAnalBase->ExactUsed );


  // ReplExist/Used, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->ReplExist,pDocInfoAnalBase->ReplExist );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->ReplUsed,pDocInfoAnalBase->ReplUsed );


  // FuzzyExist/Used, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist,pDocInfoAnalBase->FuzzyExist );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed,pDocInfoAnalBase->FuzzyUsed );

  // FuzzyExist/Used 1 , sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_1, pDocInfoAnalBase->FuzzyExist_1 );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_1, pDocInfoAnalBase->FuzzyUsed_1 );

  // FuzzyExist/Used 2, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_2,pDocInfoAnalBase->FuzzyExist_2 );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_2,pDocInfoAnalBase->FuzzyUsed_2 );

  // FuzzyExist/Used 3, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_3,pDocInfoAnalBase->FuzzyExist_3 );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_3,pDocInfoAnalBase->FuzzyUsed_3 );

  // MachExist/Used, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->MachExist,pDocInfoAnalBase->MachExist );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->MachUsed,pDocInfoAnalBase->MachUsed );

  // NoneExist, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->NoneExist,pDocInfoAnalBase->NoneExist );

  // NoneExist, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->NoneExist2,pDocInfoAnalBase->NoneExist2 );

  if ( fmemreset ) memset( pDocInfoAnalBase, 0, sizeof( DOCSAVEHISTEX ));
}// end of MergeResults


//+----------------------------------------------------------------------------+
//|Function name: CopyResults                                                  |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Copy Counting Report Struxcture for security check             |
//|             regarding Global Find and Replace bug destroying cnt data      |
//+----------------------------------------------------------------------------+
static VOID CopyResults
(
  PDOCSAVEHISTEX pDocSaveHistSumTmp,   // Target (into)
  PDOCSAVEHISTEX pDocInfoAnalBase    // Source (from)
)
{


  COPY2ALLCOUNTS( pDocSaveHistSumTmp->EditAutoSubst, pDocInfoAnalBase->EditAutoSubst );


  // ExactExist/Used, sum overall: segments, source, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->ExactExist,pDocInfoAnalBase->ExactExist );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->ExactUsed,pDocInfoAnalBase->ExactUsed );


  // ReplExist/Used, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->ReplExist,pDocInfoAnalBase->ReplExist );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->ReplUsed,pDocInfoAnalBase->ReplUsed );


  // FuzzyExist/Used, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist,pDocInfoAnalBase->FuzzyExist );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed,pDocInfoAnalBase->FuzzyUsed );

  // FuzzyExist/Used 1 , sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_1,pDocInfoAnalBase->FuzzyExist_1 );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_1,pDocInfoAnalBase->FuzzyUsed_1 );

  // FuzzyExist/Used 2, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_2,pDocInfoAnalBase->FuzzyExist_2 );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_2,pDocInfoAnalBase->FuzzyUsed_2 );

  // FuzzyExist/Used 3, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_3,pDocInfoAnalBase->FuzzyExist_3 );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_3,pDocInfoAnalBase->FuzzyUsed_3 );

  // MachExist/Used, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->MachExist,pDocInfoAnalBase->MachExist );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->MachUsed,pDocInfoAnalBase->MachUsed );

  // NoneExist, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->NoneExist,pDocInfoAnalBase->NoneExist );

  // NoneExist, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->NoneExist2, pDocInfoAnalBase->NoneExist2 );

}// end of CopyResults


BOOL  RPTCOPYALLCOUNTS2
(
PRPT            pRpt,      // pointer to Report structure
PCRITERIASUMEX  pLog,      // Log to be filled     TARGET
CRITERIASUM     Data,      // Input Data from Histlog  SOURCE, to be checked
BOOL            fHistError,// Error encountered
INT*            pTask,     // Task
ULONG           ulWords    // Number of Words in Source file
)
{

  CRITERIASUM    count_minus;
  BOOL fHistInconsistency = FALSE;

  fHistError; pRpt; pTask; ulWords;

  memset(&count_minus,0xFF,sizeof(count_minus));


  //if ( pRpt->usColumns4[0] )
  //{
  //  if ( RPTConsistencyCheck(Data,ulWords) )
  //  {
  //    PCOPYALLCOUNTS( pLog,
  //                    Data );
  //  }
  //  else
  //  {
  //    if ( !fHistError )
  //    {

  //      *pTask  = HISTDATA_INCONSISTENT_LOGTASK;
  //      fHistInconsistency = TRUE;
  //      PCOPYALLCOUNTS( pLog,
  //                      count_minus );
  //    }
  //  } /* endif */
  //}
  //else // no consistency check
  {
    PCOPYALLCOUNTS( pLog, Data );

  } // end if


  return(fHistInconsistency);

} // end of RPTCOPYALLCOUNTS2

BOOL  RPTCHECKALLCOUNTS2
(
PRPT            pRpt,       // pointer to Report structure
PDOCSAVEHISTEX  pLog,     // Log to be filled    TARGET
PDOCSAVEHIST2   pData,    // Input Data from Histlog  SOURCE, to be checked
BOOL            fHistError, // Error encountered
INT*            pTask,      // Task
ULONG           ulWords     // Number of Words in Source file
)
{

  //CRITERIASUM    count_minus;
  //  PCRITERIASUMEX pLogRec;
  BOOL fHistInconsistency = FALSE;

  fHistError; pData; pLog; pRpt; pTask; ulWords;

  // GQ: no consistency check anymore as wordcoounts will be corrupted

  //memset(&count_minus,0xFF,sizeof(count_minus));


  //if ( pRpt->usColumns4[0] )  // consistency check to be done
  //{
  //  if ( !RPTConsistencySumCheck(pData,ulWords) )
  //  {
  //    if ( !fHistError )
  //    {

  //      *pTask  = HISTDATA_INCONSISTENT_LOGTASK;
  //      fHistInconsistency = TRUE;
  //      pLogRec = &(pLog->AnalAutoSubst) ;
  //      PCOPYALLCOUNTS(  pLogRec,
  //                       (count_minus) );
  //    }
  //  } /* endif */
  //} // end if
  return(fHistInconsistency);
} // end of RPTCOPYALLCOUNTS

BOOL RptGetRecords2( BOOL bDde, PRPT pRpt, PBOOL pfCorrupted, PLONG pLastValidTime, PLONG pNextValidTime )
{
  BOOL      fOK = FALSE;                // error indicator
  BOOL      fAlloc = TRUE;              // allocation indicator
  BOOL      fNext = TRUE;               // read next record indicator
  PALLINFO  pAllInfoTmp = NULL;         // pointer to ALLINFO structure
  PPALLINFO ppAllInfoFieldTmp = NULL;   // pointer to PALLINFO array
  FILE      *hfFileHandle = NULL;       // file handle
  USHORT    usFileRc = 0;               // return from file handle
  ULONG     ulBytesRead = 0;            // number of bytes read by UtlRead
  ULONG     ulAllocEntries = 0;         // size of ALLINFOS pointer array in number of entries
  ULONG     ulNewAllocEntries = 0;      // new size of pointer array during re-allocation
  PVOID     pArray;                     // pointer to allocated memory
  ULONG     ulRecordIndex = 1;          // index of record in file
  HWND      hwndRptLongLB = NULLHANDLE; // Listbox with long names
  INT       sMaxDocs = 0;
  CHAR      szDocName[MAX_PATH144];
  CHAR      szLongFileName[MAX_LONGPATH]; // long filename

  // build absolute path of histlog file
  strcpy (pRpt->szHistLogFile, pRpt->szFolderObjName);
  strcat (pRpt->szHistLogFile, RPT_HISTLOG_DIRECTORY);
  strcat (pRpt->szHistLogFile, HISTLOGFILE);

  // allocate buffer for current record
  fOK = UtlAllocHwnd((PVOID*)&pAllInfoTmp, 0L, sizeof(ALLINFO), ERROR_STORAGE, pRpt->hwndErrMsg );

  // allocate memory for PALLINFO array, initial size INIT_SIZE_A
  ulAllocEntries = INIT_SIZE_A;
  if ( fOK ) fOK = UtlAllocHwnd((PVOID*)&pArray, 0L, (ULONG)(INIT_SIZE_A * sizeof(PALLINFO)), ERROR_STORAGE, pRpt->hwndErrMsg );

  if ( fOK )
  {
    pRpt->ppAllInfoField = (PPALLINFO)pArray;  // store memory address in RPT
    ppAllInfoFieldTmp = (PPALLINFO)pArray;     // set tmp pointer
  } /* endif */

  // fill second listbox with long filenames
  if ( fOK )
  {
    // 1. invisible listbox for long filenames
    HWND hwndLB = NULLHANDLE;                   // handle of invisible LB
    hwndLB = WinCreateWindow( pRpt->hwndRptHandlerLB, WC_LISTBOX, "", LBS_MULTIPLESEL, 0, 0, 0, 0, pRpt->hwndRptHandlerLB, 
                              HWND_TOP, (USHORT)-1, NULL, NULL);
    hwndRptLongLB = hwndLB;

    if (bDde == FALSE)
    {
        // 2. invisible listbox for short filenames
        hwndLB = WinCreateWindow (HWND_DESKTOP, WC_LISTBOX, "", LBS_MULTIPLESEL, 0, 0, 0, 0,
                                  pRpt->hwndRptHandlerLB, HWND_TOP, (USHORT)-1, NULL, NULL); 
        pRpt->hwndRptHandlerLB = hwndLB;
    }
  } /* endif */

  if (bDde == TRUE)
  {
      PSZ       pNamesBuf = NULL;

      sMaxDocs = QUERYITEMCOUNTHWND( pRpt->hwndRptHandlerLB );

      // alloc storage to hold all names from listbox
      if (!UtlAlloc((PVOID *)&pNamesBuf, 0L, sizeof(CHAR) * sMaxDocs, ERROR_STORAGE)) fOK = FALSE;

      SHORT sIndex2 = 0;
      while ( sIndex2 < sMaxDocs )
      {
        // get document name in listbox
        QUERYITEMTEXTHWND( pRpt->hwndRptHandlerLB, sIndex2++, szDocName );

        // get Long Doc Name
        CHAR szProperty[MAX_EQF_PATH];
        strcpy( szProperty,pRpt->szFolderObjName);
        strcat( szProperty,BACKSLASH_STR);
        strcat( szProperty,szDocName);
        DocQueryInfo2( szProperty, NULL,NULL,NULL,NULL, szLongFileName, NULL,NULL,FALSE);
        if ( !*szLongFileName )
        {
          strcpy( szLongFileName, szDocName );
        } // endif
        _strupr(szLongFileName);
        INSERTITEMENDHWND (hwndRptLongLB, szLongFileName);
      } // end while
  }
  else if (bDde == FALSE)
  {
      pRpt->ulAllInfoRecords = 0;
      pRpt->ulCalcInfoRecords = 0;

      // pRpt->szActualDocument always contains short file names delimited by comma
      {
          // count the documents delimited by comma
          int iCnti = 0;
          int iCntj = 0;

          while (pRpt->pszActualDocument[iCnti] != EOS)
          {
             if (pRpt->pszActualDocument[iCnti] == ',') iCntj++;
             iCnti++;
          }

          iCntj++;

          pRpt->usSelectedDocuments = (USHORT)iCntj;
          sMaxDocs = iCntj;

          if (sMaxDocs > 0)
          {
              // assemble the long filename and load the invisible Listbox
              SHORT sIndex2 = 0;
              while (sIndex2++ < sMaxDocs)
              {
                CHAR szProperty[MAX_EQF_PATH];

                  iCnti = 0;
                  iCntj = 0;

                  while (pRpt->pszActualDocument[iCntj] != ',' && pRpt->pszActualDocument[iCntj] != EOS)
                    iCntj++;

                  // copy document name to szActualDocument
                  strncpy(szDocName, pRpt->pszActualDocument, iCntj);
                  szDocName[iCntj] = EOS;

                  strcpy(szProperty, pRpt->szFolderObjName);
                  strcat(szProperty, BACKSLASH_STR);
                  strcat(szProperty, szDocName);

                  DocQueryInfo2(szProperty, NULL,NULL,NULL,NULL, szLongFileName, NULL,NULL,FALSE);

                  if ( !*szLongFileName )
                  {
                    strcpy(szLongFileName,szDocName);
                  } /*endif*/

                  _strupr(szLongFileName);

                  {
                    USHORT usRC;
                    usRC = INSERTITEMENDHWND( hwndRptLongLB, szLongFileName);
                  }
                  _strupr(szDocName);

                  {
                    USHORT usRC;
                    usRC = INSERTITEMENDHWND (pRpt->hwndRptHandlerLB, szDocName);
                  }

                  if (pRpt->pszActualDocument[iCntj] != EOS)
                  {
                    // skip the comma
                    iCntj++;

                    // copy rest of document names to start of pRpt->szActualDocument
                    while (pRpt->pszActualDocument[iCntj] != EOS)
                        pRpt->pszActualDocument[iCnti++] = pRpt->pszActualDocument[iCntj++];

                    pRpt->pszActualDocument[iCnti] = EOS;
                  } // end if
              } // end while
          } // end if
      } // end if (pRpt->fFolderSelected == FALSE)
  } // end else if (bDde == FALSE)

  if ( fOK )
  {
    // open histlog file
    hfFileHandle = fopen( pRpt->szHistLogFile, "rb" );
    if ( hfFileHandle == NULL )
    {
      fOK = FALSE;
      usFileRc = ERROR_OPEN_FAILED;
    } /* endif */
  }

  // loop over each histlog record of histlog file
  while ( fOK && fNext )
  {

    // check, if array is still large enough
    if ( pRpt->ulAllInfoRecords >= ulAllocEntries )
    {
      // realloc memory for PALLINFO array
      ULONG ulOldSize = ulAllocEntries * (ULONG)sizeof(PALLINFO);
      ulNewAllocEntries = ulAllocEntries + INIT_SIZE_A;
      ULONG ulNewSize = ulNewAllocEntries * (ULONG)sizeof(PALLINFO);
      fOK = UtlAllocHwnd ((PVOID*)&pArray, ulOldSize, ulNewSize, ERROR_STORAGE, pRpt->hwndErrMsg );

      if ( fOK )
      {
        pRpt->ppAllInfoField = (PPALLINFO)pArray;    // store memory address in RPT
        ppAllInfoFieldTmp = (PPALLINFO)pArray;       // set tmp pointer
        ppAllInfoFieldTmp += pRpt->ulAllInfoRecords; // update tmp pointer
        ulAllocEntries = ulNewAllocEntries;
      }
      else
      {
        pRpt->fErrorPosted = TRUE; // error msg has been shown
      }
    } // end if

    if ( fOK )
    {
      // set record number
      pAllInfoTmp->ulRecord = ulRecordIndex++;

      // read fixed part of histlog record
      ulBytesRead = fread( (PVOID)&pAllInfoTmp->histLogRecord, 1, sizeof(HISTLOGRECORD), hfFileHandle );
      if ( ulBytesRead == 0 )
      {
        // probably end-of-file
      }
      else if ( ulBytesRead != sizeof(HISTLOGRECORD) )
      {
        usFileRc = ERROR_READ_FAULT;
        fOK = FALSE;
      } /* endif */

      // handle reset record 
      if ( (usFileRc == NO_ERROR) && (ulBytesRead > 0) && ((pAllInfoTmp->histLogRecord.Task == HISTDATA_RESET_LOGTASK)) && (pRpt->usReport != HISTORY_REPORT) )
      {
        // release memory of all pAllInfos read up to now
        ULONG ulRecord = 0;
        while ( ulRecord < pRpt->ulAllInfoRecords )
        {
          ppAllInfoFieldTmp = (PPALLINFO)pArray;  
          ppAllInfoFieldTmp += ulRecord;
          UtlAlloc( (PVOID*)ppAllInfoFieldTmp, 0L, 0L, NOMSG );
          ulRecord++;
        } /* endwhile */           

        ppAllInfoFieldTmp = (PPALLINFO)pArray;    
        pRpt->ulAllInfoRecords = 0;
        ulAllocEntries = 0;
        ulRecordIndex = 0;
      } 
      // skip not necssary logs
      // DOCEXPORT_LOGTASK
      // DOCPROP_LOGTASK
      // FOLPROP_LOGTASK
      // FOLPROPSHIPMENT_LOGTASK
      // VERSION_LOGTASK
      else if ( usFileRc == NO_ERROR && ulBytesRead > 0 &&
           (pAllInfoTmp->histLogRecord.Task == DOCEXPORT_LOGTASK ||
            pAllInfoTmp->histLogRecord.Task == FOLPROP_LOGTASK   ||
            pAllInfoTmp->histLogRecord.Task == FOLPROP_LOGTASK2  ||
            pAllInfoTmp->histLogRecord.Task == VERSION_LOGTASK   ||
            pAllInfoTmp->histLogRecord.Task == FOLPROPSHIPMENT_LOGTASK   ||
            pAllInfoTmp->histLogRecord.Task == DOCPROP_LOGTASK)&&
           pRpt->usReport != HISTORY_REPORT )
      {
        // correct sizes in older records
        if ( fOK )
        {
          HistLogCorrectRecSizesX( &pAllInfoTmp->histLogRecord );
        } // end if

        // read any document long name
        if ( fOK )
        {
          pAllInfoTmp->szLongName[0] = EOS;

          if ( pAllInfoTmp->histLogRecord.fLongNameRecord )
          {
            USHORT usLongNameLength = pAllInfoTmp->histLogRecord.usSize -
                                      pAllInfoTmp->histLogRecord.usAddInfoLength -
                                      sizeof(HISTLOGRECORD);

            if ( usLongNameLength != 0 )
            {
              ulBytesRead = fread( (PVOID)&pAllInfoTmp->szLongName, 1, usLongNameLength, hfFileHandle );
              if ( !ulBytesRead )
              {
                usFileRc = ERROR_READ_FAULT;
                fOK = FALSE;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */

        // read variable part and skip it
        if ( fOK && (pAllInfoTmp->histLogRecord.usAddInfoLength) )
        {
          ulBytesRead = fread( (PVOID)&pAllInfoTmp->variablePart, 1, pAllInfoTmp->histLogRecord.usAddInfoLength, hfFileHandle );
          if ( !ulBytesRead )
          {
            usFileRc = ERROR_READ_FAULT;
            fOK = FALSE;
          } /* endif */
        } // end if

      }
      else if ( usFileRc == NO_ERROR && ulBytesRead > 0 )
      {
        // check, if record is valid
        if ( pAllInfoTmp->histLogRecord.lEyeCatcher != HISTLOGEYECATCHER )
        {
          LONG lCurrent = 0;
          BOOL fEOF = FALSE;
          fOK = FALSE;
          *pfCorrupted = TRUE;

          // skip data up to start of next valid record
          do
          {
            ulBytesRead = fread( &lCurrent, 1, sizeof(LONG), hfFileHandle );
            if ( ulBytesRead )
            {
              if ( lCurrent == HISTLOGEYECATCHER )
              {
                ulBytesRead = fread( &pAllInfoTmp->histLogRecord + sizeof(LONG), 1, sizeof(HISTLOGRECORD)-+ sizeof(LONG), hfFileHandle );
                *pNextValidTime = pAllInfoTmp->histLogRecord.lTime;
                fEOF = TRUE;  // force end of loop
              } /* endif */ 
            }
            else
            {
              fEOF = TRUE;
            } /* endif */
          } while ( !fEOF );
        }
        else
        {
          // remember time of last valid record
          *pLastValidTime = pAllInfoTmp->histLogRecord.lTime;
        } /* endif */

      // correct sizes in older records
      if ( fOK )
      {
        HistLogCorrectRecSizesX( &pAllInfoTmp->histLogRecord );
      } // end if


        // read or get document long name or use document short name instead
        if ( fOK )
        {
          pAllInfoTmp->szLongName[0] = EOS;

          if ( pAllInfoTmp->histLogRecord.fLongNameRecord )
          {
            USHORT usLongNameLength = pAllInfoTmp->histLogRecord.usSize -
                                      pAllInfoTmp->histLogRecord.usAddInfoLength -
                                      sizeof(HISTLOGRECORD);

            if ( usLongNameLength != 0 )
            {
              ulBytesRead = fread( (PVOID)&pAllInfoTmp->szLongName, 1, usLongNameLength, hfFileHandle );
              if ( !ulBytesRead )
              {
                usFileRc = ERROR_READ_FAULT;
                fOK = FALSE;
              } /* endif */
            }
            else
            {
              // the short name field contains the correct document name
              strcpy( pAllInfoTmp->szLongName, pAllInfoTmp->histLogRecord.szDocName );
            } /* endif */
          }
          else
          {
            strcpy( pAllInfoTmp->szLongName, pAllInfoTmp->histLogRecord.szDocName );
          } /* endif */
        } /* endif */

        // read varaiable part of histlog record
        if ( fOK && (pAllInfoTmp->histLogRecord.usAddInfoLength) )
        {
          ulBytesRead = fread( (PVOID)&pAllInfoTmp->variablePart, 1, pAllInfoTmp->histLogRecord.usAddInfoLength, hfFileHandle );
          if ( !ulBytesRead )
          {
            usFileRc = ERROR_READ_FAULT;
            fOK = FALSE;
          } /* endif */
        } // end if

        if ( fOK )
        {
          // ---------------------------------------------------------------
          // the invisible LB must contain all documents needed for reports
          // invisible LB contains in case of
          //  - selection in document list : the selected documents
          //  - selection in folder list :   all documents contained in folder
          //
          // for "History report" it is necessary that the invisible LB contains
          // all documents ever contained in selected folder
          // thus deleted documents must be insert in invisible LB
          // ---------------------------------------------------------------


          // ---------------------------------------
          // "History report" and folder is selected
          // scan the whole folder
          // ---------------------------------------
          if ( (pRpt->usReport == HISTORY_REPORT) && (pRpt->fFolderSelected == TRUE) )
          {
            // check, if invisible LB contains actual document of hist log record
            SHORT sItem = ISEARCHITEMEXACTHWND (pRpt->hwndRptHandlerLB, pAllInfoTmp->histLogRecord.szDocName);

            if ( sItem == -1 ) // document is not in LB -> insert document
            {
              // insert document in invisible LB
              INSERTITEMENDHWND (pRpt->hwndRptHandlerLB, pAllInfoTmp->histLogRecord.szDocName);

              // update number of documents, in this case of invisible LB
              pRpt->usSelectedDocuments++;
            } // end if

            // just for debugging
            if ( pRpt->ulAllInfoRecords >= ulAllocEntries )
            {
              int iSetBreakPointHere = 1;
            } /* endif */


            // allocate memory for this record
            PALLINFO pTemp = NULL;
            ULONG ulSize = RptGetRecordSize( pAllInfoTmp );
            fOK = UtlAllocHwnd ((PVOID*)&pTemp, 0L, ulSize, ERROR_STORAGE, pRpt->hwndErrMsg );
            if ( fOK )
            {
              memcpy( pTemp, pAllInfoTmp, ulSize );
              *ppAllInfoFieldTmp++ = pTemp;  // store address in PALLINFOs
              pRpt->ulAllInfoRecords++;            // update number of ALLINFOs in RPT
            } // end if
          }
          // ---------------------------------------
          // files are selected
          // scan all files
          // ---------------------------------------
          else
          {
            // check, if invisible LB contains actual document of hist log record
            // the document short name may have changed in the histlog so use
            // longname only
            SHORT sItem = ISEARCHITEMEXACTHWND( hwndRptLongLB, pAllInfoTmp->szLongName );

            if ( sItem == -1 )
            {
              // document is not in LB -> not selected or different case of name
              // try with1
              strcpy( szLongFileName, pAllInfoTmp->szLongName );
              _strupr( szLongFileName );
              sItem = ISEARCHITEMEXACTHWND( hwndRptLongLB, szLongFileName );
            } /* end if */

            if ( sItem == -1 ) // document is not in LB -> not selected
            {
              strcpy( szLongFileName, pAllInfoTmp->szLongName );
              _strlwr( szLongFileName );
              sItem = ISEARCHITEMEXACTHWND( hwndRptLongLB, szLongFileName );
            } /* end if */

            if ( sItem == -1 ) // document is not in LB -> not selected
            {
            }
            else
            {
              // document is selected, add record to our array
              PALLINFO pTemp = NULL;
              ULONG ulSize = RptGetRecordSize( pAllInfoTmp );
              fOK = UtlAllocHwnd ((PVOID*)&pTemp, 0L, ulSize, ERROR_STORAGE, pRpt->hwndErrMsg );
              if ( fOK )
              {
                memcpy( pTemp, pAllInfoTmp, ulSize );
                *ppAllInfoFieldTmp++ = pTemp;  // store address in PALLINFOs
                pRpt->ulAllInfoRecords++;            // update number of ALLINFOs in RPT
              } // end if
            } // end if
          } // end if
        } // end if
      }
      else
      {
        if ( ulBytesRead == 0 )  // file handle is still at end of file
        {
          fNext = FALSE;
        }
        else
        {
          fOK = FALSE;         // an error taken
          pRpt->fErrorPosted = TRUE; // error msg has been shown
        }
      } // end if
    } // end if
  } // end while

  // cleanup
  if ( hwndRptLongLB ) WinDestroyWindow(hwndRptLongLB);
  if ( hfFileHandle )  fclose( hfFileHandle );  
  if ( pAllInfoTmp ) UtlAlloc((PVOID*)&pAllInfoTmp, 0L, 0, NOMSG );

  // GQ 2017/11/10: Set fOK flag when at least some records have been read and the fCorrupted flag has been set to. 
  //                This will ensure that the automatic histlog correction can do it's magic
  if ( !fOK && (pfCorrupted != NULL) && *pfCorrupted && (pRpt->ulAllInfoRecords != 0) )
  {
    fOK = TRUE;
  } /* endif */

  return fOK;
} // end of RptGetRecords


BOOL RptSortRecordsX (PRPT pRpt)
{
  BOOL      fOK = FALSE;        // error indicator

  // sort using document long name, entry date
  qsort( pRpt->ppAllInfoField, pRpt->ulAllInfoRecords, sizeof(PALLINFO), RptAllInfoCompare2 );
  fOK = TRUE;

  return fOK;
} // end of RptSortRecordsX

// compare to allinfo structures (used by qsort)
int RptAllInfoCompare2( const void *pElement1, const void *pElement2 )
{
  int iResult;
  PALLINFO pAllInfo1 = *((PALLINFO *)pElement1); // ptr to first element
  PALLINFO pAllInfo2 = *((PALLINFO *)pElement2); // ptr to first element

  iResult = _stricmp( pAllInfo1->szLongName, pAllInfo2->szLongName );

  // if doc names are identical, use record number in order to keep
  // records in same order as in the history log file
  if ( iResult == 0 )
  {
    if ( pAllInfo1->ulRecord < pAllInfo2->ulRecord )
    {
      iResult = -1;
    }
    else
    {
      iResult = 1;
    } /* endif */
  } /* endif */

  return( iResult );
}


float RPT3HandleCategories2
(
PCRITERIASUMEX  pSum,
Target_Mode     mode,                  // Source,target,segments,changed
int             category               // category of details
)

{
  int iOutput = (-100);                // initial valud: n.a.

  PSUMPERCLASSEX pSumOfClass = NULL;

  switch ( category )
  {
    case simple:  pSumOfClass = &(pSum->SimpleSum); break;
    case medium:  pSumOfClass = &(pSum->MediumSum); break;
    case complex: pSumOfClass = &(pSum->ComplexSum); break;
  } /*endswitch */

  if ( pSumOfClass )
  {
    switch ( mode )
    {
      case Source:   iOutput =  pSumOfClass->ulSrcWords; break;
      case Target:   iOutput =  pSumOfClass->ulTgtWords; break;
      case Modified: iOutput =  pSumOfClass->ulModWords; break;
      case Segments: iOutput =  pSumOfClass->ulNumSegs;  break;
    } /*endswitch */
  }  /* end if */

  return((float)(iOutput));
} /* end of function RPT3HandleCategories2 */



void  RptCalcReportRow
(
  PRPT        pRpt,         // report
  PCntRow     pCntRow,      // the row to print
  Row_Mode    R_Mode,       // Details, No_Details
  int         S_Mode,       // Summary for each shipment in case of Details
  int        (*pColumn_Switch)[MAX_REPORT_COLUMNS],  // switch columns on and off
  CXmlWriter  *xw,          // active XmlWrite object
  Table_Type  iTable_Type   // type of currently processed table
)
{
  int     i;
  char    szNumber[MAX_LONGPATH];
  PCntRow pCntAct;
  int     Number_of_Cols;
  int     iStart,iEnd;
  PSZ     pszCat[4] = {"Simple", "Medium", "Complex", "Sum" };

  S_Mode; R_Mode; pRpt;

  pCntAct = pCntRow;

  xw->WriteStartElement( "wordcount" );
  if ( iTable_Type == TABLE )
  {
    // shipment only for tables
    if (pCntAct->szShipment[0]) xw->WriteAttributeString( "shipmentFolder", pCntAct->szShipment ); 
    sprintf( szNumber, "%1.0f", pCntAct->iRow[0][0] );   // document shipment
    xw->WriteAttributeString( "shipmentDocument", szNumber ); 
  } /* endif */

  if ( pCntRow==NULL ) return;

  iStart = 0; // always all detail columns
  iEnd = 3;   // always summary column

  // fuzziness settings
  if ( pCntAct->iRow[0][FuzzyLevel] || pCntAct->iRow[1][FuzzyLevel] || pCntAct->iRow[2][FuzzyLevel] )
  {
    for ( Number_of_Cols = iStart; Number_of_Cols <= iEnd; Number_of_Cols++ )
    {
      xw->WriteStartElement( "fuzzy" );
      xw->WriteAttributeString( "category", pszCat[Number_of_Cols] );
      sprintf( szNumber, "%1.0f", pCntAct->iRow[Number_of_Cols][FuzzyLevel] );
      xw->WriteAttributeString( "levelPerCent", szNumber );
      xw->WriteEndElement();
    }/* end for */
  } /* endif */

  // words
  for ( Number_of_Cols=iStart;Number_of_Cols<=iEnd;Number_of_Cols++ )
  {
    for ( i=FuzzyLevel+1; i<MAX_REPORT_COLUMNS; i++ )
    {
      if ( (*pColumn_Switch)[i]==1 )
      {
        if ( pCntAct->iRow[Number_of_Cols][i] > 0.0 )
        {
          if ( (strcmp(pCntAct->szID[0], "ComplexityFactor") == 0 )|| 
               (strcmp(pCntAct->szID[0], "PayFactor") == 0) )
          {
            sprintf(szNumber,"%1.2f", pCntAct->iRow[Number_of_Cols][i] );
          }
          else
          {
            sprintf(szNumber,"%1.0f", pCntAct->iRow[Number_of_Cols][i] );
          } /* endif */

          xw->WriteStartElement( "words" );
          xw->WriteAttributeString( "category", pszCat[Number_of_Cols] );
          xw->WriteAttributeString( "type", XmlComment_2[i] );
          xw->WriteAttributeString( "count", szNumber );
          xw->WriteEndElement();
        }
      }/* end if */
    }  /* end for */
  }/* end for */

  xw->WriteEndElement(); //  "wordcount"

}/* end of function RptCalcReportRow */


void  RPT3ZeroRow2
(
PCntRow     pCntRow
)
{
  int     i,j;

  for ( j=0; j<=3;j++ )
  {
    for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
    {
      pCntRow->iRow[j][i]=0;

    }/* end for */

  }/* end for */

}/* end of function RPT3ZeroRow2 */

float RPT3CalcStatistics2
(
float num1,
float num2
)
{
  float flResult;

  if ( num1>0.0 && num2>0.0 )
  {
    if ( num1>=num2 )
    {
      flResult = (float)(100. * num2 / num1);
    }
    else
    {
      flResult = (float)(100. * num1 / num2);
    }/* end if*/
  }
  else if ( num1>0.0 || num2>0.0 )
  {
    // zero percent used
    flResult = 0.0;
  }
  else
  {
    //n.a.
    flResult = -100.;
  } /* end if */

  return( flResult );
}/* end of function RPT3CalcStatistics2 */



//+----------------------------------------------------------------------------+
//|Internal function   RPT3AddCntRow2                                           |
//+----------------------------------------------------------------------------+
//|Function name:  RPT3AddCntRow2                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Add Details (row 1..3) to summary row 4                       |
//|                                                                            |
//|              Calculate statistics                                          |
//+----------------------------------------------------------------------------+
BOOL RPT3AddCntRow2
(
  PCntRow    pCntRow
)
{
  BOOL      fOK=TRUE;
  int       i,j;

  for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
  {
    pCntRow->iRow[3][i]=0;
  }  /* end for */


  for ( j=0;j<=2;j++ )
  {
    for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
    {
      if ( i != FuzzyLevel )   // do not sum fuzzy level values
      {
        pCntRow->iRow[3][i]+=pCntRow->iRow[j][i];
      }
    }  /* end for */
  } /* end for */

  // calculate the statistics
  pCntRow->iRow[3][PercentExactUsed]   = RPT3CalcStatistics2(pCntRow->iRow[3][ExactUsed],pCntRow->iRow[3][ExactExist]);
  pCntRow->iRow[3][PercentReplUsed]    = RPT3CalcStatistics2(pCntRow->iRow[3][ReplUsed],pCntRow->iRow[3][ReplExist]);
  pCntRow->iRow[3][PercentFuzzyUsed]   = RPT3CalcStatistics2(pCntRow->iRow[3][FuzzyUsed],pCntRow->iRow[3][FuzzyExist]);
  pCntRow->iRow[3][PercentFuzzyUsed_1] = RPT3CalcStatistics2(pCntRow->iRow[3][FuzzyUsed_1],pCntRow->iRow[3][FuzzyExist_1]);
  pCntRow->iRow[3][PercentFuzzyUsed_2] = RPT3CalcStatistics2(pCntRow->iRow[3][FuzzyUsed_2],pCntRow->iRow[3][FuzzyExist_2]);
  pCntRow->iRow[3][PercentFuzzyUsed_3] = RPT3CalcStatistics2(pCntRow->iRow[3][FuzzyUsed_3],pCntRow->iRow[3][FuzzyExist_3]);
  pCntRow->iRow[3][PercentMachUsed]    = RPT3CalcStatistics2(pCntRow->iRow[3][MachUsed],pCntRow->iRow[3][MachExist]);

  return fOK;
}/* end of function RPT3AddCntRow2 */



//+----------------------------------------------------------------------------+
//|Internal function  Rpt3SuccAddRows2                                          |
//+----------------------------------------------------------------------------+
//|Function name:   Rpt3SuccAddRows2                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Add two rows                                                  |
//|              row1 and row2 into row1                                       |
//|              row1 = row1 + row2                                            |
//|                                                                            |
//|       for the  Summary of all shipments                                    |
//|       statistics has to be handled individually                            |
//+----------------------------------------------------------------------------+


void  Rpt3SuccAddRows2
(
PCntRow     pCntRow1,
PCntRow     pCntRow2
)
{
  PCntRow     p1;
  PCntRow     p2;
  int         i,j;

  p1=pCntRow1;
  p2=pCntRow2;

  for ( j=0;j<=2;j++ )
  {
    p1->iRow[j][0] = max(p1->iRow[j][0],p2->iRow[j][0]);

    for ( i=1; i<MAX_REPORT_COLUMNS; i++ )
    {
      // -1 means Histlog Error, dont count this one
      if ( p2->iRow[j][i] != -1 && i != FuzzyLevel)
      {
        p1->iRow[j][i]+=p2->iRow[j][i];
      } // end if
    }  /* end for  i*/
  } /* end for j */

  // Statistics
  for ( j=0;j<=2;j++ )
  {
    p1->iRow[j][PercentExactUsed]   = RPT3CalcStatistics2(p1->iRow[j][ExactUsed],p1->iRow[j][ExactExist]);
    p1->iRow[j][PercentReplUsed]    = RPT3CalcStatistics2(p1->iRow[j][ReplUsed],p1->iRow[j][ReplExist]);
    p1->iRow[j][PercentFuzzyUsed]   = RPT3CalcStatistics2(p1->iRow[j][FuzzyUsed],p1->iRow[j][FuzzyExist]);
    p1->iRow[j][PercentFuzzyUsed_1] = RPT3CalcStatistics2(p1->iRow[j][FuzzyUsed_1],p1->iRow[j][FuzzyExist_1]);
    p1->iRow[j][PercentFuzzyUsed_2] = RPT3CalcStatistics2(p1->iRow[j][FuzzyUsed_2],p1->iRow[j][FuzzyExist_2]);
    p1->iRow[j][PercentFuzzyUsed_3] = RPT3CalcStatistics2(p1->iRow[j][FuzzyUsed_3],p1->iRow[j][FuzzyExist_3]);
    p1->iRow[j][PercentMachUsed]    = RPT3CalcStatistics2(p1->iRow[j][MachUsed],p1->iRow[j][MachExist]);
  } /* end for */
}/* end of function Rpt3SuccAddRows2 */


//+----------------------------------------------------------------------------+
//|Internal function  RPT3MultRows2                                             |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3MultRows2                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Mult two rows                                                 |
//|              into third one                                                |
//|              row3 = row1 * row2                                            |
//+----------------------------------------------------------------------------+
void  RPT3MultRows2
(
PCntRow     pCntRow1,
PCntRow     pCntRow2,
PCntRow     pCntRow3
)
{
  PCntRow     p1=pCntRow1;
  PCntRow     p2=pCntRow2;
  PCntRow     p3=pCntRow3;

  for ( int j=0;j<=3;j++ )
  {
    for ( int i=0; i<MAX_REPORT_COLUMNS; i++ )
    {
      p3->iRow[j][i] = p1->iRow[j][i]*p2->iRow[j][i];
    }  /* end for */
  } /* end while */
}/* end of function RPT3MultRows2 */


//+----------------------------------------------------------------------------+
//|Internal function   RPT3CopyRow2                                             |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3CopyRow2                                                |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:     COPY   row2 into row1                                      |
//|                        row1 = row2                                         |
//+----------------------------------------------------------------------------+
void  RPT3CopyRow2
(
  PCntRow     pCntRow1,
  PCntRow     pCntRow2
)
{
  PCntRow     p1=pCntRow1;
  PCntRow     p2=pCntRow2;

  for ( int j=0;j<=3;j++ )
  {

    for ( int i=0; i<MAX_REPORT_COLUMNS; i++ )
    {
      p1->iRow[j][i]=p2->iRow[j][i];
    } /* end for */
  } /* end while */
}/* end of function Copy Row */

//+----------------------------------------------------------------------------+
//|Internal function   RPT3MeanFactor2                                          |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3MeanFactor2                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:      Mean Summary Row of                                       |
//|                  complexity resp pay factor                                |
//+----------------------------------------------------------------------------+
void  RPT3MeanFactor2
(
  PCntRow     pCntRow1,     //initial
  PCntRow     pCntRow2,     //factor
  PCntRow     pCntRow3      //result
)
{
  PCntRow   p1=pCntRow1;
  PCntRow   p2=pCntRow2;
  PCntRow   p3=pCntRow3;

  for ( int i=0; i<MAX_REPORT_COLUMNS; i++ )
  {
    // mean factor
    if (p3->iRow[3][i] != 0 &&  p1->iRow[3][i] != 0)
    {
      p2->iRow[3][i] = p3->iRow[3][i] / p1->iRow[3][i] ;
    }
    else
    {
      p2->iRow[3][i]=(float) ((p2->iRow[0][i] + p2->iRow[1][i] + p2->iRow[2][i])/3.);
    } // end if
  } /* end for */
}/* end of function Mean Factor */



//+----------------------------------------------------------------------------+
//|Internal function   myRPT3UpdateSummaryRows                                   |
//+----------------------------------------------------------------------------+
//|Function name:   myRPT3UpdateSummaryRows                                      |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:      Mean Summary Row of                                       |
//|                  complexity resp pay factor                                |
//+----------------------------------------------------------------------------+


void  myRPT3UpdateSummaryRows
(
PCntRow     pCntRow1
)
{
  PCntRow   p1;
  int       i;

  p1=pCntRow1;

  for ( i=0; i<=3; i++ )
  {

    // AutoSubstitution
    p1->iRow[i][AutoSubst]  =  p1->iRow[i][AnalAutoSubst]  +  p1->iRow[i][EditAutoSubst]  +  p1->iRow[i][AnalAutoSubst2];

    // Manual Matches
    p1->iRow[i][ManualMach] =  p1->iRow[i][ExactExist]  +  p1->iRow[i][ReplExist] ;

    // Fuzzy Matches
    p1->iRow[i][FuzzyExist] =  p1->iRow[i][FuzzyExist_1]  +  p1->iRow[i][FuzzyExist_2]  +  p1->iRow[i][FuzzyExist_3];


  } /* end for */

}/* end of function UpdateSummaryRows */


//+----------------------------------------------------------------------------+
//|Internal function   myRPT3UpdateSummaryRows2                                  |
//+----------------------------------------------------------------------------+
//|Function name:   myRPT3UpdateSummaryRows2                                     |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:      Mean Summary Row of                                       |
//|                  complexity resp pay factor                                |
//|                  for statistics                                            |
//+----------------------------------------------------------------------------+

void  myRPT3UpdateSummaryRows2
(
PCntRow     pCntRow1
)
{
  PCntRow   p1;
  int       i;

  p1=pCntRow1;

  for ( i=0; i<=3; i++ )
  {

    // AutoSubstitution
    p1->iRow[i][AutoSubst]  =  (float)((p1->iRow[i][AnalAutoSubst]  +  p1->iRow[i][EditAutoSubst]  +  p1->iRow[i][AnalAutoSubst2])/3.);

    // Manual Matches
    p1->iRow[i][ManualMach] = (float)(( p1->iRow[i][ExactExist]  +  p1->iRow[i][ReplExist] )/2.);

    // Fuzzy Matches
    p1->iRow[i][FuzzyExist] = (float)(( p1->iRow[i][FuzzyExist_1]  +  p1->iRow[i][FuzzyExist_2]  +  p1->iRow[i][FuzzyExist_3])/3.);


  } /* end for */

}/* end of function UpdateSummaryRows2 */

BOOL RPT3AllocCntRow2
(
PVOID       *pCntRow
)
{
  PCntRow   pCntAct;      // actual row
  BOOL      fOK=TRUE;     // success indicator

  // three rows, details, fourth row summary
  fOK = UtlAlloc((PVOID*) &pCntAct,0L, (LONG) sizeof(CntRow),ERROR_STORAGE);

  if ( fOK )
  {
    RPT3ZeroRow2(pCntAct);
    pCntAct ->PNext = NULL;
    strcpy(pCntAct ->szID[0]," ");
    strcpy(pCntAct ->szID[1]," ");
    strcpy(pCntAct ->szID[2]," ");
    strcpy(pCntAct ->szID[3]," ");
  } // end if fok

  if ( ! fOK ) 
    *pCntRow = NULL;
  else       
    *pCntRow = pCntAct;

  return fOK;

} /* End of RPT3AllocCntRow2 */


// we have to use C calling conventions
#ifndef CPPTEST
extern "C"
{
#endif
  BOOL RPTXMLREPORT( BOOL bDde, HWND hwnd, PRPT pRpt);
#ifndef CPPTEST
}
#endif

BOOL RPTXMLREPORT( BOOL bDde, HWND hwnd, PRPT pRpt)
{
  LONG lLastValidTime = 0L;                      // last valid time info in history log
  LONG lNextValidTime = 0L;                      // next valid time info in history log
  BOOL fOK = FALSE;      // error indicator
  BOOL fCorrupted = FALSE; // TRUE = histlog file is corrupted

  RptLogStart2();

  // setup name of temporary XML file
  CHAR szOutFile[MAX_EQF_PATH];
  sprintf( szOutFile, "%s\\RPT.XML", pRpt->szFolderObjName );

  // get histlog records
  RptLogString2( "Reading history log..." );

  fOK = RptGetRecords2( bDde, pRpt, &fCorrupted, &lLastValidTime, &lNextValidTime );

  // handle corrupted history log file
  if ( fOK && fCorrupted )
  {
    CHAR szTempHistLog[MAX_EQF_PATH];

    // cleanup data of failed read operation
    if ( pRpt->ppAllInfoField )
    {
      ULONG ulIndex = 0;
      PPALLINFO ppAllInfoFieldTmp = pRpt->ppAllInfoField;   
      while ( ulIndex++ < pRpt->ulAllInfoRecords )
      {
        UtlAlloc ((PVOID*)&(*ppAllInfoFieldTmp++), 0L, 0L, NOMSG);
      } // end while
      UtlAlloc ((PVOID*)&(pRpt->ppAllInfoField), 0L, 0L, NOMSG);
    } // end if

    pRpt->ulAllInfoRecords = 0;
    pRpt->ulCalcInfoRecords = 0;

    // make copy of history log file for further evaluation
    {
      int iCounter = 1;
      BOOL fFreeSlotFound = FALSE;

      do
      {
        strcpy( szTempHistLog, pRpt->szHistLogFile);
        UtlSplitFnameFromPath( szTempHistLog );
        UtlSplitFnameFromPath( szTempHistLog );
        sprintf( szTempHistLog + strlen(szTempHistLog), "\\HISTLOG.DAMAGED%4.4ld", iCounter++ );
        fFreeSlotFound = !UtlFileExist( szTempHistLog ); 
      } while ( !fFreeSlotFound && (iCounter < 999) );

      UtlCopy(pRpt->szHistLogFile, szTempHistLog , 1, 0L, FALSE);
    }

    // cleanup history log file
    FolCleanHistlog( pRpt->szHistLogFile, FALSE, pRpt->hwndErrMsg ); 

    // show error message
    {
      PSZ pszParms[4];
      CHAR szFromDate[25];
      CHAR szToDate[25];

      if ( lLastValidTime )
      {
        struct tm     *pTimeDate;            
        pTimeDate = localtime( &lLastValidTime );
        sprintf( szFromDate , "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d", 
          pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1,
          pTimeDate->tm_mday, pTimeDate->tm_hour, 
          pTimeDate->tm_min, pTimeDate->tm_sec );
      }
      else
      {
        strcpy( szFromDate, "-" );
      } /* endif */

      if ( lNextValidTime )
      {
        struct tm     *pTimeDate;            
        pTimeDate = localtime( &lNextValidTime );
        sprintf( szToDate , "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d", 
          pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1,
          pTimeDate->tm_mday, pTimeDate->tm_hour, 
          pTimeDate->tm_min, pTimeDate->tm_sec );
      }
      else
      {
        strcpy( szToDate, "-" );
      } /* endif */

      pszParms[0] = szFromDate;
      pszParms[1] = szToDate;
      pszParms[2] = szTempHistLog;
      pszParms[3] = "TMEMEA@de.ibm.com";
      UtlErrorHwnd( INFO_HISTLOG_CORRECTED, MB_OK, 4, pszParms, EQF_INFO, pRpt->hwndErrMsg );
    }

    // repeat read of records
    fOK = RptGetRecords2( bDde, pRpt, &fCorrupted, &lLastValidTime, &lNextValidTime );

  } /* endif */

  RptLogULong( "History log read, records loaded into memory : ", pRpt->ulAllInfoRecords );

  if ( fOK && pRpt->usRptStatus == RPT_ACTIVE )
  {
    if ( fOK && pRpt->usRptStatus == RPT_ACTIVE )
    {
      switch ( pRpt->usReport )  // selected report
      {
        case HISTORY_REPORT : // history report
          switch ( pRpt->usOptions )
          {
            case BRIEF_SORT_BY_DOCUMENT : // brief, sorted by document
              if ( pRpt->usSelectedDocuments > 1 )
              {
                fOK = RptSortRecordsX (pRpt);
              } // end if
              break; // end case
          } // end switch
          break; // end case

        case CALCULATION_REPORT : // calculation report
        case SUMMARY_COUNTING_REPORT : // calculation report
          // sort PALLINFOs in document order
          RptLogString2( "Sorting history log records..." );
          fOK = RptSortRecordsX (pRpt);

          if ( fOK && pRpt->usRptStatus == RPT_ACTIVE )
          {
            // build CALCULATIONINFOs
            RptLogString2( "Creating calculation records..." );
            fOK = RptMakeCalculationRecordsX (pRpt);
          } // end if
          break; // end case

       case PRE_ANALYSIS_REPORT : // Source/Source Source/NLV report
        case REDUNDANCY_REPORT : // Source/Source Source/NLV report
          if ( fOK && pRpt->usRptStatus == RPT_ACTIVE )
          {
            // build CALCULATIONINFOs
            fOK = RPTPrepareMemoryMatchCountX (bDde, pRpt );
          } // end if
          break; // end case

          break; // end case

        case COMBINED_REPORT : // list of most used segments
          break; // end case
      } // end switch

      // create XML report
      if ( fOK && pRpt->usRptStatus == RPT_ACTIVE )
      {
        RptLogString2( "Creating XML output file..." );
        fOK = RptCreateXMLOutput( pRpt, szOutFile );
      } // end if

      // close any open HTML report file
      if ( pRpt->hHTMLControl )
      {
        UtlClose( pRpt->hHTMLControl, FALSE );
        pRpt->hHTMLControl = NULL;
      } /* endif */

      // convert and display report 
      if ( fOK && !pRpt->fBatch )
      {
        fOK = RptShowReport( pRpt, szOutFile, hwnd );
      } /* endif */

      // create report output file
      if ( fOK && pRpt->fRptFile )
      {
        fOK = RptMakeOutputFile( pRpt, szOutFile );
      } /* endif */
    } // end if
  } // end if

  RptLogEnd2();

  // case of batch, get rid of the listwindow,
  if ( pRpt->fBatch && bDde == TRUE )
  {
    if ( !fOK )
    {
      pRpt->pDDECntRpt->DDEReturn.usRc = UtlGetDDEErrorCode( pRpt->pDDECntRpt->hwndOwner );
    } /* endif */

    WinPostMsg( pRpt->pDDECntRpt->hwndOwner, WM_EQF_DDE_ANSWER, NULL, &pRpt->pDDECntRpt->DDEReturn );
    WinPostMsg( GETPARENT(hwnd), WM_CLOSE, NULL, NULL );
  } /* endif */

  // no need to free non-dde memory as this will be done when program stops execution
  if( bDde == TRUE)
  {
    if ( pRpt ) RptFreeMemoryX (pRpt);
  }

  // set report state ready
  if ( fOK ) pRpt->usRptStatus = RPT_READY;

  if ( !fOK && bDde == TRUE)
  {
    if ( hwnd != NULL )
    {
      WinPostMsg( GETPARENT(hwnd), WM_CLOSE, NULL, NULL );
    }
    else
    {
      WinPostMsg( pRpt->hwndErrMsg, WM_CLOSE, NULL, NULL );
    } /* endif */

    if ( pRpt ) pRpt->usRptStatus = RPT_KILL;
    if ( !pRpt->fErrorPosted ) UtlErrorHwnd(ERROR_INTERNAL,MB_CANCEL,0,NULL, INTERNAL_ERROR, pRpt->hwndErrMsg);
  } // end if

  return fOK;
} // end of RptMain


BOOL RptCreateXMLOutput( PRPT pRpt, PSZ pszOutFile )
{
  BOOL        fOK = TRUE;         // success indicator
  POUTMRI     pOutputMris = NULL; // pointer to OUTMRI array
  time_t      lTime;              // current time/date

  // start ouput
  CXmlWriter xw( pszOutFile );
  xw.Formatting = CXmlWriter::Indented;
  xw.Encoding = CXmlWriter::UTF8;
  xw.Indention = 2;

  if ( fOK )
  {
    pOutputMris = pRpt->pOutputMris;       // set pointer to OUTMRI
  }

  // create output
  if ( fOK  )
  {
    xw.WriteStartDocument();

    {
      CHAR szXSLFile[MAX_EQF_PATH];        // buffer for XSL stylesheet name

      fOK = RptGetStyleSheetNameForReportFormat( (RPTTYPE)pRpt->usReport, HTML, szXSLFile );

      xw.WriteStylesheet( UtlGetFnameFromPath( szXSLFile ) );
    }
	  
    switch ( pRpt->usReport )
    {
      case HISTORY_REPORT:          xw.WriteStartElement( "tmhistory" ); break; 
      case CALCULATION_REPORT:      xw.WriteStartElement( "tmwordcount" ); break; 
      case SUMMARY_COUNTING_REPORT: xw.WriteStartElement( "tmwordcount" ); break; 
      case PRE_ANALYSIS_REPORT:     xw.WriteStartElement( "tmwordcount" ); break; 
      case REDUNDANCY_REPORT:       xw.WriteStartElement( "tmwordcount" ); break; 
      case COMBINED_REPORT:         xw.WriteStartElement( "tmredsegments" ); break; 
      default:                      xw.WriteStartElement( "tmunknownreport" ); break; 
    } /*endswitch */

    xw.WriteStartElement( "description" );
    xw.WriteStartElement( "report" );
    switch ( pRpt->usReport )
    {
      case HISTORY_REPORT: // history report
        xw.WriteAttributeString( "type", "HistoryReport" );
        break; // end case

      case CALCULATION_REPORT: // calculating report
        xw.WriteAttributeString( "type", "CountingReport" );
        break; // end case

      case SUMMARY_COUNTING_REPORT: // calculating report
        xw.WriteAttributeString( "type", "CalculationReport" );
        break; // end case

      case PRE_ANALYSIS_REPORT:     // source/source  source/nlv report
        xw.WriteAttributeString( "type", "PreAnalysisReport" );
        break; // end case

      case REDUNDANCY_REPORT:     // source/source  source/nlv report
        xw.WriteAttributeString( "type", "RedundancyReport" );
        break; // end case


      case COMBINED_REPORT:     // list of most used segments
        xw.WriteAttributeString( "type", "RedundantSegmentList" );
        break; // end case

      default:
        xw.WriteAttributeString( "type", "Unknown" );
        break;
    } /*endswitch */

    if ( pRpt->usColumns4[1] )
    {
      xw.WriteAttributeString( "withDocumentList", "Yes" );
    } /* endif */

    // get time and date
    time (&lTime);                                  // get current time/date
    if ( lTime != 0L ) lTime -= 10800L;
    UtlLongToDateString( lTime, pRpt->szWorkString, sizeof(pRpt->szWorkString) );
    xw.WriteAttributeString( "generatedDate", pRpt->szWorkString );
    UtlLongToTimeString( lTime, pRpt->szWorkString, sizeof(pRpt->szWorkString) );
	  xw.WriteAttributeString( "generatedTime", pRpt->szWorkString );

	  xw.WriteStartAttribute( "documentsSelected" );
    xw.WriteInt( pRpt->fFolderSelected ? pRpt->usAllDocuments : pRpt->usSelectedDocuments );
	  xw.WriteEndAttribute();

    RptLogUShort(  "   Shipment processing flag : ", pRpt->usShipmentChk );
    RptLog2String2( "   Active shipment string   : ", pRpt->szShipmentChk );

    if ( pRpt->usReport == REDUNDANCY_REPORT )
    {
      // for redundancy report only: number of processed documents
	    xw.WriteStartAttribute( "documentsWithRedundantInfo" );
      xw.WriteInt( pRpt->usProcessedDocuments );
	    xw.WriteEndAttribute();
    }
    else if ( pRpt->usShipmentChk && 
              (strcmp(pRpt->szShipmentChk, "Single Shipments") != 0) &&
              (strcmp(pRpt->szShipmentChk, "All Shipments") != 0) )
    {
      // for single shipment reports only: number of documents in shipment

      // scan all records to get number of documents for selected shipment
      {
        ULONG ulIndex = 0;
        PPCALCINFO     ppCalcInfoFieldTmp;  // pointer to PCALCINFOs
        PCALCINFO      pCalcInfoTmp;        // pointer to CALCINFO
        CHAR           szActName[MAX_LONGPATH]=" "; // actual file-name
        PSZ            pszDocName = NULL;

        pRpt->usProcessedDocuments = 0; 
        szActName[0] = EOS;

        ppCalcInfoFieldTmp = pRpt->ppCalcInfoField; // set tmp pointer

        while ( ulIndex++ < pRpt->ulCalcInfoRecords )
        {
          pCalcInfoTmp = *ppCalcInfoFieldTmp;  // set pointer to CALCINFO

          RptLog2String2( "   CalcInfo shipment        : ", pCalcInfoTmp->szShipment );

          if ( strcmp(pCalcInfoTmp->szShipment, pRpt->szShipmentChk) == 0 )
          {
            if ( pCalcInfoTmp->szLongName[0] )
            {
              pszDocName = pCalcInfoTmp->szLongName;
            }
            else
            {
              pszDocName = pCalcInfoTmp->szDocument;
            } /* endif */

            // count document if it is new
            if ( strcmp( pszDocName, szActName ) != 0 )
            {
              strcpy( szActName, pszDocName );
              pRpt->usProcessedDocuments++; 
            } /* endif */
          } /* endif */

          // next entry
          ppCalcInfoFieldTmp++;
        } // end while
      }
        // for redundancy report only: number of processed documents
	      xw.WriteStartAttribute( "documentsInShipment" );
        xw.WriteInt( pRpt->usProcessedDocuments );
	      xw.WriteEndAttribute();
      } /* endif */

    RptLogUShort( "   Documents in folder : ", pRpt->usAllDocuments );

    // report options
    switch ( pRpt->usReport )
    {
      case HISTORY_REPORT: // history report
        switch ( pRpt->usOptions )
        {
          case BRIEF_SORT_BY_DATE     : xw.WriteAttributeString( "layout", "BriefSortByDate" ); break;
          case BRIEF_SORT_BY_DOCUMENT : xw.WriteAttributeString( "layout", "BriefSortByDocument" ); break;
          case VERSION                : xw.WriteAttributeString( "layout", "Version" ); break;
          case DETAIL                 : xw.WriteAttributeString( "layout", "Detail" ); break;
        } /*endswitch */
        break; // end case

      case CALCULATION_REPORT: // calculating report
        if ( pRpt->usOptions == WITH_TOTALS )
        {
          xw.WriteAttributeString( "layout", "WithTotals" ); break;
        }
        else
        {
          xw.WriteAttributeString( "layout", "WithoutTotals" ); break;
        } /* endif */
        break; // end case

      case REDUNDANCY_REPORT:     // source/source  source/nlv report
      case SUMMARY_COUNTING_REPORT: // calculating report
      case PRE_ANALYSIS_REPORT:     // source/source  source/nlv report
        switch ( pRpt->usOption3 )
        {
          case Standard:                   xw.WriteAttributeString( "layout", "Standard" ); break;
          case Standard_and_Group_Summary: xw.WriteAttributeString( "layout", "StdGrpSum" ); break;
          case Shrinked_in_Groups:         xw.WriteAttributeString( "layout", "ShrinkedInGrps" ); break;
        } /*endswitch */

        xw.WriteAttributeString( "autoshrink", (pRpt->usColumns[1]==1) ? "true" : "false" );
	      xw.WriteAttributeString( "showHistNum", "true" );
	      xw.WriteAttributeString( "showFuzzyLevel", "false" );
        xw.WriteAttributeString( "catDetails", (pRpt->usOption2==Details)  ? "true" : "false" );
        xw.WriteAttributeString( "catSum", pRpt->usColumns[0] ? "true" : "false" );
        switch ( pRpt->usOption1 )
        {
          case Source:   xw.WriteAttributeString( "countedElement", "Source Words" ); break;
          case Target:   xw.WriteAttributeString( "countedElement", "Target Words" ); break;
          case Modified: xw.WriteAttributeString( "countedElement", "Modified Words" ); break;
          case Segments: xw.WriteAttributeString( "countedElement", "Segments" ); break;
        } /*endswitch */

		    switch( pRpt->usOptions )
		    {
          case 0 : 
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "Base" );
            xw.WriteEndElement();
            break;

          case 1 : 
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "Base" );
            xw.WriteEndElement();
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "Summary" );
            xw.WriteEndElement();
            break;

          case 2 : 
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "Base" );
            xw.WriteEndElement();
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "Summary" );
            xw.WriteEndElement();
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "FactSheet" );
            xw.WriteEndElement();
            break;

          case 3 : 
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "Summary" );
            xw.WriteEndElement();
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "FactSheet" );
            xw.WriteEndElement();
            break;

          case 4 : 
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "FactSheet" );
            xw.WriteEndElement();
            break;

          case 5 : 
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "Base" );
            xw.WriteEndElement();
            xw.WriteStartElement( "option" );
            xw.WriteAttributeString( "name", "Summary" );
            xw.WriteEndElement();
            break;

        default:
            break;
        } /*endswitch */
        break; // end case

      case COMBINED_REPORT:     // list of most used segments
        xw.WriteStartElement( "option" );
        if ( pRpt->usOption[pRpt->usReport] == DETAILED_LIST )
        {
          xw.WriteAttributeString( "name", "Detailed" );
        }
        else
        {
          xw.WriteAttributeString( "name", "Standard" );
        } /* endif */
        xw.WriteEndElement();
        break; // end case

      default:
        break;
    } /*endswitch */
  	xw.WriteEndElement();

    BOOL fProfileOK = RptCheckProfile( pRpt );

    xw.WriteStartElement( "profile" );
    xw.WriteStartAttribute( "name" );
    if ( !fProfileOK )
    {
      pRpt->szDisplayProfileName[0] = '*';
      Utlstrccpy( pRpt->szDisplayProfileName+1, pRpt->szIntProfileName[0] ? pRpt->szIntProfileName : pRpt->szProfile, DOT );
      strcat( pRpt->szDisplayProfileName, "*" );
    }
    else
    {
      Utlstrccpy( pRpt->szDisplayProfileName, pRpt->szIntProfileName[0] ? pRpt->szIntProfileName : pRpt->szProfile, DOT );
    }
    xw.WriteString( pRpt->szDisplayProfileName );
    xw.WriteEndAttribute(); // "name"
  	xw.WriteEndElement();

    if ( pRpt->szRptDescription[0]  ) xw.WriteElementString( "text", pRpt->szRptDescription );

    xw.WriteStartElement( "folder" );
    xw.WriteAttributeString( "nameLong", pRpt->szLongFolderName );
    xw.WriteAttributeString( "nameShort", pRpt->szFolder );

    // get ISO name for target language
    if ( pRpt->szTargetLang[0] != EOS )
    {
      LanguageFactory *pLangFactory = LanguageFactory::getInstance();
      if ( pLangFactory != NULL )
      {
        char szISO[10];
        pLangFactory->getISOName( pRpt->szTargetLang, szISO );
        xw.WriteAttributeString( "targetLanguage", szISO );
      }
    }

    xw.WriteStartAttribute( "documentsAmount" );
    xw.WriteInt( (int)pRpt->usAllDocuments );
    xw.WriteEndAttribute();
	  xw.WriteEndElement(); // "Folder"

    xw.WriteStartElement( "translationManager" );
    char szVersion[10];
    sprintf( szVersion, "%d.%d", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE );  
    xw.WriteAttributeString( "version", szVersion );
    sprintf( szVersion, "%d", EQF_DRIVER_SUBRELEASE );  
    xw.WriteAttributeString( "release", szVersion  );
    sprintf( szVersion, "%d", EQF_DRIVER_BUILD );  
    xw.WriteAttributeString( "build", szVersion  );
    xw.WriteEndElement(); // "translationManager" 
    
    xw.WriteEndElement(); // "description" 
  } // end if

  if ( fOK )
  {
    switch ( pRpt->usReport )
    {
      case HISTORY_REPORT: // history report
        fOK = RptHistoryReport( pRpt, &xw );
        break; // end case

      case CALCULATION_REPORT: // counting report
        fOK = RptCountingReport( pRpt, &xw );
        break; // end case

      case SUMMARY_COUNTING_REPORT: // calculating report
      case PRE_ANALYSIS_REPORT:     // source/source  source/nlv report
      case REDUNDANCY_REPORT:     // source/source  source/nlv report
        fOK = RptCalcReport( pRpt, &xw );
        break; // end case

      case COMBINED_REPORT:     // list of most used segments
        fOK = RptRedundantSegmentList( pRpt, &xw );
        break; // end case
    } // end switch
  } // end if

  if ( fOK )
  {
    xw.WriteEndElement();
    xw.WriteEndDocument();
    xw.Close();
  }

  return fOK;
} // end of RptCreateXMLOutput

// convert to HTML/TEXT and display report
BOOL RptShowReport( PRPT pRpt, PSZ pszXMLFile, HWND hwnd  )
{
  BOOL fOK = TRUE;

  if ( pRpt->usReport != COMBINED_REPORT ) 
  {
    // convert file to HTML
    CHAR chHTMLFile[MAX_EQF_PATH];
    sprintf( chHTMLFile, "%s\\RPT.HTML", pRpt->szFolderObjName );
    fOK = RptConvertXML( pRpt, pszXMLFile, chHTMLFile, (RPTTYPE)pRpt->usReport, HTML );

    // navigate in Web control to HTML file containg the report data
    if ( fOK )
    {
      HWND hReportWindow = EqfQueryObject( pRpt->szRptInstanceObjName, clsREPORT, 0 );
      PostMessage( hReportWindow, WM_EQF_PROCESSTASK, MP1FROMSHORT(OPEN_AND_POSITION_TASK), 0L );
    } /* endif */
  }
  else
  {
    // convert file into UTF16t format
    CHAR chTextFile[MAX_EQF_PATH];
    sprintf( chTextFile, "%s\\RPT.TXT", pRpt->szFolderObjName );
    fOK = RptConvertXML( pRpt, pszXMLFile, chTextFile, (RPTTYPE)pRpt->usReport, UTF16_FILEFORMAT );

    // display report in our report list box
    if ( fOK )
    {
      // GQ 2014-03-04: use static buffer for output to listbox, direct usage of pRpt->szWorkStringW did not work at all...
      static wchar_t szOutLine[2400];

      FILE *hfReport = fopen( chTextFile, "rb" );
      if ( hfReport )
      {
        int iLines = 0;

        fgetws( pRpt->szWorkStringW, sizeof(pRpt->szWorkStringW)/sizeof(CHAR_W), hfReport );
        while ( !feof( hfReport ) )
        {
          int iLen = wcslen(pRpt->szWorkStringW);
          if ( (iLen > 0) && (pRpt->szWorkStringW[iLen-1] == L'\n') )
          {
            pRpt->szWorkStringW[iLen-1] = 0;
          } /* endif */
          wcscpy( szOutLine, pRpt->szWorkStringW  );
          SendMessageW( hwnd, LB_INSERTSTRING, (WPARAM) -1, (LPARAM)szOutLine );
          iLines += 1;

          fgetws( pRpt->szWorkStringW, sizeof(pRpt->szWorkStringW)/sizeof(CHAR_W), hfReport );
        } /*endwhile */

        fclose( hfReport );

        if ( iLines == 0 )
        {
          swprintf( szOutLine, L"Error: Temporary report file %s empty, most likely a XSLT conversion failure", chTextFile );
          SendMessageW( hwnd, LB_INSERTSTRING, (WPARAM) -1, (LPARAM)  szOutLine );
        } /* endif */           
		
      }
      else
      {
        swprintf( szOutLine, L"Error: Open of temporary report file %s failed, most likely a XSLT conversion failure", chTextFile );
        SendMessageW( hwnd, LB_INSERTSTRING, (WPARAM) -1, (LPARAM)  szOutLine );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of function RptShowReport */ 

// get name and path of stylesheet required for the given report and output format
// The stylsheet is always located in the TABLE directory
// the first 4 characters identify the report, the last 4 characters the output format
BOOL RptGetStyleSheetNameForReportFormat( RPTTYPE Report, OPTION_FILE_FORMAT Format, PSZ pszXSLFile )
{
  BOOL fOK = TRUE;

  Report; Format;

  UtlMakeEQFPath( pszXSLFile, NULC, TABLE_PATH, NULL );
  strcat( pszXSLFile, "\\" );
  
  switch ( Report )
  {
    case HISTORY_REPORT:          strcat( pszXSLFile, "HIST" );  break; 
    case CALCULATION_REPORT:      strcat( pszXSLFile, "CNT" );   break; 
    case SUMMARY_COUNTING_REPORT: strcat( pszXSLFile, "CALC" );  break; 
    case PRE_ANALYSIS_REPORT:     strcat( pszXSLFile, "CALC" );  break; 
    case REDUNDANCY_REPORT:       strcat( pszXSLFile, "CALC" );  break; 
    case COMBINED_REPORT:         strcat( pszXSLFile, "RESL" );  break; 
    default:                      fOK = FALSE; break; 
  } /*endswitch */

  switch ( Format)
  {
    case UTF16_FILEFORMAT: strcat( pszXSLFile, "UTF16" ); break;
    case HTML:             strcat( pszXSLFile, "HTML" ); break;
    case ASCII:            strcat( pszXSLFile, "TEXT" ); break;
    case RTF:              strcat( pszXSLFile, "RTF" ); break;
    default:               strcat( pszXSLFile, "XXXX" ); break;
  } /*endswitch */

  strcat( pszXSLFile, ".XSL" );

  return( fOK );
} /* end of function RptGetStyleSheetNameForReportFormat */ 

// convert XML to given file format
BOOL RptConvertXML( PRPT pRpt, PSZ pszXMLFile, PSZ pszOutFile, RPTTYPE Report, OPTION_FILE_FORMAT Format )
{
  BOOL fOK = TRUE;
  CHAR szXSLFile[MAX_EQF_PATH];        // buffer for XSL stylesheet name

  pRpt;                                // currently not used

#ifdef XSLTCONVERSIONLOG
  WIN32_FIND_DATA FindData;
  FILE *hfLog = NULL;

  UtlMakeEQFPath( szXSLFile, NULC, LOG_PATH, NULL );
  UtlMkDir( szXSLFile, 0, FALSE );
  strcat( szXSLFile, "\\XSLTCONV.LOG" );
  hfLog = fopen( szXSLFile, "a" );
  if ( hfLog != NULL )
  {
    HANDLE hFind;

    fprintf( hfLog, "XSLT conversion of file %s to %s\r\n", pszXMLFile, pszOutFile );
    hFind = FindFirstFile( pszXMLFile, &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
      fprintf( hfLog, "   Input XML file %s has a size of %ld bytes\r\n", pszXMLFile, FindData.nFileSizeLow );
      FindClose(hFind); 
    }
    else
    {
      fprintf( hfLog, "   Error: Input XML file %s does not exist!\r\n", pszXMLFile );
    } /* endif */       
  } /* endif */     
#endif
  
  // get name and path of stylesheet required for the conversion
  fOK = RptGetStyleSheetNameForReportFormat( Report, Format, szXSLFile );
  if ( !fOK )
  {
    pRpt->fErrorPosted = TRUE;
    UtlErrorHwnd( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR, pRpt->hwndErrMsg );
  }
  else if ( !UtlFileExist(szXSLFile) )
  {
    PSZ pszParm = szXSLFile;
    pRpt->fErrorPosted = TRUE;
    UtlErrorHwnd( ERROR_XSL_NOT_FOUND, MB_CANCEL, 1, &pszParm, EQF_ERROR, pRpt->hwndErrMsg );
    fOK = FALSE;
  } /* endif */
#ifdef XSLTCONVERSIONLOG
  if ( hfLog != NULL ) fprintf( hfLog, "   using XSLT stysheet \"%s\"\r\n", szXSLFile );
#endif
  if ( fOK )
  {
    CHAR szCommand[1024];

    UtlMakeEQFPath( szCommand, NULC, SYSTEM_PATH, NULL );
    strcat( szCommand, "\\WIN\\XalanTransform \"" );
    strcat( szCommand, pszXMLFile );
    strcat( szCommand, "\" \"" );
    strcat( szCommand, szXSLFile );
    strcat( szCommand, "\" \"" );
    strcat( szCommand, pszOutFile );
    strcat( szCommand, "\"" );
    system( szCommand );

  } /* endif */

  #ifdef XSLTCONVERSIONLOG
  {
    HANDLE hFind;

    hFind = FindFirstFile( pszOutFile, &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
      if ( hfLog != NULL ) fprintf( hfLog, "   Output file \"%s\" has a size of %ld bytes\r\n", pszOutFile, FindData.nFileSizeLow );
      FindClose( hFind );
    }
    else
    {
      if ( hfLog != NULL ) fprintf( hfLog, "   Error: Output file \"%s\" does not exist!\r\n", pszOutFile );
    } /* endif */       
    if ( hfLog != NULL ) fclose( hfLog );
  }
#endif

  
  return( fOK );
} /* end of function RptConvertXML */ 

// create the output file
BOOL RptMakeOutputFile( PRPT pRpt, PSZ pszXMLFile )
{
  BOOL    fOK = TRUE;
  RPTTYPE Report = (RPTTYPE)pRpt->usReport;
  USHORT  usOrgFormat = pRpt->usFormat;

  // for nonDDE API: always use XML format but select approbriate stylesheet for original format
  // GQ: 2016/08/08: fix for P403281: Keep original selected output format
  //if ( (pRpt->fBatch) && ((pRpt->hwndErrMsg == HWND_FUNCIF) || (pRpt->hwndErrMsg == NULLHANDLE)) )
  //{
  //  if ( UtlQueryUShort( QS_PROGRAMID ) != BATCHUTIL_PROGID )
  //  {
  //    pRpt->usFormat = XMLFILEFORMAT;
  //  } /* endif */
  //} /* endif */

  switch ( pRpt->usFormat )
  {
    case RTF   : fOK = RptConvertXML( pRpt, pszXMLFile, pRpt->szRptOutputFile, Report, RTF );  break; 
    case HTML  : fOK = RptConvertXML( pRpt, pszXMLFile, pRpt->szRptOutputFile, Report, HTML );  break; 
    case XMLFILEFORMAT :
      {
        CHAR szSourceXSLFile[MAX_EQF_PATH];        // buffer for source XSL stylesheet path
        CHAR szTargetXSLFile[MAX_LONGPATH];        // buffer for target XSL stylesheet path

        // file is already in XML format, so copy it to the target location
        fOK = CopyFile( pszXMLFile, pRpt->szRptOutputFile, FALSE );

        // copy the HTML stylesheet or the stylesheet for the original format to the same location as the XML file
        if ( usOrgFormat == XMLFILEFORMAT )
        {
          fOK = RptGetStyleSheetNameForReportFormat( (RPTTYPE)pRpt->usReport, HTML, szSourceXSLFile );
        }
        else
        {
          fOK = RptGetStyleSheetNameForReportFormat( (RPTTYPE)pRpt->usReport, (OPTION_FILE_FORMAT)usOrgFormat, szSourceXSLFile );
        } /* endif */
        strcpy( szTargetXSLFile, pRpt->szRptOutputFile );
        UtlSplitFnameFromPath( szTargetXSLFile );
        strcat( szTargetXSLFile, BACKSLASH_STR );
        strcat( szTargetXSLFile, UtlGetFnameFromPath( szSourceXSLFile ) );
        CopyFile( szSourceXSLFile, szTargetXSLFile, FALSE );

      }
      break;
    case ASCII : 
    default:
      fOK = RptConvertXML( pRpt, pszXMLFile, pRpt->szRptOutputFile, Report, ASCII );  break;
      break;
  } /*endswitch */

  return fOK;

}

// helper function: write a single folder property changes in XML format
void WriteFolPropChanges( CXmlWriter *xw, PSZ pszName, CHAR chObjectID, PSZ pszChangeList, BOOL fIsList )
{
  BOOL fFirstElement = TRUE;

  PSZ pszCurrentPos = pszChangeList;
  while ( *pszCurrentPos != EOS )
  {
    if ( fIsList && fFirstElement )
    {
      xw->WriteStartElement( pszName );
      fFirstElement = FALSE;
    } /* endif */

    if ( *pszCurrentPos == chObjectID )
    {
      xw->WriteStartElement( fIsList ? "name" : pszName );
      
      if ( pszCurrentPos[1] == FOLPROP_REMOVED_ACTION )
      {
        xw->WriteAttributeString( "type", "removed" );
      }
      else if ( pszCurrentPos[1] == FOLPROP_UNCHANGED_ACTION )
      {
        xw->WriteAttributeString( "type", "unchanged" );
      }
      else if ( pszCurrentPos[1] == FOLPROP_NEW_ACTION )
      {
        xw->WriteAttributeString( "type", "new" );
      } /* endif */

      xw->WriteString( pszCurrentPos + 2 );
      xw->WriteEndElement();

    } /* endif */
    pszCurrentPos += strlen(pszCurrentPos) + 1;
  } /* endwhile */

  if ( fIsList && !fFirstElement )
  {
    xw->WriteEndElement();
  } /* endif */
}

// create an XML history report
BOOL RptHistoryReport( PRPT pRpt, CXmlWriter *xw )
{
  BOOL           fOK = TRUE;          // error indicator
  PPALLINFO      ppAllInfoFieldTmp;   // pointer to PALLINFOs
  POUTMRI        pOutputMris;         // pointer to OUTMRI array
  ULONG          ulIndex = 0;         // index of PALLINFOs
  SHORT          sTypeTmp = 0;        // IMPEXPSUBTYPE of document
  PSZ            szDictionary = EOS;  // pointer to dictionary
  USHORT         usDicIndex = 0;      // index of current dictionary
  PALLINFO       pAllInfoTmp;         // pointer to ALLINFO
  PHISTLOGRECORD pHistLogRecordTmp;   // pointer to HISTLOGRECORD
  PVARPART       pVarPartTmp;         // pointer to VARPART
  PDOCIMPORTHIST pDocImportTmp;       // pointer to DOCIMPORTHIST
  PDOCIMPORTHIST2 pDocImportTmp2;       // pointer to DOCIMPORTHIST
  PANALYSISHIST  pAnalysisTmp;        // pointer to ANALYSISHIST
  PDOCEXPORTHIST pDocExportTmp;       // pointer to DOCEXPORTHIST
  PDOCPROPHIST   pDocPropTmp;         // pointer to DOCPROPHIST
  PFOLPROPHIST   pFolPropTmp;         // pointer to FOLPROPHIST
  PFOLPROPHISTSHIPMENT   pFolPropTmpShipment;         // pointer to FOLPROPHIST
  PVERSIONHIST   pVersionHistTmp;   // version of eqfdll, eqfd
  // Version consistency check
  INT            iWindows = -1;     // Windows or OS/2
  LONG           lProductDate = 0;    // eqfdll date
  BOOL           fInconsistency = FALSE; // inconsistent Versions used

  CHAR  szLongFileName[MAX_LONGPATH]; // long filename
  PSZ   pszLongFileName=NULL; // long filename
  CHAR  szPrevPath[MAX_LONGPATH]; // long filename

  pRpt->usStringIndex = 0;            // index for output field
  ppAllInfoFieldTmp = pRpt->ppAllInfoField;  // set tmp pointer to PALLINFOs
  pOutputMris = pRpt->pOutputMris;           // set pointer to OUTMRI

  switch ( pRpt->usOptions )
  {
    case BRIEF_SORT_BY_DATE:     // brief, sorted by date
    case BRIEF_SORT_BY_DOCUMENT: // brief, sorted by document
      xw->WriteStartElement( "summary" );
      while ( ulIndex < pRpt->ulAllInfoRecords )  // loop over all PALLINFOs
      {

        // loop over OUTPUT
        while ( ulIndex < pRpt->ulAllInfoRecords && pRpt->usStringIndex < MAX_O_LINES )
        {
          pAllInfoTmp = *ppAllInfoFieldTmp;                   // set pointer to ALLINFO
          pHistLogRecordTmp = &(pAllInfoTmp->histLogRecord);  // set pointer to HISTLOGRECORD


          xw->WriteStartElement( "histlogRecord" );
          xw->WriteStartElement( "header" );
          xw->WriteStartAttribute( "num" );
          xw->WriteInt( pAllInfoTmp->ulRecord );
          xw->WriteEndAttribute();
          xw->WriteAttributeString( "nameLong", pAllInfoTmp->szLongName );
          xw->WriteAttributeString( "nameShort", pRpt->szWorkString );
          LONG2DATETIME ( pHistLogRecordTmp->lTime, pRpt->szWorkString);
          xw->WriteAttributeString( "time", pRpt->szWorkString );
          RPTTaskIdToTask( (LOGTASK)pHistLogRecordTmp->Task, pRpt->szWorkString  );
          xw->WriteAttributeString( "task", pRpt->szWorkString );
          xw->WriteEndElement();  // "header" 
          xw->WriteEndElement(); // "history"

          ppAllInfoFieldTmp++;    // next PALLINFO
          ulIndex++;              // update index of PALLINFO
        }  // end while
        xw->WriteEndElement(); // "histlogRecord" );
      }   // end while

      // build records, second run: document names
      if ( pRpt->usColumns4[1] )
      {
        xw->WriteStartElement( "documentList" ); 
        ppAllInfoFieldTmp = pRpt->ppAllInfoField;  // set tmp pointer to PALLINFOs

        ulIndex = 0;

        strcpy(szPrevPath,"");

        // LOOP PALLINFOs
        while ( ulIndex < pRpt->ulAllInfoRecords )  // loop over all PALLINFOs
        {
          // LOOP OUTPUT
          while ( ulIndex < pRpt->ulAllInfoRecords && pRpt->usStringIndex < MAX_O_LINES )
          {
            pAllInfoTmp = *ppAllInfoFieldTmp;                   // set pointer to ALLINFO
            pHistLogRecordTmp = &(pAllInfoTmp->histLogRecord);  // set pointer to HISTLOGRECORD

            // fetch LongFileName
            strcpy( szLongFileName, pAllInfoTmp->szLongName );

            // try to split Path information
            pszLongFileName = strrchr(szLongFileName,'\\');

            if ( pszLongFileName )
            {
              *pszLongFileName = EOS;
              pszLongFileName ++;
            } /* endif */

            xw->WriteStartElement( "document" );
            xw->WriteStartAttribute( "num" );
            xw->WriteInt( pAllInfoTmp->ulRecord );
            xw->WriteEndAttribute();
            if ( pszLongFileName )
            {
              xw->WriteElementString( "path", szLongFileName );
              xw->WriteElementString( "name", pszLongFileName );
            }
            else
            {
              xw->WriteElementString( "name", szLongFileName );
            } // end if
            xw->WriteEndElement(); // "document" 

            ppAllInfoFieldTmp++;    // next PALLINFO
            ulIndex++;              // update index of PALLINFO
          }  // end while

          if ( ulIndex == pRpt->ulAllInfoRecords )
          {
//            xw->WriteEndElement(); // "???"
          } /* endif */
        }   // end while
        xw->WriteEndElement(); // "documentList" 
      } // end if Document List
      break; // end case


    case VERSION:
      while ( ulIndex < pRpt->ulAllInfoRecords )
      {
        pAllInfoTmp = *ppAllInfoFieldTmp;                   // set pointer to ALLINFO
        pHistLogRecordTmp = &(pAllInfoTmp->histLogRecord);  // set pointer to HISTLOGRECORD
        pVarPartTmp = &(pAllInfoTmp->variablePart);         // set pointer to VARPART

        // build output lines depending on process task
        switch ( pHistLogRecordTmp->Task )
        {
          case VERSION_LOGTASK:
            pRpt->usStringIndex = 0; // set index of actual string

            xw->WriteStartElement( "histlogRecord" );
            xw->WriteStartElement( "header" );

            // record nr., task performed
            xw->WriteStartAttribute( "num" );
            xw->WriteInt( pAllInfoTmp->ulRecord );
            xw->WriteEndAttribute();
            RPTTaskIdToTask( (LOGTASK)pHistLogRecordTmp->Task, pRpt->szWorkString  );
            xw->WriteAttributeString( "task", pRpt->szWorkString );

            // time
            LONG2DATETIME (pAllInfoTmp->histLogRecord.lTime, pRpt->szWorkString);
            xw->WriteAttributeString( "time", pRpt->szWorkString );
            xw->WriteEndElement(); // "header" 

            xw->WriteStartElement( "data" );
            pVersionHistTmp = &(pVarPartTmp->VersionHist);
            xw->WriteAttributeString( "type", "Version" );
            xw->WriteAttributeString( "platform", pVersionHistTmp->fWindows ? "Windows" : "OS/2" );
            LONG2DATETIME (pVersionHistTmp->lEqfdllDateTime, pRpt->szWorkString);
            xw->WriteAttributeString( "productDate", pRpt->szWorkString );
            xw->WriteAttributeString( "version", pVersionHistTmp->szVersionString );
            xw->WriteEndElement(); // "data"

            // consistency checking
            if ( iWindows == -1 )
            {
              iWindows = pVersionHistTmp->fWindows;
            }
            else
            {
              if ( iWindows != pVersionHistTmp->fWindows ) fInconsistency = TRUE;
            } // end if

            if ( pVersionHistTmp->lEqfdllDateTime < lProductDate )
            {
              fInconsistency = TRUE;
            }
            else
            {
              lProductDate =  pVersionHistTmp->lEqfdllDateTime;
            } // end if
            xw->WriteEndElement(); // "histlogRecord"

            break; // end case
        } // end switch

        ppAllInfoFieldTmp++;                        // next PALLINFO
        ulIndex++;                                  // update index of PALLINFOs
      } // end while

      xw->WriteStartElement( "versionConsistent" );
      if ( fInconsistency )
      {
        xw->WriteString( "No" );
      }
      else if ( iWindows != -1 )
      {
        xw->WriteString( "Yes" );
      }
      else
      {
        xw->WriteString( "NA" );
      } // end if
      xw->WriteEndElement(); // "versionConsistent"
      break;

    case DETAIL: // detail
      // loop over all PALLINFOs
      while ( ulIndex < pRpt->ulAllInfoRecords )
      {
        pRpt->usStringIndex = 0; // set index of actual string

        pAllInfoTmp = *ppAllInfoFieldTmp;                   // set pointer to ALLINFO
        pHistLogRecordTmp = &(pAllInfoTmp->histLogRecord);  // set pointer to HISTLOGRECORD
        pVarPartTmp = &(pAllInfoTmp->variablePart);         // set pointer to VARPART

        // fetch LongFileName
        strcpy( szLongFileName, pAllInfoTmp->szLongName );

        xw->WriteStartElement( "histlogRecord" );

        xw->WriteStartElement( "header" );

        // record nr., task performed
        xw->WriteStartAttribute( "num" );
        xw->WriteInt( pAllInfoTmp->ulRecord );
        xw->WriteEndAttribute();
        RPTTaskIdToTask( (LOGTASK)pHistLogRecordTmp->Task, pRpt->szWorkString  );
        xw->WriteAttributeString( "task", pRpt->szWorkString );

        // time
        LONG2DATETIME (pAllInfoTmp->histLogRecord.lTime, pRpt->szWorkString);
        xw->WriteAttributeString( "time", pRpt->szWorkString );

        // FILE Name

        // try to split Path information
        pszLongFileName = strrchr(szLongFileName,'\\');

        if ( pszLongFileName )
        {
          *pszLongFileName = EOS;
          pszLongFileName ++;
        } /* endif */


        if ( pszLongFileName )
        {
          xw->WriteAttributeString( "path", szLongFileName );
          xw->WriteAttributeString( "name", pszLongFileName );
        }
        else
        {
          xw->WriteAttributeString( "name", pAllInfoTmp->szLongName );
        } // end if

        xw->WriteEndElement(); // "header"

        // build output depending on process task
        xw->WriteStartElement( "data" );
        switch ( pHistLogRecordTmp->Task )
        {
          case DOCIMPORT_LOGTASK2: 
          case DOCIMPORT_LOGTASK:
            {
              SHORT sType;
              PSZ pszFolder, pszPath, pszMarkup, pszMemory, pszSourceLang, pszTargetLang, pszShipment;
              BOOL fSourceReplaced, fTargetReplaced;

              xw->WriteAttributeString( "type", "DocImport" );

              if ( pHistLogRecordTmp->Task == DOCIMPORT_LOGTASK )
              {
                pDocImportTmp     = &(pVarPartTmp->DocImport);  // set pointer to DOCIMPORTHIST
                sType             = pDocImportTmp->sType;
                pszFolder         = pDocImportTmp->szFolder;
                pszPath           = pDocImportTmp->szPath;
                pszMarkup         = pDocImportTmp->szMarkup;
                pszMemory         = pDocImportTmp->szMemory;
                pszSourceLang     = pDocImportTmp->szSourceLang;
                pszTargetLang     = pDocImportTmp->szTargetLang;
                fSourceReplaced   = pDocImportTmp->fSourceDocReplaced;
                fTargetReplaced   = pDocImportTmp->fTargetDocReplaced;
                pszShipment       = "";
              }
              else
              {
                pDocImportTmp2    = &(pVarPartTmp->DocImport2);  // set pointer to DOCIMPORTHIST
                sType             = pDocImportTmp2->sType;
                pszFolder         = pDocImportTmp2->szFolder;
                pszPath           = pDocImportTmp2->szPath;
                pszMarkup         = pDocImportTmp2->szMarkup;
                pszMemory         = pDocImportTmp2->szMemory;
                pszSourceLang     = pDocImportTmp2->szSourceLang;
                pszTargetLang     = pDocImportTmp2->szTargetLang;
                fSourceReplaced   = pDocImportTmp2->fSourceDocReplaced;
                fTargetReplaced   = pDocImportTmp2->fTargetDocReplaced;
                pszShipment = "";
              } /* endif */

              xw->WriteStartElement( "importType" );
              switch( sType )
              {
                case INTERN_SUBTYPE: xw->WriteString( "Intern" ); break;
                case EXTERN_SUBTYPE: xw->WriteString( "Extern" ); break;
                case FOLDER_SUBTYPE: xw->WriteString( "Folder" ); break;
                case NEWSOURCE_NOTIMPORTED_SUBTYPE: xw->WriteString( "NewSourceNotImported" ); break;
                case XLIFF_SUBTYPE:  xw->WriteString( "XLIFF" ); break;
                default: xw->WriteString( "Unknown" ); break; // type unknown:
              } /* endswitch */
              xw->WriteEndElement(); // "importType"

              if ( sType == INTERN_SUBTYPE || sType == FOLDER_SUBTYPE )
              {
                xw->WriteElementString( "folder", pszFolder );
              } // end if

              if ( (sType == EXTERN_SUBTYPE) || (sType == XLIFF_SUBTYPE) )
              {
                xw->WriteElementString( "path", pszPath );
                xw->WriteElementString( "document", pAllInfoTmp->szLongName );

                if ( pszMarkup[0] ) xw->WriteElementString( "markup", pszMarkup );
                if ( pszMemory[0] ) xw->WriteElementString( "memory", pszMemory );
                if ( pszSourceLang[0] ) xw->WriteElementString( "sourcelang", pszSourceLang );
                if ( pszTargetLang[0] ) xw->WriteElementString( "targetlang", pszTargetLang );
              } // end if

              xw->WriteElementString( "sourcereplaced", fSourceReplaced ? "Yes" : "No" );
              xw->WriteElementString( "targetreplaced", fTargetReplaced ? "Yes" : "No" );
              if ( pszShipment[0] ) xw->WriteElementString( "shipment", pszShipment );
            }
            break; // end case

          case DOCIMPNEWTARGET_LOGTASK:
          case DOCSAVE_LOGTASK:
          case DOCIMPNEWTARGET_LOGTASK2:
          case DOCSAVE_LOGTASK2:
          case DOCAPI_LOGTASK :
          case DOCIMPNEWTARGET_LOGTASK3:
          case DOCSAVE_LOGTASK3:
          case DOCAPI_LOGTASK3:
            switch ( pHistLogRecordTmp->Task )
            {
              case DOCIMPNEWTARGET_LOGTASK:  xw->WriteAttributeString( "type", "DocImpNewTarget" ); break;
              case DOCSAVE_LOGTASK:          xw->WriteAttributeString( "type", "DocSave" ); break;
              case DOCIMPNEWTARGET_LOGTASK2: xw->WriteAttributeString( "type", "DocImpNewTarget2" ); break;
              case DOCSAVE_LOGTASK2:         xw->WriteAttributeString( "type", "DocSave2" ); break;
              case DOCAPI_LOGTASK :          xw->WriteAttributeString( "type", "DocApi" ); break;
              case DOCIMPNEWTARGET_LOGTASK3: xw->WriteAttributeString( "type", "DocImpNewTarget3" ); break;
              case DOCSAVE_LOGTASK3:         xw->WriteAttributeString( "type", "DocSave3" ); break;
              case DOCAPI_LOGTASK3:          xw->WriteAttributeString( "type", "DocApi3" ); break;
            } /*endswitch */

            // write fuzzy level settings
            if ( pHistLogRecordTmp->Task == DOCSAVE_LOGTASK3 )
            { 
              xw->WriteStartElement( "fuzzyLevel" );
              RptWriteFuzzyLevel( xw, "small", pVarPartTmp->DocSave3.lSmallFuzzLevel );
              RptWriteFuzzyLevel( xw, "medium", pVarPartTmp->DocSave3.lMediumFuzzLevel );
              RptWriteFuzzyLevel( xw, "large", pVarPartTmp->DocSave3.lLargeFuzzLevel );
              xw->WriteEndElement(); // "fuzzyLevel" 
            } /* endif */

            if ( pHistLogRecordTmp->Task == DOCIMPNEWTARGET_LOGTASK ||
                 pHistLogRecordTmp->Task == DOCSAVE_LOGTASK )
            {
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.AnalAutoSubst), "analysisAutosubst" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.EditAutoSubst), "editAutosubst" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.ExactExist), "exact" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.ReplExist), "replace" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.FuzzyExist), "fuzzy" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.MachExist), "machine" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.NoneExist), "nomatch" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.NotXlated), "notTranslated" );
            }
            else
            {
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave2.AnalAutoSubst), "analysisAutosubst" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave2.EditAutoSubst), "editAutosubst" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave2.ExactExist), "exact" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave2.ReplExist), "replace" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave2.FuzzyExist), "fuzzy" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave2.MachExist), "machine" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave2.NoneExist), "nomatch" );
              RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave2.NotXlated), "notTranslated" );
            } // end if
            break;  // end case


          case ANALYSIS_LOGTASK:
            pAnalysisTmp = &(pVarPartTmp->Analysis);       // set pointer to ANALYSIS

            xw->WriteAttributeString( "type", "Analysis" );

            xw->WriteStartElement( "options" );
            xw->WriteAttributeString( "addToMem", pAnalysisTmp->fAddToMem ? "On" : "Off" );
            xw->WriteAttributeString( "autoReplace", pAnalysisTmp->fAutoReplace ? "On" : "Off" );
            xw->WriteAttributeString( "useLatestMatch", pAnalysisTmp->fLatestMatch ? "On" : "Off" );
            xw->WriteAttributeString( "autoJoin", pAnalysisTmp->fAutoJoin ? "On" : "Off" );
            xw->WriteEndElement(); // "options"

            xw->WriteElementString( "memory", pAnalysisTmp->szMemory );
            xw->WriteElementString( "markup", pAnalysisTmp->szMarkup );
            break; // end case


          case AUTOMATICSUBST_LOGTASK:
          case AUTOMATICSUBST_LOGTASK3:
            if (pHistLogRecordTmp->Task == AUTOMATICSUBST_LOGTASK3)
            {
              xw->WriteAttributeString( "type", "AutomaticSubst3" );
              xw->WriteStartElement( "fuzzyLevel" );
              RptWriteFuzzyLevel( xw, "small", pVarPartTmp->DocSave3.lSmallFuzzLevel );
              RptWriteFuzzyLevel( xw, "medium", pVarPartTmp->DocSave3.lMediumFuzzLevel );
              RptWriteFuzzyLevel( xw, "large", pVarPartTmp->DocSave3.lLargeFuzzLevel );
              xw->WriteEndElement(); // "fuzzyLevel" 
            }
            else
            {
              xw->WriteAttributeString( "type", "AutomaticSubst" );
            } /* endif */
            RptSumOut( pRpt, xw, &(pVarPartTmp->DocSave.AnalAutoSubst), "analysisAutosubst" );
            break;  // end case


          case DOCEXPORT_LOGTASK:
            pDocExportTmp = &(pVarPartTmp->DocExport);  // set pointer to DOCEXPORTHIST
            sTypeTmp = pDocExportTmp->sType;            // set export type

            xw->WriteAttributeString( "type", "DocExport" );

            xw->WriteStartElement( "exportType" );
            switch( pDocExportTmp->sType )
            {
              case INTERN_SUBTYPE: xw->WriteString( "Intern" ); break;
              case EXTERN_SUBTYPE: xw->WriteString( "Extern" ); break;
              case FOLDER_SUBTYPE: xw->WriteString( "Folder" ); break;
              case NEWSOURCE_NOTIMPORTED_SUBTYPE: xw->WriteString( "NewSourceNotImported" ); break;
              case XLIFF_SUBTYPE: xw->WriteString( "XLIFF" ); break;
              default: xw->WriteString( "Unknown" ); break; // type unknown:
            } /* endswitch */
            xw->WriteEndElement(); // "exportType"

            if ( (sTypeTmp == EXTERN_SUBTYPE) || (sTypeTmp == XLIFF_SUBTYPE) )
            {
              xw->WriteElementString( "sourceExported", pDocExportTmp->fSource ? "Yes" : "No" );
              xw->WriteElementString( "targetExported", pDocExportTmp->fTarget ? "Yes" : "No" );
              xw->WriteElementString( "revisionMarks", pDocExportTmp->fRevisionMarks ? "Yes" : "No" );
              xw->WriteElementString( "snoMatch", pDocExportTmp->fSNOMATCH ? "Yes" : "No" );
              xw->WriteElementString( "sourcePath", pDocExportTmp->szSourcePath );
              xw->WriteElementString( "targetPath", pDocExportTmp->szTargetPath );
              if ( pDocExportTmp->fSNOMATCH )  
              {
                xw->WriteElementString( "snoMatchPath", pDocExportTmp->szSNOMATCH );
              } // end if
            } // end if
            break; // end case


          case DOCPROP_LOGTASK:
            pDocPropTmp = &(pVarPartTmp->DocProp);  // set pointer to DOCPROPHIST

            xw->WriteAttributeString( "type", "DocProp" );

            if ( pDocPropTmp->szMarkup[0] ) xw->WriteElementString( "markup", pDocPropTmp->szMarkup );
            if ( pDocPropTmp->szMemory[0] ) xw->WriteElementString( "memory", pDocPropTmp->szMemory );
            if ( pDocPropTmp->szSourceLang[0] ) xw->WriteElementString( "sourcelang", pDocPropTmp->szSourceLang );
            if ( pDocPropTmp->szTargetLang[0] ) xw->WriteElementString( "targetlang", pDocPropTmp->szTargetLang );
            break; // end case

          case VERSION_LOGTASK:
            pVersionHistTmp = &(pVarPartTmp->VersionHist);

            xw->WriteAttributeString( "type", "Version" );

            xw->WriteAttributeString( "platform", pVersionHistTmp->fWindows ? "Windows" : "OS/2" );

            LONG2DATETIME (pVersionHistTmp->lEqfdllDateTime, pRpt->szWorkString);
            xw->WriteAttributeString( "productDate", pRpt->szWorkString );

            xw->WriteAttributeString( "version", pVersionHistTmp->szVersionString );
            break; // end case


          case FOLPROP_LOGTASK:
            pFolPropTmp = &(pVarPartTmp->FolProp);  // set pointer to FOLPROPHIST

            xw->WriteAttributeString( "type", "FolProp" );

            if ( pFolPropTmp->szDescription[0] ) xw->WriteElementString( "description", pFolPropTmp->szDescription );
            if ( pFolPropTmp->szEditor[0] ) xw->WriteElementString( "editor", pFolPropTmp->szEditor );
            if ( pFolPropTmp->szMarkup[0] ) xw->WriteElementString( "markup", pFolPropTmp->szMarkup );
            if ( pFolPropTmp->szMemory[0] ) xw->WriteElementString( "memory", pFolPropTmp->szMemory );
            if ( pFolPropTmp->szSourceLang[0] ) xw->WriteElementString( "sourceLang", pFolPropTmp->szSourceLang );
            if ( pFolPropTmp->szTargetLang[0] ) xw->WriteElementString( "targetLang", pFolPropTmp->szTargetLang );

            // list of max NUM_OF_FOLDER_DICS dictionaries
            if ( pFolPropTmp->DictTable[0] )
            {
              usDicIndex = 0;
              szDictionary = UtlParseX15 (pFolPropTmp->DictTable, usDicIndex);
              if ( szDictionary[0] )
              {
                xw->WriteStartElement( "dictionaries" );
                xw->WriteElementString( "name", szDictionary );
                while ( usDicIndex < NUM_OF_FOLDER_DICS )
                {
                  szDictionary = UtlParseX15 (pFolPropTmp->DictTable, usDicIndex);
                  if ( !szDictionary[0] ) break;

                  usDicIndex++;
                  xw->WriteElementString( "name", szDictionary );
                } // end while
                xw->WriteEndElement() ; // "dictionaries"
              } // end if
            } //end if
            break; // end case

          case FOLPROP_LOGTASK2:
            {
              PFOLPROPHIST2 pFolPropHist = &(pVarPartTmp->FolProp2);  // set pointer to FOLPROPHIST

              xw->WriteAttributeString( "type", "FolProp2" );

              WriteFolPropChanges( xw, "description", FOLPROP_DESCRIPTION_OBJ, pFolPropHist->szChanges, FALSE );
              WriteFolPropChanges( xw, "editor", FOLPROP_DESCRIPTION_OBJ, pFolPropHist->szChanges, FALSE );
              WriteFolPropChanges( xw, "markup", FOLPROP_MARKUP_OBJ, pFolPropHist->szChanges, FALSE );
              WriteFolPropChanges( xw, "memory", FOLPROP_MEMORY_OBJ, pFolPropHist->szChanges, FALSE );
              WriteFolPropChanges( xw, "sourceLang", FOLPROP_SOURCELANG_OBJ, pFolPropHist->szChanges, FALSE );
              WriteFolPropChanges( xw, "targetLang", FOLPROP_TARGETLANG_OBJ, pFolPropHist->szChanges, FALSE );
              WriteFolPropChanges( xw, "dictionaries", FOLPROP_DICTIONARY_OBJ, pFolPropHist->szChanges, TRUE );
              WriteFolPropChanges( xw, "romemories", FOLPROP_RO_MEMORY_OBJ, pFolPropHist->szChanges, TRUE );
            }
            break; // end case

            // SHIPMENT_HANDLER
          case FOLPROPSHIPMENT_LOGTASK:
            pFolPropTmpShipment = &(pVarPartTmp->FolPropShipment);  // set pointer to FOLPROPHIST

            xw->WriteAttributeString( "type", "FolPropShipment" );

            xw->WriteElementString( "shipment", pFolPropTmpShipment->szShipment );
            break; // end case

          case HISTDATA_RESET_LOGTASK:
            xw->WriteAttributeString( "type", "ResetPayableWordCount" );
            xw->WriteElementString( "reset", "" );
            break; 

          default:
            xw->WriteAttributeString( "type", "Unknown" );
            break; // end case
        } // end switch
        xw->WriteEndElement(); // "data"

        xw->WriteEndElement(); // "histlogRecord" 


        ppAllInfoFieldTmp++;                        // next PALLINFO
        ulIndex++;                                  // update index of PALLINFOs
      } // end while
      break; // end case
  } // end switch

  return fOK;
} // end of RptHistoryReport

BOOL RptCalcReport( PRPT pRpt, CXmlWriter *xw )
{
  Target_Mode    T_Mode = (Target_Mode)pRpt->usOption1;  // source,target,segments,modified
  Row_Mode       R_Mode = (Row_Mode)pRpt->usOption2;  // Details, No_Details

  Count_Mode     C_Mode;                    // Used, Exists
  float          Pay_per_Word=pRpt->Pay_per_Standard;          // Payment per word / line/ page

  BOOL           Include_Statistics;        // Include Statistics

  int            Column_Switch[MAX_REPORT_COLUMNS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0};
  // Columns switched on/off

  BOOL           Auto_Shrink = (pRpt->usColumns[1]==1);

  PCntRow       pComplexityFactor;           // row for complexity factor
  PCntRow       pPayFactor = NULL;                  // row for payment factor

  float          Payable_Words = 0;       // Words to be payed in final fact sheet
  float          Total_Pay = 0;           // Words to be payed in final fact sheet
  float          Total_Standard = 0;      // Payable words per standard page/line
  float          Standard_Factor = 0;     // factor for standard page/ standard line
  BOOL           fOK = TRUE;          // error indicator
  PPCALCINFO     ppCalcInfoFieldTmp = NULL;  // pointer to PCALCINFOs
  POUTMRI        pOutputMris = NULL;         // pointer to OUTMRI array
  ULONG          ulIndex = 0;         // index of PALLINFOs
  PCALCINFO      pCalcInfoTmp;        // pointer to CALCINFO
  CHAR  szLongFileName[MAX_LONGPATH]; // long filename
  PSZ   pszLongFileName;              // long filename
  static CHAR  szLongFolderName[MAX_LONGPATH]; // long filename

  // Structures for all tables of a folder
  PSumCntFile    pSumCntFileFirst=NULL;
  PSumCntFile    pSumCntFileAct=NULL;
  PSumCntFile    pSumCntFileTmp;
  PSumCntFile    pSumCntFile=NULL;    // Summary of the folder
  PSumCntFile    pFactSheet=NULL;     // Table for final fact sheet

  BOOL           fFirstRow=TRUE;      // first row now

  // structures for one row
  PCntRow        pCntRow = NULL;             // allocated row
  PCntRow        pCntRow1 = NULL;            // allocated row fact sheet
  PCntRow        pCntRow2 = NULL;            // allocated row fact Sheet
  PCntRow        pCntRow3 = NULL;            // allocated row fact Sheet
  PCntRow        pCntRow4 = NULL;            // allocated row fact Sheet
  PCntRow        pCntRow5 = NULL;            // allocated row fact Sheet
  PCntRow        pCntRowAct = NULL;          // pointer to one row
  PCntRow        pCntRowTotalSum = NULL;     // pointer to one row
  PCntRow        pCntRowSum = NULL;          // pointer to one row
  PCntRow        pCntRowLoop = NULL;         // pointer to one row

  CHAR           szActName[MAX_LONGPATH]=" "; // actual file-name
  int            iShipm = 0;
  char           szNumber[20];
  int            i,j;                   // running, index
  int            S_Mode;                // summary or not
  Table_Type     iTable_Type;
//  HPROP          hpropFolder;
//  PPROPFOLDER    ppropFolder;
//  ULONG          ulErrorInfo;
//  HAB            hab;
  LONG           lRunningNo = 0;         // No for document Identification
  SHORT          iEndShipm=0;
  SHORT          iRunShipm;

  // SHIPMENT HANDLER
  if (!strcmp(pRpt->szShipmentChk, "Single Shipments"))
  {
    iEndShipm = pRpt->iShipmentsUsed - 1 ;
  }// end if


  if (fOK)
  {
    ppCalcInfoFieldTmp = pRpt->ppCalcInfoField;  // set tmp pointer to PCALCINFOs
    pOutputMris = pRpt->pOutputMris;             // set pointer to OUTMRI
  }




  for (iRunShipm=0; iRunShipm <= iEndShipm; iRunShipm++)
  {
    // Reset
    pSumCntFileFirst = NULL;
    pSumCntFileFirst=NULL;
    pSumCntFileAct=NULL;
    pSumCntFileTmp=NULL;
    pSumCntFile=NULL;
    pFactSheet=NULL;
    fFirstRow=TRUE;
    lRunningNo = 0;
    ulIndex = 0;
    strcpy(szActName," ");

    if (iEndShipm > 0)
    {
      strcpy( pRpt->szShipmentChk, pRpt->aszShipments[iRunShipm] );
    }



    // security
    if ( Pay_per_Word<0.00001 ) Pay_per_Word =  0.0;

    // SET Options according Dialog input
    // report based on actually used/copied proposals
    // or on existing proposals

    Include_Statistics = (pRpt->usColumns[2]);

    C_Mode = (pRpt->usColumns[3] == 1) ? Exists : Used;

    // Factors for one standard Page, resp Standard line resp. unity
    // for final fact sheet summary
    if ( pRpt->usStandard==RPT_UNITY )
    {
      Standard_Factor=1.;
    }
    else if ( pRpt->usStandard==RPT_LINE )
    {
      Standard_Factor=Standard_Words_per_Line ;
    }
    else
    {
      Standard_Factor=Standard_Words_per_Page ;
    }/* end if */


    // reset Complexity factor for final fact sheet
    fOK = RPT3AllocCntRow2((PVOID*) &pComplexityFactor);

    if ( fOK )
    {

      for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
      {
        pComplexityFactor->iRow[0][i] = 0.00;
        pComplexityFactor->iRow[1][i] = 0.00;
        pComplexityFactor->iRow[2][i] = 0.00;
        pComplexityFactor->iRow[3][i] = 0.00;
      }
    } // end if fok

    // reset Pay factor
    if ( fOK )
    {
      fOK = RPT3AllocCntRow2((PVOID*) &pPayFactor);
    } // end if ok

    if ( fOK )
    {
      for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
      {
        pPayFactor->iRow[0][i] = 0.00;
        pPayFactor->iRow[1][i] = 0.00;
        pPayFactor->iRow[2][i] = 0.00;
        pPayFactor->iRow[3][i] = 0.00;
      }


// set complexity_factor and pay factor according  folder properties


      for ( i=0;i<4;i++ )
      {

        pComplexityFactor->iRow[i][AnalAutoSubst] = pRpt->Complexity_Factor[PayAnalAutoSubst][i];
        pComplexityFactor->iRow[i][AnalAutoSubst2] = pRpt->Complexity_Factor[PayAnalAutoSubst2][i];
        pComplexityFactor->iRow[i][EditAutoSubst] = pRpt->Complexity_Factor[PayEditAutoSubst][i];
        pComplexityFactor->iRow[i][ExactExist] = pRpt->Complexity_Factor[PayExactExist][i];
        pComplexityFactor->iRow[i][ReplExist] = pRpt->Complexity_Factor[PayReplExist][i];
        pComplexityFactor->iRow[i][FuzzyExist_1] = pRpt->Complexity_Factor[PayFuzzyExist_1][i];
        pComplexityFactor->iRow[i][FuzzyExist_2] = pRpt->Complexity_Factor[PayFuzzyExist_2][i];
        pComplexityFactor->iRow[i][FuzzyExist_3] = pRpt->Complexity_Factor[PayFuzzyExist_3][i];
        pComplexityFactor->iRow[i][MachExist] = pRpt->Complexity_Factor[PayMachExist][i];
        pComplexityFactor->iRow[i][NoneExist] = pRpt->Complexity_Factor[PayNoneExist][i];

      }// end for

      for ( i=0;i<4;i++ )
      {

        pPayFactor->iRow[i][AnalAutoSubst] = pRpt->Pay_Factor[PayAnalAutoSubst][i];
        pPayFactor->iRow[i][AnalAutoSubst2] = pRpt->Pay_Factor[PayAnalAutoSubst2][i];
        pPayFactor->iRow[i][EditAutoSubst] = pRpt->Pay_Factor[PayEditAutoSubst][i];
        pPayFactor->iRow[i][ExactExist] = pRpt->Pay_Factor[PayExactExist][i];
        pPayFactor->iRow[i][ReplExist] = pRpt->Pay_Factor[PayReplExist][i];
        pPayFactor->iRow[i][FuzzyExist_1] = pRpt->Pay_Factor[PayFuzzyExist_1][i];
        pPayFactor->iRow[i][FuzzyExist_2] = pRpt->Pay_Factor[PayFuzzyExist_2][i];
        pPayFactor->iRow[i][FuzzyExist_3] = pRpt->Pay_Factor[PayFuzzyExist_3][i];
        pPayFactor->iRow[i][MachExist] = pRpt->Pay_Factor[PayMachExist][i];
        pPayFactor->iRow[i][NoneExist] = pRpt->Pay_Factor[PayNoneExist][i];

      }// end for

    }// end if ok

    // ***********************************
    // SET Column_Switch according options
    // columns to be switched on/off
    // ***********************************
    if ( fOK )
    {

      if ( pRpt->usOption3  == Standard )  
        Column_Switch[AutoSubst] =
        Column_Switch[ManualMach] =
        Column_Switch[FuzzyExist] =
        Column_Switch[ChangedFuzzy] = 0;

      if ( pRpt->usOption3  == Standard )  
        Column_Switch[PercentExactUsed] =
        Column_Switch[PercentReplUsed] =
        Column_Switch[PercentFuzzyUsed] =
        Column_Switch[PercentFuzzyUsed_1] =
        Column_Switch[PercentFuzzyUsed_2] =
        Column_Switch[PercentFuzzyUsed_3] =
        Column_Switch[PercentMachUsed] =  0;


      if ( pRpt->usOption3 == Shrinked_in_Groups )  Column_Switch[AnalAutoSubst] =
        Column_Switch[AnalAutoSubst2] =
        Column_Switch[EditAutoSubst] =
        Column_Switch[ExactExist] =
        Column_Switch[ReplExist] =
        Column_Switch[FuzzyExist_1] =
        Column_Switch[FuzzyExist_2] =
        Column_Switch[FuzzyExist_3] =  0;

      if ( pRpt->usOption3  == Shrinked_in_Groups )  Column_Switch[PercentExactUsed] =
        Column_Switch[PercentReplUsed] =
        Column_Switch[PercentFuzzyUsed] =
        Column_Switch[PercentFuzzyUsed_1] =
        Column_Switch[PercentFuzzyUsed_2] =
        Column_Switch[PercentFuzzyUsed_3] =
        Column_Switch[PercentMachUsed] =  0;

      if ( pRpt->usOption3  == Standard_and_Group_Summary )  Column_Switch[PercentExactUsed] =
        Column_Switch[PercentReplUsed] =
        Column_Switch[PercentFuzzyUsed] =
        Column_Switch[PercentFuzzyUsed_1] =
        Column_Switch[PercentFuzzyUsed_2] =
        Column_Switch[PercentFuzzyUsed_3] =
        Column_Switch[PercentMachUsed] =  0;


      // changed fuzzy only for Exists Mode
      if ( C_Mode != Exists )  Column_Switch[ChangedFuzzy] =  0;


    } // end if ok

    // build records, loop over all PCALCINFOs 
    if ( fOK )
    {
      pRpt->usStringIndex = 0;            // index for output field
      ppCalcInfoFieldTmp = pRpt->ppCalcInfoField; // set tmp pointer to PCALCINFOs
    } // end if ok

    while ( fOK && ulIndex++ < pRpt->ulCalcInfoRecords )
    {
      pCalcInfoTmp = *ppCalcInfoFieldTmp;  // set pointer to CALCINFO
 
      if (!pRpt->usShipmentChk || !strcmp(pRpt->szShipmentChk, "All Shipments") ||
          !strcmp(pCalcInfoTmp->szShipment, pRpt->szShipmentChk))
      {
        /***************************/
        /* convert ltime to string */
        /***************************/
        LONG2DATETIME (pCalcInfoTmp->lTime, pRpt->szWorkString);

        /**********************/
        /* fetch LongFileName */
        /**********************/
        if ( pCalcInfoTmp->szLongName[0] )
        {
          strcpy( szLongFileName, pCalcInfoTmp->szLongName );
        }
        else
        {
          strcpy( szLongFileName, pCalcInfoTmp->szDocument);
        }

        // New document? then alloc space  
        if ( _stricmp(szActName,szLongFileName)!=0 )
        {
          iShipm=1;
          fFirstRow=TRUE;
          strcpy(szActName,szLongFileName);

          /***********************/
          /* alloc new document  */
          /***********************/

          fOK = UtlAlloc((PVOID*) &pSumCntFileTmp,0L,
                         (LONG) sizeof(SumCntFile),ERROR_STORAGE);

          if ( fOK && pSumCntFileFirst==NULL )
          {
            pSumCntFileFirst=pSumCntFileTmp;
          } /* end if */

          if ( fOK )
          {
            if ( pSumCntFileAct!=NULL )
            {
              pSumCntFileAct->PNext = pSumCntFileTmp;
            } /* end if */

            pSumCntFileAct = pSumCntFileTmp;
            pSumCntFileAct->PRow  = NULL;
            pSumCntFileAct->PSum  = NULL;
            pSumCntFileAct->PNext = NULL;

          } /* endif ok */

        } /* end if FileNames */

        fOK = RPT3AllocCntRow2((PVOID*) &pCntRow);          // Allocate One Row 

        /*************************/
        /*  fill header and row  */
        /*************************/
        if ( fOK && fFirstRow )
        {
          int icat;

          fFirstRow=FALSE;
          strcpy(pSumCntFileAct->szFolder, pRpt->szFolderObjName);
          strcpy(pSumCntFileAct->szName , pCalcInfoTmp->szDocument);
          strcpy(pSumCntFileAct->szLongName , szLongFileName);
// GQ: enabled code below!
          for (icat=0; icat<3; icat++)
          {
            pSumCntFileAct->docFuzzyLevel[icat] = pCalcInfoTmp->docFuzzyLevel[icat];
          }
          pSumCntFileAct->iTable = TABLE;
          lRunningNo ++;
          pSumCntFileAct->lRunningNo = lRunningNo;

          // CHECK FOR INCONSISTENT HISTORY LOGS

          if ( pCalcInfoTmp->Task == HISTDATA_INVALID_LOGTASK )
          {
            pSumCntFileAct->fHistError = TRUE;
          }
          else
          {
            pSumCntFileAct->fHistError = FALSE;
          }/* end if */

          if ( pCalcInfoTmp->Task == HISTDATA_INCONSISTENT_LOGTASK )
          {
            pSumCntFileAct->fHistInconsistency = TRUE;
          }
          else
          {
            pSumCntFileAct->fHistInconsistency = FALSE;
          }/* end if */

          pSumCntFileAct->PRow = pCntRow;
          pCntRowAct=pCntRow;
          pCntRowAct->PNext = NULL;
          pCntRowAct->lRunningNo = lRunningNo;


        }
        else if ( fOK )  // all other rows
        {

          // CHECK FOR INCONSISTENT HISTORY LOGS

          if ( pCalcInfoTmp->Task == HISTDATA_INVALID_LOGTASK )
          {
            pSumCntFileAct->fHistError = TRUE;
          }/* end if */

          if ( pCalcInfoTmp->Task == HISTDATA_INCONSISTENT_LOGTASK )
          {
            pSumCntFileAct->fHistInconsistency = TRUE;
          }/* end if */


          pCntRowAct->PNext = pCntRow;
          pCntRowAct = pCntRow ;
          pCntRowAct->PNext = NULL;
          pCntRowAct->lRunningNo = lRunningNo;


        }/* end if FirstRow */

        if ( fOK )
        {


          // insert (folder) Shipment number string
          // SHIPMENT_HANDLER
          if (pCalcInfoTmp->szShipment[0]!=EOS)
          {
            strcpy(pCntRowAct->szShipment , pCalcInfoTmp->szShipment);
          }

          // insert number of document shipment into NumberShip
          sprintf(szNumber,"%i",iShipm);
          strcat(szNumber,".");
          pCntRowAct->iRow[0][NumberShip]= (float)iShipm;
          iShipm++;


          // CHECK FOR INCONSISTENT HISTORY LOGS

          if ( pCalcInfoTmp->Task == HISTDATA_INVALID_LOGTASK )
          {
            pCntRowAct->fHistError = TRUE;
          }
          else
          {
            pCntRowAct->fHistError = FALSE;
          }/* end if */

          if ( pCalcInfoTmp->Task == HISTDATA_INCONSISTENT_LOGTASK )
          {
            pCntRowAct->fHistInconsistency = TRUE;
          }
          else
          {
            pCntRowAct->fHistInconsistency = FALSE;
          }/* end if */

          // File name into szID
          strcpy(pCntRowAct->szID[0],szLongFileName);
          if ( pCntRowAct->fHistError )
          {
            strcpy(pCntRowAct->szID[1],"RESET FORCED");
            strcpy(pCntRowAct->szID[2]," ");
          }
          else if ( pCntRowAct->fHistInconsistency )
          {
            strcpy(pCntRowAct->szID[1],"INCONSISTENT");
            strcpy(pCntRowAct->szID[2],"DATA LOST");

          }
          else
          {
            strcpy(pCntRowAct->szID[1]," ");
            strcpy(pCntRowAct->szID[2]," ");

          } // end if

          strcpy(pCntRowAct->szID[3],szLongFileName);

          /************************************************/
          /* NOW FILL THE ROW WITH WORD-COUNT INFORMATION */
          /************************************************/

          pCntRowLoop = pCntRowAct;

        }// end if fok

        if ( fOK )
        {
          /* LOOP CATEGORIES */

          for ( j=0;j<=2;j++ )
          {

            // adjusted fuzzy level
            pCntRowLoop->iRow[j][FuzzyLevel] = (float) pCalcInfoTmp->docFuzzyLevel[j];

            // analysis autosubst
            pCntRowLoop->iRow[j][AnalAutoSubst]  =RPT3HandleCategories2
                                                  (&(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst),
                                                   T_Mode,j);

            // analysis autosubst after edit
            pCntRowLoop->iRow[j][AnalAutoSubst2]  =RPT3HandleCategories2
                                                   (&(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst2),
                                                    T_Mode,j);


            // edit autosubst
            pCntRowLoop->iRow[j][EditAutoSubst]  =RPT3HandleCategories2
                                                  (&(pCalcInfoTmp->docSaveHistSum.EditAutoSubst),
                                                   T_Mode,j);


            if ( C_Mode == Exists )
            {

              // Exist
              //------

              // exact matches
              pCntRowLoop->iRow[j][ExactExist]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.ExactExist), T_Mode,j);

              // replace matches
              pCntRowLoop->iRow[j][ReplExist]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.ReplExist), T_Mode,j);

              // fuzzy exists
              pCntRowLoop->iRow[j][FuzzyExist]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist), T_Mode,j);

              // fuzzy exists_1
              pCntRowLoop->iRow[j][FuzzyExist_1]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_1), T_Mode,j);

              // fuzzy exists_2
              pCntRowLoop->iRow[j][FuzzyExist_2]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_2),T_Mode,j);

              // fuzzy exists_3
              pCntRowLoop->iRow[j][FuzzyExist_3]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_3), T_Mode,j);


              // machine matches
              pCntRowLoop->iRow[j][MachExist]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.MachExist), T_Mode,j);


              // Used
              //-----

              // exact matches
              pCntRowLoop->iRow[j][ExactUsed]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.ExactUsed),T_Mode,j);

              // replace matches
              pCntRowLoop->iRow[j][ReplUsed]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.ReplUsed),T_Mode,j);

              // fuzzy exists
              pCntRowLoop->iRow[j][FuzzyUsed]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed),T_Mode,j);

              // fuzzy exists_1
              pCntRowLoop->iRow[j][FuzzyUsed_1]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_1),T_Mode,j);

              // fuzzy exists_2
              pCntRowLoop->iRow[j][FuzzyUsed_2]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_2),T_Mode,j);

              // fuzzy exists_3
              pCntRowLoop->iRow[j][FuzzyUsed_3]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_3),T_Mode,j);


              // machine matches
              pCntRowLoop->iRow[j][MachUsed]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.MachUsed),T_Mode,j);

              // none matches
              pCntRowLoop->iRow[j][NoneExist]  =RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.NoneExist),T_Mode,j);
            }
            else
            {

              //Exists
              //------

              // exact matches
              pCntRowLoop->iRow[j][ExactExist]  =RPT3HandleCategories2
                                                 (&(pCalcInfoTmp->docSaveHistSum.ExactUsed),
                                                  T_Mode,j);

              // replace matches
              pCntRowLoop->iRow[j][ReplExist]  =RPT3HandleCategories2
                                                (&(pCalcInfoTmp->docSaveHistSum.ReplUsed),
                                                 T_Mode,j);

              // fuzzy exists
              pCntRowLoop->iRow[j][FuzzyExist]  =RPT3HandleCategories2
                                                 (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed),
                                                  T_Mode,j);

              // fuzzy exists_1
              pCntRowLoop->iRow[j][FuzzyExist_1]  =RPT3HandleCategories2
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_1),
                                                    T_Mode,j);

              // fuzzy exists_2
              pCntRowLoop->iRow[j][FuzzyExist_2]  =RPT3HandleCategories2
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_2),
                                                    T_Mode,j);

              // fuzzy exists_3
              pCntRowLoop->iRow[j][FuzzyExist_3]  =RPT3HandleCategories2
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_3),
                                                    T_Mode,j);


              // machine matches
              pCntRowLoop->iRow[j][MachExist]  =RPT3HandleCategories2
                                                (&(pCalcInfoTmp->docSaveHistSum.MachUsed),
                                                 T_Mode,j);


              // Used
              //-----

              // exact matches
              pCntRowLoop->iRow[j][ExactUsed]  =RPT3HandleCategories2
                                                (&(pCalcInfoTmp->docSaveHistSum.ExactExist),
                                                 T_Mode,j);

              // replace matches
              pCntRowLoop->iRow[j][ReplUsed]  =RPT3HandleCategories2
                                               (&(pCalcInfoTmp->docSaveHistSum.ReplExist),
                                                T_Mode,j);

              // fuzzy exists
              pCntRowLoop->iRow[j][FuzzyUsed]  =RPT3HandleCategories2
                                                (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist),
                                                 T_Mode,j);

              // fuzzy exists_1
              pCntRowLoop->iRow[j][FuzzyUsed_1]  =RPT3HandleCategories2
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_1),
                                                   T_Mode,j);

              // fuzzy exists_2
              pCntRowLoop->iRow[j][FuzzyUsed_2]  =RPT3HandleCategories2
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_2),
                                                   T_Mode,j);

              // fuzzy exists_3
              pCntRowLoop->iRow[j][FuzzyUsed_3]  =RPT3HandleCategories2
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_3),
                                                   T_Mode,j);


              // machine matches
              pCntRowLoop->iRow[j][MachUsed]  =RPT3HandleCategories2
                                               (&(pCalcInfoTmp->docSaveHistSum.MachExist),
                                                T_Mode,j);

              // none matches
              pCntRowLoop->iRow[j][NoneExist]  =RPT3HandleCategories2
                                                (&(pCalcInfoTmp->docSaveHistSum.NoneExist2),
                                                 T_Mode,j);


            } // *end if */


            // not translated
            pCntRowLoop->iRow[j][NotXlated]  =RPT3HandleCategories2
                                              (&(pCalcInfoTmp->docSaveHistSum.NotXlated),
                                               T_Mode,j);


            // calculate summary columns
            //**************************

            // summary: Auto Subst
            pCntRowLoop->iRow[j][AutoSubst]  =RPT3HandleCategories2
                                              (&(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst),
                                               T_Mode,j) + RPT3HandleCategories2
                                              (&(pCalcInfoTmp->docSaveHistSum.EditAutoSubst),
                                               T_Mode,j) + RPT3HandleCategories2
                                              (&(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst2),
                                               T_Mode,j);

            // summary: Manual Subst
            pCntRowLoop->iRow[j][ManualMach]  =RPT3HandleCategories2
                                               (&(pCalcInfoTmp->docSaveHistSum.ExactExist),
                                                T_Mode,j) + RPT3HandleCategories2
                                               (&(pCalcInfoTmp->docSaveHistSum.ReplExist),
                                                T_Mode,j);


            // changed words in fuzzy
            //***********************

            if ( T_Mode==Source )
            {
              pCntRowLoop->iRow[j][ChangedFuzzy]  =RPT3HandleCategories2
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist),
                                                    Modified,j);
            }
            else
            {
              //n.a.
              pCntRowLoop->iRow[j][ChangedFuzzy]  =  -100;
            }


            // Statistics
            //***********

            pCntRowLoop->iRow[j][PercentExactUsed] =  RPT3CalcStatistics2(RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.ExactUsed),T_Mode,j),
                                                                         RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.ExactExist),T_Mode,j));

            pCntRowLoop->iRow[j][PercentReplUsed] =  RPT3CalcStatistics2(RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.ReplUsed),T_Mode,j),
                                                                        RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.ReplExist),T_Mode,j));

            pCntRowLoop->iRow[j][PercentFuzzyUsed] =  RPT3CalcStatistics2(RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed),T_Mode,j),
                                                                         RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist),T_Mode,j));

            pCntRowLoop->iRow[j][PercentFuzzyUsed_1] =  RPT3CalcStatistics2(RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_1),T_Mode,j),
                                                                           RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_1),T_Mode,j));

            pCntRowLoop->iRow[j][PercentFuzzyUsed_2] =  RPT3CalcStatistics2(RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_2),T_Mode,j),
                                                                           RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_2),T_Mode,j));

            pCntRowLoop->iRow[j][PercentFuzzyUsed_3] =  RPT3CalcStatistics2(RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_3),T_Mode,j),
                                                                           RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_3),T_Mode,j));

            pCntRowLoop->iRow[j][PercentMachUsed] =  RPT3CalcStatistics2(RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.MachUsed),T_Mode,j),
                                                                        RPT3HandleCategories2(&(pCalcInfoTmp->docSaveHistSum.MachExist),T_Mode,j));



          } /* end for, next category if existent */

          // add categories to summary
          RPT3AddCntRow2(pCntRowLoop) ;
        } // end if fok
      } // end if shipment handler

      ppCalcInfoFieldTmp++;  // NEXT  PCALCINFO 

    } // end while



    /***************************************************/
    /*                                                 */
    /*  Build Summary  Row                             */
    /*        ------------                             */
    /*    for each file width a number of shipments    */
    /*  and Summary Table                              */
    /*      -------------                              */
    /*    for all files of a folder                    */
    /*                                                 */
    /***************************************************/

    /******************************************/
    /* alloc mem for summary table (of folder)*/
    /* and cout row                           */
    /******************************************/

    if ( fOK )
    {
      fOK = UtlAlloc((PVOID*) &pSumCntFile,0L,
                     (LONG) sizeof(SumCntFile),ERROR_STORAGE);
    } // end if fok


    if ( fOK )
    {
      pSumCntFile->PRow  = NULL;
      pSumCntFile->PSum  = NULL;
      pSumCntFile->PNext = NULL;
      strcpy(pSumCntFile->szFolder, pRpt->szFolderObjName);
      strcpy(pSumCntFile->szName , STR_RPT3_SUMMARY);
      strcpy(pSumCntFile->szLongName , STR_RPT3_SUMMARY);
      pSumCntFile->iTable = SUMMARY_TABLE;
      pSumCntFile->fHistError = FALSE;
      pSumCntFile->fHistInconsistency = FALSE;
    } // end if fok

    /***********************************/
    /* Alloc one row for total summary */
    /***********************************/

    if ( fOK )
    {
      fOK = RPT3AllocCntRow2((PVOID*) &pCntRowTotalSum);
    }// end if fok


    if ( fOK )
    {
      pSumCntFile->PSum = pCntRowTotalSum;
      // summary row
      RPT3ZeroRow2(pCntRowTotalSum);
      strcpy(pCntRowTotalSum->szID[0],STR_RPT3_SUM);
      strcpy(pCntRowTotalSum->szID[1]," ");
      strcpy(pCntRowTotalSum->szID[2]," ");
      if ( R_Mode==Details ) strcpy(pCntRowTotalSum->szID[3]," ");
      else  strcpy(pCntRowTotalSum->szID[3],STR_RPT3_SUM);
    } // end if fok

    /**************************/
    /* LOOP ALL FILES         */
    /**************************/

    if ( fOK )
    {
      pSumCntFileTmp = pSumCntFileFirst;
    } // enf if fok

    while ( pSumCntFileTmp!=NULL  && fOK )
    {

      if ( pSumCntFileTmp->fHistError )
      {
        pSumCntFile->fHistError = TRUE;
      } /* endif */
      if ( pSumCntFileTmp->fHistInconsistency )
      {
        pSumCntFile->fHistInconsistency = TRUE;
      } /* endif */

      /*****************************************************/
      /* Alloc memory for summary of shipments of one file */
      /*****************************************************/

      if ( fOK )
      {
        fOK = RPT3AllocCntRow2((PVOID*) &pCntRowSum);
      } /* endif */

      if ( fOK )
      {
        RPT3ZeroRow2(pCntRowSum);
        strcpy(pCntRowSum->szID[0],STR_RPT3_SUM);
        strcpy(pCntRowSum->szID[1]," ");
        strcpy(pCntRowSum->szID[2]," ");
        if ( R_Mode==Details ) strcpy(pCntRowSum->szID[3]," ");
        else  strcpy(pCntRowSum->szID[3],STR_RPT3_SUM);
        pCntRow = pSumCntFileTmp->PRow;
      } // end if fok


      iShipm=0;
      while ( pCntRow!=NULL && fOK )
      {
        // GQ: reset not translated count, this number is not
        //     accumulated over several shipments
        {
          int i;
          for ( i=0; i<4; i++)
          {
            pCntRowSum->iRow[i][NotXlated] = 0L;
          } /* endfor */
        }

        iShipm++;
        if ( !pCntRow->fHistInconsistency )
        {
          Rpt3SuccAddRows2(pCntRowSum,pCntRow);
        } // end if

        // insert (folder) Shipment number string
        // SHIPMENT_HANDLER

        if (pCntRow->szShipment[0]!=EOS)
        {


          if (!strcmp(pRpt->szShipmentChk, "All Shipments"))
          {
            strcpy(pCntRowSum->szShipment , "All Shipments");
          }
          else
          {
            strcpy(pCntRowSum->szShipment , pCntRow->szShipment);
          }


        } //end if


        pCntRow = pCntRow->PNext;

      } /* end while */

      if ( fOK )
      {
        int icat;

        pCntRowSum->iRow[0][NumberShip]=(float)iShipm;
        RPT3AddCntRow2(pCntRowSum);

        // summary of all files
        Rpt3SuccAddRows2(pCntRowTotalSum,pCntRowSum);

        // but zero adjusted fuzzy level
        for (icat=0; icat<4; icat++)
        {
          pCntRowTotalSum->iRow[icat][FuzzyLevel] = 0L;
        }

        /***********************************/
        /* Alloc Row                       */
        /* Add Row to Summary of all files */
        /***********************************/

        fOK = RPT3AllocCntRow2((PVOID*) &pCntRow);
      } // end if fOK

      if ( fOK )
      {
        strcpy(pCntRow->szID[0], pSumCntFileTmp->szLongName);
        strcpy(pCntRow->szID[1], " ");
        strcpy(pCntRow->szID[2], " ");
        strcpy(pCntRow->szID[3], pSumCntFileTmp->szLongName);


        // insert (folder) Shipment number string
        // SHIPMENT_HANDLER

        if (pCntRowSum->szShipment[0]!=EOS)
        {

          if (!strcmp(pRpt->szShipmentChk, "All Shipments"))
          {
            strcpy(pCntRowTotalSum->szShipment , "All Shipments");
            strcpy(pCntRow->szShipment , "All Shipments");
          }
          else
          {
            strcpy(pCntRowTotalSum->szShipment , pCntRowSum->szShipment);
            strcpy(pCntRow->szShipment , pCntRowSum->szShipment);
          }

        } //end if



        pCntRow->lRunningNo = pSumCntFileTmp->lRunningNo;
        // copy row
        RPT3CopyRow2(pCntRow,pCntRowSum);

        if ( pSumCntFile->PRow == NULL )
        {
          pSumCntFile->PRow=pCntRow;
          pCntRow->PNext = NULL;
          pCntRowAct = pCntRow;
        }
        else
        {
          pCntRowAct->PNext = pCntRow;
          pCntRowAct=pCntRow;
          pCntRowAct->PNext = NULL;
        }

        // save summary of last file
        if ( iShipm>1 )
        {
          pSumCntFileTmp->PSum = pCntRowSum;
        }
        else
        {
          pSumCntFileTmp->PSum = NULL;
          UtlAlloc ((PVOID*)&(pCntRowSum), 0L, 0L, NOMSG);
        } /* endif */

        if ( pSumCntFileTmp->PNext==NULL )
        {
          pSumCntFileTmp->PNext =  pSumCntFile;
          pSumCntFileTmp = NULL;

        }
        else
        {
          pSumCntFileTmp = pSumCntFileTmp->PNext;
        } /* end if*/

      } // end if fOK

    } /* end while */


    // add the fth row (summary of three detail rows)

    if ( fOK )
    {
      RPT3AddCntRow2(pCntRowTotalSum);
    } // end if fOK


//************************
// eliminate empty columns
// Auto Shrink
//************************


// have a look at summary row of all files of the folder

    if ( Auto_Shrink && fOK )
    {
      for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
      {
        if ( pCntRowTotalSum->iRow[3][i]==0 && i != FuzzyLevel ) Column_Switch[i]=0;  // do not shrink fuzzy level
      } /* end for */

    }  /* end if */

    // write list of active columns to XML file
    if ( fOK )
    {
      xw->WriteStartElement( "columnList" );
      for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
      {
        PSZ pszName = NULL;
        PSZ pszColGroup = NULL;

        switch ( i )
        {
          case NumberShip :       pszName = XmlComment_2[i]; break;
          case FuzzyLevel :       pszName = XmlComment_2[i]; break;
          case AutoSubst :        pszName = XmlComment_2[i]; pszColGroup = "Analysis"; break;
          case AnalAutoSubst :    pszName = XmlComment_2[i]; pszColGroup = "Analysis"; break;
          case AnalAutoSubst2 :   pszName = XmlComment_2[i]; pszColGroup = "Analysis"; break;
          case EditAutoSubst :    pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case ManualMach :       pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case ExactExist :       pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case ReplExist :        pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case FuzzyExist :       pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case FuzzyExist_1 :     pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case FuzzyExist_2 :     pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case FuzzyExist_3 :     pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case ChangedFuzzy :     pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case MachExist :        pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case NoneExist :        pszName = XmlComment_2[i]; pszColGroup = "Edit";     break;
          case NotXlated :        pszName = XmlComment_2[i]; pszColGroup = NULL;       break;
          default: break;
        } /*endswitch */

        if ( pszName )
        {
          xw->WriteStartElement( "column" );
          xw->WriteAttributeString( "name", pszName );
          xw->WriteAttributeString( "shrink", (Column_Switch[i] == 0) ? "Yes" : "No" );
          if ( pszColGroup )
          {
            xw->WriteAttributeString( "group", pszColGroup );
          } /* endif */
          xw->WriteEndElement(); // "column"
        } /* endif */
      } /* end for */
      xw->WriteEndElement();             //  "columnList" 
    } /* endif */

/*****************************/
/*                           */
/*    final fact sheet       */
/*                           */
/*****************************/


    /******************************************/
    /* alloc mem for final fact sheet         */
    /******************************************/

    if ( fOK )
    {

      fOK = UtlAlloc((PVOID*) &pFactSheet,0L,
                     (LONG) sizeof(SumCntFile),ERROR_STORAGE);
    } // end if fOK

    if ( fOK )
    {
      pSumCntFile->PNext = pFactSheet;

      pFactSheet->PRow  = NULL;
      pFactSheet->PSum  = NULL;
      pFactSheet->PNext = NULL;
      strcpy(pFactSheet->szFolder, pRpt->szFolderObjName);
      strcpy(pFactSheet->szName , STR_RPT3_FACT);
      strcpy(pFactSheet->szLongName , STR_RPT3_FACT);
      pFactSheet->iTable = FACT_SHEET;
      pFactSheet->fHistError = pSumCntFile->fHistError;
      pFactSheet->fHistInconsistency = pSumCntFile->fHistInconsistency;
    }// end if fOK

    /***********************************/
    /* Alloc Rows                      */
    /* summary of summary folder table */
    /***********************************/

    // Actual Words
    //*************

    if ( fOK )
    {
      fOK = RPT3AllocCntRow2((PVOID*) &pCntRow1);
    }// end if fOK

    if ( fOK )
    {
      pFactSheet->PRow = pCntRow1;
      strcpy(pCntRow1->szID[0], "ActualWords" );
      strcpy(pCntRow1->szID[1], " ");
      strcpy(pCntRow1->szID[2], " ");
      strcpy(pCntRow1->szID[3], STR_RPT3_ACTUAL_WORDS);

      // copy row
      RPT3CopyRow2(pCntRow1,pSumCntFile->PSum);
    } // end if fOK



    // insert (folder) Shipment number string
    // SHIPMENT_HANDLER
    if (pSumCntFile->PSum->szShipment[0]!=EOS)
    {
      strcpy(pCntRow1->szShipment , pSumCntFile->PSum->szShipment);
    } //end if

    // Complexity Factor
    if ( fOK )
    {
      fOK = RPT3AllocCntRow2((PVOID*) &pCntRow2);
    }

    if ( fOK )
    {
      pCntRow1->PNext = pCntRow2;
      strcpy(pCntRow2->szID[0], "ComplexityFactor" );
      strcpy(pCntRow2->szID[1], STR_RPT3_FACTOR);
      strcpy(pCntRow2->szID[2], " ");
      strcpy(pCntRow2->szID[3], STR_RPT3_MEAN_COMP);

      // copy row
      RPT3CopyRow2(pCntRow2,pComplexityFactor);

      // build summary row of factor by means
      //RPT3MeanFactor2(pCntRow2);
    }// end if fOK

    // Complexity weighted words
    if ( fOK )
    {
      fOK = RPT3AllocCntRow2((PVOID*) &pCntRow3);
    }

    if ( fOK )
    {
      pCntRow2->PNext = pCntRow3;
      strcpy(pCntRow3->szID[0], "WeightedWords" );
      strcpy(pCntRow3->szID[1], STR_RPT3_FACTORT);
      strcpy(pCntRow3->szID[2], STR_RPT3_ACTUAL_WORDS);
      strcpy(pCntRow3->szID[3], STR_RPT3_COMPLEXITY_WORDS);

      RPT3MultRows2(pCntRow1,pCntRow2,pCntRow3);

      // add the fth row (summary of three detail rows)
      RPT3AddCntRow2(pCntRow3);   // Summary

      // Build Summary Rows
      myRPT3UpdateSummaryRows(pCntRow3);

      // build summary row of factor by means
      RPT3MeanFactor2(pCntRow1, pCntRow2, pCntRow3);
    } // end if fok

    // Pay Factor
    if ( fOK )
    {
      fOK = RPT3AllocCntRow2((PVOID*) &pCntRow4);
    } // end if fOK

    if ( fOK )
    {
      pCntRow3->PNext = pCntRow4;
      strcpy(pCntRow4->szID[0], "PayFactor");
      strcpy(pCntRow4->szID[1], STR_RPT3_FACTOR);
      strcpy(pCntRow4->szID[2], " ");
      strcpy(pCntRow4->szID[3], STR_RPT3_MEAN_PAY);

      // copy row
      RPT3CopyRow2(pCntRow4,pPayFactor);

      // build summary row of factor by means
      //RPT3MeanFactor2(pCntRow4);
    }

    // PayFactor weighted words
    if ( fOK )
    {
      fOK = RPT3AllocCntRow2((PVOID*) &pCntRow5);
    }

    if ( fOK )
    {
//      pFactSheet->PSum  = pCntRow5;
      pCntRow4->PNext = pCntRow5;
      strcpy(pCntRow5->szID[0], "PayWords" );
      strcpy(pCntRow5->szID[1], STR_RPT3_FACTORT);
      strcpy(pCntRow5->szID[2], STR_RPT3_COMPLEXITY_WORDS);
      strcpy(pCntRow5->szID[3], STR_RPT3_PAY_WORDS);

      RPT3MultRows2(pCntRow3,pCntRow4,pCntRow5);

      // add the fth row (summary of three detail rows)
      RPT3AddCntRow2(pCntRow5);

      // Build Summary Rows
      myRPT3UpdateSummaryRows(pCntRow5);

      // build means  of factors
      myRPT3UpdateSummaryRows2(pCntRow2);
      myRPT3UpdateSummaryRows2(pCntRow4);

      // build summary row of factor by means
      RPT3MeanFactor2(pCntRow3,pCntRow4, pCntRow5);

      // NO DO THE FINAL COUNTING PAYABLE WORDS
      Payable_Words = pCntRow5->iRow[3][AnalAutoSubst] +
                      pCntRow5->iRow[3][AnalAutoSubst2] +
                      pCntRow5->iRow[3][EditAutoSubst] +
                      pCntRow5->iRow[3][ExactExist] +
                      pCntRow5->iRow[3][ReplExist] +
                      pCntRow5->iRow[3][FuzzyExist_1] +
                      pCntRow5->iRow[3][FuzzyExist_2] +
                      pCntRow5->iRow[3][FuzzyExist_3] +
                      pCntRow5->iRow[3][MachExist] +
                      pCntRow5->iRow[3][NoneExist];

      Total_Standard = Payable_Words / Standard_Factor;

      Total_Pay  =  Total_Standard  * Pay_per_Word;
    } // end if fOK

    /***********************************************************/
    /* tables                                                  */
    /* to Output                                               */
    /***********************************************************/
    if ( fOK )
    {
      pSumCntFileTmp = pSumCntFileFirst;
    }

    xw->WriteStartElement( "documents" );

    while ( fOK && pSumCntFileTmp!=NULL )
    {

      // Type of the Table to Print
      iTable_Type =  pSumCntFileTmp->iTable;

      // for XML output process all tables
      {
        if ( iTable_Type == SUMMARY_TABLE )
        {
          xw->WriteEndElement(); // "documents"
          xw->WriteStartElement( "summary" );
        }
        else if ( iTable_Type == FACT_SHEET )
        {
          xw->WriteEndElement(); // "documents" / "summary"
          xw->WriteStartElement( "factsheet" );
        } /* endif */

        strcpy(szLongFileName,pSumCntFileTmp->szLongName);


        // try to split Path information
        pszLongFileName = strrchr(szLongFileName,'\\');

        if ( pszLongFileName )
        {
          *pszLongFileName = EOS;
          pszLongFileName ++;
        } /* endif */

        if ( iTable_Type == TABLE )
        {
          xw->WriteStartElement( "document" );

          if ( pszLongFileName )
          {
            xw->WriteAttributeString( "path", szLongFileName );
            xw->WriteAttributeString( "nameLong", pszLongFileName );
          }
          else
          {
            xw->WriteAttributeString( "nameLong", szLongFileName );
          } // end if
          xw->WriteAttributeString( "nameShort", pSumCntFileTmp->szName );
          xw->WriteStartAttribute( "id" );
          xw->WriteInt( pSumCntFileTmp->lRunningNo );
          xw->WriteEndAttribute();

          // add memory to output for pre-analysis files
          if ( pRpt->usReport  == PRE_ANALYSIS_REPORT )
          {
            ULONG ulErrorInfo = 0;
            HPROP hpropFolder = OpenProperties( pRpt->szFolderObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo );

            if ( hpropFolder != NULL ) 
            {
              PPROPFOLDER ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
              xw->WriteStartAttribute( "memory" );
              xw->WriteString( ppropFolder->szMemory );
              if ( ppropFolder->aLongMemTbl[0][0] != EOS )
              {
                int i = 0;
                while ( (i < MAX_NUM_OF_READONLY_MDB) && (ppropFolder->aLongMemTbl[i][0] != EOS) ) 
                {
                  xw->WriteString( "," );
                  xw->WriteString( ppropFolder->aLongMemTbl[i] );
                  i++;
                } /*endwhile */
              }
              else
              {
                xw->WriteString( "," );
                xw->WriteString( ppropFolder->MemTbl );
              } /* endif */
              xw->WriteEndAttribute();
              CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
            } /* end if */
          } /* end if */

        } /* endif */

        /* print rows */
        pCntRow = pSumCntFileTmp->PRow;

        while ( fOK && pCntRow != NULL )
        {
          if ( iTable_Type == FACT_SHEET )
          {
            xw->WriteStartElement( "group" );
            xw->WriteAttributeString( "name", pCntRow->szID[0] );
            pCntRow->szShipment[0] = EOS;      // suppress shipment number
          }
          else if ( iTable_Type == SUMMARY_TABLE )
          {
            xw->WriteStartElement( "document" );
            xw->WriteAttributeString( "name", pCntRow->szID[0] );
            xw->WriteStartAttribute( "id" );
            xw->WriteInt( pCntRow->lRunningNo );
            xw->WriteEndAttribute();
            pCntRow->szShipment[0] = EOS;      // suppress shipment number
          } /* endif */
          S_Mode = pRpt->usColumns[0]; 
          RptCalcReportRow(pRpt, pCntRow,R_Mode,S_Mode,&Column_Switch, xw, iTable_Type );  //zsNumber
          pCntRow = pCntRow->PNext;
          if ( (iTable_Type == FACT_SHEET) || (iTable_Type == SUMMARY_TABLE ) )
          {
            xw->WriteEndElement(); // "group" / "document"
          } /* endif */
        } /* end while  each row*/

        // summary row
        if ( fOK )
        {
          pCntRow = pSumCntFileTmp->PSum;
          if ( pCntRow != NULL )
          {
            if ( iTable_Type == FACT_SHEET )
            {
              xw->WriteStartElement( "group" );
              xw->WriteAttributeString( "name", pCntRow->szID[0] );
              pCntRow->szShipment[0] = EOS;      // suppress shipment number
            }
            else if ( iTable_Type == SUMMARY_TABLE )
            {
              xw->WriteStartElement( "document" );
              xw->WriteAttributeString( "name", pCntRow->szID[0] );
              xw->WriteStartAttribute( "id" );
              xw->WriteInt( pCntRow->lRunningNo );
              xw->WriteEndAttribute();
              pCntRow->szShipment[0] = EOS;      // suppress shipment number
            } /* endif */
            S_Mode=1;
            RptCalcReportRow(pRpt, pCntRow,R_Mode,S_Mode,&Column_Switch, xw, iTable_Type ); // SUM
            if ( (iTable_Type == FACT_SHEET) || (iTable_Type == SUMMARY_TABLE ) )
            {
              xw->WriteEndElement(); // "group" / "document"
            } /* endif */
          } /* endif */
        } /* endif */

        pSumCntFileTmp = pSumCntFileTmp->PNext;
      } 

      if ( iTable_Type == TABLE )
      {
        xw->WriteEndElement(); // "document"
      } /* endif */

    } // end while
    xw->WriteEndElement(); // "documents" / "summary" / "factsheet" 

    // final fact sheet summary                 
    {
      char szNumber[30];

      xw->WriteStartElement( "finalSummary" );
      if ( pRpt->szShipmentChk[0] ) xw->WriteAttributeString( "shipment", pRpt->szShipmentChk );  
      sprintf( szNumber, "%2.2f", Payable_Words );
      xw->WriteAttributeString( "payableWords", szNumber );  
      sprintf( szNumber, "%2.2f", Total_Standard );
      if ( pRpt->usStandard == RPT_LINE )
      {
        xw->WriteAttributeString( "standardLines", szNumber );  
      }
      else if ( pRpt->usStandard == RPT_PAGE )
      {
        xw->WriteAttributeString( "standardPages", szNumber );  
      }
      xw->WriteAttributeString( "localCurrency", pRpt->szCurrency );
      sprintf( szNumber, "%2.2f", Total_Pay );
      xw->WriteAttributeString( "totalPay", szNumber );
      xw->WriteEndElement(); // "FinalSummary" 
    }


    // document list
    if ( pRpt->usColumns4[1] )
    {
      xw->WriteStartElement( "documentList" ); 
      pSumCntFileTmp = pSumCntFileFirst;
      ulIndex = 0;

      while ( pSumCntFileTmp != NULL )
      {
        if ( pSumCntFileTmp->lRunningNo > 0 )
        {
          // fetch LongFileName
          strcpy( szLongFileName, pSumCntFileTmp->szLongName );

          // try to split Path information
          pszLongFileName = strrchr(szLongFileName,'\\');

          if ( pszLongFileName )
          {
            *pszLongFileName = EOS;
            pszLongFileName ++;
          } /* endif */

          xw->WriteStartElement( "document" );
          xw->WriteStartAttribute( "num" );
          xw->WriteInt( pSumCntFileTmp->lRunningNo );
          xw->WriteEndAttribute();
          if ( pszLongFileName )
          {
            xw->WriteElementString( "path", szLongFileName );
            xw->WriteElementString( "name", pszLongFileName );
          }
          else
          {
            xw->WriteElementString( "name", szLongFileName );
          } // end if
          xw->WriteEndElement(); // "document" 
          } /* endif */

          pSumCntFileTmp = pSumCntFileTmp->PNext;
        }  // end while
      xw->WriteEndElement(); // "documentList" 
    } // end if

    // fill our binary calulation report data and write it to disk
    if ( fOK )
    {
      memset( (PVOID)&pRpt->CalcRepBinData, 0, sizeof(pRpt->CalcRepBinData) );

       // actual words per column
      pRpt->CalcRepBinData.ulAnaAustoSubst  = (ULONG)pCntRow1->iRow[3][AnalAutoSubst];
      pRpt->CalcRepBinData.ulAnaAustoSubst2 = (ULONG)pCntRow1->iRow[3][AnalAutoSubst2];;
      pRpt->CalcRepBinData.ulEditAustoSubst = (ULONG)pCntRow1->iRow[3][EditAutoSubst];
      pRpt->CalcRepBinData.ulEditExact      = (ULONG)pCntRow1->iRow[3][ExactExist];
      pRpt->CalcRepBinData.ulEditReplace    = (ULONG)pCntRow1->iRow[3][ReplExist];
      pRpt->CalcRepBinData.ulEditFuzzy5070  = (ULONG)pCntRow1->iRow[3][FuzzyExist_1];
      pRpt->CalcRepBinData.ulEditFuzzy7190  = (ULONG)pCntRow1->iRow[3][FuzzyExist_2];
      pRpt->CalcRepBinData.ulEditFuzzy91    = (ULONG)pCntRow1->iRow[3][FuzzyExist_3];
      pRpt->CalcRepBinData.ulEditMachine    = (ULONG)pCntRow1->iRow[3][MachExist];
      pRpt->CalcRepBinData.ulEditManual     = (ULONG)pCntRow1->iRow[3][NoneExist];
      pRpt->CalcRepBinData.ulNotTranslated  = (ULONG)pCntRow1->iRow[3][NotXlated];

      pRpt->CalcRepBinData.ulPayable        = (ULONG)Payable_Words;

      pRpt->CalcRepBinData.flPayable = Payable_Words;

      strcpy( pRpt->CalcRepBinData.szProfile, pRpt->szDisplayProfileName );

      // write the file
      UtlMakeEQFPath( pRpt->szWorkString, pRpt->szFolderObjName[0], SYSTEM_PATH, pRpt->szFolderName );
      strcat( pRpt->szWorkString, "\\" );
      strcat( pRpt->szWorkString, CALREPORTDATAFILE );
      UtlWriteFile( pRpt->szWorkString, sizeof(pRpt->CalcRepBinData), &(pRpt->CalcRepBinData), FALSE );
    }

    // Garbage collection            
    if ( fOK )
    {
      pSumCntFile = pSumCntFileFirst;
    }

    while ( fOK && pSumCntFile!=NULL )
    {
      /* next */
      pSumCntFileTmp = pSumCntFile->PNext;

      pCntRow =  pSumCntFile->PRow;
      while ( pCntRow!=NULL )
      {
        pCntRow1 = pCntRow->PNext;
        /* free Row*/
        UtlAlloc ((PVOID*)&(pCntRow), 0L, 0L, NOMSG);
        /*next*/
        pCntRow = pCntRow1;
      } // end while

      pCntRow =  pSumCntFile->PSum;
      while ( pCntRow!=NULL )
      {
        pCntRow1 = pCntRow->PNext;
        /* free Row*/
        UtlAlloc ((PVOID*)&(pCntRow), 0L, 0L, NOMSG);
        /*next*/
        pCntRow = pCntRow1;
      } // end while

      /* free SumCntFile*/
      UtlAlloc ((PVOID*)&(pSumCntFile), 0L, 0L, NOMSG);

      /* next*/
      pSumCntFile = pSumCntFileTmp;
    } // end while
  } // end for iRunShipm

  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////





  return fOK;

}  // end of RptReport3


// function to correct size values in older history log records (in these records
// the usSize value was not set correctly)
void HistLogCorrectRecSizesX( PHISTLOGRECORD pRecord )
{
  // only process older records ....
  if ( !pRecord->fLongNameRecord )
  {
    USHORT usTotalSize, usAddInfoSize, usNewSize;
    usTotalSize = pRecord->usSize;
    usAddInfoSize = pRecord->usAddInfoLength;
    if ( usAddInfoSize != usTotalSize - sizeof(HISTLOGRECORD) )
    {
      switch ( pRecord->Task )
      {
        case DOCIMPORT_LOGTASK :
          usNewSize = sizeof(DOCIMPORTHIST);
          break;
        case DOCIMPORT_LOGTASK2 :
          usNewSize = sizeof(DOCIMPORTHIST2); //SHIPMENT_HANDLER
          break;

        case ANALYSIS_LOGTASK :
          usNewSize = sizeof(ANALYSISHIST);
          break;
        case DOCDELETE_LOGTASK :
          usNewSize = 0;
          break;
        case AUTOMATICSUBST_LOGTASK :
        case DOCIMPNEWTARGET_LOGTASK :
        case DOCSAVE_LOGTASK  :
          usNewSize = sizeof(DOCSAVEHIST);
          break;
        case DOCEXPORT_LOGTASK :
          usNewSize = sizeof(DOCEXPORTHIST);
          break;
        case DOCPROP_LOGTASK :
          usNewSize = sizeof(DOCPROPHIST);
          break;
        case FOLPROP_LOGTASK :
          usNewSize = sizeof(FOLPROPHIST);
          break;
        case FOLPROP_LOGTASK2 :
          usNewSize = sizeof(FOLPROPHIST2);
          break;
        case FOLPROPSHIPMENT_LOGTASK :
          usNewSize = sizeof(FOLPROPHISTSHIPMENT); //SHIPMENT_HANDLER
          break;

        case LONGNAME_LOGTASK :
          usNewSize = sizeof(LONGNAMEHIST);
          break;
        case DOCIMPNEWTARGET_LOGTASK2 :
        case DOCSAVE_LOGTASK2 :
        case DOCAPI_LOGTASK :
          usNewSize = sizeof(DOCSAVEHIST2);
          break;
        case VERSION_LOGTASK :
          usNewSize = sizeof(VERSIONHIST);
          break;
        default:
          usNewSize = usTotalSize - sizeof(HISTLOGRECORD);
      } /* endswitch */
      pRecord->usSize = usNewSize + sizeof(HISTLOGRECORD);
      pRecord->usAddInfoLength = usNewSize;
    } /* endif */
  } /* endif */
} /* end of function HistLogCorrectRecSizesX */


VOID RptFreeMemoryX (PRPT pRpt)
{
  ULONG      ulIndex = 0;         // tmp index for all arrays
  PPALLINFO  ppAllInfoFieldTmp;   // pointer to ALLINFO array
  PPCALCINFO ppCalcInfoFieldTmp;  // pointer to CALCINFO array


  // free all ALLINFOs and PALLINFOs
  if ( pRpt->ppAllInfoField )
  {
    ulIndex = 0;
    ppAllInfoFieldTmp = pRpt->ppAllInfoField;   // set tmp pointer

    // loop over all PALLINFOs
    while ( ulIndex++ < pRpt->ulAllInfoRecords )
    {
      // free all ALLINFOs
      UtlAlloc ((PVOID*)&(*ppAllInfoFieldTmp++), 0L, 0L, NOMSG);
    } // end while

    // free PALLINFO array
    UtlAlloc ((PVOID*)&(pRpt->ppAllInfoField), 0L, 0L, NOMSG);
  } // end if

  // free all CALCINFOs and PCALCINFOs
  if ( pRpt->ppCalcInfoField )
  {
    ulIndex = 0;
    ppCalcInfoFieldTmp = pRpt->ppCalcInfoField;   // set tmp pointer

    // loop over all PCALCINFOs
    while ( ulIndex++ < pRpt->ulCalcInfoRecords )
    {
      // free all CALCINFOs
      UtlAlloc ((PVOID*)&(*ppCalcInfoFieldTmp++), 0L, 0L, NOMSG);
    } // end while

    // free PCALCINFO array
    UtlAlloc ((PVOID*)&(pRpt->ppCalcInfoField), 0L, 0L, NOMSG);
  } // end if

  // free OUTMRI of pOutputMris
  if ( pRpt->pOutputMris )
  {
    // free OUTMRI array
    UtlAlloc ((PVOID*)&(pRpt->pOutputMris), 0L, 0L, NOMSG);
  } // end if

} // end of RptFreeMemoryX

BOOL RPTLT_CRITERIASUM2
(
CRITERIASUM    record,
CRITERIASUMEX  check
)
{
  if
     (
     record.SimpleSum.usNumSegs   <  check.SimpleSum.ulNumSegs   ||
     record.SimpleSum.ulSrcWords  <  check.SimpleSum.ulSrcWords  ||
     record.SimpleSum.ulModWords  <  check.SimpleSum.ulModWords  ||
     record.SimpleSum.ulTgtWords  <  check.SimpleSum.ulTgtWords  ||

     record.MediumSum.usNumSegs   <  check.MediumSum.ulNumSegs   ||
     record.MediumSum.ulSrcWords  <  check.MediumSum.ulSrcWords  ||
     record.MediumSum.ulModWords  <  check.MediumSum.ulModWords  ||
     record.MediumSum.ulTgtWords  <  check.MediumSum.ulTgtWords  ||

     record.ComplexSum.usNumSegs  <  check.ComplexSum.ulNumSegs  ||
     record.ComplexSum.ulSrcWords <  check.ComplexSum.ulSrcWords ||
     record.ComplexSum.ulModWords <  check.ComplexSum.ulModWords ||
     record.ComplexSum.ulTgtWords <  check.ComplexSum.ulTgtWords

     )
  {
    return TRUE;
  }
  else
  {
    return FALSE ;
  } // end if


} // end of function RPTLT_CRITERIASUM2


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptMakeCalculationRecordsX                                |
//+----------------------------------------------------------------------------+
//|Function call:    RptMakeCalculationRecordsX (pRpt)                         |
//+----------------------------------------------------------------------------+
//|Description:      This function builds the calculating info of the selected |
//|                  documents, and stores it in a dynamic increasing array    |
//|                  of PCALCINFOs                                             |
//+----------------------------------------------------------------------------+
//|Parameters:       PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    allocate memory for PCALCINFOs                            |
//|                  loop over all ALLINFOs                                    |
//|                    reallocate memory for PCALCINFOs, if necessary          |
//|                    if document of actual ALLINFO record is not equal to    |
//|                     actual document in process                             |
//|                      allocate memory for CALCINFO structure                |
//|                      update actual document in RPT and CALCINFO            |
//|                      store address in PCALCINFO array                      |
//|                    build summary depending on task ID                      |
//|                  if option with summary selected                           |
//|                    reallocate memory for PCALCINFOs, if necessary          |
//|                    allocate memory for CALCINFO summary                    |
//|                    store address in PCALCINFO array                        |
//|                    loop over all CALCINFOs                                 |
//|                      build summary for each CALCINFO record depending on   |
//|                       SUMPERCLASS and CRITERIASUM                          |
//|                      build summary of all CALCINFOs depending on task ID   |
//+----------------------------------------------------------------------------+
BOOL RptMakeCalculationRecordsX (PRPT pRpt)
{
  BOOL           fOK = FALSE;         // error indicator
  PVOID          pArray = NULL;       // pointer to allocated memory
  PPCALCINFO     ppCalcInfoFieldTmp = NULL;  // pointer to PCALCINFO array
  PPALLINFO      ppAllInfoFieldTmp = NULL;   // pointer to PALLINFO array
  PALLINFO       pAllInfoFieldTmp = NULL;    // pointer tp PALLINFOs
  ULONG          ulAllocEntries = 0;        // number of allocated CALCIFOs entries
  PCALCINFO      pCalcInfoTmp = NULL;        // pointer to CALCINFO structure
  PCALCINFO      pCalcInfoSum = NULL;        // pointer to CALCINFO structure
  PDOCSAVEHISTEX pDocInfoAnalBase = NULL;   // CALCINFO structure with base info
  BOOL           fDocImport = FALSE;  // indicator of document import
  ULONG          usIndex = 0;         // counter
  time_t         lTime;               // current time/date
  PVERSIONHIST   pVersionHistTmp = NULL;     // version of eqfdll, eqfd
  PDOCSAVEHIST   pDocSaveHistTmp = NULL;     // pointer to DOCSAVEHIST of ALLINFOs (INPUT)
  PDOCSAVEHIST2  pDocSaveHistTmp2 = NULL;    // pointer to DOCSAVEHIST2 (enlarged structure) of ALLINFOs
  PDOCSAVEHIST3  pDocSaveHistTmp3 = NULL;    // pointer to DOCSAVEHIST3
  PDOCSAVEHISTEX pDocSaveHistSumTmp = NULL;  // pointer to DOCSAVEHISTEX of CALCINFOs
  PDOCSAVEHISTEX pDocSaveCopy;        // for security check
  PCRITERIASUMEX pDocCriteriaSumTmp = NULL;  // pointer to CRITERIASUM of CALCINFOs
  PDOCSAVEHISTEX pDocSaveHistSumSum = NULL;  // pointer to DOCSAVEHISTEX of CALCINFO summary
  PCRITERIASUMEX pDocCriteriaSumSum = NULL;  // pointer to CRITERIASUM of CALCINFO summary
  BOOL           fFound;              // Document found
  //TC summary counting report
  CRITERIASUM    count_null;
  CRITERIASUM    count_minus;
  BOOL           fHistError=FALSE;     // error in Histlog
  BOOL           fHistInconsistency=FALSE; // inconsistency in HistLog
  BOOL           fDocDelete=FALSE;    // indicator of document delete
  BOOL           fSecondAnalysis=FALSE;  // second analysis
  CHAR           SegSource[500];      // fully qualified Segmented Source
  ULONG          ulWords = 0;         // # source words
  INT            nDocImport = 0;          // Number of Document Imports
  BOOL           fAnalyzed = FALSE;
  BOOL           fShipmForced = FALSE;
  PDOCIMPORTHIST2 pDocImport2;
  BOOL           fAnaAutoSubstAvail = FALSE; // TRUE = matches substituted by analysis
  CRITERIASUM    AnaAutoSubstSave;     // buffer fo saved AnaAutoSubst counts
  BOOL           fInternalImport;      // type of document import internal/external

  memset(&count_null,0,sizeof(count_null));
  memset(&count_minus,0xFF,sizeof(count_null));

  memset(&AnaAutoSubstSave,0,sizeof(AnaAutoSubstSave));

  // reset actual document name,
  // to alloc space for the first time
  strcpy(pRpt->pszActualDocument,"");

  // Alloc space for old data
  fOK = UtlAlloc ((PVOID*)&pDocSaveCopy, 0L, sizeof(DOCSAVEHISTEX), ERROR_STORAGE);

  if ( fOK )  // RESET
  {
    memset( pDocSaveCopy, 0, sizeof( DOCSAVEHISTEX )  );
  } // end if



  // alloc memory for PCALCINFO array, initial size INIT_SIZE_C
  if ( fOK )
  {
    ulAllocEntries = INIT_SIZE_C;
    fOK = UtlAlloc ((PVOID*)&pArray, 0L, (ULONG)(ulAllocEntries * sizeof(PCALCINFO)), ERROR_STORAGE);
  } // end if

  if ( fOK )
  {
    fOK = UtlAlloc ((PVOID*)&pDocInfoAnalBase, 0L, sizeof(DOCSAVEHISTEX),
                    ERROR_STORAGE);
    memset( pDocInfoAnalBase, 0, sizeof( DOCSAVEHISTEX )  );

  } /* endif */


  if ( fOK )
  {
    pRpt->ppCalcInfoField = (PPCALCINFO)pArray;  // store memory address in RPT
    ppCalcInfoFieldTmp = (PPCALCINFO)pArray;     // set tmp pointer to PCALCINFOs
    ppAllInfoFieldTmp = pRpt->ppAllInfoField;    // set tmp pointer to PALLINFOs
    pAllInfoFieldTmp = *ppAllInfoFieldTmp;                 // set tmp pointer to ALLINFOs
  } // end if

  if ( fOK )
  {
    // -----------------------
    // loop over all PALLINFOs
    // -----------------------

    while ( pRpt && fOK && (usIndex++ < pRpt->ulAllInfoRecords) )
    {
      // check, if array is still large enough
      if ( pRpt->ulCalcInfoRecords >= ulAllocEntries )
      {
        // realloc memory for PCALCINFO array
        fOK = UtlAlloc ((PVOID*)&pArray,
                        (ULONG)(ulAllocEntries * (ULONG)sizeof(PCALCINFO)),
                        (ULONG)(INIT_SIZE_C + ulAllocEntries) * (ULONG)sizeof(PCALCINFO),
                        ERROR_STORAGE);

        ulAllocEntries += INIT_SIZE_C;
        if ( fOK )
        {
          pRpt->ppCalcInfoField = (PPCALCINFO)pArray;    // store memory address in RPT
          ppCalcInfoFieldTmp = (PPCALCINFO)pArray;       // set tmp pointer
          ppCalcInfoFieldTmp += pRpt->ulCalcInfoRecords; // update tmp pointer
        } // end if
      } // end if

      //
      // Consistency Checker , Force new shipment
      // in case of reset data in Segmented source x=....
      //
      //----------------------------------------------------
      //  set tmp pointer to DOCSAVEHIST of PALLINFO         (source)
      //        pDocSaveHistTmp = &(pAllInfoFieldTmp->variablePart.DocSave);



      fShipmForced = FALSE;

      if ( pRpt->usColumns4[2] )
      {
        // -------------------
        // LOST DATA:
        // FORCE NEW SHIPMENT;
        // -------------------

        if ( pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK )
        {

          pDocSaveHistTmp = &(pAllInfoFieldTmp->variablePart.DocSave);

          if ( RPTLT_CRITERIASUM2(pDocSaveHistTmp->ExactExist, pDocSaveCopy->ExactExist) )
          {
            fShipmForced = TRUE;
          } // end if

        }
        else if ( pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK2 ||
                  pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK3)
        {

          if(pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK2)  // Is it really true to copy DOCSAVEHIST2/3 to DOCSAVEHIST??
             pDocSaveHistTmp = (PDOCSAVEHIST)&(pAllInfoFieldTmp->variablePart.DocSave2);
          else
             pDocSaveHistTmp = (PDOCSAVEHIST)&(pAllInfoFieldTmp->variablePart.DocSave3);

          if
             (
             RPTLT_CRITERIASUM2(pDocSaveHistTmp->ExactExist, pDocSaveCopy->ExactExist) ||
             RPTLT_CRITERIASUM2(pDocSaveHistTmp->ReplExist, pDocSaveCopy->ReplExist) ||
             RPTLT_CRITERIASUM2(pDocSaveHistTmp->FuzzyExist, pDocSaveCopy->FuzzyExist) ||
             RPTLT_CRITERIASUM2(pDocSaveHistTmp->MachExist, pDocSaveCopy->MachExist) ||
             RPTLT_CRITERIASUM2(pDocSaveHistTmp->NoneExist, pDocSaveCopy->NoneExist)
             )
          {
            fShipmForced = TRUE;
          } // end if
        } // end if
      } // end if

      // --------------------------------------------------
      // TC summary counting report
      // check, if document of ALLINFO record is equal to
      // actual document in process or document deleted for
      // new shipment
      //
      // NEW SHIPMENT
      // --------------------------------------------------

      fFound=FALSE;
      fFound= (_stricmp( pAllInfoFieldTmp->szLongName,
                       pRpt->pszActualDocument) == 0);

      //
      // HANDLING OF NEW SHIPMENT      (SHIPMENT_HANDLER)
      // ------------------------
      //
      // DOCIMPNEWTARGET_LOGTASK means new shipment as well as
      //                               folder merge
      //
      //   - new shipment only in case of document delete (source and not only target
      //     changed)
      //   - shipment per document only
      //

      // get type of import
      fInternalImport = FALSE;
      if ( pAllInfoFieldTmp->histLogRecord.Task == DOCIMPORT_LOGTASK )
      {
        PDOCIMPORTHIST pDocImportTmp;
        pDocImportTmp = &(pAllInfoFieldTmp->variablePart.DocImport);
        if ( (pDocImportTmp->sType == INTERN_SUBTYPE) ||
             (pDocImportTmp->sType == FOLDER_SUBTYPE) )
        {
          fInternalImport = TRUE;
        } /* endif */
      }
      else if ( pAllInfoFieldTmp->histLogRecord.Task == DOCIMPORT_LOGTASK2 )
      {
        pDocImport2 = &(pAllInfoFieldTmp->variablePart.DocImport2);
        if ( (pDocImport2->sType == INTERN_SUBTYPE) ||
             (pDocImport2->sType == FOLDER_SUBTYPE) )
        {
          fInternalImport = TRUE;
        } /* endif */
      } /* endif */

      if ( fOK && ( !fFound ||
                    fShipmForced  ||
                    (fDocDelete &&  (pAllInfoFieldTmp->histLogRecord.Task == DOCIMPORT_LOGTASK) && !fInternalImport ) ||
                    (fDocDelete &&  (pAllInfoFieldTmp->histLogRecord.Task == DOCIMPORT_LOGTASK2) && !fInternalImport ) ||
                    //    pAllInfoFieldTmp->histLogRecord.Task == DOCIMPNEWTARGET_LOGTASK ||
                    //    pAllInfoFieldTmp->histLogRecord.Task == DOCIMPNEWTARGET_LOGTASK2 ||
                    (pAllInfoFieldTmp->histLogRecord.Task == HISTDATA_INVALID_LOGTASK)
                  ) )
      {


        //
        // update lost information
        // at each new shipment, if necessary
        //

        if ( fOK && fSecondAnalysis )
        {
          // Reset pDocInfoAnalBase
          // Merge pDocInfoAnalBase into pDocSaveHistSumTmp
          // Data lost due to second analysis
          MergeResults( pDocSaveHistSumTmp, pDocInfoAnalBase, TRUE );
        } /* endif */


        //
        // reset Data
        //

        nDocImport = 0;
        fAnalyzed = FALSE;
        memset( pDocInfoAnalBase, 0, sizeof( DOCSAVEHISTEX )  );
        fSecondAnalysis = FALSE;




        if ( fOK )  // RESET Consistency saved data
        {
          memset( pDocSaveCopy, 0, sizeof( DOCSAVEHISTEX )  );

        } // end if

        // RESET AnalAutoSubst buffer
        {
          fAnaAutoSubstAvail = FALSE;
          memset( &AnaAutoSubstSave, 0, sizeof(AnaAutoSubstSave)  );
        } // end if


        // ------------------------------------
        // for new document, do total Wordcount
        // to check plausibility of entries
        // ------------------------------------

        if ( !fFound )
        {

          strcpy(SegSource, pRpt->szFolderObjName);
          strcat(SegSource, "\\SSOURCE\\");
          strcat(SegSource, pAllInfoFieldTmp->histLogRecord.szDocName);

//          if ( pRpt->usColumns4[0] )
//          {
//            ulWords = RPTCheckSourceWords( SegSource );
//          }
//          else
          {
            ulWords = 0xFFFFFFFF;
          } // end if

        } /* endif */

        // documents are not equal
        // alloc memory for CALCINFO structure, new shipment
        fOK = UtlAlloc ((PVOID*)&pCalcInfoTmp, 0L, sizeof(CALCINFO),
                        ERROR_STORAGE);

        // resetset histlog_error for new document
        if ( !fFound ) fHistError = FALSE;
        if ( !fFound ) fHistInconsistency = FALSE;

        if ( fOK )
        {
          // set document name in RPT and CALCINFO
          strcpy (pRpt->pszActualDocument,
                  pAllInfoFieldTmp->szLongName );
          strncpy (pCalcInfoTmp->szDocument, pAllInfoFieldTmp->histLogRecord.szDocName, sizeof(pCalcInfoTmp->szDocument)-1 );
          pCalcInfoTmp->szDocument[sizeof(pCalcInfoTmp->szDocument)-1] = NULC;
          strcpy (pCalcInfoTmp->szLongName,
                  pAllInfoFieldTmp->szLongName );

          // ------ produces errors in calculating report
          // strcpy (pCalcInfoTmp->szLongName,
          // pAllInfoFieldTmp->szLongName );

          //
          // SHIPMENT_HANDLER: Set shipment number according folder shipment number if given
          //
          if (pAllInfoFieldTmp->histLogRecord.Task == DOCIMPORT_LOGTASK2)
          {

            pDocImport2 = &(pAllInfoFieldTmp->variablePart.DocImport2);

            strcpy (pRpt->szShipment,
                    pDocImport2->szShipment);
            strcpy (pCalcInfoTmp->szShipment,
                    pDocImport2->szShipment);


            // search shipment in the shipment array
            int i = 0;
            BOOL fFound = FALSE;
            while ( !fFound && (i < pRpt->iShipmentsUsed) )
            {
              int iResult = stricmp( pDocImport2->szShipment, pRpt->aszShipments[i] );
              if ( iResult == 0 ) 
              {
                // nothing to do, this shipment number is already contained in the table
                fFound = TRUE;
              }
              else if ( iResult < 0 ) 
              {
                // correct place to insert new shipment string has been found

                // make room at current location
                for( int j = pRpt->iShipmentsUsed - 1; j > i; j-- )
                {
                  strcpy( pRpt->aszShipments[j + 1], pRpt->aszShipments[j] );
                } /* endfor */

                // add the shipment
                strcpy( pRpt->aszShipments[i], pDocImport2->szShipment );
                pRpt->iShipmentsUsed += 1;
                fFound = TRUE;
              }
              else
              {
                // try next entry
                i++;
              } /* endif */
            } /* endwhile */

            // add entry to end of table if not found (and not inserted)
            if ( !fFound )
            {
              strcpy( pRpt->aszShipments[pRpt->iShipmentsUsed], pDocImport2->szShipment );
              pRpt->iShipmentsUsed += 1;
            } /* endif */
          }
          else
          {
            strcpy (pRpt->szShipment, "NA");
            strcpy (pCalcInfoTmp->szShipment, "NA");
          } // end if


          *ppCalcInfoFieldTmp++ = pCalcInfoTmp;  // store address in PCALCINFO array
          pRpt->ulCalcInfoRecords++;             // update number of CALCINFOs in RPT

          fDocImport = FALSE;  // set document import indicator


        } // end if
      } // end if new shipment
      //        ------------


      // -----------------------------------------
      // Set Task id; especially in case of Errors
      // -----------------------------------------

      if ( fHistError ||
           pAllInfoFieldTmp->histLogRecord.Task == HISTDATA_INVALID_LOGTASK )
        fHistInconsistency = FALSE;


      if ( fOK )
      {
        // set task ID and date/time in CALCINFO structure
        if ( fHistError ||
             pAllInfoFieldTmp->histLogRecord.Task == HISTDATA_INVALID_LOGTASK )
        {
          pCalcInfoTmp->Task  = HISTDATA_INVALID_LOGTASK;
        }
        else if ( fHistInconsistency )
        {
          pCalcInfoTmp->Task  = HISTDATA_INCONSISTENT_LOGTASK;
        }
        else
        {
          if ( pCalcInfoTmp->Task != HISTDATA_INVALID_LOGTASK &&
               pCalcInfoTmp->Task != HISTDATA_INCONSISTENT_LOGTASK )
          {
            pCalcInfoTmp->Task  = pAllInfoFieldTmp->histLogRecord.Task;
          } /* endif */
        } /* endif */

        pCalcInfoTmp->lTime = pAllInfoFieldTmp->histLogRecord.lTime;

        fDocDelete=FALSE;


        // -----------------------------------------
        // Add counting Information
        // First Check for Errors
        // -----------------------------------------

        // GQ 2002/03/12 Added API DocSave records
        if ( pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK ||
             pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK2 ||
             pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK3 ||
             pAllInfoFieldTmp->histLogRecord.Task == DOCAPI_LOGTASK ||
             pAllInfoFieldTmp->histLogRecord.Task == DOCAPI_LOGTASK3 )
        {
          if ( fAnalyzed )
          {
            fSecondAnalysis = TRUE;
            fAnalyzed = FALSE;
            RptLogString2( "MergingResults - SaveAfterAnalysis" );
            MergeResults ( pDocInfoAnalBase, pDocSaveHistSumTmp,FALSE );

          } // end if
        } // end if

        switch ( pAllInfoFieldTmp->histLogRecord.Task )
        {

          // -----------------------------------------
          case VERSION_LOGTASK :
            // -----------------------------------------

            // set tmp pointer to VERSIONHIST
            pVersionHistTmp = (PVERSIONHIST)&(pAllInfoFieldTmp->variablePart.DocSave);


            break; // end case

            // -----------------------------------------
          case HISTDATA_INVALID_LOGTASK :
            // -----------------------------------------
            //set histlog_error for THIS document
            fHistError = TRUE;
            fHistInconsistency = FALSE;

            break; // end case
            // -----------------------------------------
          case DOCDELETE_LOGTASK :
            // -----------------------------------------
            fDocDelete=!fDocDelete;
            break; // end case

            // -----------------------------------------
          case DOCIMPORT_LOGTASK :
            // -----------------------------------------
#ifdef IGNOREINTERNALIMPORTS
            if ( !fInternalImport )
#endif
            {
              fDocImport = TRUE;  // set document import indicator
              nDocImport ++;
            }
            break; // end case

            // -----------------------------------------
          case DOCIMPORT_LOGTASK2 :
            // -----------------------------------------
#ifdef IGNOREINTERNALIMPORTS
            if ( !fInternalImport )
#endif
            {
              fDocImport = TRUE;  // set document import indicator
              nDocImport ++;
            } 
            break; // end case


            // -----------------------------------------
          case AUTOMATICSUBST_LOGTASK :    // old structure - seems to be buggy. Does not reflect change to DocSave2 structure
            // -----------------------------------------
            // AnalAutoSubst, only first task since last DOCIMPORT_LOGTASK:
            // segments, source, target

            // RESET AnalAutoSubst buffer
            fAnaAutoSubstAvail = FALSE;
            memset( &AnaAutoSubstSave, 0, sizeof(AnaAutoSubstSave)  );

            // In case of ANALYZE/EDIT ANALYZE/EDIT restore old values
            fAnalyzed = TRUE;

            // set tmp pointer to DOCSAVEHIST of PALLINFO         (source)
            pDocSaveHistTmp = &(pAllInfoFieldTmp->variablePart.DocSave);

            // save values to AnaAutoSubst buffer
            if ( pDocSaveHistTmp->AnalAutoSubst.ComplexSum.ulSrcWords ||
                 pDocSaveHistTmp->AnalAutoSubst.MediumSum.ulSrcWords  ||
                 pDocSaveHistTmp->AnalAutoSubst.SimpleSum.ulSrcWords )
            {
              fAnaAutoSubstAvail = TRUE;
              memcpy( &AnaAutoSubstSave, &(pDocSaveHistTmp->AnalAutoSubst),
                      sizeof(AnaAutoSubstSave)  );
            } /* endif */

            // only first Document Import
            // counts for automatic substitution
            if ( fDocImport && nDocImport == 1 )
            {
              fDocImport = FALSE; // set document import indicator
              fSecondAnalysis = FALSE;

              // set tmp pointer to DOCSAVEHISTEX of PCALCINFO      (target)
              pDocSaveHistSumTmp = &(pCalcInfoTmp->docSaveHistSum);

              // set tmp pointer to DOCSAVEHIST of PALLINFO         (source)
              pDocSaveHistTmp = &(pAllInfoFieldTmp->variablePart.DocSave);

              myRptLogDocSave( "AnalysisAutoSubst(AfterImport)", pDocSaveHistTmp );

              // Analyze Automatic Substitution
              fHistInconsistency |=
              RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst),
                               pDocSaveHistTmp->AnalAutoSubst,
                               fHistError,
                               &(pCalcInfoTmp->Task),
                               ulWords
                              );

              // NotXlated, only last task: segments, source
              fHistInconsistency |=
              RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->NotXlated),
                               pDocSaveHistTmp->NotXlated,
                               fHistError,
                               &(pCalcInfoTmp->Task),
                               ulWords
                              );

              memset( pDocInfoAnalBase, 0, sizeof( DOCSAVEHISTEX )  );
            }
            // Automatic substitution last task afer edit
            // not for payment
            else
            {

              // set tmp pointer to DOCSAVEHISTEX of PCALCINFO      (target)
              pDocSaveHistSumTmp = &(pCalcInfoTmp->docSaveHistSum);

              // set tmp pointer to DOCSAVEHIST of PALLINFO         (source)
              pDocSaveHistTmp = &(pAllInfoFieldTmp->variablePart.DocSave);

              myRptLogDocSave( "AnalysisAutoSubst(2nd)", pDocSaveHistTmp );

              // Analyze Automatic Substitution
              fHistInconsistency |=
              RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst2),
                               pDocSaveHistTmp->AnalAutoSubst,
                               fHistError,
                               &(pCalcInfoTmp->Task),
                               ulWords
                              );

              // NotXlated, only last task: segments, source
              fHistInconsistency |=
              RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->NotXlated),
                               pDocSaveHistTmp->NotXlated,
                               fHistError,
                               &(pCalcInfoTmp->Task),
                               ulWords
                              );


            }// end if

            // adjusted fuzzy level
            if (pAllInfoFieldTmp->histLogRecord.Task == AUTOMATICSUBST_LOGTASK3)
            {
              pCalcInfoTmp->docFuzzyLevel[0] = pAllInfoFieldTmp->variablePart.DocSave3.lSmallFuzzLevel/100L;
              pCalcInfoTmp->docFuzzyLevel[1] = pAllInfoFieldTmp->variablePart.DocSave3.lMediumFuzzLevel/100L;
              pCalcInfoTmp->docFuzzyLevel[2] = pAllInfoFieldTmp->variablePart.DocSave3.lLargeFuzzLevel/100L;
            }
            break; // end case
        case AUTOMATICSUBST_LOGTASK3:
          // -----------------------------------------
          // AnalAutoSubst, only first task since last DOCIMPORT_LOGTASK:
          // segments, source, target

          // RESET AnalAutoSubst buffer
          fAnaAutoSubstAvail = FALSE;
          memset( &AnaAutoSubstSave, 0, sizeof(AnaAutoSubstSave)  );

          // In case of ANALYZE/EDIT ANALYZE/EDIT restore old values
          fAnalyzed = TRUE;

          // set tmp pointer to DOCSAVEHIST of PALLINFO         (source)
          pDocSaveHistTmp3 = &(pAllInfoFieldTmp->variablePart.DocSave3);

          // set tmp pointer to DOCSAVEHISTEX of PCALCINFO      (target)
          pDocSaveHistSumTmp = &(pCalcInfoTmp->docSaveHistSum);

          // save values to AnaAutoSubst buffer
          if ( pDocSaveHistTmp3->AnalAutoSubst.ComplexSum.ulSrcWords ||
               pDocSaveHistTmp3->AnalAutoSubst.MediumSum.ulSrcWords  ||
               pDocSaveHistTmp3->AnalAutoSubst.SimpleSum.ulSrcWords )
          {
            fAnaAutoSubstAvail = TRUE;
            memcpy( &AnaAutoSubstSave, &(pDocSaveHistTmp3->AnalAutoSubst),
                    sizeof(AnaAutoSubstSave)  );
          } /* endif */

          // only first Document Import
          // counts for automatic substitution
          if ( fDocImport && nDocImport == 1 )
          {
            fDocImport = FALSE; // set document import indicator
            fSecondAnalysis = FALSE;

            myRptLogDocSave3( "AnalysisAutoSubst(AfterImport)", pDocSaveHistTmp3 );

            // Analyze Automatic Substitution
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst),
                             pDocSaveHistTmp3->AnalAutoSubst,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // NotXlated, only last task: segments, source
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->NotXlated),
                             pDocSaveHistTmp3->NotXlated,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            memset( pDocInfoAnalBase, 0, sizeof( DOCSAVEHISTEX )  );
          }
          // Automatic substitution last task afer edit
          // not for payment
          else
          {

            // set tmp pointer to DOCSAVEHISTEX of PCALCINFO      (target)
            pDocSaveHistSumTmp = &(pCalcInfoTmp->docSaveHistSum);

            myRptLogDocSave3( "AnalysisAutoSubst(2nd)", pDocSaveHistTmp3 );

            // Analyze Automatic Substitution
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst2),
                             pDocSaveHistTmp3->AnalAutoSubst,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // NotXlated, only last task: segments, source
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->NotXlated),
                             pDocSaveHistTmp3->NotXlated,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


          }// end if

          // adjusted fuzzy level
          if (pAllInfoFieldTmp->histLogRecord.Task == AUTOMATICSUBST_LOGTASK3)
          {
            pCalcInfoTmp->docFuzzyLevel[0] = pAllInfoFieldTmp->variablePart.DocSave3.lSmallFuzzLevel/100L;
            pCalcInfoTmp->docFuzzyLevel[1] = pAllInfoFieldTmp->variablePart.DocSave3.lMediumFuzzLevel/100L;
            pCalcInfoTmp->docFuzzyLevel[2] = pAllInfoFieldTmp->variablePart.DocSave3.lLargeFuzzLevel/100L;
          }
          break; // end case


            // old structure
            // -----------------------------------------
          case DOCIMPNEWTARGET_LOGTASK :
#ifdef IGNOREIMPORTSAVERECORDS 
               break;
#endif
          case DOCSAVE_LOGTASK :
            // -----------------------------------------


            // set tmp pointer to DOCSAVEHISTEX of PCALCINFO
            pDocSaveHistSumTmp = &(pCalcInfoTmp->docSaveHistSum);

            // set tmp pointer to DOCSAVEHIST of PALLINFO
            pDocSaveHistTmp = &(pAllInfoFieldTmp->variablePart.DocSave);

            myRptLogDocSave( "DocSave", pDocSaveHistTmp );

             if ( pAllInfoFieldTmp->histLogRecord.Task == DOCIMPNEWTARGET_LOGTASK )
             {

              // RESET AnalAutoSubst buffer
              fAnaAutoSubstAvail = FALSE;
              memset( &AnaAutoSubstSave, 0, sizeof(AnaAutoSubstSave)  );

              // Analyze Automatic Substitution
              fHistInconsistency |=
              RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst),
                               pDocSaveHistTmp->AnalAutoSubst,
                               fHistError,
                               &(pCalcInfoTmp->Task),
                               ulWords
                              );

            } /* endif */

             // EditAutoSubst, only first task since last DOCIMPORT_LOGTASK:
            // segments, source, target NO!! XJR 25.4.97
            if ( fDocImport )
            {
              fDocImport = FALSE; // set document import indicator
            } // end if

            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->EditAutoSubst),
                             pDocSaveHistTmp->EditAutoSubst,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // ExactExist, sum overall: segments, source, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->ExactExist),
                             pDocSaveHistTmp->ExactExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // ReplExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->ReplExist),
                             pDocSaveHistTmp->ReplExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // FuzzyExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyExist),
                             pDocSaveHistTmp->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // MachExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->MachExist),
                             pDocSaveHistTmp->MachExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

#ifdef COUNTING_REPORT
            // FILL NEW STRUCTURE WITH OLD INFORMATION
            // ---------------------------------------


            // ExactUsed, sum overall: segments, source, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->ExactUsed),
                             pDocSaveHistTmp->ExactExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // ReplUsed, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->ReplUsed),
                             pDocSaveHistTmp->ReplExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyUsed, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed),
                             pDocSaveHistTmp->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // MachUsed, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->MachUsed),
                             pDocSaveHistTmp->MachExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist_1, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyExist_1),
                             pDocSaveHistTmp->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyUsed_1, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed_1),
                             pDocSaveHistTmp->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist_2, sum overall: segments, source, modified, target
            COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_2,
                           count_null);

            // FuzzyUsed_2, sum overall: segments, source, modified, target
            COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_2,
                           count_null );


            // FuzzyExist_3, sum overall: segments, source, modified, target
            COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_3,
                           count_null);

            // FuzzyUsed_3, sum overall: segments, source, modified, target
            COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_3,
                           count_null );


            // NoneExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt,  &(pDocSaveHistSumTmp->NoneExist2),
                             pDocSaveHistTmp->NoneExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

#endif

            // NoneExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt,  &(pDocSaveHistSumTmp->NoneExist),
                             pDocSaveHistTmp->NoneExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // NotXlated, only last task: segments, source
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->NotXlated),
                             pDocSaveHistTmp->NotXlated,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            break; // end case


            // new summary counting report structure
            // -----------------------------------------
          case DOCIMPNEWTARGET_LOGTASK2 :
          case DOCIMPNEWTARGET_LOGTASK3:
#ifdef IGNOREIMPORTSAVERECORDS 
               break;
#endif
          case DOCSAVE_LOGTASK2 :
          case DOCSAVE_LOGTASK3:
          case DOCAPI_LOGTASK :
          case DOCAPI_LOGTASK3 :
            // -----------------------------------------


            // set tmp pointer to DOCSAVEHISTEX of PCALCINFO
            pDocSaveHistSumTmp = &(pCalcInfoTmp->docSaveHistSum);

            // set tmp pointer to DOCSAVEHIST of PALLINFO   (enlarged structure)
            pDocSaveHistTmp2 = &(pAllInfoFieldTmp->variablePart.DocSave2);


            // EditAutoSubst, only first task since last DOCIMPORT_LOGTASK:
            // segments, source, target NO!! XJR 25.4.97
            if ( fDocImport )
            {
              fDocImport = FALSE; // set document import indicator
            } // end if


            //
            // Consistency check for the whole row
            //
            fHistInconsistency |=
            RPTCHECKALLCOUNTS2(pRpt, pDocSaveHistSumTmp,
                              pDocSaveHistTmp2,
                              fHistError,
                              &(pCalcInfoTmp->Task),
                              ulWords
                             );

            // GQ: Special handling for DOCSAVE records with missing
            //     AnalAutosubst values (are caused by Unicode<->ASCII
            //     conversion of segmented target file with loss of the
            //     X= info), the AnalAutoSubst words are part of the
            //     NonExist words

            if ( fAnaAutoSubstAvail )
            {
              if ( (pDocSaveHistTmp2->AnalAutoSubst.ComplexSum.ulSrcWords == 0L) &&
                   (pDocSaveHistTmp2->AnalAutoSubst.MediumSum.ulSrcWords == 0L) &&
                   (pDocSaveHistTmp2->AnalAutoSubst.SimpleSum.ulSrcWords == 0L) )
              {
                PCRITERIASUM pNonExist = &(pDocSaveHistTmp2->NoneExist);
                PCRITERIASUM pNonExist2 = &(pDocSaveHistTmp2->NoneExist2);

                RptLogString2( "DocSaveRecord with missing AnalAutoSubst values, correcting NonExist matches" );

                if ( (pNonExist->ComplexSum.ulSrcWords >= AnaAutoSubstSave.ComplexSum.ulSrcWords) &&
                     (pNonExist->MediumSum.ulSrcWords  >= AnaAutoSubstSave.MediumSum.ulSrcWords) &&
                     (pNonExist->SimpleSum.ulSrcWords  >= AnaAutoSubstSave.SimpleSum.ulSrcWords) )
                {
                  pNonExist->ComplexSum.ulSrcWords -= AnaAutoSubstSave.ComplexSum.ulSrcWords;
                  SUBTRACTWITHCHECKING( pNonExist->ComplexSum.usNumSegs, AnaAutoSubstSave.ComplexSum.usNumSegs );
                  pNonExist->MediumSum.ulSrcWords  -= AnaAutoSubstSave.MediumSum.ulSrcWords;
                  SUBTRACTWITHCHECKING( pNonExist->MediumSum.usNumSegs, AnaAutoSubstSave.MediumSum.usNumSegs );
                  pNonExist->SimpleSum.ulSrcWords  -= AnaAutoSubstSave.SimpleSum.ulSrcWords;
                  SUBTRACTWITHCHECKING( pNonExist->SimpleSum.usNumSegs, AnaAutoSubstSave.SimpleSum.usNumSegs );
                }
                else
                {
                  RptLogString2( "NonExist matchs to small, nothing corrected" );
                } /* endif */

                if ( (pNonExist2->ComplexSum.ulSrcWords >= AnaAutoSubstSave.ComplexSum.ulSrcWords) &&
                     (pNonExist2->MediumSum.ulSrcWords  >= AnaAutoSubstSave.MediumSum.ulSrcWords) &&
                     (pNonExist2->SimpleSum.ulSrcWords  >= AnaAutoSubstSave.SimpleSum.ulSrcWords) )
                {
                  pNonExist2->ComplexSum.ulSrcWords -= AnaAutoSubstSave.ComplexSum.ulSrcWords;
                  SUBTRACTWITHCHECKING( pNonExist2->ComplexSum.usNumSegs, AnaAutoSubstSave.ComplexSum.usNumSegs );
                  pNonExist2->MediumSum.ulSrcWords  -= AnaAutoSubstSave.MediumSum.ulSrcWords;
                  SUBTRACTWITHCHECKING( pNonExist2->MediumSum.usNumSegs, AnaAutoSubstSave.MediumSum.usNumSegs );
                  pNonExist2->SimpleSum.ulSrcWords  -= AnaAutoSubstSave.SimpleSum.ulSrcWords;
                  SUBTRACTWITHCHECKING( pNonExist2->SimpleSum.usNumSegs, AnaAutoSubstSave.SimpleSum.usNumSegs );
                }
                else
                {
                  RptLogString2( "NonExist2 matchs to small, nothing corrected" );
                } /* endif */
              } /* endif */
            } /* edif */



            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->EditAutoSubst),
                             pDocSaveHistTmp2->EditAutoSubst,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // ExactExist/Used, sum overall: segments, source, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->ExactExist),
                             pDocSaveHistTmp2->ExactExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->ExactUsed),
                             pDocSaveHistTmp2->ExactUsed,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // ReplExist/Used, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->ReplExist),
                             pDocSaveHistTmp2->ReplExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->ReplUsed),
                             pDocSaveHistTmp2->ReplUsed,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // FuzzyExist/Used, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyExist),
                             pDocSaveHistTmp2->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed),
                             pDocSaveHistTmp2->FuzzyUsed,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist/Used 1 , sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyExist_1),
                             pDocSaveHistTmp2->FuzzyExist_1,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed_1),
                             pDocSaveHistTmp2->FuzzyUsed_1,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist/Used 2, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyExist_2),
                             pDocSaveHistTmp2->FuzzyExist_2,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed_2),
                             pDocSaveHistTmp2->FuzzyUsed_2,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist/Used 3, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyExist_3),
                             pDocSaveHistTmp2->FuzzyExist_3,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed_3),
                             pDocSaveHistTmp2->FuzzyUsed_3,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // MachExist/Used, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->MachExist),
                             pDocSaveHistTmp2->MachExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->MachUsed),
                             pDocSaveHistTmp2->MachUsed,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // NoneExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt,  &(pDocSaveHistSumTmp->NoneExist),
                             pDocSaveHistTmp2->NoneExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // NoneExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt,  &(pDocSaveHistSumTmp->NoneExist2),
                             pDocSaveHistTmp2->NoneExist2,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );



            // NotXlated, only last task: segments, source
            fHistInconsistency |=
            RPTCOPYALLCOUNTS2(pRpt, &( pDocSaveHistSumTmp->NotXlated),
                             pDocSaveHistTmp2->NotXlated,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            if (pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK3)
            {
              pCalcInfoTmp->docFuzzyLevel[0] = pAllInfoFieldTmp->variablePart.DocSave3.lSmallFuzzLevel/100L;
              pCalcInfoTmp->docFuzzyLevel[1] = pAllInfoFieldTmp->variablePart.DocSave3.lMediumFuzzLevel/100L;
              pCalcInfoTmp->docFuzzyLevel[2] = pAllInfoFieldTmp->variablePart.DocSave3.lLargeFuzzLevel/100L;
            }

            RptLogCalcInfo2( "CalcInfo after SaveRecord", pCalcInfoTmp );
            break; // end case

        } // end switch


        // save data for security comparison
        if ( pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK ||
             pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK2 ||
             pAllInfoFieldTmp->histLogRecord.Task == DOCSAVE_LOGTASK3 )
        {
          CopyResults(pDocSaveCopy,pDocSaveHistSumTmp);
        } // end if

        ppAllInfoFieldTmp++;                    // next PALLINFO
        pAllInfoFieldTmp = *ppAllInfoFieldTmp;  // set tmp pointer to ALLINFO
      } // end if

      if ( !pRpt )
      {
        fOK=FALSE;
        return fOK;
      } // end if

    }  // end while

    // in case of a second analysis add base values....
    if ( fOK && fSecondAnalysis )
    {
      // Reset pDocInfoAnalBase
      // Merge pDocInfoAnalBase into pDocSaveHistSumTmp
      // Data lost due to second analysis
      MergeResults( pDocSaveHistSumTmp, pDocInfoAnalBase, TRUE );
    } /* endif */
  } // end if



  // -------------------------------
  // calculation report with totals
  // -------------------------------

  if ( fOK && (pRpt->usOptions == WITH_TOTALS) && (pRpt->usReport == CALCULATION_REPORT) )
  {
    *(pRpt->pszActualDocument) = EOS;  // set actual document in RPT

    // add one PCALCINFO for summary CALCINFO in PCALCINFO array
    // if necessary, realloc PCALCINFO array
    if ( pRpt->ulCalcInfoRecords >= ulAllocEntries )
    {
      // realloc memory for PCALCINFO array
      fOK = UtlAlloc ((PVOID*)&pArray,
                      (ULONG)(ulAllocEntries * (ULONG)sizeof(PCALCINFO)),
                      (ULONG)(ulAllocEntries + INIT_SIZE_C) * (ULONG)sizeof(PCALCINFO),
                      ERROR_STORAGE);

      if ( fOK )
      {
        pRpt->ppCalcInfoField = (PPCALCINFO)pArray;  // store memory address in RPT
        ulAllocEntries += INIT_SIZE_C;
      }
    } // end if

    usIndex = 0;  // set counter

    // alloc memory for CALCINFO summary
    fOK = UtlAlloc ((PVOID*)&pArray, 0L, sizeof(CALCINFO), ERROR_STORAGE);

    if ( fOK )
    {
      pRpt->pCalcInfoSum = (PCALCINFO)pArray;      // store memory address in RPT
      pCalcInfoSum = (PCALCINFO)pArray;            // set tmp pointer to CALCINFO for summary
      ppCalcInfoFieldTmp = pRpt->ppCalcInfoField;  // set tmp pointer to PCALCINFOs

      // set tmp pointer to DOCSAVEHISTEX of PCALCINFO for summary
      pDocSaveHistSumSum = &(pCalcInfoSum->docSaveHistSum);
      // set tmp pointer to CRITERIASUMEX of PCALCINFO for summary
      pDocCriteriaSumSum = &(pCalcInfoSum->docCriteriaSum);

      time (&lTime);  // get current time/date

      // set name and time/date in CALCINFO structure
      strcpy (pCalcInfoSum->szDocument, "Summary" );
      strcpy (pCalcInfoSum->szLongName, "Summary" );
      pCalcInfoSum->lTime = lTime;
    } // end if

    if ( fOK )
    {
      char szNumber[10];
      sprintf( szNumber, "%lu", pRpt->ulCalcInfoRecords );
      RptLog2String2( "Building summary, number of calc info records: ", szNumber );

      // loop over all PCALCINFOs, to build summary
      while ( usIndex++ < pRpt->ulCalcInfoRecords )
      {

        // set tmp pointer to DOCSAVEHISTEX of PCALCINFO
        pDocSaveHistSumTmp = &((*ppCalcInfoFieldTmp)->docSaveHistSum);

        // set tmp pointer to DOCCRITERIASUMEX of PCALCINFO
        pDocCriteriaSumTmp = &((*ppCalcInfoFieldTmp)->docCriteriaSum);

        // AnalAutoSubst, sum overall: segments, source, target
        ADDALLCOUNTS2( (*pDocCriteriaSumTmp),
                       pDocSaveHistSumTmp->AnalAutoSubst );
        ADDALLCOUNTS2( pDocSaveHistSumSum->AnalAutoSubst,
                       pDocSaveHistSumTmp->AnalAutoSubst );
        ADDALLCOUNTS2( (*pDocCriteriaSumSum),
                       pDocSaveHistSumTmp->AnalAutoSubst );

        // EditAutoSubst, sum overall: segments, source, target
        ADDALLCOUNTS2( (*pDocCriteriaSumTmp),
                       pDocSaveHistSumTmp->EditAutoSubst );
        ADDALLCOUNTS2( pDocSaveHistSumSum->EditAutoSubst,
                       pDocSaveHistSumTmp->EditAutoSubst );
        ADDALLCOUNTS2( (*pDocCriteriaSumSum),
                       pDocSaveHistSumTmp->EditAutoSubst );

        // ExactExist, sum overall: segments, source, target
        ADDALLCOUNTS2( (*pDocCriteriaSumTmp),
                       pDocSaveHistSumTmp->ExactExist );
        ADDALLCOUNTS2( pDocSaveHistSumSum->ExactExist,
                       pDocSaveHistSumTmp->ExactExist );
        ADDALLCOUNTS2( (*pDocCriteriaSumSum),
                       pDocSaveHistSumTmp->ExactExist );


        // ReplExist, sum overall: segments, source, modified, target
        ADDALLCOUNTS2( (*pDocCriteriaSumTmp),
                       pDocSaveHistSumTmp->ReplExist );
        ADDALLCOUNTS2( pDocSaveHistSumSum->ReplExist,
                       pDocSaveHistSumTmp->ReplExist );
        ADDALLCOUNTS2( (*pDocCriteriaSumSum),
                       pDocSaveHistSumTmp->ReplExist );

        // FuzzyExist, sum overall: segments, source, modified, target
        ADDALLCOUNTS2( (*pDocCriteriaSumTmp),
                       pDocSaveHistSumTmp->FuzzyExist );
        ADDALLCOUNTS2( pDocSaveHistSumSum->FuzzyExist,
                       pDocSaveHistSumTmp->FuzzyExist );
        ADDALLCOUNTS2( (*pDocCriteriaSumSum),
                       pDocSaveHistSumTmp->FuzzyExist );

        // MachExist, sum overall: segments, source, modified, target
        ADDALLCOUNTS2( (*pDocCriteriaSumTmp),
                       pDocSaveHistSumTmp->MachExist );
        ADDALLCOUNTS2( pDocSaveHistSumSum->MachExist,
                       pDocSaveHistSumTmp->MachExist );
        ADDALLCOUNTS2( (*pDocCriteriaSumSum),
                       pDocSaveHistSumTmp->MachExist );

        // NoneExist, sum overall: segments, source, modified, target
        ADDALLCOUNTS2( (*pDocCriteriaSumTmp),
                       pDocSaveHistSumTmp->NoneExist );
        ADDALLCOUNTS2( pDocSaveHistSumSum->NoneExist,
                       pDocSaveHistSumTmp->NoneExist );
        ADDALLCOUNTS2( (*pDocCriteriaSumSum),
                       pDocSaveHistSumTmp->NoneExist );


        // NotXlated, only sum of docSaveHistSum: segments, source
        ADDALLCOUNTS2( pDocSaveHistSumSum->NotXlated,
                       pDocSaveHistSumTmp->NotXlated );

        ppCalcInfoFieldTmp++;  // next PCALCINFO
      } // end while

      *ppCalcInfoFieldTmp++ = pCalcInfoSum;  // store address in PCALCINFO array
      pRpt->ulCalcInfoRecords++;             // update number of CALCINFOs in RPT
    } // end if
  }                                    // end if

  // free resource
  UtlAlloc ((PVOID*)&pDocInfoAnalBase, 0L, 0L, NOMSG);

  return fOK;
} // end of RptMakeCalculationRecordsX

// load a calculation profile into the RPT structure
int CalcRepLoadProfile( PRPT pRpt, PSZ pszProfile, BOOL fMsg )
{
  int iRC = 0;
  PPROPFOLDER  ppropFolder = NULL;
  char szProfile[MAX_EQF_PATH];

  // allocate profile buffer
  ppropFolder = (PPROPFOLDER) malloc( sizeof(PROPFOLDER) );
  if ( ppropFolder == NULL)
  {
    iRC = ERROR_NOT_ENOUGH_MEMORY;
    if ( fMsg ) printf( "Error: Memory allocation for profile data failed\n" );
  } /* endif */

  // create fully qualified profile name
  if ( !iRC )
  {
    UtlMakeEQFPath( szProfile, NULC, PROPERTY_PATH, NULL );
    sprintf( szProfile + strlen(szProfile), "\\%s.R00", pszProfile );
  } /* endif */

  // load profile into memory
  if ( !iRC )
  {
    FILE *hfProfile = fopen( szProfile, "rb" );
    if ( hfProfile )
    {
      memset( ppropFolder, 0, sizeof(PROPFOLDER) );
      fread( (PVOID)ppropFolder, 1, sizeof(PROPFOLDER), hfProfile );
      fclose( hfProfile );
    }
    else
    {
      iRC = ERROR_FILE_NOT_FOUND;
      if ( fMsg ) printf( "Error: Could not open profile \"%s\"\n", szProfile );
    } /* endif */
      
  } /* endif */

  // copy profile settings to RPT area
  if ( !iRC )
  {
    pRpt->usFormat      =  ppropFolder->usFormat      ;
    pRpt->usOption[0]   =  ppropFolder->usOption[0]   ;
    pRpt->usOption[1]   =  ppropFolder->usOption[1]   ;
    pRpt->usOption[2]   =  ppropFolder->usOption[2]   ;
    pRpt->usOption[3]   =  ppropFolder->usOption[3]   ;
    pRpt->usOption[4]   =  ppropFolder->usOption[4]   ;
    pRpt->usOption[5]   =  ppropFolder->usOption[5]   ;
    pRpt->usOption1     =  ppropFolder->usOption1     ;
    pRpt->usOption2     =  ppropFolder->usOption2     ;
    pRpt->usOption3     =  ppropFolder->usOption3     ;
    pRpt->usOption4     =  ppropFolder->usOption4     ;
    pRpt->usOption5     =  ppropFolder->usOption5     ;
    pRpt->usOption21    =  ppropFolder->usOption21    ;
    pRpt->usOption22    =  ppropFolder->usOption22    ;
    pRpt->usColumns[0]  =  ppropFolder->usColumns[0]  ;
    pRpt->usColumns[1]  =  ppropFolder->usColumns[1]  ;
    pRpt->usColumns[2]  =  ppropFolder->usColumns[2]  ;
    pRpt->usColumns[3]  =  ppropFolder->usColumns[3]  ;
//    pRpt->usColumns4[0]  =  ppropFolder->usColumns4[0] ;
    pRpt->usColumns4[0]  =  0;
    pRpt->usColumns4[1]  =  ppropFolder->usColumns4[1]  ;
    pRpt->usColumns4[2]  =  ppropFolder->usColumns4[2]  ;
    pRpt->usColumns4[3]  =  ppropFolder->usColumns4[3]  ;
    pRpt->usColumns4[4]  =  ppropFolder->usColumns4[4]  ;
    pRpt->usStandard    =  ppropFolder->usStandard    ;
    pRpt->Pay_per_Standard = ppropFolder->Pay_per_Standard;
    pRpt->usCurrency    =  ppropFolder->usCurrency ;
    strcpy( pRpt->szCurrency, MyszRptCurrency2[pRpt->usCurrency]) ;  
    strcpy(pRpt->szShipment, ppropFolder->szShipment);
    pRpt->usShipmentChk = ppropFolder->usShipmentChk;
    strcpy( pRpt->szTargetLang, ppropFolder->szTargetLang );

    for (int i=0; i < MAX_PAYREPORT_COLUMNS; i++ )
    {
      for ( int j=0; j<3; j++)
      {
        pRpt->Complexity_Factor[i][j] = ppropFolder->Complexity_Factor[i][j];
        pRpt->Pay_Factor[i][j] = ppropFolder->Pay_Factor[i][j];
      }  //end for
    } //end for
  } /* endif */

  // cleanup
  if ( ppropFolder ) free( ppropFolder );

  RptLog2String2( "Using profile ", pszProfile );
  RptLog2String2( "   szShipment    : ", pRpt->szShipment );
  RptLogUShort(  "   usShipmentChk : ", ppropFolder->usShipmentChk );

  return( iRC );
} /* end of function CalcRepLoadProfile */

// output a criteria sum
BOOL RptSumOut
(
  PRPT        pRpt,
  CXmlWriter  *xw,                     // active XmlWrite object
  PCRITERIASUM pSum,                   // pointer to counts
  PSZ         pszType                  // type of counts
)
{
  BOOL           fOK = TRUE;           // success indicator

  pRpt;

  BOOL fEmpty = (pSum->SimpleSum.usNumSegs == 0)  &&
                (pSum->SimpleSum.ulSrcWords == 0) &&
                (pSum->SimpleSum.ulModWords == 0) && 
                (pSum->SimpleSum.ulTgtWords == 0) && 
                (pSum->MediumSum.usNumSegs == 0)  &&
                (pSum->MediumSum.ulSrcWords == 0) &&
                (pSum->MediumSum.ulModWords == 0) && 
                (pSum->MediumSum.ulTgtWords == 0) && 
                (pSum->ComplexSum.usNumSegs == 0)  &&
                (pSum->ComplexSum.ulSrcWords == 0) &&
                (pSum->ComplexSum.ulModWords == 0) && 
                (pSum->ComplexSum.ulTgtWords == 0);

  xw->WriteStartElement( "wordcount" );
  xw->WriteAttributeString( "type", pszType );
  xw->WriteAttributeString( "empty", fEmpty ? "Yes" : "No" );
  fOK = RptSingleCatOut( xw, &(pSum->SimpleSum), "simple" );
  if ( fOK ) fOK = RptSingleCatOut( xw, &(pSum->MediumSum), "medium" );
  if ( fOK ) fOK = RptSingleCatOut( xw, &(pSum->ComplexSum), "complex" );
  xw->WriteEndElement();

  return( fOK );
} /* end of function RptSumOut */

// output the numbers of a single category
BOOL RptSingleCatOut
(
  CXmlWriter  *xw,                     // active XmlWrite object
  PSUMPERCLASS pNumbers,               // numbers of this category
  PSZ          pszCategory             // name of category
)
{
  BOOL fOK = TRUE;

  fOK = RptSingleCountOut( xw, pNumbers->usNumSegs, pszCategory, "segments" );
  if ( fOK ) fOK = RptSingleCountOut( xw, pNumbers->ulSrcWords, pszCategory, "sourceWords" );
  if ( fOK ) fOK = RptSingleCountOut( xw, pNumbers->ulModWords, pszCategory, "modWords" );
  if ( fOK ) fOK = RptSingleCountOut( xw, pNumbers->ulTgtWords, pszCategory, "targetWords" );
  return( fOK );
} /* end of function RptSingleCatOut */

// output a criteria sum (same as RptSumOut but for PCRITERIASUMEX)
BOOL RptSumOut2
(
  PRPT        pRpt,
  CXmlWriter  *xw,                     // active XmlWrite object
  PCRITERIASUMEX pSum,                   // pointer to counts
  PSZ         pszType                  // type of counts
)
{
  BOOL           fOK = TRUE;           // success indicator

  pRpt;

  BOOL fEmpty = (pSum->SimpleSum.ulNumSegs == 0)  &&
                (pSum->SimpleSum.ulSrcWords == 0) &&
                (pSum->SimpleSum.ulModWords == 0) && 
                (pSum->SimpleSum.ulTgtWords == 0) && 
                (pSum->MediumSum.ulNumSegs == 0)  &&
                (pSum->MediumSum.ulSrcWords == 0) &&
                (pSum->MediumSum.ulModWords == 0) && 
                (pSum->MediumSum.ulTgtWords == 0) && 
                (pSum->ComplexSum.ulNumSegs == 0)  &&
                (pSum->ComplexSum.ulSrcWords == 0) &&
                (pSum->ComplexSum.ulModWords == 0) && 
                (pSum->ComplexSum.ulTgtWords == 0);

  xw->WriteStartElement( "wordcount" );
  xw->WriteAttributeString( "type", pszType );
  xw->WriteAttributeString( "empty", fEmpty ? "Yes" : "No" );
  fOK = RptSingleCatOut2( xw, &(pSum->SimpleSum), "Simple" );
  if ( fOK ) fOK = RptSingleCatOut2( xw, &(pSum->MediumSum), "Medium" );
  if ( fOK ) fOK = RptSingleCatOut2( xw, &(pSum->ComplexSum), "Complex" );
  xw->WriteEndElement(); // counts

  return( fOK );
} /* end of function RptSumOut2 */

// output the numbers of a single category ( same as RptSingleCatOut but for PSUMPERCLASSEX) 
BOOL RptSingleCatOut2
(
  CXmlWriter  *xw,                     // active XmlWrite object
  PSUMPERCLASSEX pNumbers,               // numbers of this category
  PSZ          pszCategory             // name of category
)
{
  BOOL fOK = TRUE;

  fOK = RptSingleCountOut( xw, pNumbers->ulNumSegs, pszCategory, "segments" );
  if ( fOK ) fOK = RptSingleCountOut( xw, pNumbers->ulSrcWords, pszCategory, "sourceWords" );
  if ( fOK ) fOK = RptSingleCountOut( xw, pNumbers->ulModWords, pszCategory, "modWords" );
  if ( fOK ) fOK = RptSingleCountOut( xw, pNumbers->ulTgtWords, pszCategory, "targetWords" );
  return( fOK );
} /* end of function RptSingleCatOut2 */



// output a single value
BOOL RptSingleCountOut
(
  CXmlWriter  *xw,                     // active XmlWrite object
  ULONG       ulCount,                 // count 
  PSZ         pszCategory,             // name of category
  PSZ         pszType                  // type of the count
)
{
  CHAR szNumber[20];

  sprintf( szNumber, "%lu", ulCount );

  xw->WriteStartElement( "words" );
  xw->WriteAttributeString( "category", pszCategory );
  xw->WriteAttributeString( "type", pszType );
  xw->WriteAttributeString( "count", szNumber );
  xw->WriteEndElement();

  return( TRUE );
} /* end of function RptSingleCountOut */

// functions for logging

void RptCheckLogging2()
{
  char szTriggerFile[MAX_EQF_PATH];

  UtlMakeEQFPath( szTriggerFile, NULC, SYSTEM_PATH, NULL );
  strcat( szTriggerFile, BACKSLASH_STR );
  strcat( szTriggerFile, RPTLOGTRIGGER );

  if ( !fRptLogging2 ) fRptLogging2 = UtlFileExist( szTriggerFile );
} /* end of function RptCheckLogging2 */

FILE *RptOpenLogFile( PSZ pszLogFile, PSZ pszLogName )
{
  FILE *hfLog;

  // fully qualified log directory name
  CHAR szLogFile[MAX_EQF_PATH];
  UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );

  // create log directory if necessary
  UtlMkDir( szLogFile, 0, FALSE );

  // complete log file name
  strcat( szLogFile, "\\" );
  strcat( szLogFile, pszLogFile );

  // open log file
  hfLog = fopen( szLogFile, "a" );

  if ( hfLog != NULLHANDLE )
  {
    char szDate[20], szTime[20];
    LONG lTime;
    UtlTime( &lTime );
    UtlLongToDateString( lTime, szDate, sizeof(szDate)-1 );
    UtlLongToTimeString( lTime, szTime, sizeof(szTime)-1 );
    fprintf( hfLog, "***** %s Log %s %s *****\n\n", pszLogName, szDate, szTime );
  } /* endif */
  return( hfLog );
}
 
void RptLogStart2()
{
  RptCheckLogging2();

  if ( fRptLogging2 )
  {
    CHAR szLogFile[MAX_EQF_PATH];


    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, BACKSLASH_STR );
    strcat( szLogFile, RPTLOGFILE );

    hRptLog2 = fopen( szLogFile, "a" );
    if ( hRptLog2 == NULLHANDLE )
    {
      fRptLogging2 = FALSE; // no logging possible, open of log file failed
    }
    else
    {
      char szDate[20], szTime[20];
      LONG lTime;
      UtlTime( &lTime );
      UtlLongToDateString( lTime, szDate, sizeof(szDate)-1 );
      UtlLongToTimeString( lTime, szTime, sizeof(szTime)-1 );
      fprintf( hRptLog2, "***** CalcReport Log %s %s *****\n\n", szDate, szTime );
    } /* endif */
  } /* endif */
} /* end of function RptLogStart2 */

void RptLogEnd2()
{
  if ( fRptLogging2 && (hRptLog2 != NULLHANDLE) )
  {
    fprintf( hRptLog2, "***** End of Log *****\n" );
    fclose( hRptLog2 );
    hRptLog2 = NULLHANDLE;
  } /* endif */
} /* end of function RptLogEnd2 */

void RptLogString2( PSZ pszString )
{
  if ( fRptLogging2 )
  {
    fprintf( hRptLog2, "%s\n", pszString );
  } /* endif */
} /* end of function RptLogString2 */

void RptLog2String2( PSZ pszString1, PSZ pszString2 )
{
  if ( fRptLogging2 )
  {
    fprintf( hRptLog2, "%s %s\n", pszString1, pszString2 );
  } /* endif */
} /* end of function RptLog2String2 */

void RptLogUShort( PSZ pszString, USHORT usValue )
{
  if ( fRptLogging2 )
  {
    fprintf( hRptLog2, "%s %u\n", pszString, usValue );
  } /* endif */
} /* end of function RptLogUShort */

void RptLogULong( PSZ pszString, ULONG ulValue )
{
  if ( fRptLogging2 )
  {
    fprintf( hRptLog2, "%s %lu\n", pszString, ulValue );
  } /* endif */
} /* end of function RptLogUShort */

void RptLogCalcInfo2( PSZ pszString, PCALCINFO pCalcInfo )
{
  if ( fRptLogging2 )
  {
    static char szSums[1024];

    szSums[0] = EOS;

    RptLogBuildSumEx2( szSums, "AnaSubst",    &(pCalcInfo->docSaveHistSum.AnalAutoSubst) );
    RptLogBuildSumEx2( szSums, "AnaSubst2",   &(pCalcInfo->docSaveHistSum.AnalAutoSubst2) );
    RptLogBuildSumEx2( szSums, "EditSubst",   &(pCalcInfo->docSaveHistSum.EditAutoSubst) );
    RptLogBuildSumEx2( szSums, "ExactExist",  &(pCalcInfo->docSaveHistSum.ExactExist) );
    RptLogBuildSumEx2( szSums, "ExactUsed",   &(pCalcInfo->docSaveHistSum.ExactUsed) );
    RptLogBuildSumEx2( szSums, "ReplExist",   &(pCalcInfo->docSaveHistSum.ReplExist) );
    RptLogBuildSumEx2( szSums, "ReplUsed",    &(pCalcInfo->docSaveHistSum.ReplUsed) );
    RptLogBuildSumEx2( szSums, "FuzzyExist",  &(pCalcInfo->docSaveHistSum.FuzzyExist) );
    RptLogBuildSumEx2( szSums, "FuzzyUsed",   &(pCalcInfo->docSaveHistSum.FuzzyUsed) );
    RptLogBuildSumEx2( szSums, "FuzzyExist1", &(pCalcInfo->docSaveHistSum.FuzzyExist_1) );
    RptLogBuildSumEx2( szSums, "FuzzyUsed1",  &(pCalcInfo->docSaveHistSum.FuzzyUsed_1) );
    RptLogBuildSumEx2( szSums, "FuzzyExist2", &(pCalcInfo->docSaveHistSum.FuzzyExist_2) );
    RptLogBuildSumEx2( szSums, "FuzzyUsed2",  &(pCalcInfo->docSaveHistSum.FuzzyUsed_2) );
    RptLogBuildSumEx2( szSums, "FuzzyExist3", &(pCalcInfo->docSaveHistSum.FuzzyExist_3) );
    RptLogBuildSumEx2( szSums, "FuzzyUsed3",  &(pCalcInfo->docSaveHistSum.FuzzyUsed_3) );
    RptLogBuildSumEx2( szSums, "MachExist",   &(pCalcInfo->docSaveHistSum.MachExist) );
    RptLogBuildSumEx2( szSums, "MachUsed",   &(pCalcInfo->docSaveHistSum.MachUsed) );
    RptLogBuildSumEx2( szSums, "NoneExist",   &(pCalcInfo->docSaveHistSum.NoneExist) );
    RptLogBuildSumEx2( szSums, "NoneExist2",  &(pCalcInfo->docSaveHistSum.NoneExist2) );
    RptLogBuildSumEx2( szSums, "NoneXlated",  &(pCalcInfo->docSaveHistSum.NotXlated) );

    fprintf( hRptLog2, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function RptLogString2 */


void myRptLogDocSave( PSZ pszString, PDOCSAVEHIST pDocSave )
{
  if ( fRptLogging2 )
  {
    static char szSums[1024];

    szSums[0] = EOS;
    RptLogBuildSum2( szSums, "AnaSubst",    &(pDocSave->AnalAutoSubst) );
    RptLogBuildSum2( szSums, "EditSubst",   &(pDocSave->EditAutoSubst) );
    RptLogBuildSum2( szSums, "ExactExist",  &(pDocSave->ExactExist) );
    RptLogBuildSum2( szSums, "ReplExist",   &(pDocSave->ReplExist) );
    RptLogBuildSum2( szSums, "FuzzyExist",  &(pDocSave->FuzzyExist) );
    RptLogBuildSum2( szSums, "MachExist",   &(pDocSave->MachExist) );
    RptLogBuildSum2( szSums, "NoneExist",   &(pDocSave->NoneExist) );
    RptLogBuildSum2( szSums, "NotXlated",   &(pDocSave->NotXlated) );

    fprintf( hRptLog2, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function myRptLogDocSave */



void myRptLogDocSave3( PSZ pszString, PDOCSAVEHIST3 pDocSave3 )
{
  if ( fRptLogging2 )
  {
    static char szSums[1024];

    szSums[0] = EOS;
    RptLogBuildSum2( szSums, "AnaSubst",    &(pDocSave3->AnalAutoSubst) );
    RptLogBuildSum2( szSums, "EditSubst",   &(pDocSave3->EditAutoSubst) );
    RptLogBuildSum2( szSums, "ExactExist",  &(pDocSave3->ExactExist) );
    RptLogBuildSum2( szSums, "ExactUsed",   &(pDocSave3->ExactUsed) );
    RptLogBuildSum2( szSums, "ReplExist",   &(pDocSave3->ReplExist) );
    RptLogBuildSum2( szSums, "ReplUsed",    &(pDocSave3->ReplUsed) );
    RptLogBuildSum2( szSums, "FuzzyExist",  &(pDocSave3->FuzzyExist) );
    RptLogBuildSum2( szSums, "FuzzyUsed",   &(pDocSave3->FuzzyUsed) );
    RptLogBuildSum2( szSums, "FuzzyExist1", &(pDocSave3->FuzzyExist_1) );
    RptLogBuildSum2( szSums, "FuzzyUsed1",  &(pDocSave3->FuzzyUsed_1) );
    RptLogBuildSum2( szSums, "FuzzyExist2", &(pDocSave3->FuzzyExist_2) );
    RptLogBuildSum2( szSums, "FuzzyUsed2",  &(pDocSave3->FuzzyUsed_2) );
    RptLogBuildSum2( szSums, "FuzzyExist3", &(pDocSave3->FuzzyExist_3) );
    RptLogBuildSum2( szSums, "FuzzyUsed3",  &(pDocSave3->FuzzyUsed_3) );
    RptLogBuildSum2( szSums, "MachExist",   &(pDocSave3->MachExist) );
    RptLogBuildSum2( szSums, "MachUsed",    &(pDocSave3->MachUsed) );
    RptLogBuildSum2( szSums, "NoneExist",   &(pDocSave3->NoneExist) );
    RptLogBuildSum2( szSums, "NoneExist2",  &(pDocSave3->NoneExist2) );
    RptLogBuildSum2( szSums, "NotXlated",   &(pDocSave3->NotXlated) );

    #define CATSUM( s ) ( s.ComplexSum.ulSrcWords + s.MediumSum.ulSrcWords + s.SimpleSum.ulSrcWords )

    ULONG ulTotal = CATSUM( pDocSave3->AnalAutoSubst ) + CATSUM( pDocSave3->EditAutoSubst ) + CATSUM( pDocSave3->ExactExist) +
                            CATSUM( pDocSave3->ReplExist ) + CATSUM( pDocSave3->FuzzyExist ) +
                            CATSUM( pDocSave3->MachExist) + CATSUM( pDocSave3->NoneExist ) + 
                            CATSUM( pDocSave3->NotXlated );

    sprintf( szSums + strlen(szSums), "TotalWords(%lu)", ulTotal );

    fprintf( hRptLog2, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function myRptLogDocSave3 */

void RptLogDocPropSums2( PSZ pszString, PPROPDOCUMENT pProp )
{
  if ( fRptLogging2 )
  {
    static char szSums[1024];

    szSums[0] = EOS;
    RptLogBuildCountSum2( szSums, "Total",      &(pProp->Total) );
    RptLogBuildCountSum2( szSums, "ExactExact", &(pProp->ExactExact) );
    RptLogBuildCountSum2( szSums, "Exact1",     &(pProp->ExactOne) );
    RptLogBuildCountSum2( szSums, "Exact+",     &(pProp->ExactMore) );
    RptLogBuildCountSum2( szSums, "Repl",       &(pProp->Repl) );
    RptLogBuildCountSum2( szSums, "Fuzzy1",     &(pProp->Fuzzy1 ) );
    RptLogBuildCountSum2( szSums, "Fuzzy2",     &(pProp->Fuzzy2 ) );
    RptLogBuildCountSum2( szSums, "Fuzzy3",     &(pProp->Fuzzy3 ) );
    RptLogBuildCountSum2( szSums, "NoProp",     &(pProp->NoProps ) );
    RptLogBuildCountSum2( szSums, "MTProps",    &(pProp->MTProps ) );

    fprintf( hRptLog2, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function RptLogDocPropSums2 */


void RptLogBuildSum2( PSZ pszBuffer, PSZ pszCol, PCRITERIASUM pSum )
{
  ULONG ulSum = pSum->ComplexSum.ulSrcWords +
                pSum->MediumSum.ulSrcWords +
                pSum->SimpleSum.ulSrcWords;
  if ( ulSum )
  {
    char szTemp[20];
    sprintf( szTemp, "(%lu)", ulSum );
    sprintf( pszBuffer + strlen(pszBuffer), "%s%-8s ", pszCol, szTemp );
  } /* endif */
} /* end of function RptLogBuildSum2 */

void RptLogBuildSumEx2( PSZ pszBuffer, PSZ pszCol, PCRITERIASUMEX pSum )
{
  ULONG ulSum = pSum->ComplexSum.ulSrcWords +
                pSum->MediumSum.ulSrcWords +
                pSum->SimpleSum.ulSrcWords;
  if ( ulSum )
  {
    char szTemp[20];
    sprintf( szTemp, "(%lu)", ulSum );
    sprintf( pszBuffer + strlen(pszBuffer), "%s%-8s ", pszCol, szTemp );
  } /* endif */
} /* end of function RptLogBuildSumEx2 */

void RptLogBuildCountSum2( PSZ pszBuffer, PSZ pszCol, PCOUNTSUMS pSum )
{
  ULONG ulSum = pSum->ulComplexWords +
                pSum->ulMediumWords +
                pSum->ulSimpleWords;
  if ( ulSum )
  {
    char szTemp[20];
    sprintf( szTemp, "(%lu)", ulSum );
    sprintf( pszBuffer + strlen(pszBuffer), "%s%-8s ", pszCol, szTemp );
  } /* endif */
} /* end of function RptLogBuildCountSum2 */

// convert a history log task ID to a task name 
void RPTTaskIdToTask
( 
  LOGTASK Task,                 // histlog task ID
  PSZ pszBuffer                 // buffer for task name
)
{
  int i = 0;

  strcpy( pszBuffer, "unknown task" ); // preset with default

  while ( TaskIDtoName[i].szName[0] )
  {
    if ( TaskIDtoName[i].TaskID == Task )
    {
      strcpy( pszBuffer, TaskIDtoName[i].szName ); 
      break;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
} /* end of function RPTTaskIdToTask */

// check if profile has been modified
BOOL RptCheckProfile( PRPT pRpt )
{
  BOOL fProfileOK = TRUE;

  Utlstrccpy( pRpt->szWorkString, pRpt->szProfile, DOT );

  // check if we are dealing with a protected profile
  if ( (pRpt->szProfile[0] != EOS) && (RPTIsProtectedProfile( pRpt->szWorkString  ) ) )
  {
    // this is a protected profile (PUB/PII) check if protection is intact
    HPROP        hpropFolder;
    PPROPFOLDER  ppropFolder;
    ULONG        ulErrorInfo;
    CHAR         szProfile[MAX_EQF_PATH];
    BOOL         fProfileIsIntact = FALSE;
    
    UtlMakeEQFPath( szProfile, NULC, SYSTEM_PATH, NULL );
    strcat( szProfile, BACKSLASH_STR );
    strcat( szProfile, pRpt->szProfile );
    hpropFolder = OpenProperties( szProfile, NULL, PROP_ACCESS_READ, &ulErrorInfo );
    if ( hpropFolder )
    {
      ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd(hpropFolder);
      if ( RptCheckProfileStamp( ppropFolder ) )
      {
        if ( ppropFolder->szIntProfileName[0] != EOS )
        { 
          fProfileIsIntact = TRUE;
          strcpy( pRpt->szIntProfileName, ppropFolder->szIntProfileName );
        } /* endif */
      } /* endif */
      CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo);
    } /* endif */
    
    if ( !fProfileIsIntact )
    {
      fProfileOK  = FALSE;
    } /* endif */
  } /* endif */
  return( fProfileOK );
} /* end of function RptCheckProfile */

// write the given fuzzyness level as attribute
BOOL RptWriteFuzzyLevel( CXmlWriter *xw, PSZ pszType, LONG lLevel )
{
  CHAR szLevel[20];  // buffer for level string

  sprintf( szLevel, "%.1ld.%ld", lLevel / 100, lLevel % 100 );
  xw->WriteAttributeString( pszType, szLevel );

  return( TRUE );
} /* end of function RptWriteFuzzyLevel */

BOOL RPTPrepareMemoryMatchCountX(BOOL bDde, PRPT pRpt)
{
  SHORT     sIndex2 = 0;        // index for PALLINFOs
  SHORT     sMaxDocs = 0;       // number of documents in invisible listbox
  SHORT     sMaxLoop;           // number of documents in invisible listbox
  BOOL      fOk = TRUE;         // error indicator
  PVOID     hProp;              // handle of document properties
  EQFINFO   ErrorInfo;          // error info from EQF API call


  PVOID          pArray = NULL;        // pointer to allocated memory
  PPCALCINFO     ppCalcInfoFieldTmp = NULL;  // pointer to PCALCINFO array
  PCALCINFO      pCalcInfoTmp = NULL;        // pointer to CALCINFO structure
  BOOL           fDocImport = FALSE;  // indicator of document import
  PDOCSAVEHISTEX pDocSaveHistSumTmp;  // pointer to DOCSAVEHISTEX of CALCINFOs
  CRITERIASUM    DocSaveHistTmp;      // Count information
  CRITERIASUM    DocSaveHistFuzzy;    // pointer to DOCSAVEHIST of ALLINFOs (INPUT)

  // redundancy counting
  CHAR           szInFile[MAX_LONGFILESPEC] ;
  FILE           *hInput = NULL;      // input file handle
  REDCOUNTHEADER Header;
  REDCOUNTDOC    DocHeader;
  ULONG          ulLength;            // overall file length
  LONG           NumberOfDocuments=0; // Number of Documents in Redundancy int iDoc = 0;
  COUNTSUMS      RedundancySum;       // total sum of inner document
                                      // redundancies in the folder
  COUNTSUMS      UniqueCount;         // Number of segments to be trans
  COUNTSUMS      CrossDocRedund;      // cross-document redundancys
  INT            iDoc;
  PRPT_REDUND    pPRPT_REDUND = NULL; // pointer to Redundancy Data
  PRPT_REDUND    pRPT_REDUND_START = NULL;   // Array Starting Point
  PRPT_REDUND    pRPT_REDUND_END = NULL;     // Array END
  RPT_REDUND     RPT_REDUND_COPY;     // Array Starting Point
  INT            iRed,jRed;           // Redundancy index
  PRPT_REDUND    pPRPT_REDUND_1;      // pointer to Redundancy Data
  PRPT_REDUND    pPRPT_REDUND_2;      // pointer to Redundancy Data
  PSZ            pszFolder;

  memset( &RedundancySum, 0, sizeof(RedundancySum) );
  memset( &Header, 0, sizeof(Header) );
  memset( &UniqueCount, 0, sizeof(UniqueCount) );
  memset( &CrossDocRedund, 0, sizeof(CrossDocRedund) );
  memset( &DocSaveHistFuzzy, 0, sizeof(DocSaveHistFuzzy) );

  /************************************************************/
  /*     Redundancy Counting                                  */
  /************************************************************/
  if ( pRpt->usReport == REDUNDANCY_REPORT )
  {
    RedundancySum.ulSimpleSegs   = 0;
    RedundancySum.ulSimpleWords  = 0;
    RedundancySum.ulMediumSegs   = 0;
    RedundancySum.ulMediumWords  = 0;
    RedundancySum.ulComplexSegs  = 0;
    RedundancySum.ulComplexWords = 0;

    /* Open input file */
    if ( fOk )
    {
      strcpy(szInFile,pRpt->szFolderObjName);
      strcat(szInFile,"\\property\\redund.log");

      hInput = fopen( szInFile, "rb" );
      if ( hInput == NULL )
      {
        pszFolder =  (pRpt->szFolder);
        fOk = FALSE;

        if ( pRpt ) pRpt->usRptStatus = RPT_KILL;

        if (bDde == TRUE)
        {
          UtlErrorHwnd (MESSAGE_RPT_NO_REDUND_AVAIL, MB_OK, 1,
                        &pszFolder, EQF_INFO, pRpt->hwndErrMsg);
        }
        else if (bDde == FALSE)
        {
          printf( "Info==>RPTPrepareMemoryMatchCountX InFile could not be opened %s\n", szInFile );
        } // end if
        pRpt->fErrorPosted = TRUE;

        // Kill the window
      }
      else
      {
        ulLength = _filelength( _fileno(hInput) );
      } // endif
    } /* endif */

    /***************************/
    /* Read through input file */
    /***************************/
    if ( fOk )
    {
      iDoc = 0;

      // read header
      if ( fread( (PVOID)&Header, sizeof(Header), 1, hInput ) != 1 )
      {
        printf( "Error: Read of file header failed!.\n" );
        fOk = FALSE;
      }
      else
      {
        struct tm   *pTimeDate;    // time/date structure
        // correction: + 3 hours
        if ( Header.lTime != 0L )
        {
          Header.lTime += 10800L;         // not necessary any more
        } /* endif */

        pTimeDate = localtime( &Header.lTime );
        NumberOfDocuments = Header.lDocuments;
        UniqueCount = Header.UniqueCount;
        CrossDocRedund = Header.UniqueCount;  

        // alloc memory for PRPT_REDUND size NumberOfDocuments
        fOk = UtlAlloc ((PVOID*)&pArray, 0L, (ULONG)(NumberOfDocuments*sizeof(RPT_REDUND)), ERROR_STORAGE);

        pPRPT_REDUND = (PRPT_REDUND)pArray;     // set tmp pointer to Array
        pRPT_REDUND_START = (PRPT_REDUND)pArray;
      } /* endif */
    } /* endif */


    /*******************************/
    /* loop through document table */
    /*******************************/
    if ( fOk )
    {
      iDoc = 0;
      while ( fOk && (iDoc < Header.lDocuments) && !feof( hInput ) )
      {
        // read fixed part of document data
        if ( fread( (PVOID)&DocHeader, sizeof(DocHeader), 1, hInput ) != 1 )
        {
          printf( "Error: Read of document header failed!.\n" );
          fOk = FALSE;
        }
        else
        {
          strcpy(pPRPT_REDUND->szShortName, DocHeader.szDocShortName);

          pPRPT_REDUND->DocuRed.ulSimpleSegs   = 0;
          pPRPT_REDUND->DocuRed.ulSimpleWords  = 0;
          pPRPT_REDUND->DocuRed.ulMediumSegs   = 0;
          pPRPT_REDUND->DocuRed.ulMediumWords  = 0;
          pPRPT_REDUND->DocuRed.ulComplexSegs  = 0;
          pPRPT_REDUND->DocuRed.ulComplexWords = 0;

          pPRPT_REDUND->OtherRed.ulSimpleSegs   = 0;
          pPRPT_REDUND->OtherRed.ulSimpleWords  = 0;
          pPRPT_REDUND->OtherRed.ulMediumSegs   = 0;
          pPRPT_REDUND->OtherRed.ulMediumWords  = 0;
          pPRPT_REDUND->OtherRed.ulComplexSegs  = 0;
          pPRPT_REDUND->OtherRed.ulComplexWords = 0;
        } /* endif */

        // read/list count sums
        if ( fOk )
        {
          int j = 0;

          while ( fOk && (j < Header.lDocuments) )
          {
            COUNTSUMS Count;

            if ( fread( (PVOID)&Count, sizeof(Count), 1, hInput ) != 1 )
            {
              fOk = FALSE;
            }
            else
            {
              // calculate inner document redundancies
              //
              if ( j==iDoc )
              {
                pPRPT_REDUND->DocuRed.ulSimpleSegs   = Count.ulSimpleSegs  ;
                pPRPT_REDUND->DocuRed.ulSimpleWords  = Count.ulSimpleWords ;
                pPRPT_REDUND->DocuRed.ulMediumSegs   = Count.ulMediumSegs  ;
                pPRPT_REDUND->DocuRed.ulMediumWords  = Count.ulMediumWords ;
                pPRPT_REDUND->DocuRed.ulComplexSegs  = Count.ulComplexSegs ;
                pPRPT_REDUND->DocuRed.ulComplexWords = Count.ulComplexWords;
              } /* endif */

              /*************************************/
              /* Redundancies from other documents */
              /*************************************/
              if ( j!=iDoc )
              {
                pPRPT_REDUND->OtherRed.ulSimpleSegs   += Count.ulSimpleSegs  ;
                pPRPT_REDUND->OtherRed.ulSimpleWords  += Count.ulSimpleWords ;
                pPRPT_REDUND->OtherRed.ulMediumSegs   += Count.ulMediumSegs  ;
                pPRPT_REDUND->OtherRed.ulMediumWords  += Count.ulMediumWords ;
                pPRPT_REDUND->OtherRed.ulComplexSegs  += Count.ulComplexSegs ;
                pPRPT_REDUND->OtherRed.ulComplexWords += Count.ulComplexWords;
                pPRPT_REDUND->OtherRedWords = Count.ulSimpleWords +
                                              Count.ulMediumWords +
                                              Count.ulComplexWords;
              } /* endif */
            } /* endif */
            j++;
          } /* endwhile */
        } /* endif */

        // next document
        iDoc++;
        pPRPT_REDUND ++;
        if ( (iDoc == (Header.lDocuments - 1)) )
        {
          pRPT_REDUND_END = pPRPT_REDUND ;
        } /* endif */
      } /* endwhile */
    } /* endif */

    /*************/
    /* Cleanup   */
    /*************/
    if ( hInput )       fclose( hInput );


    fOk = RptCheckDocNames( pRpt, pRPT_REDUND_START, NULL, NumberOfDocuments );
    //// check if the report is done for the same set of documents which have been prepared for redundance report 
    //if ( fOk )
    //{
    //  BOOL fBufferIsFull = FALSE;
    //  int iMissingDocs = 0;                      // # of documents not contained in redundancy data
    //  int iNotSelectedDocs = 0;                  // # of documents not selected for report
    //  SHORT sIndex = 0;

    //  sMaxDocs = QUERYITEMCOUNTHWND( pRpt->hwndRptHandlerLB );
    //  pRpt->szWorkString[0] = EOS;

    //  while ( sIndex < sMaxDocs )
    //  {
    //    int iDoc = 0;
    //    BOOL fFound = FALSE;

    //    pPRPT_REDUND = pRPT_REDUND_START; 
    //    QUERYITEMTEXTHWND( pRpt->hwndRptHandlerLB, sIndex++, pRpt->pszActualDocument );
    //    while ( !fFound && (iDoc < NumberOfDocuments) )
    //    {
    //      if ( _stricmp( pRpt->pszActualDocument, pPRPT_REDUND->szShortName ) == 0  )
    //      {
    //        fFound = TRUE;
    //        pPRPT_REDUND->fUsed = TRUE;
    //      }
    //      else
    //      {
    //        // try next one
    //        pPRPT_REDUND ++;
    //        iDoc++;
    //      } /* endif */
    //    } /* end while */

    //    // handle not found documents
    //    if ( !fFound )
    //    {
    //      int iDocNameLen = 0;
    //      int iFilled = strlen( pRpt->szWorkString );

    //      if ( fBufferIsFull )
    //      {
    //        // no more room left in doc name buffer..
    //      }
    //      else
    //      {
    //        RptGetDocLongName( pRpt, pRpt->pszActualDocument, pRpt->szLongActualDocument );
    //        iDocNameLen = strlen( pRpt->szLongActualDocument );
    //        if ( (iFilled + iDocNameLen + 2) < (sizeof(pRpt->szWorkString)/2) )
    //        {
    //          if ( iMissingDocs == 0 )
    //          {
    //            strcat( pRpt->szWorkString, "Not prepared: " );
    //          }
    //          else
    //          {
    //            strcat( pRpt->szWorkString, ", " );
    //          } /* endif */
    //          strcat( pRpt->szWorkString, pRpt->szLongActualDocument );
    //        }
    //        else
    //        {
    //          strcat( pRpt->szWorkString, "..." );
    //          fBufferIsFull = TRUE;
    //        } /* endif */
    //      } /* endif */
    //      iMissingDocs++;
    //    } /* endif */
    //  } /*endwhile */

    //  // handle documents contained in redundancy data but not selected for the report
    //  {
    //    BOOL fBufferIsFull = FALSE;
    //    int iDoc = 0;

    //    pPRPT_REDUND = pRPT_REDUND_START; 

    //    while ( iDoc < NumberOfDocuments )
    //    {
    //      if ( !pPRPT_REDUND->fUsed )
    //      {
    //        int iDocNameLen = 0;
    //        int iFilled = strlen( pRpt->szWorkString );

    //        if ( fBufferIsFull )
    //        {
    //          // ignore document, no more room in buffer
    //        }
    //        else
    //        {
    //          RptGetDocLongName( pRpt, pPRPT_REDUND->szShortName, pRpt->szLongActualDocument );
    //          iDocNameLen = strlen( pRpt->szLongActualDocument );
    //          if ( (iFilled + iDocNameLen + 2) < (sizeof(pRpt->szWorkString) - 5) )
    //          {
    //            if ( iNotSelectedDocs == 0 )
    //            {
    //              if ( iMissingDocs != 0 ) strcat( pRpt->szWorkString, ",  " );
    //              strcat( pRpt->szWorkString, "Prepared, but not selected: " );
    //            }
    //            else
    //            {
    //              strcat( pRpt->szWorkString, ", " );
    //            } /* endif */
    //            strcat( pRpt->szWorkString, pRpt->szLongActualDocument );
    //          }
    //          else 
    //          {
    //            strcat( pRpt->szWorkString, ",..." );
    //            fBufferIsFull = TRUE;
    //          } /* endif */
    //        } /* endif */
    //        iNotSelectedDocs++;
    //      } /* endif */
    //      iDoc++;
    //      pPRPT_REDUND++;
    //    } /* end while */
    //  }

    //  // show warning
    //  if ( (iMissingDocs > 0) || (iNotSelectedDocs > 0) )
    //  {
    //    PSZ pszParm = pRpt->szWorkString;
    //    OEMTOANSI( pRpt->szWorkString );
    //    UtlErrorHwnd( ERROR_REDREPORT_DOCMISMATCH, MB_OK, 1, &pszParm, EQF_WARNING, pRpt->hwndErrMsg );
    //  } /* endif */
    //} /* endif */


    /**********************************/
    /* Bubble sort pRPT_REDUND        */
    /* use alphabetic order           */
    /* or order give by OtherRedWords */
    /**********************************/
    if ( fOk && (NumberOfDocuments > 1) )
    {

      for ( iRed=1 ;iRed<NumberOfDocuments ; iRed++ )
      {

        pPRPT_REDUND = pRPT_REDUND_END;
        for ( jRed=NumberOfDocuments-1 ;jRed>=iRed ;jRed-- )
        {
          pPRPT_REDUND_1 = pPRPT_REDUND; // j
          pPRPT_REDUND --;
          pPRPT_REDUND_2 = pPRPT_REDUND; // j - 1

          // process optimized
          //
          if ( pRpt->usOption5 == Optimized )
          {
            if ( (pPRPT_REDUND_1->OtherRedWords -
                  pPRPT_REDUND_2->OtherRedWords)> 0 )
            {
              RPT_REDUND_COPY   = *(pPRPT_REDUND_1);
              *(pPRPT_REDUND_1) = *(pPRPT_REDUND_2);
              *(pPRPT_REDUND_2) = RPT_REDUND_COPY  ;
            } /* endif */
          }
          // alphabetically ordered
          //
          else
          {
            if ( strcmp(pPRPT_REDUND_1->szShortName,
                        pPRPT_REDUND_2->szShortName)< 0 )
            {
              RPT_REDUND_COPY   = *(pPRPT_REDUND_1);
              *(pPRPT_REDUND_1) = *(pPRPT_REDUND_2);
              *(pPRPT_REDUND_2) = RPT_REDUND_COPY  ;
            } /* endif */
          } /* endif */
        } /* endfor */
      } /* endfor */
    } /* endif */
  } /* endif REDUNDANCY_REPORT*/

  /************************************************************/
  /*    Memory Match Counting                                 */
  /************************************************************/
  if ( fOk )
  {
    /************************************/
    /* Loop all documents in listbox    */
    /************************************/
    if ( pRpt->usReport == REDUNDANCY_REPORT )
    {
      WinSendMsg( pRpt->hwndRptHandlerLB, LM_DELETEALL, 0L, 0L );

      pPRPT_REDUND = pRPT_REDUND_START;     // set tmp pointer to Array
      iDoc = 0;
      while ( (iDoc < NumberOfDocuments) )
      {
        WinSendMsg( pRpt->hwndRptHandlerLB, LM_INSERTITEM,
                    MP1FROMSHORT( LIT_END ),
                    pPRPT_REDUND->szShortName );
        pPRPT_REDUND ++;
        iDoc++;
        pRpt->usProcessedDocuments++; 
      } /* end while */
    } /* end if */

    sMaxDocs = QUERYITEMCOUNTHWND( pRpt->hwndRptHandlerLB );

    // Alloc Memory
    if ( pRpt->usReport == REDUNDANCY_REPORT )
    {
      // alloc memory for PCALCINFO array, size sMaxDocs+1; Cross Document Redundancies
      fOk = UtlAlloc ((PVOID*)&pArray, 0L,
                      (ULONG)((sMaxDocs+1)*sizeof(CALCINFO)), ERROR_STORAGE);
    }
    else
    {
      // alloc memory for PCALCINFO array, size sMaxDocs
      fOk = UtlAlloc ((PVOID*)&pArray, 0L,
                      (ULONG)(sMaxDocs*sizeof(CALCINFO)), ERROR_STORAGE);
    } /* end if */

  } /* end if */

  if ( fOk )
  {
    pRpt->ppCalcInfoField = (PPCALCINFO)pArray;  // store memory address in RPT
    ppCalcInfoFieldTmp = (PPCALCINFO)pArray;     // set tmp pointer to PCALCINFOs
  } // end if


  if ( fOk )
  {
    /************************************/
    /* Loop all Documents in Selection  */
    /************************************/

    if ( pRpt->usReport == REDUNDANCY_REPORT )
    {
      sMaxLoop = sMaxDocs+1; // Cross Document Redundancies
    }
    else
    {
      sMaxLoop = sMaxDocs;
    } /* end if */


    sIndex2 = 0;
    while ( sIndex2 < sMaxLoop )
    {
      // get document name in listbox
      QUERYITEMTEXTHWND( pRpt->hwndRptHandlerLB, sIndex2++,
                         pRpt->pszActualDocument );

      if ( pRpt->usReport == PRE_ANALYSIS_REPORT ||
           pRpt->usReport == COMBINED_REPORT ||
           pRpt->usReport == REDUNDANCY_REPORT )
      {

        if ( sIndex2<=sMaxDocs )
        {
          //--- open properties of document ---
          hProp = OpenProperties( pRpt->pszActualDocument, pRpt->szFolderObjName, PROP_ACCESS_READ, &ErrorInfo);
          if ( hProp )
          {
            PPROPDOCUMENT   pProp;              // ptr to document properties
            pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );

            // alloc memory for CALCINFO structure
            fOk = UtlAlloc ((PVOID*)&pCalcInfoTmp, 0L, sizeof(CALCINFO),
                            ERROR_STORAGE);

            if ( fOk )
            {
              // ---------------------------
              // Scan Redundancy Information
              // find corresponding record
              // ---------------------------

              if ( pRpt->usReport == REDUNDANCY_REPORT )
              {
                BOOL fFound = FALSE;

                pPRPT_REDUND = pRPT_REDUND_START;     // set tmp pointer to Array
                iDoc = 0;
                while ( fOk && (iDoc < NumberOfDocuments) && !fFound )
                {
                  if ( !strcmp(pPRPT_REDUND->szShortName,pRpt->pszActualDocument) )
                  {
                    fFound = TRUE;
                  }
                  else
                  {
                    pPRPT_REDUND ++;
                    iDoc++;
                  } /* endif */
                } /* end while */
              } /* end if */

              // set document name in RPT and CALCINFO
              strcpy (pCalcInfoTmp->szDocument, pRpt->pszActualDocument);
              strcpy( pCalcInfoTmp->szLongName, pProp->szLongName[0] ? pProp->szLongName : pRpt->pszActualDocument );

              // set adjusted fuzzy level
              pCalcInfoTmp->docFuzzyLevel[0] = pProp->lSmallFuzzLevel/100L;
              pCalcInfoTmp->docFuzzyLevel[1] = pProp->lMediumFuzzLevel/100L;
              pCalcInfoTmp->docFuzzyLevel[2] = pProp->lLargeFuzzLevel/100L;

              // save document shipment
              strcpy( pCalcInfoTmp->szShipment, pProp->szShipment );

              *ppCalcInfoFieldTmp++ = pCalcInfoTmp;  // store address in PCALCINFO array
              pRpt->ulCalcInfoRecords++;             // update number of CALCINFOs in RPT

              fDocImport = FALSE;  // set document import indicator
            } // end if


            if ( fOk )
            {
              // set task ID and date/time in CALCINFO structure
              //pCalcInfoTmp->Task  = pAllInfoFieldTmp->histLogRecord.Task;
              //pCalcInfoTmp->lTime = pAllInfoFieldTmp->histLogRecord.lTime;


              RptLogDocPropSums2( "SumsInDocProp", pProp );

              // set tmp pointer to DOCSAVEHISTEX of PCALCINFO      (target)
              pDocSaveHistSumTmp = &(pCalcInfoTmp->docSaveHistSum);

              DocSaveHistTmp.SimpleSum.ulSrcWords = 0;
              DocSaveHistTmp.MediumSum.ulSrcWords = 0;
              DocSaveHistTmp.ComplexSum.ulSrcWords = 0;
              DocSaveHistTmp.SimpleSum.usNumSegs = 0;
              DocSaveHistTmp.MediumSum.usNumSegs = 0;
              DocSaveHistTmp.ComplexSum.usNumSegs = 0;

              // not provided for the pre-analysis counting
              DocSaveHistTmp.SimpleSum.ulTgtWords = 0;
              DocSaveHistTmp.MediumSum.ulTgtWords = 0;
              DocSaveHistTmp.ComplexSum.ulTgtWords = 0;
              DocSaveHistTmp.SimpleSum.ulModWords = 0;
              DocSaveHistTmp.MediumSum.ulModWords = 0;
              DocSaveHistTmp.ComplexSum.ulModWords = 0;

              // ----------------------
              // Source Words/ Segments
              // ----------------------

              // Analyze Automatic Substitution
              // Exact-Exact  and Exact-One
              DocSaveHistTmp.SimpleSum.ulSrcWords = pProp->ExactExact.ulSimpleWords + pProp->ExactOne.ulSimpleWords;
              DocSaveHistTmp.SimpleSum.usNumSegs = (USHORT)(pProp->ExactExact.ulSimpleSegs + pProp->ExactOne.ulSimpleSegs);
              DocSaveHistTmp.MediumSum.ulSrcWords = pProp->ExactExact.ulMediumWords + pProp->ExactOne.ulMediumWords;
              DocSaveHistTmp.MediumSum.usNumSegs = (USHORT)(pProp->ExactExact.ulMediumSegs + pProp->ExactOne.ulMediumSegs);
              DocSaveHistTmp.ComplexSum.ulSrcWords = pProp->ExactExact.ulComplexWords + pProp->ExactOne.ulComplexWords;
              DocSaveHistTmp.ComplexSum.usNumSegs = (USHORT)(pProp->ExactExact.ulComplexSegs + pProp->ExactOne.ulComplexSegs);
              COPYALLCOUNTS( pDocSaveHistSumTmp->AnalAutoSubst, DocSaveHistTmp);

              // ExactExist/Used
              // Exact-Two

              // memory
              DocSaveHistTmp.SimpleSum.ulSrcWords  = pProp->ExactMore.ulSimpleWords;
              DocSaveHistTmp.SimpleSum.usNumSegs   = (USHORT)pProp->ExactMore.ulSimpleSegs;
              DocSaveHistTmp.MediumSum.ulSrcWords  = pProp->ExactMore.ulMediumWords;
              DocSaveHistTmp.MediumSum.usNumSegs   = (USHORT)pProp->ExactMore.ulMediumSegs;
              DocSaveHistTmp.ComplexSum.ulSrcWords = pProp->ExactMore.ulComplexWords;
              DocSaveHistTmp.ComplexSum.usNumSegs  = (USHORT)pProp->ExactMore.ulComplexSegs;
              // redundancy
              if ( pRpt->usReport == REDUNDANCY_REPORT )
              {
                DocSaveHistTmp.SimpleSum.ulSrcWords  += pPRPT_REDUND->DocuRed.ulSimpleWords;
                DocSaveHistTmp.SimpleSum.usNumSegs   = DocSaveHistTmp.SimpleSum.usNumSegs  + (USHORT)pPRPT_REDUND->DocuRed.ulSimpleSegs;
                DocSaveHistTmp.MediumSum.ulSrcWords  += pPRPT_REDUND->DocuRed.ulMediumWords;
                DocSaveHistTmp.MediumSum.usNumSegs   = DocSaveHistTmp.MediumSum.usNumSegs + (USHORT)pPRPT_REDUND->DocuRed.ulMediumSegs ;
                DocSaveHistTmp.ComplexSum.ulSrcWords += pPRPT_REDUND->DocuRed.ulComplexWords;
                DocSaveHistTmp.ComplexSum.usNumSegs  = DocSaveHistTmp.ComplexSum.usNumSegs + (USHORT)pPRPT_REDUND->DocuRed.ulComplexSegs;

                // reduce total redundancies by inner document redundancies
                CrossDocRedund.ulSimpleWords  -= pPRPT_REDUND->DocuRed.ulSimpleWords;
                CrossDocRedund.ulSimpleSegs   -= pPRPT_REDUND->DocuRed.ulSimpleSegs;
                CrossDocRedund.ulMediumWords  -= pPRPT_REDUND->DocuRed.ulMediumWords;
                CrossDocRedund.ulMediumSegs   -= pPRPT_REDUND->DocuRed.ulMediumSegs ;
                CrossDocRedund.ulComplexWords -= pPRPT_REDUND->DocuRed.ulComplexWords;
                CrossDocRedund.ulComplexSegs  -= pPRPT_REDUND->DocuRed.ulComplexSegs;

              } /* endif */
              COPYALLCOUNTS( pDocSaveHistSumTmp->ExactExist, DocSaveHistTmp);
              COPYALLCOUNTS( pDocSaveHistSumTmp->ExactUsed, DocSaveHistTmp);

              // Fuzzy and Replace matches do not make sence fot the redundancy report
              // as we might get double counting
              if (pRpt->usReport != REDUNDANCY_REPORT)
              {
                // ReplExist/Used
                DocSaveHistTmp.SimpleSum.ulSrcWords = pProp->Repl.ulSimpleWords;
                DocSaveHistTmp.SimpleSum.usNumSegs = (USHORT)pProp->Repl.ulSimpleSegs;
                DocSaveHistTmp.MediumSum.ulSrcWords = pProp->Repl.ulMediumWords;
                DocSaveHistTmp.MediumSum.usNumSegs = (USHORT)pProp->Repl.ulMediumSegs;
                DocSaveHistTmp.ComplexSum.ulSrcWords = pProp->Repl.ulComplexWords;
                DocSaveHistTmp.ComplexSum.usNumSegs = (USHORT)pProp->Repl.ulComplexSegs;
                COPYALLCOUNTS( pDocSaveHistSumTmp->ReplExist, DocSaveHistTmp);
                COPYALLCOUNTS( pDocSaveHistSumTmp->ReplUsed, DocSaveHistTmp);

                // FuzzyExist/Used
                //
                DocSaveHistTmp.SimpleSum.ulSrcWords = pProp->Fuzzy1.ulSimpleWords +
                                                      pProp->Fuzzy2.ulSimpleWords +
                                                      pProp->Fuzzy3.ulSimpleWords;
                DocSaveHistTmp.SimpleSum.usNumSegs  = (USHORT)(pProp->Fuzzy1.ulSimpleSegs +
                                                               pProp->Fuzzy2.ulSimpleSegs +
                                                               pProp->Fuzzy3.ulSimpleSegs);
                DocSaveHistTmp.MediumSum.ulSrcWords = pProp->Fuzzy1.ulMediumWords +
                                                      pProp->Fuzzy2.ulMediumWords +
                                                      pProp->Fuzzy3.ulMediumWords;
                DocSaveHistTmp.MediumSum.usNumSegs  = (USHORT)(pProp->Fuzzy1.ulMediumSegs +
                                                               pProp->Fuzzy2.ulMediumSegs +
                                                               pProp->Fuzzy3.ulMediumSegs);
                DocSaveHistTmp.ComplexSum.ulSrcWords = pProp->Fuzzy1.ulComplexWords +
                                                       pProp->Fuzzy2.ulComplexWords +
                                                       pProp->Fuzzy3.ulComplexWords;
                DocSaveHistTmp.ComplexSum.usNumSegs  = (USHORT)(pProp->Fuzzy1.ulComplexSegs +
                                                                pProp->Fuzzy2.ulComplexSegs +
                                                                pProp->Fuzzy3.ulComplexSegs);

                COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist, DocSaveHistTmp);
                COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed, DocSaveHistTmp);

                // FuzzyExist/Used 1
                DocSaveHistTmp.SimpleSum.ulSrcWords = pProp->Fuzzy1.ulSimpleWords;
                DocSaveHistTmp.SimpleSum.usNumSegs = (USHORT)pProp->Fuzzy1.ulSimpleSegs;
                DocSaveHistTmp.MediumSum.ulSrcWords = pProp->Fuzzy1.ulMediumWords;
                DocSaveHistTmp.MediumSum.usNumSegs = (USHORT)pProp->Fuzzy1.ulMediumSegs;
                DocSaveHistTmp.ComplexSum.ulSrcWords = pProp->Fuzzy1.ulComplexWords;
                DocSaveHistTmp.ComplexSum.usNumSegs = (USHORT)pProp->Fuzzy1.ulComplexSegs;
                COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_1, DocSaveHistTmp);
                COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_1, DocSaveHistTmp);

                // FuzzyExist/Used 2
                DocSaveHistTmp.SimpleSum.ulSrcWords = pProp->Fuzzy2.ulSimpleWords;
                DocSaveHistTmp.SimpleSum.usNumSegs = (USHORT)pProp->Fuzzy2.ulSimpleSegs;
                DocSaveHistTmp.MediumSum.ulSrcWords = pProp->Fuzzy2.ulMediumWords;
                DocSaveHistTmp.MediumSum.usNumSegs = (USHORT)pProp->Fuzzy2.ulMediumSegs;
                DocSaveHistTmp.ComplexSum.ulSrcWords = pProp->Fuzzy2.ulComplexWords;
                DocSaveHistTmp.ComplexSum.usNumSegs = (USHORT)pProp->Fuzzy2.ulComplexSegs;
                COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_2, DocSaveHistTmp);
                COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_2, DocSaveHistTmp);

                // FuzzyExist/Used 3
                DocSaveHistTmp.SimpleSum.ulSrcWords = pProp->Fuzzy3.ulSimpleWords;
                DocSaveHistTmp.SimpleSum.usNumSegs = (USHORT)pProp->Fuzzy3.ulSimpleSegs;
                DocSaveHistTmp.MediumSum.ulSrcWords = pProp->Fuzzy3.ulMediumWords;
                DocSaveHistTmp.MediumSum.usNumSegs = (USHORT)pProp->Fuzzy3.ulMediumSegs;
                DocSaveHistTmp.ComplexSum.ulSrcWords = pProp->Fuzzy3.ulComplexWords;
                DocSaveHistTmp.ComplexSum.usNumSegs = (USHORT)pProp->Fuzzy3.ulComplexSegs;
                COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_3, DocSaveHistTmp);
                COPYALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_3, DocSaveHistTmp);
              }
              else   // add  fuzzy and replace matches to not translated
              {
                // ReplExist/Used
                //
                DocSaveHistFuzzy.SimpleSum.ulSrcWords =  pProp->Repl.ulSimpleWords;
                DocSaveHistFuzzy.SimpleSum.usNumSegs =   (USHORT)pProp->Repl.ulSimpleSegs;
                DocSaveHistFuzzy.MediumSum.ulSrcWords =  pProp->Repl.ulMediumWords;
                DocSaveHistFuzzy.MediumSum.usNumSegs =   (USHORT)pProp->Repl.ulMediumSegs;
                DocSaveHistFuzzy.ComplexSum.ulSrcWords = pProp->Repl.ulComplexWords;
                DocSaveHistFuzzy.ComplexSum.usNumSegs =  (USHORT)pProp->Repl.ulComplexSegs;

                // FuzzyExist/Used
                DocSaveHistFuzzy.SimpleSum.ulSrcWords  += pProp->Fuzzy1.ulSimpleWords +
                                                          pProp->Fuzzy2.ulSimpleWords +
                                                          pProp->Fuzzy3.ulSimpleWords;
                DocSaveHistFuzzy.SimpleSum.usNumSegs   = DocSaveHistFuzzy.SimpleSum.usNumSegs  +
                                                          (USHORT)(pProp->Fuzzy1.ulSimpleSegs +
                                                                   pProp->Fuzzy2.ulSimpleSegs +
                                                                   pProp->Fuzzy3.ulSimpleSegs);
                DocSaveHistFuzzy.MediumSum.ulSrcWords  += pProp->Fuzzy1.ulMediumWords +
                                                          pProp->Fuzzy2.ulMediumWords +
                                                          pProp->Fuzzy3.ulMediumWords;
                DocSaveHistFuzzy.MediumSum.usNumSegs   = DocSaveHistFuzzy.MediumSum.usNumSegs   +
                                                          (USHORT)(pProp->Fuzzy1.ulMediumSegs +
                                                                   pProp->Fuzzy2.ulMediumSegs +
                                                                   pProp->Fuzzy3.ulMediumSegs);
                DocSaveHistFuzzy.ComplexSum.ulSrcWords += pProp->Fuzzy1.ulComplexWords +
                                                          pProp->Fuzzy2.ulComplexWords +
                                                          pProp->Fuzzy3.ulComplexWords;
                DocSaveHistFuzzy.ComplexSum.usNumSegs  = DocSaveHistFuzzy.ComplexSum.usNumSegs + 
                                                          (USHORT)(pProp->Fuzzy1.ulComplexSegs +
                                                                   pProp->Fuzzy2.ulComplexSegs +
                                                                   pProp->Fuzzy3.ulComplexSegs);
              } // end if


              // NoneExist
              DocSaveHistTmp.SimpleSum.ulSrcWords  = pProp->NoProps.ulSimpleWords;
              DocSaveHistTmp.SimpleSum.usNumSegs   = (USHORT)pProp->NoProps.ulSimpleSegs;
              DocSaveHistTmp.MediumSum.ulSrcWords  = pProp->NoProps.ulMediumWords;
              DocSaveHistTmp.MediumSum.usNumSegs   = (USHORT)pProp->NoProps.ulMediumSegs;
              DocSaveHistTmp.ComplexSum.ulSrcWords = pProp->NoProps.ulComplexWords;
              DocSaveHistTmp.ComplexSum.usNumSegs  = (USHORT)pProp->NoProps.ulComplexSegs;

              // Subtract Redundancies and add fuzzies
              if ( pRpt->usReport == REDUNDANCY_REPORT )
              {
                // Subtract Redundancies
                DocSaveHistTmp.SimpleSum.ulSrcWords  -= pPRPT_REDUND->DocuRed.ulSimpleWords;
                DocSaveHistTmp.SimpleSum.usNumSegs   = DocSaveHistTmp.SimpleSum.usNumSegs - 
                                                       (USHORT)pPRPT_REDUND->DocuRed.ulSimpleSegs;
                DocSaveHistTmp.MediumSum.ulSrcWords  -= pPRPT_REDUND->DocuRed.ulMediumWords;
                DocSaveHistTmp.MediumSum.usNumSegs   = DocSaveHistTmp.MediumSum.usNumSegs -
                                                       (USHORT)pPRPT_REDUND->DocuRed.ulMediumSegs ;
                DocSaveHistTmp.ComplexSum.ulSrcWords -= pPRPT_REDUND->DocuRed.ulComplexWords;
                DocSaveHistTmp.ComplexSum.usNumSegs  = DocSaveHistTmp.ComplexSum.usNumSegs -
                                                       (USHORT)pPRPT_REDUND->DocuRed.ulComplexSegs;

                // Add Fuzzies
                DocSaveHistTmp.SimpleSum.ulSrcWords  += DocSaveHistFuzzy.SimpleSum.ulSrcWords;
                DocSaveHistTmp.SimpleSum.usNumSegs   = DocSaveHistTmp.SimpleSum.usNumSegs + DocSaveHistFuzzy.SimpleSum.usNumSegs ;
                DocSaveHistTmp.MediumSum.ulSrcWords  += DocSaveHistFuzzy.MediumSum.ulSrcWords;
                DocSaveHistTmp.MediumSum.usNumSegs   = DocSaveHistTmp.MediumSum.usNumSegs + DocSaveHistFuzzy.MediumSum.usNumSegs ;
                DocSaveHistTmp.ComplexSum.ulSrcWords += DocSaveHistFuzzy.ComplexSum.ulSrcWords;
                DocSaveHistTmp.ComplexSum.usNumSegs  = DocSaveHistTmp.ComplexSum.usNumSegs + DocSaveHistFuzzy.ComplexSum.usNumSegs;

              } /* endif */
              COPYALLCOUNTS( pDocSaveHistSumTmp->NoneExist2, DocSaveHistTmp);
              COPYALLCOUNTS( pDocSaveHistSumTmp->NoneExist, DocSaveHistTmp);

              // NotXlated (eliminated)
              DocSaveHistTmp.SimpleSum.ulSrcWords =  0;
              DocSaveHistTmp.SimpleSum.usNumSegs =   0;
              DocSaveHistTmp.MediumSum.ulSrcWords =  0;
              DocSaveHistTmp.MediumSum.usNumSegs =   0;
              DocSaveHistTmp.ComplexSum.ulSrcWords = 0;
              DocSaveHistTmp.ComplexSum.usNumSegs =  0;
              COPYALLCOUNTS( pDocSaveHistSumTmp->NotXlated, DocSaveHistTmp); 

              // sum of all not translated stuff 
              // now sum up all document redundancies for Redundancy of whole folder
              if ( pRpt->usReport == REDUNDANCY_REPORT )
              {
                RedundancySum.ulSimpleWords  +=  pPRPT_REDUND->DocuRed.ulSimpleWords;
                RedundancySum.ulSimpleSegs   +=  pPRPT_REDUND->DocuRed.ulSimpleSegs;
                RedundancySum.ulMediumWords  +=  pPRPT_REDUND->DocuRed.ulMediumWords;
                RedundancySum.ulMediumSegs   +=  pPRPT_REDUND->DocuRed.ulMediumSegs ;
                RedundancySum.ulComplexWords +=  pPRPT_REDUND->DocuRed.ulComplexWords;
                RedundancySum.ulComplexSegs  +=  pPRPT_REDUND->DocuRed.ulComplexSegs;
              } /* end if */

              // copy MT matches
              if ( pRpt->usReport != REDUNDANCY_REPORT )
              {
                memset( &DocSaveHistTmp, 0, sizeof(DocSaveHistTmp) );
                DocSaveHistTmp.ComplexSum.usNumSegs   = (USHORT)pProp->MTProps.ulComplexSegs;
                DocSaveHistTmp.ComplexSum.ulSrcWords  = pProp->MTProps.ulComplexWords;
                DocSaveHistTmp.MediumSum.usNumSegs    = (USHORT)pProp->MTProps.ulMediumSegs;
                DocSaveHistTmp.MediumSum.ulSrcWords   = pProp->MTProps.ulMediumWords;
                DocSaveHistTmp.SimpleSum.usNumSegs    = (USHORT)pProp->MTProps.ulSimpleSegs;
                DocSaveHistTmp.SimpleSum.ulSrcWords   = pProp->MTProps.ulSimpleWords;
                COPYALLCOUNTS( pDocSaveHistSumTmp->MachExist, DocSaveHistTmp);
                COPYALLCOUNTS( pDocSaveHistSumTmp->MachUsed, DocSaveHistTmp);
              } /* end if */
            } /* end if*/
          }  /* end if */

          if ( hProp ) CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
        }

        /*********************************************/
        /*   Redundancies                            */
        /*********************************************/

        else if ( pRpt->usReport == REDUNDANCY_REPORT && sMaxDocs > 1 )
        {
          sIndex2++;
          // alloc memory for CALCINFO structure
          fOk = UtlAlloc ((PVOID*)&pCalcInfoTmp, 0L, sizeof(CALCINFO), ERROR_STORAGE);

          // set document name in RPT and CALCINFO
          strcpy (pCalcInfoTmp->szDocument, "Redundancies");

          *ppCalcInfoFieldTmp++ = pCalcInfoTmp;  // store address in PCALCINFO array
          pRpt->ulCalcInfoRecords++;             // update number of CALCINFOs in RPT

          fDocImport = FALSE;        // set document import indicator

          // set tmp pointer to DOCSAVEHISTEX of PCALCINFO      (target)
          pDocSaveHistSumTmp = &(pCalcInfoTmp->docSaveHistSum);

          DocSaveHistTmp.SimpleSum.ulSrcWords = 0;
          DocSaveHistTmp.MediumSum.ulSrcWords = 0;
          DocSaveHistTmp.ComplexSum.ulSrcWords = 0;
          DocSaveHistTmp.SimpleSum.usNumSegs = 0;
          DocSaveHistTmp.MediumSum.usNumSegs = 0;
          DocSaveHistTmp.ComplexSum.usNumSegs = 0;

          // not provided for the pre-analysis counting
          DocSaveHistTmp.SimpleSum.ulTgtWords = 0;
          DocSaveHistTmp.MediumSum.ulTgtWords = 0;
          DocSaveHistTmp.ComplexSum.ulTgtWords = 0;
          DocSaveHistTmp.SimpleSum.ulModWords = 0;
          DocSaveHistTmp.MediumSum.ulModWords = 0;
          DocSaveHistTmp.ComplexSum.ulModWords = 0;



          // final calculation of Redundancy sum
          RedundancySum.ulSimpleWords  =  UniqueCount.ulSimpleWords  ;
          RedundancySum.ulSimpleSegs   =  UniqueCount.ulSimpleSegs   ;
          RedundancySum.ulMediumWords  =  UniqueCount.ulMediumWords  ;
          RedundancySum.ulMediumSegs   =  UniqueCount.ulMediumSegs   ;
          RedundancySum.ulComplexWords =  UniqueCount.ulComplexWords ;
          RedundancySum.ulComplexSegs  =  UniqueCount.ulComplexSegs  ;

/*GQTEST*/ memcpy( &RedundancySum, &CrossDocRedund, sizeof(RedundancySum) );

          // ExactExist/Used Exact-Two
          DocSaveHistTmp.SimpleSum.ulSrcWords  = RedundancySum.ulSimpleWords ;
          DocSaveHistTmp.SimpleSum.usNumSegs   = (USHORT)RedundancySum.ulSimpleSegs  ;
          DocSaveHistTmp.MediumSum.ulSrcWords  = RedundancySum.ulMediumWords ;
          DocSaveHistTmp.MediumSum.usNumSegs   = (USHORT)RedundancySum.ulMediumSegs  ;
          DocSaveHistTmp.ComplexSum.ulSrcWords = RedundancySum.ulComplexWords;
          DocSaveHistTmp.ComplexSum.usNumSegs  = (USHORT)RedundancySum.ulComplexSegs ;
          COPYALLCOUNTS( pDocSaveHistSumTmp->ExactExist, DocSaveHistTmp );
          COPYALLCOUNTS( pDocSaveHistSumTmp->ExactUsed, DocSaveHistTmp );

          /**********************************************/
          /* substract Redundancies from manual matches */
          /**********************************************/
          //if ( RedundancySum.ulSimpleWords >= RedundancySum.ulSimpleWords ) DocSaveHistTmp.SimpleSum.ulSrcWords   = 0 - RedundancySum.ulSimpleWords ;
          //if ( RedundancySum.ulSimpleSegs  >= RedundancySum.ulSimpleSegs ) DocSaveHistTmp.SimpleSum.usNumSegs     = (USHORT)(0 - RedundancySum.ulSimpleSegs)  ;
          //if ( RedundancySum.ulMediumWords >= RedundancySum.ulMediumWords ) DocSaveHistTmp.MediumSum.ulSrcWords   = 0 - RedundancySum.ulMediumWords ;
          //if ( RedundancySum.ulMediumSegs  >= RedundancySum.ulMediumSegs ) DocSaveHistTmp.MediumSum.usNumSegs     = (USHORT)(0 - RedundancySum.ulMediumSegs)  ;
          //if ( RedundancySum.ulComplexWords>= RedundancySum.ulComplexWords ) DocSaveHistTmp.ComplexSum.ulSrcWords = 0 - RedundancySum.ulComplexWords;
          //if ( RedundancySum.ulComplexSegs >= RedundancySum.ulComplexSegs ) DocSaveHistTmp.ComplexSum.usNumSegs   = (USHORT)(0 - RedundancySum.ulComplexSegs) ;

          if ( RedundancySum.ulSimpleWords >= UniqueCount.ulSimpleWords ) DocSaveHistTmp.SimpleSum.ulSrcWords   = 0 - RedundancySum.ulSimpleWords ;
          if ( RedundancySum.ulSimpleSegs  >= UniqueCount.ulSimpleSegs ) DocSaveHistTmp.SimpleSum.usNumSegs     = (USHORT)(0 - RedundancySum.ulSimpleSegs)  ;
          if ( RedundancySum.ulMediumWords >= UniqueCount.ulMediumWords ) DocSaveHistTmp.MediumSum.ulSrcWords   = 0 - RedundancySum.ulMediumWords ;
          if ( RedundancySum.ulMediumSegs  >= UniqueCount.ulMediumSegs ) DocSaveHistTmp.MediumSum.usNumSegs     = (USHORT)(0 - RedundancySum.ulMediumSegs)  ;
          if ( RedundancySum.ulComplexWords>= UniqueCount.ulComplexWords ) DocSaveHistTmp.ComplexSum.ulSrcWords = 0 - RedundancySum.ulComplexWords;
          if ( RedundancySum.ulComplexSegs >= UniqueCount.ulComplexSegs ) DocSaveHistTmp.ComplexSum.usNumSegs   = (USHORT)(0 - RedundancySum.ulComplexSegs) ;

          COPYALLCOUNTS( pDocSaveHistSumTmp->NoneExist2, DocSaveHistTmp );
          COPYALLCOUNTS( pDocSaveHistSumTmp->NoneExist, DocSaveHistTmp );
        } /* endif sIndex2<MaxDocs */
      } /* endif */
    } // end while documents

  } /* endif */

  return fOk;
} /* end of function RPTPrepareMemoryMatchConut */


// counting report (ex RptReport2)
BOOL RptCountingReport( PRPT pRpt, CXmlWriter *xw )
{
  BOOL           fOk = TRUE;          // error indicator
  PPCALCINFO     ppCalcInfoFieldTmp;  // pointer to PCALCINFOs
  POUTMRI        pOutputMris;         // pointer to OUTMRI array
  ULONG          ulIndex = 0;         // index of PALLINFOs
  PCALCINFO      pCalcInfoTmp;        // pointer to CALCINFO

  CHAR  szLongFileName[MAX_LONGPATH]; // long filename
  PSZ   pszLongFileName;
  CHAR  szProperty[MAX_LONGPATH];     // properties of file

  pRpt->usStringIndex = 0;             // index for output field

  ppCalcInfoFieldTmp = pRpt->ppCalcInfoField;  // set tmp pointer to PCALCINFOs
  pOutputMris = pRpt->pOutputMris;             // set pointer to OUTMRI

  xw->WriteStartElement( "documents" );

  // build records, loop over all PCALCINFOs
  while ( ulIndex++ < pRpt->ulCalcInfoRecords )
  {
    pRpt->usStringIndex = 0;        // set index of actual output string

    pCalcInfoTmp = *ppCalcInfoFieldTmp;  // set pointer to CALCINFO

    // convert ltime to string
    LONG2DATETIME (pCalcInfoTmp->lTime, pRpt->szWorkString);

    if ( pCalcInfoTmp->szLongName[0] )
    {
      strcpy( szLongFileName, pCalcInfoTmp->szLongName );
    }
    else
    {
      // fetch LongFileName
      strcpy(szProperty,pRpt->szFolderObjName);
      strcat(szProperty,BACKSLASH_STR);
      strcat(szProperty,pCalcInfoTmp->szDocument);
      DocQueryInfo2( szProperty, NULL,NULL,NULL,NULL, szLongFileName, NULL,NULL,FALSE);
      if ( !*szLongFileName )
      {
        strcpy(szLongFileName,pCalcInfoTmp->szDocument);
      }
    }

    switch ( pRpt->usOptions )
    {
      case WITH_TOTALS: // with totals
        // build header, check if actual record is last record -> summary record
        if ( ulIndex < (pRpt->ulCalcInfoRecords) )
        {
          // FILE Name

          // try to split Path information
          pszLongFileName = strrchr(szLongFileName,'\\');

          if ( pszLongFileName )
          {
            *pszLongFileName = EOS;
            pszLongFileName ++;
          } /* endif */

          xw->WriteStartElement( "document" );
          if ( pszLongFileName )
          {
            xw->WriteAttributeString( "path", szLongFileName );
            xw->WriteAttributeString( "nameLong", pszLongFileName );
          }
          else
          {
            xw->WriteAttributeString( "nameLong", szLongFileName );
          } // end if
          xw->WriteAttributeString( "date", pRpt->szWorkString );

          RPTTaskIdToTask( (LOGTASK)pCalcInfoTmp->Task, pRpt->szWorkString  );
          xw->WriteAttributeString( "lastProcess", pRpt->szWorkString );
        }
        else
        {
          // summary record
          xw->WriteStartElement( "document" );
          xw->WriteAttributeString( "nameLong", "Summary" );
          xw->WriteAttributeString( "folder", pRpt->szFolder );
        } // end if
        break; // end case


      case WITHOUT_TOTALS: // without totals
        // try to split Path information
        pszLongFileName = strrchr(szLongFileName,'\\');

        if ( pszLongFileName )
        {
          *pszLongFileName = EOS;
          pszLongFileName ++;
        } /* endif */

        xw->WriteStartElement( "document" );
        if ( pszLongFileName )
        {
          xw->WriteAttributeString( "path", szLongFileName );
          xw->WriteAttributeString( "nameLong", pszLongFileName );
        }
        else
        {
          xw->WriteAttributeString( "nameLong", szLongFileName );
        } // end if
        xw->WriteAttributeString( "date", pRpt->szWorkString );

        RPTTaskIdToTask( (LOGTASK)pCalcInfoTmp->Task, pRpt->szWorkString  );
        xw->WriteAttributeString( "lastProcess", pRpt->szWorkString );
        break; // end case
    } // end switch

    // build table

    // analysis autosubst
    RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst), "analysisAutosubst" );
    RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docSaveHistSum.EditAutoSubst), "editAutosubst" );
    RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docSaveHistSum.ExactExist), "exact" );
    RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docSaveHistSum.ReplExist), "replace" );
    RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docSaveHistSum.FuzzyExist), "fuzzy" );
    RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docSaveHistSum.MachExist), "machine" );
    RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docSaveHistSum.NoneExist), "nomatch" );
    RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docSaveHistSum.NotXlated), "notTranslated" );

    // additional summary lines, if option "with totals" is selected
    if ( pRpt->usOptions == WITH_TOTALS )
    {
      RptSumOut2( pRpt, xw, &(pCalcInfoTmp->docCriteriaSum), "summary" );
    } // end if
    xw->WriteEndElement(); // "document"

    // next entry
    ppCalcInfoFieldTmp++;
  } // end while

  xw->WriteEndElement(); // documents

  return fOk;
} // end of RptCountingReport

// Redundant Segment List (ex RptReport4)
BOOL RptRedundantSegmentList( PRPT pRpt, CXmlWriter *xw )
{
  BOOL           fOk = TRUE;
  CHAR           szInFile[MAX_LONGFILESPEC] ;
  FILE           *hInput = NULL;      // input file handle
  REDCOUNTHEADER Header;
  REDCOUNTDOC    DocHeader;
  ULONG          ulLength;            // overall file length
  ULONG          NumberOfDocuments=0; // Number of Documents in Redundancy int iDoc = 0;
  // redundancies in the folder
  COUNTSUMS      UniqueCount;         // Number of segments to be trans
  INT            iDoc=0;
  MOSTUSEDSEGA   MostUsedSeg;
  MOSTUSEDSEG    MostUsedSegUTF16;
  PDOCNAME       pDocNames = NULL;    // array for document names
  PSZ            pszFolder;

  memset( &Header, 0, sizeof(Header) );
  memset( &MostUsedSeg, 0, sizeof(MostUsedSeg) );

  // Open input file 
  if ( fOk )
  {
    strcpy( szInFile, pRpt->szFolderObjName);
    strcat( szInFile,"\\PROPERTY\\REDUND.LOG");

    hInput = fopen( szInFile, "rb" );
    if ( hInput == NULL )
    {
      pszFolder =  (pRpt->szFolder);
      fOk = FALSE;
      if ( pRpt ) pRpt->usRptStatus = RPT_KILL;
      UtlErrorHwnd (MESSAGE_RPT_NO_REDUND_AVAIL, MB_OK, 1, &pszFolder, EQF_INFO, pRpt->hwndErrMsg);
      pRpt->fErrorPosted = TRUE;
    }
    else
    {
      ulLength = _filelength( _fileno(hInput) );
    } /* endif */
  } /* endif */

  // Read through input file 
  if ( fOk )
  {
    iDoc = 0;

    // read header
    if ( fread( (PVOID)&Header, sizeof(Header), 1, hInput ) != 1 )
    {
      fOk = FALSE;
    }
    else
    {
      struct tm   *pTimeDate;    // time/date structure
      if ( Header.lTime != 0L ) Header.lTime += 10800L;       // correction: + 3 hours
      pTimeDate = localtime( &Header.lTime );
      NumberOfDocuments = Header.lDocuments;
      UniqueCount = Header.UniqueCount;
    } /* endif */
  } /* endif */

  // Allocate buffer for document name table (required to resolve document index numbers in most used segment area)              */
  if ( fOk )
  {
    pDocNames = (PDOCNAME)malloc( Header.lDocuments * sizeof(DOCNAME) );
    if ( pDocNames == NULL )
    {
      fOk = FALSE;
    }
    else
    {
      memset( pDocNames, 0, Header.lDocuments * sizeof(DOCNAME) );
    } /* endif */
  } /* endif */

  // loop through document table                                    
  if ( fOk )
  {
    iDoc=0;
    while ( fOk && (iDoc < Header.lDocuments) && !feof( hInput ) )
    {
      // read fixed part of document data
      if ( fread( (PVOID)&DocHeader, sizeof(DocHeader), 1, hInput ) != 1 )
      {
        fOk = FALSE;
      }
      else
      {
        // add document name to document name array
        strcpy( pDocNames[iDoc].szName, DocHeader.szDocName );
        strcpy( pDocNames[iDoc].szShortName, DocHeader.szDocShortName );
      } /* endif */

      // read/list count sums
      if ( fOk )
      {
        int j = 0;

        while ( fOk && (j < Header.lDocuments) )
        {
          COUNTSUMS Count;

          if ( fread( (PVOID)&Count, sizeof(Count), 1, hInput ) != 1 )
          {
            fOk = FALSE;
          }
          else
          {
            // nothing to do

          } /* endif */
          j++;
        } /* endwhile */
      } /* endif */

      // next document
      iDoc++;

    } /* endwhile */

  } /* endif */

  if ( fOk )
  {
    fOk = RptCheckDocNames( pRpt, NULL, pDocNames, iDoc );
  } /* endif */

  // List most used segment data                                      
  if ( fOk )
  {
    // position to most used segment area
    fseek( hInput, Header.lMostUsedOffset, SEEK_SET );

    // list all entries in the most used segment table
    if ( fOk )
    {
      LONG lEntry = 0;

      // reduce number of entries if only base list is shown
      if ( pRpt->usOption[pRpt->usReport] == BASE_LIST )
      {
        if ( Header.lMostUsedSegments > 100 ) Header.lMostUsedSegments = 100;
      } /* endif */

      while ( fOk && (lEntry < Header.lMostUsedSegments) && !feof( hInput ) )
      {
        // read-in current entry
        if (fread((PVOID) &MostUsedSegUTF16, sizeof(MostUsedSegUTF16), 1, hInput) != 1)
        {
          fOk = FALSE;
        } // endif

        // list entry only if frequency is a above base list limit or if we show the detialed list
        if ( (pRpt->usOption[pRpt->usReport] == DETAILED_LIST) ||  (MostUsedSegUTF16.lFrequency >= 3) )
        {
          // list current entry
          if ( fOk )
          {
            int i = 0;
            PSZ  pszDocTmp;

            xw->WriteStartElement( "redundantEntry" );
            xw->WriteStartAttribute( "num" );
            xw->WriteInt( lEntry );
            xw->WriteEndAttribute();
            xw->WriteStartAttribute( "frequency" );
            xw->WriteInt( MostUsedSegUTF16.lFrequency );
            xw->WriteEndAttribute();

            while ( (i < MAX_MOSTUSEDDOCSPERSEG) && (MostUsedSegUTF16.aDocs[i].ulSegNo != 0) && i < 5 )
            {
              pszDocTmp =  (PSZ)(pDocNames + MostUsedSegUTF16.aDocs[i].sDocIndex);

              xw->WriteStartElement( "document" );
              xw->WriteAttributeString( "folderobjname", pRpt->szFolderObjName );
              xw->WriteAttributeString( "docname", pszDocTmp );
              xw->WriteStartAttribute( "segnum" );
              xw->WriteInt( MostUsedSegUTF16.aDocs[i].ulSegNo );
              xw->WriteEndAttribute();
              xw->WriteEndElement();
              i++;
            } /* endwhile */

            // replace LF with blanks
            {
              PSZ_W pszTemp = MostUsedSegUTF16.szSegment;
              while ( *pszTemp )
              {
                if ( *pszTemp == L'\n' ) *pszTemp = L' ';
                pszTemp++;
              } /*endwhile */
            }

            const WCHAR *pszSeg = MostUsedSegUTF16.szSegment;
            xw->WriteStartElement( "segment" );
            xw->WriteString( pszSeg );
            xw->WriteEndElement(); // "segment"
            xw->WriteEndElement(); // "redundantEntry" 

          } /* endif */
        } /* endif */
        // next entry
        lEntry++;
      } /* endwhile */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( hInput )       fclose( hInput );
  if ( pDocNames )    free( pDocNames );

  return fOk;
} /* end of function RptRedundantSegmentList */

// get document long name from properties
BOOL RptGetDocLongName( PRPT pRpt, PSZ pszShortName, PSZ pszLongName )
{
  BOOL fOK = TRUE;
  PVOID     hProp = NULL;       // handle of document properties
  EQFINFO   ErrorInfo = 0;      // error info from EQF API call

  hProp = OpenProperties( pszShortName, pRpt->szFolderObjName, PROP_ACCESS_READ, &ErrorInfo);
  if ( hProp )
  {
    PPROPDOCUMENT   pProp;              // ptr to document properties
    pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );
    strcpy( pszLongName, pProp->szLongName[0] ? pProp->szLongName : pszShortName );
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
 }
  else
  {
    fOK = FALSE;
    strcpy( pszLongName, pszShortName );
  } /* endif */
  return( fOK );
}


// check if specified documents match the documents prepared for the report
BOOL RptCheckDocNames( PRPT pRpt, PRPT_REDUND pRPT_REDUND_START, PDOCNAME pDocNames, int iNumberOfDocuments )
{
  BOOL fOK = TRUE;

  BOOL fBufferIsFull = FALSE;
  int iMissingDocs = 0;                      // # of documents not contained in redundancy data
  int iNotSelectedDocs = 0;                  // # of documents not selected for report
  SHORT sIndex = 0;
  PRPT_REDUND    pPRPT_REDUND = NULL;        // pointer to Redundancy Data

  SHORT sMaxDocs = QUERYITEMCOUNTHWND( pRpt->hwndRptHandlerLB );
  pRpt->szWorkString[0] = EOS;

  while ( sIndex < sMaxDocs )
  {
    int iDoc = 0;
    BOOL fFound = FALSE;

    pPRPT_REDUND = pRPT_REDUND_START; 
    QUERYITEMTEXTHWND( pRpt->hwndRptHandlerLB, sIndex++, pRpt->pszActualDocument );
    RptGetDocLongName( pRpt, pRpt->pszActualDocument, pRpt->szLongActualDocument );

    while ( !fFound && (iDoc < iNumberOfDocuments) )
    {
      PSZ pszDocName = NULL;

      if ( pRPT_REDUND_START != NULL )
      {
        pszDocName = pPRPT_REDUND->szShortName;
      }
      else
      {
        pszDocName = pDocNames[iDoc].szShortName;
      } /* endif */

      RptGetDocLongName( pRpt, pszDocName, pRpt->szLongNameBuffer );

      if ( _stricmp( pRpt->szLongActualDocument, pRpt->szLongNameBuffer ) == 0  )
      {
        fFound = TRUE;
        if ( pRPT_REDUND_START != NULL )
        {
          pPRPT_REDUND->fUsed = TRUE;
        }
        else
        {
          pDocNames[iDoc].fUsed = TRUE;
        } /* endif */
      }
      else
      {
        // try next one
        if ( pRPT_REDUND_START != NULL )
        {
          pPRPT_REDUND ++;
        } /* endif */
        iDoc++;
      } /* endif */
    } /* end while */

    // handle not found documents
    if ( !fFound )
    {
      // ignore fully translated documents (as these documents will not be listed in the report data)
      EQFINFO ErrorInfo = 0;
      HPROP hProp = OpenProperties( pRpt->pszActualDocument, pRpt->szFolderObjName, PROP_ACCESS_READ, &ErrorInfo);
      if ( hProp )
      {
        PPROPDOCUMENT pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );
        if ( pProp )
        {
          fFound = (pProp->usComplete == 100);
        } /* endif */
        CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
      }//end if
    }//end if

    // handle not found documents
    if ( !fFound )
    {
      int iDocNameLen = 0;
      int iFilled = strlen( pRpt->szWorkString );

      if ( fBufferIsFull )
      {
        // no more room left in doc name buffer..
      }
      else
      {
        RptGetDocLongName( pRpt, pRpt->pszActualDocument, pRpt->szLongActualDocument );
        iDocNameLen = strlen( pRpt->szLongActualDocument );
        if ( (iFilled + iDocNameLen + 2) < (sizeof(pRpt->szWorkString)/2) )
        {
          if ( iMissingDocs == 0 )
          {
            strcat( pRpt->szWorkString, "Not prepared: " );
          }
          else
          {
            strcat( pRpt->szWorkString, ", " );
          } /* endif */
          strcat( pRpt->szWorkString, pRpt->szLongActualDocument );
        }
        else
        {
          strcat( pRpt->szWorkString, "..." );
          fBufferIsFull = TRUE;
        } /* endif */
      } /* endif */
      iMissingDocs++;
    } /* endif */
  } /*endwhile */

  // handle documents contained in redundancy data but not selected for the report
  {
    BOOL fBufferIsFull = FALSE;
    int iDoc = 0;

    pPRPT_REDUND = pRPT_REDUND_START; 

    while ( iDoc < iNumberOfDocuments )
    {
      BOOL fUsed = FALSE;
      if ( pRPT_REDUND_START != NULL )
      {
        fUsed = pPRPT_REDUND->fUsed;
      }
      else
      {
        fUsed = pDocNames[iDoc].fUsed;
      } /* endif */
      if ( !fUsed )
      {
        int iDocNameLen = 0;
        int iFilled = strlen( pRpt->szWorkString );

        if ( fBufferIsFull )
        {
          // ignore document, no more room in buffer
        }
        else
        {
          PSZ pszLongName = NULL;
          if ( pRPT_REDUND_START != NULL )
          {
            RptGetDocLongName( pRpt, pPRPT_REDUND->szShortName, pRpt->szLongActualDocument );
          }
          else
          {
            pszLongName = pDocNames[iDoc].szName;
          } /* endif */
          iDocNameLen = strlen( pszLongName );
          if ( (iFilled + iDocNameLen + 2) < (sizeof(pRpt->szWorkString) - 5) )
          {
            if ( iNotSelectedDocs == 0 )
            {
              if ( iMissingDocs != 0 ) strcat( pRpt->szWorkString, ",  " );
              strcat( pRpt->szWorkString, "Prepared, but not selected: " );
            }
            else
            {
              strcat( pRpt->szWorkString, ", " );
            } /* endif */
            strcat( pRpt->szWorkString, pszLongName );
          }
          else 
          {
            strcat( pRpt->szWorkString, ",..." );
            fBufferIsFull = TRUE;
          } /* endif */
        } /* endif */
        iNotSelectedDocs++;
      } /* endif */
      iDoc++;
      if ( pRPT_REDUND_START != NULL )
      {
        pPRPT_REDUND++;
      } /* endif */
    } /* end while */
  }

  // show warning
  if ( (iMissingDocs > 0) || (iNotSelectedDocs > 0) )
  {
    PSZ pszParm = pRpt->szWorkString;
    OEMTOANSI( pRpt->szWorkString );
    UtlErrorHwnd( ERROR_REDREPORT_DOCMISMATCH, MB_OK, 1, &pszParm, EQF_WARNING, pRpt->hwndErrMsg );
  } /* endif */
  return( fOK );

} /* end of function RptCheckDocNames */

// compute actual ALLINFO record size
ULONG RptGetRecordSize( PALLINFO pAllInfo )
{
  // layout of ALLINFO record: 
  //    ULONG         ulRecord;       // number of record
  //    HISTLOGRECORD histLogRecord;  // HISTLOGRECORD
  //    CHAR          szLongName[MAX_LONGFILESPEC]; // document long name
  //    VARPART       variablePart;  
  
  // the histLogRecord.usSize contains the size of the record in the file, so the size of the variable part of the record
  // is usSize - sizeof(HISTLOGRECORD) - length of the document long name
  USHORT usVarPartSize = pAllInfo->histLogRecord.usSize - sizeof(pAllInfo->histLogRecord) - strlen(pAllInfo->szLongName);

  ULONG ulSize = (PBYTE)&(pAllInfo->variablePart) - (PBYTE)pAllInfo;
  ulSize += usVarPartSize;

  return( ulSize );
}