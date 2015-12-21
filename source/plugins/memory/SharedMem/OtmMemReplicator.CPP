/*! \file

 \brief OtmMemReplikator Replication Tool for Shared Memory Databases

Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "TransportThread.h"

// application name in the registry
#define APPL_NAME "OpenTM2"

// key for the OpenTM2 system path
#define KEY_PATH    "Path"

// key for the OpenTM2 system drive
#define KEY_DRIVE    "Drive"

const char g_szClassName[] = "OtmMemReplicator";

#define IDC_MAIN_STOP	101
#define IDC_MAIN_SEND	102
#define IDC_MAIN_RECEIVED	103
#define IDC_MAIN_NUMOFMEMS	104

#define TIMER_ID 14

static int iTimerID = -1;

static char szPropertyPath[512];

static char szBuffer[512];

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			CreateWindow( "BUTTON", "Stop Replication", 
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
				0, 0, 180, 30, hwnd, (HMENU)IDC_MAIN_STOP, GetModuleHandle(NULL), NULL);

			CreateWindow( "STATIC", "Send proposals: ", 
				WS_CHILD | WS_VISIBLE | SS_CENTER, 
				0, 0, 300, 20, hwnd, (HMENU)IDC_MAIN_SEND, GetModuleHandle(NULL), NULL);

      CreateWindow( "STATIC", "Received proposals: ", 
				WS_CHILD | WS_VISIBLE | SS_CENTER, 
				0, 0, 300, 20, hwnd, (HMENU)IDC_MAIN_RECEIVED, GetModuleHandle(NULL), NULL);

      CreateWindow( "STATIC", "Number of shared memory databases: ", 
				WS_CHILD | WS_VISIBLE | SS_CENTER, 
				0, 0, 300, 20, hwnd, (HMENU)IDC_MAIN_NUMOFMEMS, GetModuleHandle(NULL), NULL);

      StartTransportThread( szPropertyPath );

      iTimerID = SetTimer( hwnd, TIMER_ID, 2000, NULL );
		}
		break;

    case WM_COMMAND:
		{
      if ( LOWORD(wParam) == IDC_MAIN_STOP )
      {
        PostMessage( hwnd, WM_CLOSE, 0, 0 );
      }
		}
		break;

    case WM_TIMER:
		{
			HWND hControl;
      int iSend = 0;
      int iReceived = 0;
      int iMems = 0;

      GetTransportStatistics( &iSend, &iReceived, &iMems );

			hControl = GetDlgItem(hwnd, IDC_MAIN_SEND );
      sprintf( szBuffer, "Send proposals: %ld", iSend );
			SetWindowText( hControl, szBuffer );

			hControl = GetDlgItem(hwnd, IDC_MAIN_RECEIVED );
      sprintf( szBuffer, "Received proposals: %ld", iReceived );
			SetWindowText( hControl, szBuffer );

			hControl = GetDlgItem(hwnd, IDC_MAIN_NUMOFMEMS );
      sprintf( szBuffer, "Number of shared memory databases: %ld", iMems );
			SetWindowText( hControl, szBuffer );
		}
		break;

		case WM_SIZE:
		{
			HWND hControl;
			RECT rcClient;

			GetClientRect(hwnd, &rcClient);
      
			hControl = GetDlgItem(hwnd, IDC_MAIN_STOP);
			SetWindowPos( hControl, NULL, (rcClient.right - rcClient.left - 180) / 2, rcClient.bottom - 40, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

			hControl = GetDlgItem(hwnd, IDC_MAIN_SEND );
			SetWindowPos(hControl, NULL, 0, rcClient.bottom - 64, rcClient.right, 20, SWP_NOZORDER );

			hControl = GetDlgItem(hwnd, IDC_MAIN_RECEIVED );
			SetWindowPos(hControl, NULL, 0, rcClient.bottom - 88, rcClient.right, 20, SWP_NOZORDER );

			hControl = GetDlgItem(hwnd, IDC_MAIN_NUMOFMEMS );
			SetWindowPos(hControl, NULL, 0, rcClient.bottom - 112, rcClient.right, 20, SWP_NOZORDER );
    }
		break;
		case WM_CLOSE:
      if ( iTimerID != -1 ) KillTimer( hwnd, iTimerID );
      StopTransportThread();
			DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

  // register outr window cloass
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -2;
	}

  // get OpenTM2 properties path
  HKEY hKey = NULL;
  szPropertyPath[0] = 0;
  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
    HKEY hSubKey = NULL;
    if ( RegOpenKeyEx( hKey, APPL_NAME, 0, KEY_ALL_ACCESS, &hSubKey ) == ERROR_SUCCESS )
    {
      DWORD dwType = REG_SZ;
      DWORD iSize = sizeof( szPropertyPath );
      int iSuccess = RegQueryValueEx( hSubKey, KEY_DRIVE, 0, &dwType, (LPBYTE)szPropertyPath, &iSize );
      if ( iSuccess == ERROR_SUCCESS )
      {
        dwType = REG_SZ;
        iSize = sizeof( szPropertyPath );
        strcat( szPropertyPath, "\\" );
        iSuccess = RegQueryValueEx( hSubKey, KEY_PATH, 0, &dwType, (LPBYTE)(szPropertyPath+strlen(szPropertyPath)), &iSize );
        if ( iSuccess != ERROR_SUCCESS )
        {
          szPropertyPath[0] = 0;
        } /* endif */         
      } /* endif */         
      RegCloseKey(hSubKey);
    } /* endif */        
    RegCloseKey( hKey );
  } /* endif */     
  if ( szPropertyPath[0] == 0 )
  {
		MessageBox(NULL, "Could not access OpenTM2 registry information!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
  }
  else
  {
    strcat( szPropertyPath, "\\PROPERTY" );
  } /* endif */     


	hwnd = CreateWindowEx( 0, g_szClassName, "OpenTM2 Memory Replicator", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 480, 180,NULL, NULL, hInstance, NULL);

	if(hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -3;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
