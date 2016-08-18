//+----------------------------------------------------------------------------+
//|EQFRPT01.H                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Michael Sekinger                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Private include file for Counting Report                       |
//|                                                                            |
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
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 31 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
// 
// 
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
// 
// 
// $Revision: 1.3 $ ----------- 6 Feb 2002
// --RJ: KBT969: add RPT_OUT_FOLDERFORMAT, RPT_OUT_NEWSOURCENOTIMPORTED
// 
//
// $Revision: 1.2 $ ----------- 3 Dec 2001
// MK(01/12/2001)
// -- Added output of adjusted fuzzy level (R004901)
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFRPT01.HV_   1.2   12 Feb 1999 09:52:24   BUILD  $
 *
 * $Log:   K:\DATA\EQFRPT01.HV_  $
 *
 *    Rev 1.2   12 Feb 1999 09:52:24   BUILD
 * changed line3
 *
 *    Rev 1.1   13 Nov 1998 12:27:30   BUILD
 * changed lines
 *
 *    Rev 1.0   02 Dec 1996 10:42:54   BUILD
 * - initial put
 *
*/
//+----------------------------------------------------------------------------+


// additional one to LONGTODATETIME macro in eqf.h
#define LONG2DATETIME(ltime, buffer) \
{ \
  UtlLongToDateString(ltime, buffer, sizeof (buffer)); \
  strcat(buffer, "  ");  \
  UtlLongToTimeString(ltime, buffer + strlen (buffer), \
                      sizeof (buffer) - strlen (buffer)); \
}


// type of output string
// WARNING !!!!!!
// this enumeration depends on the order of the MRIs in the files EQFRPT.ID
//                                                                      .MRI
//                                                                      .STR
typedef enum
{
  RPT_FORMAT, RPT_EXT_FORMAT, RPT_INT_FORMAT, RPT_FOLDER, RPT_DOCUMENT,
  RPT_MARKUP, RPT_MEMORY, RPT_DESCRIPTION, RPT_EDITOR, RPT_DICTIONARY,
  RPT_SRC_LNG, RPT_TGT_LNG, RPT_SRCDOCREPL, RPT_TGTDOCREPL, RPT_IMPORT_PATH,
  RPT_SRC_PATH, RPT_TGT_PATH, RPT_ADDTOTM, RPT_AUTOREPLACE, RPT_LATESTMATCH,
  RPT_AUTOJOIN, RPT_ORIGINAL, RPT_TRANSLATION, RPT_SNOMATCH, RPT_SNOMATCH_PATH,
  RPT_WITHREVMARK, RPT_SEGMENTS, RPT_SOURCE, RPT_MODIFIED, RPT_TARGET,
  RPT_WORDS, RPT_ANALYSIS, RPT_AUTOSUBST, RPT_EDIT, RPT_EXACT,
  RPT_REPLACE, RPT_FUZZY, RPT_MACHINE, RPT_NONE, RPT_MATCHES,
  RPT_NOT, RPT_TRANSLATED, RPT_SUMMARY, RPT_KL5, RPT_KL15,
  RPT_GRGL15, RPT_NUMBER, RPT_DATE, RPT_TIME, RPT_PROCESSTASK,
  RPT_NULLSTRING, RPT_DOCOFFOLDER, RPT_SUMGENERATED, RPT_TRUE, RPT_FALSE,
  RPT_LASTPROCESS, RPT_STATE, RPT_DOCSSELECTED, RPT_OF, RPT_RPTGENERATED,
  RPT_ACTION_1, RPT_ACTION_2, RPT_ACTION_3, RPT_ACTION_4, RPT_ACTION_5,
  RPT_ACTION_6, RPT_ACTION_7, RPT_ACTION_8, RPT_ACTION_9, RPT_ACTION_10,
  RPT_ACTION_11, RPT_ACTION_12, RPT_ACTION_13, RPT_ACTION_14, RPT_ACTION_15,
  RPT_ACTION_16, RPT_ACTION_17, RPT_ACTION_18, RPT_FUZZY_LEVEL,
  RPT_OUT_FOLDERFORMAT, RPT_OUT_NEWSOURCENOTIMPORTED
} OUTPUTTYPE;


// Output Lines
static CHAR szRptLine1[80] = "=========================================================================";
static CHAR szRptLine2[80] = "-------------------------------------------------------------------------";

static CHAR szRptLine3[110] = "=========================================================================================";
static CHAR szRptLine4[100] = "----------------------------------------------------------------------------";

