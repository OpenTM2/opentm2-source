// Copyright (c) 2007-2011, International Business Machines
// Corporation and others.  All rights reserved.
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
// $Revision: 1.2 $ ----------- 13 Mar 2007
// GQ: - added define XSLT_CONVERSION_PROC and XSLT_CONVPROC_PTR
// 
// 
// $Revision: 1.1 $ ----------- 9 Mar 2007
// GQ:- inital put
// 
// 
extern "C"
{
  BOOL XSLCONVERT( PSZ pszXMLFile, PSZ pszXSLStyleSheet, PSZ pszOutFile, BOOL fMsg, HWND hwndErrMsg );



}

#define XSLT_CONVERSION_PROC "XSLCONVERT"
#define XSLT_CONVPROC_PTR(name) BOOL (*name)( PSZ pszXMLFile, PSZ pszXSLStyleSheet, PSZ pszOutFile, BOOL fMsg, HWND hwndErrMsg );
