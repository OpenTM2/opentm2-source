//+----------------------------------------------------------------------------+
//| OtmMemoryService.CPP                                                       |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (c) 2016, QSoft GmbH. All rights reserved.                  |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Web Server providing memory services using OpenTM2            |
//+----------------------------------------------------------------------------+
//

#include <time.h>
#include <stdarg.h>
#include <windows.h>
#include <process.h>  
#include <memory>
#include <thread>
#include <cstdlib>
#include <restbed>
#include <sstream>
#include "core\utilities\LanguageFactory.h"
#include "OtmProposal.h"
#include "OTMMSJSONFactory.h"
#include "OtmMemoryServiceWorker.h"
#include "OtmMemoryService.h"
#include "eqfserno.h"


const char g_szClassName[] = "OtmMemoryServiceClass";
const int idStopButton = 1600;
const int idLogButton = 1601;
const int idStatusLine = 1602;

static char szLogFile[MAX_PATH];
static char szLogFileDir[MAX_PATH];
static FILE *hfLog = NULL;
static HWND hwndLogButton = NULL;
static BOOL fProcessingUnhandledException = FALSE;

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
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_COMMAND:
	  if ( LOWORD( wParam ) == idStopButton )
		{
		  PostMessage( hwnd, WM_CLOSE, 0, 0 );
		}
    else if ( LOWORD( wParam ) == idLogButton )
    {
      if ( hfLog != NULL )
      {
        FILE *hf = hfLog;
        hfLog = NULL;
        fclose( hf );
        SetWindowText( hwndLogButton, "Start logging" );
      }
      else
      {
        hfLog = fopen( szLogFile, "a" );
        SetWindowText( hwndLogButton, "Stop logging" );
      }
      SetLogFile( hfLog );
    }
    break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// handler for unhandled excpetions
LONG CALLBACK unhandled_handler( EXCEPTION_POINTERS* e ){  if ( !fProcessingUnhandledException )  {    fProcessingUnhandledException = TRUE;    WriteCrashLog( szLogFileDir );  }  return EXCEPTION_CONTINUE_SEARCH;}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  // prepare log file
  MakeLogFileName();

  SetUnhandledExceptionFilter( unhandled_handler );

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx( WS_EX_CLIENTEDGE, g_szClassName, "OtmMemoryService", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 200, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// create stop service button
  CreateWindow( "BUTTON", "Stop Service", WS_CHILD | WS_VISIBLE, 40, 10, 200, 100, hwnd, (HMENU)idStopButton, hInstance, NULL );

  // create log button
  hwndLogButton = CreateWindow( "BUTTON", "Start logging", WS_CHILD | WS_VISIBLE, 250, 10, 100, 50, hwnd, (HMENU)idLogButton, hInstance, NULL );

  // create status line static
  CreateWindow( "STATIC", "", WS_CHILD | WS_VISIBLE, 20, 130, 460, 20, hwnd, (HMENU)idStatusLine, hInstance, NULL );

  // prepare web service
  char szServiceName[80];
  unsigned int uiPort = 0;
  PrepareOtmMemoryService( szServiceName, &uiPort );


  // show port and service name in status line
  char szValue[100];
  sprintf( szValue, "Port=%lu, ServiceName=%s", uiPort, szServiceName );
  SetDlgItemText( hwnd, idStatusLine, szValue );

  // show version info in titlebar
  sprintf( szValue, "OtmMemoryService Version %s", STR_DRIVER_LEVEL_NUMBER );
  SetWindowText( hwnd, szValue );

  // start web service
  StartOtmMemoryService();

  // show our service window
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

  StopOtmMemoryService();

  if ( hfLog ) fclose( hfLog );

	return Msg.wParam;
}


