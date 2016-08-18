//+----------------------------------------------------------------------------+
//|EQFRPT03.H                                                                  |
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
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
// 
// 
// $Revision: 1.3 $ ----------- 9 Jul 2001
// --RJ: add LVL, LTL, EEK (KBT1011)
// 
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFRPT03.HV_   1.1   12 Feb 1999 09:52:02   BUILD  $
 *
 * $Log:   K:\DATA\EQFRPT03.HV_  $
 *
 *    Rev 1.1   12 Feb 1999 09:52:02   BUILD
 * added EURO
 *
 *    Rev 1.0   20 Jan 1999 18:37:36   BUILD
 * Initial revision.
 *
 *
*/
//+----------------------------------------------------------------------------+


#define MAX2_OPTIONS4                       73  // # of currencies

// local currencies


typedef CHAR LOCAL_CURRENCY[5];


LOCAL_CURRENCY szRptCurrency[MAX2_OPTIONS4] = {"ARP",
                                               "AUD",
                                               "ATS",
                                               "BSD",
                                               "BBD",
                                               "BEF",
                                               "BMD",
                                               "BRR",
                                               "BGL",
                                               "CAD",
                                               "CHF",
                                               "CLP",
                                               "CNY",
                                               "CYP",
                                               "CSK",
                                               "DEM",
                                               "DKK",
                                               "DZD",
                                               "EEK",  // XJR:KBT1011;Estonia
                                               "EGP",
                                               "EUR",
                                               "FJD",
                                               "FIM",
                                               "FRF",
                                               "GBP",
                                               "GRD",
                                               "HKD",
                                               "HUF",
                                               "ISK",
                                               "INR",
                                               "IDR",
                                               "IEP",
                                               "ILS",
                                               "ITL",
                                               "JMD",
                                               "JOD",
                                               "JPY",
                                               "LBP",
                                               "LTL",  //XJR:KBT1011:Lithuania
                                               "LUF",
                                               "LVL",  //XJR: KBT1011: Latvia
                                               "MYR",
                                               "MXP",
                                               "NLG",
                                               "NZD",
                                               "NOK",
                                               "PKR",
                                               "PHP",
                                               "PLZ",
                                               "PTE",
                                               "ROL",
                                               "SUR",
                                               "SAR",
                                               "SGD",
                                               "SKK",
                                               "ZAR",
                                               "KRW",
                                               "ESP",
                                               "SDD",
                                               "SEK",
                                               "TWD",
                                               "THB",
                                               "TTD",
                                               "TRL",
                                               "USD",
                                               "VEB",
                                               "ZMK",
                                               "XEC",
                                               "XEU",
                                               "XDR",
                                               "XAG",
                                               "XAU",
                                               "XPT" };







