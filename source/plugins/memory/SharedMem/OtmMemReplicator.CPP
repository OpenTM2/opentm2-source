/*! \file

 \brief OtmMemReplikator Replication Tool for Shared Memory Databases

Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/
#include <stdio.h>
#include "TransportThread.h"
#include "resource.h"

#define MRFROMSHORT(s)    ((LRESULT)(SHORT)(s))
#define MP2FROMP(p)       ((LPARAM)(VOID *)(p))

#define TIMER_ID 14

// application name in the registry
#define APPL_NAME "OpenTM2"

// key for the OpenTM2 system path
#define KEY_PATH    "Path"

// key for the OpenTM2 system drive
#define KEY_DRIVE    "Drive"

const char g_szClassName[] = "OtmMemReplicator";
static char g_szPropertyPath[512];
static int g_iTimerID = -1;
static HANDLE hMonitorThread = 0;
static volatile bool g_Monintor = true;

static int getOpenTM2InstallPath();
static void timerEvent(HWND hwnd);
static void centerWindow(HWND hwnd);


static void addLeftToRight(HWND hwndFrom, HWND hwndTo);
static void addToSelected(HWND hwndDlg);
static void addToAvailable(HWND hwndDlg);

static void listAllSharedMemories(std::vector<std::string>& availMemories);
static void fillAvailableList(HWND hwndDlg,std::vector<std::string>& availMems,std::vector<std::string>& selMems);
static void listSelectedMemories(std::vector<std::string>& selMems);
static void fillSelectedList(HWND hwndDlg,std::vector<std::string>& availMems,std::vector<std::string>& selMems);
static void saveSelectedMemories(HWND hwnd);
static void sendSelectedMemoriesToGlobal(HWND hwndDlg);

static void processCommandEvent(HWND hwndDlg,WPARAM mp1,LPARAM mp2);
static  INT_PTR CALLBACK ReplicatorDlgProc(HWND hwndDlg,UINT msg,WPARAM mp1,LPARAM mp2);

unsigned int __stdcall  MonitorThreadProc( void *pData );
static void beginMonitorThread();
static void stopMonitorThread();

static CRITICAL_SECTION g_MonitorCS;
void  initMonitorCS()
{
    InitializeCriticalSection(&g_MonitorCS);
}
void  deleteMonitorCS()
{
	DeleteCriticalSection(&g_MonitorCS);
}

static bool g_FileChanged = false;
void setFileChangedFlag(bool bChanged)
{
	EnterCriticalSection(&g_MonitorCS);
	g_FileChanged = bChanged;
	LeaveCriticalSection(&g_MonitorCS);
}
bool getFileChangedFlag()
{
	bool changed =false;
	EnterCriticalSection(&g_MonitorCS);
	changed = g_FileChanged;
	LeaveCriticalSection(&g_MonitorCS);
	return changed;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWndReplicatorDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_REPLICATORDIALOG_DIALOG), NULL, ReplicatorDlgProc, 0);

    MSG msgReplicatorDlg;

    while (GetMessage(&msgReplicatorDlg, NULL, NULL, NULL))
    {
     if(!IsDialogMessage(hWndReplicatorDlg, &msgReplicatorDlg))
        {
            TranslateMessage(&msgReplicatorDlg);
            DispatchMessage(&msgReplicatorDlg);
        }
    }
	return msgReplicatorDlg.wParam;

}

INT_PTR CALLBACK ReplicatorDlgProc
(
HWND hwndDlg,
UINT msg,
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT  mResult = MRFROMSHORT(FALSE);


    switch (msg)
    {
    case WM_INITDIALOG:
        {
           centerWindow(hwndDlg);
           SetForegroundWindow(hwndDlg);

		   //
		   int replicatorInterval = 5;
		   HWND hwndRI = GetDlgItem(hwndDlg, IDC_EDIT_REPLICATIONINTERVAL );
		   char szBuffer[10+1]={'\0'};
		   _snprintf( szBuffer,10*sizeof(char), "%ld", replicatorInterval );
		   szBuffer[10]='\0';
	       SetWindowText( hwndRI, szBuffer );
		   setReplicatorInterval(replicatorInterval*1000);

		   //
		   getOpenTM2InstallPath();
		   //	
	       std::vector<std::string> selMems;
	       listSelectedMemories(selMems);
		  
		   std::vector<std::string> availMems;
		   listAllSharedMemories(availMems);

		   fillSelectedList(hwndDlg,availMems,selMems);
		   fillAvailableList(hwndDlg,availMems,selMems);

		   // for monitor thread
		   initMonitorCS();
		   beginMonitorThread();

		   // start transport thread, start timer
		   initCriticalSection();
		   sendSelectedMemoriesToGlobal(hwndDlg);
		   StartTransportThread( g_szPropertyPath );
           g_iTimerID = SetTimer( hwndDlg, TIMER_ID, 3000, NULL );

        }
        break;

    case WM_COMMAND:
		processCommandEvent(hwndDlg,mp1,mp2);
        break;

   case WM_TIMER:
		timerEvent(hwndDlg);
		break;

    case WM_CLOSE:
		{
			if ( g_iTimerID != -1 ) 
			{   
				KillTimer( hwndDlg, g_iTimerID );
				g_iTimerID = -1;
			}

			deleteMonitorCS();
			stopMonitorThread();

			saveSelectedMemories(hwndDlg);
			deleteCrititalSection();
            StopTransportThread();

            DestroyWindow(hwndDlg);
		}
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;

    default:
        break;
    }

    return mResult;
}

void getListBoxItemx(HWND hwnd, std::vector<std::string>& vec)
{
	int selCnts = SendMessage( hwnd, LB_GETCOUNT, 0,0);

    char item[128+1] = {'\0'};
	for(int i=0; i<selCnts; i++)
	{
		memset(item,0,sizeof(item));
		SendMessage( hwnd, LB_GETTEXT, (WPARAM)i,
                     MP2FROMP(item) );
		item[128] = '\0';
		vec.push_back(item);
	}
}

void addLeftToRight(HWND hwndFrom, HWND hwndTo)
{
	int selCnts = SendMessage( hwndFrom, LB_GETSELCOUNT, 0,0);
	int* pSelItems = new int[selCnts];
	if(pSelItems == NULL)
		return;

	SendMessage( hwndFrom, LB_GETSELITEMS, (WPARAM)selCnts,
                     MP2FROMP(pSelItems) );

	int addBeginIdx = SendMessage( hwndTo, LB_GETCOUNT, 0, 0);
    char item[128+1] = {'\0'};

	std::vector<std::string> selItems;
	for(int i=0; i<selCnts; i++)
	{
		memset(item,0,sizeof(item));
		SendMessage( hwndFrom, LB_GETTEXT, (WPARAM)pSelItems[i],
                     MP2FROMP(item) );
		selItems.push_back(item);
		item[128] = '\0';
		SendMessage( hwndTo, LB_ADDSTRING, addBeginIdx++,MP2FROMP(item) );
	}

	for( int i=0; i<selCnts; i++)
	{
		int idx = SendMessage(hwndFrom,LB_FINDSTRINGEXACT,0,MP2FROMP(selItems[i].c_str()));
		if(idx!=-1)
			SendMessage( hwndFrom, LB_DELETESTRING, (WPARAM)idx,0 );
	}
	if(pSelItems != NULL)
		delete []pSelItems;
}

void addToSelected(HWND hwndDlg)
{
    HWND hwndTo = GetDlgItem(hwndDlg, IDC_LIST_SHMEM_SELECTED ); 
	HWND hwndFrom = GetDlgItem(hwndDlg, IDC_LIST_SHMEM_AVAIABLE ); 

	addLeftToRight(hwndFrom,hwndTo);
}

void addToAvailable(HWND hwndDlg)
{
	HWND hwndFrom = GetDlgItem(hwndDlg, IDC_LIST_SHMEM_SELECTED ); 
	HWND hwndTo = GetDlgItem(hwndDlg, IDC_LIST_SHMEM_AVAIABLE ); 

	addLeftToRight(hwndFrom,hwndTo);
}

void saveSelectedMemories(HWND hwndDlg)
{
	HWND hwnd = GetDlgItem(hwndDlg, IDC_LIST_SHMEM_SELECTED ); 

	int cnts = SendMessage( hwnd, LB_GETCOUNT, 0, 0);
    char item[128+1] = {'\0'};

	std::string selMemFiles = std::string(g_szPropertyPath)+"\\ReplicatedMemories.txt";
	FILE* fout = fopen(selMemFiles.c_str(),"w");
	if(fout == NULL)
		return;

	for(int i=0; i<cnts; i++)
	{
		memset(item,0,sizeof(item));
		SendMessage( hwnd, LB_GETTEXT, (WPARAM)i,
                     MP2FROMP(item) );
		fputs(item,fout);
		if(strchr(item,'\n') == NULL)
		    fputs("\n",fout);
	}
	fclose(fout);
}

void sendSelectedMemoriesToGlobal(HWND hwndDlg)
{
	HWND hwnd = GetDlgItem(hwndDlg, IDC_LIST_SHMEM_SELECTED ); 

	int cnts = SendMessage( hwnd, LB_GETCOUNT, 0, 0);
    char item[128+1] = {'\0'};

	std::vector<std::string> vec;
	for(int i=0; i<cnts; i++)
	{
		memset(item,0,sizeof(item));
		SendMessage( hwnd, LB_GETTEXT, (WPARAM)i,
                     MP2FROMP(item) );
		vec.push_back(item);
	}

	setSelectedMemories(vec);
}

void listAllSharedMemories(std::vector<std::string>& availMemories)
{
  availMemories.clear();
  WIN32_FIND_DATA FindData;
  std::string strSearchPattern = std::string(g_szPropertyPath) + "\\*.SHP";
  HANDLE hDir = FindFirstFile( strSearchPattern.c_str(), &FindData );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    BOOL fMoreFiles = TRUE;
    do
    {
	  std::string temp(FindData.cFileName);
	  std::size_t ix = temp.find(".SHP");
	  if(ix != std::string::npos)
		  availMemories.push_back( temp.substr(0,ix) );
      fMoreFiles = FindNextFile( hDir, &FindData );
    } while ( fMoreFiles );
    FindClose( hDir );
  }
//MessageBox(NULL, g_AvailMems[0].c_str(), "Error!", MB_ICONEXCLAMATION | MB_OK);
}

void fillAvailableList(HWND hwndDlg,std::vector<std::string>& availMems,std::vector<std::string>& selMems)
{
	HWND hwnd = GetDlgItem(hwndDlg, IDC_LIST_SHMEM_AVAIABLE );
	if(hwnd == NULL)
		return;

	// delete all the memories in available
	SendMessage( hwnd, LB_RESETCONTENT, 0,0);
	
	int idx = 0;
	for(std::vector<std::string>::iterator iter = availMems.begin();
		iter != availMems.end();
		iter++)
	{

		bool bFound = false;
		for(std::vector<std::string>::iterator selIter = selMems.begin();
			selIter!=selMems.end();
			selIter++)
		{
			if(*iter == *selIter)
			{
				bFound = true;
				break;
			}
		}

		if(bFound)
			continue;

		SendMessage( hwnd, LB_ADDSTRING, idx++,MP2FROMP((*iter).c_str()) );

	}
}

void listSelectedMemories(std::vector<std::string>& selMems)
{
	selMems.clear();
	std::string selMemFiles = std::string(g_szPropertyPath)+"\\ReplicatedMemories.txt";
	FILE* fin = fopen(selMemFiles.c_str(),"r");
	if(fin != NULL)
	{
		char szBuf[128+1] = {'\0'};
		while( fgets( szBuf, sizeof(char)*128, fin ) )
		{
			if(strlen(szBuf)==0)
				break;
			char* pNewLine = strchr(szBuf,'\n');
			if(pNewLine!=NULL)
				*pNewLine = '\0';
		    selMems.push_back(szBuf);
		}
		fclose(fin);
	}
}


void fillSelectedList(HWND hwndDlg,std::vector<std::string>& availMems,std::vector<std::string>& selMems)
{
    HWND hwnd = GetDlgItem(hwndDlg, IDC_LIST_SHMEM_SELECTED );
	if(hwnd == NULL)
		return;

	// delete all the memories in available
	SendMessage( hwnd, LB_RESETCONTENT, 0,0);
	
	int idx = 0;
	for(std::vector<std::string>::iterator iter = selMems.begin();
		iter != selMems.end();
		iter++)
	{
		bool bFound = false;
		for(std::vector<std::string>::iterator selIter = availMems.begin();
			selIter!=availMems.end();
			selIter++)
		{
			if(*iter == *selIter)
			{
				bFound = true;
				break;
			}
		}

		if(!bFound)
			continue;

		SendMessage( hwnd, LB_ADDSTRING, idx++,MP2FROMP((*iter).c_str()) );

	}

}


int getOpenTM2InstallPath()
{
  HKEY hKey = NULL;
  g_szPropertyPath[0] = 0;
  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
    HKEY hSubKey = NULL;
    if ( RegOpenKeyEx( hKey, APPL_NAME, 0, KEY_ALL_ACCESS, &hSubKey ) == ERROR_SUCCESS )
    {
      DWORD dwType = REG_SZ;
      DWORD iSize = sizeof( g_szPropertyPath );
      int iSuccess = RegQueryValueEx( hSubKey, KEY_DRIVE, 0, &dwType, (LPBYTE)g_szPropertyPath, &iSize );
      if ( iSuccess == ERROR_SUCCESS )
      {
        dwType = REG_SZ;
        iSize = sizeof( g_szPropertyPath );
        strcat( g_szPropertyPath, "\\" );
        iSuccess = RegQueryValueEx( hSubKey, KEY_PATH, 0, &dwType, (LPBYTE)(g_szPropertyPath+strlen(g_szPropertyPath)), &iSize );
        if ( iSuccess != ERROR_SUCCESS )
        {
          g_szPropertyPath[0] = 0;
        }       
      }         
      RegCloseKey(hSubKey);
    }         
    RegCloseKey( hKey );
  }     
  if ( g_szPropertyPath[0] == 0 )
  {
	 MessageBox(NULL, "Could not access OpenTM2 registry information!", "Error!", MB_ICONEXCLAMATION | MB_OK);
	 return -1;
  }
  else
  {
    strcat( g_szPropertyPath, "\\PROPERTY" );
  } 
  return 0;
}

void centerWindow(HWND hwnd)
{
	HWND hParentOrOwner;
    RECT rcParent, rcWnd;
    int x, y;
    if ((hParentOrOwner = GetParent(hwnd)) == NULL)
    {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcParent, 0);
    }
    else
    {
        GetWindowRect(hParentOrOwner, &rcParent);
    }
    GetWindowRect(hwnd, &rcWnd);
    x = ((rcParent.right-rcParent.left) - (rcWnd.right-rcWnd.left)) / 2 + rcParent.left;
    y = ((rcParent.bottom-rcParent.top) - (rcWnd.bottom-rcWnd.top)) / 2 + rcParent.top;
    SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void timerEvent(HWND hwnd)
{
	HWND hControl;
	int iSend = 0;
	int iReceived = 0;
	int iMems = 0;
	char szBuffer[512]={'\0'};

	GetTransportStatistics( &iSend, &iReceived, &iMems );

	hControl = GetDlgItem(hwnd, IDC_EDIT_PROP_SEND );
	sprintf( szBuffer, "%ld", iSend );
	SetWindowText( hControl, szBuffer );

	hControl = GetDlgItem(hwnd, IDC_EDIT_PROP_RECEIVED );
	sprintf( szBuffer, "%ld", iReceived );
	SetWindowText( hControl, szBuffer );

	// refresh memory list window
	bool bChanged = getFileChangedFlag();
	if(bChanged)
	{
		setFileChangedFlag(false);

		std::vector<std::string> availMems;
	    listAllSharedMemories(availMems);

	    std::vector<std::string> selMems;
		HWND hwndSel = GetDlgItem(hwnd, IDC_LIST_SHMEM_SELECTED );
	    getListBoxItemx(hwndSel,selMems);

		fillSelectedList(hwnd,availMems,selMems);
		fillAvailableList(hwnd,availMems,selMems);
	}

}

void processCommandEvent(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2)
{
	 int controlID = LOWORD(mp1);
	 switch(controlID)
	 {
	 case ID_BTN_STOP:
		 PostMessage( hwndDlg, WM_CLOSE, 0, 0 );
		 break;

	 case IDC_BUTTON_TOSELECT:
		 addToSelected(hwndDlg);

		 sendSelectedMemoriesToGlobal(hwndDlg);

		 break;

	 case IDC_BUTTON_TOAVAIABLE:
		 addToAvailable(hwndDlg);

		 sendSelectedMemoriesToGlobal(hwndDlg);
		 break;

	 case IDC_LIST_SHMEM_SELECTED:
		 if( HIWORD(mp1) == LBN_DBLCLK)
		 {
			 addToAvailable(hwndDlg);

			sendSelectedMemoriesToGlobal(hwndDlg);

		 }
		 break;

	 case IDC_LIST_SHMEM_AVAIABLE:
		 if( HIWORD(mp1) == LBN_DBLCLK)
		 {
			 addToSelected(hwndDlg);
			 		 
			 sendSelectedMemoriesToGlobal(hwndDlg);
		 }
		 break;

	 case IDC_EDIT_REPLICATIONINTERVAL:
		 if(HIWORD(mp1) == EN_CHANGE)
		 {
			  HWND hwndRI = GetDlgItem(hwndDlg, IDC_EDIT_REPLICATIONINTERVAL );
		      char szBuffer[10+1]={'\0'};
	          GetWindowText( hwndRI, szBuffer, 10);
			  szBuffer[10]='\0';
			  if(strlen(szBuffer) == 0)
				  return;
			  int replicatorInterval = atoi(szBuffer);
			  if(replicatorInterval >0 )
		          setReplicatorInterval(replicatorInterval*1000);
		 }
		 break;
	 }
}

unsigned int __stdcall  MonitorThreadProc( void *pData )
{
    HANDLE hFind = ::FindFirstChangeNotification(g_szPropertyPath, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        return -1;
    }

    while (g_Monintor)
    {
        DWORD dwWait = ::WaitForSingleObject(hFind, 1000);
        if (WAIT_OBJECT_0 == dwWait)
        {
            setFileChangedFlag(true);
            if (!::FindNextChangeNotification(hFind))
            {
                ::FindCloseChangeNotification(hFind);
                hFind = NULL;
                return -2;
            }
        }
    }

    ::FindCloseChangeNotification(hFind);
    hFind = NULL;
    return 0;
}

void beginMonitorThread()
{
	hMonitorThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &MonitorThreadProc, NULL, 0, NULL));
}

void stopMonitorThread()
{
	g_Monintor = false;
    DWORD dwWait = ::WaitForSingleObject(hMonitorThread, 2000);
    if (WAIT_OBJECT_0 != dwWait)
    {
        ::TerminateThread(hMonitorThread, -1);
    }
    ::CloseHandle(hMonitorThread);
    hMonitorThread = NULL;
}
