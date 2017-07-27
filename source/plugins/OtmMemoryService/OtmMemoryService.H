/**
* \file OtmMemoryService.H
*
* \brief Header file for OtmMemoryService 
*
*	Copyright Notice:
*
*	Copyright (C) 2017, QSoft GmbH. All rights reserved
*	
**/
#ifndef _OtmMemoryService_H_
#define _OtmMemoryService_H_

typedef struct _THREADDATA
{
  HANDLE g_ServiceStopEvent;
  HINSTANCE hInstance;
  LPSTR lpCmdLine;
  int nCmdShow;
} THREADDATA, *PTHREADDATA;

BOOL PrepareOtmMemoryService( char *pszService, unsigned *puiPort );
BOOL StartOtmMemoryService();
void StopOtmMemoryService();
void SetLogFile( FILE *hfLog );
void WriteCrashLog( char *pszLogDir );



#endif