//+----------------------------------------------------------------------------+
//| OTMCFXP.H                                                                  |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Header file for the CFXP class                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
// 
class CFXP
{
public:
	CFXP();
	~CFXP();
	int Open( const char *strPackageName );
	int Close();
//	long GetFirstFile( const char *strFilePattern ); // method not implemented yet
	long GetFirstFile( int iFileType );
	long GetNextFile();
	long GetFileByName( const char *pszFileName );
	int GetFileName( long lFileToken, char *pszFileName, int iBufSize );
	int GetFileSize( long lFileToken, int *piSize );
	int ReadFile( long lFileToken, void *pBuffer, int iBufSize );

  // return error message text of last error condition
  // RC = 0; successful
  // other: buffer to small, RC = size of required buffer
	int GetLastMessage( char *pszMessageBuffer, int iBufSize );

  // return codes
  enum 
  {
    FXP_INSUFFICIENT_STORAGE,
    FXP_OPEN_FAILED,
    FXP_INVALID_OPERATION,
    FXP_PACKAGE_INVALID
  } ReturnCodes;
protected:
  FILE *m_hfPackage;                   // handle of package file
  char m_szMessage[256];               // uffer for error messages
  PACKHEADER2 m_PackageHeader;         // buffer for package header    
  FILELIST   m_FileList;               // package's file list
  PFOLEXPHEADER m_pFolderHeader;       // ptr to exported folder header 
  ULONG m_ulNext;                      // next entry to continue search 
  BOOL m_fIsOpen;                      // package-is-open flag
  int  m_iFileType;                    // type of files being searched 
  BOOL m_fIsOpenTM2Folder;             // TRUE = package has been exported by OpenTM2

};
