/*! \file
	Description: Generic API codes for Machine Translation support

	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

/**********************************************************************/
/* check if our EQF standard include file is already included -- if   */
/* not make here the needed defines/typedefs                          */
/**********************************************************************/
#ifndef _EQFMT_H_INCLUDED
  #define _EQFMT_H_INCLUDED

  #ifndef _EQF_H_INCLUDED
    typedef USHORT EQF_BOOL;
    #define EOS '\0'
    #define NULLHANDLE 0
    typedef int   (pascal far  *PFN)();
  #endif


/**************************************/
/*  Machine Translation Engine Status */
/**************************************/


typedef enum
{

    MTEngine_NotReady,
    MTEngine_Prepared,
    MTEngine_Waiting,
    MTEngine_Processing,
    MTEngine_Translated,
    MTEngine_Merged,
    MTEngine_Failed,
    MTEngine_Merging,
    MTEngine_MergeStart                // internal state

} MTEngineStatus;

// states of MT communication
typedef enum
{

    MTComm_OK,
    MTComm_ServerConnectionFailed,
    MTComm_ServerQueueFailed

} MTCommunicationState;

typedef struct _mt_pass
{
    CHAR chServer[50];            // used for message queue manager name / FTP server
    CHAR chServerInQueue[50];     // name for server input queue
    CHAR chServerOutQueue[50];    // name for server output queue
    CHAR chQueueMgr[50];          // name of queue manager / FTP user ID
    CHAR chTCPIP[20];
    CHAR chPORT[50];
    CHAR chUSER[50];
    CHAR chPASS[50];
    CHAR chTEMP1[256];            // used for folder long name
    CHAR chTEMP2[256];            // used for number of words in job
    CHAR chTEMP3[256];            // FTP password
    CHAR chSourceLang[20];        // source language
    char chTargetLang[20];        // target language
    CHAR chSUBJAREA[1024];        // list of comma seperated subject areas
    CHAR chSENDEREMAIL[256];      // email address of job submitter
    CHAR chMESSAGEID[51];         // message ID created by MQSeries
    CHAR chZIPFILE[256];          // name to be used for ZIP file
    LONG lCommStatus;             // communication status
    BOOL fDLLLoadFailed;          // TRUE = load of DLL failed
    BOOL fStateAvailable;         // TRUE = if a new MT server state is available
    CHAR chProxy[50];             // FTP: proxy server
    CHAR chProxyPort[50];         // FTP: Proxy port
    BOOL fPassiveMode;            // FTP: passive mode
    int  iFTPMode;                // FTP mode: 0 = direct access, 1 = socks proxy, 2 = FTP proxy
} MTPASS, *PMTPASS;

// new states used for MT job status
typedef enum _MTJOBSTATUS
{
  MTJOB_Preparing,
  MTJOB_Prepared,
  MTJOB_Sending,
  MTJOB_Sended,
  MTJOB_ServerWait,
  MTJOB_Translated,
  MTJOB_Receiving,
  MTJOB_Received,
  MTJOB_Merging,
  MTJOB_Merged,
  MTJOB_Completed,
  MTJOB_Failed
} MTJOBSTATUS;

// structure used for changing the status of a MT job
typedef struct _MTSTATUSCHANGE
{
  PSZ    pszMTJobObject;               // ptr to name of MT job object name
  int    iStatus;                      // new MT job status
  CHAR   szStatus[80];                 // buffer for status string (not required)
  LONG   lNumOfWords;                  // number of words (only MTJOB_Prepared state)
} MTSTATUSCHANGE, *PMTSTATUSCHANGE;

// Function Prototypes
// -------------------

EQF_BOOL APIENTRY MT_GET_STATUS
(
    PMTPASS pMTPass,
    PSZ pszMTId,
    PSZ pszMTError,
    PLONG plJobStatus,
    PUSHORT pusComplete
);

#ifdef EQFMT00_C
EQF_BOOL   (APIENTRY *pfnMTGetStatus) (PMTPASS, PSZ, PSZ, PLONG, PUSHORT );
#endif

EQF_BOOL APIENTRY MT_SEND_FILE
(
    PMTPASS pMTPass,
    PSZ pszDocumentPropertyName,
    PSZ pSourceLang,
    PSZ pTargetLang,
    PSZ pszMTId,
    PSZ pszProfile,
    PSZ pszMTError

);

#ifdef EQFMT00_C
  EQF_BOOL   (APIENTRY *pfnMTSendFile) (PMTPASS, PSZ, PSZ, PSZ, PSZ, PSZ, PSZ );
#endif

EQF_BOOL APIENTRY MT_RECEIVE_FILE
(
    PMTPASS pMTPass,
    PSZ pszMTId,
    PSZ pszDocument,
    PSZ pszMTError
);

#ifdef EQFMT00_C
  EQF_BOOL   (APIENTRY *pfnMTReceiveFile) (PMTPASS, PSZ, PSZ, PSZ );
#endif

EQF_BOOL APIENTRY MT_DELETE_JOB
(
     PMTPASS pMTPass,
     PSZ pszMTJobId,
     PSZ pszMTError
);

#ifdef EQFMT00_C
  EQF_BOOL   (APIENTRY *pfnMTDeleteJob) ( PMTPASS,PSZ, PSZ );
#endif

EQF_BOOL APIENTRY MT_INIT
(
     PMTPASS pMTPass
);

#ifdef EQFMT00_C
  EQF_BOOL   (APIENTRY *pfnMTInit) ( PMTPASS );
#endif

EQF_BOOL APIENTRY MT_TERMINATE
(
);

#ifdef EQFMT00_C
  EQF_BOOL   (APIENTRY *pfnMTTerminate)();
#endif

#endif
