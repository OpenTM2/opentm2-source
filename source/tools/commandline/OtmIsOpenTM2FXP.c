//+----------------------------------------------------------------------------+
//| OTMIsOpenTM2FXP.C                                                          |
//+----------------------------------------------------------------------------+
//| Copyright (c) 2014, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Tool to check if a FXP package has been exported by OpenTM2                |
//+----------------------------------------------------------------------------+
#undef  DLL
#undef  _WINDLL

#include <windows.h>
#include <stdio.h>

// values used in the szToolID field of the header
#define TM_TOOLID "TM"
#define OPENTM2_TOOLID "OTM"

// values for the package header ID
#define PACKHEADID  "EQF\x1A"           // ID of package headers
#define PACKHEAD2ID "EQF\x1C"          // ID of package headers - version 2
#define PACKHEAD3ID "EQF\x1D"          // ID of package headers - version 3

typedef USHORT EQF_BOOL;

// layout of package header in version 2 packages
typedef struct _PACKHEADER2
{
   BYTE    bPackID[4];                 // package ID bytes
   USHORT  usVersion;                  // version of package pack method
   ULONG   ulPackDate;                 // date when package was created
   USHORT  usSequence;                 // package sequence number
   EQF_BOOL fCompleted;                // package file has been completed flag
   EQF_BOOL fLastFileOfPackage;        // no more package files to follow
   ULONG   ulUserHeaderSize;           // size of user header
   ULONG   ulFileListSize;             // size of file list [Bytes]
   ULONG   ulFileListEntries;          // no of entries in file list
   ULONG   ulFileNameBufferSize;       // size of file name buffer [Bytes]
   CHAR    szToolID[4];                // ID of tool creating the package
   CHAR    chReserve[16];              // for future use
} PACKHEADER2, *PPACKHEADER2;

PACKHEADER2 PackageHeader;

int main( int argc, char **argv )
{
  FILE *hfPackage = NULL;

  // skip program name
  argc--; argv++;
  if ( argc == 0 ) return( 0 ); 

  // open package file
  hfPackage = fopen( *argv, "rb" );
  if ( hfPackage == NULL ) return( 0 ); 

  // read header
  memset( &PackageHeader, 0, sizeof(PackageHeader) );   
  fread( &PackageHeader, 1, sizeof(PackageHeader), hfPackage );   
  fclose( hfPackage  );

  // check header
  if ( ((memcmp( PackageHeader.bPackID, PACKHEADID, 4 ) == 0) ||
        (memcmp( PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) ||
        (memcmp( PackageHeader.bPackID, PACKHEAD3ID, 4 ) == 0)) &&
        (PackageHeader.usVersion >= 2) && 
        (strcmp( PackageHeader.szToolID, OPENTM2_TOOLID ) == 0 ) )
  {
    return( 1 );
  }
  else
  {
    // invalid package header ID or package not from OpenTM2
    return( 0 );
  }
}