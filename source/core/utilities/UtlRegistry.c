// helper functions to retrieve registry values
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#include "eqf.h"


// get a string from the registry
BOOL GetStringFromRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszBuffer, int iBufSize, PSZ pszDefault )
{
  BOOL fOK = FALSE;
  HKEY hKey = NULL;

  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
    HKEY hSubKey = NULL;
    if ( RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey ) == ERROR_SUCCESS )
    {
      DWORD dwType = REG_SZ;
      DWORD iSize = iBufSize;
      int iSuccess = RegQueryValueEx( hSubKey, pszKey, 0, &dwType, (LPBYTE)pszBuffer, &iSize );
      fOK = (iSuccess == ERROR_SUCCESS);
      RegCloseKey(hSubKey);
    } /* endif */        
    RegCloseKey( hKey );
  } /* endif */     

  if ( !fOK )
  {
    strcpy( pszBuffer, pszDefault );
  } /* endif */     
  return( fOK );
} /* end of function GetStringFromRegistry */

// get a integer from the registry
int GetIntFromRegistry( PSZ pszAppl, PSZ pszKey, int iDefault )
{
  BOOL fOK = FALSE;
  HKEY hKey = NULL;
  int iResult = 0;

  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
    HKEY hSubKey = NULL;
    if ( RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey ) == ERROR_SUCCESS )
    {
      DWORD dwType = REG_SZ;
      char szBuffer[20];
      DWORD iSize = sizeof(szBuffer);
      int iSuccess = RegQueryValueEx( hSubKey, pszKey, 0, &dwType, (LPBYTE)szBuffer, &iSize );
      if ( iSuccess == ERROR_SUCCESS )
      {
        if ( dwType == REG_DWORD )
        {
          fOK = TRUE;
          iResult = *((int *)szBuffer);
        } 
        else if ( dwType == REG_SZ )
        {
          fOK = TRUE;
          iResult = atol( szBuffer );
        } /* endif */           
      } /* endif */         
      RegCloseKey(hSubKey);
    } /* endif */        
    RegCloseKey( hKey );
  } /* endif */     
  if ( !fOK )
  {
    iResult = iDefault;
  } /* endif */     
  return( iResult );
} /* end of function GetIntFromRegistry */

// write a string to the registry
BOOL WriteStringToRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszValue )
{
  HKEY hKey = NULL;
  BOOL fOK = FALSE;

  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
      HKEY hSubKey = NULL;
      int iSuccess = RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey );
      if ( iSuccess != ERROR_SUCCESS )
      {
        DWORD dwDisp = 0;
        iSuccess = RegCreateKeyEx( hKey, pszAppl, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp );
      } /* endif */       

      if( iSuccess == ERROR_SUCCESS )
      {
        RegSetValueEx( hSubKey, pszKey, 0, REG_SZ, (LPBYTE)pszValue , strlen(pszValue )+ 1);
        RegCloseKey(hSubKey);
        fOK = TRUE;
      } /* endif */        

      RegCloseKey( hKey );
  } /* endif */     
  return( fOK );
} /* end of function WriteStringToRegistry */

// write a int to the registry
BOOL WriteIntToRegistry( PSZ pszAppl, PSZ pszKey, int iValue )
{
  HKEY hKey = NULL;
  BOOL fOK = FALSE;

  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
      HKEY hSubKey = NULL;
      int iSuccess = RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey );
      if ( iSuccess != ERROR_SUCCESS )
      {
        DWORD dwDisp = 0;
        iSuccess = RegCreateKeyEx( hKey, pszAppl, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp );
      } /* endif */       

      if( iSuccess == ERROR_SUCCESS )
      {
        RegSetValueEx( hSubKey, pszKey, 0, REG_DWORD, (LPBYTE)&iValue , sizeof(DWORD) );
        RegCloseKey(hSubKey);
        fOK = TRUE;
      } /* endif */        

      RegCloseKey( hKey );
  } /* endif */     
  return( fOK );
} /* end of function WriteIntToRegistry */
