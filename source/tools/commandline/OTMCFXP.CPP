//+----------------------------------------------------------------------------+
//| EQFCFXP.CPP                                                                |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Description: Interface to access exported folders (FXP)                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
// 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

extern "C"
{
  #define INCL_EQF_FOLDER           // folder list and document list functions

#pragma pack( push, TM2StructPacking, 1 )
  #include "eqf.h"                  // General .H for EQF
  #include <eqfdde.h>               // batch mode definitions
  #include <eqffol00.h>             // Private folder defines
  #include "eqfutpck.h"             // FXP package header file
#pragma pack( pop, TM2StructPacking )

}

#include "OTMCFXP.H"

// constructor
CFXP::CFXP()
{
  m_hfPackage = NULL;
  m_FileList.pEntries = NULL;
  m_FileList.pBuffer = NULL; 
  m_pFolderHeader = NULL;
  memset( &m_PackageHeader, 0, sizeof(m_PackageHeader) );
  m_szMessage[0] = '\0';
  m_ulNext = 0;
  m_fIsOpen = FALSE;
  m_iFileType = 0;
}

// destructor
CFXP::~CFXP()
{
}

// open a package file
int CFXP::Open( const char *strPackageName )
{
  int iRC = 0;

  m_szMessage[0] = '\0';

  // open package file
  m_hfPackage = fopen( strPackageName, "rb" );
  if ( !m_hfPackage )
  {
    iRC = FXP_OPEN_FAILED;
    sprintf( m_szMessage, "Error: Failed to open package file \"%s\"", strPackageName );
  } /* endif */

  // read system package header
  if ( !iRC )
  {
    int iRead = fread( &m_PackageHeader, 1, sizeof(m_PackageHeader), m_hfPackage );
    if ( iRead != sizeof(m_PackageHeader) )
    {
      sprintf( m_szMessage, "Error: Read of package header failed\n" );
      iRC = FXP_PACKAGE_INVALID;
    } /* endif */
  } /* endif */

  // convert package header to new format if necessary and correct file pointer position
  if ( !iRC )
  {
    if ( (memcmp( m_PackageHeader.bPackID, PACKHEADID, 4 ) == 0) ||
         (memcmp( m_PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) )
    {
      PACKHEADER *pOldPackHead = (PACKHEADER *)malloc(sizeof(PACKHEADER) );
      if ( pOldPackHead )
      {
        memcpy( pOldPackHead, &m_PackageHeader, sizeof(PACKHEADER) );
        memset( &m_PackageHeader, 0, sizeof(m_PackageHeader) );
        memcpy( &m_PackageHeader.bPackID, pOldPackHead->bPackID, 4 );
        m_PackageHeader.usVersion              = pOldPackHead->usVersion;
        m_PackageHeader.ulPackDate             = pOldPackHead->ulPackDate;
        m_PackageHeader.usSequence             = pOldPackHead->usSequence;
        m_PackageHeader.fCompleted             = pOldPackHead->fCompleted;
        m_PackageHeader.fLastFileOfPackage     = pOldPackHead->fLastFileOfPackage;
        m_PackageHeader.ulUserHeaderSize       = pOldPackHead->usUserHeaderSize;
        m_PackageHeader.ulFileListSize         = pOldPackHead->usFileListSize;
        m_PackageHeader.ulFileListEntries      = pOldPackHead->usFileListEntries;
        m_PackageHeader.ulFileNameBufferSize   = pOldPackHead->usFileNameBufferSize;

        fseek( m_hfPackage, sizeof(PACKHEADER), SEEK_SET ); // adjust file pointer position
      }
      else
      {
        sprintf( m_szMessage, "Error: Out of memory when allocating package header\n" );
        iRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  // check header info
  if ( !iRC )
  {
    if ( !m_PackageHeader.fCompleted )
    {
      sprintf( m_szMessage, "Error: The folder export did not complete, folder package is not usable" );
      iRC = FXP_PACKAGE_INVALID;
    }
    else if ( m_PackageHeader.usSequence != 1 )
    {
      sprintf( m_szMessage, "Error: Package is only part of an exported folder." );
      iRC = FXP_PACKAGE_INVALID;
    } 
    else
    {
      this->m_fIsOpenTM2Folder = stricmp( m_PackageHeader.szToolID, "OTM" ) == 0;
    } /* endif */
  } /* endif */

  // read user header
  if ( !iRC )
  {
    m_pFolderHeader = (PFOLEXPHEADER)malloc( m_PackageHeader.ulUserHeaderSize );

    if ( !m_pFolderHeader )
    {
      iRC = FXP_INSUFFICIENT_STORAGE;
      sprintf( m_szMessage, "Error: Memory allocation failed." );
    }
    else
    {
      // position to begin of user header
      if ( (memcmp( m_PackageHeader.bPackID, PACKHEADID, 4 ) == 0) ||
           (memcmp( m_PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) )
      {
        // version 0 or 1 header
        fseek( m_hfPackage, sizeof(PACKHEADER), SEEK_SET );
      }
      else
      {
        // version 2 header
        fseek( m_hfPackage, sizeof(PACKHEADER2), SEEK_SET );
      } /* endif */

      // read user header into the buffer
      if ( fread( m_pFolderHeader, m_PackageHeader.ulUserHeaderSize, 1, m_hfPackage ) != 1 )
      {
        iRC = FXP_PACKAGE_INVALID;
        sprintf( m_szMessage, "Error: Read of user header area failed" );
      } /* endif */
    } /* endif */
  } /* endif */

  // read file list
  if ( !iRC )
  {
    m_FileList.pEntries = (PFILELISTENTRY)malloc( m_PackageHeader.ulFileListSize );
    m_FileList.pBuffer  = (PSZ)malloc( m_PackageHeader.ulFileNameBufferSize );
    if ( (m_FileList.pEntries == NULL) || (m_FileList.pBuffer == NULL) )
    {
      sprintf( m_szMessage, "Error: Allocation of file list data area failed." );
      iRC = FXP_INSUFFICIENT_STORAGE;
    }
    else
    {
      LONG lPos = 0;

      // position to file list
      if ( (memcmp( m_PackageHeader.bPackID, PACKHEADID, 4 ) == 0) ||
           (memcmp( m_PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) )
      {
        // version 0 or 1 header
        lPos = sizeof(PACKHEADER);
      }
      else
      {
        // version 2 header
        lPos = sizeof(PACKHEADER2);
      } /* endif */
      lPos += m_PackageHeader.ulUserHeaderSize,
      fseek( m_hfPackage, lPos, SEEK_SET );

      // read file list and file names 
      if ( fread( m_FileList.pEntries, m_PackageHeader.ulFileListSize, 1, m_hfPackage ) != 1 )
      {
        iRC = FXP_PACKAGE_INVALID;
        sprintf( m_szMessage, "Error: Read of file list failed" );
      }
      else
      {
        m_FileList.ulListSize = m_FileList.ulListUsed = m_PackageHeader.ulFileListEntries;
        if ( fread( m_FileList.pBuffer, m_PackageHeader.ulFileNameBufferSize, 1, m_hfPackage ) != 1 )
        {
          iRC = FXP_PACKAGE_INVALID;
          sprintf( m_szMessage, "Error: Read of file list failed" );
        }
        else
        {
          ULONG       ulNoOfEntries;         // no of file list entries to process

          m_FileList.ulBufferSize = m_FileList.ulBufferUsed = m_PackageHeader.ulFileNameBufferSize;

          // change offset of file names to pointers 
          PFILELISTENTRY pFileEntry = m_FileList.pEntries;
          ulNoOfEntries = m_FileList.ulListUsed;
          while ( ulNoOfEntries )
          {
            ULONG ulOffs;
            ulOffs = (ULONG)pFileEntry->pszName;
            // fix for offsets corrupted bug!
            if ( ulOffs > m_FileList.ulBufferUsed )
            {
              ulOffs &= 0x0000FFFF;
            } /* endif */
            pFileEntry->pszName = m_FileList.pBuffer + ulOffs;
            pFileEntry->usProcessFlags = 0;
            pFileEntry++;
            ulNoOfEntries--;
          } /* endwhile */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !iRC )
  {
    m_fIsOpen = TRUE;
  } /* endif */
  return( iRC );
} /* end of CFXP::Open */

// close a package file
int CFXP::Close()
{
  int iRC = 0;

  m_szMessage[0] = '\0';

  if ( m_hfPackage )         fclose( m_hfPackage );
  m_hfPackage = NULL;
   
  if ( m_FileList.pEntries ) free( m_FileList.pEntries );
  m_FileList.pEntries = NULL;

  if ( m_FileList.pBuffer )  free( m_FileList.pBuffer );
  m_FileList.pBuffer = NULL; 

  if ( m_pFolderHeader )     free ( m_pFolderHeader );
  m_pFolderHeader = NULL;

  memset( &m_PackageHeader, 0, sizeof(m_PackageHeader) );
  m_fIsOpen = FALSE;

  return( iRC );
} /* end of CFXP::Close */

// get first file of a given type
long CFXP::GetFirstFile( int iFileType )
{
  long lFileToken = 0;
  ULONG ulI = 0;

  m_szMessage[0] = '\0';
  m_ulNext = 0;
  m_iFileType = 0;


  // special handling for TranslationManager folders: the file types have an offset of +2 starting with HISTLOG_DATA_FILE
  // because two file types had been removed from the enumeration PACKFILETYPES
  if ( !m_fIsOpenTM2Folder && (iFileType >= HISTLOG_DATA_FILE) ) iFileType += 2;

  if ( m_fIsOpen )
  {
    PFILELISTENTRY pFileEntry = m_FileList.pEntries;

    for ( ulI = 0; ulI < m_PackageHeader.ulFileListEntries; ulI++ )
    {
      if ( pFileEntry->usFileType == (USHORT)iFileType )
      {
        break;
      } /* endif */
      pFileEntry++;
    } /* endfor */
   
    if ( ulI < m_PackageHeader.ulFileListEntries )
    {
      lFileToken = ulI + 1;
      m_ulNext = ulI + 1;
      m_iFileType = iFileType;
    } /* endif */
  } /* endif */

  return( lFileToken );
} /* end of CFXP::GetFirstFile */

// get next file
long CFXP::GetNextFile()
{
  long lFileToken = 0;
  m_szMessage[0] = '\0';
  ULONG ulI;

  if ( m_fIsOpen && m_ulNext )
  {
    PFILELISTENTRY pFileEntry = m_FileList.pEntries + m_ulNext;

    for ( ulI = m_ulNext; ulI < m_PackageHeader.ulFileListEntries; ulI++ )
    {
      if ( pFileEntry->usFileType == (USHORT)m_iFileType )
      {
        break;
      } /* endif */
      pFileEntry++;
    } /* endfor */
   
    if ( ulI < m_PackageHeader.ulFileListEntries )
    {
      lFileToken = ulI + 1;
      m_ulNext = ulI + 1;
    }
    else
    {
      m_ulNext = 0;
    } /* endif */
  } /* endif */

  return( lFileToken );
} /* end of CFXP::GetNextFile */

// get file with the given name
long CFXP::GetFileByName( const char *pszFileName )
{
  long lFileToken = 0;
  ULONG ulI;

  m_szMessage[0] = '\0';

  if ( m_fIsOpen )
  {
    PFILELISTENTRY pFileEntry = m_FileList.pEntries;

    for ( ulI = 0; ulI < m_PackageHeader.ulFileListEntries; ulI++ )
    {
      if ( strcmp( pFileEntry->pszName, pszFileName ) == 0 )
      {
        break;
      } /* endif */
      pFileEntry++;
    } /* endfor */
   
    if ( ulI < m_PackageHeader.ulFileListEntries )
    {
      lFileToken = ulI + 1;
    } /* endif */
  } /* endif */

  return( lFileToken );
} /* end of CFXP::GetFileByName */

// get the full name of a file
int CFXP::GetFileName( long lFileToken, char *pszFileName, int iBufSize )
{
  int iRC = 0;

  m_szMessage[0] = '\0';

  if ( m_fIsOpen && (lFileToken > 0) && (lFileToken <= (long)m_PackageHeader.ulFileListEntries))
  {
    lFileToken--;
    PFILELISTENTRY pFileEntry = m_FileList.pEntries + lFileToken;
    strncpy( pszFileName, pFileEntry->pszName, iBufSize );
  }
  else
  {
    iRC = FXP_INVALID_OPERATION;
  } /* endif */

  return( iRC );
} /* end of CFXP::GetFileName */

// get the size of a file
int CFXP::GetFileSize( long lFileToken, int *piSize  )
{
  int iRC = 0;

  if ( m_fIsOpen && (lFileToken > 0) && (lFileToken <= (long)m_PackageHeader.ulFileListEntries))
  {
    lFileToken--;
    PFILELISTENTRY pFileEntry = m_FileList.pEntries + lFileToken;
    *piSize = (int)pFileEntry->ulFileSize;
  }
  else
  {
    iRC = FXP_INVALID_OPERATION;
  } /* endif */

  m_szMessage[0] = '\0';

  return( iRC );
} /* end of CFXP::GetFileSize */

// read a file into the supplied buffer area
int CFXP::ReadFile( long lFileToken, void *pBuffer, int iBufSize )
{
  int iRC = 0;
  m_szMessage[0] = '\0';

  int      iLen;

  if ( m_fIsOpen && (lFileToken > 0) && (lFileToken <= (long)m_PackageHeader.ulFileListEntries))
  {
    lFileToken--;
    PFILELISTENTRY pFileEntry = m_FileList.pEntries + lFileToken;

    // position to start of requested file in package file
    fseek( m_hfPackage, pFileEntry->ulFilePos, SEEK_SET );

    // read file into buffer
    memset( pBuffer, 0, iBufSize );
    iLen = min( iBufSize, (int)pFileEntry->ulFileSize );
    fread( pBuffer, iLen, 1, m_hfPackage );
  }
  else
  {
    iRC = FXP_INVALID_OPERATION;
  } /* endif */

  return( iRC );
} /* end of CFXP::ReadFile */

// return error message text of last error condition
// RC = 0; successful
// other: buffer to small, RC = size of required buffer
int CFXP::GetLastMessage( char *pszMessageBuffer, int iBufSize )
{
  int iRC = 0;
  int iLen = strlen( m_szMessage ) + 1;
  if ( iLen <= iBufSize)
  {
    strcpy( pszMessageBuffer, m_szMessage );
  }
  else
  {
    iRC = iLen;
  } /* endif */
  return( iRC );
} /* end of CFXP::GetLastMessage */
