//+----------------------------------------------------------------------------+
//|EQFRPT01.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author: Michael Sekinger                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Pogramm to build reports for Counting Report                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.2 $ ----------- 21 Sep 2007
// GQ: - modified history log file handling to avoid traps and endless lops when
//       reading corrupted history logs
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.3 $ ----------- 7 Apr 2005
// GQ: - fixed P021530: Deleting a document does not clear the counting information
// 
// 
// $Revision: 1.2 $ ----------- 8 Mar 2005
// GQ: - fixed P021229: incorrect message "no shipments found" when creating pre-analysis report
//     - fixed P021324: Limitations in redundant segment list, display problems with HTML list
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.2 $ ----------- 5 Nov 2004
// GQ: - added new header "documents in shipment" for individual shipments
//     - added "shipment " to shipment number 
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.3 $ ----------- 28 Jul 2004
// GQ: - fixed P018988: Display precessed file count instead of seleted in RR
//       by restructring output in report header
// 
// 
// $Revision: 1.2 $ ----------- 29 Jun 2004
// GQ: - fixed P019656 Problems with redundance report which were caused by an incorrect
//       typecast used for the document name table in RptReport4
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.7 $ ----------- 20 Apr 2004
// GQ: - R008575: added handling for internal profile name
// 
// 
// $Revision: 1.6 $ ----------- 1 Apr 2004
// GQ: - corrected property file handling in pr-analysis report
// 
// 
// $Revision: 1.5 $ ----------- 1 Apr 2004
// -- change redone.. search original file!.
// 
//
// $Revision: 1.4 $ ----------- 3 Mar 2004
// GQ: - added defines to ignore internal import records and import save records
//     - avoid some compiler warnings
// 
// 
// $Revision: 1.3 $ ----------- 2 Feb 2004
// GQ: - fixed P017455: Calculation reports are not shown in TM
//       by restructuring the output file handling
// 
// 
// $Revision: 1.2 $ ----------- 15 Jan 2004
// RJ: delete obsolete code
// 
//
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.2 $ ----------- 14 Nov 2003
// GQ: - changed szActualName in RPT structure to a pointer to a dynamically allocated area
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.4 $ ----------- 30 Sep 2003
// GQ: - changed usAllocs to ulAllocs to avoid overflow of memory tables when processing larger
//       history logs
// 
// 
// $Revision: 1.3 $ ----------- 30 Jul 2003
// GQ: - show profile name in report header and indicate if PUB/PII profile is correctly
//       protected
// 
// 
// $Revision: 1.2 $ ----------- 18 Jul 2003
// GQ: - fixed P017709: Redundancy report counts cross document redundancy word count twice
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.5 $ ----------- 15 Apr 2003
// GQ: - fixed P016800: Special characters of folder names in the calculating report
//       are corrupted
// 
// 
// $Revision: 1.4 $ ----------- 26 Mar 2003
// GQ: - continued code cleanup
// 
// 
// $Revision: 1.3 $ ----------- 12 Mar 2003
// GQ: - added handling for fuzziness levels
// 
// 
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
// 
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.5 $ ----------- 25 Oct 2002
// GQ: fixed IV000081: P0011 folder export with word count data only
//
//
// $Revision: 1.4 $ ----------- 16 Aug 2002
// GQ: - fixed P014997: System insufficient when creating calculating report
//       caused by not freeing the document memory
//
//
// $Revision: 1.3 $ ----------- 30 Jul 2002
// --RJ: fill pDOc->ulOemCodePage prior to EQFBFileRead
//
//
// $Revision: 1.2 $ ----------- 29 Jul 2002
// --RJ: add cp param where needed: use system pref. language
// -- Fix obvious errors in RPTCheckSOurceWOrds: use correct SrcLang, free loaded tables
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.22 $ ----------- 15 Jul 2002
// GQ: - Added addiotional log statements
//
//
// $Revision: 1.21 $ ----------- 15 May 2002
// GQ: - fixed P014799 Additional shipment although no shipment had been done
//       caused by misinterpreting document import in internal format as
//       new shipment
//
//
// $Revision: 1.20 $ ----------- 3 May 2002
// GQ: - Added TranslationManager verion line to reports
//
//
// $Revision: 1.19 $ ----------- 30 Apr 2002
// GQ: - 2nd fix for P014590/P014591: correct NonExist matches when the
//       AutoSubst matches are equal to NonExist matches
//
//
// $Revision: 1.18 $ ----------- 25 Apr 2002
// GQ: - added logging
//     - fixed P014591: CalcReport: number for "Non Matches" show huge difference to WCT
//       which was caused by loosing the X= information in the segmented files due
//       to a bug in the conversion of the segmented files from Unicode to ASCII
//
//
// $Revision: 1.17 $ ----------- 12 Apr 2002
// GQ: - modified sort of histlog record to ensure that records stay in same order
//       as in the history log file (base sort on name and record number rather
//       than on name and date/time)
//
//
// $Revision: 1.16 $ ----------- 27 Mar 2002
// GQ: - fixed P013882: Layout of calculating report not inline anymore
//     - fixed P013650: Different number in WCT and RPT
//
//
// $Revision: 1.15 $ ----------- 18 Mar 2002
// GQ: - Fixed P014125 Count figures doubled after proofread
//       which was caused by incorrect handling of histlog entries
//       created using the API call EQFWRITEHISTLOG
//
//
// $Revision: 1.14 $ ----------- 20 Feb 2002
// GQ: - Fixed KBT1192: Duplicate files in CalculatingReport
//     - Code cleanup 1. step
//
//
// $Revision: 1.13 $ ----------- 6 Feb 2002
// --RJ: kbt969: correct wording for format at docimp/exp ( intern, external, folder, notimported)
// -- avoid duplicated "Source replaced" instead of "Target replaced" in Docimport record
// -- remove warning with call of RptAbbrevFileName
//
//
// $Revision: 1.12 $ ----------- 21 Dec 2001
// MK(21/12/01)
// -- Redundant segmetn list adapted to Unicode
//
//
// $Revision: 1.11 $ ----------- 19 Dec 2001
// AN (19.12.2001)
// -- Quick fix for IVT V000096: Store segments from REDUND.LOG temporarily as
//    UTF-16, then convert them to ASCII for further processing. (RptReport4())
//
//
// $Revision: 1.10 $ ----------- 19 Dec 2001
// MK(19/12/01)
// -- RPTCheckSourceWords(&SegSource) counted words on pData not on pDataW
//
//
// $Revision: 1.9 $ ----------- 17 Dec 2001
// MK(12/12/2001)
// -- Bug in Word Count with AUTOMATICSUBST_LOGTASK3.
//
//
//
// $Revision: 1.8 $ ----------- 10 Dec 2001
// MK(09/12/2001)
// -- Omiit summing of FuzzyLevel column in Rpt3SuccAddRows
// -- Seperated AUTOMATICSUS_LOGTASK3 for DOCSAHVEHIST3
//
//
//
// $Revision: 1.7 $ ----------- 3 Dec 2001
// MK(01/12/2001)
// -- RptBuildOutputString(): Output of header was moved behind creation/output of report.
//    This was bullshit.
// -- Added xxxxLOGTASK3 for DOCSAHVEHIST3
// -- Added output of adjusted fuzzy level (R004901)
//
//
// $Revision: 1.6 $ ----------- 19 Oct 2001
// GQ: - fixed calculating report problem caused by wrong sorting of history log
//       records having the the same document name and the same time stamp
//
//
// $Revision: 1.5 $ ----------- 26 Sep 2001
// MK(25/09/01)
// -- HTML output was corrupt due to DBCS changes. Had to change '0xF5' to 0xf5 of course.
//
//
// $Revision: 1.4 $ ----------- 6 Sep 2001
// KA 08282001
// changed the umlaut with x'F5' to 0xF5, in order to avoide compile error on Japanese(DBCS) build environment.
//
//
// $Revision: 1.3 $ ----------- 4 Sep 2001
// xmh - 23 August 2001 - supressing output when redund.log does not exist
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: Unicode enabling
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
// $Revision: 1.22 $ ----------- 8 Aug 2001
// xmh - 02 August 2001 - fix on output in RPT3PrintRow
//
//
// $Revision: 1.21 $ ----------- 26 Jul 2001
// xmh - 26 July 2001 - fix on history report
//
//
//
//
// $Revision: 1.20 $ ----------- 25 Jul 2001
// xmh - 24 July 2001 - minor changes
//
//
//
//
// $Revision: 1.19 $ ----------- 12 Jul 2001
// xmh - 05 July 2001
// - minor change in RptGetRecords
// - function corrupting calculation report data deleted
//
//
//
//
// $Revision: 1.18 $ ----------- 29 Jun 2001
// xmh - 29 June 2001 - Test statements deleted.
//
//
//
// $Revision: 1.17 $ ----------- 26 Jun 2001
// GQ: Handle long name when skipping history log records
//
//
// $Revision: 1.16 $ ----------- 25 Jun 2001
// GQ: Handle long name when skipping history log records
//
//
// $Revision: 1.15 $ ----------- 20 Jun 2001
// xmh - 19 June 2001 - ISEARCHITEMEXACTHWND added to find exact string in listbox.
//
//
//
// $Revision: 1.14 $ ----------- 18 Jun 2001
// GQ: Added handling for document long names in histlog (only store long name
//     in ALLINO structure right now...)
//
//
// $Revision: 1.13 $ ----------- 1 Jun 2001
// xmh - 31 May 2001 - non-dde code for counting report added
//
//
//
// $Revision: 1.12 $ ----------- 27 Mar 2001
// Fix for PR010445: wrong parameter passing in RptAbbrevFileName(&szLongFileName,65,&szLongFileNameCopy)
//
//
// $Revision: 1.11 $ ----------- 30 Oct 2000
// added <html>- start tag to html view
//
//
// $Revision: 1.10 $ ----------- 25 Oct 2000
// + P009441: changed initial handling of pRpt->szActualDocument in
//            RptMakeCalculationRecords
// + changed RPT3MeanFactor
//
//
//
// $Revision: 1.9 $ ----------- 25 Sep 2000
// -- add support for more than 64k segments
//
//
// $Revision: 1.8 $ ----------- 24 Aug 2000
// added SHIPMENT_HANDLER
//
//
// $Revision: 1.7 $ ----------- 4 May 2000
// - moved function StartBrowser to EQFMAIN.C
//
//
// $Revision: 1.6 $ ----------- 30 Mar 2000
// French vendor problem: New shipment only in case of document delete and document import logtask
//
//
// $Revision: 1.5 $ ----------- 13 Mar 2000
// - fixed PTM KBT0715: Short folder names are used in calculating report
//
//
// $Revision: 1.4 $ ----------- 14 Feb 2000
// - added specific message for damaged history log files
//
//
// $Revision: 1.3 $ ----------- 14 Dec 1999
// standard browser setup
// changed display of complexity factors
// cross document redundancies adapted to new counting
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFRPT01.CV_   1.53   01 Sep 1999 13:33:46   BUILD  $
 *
 * $Log:   K:\DATA\EQFRPT01.CV_  $
 *
 *    Rev 1.53   01 Sep 1999 13:33:46   BUILD
 * fixes for masshandling
 *
 *    Rev 1.52   02 Aug 1999 16:45:58   BUILD
 * possibility to disable export logtask
 *
 *    Rev 1.51   30 Jun 1999 16:02:54   BUILD
 * Adjusted Consistency Checker
 *
 *    Rev 1.50   17 Jun 1999 08:46:18   BUILD
 * No change.
 *
 *    Rev 1.49   17 Jun 1999 08:44:04   BUILD
 * corrected putfiles
 *
 *    Rev 1.48   14 Jun 1999 10:49:30   BUILD
 * added msg's for version control
 *
 *    Rev 1.47   10 Jun 1999 13:01:46   BUILD
 * Implement Version in HistLog
 *
 *    Rev 1.46   07 Jun 1999 18:37:50   BUILD
 * fix for E001141 (quick and dirty)
 *
 *    Rev 1.45   07 Jun 1999 11:02:18   BUILD
 * added plauability testing
 *
 *    Rev 1.44   07 Jun 1999 10:23:52   BUILD
 * - added handling for VERSION_LOGTASK
 *
 *    Rev 1.43   31 May 1999 13:19:24   BUILD
 * Enabled redundand segment list for long path support
 * Eliminated Cross Redundancy wrong counting (E001141)
 *
 *    Rev 1.42   30 Mar 1999 11:59:30   BUILD
 * -
 *
 *    Rev 1.41   29 Mar 1999 15:35:02   BUILD
 * Added StartBrowser to use the default Web-Browser
 *
 *    Rev 1.40   05 Mar 1999 10:58:04   BUILD
 * changed merge mechanism in case of second analyzes
 *
 *    Rev 1.39   03 Mar 1999 15:58:12   BUILD
 * wordcount fix
 *
 *    Rev 1.38   22 Feb 1999 17:43:30   BUILD
 * Export only word count information enabled
 *
 *    Rev 1.37   19 Feb 1999 12:28:46   BUILD
 * Histlog enabled for DocSave2 structure
 * Summary Report only: statistics problem fixed
 *
 *    Rev 1.36   12 Feb 1999 16:13:04   BUILD
 * change in Hist report NO to Task-ID
 * added summary option to count report
 *
 *    Rev 1.35   08 Feb 1999 11:57:04   BUILD
 * changed merge routine in case of 2nd analysis + edit
 *
 *    Rev 1.34   04 Feb 1999 17:53:24   BUILD
 * trap fixed set SHORT -> ULONG in RptSortRecords init
 *
 *    Rev 1.33   01 Feb 1999 11:30:20   BUILD
 * bug fixes for TP52/TM26 alpha
 *
 *    Rev 1.32   01 Feb 1999 09:45:30   BUILD
 * -- reset HTML control pointer in case of errors
 *
 *    Rev 1.31   20 Jan 1999 18:25:20   BUILD
 * histlog inconsistency checker
 *
 *    Rev 1.30   19 Jan 1999 16:44:58   BUILD
 * Changed HTML output format
 *
 *    Rev 1.29   13 Jan 1999 11:04:36   BUILD
 * changed HTML output format
 *
 *    Rev 1.28   11 Jan 1999 16:04:02   BUILD
 * Correct counting in case of second analysis (especially NON MATCHES)
 * Added APIDOC_LOGTASK
 * Added function RptAbbrevFileName
 *
 *    Rev 1.27   18 Dec 1998 11:52:08   BUILD
 * error handling for corrupted histlog
 *
 *    Rev 1.26   15 Dec 1998 11:47:24   BUILD
 * -- adjust to use HTML control
 *
 *    Rev 1.25   07 Dec 1998 17:07:34   BUILD
 * change MAX_PATH
 *
 *    Rev 1.24   07 Dec 1998 11:54:08   BUILD
 * docimpnewtarget_logtask opens new shipments, too
 * problem P006235 Santi Pont
 * implemented handling for longpath
 *
 *    Rev 1.23   07 Dec 1998 10:39:38   BUILD
 * -- change RptOutputToHTMLControlOpen
 *
 *    Rev 1.22   30 Nov 1998 09:51:24   BUILD
 * added output of description field
 * no relative path for segment list
 *
 *    Rev 1.21   23 Nov 1998 14:07:04   BUILD
 * list of most used segments added
 * Fuzzy matches in case of old structure corrected (fuzzy1,..,fuzzy3)
 *
 *    Rev 1.20   16 Nov 1998 17:20:28   BUILD
 * correct HTML output format
 *
 *    Rev 1.19   13 Nov 1998 17:32:16   BUILD
 * change max_path to max_path144
 *
 *    Rev 1.18   13 Nov 1998 12:26:48   BUILD
 * unique counts added
 *
 *    Rev 1.17   09 Nov 1998 18:39:06   BUILD
 * Source/Source Source/NLV Analysis tool implemented
 *
 *    Rev 1.16   15 Oct 1998 19:50:32   BUILD
 * -HTML output table changed
 *
 *    Rev 1.15   12 Oct 1998 10:17:16   BUILD
 * - add support for HTML control
 *
 *    Rev 1.14   29 Sep 1998 07:40:46   BUILD
 * - added auto-correction for defective hist log records
 *
 *    Rev 1.13   14 Sep 1998 15:29:54   BUILD
 * changed for HTML output
 *
 *    Rev 1.12   08 Apr 1998 16:09:56   BUILD
 * change calculation of fuzzyness
 * and counting of copied flags
 *
 *    Rev 1.11   09 Mar 1998 15:07:16   BUILD
 * - corrected some minor errors to enable compile under Windows
 *
 *    Rev 1.10   06 Mar 1998 15:26:12   BUILD
 * included summary counting report for TCs
 *
 *    Rev 1.9   14 Jan 1998 15:54:10   BUILD
 * - enabled compile for Windows 32bit environment
 *
 *    Rev 1.8   07 Oct 1997 07:46:26   BUILD
 * --KBT0006: display the count of NotXLATED fields
 *
 *    Rev 1.7   10 Jul 1997 12:44:10   BUILD
 * corrected compiler switches for
 * FAST97
 *
 *    Rev 1.6   04 Jul 1997 09:08:48   BUILD
 * Long FileNames for history and
 * counting report implemented.
 *
 *    Rev 1.5   11 Jun 1997 17:07:34   BUILD
 * - minor rework to avoid compiler warnings
 *
 *    Rev 1.4   06 May 1997 07:45:56   BUILD
 * -- KAT336: pszNumbers_SSLLLL must have lu format to copy ulong variables
 * -- KAT0333: COPYCOUNTS also if usNumSegs is zero!
 * -- KAT0333: COPYCOUNTS of EditAutoSubst always, not only on first docsave task
 *             after FileImport
 *
 *    Rev 1.3   28 Apr 1997 12:02:50   BUILD
 * - corrected format string for print out
 *
 *    Rev 1.2   05 Mar 1997 19:25:42   BUILD
 * -- changed calculation of not-yet-translated (use only last value)
 *
 *    Rev 1.1   11 Dec 1996 16:27:58   BUILD
 * - avoid compiler warnings
 * - fixed endless loop in RptSortRecords when working under Windows
 * - fixed other errors in file read logic
 *
 *    Rev 1.0   03 Dec 1996 10:00:56   BUILD
 * Initial revision.
 *
*/
//+----------------------------------------------------------------------------+


#define INCL_EQF_TP
#define INCL_EQF_TM
#define INCL_EQF_MORPH
#define INCL_EQF_EDITORAPI
#define INCL_EQF_TAGTABLE


#include "eqf.h"        // general TranslationManager include file
#include "eqfhlog.h"                   // defines and structures of history log
#include "eqfdde.h"
#include "eqfrpt00.h"   // private include file for report
#include "eqfrpt.mri"   // private mri's for report
#include "eqfrpt01.h"   // private include file for report
#include "eqfrpt.id"    // private id's for report
#include "eqffol.h"     // long filenames
#include "eqfrpt.h"
#include "eqfstart.id"  // contains ID of version string

#include "eqfdoc00.h"

#include <io.h>         // C library for I/O
#include <time.h>       // C library for time/date

#ifdef _WINDOWS

  #include <Windows.h>

#endif

// if IGNOREINTERNALIMPORTS is defined, internal document imports are ignored
#define IGNOREINTERNALIMPORTS

// if IGNOREIMPORTSAVERECORDS is defined IMPORT save records are ignored 
#define IGNOREIMPORTSAVERECORDS 

// variables, defines, and prototypes for logging for logging
#define RPTLOGFILE    "C:\\CALCRPT.LOG"
#define RPTLOGTRIGGER "RPTLOGIT.DAT"
FILE *hRptLog = NULLHANDLE;
BOOL fRptLogging = FALSE;

void RptLogStart();
void RptLogEnd();
void RptLogString( PSZ pszString );
void RptLog2String( PSZ pszString1, PSZ pszString2 );
void RptLogBuildSum( PSZ pszBuffer, PSZ pszCol, PCRITERIASUM pSum );
void RptLogBuildSumEx( PSZ pszBuffer, PSZ pszCol, PCRITERIASUMEX pSum );
void RptLogDocSave( PSZ pszString, PDOCSAVEHIST pDocSave );
void RptLogDocSave2( PSZ pszString, PDOCSAVEHIST2 pDocSave2 );
void RptLogDocSave3( PSZ pszString, PDOCSAVEHIST3 pDocSave3 );
void RptLogCalcInfo( PSZ pszString, PCALCINFO pCalcInfo );
void RptLogDocPropSums( PSZ pszString, PPROPDOCUMENT pProp );
void RptLogBuildCountSum( PSZ pszBuffer, PSZ pszCol, PCOUNTSUMS pSum );

// macro to subtract only if result will not be negative
#define SUBTRACTWITHCHECKING( a, b ) if ( a >= b ) a = a - b;

// disable this define to base the processing on the short names in the history log
#define LONG_NAME_IN_HISTLOG


#ifdef LONG_NAME_IN_HISTLOG
int RptAllInfoCompare( const void *pElement1, const void *pElement2 );
#endif

// PageNumber

int PageNumber = 1;


// Table construction, Text Bolock for HTML output

BOOL    fTable  = FALSE;            // within table construction
BOOL    fText   = FALSE;            // within text construction
BOOL    fHeader = FALSE;            // within Table Text

BOOL    fViewTable  = FALSE;            // within table construction
BOOL    fViewText   = FALSE;            // within text construction
BOOL    fViewHeader = FALSE;            // within Table Text



int NumberOfLines = 0;              // number of rows in a table construction
int NumberOfViewLines = 0;              // number of rows in a table construction

// Table header
//*************

typedef char DOCNAME[MAX_LONGFILESPEC];
typedef DOCNAME *PDOCNAME;


typedef char ONE_COMMENT[30];
ONE_COMMENT   Comment_1[MAX_REPORT_COLUMNS]=
{
  // data
  STR_RPT3_COLUMN1_1,
  STR_RPT3_COLUMN1_2,
  STR_RPT3_COLUMN1_3,
  STR_RPT3_COLUMN1_4,
  STR_RPT3_COLUMN1_5,
  STR_RPT3_COLUMN1_6,
  STR_RPT3_COLUMN1_7,
  STR_RPT3_COLUMN1_8,
  STR_RPT3_COLUMN1_9,
  STR_RPT3_COLUMN1_10,
  STR_RPT3_COLUMN1_11,
  STR_RPT3_COLUMN1_12,
  STR_RPT3_COLUMN1_13,
  STR_RPT3_COLUMN1_14,
  STR_RPT3_COLUMN1_15,
  STR_RPT3_COLUMN1_16,
  STR_RPT3_COLUMN1_17,
  // Statistics
  STR_RPT3_COLUMN1_18,
  STR_RPT3_COLUMN1_19,
  STR_RPT3_COLUMN1_20,
  STR_RPT3_COLUMN1_21,
  STR_RPT3_COLUMN1_22,
  STR_RPT3_COLUMN1_23,
  // Used for statistics
  STR_RPT3_COLUMN1_24,
  STR_RPT3_COLUMN1_25,
  STR_RPT3_COLUMN1_26,
  STR_RPT3_COLUMN1_27,
  STR_RPT3_COLUMN1_28,
  STR_RPT3_COLUMN1_29,
  STR_RPT3_COLUMN1_30,
  STR_RPT3_COLUMN1_31
};

ONE_COMMENT   Comment_2[MAX_REPORT_COLUMNS]=
{
  // data
  STR_RPT3_COLUMN2_1,
  STR_RPT3_COLUMN2_2,
  STR_RPT3_COLUMN2_3,
  STR_RPT3_COLUMN2_4,
  STR_RPT3_COLUMN2_5,
  STR_RPT3_COLUMN2_6,
  STR_RPT3_COLUMN2_7,
  STR_RPT3_COLUMN2_8,
  STR_RPT3_COLUMN2_9,
  STR_RPT3_COLUMN2_10,
  STR_RPT3_COLUMN2_11,
  STR_RPT3_COLUMN2_12,
  STR_RPT3_COLUMN2_13,
  STR_RPT3_COLUMN2_14,
  STR_RPT3_COLUMN2_15,
  STR_RPT3_COLUMN2_16,
  STR_RPT3_COLUMN2_17,
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

/**********************************************************************/
/* Format strings for report lines (these strings are used as format  */
/* string for a call to sprintf)                                      */
/*                                                                    */
/* Naming convention:                                                 */
/* pszXXXX_YYYY                                                       */
/*   where XXXX is a descriptive name                                 */
/*         YYYY is a list of parameters (the characters used in       */
/*              sprintf to declare the parameter:                     */
/*               D for SHORT (or USHORT) decimals                     */
/*               S for strings                                        */
/*               C for a single character                             */
/*               X for a hex SHORT value                              */
/*               L for a LONG (or ULONG) decimal)                     */
/*         e.g. the format string "%s %d %c %ld" used as a title      */
/*              string would have the following name:                 */
/*         pszTitle_SDCL                                              */
/**********************************************************************/

PSZ pszStringValue_SS = " %-30.30s :  %s";
PSZ pszNumbers_SSLLLL = "|%-10.10s |%5.5s  |%10.1lu  |%10.1lu  |%10.1lu  |%10.1lu  |";


// static functions
static VOID MergeResults ( PDOCSAVEHISTEX pDoc1, PDOCSAVEHISTEX pDoc2, BOOL fmemreset );
static VOID CopyResults ( PDOCSAVEHISTEX pDoc1, PDOCSAVEHISTEX pDoc2);




void  RPT3PrintLine
(
PRPT        pRpt,
int         Table_Width,
PSZ         szMarker
);


BOOL  RPT3ManageOutput
(
HWND           hwnd,
PRPT           pRpt,
POUTPUT        pOutputField,         // pointer to OUTPUT array
ULONG          ulIndex
);


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptMain                                                   |
//+----------------------------------------------------------------------------+
//|Function call:    RptMain (bDde, hwnd, pRpt)                                 |
//+----------------------------------------------------------------------------+
//|Description:      Main function of building counting reports                |
//+----------------------------------------------------------------------------+
//|Parameters:       HWND hwnd   handle to listbox of list window              |
//|                  PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    Lock at the following code, it explains himself           |
//+----------------------------------------------------------------------------+

BOOL RptMain (BOOL bDde, HWND hwnd, PRPT pRpt)
{
  BOOL fOk = FALSE;      // error indicator


  // SET Page Number

  PageNumber = 1;


  //RptTestApi();

  RptLogStart();

  // get histlog records
  fOk = RptGetRecords (bDde, pRpt);

  if ( fOk && pRpt->usRptStatus == RPT_ACTIVE )
  {
    // load output mris
    fOk = RptLoadOutputMris (pRpt);

    if ( fOk && pRpt->usRptStatus == RPT_ACTIVE )
    {
      switch ( pRpt->usReport )  // selected report
      {
        case HISTORY_REPORT : // history report
          //-----------------------------
          switch ( pRpt->usOptions )
          {
            case BRIEF_SORT_BY_DOCUMENT : // brief, sorted by document
              // more than one document selected ?
              if ( pRpt->usSelectedDocuments > 1 )
              {
                // sort PALLINFOs in document order
                fOk = RptSortRecords (pRpt);
              } // end if
              break; // end case

          } // end switch
          break; // end case

        case CALCULATION_REPORT : // calculation report
          //-----------------------------
          // more than one document selected ?
          if ( pRpt->usSelectedDocuments > 1 )
          {
            // sort PALLINFOs in document order
            fOk = RptSortRecords (pRpt);
          } // end if

          if ( fOk && pRpt->usRptStatus == RPT_ACTIVE )
          {
            // build CALCULATIONINFOs
            fOk = RptMakeCalculationRecords (pRpt);
          } // end if
          break; // end case

        case SUMMARY_COUNTING_REPORT : // calculation report
          //-----------------------------
          // more than one document selected ?
          if ( pRpt->usSelectedDocuments > 1 )
          {
            // sort PALLINFOs in document order
            fOk = RptSortRecords (pRpt);
          } // end if

          if ( fOk && pRpt->usRptStatus == RPT_ACTIVE )
          {
            // build CALCULATIONINFOs
            fOk = RptMakeCalculationRecords (pRpt);
          } // end if
          break; // end case

        case PRE_ANALYSIS_REPORT : // Source/Source Source/NLV report
          //-----------------------------

          if ( fOk && pRpt->usRptStatus == RPT_ACTIVE )
          {
            // build CALCULATIONINFOs
            fOk = RPTPrepareMemoryMatchCount (bDde, pRpt);
          } // end if
          break; // end case

        case REDUNDANCY_REPORT : // Source/Source Source/NLV report
          //-----------------------------

          if ( fOk && pRpt->usRptStatus == RPT_ACTIVE )
          {
            // build CALCULATIONINFOs
            fOk = RPTPrepareMemoryMatchCount (bDde, pRpt);
          } // end if
          break; // end case

        case COMBINED_REPORT : // list of most used segments
          //-----------------------------

          if ( fOk && pRpt->usRptStatus == RPT_ACTIVE )
          {
            ; // nothing to do
          } // end if
          break; // end case
      } // end switch

      if ( fOk && pRpt->usRptStatus == RPT_ACTIVE )
      {
        // build and put output strings
        fOk = RptBuildOutputString (bDde, hwnd, pRpt);


      } // end if
    } // end if
  } // end if

  RptLogEnd();

  // -----------------------------------------
  // case of batch, get rid of the listwindow,
  // DDE_ANSWER
  // -----------------------------------------

  if ( pRpt->fBatch && bDde == TRUE )
  {
    if ( !fOk )
    {
      pRpt->pDDECntRpt->DDEReturn.usRc = UtlGetDDEErrorCode( pRpt->pDDECntRpt->hwndOwner );
    } /* endif */

    WinPostMsg( pRpt->pDDECntRpt->hwndOwner,
                WM_EQF_DDE_ANSWER, NULL,
                &pRpt->pDDECntRpt->DDEReturn );


    WinPostMsg( GETPARENT(hwnd),
                WM_CLOSE, NULL, NULL );

  } /* endif */


  // no need to free non-dde memory as this will be done when program stops execution
  if( bDde == TRUE)
  {
      // free allocated memory
      if ( pRpt ) RptFreeMemory (pRpt);
  }

  // set report state ready
  if ( fOk ) pRpt->usRptStatus = RPT_READY;

  if ( ! fOk && bDde == TRUE)
  {

    WinPostMsg( GETPARENT(hwnd),
                WM_CLOSE, NULL, NULL );

    if ( pRpt ) pRpt->usRptStatus = RPT_KILL;
    if ( !pRpt->fErrorPosted )
      UtlErrorHwnd(ERROR_INTERNAL,MB_CANCEL,0,NULL,INTERNAL_ERROR,
                   pRpt->hwndErrMsg);
  } // end if

  return fOk;
} // end of RptMain


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptGetRecords                                             |
//+----------------------------------------------------------------------------+
//|Function call:    RptGetRecords (bDde, pRpt)                                |
//+----------------------------------------------------------------------------+
//|Description:      This function reads the histlog records of the selected   |
//|                  documents into the ALLINFO structure, and stores it in    |
//|                  a dynamic increasing array of PALLINFOs                   |
//+----------------------------------------------------------------------------+
//|Parameters:       PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    create absolute path of histlog file                      |
//|                  allocate memory for PALLINFOs                             |
//|                  open history log file                                     |
//|                  loop over each histlog record                             |
//|                    reallocate memory for PALLINFOs, if necessary           |
//|                    if fAlloc = TRUE                                        |
//|                      allocate memory for ALLINFO                           |
//|                    read fixed part, check if record is valid               |
//|                    read variable part                                      |
//|                    if history report and folder are selected               |
//|                      if document is in invisible LB                        |
//|                        fContains = TRUE, break                             |
//|                      else                                                  |
//|                        insert document in invisible LB                     |
//|                      store addess in PALLINFO array, fAlloc = TRUE         |
//|                    else                                                    |
//|                      if document is not selected                           |
//|                        fAlloc = FALSE                                      |
//|                      else                                                  |
//|                        store addess in PALLINFO array, fAlloc = TRUE       |
//|                  close history log file                                    |
//+----------------------------------------------------------------------------+

BOOL RptGetRecords (BOOL bDde, PRPT pRpt)
{
  BOOL      fOk = FALSE;                // error indicator
  BOOL      fAlloc = TRUE;              // allocation indicator
  BOOL      fNext = TRUE;               // read next record indicator
  PALLINFO  pAllInfoTmp = NULL;         // pointer to ALLINFO structure
  PPALLINFO ppAllInfoFieldTmp = NULL;   // pointer to PALLINFO array
  HFILE     hfFileHandle = NULLHANDLE;  // file handle
  USHORT    usFileRc = 0;               // return from file handle
  USHORT    usActionTaken = 0;          // action taken by UtlOpen
  ULONG     ulBytesRead = 0;            // number of bytes read by UtlRead
  ULONG     ulAllocs = 1;               // number of allocs of ALLINFOSs
  PVOID     pArray;                     // pointer to allocated memory
  ULONG     ulRecordIndex = 1;          // index of record in file
  SHORT     sItem = 0;                  // return from ISEARCHITEMHWND
  CHAR      szDocName[MAX_PATH144];
  CHAR      szLongFileName[MAX_LONGPATH]; // long filename
  INT       sMaxDocs = 0;
  INT       sIndex2;
  HWND      hwndRptLongLB = NULLHANDLE; // Listbox with long names
  CHAR      szProperty[MAX_LONGPATH];   // properties of file
  HWND      hwnd;
  USHORT    usCnti;
  USHORT    usCntj;
  BOOL      bLastDoc = FALSE;

  // build absolute path of histlog file
  strcpy (pRpt->szHistLogFile, pRpt->szFolderObjName);
  strcat (pRpt->szHistLogFile, RPT_HISTLOG_DIRECTORY);
  strcat (pRpt->szHistLogFile, HISTLOGFILE);

  // allocate memory for PALLINFO array, initial size INIT_SIZE_A
  fOk = UtlAllocHwnd((PVOID*)&pArray, 0L,
                     (ULONG)(INIT_SIZE_A*sizeof(PALLINFO)),
                     ERROR_STORAGE, pRpt->hwndErrMsg );

  if ( fOk )
  {
    pRpt->ppAllInfoField = (PPALLINFO)pArray;  // store memory address in RPT
    ppAllInfoFieldTmp = (PPALLINFO)pArray;     // set tmp pointer
  }

  // ----------------------------------------
  // fill second listbox with long filenames
  // ----------------------------------------


  if ( fOk )
  {
    // 1. invisible listbox for long filenames
    HWND hwndLB = NULLHANDLE;                   // handle of invisible LB
    hwnd =  pRpt->hwndRptHandlerLB ;
    hwndLB = WinCreateWindow (hwnd, WC_LISTBOX, "",
                              LBS_MULTIPLESEL,
                              0, 0, 0, 0,
                              hwnd, HWND_TOP,
                              DID_RPT_INVISIBLE_LB,
                              NULL, NULL);

    hwndRptLongLB = hwndLB;

    if (bDde == FALSE)
    {
        // 2. invisible listbox for short filenames
        hwnd =  pRpt->hwndRptHandlerLB ;
        hwndLB = WinCreateWindow (HWND_DESKTOP,
                                  WC_LISTBOX, "",
                                  LBS_MULTIPLESEL,
                                  0, 0, 0, 0,
                                  hwnd, HWND_TOP,
                                  DID_RPT_INVISIBLE_LB2,
                                  NULL, NULL);

        pRpt->hwndRptHandlerLB = hwndLB;
    }

  } /* endif */


  if (bDde == TRUE)
  {
      PSZ       pNamesBuf = NULL;

      sMaxDocs = QUERYITEMCOUNTHWND( pRpt->hwndRptHandlerLB );

      // alloc storage to hold all names from listbox
      if (!UtlAlloc((PVOID *)&pNamesBuf, 0L, sizeof(CHAR) * sMaxDocs, ERROR_STORAGE))
        fOk = FALSE;

      sIndex2 = 0;
      while ( sIndex2 < sMaxDocs )
      {
        // get document name in listbox
        QUERYITEMTEXTHWND( pRpt->hwndRptHandlerLB, sIndex2++,
                           szDocName );
        // get Long Doc Name

        strcpy(szProperty,pRpt->szFolderObjName);
        strcat(szProperty,BACKSLASH_STR);
        strcat(szProperty,szDocName);
        DocQueryInfo2(szProperty,
                      NULL,NULL,NULL,NULL,
                      szLongFileName,
                      NULL,NULL,FALSE);
        if ( !*szLongFileName )
        {
          strcpy(szLongFileName,szDocName);
        } // endif

        strupr(szLongFileName);

        INSERTITEMENDHWND (hwndRptLongLB, szLongFileName);
      } // end while
  }
#ifdef NON_DDE_CNT_REPORT
  else if (bDde == FALSE)
  {
      pRpt->ulAllInfoRecords = 0;
      pRpt->ulCalcInfoRecords = 0;

      // pRpt->szActualDocument always contains short file names delimited by comma
      {
          // count the documents delimited by comma
          usCnti = 0;
          usCntj = 0;

          while (pRpt->pszActualDocument[usCnti] != EOS)
          {
             if (pRpt->pszActualDocument[usCnti] == ',') usCntj++;
             usCnti++;
          }

          usCntj++;

          pRpt->usSelectedDocuments = usCntj;
          sMaxDocs = usCntj;

          if (sMaxDocs > 0)
          {
              // assemble the long filename and load the invisible Listbox
              sIndex2 = 0;
              while (sIndex2++ < sMaxDocs)
              {
                  usCnti = 0;
                  usCntj = 0;

                  while (pRpt->pszActualDocument[usCntj] != ',' && pRpt->pszActualDocument[usCntj] != EOS)
                    usCntj++;

                  if (pRpt->pszActualDocument[usCntj] != EOS)
                      bLastDoc = TRUE;

                  // copy document name to szActualDocument
                  strncpy(szDocName, pRpt->pszActualDocument, usCntj);
                  szDocName[usCntj] = EOS;

                  strcpy(szProperty, pRpt->szFolderObjName);
                  strcat(szProperty, BACKSLASH_STR);
                  strcat(szProperty, szDocName);

                  DocQueryInfo2(szProperty,
                                NULL,NULL,NULL,NULL,
                                szLongFileName,
                                NULL,NULL,FALSE);

                  if ( !*szLongFileName )
                  {
                    strcpy(szLongFileName,szDocName);
                  } /*endif*/

                  strupr(szLongFileName);

                  {
                    USHORT usRC;
                    usRC = INSERTITEMENDHWND (hwndRptLongLB,
                                              szLongFileName);
                  }
                  strupr(szDocName);

                  {
                    USHORT usRC;
                    usRC = INSERTITEMENDHWND (pRpt->hwndRptHandlerLB,
                                              szDocName);
                  }

                  if (pRpt->pszActualDocument[usCntj] != EOS)
                  {
                    // skip the comma
                    usCntj++;

                    // copy rest of document names to start of pRpt->szActualDocument
                    while (pRpt->pszActualDocument[usCntj] != EOS)
                        pRpt->pszActualDocument[usCnti++] = pRpt->pszActualDocument[usCntj++];

                    pRpt->pszActualDocument[usCnti] = EOS;
                  } // end if
              } // end while
          } // end if
      } // end if (pRpt->fFolderSelected == FALSE)
  } // end else if (bDde == FALSE)
#endif

  if ( fOk )
  {
    // open histlog file
    usFileRc = UtlOpenHwnd (pRpt->szHistLogFile,
                            &hfFileHandle,
                            &usActionTaken,
                            0L,
                            FILE_NORMAL,
                            FILE_OPEN,
                            OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                            0L,
                            TRUE,
                            pRpt->hwndErrMsg );
    fOk = (usFileRc == NO_ERROR);
    if ( !fOk ) pRpt->fErrorPosted = TRUE; // error msg has been shown
  }

  // ---------------------------------------------
  //
  // loop over each histlog record of histlog file
  //
  // ---------------------------------------------

  while ( fOk == TRUE && fNext == TRUE )
  {
    // check, if array is still large enough
    if ( !(pRpt->ulAllInfoRecords % INIT_SIZE_A) && pRpt->ulAllInfoRecords )
    {
      //
      // realloc memory for PALLINFO array
      //
      fOk = UtlAllocHwnd ((PVOID*)&pArray,
                          (ULONG)(INIT_SIZE_A* ulAllocs *  sizeof(PALLINFO)),
                          (ULONG)(INIT_SIZE_A*(ulAllocs+1)*sizeof(PALLINFO)),
                          ERROR_STORAGE, pRpt->hwndErrMsg );
      if ( !fOk ) pRpt->fErrorPosted = TRUE; // error msg has been shown

      ulAllocs++;

      if ( fOk )
      {
        pRpt->ppAllInfoField = (PPALLINFO)pArray;    // store memory address in RPT
        ppAllInfoFieldTmp = (PPALLINFO)pArray;       // set tmp pointer
        ppAllInfoFieldTmp += pRpt->ulAllInfoRecords; // update tmp pointer
      }
    } // end if

    //
    // allocate memory for ALLINFO, if necessary
    //
    if ( fAlloc )
    {
      fOk = UtlAllocHwnd ((PVOID*)&pAllInfoTmp, 0L,
                          (ULONG)sizeof (ALLINFO), ERROR_STORAGE, pRpt->hwndErrMsg );
      if ( !fOk ) pRpt->fErrorPosted = TRUE; // error msg has been shown
    } // end if

    if ( fOk )
    {
      // set record number
      pAllInfoTmp->ulRecord = ulRecordIndex++;

      // read fixed part of histlog record
      usFileRc = UtlReadHwnd (hfFileHandle,
                              (PVOID)&pAllInfoTmp->histLogRecord,
                              sizeof (HISTLOGRECORD),
                              &ulBytesRead,
                              TRUE, pRpt->hwndErrMsg );

      // skip not necssary logs
      // DOCEXPORT_LOGTASK
      // DOCPROP_LOGTASK
      // FOLPROP_LOGTASK
      // FOLPROPSHIPMENT_LOGTASK
      // VERSION_LOGTASK


      if ( usFileRc == NO_ERROR && ulBytesRead > 0 &&
           (pAllInfoTmp->histLogRecord.Task == DOCEXPORT_LOGTASK ||
            pAllInfoTmp->histLogRecord.Task == FOLPROP_LOGTASK   ||
            pAllInfoTmp->histLogRecord.Task == VERSION_LOGTASK   ||
            pAllInfoTmp->histLogRecord.Task == FOLPROPSHIPMENT_LOGTASK   ||
            pAllInfoTmp->histLogRecord.Task == DOCPROP_LOGTASK)&&
           pRpt->usReport != HISTORY_REPORT )
      {
        // correct sizes in older records
        if ( fOk )
        {
          HistLogCorrectRecSizes( &pAllInfoTmp->histLogRecord );
        } // end if

        // read any document long name
        if ( fOk )
        {
          pAllInfoTmp->szLongName[0] = EOS;

          if ( pAllInfoTmp->histLogRecord.fLongNameRecord )
          {
            USHORT usLongNameLength = pAllInfoTmp->histLogRecord.usSize -
                                      pAllInfoTmp->histLogRecord.usAddInfoLength -
                                      sizeof(HISTLOGRECORD);

            if ( usLongNameLength != 0 )
            {
              usFileRc = UtlReadL( hfFileHandle,
                                  (PVOID)&pAllInfoTmp->szLongName,
                                  usLongNameLength,
                                  &ulBytesRead,
                                  NOMSG );
            } /* endif */
          } /* endif */
        } /* endif */

        // read variable part and skip it
        fAlloc = FALSE;
        if ( fOk && (pAllInfoTmp->histLogRecord.usAddInfoLength) )
        {
          // ensure that the additional info size is not corrupted
          if ( pAllInfoTmp->histLogRecord.usAddInfoLength > sizeof(pAllInfoTmp->variablePart) )
          {
            pAllInfoTmp->histLogRecord.usAddInfoLength = sizeof(pAllInfoTmp->variablePart);
          } /* endif */

          usFileRc = UtlReadL( hfFileHandle,
                              (PVOID)&pAllInfoTmp->variablePart,
                              pAllInfoTmp->histLogRecord.usAddInfoLength,
                              &ulBytesRead,
                              NOMSG );
        } // end if

      }
      else if ( usFileRc == NO_ERROR && ulBytesRead > 0 )
      {
        fAlloc = TRUE;

        // check, if record is valid
        if ( pAllInfoTmp->histLogRecord.lEyeCatcher != HISTLOGEYECATCHER )
        {
          fOk = FALSE;
          UtlErrorHwnd( ERROR_HISTLOG_CORRUPTED, MB_CANCEL, 0, NULL,
                        EQF_ERROR,
                        pRpt->hwndErrMsg);
          pRpt->fErrorPosted = TRUE; // error msg has been shown
        } // end if

        // correct sizes in older records
        if ( fOk )
        {
          HistLogCorrectRecSizes( &pAllInfoTmp->histLogRecord );
        } // end if


        // read or get document long name or use document short name instead
        if ( fOk )
        {
          pAllInfoTmp->szLongName[0] = EOS;

          if ( pAllInfoTmp->histLogRecord.fLongNameRecord )
          {
            USHORT usLongNameLength = pAllInfoTmp->histLogRecord.usSize -
                                      pAllInfoTmp->histLogRecord.usAddInfoLength -
                                      sizeof(HISTLOGRECORD);

            if ( usLongNameLength != 0 )
            {
              usFileRc = UtlReadL( hfFileHandle,
                                   (PVOID)&pAllInfoTmp->szLongName,
                                   usLongNameLength,
                                   &ulBytesRead,
                                   NOMSG );
            }
            else
            {
              // the short name field contains the correct document name
              strcpy( pAllInfoTmp->szLongName, pAllInfoTmp->histLogRecord.szDocName );
            } /* endif */
          }
          else
          {
            // older record without long name information, so get long name from
            // document properties
            strcpy( szProperty, pRpt->szFolderObjName );
            strcat( szProperty, BACKSLASH_STR );
            strcat( szProperty, pAllInfoTmp->histLogRecord.szDocName );
            szLongFileName[0] = EOS;
            DocQueryInfo2( szProperty, NULL,NULL,NULL,NULL,szLongFileName,NULL,NULL,FALSE);
            if ( szLongFileName[0] )
            {
              strcpy( pAllInfoTmp->szLongName,szLongFileName);
            }
            else
            {
              strcpy( pAllInfoTmp->szLongName, pAllInfoTmp->histLogRecord.szDocName );
            } // endif
          } /* endif */
        } /* endif */

        // read varaiable part of histlog record
        if ( fOk && (pAllInfoTmp->histLogRecord.usAddInfoLength) )
        {
          usFileRc = UtlReadL( hfFileHandle,
                               (PVOID)&pAllInfoTmp->variablePart,
                               pAllInfoTmp->histLogRecord.usAddInfoLength,
                               &ulBytesRead,
                               NOMSG );
        } // end if

        if ( fOk )
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
            sItem = ISEARCHITEMEXACTHWND (pRpt->hwndRptHandlerLB,
                                     pAllInfoTmp->histLogRecord.szDocName);

            if ( sItem == -1 ) // document is not in LB -> insert document
            {
              // insert document in invisible LB
              INSERTITEMENDHWND (pRpt->hwndRptHandlerLB,
                                 pAllInfoTmp->histLogRecord.szDocName);



              // update number of documents, in this case of invisible LB
              pRpt->usSelectedDocuments++;
            } // end if

            *ppAllInfoFieldTmp++ = pAllInfoTmp;  // store address in PALLINFOs
            pRpt->ulAllInfoRecords++;            // update number of ALLINFOs in RPT
            fAlloc = TRUE;                       // set allocation state
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

            sItem = ISEARCHITEMEXACTHWND( hwndRptLongLB,
                                          pAllInfoTmp->szLongName );

            if ( sItem == -1 )
            {
              // document is not in LB -> not selected or different case of name
              // try with1
              strcpy( szLongFileName, pAllInfoTmp->szLongName );
              strupr( szLongFileName );
              sItem = ISEARCHITEMEXACTHWND( hwndRptLongLB, szLongFileName );
            } /* end if */

            if ( sItem == -1 ) // document is not in LB -> not selected
            {
              strcpy( szLongFileName, pAllInfoTmp->szLongName );
              strlwr( szLongFileName );
              sItem = ISEARCHITEMEXACTHWND( hwndRptLongLB, szLongFileName );
            } /* end if */


            if ( sItem == -1 ) // document is not in LB -> not selected
            {
              fAlloc = FALSE;  // set allocation state
            }
            else
            {
              // document is selected
              *ppAllInfoFieldTmp++ = pAllInfoTmp;  // store address in PALLINFOs
              pRpt->ulAllInfoRecords++;            // update number of ALLINFOs in RPT
              fAlloc = TRUE;                       // set allocation state
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
          fOk = FALSE;         // an error taken
          pRpt->fErrorPosted = TRUE; // error msg has been shown
        }
      } // end if
    } // end if
  } // end while

  if ( hwndRptLongLB ) WinDestroyWindow(hwndRptLongLB);


  if ( hfFileHandle )
  {
    UtlClose (hfFileHandle, FALSE );  // close histlog file
  }

  return fOk;
} // end of RptGetRecords


//+----------------------------------------------------------------------------+
//|internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptSortRecords                                            |
//+----------------------------------------------------------------------------+
//|Function call:    RptSortRecords (pRpt)                                     |
//+----------------------------------------------------------------------------+
//|Description:      This function sorts the ALLINFOs in document order        |
//+----------------------------------------------------------------------------+
//|Parameters:       PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    allocate temporary array for PALLINFOs                    |
//|                  copy original to temorary array                           |
//|                  sort ALLINFOS from temporay to original array in          |
//|                   document order                                           |
//|                  free temporary array                                      |
//+----------------------------------------------------------------------------+

BOOL RptSortRecords (PRPT pRpt)
{
  BOOL      fOk = FALSE;        // error indicator



#ifdef LONG_NAME_IN_HISTLOG
  // sort using document long name, entry date
  qsort( pRpt->ppAllInfoField, pRpt->ulAllInfoRecords, sizeof(PALLINFO), RptAllInfoCompare );
  fOk = TRUE;
#else
  PVOID     pArray;             // pointer to allocated memory
  PPALLINFO ppAllInfoFieldOrg;  // pointer to original array of PALLINFOs
  PPALLINFO ppAllInfoFieldTmp;  // pointer to temporary array of PALLINFOs
  USHORT    usIndex1 = 0;       // index for invisible LB
  SHORT     sIndex2 = 0;        // index for PALLINFOs
  SHORT     sMaxDocs;           // number of documents in invisible listbox

  // allocate temporary memory for PALLINFOs
  fOk = UtlAlloc ((PVOID*)&pArray, 0L,
                  (ULONG)(pRpt->ulAllInfoRecords*sizeof(PALLINFO)),
                  ERROR_STORAGE);

  if ( fOk )
  {
    ppAllInfoFieldOrg = pRpt->ppAllInfoField;  // set pointer to original array
    ppAllInfoFieldTmp = (PPALLINFO)pArray;     // set pointer to tmp array

    // copy PALLINFOs to temporary field
    memcpy ((PVOID)ppAllInfoFieldTmp, (PVOID)ppAllInfoFieldOrg,
            (ULONG)((pRpt->ulAllInfoRecords)*sizeof(PALLINFO)));

    // sort ALLINFOs in document order
    sMaxDocs = QUERYITEMCOUNTHWND( pRpt->hwndRptHandlerLB );
    while ( sIndex2 < sMaxDocs )
    {
      QUERYITEMTEXTHWND( pRpt->hwndRptHandlerLB, sIndex2++,
                         pRpt->pszActualDocument );
      usIndex1 = 0;                           // set index of PALLINFO
      ppAllInfoFieldTmp = (PPALLINFO)pArray;  // set pointer to Tmp array

      // loop over all documents in invisible LB
      while ( (usIndex1++ < pRpt->ulAllInfoRecords) && ppAllInfoFieldTmp )
      {
        if ( *ppAllInfoFieldTmp != 0 )
        {
          if ( strcmp (pRpt->pszActualDocument,
                       (*ppAllInfoFieldTmp)->histLogRecord.szDocName) == 0 )
          {
            // document of ALLINFO is equal to document of invisible LB
            *ppAllInfoFieldOrg++ = *ppAllInfoFieldTmp;
            *ppAllInfoFieldTmp++ = 0L;
          }
          else
          {
            ppAllInfoFieldTmp++;  // next PALLINFO
          }// end if
        }
        else
        {
          ppAllInfoFieldTmp++;  // next PALLINFO
        } // end if
      } // end while
    } // end while
  } // end if

  if ( fOk )
  {
    // free temporary memory for PALLINFOs
    UtlAlloc ((PVOID*)&pArray, 0L, 0L, NOMSG);
  } // end if

#endif

  return fOk;
} // end of RptSortRecords


#ifdef LONG_NAME_IN_HISTLOG
// compare to allinfo structures (used by qsort)
int RptAllInfoCompare( const void *pElement1, const void *pElement2 )
{
  int iResult;
  PALLINFO pAllInfo1 = *((PALLINFO *)pElement1); // ptr to first element
  PALLINFO pAllInfo2 = *((PALLINFO *)pElement2); // ptr to first element

  iResult = stricmp( pAllInfo1->szLongName, pAllInfo2->szLongName );

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
#endif


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptLoadOutputMris                                         |
//+----------------------------------------------------------------------------+
//|Function call:    RptLoadOutputMris (pRpt)                                  |
//+----------------------------------------------------------------------------+
//|Description:      This function loads the output mris for counting report   |
//|                  from resource DLL                                         |
//+----------------------------------------------------------------------------+
//|Parameters:       PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    allocate memory for OUTMRI array                          |
//|                  loop over all output mris                                 |
//|                    store mri in OUTMRI array                               |
//+----------------------------------------------------------------------------+

BOOL RptLoadOutputMris (PRPT pRpt)
{
  BOOL    fOk = FALSE;  // error indicator
  PVOID   pArray = NULL;// pointer to allocated memory
  POUTMRI pOutputMris = NULL;  // pointer to OUTMRI array
  USHORT  usIndex = 0;  // index for OUTMRI array


  // alloc memory for output mris
  fOk = UtlAlloc ((PVOID*)&pArray, 0L, sizeof (OUTMRI), ERROR_STORAGE);

  if ( fOk )
  {
    pRpt->pOutputMris = (POUTMRI)pArray;  // store memory address
    pOutputMris = (POUTMRI)pArray;        // set pointer to mri array
  } // end if

  if ( fOk )
  {
    // loop over all output mris
    while ( usIndex < MAX_MRI_LINES )
    {
      // load mri in OUTMRI array
      LOADSTRING (NULLHANDLE, hResMod, SID_RPT_OUT_FORMAT + usIndex,
                  pOutputMris->szFeld[usIndex++]);
    } // end while
  } // end if

  return fOk;
} // end of RptLoadOutputMris



//+----------------------------------------------------------------------------+
//|Function name: RPTCheckSourceWords                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Calculate Number of Words in Document                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

ULONG RPTCheckSourceWords(PSZ pSegSource)
{


  PTBDOCUMENT  pDoc;                 // ptr to document control structure
  SHORT        sRc = 0;              // success
  ULONG        ulSegNum = 1;
  PTBSEGMENT   pSrcSeg;
  ULONG        uWords = 0;
  SHORT        sSrcLanguage = 0;

  CHAR         szFileName[MAX_LONGPATH];
  CHAR         szFolderName[MAX_LONGPATH];
  CHAR         szFormat[MAX_FORMAT];
  CHAR         szLang[MAX_LANG_LENGTH];

  PSZ          pszFileName;
  ULONG        ulOemCP = 0L;
  ULONG ulSrcWords  = 0L;
  ULONG ulSrcMarkUp = 0L;

  // Get File Name + Path

  strcpy(szFolderName, pSegSource);
  pszFileName = UtlSplitFnameFromPath(szFolderName);
  strcpy(szFileName,pszFileName);
  pszFileName = UtlSplitFnameFromPath(szFolderName);
  strcat(szFolderName,BACKSLASH_STR);
  strcat(szFolderName,szFileName);
  /*******************************************************************/
  /* allocate TBDOC structure                                        */
  /*******************************************************************/
  if ( ! UtlAlloc( (PVOID *) &pDoc, 0L, (LONG) sizeof( TBDOCUMENT ), TRUE ) )
  {
    sRc = ERR_NOMEMORY;
  } /* endif */

  /* load tag table                                                  */
  sRc = DocQueryInfo2(szFolderName, NULL, szFormat, szLang, NULL,
                        NULL, NULL,NULL,FALSE);
  sRc = (SHORT) MorphGetLanguageID( szLang, &sSrcLanguage );
  ulOemCP = GetLangOEMCP( szLang);

  if ( !sRc )
  {
    sRc = (SHORT)TALoadTagTable( DEFAULT_QFTAG_TABLE,
                                 (PLOADEDTABLE *) &pDoc->pQFTagTable,
                                 TRUE, FALSE );
  } /* endif */
  if ( !sRc )
  {
    sRc = (SHORT)TALoadTagTable( szFormat,
                                 (PLOADEDTABLE *) &pDoc->pDocTagTable,
                                 FALSE, FALSE );
  } /* endif */

  // check if file exists
  // --------------------

  if ( !sRc && !UtlFileExist(pSegSource) )
  {
    sRc = ERR_READFILE;

  } // end if
  if ( !sRc )
  {
    pDoc->ulOemCodePage = ulOemCP;
    sRc = EQFBFileRead( pSegSource, pDoc );
  } /* endif */

  pSrcSeg = EQFBGetSeg(pDoc, ulSegNum);

  while ( !sRc && pSrcSeg && (ulSegNum < pDoc->ulMaxSeg) )
  {
    if ( (pSrcSeg->qStatus == QF_XLATED)
         || (pSrcSeg->qStatus == QF_TOBE )
         || (pSrcSeg->qStatus == QF_ATTR )
         || (pSrcSeg->qStatus == QF_CURRENT)
         || (pSrcSeg->qStatus == QF_NOP)
       )
    {
      if ( sRc == 0 && pDoc )
      {
        pSrcSeg = EQFBGetSeg(pDoc, ulSegNum);

        if ( pSrcSeg && pSrcSeg->pData )
        {
          ulSrcWords = ulSrcMarkUp = 0L;
          sRc = EQFBWordCntPerSeg((PLOADEDTABLE) pDoc->pDocTagTable,
                                  (PTOKENENTRY) pDoc->pTokBuf,
                                  pSrcSeg->pDataW,
                                  sSrcLanguage,
                                  &ulSrcWords, &ulSrcMarkUp, ulOemCP);

          uWords += (ULONG) ulSrcWords;
        } /* endif */
      } /* endif */
    } /* endif */

    ulSegNum++;
    pSrcSeg = EQFBGetSeg(pDoc, ulSegNum);
  } /* endwhile */

  // Free resources
  // --------------
  if ( pDoc )
  {
    if ( pDoc->pQFTagTable ) TAFreeTagTable( (PLOADEDTABLE) pDoc->pQFTagTable );

    if ( pDoc->pDocTagTable) TAFreeTagTable( (PLOADEDTABLE ) pDoc->pDocTagTable );

    EQFBFreeDoc( &pDoc, EQFBFREEDOC_NOTAGTABLEFREE | EQFBFREEDOC_NOPROTECTFREE );
  } /* endif */

  if ( !sRc )
  {
    return uWords;
  }
  else
  {
    return 0xFFFFFFFF;
  } // end if

} /* end of function RPTCheckSourceWords */




//+----------------------------------------------------------------------------+
//|Function name: RPTConsistencyCheck                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


//
// checks one cell against the total number of (source) words
//


BOOL RPTConsistencyCheck( CRITERIASUM Data,ULONG ulWords)
{
  BOOL    fOk=TRUE;
  ULONG   ulSum, ulTargetSum;

  ulSum = (ULONG)
          Data.SimpleSum.ulSrcWords +
          Data.MediumSum.ulSrcWords +
          Data.ComplexSum.ulSrcWords ;

  ulTargetSum = (ULONG)
                Data.SimpleSum.ulTgtWords +
                Data.MediumSum.ulTgtWords +
                Data.ComplexSum.ulTgtWords ;





  // different shipments possible
  // ulWords only last shipment

  fOk = (  (ulSum < max(100, (250*ulWords)/100))   &&
           (ulTargetSum < max(100, (2500*ulWords)/100))   );


  return fOk;

} /* end of function RPTConsistencyCheck */



//+----------------------------------------------------------------------------+
//|Function name: RPTConsistencySumCheck                                       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


//
// checks one row against the total number of (source) words
//



BOOL RPTConsistencySumCheck( PDOCSAVEHIST2 pData,ULONG ulWords)
{
  BOOL    fOk=TRUE;
  ULONG   ulSum, ulTargetSum;
  ULONG   ulExactExist, ulExactUsed;
  ULONG   ulReplExist, ulReplUsed;
  ULONG   ulFuzzyExist, ulFuzzyUsed;
  ULONG   ulMachExist, ulMachUsed;
  ULONG   ulFuzzyExist_1, ulFuzzyUsed_1;
  ULONG   ulFuzzyExist_2, ulFuzzyUsed_2;
  ULONG   ulFuzzyExist_3, ulFuzzyUsed_3;



  // Source Words TEST
  // -----------------

  ulExactExist = (ULONG)
                 pData->ExactExist.SimpleSum.ulSrcWords  +
                 pData->ExactExist.MediumSum.ulSrcWords  +
                 pData->ExactExist.ComplexSum.ulSrcWords;
  ulExactUsed = (ULONG)
                pData->ExactUsed.SimpleSum.ulSrcWords  +
                pData->ExactUsed.MediumSum.ulSrcWords  +
                pData->ExactUsed.ComplexSum.ulSrcWords;


  ulReplExist = (ULONG)
                pData->ReplExist.SimpleSum.ulSrcWords  +
                pData->ReplExist.MediumSum.ulSrcWords  +
                pData->ReplExist.ComplexSum.ulSrcWords;
  ulReplUsed = (ULONG)
               pData->ReplUsed.SimpleSum.ulSrcWords  +
               pData->ReplUsed.MediumSum.ulSrcWords  +
               pData->ReplUsed.ComplexSum.ulSrcWords;


  ulFuzzyExist = (ULONG)
                 pData->FuzzyExist.SimpleSum.ulSrcWords  +
                 pData->FuzzyExist.MediumSum.ulSrcWords  +
                 pData->FuzzyExist.ComplexSum.ulSrcWords;
  ulFuzzyUsed = (ULONG)
                pData->FuzzyUsed.SimpleSum.ulSrcWords  +
                pData->FuzzyUsed.MediumSum.ulSrcWords  +
                pData->FuzzyUsed.ComplexSum.ulSrcWords;


  ulFuzzyExist_1 = (ULONG)
                   pData->FuzzyExist_1.SimpleSum.ulSrcWords  +
                   pData->FuzzyExist_1.MediumSum.ulSrcWords  +
                   pData->FuzzyExist_1.ComplexSum.ulSrcWords;
  ulFuzzyUsed_1 = (ULONG)
                  pData->FuzzyUsed_1.SimpleSum.ulSrcWords  +
                  pData->FuzzyUsed_1.MediumSum.ulSrcWords  +
                  pData->FuzzyUsed_1.ComplexSum.ulSrcWords;

  ulFuzzyExist_2 = (ULONG)
                   pData->FuzzyExist_2.SimpleSum.ulSrcWords  +
                   pData->FuzzyExist_2.MediumSum.ulSrcWords  +
                   pData->FuzzyExist_2.ComplexSum.ulSrcWords;
  ulFuzzyUsed_2 = (ULONG)
                  pData->FuzzyUsed_2.SimpleSum.ulSrcWords  +
                  pData->FuzzyUsed_2.MediumSum.ulSrcWords  +
                  pData->FuzzyUsed_2.ComplexSum.ulSrcWords;

  ulFuzzyExist_3 = (ULONG)
                   pData->FuzzyExist_3.SimpleSum.ulSrcWords  +
                   pData->FuzzyExist_3.MediumSum.ulSrcWords  +
                   pData->FuzzyExist_3.ComplexSum.ulSrcWords;
  ulFuzzyUsed_3 = (ULONG)
                  pData->FuzzyUsed_3.SimpleSum.ulSrcWords  +
                  pData->FuzzyUsed_3.MediumSum.ulSrcWords  +
                  pData->FuzzyUsed_3.ComplexSum.ulSrcWords;


  ulMachExist = (ULONG)
                pData->MachExist.SimpleSum.ulSrcWords  +
                pData->MachExist.MediumSum.ulSrcWords  +
                pData->MachExist.ComplexSum.ulSrcWords;
  ulMachUsed = (ULONG)
               pData->MachUsed.SimpleSum.ulSrcWords  +
               pData->MachUsed.MediumSum.ulSrcWords  +
               pData->MachUsed.ComplexSum.ulSrcWords;


  // Total Source Words
  // ------------------

  ulSum = (ULONG)
          pData->AnalAutoSubst.SimpleSum.ulSrcWords  +
          pData->AnalAutoSubst.MediumSum.ulSrcWords  +
          pData->AnalAutoSubst.ComplexSum.ulSrcWords +

          pData->EditAutoSubst.SimpleSum.ulSrcWords  +
          pData->EditAutoSubst.MediumSum.ulSrcWords  +
          pData->EditAutoSubst.ComplexSum.ulSrcWords +

          pData->ExactUsed.SimpleSum.ulSrcWords  +
          pData->ExactUsed.MediumSum.ulSrcWords  +
          pData->ExactUsed.ComplexSum.ulSrcWords +

          pData->ReplUsed.SimpleSum.ulSrcWords  +
          pData->ReplUsed.MediumSum.ulSrcWords  +
          pData->ReplUsed.ComplexSum.ulSrcWords +

          pData->FuzzyUsed.SimpleSum.ulSrcWords  +
          pData->FuzzyUsed.MediumSum.ulSrcWords  +
          pData->FuzzyUsed.ComplexSum.ulSrcWords +

          pData->MachUsed.SimpleSum.ulSrcWords  +
          pData->MachUsed.MediumSum.ulSrcWords  +
          pData->MachUsed.ComplexSum.ulSrcWords +

          pData->NoneExist2.SimpleSum.ulSrcWords  +
          pData->NoneExist2.MediumSum.ulSrcWords  +
          pData->NoneExist2.ComplexSum.ulSrcWords +

          pData->NotXlated.SimpleSum.ulSrcWords  +
          pData->NotXlated.MediumSum.ulSrcWords  +
          pData->NotXlated.ComplexSum.ulSrcWords   ;


  // Total Target Words
  // ------------------

  ulTargetSum = (ULONG)
                pData->AnalAutoSubst.SimpleSum.ulTgtWords  +
                pData->AnalAutoSubst.MediumSum.ulTgtWords  +
                pData->AnalAutoSubst.ComplexSum.ulTgtWords +

                pData->EditAutoSubst.SimpleSum.ulTgtWords  +
                pData->EditAutoSubst.MediumSum.ulTgtWords  +
                pData->EditAutoSubst.ComplexSum.ulTgtWords +

                pData->ExactUsed.SimpleSum.ulTgtWords  +
                pData->ExactUsed.MediumSum.ulTgtWords  +
                pData->ExactUsed.ComplexSum.ulTgtWords +

                pData->ReplUsed.SimpleSum.ulTgtWords  +
                pData->ReplUsed.MediumSum.ulTgtWords  +
                pData->ReplUsed.ComplexSum.ulTgtWords +

                pData->FuzzyUsed.SimpleSum.ulTgtWords  +
                pData->FuzzyUsed.MediumSum.ulTgtWords  +
                pData->FuzzyUsed.ComplexSum.ulTgtWords +

                pData->MachUsed.SimpleSum.ulTgtWords  +
                pData->MachUsed.MediumSum.ulTgtWords  +
                pData->MachUsed.ComplexSum.ulTgtWords +

                pData->NoneExist2.SimpleSum.ulTgtWords  +
                pData->NoneExist2.MediumSum.ulTgtWords  +
                pData->NoneExist2.ComplexSum.ulTgtWords +

                pData->NotXlated.SimpleSum.ulTgtWords  +
                pData->NotXlated.MediumSum.ulTgtWords  +
                pData->NotXlated.ComplexSum.ulTgtWords   ;


  // different shipments possible
  // ulWords only last shipment
  // so some security needed

  //  Used<=Exist can not be used as we have more then one proposal

  fOk = (
        (ulSum < max(100, (250*ulWords)/100))   &&
        (ulTargetSum < max(100, (2500*ulWords)/100)) &&
        //  (ulExactUsed <= ulExactExist + 1)  &&
        //  (ulReplUsed  <= ulReplExist + 1)  &&
        //  (ulMachUsed  <= ulMachExist + 1)  &&
        (ulFuzzyExist_1 + ulFuzzyExist_2 + ulFuzzyExist_3 ==  ulFuzzyExist  )   &&
        (ulFuzzyUsed_1 + ulFuzzyUsed_2 + ulFuzzyUsed_3 ==  ulFuzzyUsed  )


        );


  return fOk;

} /* end of function RPTConsistencyCheck */




//+----------------------------------------------------------------------------+
//|Function name: RPTCOPYALLCOUNTS                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: COPYALLCOUNTS WITH SECURITY                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:  fHistInconsistency = TRUE  (inconsistency found)              |
//|                                   FALSE (row ok)                           |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


BOOL  RPTCOPYALLCOUNTS
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


  memset(&count_minus,0xFF,sizeof(count_minus));


  if ( pRpt->usColumns4[0] )
  {
    if ( RPTConsistencyCheck(Data,ulWords) )
    {
      PCOPYALLCOUNTS( pLog,
                      Data );
    }
    else
    {
      if ( !fHistError )
      {

        *pTask  = HISTDATA_INCONSISTENT_LOGTASK;
        fHistInconsistency = TRUE;
        PCOPYALLCOUNTS( pLog,
                        count_minus );
      }
    } /* endif */
  }
  else // no consistency check
  {
    PCOPYALLCOUNTS( pLog, Data );

  } // end if


  return(fHistInconsistency);

} // end of RPTCOPYALLCOUNTS



//+----------------------------------------------------------------------------+
//|Function name: RPTCHECKALLCOUNTS                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: CHECK ONE ROW FOR CONSISTENCY                                  |                                                                          |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:  fHistInconsistency = TRUE  (inconsistency found)              |
//|                                   FALSE (row ok)                           |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


BOOL  RPTCHECKALLCOUNTS
(
PRPT            pRpt,       // pointer to Report structure
PDOCSAVEHISTEX  pLog,     // Log to be filled    TARGET
PDOCSAVEHIST2   pData,    // Input Data from Histlog  SOURCE, to be checked
BOOL            fHistError, // Error encountered
INT*            pTask,      // Task
ULONG           ulWords     // Number of Words in Source file
)
{

  CRITERIASUM    count_minus;
  PCRITERIASUMEX pLogRec;
  BOOL fHistInconsistency = FALSE;


  memset(&count_minus,0xFF,sizeof(count_minus));


  if ( pRpt->usColumns4[0] )  // consistency check to be done
  {
    if ( !RPTConsistencySumCheck(pData,ulWords) )
    {
      if ( !fHistError )
      {

        *pTask  = HISTDATA_INCONSISTENT_LOGTASK;
        fHistInconsistency = TRUE;
        pLogRec = &(pLog->AnalAutoSubst) ;
        PCOPYALLCOUNTS(  pLogRec,
                         (count_minus) );
      }
    } /* endif */
  } // end if


  return(fHistInconsistency);

} // end of RPTCOPYALLCOUNTS




//+----------------------------------------------------------------------------+
//|Function name: RPTLT                                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  Security, less then                                           |
//|              FORCE NEW SHIPMENT IN CASE OF RESET DATA                      |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:  fHistInconsistency = TRUE  (inconsistency found)              |
//|                                   FALSE (row ok)                           |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


BOOL RPTLT_CRITERIASUM
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


} // end of function RPTLT_CRITERIASUM


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptMakeCalculationRecords                                 |
//+----------------------------------------------------------------------------+
//|Function call:    RptMakeCalculationRecords (pRpt)                          |
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

BOOL RptMakeCalculationRecords (PRPT pRpt)
{
  BOOL           fOk = FALSE;         // error indicator
  PVOID          pArray = NULL;       // pointer to allocated memory
  PPCALCINFO     ppCalcInfoFieldTmp = NULL;  // pointer to PCALCINFO array
  PPALLINFO      ppAllInfoFieldTmp = NULL;   // pointer to PALLINFO array
  PALLINFO       pAllInfoFieldTmp = NULL;    // pointer tp PALLINFOs
  ULONG          ulAllocs = 1;        // number of allocs of CALCIFOs
  PCALCINFO      pCalcInfoTmp = NULL; // pointer to CALCINFO structure
  PCALCINFO      pCalcInfoSum = NULL; // pointer to CALCINFO structure
  PDOCSAVEHISTEX pDocInfoAnalBase = NULL;   // CALCINFO structure with base info
  BOOL           fDocImport = FALSE;  // indicator of document import
  ULONG          usIndex = 0;         // counter
  time_t         lTime;               // current time/date
  PVERSIONHIST   pVersionHistTmp = NULL;     // version of eqfdll, eqfd
  PDOCSAVEHIST   pDocSaveHistTmp = NULL;     // pointer to DOCSAVEHIST of ALLINFOs (INPUT)
  PDOCSAVEHIST2  pDocSaveHistTmp2 = NULL;    // pointer to DOCSAVEHIST2 (enlarged structure) of ALLINFOs
  PDOCSAVEHIST3  pDocSaveHistTmp3 = NULL;    // pointer to DOCSAVEHIST3
  PDOCSAVEHISTEX pDocSaveHistSumTmp = NULL;  // pointer to DOCSAVEHISTEX of CALCINFOs
  PDOCSAVEHISTEX pDocSaveCopy = NULL;        // for security check
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
  // ------------------------
  fOk = UtlAlloc ((PVOID*)&pDocSaveCopy, 0L,
                  sizeof(DOCSAVEHISTEX), ERROR_STORAGE);

  if ( fOk )  // RESET
  {
    memset( pDocSaveCopy, 0, sizeof( DOCSAVEHISTEX )  );

  } // end if



  // alloc memory for PCALCINFO array, initial size INIT_SIZE_C
  if ( fOk )
  {
    fOk = UtlAlloc ((PVOID*)&pArray, 0L,
                    (ULONG)(INIT_SIZE_C*sizeof(CALCINFO)), ERROR_STORAGE);
  } // end if

  if ( fOk )
  {
    fOk = UtlAlloc ((PVOID*)&pDocInfoAnalBase, 0L, sizeof(DOCSAVEHISTEX),
                    ERROR_STORAGE);
    memset( pDocInfoAnalBase, 0, sizeof( DOCSAVEHISTEX )  );

  } /* endif */
  if ( fOk )
  {
    pRpt->ppCalcInfoField = (PPCALCINFO)pArray;  // store memory address in RPT
    ppCalcInfoFieldTmp = (PPCALCINFO)pArray;     // set tmp pointer to PCALCINFOs
    ppAllInfoFieldTmp = pRpt->ppAllInfoField;    // set tmp pointer to PALLINFOs
    pAllInfoFieldTmp = *ppAllInfoFieldTmp;                 // set tmp pointer to ALLINFOs



  } // end if


  if (fOk)
  {
    // fill hwndShipmentLB

    if (!pRpt->hwndShipmentLB)
    {

      HWND hwnd =  pRpt->hwndRptHandlerLB ;
      pRpt->hwndShipmentLB = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                              WS_CHILD | LBS_STANDARD,
                                              0, 0, 0, 0,
                                              hwnd, HWND_TOP, 1, NULL, NULL );
    }// end if
  } // end if

  if ( fOk )
  {
    // -----------------------
    // loop over all PALLINFOs
    // -----------------------

    while ( pRpt && fOk && (usIndex++ < pRpt->ulAllInfoRecords) )
    {
      // check, if array is still large enough
      if ( !(pRpt->ulCalcInfoRecords % INIT_SIZE_C) && pRpt->ulCalcInfoRecords )
      {
        // realloc memory for PCALCINFO array
        fOk = UtlAlloc ((PVOID*)&pArray,
                        (ULONG)(INIT_SIZE_C* ulAllocs *  sizeof(PCALCINFO)),
                        (ULONG)(INIT_SIZE_C*(ulAllocs+1)*sizeof(PCALCINFO)),
                        ERROR_STORAGE);

        ulAllocs++;
        if ( fOk )
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

          if ( RPTLT_CRITERIASUM(pDocSaveHistTmp->ExactExist, pDocSaveCopy->ExactExist) )
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
             RPTLT_CRITERIASUM(pDocSaveHistTmp->ExactExist, pDocSaveCopy->ExactExist) ||
             RPTLT_CRITERIASUM(pDocSaveHistTmp->ReplExist, pDocSaveCopy->ReplExist) ||
             RPTLT_CRITERIASUM(pDocSaveHistTmp->FuzzyExist, pDocSaveCopy->FuzzyExist) ||
             RPTLT_CRITERIASUM(pDocSaveHistTmp->MachExist, pDocSaveCopy->MachExist) ||
             RPTLT_CRITERIASUM(pDocSaveHistTmp->NoneExist, pDocSaveCopy->NoneExist)
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
#ifdef LONG_NAME_IN_HISTLOG
      fFound= (stricmp( pAllInfoFieldTmp->szLongName,
                       pRpt->pszActualDocument) == 0);
#else
      fFound= (stricmp (pAllInfoFieldTmp->histLogRecord.szDocName,
                       pRpt->pszActualDocument) == 0);
#endif

      if ( !fFound ) RptLog2String( "New document", pAllInfoFieldTmp->szLongName );
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

      if ( fOk && ( !fFound ||
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

        if ( fOk && fSecondAnalysis )
        {
          // Reset pDocInfoAnalBase
          // Merge pDocInfoAnalBase into pDocSaveHistSumTmp
          // Data lost due to second analysis
          RptLogString( "MergingResults - LostShipment" );
          MergeResults( pDocSaveHistSumTmp, pDocInfoAnalBase, TRUE );
        } /* endif */


        //
        // reset Data
        //

        nDocImport = 0;
        fAnalyzed = FALSE;
        memset( pDocInfoAnalBase, 0, sizeof( DOCSAVEHISTEX )  );
        fSecondAnalysis = FALSE;




        if ( fOk )  // RESET Consistency saved data
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

          if ( pRpt->usColumns4[0] )
          {
            ulWords = RPTCheckSourceWords( SegSource );
          }
          else
          {
            ulWords = 0xFFFFFFFF;
          } // end if

        } /* endif */


        // documents are not equal
        // alloc memory for CALCINFO structure, new shipment
        fOk = UtlAlloc ((PVOID*)&pCalcInfoTmp, 0L, sizeof(CALCINFO),
                        ERROR_STORAGE);



        // resetset histlog_error for new document
        if ( !fFound ) fHistError = FALSE;
        if ( !fFound ) fHistInconsistency = FALSE;

        if ( fOk )
        {
          // set document name in RPT and CALCINFO
#ifdef LONG_NAME_IN_HISTLOG
          strcpy (pRpt->pszActualDocument,
                  pAllInfoFieldTmp->szLongName );
          strcpy (pCalcInfoTmp->szDocument,
                  pAllInfoFieldTmp->histLogRecord.szDocName);
          strcpy (pCalcInfoTmp->szLongName,
                  pAllInfoFieldTmp->szLongName );

          // ------ produces errors in calculating report
          // strcpy (pCalcInfoTmp->szLongName,
          // pAllInfoFieldTmp->szLongName );

#else
          strcpy (pRpt->pszActualDocument,
                  pAllInfoFieldTmp->histLogRecord.szDocName);
          strcpy (pCalcInfoTmp->szDocument,
                  pAllInfoFieldTmp->histLogRecord.szDocName);
#endif
          //
          // SHIPMENT_HANDLER: Set shipment number according folder shipment number if given
          //

          if (pAllInfoFieldTmp->histLogRecord.Task == DOCIMPORT_LOGTASK2)
          {

            pDocImport2 = &(pAllInfoFieldTmp->variablePart.DocImport2);

            strcpy (pRpt->szShipment, pDocImport2->szShipment);
            strcpy (pCalcInfoTmp->szShipment, pDocImport2->szShipment);

            if ( SEARCHITEMHWND( pRpt->hwndShipmentLB, pDocImport2->szShipment )== LIT_NONE )
            {
              char szBuffer[100];
              SHORT sInsertItem;
              strcpy(szBuffer, pDocImport2->szShipment);
              OEMTOANSI(szBuffer);
              sInsertItem = INSERTITEMHWND( pRpt->hwndShipmentLB, szBuffer );
            }//end if


          }
          else
          {
            strcpy (pRpt->szShipment,
                    "NA");
            strcpy (pCalcInfoTmp->szShipment,
                    "NA");


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


      if ( fOk )
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
            RptLogString( "MergingResults - SaveAfterAnalysis" );
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

              RptLogDocSave( "AnalysisAutoSubst(AfterImport)", pDocSaveHistTmp );

              // Analyze Automatic Substitution
              fHistInconsistency |=
              RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst),
                               pDocSaveHistTmp->AnalAutoSubst,
                               fHistError,
                               &(pCalcInfoTmp->Task),
                               ulWords
                              );

              // NotXlated, only last task: segments, source
              fHistInconsistency |=
              RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->NotXlated),
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

              RptLogDocSave( "AnalysisAutoSubst(2nd)", pDocSaveHistTmp );

              // Analyze Automatic Substitution
              fHistInconsistency |=
              RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst2),
                               pDocSaveHistTmp->AnalAutoSubst,
                               fHistError,
                               &(pCalcInfoTmp->Task),
                               ulWords
                              );

              // NotXlated, only last task: segments, source
              fHistInconsistency |=
              RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->NotXlated),
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

            RptLogDocSave3( "AnalysisAutoSubst(AfterImport)", pDocSaveHistTmp3 );

            // Analyze Automatic Substitution
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst),
                             pDocSaveHistTmp3->AnalAutoSubst,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // NotXlated, only last task: segments, source
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->NotXlated),
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

            RptLogDocSave3( "AnalysisAutoSubst(2nd)", pDocSaveHistTmp3 );

            // Analyze Automatic Substitution
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst2),
                             pDocSaveHistTmp3->AnalAutoSubst,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // NotXlated, only last task: segments, source
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->NotXlated),
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

            RptLogDocSave( "DocSave", pDocSaveHistTmp );

             if ( pAllInfoFieldTmp->histLogRecord.Task == DOCIMPNEWTARGET_LOGTASK )
             {

              // RESET AnalAutoSubst buffer
              fAnaAutoSubstAvail = FALSE;
              memset( &AnaAutoSubstSave, 0, sizeof(AnaAutoSubstSave)  );

              // Analyze Automatic Substitution
              fHistInconsistency |=
              RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->AnalAutoSubst),
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
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->EditAutoSubst),
                             pDocSaveHistTmp->EditAutoSubst,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // ExactExist, sum overall: segments, source, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->ExactExist),
                             pDocSaveHistTmp->ExactExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // ReplExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->ReplExist),
                             pDocSaveHistTmp->ReplExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // FuzzyExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyExist),
                             pDocSaveHistTmp->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // MachExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->MachExist),
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
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->ExactUsed),
                             pDocSaveHistTmp->ExactExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // ReplUsed, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->ReplUsed),
                             pDocSaveHistTmp->ReplExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyUsed, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed),
                             pDocSaveHistTmp->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // MachUsed, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->MachUsed),
                             pDocSaveHistTmp->MachExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist_1, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyExist_1),
                             pDocSaveHistTmp->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyUsed_1, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed_1),
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
            RPTCOPYALLCOUNTS(pRpt,  &(pDocSaveHistSumTmp->NoneExist2),
                             pDocSaveHistTmp->NoneExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

#endif

            // NoneExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt,  &(pDocSaveHistSumTmp->NoneExist),
                             pDocSaveHistTmp->NoneExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // NotXlated, only last task: segments, source
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->NotXlated),
                             pDocSaveHistTmp->NotXlated,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            RptLogCalcInfo( "CalcInfo after SaveRecord", pCalcInfoTmp );

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

            RptLogDocSave2( "DocSave", pDocSaveHistTmp2 );

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
            RPTCHECKALLCOUNTS(pRpt, pDocSaveHistSumTmp,
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

                RptLogString( "DocSaveRecord with missing AnalAutoSubst values, correcting NonExist matches" );

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
                  RptLogString( "NonExist matchs to small, nothing corrected" );
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
                  RptLogString( "NonExist2 matchs to small, nothing corrected" );
                } /* endif */
              } /* endif */
            } /* edif */



            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->EditAutoSubst),
                             pDocSaveHistTmp2->EditAutoSubst,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // ExactExist/Used, sum overall: segments, source, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->ExactExist),
                             pDocSaveHistTmp2->ExactExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->ExactUsed),
                             pDocSaveHistTmp2->ExactUsed,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // ReplExist/Used, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->ReplExist),
                             pDocSaveHistTmp2->ReplExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->ReplUsed),
                             pDocSaveHistTmp2->ReplUsed,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // FuzzyExist/Used, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyExist),
                             pDocSaveHistTmp2->FuzzyExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed),
                             pDocSaveHistTmp2->FuzzyUsed,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist/Used 1 , sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyExist_1),
                             pDocSaveHistTmp2->FuzzyExist_1,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed_1),
                             pDocSaveHistTmp2->FuzzyUsed_1,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist/Used 2, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyExist_2),
                             pDocSaveHistTmp2->FuzzyExist_2,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed_2),
                             pDocSaveHistTmp2->FuzzyUsed_2,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // FuzzyExist/Used 3, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyExist_3),
                             pDocSaveHistTmp2->FuzzyExist_3,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->FuzzyUsed_3),
                             pDocSaveHistTmp2->FuzzyUsed_3,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // MachExist/Used, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->MachExist),
                             pDocSaveHistTmp2->MachExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->MachUsed),
                             pDocSaveHistTmp2->MachUsed,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );


            // NoneExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt,  &(pDocSaveHistSumTmp->NoneExist),
                             pDocSaveHistTmp2->NoneExist,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );

            // NoneExist, sum overall: segments, source, modified, target
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt,  &(pDocSaveHistSumTmp->NoneExist2),
                             pDocSaveHistTmp2->NoneExist2,
                             fHistError,
                             &(pCalcInfoTmp->Task),
                             ulWords
                            );



            // NotXlated, only last task: segments, source
            fHistInconsistency |=
            RPTCOPYALLCOUNTS(pRpt, &( pDocSaveHistSumTmp->NotXlated),
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

            RptLogCalcInfo( "CalcInfo after SaveRecord", pCalcInfoTmp );
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
        fOk=FALSE;
        return fOk;
      } // end if

    }  // end while

    // in case of a second analysis add base values....
    if ( fOk && fSecondAnalysis )
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

  if ( fOk && (pRpt->usOptions == WITH_TOTALS) && (pRpt->usReport == CALCULATION_REPORT) )
  {
    *(pRpt->pszActualDocument) = EOS;  // set actual document in RPT

    // add one PCALCINFO for summary CALCINFO in PCALCINFO array
    // if necessary, realloc PCALCINFO array
    if ( !(pRpt->ulCalcInfoRecords % INIT_SIZE_C) && pRpt->ulCalcInfoRecords )
    {
      // realloc memory for PCALCINFO array
      fOk = UtlAlloc ((PVOID*)&pArray,
                      (ULONG)( INIT_SIZE_C*ulAllocs *  sizeof(PCALCINFO)),
                      (ULONG)((INIT_SIZE_C*(ulAllocs+1))*sizeof(PCALCINFO)),
                      ERROR_STORAGE);

      if ( fOk )
      {
        pRpt->ppCalcInfoField = (PPCALCINFO)pArray;  // store memory address in RPT
      }
    } // end if

    usIndex = 0;  // set counter

    // alloc memory for CALCINFO summary
    fOk = UtlAlloc ((PVOID*)&pArray, 0L, sizeof(CALCINFO), ERROR_STORAGE);

    if ( fOk )
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
      strcpy (pCalcInfoSum->szDocument, pRpt->pOutputMris->szFeld[RPT_SUMMARY]);
#ifdef LONG_NAME_IN_HISTLOG
      strcpy (pCalcInfoSum->szLongName, pRpt->pOutputMris->szFeld[RPT_SUMMARY]);
#endif
      pCalcInfoSum->lTime = lTime;
    } // end if

    if ( fOk )
    {
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

  return fOk;
} // end of RptMakeCalculationRecords



//+----------------------------------------------------------------------------+
//|Function name: MergeResults                                                 |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

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


  MERGEALLCOUNTS( pDocSaveHistSumTmp->EditAutoSubst,
                  pDocInfoAnalBase->EditAutoSubst );


  // ExactExist/Used, sum overall: segments, source, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->ExactExist,
                  pDocInfoAnalBase->ExactExist );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->ExactUsed,
                  pDocInfoAnalBase->ExactUsed );


  // ReplExist/Used, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->ReplExist,
                  pDocInfoAnalBase->ReplExist );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->ReplUsed,
                  pDocInfoAnalBase->ReplUsed );


  // FuzzyExist/Used, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist,
                  pDocInfoAnalBase->FuzzyExist );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed,
                  pDocInfoAnalBase->FuzzyUsed );

  // FuzzyExist/Used 1 , sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_1,
                  pDocInfoAnalBase->FuzzyExist_1 );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_1,
                  pDocInfoAnalBase->FuzzyUsed_1 );

  // FuzzyExist/Used 2, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_2,
                  pDocInfoAnalBase->FuzzyExist_2 );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_2,
                  pDocInfoAnalBase->FuzzyUsed_2 );

  // FuzzyExist/Used 3, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_3,
                  pDocInfoAnalBase->FuzzyExist_3 );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_3,
                  pDocInfoAnalBase->FuzzyUsed_3 );

  // MachExist/Used, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->MachExist,
                  pDocInfoAnalBase->MachExist );
  MERGEALLCOUNTS( pDocSaveHistSumTmp->MachUsed,
                  pDocInfoAnalBase->MachUsed );

  // NoneExist, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->NoneExist,
                  pDocInfoAnalBase->NoneExist );

  // NoneExist, sum overall: segments, source, modified, target
  MERGEALLCOUNTS( pDocSaveHistSumTmp->NoneExist2,
                  pDocInfoAnalBase->NoneExist2 );

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
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

static VOID CopyResults
(
PDOCSAVEHISTEX pDocSaveHistSumTmp,   // Target (into)
PDOCSAVEHISTEX pDocInfoAnalBase    // Source (from)
)
{


  COPY2ALLCOUNTS( pDocSaveHistSumTmp->EditAutoSubst,
                  pDocInfoAnalBase->EditAutoSubst );


  // ExactExist/Used, sum overall: segments, source, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->ExactExist,
                  pDocInfoAnalBase->ExactExist );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->ExactUsed,
                  pDocInfoAnalBase->ExactUsed );


  // ReplExist/Used, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->ReplExist,
                  pDocInfoAnalBase->ReplExist );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->ReplUsed,
                  pDocInfoAnalBase->ReplUsed );


  // FuzzyExist/Used, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist,
                  pDocInfoAnalBase->FuzzyExist );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed,
                  pDocInfoAnalBase->FuzzyUsed );

  // FuzzyExist/Used 1 , sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_1,
                  pDocInfoAnalBase->FuzzyExist_1 );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_1,
                  pDocInfoAnalBase->FuzzyUsed_1 );

  // FuzzyExist/Used 2, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_2,
                  pDocInfoAnalBase->FuzzyExist_2 );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_2,
                  pDocInfoAnalBase->FuzzyUsed_2 );

  // FuzzyExist/Used 3, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyExist_3,
                  pDocInfoAnalBase->FuzzyExist_3 );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->FuzzyUsed_3,
                  pDocInfoAnalBase->FuzzyUsed_3 );

  // MachExist/Used, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->MachExist,
                  pDocInfoAnalBase->MachExist );
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->MachUsed,
                  pDocInfoAnalBase->MachUsed );

  // NoneExist, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->NoneExist,
                  pDocInfoAnalBase->NoneExist );

  // NoneExist, sum overall: segments, source, modified, target
  COPY2ALLCOUNTS( pDocSaveHistSumTmp->NoneExist2,
                  pDocInfoAnalBase->NoneExist2 );

}// end of CopyResults




//+----------------------------------------------------------------------------+
//|Function name: RPTPrepareMemoryMatchCount                                   |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


BOOL RPTPrepareMemoryMatchCount(BOOL bDde, PRPT pRpt)
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

  //
  // redundancy counting
  //
  CHAR           szInFile[MAX_LONGFILESPEC] ;
  FILE           *hInput = NULL;      // input file handle
  REDCOUNTHEADER Header;
  REDCOUNTDOC    DocHeader;
  ULONG          ulLength;            // overall file length
  LONG           NumberOfDocuments=0; // Number of Documents in Redundancy int iDoc = 0;
  COUNTSUMS      RedundancySum;       // total sum of inner document
                                      // redundancies in the folder
  COUNTSUMS      UniqueCount;         // Number of segments to be trans
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


    /*******************/
    /* Open input file */
    /*******************/
    if ( fOk )
    {

      strcpy(szInFile,pRpt->szFolderObjName);
      strcat(szInFile,"\\property\\redund.log");

      hInput = fopen( szInFile, "rb" );
      if ( hInput == NULL )
      {
        // displayError message
        // GS
        pszFolder =  (pRpt->szFolder);
        fOk = FALSE;

        if ( pRpt ) pRpt->usRptStatus = RPT_KILL;

        if (bDde == TRUE)
        {
          UtlErrorHwnd (MESSAGE_RPT_NO_REDUND_AVAIL, MB_OK, 1,
                        &pszFolder, EQF_INFO, pRpt->hwndErrMsg);
        }

#ifdef NON_DDE_CNT_REPORT
        else if (bDde == FALSE)
        {
          printf( "Info==>RPTPrepareMemoryMatchCount InFile could not be opened %s\n", szInFile );
        } // end if
#endif

        pRpt->fErrorPosted = TRUE;

        // Kill the window
      }
      else
      {
#ifdef _WINDOWS
        ulLength = _filelength( _fileno(hInput) );
#else
        ulLength = filelength( fileno(hInput) );
#endif
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
#ifdef _WINDOWS
        // correction: + 3 hours
        if ( Header.lTime != 0L )
        {
          Header.lTime += 10800L;         // not necessary any more
        } /* endif */
#endif
        pTimeDate = localtime( &Header.lTime );
        NumberOfDocuments = Header.lDocuments;
        UniqueCount = Header.UniqueCount;

        // alloc memory for PRPT_REDUND size NumberOfDocuments
        fOk = UtlAlloc ((PVOID*)&pArray, 0L,
                        (ULONG)(NumberOfDocuments*sizeof(RPT_REDUND)), ERROR_STORAGE);


        pPRPT_REDUND = (PRPT_REDUND)pArray;     // set tmp pointer to Array
        pRPT_REDUND_START = (PRPT_REDUND)pArray;

        // Header info
        // -----------
        //        Header.szFolder,
        //        Header.lDocuments,
        //        pTimeDate->tm_year,
        //        pTimeDate->tm_mon + 1,
        //        pTimeDate->tm_mday,
        //        pTimeDate->tm_hour,
        //        pTimeDate->tm_min,
        //        pTimeDate->tm_sec );
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

          //DocHeader.szDocName
          //DocHeader.szDocShortName
          //DocHeader.TotalCount
          //DocHeader.ExactCount
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

              //CHAR szLabel[20];
              //sprintf( szLabel, "Red. in doc %4d:", j );
              //ListCountSum( &(Count), szLabel );

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
          hProp = OpenProperties( pRpt->pszActualDocument,
                                  pRpt->szFolderObjName,
                                  PROP_ACCESS_READ,
                                  &ErrorInfo);
          if ( hProp )
          {
            PPROPDOCUMENT   pProp;              // ptr to document properties
            pProp = (PPROPDOCUMENT) MakePropPtrFromHnd( hProp );

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


              RptLogDocPropSums( "SumsInDocProp", pProp );

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
              COPYALLCOUNTS( pDocSaveHistSumTmp->NotXlated,
                             DocSaveHistTmp);

              //                        // sum of all not translated stuff
              //                        // for Redundancy of whole folder
              //                        if ( pRpt->usReport == REDUNDANCY_REPORT )
              //                        {
              //                            RedundancySum.ulSimpleWords  += pProp->NoProps.ulSimpleWords;
              //                            RedundancySum.ulSimpleSegs   += pProp->NoProps.ulSimpleSegs;
              //                            RedundancySum.ulMediumWords  += pProp->NoProps.ulMediumWords;
              //                            RedundancySum.ulMediumSegs   += pProp->NoProps.ulMediumSegs;
              //                            RedundancySum.ulComplexWords += pProp->NoProps.ulComplexWords;
              //                            RedundancySum.ulComplexSegs  += pProp->NoProps.ulComplexSegs;
              //
              //                            RedundancySum.ulSimpleWords  -=  pPRPT_REDUND->DocuRed.ulSimpleWords;
              //                            RedundancySum.ulSimpleSegs   -=  pPRPT_REDUND->DocuRed.ulSimpleSegs;
              //                            RedundancySum.ulMediumWords  -=  pPRPT_REDUND->DocuRed.ulMediumWords;
              //                            RedundancySum.ulMediumSegs   -=  pPRPT_REDUND->DocuRed.ulMediumSegs ;
              //                            RedundancySum.ulComplexWords -=  pPRPT_REDUND->DocuRed.ulComplexWords;
              //                            RedundancySum.ulComplexSegs  -=  pPRPT_REDUND->DocuRed.ulComplexSegs;
              //
              //
              //                         } /* end if */

              // sum of all not translated stuff
              // now sum up all document redundancies
              // for Redundancy of whole folder
              if ( pRpt->usReport == REDUNDANCY_REPORT )
              {

                RedundancySum.ulSimpleWords  +=  pPRPT_REDUND->DocuRed.ulSimpleWords;
                RedundancySum.ulSimpleSegs   +=  pPRPT_REDUND->DocuRed.ulSimpleSegs;
                RedundancySum.ulMediumWords  +=  pPRPT_REDUND->DocuRed.ulMediumWords;
                RedundancySum.ulMediumSegs   +=  pPRPT_REDUND->DocuRed.ulMediumSegs ;
                RedundancySum.ulComplexWords +=  pPRPT_REDUND->DocuRed.ulComplexWords;
                RedundancySum.ulComplexSegs  +=  pPRPT_REDUND->DocuRed.ulComplexSegs;
              } /* end if */
            } /* end if*/
          }  /* end if */


          /**************************************************************/
          /* Close properties                                           */
          /**************************************************************/
          if ( hProp )
          {
            CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
          } /* endif */

        }
        /*********************************************/
        /*   Redundancies                            */
        /*********************************************/

        else if ( pRpt->usReport == REDUNDANCY_REPORT && sMaxDocs > 1 )
        {

          sIndex2++;
          // alloc memory for CALCINFO structure
          fOk = UtlAlloc ((PVOID*)&pCalcInfoTmp, 0L, sizeof(CALCINFO),
                          ERROR_STORAGE);

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


          //             // final calculation of Redundancy sum
          //             //
          //             if ( RedundancySum.ulSimpleWords >= UniqueCount.ulSimpleWords ) RedundancySum.ulSimpleWords   -= UniqueCount.ulSimpleWords ;
          //             if ( RedundancySum.ulSimpleSegs  >= UniqueCount.ulSimpleSegs ) RedundancySum.ulSimpleSegs    -= UniqueCount.ulSimpleSegs  ;
          //             if ( RedundancySum.ulMediumWords >= UniqueCount.ulMediumWords ) RedundancySum.ulMediumWords   -= UniqueCount.ulMediumWords ;
          //             if ( RedundancySum.ulMediumSegs  >= UniqueCount.ulMediumSegs ) RedundancySum.ulMediumSegs    -= UniqueCount.ulMediumSegs  ;
          //             if ( RedundancySum.ulComplexWords>= UniqueCount.ulComplexWords ) RedundancySum.ulComplexWords  -= UniqueCount.ulComplexWords;
          //             if ( RedundancySum.ulComplexSegs >= UniqueCount.ulComplexSegs ) RedundancySum.ulComplexSegs   -= UniqueCount.ulComplexSegs ;
          //
          //
          //             // Force count > 0; should be fixed on memory side
          //             //
          //             if ( RedundancySum.ulSimpleWords < UniqueCount.ulSimpleWords ) RedundancySum.ulSimpleWords   = 0;
          //             if ( RedundancySum.ulSimpleSegs  < UniqueCount.ulSimpleSegs ) RedundancySum.ulSimpleSegs    = 0;
          //             if ( RedundancySum.ulMediumWords < UniqueCount.ulMediumWords ) RedundancySum.ulMediumWords   = 0;
          //             if ( RedundancySum.ulMediumSegs  < UniqueCount.ulMediumSegs ) RedundancySum.ulMediumSegs    = 0;
          //             if ( RedundancySum.ulComplexWords< UniqueCount.ulComplexWords ) RedundancySum.ulComplexWords  = 0;
          //             if ( RedundancySum.ulComplexSegs < UniqueCount.ulComplexSegs ) RedundancySum.ulComplexSegs   = 0;



          // final calculation of Redundancy sum
          //
          RedundancySum.ulSimpleWords  =  UniqueCount.ulSimpleWords  ;
          RedundancySum.ulSimpleSegs   =  UniqueCount.ulSimpleSegs   ;
          RedundancySum.ulMediumWords  =  UniqueCount.ulMediumWords  ;
          RedundancySum.ulMediumSegs   =  UniqueCount.ulMediumSegs   ;
          RedundancySum.ulComplexWords =  UniqueCount.ulComplexWords ;
          RedundancySum.ulComplexSegs  =  UniqueCount.ulComplexSegs  ;


          // ExactExist/Used
          // Exact-Two
          //
          // redundancy
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


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptBuildOutputString                                      |
//+----------------------------------------------------------------------------+
//|Function call:    RptBuildOutputString (bDde, hwnd, pRpt)                   |
//+----------------------------------------------------------------------------+
//|Description:      This function controls the build of the selected report   |
//+----------------------------------------------------------------------------+
//|Parameters:       HWND hwnd   handle to listbox of list window              |
//|                  PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    allocate memory for output strings, stores it in RPT      |
//|                  if output to file is selected                             |
//|                    build he   r information with kind and option of        |
//|                     report, time and date of creation, documents or        |
//|                     folder selected                                        |
//|                  call function of selected report                          |
//+----------------------------------------------------------------------------+

BOOL RptBuildOutputString (BOOL bDde, HWND hwnd, PRPT pRpt)
{
  BOOL        fOk = FALSE;        // error indicator
  PVOID       pArray;             // pointer to allocated memory
  POUTPUT     pOutputField = NULL;// pointer to OUTPUT array
  POUTMRI     pOutputMris = NULL; // pointer to OUTMRI array
  time_t      lTime;              // current time/date
  struct tm   *pTimeDateTmp;      // pointer to time/date structure

  pRpt->usStringIndex = 0;  // index for output field

  // alloc memory for output strings
  fOk = UtlAlloc ((PVOID*)&pArray, 0L, sizeof (OUTPUT), ERROR_STORAGE);

  if ( fOk )
  {
    pRpt->pOutputField = (POUTPUT)pArray;  // store memory address
    pOutputField = (POUTPUT)pArray;        // set pointer to OUTPUT
    pOutputMris = pRpt->pOutputMris;       // set pointer to OUTMRI
  }

  // output now to file and screen
  //   if ( fOk && pRpt->fRptFile  )
  if ( fOk  )
  {

    if (pRpt->fRptFile)
    {

      // set output file process to open and process it
      pRpt->usOutputFileProcess = OPEN_OUTPUT_FILE;
      fOk = RptOutputToFile (pRpt);

    }

    if ( fOk )
    {


      // REPORT HEADER

      // begin title
      sprintf (pOutputField->szFeld[pRpt->usStringIndex++], "%-30.30s ", STR_RPT3_BEGIN_TITLE);

      // build kind of report, option
      sprintf (pOutputField->szFeld[pRpt->usStringIndex++], "%s - %s",
               pRpt->szReport, pRpt->szOption );
      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

      if ( pRpt->szRptDescription[0]!=EOS )
      {
        sprintf (pOutputField->szFeld[pRpt->usStringIndex++], "%s",
                 pRpt->szRptDescription);

        *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
      } /* endif */

      // version string
      {
        LOADSTRING( NULLHANDLE, hResMod, SID_LOGO_REVISION, pRpt->szWorkString );

        sprintf( pOutputField->szFeld[pRpt->usStringIndex++],
                 "%-25.25s :   %s", "TranslationManager", pRpt->szWorkString );
      }

      // calculation profile name
      {
        Utlstrccpy( pRpt->szWorkString, pRpt->szProfile, DOT );

        // check if we are dealing with a protected profile
        // check if we are dealing with a protected profile
        if ( RPTIsProtectedProfile( pRpt->szWorkString  ) )
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
            ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd(hpropFolder);
            if ( RptCheckProfileStamp( ppropFolder ) )
            {
              if ( ppropFolder->szIntProfileName[0] != EOS )
              { 
                fProfileIsIntact = TRUE;
                strcpy( pRpt->szWorkString, ppropFolder->szIntProfileName );
              } /* endif */
            } /* endif */
            CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo);
          } /* endif */
          
          if ( !fProfileIsIntact )
          {
            pRpt->szWorkString[0] = '*';
            Utlstrccpy( pRpt->szWorkString+1, pRpt->szProfile, DOT );
            strcat( pRpt->szWorkString, "*" );
          } /* endif */
        } /* endif */

        sprintf( pOutputField->szFeld[pRpt->usStringIndex++],
          "%-25.25s :   %s", "Calculation Profile", pRpt->szWorkString );
      }


      // build header
      time (&lTime);                                  // get current time/date

      // correction: - 3 hours
      if ( lTime != 0L )
      {
        lTime -= 10800L;
      } /* endif */
 
      pTimeDateTmp = localtime (&lTime);              // fill time/date structure




      LONG2DATETIME (lTime, pRpt->szWorkString);      // convert ltime to string

      sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
               "%-25.25s :   %s",
               pOutputMris->szFeld[RPT_RPTGENERATED], // Report generatet at
               pRpt->szWorkString
              );

      // output of folder name as long name
      {
          BOOL bSubFolder = (strchr( pRpt->szLongFolderName, BACKSLASH ) != NULL);

          if (bSubFolder == FALSE)
          {
            ObjShortToLongName( pRpt->szFolder, pRpt->szLongFolderName, FOLDER_OBJECT );
            OEMTOANSI( pRpt->szLongFolderName );
          }
      }

      sprintf (pOutputField->szFeld[pRpt->usStringIndex++], "%-25.25s :   %s",
               pOutputMris->szFeld[RPT_FOLDER],       // Folder
               pRpt->szLongFolderName
              );

      // number of documents in folder
      sprintf( pOutputField->szFeld[pRpt->usStringIndex++], "%-25.25s :   %u",
                "Documents in folder", 
                pRpt->usAllDocuments
              );

      // number of selected documents 
      sprintf( pOutputField->szFeld[pRpt->usStringIndex++], "%-25.25s :   %u",
                "Documents selected", 
                (USHORT)(pRpt->fFolderSelected ? pRpt->usAllDocuments : pRpt->usSelectedDocuments)
              );

      if ( pRpt->usReport == REDUNDANCY_REPORT )
      {
        // for redundancy report only: number of processed documents
        sprintf( pOutputField->szFeld[pRpt->usStringIndex++], "%-25s :   %u",
                  "Documents prepared for redundany report", 
                  pRpt->usProcessedDocuments );
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

        sprintf( pOutputField->szFeld[pRpt->usStringIndex++], "%-25s :   %u",
                  "Documents processed in shipment", 
                  pRpt->usProcessedDocuments );
      } /* endif */

      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

      // end title
      sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
               "%-30.30s ",
               STR_RPT3_END_TITLE);

      pOutputField->usNum = pRpt->usStringIndex;  // update number of lines
      pRpt->usStringIndex = 0;                    // set index


      if (pRpt->fRptFile)
      {

        fOk = RptOutputToFile (pRpt);  // output to file

    }// end if


#ifdef NON_DDE_CNT_REPORT
      if (bDde == TRUE)
#endif
      // output to screen

      fOk = RptOutputToWindow (hwnd, pRpt);      // output to list window


    }// end if
  } // end if

  if ( fOk )
  {
    switch ( pRpt->usReport )
    {
      case HISTORY_REPORT: // history report
        fOk = RptReport1 (hwnd, pRpt);
        break; // end case

      case CALCULATION_REPORT: // calculating report
        fOk = RptReport2 (hwnd, pRpt);
        break; // end case

      case SUMMARY_COUNTING_REPORT: // calculating report
      case PRE_ANALYSIS_REPORT:     // source/source  source/nlv report
        fOk = RptReport3 (hwnd, pRpt);
        break; // end case

      case REDUNDANCY_REPORT:     // source/source  source/nlv report
        fOk = RptReport3 (hwnd, pRpt);
        break; // end case

      case COMBINED_REPORT:     // list of most used segments
        fOk = RptReport4 (hwnd, pRpt);
        break; // end case
        break;
    } // end switch
  } // end if

  if ( fOk && pRpt->hHTMLControl )
  {
    // close HTML report file
    UtlClose( pRpt->hHTMLControl, FALSE );

    pRpt->hHTMLControl = 0;

    // navigate in Web control to HTML file containg the report data
    {
      HWND hReportWindow = EqfQueryObject( pRpt->szRptInstanceObjName, clsREPORT, 0 );
      PostMessage( hReportWindow, WM_EQF_PROCESSTASK, MP1FROMSHORT(OPEN_AND_POSITION_TASK), 0L );
    }
  } /* endif */

  if ( fOk && pRpt->fRptFile )
  {
    // set output file process to close and process it
    pRpt->usOutputFileProcess = CLOSE_OUTPUT_FILE;
    fOk = RptOutputToFile (pRpt);
  }

  return fOk;
} // end of RptBuildOutputString


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptOutputToWindow                                         |
//+----------------------------------------------------------------------------+
//|Function call:    RptOutputToWindow (hwnd, pRpt)                            |
//+----------------------------------------------------------------------------+
//|Description:      This function inserts the output strings into the LB      |
//|                  of the list window                                        |
//+----------------------------------------------------------------------------+
//|Parameters:       HWND hwnd   handle to listbox of list window              |
//|                  PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    while output string exists                                |
//|                    insert string in LB of list window                      |
//+----------------------------------------------------------------------------+

BOOL RptOutputToWindow (HWND hwnd, PRPT pRpt)
{
  BOOL   fOk = TRUE;    // error indicator
  USHORT usIndex = 0;    // index for output strings

  if ( pRpt->hHTMLControl )
  {
//    pRpt->usOutputFileProcess = WRITE_OUTPUT_FILE;
    pRpt->usOutputFileProcess = OPEN_OUTPUT_FILE;
    fOk = RptOutputToHTMLFile (pRpt, pRpt->hHTMLControl, TRUE );

    pRpt->usOutputFileProcess = WRITE_OUTPUT_FILE;
    fOk = RptOutputToHTMLFile (pRpt, pRpt->hHTMLControl, TRUE );

  }
  else
  {
    // loop over all output strings, eliminate tags
    while ( usIndex < pRpt->pOutputField->usNum )
    {
      // insert string at end of LB
      if ( pRpt->pOutputField->szFeld[usIndex][0] != '<' )
      {
        INSERTITEMENDHWND (hwnd, pRpt->pOutputField->szFeld[usIndex]);
      }/* end if */

      // Insert doc name into Itemhandle
      // -------------------------------
      if ( pRpt->usReport == COMBINED_REPORT )
      {
      } /* endif */


      usIndex++;
    }

  } /* endif */



  return fOk;
} // end of function RptOutputToWindow


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptOutputToFile                                           |
//+----------------------------------------------------------------------------+
//|Function call:    RptOutputToFile (pRpt)                                    |
//+----------------------------------------------------------------------------+
//|Description:      This function opens the output file, writes the output    |
//|                  strings to the file and close it                          |
//+----------------------------------------------------------------------------+
//|Parameters:       PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    switch output file process                                |
//|                    case OPEN:                                              |
//|                      open output file                                      |
//|                      if error                                              |
//|                        set file process to ERROR                           |
//|                      else                                                  |
//|                        set file process to WRITE                           |
//|                    case WRITE:                                             |
//|                      loop over all output strings of OUTPUT                |
//|                        append CRLF to output line, write line to file      |
//|                        if error                                            |
//|                          set file process to ERROR, close output file      |
//|                    case CLOSE:                                             |
//|                      if file handle                                        |
//|                        close file handle                                   |
//|                    default:                                                |
//|                      break                                                 |
//+----------------------------------------------------------------------------+

BOOL RptOutputToFile (PRPT pRpt)
{
  BOOL    fOk = TRUE;

  if ( pRpt->usFormat == ASCII )
  {
    fOk = RptOutputToFileWork (pRpt);
  }
  else if ( pRpt->usFormat == RTF )
  {
    fOk =  RptOutputToRTFFile (pRpt);
  }
  else if ( pRpt->usFormat == HTML )
  {

    fOk =  RptOutputToHTMLFile (pRpt, pRpt->hfOutputFileHandle, FALSE);
  }
  else
  {
    fOk = RptOutputToFileWork (pRpt);
  }

  return fOk;

}



BOOL RptOutputToFileWork (PRPT pRpt)
{
  BOOL    fOk = TRUE;                 // error indicator
  USHORT  usFileRc = 0;               // return from file handle
  USHORT  usActionTaken = 0;          // action taken by UtlOpen
  USHORT  usBytesWrite = 0;           // number of bytes written by UtlWrite
  POUTPUT pOutputFieldTmp;            // tmp pointer to OUTPUT array


  pRpt->usStringIndex = 0;            // index of current string

  switch ( pRpt->usOutputFileProcess )
  {
    case OPEN_OUTPUT_FILE:  // open output file
      usFileRc = UtlOpen (pRpt->szRptOutputFile,
                          &(pRpt->hfOutputFileHandle),
                          &usActionTaken,
                          0L,
                          FILE_NORMAL,
                          OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                          OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYREAD,
                          0L,
                          NOMSG);

      if ( usFileRc == NO_ERROR )
      {
        pRpt->usOutputFileProcess = WRITE_OUTPUT_FILE; // set process to write
      }
      else
      {
        pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
        fOk = FALSE;
      }
      break; // end case

    case WRITE_OUTPUT_FILE:  // write to output file
      pOutputFieldTmp = pRpt->pOutputField;  // set pointer to OUTPUT

      // loop over all output strings, eliminate tags
      while ( fOk && pRpt->usStringIndex < pRpt->pOutputField->usNum )
      {
        // append CR/LF
        sprintf (pRpt->szWorkString, "%s\r\n",
                 pOutputFieldTmp->szFeld[pRpt->usStringIndex++]);

        // write string to file
        if ( !strncmp(pRpt->szWorkString,STR_RPT3_BEGIN_HEADER,strlen(STR_RPT3_BEGIN_HEADER)) )
        {
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               ASCII_Page_Break,
                               (USHORT)strlen (ASCII_Page_Break),
                               &usBytesWrite,
                               FALSE );
          sprintf(pRpt->szWorkString,"          Page  %d \n\n", PageNumber);
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               pRpt->szWorkString,
                               (USHORT)strlen (pRpt->szWorkString),
                               &usBytesWrite,
                               FALSE );

          PageNumber += 1;
        }
        else if ( pRpt->szWorkString[0] != '<' )
        {
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               pRpt->szWorkString,
                               (USHORT)strlen (pRpt->szWorkString),
                               &usBytesWrite,
                               FALSE );

          if ( usFileRc != NO_ERROR )
          {
            pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
            UtlClose (pRpt->hfOutputFileHandle, NOMSG);     // close output file
            pRpt->hfOutputFileHandle = NULLHANDLE;          // set file handle to NULL
            fOk = FALSE;
          } // end if
        }// end if
      } // end while
      break; // end case

    case CLOSE_OUTPUT_FILE:  // close output file
      if ( pRpt->hfOutputFileHandle )
      {
        // close output file
        usFileRc = UtlClose (pRpt->hfOutputFileHandle, NOMSG);

        if ( usFileRc != NO_ERROR )
        {
          pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
          fOk = FALSE;
        } // end if
        else
        {
          pRpt->hfOutputFileHandle = NULLHANDLE;
        }
      } // end if
      break; // end case

    default:
      break;
  } // end switch

  return fOk;
} // end of RptOutputToFile




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptOutputToRTFFile                                        |
//|                  TC Summary Calculating report                             |
//+----------------------------------------------------------------------------+
//|Function call:    RptOutputToRTFFile (pRpt)                                 |
//+----------------------------------------------------------------------------+
//|Description:      This function opens the output file, writes the output    |
//|                  strings to the file in RTF format and close it            |
//+----------------------------------------------------------------------------+
//|Parameters:       PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    switch output file process                                |
//|                    case OPEN:                                              |
//|                      open output file                                      |
//|                      if error                                              |
//|                        set file process to ERROR                           |
//|                      else                                                  |
//|                        set file process to WRITE                           |
//|                    case WRITE:                                             |
//|                      loop over all output strings of OUTPUT                |
//|                        append CRLF to output line, write line to file      |
//|                        if error                                            |
//|                          set file process to ERROR, close output file      |
//|                    case CLOSE:                                             |
//|                      if file handle                                        |
//|                        close file handle                                   |
//|                    default:                                                |
//|                      break                                                 |
//+----------------------------------------------------------------------------+

BOOL RPT_STRING_REPLACE(PSZ pSource, char Pattern1, PSZ Pattern2)
{
  BOOL fOk=TRUE;
  int  i;
  int  pos_target=0;
  char pTarget[2*MAX_O_LENGTH];
  PSZ  pBegin = pSource;
  int  iLenPattern2 = strlen(Pattern2);

  while ( pSource[0]!=EOS )
  {
    if ( pSource[0]==Pattern1 )
    {
      for ( i=0; i < iLenPattern2; i++ )
      {
        pTarget[pos_target++] = Pattern2[i];
      } // end for
    }
    else
    {
      pTarget[pos_target++] = pSource[0];

    } // end if

    pSource++;
  }// end while

  pTarget[pos_target++]=EOS;

  for ( i=0; i<pos_target;i++ )
  {
    pBegin[i] = pTarget[i];
  } // end for

  return fOk;



} // end of function RPT_REPLACE


BOOL RptOutputToRTFFile (PRPT pRpt)
{
  BOOL    fOk = TRUE;                 // error indicator
  USHORT  usFileRc = 0;               // return from file handle
  USHORT  usActionTaken = 0;          // action taken by UtlOpen
  USHORT  usBytesWrite = 0;           // number of bytes written by UtlWrite
  POUTPUT pOutputFieldTmp;            // tmp pointer to OUTPUT array
  int     i;



  pRpt->usStringIndex = 0;            // index of current string

  switch ( pRpt->usOutputFileProcess )
  {
    case OPEN_OUTPUT_FILE:  // open output file
      usFileRc = UtlOpen (pRpt->szRptOutputFile,
                          &(pRpt->hfOutputFileHandle),
                          &usActionTaken,
                          0L,
                          FILE_NORMAL,
                          OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                          OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYREAD,
                          0L,
                          NOMSG);

      // RTF Header
      usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                           RTF_Header_1,
                           (USHORT)strlen (RTF_Header_1),
                           &usBytesWrite,
                           FALSE );

      if ( usFileRc == NO_ERROR )
      {
        pRpt->usOutputFileProcess = WRITE_OUTPUT_FILE; // set process to write
      }
      else
      {
        pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
        fOk = FALSE;
      }
      break; // end case

    case WRITE_OUTPUT_FILE:  // write to output file
      //----------------------------------------------

      pOutputFieldTmp = pRpt->pOutputField;  // set pointer to OUTPUT

      // loop over all output strings
      while ( fOk && pRpt->usStringIndex < pRpt->pOutputField->usNum )
      {
        // append CR/LF
        sprintf (pRpt->szWorkString, "%s\r\n",
                 pOutputFieldTmp->szFeld[pRpt->usStringIndex++]);

        // write string to file
        // ********************

        // Handling of tables
        if ( !strncmp(pRpt->szWorkString,STR_RPT3_BEGIN_HEADER,strlen(STR_RPT3_BEGIN_HEADER)) )
        {
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_Page_Break,
                               (USHORT)strlen (RTF_Page_Break),
                               &usBytesWrite,
                               FALSE );

          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_Line_Begin,
                               (USHORT)strlen (RTF_Line_Begin),
                               &usBytesWrite,
                               FALSE );
          sprintf(pRpt->szWorkString,"          Page  %d \n\n", PageNumber);
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               pRpt->szWorkString,
                               (USHORT)strlen (pRpt->szWorkString),
                               &usBytesWrite,
                               FALSE );
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_Line_End,
                               (USHORT)strlen (RTF_Line_End),
                               &usBytesWrite,
                               FALSE );

          PageNumber += 1;
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_BOLD,
                               (USHORT)strlen (RTF_BOLD),
                               &usBytesWrite,
                               FALSE );

        }
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_END_HEADER,strlen(STR_RPT3_END_HEADER)) )
        {
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_UNMAKE,
                               (USHORT)strlen (RTF_UNMAKE),
                               &usBytesWrite,
                               FALSE );

        }
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_BEGIN_TITLE,strlen(STR_RPT3_BEGIN_TITLE)) )
        {
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_BOLD,
                               (USHORT)strlen (RTF_BOLD),
                               &usBytesWrite,
                               FALSE );

        }
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_END_HEADER,strlen(STR_RPT3_END_TITLE)) )
        {
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_UNMAKE,
                               (USHORT)strlen (RTF_UNMAKE),
                               &usBytesWrite,
                               FALSE );

        }

        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_BEGIN_TEXT,strlen(STR_RPT3_BEGIN_TEXT)) )
        {
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_Page_Break,
                               (USHORT)strlen (RTF_Page_Break),
                               &usBytesWrite,
                               FALSE );
        }

        else if ( pRpt->szWorkString[0]=='<' )
        {
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               EMPTY_STRING,
                               (USHORT)strlen (EMPTY_STRING),
                               &usBytesWrite,
                               FALSE );
        }
        else
        {

          // RTF line begin
          usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                               RTF_Line_Begin,
                               (USHORT)strlen (RTF_Line_Begin),
                               &usBytesWrite,
                               FALSE );

          if ( usFileRc == NO_ERROR )
          {

            if ( strlen (pRpt->szWorkString) >=2 )
            {

              // strip blanks at end
              pRpt->szWorkString[strlen(pRpt->szWorkString)-2] = EOS;
              i=strlen(pRpt->szWorkString)-1;
              while ( i>=0 && pRpt->szWorkString[i]==BLANK )
              {
                i--;
              } // end while
              pRpt->szWorkString[i+1] = EOS;

              // necessary Replacements

              RPT_STRING_REPLACE(pRpt->szWorkString,'\\',"\\\\");

              usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                                   pRpt->szWorkString,
                                   (USHORT)strlen (pRpt->szWorkString),
                                   &usBytesWrite,
                                   FALSE );
            }
            else
            {
              usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                                   pRpt->szWorkString,
                                   (USHORT)strlen (pRpt->szWorkString),
                                   &usBytesWrite,
                                   FALSE );
            }
          }

          // RTF line end

          if ( usFileRc == NO_ERROR )
          {
            usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                                 RTF_Line_End,
                                 (USHORT)strlen (RTF_Line_End),
                                 &usBytesWrite,
                                 FALSE );
          }

        } //end if BEGIN_TABLE,END_TABLE

        if ( usFileRc != NO_ERROR )
        {
          pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
          UtlClose (pRpt->hfOutputFileHandle, NOMSG);     // close output file
          pRpt->hfOutputFileHandle = NULLHANDLE;          // set file handle to NULL
          fOk = FALSE;
        } // end if
      } // end while
      break; // end case

    case CLOSE_OUTPUT_FILE:  // close output file
      if ( pRpt->hfOutputFileHandle )
      {

        // RTF document end
        usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                             RTF_Document_End,
                             (USHORT)strlen (RTF_Document_End),
                             &usBytesWrite,
                             FALSE );
        // close output file
        usFileRc = UtlClose (pRpt->hfOutputFileHandle, NOMSG);

        if ( usFileRc != NO_ERROR )
        {
          pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
          fOk = FALSE;
        } // end if
        else
        {
          pRpt->hfOutputFileHandle = NULLHANDLE;
        }
      } // end if
      break; // end case

    default:
      break;
  } // end switch

  return fOk;
} // end of RptOutputToRTFFile




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptOutputToHTMLFile                                       |
//|                  TC Summary Calculating report                             |
//+----------------------------------------------------------------------------+
//|Function call:    RptOutputToHTMLFile (pRpt)                                |
//+----------------------------------------------------------------------------+
//|Description:      This function opens the output file, writes the output    |
//|                  strings to the file in RTF format and close it            |
//+----------------------------------------------------------------------------+
//|Parameters:       PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    switch output file process                                |
//|                    case OPEN:                                              |
//|                      open output file                                      |
//|                      if error                                              |
//|                        set file process to ERROR                           |
//|                      else                                                  |
//|                        set file process to WRITE                           |
//|                    case WRITE:                                             |
//|                      loop over all output strings of OUTPUT                |
//|                        append CRLF to output line, write line to file      |
//|                        if error                                            |
//|                          set file process to ERROR, close output file      |
//|                    case CLOSE:                                             |
//|                      if file handle                                        |
//|                        close file handle                                   |
//|                    default:                                                |
//|                      break                                                 |
//+----------------------------------------------------------------------------+



BOOL RptOutputToHTMLFile
(
  PRPT  pRpt,
  HFILE hFile,
  BOOL  fView
)
{
  BOOL    fOk = TRUE;                 // error indicator
  USHORT  usFileRc = 0;               // return from file handle
  USHORT  usActionTaken = 0;          // action taken by UtlOpen
  USHORT  usBytesWrite = 0;           // number of bytes written by UtlWrite
  POUTPUT pOutputFieldTmp;            // tmp pointer to OUTPUT array
  int     i;
  CHAR    szLine[1000];               // output of header Information
  PSZ     pszSum;
  RPT_HEADER rptHeader[50];           // Scan header information
  int     iHeader=0;                  //   -"-
  int     iwidth=0;                   // header width



  pRpt->usStringIndex = 0;            // index of current string

  switch ( pRpt->usOutputFileProcess )
  {
    case OPEN_OUTPUT_FILE:  // open output file
      //----------------------------------------------


      if (!fView)
      {

        usFileRc = UtlOpen (pRpt->szRptOutputFile,
                            &(pRpt->hfOutputFileHandle),
                            &usActionTaken,
                            0L,
                            FILE_NORMAL,
                            OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                            OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYREAD,
                            0L,
                            NOMSG);


      }

      // HTML Header
      // -----------
      strcpy(szLine,"<HTML><BODY bgcolor=white>\n");


      if (!fView)
      {

        usFileRc = UtlWrite (pRpt->hfOutputFileHandle,
                             szLine,
                             (USHORT)strlen (szLine),
                             &usBytesWrite,
                             FALSE );


      }
      else
      {
        usFileRc = UtlWrite (hFile,
                             szLine,
                             (USHORT)strlen (szLine),
                             &usBytesWrite,
                             FALSE );

      }//end if


      if ( usFileRc == NO_ERROR )
      {
        pRpt->usOutputFileProcess = WRITE_OUTPUT_FILE; // set process to write
      }
      else
      {
        pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
        fOk = FALSE;
      }
      break; // end case

    case WRITE_OUTPUT_FILE:  // write to output file
      //----------------------------------------------

      pOutputFieldTmp = pRpt->pOutputField;  // set pointer to OUTPUT

      // ****************************
      // loop over all output strings
      // ****************************

      while ( fOk && pRpt->usStringIndex < pRpt->pOutputField->usNum )
      {
        // copy current string to work string and append CRLF
        // for segment lines of the redundand segment list we have to escape the characters "<" and ">" as well
        {
          PSZ pszSource = pOutputFieldTmp->szFeld[pRpt->usStringIndex++];
          PSZ pszTarget = pRpt->szWorkString;
          BOOL fEscapingRequired = (pRpt->usReport == COMBINED_REPORT) && (strncmp( pszSource, "Segment:", 8 ) == 0);

          while ( *pszSource )
          {
            if ( fEscapingRequired && (*pszSource == '<') )
            {
              strcpy( pszTarget, "&lt;" );
              pszTarget += strlen(pszTarget);
              pszSource++;
            }
            else if ( fEscapingRequired && (*pszSource == '<') )
            {
              strcpy( pszTarget, "&gt;" );
              pszTarget += strlen(pszTarget);
              pszSource++;
            }
            else
            {
              *pszTarget++ = *pszSource++;
            } /* endif */
          } /*endwhile */
          strcpy( pszTarget, "\r\n" );
        }

        // ********************
        // write string to file
        // Handling of tables
        // ********************

        // Begin of (Table) Header Information
        // -----------------------------------
        if ( !strncmp(pRpt->szWorkString,STR_RPT3_BEGIN_HEADER,strlen(STR_RPT3_BEGIN_HEADER)) )
        {

          if (fView) fViewHeader = TRUE;
          else  fHeader = TRUE;


          strcpy(szLine,
                 "<BR><BR><TABLE BORDER=1 ><tr><td><TABLE  BORDER=0 CELLPADDING=0 CELLSPACING=0 >\n");
          strcat(szLine,"<tr><td width=130></td><td width=200></td></tr>");
          strcpy(pRpt->szWorkString," ");
          usFileRc = UtlWrite (  hFile,
                                 szLine,
                                 (USHORT)strlen (szLine),
                                 &usBytesWrite,
                                 FALSE );


        }
        // End of (Table) Header Information
        // ---------------------------------
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_END_HEADER,strlen(STR_RPT3_END_HEADER)) )
        {

          if (fView) fViewHeader = FALSE;
          else  fHeader = FALSE;


          strcpy(szLine,"</TABLE></td></tr></Table>\n") ;
          strcpy(pRpt->szWorkString," ");
          usFileRc = UtlWrite (  hFile,
                                 szLine,
                                 (USHORT)strlen (szLine),
                                 &usBytesWrite,
                                 FALSE );
        }
        // Begin of Text Information
        // ---------------------------
        if ( !strncmp(pRpt->szWorkString,STR_RPT3_BEGIN_TEXT,strlen(STR_RPT3_BEGIN_TEXT)) )
        {

          if (fView) fViewText = TRUE;
          else  fText = TRUE;


          strcpy(szLine,"<h3>\n") ;
          usFileRc = UtlWrite (  hFile,
                                 szLine,
                                 (USHORT)strlen (szLine),
                                 &usBytesWrite,
                                 FALSE );


        }
        // End of Text Information
        // ---------------------------
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_END_TEXT,strlen(STR_RPT3_END_TEXT)) )
        {
          if (fView) fViewText = FALSE;
          else  fText = FALSE;

          strcpy(szLine,"</h3>\n") ;
          usFileRc = UtlWrite (  hFile,
                                 szLine,
                                 (USHORT)strlen (szLine),
                                 &usBytesWrite,
                                 FALSE );
        }
        // Begin of Title Information
        // ---------------------------
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_BEGIN_TITLE,strlen(STR_RPT3_BEGIN_TITLE)) )
        {

          if (fView) fViewText = TRUE;
          else  fText = TRUE;


          strcpy(szLine,"<h2>\n") ;
          usFileRc = UtlWrite (  hFile,
                                 szLine,
                                 (USHORT)strlen (szLine),
                                 &usBytesWrite,
                                 FALSE );
        }
        // End of Title Information
        // ---------------------------
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_END_TITLE,strlen(STR_RPT3_END_TITLE)) )
        {
          if (fView) fViewText = FALSE;
          else  fText = FALSE;


          strcpy(szLine,"</h2>\n") ;
          usFileRc = UtlWrite (  hFile,
                                 szLine,
                                 (USHORT)strlen (szLine),
                                 &usBytesWrite,
                                 FALSE );
        }
        // Begin of Table Construction
        // ---------------------------
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_BEGIN_TABLE,strlen(STR_RPT3_BEGIN_TABLE)) )
        {
          if (fView) fViewTable = TRUE;
          else  fTable = TRUE;


          strcpy(pRpt->szWorkString, " ");
//         strcpy(pRpt->szWorkString,
//         "<TABLE WIDTH=800 BORDER=1 CELLPADDING=0 CELLSPACING=0 >\n");
//         usFileRc = UtlWrite (  hFile,
//                            pRpt->szWorkString,
//                            strlen (pRpt->szWorkString),
//                            &usBytesWrite,
//                            FALSE );
        }
        // End of Table Construction
        // ---------------------------
        else if ( !strncmp(pRpt->szWorkString,STR_RPT3_END_TABLE,strlen(STR_RPT3_END_TABLE)) )
        {

          if (fView) fViewTable = FALSE;
          else  fTable = FALSE;


          if (fView) NumberOfViewLines = 0;
          else   NumberOfLines = 0;

          //strcat(pRpt->szWorkString,"<br>");
          strcat(pRpt->szWorkString,"\n");
          usFileRc = UtlWrite (  hFile,
                                 pRpt->szWorkString,
                                 (USHORT)strlen (pRpt->szWorkString),
                                 &usBytesWrite,
                                 FALSE );
        }
        // Line in Table Contruction
        // ---------------------------
        else if ( pRpt->szWorkString[0]=='=' ||
                  (pRpt->szWorkString[0]=='-' &&
                   (  (fView && !fViewHeader) || (!fView && !fHeader)   )   ) )
        {
          // eliminate
        }
        else if ( pRpt->szWorkString[1]=='=' ||
                  (pRpt->szWorkString[1]=='-' &&
                   (  (fView && !fViewHeader) || (!fView && !fHeader)   )   ) )
        {
          // eliminate
        }
        // Standard Output
        // ---------------------------
        else
        {
          int iMax = 0;


          if ( (fView && fViewText) ||  (!fView && fText) )
          {
            if ( strlen(pRpt->szWorkString)>10 )
            {
              strcat(pRpt->szWorkString,"<br>\n");
            } /* endif */
          } /* endif */


          if ( (  (fView && fViewHeader) || (!fView && fHeader)   )  && pRpt->szWorkString[0]=='-' )
          {
            strcpy(pRpt->szWorkString,"<tr><td>&nbsp;</td><tr>");

          }
          else if ( (  (fView && fViewHeader) || (!fView && fHeader)   ) )
          {
            int i=strlen(pRpt->szWorkString)-1;
            BOOL FOUND=FALSE;
            while ( i>=0 && !FOUND )
            {
              if ( pRpt->szWorkString[i]!=' ' )
              {
                FOUND=TRUE;
                pRpt->szWorkString[i+1]=EOS;
              } /* end if */
              i--;
            }/* end while */

            if ( strlen(pRpt->szWorkString)>5 )
            {
              PSZ pszFirst;

              strcpy(szLine,"<tr><td><b>");
              pszFirst=strchr(pRpt->szWorkString,':');
              if ( pszFirst && *(pszFirst+1)=='\\' )
              {
                pszFirst=strrchr(pRpt->szWorkString,':');
              } /* endif */

              if ( pszFirst )
              {
                *pszFirst='@';
              }
              else
              {
                strcat(szLine, "&nbsp; </b></td><td> <b>");
              } /* endif */
              RPT_STRING_REPLACE(pRpt->szWorkString,'@',
                                 " &nbsp;</b></td><td> <b>");
              strcat(szLine,pRpt->szWorkString);
              strcat(szLine,"</b></td></tr>");
              strcpy(pRpt->szWorkString,szLine);
            } /* endif */
          } /* endif */

          if ( (fView && fViewTable) || (!fView && fTable)  )
          {
            char * pszEnd;

            if (fView) NumberOfViewLines ++;    // globale variable
            else NumberOfLines ++;

            // Begin of Table scan header information
            if (     ((fView && NumberOfViewLines == 1 )
                      || (!fView && NumberOfLines == 1 ) )
                     &&  pRpt->szWorkString[0]=='|' )
            {
              int i;
              int j=0;
              iHeader = 0;
              if ( pRpt->usReport==HISTORY_REPORT )
              {
                iwidth=320;
              }
              else
              {
                iwidth=60;
              } /* end if */

              iMax = strlen(pRpt->szWorkString);
              for ( i=1 ; i<iMax; i++ )
              {

                if ( pRpt->szWorkString[i] == '|' )
                {
                  rptHeader[iHeader].szText[j]=EOS;
                  rptHeader[iHeader].iColSpan = 0;
                  j=0;
                  iHeader ++;
                  iwidth += 60;
                }
                else
                {
                  if ( (pRpt->szWorkString[i])>5 )
                  {
                    rptHeader[iHeader].szText[j]=pRpt->szWorkString[i];
                  } /* endif */
                  j++;
                } /* endif */
              } /* endfor */

              // calculate colspan, only colspan=3
              for ( i=0 ;i<iHeader ;i++ )
              {
                if ( strcmp(rptHeader[i].szText,rptHeader[i+1].szText) )
                {
                  rptHeader[i].iColSpan = 1;
                }
                else if ( (!strcmp(rptHeader[i].szText,"Analyze") ||
                           !strcmp(rptHeader[i].szText,"   Sum ") ||
                           !strcmp(rptHeader[i].szText," Fuzzy ") ) &&
                          !strcmp(rptHeader[i].szText,rptHeader[i+1].szText) &&
                          !strcmp(rptHeader[i].szText,rptHeader[i+2].szText) )
                {
                  rptHeader[i].iColSpan = 3;
                  rptHeader[i+1].iColSpan = 0;
                  rptHeader[i+2].iColSpan = 0;
                  i=i+2;
                }
                else if ( !strcmp(rptHeader[i].szText,"  Edit ") &&
                          !strcmp(rptHeader[i].szText,rptHeader[i+1].szText) )
                {
                  rptHeader[i].iColSpan = 2;
                  rptHeader[i+1].iColSpan = 0;
                  i=i+1;
                }
                else
                {
                  rptHeader[i].iColSpan = 1;
                } /* endif */
              } /* endfor */

              // print table header

              //xx
              sprintf(pRpt->szWorkString,
                      "\n<TABLE WIDTH=%d BORDER=1 CELLPADDING=0 CELLSPACING=0 >\n",
                      iwidth);
              usFileRc = UtlWrite (  hFile,
                                     pRpt->szWorkString,
                                     (USHORT)strlen (pRpt->szWorkString),
                                     &usBytesWrite,
                                     FALSE );

              // print first header line
              strcpy(szLine,"<tr align=center bgcolor=\"D6D6D6\">");
              for ( i=0 ;i<iHeader ;i++ )
              {
                if ( rptHeader[i].iColSpan == 1 )
                {
                  strcat(szLine,"<td colspan=1><b>");
                  strcat(szLine,rptHeader[i].szText);
                  strcat(szLine,"&nbsp;</b></td>");
                }
                else if ( rptHeader[i].iColSpan == 2 )
                {
                  strcat(szLine,"<td  colspan=2 bgcolor=antiquewhite><b>"); strcat(szLine,rptHeader[i].szText);
                  strcat(szLine,"&nbsp;</b></td>");
                } /* endif */
                else if ( rptHeader[i].iColSpan == 3 )
                {
                  strcat(szLine,"<td  colspan=3 bgcolor=antiquewhite><b>");
                  strcat(szLine,rptHeader[i].szText);
                  strcat(szLine,"&nbsp;</b></td>");
                } /* endif */

              } /* endfor */

              strcat(szLine,"</tr>\n");

            } /* endif first line */



            if ( pRpt->szWorkString[0]=='|' )
            {
              pRpt->szWorkString[0] = '\xF5';
            } /* endif */


            pszEnd=strrchr(pRpt->szWorkString,'|');
            if ( pszEnd )
            {
              *pszEnd='@';
            } /* endif */

            // scan for "sum" row
            // ------------------
            pszSum = strstr(pRpt->szWorkString,"Sum");



            // Table Header

            if ( ((fView && NumberOfViewLines == 1 )
                  || (!fView && NumberOfLines == 1 ) ))
            {
              // job done
            }
            else if ( ((fView && NumberOfViewLines == 2 )
                       || (!fView && NumberOfLines == 2 ) ) )
            {
              strcpy(szLine,"<tr align=center bgcolor=\"D6D6D6\">");
              RPT_STRING_REPLACE(pRpt->szWorkString,'\xF5',"<td><b>");
              RPT_STRING_REPLACE(pRpt->szWorkString,'|',"&nbsp;</b></td><td><b>");
              RPT_STRING_REPLACE(pRpt->szWorkString,'@',"&nbsp;</b></td>");
              strcat(szLine,pRpt->szWorkString);
              strcat(szLine,"</tr>\n");
            }
            // sum row, colored
            else if ( pszSum && (pszSum-&(pRpt->szWorkString[0]))>15 )
            {
              strcpy(szLine,"<tr align=right bgcolor=\"D6D6D6\">");
              RPT_STRING_REPLACE(pRpt->szWorkString,'\xF5',"<td><b>");
              RPT_STRING_REPLACE(pRpt->szWorkString,'|',"&nbsp;</td><td> ");
              RPT_STRING_REPLACE(pRpt->szWorkString,'@',"&nbsp;</b></td>");
              strcat(szLine,pRpt->szWorkString);
              strcat(szLine,"</tr>\n");
            }
            // standard row
            else
            {
              strcpy(szLine,"<tr align=right bgcolor=white>");
              RPT_STRING_REPLACE(pRpt->szWorkString,'\xF5',"<td><b>");
              RPT_STRING_REPLACE(pRpt->szWorkString,'|',"&nbsp;</td><td> ");
              RPT_STRING_REPLACE(pRpt->szWorkString,'@',"&nbsp;</b></td>");
              strcat(szLine,pRpt->szWorkString);
              strcat(szLine,"</tr>\n");
            } /* endif */

            strcpy(pRpt->szWorkString,szLine);

          } /* endif */

          iMax = strlen(pRpt->szWorkString);
          for ( i=0 ; i<iMax; i++ )
          {
            BYTE bTemp = (BYTE)pRpt->szWorkString[i];
            if ( bTemp < 5 )
            {
              pRpt->szWorkString[i]=' ';
            }
            else if ( bTemp == 21 )
            {
              pRpt->szWorkString[i]=',';
            } /* endif */

          } /* endfor */

          usFileRc = UtlWrite (  hFile,
                                 pRpt->szWorkString,
                                 (USHORT)strlen (pRpt->szWorkString),
                                 &usBytesWrite,
                                 FALSE );

        } //end if BEGIN_TABLE,END_TABLE

        if ( usFileRc != NO_ERROR )
        {
          pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
          UtlClose (pRpt->hfOutputFileHandle, NOMSG);     // close output file
          pRpt->hfOutputFileHandle = NULLHANDLE;          // set file handle to NULL
          fOk = FALSE;
        } // end if
      } // end while


      break; // end case

    case CLOSE_OUTPUT_FILE:  // close output file
      //----------------------------------------------
      if ( pRpt->hfOutputFileHandle )
      {



        // close output file
        usFileRc = UtlClose (pRpt->hfOutputFileHandle, NOMSG);

        if ( usFileRc != NO_ERROR )
        {
          pRpt->usOutputFileProcess = ERROR_OUTPUT_FILE;  // set process to error
          fOk = FALSE;
        } // end if
        else
        {
          pRpt->hfOutputFileHandle = NULLHANDLE;
        }
      } // end if
      break; // end case

    default:
      break;
  } // end switch

  return fOk;
} // end of RptOutputToRTFFile




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptFreeMemory                                             |
//+----------------------------------------------------------------------------+
//|Function call:    RptFreeMemory (pRpt)                                      |
//+----------------------------------------------------------------------------+
//|Description:      This function frees all allocated memory used by          |
//|                  building counting report                                  |
//+----------------------------------------------------------------------------+
//|Parameters:       PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  VOID                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    if PALLINFO                                               |
//|                    free ALLINFOs and PALLINFO array                        |
//|                  if PCALCINFO                                              |
//|                    free CALCINFOs and PCALCINFO array                      |
//|                  if POUTMRI                                                |
//|                    free OUTMRI array                                       |
//|                  if POUTPUT                                                |
//|                    free OUTPUT array                                       |
//+----------------------------------------------------------------------------+

VOID RptFreeMemory (PRPT pRpt)
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

  // free OUTPUT of pOutputField
  if ( pRpt->pOutputField )
  {
    // free OUTPUT array
    UtlAlloc ((PVOID*)&(pRpt->pOutputField), 0L, 0L, NOMSG);
  } // end if
} // end of RptFreeMemory


//+----------------------------------------------------------------------------+
//|Function name: RptAbbrevFileName                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

static VOID RptAbbrevFileName
(
PSZ      pszAbbrFile,
USHORT   usMax,
PSZ      pszInFile
)
{
  USHORT           usInFileLen;
  CHAR             chStop;
  PSZ              pszEnd;
  USHORT           usLen;
  USHORT           usI = 0;
  USHORT           usBack = 0;
  USHORT           uCut;


  usInFileLen = (USHORT)strlen(pszInFile);
  if ( usInFileLen>usMax )
  {

    // Estimated Cut Value
    if ( usMax<17 )
    {
      uCut = (USHORT) (0.3 * usMax);
    }
    else if ( usMax<30 )
    {
      uCut = (USHORT) (0.4 * usMax);
    }
    else
    {
      uCut = (USHORT) (0.5 * usMax);
    } /* endif */

    // check with Backslash number
    while ( usI<uCut && (usBack < 2) )
    {
      if ( *(pszInFile+usI ) == BACKSLASH )
      {
        usBack ++;
      } /* endif */
      usI++;
    } /* endwhile */
    chStop = pszInFile[usI];
    pszInFile[usI] = EOS;

    // remaining file name
    if ( usMax < 17 )
    {
      usLen = usInFileLen-(usMax-(usI+1));
    }
    else
    {
      usLen = usInFileLen-(usMax-(usI+3));
    } /* endif */

    pszEnd = pszInFile + usLen;

    if ( usMax < 17 )
    {
      sprintf( pszAbbrFile, "%s~%s", pszInFile,pszEnd);
    }
    else
    {
      sprintf( pszAbbrFile, "%s...%s", pszInFile,pszEnd);
    } /* end if */

    // recover pszInFile
    pszInFile[usI] = chStop;


  } /* endif */

  return ;


} /* end of function RptAbbrevFileName */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:  ShortenLongName                                             |
//|                  TC Summary Calculating report                             |
//+----------------------------------------------------------------------------+
//|Function call:  ShortenLongName(nLength,pszFileName)                        |
//+----------------------------------------------------------------------------+
//|Description:    cuts the string pszFileName if longer than nLength          |
//|                puts ... at end of the FileName                             |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//+----------------------------------------------------------------------------+

BOOL ShortenLongName(int nLength, PSZ pszFileName)
{
  int iNameLen = strlen(pszFileName);
  if ( iNameLen > nLength )
  {

    pszFileName[nLength]   = EOS;
    pszFileName[nLength-1] = '.';
    pszFileName[nLength-2] = '.';
    pszFileName[nLength-3] = '.';
    return TRUE;
  }
  else
  {
    return FALSE;
  } /* endif strlen */



}/* end of function ShortenLongName */




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptReport1   (HISTORY REPORT)                             |
//+----------------------------------------------------------------------------+
//|Function call:    RptReport1 (hwnd, pRpt)                                   |
//+----------------------------------------------------------------------------+
//|Description:      This function build and put the main body of report 1     |
//|                  to LB of list window and if selected to file              |
//+----------------------------------------------------------------------------+
//|Parameters:       HWND hwnd   handle to listbox of list window              |
//|                  PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    switch option                                             |
//|                    case BRIEF_SORT_BY_DATE:                                |
//|                         BRIEF_SORT_BY_DOCUMENT:                            |
//|                      build header, output to file and/or screen            |
//|                      loop over all ALLINFOs                                |
//|                        build output line of fixed part HISTLOGRECORD       |
//|                        output to file and/or screen                        |
//|                    case DETAIL:                                            |
//|                      build header, output to file and/or screen            |
//|                      loop over all ALLINFOs                                |
//|                        build output line of fixed part HISTLOGRECORD       |
//|                        build output lines of variable part VARPART         |
//|                         depending on process task                          |
//|                        output to file and/or screen                        |
//+----------------------------------------------------------------------------+

BOOL RptReport1 (HWND hwnd, PRPT pRpt)
{
  BOOL           fOk = FALSE;         // error indicator
  PPALLINFO      ppAllInfoFieldTmp;   // pointer to PALLINFOs
  POUTPUT        pOutputField;        // pointer to OUTPUT array
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
  SHORT          sImpExpSubType = 0;



  CHAR  szLongFileName[MAX_LONGPATH]; // long filename
  PSZ   pszLongFileName=NULL; // long filename
  CHAR  szLongFileNameCopy[MAX_LONGPATH]; // long filename
  CHAR  szPrevPath[MAX_LONGPATH]; // long filename
  BOOL  fNewPath = FALSE;

  pRpt->usStringIndex = 0;            // index for output field
  ppAllInfoFieldTmp = pRpt->ppAllInfoField;  // set tmp pointer to PALLINFOs
  pOutputField = pRpt->pOutputField;         // set pointer to OUTPUT array
  pOutputMris = pRpt->pOutputMris;           // set pointer to OUTMRI



  switch ( pRpt->usOptions )
  {
    case BRIEF_SORT_BY_DATE:     // brief, sorted by date
    case BRIEF_SORT_BY_DOCUMENT: // brief, sorted by document
      ///////////////////////////////
      // build header

      sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
               "%s",
               STR_RPT3_BEGIN_TABLE
              );

      strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine3);
      sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
               "|%7.7s  |%-20.20s  |%-8.8s  %-11.11s  |%-23.23s       |",
               pOutputMris->szFeld[RPT_NUMBER],
               pOutputMris->szFeld[RPT_DOCUMENT],
               pOutputMris->szFeld[RPT_DATE],
               pOutputMris->szFeld[RPT_TIME],
               pOutputMris->szFeld[RPT_PROCESSTASK]
              );
      sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
               "|%7.7s  |%-20.20s  |%-8.8s  %-11.11s  |%-23.23s       |",
               " ",
               " ",
               " ",
               " ",
               " "
              );


      if ( pRpt->usOptions == BRIEF_SORT_BY_DATE )
      {
        strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine3);
      } /* end if */

      pOutputField->usNum = pRpt->usStringIndex;  // update number of lines

      // output
      fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window

      if ( pRpt->fRptFile )
      {
        fOk = RptOutputToFile (pRpt);        // output to file
      } // end if

      // ----------------
      // build records
      // first run
      // standard report
      // ----------------

      while ( ulIndex < pRpt->ulAllInfoRecords )  // loop over all PALLINFOs
      {
        pRpt->usStringIndex = 0;        // set index of actual string
        pOutputField->usNum = 0;  // set number of output strings in RPT

        // loop over OUTPUT
        while ( ulIndex < pRpt->ulAllInfoRecords && pRpt->usStringIndex < MAX_O_LINES )
        {
          pAllInfoTmp = *ppAllInfoFieldTmp;                   // set pointer to ALLINFO
          pHistLogRecordTmp = &(pAllInfoTmp->histLogRecord);  // set pointer to HISTLOGRECORD

#ifdef LONG_NAME_IN_HISTLOG
          strcpy( szLongFileName, pAllInfoTmp->szLongName );
#else
          // fetch LongFileName
          strcpy(szProperty,pRpt->szFolderObjName);
          strcat(szProperty,BACKSLASH_STR);
          strcat(szProperty,pHistLogRecordTmp->szDocName);
          DocQueryInfo2(szProperty,
                        NULL,NULL,NULL,NULL,
                        szLongFileName,
                        NULL,NULL,FALSE);
          if ( !*szLongFileName )
          {
            strcpy(szLongFileName,pHistLogRecordTmp->szDocName);
          } /*endif*/
#endif
          strcpy(szLongFileNameCopy,szLongFileName);
          RptAbbrevFileName(szLongFileName,20,szLongFileNameCopy);

          if ( pRpt->usOptions == BRIEF_SORT_BY_DOCUMENT )
          {
            // insert blank line if documents differ
#ifdef LONG_NAME_IN_HISTLOG
            if ( stricmp (pRpt->pszActualDocument, pAllInfoTmp->szLongName ) != 0 )
            {
              // update actual document
              strcpy (pRpt->pszActualDocument, pAllInfoTmp->szLongName );
#else
            if ( strcmp (pRpt->pszActualDocument, pHistLogRecordTmp->szDocName) != 0 )
            {
              // update actual document
              strcpy (pRpt->pszActualDocument, pHistLogRecordTmp->szDocName);
#endif
              // insert blank line to seperate different documents
              //*(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
              strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine3);

              pOutputField->usNum++;  // update number of output strings
            } /* end if */
          } /* end if  BRIEF_SORT_BY_DOCUMENT */

          // convert ltime to string
          LONG2DATETIME (pHistLogRecordTmp->lTime, pRpt->szWorkString);

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "|%7lu  |%-20.20s  |%-21.21s  |%-30.30s|",
                   pAllInfoTmp->ulRecord,
                   szLongFileName,
                   pRpt->szWorkString,
                   pOutputMris->szFeld[RPT_ACTION_1 + pHistLogRecordTmp->Task - 1]
                  );

          ppAllInfoFieldTmp++;    // next PALLINFO
          ulIndex++;              // update index of PALLINFO
          pOutputField->usNum++;  // update number of output strings
        }  // end while

        if ( ulIndex == pRpt->ulAllInfoRecords )
        {

          pOutputField->usNum++;  // update number of output strings
          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%s",
                   STR_RPT3_END_TABLE
                  );
        } /* endif */


        // output
        pOutputField->usNum = pRpt->usStringIndex;
        fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window

        if ( pRpt->fRptFile )
        {
          fOk = RptOutputToFile (pRpt);        // output to file
        } // end if

      }   // end while


      // ----------------
      // build records
      // second run
      // document names
      // ----------------

      if ( pRpt->usColumns4[1] )
      {


        ppAllInfoFieldTmp = pRpt->ppAllInfoField;  // set tmp pointer to PALLINFOs
        pOutputField = pRpt->pOutputField;

        ulIndex = 0;
        pRpt->usStringIndex = 0;        // set index of actual string
        pOutputField->usNum = 0;  // set number of output strings in RPT


        // BEGIN OF HEADER
        // ---------------

        *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-20.20s",STR_RPT3_BEGIN_HEADER);

        *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-15.15s",
                "Document Index");

        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-15.15s",
                "==============");

        *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-20.20s",STR_RPT3_END_HEADER);

        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-20.20s",STR_RPT3_BEGIN_HEADER);
        //output
        fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);



        strcpy(szPrevPath,"");

        // LOOP PALLINFOs
        //---------------

        while ( ulIndex < pRpt->ulAllInfoRecords )  // loop over all PALLINFOs
        {
          pRpt->usStringIndex = 0;        // set index of actual string
          pOutputField->usNum = 0;  // set number of output strings in RPT


          // LOOP OUTPUT
          // ----------------
          while ( ulIndex < pRpt->ulAllInfoRecords && pRpt->usStringIndex < MAX_O_LINES )
          {
            pAllInfoTmp = *ppAllInfoFieldTmp;                   // set pointer to ALLINFO
            pHistLogRecordTmp = &(pAllInfoTmp->histLogRecord);  // set pointer to HISTLOGRECORD

            // fetch LongFileName
#ifdef LONG_NAME_IN_HISTLOG
           strcpy( szLongFileName, pAllInfoTmp->szLongName );
#else
            strcpy(szProperty,pRpt->szFolderObjName);
            strcat(szProperty,BACKSLASH_STR);
            strcat(szProperty,pHistLogRecordTmp->szDocName);
            DocQueryInfo2(szProperty,
                          NULL,NULL,NULL,NULL,
                          szLongFileName,
                          NULL,NULL,FALSE);
            if ( !*szLongFileName )
            {
              strcpy(szLongFileName,pHistLogRecordTmp->szDocName);
            } /*endif*/
#endif

            // only output if doc name is abbreviated
            if ( strlen(szLongFileName) > 20 )
            {

              // try to split Path information
              pszLongFileName = strrchr(szLongFileName,'\\');

              if ( pszLongFileName )
              {
                *pszLongFileName = EOS;
                pszLongFileName ++;
              } /* endif */

              if ( pszLongFileName )
              {

                fNewPath = TRUE;
                if ( strcmp(szLongFileName,szPrevPath) )
                {
                  RPT3PrintLine(pRpt,75,STR_RPT3_LINE);
                  strcpy(szPrevPath,szLongFileName);
                  // Folder (Long Path Information)
                  strcpy(szLongFileNameCopy,szLongFileName);
                  RptAbbrevFileName(szLongFileName,65,szLongFileNameCopy);
                  sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                          "%-5.5s : %-65.65s",
                          "Path ",
                          szLongFileName);

                } // end if


                // Document Name
                strcpy(szLongFileNameCopy,pszLongFileName);
                RptAbbrevFileName(szLongFileName,65,szLongFileNameCopy);
                sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                        "%-5.5ld : %-65.65s",
                        pAllInfoTmp->ulRecord,
                        pszLongFileName);
              }
              else
              {

                if ( fNewPath )
                {
                  RPT3PrintLine(pRpt,75,STR_RPT3_LINE);
                } // end if
                fNewPath = FALSE;

                // Document Name
                strcpy(szLongFileNameCopy,szLongFileName);
                RptAbbrevFileName(szLongFileName,65,szLongFileNameCopy);
                sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                        "%-5.5ld : %-65.65s",
                        pAllInfoTmp->ulRecord,
                        szLongFileName);
              } // end if


              fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);

            } // end if strlen


            ppAllInfoFieldTmp++;    // next PALLINFO
            ulIndex++;              // update index of PALLINFO

          }  // end while

          if ( ulIndex == pRpt->ulAllInfoRecords )
          {

            RPT3PrintLine(pRpt,75,STR_RPT3_LINE);
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
          } /* endif */


        }   // end while


        fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);

      } // end if Document List

      break; // end case


    case VERSION:
      //////////////////////
      // loop over all PALLINFOs
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


            // convert ltime to string
            LONG2DATETIME (pAllInfoTmp->histLogRecord.lTime, pRpt->szWorkString);


            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_BEGIN_HEADER
                    );


            // record nr., task performed
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%4lu                 : %-30.30s",
                     pAllInfoTmp->ulRecord,
                     pOutputMris->szFeld[RPT_ACTION_1 + pHistLogRecordTmp->Task - 1]
                    );

            // time
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%-20.20s :  %-21.21s ",
                     "Date",
                     pRpt->szWorkString
                    );

            strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine4);

            pVersionHistTmp = &(pVarPartTmp->VersionHist);


            if ( pVersionHistTmp->fWindows )
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       "Platform",
                       "Windows"  );
            }
            else
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       "Platform",
                       "OS/2"  );


            } // end if


            // convert ltime to string
            LONG2DATETIME (pVersionHistTmp->lEqfdllDateTime, pRpt->szWorkString);
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     "Product Date",
                     pRpt->szWorkString
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     "TM Version",
                     pVersionHistTmp->szVersionString
                    );


            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );


            //
            // consistency checking
            //

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

            *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
            *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
            *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
            pRpt->pOutputField->usNum = pRpt->usStringIndex;  // update number of output strings

            // output
            fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window

            if ( pRpt->fRptFile )
            {
              fOk =  RptOutputToFile (pRpt);       // output to file
            } // end if



            break; // end case



        } // end switch


        ppAllInfoFieldTmp++;                        // next PALLINFO
        ulIndex++;                                  // update index of PALLINFOs


      } // end while



      pRpt->usStringIndex = 0; // set index of actual string

      //*(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
      //*(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;


      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%s",STR_RPT3_BEGIN_HEADER);


      if ( fInconsistency )
      {
        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 pszStringValue_SS,
                 pRpt->szFolderObjName,
                 "Inconsistent Versions Used");
        UtlErrorHwnd( MESSAGE_RPT_INCONSISTENT_VER,
                      MB_OK, 0, (PSZ *) NULP, EQF_WARNING,hwnd );


      }
      else
      {

        if ( iWindows != -1 )
        {

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   pszStringValue_SS,
                   pRpt->szFolderObjName,
                   "Consistent Versions Used");
          UtlErrorHwnd( MESSAGE_RPT_CONSISTENT_VER,
                        MB_OK, 0, (PSZ *) NULP, EQF_WARNING,hwnd );
        }
        else
        {
          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   pszStringValue_SS,
                   pRpt->szFolderObjName,
                   "No Version Information available");


        } // end if




      } // end if

      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%s",STR_RPT3_END_HEADER);

      pRpt->pOutputField->usNum = pRpt->usStringIndex;  // update number of output strings
      // output
      fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window

      if ( pRpt->fRptFile )
      {
        fOk =  RptOutputToFile (pRpt);       // output to file
      } // end if



      break;



    case DETAIL: // detail
      //////////////////////
      // loop over all PALLINFOs
      while ( ulIndex < pRpt->ulAllInfoRecords )
      {
        pRpt->usStringIndex = 0; // set index of actual string

        pAllInfoTmp = *ppAllInfoFieldTmp;                   // set pointer to ALLINFO
        pHistLogRecordTmp = &(pAllInfoTmp->histLogRecord);  // set pointer to HISTLOGRECORD
        pVarPartTmp = &(pAllInfoTmp->variablePart);         // set pointer to VARPART

        // fetch LongFileName
#ifdef LONG_NAME_IN_HISTLOG
        strcpy( szLongFileName, pAllInfoTmp->szLongName );
#else
        strcpy(szProperty,pRpt->szFolderObjName);
        strcat(szProperty,BACKSLASH_STR);
        strcat(szProperty,pHistLogRecordTmp->szDocName);
        DocQueryInfo2(szProperty,
                      NULL,NULL,NULL,NULL,
                      szLongFileName,
                      NULL,NULL,FALSE);
        if ( !*szLongFileName )
        {
          strcpy(szLongFileName,pHistLogRecordTmp->szDocName);
        } /*endif*/
#endif
        //strcpy(szLongFileNameCopy,szLongFileName);
        //RptAbbrevFileName(&szLongFileName,65,&szLongFileNameCopy);
        //ShortenLongName(20,szLongFileName);

        // convert ltime to string
        LONG2DATETIME (pAllInfoTmp->histLogRecord.lTime, pRpt->szWorkString);


        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 "%s",
                 STR_RPT3_BEGIN_HEADER
                );

        // record nr., task performed
        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 "%4lu                 : %-30.30s",
                 pAllInfoTmp->ulRecord,
                 pOutputMris->szFeld[RPT_ACTION_1 + pHistLogRecordTmp->Task - 1]
                );

        // time
        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 "%-20.20s :  %-21.21s ",
                 "Date",
                 pRpt->szWorkString
                );


        // FILE Name
        // ---------

        // try to split Path information
        pszLongFileName = strrchr(szLongFileName,'\\');

        if ( pszLongFileName )
        {
          *pszLongFileName = EOS;
          pszLongFileName ++;
        } /* endif */


        if ( pszLongFileName )
        {
          // Folder (Long Path Information)
          strcpy(szLongFileNameCopy,szLongFileName);
          RptAbbrevFileName(szLongFileName,65,szLongFileNameCopy);
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-20.20s : %-65.65s",
                  "Long Path",
                  szLongFileName);

          // Document Name
          strcpy(szLongFileNameCopy,pszLongFileName);
          RptAbbrevFileName(szLongFileName,65,szLongFileNameCopy);
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-20.20s : %-65.65s",
                  pOutputMris->szFeld[RPT_DOCUMENT],
                  pszLongFileName);
        }
        else
        {

          // Document Name
#ifdef LONG_NAME_IN_HISTLOG
          strcpy( szLongFileName, pAllInfoTmp->szLongName );
#else
          strcpy(szProperty,pRpt->szFolderObjName);
          strcat(szProperty,BACKSLASH_STR);
          strcat(szProperty,pHistLogRecordTmp->szDocName);
          DocQueryInfo2(szProperty,
                        NULL,NULL,NULL,NULL,
                        szLongFileName,
                        NULL,NULL,FALSE);
          if ( !*szLongFileName )
          {
            strcpy(szLongFileName,pHistLogRecordTmp->szDocName);
          } /*endif*/
#endif
//        strcpy(szLongFileNameCopy,szLongFileName);
//        RptAbbrevFileName(&szLongFileName,65,&szLongFileNameCopy);
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-20.20s : %-65.65s",
                  pOutputMris->szFeld[RPT_DOCUMENT],
                  szLongFileName);
        } // end if

        strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine4);

        // build output lines depending on process task
        switch ( pHistLogRecordTmp->Task )
        {

          case DOCIMPORT_LOGTASK:
            //-----------------------
            pDocImportTmp = &(pVarPartTmp->DocImport);  // set pointer to DOCIMPORTHIST
            sTypeTmp = pDocImportTmp->sType;            // set type of import

            switch( sTypeTmp)
            {
              case INTERN_SUBTYPE: sImpExpSubType = RPT_INT_FORMAT; break;
              case EXTERN_SUBTYPE: sImpExpSubType = RPT_EXT_FORMAT; break;
              case FOLDER_SUBTYPE: sImpExpSubType = RPT_OUT_FOLDERFORMAT; break;
              case NEWSOURCE_NOTIMPORTED_SUBTYPE: sImpExpSubType = RPT_OUT_NEWSOURCENOTIMPORTED; break;
              default: sImpExpSubType = 0; break; // type unknown:
            } /* endswitch */

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_FORMAT],
                     pOutputMris->szFeld[sImpExpSubType] );

            if ( sTypeTmp == INTERN_SUBTYPE || sTypeTmp == FOLDER_SUBTYPE )
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_FOLDER],
                       pDocImportTmp->szFolder
                      );
            } // end if

            if ( sTypeTmp == EXTERN_SUBTYPE )
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                                     pszStringValue_SS,
                                     pOutputMris->szFeld[RPT_IMPORT_PATH],
                                     pDocImportTmp->szPath );

              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_DOCUMENT],
                       szLongFileNameCopy
                      );

              if ( strlen (pDocImportTmp->szMarkup) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_MARKUP],
                         pDocImportTmp->szMarkup
                        );
              } // end if

              if ( strlen (pDocImportTmp->szMemory) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_MEMORY],
                         pDocImportTmp->szMemory
                        );
              } // end if

              if ( strlen (pDocImportTmp->szSourceLang) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_SRC_LNG],
                         pDocImportTmp->szSourceLang
                        );
              } // end if

              if ( strlen (pDocImportTmp->szTargetLang) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_TGT_LNG],
                         pDocImportTmp->szTargetLang
                        );
              } // end if
            } // end if

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_SRCDOCREPL],
                     pDocImportTmp->fSourceDocReplaced ? pOutputMris->szFeld[RPT_TRUE] :
                     pOutputMris->szFeld[RPT_FALSE]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_TGTDOCREPL],
                     pDocImportTmp->fTargetDocReplaced ? pOutputMris->szFeld[RPT_TRUE] :
                     pOutputMris->szFeld[RPT_FALSE]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            break; // end case


          case DOCIMPORT_LOGTASK2: // SHIPMENT_HANDLER
            //-----------------------
            pDocImportTmp2 = &(pVarPartTmp->DocImport2);  // set pointer to DOCIMPORTHIST
            sTypeTmp = pDocImportTmp2->sType;            // set type of import
            switch( sTypeTmp)
            {
              case INTERN_SUBTYPE: sImpExpSubType = RPT_INT_FORMAT; break;
              case EXTERN_SUBTYPE: sImpExpSubType = RPT_EXT_FORMAT; break;
              case FOLDER_SUBTYPE: sImpExpSubType = RPT_OUT_FOLDERFORMAT; break;
              case NEWSOURCE_NOTIMPORTED_SUBTYPE: sImpExpSubType = RPT_OUT_NEWSOURCENOTIMPORTED; break;
              default: sImpExpSubType = 0; break; // type unknown:
            } /* endswitch */

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_FORMAT],
                     pOutputMris->szFeld[sImpExpSubType] );

            if ( sTypeTmp == INTERN_SUBTYPE || sTypeTmp == FOLDER_SUBTYPE )
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_FOLDER],
                       pDocImportTmp2->szFolder
                      );
            } // end if

            if ( sTypeTmp == EXTERN_SUBTYPE )
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                                     pszStringValue_SS,
                                     pOutputMris->szFeld[RPT_IMPORT_PATH],
                                     pDocImportTmp2->szPath );
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_DOCUMENT],
                       szLongFileName
                      );

              if ( strlen (pDocImportTmp2->szMarkup) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_MARKUP],
                         pDocImportTmp2->szMarkup
                        );
              } // end if

              if ( strlen (pDocImportTmp2->szMemory) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_MEMORY],
                         pDocImportTmp2->szMemory
                        );
              } // end if

              if ( strlen (pDocImportTmp2->szSourceLang) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_SRC_LNG],
                         pDocImportTmp2->szSourceLang
                        );
              } // end if

              if ( strlen (pDocImportTmp2->szTargetLang) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_TGT_LNG],
                         pDocImportTmp2->szTargetLang
                        );
              } // end if

              if ( strlen (pDocImportTmp2->szShipment) )  // check for empty string
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         "Shipment Number",
                         pDocImportTmp2->szShipment
                        );
              } // end if


            } // end if

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_SRCDOCREPL],
                     pDocImportTmp2->fSourceDocReplaced ? pOutputMris->szFeld[RPT_TRUE] :
                     pOutputMris->szFeld[RPT_FALSE]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_TGTDOCREPL],
                     pDocImportTmp2->fTargetDocReplaced ? pOutputMris->szFeld[RPT_TRUE] :
                     pOutputMris->szFeld[RPT_FALSE]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            break; // end case




          case DOCIMPNEWTARGET_LOGTASK:
          case DOCSAVE_LOGTASK:
          case DOCIMPNEWTARGET_LOGTASK2:
          case DOCSAVE_LOGTASK2:
          case DOCAPI_LOGTASK :
          case DOCIMPNEWTARGET_LOGTASK3:
          case DOCSAVE_LOGTASK3:
          case DOCAPI_LOGTASK3:

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            // build header
            *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_BEGIN_TABLE
                    );
            strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine1);
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     //           "                     %-12.12s%-12.12s%-12.12s%-12.12s",
                     "|           |       |%-12.12s|%-12.12s|%-12.12s|%-12.12s|",
                     pOutputMris->szFeld[RPT_SEGMENTS],
                     pOutputMris->szFeld[RPT_SOURCE],
                     pOutputMris->szFeld[RPT_MODIFIED],
                     pOutputMris->szFeld[RPT_TARGET]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     //          "                                 %-12.12s%-12.12s%-12.12s",
                     "|           |       |            |%-12.12s|%-12.12s|%-12.12s|",
                     pOutputMris->szFeld[RPT_WORDS],
                     pOutputMris->szFeld[RPT_WORDS],
                     pOutputMris->szFeld[RPT_WORDS]
                    );

            strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine1);




            if ( pHistLogRecordTmp->Task == DOCIMPNEWTARGET_LOGTASK ||
                 pHistLogRecordTmp->Task == DOCSAVE_LOGTASK )
            {

              // analysis autosubst
              RptSumOut( pRpt, &(pVarPartTmp->DocSave.AnalAutoSubst),
                         pOutputMris->szFeld[RPT_ANALYSIS],
                         pOutputMris->szFeld[RPT_AUTOSUBST],
                         TRUE );

              // edit autosubst
              RptSumOut( pRpt, &(pVarPartTmp->DocSave.EditAutoSubst),
                         pOutputMris->szFeld[RPT_EDIT],
                         pOutputMris->szFeld[RPT_AUTOSUBST],
                         TRUE );

              // exact matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave.ExactExist),
                         pOutputMris->szFeld[RPT_EXACT],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // replace matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave.ReplExist),
                         pOutputMris->szFeld[RPT_REPLACE],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // fuzzy matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave.FuzzyExist),
                         pOutputMris->szFeld[RPT_FUZZY],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // machine matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave.MachExist),
                         pOutputMris->szFeld[RPT_MACHINE],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // none matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave.NoneExist),
                         pOutputMris->szFeld[RPT_NONE],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // not translated
              RptSumOut( pRpt, &(pVarPartTmp->DocSave.NotXlated),
                         pOutputMris->szFeld[RPT_NOT],
                         pOutputMris->szFeld[RPT_TRANSLATED],
                         TRUE );


            }
            else
            {

              // analysis autosubst
              RptSumOut( pRpt, &(pVarPartTmp->DocSave2.AnalAutoSubst),
                         pOutputMris->szFeld[RPT_ANALYSIS],
                         pOutputMris->szFeld[RPT_AUTOSUBST],
                         TRUE );

              // edit autosubst
              RptSumOut( pRpt, &(pVarPartTmp->DocSave2.EditAutoSubst),
                         pOutputMris->szFeld[RPT_EDIT],
                         pOutputMris->szFeld[RPT_AUTOSUBST],
                         TRUE );

              // exact matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave2.ExactExist),
                         pOutputMris->szFeld[RPT_EXACT],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // replace matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave2.ReplExist),
                         pOutputMris->szFeld[RPT_REPLACE],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // fuzzy matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave2.FuzzyExist),
                         pOutputMris->szFeld[RPT_FUZZY],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // machine matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave2.MachExist),
                         pOutputMris->szFeld[RPT_MACHINE],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // none matches
              RptSumOut( pRpt, &(pVarPartTmp->DocSave2.NoneExist),
                         pOutputMris->szFeld[RPT_NONE],
                         pOutputMris->szFeld[RPT_MATCHES],
                         TRUE );

              // not translated
              RptSumOut( pRpt, &(pVarPartTmp->DocSave2.NotXlated),
                         pOutputMris->szFeld[RPT_NOT],
                         pOutputMris->szFeld[RPT_TRANSLATED],
                         TRUE );


            } // end if


            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_TABLE
                    );


            break;  // end case


          case ANALYSIS_LOGTASK:
            pAnalysisTmp = &(pVarPartTmp->Analysis);       // set pointer to ANALYSIS
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_ADDTOTM],
                     pAnalysisTmp->fAddToMem ? pOutputMris->szFeld[RPT_TRUE] :
                     pOutputMris->szFeld[RPT_FALSE]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_AUTOREPLACE],
                     pAnalysisTmp->fAutoReplace ? pOutputMris->szFeld[RPT_TRUE] :
                     pOutputMris->szFeld[RPT_FALSE]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_LATESTMATCH],
                     pAnalysisTmp->fLatestMatch ? pOutputMris->szFeld[RPT_TRUE] :
                     pOutputMris->szFeld[RPT_FALSE]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_AUTOJOIN],
                     pAnalysisTmp->fAutoJoin ? pOutputMris->szFeld[RPT_TRUE] :
                     pOutputMris->szFeld[RPT_FALSE]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_MEMORY],
                     pAnalysisTmp->szMemory
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_MARKUP],
                     pAnalysisTmp->szMarkup
                    );
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            break; // end case


          case AUTOMATICSUBST_LOGTASK:
          case AUTOMATICSUBST_LOGTASK3:


            if (pHistLogRecordTmp->Task == AUTOMATICSUBST_LOGTASK3)
            {
              sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-40.40s %-5.5s: % 3.3d%s",
                  pOutputMris->szFeld[RPT_FUZZY_LEVEL],
                  pOutputMris->szFeld[RPT_KL5],
                  pVarPartTmp->DocSave3.lSmallFuzzLevel/100L,"\\%");
              sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-40.40s %-5.5s: % 3.3d%s",
                  pOutputMris->szFeld[RPT_FUZZY_LEVEL],
                  pOutputMris->szFeld[RPT_KL15],
                  pVarPartTmp->DocSave3.lMediumFuzzLevel/100L,"\\%");
              sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-40.40s %-5.5s: % 3.3d%s",
                  pOutputMris->szFeld[RPT_FUZZY_LEVEL],
                  pOutputMris->szFeld[RPT_GRGL15],
                  pVarPartTmp->DocSave3.lLargeFuzzLevel/100L,"\\%");

            }
            // build header

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_BEGIN_TABLE
                    );

            strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine1);
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "|           |       |%-12.12s|%-12.12s|%-12.12s|%-12.12s|",
                     pOutputMris->szFeld[RPT_SEGMENTS],
                     pOutputMris->szFeld[RPT_SOURCE],
                     pOutputMris->szFeld[RPT_MODIFIED],
                     pOutputMris->szFeld[RPT_TARGET]
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "|           |       |            |%-12.12s|%-12.12s|%-12.12s|",
                     pOutputMris->szFeld[RPT_WORDS],
                     pOutputMris->szFeld[RPT_WORDS],
                     pOutputMris->szFeld[RPT_WORDS]
                    );

            strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine1);

            // analysis autosubst
            RptSumOut( pRpt, &(pVarPartTmp->DocSave.AnalAutoSubst),
                       pOutputMris->szFeld[RPT_ANALYSIS],
                       pOutputMris->szFeld[RPT_AUTOSUBST], FALSE );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_TABLE
                    );
            break;  // end case


          case DOCEXPORT_LOGTASK:
            pDocExportTmp = &(pVarPartTmp->DocExport);  // set pointer to DOCEXPORTHIST
            sTypeTmp = pDocExportTmp->sType;            // set export type
            switch( sTypeTmp)
            {
              case INTERN_SUBTYPE: sImpExpSubType = RPT_INT_FORMAT; break;
              case EXTERN_SUBTYPE: sImpExpSubType = RPT_EXT_FORMAT; break;
              case FOLDER_SUBTYPE: sImpExpSubType = RPT_OUT_FOLDERFORMAT; break;
              case NEWSOURCE_NOTIMPORTED_SUBTYPE: sImpExpSubType = RPT_OUT_NEWSOURCENOTIMPORTED; break;
              default: sImpExpSubType = 0; break; // type unknown:
            } /* endswitch */

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_FORMAT],
                     pOutputMris->szFeld[sImpExpSubType] );

            if ( sTypeTmp == EXTERN_SUBTYPE )
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_SRCDOCREPL],
                       pDocExportTmp->fSource ? pOutputMris->szFeld[RPT_TRUE] :
                       pOutputMris->szFeld[RPT_FALSE]
                      );

              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_TGTDOCREPL],
                       pDocExportTmp->fTarget ? pOutputMris->szFeld[RPT_TRUE] :
                       pOutputMris->szFeld[RPT_FALSE]
                      );

              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_WITHREVMARK],
                       pDocExportTmp->fRevisionMarks ? pOutputMris->szFeld[RPT_TRUE] :
                       pOutputMris->szFeld[RPT_FALSE]
                      );

              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_SNOMATCH],
                       pDocExportTmp->fSNOMATCH ? pOutputMris->szFeld[RPT_TRUE] :
                       pOutputMris->szFeld[RPT_FALSE]
                      );

              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_SRC_PATH],
                       pDocExportTmp->szSourcePath
                      );

              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_TGT_PATH],
                       pDocExportTmp->szTargetPath
                      );

              if ( pDocExportTmp->fSNOMATCH )  // check if fSNOMATCH is TRUE
              {
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_SNOMATCH_PATH],
                         pDocExportTmp->szSNOMATCH
                        );
              } // end if
            } // end if
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            break; // end case


          case DOCPROP_LOGTASK:
            pDocPropTmp = &(pVarPartTmp->DocProp);  // set pointer to DOCPROPHIST

            if ( strlen (pDocPropTmp->szMarkup) )  // check for empty string
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_MARKUP],
                       pDocPropTmp->szMarkup
                      );
            } // enf if

            if ( strlen (pDocPropTmp->szMemory) )  // check for empty string
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_MEMORY],
                       pDocPropTmp->szMemory
                      );
            } // enf if

            if ( strlen (pDocPropTmp->szSourceLang) )  // check for empty string
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_SRC_LNG],
                       pDocPropTmp->szSourceLang
                      );
            } // enf if

            if ( strlen (pDocPropTmp->szTargetLang) )  // check for empty string
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       pOutputMris->szFeld[RPT_TGT_LNG],
                       pDocPropTmp->szTargetLang
                      );
            } // enf if
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            break; // end case


          case VERSION_LOGTASK:
            pVersionHistTmp = &(pVarPartTmp->VersionHist);


            if ( pVersionHistTmp->fWindows )
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       "Platform",
                       "Windows"  );
            }
            else
            {
              sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                       pszStringValue_SS,
                       "Platform",
                       "OS/2"  );


            } // end if


            // convert ltime to string
            LONG2DATETIME (pVersionHistTmp->lEqfdllDateTime, pRpt->szWorkString);
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     "Product Date",
                     pRpt->szWorkString
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     "TM Version",
                     pVersionHistTmp->szVersionString
                    );


            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );


            break; // end case


          case FOLPROP_LOGTASK:

            pFolPropTmp = &(pVarPartTmp->FolProp);  // set pointer to FOLPROPHIST

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_DESCRIPTION],
                     pFolPropTmp->szDescription
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_EDITOR],
                     pFolPropTmp->szEditor
                    );


            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_MARKUP],
                     pFolPropTmp->szMarkup
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_MEMORY],
                     pFolPropTmp->szMemory
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_SRC_LNG],
                     pFolPropTmp->szSourceLang
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     pOutputMris->szFeld[RPT_TGT_LNG],
                     pFolPropTmp->szTargetLang
                    );

            // list of max NUM_OF_FOLDER_DICS dictionaries
            usDicIndex = 0;

            if ( strlen(pFolPropTmp->DictTable) != 0 )
            {
              szDictionary = UtlParseX15 (pFolPropTmp->DictTable, usDicIndex);

              if ( strlen (szDictionary) != 0 )
              {
                usDicIndex++;
                sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                         pszStringValue_SS,
                         pOutputMris->szFeld[RPT_DICTIONARY],
                         szDictionary
                        );


                while ( usDicIndex < NUM_OF_FOLDER_DICS )
                {
                  szDictionary = UtlParseX15 (pFolPropTmp->DictTable, usDicIndex);
                  if ( strlen (szDictionary) == 0 )
                  {
                    break;
                  } // end if

                  usDicIndex++;
                  sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                           "                                :  %s",
                           szDictionary
                          );
                } // end while
              } // end if
            } //end if
            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            break; // end case



            // SHIPMENT_HANDLER
          case FOLPROPSHIPMENT_LOGTASK:

            pFolPropTmpShipment = &(pVarPartTmp->FolPropShipment);  // set pointer to FOLPROPHIST


            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     pszStringValue_SS,
                     "Shipment",
                     pFolPropTmpShipment->szShipment
                    );

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );
            break; // end case




          default:

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                     "%s",
                     STR_RPT3_END_HEADER
                    );

            break; // end case
        } // end switch

        *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
        *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
        *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

        ppAllInfoFieldTmp++;                        // next PALLINFO
        ulIndex++;                                  // update index of PALLINFOs
        pRpt->pOutputField->usNum = pRpt->usStringIndex;  // update number of output strings

        // output
        fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window

        if ( pRpt->fRptFile )
        {
          fOk =  RptOutputToFile (pRpt);       // output to file
        } // end if

      } // end while
      break; // end case

  } // end switch

  return fOk;
} // end of RptReport1


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RptReport2    (Counting report, detailed)                 |
//+----------------------------------------------------------------------------+
//|Function call:    RptReport2 (hwnd, pRpt)                                   |
//+----------------------------------------------------------------------------+
//|Description:      This function build and put the main body of report 2     |
//|                  to LB of list window and if selected to file              |
//+----------------------------------------------------------------------------+
//|Parameters:       HWND hwnd   handle to listbox of list window              |
//|                  PRPT pRpt   pointer to report instance data               |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    loop over all CALCINFOs                                   |
//|                    build header depending on options WITH_TOTALS /         |
//|                     WITHOUT_TOTALS and in case of WITH_TOTALS summary      |
//|                     record or not                                          |
//|                    build output lines                                      |
//|                    in case of WITH_TOTALS add additional lines with totals |
//|                    output to file and/or screen                            |
//+----------------------------------------------------------------------------+

BOOL RptReport2 (HWND hwnd, PRPT pRpt)
{
  BOOL           fOk = FALSE;         // error indicator
  PPCALCINFO     ppCalcInfoFieldTmp;  // pointer to PCALCINFOs
  POUTPUT        pOutputField;        // pointer to OUTPUT array
  POUTMRI        pOutputMris;         // pointer to OUTMRI array
  ULONG          ulIndex = 0;         // index of PALLINFOs
  PCALCINFO      pCalcInfoTmp;        // pointer to CALCINFO

  CHAR  szLongFileName[MAX_LONGPATH]; // long filename
  PSZ   pszLongFileName;
  CHAR  szProperty[MAX_LONGPATH];     // properties of file
  CHAR  szLongFileNameCopy[MAX_LONGPATH]; // long filename

  pRpt->usStringIndex = 0;             // index for output field

  ppCalcInfoFieldTmp = pRpt->ppCalcInfoField;  // set tmp pointer to PCALCINFOs
  pOutputField = pRpt->pOutputField;           // set pointer to OUTPUT array
  pOutputMris = pRpt->pOutputMris;             // set pointer to OUTMRI

  // build records, loop over all PCALCINFOs
  while ( ulIndex++ < pRpt->ulCalcInfoRecords )
  {
    pRpt->usStringIndex = 0;        // set index of actual output string
    pOutputField->usNum = 0;  // set number of output strings

    pCalcInfoTmp = *ppCalcInfoFieldTmp;  // set pointer to CALCINFO

    // convert ltime to string
    LONG2DATETIME (pCalcInfoTmp->lTime, pRpt->szWorkString);

#ifdef LONG_NAME_IN_HISTLOG
    if ( pCalcInfoTmp->szLongName[0] )
    {
      strcpy( szLongFileName, pCalcInfoTmp->szLongName );
    }
    else
#endif
    {
      // fetch LongFileName
      strcpy(szProperty,pRpt->szFolderObjName);
      strcat(szProperty,BACKSLASH_STR);
      strcat(szProperty,pCalcInfoTmp->szDocument);
      DocQueryInfo2(szProperty,
                    NULL,NULL,NULL,NULL,
                    szLongFileName,
                    NULL,NULL,FALSE);
      if ( !*szLongFileName )
      {
        strcpy(szLongFileName,pCalcInfoTmp->szDocument);
      }
    }
    //strcpy(szLongFileNameCopy,szLongFileName);
    //RptAbbrevFileName(&szLongFileName,25,&szLongFileNameCopy);
    //ShortenLongName(25,szLongFileName);

    switch ( pRpt->usOptions )
    {
      case WITH_TOTALS: // with totals
        // build header, check if actual record is last record -> summary record
        if ( ulIndex < (pRpt->ulCalcInfoRecords) )
        {
          // document record

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%s",
                   STR_RPT3_BEGIN_HEADER
                  );

          // FILE Name
          // ---------

          // try to split Path information
          pszLongFileName = strrchr(szLongFileName,'\\');

          if ( pszLongFileName )
          {
            *pszLongFileName = EOS;
            pszLongFileName ++;
          } /* endif */


          if ( pszLongFileName )
          {
            // Folder (Long Path Information)
            strcpy(szLongFileNameCopy,szLongFileName);
            RptAbbrevFileName(szLongFileName,65,szLongFileNameCopy);
            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-25.25s :   %-65.65s",
                    "Long Path",
                    szLongFileName);

            // Document Name
            strcpy(szLongFileNameCopy,pszLongFileName);
            RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy );
            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-25.25s :   %-65.65s",
                    pOutputMris->szFeld[RPT_DOCUMENT],
                    pszLongFileName);
          }
          else
          {

            // Document Name
            strcpy(szLongFileNameCopy,szLongFileName);
            RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy );
            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-25.25s :   %-65.65s",
                    pOutputMris->szFeld[RPT_DOCUMENT],
                    szLongFileName);
          } // end if

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%-25.25s :   %-21.21s",
                   pOutputMris->szFeld[RPT_DATE],
                   pRpt->szWorkString
                  );

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%-25.25s :   %s",
                   pOutputMris->szFeld[RPT_LASTPROCESS],
                   pOutputMris->szFeld[RPT_ACTION_1 + pCalcInfoTmp->Task - 1]
                  );

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%s",
                   STR_RPT3_END_HEADER
                  );
          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%s",
                   STR_RPT3_BEGIN_TABLE
                  );
        }
        else
        {
          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%s",
                   STR_RPT3_BEGIN_HEADER
                  );
          // summary record
          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "---    %s    ---",
                   pOutputMris->szFeld[RPT_SUMMARY]
                  );

          *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   pszStringValue_SS,
                   pOutputMris->szFeld[RPT_DOCOFFOLDER],
                   pRpt->szFolder
                  );

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   pszStringValue_SS,
                   pOutputMris->szFeld[RPT_SUMGENERATED],
                   pRpt->szWorkString
                  );

          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%s",
                   STR_RPT3_END_HEADER
                  );
          sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                   "%s",
                   STR_RPT3_BEGIN_TABLE
                  );
        } // end if
        break; // end case


      case WITHOUT_TOTALS: // without totals
        // build header
        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 "%s",
                 STR_RPT3_BEGIN_HEADER
                );

        // FILE Name
        // ---------

        // try to split Path information
        pszLongFileName = strrchr(szLongFileName,'\\');

        if ( pszLongFileName )
        {
          *pszLongFileName = EOS;
          pszLongFileName ++;
        } /* endif */


        if ( pszLongFileName )
        {
          // Folder (Long Path Information)
          strcpy(szLongFileNameCopy,szLongFileName);
          RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy );
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-25.25s :   %-65.65s",
                  "Long Path",
                  szLongFileName);

          // Document Name
          strcpy(szLongFileNameCopy,pszLongFileName);
          RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy );
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-25.25s :   %-65.65s",
                  pOutputMris->szFeld[RPT_DOCUMENT],
                  pszLongFileName);
        }
        else
        {

          // Document Name
          strcpy(szLongFileNameCopy,szLongFileName);
          RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy );
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-25.25s :   %-65.65s",
                  pOutputMris->szFeld[RPT_DOCUMENT],
                  szLongFileName);
        } // end if


        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 "%-25.25s :   %-21.21s",
                 pOutputMris->szFeld[RPT_DATE],
                 pRpt->szWorkString
                );

        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 "%-25.25s :   %-50.50s",
                 pOutputMris->szFeld[RPT_LASTPROCESS],
                 pOutputMris->szFeld[RPT_ACTION_1 + pCalcInfoTmp->Task - 1]
                );

        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 "%s",
                 STR_RPT3_END_HEADER
                );
        sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
                 "%s",
                 STR_RPT3_BEGIN_TABLE
                );
        break; // end case

    } // end switch

    // build header
    strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine1);
    //  *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;


    sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
             "|           |       |%-12.12s|%-12.12s|%-12.12s|%-12.12s|",
             pOutputMris->szFeld[RPT_SEGMENTS],
             pOutputMris->szFeld[RPT_SOURCE],
             pOutputMris->szFeld[RPT_MODIFIED],
             pOutputMris->szFeld[RPT_TARGET]);

    sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
             "|           |       |            |%-12.12s|%-12.12s|%-12.12s|",
             pOutputMris->szFeld[RPT_WORDS],
             pOutputMris->szFeld[RPT_WORDS],
             pOutputMris->szFeld[RPT_WORDS]
            );

    strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine1);

    // output
    //*******
    pOutputField->usNum = pRpt->usStringIndex;  // update number of output strings in RPT
    fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window
    if ( pRpt->fRptFile )
    {
      // insert form feed expect last record
      if ( ulIndex < pRpt->ulCalcInfoRecords )
      {
        *(pOutputField->szFeld[--pRpt->usStringIndex]) = '\x0C';
      } // end if
      fOk =  RptOutputToFile (pRpt);  // output to file
    } // end if
    pRpt->usStringIndex = 0;  // set index of actual output string
    pOutputField->usNum = 0;  // set number of output strings
    // end of output
    //**************

    // build table

    // analysis autosubst
    RptSumOut2( pRpt, &(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst),
                pOutputMris->szFeld[RPT_ANALYSIS],
                pOutputMris->szFeld[RPT_AUTOSUBST] );

    // edit autosubst
    RptSumOut2( pRpt, &(pCalcInfoTmp->docSaveHistSum.EditAutoSubst),
                pOutputMris->szFeld[RPT_EDIT],
                pOutputMris->szFeld[RPT_AUTOSUBST] );

    // exact matches
    RptSumOut2( pRpt, &(pCalcInfoTmp->docSaveHistSum.ExactExist),
                pOutputMris->szFeld[RPT_EXACT],
                pOutputMris->szFeld[RPT_MATCHES] );

    // output
    //*******
    pOutputField->usNum = pRpt->usStringIndex;  // update number of output strings in RPT
    fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window
    if ( pRpt->fRptFile )
    {
      // insert form feed expect last record
      if ( ulIndex < pRpt->ulCalcInfoRecords )
      {
        *(pOutputField->szFeld[--pRpt->usStringIndex]) = '\x0C';
      } // end if
      fOk =  RptOutputToFile (pRpt);  // output to file
    } // end if
    pRpt->usStringIndex = 0;  // set index of actual output string
    pOutputField->usNum = 0;  // set number of output strings
    // end of output
    //**************

    // replace matches
    RptSumOut2( pRpt, &(pCalcInfoTmp->docSaveHistSum.ReplExist),
                pOutputMris->szFeld[RPT_REPLACE],
                pOutputMris->szFeld[RPT_MATCHES] );

    // fuzzy matches
    RptSumOut2( pRpt, &(pCalcInfoTmp->docSaveHistSum.FuzzyExist),
                pOutputMris->szFeld[RPT_FUZZY],
                pOutputMris->szFeld[RPT_MATCHES] );

    // machine matches
    RptSumOut2( pRpt, &(pCalcInfoTmp->docSaveHistSum.MachExist),
                pOutputMris->szFeld[RPT_MACHINE],
                pOutputMris->szFeld[RPT_MATCHES] );

    // output
    //*******
    pOutputField->usNum = pRpt->usStringIndex;  // update number of output strings in RPT
    fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window
    if ( pRpt->fRptFile )
    {
      // insert form feed expect last record
      if ( ulIndex < pRpt->ulCalcInfoRecords )
      {
        *(pOutputField->szFeld[--pRpt->usStringIndex]) = '\x0C';
      } // end if
      fOk =  RptOutputToFile (pRpt);  // output to file
    } // end if
    pRpt->usStringIndex = 0;  // set index of actual output string
    pOutputField->usNum = 0;  // set number of output strings
    // end of output
    //**************

    // none matches
    RptSumOut2( pRpt, &(pCalcInfoTmp->docSaveHistSum.NoneExist),
                pOutputMris->szFeld[RPT_NONE],
                pOutputMris->szFeld[RPT_MATCHES] );

    // not translated
    RptSumOut2( pRpt, &(pCalcInfoTmp->docSaveHistSum.NotXlated),
                pOutputMris->szFeld[RPT_NOT],
                pOutputMris->szFeld[RPT_TRANSLATED] );

    // additional summary lines, if option "with totals" is selected
    if ( pRpt->usOptions == WITH_TOTALS )
    {
      RptSumOut2( pRpt, &(pCalcInfoTmp->docCriteriaSum),
                  pOutputMris->szFeld[RPT_SUMMARY],
                  pOutputMris->szFeld[RPT_NULLSTRING] );
    } // end if


    sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
             "%s",
             STR_RPT3_END_TABLE
            );
    *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
    *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

    ppCalcInfoFieldTmp++;                 // next PCALCINFO

    // output
    //*******
    pOutputField->usNum = pRpt->usStringIndex;  // update number of output strings in RPT
    fOk = RptOutputToWindow (hwnd, pRpt);  // output to list window
    if ( pRpt->fRptFile )
    {
      // insert form feed expect last record
      if ( ulIndex < pRpt->ulCalcInfoRecords )
      {
        *(pOutputField->szFeld[--pRpt->usStringIndex]) = '\x0C';
      } // end if
      fOk =  RptOutputToFile (pRpt);  // output to file
    } // end if
    pRpt->usStringIndex = 0;  // set index of actual output string
    pOutputField->usNum = 0;  // set number of output strings
    // end of output
    //**************

  } // end while

  return fOk;
} // end of RptReport2



// Output the counts for the given criteria sum (ulSegNum Version)
BOOL RptSumOut2
(
PRPT        pRpt,                    // pointer to report instance data
PCRITERIASUMEX pSum,                 // pointer to counts
PSZ         pszFirstLine,            // string to be used in first line
PSZ         pszSecondLine            // string to be used for second line
)
{
  BOOL           fOk = TRUE;           // success indicator
  POUTPUT        pOutputField;         // pointer to OUTPUT array
  POUTMRI        pOutputMris;          // pointer to OUTMRI array

  pOutputField = pRpt->pOutputField;   // set pointer to OUTPUT array
  pOutputMris = pRpt->pOutputMris;     // set pointer to OUTMRI


  sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
           pszNumbers_SSLLLL,
           pszFirstLine,
           pOutputMris->szFeld[RPT_KL5],
           pSum->SimpleSum.ulNumSegs,
           pSum->SimpleSum.ulSrcWords,
           pSum->SimpleSum.ulModWords,
           pSum->SimpleSum.ulTgtWords
          );
  sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
           pszNumbers_SSLLLL,
           pszSecondLine,
           pOutputMris->szFeld[RPT_KL15],
           pSum->MediumSum.ulNumSegs,
           pSum->MediumSum.ulSrcWords,
           pSum->MediumSum.ulModWords,
           pSum->MediumSum.ulTgtWords
          );
  sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
           pszNumbers_SSLLLL,
           pOutputMris->szFeld[RPT_NULLSTRING],
           pOutputMris->szFeld[RPT_GRGL15],
           pSum->ComplexSum.ulNumSegs,
           pSum->ComplexSum.ulSrcWords,
           pSum->ComplexSum.ulModWords,
           pSum->ComplexSum.ulTgtWords
          );
  strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine1);


  return( fOk );

} /* end of function RptSumOut2 */

// Output the counts for the given criteria sum (ulSegNum Version)
BOOL RptSumOut
(
PRPT        pRpt,                    // pointer to report instance data
PCRITERIASUM pSum,                   // pointer to counts
PSZ         pszFirstLine,            // string to be used in first line
PSZ         pszSecondLine,           // string to be used for second line
BOOL        fIfData                  // display summary only if data available
)
{
  BOOL           fOk = TRUE;           // success indicator
  POUTPUT        pOutputField;         // pointer to OUTPUT array
  POUTMRI        pOutputMris;          // pointer to OUTMRI array

  pOutputField = pRpt->pOutputField;   // set pointer to OUTPUT array
  pOutputMris = pRpt->pOutputMris;     // set pointer to OUTMRI

  if ( pSum->SimpleSum.usNumSegs ||
       pSum->MediumSum.usNumSegs ||
       pSum->ComplexSum.usNumSegs ||
       !fIfData )
  {
    sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
             pszNumbers_SSLLLL,
             pszFirstLine,
             pOutputMris->szFeld[RPT_KL5],
             (ULONG)pSum->SimpleSum.usNumSegs,
             pSum->SimpleSum.ulSrcWords,
             pSum->SimpleSum.ulModWords,
             pSum->SimpleSum.ulTgtWords
            );
    sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
             pszNumbers_SSLLLL,
             pszSecondLine,
             pOutputMris->szFeld[RPT_KL15],
             (ULONG)pSum->MediumSum.usNumSegs,
             pSum->MediumSum.ulSrcWords,
             pSum->MediumSum.ulModWords,
             pSum->MediumSum.ulTgtWords
            );
    sprintf (pOutputField->szFeld[pRpt->usStringIndex++],
             pszNumbers_SSLLLL,
             pOutputMris->szFeld[RPT_NULLSTRING],
             pOutputMris->szFeld[RPT_GRGL15],
             (ULONG)pSum->ComplexSum.usNumSegs,
             pSum->ComplexSum.ulSrcWords,
             pSum->ComplexSum.ulModWords,
             pSum->ComplexSum.ulTgtWords
            );
    strcpy (pOutputField->szFeld[pRpt->usStringIndex++], szRptLine1);
  } /* endif */


  return( fOk );

} /* end of function RptSumOut */





//+----------------------------------------------------------------------------+
//| SUMMARY COUNTING REPORT                                                    |
//+----------------------------------------------------------------------------+
//|                  RptReport3  (summary counting report)                     |
//|                  TC Summary Calculating report                             |
//+----------------------------------------------------------------------------+
//|Function call:    RptReport3 (hwnd, pRpt)                                   |
//+----------------------------------------------------------------------------+
//|Description:      Help functions and main function for                      |
//|                  summary counting report for TCs                           |
//|                                                                            |
//+----------------------------------------------------------------------------+


//+----------------------------------------------------------------------------+
//|Internal function  RPT3ZeroRow                                              |
//+----------------------------------------------------------------------------+
//|Function name:  RPT3ZeroRow                                                 |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  zero one row of new table                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+


void  RPT3ZeroRow
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

}/* end of function RPT3ZeroRow */



//+----------------------------------------------------------------------------+
//|Internal function  RPT3CalcStatistics                                       |
//+----------------------------------------------------------------------------+
//|Function name:  RPT3CalcStatistics                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+


float RPT3CalcStatistics
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
}/* end of function RPT3CalcStatistics */



//+----------------------------------------------------------------------------+
//|Internal function    RPT3AllocCntRow                                        |
//+----------------------------------------------------------------------------+
//|Function name:    RPT3AllocCntRow                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Allocate one row of the counting structure                    |
//|                                                                            |
//+----------------------------------------------------------------------------+


BOOL RPT3AllocCntRow
(
PVOID       *pCntRow
)
{
  PCntRow   pCntAct;      // actual row
  BOOL      fOk=TRUE;     // success indicator

  // three rows, details
  // fourth row summary
  //********************

  fOk = UtlAlloc((PVOID*) &pCntAct,0L,
                 (LONG) sizeof(CntRow),ERROR_STORAGE);


  if ( fOk )
  {
    RPT3ZeroRow(pCntAct);
    pCntAct ->PNext = NULL;
    strcpy(pCntAct ->szID[0]," ");
    strcpy(pCntAct ->szID[1]," ");
    strcpy(pCntAct ->szID[2]," ");
    strcpy(pCntAct ->szID[3]," ");
  } // end if fok

  if ( ! fOk ) *pCntRow = NULL;
  else       *pCntRow = pCntAct;

  return fOk;

} /* End of RPT3AllocCntRow */


//+----------------------------------------------------------------------------+
//|Internal function   RPT3AddCntRow                                           |
//+----------------------------------------------------------------------------+
//|Function name:  RPT3AddCntRow                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Add Details (row 1..3) to summary row 4                       |
//|                                                                            |
//|              Calculate statistics                                          |
//+----------------------------------------------------------------------------+



BOOL RPT3AddCntRow
(
PCntRow    pCntRow
)
{

  BOOL      fOk=TRUE;
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

  // percent exact used
  pCntRow->iRow[3][PercentExactUsed]   = RPT3CalcStatistics(pCntRow->iRow[3][ExactUsed],pCntRow->iRow[3][ExactExist]);

  // PercentReplUsed
  pCntRow->iRow[3][PercentReplUsed]    = RPT3CalcStatistics(pCntRow->iRow[3][ReplUsed],pCntRow->iRow[3][ReplExist]);

  // PercentFuzzyUsed,
  pCntRow->iRow[3][PercentFuzzyUsed]   = RPT3CalcStatistics(pCntRow->iRow[3][FuzzyUsed],pCntRow->iRow[3][FuzzyExist]);

  // PercentFuzzyUsed_1,
  pCntRow->iRow[3][PercentFuzzyUsed_1] = RPT3CalcStatistics(pCntRow->iRow[3][FuzzyUsed_1],pCntRow->iRow[3][FuzzyExist_1]);

  // PercentFuzzyUsed_2,
  pCntRow->iRow[3][PercentFuzzyUsed_2] = RPT3CalcStatistics(pCntRow->iRow[3][FuzzyUsed_2],pCntRow->iRow[3][FuzzyExist_2]);

  // PercentFuzzyUsed_3,
  pCntRow->iRow[3][PercentFuzzyUsed_3] = RPT3CalcStatistics(pCntRow->iRow[3][FuzzyUsed_3],pCntRow->iRow[3][FuzzyExist_3]);

  // PercentMachUsed,
  pCntRow->iRow[3][PercentMachUsed]    = RPT3CalcStatistics(pCntRow->iRow[3][MachUsed],pCntRow->iRow[3][MachExist]);


  return fOk;


}/* end of function RPT3AddCntRow */


//+----------------------------------------------------------------------------+
//|Internal function    RPT3HandleCategories                                   |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3HandleCategories                                       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Output of one table entry                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+


float RPT3HandleCategories
(
PCRITERIASUMEX  pSum,
Target_Mode     mode,                  // Source,target,segments,changed
int             category               // category of details
)

{
  int iOutput;


  if ( category == simple )
  {
    if ( mode==Source )
    {
      iOutput =  (pSum->SimpleSum.ulSrcWords);
    }
    else if ( mode==Target )
    {
      iOutput =  (pSum->SimpleSum.ulTgtWords);

    }
    else if ( mode==Modified )
    {
      iOutput =  (pSum->SimpleSum.ulModWords);

    }
    else if ( mode==Segments )
    {
      iOutput =  (pSum->SimpleSum.ulNumSegs);
    }
    else
    {
      //n.a.
      iOutput =  (-100);
    }  /* end if */
  }
  else if ( category == medium )
  {
    if ( mode==Source )
    {
      iOutput =  (pSum->MediumSum.ulSrcWords);
    }
    else if ( mode==Target )
    {
      iOutput =  (pSum->MediumSum.ulTgtWords);

    }
    else if ( mode==Modified )
    {
      iOutput =  (pSum->MediumSum.ulModWords);

    }
    else if ( mode==Segments )
    {
      iOutput =  (pSum->MediumSum.ulNumSegs);
    }
    else
    {
      //n.a.
      iOutput =  (-100);
    }  /* end if */
  }
  else if ( category == complex )
  {
    if ( mode==Source )
    {
      iOutput =  (pSum->ComplexSum.ulSrcWords);
    }
    else if ( mode==Target )
    {
      iOutput =  (pSum->ComplexSum.ulTgtWords);

    }
    else if ( mode==Modified )
    {
      iOutput =  (pSum->ComplexSum.ulModWords);

    }
    else if ( mode==Segments )
    {
      iOutput =  (pSum->ComplexSum.ulNumSegs);
    }
    else
    {

      // n.a.
      iOutput =  (-100);
    }  /* end if */
  }
  else
  {
    //n.a.
    iOutput = (-100);
  }  /* end if */


  return((float)(iOutput));



} /* end of function RPT3HandleCategories */


//+----------------------------------------------------------------------------+
//|Internal function   RPT3FieldControl                                        |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3FieldControl                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Cuts a string to maxLen, ends with "." in case of trunc,      |
//|              centering mechanism                                           |
//+----------------------------------------------------------------------------+


void RPT3FieldControl (PSZ  pString, int maxLen)
{

  int i;
  int slen = strlen(pString);
  CHAR  szLongFileNameCopy[MAX_LONGPATH]; // long filename

  if ( slen>maxLen )
  {

    if ( maxLen<10 )
    {
      pString[maxLen]=EOS;
      pString[maxLen-1]='.';
    }
    else
    {
      strcpy(szLongFileNameCopy,pString);
      RptAbbrevFileName( pString, (USHORT)maxLen, szLongFileNameCopy );
    } /* endif */
  }
  else if ( slen<maxLen )
  {

    pString[maxLen]=EOS;
    pString[maxLen-1]=' ';

    for ( i=1; i<=slen; i++ )
    {
      pString[maxLen-i-1]=pString[slen-i];
    }

    for ( i=0; i<maxLen-slen-1; i++ )
    {
      pString[i]=' ';
    }

  } /* end if maxLen */

} /* end of function RPT3FieldControl */


//+----------------------------------------------------------------------------+
//|Internal function  RPT3PrintLine                                            |
//+----------------------------------------------------------------------------+
//|Function name:  RPT3PrintLine                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Print one line with user defined characters                   |
//|                                                                            |
//+----------------------------------------------------------------------------+


void  RPT3PrintLine
(
PRPT        pRpt,
int         Table_Width,
PSZ         szMarker
)
{
  int     i;
  char    szString[MAX_O_LENGTH];
  POUTPUT pOutputField;

  // init OutputField
  pOutputField = pRpt->pOutputField;

  szString[0]=EOS;
  for ( i=1;i<=Table_Width;i++ )
  {
    strcat(szString,szMarker);
  }
  sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
          "%-160.160s",
          szString);
}


//+----------------------------------------------------------------------------+
//|Internal function  RPT3TableWidth                                           |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3TableWidth                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+


int  RPT3TableWidth
(
Row_Mode    R_Mode,
int        (*pColumn_Switch)[MAX_REPORT_COLUMNS],  // switch columns on and off
USHORT      usShipmentChk
)
{
  int     Table_Width=0;
  int     i;

  // calculate Table_Width

  Table_Width=3*ROW_FIELD_LENGTH;  // Document Name + Running Number
  for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
  {
    if ( (*pColumn_Switch)[i]==1 )
    {
      Table_Width+=1+ROW_FIELD_LENGTH;
    }
  }
  Table_Width+=2;
  if ( R_Mode==Details ) Table_Width+=1+ROW_FIELD_LENGTH;

  if (usShipmentChk)
  {
    Table_Width+=16;

  } //end if



  return Table_Width;

}


//+----------------------------------------------------------------------------+
//|Internal function    RPT3PrintRow                                           |
//+----------------------------------------------------------------------------+
//|Function name:    RPT3PrintRow                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Print one row of the summary table                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


void  RPT3PrintRow
(
PRPT        pRpt,         // report
PCntRow     pCntRow,      // the row to print
Row_Mode    R_Mode,       // Details, No_Details
int         S_Mode,       // Summary for each shipment in case of Details
int        (*pColumn_Switch)[MAX_REPORT_COLUMNS]  // switch columns on and off
)
{
  int     i;
  char    szString[MAX_O_LENGTH];
  char    szNumber[MAX_LONGPATH];
  char    szShipm[2*MAX_LONGPATH];
  POUTPUT pOutputField;
  PCntRow pCntAct;
  int     Table_Width;
  int     Number_of_Cols;
  int     iStart,iEnd;

  pCntAct = pCntRow;

  if ( pCntRow==NULL ) return;

  if ( R_Mode==Details ) iStart=0;
  else                 iStart=3;

  if ( (R_Mode==Details && S_Mode!=1) )
  {
    iEnd=2;
  }
  else
  {
    iEnd=3;
  }/* end if */

  pOutputField = pRpt->pOutputField;

  Table_Width=RPT3TableWidth(R_Mode,pColumn_Switch, pRpt->usShipmentChk);


  for ( Number_of_Cols=iStart;Number_of_Cols<=iEnd;Number_of_Cols++ )
  {

    if ( Number_of_Cols==3 && R_Mode==Details )
    {
      RPT3PrintLine(pRpt,Table_Width,STR_RPT3_LINE);
    }

    // running no
    // ----------
    szString[0]=EOS;
    if ( pCntAct->lRunningNo>0  && ((Number_of_Cols==0 && R_Mode==Details) ||
                                    (Number_of_Cols==3 && R_Mode==No_Details)) )
    {
      sprintf(szShipm,"%d",pCntAct->lRunningNo);
    }
    else
    {
      strcpy(szShipm," ");
    } // end if
    strcat(szString,COLUMN_DELIMITER_STR);
    RPT3FieldControl(szShipm, ROW_FIELD_LENGTH);
    strcat(szString,szShipm);


    // print document Name
    strcpy(szShipm,pCntAct->szID[Number_of_Cols]);
    strcat(szString,COLUMN_DELIMITER_STR);
    RPT3FieldControl(szShipm, 2*ROW_FIELD_LENGTH);
    strcat(szString,szShipm);


    // print shipment (SHIPMENT_HANDLER)


    if (pRpt->usShipmentChk)
    {

      if (iStart==3 || Number_of_Cols==0)
      {
        strcpy(szShipm,pCntAct->szShipment);
      }
      else
      {
        strcpy(szShipm,"");
      }// end if

      strcat(szString,COLUMN_DELIMITER_STR);
      RPT3FieldControl(szShipm, 2*ROW_FIELD_LENGTH);
      strcat(szString,szShipm);


    }// end if


    for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
    {

      // print details, if necessary
      if ( R_Mode==Details && i==1 )
      {

        strcat(szString,COLUMN_DELIMITER_STR);
        switch ( Number_of_Cols )
        {
          case 0: strcpy(szShipm,STR_RPT3_SIMPLE);
            break;
          case 1: strcpy(szShipm,STR_RPT3_MEDIUM);
            break;
          case 2: strcpy(szShipm,STR_RPT3_COMPLEX);
            break;
          case 3: strcpy(szShipm,STR_RPT3_SUM);
            break;
        } /* end switch */

        RPT3FieldControl(szShipm, ROW_FIELD_LENGTH);
        strcat(szString,szShipm);

      } /* end if */

      if ( (*pColumn_Switch)[i]==1 )
      {
        if ( pCntAct->iRow[Number_of_Cols][i]==0.0 )
        {
          if ( i<CNT_REPORT_COLUMN )
          {
            strcpy(szNumber," ");
          }
          else
          {
            strcpy(szNumber,STR_RPT3_NULL);
          }/* end if */

          if ( i>=PercentExactUsed )
          {
            strcat(szNumber,"%");
          } /* endif */
        }
        else if ( pCntAct->iRow[Number_of_Cols][i]<0.0 )
        {
          strcpy(szNumber,STR_RPT3_NA);
        }
        else
        {
          if ((!strcmp(pCntAct->szID[0], "Complexity") || !strcmp(pCntAct->szID[0], "Pay"))
            && !strcmp(pCntAct->szID[1], "Factor"))
            sprintf(szNumber,"%1.2f",(pCntAct->iRow[Number_of_Cols][i]));
          else
            sprintf(szNumber,"%1.0f",(pCntAct->iRow[Number_of_Cols][i]));

          if ( i>=PercentExactUsed || i==FuzzyLevel )
          {
            strcat(szNumber,"%");
          } /* endif */
        }
        RPT3FieldControl(szNumber,ROW_FIELD_LENGTH);
        strcat(szString,COLUMN_DELIMITER_STR);
        strcat(szString,szNumber);
      }/* end if */
    }  /* end for */

    strcat(szString,COLUMN_DELIMITER_STR);

    sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
            "%-160.160s",
            szString);


  }/* end for */

}/* end of function RPT3PrintRow */


//+----------------------------------------------------------------------------+
//|Internal function  RPT3PrintHeader                                          |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3PrintHeader                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Print Table Header                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


void  RPT3PrintHeader
(
PRPT        pRpt,                // Report
Row_Mode    R_Mode,              // Details, No_details
int        (*pColumn_Switch)[MAX_REPORT_COLUMNS]  // switch columns on and off
)
{
  int     i;
  char    szString[MAX_O_LENGTH];
  char    szShort[30];
  POUTPUT pOutputField;
  int     Table_Width=0;

  // init OutputField
  pOutputField = pRpt->pOutputField;


  // calculate Table_Width
  Table_Width=RPT3TableWidth(R_Mode,pColumn_Switch, pRpt->usShipmentChk);

  RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);

  /**************/
  /* first row  */
  /**************/

  szString[0]=EOS;
  strcat(szString,COLUMN_DELIMITER_STR);
  sprintf(szShort,"%s"," ");
  RPT3FieldControl(szShort, ROW_FIELD_LENGTH);
  strcat(szString,szShort);


  strcat(szString,COLUMN_DELIMITER_STR);
  sprintf(szShort,"%s"," ");
  RPT3FieldControl(szShort, 2*ROW_FIELD_LENGTH);
  strcat(szString,szShort);

  // Folder shipment number (SHIPMENT_HANDLER)

  if (pRpt->usShipmentChk)
  {


    strcat(szString,COLUMN_DELIMITER_STR);
    sprintf(szShort,"%s","Folder");
    RPT3FieldControl(szShort, 2*ROW_FIELD_LENGTH);
    strcat(szString,szShort);

  } // end if

  for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
  {

    if ( R_Mode==Details && i==1 )
    {
      strcat(szString,COLUMN_DELIMITER_STR);
      sprintf(szShort,"%s"," ");
      RPT3FieldControl(szShort, ROW_FIELD_LENGTH);
      strcat(szString,szShort);
    }

    if ( (*pColumn_Switch)[i]==1 )
    {
      strcat(szString,COLUMN_DELIMITER_STR);
      sprintf(szShort,"%s",Comment_1[i]);
      RPT3FieldControl(szShort, ROW_FIELD_LENGTH);
      strcat(szString,szShort);
    } /* end if */
  }

  strcat(szString,COLUMN_DELIMITER_STR);

  sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
          "%-160.160s",
          szString);

  /***************/
  /* second row  */
  /***************/

  // running Number
  szString[0]=EOS;
  strcat(szString,COLUMN_DELIMITER_STR);
  sprintf(szShort,"%s","Doc Id");
  RPT3FieldControl(szShort, 1*ROW_FIELD_LENGTH);
  strcat(szString,szShort);


  // Document Name
  strcat(szString,COLUMN_DELIMITER_STR);
  sprintf(szShort,"%s",STR_RPT3_DOCUMENT);
  RPT3FieldControl(szShort, 2*ROW_FIELD_LENGTH);
  strcat(szString,szShort);


  // Folder shipment number (SHIPMENT_HANDLER)

  if (pRpt->usShipmentChk)
  {

    strcat(szString,COLUMN_DELIMITER_STR);
    sprintf(szShort,"%s","Shipment Nr.");
    RPT3FieldControl(szShort, 2*ROW_FIELD_LENGTH);
    strcat(szString,szShort);

  }// end if



  for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
  {

    if ( R_Mode==Details && i==1 )
    {
      strcat(szString,COLUMN_DELIMITER_STR);
      sprintf(szShort,"%s",STR_RPT3_CATEGORY);
      RPT3FieldControl(szShort, ROW_FIELD_LENGTH);
      strcat(szString,szShort);
    }

    if ( (*pColumn_Switch)[i]==1 )
    {
      strcat(szString,COLUMN_DELIMITER_STR);
      sprintf(szShort,"%s",Comment_2[i]);
      RPT3FieldControl(szShort, ROW_FIELD_LENGTH);
      strcat(szString,szShort);
    } /* end if*/

  }

  strcat(szString,COLUMN_DELIMITER_STR);

  sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
          "%-160.160s",
          szString);

  RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);


}/* end of function RPT3PrintRow */



//+----------------------------------------------------------------------------+
//|Internal function  Rpt3SuccAddRows                                          |
//+----------------------------------------------------------------------------+
//|Function name:   Rpt3SuccAddRows                                            |
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


void  Rpt3SuccAddRows
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

      // -1 means Histlog Error
      // dont count this one
      if ( p2->iRow[j][i] != -1 && i != FuzzyLevel)
      {
        p1->iRow[j][i]+=p2->iRow[j][i];
      } // end if

    }  /* end for  i*/

  } /* end for j */

  // Statistics

  for ( j=0;j<=2;j++ )
  {
    // percent exact used
    p1->iRow[j][PercentExactUsed]   = RPT3CalcStatistics(p1->iRow[j][ExactUsed],p1->iRow[j][ExactExist]);

    // PercentReplUsed
    p1->iRow[j][PercentReplUsed]    = RPT3CalcStatistics(p1->iRow[j][ReplUsed],p1->iRow[j][ReplExist]);

    // PercentFuzzyUsed
    p1->iRow[j][PercentFuzzyUsed]   = RPT3CalcStatistics(p1->iRow[j][FuzzyUsed],p1->iRow[j][FuzzyExist]);

    // PercentFuzzyUsed_1
    p1->iRow[j][PercentFuzzyUsed_1] = RPT3CalcStatistics(p1->iRow[j][FuzzyUsed_1],p1->iRow[j][FuzzyExist_1]);

    // PercentFuzzyUsed_2
    p1->iRow[j][PercentFuzzyUsed_2] = RPT3CalcStatistics(p1->iRow[j][FuzzyUsed_2],p1->iRow[j][FuzzyExist_2]);

    // PercentFuzzyUsed_3
    p1->iRow[j][PercentFuzzyUsed_3] = RPT3CalcStatistics(p1->iRow[j][FuzzyUsed_3],p1->iRow[j][FuzzyExist_3]);

    // PercentMachUsed
    p1->iRow[j][PercentMachUsed]    = RPT3CalcStatistics(p1->iRow[j][MachUsed],p1->iRow[j][MachExist]);


  } /* end for */


}/* end of function Rpt3SuccAddRows */


//+----------------------------------------------------------------------------+
//|Internal function  RPT3MultRows                                             |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3MultRows                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Mult two rows                                                 |
//|              into third one                                                |
//|              row3 = row1 * row2                                            |
//+----------------------------------------------------------------------------+


void  RPT3MultRows
(
PCntRow     pCntRow1,
PCntRow     pCntRow2,
PCntRow     pCntRow3
)
{

  PCntRow     p1;
  PCntRow     p2;
  PCntRow     p3;
  int         i,j;

  p1=pCntRow1;
  p2=pCntRow2;
  p3=pCntRow3;

  for ( j=0;j<=3;j++ )
  {

    for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
    {
      p3->iRow[j][i] = p1->iRow[j][i]*p2->iRow[j][i];

    }  /* end for */

  } /* end while */

}/* end of function RPT3MultRows */


//+----------------------------------------------------------------------------+
//|Internal function   RPT3CopyRow                                             |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3CopyRow                                                |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:     COPY   row2 into row1                                      |
//|                        row1 = row2                                         |
//+----------------------------------------------------------------------------+


void  RPT3CopyRow
(
PCntRow     pCntRow1,
PCntRow     pCntRow2
)
{
  PCntRow     p1;
  PCntRow     p2;
  int     i,j;

  p1=pCntRow1;
  p2=pCntRow2;

  for ( j=0;j<=3;j++ )
  {

    for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
    {
      p1->iRow[j][i]=p2->iRow[j][i];

    } /* end for */

  } /* end while */


}/* end of function Copy Row */


//+----------------------------------------------------------------------------+
//|Internal function   RPT3MeanFactor                                          |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3MeanFactor                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:      Mean Summary Row of                                       |
//|                  complexity resp pay factor                                |
//+----------------------------------------------------------------------------+


void  RPT3MeanFactor
(
PCntRow     pCntRow1,     //initial
PCntRow     pCntRow2,     //factor
PCntRow     pCntRow3      //result
)
{
  PCntRow   p1;
  PCntRow   p2;
  PCntRow   p3;
  int       i;

  p1=pCntRow1;
  p2=pCntRow2;
  p3=pCntRow3;


  for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
  {
    // p1->iRow[3][i]=(p1->iRow[0][i] + p1->iRow[1][i] + p1->iRow[2][i])/3.;
    //p1->iRow[3][i]=( p1->iRow[1][i]);  // second factor is the
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
//|Internal function   RPT3UpdateSummaryRows                                   |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3UpdateSummaryRows                                      |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:      Mean Summary Row of                                       |
//|                  complexity resp pay factor                                |
//+----------------------------------------------------------------------------+


void  RPT3UpdateSummaryRows
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
//|Internal function   RPT3UpdateSummaryRows2                                  |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3UpdateSummaryRows2                                     |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:      Mean Summary Row of                                       |
//|                  complexity resp pay factor                                |
//|                  for statistics                                            |
//+----------------------------------------------------------------------------+

void  RPT3UpdateSummaryRows2
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


//+----------------------------------------------------------------------------+
//|Internal function   RPT3ManageOutput                                        |
//+----------------------------------------------------------------------------+
//|Function name:   RPT3ManageOutput                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:     OUTPUT TO FILE AND WINDOW                                  |
//+----------------------------------------------------------------------------+

BOOL  RPT3ManageOutput
(
HWND           hwnd,
PRPT           pRpt,
POUTPUT        pOutputField,         // pointer to OUTPUT array
ULONG          ulIndex
)
{

  BOOL fOk = TRUE;
  pOutputField = pRpt->pOutputField;  // set pointer to OUTPUT array

  /*********************/
  /*     output        */
  /*********************/

  pOutputField->usNum = pRpt->usStringIndex; // update number of output
                                             // strings in RPT
  fOk = RptOutputToWindow (hwnd, pRpt);      // output to list window

  if ( pRpt->fRptFile )
  {
    // insert form feed except last record
    if ( ulIndex < pRpt->ulCalcInfoRecords )
    {
      *(pOutputField->szFeld[--pRpt->usStringIndex]) = '\x0C';
    } // end if


    fOk =  RptOutputToFile (pRpt);  // output to file
  } // end if


  //fOk = RptOutputToWindow (hwnd, pRpt);      // output to list window


  if ( fOk )
  {
    // zero output
    pRpt->usStringIndex = 0; // set index of actual output string
    pOutputField->usNum = 0; // set number of output strings
  } // end if fOk


  return fOk;

} // end of function RPT3ManageOutput



//+----------------------------------------------------------------------------+
//|Internal function   RptReport3                                              |
//+----------------------------------------------------------------------------+
//|Function name:    RptReport3                                                |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:  Summary Counting report for TCs                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:    loop over all CALCINFOs                                   |
//|                    build header depending on options WITH_TOTALS /         |
//|                     WITHOUT_TOTALS and in case of WITH_TOTALS summary      |
//|                     record or not                                          |
//|                    build output lines                                      |
//|                    in case of WITH_TOTALS add additional lines with totals |
//|                    output to file and/or screen                            |
//+----------------------------------------------------------------------------+

BOOL RptReport3 (HWND hwnd, PRPT pRpt)
{

  // MODE ///////   SET OPTIONS GIVEN BY DIALOG

  Target_Mode    T_Mode = (Target_Mode) pRpt->usOption1;  // source,target,segments,modified
  Row_Mode       R_Mode = (Row_Mode) pRpt->usOption2;  // Details, No_Details

  Count_Mode     C_Mode;                    // Used, Exists
  float          Pay_per_Word=pRpt->Pay_per_Standard;          // Payment per word / line/ page

  BOOL           Include_Statistics;        // Include Statistics

  int            Column_Switch[MAX_REPORT_COLUMNS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0};
  // Columns switched on/off

  BOOL           Auto_Shrink = (pRpt->usColumns[1]==1);

  PCntRow       pComplexityFactor;           // row for complexity factor
  PCntRow       pPayFactor = NULL;           // row for payment factor

  ///////////////

  float          Payable_Words = 0;   // Words to be payed in final fact sheet
  float          Total_Pay = 0;       // Words to be payed in final fact sheet
  float          Total_Standard = 0;  // Payable words per standard page/line
  float          Standard_Factor;     // factor for standard page/ standard line
  BOOL           fOk = TRUE;          // error indicator
  PPCALCINFO     ppCalcInfoFieldTmp = NULL;  // pointer to PCALCINFOs
  POUTPUT        pOutputField = NULL;        // pointer to OUTPUT array
  POUTMRI        pOutputMris = NULL;         // pointer to OUTMRI array
  ULONG          ulIndex = 0;         // index of PALLINFOs
  PCALCINFO      pCalcInfoTmp;        // pointer to CALCINFO
  CHAR  szLongFileName[MAX_LONGPATH]; // long filename
  CHAR  szPrevPath[MAX_LONGPATH];     // long pathname
  PSZ   pszLongFileName;              // long filename
  CHAR  szShortFileName[MAX_LONGPATH];// to be deleted
  CHAR  szProperty[MAX_LONGPATH];     // properties of file
  CHAR  szLongFileNameCopy[MAX_LONGPATH]; // long filename
  static CHAR  szLongFolderName[MAX_LONGPATH]; // long filename

  // Structures for all tables of a folder
  PSumCntFile    pSumCntFileFirst=NULL;
  PSumCntFile    pSumCntFileAct=NULL;
  PSumCntFile    pSumCntFileTmp;
  PSumCntFile    pSumCntFile=NULL;    // Summary of the folder
  PSumCntFile    pFactSheet=NULL;     // Table for final fact sheet

  BOOL           fFirstRow=TRUE;      // first row now

  // structures for one row
  PCntRow        pCntRow = NULL;      // allocated row
  PCntRow        pCntRow1 = NULL;     // allocated row fact sheet
  PCntRow        pCntRow2 = NULL;     // allocated row fact Sheet
  PCntRow        pCntRow3 = NULL;     // allocated row fact Sheet
  PCntRow        pCntRow4 = NULL;     // allocated row fact Sheet
  PCntRow        pCntRow5 = NULL;     // allocated row fact Sheet
  PCntRow        pCntRowAct = NULL;   // pointer to one row
  PCntRow        pCntRowTotalSum = NULL;// pointer to one row
  PCntRow        pCntRowSum = NULL;   // pointer to one row
  PCntRow        pCntRowLoop = NULL;  // pointer to one row

  CHAR           szActName[MAX_LONGPATH]=" "; // actual file-name
  int            iShipm = 0;
  char           szNumber[20];
  int            Table_Width;
  int            i,j;                   // running, index
  int            S_Mode;                // summary or not
  Table_Type     iTable_Type;
  HPROP          hpropFolder;
  PPROPFOLDER    ppropFolder;
  ULONG          ulErrorInfo;
  HAB            hab;
  LONG           lRunningNo = 0;         // No for document Identification
  BOOL           fNewPath=FALSE;         // New Long Path
  SHORT          iEndShipm=0;
  SHORT          iRunShipm;



  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////// SHIPMENT HANDLER ////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////



  if (!strcmp(pRpt->szShipmentChk, "Single Shipments"))
  {
    iEndShipm = QUERYITEMCOUNTHWND( pRpt->hwndShipmentLB ) -1 ;
  }// end if


  if (fOk)
  {

    /* prepare output */
    /******************/



    ppCalcInfoFieldTmp = pRpt->ppCalcInfoField;  // set tmp pointer to PCALCINFOs
    pOutputField = pRpt->pOutputField;           // set pointer to OUTPUT array
    pOutputMris = pRpt->pOutputMris;             // set pointer to OUTMRI

    // zero output
    pRpt->usStringIndex = 0; // set index of actual output string
    pOutputField->usNum = 0; // set number of output strings


  }


  if (iEndShipm < 0)
  {

    char    szString[MAX_O_LENGTH];


    sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
            "%-20.20s",STR_RPT3_BEGIN_HEADER);

    strcpy(szString, "No shipments defined!");

    sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
            "%-160.160s",
            szString);

    sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
            "%-20.20s",STR_RPT3_END_HEADER);


    fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);



  } //end if


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
    fNewPath=FALSE;
    ulIndex = 0;
    strcpy(szActName," ");


    if (iEndShipm > 0)
    {
      QUERYITEMTEXTHWND( pRpt->hwndShipmentLB, iRunShipm , pRpt->szShipmentChk );
    }


    // security

    if ( Pay_per_Word<0.00001 ) Pay_per_Word =  0.0;

    // SET Page Number

    PageNumber = 1;

    // SET Options according Dialog input
    // report based on actually used/copied proposals
    // or on existing proposals

    Include_Statistics = (pRpt->usColumns[2]);

    if ( pRpt->usColumns[3] == 1 )
    {
      C_Mode = Exists;
    }
    else
    {
      C_Mode = Used;
    } /* end if */


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

    fOk = RPT3AllocCntRow((PVOID*) &pComplexityFactor);

    if ( fOk )
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

    if ( fOk )
    {
      fOk = RPT3AllocCntRow((PVOID*) &pPayFactor);
    } // end if ok


    if ( fOk )
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


    if ( fOk )
    {

      if ( pRpt->usReport  == PRE_ANALYSIS_REPORT )
      {
        Column_Switch[NumberShip] =
        Column_Switch[AnalAutoSubst2] =
        Column_Switch[EditAutoSubst] =
        Column_Switch[ChangedFuzzy] = 0;

      } /* end if */

    } /* end if */

    if ( fOk )
    {

      if ( pRpt->usOption3  == Standard )  Column_Switch[AutoSubst] =
        Column_Switch[ManualMach] =
        Column_Switch[FuzzyExist] =
        Column_Switch[ChangedFuzzy] = 0;

      if ( pRpt->usOption3  == Standard )  Column_Switch[PercentExactUsed] =
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

    /* build records, loop over all PCALCINFOs */
    if ( fOk )
    {
      pRpt->usStringIndex = 0;            // index for output field
      ppCalcInfoFieldTmp = pRpt->ppCalcInfoField; // set tmp pointer to PCALCINFOs
    } // end if ok

    while ( fOk && ulIndex++ < pRpt->ulCalcInfoRecords )
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

#ifdef LONG_NAME_IN_HISTLOG
        if ( pCalcInfoTmp->szLongName[0] )
        {
          strcpy( szLongFileName, pCalcInfoTmp->szLongName );
        }
        else
#endif
        {
          strcpy(szProperty,pRpt->szFolderObjName);
          strcpy(szShortFileName,pCalcInfoTmp->szDocument);
          strcat(szProperty,BACKSLASH_STR);
          strcat(szProperty,szShortFileName);
          DocQueryInfo2(szProperty,
                        NULL,NULL,NULL,NULL,
                        szLongFileName,
                        NULL,NULL,FALSE);
          if ( !szLongFileName || !strcmp(szLongFileName,"") )
          {
            strcpy(szLongFileName,pCalcInfoTmp->szDocument);
          }
        }


        /***********************************/
        /* New document? then alloc space  */
        /***********************************/

        if ( stricmp(szActName,szLongFileName)!=0 )
        {

          iShipm=1;
          fFirstRow=TRUE;
          strcpy(szActName,szLongFileName);

          /***********************/
          /* alloc new document  */
          /***********************/

          fOk = UtlAlloc((PVOID*) &pSumCntFileTmp,0L,
                         (LONG) sizeof(SumCntFile),ERROR_STORAGE);

          if ( fOk && pSumCntFileFirst==NULL )
          {
            pSumCntFileFirst=pSumCntFileTmp;
          } /* end if */

          if ( fOk )
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

        /********************/
        /* Allocate One Row */
        /********************/

        fOk = RPT3AllocCntRow((PVOID*) &pCntRow);

        /*************************/
        /*  fill header and row  */
        /*************************/


        if ( fOk && fFirstRow )
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
        else if ( fOk )  // all other rows
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

        if ( fOk )
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

        if ( fOk )
        {

          /* LOOP CATEGORIES */

          for ( j=0;j<=2;j++ )
          {

            // adjusted fuzzy level
            pCntRowLoop->iRow[j][FuzzyLevel] = (float) pCalcInfoTmp->docFuzzyLevel[j];

            // analysis autosubst
            pCntRowLoop->iRow[j][AnalAutoSubst]  =RPT3HandleCategories
                                                  (&(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst),
                                                   T_Mode,j);

            // analysis autosubst after edit
            pCntRowLoop->iRow[j][AnalAutoSubst2]  =RPT3HandleCategories
                                                   (&(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst2),
                                                    T_Mode,j);


            // edit autosubst
            pCntRowLoop->iRow[j][EditAutoSubst]  =RPT3HandleCategories
                                                  (&(pCalcInfoTmp->docSaveHistSum.EditAutoSubst),
                                                   T_Mode,j);


            if ( C_Mode == Exists )
            {

              // Exist
              //------

              // exact matches
              pCntRowLoop->iRow[j][ExactExist]  =RPT3HandleCategories
                                                 (&(pCalcInfoTmp->docSaveHistSum.ExactExist),
                                                  T_Mode,j);

              // replace matches
              pCntRowLoop->iRow[j][ReplExist]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.ReplExist),
                                                 T_Mode,j);

              // fuzzy exists
              pCntRowLoop->iRow[j][FuzzyExist]  =RPT3HandleCategories
                                                 (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist),
                                                  T_Mode,j);

              // fuzzy exists_1
              pCntRowLoop->iRow[j][FuzzyExist_1]  =RPT3HandleCategories
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_1),
                                                    T_Mode,j);

              // fuzzy exists_2
              pCntRowLoop->iRow[j][FuzzyExist_2]  =RPT3HandleCategories
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_2),
                                                    T_Mode,j);

              // fuzzy exists_3
              pCntRowLoop->iRow[j][FuzzyExist_3]  =RPT3HandleCategories
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_3),
                                                    T_Mode,j);


              // machine matches
              pCntRowLoop->iRow[j][MachExist]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.MachExist),
                                                 T_Mode,j);


              // Used
              //-----

              // exact matches
              pCntRowLoop->iRow[j][ExactUsed]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.ExactUsed),
                                                 T_Mode,j);

              // replace matches
              pCntRowLoop->iRow[j][ReplUsed]  =RPT3HandleCategories
                                               (&(pCalcInfoTmp->docSaveHistSum.ReplUsed),
                                                T_Mode,j);

              // fuzzy exists
              pCntRowLoop->iRow[j][FuzzyUsed]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed),
                                                 T_Mode,j);

              // fuzzy exists_1
              pCntRowLoop->iRow[j][FuzzyUsed_1]  =RPT3HandleCategories
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_1),
                                                   T_Mode,j);

              // fuzzy exists_2
              pCntRowLoop->iRow[j][FuzzyUsed_2]  =RPT3HandleCategories
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_2),
                                                   T_Mode,j);

              // fuzzy exists_3
              pCntRowLoop->iRow[j][FuzzyUsed_3]  =RPT3HandleCategories
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_3),
                                                   T_Mode,j);


              // machine matches
              pCntRowLoop->iRow[j][MachUsed]  =RPT3HandleCategories
                                               (&(pCalcInfoTmp->docSaveHistSum.MachUsed),
                                                T_Mode,j);

              // none matches
              pCntRowLoop->iRow[j][NoneExist]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.NoneExist),
                                                 T_Mode,j);


            }
            else
            {

              //Exists
              //------

              // exact matches
              pCntRowLoop->iRow[j][ExactExist]  =RPT3HandleCategories
                                                 (&(pCalcInfoTmp->docSaveHistSum.ExactUsed),
                                                  T_Mode,j);

              // replace matches
              pCntRowLoop->iRow[j][ReplExist]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.ReplUsed),
                                                 T_Mode,j);

              // fuzzy exists
              pCntRowLoop->iRow[j][FuzzyExist]  =RPT3HandleCategories
                                                 (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed),
                                                  T_Mode,j);

              // fuzzy exists_1
              pCntRowLoop->iRow[j][FuzzyExist_1]  =RPT3HandleCategories
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_1),
                                                    T_Mode,j);

              // fuzzy exists_2
              pCntRowLoop->iRow[j][FuzzyExist_2]  =RPT3HandleCategories
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_2),
                                                    T_Mode,j);

              // fuzzy exists_3
              pCntRowLoop->iRow[j][FuzzyExist_3]  =RPT3HandleCategories
                                                   (&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_3),
                                                    T_Mode,j);


              // machine matches
              pCntRowLoop->iRow[j][MachExist]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.MachUsed),
                                                 T_Mode,j);


              // Used
              //-----

              // exact matches
              pCntRowLoop->iRow[j][ExactUsed]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.ExactExist),
                                                 T_Mode,j);

              // replace matches
              pCntRowLoop->iRow[j][ReplUsed]  =RPT3HandleCategories
                                               (&(pCalcInfoTmp->docSaveHistSum.ReplExist),
                                                T_Mode,j);

              // fuzzy exists
              pCntRowLoop->iRow[j][FuzzyUsed]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist),
                                                 T_Mode,j);

              // fuzzy exists_1
              pCntRowLoop->iRow[j][FuzzyUsed_1]  =RPT3HandleCategories
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_1),
                                                   T_Mode,j);

              // fuzzy exists_2
              pCntRowLoop->iRow[j][FuzzyUsed_2]  =RPT3HandleCategories
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_2),
                                                   T_Mode,j);

              // fuzzy exists_3
              pCntRowLoop->iRow[j][FuzzyUsed_3]  =RPT3HandleCategories
                                                  (&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_3),
                                                   T_Mode,j);


              // machine matches
              pCntRowLoop->iRow[j][MachUsed]  =RPT3HandleCategories
                                               (&(pCalcInfoTmp->docSaveHistSum.MachExist),
                                                T_Mode,j);

              // none matches
              pCntRowLoop->iRow[j][NoneExist]  =RPT3HandleCategories
                                                (&(pCalcInfoTmp->docSaveHistSum.NoneExist2),
                                                 T_Mode,j);


            } // *end if */


            // not translated
            pCntRowLoop->iRow[j][NotXlated]  =RPT3HandleCategories
                                              (&(pCalcInfoTmp->docSaveHistSum.NotXlated),
                                               T_Mode,j);


            // calculate summary columns
            //**************************

            // summary: Auto Subst
            pCntRowLoop->iRow[j][AutoSubst]  =RPT3HandleCategories
                                              (&(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst),
                                               T_Mode,j) + RPT3HandleCategories
                                              (&(pCalcInfoTmp->docSaveHistSum.EditAutoSubst),
                                               T_Mode,j) + RPT3HandleCategories
                                              (&(pCalcInfoTmp->docSaveHistSum.AnalAutoSubst2),
                                               T_Mode,j);

            // summary: Manual Subst
            pCntRowLoop->iRow[j][ManualMach]  =RPT3HandleCategories
                                               (&(pCalcInfoTmp->docSaveHistSum.ExactExist),
                                                T_Mode,j) + RPT3HandleCategories
                                               (&(pCalcInfoTmp->docSaveHistSum.ReplExist),
                                                T_Mode,j);


            // changed words in fuzzy
            //***********************

            if ( T_Mode==Source )
            {
              pCntRowLoop->iRow[j][ChangedFuzzy]  =RPT3HandleCategories
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

            pCntRowLoop->iRow[j][PercentExactUsed] =  RPT3CalcStatistics(RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.ExactUsed),T_Mode,j),
                                                                         RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.ExactExist),T_Mode,j));

            pCntRowLoop->iRow[j][PercentReplUsed] =  RPT3CalcStatistics(RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.ReplUsed),T_Mode,j),
                                                                        RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.ReplExist),T_Mode,j));

            pCntRowLoop->iRow[j][PercentFuzzyUsed] =  RPT3CalcStatistics(RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed),T_Mode,j),
                                                                         RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist),T_Mode,j));

            pCntRowLoop->iRow[j][PercentFuzzyUsed_1] =  RPT3CalcStatistics(RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_1),T_Mode,j),
                                                                           RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_1),T_Mode,j));

            pCntRowLoop->iRow[j][PercentFuzzyUsed_2] =  RPT3CalcStatistics(RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_2),T_Mode,j),
                                                                           RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_2),T_Mode,j));

            pCntRowLoop->iRow[j][PercentFuzzyUsed_3] =  RPT3CalcStatistics(RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.FuzzyUsed_3),T_Mode,j),
                                                                           RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.FuzzyExist_3),T_Mode,j));

            pCntRowLoop->iRow[j][PercentMachUsed] =  RPT3CalcStatistics(RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.MachUsed),T_Mode,j),
                                                                        RPT3HandleCategories(&(pCalcInfoTmp->docSaveHistSum.MachExist),T_Mode,j));



          } /* end for, next category if existent */

          // add categories to summary

          RPT3AddCntRow(pCntRowLoop) ;


          /*******************/
          /* NEXT  PCALCINFO */
          /*******************/


        } // end if fok


      } // end if shipment handler

      ppCalcInfoFieldTmp++;

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

    if ( fOk )
    {
      fOk = UtlAlloc((PVOID*) &pSumCntFile,0L,
                     (LONG) sizeof(SumCntFile),ERROR_STORAGE);
    } // end if fok


    if ( fOk )
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

    if ( fOk )
    {
      fOk = RPT3AllocCntRow((PVOID*) &pCntRowTotalSum);
    }// end if fok


    if ( fOk )
    {
      pSumCntFile->PSum = pCntRowTotalSum;
      // summary row
      RPT3ZeroRow(pCntRowTotalSum);
      strcpy(pCntRowTotalSum->szID[0],STR_RPT3_SUM);
      strcpy(pCntRowTotalSum->szID[1]," ");
      strcpy(pCntRowTotalSum->szID[2]," ");
      if ( R_Mode==Details ) strcpy(pCntRowTotalSum->szID[3]," ");
      else  strcpy(pCntRowTotalSum->szID[3],STR_RPT3_SUM);
    } // end if fok

    /**************************/
    /* LOOP ALL FILES         */
    /**************************/

    if ( fOk )
    {
      pSumCntFileTmp = pSumCntFileFirst;
    } // enf if fok

    while ( pSumCntFileTmp!=NULL  && fOk )
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

      if ( fOk )
      {
        fOk = RPT3AllocCntRow((PVOID*) &pCntRowSum);
      } /* endif */

      if ( fOk )
      {
        RPT3ZeroRow(pCntRowSum);
        strcpy(pCntRowSum->szID[0],STR_RPT3_SUM);
        strcpy(pCntRowSum->szID[1]," ");
        strcpy(pCntRowSum->szID[2]," ");
        if ( R_Mode==Details ) strcpy(pCntRowSum->szID[3]," ");
        else  strcpy(pCntRowSum->szID[3],STR_RPT3_SUM);
        pCntRow = pSumCntFileTmp->PRow;
      } // end if fok


      iShipm=0;
      while ( pCntRow!=NULL && fOk )
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
          Rpt3SuccAddRows(pCntRowSum,pCntRow);
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

      if ( fOk )
      {
        int icat;

        pCntRowSum->iRow[0][NumberShip]=(float)iShipm;
        RPT3AddCntRow(pCntRowSum);

        // summary of all files
        Rpt3SuccAddRows(pCntRowTotalSum,pCntRowSum);

        // but zero adjusted fuzzy level
        for (icat=0; icat<4; icat++)
        {
          pCntRowTotalSum->iRow[icat][FuzzyLevel] = 0L;
        }

        /***********************************/
        /* Alloc Row                       */
        /* Add Row to Summary of all files */
        /***********************************/

        fOk = RPT3AllocCntRow((PVOID*) &pCntRow);
      } // end if fOk

      if ( fOk )
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
        RPT3CopyRow(pCntRow,pCntRowSum);

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

      } // end if fOk

    } /* end while */


    // add the fth row (summary of three detail rows)

    if ( fOk )
    {
      RPT3AddCntRow(pCntRowTotalSum);
    } // end if fOK


//************************
// eliminate empty columns
// Auto Shrink
//************************


// have a look at summary row of all files of the folder

    if ( Auto_Shrink && fOk )
    {
      for ( i=0; i<MAX_REPORT_COLUMNS; i++ )
      {
        if ( pCntRowTotalSum->iRow[3][i]==0 && i != FuzzyLevel ) Column_Switch[i]=0;  // do not shrink fuzzy level
      } /* end for */

    }  /* end if */



/*****************************/
/*                           */
/*    final fact sheet       */
/*                           */
/*****************************/


    /******************************************/
    /* alloc mem for final fact sheet         */
    /******************************************/

    if ( fOk )
    {

      fOk = UtlAlloc((PVOID*) &pFactSheet,0L,
                     (LONG) sizeof(SumCntFile),ERROR_STORAGE);
    } // end if fOk

    if ( fOk )
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
    }// end if fOk

    /***********************************/
    /* Alloc Rows                      */
    /* summary of summary folder table */
    /***********************************/

    // Actual Words
    //*************

    if ( fOk )
    {
      fOk = RPT3AllocCntRow((PVOID*) &pCntRow1);
    }// end if fOk

    if ( fOk )
    {
      pFactSheet->PRow = pCntRow1;
      strcpy(pCntRow1->szID[0], STR_RPT3_ACTUAL_WORDS);
      strcpy(pCntRow1->szID[1], " ");
      strcpy(pCntRow1->szID[2], " ");
      strcpy(pCntRow1->szID[3], STR_RPT3_ACTUAL_WORDS);

      // copy row
      RPT3CopyRow(pCntRow1,pSumCntFile->PSum);
    } // end if fOk



    // insert (folder) Shipment number string
    // SHIPMENT_HANDLER

    if (pSumCntFile->PSum->szShipment[0]!=EOS)
    {
      strcpy(pCntRow1->szShipment , pSumCntFile->PSum->szShipment);

    } //end if




    // Complexity Factor
    //******************

    if ( fOk )
    {
      fOk = RPT3AllocCntRow((PVOID*) &pCntRow2);
    }

    if ( fOk )
    {
      pCntRow1->PNext = pCntRow2;
      strcpy(pCntRow2->szID[0], STR_RPT3_COMPLEXITY);
      strcpy(pCntRow2->szID[1], STR_RPT3_FACTOR);
      strcpy(pCntRow2->szID[2], " ");
      strcpy(pCntRow2->szID[3], STR_RPT3_MEAN_COMP);

      // copy row
      RPT3CopyRow(pCntRow2,pComplexityFactor);

      // build summary row of factor by means
      //RPT3MeanFactor(pCntRow2);
    }// end if fOk

    // Complexity weighted words
    //**************************

    if ( fOk )
    {
      fOk = RPT3AllocCntRow((PVOID*) &pCntRow3);
    }

    if ( fOk )
    {
      pCntRow2->PNext = pCntRow3;
      strcpy(pCntRow3->szID[0], STR_RPT3_COMPLEXITY);
      strcpy(pCntRow3->szID[1], STR_RPT3_FACTORT);
      strcpy(pCntRow3->szID[2], STR_RPT3_ACTUAL_WORDS);
      strcpy(pCntRow3->szID[3], STR_RPT3_COMPLEXITY_WORDS);

      RPT3MultRows(pCntRow1,pCntRow2,pCntRow3);

      // add the fth row (summary of three detail rows)

      RPT3AddCntRow(pCntRow3);   // Summary

      // Build Summary Rows
      RPT3UpdateSummaryRows(pCntRow3);

      // build summary row of factor by means
      RPT3MeanFactor(pCntRow1, pCntRow2, pCntRow3);



    } // end if fok

    // Pay Factor
    //************

    if ( fOk )
    {
      fOk = RPT3AllocCntRow((PVOID*) &pCntRow4);
    } // end if fOk

    if ( fOk )
    {
      pCntRow3->PNext = pCntRow4;
      strcpy(pCntRow4->szID[0], STR_RPT3_PAY);
      strcpy(pCntRow4->szID[1], STR_RPT3_FACTOR);
      strcpy(pCntRow4->szID[2], " ");
      strcpy(pCntRow4->szID[3], STR_RPT3_MEAN_PAY);

      // copy row
      RPT3CopyRow(pCntRow4,pPayFactor);

      // build summary row of factor by means
      //RPT3MeanFactor(pCntRow4);
    }

    // PayFactor weighted words
    //**************************

    if ( fOk )
    {
      fOk = RPT3AllocCntRow((PVOID*) &pCntRow5);
    }

    if ( fOk )
    {
      pFactSheet->PSum  = pCntRow5;
      strcpy(pCntRow5->szID[0], STR_RPT3_PAY);
      strcpy(pCntRow5->szID[1], STR_RPT3_FACTORT);
      strcpy(pCntRow5->szID[2], STR_RPT3_COMPLEXITY_WORDS);
      strcpy(pCntRow5->szID[3], STR_RPT3_PAY_WORDS);

      RPT3MultRows(pCntRow3,pCntRow4,pCntRow5);

      // add the fth row (summary of three detail rows)

      RPT3AddCntRow(pCntRow5);

      // Build Summary Rows
      RPT3UpdateSummaryRows(pCntRow5);

      // build means  of factors

      RPT3UpdateSummaryRows2(pCntRow2);
      RPT3UpdateSummaryRows2(pCntRow4);


      // build summary row of factor by means
      RPT3MeanFactor(pCntRow3,pCntRow4, pCntRow5);



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

    } // end if fOk




    /***********************************************************/
    /* tables                                                  */
    /*                                                         */
    /* to Output                                               */
    /*                                                         */
    /***********************************************************/

    if ( fOk )
    {
      pSumCntFileTmp = pSumCntFileFirst;
    }

    while ( fOk && pSumCntFileTmp!=NULL )
    {

      // zero output
      pRpt->usStringIndex = 0; // set index of actual output string
      pOutputField->usNum = 0; // set number of output strings

      // Type of the Table to Print
      iTable_Type =  pSumCntFileTmp->iTable;

      if ( ((iTable_Type == FACT_SHEET) &&   (pRpt->usOptions == REPORT_TOTALS_FINAL_FACT_SHEET ||
                                              pRpt->usOptions == ONLY_TOTALS_FINAL_FACT_SHEET   ||
                                              pRpt->usOptions == ONLY_FINAL_FACT_SHEET)) ||
           ((iTable_Type == TABLE) && ! (pRpt->usOptions == ONLY_TOTALS_FINAL_FACT_SHEET ||
                                         pRpt->usOptions == ONLY_FINAL_FACT_SHEET ||
                                         pRpt->usOptions == ONLY_SUMMARY )) ||
           ((iTable_Type == SUMMARY_TABLE) && ! (pRpt->usOptions == PLAIN_REPORT || pRpt->usOptions == ONLY_FINAL_FACT_SHEET))
         )
      {


        // calculate Table_Width
        Table_Width = RPT3TableWidth(R_Mode,&Column_Switch, pRpt->usShipmentChk);

        /****************/
        /* print header */
        /****************/
        // of the table including document/folder name

        // BEGIN OF HEADER
        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-20.20s",STR_RPT3_BEGIN_HEADER);

        // empty line
        *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;


        // LONG FILENAMES
        // SPLIT INTO LONGPATH AND NAME
        // ............................
        strcpy(szLongFileName,pSumCntFileTmp->szLongName);


        // try to split Path information
        pszLongFileName = strrchr(szLongFileName,'\\');

        if ( pszLongFileName )
        {
          *pszLongFileName = EOS;
          pszLongFileName ++;
        } /* endif */


        if ( pszLongFileName )
        {
          // Folder (Long Path Information)
          strcpy(szLongFileNameCopy,szLongFileName);
          RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy);
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-15.15s : %-65.65s",
                  "Long Path",
                  szLongFileName);

          // Document Name
          strcpy(szLongFileNameCopy,pszLongFileName);
          RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy);
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-15.15s : %-65.65s",
                  pOutputMris->szFeld[RPT_DOCUMENT],
                  pszLongFileName);
        }
        else
        {

          // Document Name
          strcpy(szLongFileNameCopy,szLongFileName);
          RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy);
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-15.15s : %-65.65s",
                  pOutputMris->szFeld[RPT_DOCUMENT],
                  szLongFileName);
        } // end if



        Utlstrccpy( szLongFolderName, UtlGetFnameFromPath( pSumCntFileTmp->szFolder ), DOT );
        ObjShortToLongName( szLongFolderName, szLongFolderName, FOLDER_OBJECT );
        OEMTOANSI(szLongFolderName);

        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-15.15s : %-25.25s",
                pOutputMris->szFeld[RPT_FOLDER],
                szLongFolderName);


        if ( pRpt->usReport  == PRE_ANALYSIS_REPORT )
        {

          if ( (hpropFolder =                              // open folder
                OpenProperties (pRpt->szFolderObjName, NULL,    // properties fails
                                PROP_ACCESS_READ, &ulErrorInfo)) != NULL )  // foldername set
          {

            // get pointer to folder properties and save it to ida
            ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd (hpropFolder);
            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-15.15s : %s , %s ",
                    "Memory's",
                    ppropFolder->szMemory,
                    ppropFolder->MemTbl);


            // close folder properties
            if ( hpropFolder ) CloseProperties (hpropFolder,
                                                PROP_FILE, &ulErrorInfo);

          } /* end if */
        } /* end if */

        hab = (HAB)UtlQueryULong(QL_HAB);

        // Source/Target/Segments
        LOADSTRING(hab,hResMod,SID_RPT1_OPTION1_1+T_Mode,pRpt->szWorkString);

        if ( pRpt->usReport  == REDUNDANCY_REPORT )
        {
          if ( !strcmp(szLongFileName,"Redundancies") )
          {

            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-15.15s : %-30.30s",
                    pRpt->szWorkString,
                    "Cross Document Redundancies");
          }
          else if ( !strcmp(szLongFileName,"Summary") )
          {

            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-15.15s : %-30.30s",
                    pRpt->szWorkString,
                    "All Redundancies");
          }
          else
          {
            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-15.15s : %-30.30s",
                    pRpt->szWorkString,
                    "Inner Document Redundancies");
          } /* endif */


        }
        else
        {
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-15.15s",
                  pRpt->szWorkString);

        } /* end if */


        if ( pSumCntFileTmp->fHistInconsistency )
        {
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-50.50s",
                  "WordCount data inconsistent" );

        } /* endif */


        if ( pSumCntFileTmp->fHistError )
        {
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-50.50s",
                  "WordCount data invalid: Reset forced" );

        } /* endif */


        // END OF HEADER
        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-20.20s",STR_RPT3_END_HEADER);



        // BEGIN OF TABLE
        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-20.20s",STR_RPT3_BEGIN_TABLE);

        RPT3PrintHeader (pRpt,R_Mode,&Column_Switch);



        /**************/
        /* print rows */
        /**************/

        pCntRow = pSumCntFileTmp->PRow;

        while ( fOk && pCntRow!=NULL )
        {

          S_Mode=pRpt->usColumns[0];  // previous =0
          RPT3PrintRow (pRpt, pCntRow,R_Mode,S_Mode,&Column_Switch);  //zsNumber

          // next
          pCntRow = pCntRow->PNext;

          if ( pCntRow!=NULL )
          {
            if ( R_Mode==Details )
            {
              RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
            }
            else
            {
              RPT3PrintLine(pRpt,Table_Width,STR_RPT3_LINE);
            }
          } /* end if */

          /*********************/
          /* output of each row*/
          /*********************/

          fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);

        } /* end while  each row*/


        if ( fOk )
        {
          if ( R_Mode==Details )
          {
            RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
          }
          else
          {
            RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
          } /* end if */

          pCntRow = pSumCntFileTmp->PSum;

          if ( pCntRow != NULL )
          {
            S_Mode=1;
            RPT3PrintRow (pRpt, pCntRow,R_Mode,S_Mode,&Column_Switch);  //"SUM"
            if ( pSumCntFileTmp->PNext==NULL )   RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
            else RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
          } /* endif */


          // empty line
          *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;


          /* next */
          pSumCntFileTmp = pSumCntFileTmp->PNext;

        } //end if fok


        // END OF TABLE
        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-20.20s",STR_RPT3_END_TABLE);

        /**********/
        /* output */
        /**********/


        if ( fOk )
        {
          fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);

        } // end if fok


      } //end if
      else
      {
        if ( fOk )
        {
          /* next */
          pSumCntFileTmp = pSumCntFileTmp->PNext;
        } // end if fok

      }


    } // end while


    /********************************************/
    /*                                          */
    /* final fact sheet summary                 */
    /*                                          */
    /********************************************/

    if ( pRpt->usOptions != PLAIN_REPORT   &&  pRpt->usOptions != REPORT_TOTALS && fOk
         && pRpt->usOptions != ONLY_SUMMARY )
    {
      // BEGIN OF TEXT
      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-20.20s",STR_RPT3_BEGIN_HEADER);

      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-15.15s",
              STR_RPT3_FINAL_SUMMARY);

      if ( strcmp(pRpt->szShipmentChk, "All Shipments") == 0 )
      {
        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-15.15s",
                pRpt->szShipmentChk);
      }
      else
      {
        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "Shipment %-6.6s",
                pRpt->szShipmentChk);
      } /* endif */


      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-15.15s",
              "=============");

      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-15.15s : %2.2f",
              STR_RPT3_PAY_WORDS,
              Payable_Words);


      if ( pRpt->usStandard == RPT_LINE )
      {
        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-15.15s : %2.2f",
                STR_RPT3_STANDARD_LINES ,
                Total_Standard);
      }
      else if ( pRpt->usStandard == RPT_PAGE )
      {
        sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                "%-15.15s : %2.2f",
                STR_RPT3_STANDARD_PAGES ,
                Total_Standard);
      }

      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-15.15s : %2.2f %-5.5s",
              STR_RPT3_LOCAL_CURRENCY ,
              Pay_per_Word,
              pRpt->szCurrency);

      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-15.15s : %2.2f %-5.5s",
              STR_RPT3_TOTAL_PAY,
              Total_Pay,
              pRpt->szCurrency);

      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;
      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;


      // END OF HEADER
      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-20.20s",STR_RPT3_END_HEADER);

      fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);

    }

    /**********************************************************/
    /* statistics                                             */
    /* to Output                                              */
    /**********************************************************/

    // statistics on

    if ( Include_Statistics && fOk && pRpt->usOptions != ONLY_FINAL_FACT_SHEET &&
         pRpt->usReport  != PRE_ANALYSIS_REPORT &&
         pRpt->usReport  != REDUNDANCY_REPORT )
    {

      for ( i=0; i<MAX_REPORT_COLUMNS;i++ )
      {
        Column_Switch[i] = 0;

      }/* end for */

      Column_Switch[NumberShip] = 1 ;
      Column_Switch[FuzzyLevel] = 1 ;
      Column_Switch[PercentExactUsed] =
      Column_Switch[PercentReplUsed] =
      Column_Switch[PercentFuzzyUsed] =
      Column_Switch[PercentFuzzyUsed_1] =
      Column_Switch[PercentFuzzyUsed_2] =
      Column_Switch[PercentFuzzyUsed_3] =
      Column_Switch[PercentMachUsed] =  1;

      // Enhanced Statistics
      if ( pRpt->usOption4 == ADVANCED_STAT )
      {
        Column_Switch[ExactExist] =
        Column_Switch[ReplExist] =
        Column_Switch[FuzzyExist] =
        Column_Switch[MachExist] =  1;
      } /* end if */


      pSumCntFileTmp = pSumCntFileFirst;

      while ( fOk && pSumCntFileTmp!=NULL )
      {

        // zero output
        pRpt->usStringIndex = 0; // set index of actual output string
        pOutputField->usNum = 0; // set number of output strings


        // Type of the Table to Print
        iTable_Type =  pSumCntFileTmp->iTable;

        if ( ((iTable_Type == TABLE) && ! (pRpt->usOptions == ONLY_TOTALS_FINAL_FACT_SHEET)
              && ! (pRpt->usOptions == ONLY_SUMMARY))      ||
             ((iTable_Type == SUMMARY_TABLE) && ! (pRpt->usOptions == PLAIN_REPORT))
           )
        {

          // calculate Table_Width
          Table_Width = RPT3TableWidth(R_Mode,&Column_Switch, pRpt->usShipmentChk);

          /****************/
          /* print header */
          /****************/
          // of the table including document/folder name

          // BEGIN OF HEADER
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-20.20s",STR_RPT3_BEGIN_HEADER);

          // empty line
          *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-25.25s",STR_RPT3_STATISTICS );

          strcpy(szLongFileName,pSumCntFileTmp->szLongName);
          strcpy(szLongFileNameCopy,szLongFileName);
          RptAbbrevFileName( szLongFileName, 35, szLongFileNameCopy);
          //ShortenLongName(25,szLongFileName);

          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-15.15s : %-25.25s",
                  pOutputMris->szFeld[RPT_DOCUMENT],
                  szLongFileName);

          ObjShortToLongName( pSumCntFileTmp->szFolder, pRpt->szLongFolderName, FOLDER_OBJECT );

          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-15.15s : %-25.25s",
                  pOutputMris->szFeld[RPT_FOLDER],
                  pRpt->szLongFolderName);

          // END OF HEADER
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-20.20s",STR_RPT3_END_HEADER);


          // BEGIN OF TABLE
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-20.20s",STR_RPT3_BEGIN_TABLE);

          RPT3PrintHeader (pRpt,R_Mode,&Column_Switch);


          /**************/
          /* print rows */
          /**************/

          pCntRow = pSumCntFileTmp->PRow;

          while ( fOk && pCntRow!=NULL )
          {

            S_Mode=pRpt->usColumns[0];  // previous =0
            RPT3PrintRow (pRpt, pCntRow,R_Mode,S_Mode,&Column_Switch);  //zsNumber

            // next
            pCntRow = pCntRow->PNext;

            if ( pCntRow!=NULL )
            {
              if ( R_Mode==Details )
              {
                RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
              }
              else
              {
                RPT3PrintLine(pRpt,Table_Width,STR_RPT3_LINE);
              } /* end if */
            } /* end if */


            /*********************/
            /* output of each row*/
            /*********************/

            fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);

            /*****************/
            /* end of output */
            /*****************/

          } /* end while */

          if ( fOk )
          {
            if ( R_Mode==Details )
            {
              RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
            }
            else
            {
              RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
            } /* end if */


            pCntRow = pSumCntFileTmp->PSum;
            S_Mode=1;
            RPT3PrintRow (pRpt, pCntRow,R_Mode,S_Mode,&Column_Switch);  //"SUM"



            if ( pCntRow != NULL )
            {
              if ( pSumCntFileTmp->PNext==NULL ) RPT3PrintLine(pRpt,Table_Width,STR_RPT3_LINE);
              else RPT3PrintLine(pRpt,Table_Width,STR_RPT3_HEADER);
            }/* end if */

            // empty line
            *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;


            /* next */
            pSumCntFileTmp = pSumCntFileTmp->PNext;

          } // end if fok

          // END OF TABLE
          sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                  "%-20.20s",STR_RPT3_END_TABLE);

          /**********/
          /* output */
          /**********/



          if ( fOk )
          {
            fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);
          } // end if fOk

          /*****************/
          /* end of output */
          /*****************/


        } //end if
        else
        {
          /* next */
          if ( fOk )
          {
            pSumCntFileTmp = pSumCntFileTmp->PNext;
          } // end if fOk

        } // end if

      } /* end while */

    } /* end if */





    /*************************/
    /* List of Document      */
    /* names                 */
    /*************************/

    if ( pRpt->usColumns4[1] )
    {


      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-20.20s",STR_RPT3_BEGIN_HEADER);

      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-15.15s",
              "Document Index");

      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-15.15s",
              "==============");

      *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;

      // END OF HEADER
      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-20.20s",STR_RPT3_END_HEADER);

      fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);



      if ( fOk )
      {
        pSumCntFileTmp = pSumCntFileFirst;
      }

      // zero output
      pRpt->usStringIndex = 0; // set index of actual output string
      pOutputField->usNum = 0; // set number of output strings


      // BEGIN OF HEADER
      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-20.20s",STR_RPT3_BEGIN_HEADER);

      RPT3PrintLine(pRpt,75,STR_RPT3_LINE);


      strcpy(szPrevPath,"");

      while ( fOk && pSumCntFileTmp!=NULL )
      {


        // empty line
        //*(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;


        if ( pSumCntFileTmp->lRunningNo >0 )
        {

          // LONG FILENAMES
          // SPLIT INTO LONGPATH AND NAME
          // ............................
          strcpy(szLongFileName,pSumCntFileTmp->szLongName);


          // try to split Path information
          pszLongFileName = strrchr(szLongFileName,'\\');

          if ( pszLongFileName )
          {
            *pszLongFileName = EOS;
            pszLongFileName ++;
          } /* endif */


          if ( pszLongFileName )
          {

            fNewPath = TRUE;
            if ( strcmp(szLongFileName,szPrevPath) )
            {
              RPT3PrintLine(pRpt,75,STR_RPT3_LINE);
              strcpy(szPrevPath,szLongFileName);
              // Folder (Long Path Information)
              strcpy(szLongFileNameCopy,szLongFileName);
              RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy);
              sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                      "%-5.5s : %-65.65s",
                      "Path ",
                      szLongFileName);

            } // end if


            // Document Name
            strcpy(szLongFileNameCopy,pszLongFileName);
            RptAbbrevFileName( szLongFileName,65, szLongFileNameCopy);
            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-5.5d : %-65.65s",
                    pSumCntFileTmp->lRunningNo,
                    pszLongFileName);
          }
          else
          {

            if ( fNewPath )
            {
              RPT3PrintLine(pRpt,75,STR_RPT3_LINE);
            } // end if
            fNewPath = FALSE;

            // Document Name
            strcpy(szLongFileNameCopy,szLongFileName);
            RptAbbrevFileName( szLongFileName, 65, szLongFileNameCopy);
            sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
                    "%-5.5d : %-65.65s",
                    pSumCntFileTmp->lRunningNo,
                    szLongFileName);
          } // end if


          if ( fOk )
          {
            fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);
          } // end if fOk

        } // end if

        if ( fOk )
        {
          /* next */
          pSumCntFileTmp = pSumCntFileTmp->PNext;
        } // end if fok

      } // end while

      RPT3PrintLine(pRpt,75,STR_RPT3_LINE);

      // END OF HEADER
      sprintf(pOutputField->szFeld[pRpt->usStringIndex++],
              "%-20.20s",STR_RPT3_END_HEADER);


      if ( fOk )
      {
        fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);
      } // end if fOk


    } // end if


    /*************************/
    /* Garbage               */
    /* collection            */
    /*************************/


    if ( fOk )
    {
      pSumCntFile = pSumCntFileFirst;
    }

    while ( fOk && pSumCntFile!=NULL )
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





  return fOk;

} // end of RptReport3


// 
// Redundant Segment List
//
BOOL RptReport4(HWND hwnd, PRPT pRpt){

  BOOL fOk = TRUE;

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
  ULONG          ulIndex = 0;
  POUTPUT        pOutputField;        // pointer to OUTPUT array
  PSZ            pszFolder;


  memset( &Header, 0, sizeof(Header) );
  memset( &MostUsedSeg, 0, sizeof(MostUsedSeg) );

  /*******************/
  /* Open input file */
  /*******************/
  if ( fOk )
  {

    strcpy(szInFile,pRpt->szFolderObjName);
    strcat(szInFile,"\\property\\redund.log");


    //fOk = StartBrowser();
    //
    //fOk = WinExec("start g:\\database\\ibmtrans.htm", SW_SHOW); //for 16bit apps


    hInput = fopen( szInFile, "rb" );
    if ( hInput == NULL )
    {
      // displayError message
      // GS
      pszFolder =  (pRpt->szFolder);
      fOk = FALSE;
      if ( pRpt ) pRpt->usRptStatus = RPT_KILL;
      UtlErrorHwnd (MESSAGE_RPT_NO_REDUND_AVAIL, MB_OK, 1,
                    &pszFolder, EQF_INFO, pRpt->hwndErrMsg);

      pRpt->fErrorPosted = TRUE;

    }
    else
    {
      ulLength = _filelength( _fileno(hInput) );
    } /* endif */
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

  /******************************************************************/
  /* Allocate buffer for document name table (required to resolve   */
  /* document index numbers in most used segment area)              */
  /******************************************************************/
  if ( fOk )
  {
    pDocNames = (PDOCNAME) malloc( Header.lDocuments * sizeof(DOCNAME) );
    if ( pDocNames == NULL )
    {
      fOk = FALSE;
    }
    else
    {
      memset( pDocNames, 0, Header.lDocuments * sizeof(DOCNAME) );
    } /* endif */
  } /* endif */

  /******************************************************************/
  /* loop through document table                                    */
  /******************************************************************/
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
        strcpy( pDocNames[iDoc], DocHeader.szDocName );
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

  /********************************************************************/
  /* List most used segment data                                      */
  /********************************************************************/
  if ( fOk )
  {

    pRpt->usStringIndex = 0;            // index for output field
    pOutputField = pRpt->pOutputField;  // set pointer to OUTPUT array

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

        // zero output
        pRpt->usStringIndex = 0; // set index of actual output string
        pOutputField->usNum = 0; // set number of output strings
        // read-in current entry
        // REDUND.LOG is UTF-16 encoded, so read first into a UTF-16 enabled structure,
        // then copy its contents into a "legacy" structure with szSegment as ASCII.

        if (fread((PVOID) &MostUsedSegUTF16, sizeof(MostUsedSegUTF16), 1, hInput) != 1)
        {
          fOk = FALSE;
        } // endif


        // list entry only if frequency is a above base list limit or if we show the detialed list
        if ( (pRpt->usOption[pRpt->usReport] == DETAILED_LIST) ||  (MostUsedSegUTF16.lFrequency >= 3) )
        {
          if (fOk) 
          {
            MostUsedSeg.lFrequency = MostUsedSegUTF16.lFrequency;
            Unicode2ASCII(MostUsedSegUTF16.szSegment, MostUsedSeg.szSegment, 0L);
            memcpy(MostUsedSeg.aDocs, MostUsedSegUTF16.aDocs, MAX_MOSTUSEDDOCSPERSEG * sizeof(MOSTUSEDDOCANDSEG));
          }

          // list current entry
          // print one segment with all relevant information
          //
          if ( fOk )
          {
            int i = 0;
            char szDocName[500];
            PSZ  pszDocTmp;

            sprintf (pOutputField->szFeld[pRpt->usStringIndex++], "%s", STR_RPT3_BEGIN_HEADER );

            sprintf(pOutputField->szFeld[pRpt->usStringIndex++], "Entry %ld: Frequency=%ld ", lEntry, MostUsedSeg.lFrequency );

            while ( (i < MAX_MOSTUSEDDOCSPERSEG) &&
                    (MostUsedSeg.aDocs[i].ulSegNo != 0) &&
                    i < 5 )
            {
              pszDocTmp =  (PSZ)(pDocNames + MostUsedSeg.aDocs[i].sDocIndex);

              strcpy(szDocName, pRpt->szFolderObjName );
              strcat(szDocName, BACKSLASH_STR );
              strcat(szDocName, pszDocTmp);

              sprintf(pOutputField->szFeld[pRpt->usStringIndex++], "[%d] %s : #%d", i+1, szDocName, MostUsedSeg.aDocs[i].ulSegNo );
              i++;
            } /* endwhile */

            RPT_STRING_REPLACE(MostUsedSeg.szSegment,'\n', " ");

            if ( strlen(MostUsedSeg.szSegment) >= 70 )
            {
              MostUsedSeg.szSegment[69] = EOS;
              MostUsedSeg.szSegment[68] = '.';
              MostUsedSeg.szSegment[67] = '.';
              MostUsedSeg.szSegment[66] = '.';
            } /* endif */

            sprintf(pOutputField->szFeld[pRpt->usStringIndex++], "Segment: %s ", MostUsedSeg.szSegment );

          } /* endif */

          /*********************/
          /* output of each row*/
          /*********************/
          *(pOutputField->szFeld[pRpt->usStringIndex++]) = EOS;           // empty line
          RPT3PrintLine(pRpt,80,STR_RPT3_HEADER);
          sprintf (pOutputField->szFeld[pRpt->usStringIndex++], "%s", STR_RPT3_END_HEADER );
          fOk = RPT3ManageOutput( hwnd, pRpt, pOutputField,ulIndex);

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

} /* end of function RprReport4 */



/**********************************************************************/
/* Open file for HTML output                                          */
/**********************************************************************/
BOOL RptOutputToHTMLControlOpen( PRPT pRPT ){
  BOOL fOK = TRUE;
  USHORT usAction;             // file action performed by DosOpen

  sprintf( pRPT->chHTMLControl, "%s\\RPT.HTML", pRPT->szFolderObjName );

  //fOK = ! UtlOpen( pRPT->chHTMLControl,
  //                 &pRPT->hHTMLControl,
  //                 &usAction,
  //                 0L,
  //                 FILE_NORMAL | OPEN_FLAGS_WRITE_THROUGH,
  //                 FILE_CREATE | FILE_TRUNCATE,
  //                 OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
  //                 0L,
  //                 FALSE );

  // we don't need write through and share_denynon anymore!
  fOK = ! UtlOpen( pRPT->chHTMLControl,
                   &pRPT->hHTMLControl,
                   &usAction,
                   0L,
                   FILE_NORMAL,
                   FILE_TRUNCATE | FILE_CREATE,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L,
                   TRUE );

  return fOK;}

/**********************************************************************/
/* Close HTML output file                                             */
/**********************************************************************/
VOID RptOutputToHTMLControlClose( PRPT pRPT ){

  if ( pRPT->hHTMLControl )
  {
    UtlClose( pRPT->hHTMLControl, FALSE );
    pRPT->hHTMLControl = 0;
  } /* endif */
  UtlDelete( pRPT->chHTMLControl, 0L, FALSE );
}


//+----------------------------------------------------------------------------+
//|Function name: RptTestApi                                                   |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+




VOID RptTestApi(){

  BOOL     usRC;
  CHAR     szFolder[100];              //  Folder path
  CHAR     szFileName[100];            //  LongFileName
  CHAR     szSrcLang[50];              //  Source Language out
  CHAR     szTrgLang[50];              //  Source Language out
  CHAR     szFormat[50];
  CHAR     szSeg[200];
  ULONG    ulResult = 0;
  ULONG    ulMarkUp = 0;
  APIDOCSAVEHIST  HistLogApi;           // HistLog Record structure


  // example folder, document, segment

  strcpy(szFolder,"E:\\eqf\\lisa.f00");
  strcpy(szFileName,"TEST.HTM");
  strcpy(szSeg,"<h1>Hallo, this is a test </h1>");



  // reset histlog record

  memset(&HistLogApi,0,sizeof(APIDOCSAVEHIST));



  // Get source language
  // --------------------------
  usRC =  EQFGETSOURCELANG
          (
           szFolder,       //  Folder path
           szFileName,     //  LongFileName
           szSrcLang       //  Source Language out
          );



  // Get target language
  // --------------------------
  usRC =  EQFGETTARGETLANG
          (
           szFolder,       //  Folder path
           szFileName,     //  LongFileName
           szTrgLang       //  Source Language out
          );


  // Get document format / markup
  // --------------------------
  usRC = EQFGETDOCFORMAT
         (
          szFolder,       //  Folder path
          szFileName,     //  LongFileName
          szFormat        //  Document Format out
         );


  // count words in one segment
  // --------------------------
  usRC = EQFWORDCNTPERSEG
         (
           szSeg,               // ptr to Segment
           szTrgLang,           // ptr to Language of Segment
           szFormat,            // ptr to Format
           &ulResult,            // result to be counted
           &ulMarkUp             // result for markup
         );



  // fill parts of histlog record
  // source words

  HistLogApi.ExactExist.SimpleSum.ulSrcWords = 33;
  HistLogApi.ExactUsed.SimpleSum.ulSrcWords = 33;
  HistLogApi.NoneExist.SimpleSum.ulSrcWords = 22;
  HistLogApi.NoneExist.MediumSum.ulSrcWords = 23;
  HistLogApi.NoneExist.ComplexSum.ulSrcWords = 24;


  // fill parts of histlog record
  // segments words

  HistLogApi.ExactExist.SimpleSum.usNumSegs = 33;
  HistLogApi.ExactUsed.SimpleSum.usNumSegs = 33;
  HistLogApi.NoneExist.SimpleSum.usNumSegs = 22;
  HistLogApi.NoneExist.MediumSum.usNumSegs = 23;
  HistLogApi.NoneExist.ComplexSum.usNumSegs = 24;


  // Write the filled counting information to Histlog.dat
  // ----------------------------------------------------
  usRC = EQFWRITEHISTLOG
         (
          szFolder,    // folder object name
          szFileName,  // name of document
          &HistLogApi   // HistLog Record structure
         );


} /* end of function RptTestApi */




// functions for logging

void RptCheckLogging()
{
  char szTriggerFile[MAX_EQF_PATH];

  UtlMakeEQFPath( szTriggerFile, NULC, SYSTEM_PATH, NULL );
  strcat( szTriggerFile, BACKSLASH_STR );
  strcat( szTriggerFile, RPTLOGTRIGGER );

  fRptLogging = UtlFileExist( szTriggerFile );
} /* end of function RptCheckLogging */

void RptLogStart()
{
  RptCheckLogging();

  if ( fRptLogging )
  {
    hRptLog = fopen( RPTLOGFILE, "a" );
    if ( hRptLog != NULLHANDLE )
    {
      char szDate[20], szTime[20];
      LONG lTime;
      UtlTime( &lTime );
      UtlLongToDateString( lTime, szDate, sizeof(szDate)-1 );
      UtlLongToTimeString( lTime, szTime, sizeof(szTime)-1 );
      fprintf( hRptLog, "***** CalcRep Log %s %s *****\n\n", szDate, szTime );
    }
    else
    {
      fRptLogging = FALSE; // no logging possible, open of log file failed
    } /* endif */
  } /* endif */
} /* end of function RptLogStart */

void RptLogEnd()
{
  if ( fRptLogging && (hRptLog != NULLHANDLE) )
  {
    fprintf( hRptLog, "***** End of Log *****\n" );
    fclose( hRptLog );
    hRptLog = NULLHANDLE;
  } /* endif */
} /* end of function RptLogEnd */

void RptLogString( PSZ pszString )
{
  if ( fRptLogging )
  {
    fprintf( hRptLog, "%s\n", pszString );
  } /* endif */
} /* end of function RptLogString */

void RptLog2String( PSZ pszString1, PSZ pszString2 )
{
  if ( fRptLogging )
  {
    fprintf( hRptLog, "%s %s\n", pszString1, pszString2 );
  } /* endif */
} /* end of function RptLog2String */

void RptLogCalcInfo( PSZ pszString, PCALCINFO pCalcInfo )
{
  if ( fRptLogging )
  {
    static char szSums[1024];

    szSums[0] = EOS;

    RptLogBuildSumEx( szSums, "AnaSubst",    &(pCalcInfo->docSaveHistSum.AnalAutoSubst) );
    RptLogBuildSumEx( szSums, "AnaSubst2",   &(pCalcInfo->docSaveHistSum.AnalAutoSubst2) );
    RptLogBuildSumEx( szSums, "EditSubst",   &(pCalcInfo->docSaveHistSum.EditAutoSubst) );
    RptLogBuildSumEx( szSums, "ExactExist",  &(pCalcInfo->docSaveHistSum.ExactExist) );
    RptLogBuildSumEx( szSums, "ExactUsed",   &(pCalcInfo->docSaveHistSum.ExactUsed) );
    RptLogBuildSumEx( szSums, "ReplExist",   &(pCalcInfo->docSaveHistSum.ReplExist) );
    RptLogBuildSumEx( szSums, "ReplUsed",    &(pCalcInfo->docSaveHistSum.ReplUsed) );
    RptLogBuildSumEx( szSums, "FuzzyExist",  &(pCalcInfo->docSaveHistSum.FuzzyExist) );
    RptLogBuildSumEx( szSums, "FuzzyUsed",   &(pCalcInfo->docSaveHistSum.FuzzyUsed) );
    RptLogBuildSumEx( szSums, "FuzzyExist1", &(pCalcInfo->docSaveHistSum.FuzzyExist_1) );
    RptLogBuildSumEx( szSums, "FuzzyUsed1",  &(pCalcInfo->docSaveHistSum.FuzzyUsed_1) );
    RptLogBuildSumEx( szSums, "FuzzyExist2", &(pCalcInfo->docSaveHistSum.FuzzyExist_2) );
    RptLogBuildSumEx( szSums, "FuzzyUsed2",  &(pCalcInfo->docSaveHistSum.FuzzyUsed_2) );
    RptLogBuildSumEx( szSums, "FuzzyExist3", &(pCalcInfo->docSaveHistSum.FuzzyExist_3) );
    RptLogBuildSumEx( szSums, "FuzzyUsed3",  &(pCalcInfo->docSaveHistSum.FuzzyUsed_3) );
    RptLogBuildSumEx( szSums, "MachExist",   &(pCalcInfo->docSaveHistSum.MachExist) );
    RptLogBuildSumEx( szSums, "MachUsed",   &(pCalcInfo->docSaveHistSum.MachUsed) );
    RptLogBuildSumEx( szSums, "NoneExist",   &(pCalcInfo->docSaveHistSum.NoneExist) );
    RptLogBuildSumEx( szSums, "NoneExist2",  &(pCalcInfo->docSaveHistSum.NoneExist2) );
    RptLogBuildSumEx( szSums, "NoneXlated",  &(pCalcInfo->docSaveHistSum.NotXlated) );

    fprintf( hRptLog, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function RptLogString */


void RptLogDocSave( PSZ pszString, PDOCSAVEHIST pDocSave )
{
  if ( fRptLogging )
  {
    static char szSums[1024];

    szSums[0] = EOS;
    RptLogBuildSum( szSums, "AnaSubst",    &(pDocSave->AnalAutoSubst) );
    RptLogBuildSum( szSums, "EditSubst",   &(pDocSave->EditAutoSubst) );
    RptLogBuildSum( szSums, "ExactExist",  &(pDocSave->ExactExist) );
    RptLogBuildSum( szSums, "ReplExist",   &(pDocSave->ReplExist) );
    RptLogBuildSum( szSums, "FuzzyExist",  &(pDocSave->FuzzyExist) );
    RptLogBuildSum( szSums, "MachExist",   &(pDocSave->MachExist) );
    RptLogBuildSum( szSums, "NoneExist",   &(pDocSave->NoneExist) );
    RptLogBuildSum( szSums, "NotXlated",   &(pDocSave->NotXlated) );

    fprintf( hRptLog, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function RptLogDocSave3 */

void RptLogDocSave2( PSZ pszString, PDOCSAVEHIST2 pDocSave2 )
{
  if ( fRptLogging )
  {
    static char szSums[1024];

    szSums[0] = EOS;
    RptLogBuildSum( szSums, "AnaSubst",    &(pDocSave2->AnalAutoSubst) );
    RptLogBuildSum( szSums, "EditSubst",   &(pDocSave2->EditAutoSubst) );
    RptLogBuildSum( szSums, "ExactExist",  &(pDocSave2->ExactExist) );
    RptLogBuildSum( szSums, "ExactUsed",   &(pDocSave2->ExactUsed) );
    RptLogBuildSum( szSums, "ReplExist",   &(pDocSave2->ReplExist) );
    RptLogBuildSum( szSums, "ReplUsed",    &(pDocSave2->ReplUsed) );
    RptLogBuildSum( szSums, "FuzzyExist",  &(pDocSave2->FuzzyExist) );
    RptLogBuildSum( szSums, "FuzzyUsed",   &(pDocSave2->FuzzyUsed) );
    RptLogBuildSum( szSums, "FuzzyExist1", &(pDocSave2->FuzzyExist_1) );
    RptLogBuildSum( szSums, "FuzzyUsed1",  &(pDocSave2->FuzzyUsed_1) );
    RptLogBuildSum( szSums, "FuzzyExist2", &(pDocSave2->FuzzyExist_2) );
    RptLogBuildSum( szSums, "FuzzyUsed2",  &(pDocSave2->FuzzyUsed_2) );
    RptLogBuildSum( szSums, "FuzzyExist3", &(pDocSave2->FuzzyExist_3) );
    RptLogBuildSum( szSums, "FuzzyUsed3",  &(pDocSave2->FuzzyUsed_3) );
    RptLogBuildSum( szSums, "MachExist",   &(pDocSave2->MachExist) );
    RptLogBuildSum( szSums, "MachUsed",    &(pDocSave2->MachUsed) );
    RptLogBuildSum( szSums, "NoneExist",   &(pDocSave2->NoneExist) );
    RptLogBuildSum( szSums, "NoneExist2",  &(pDocSave2->NoneExist2) );
    RptLogBuildSum( szSums, "NotXlated",   &(pDocSave2->NotXlated) );

    fprintf( hRptLog, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function RptLogDocSave2 */


void RptLogDocSave3( PSZ pszString, PDOCSAVEHIST3 pDocSave3 )
{
  if ( fRptLogging )
  {
    static char szSums[1024];

    szSums[0] = EOS;
    RptLogBuildSum( szSums, "AnaSubst",    &(pDocSave3->AnalAutoSubst) );
    RptLogBuildSum( szSums, "EditSubst",   &(pDocSave3->EditAutoSubst) );
    RptLogBuildSum( szSums, "ExactExist",  &(pDocSave3->ExactExist) );
    RptLogBuildSum( szSums, "ExactUsed",   &(pDocSave3->ExactUsed) );
    RptLogBuildSum( szSums, "ReplExist",   &(pDocSave3->ReplExist) );
    RptLogBuildSum( szSums, "ReplUsed",    &(pDocSave3->ReplUsed) );
    RptLogBuildSum( szSums, "FuzzyExist",  &(pDocSave3->FuzzyExist) );
    RptLogBuildSum( szSums, "FuzzyUsed",   &(pDocSave3->FuzzyUsed) );
    RptLogBuildSum( szSums, "FuzzyExist1", &(pDocSave3->FuzzyExist_1) );
    RptLogBuildSum( szSums, "FuzzyUsed1",  &(pDocSave3->FuzzyUsed_1) );
    RptLogBuildSum( szSums, "FuzzyExist2", &(pDocSave3->FuzzyExist_2) );
    RptLogBuildSum( szSums, "FuzzyUsed2",  &(pDocSave3->FuzzyUsed_2) );
    RptLogBuildSum( szSums, "FuzzyExist3", &(pDocSave3->FuzzyExist_3) );
    RptLogBuildSum( szSums, "FuzzyUsed3",  &(pDocSave3->FuzzyUsed_3) );
    RptLogBuildSum( szSums, "MachExist",   &(pDocSave3->MachExist) );
    RptLogBuildSum( szSums, "MachUsed",    &(pDocSave3->MachUsed) );
    RptLogBuildSum( szSums, "NoneExist",   &(pDocSave3->NoneExist) );
    RptLogBuildSum( szSums, "NoneExist2",  &(pDocSave3->NoneExist2) );
    RptLogBuildSum( szSums, "NotXlated",   &(pDocSave3->NotXlated) );

    fprintf( hRptLog, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function RptLogDocSave3 */

void RptLogDocPropSums( PSZ pszString, PPROPDOCUMENT pProp )
{
  if ( fRptLogging )
  {
    static char szSums[1024];

    szSums[0] = EOS;
    RptLogBuildCountSum( szSums, "Total",      &(pProp->Total) );
    RptLogBuildCountSum( szSums, "ExactExact", &(pProp->ExactExact) );
    RptLogBuildCountSum( szSums, "Exact1",     &(pProp->ExactOne) );
    RptLogBuildCountSum( szSums, "Exact+",     &(pProp->ExactMore) );
    RptLogBuildCountSum( szSums, "Repl",       &(pProp->Repl) );
    RptLogBuildCountSum( szSums, "Fuzzy1",     &(pProp->Fuzzy1 ) );
    RptLogBuildCountSum( szSums, "Fuzzy2",     &(pProp->Fuzzy2 ) );
    RptLogBuildCountSum( szSums, "Fuzzy3",     &(pProp->Fuzzy3 ) );
    RptLogBuildCountSum( szSums, "NoProp",     &(pProp->NoProps ) );
    RptLogBuildCountSum( szSums, "MTProps",    &(pProp->MTProps ) );

    fprintf( hRptLog, "%s %s\n", pszString, szSums );
  } /* endif */
} /* end of function RptLogDocPropSums */


void RptLogBuildSum( PSZ pszBuffer, PSZ pszCol, PCRITERIASUM pSum )
{
  ULONG ulSum = pSum->ComplexSum.ulSrcWords +
                pSum->MediumSum.ulSrcWords +
                pSum->SimpleSum.ulSrcWords;
  if ( ulSum )
  {
    sprintf( pszBuffer + strlen(pszBuffer), "%s(%lu) ", pszCol, ulSum );
  } /* endif */
} /* end of function RptLogBuildSum */

void RptLogBuildSumEx( PSZ pszBuffer, PSZ pszCol, PCRITERIASUMEX pSum )
{
  ULONG ulSum = pSum->ComplexSum.ulSrcWords +
                pSum->MediumSum.ulSrcWords +
                pSum->SimpleSum.ulSrcWords;
  if ( ulSum )
  {
    sprintf( pszBuffer + strlen(pszBuffer), "%s(%lu) ", pszCol, ulSum );
  } /* endif */
} /* end of function RptLogBuildSum */

void RptLogBuildCountSum( PSZ pszBuffer, PSZ pszCol, PCOUNTSUMS pSum )
{
  ULONG ulSum = pSum->ulComplexWords +
                pSum->ulMediumWords +
                pSum->ulSimpleWords;
  if ( ulSum )
  {
    sprintf( pszBuffer + strlen(pszBuffer), "%s(%lu) ", pszCol, ulSum );
  } /* endif */
} /* end of function RptLogBuildSum */
