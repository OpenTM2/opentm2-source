//+----------------------------------------------------------------------------+
//| WinService.CPP                                                             |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (c) 2016, QSoft GmbH. All rights reserved.                  |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Simple Windows Service based         
//| Simple Windows Service in C++
//| Mohit Arora, 30 May 2013 CPOL
//+----------------------------------------------------------------------------+
//

#include <time.h>
#include <stdarg.h>
#include <windows.h>
#include <process.h>  
#include <stdarg.h>  
#include <memory>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "OtmMemoryService.h"
#include "eqfserno.h"

#define SERVICELOGGING

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
char                  g_szExecutablePath[MAX_PATH];
static char szLogFileDir[MAX_PATH];
static char szLogFile[MAX_PATH];
static BOOL fProcessingUnhandledException = FALSE;
FILE *hfLog = NULL;

VOID WINAPI ServiceMain( DWORD argc, LPTSTR *argv );
DWORD WINAPI ServiceCtrlHandler( DWORD  dwControl, DWORD  dwEventType, LPVOID lpEventData, LPVOID lpContext );
DWORD WINAPI ServiceWorkerThread( LPVOID lpParam );

#define SERVICE_NAME  "OtmMemoryService"

void MakeLogFileName()
{
  GetModuleFileName( NULL, szLogFileDir, sizeof( szLogFile ) );


  // strip exe name and binary path
  char *pszPathEnd = strrchr( szLogFileDir, '\\' );
  if ( pszPathEnd != NULL ) *pszPathEnd = '\0';
  pszPathEnd = strrchr( szLogFileDir, '\\' );
  if ( pszPathEnd != NULL ) *pszPathEnd = '\0';

  // build logs directory
  strcat( szLogFileDir, "\\logs" );
  CreateDirectory( szLogFileDir, NULL );

  // append log file name
  strcpy( szLogFile, szLogFileDir );
  strcat( szLogFile, "\\OtmMemoryServiceGUI.log" );

}

void WriteLogEntry( const char *pszFormat, ... )
{
  va_list vArgs;
  BOOL fLogOpened = FALSE;

  if ( hfLog == NULL )
  {
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog != NULL ) fLogOpened = TRUE;
  }

  if ( hfLog != NULL )
  {
    va_start( vArgs, pszFormat );

    time_t lTime = 0;
    time( &lTime );
    tm *pTime = localtime( &lTime );
    fprintf( hfLog, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d ", pTime->tm_year + 1900, pTime->tm_mon, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec );
    vfprintf( hfLog, pszFormat, vArgs );
    va_end( vArgs );
  } /* endif */

  if ( fLogOpened )
  {
    fclose( hfLog );
    hfLog = NULL;
  }
}

// Main Entry Point
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  SERVICE_TABLE_ENTRY ServiceTable[] =
  {
    { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
    { NULL, NULL }
  };

  // prepare log file name
  MakeLogFileName();

  // save executable path to global variable
  GetModuleFileName( NULL, g_szExecutablePath, sizeof( g_szExecutablePath ) );
  char *pszPathEnd = strrchr( g_szExecutablePath, '\\' );
  if ( pszPathEnd != NULL ) *pszPathEnd = '\0';

  if ( StartServiceCtrlDispatcher( ServiceTable ) == FALSE )
  {
    return GetLastError();
  }

  return 0;
}

// handler for unhandled excpetions
LONG CALLBACK unhandled_handler( EXCEPTION_POINTERS* e ){  if ( !fProcessingUnhandledException )  {    fProcessingUnhandledException = TRUE;    WriteLogEntry( "Error: Unhandled exception detected\n" );    WriteCrashLog( szLogFileDir );  }  return EXCEPTION_CONTINUE_SEARCH;}

// Service Entry Point
VOID WINAPI ServiceMain( DWORD argc, LPTSTR *argv )
{
  DWORD Status = E_FAIL;

  SetUnhandledExceptionFilter( unhandled_handler );

  // Register our service control handler with the SCM
  g_StatusHandle = RegisterServiceCtrlHandlerEx( SERVICE_NAME, ServiceCtrlHandler, NULL );

  if ( g_StatusHandle == NULL )
  {
    goto EXIT;
  }

  // Tell the service controller we are starting
  ZeroMemory( &g_ServiceStatus, sizeof( g_ServiceStatus ) );
  g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  g_ServiceStatus.dwControlsAccepted = 0;
  g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwServiceSpecificExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 0;

  if ( SetServiceStatus( g_StatusHandle, &g_ServiceStatus ) == FALSE )
  {
    WriteLogEntry( "Error: OtmMemoryService: ServiceMain: SetServiceStatus returned error\n" );
  }

  /*
  * Perform tasks necessary to start the service here
  */

  // Create a service stop event to wait on later
  g_ServiceStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
  if ( g_ServiceStopEvent == NULL )
  {
    // Error creating event
    // Tell service controller we are stopped and exit
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = GetLastError();
    g_ServiceStatus.dwCheckPoint = 1;

    if ( SetServiceStatus( g_StatusHandle, &g_ServiceStatus ) == FALSE )
    {
      WriteLogEntry( "Error: OtmMemoryService: ServiceMain: SetServiceStatus returned error\n" );
    }
    goto EXIT;
  }

  // Tell the service controller we are started
  g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_STOP;
  g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 0;

  if ( SetServiceStatus( g_StatusHandle, &g_ServiceStatus ) == FALSE )
  {
    WriteLogEntry( "Error: OtmMemoryService: ServiceMain: SetServiceStatus returned error\n" );
  }

  // Start a thread that will perform the main task of the service
  HANDLE hThread = CreateThread( NULL, 0, ServiceWorkerThread, NULL, 0, NULL );

  // Wait until our worker thread exits signaling that the service needs to stop
  WaitForSingleObject( hThread, INFINITE );


  /*
  * Perform any cleanup tasks
  */

  CloseHandle( g_ServiceStopEvent );

  // Tell the service controller we are stopped
  g_ServiceStatus.dwControlsAccepted = 0;
  g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 3;

  if ( SetServiceStatus( g_StatusHandle, &g_ServiceStatus ) == FALSE )
  {
    WriteLogEntry( "Error: OtmMemoryService: ServiceMain: SetServiceStatus returned error\n" );
  }

EXIT:
  return;
}

// Service Control Handler

DWORD WINAPI ServiceCtrlHandler( DWORD  dwControl, DWORD  dwEventType, LPVOID lpEventData, LPVOID lpContext )
{
  switch ( dwControl )
  {
    case SERVICE_CONTROL_PRESHUTDOWN:
      WriteLogEntry( "Info: OtmMemoryService: ServiceCtrlHandler: SERVICE_CONTROL_PRESHUTDOWN received\n" );
      g_ServiceStatus.dwControlsAccepted = 0;
      g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
      g_ServiceStatus.dwWin32ExitCode = 0;
      g_ServiceStatus.dwCheckPoint = 4;
      if ( SetServiceStatus( g_StatusHandle, &g_ServiceStatus ) == FALSE )
      {
        WriteLogEntry( "Error: OtmMemoryService: ServiceCtrlHandler: SetServiceStatus returned error\n" );
      }

      // This will signal the worker thread to start shutting down
      SetEvent( g_ServiceStopEvent );
      break;

    case SERVICE_CONTROL_STOP:
      WriteLogEntry( "Info: OtmMemoryService: ServiceCtrlHandler: SERVICE_CONTROL_STOP received\n" );
      if ( g_ServiceStatus.dwCurrentState != SERVICE_RUNNING )
        break;

      /*
      * Perform tasks necessary to stop the service here
      */

      g_ServiceStatus.dwControlsAccepted = 0;
      g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
      g_ServiceStatus.dwWin32ExitCode = 0;
      g_ServiceStatus.dwCheckPoint = 4;

      if ( SetServiceStatus( g_StatusHandle, &g_ServiceStatus ) == FALSE )
      {
        WriteLogEntry( "Error: OtmMemoryService: ServiceCtrlHandler: SetServiceStatus returned error\n" );
      }

      // This will signal the worker thread to start shutting down
      SetEvent( g_ServiceStopEvent );

      break;

    default:
      break;
  }
  return( NO_ERROR );
}

// Service Worker Thread
DWORD WINAPI ServiceWorkerThread( LPVOID lpParam )
{
  hfLog = fopen( szLogFile, "a" );
  WriteLogEntry( "Info: ServiceWorkerThread: Service is starting, version=%s\n", STR_DRIVER_LEVEL_NUMBER );

  char szServiceName[80];
  unsigned int uiPort = 0;
  if ( !PrepareOtmMemoryService( szServiceName, &uiPort ) )
  {
    WriteLogEntry( "Error: ServiceWorkerThread: Initialization of OtmMemoryService failed\n" );
    fclose( hfLog );
    hfLog = NULL;
    return( ERROR_UNIDENTIFIED_ERROR );
  }

  StartOtmMemoryService();
  WriteLogEntry( "Info: ServiceWorkerThread: OtmMemoryService started,  ServiceName=%s, Port=%lu\n", szServiceName, uiPort );
  if ( hfLog ) fclose( hfLog );
  hfLog = NULL;

  //  Periodically check if the service has been requested to stop
  while ( WaitForSingleObject( g_ServiceStopEvent, 0 ) != WAIT_OBJECT_0 )
  {
    //  Wait some time before checking the stop event
    Sleep( 1000 );
  }

  // stop the OtmMemoryService
  StopOtmMemoryService();
  WriteLogEntry( "Info: ServiceWorkerThread: OtmMemoryService has been stopped\n" );

  return ERROR_SUCCESS;
}