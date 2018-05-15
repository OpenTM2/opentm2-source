//+----------------------------------------------------------------------------+
//|  EQFMT00.H - Internal header file for the EQF MTListHandler
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:                                                              |
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
// $Revision: 1.2 $ ----------- 3 Nov 2004
// GQ: - added additional FTP fields to MTPROP structure
// 
// 
// $Revision: 1.1 $ ----------- 31 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.2 $ ----------- 16 Jul 2004
// GQ: - added FTP fields to MT job properties
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
// $Revision: 1.2 $ ----------- 17 Mar 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
// 
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.6 $ ----------- 6 Nov 2002
// GQ: - added lNumOfWords to MT job properties
//
//
// $Revision: 1.5 $ ----------- 16 Oct 2002
// GQ: - added new fields to MT job properties
//
//
// $Revision: 1.4 $ ----------- 23 Sep 2002
// GQ: - Added statistic variables to MT job properties
//
//
// $Revision: 1.3 $ ----------- 20 Sep 2002
// GQ: - added some fields to MT Job properties
//
//
// $Revision: 1.2 $ ----------- 16 Sep 2002
// GQ: - added definitions for new MT support
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
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFMT00.HV_   1.2   15 Oct 1998 19:52:20   BUILD  $
 *
 * $Log:   K:\DATA\EQFMT00.HV_  $
 *
 *    Rev 1.2   15 Oct 1998 19:52:20   BUILD
 * - restructure header content
 *
 *    Rev 1.1   14 Sep 1998 15:31:48   BUILD
 * defines for MT profiles
 *
 *    Rev 1.0   25 Aug 1998 19:06:02   BUILD
 * Initial revision.
 *
*/
//+----------------------------------------------------------------------------+

#ifndef INCL_EQFMT00
  #define INCL_EQFMT00
  #define  HPS_DRIVLIST_cy   30
  #define  HPS_HEADLINE_cy   25

#define  MT_MAX_NUMBER_OF_PROFILES 100
#define  MT_MAX_PROFILE_LENGTH     30

#include "EQFMT.h"       // include special MT defines

// name of file containing subject areas
#define SUBJAREALISTFILE  "MTSUBJAREAS.LST"

// extension of MT job property files
#define EXT_OF_MTPROP ".MTP"

// extension of file containing segments for MT
#define EXT_OF_MTSEGS ".SNO"

// name of file containing last used values
#define MTLASTUSED  "LASTUSED.MT"

// name of file containing defaults
#define MTDEFAULTS  "DEFAULTS.MT"

// name of engine DLL for MQSeries
#define MT_ENGINE_MQS "EQFMTMQS.DLL"

// name of engine DLL for MQSeries via FTP
#define MT_ENGINE_FTP "EQFMTFTP.DLL"

typedef enum
{
   MT_OBJECT_IND,
   MT_INTSTATUS_IND,
   MT_JOBNAME_IND,
   MT_FOLDER_IND,
   MT_STATUS_IND,
   MT_DATE_IND,
   MT_SOURCELANG_IND,
   MT_TARGETLANG_IND
} MT_ITEMINDX;


/* FolderListHandler Instance Data Area */
typedef struct _MTLIST
{
    IDA_HEAD     IdaHead;              // common header of instance area
    HWND         hLBox;                // list box wnd handle
    CHAR         szColumnTitles[256];  // column titles
    SHORT  sLastUsedViewList[MAX_VIEW];// last used view list
    SHORT  sDetailsViewList[MAX_VIEW]; // user's details view list
    SHORT  sSortList[MAX_VIEW];        // user's sort criteria list
    OBJNAME     szObjName;             // general use object name buffer
} MTLIST_IDA, *PMTLIST_IDA;

// length of MT server names (server, queues, message ID,...)
#define MAX_MT_NAME 50

// length of subject area list
#define MAX_SUBJAREALIST 1024

// length of subject area list
#define MAX_STATUS_LENGTH 80

// FTP modes
typedef enum _EQFFTPMODE
{
  FTPMODE_DIRECTACCESS,
  FTPMODE_FTPPROXY,
  FTPMODE_SOCKSPROXY
} EQFFTPMODE;

// MT job properties file
typedef struct _MTJOBPROP
{
  CHAR        szFolderName[MAX_LONGFILESPEC];    // long name of folder (256)
  CHAR        szMQServer[MAX_MT_NAME];           // MQ server channel/IP/port address
  CHAR        szQueueMgr[MAX_MT_NAME];           // message queue manager name
  CHAR        szServerInQueue[MAX_MT_NAME];      // name of server input queue
  CHAR        szServerOutQueue[MAX_MT_NAME];     // name of server output queue
  CHAR        szSubjArea[MAX_SUBJAREALIST];      // list of subject areas
  CHAR        szSenderEMail[MAX_LONGFILESPEC];   // senders email address
  CHAR        szMessageID[MAX_MT_NAME];          // message ID returned by MQ series
  CHAR        szZipFile[MAX_LONGFILESPEC];       // name of used ZIP file
  CHAR        szSourceLang[MAX_LANG_LENGTH];     // source language (20)
  CHAR        szTargetLang[MAX_LANG_LENGTH];     // target language (20)
  LONG        lJobCreateDate;                    // creation time stamp
  LONG        lJobStatusDate;                    // time stamp of last received status message
  LONG        lStatus;                           // current job status
  LONG        lLastReceivedStatus;               // last status received from server
  LONG        lSendDate;                         // date/time job has been send to server
  USHORT      usMaxWords;                        // max words per segment
  USHORT      usSmallFuzzy;                      // fuzziness limit for small segments
  USHORT      usMediumFuzzy;                     // fuzziness limit for medium segments
  USHORT      usLargeFuzzy;                      // fuzziness limit for large segments
  USHORT      usUnused;                          // for future use
  CHAR        szStatus[MAX_STATUS_LENGTH];       // buffer for server status mesages
  CHAR        szMTEngineDLL[MAX_MT_NAME];        // DLL containing MT engine IF
  CHAR        szJobObject[MAX_LONGFILESPEC];     // MT job obejct name
  CHAR        szSegFile[MAX_LONGFILESPEC];       // file containing segments
  CHAR        szFree1[MAX_LONGFILESPEC];         // currently not used
  CHAR        szFree2[MAX_LONGFILESPEC];         // currently not used
  LONG        lTranslatedDate;                   // date/time job has been send to server
  LONG        lReceivedDate;                     // date/time job has been send to server
  LONG        lMergedDate;                       // date/time job has been send to server
  CHAR        szAvailSubjArea[2000];             // list of preselected subject areas
  BOOL        fMaxWordsFilter;                   // TRUE = max words filter is checked
  BOOL        fFuzzyFilter;                      // TRUE = fuzzy filter is checked
  LONG        lNumOfWords;                       // number of words for translation
  BOOL        fUseFTP;                           // TRUE = use FTP for file transfer
  CHAR        szFTPChargeToID[MAX_MT_NAME];      // FTP charge to user id
  CHAR        szFTPServer[MAX_MT_NAME];          // FTP server name  
  CHAR        szFTPUserID[MAX_MT_NAME];          // FTP user ID
  CHAR        szFTPPassword[MAX_MT_NAME*2];      // FTP password in encrypted form
  LONG        lLastStatusQuery;                  // last time the status was retrieved

  CHAR        szProxyAddress[MAX_MT_NAME];       // proxy server
  CHAR        szProxyPort[6];                    // port for proxy server
  CHAR        szFTPPort[6]  ;                    // port for FTP server
  EQFFTPMODE  FTPMode;                           // mode of FTP access
  BOOL        fPassiveMode;                      // use-passive-mode flag

  CHAR        szFiller[2602];                    // for future use
} MTJOBPROP, *PMTJOBPROP;


// properties of MT list window
typedef struct _MTLISTPROP
{
  EQF_SWP     SwpMTListWindow;               // Window position of MT List
  SHORT       sLastUsedMTViewList[MAX_VIEW]; // last used view list
  SHORT       sDetailsMTViewList[MAX_VIEW];  // user's details view list
  SHORT       sSortListMT[MAX_VIEW];         // user's sort criteria list
  SHORT       sLastUsedViewWidth[MAX_VIEW]; // last used view widths

  CHAR        szFiller[4096];         // for future use
} MTLISTPROP, * PMTLISTPROP;


__declspec(dllexport) MRESULT MTListHandlerCallBack
(
  PHANDLERCOMMAREA pCommArea,
  HWND             hwnd,
  WINMSG           message,
  WPARAM           mp1,
  LPARAM           mp2
);


/* Export window proc */

MRESULT MTListCallBack( PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

#endif




/********************************/
/* structure of MT property ida */
/* use document properties      */
/********************************/

typedef struct _mtida
{

  HPROP          hpropDocument;         // handle of document properties
  PPROPDOCUMENT  ppropDocument;         // pointer to document properties
  CHAR           szDocumentPropName[MAX_PATH144+MAX_FILESPEC];
  CHAR           szDocumentName[MAX_PATH144+MAX_FILESPEC];
  CHAR           szWorkString[400];                    // string buffer
  CHAR           szEngineString[400];                  // string buffer
  CHAR           szMachineString[400];                  // string buffer
  CHAR           szMTError[400];                       // error from MT engine


  // MT engine and Name out of ini file
  INT            nEngines;
  CHAR           szEngineName[10][100];
  CHAR           szEngineDll[10][20];

  // MT machines (servers) out of ini file
  INT            nMachines;
  CHAR           szMachineName[10][100];
  CHAR           szMachineTcpIp[10][100];
  CHAR           szMachinePort[10][20];

  // MT profiles defined by the MT engine
  INT            nProfiles;
  CHAR           szProfileName[MT_MAX_NUMBER_OF_PROFILES][MT_MAX_PROFILE_LENGTH];


} MTIDA, *PMTIDA;



INT_PTR CALLBACK EQFMTPROPERTYDLGPROC (HWND hwnd,
                                       WINMSG msg,
                                       WPARAM mp1,
                                       LPARAM mp2);



#define STR_MTEngine_NA "NA"
#define STR_MTEngine_0  "Not Prepared"
#define STR_MTEngine_1  "Prepared"
#define STR_MTEngine_2  "Enqueued"
#define STR_MTEngine_3  "Translating"
#define STR_MTEngine_3_ALL  "Translating (%d %)"
#define STR_MTEngine_4  "Translated"
#define STR_MTEngine_5  "Merged"
#define STR_MTEngine_6  "Failed"
#define STR_MTEngine_7  "Merging (%d %)"
#define STR_MTEngine_8  "Merging"                // internal state



BOOL FillMTPass ( PMTPASS pMTPass, PMTJOBPROP pProp, PSZ pszMTObject );

BOOL MTStatusString( LONG lStatus, PSZ pszStatus, PSZ pszBuffer );

//   End of EQFMT00.H
//ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß
