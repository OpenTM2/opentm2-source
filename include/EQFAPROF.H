//+----------------------------------------------------------------------------+
//| EQFAPROF.C                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|   Analysis profile related functions                                       |
//|                                                                            |
//|     TaBalanceList                                                          |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//
// PVCS Section
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
// $Revision: 1.2 $ ----------- 3 Sep 2008
// GQ: - added fCombineFlags parameter to AnaProfApplyProfile
// 
// 
// $Revision: 1.1 $ ----------- 31 Jul 2008
// GQ: - initial put
// 
// 


__declspec(dllexport) BOOL AnaProfApplyProfile( LONG hProfile, PVOID pvTAIn, PSZ pszMarkup, BOOL fCombineFlags );
__declspec(dllexport) BOOL AnaProfileDialog( PSZ pszProfile, HWND hwndParent );
__declspec(dllexport) BOOL AnaProfileCheckName( PSZ pszName, HWND hwndParent, BOOL fShowMessage  );
__declspec(dllexport) BOOL AnaProfListProfiles( HWND hwndLB, BOOL fIsCombo );
__declspec(dllexport) BOOL AnaProfLoadProfile( PSZ pszProfile, PLONG plProfileHandle, HWND hwndErrMsg );
__declspec(dllexport) BOOL AnaProfFreeProfile( LONG lProfileHandle );
__declspec(dllexport) BOOL AnaProfBuildFullProfileName( PSZ pszProfile, PSZ pszFullPathName );

// file extention of analysis profile files
#define EXT_OF_ANAPROFILE ".APR"

// max length of an analysis profile name
#define MAX_PROFILENAME_LEN 40
