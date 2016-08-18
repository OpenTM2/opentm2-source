
//+----------------------------------------------------------------------------+
//|EQFMTLST.H      Predefines lists for Machine Translation support            |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
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
// $Revision: 1.2 $ ----------- 9 Mar 2007
// GQ: - added "English(U.K.)" to the list of supported MT source languages
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
// $Revision: 1.2 $ ----------- 19 Apr 2004
// GQ: - added define for name of supported markup tables (MT_SUPPORTEDMARKUPS_LIST)
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
// $Revision: 1.3 $ ----------- 14 May 2003
// GQ: - made language lists static to avoid link problems
// 
//
// $Revision: 1.2 $ ----------- 10 Apr 2003
// GQ: - reduced list of markups requiring no inline tag handling to HTML and
//       IBMIDDOC
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.1 $ ----------- 20 Sep 2002
// GQ: - initial put
//
//
//+----------------------------------------------------------------------------+

// name of external file with list of supported markup tables
#define MT_SUPPORTEDMARKUPS_LIST "EQFMTMUT.LST"

#ifdef MTLIST_ALLOWEDSOURCELANG

// list of allowed source languages for machine translation
static PSZ apszAllowedMTSourceLang[] =
{
  "English(U.S.)",
  "English(U.K.)",
  NULL
};
#endif

#ifdef MTLIST_ALLOWEDTARGETLANG

// list of allowed target languages for machine translation
static PSZ apszAllowedMTTargetLang[] =
{
  "French(Canadian)",
  "French(national)",
  "German(DPAnat)",
  "German(national)",
  "German(newnat)",
  "German(Swiss)",
  "Italian",
  "Spanish",
  "Portuguese(Brasil)",
  NULL
};

#endif

#ifdef MTLIST_DEFINE_NOINLINETAGMASKING_ARRAY

// markup tables which require no inline tag masking

// currently only HTML and IBMIDDOC are supported

PSZ apszMTNoInlineTagMasking[] =
{
  "EQFHTML2",
  "EQFHTML4",
  "IBMHTM32",
  "IBMIDDOC",
  NULL
};

#endif
