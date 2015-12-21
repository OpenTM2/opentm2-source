//+----------------------------------------------------------------------------+
//|  EQFUTDOS.C - EQF DosXXX calls intefac functions                           |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:         G. Queck (QSoft)                                            |
//+----------------------------------------------------------------------------+
//|Description:    Interface to DosXXX OS/2 calls                              |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|   UtlOpen                DosOpen: Open a file                              |
//|   UtlClose               DosClose: Close a file                            |
//|   UtlRead                DosRead: Read data from a file                    |
//|   UtlWrite               DosWrite: Write data to a file                    |
//|   UtlDupHandle           DosDupHandle: Copy a file handle     ->removed    |
//|   UtlChgFilePtr          DosChgFilePtr: Change file pointer                |
//|   UtlDelete              DosDelete: Delete a file                          |
//|   UtlMkDir               DosMkDir: Create a directory                      |
//|   UtlRmDir               DosRmDir: Remove a directory                      |
//|   UtlCopy                DosCopy: Copy a file                              |
//|   UtlMove                DosMove: Move a file                              |
//|   UtlFindFirst           DosFindFirst: Find file(s)                        |
//|   UtlFindNext            DosFindNext: Find next file(s)                    |
//|   UtlFindClose           DosFindClose: Close a find handle                 |
//|   UtlQFileInfo           DosQFileInfo: Query file info                     |
//|   UtlQFSInfo             DosQFSInfo: Query file system info                |
//|   UtlSetFileInfo         DosSetFileInfo: Change file info                  |
//|   UtlSetFileMode         DosSetFileMode: Change file mode                  |
//|   UtlQPathInfo           DosQPathInfo: Query path info                     |
//|   UtlQFileMode           DosQFileMode: Query a file mode                   |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|     Windows Portation: HardResume (Hard Error pop up )                     |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.2 $ ----------- 1 Apr 2004
// --RJ: R008331: add GETMUTEX, RELEASEMUTEX in utlDelete
// 
//
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
//
//
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
//
//
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
//
//
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.4 $ ----------- 14 Dec 2001
// CV (12/14/01)
// -- UtlOpenHwnd(), handle also ERROR_SHARING_VIOLATION
//
//
// $Revision: 1.3 $ ----------- 19 Nov 2001
// GQ: - Correctly handle ERROR_ACCESS_DENIED condition in UtlOpenHwnd
//
//
// $Revision: 1.2 $ ----------- 19 Oct 2001
// --RJ: add DosInsMessageW for unicode enabling
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.5 $ ----------- 7 May 2001
// -- RJ: replace OemToAnsi by EQFOemToAnsi
//
//
// $Revision: 1.4 $ ----------- 30 Oct 2000
// FindFirst handling for root dirs fixed
//
//
// $Revision: 1.3 $ ----------- 21 Jun 2000
// - corrected open flag handling in Win32 part of UtlOpenFileHwnd
//
//
//
// $Revision: 1.2 $ ----------- 3 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFUTDOS.CV_   1.30   22 Nov 1999 18:47:56   BUILD  $
 *
 * $Log:   K:\DATA\EQFUTDOS.CV_  $
 *
 *    Rev 1.30   22 Nov 1999 18:47:56   BUILD
 * -- get rid of compiler warnings (VC6.0)
 *
 *    Rev 1.29   16 Mar 1999 09:15:20   BUILD
 * compile error
 *
 *    Rev 1.28   15 Mar 1999 12:18:50   BUILD
 * - added new functions UtlgetFileSize and UtlgetFileSizeHwnd
 *
 *    Rev 1.27   25 Feb 1999 09:43:54   BUILD
 * -- KBT454: increase number of file handles if necessary
 *
 *    Rev 1.26   15 Feb 1999 08:09:50   BUILD
 * -- reset handle if FindClose called ...
 *
 *    Rev 1.25   26 Oct 1998 18:42:34   BUILD
 * - corrected file attribute handling in functions UlFindFirstLong and
 *   UtlFindNextLong
 *
 *    Rev 1.24   12 Oct 1998 10:04:10   BUILD
 * - ignore long files in UtlFindFirst/UtlFindNext which have a long file name
 *   only (may be the cause for page violation problem on Warp server drive)
 *
 *    Rev 1.23   29 Sep 1998 07:30:14   BUILD
 * - UtlOpen: set file handle to NULLHANDLE if file system returns
 *   INVALID_HANDLE_VALUE
 *
 *    Rev 1.22   24 Jul 1998 16:38:26   BUILD
 * - UtlOpen: set file handle to NULLHANDLE if file system returns
 *   INVALID_HANDLE_VALUE
 *
 *    Rev 1.21   08 May 1998 13:59:58   BUILD
 * - Win32: fixed 'missing drive letter in error message' problem
 * - Win32: Reset error code for ERROR_DISK_FULL in function
 *   UtlWriteWoCheck to NO_ERROR and leave it to caller to handle
 *   disk full conditions correctly
 *
 *    Rev 1.20   26 Mar 1998 10:53:12   BUILD
 * - corrected default handling in UtlFindFirst if no file attributes are given
 *
 *    Rev 1.19   23 Mar 1998 16:05:38   BUILD
 * - corrected default handling in UtlFindFirst if no file attributes are given
 *
 *    Rev 1.18   23 Mar 1998 10:22:08   BUILD
 * - Win32: fixed bug in FindFirst/FindNext returning no normal files when called
 *   with FILE_NORMAL | FILE_DIRECTORY
 *
 *    Rev 1.17   23 Feb 1998 11:52:32   BUILD
 * - Win32: corrected drive handling in UtlQFSInfo function
 *
 *    Rev 1.16   16 Feb 1998 07:16:20   BUILD
 * - Win32: corrected share mode handling in UtlOpen
 * - Win32: corrected file attribute handling in UtlFindFirstLong and
 *   UtlFindNextLong
 *
 *    Rev 1.15   09 Feb 1998 17:22:08   BUILD
 * - corrected return code handling of CreateFile in Win32 environment
 * - map ERROR_FILE_NOT_FOUND condtion to ERROR_NO_MORE_FILES in FindFirst and
 *   FindNext in Win32 environment
 *
 *    Rev 1.14   14 Jan 1998 15:23:18   BUILD
 * - Rework for compile under Win32
 *
 *    Rev 1.13   01 Dec 1997 18:12:42   BUILD
 * - fixed PTM KBT0150: Shared memory problems with LAN authorization
 *
 *    Rev 1.12   30 Sep 1997 10:32:34   BUILD
 * - fixed bug in UtlGetArgs which caused a trap D if command line contained
 *   strings enclosed in double quotes.
 *
 *    Rev 1.11   01 Jul 1997 16:08:00   BUILD
 * ERROR_EQF_DRIVE_NOT_READY ADDED TO
 * UTLCOPYHWND2
 *
 *    Rev 1.10   15 Apr 1997 11:38:50   BUILD
 * - fixed PTM KAT0308: Not possible to export docs in external format to HPFS
 *
 *    Rev 1.9   25 Mar 1997 16:50:56   BUILD
 * - modified error checking after intdosx calls to handle down-level systems
 *   correctly
 *
 *    Rev 1.8   05 Mar 1997 09:34:42   BUILD
 * - corrected UtlFindFirstLong for OS/2 environment
 * - reset dos error code in long document enabled Dos... functions
 * - added new function UtlCopyHwnd2 (enhancement to UtlCopyHwnd)
 *
 *    Rev 1.7   26 Feb 1997 17:07:00   BUILD
 * -- Compiler defines for _POE22, _TKT21, and NEWTCSTUFF eliminated
 *
 *    Rev 1.6   24 Feb 1997 10:53:24   BUILD
 * - added new functions for long file name support
 *
 *    Rev 1.5   23 Oct 1996 10:47:54   BUILD
 * - modified UtlFileLocks routine in order to avoid NULL pointers
 *
 *    Rev 1.4   28 Mar 1996 10:07:06   BUILD
 * - removed unused preprocessor directives
 *
 *    Rev 1.3   18 Mar 1996 16:48:26   BUILD
 * -- KWT0462: display last msg of msg file...
 *
 *    Rev 1.1   04 Mar 1996 10:33:02   BUILD
 * - added code to restore wait cursor after errors in UtlCopy function
 *
 *    Rev 1.0   09 Jan 1996 09:20:32   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#include <direct.h>
#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "eqf.h"                       // includes our .h file
#include "eqfutpri.h"                  // private utility header file

#define MAX_OPEN_FILES    256          // max number of stored filehandles

typedef struct _EQFHANDLEDRIVES
{
  HANDLE      hf;                      // file handle
  // EOS in chDrive = End of array
  // BLANK in chDrive = empty entry
  CHAR        chDrive;                 // Drive belonging to handle
} EQFHANDLEDRIVES;
EQFHANDLEDRIVES DriveHandles[MAX_OPEN_FILES+1] = {{ NULLHANDLE, EOS} };

static CHAR chMsg[2048];               // buffer for messages
static HMODULE hMessagesMod = NULL;    // loaded DLL with message strings

// Array to keep track of file attributes used during UtlFindFirstFile
// as this ifnormation is not available in UtlFindNextFile but is required
// under Win32 as the file find function under Win32 does not allow search
// for files only or directories only

  #define MAX_FILEFINDHANDLES 20
  typedef struct _EQFOPENFILEFIND
  {
    HDIR      hdir;                      // file find handle
    USHORT   usAttr;          // attribute used in UtlFindFirst
    time_t   time;            // time of UtlFindFirst call
  } EQFOPENFILEFIND;
  EQFOPENFILEFIND OpenFileFinds[MAX_FILEFINDHANDLES+1] =
  { { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L},
    { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L},
    { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L},
    { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L}, { 0L, 0, 0L} };

USHORT UtlAddSearchHandle( HDIR hdirNew, USHORT  usAttr );
USHORT UtlAttrOfSearchHandle( HDIR hdir );
USHORT UtlRemoveSearchHandle( HDIR hdir );


// macro for checking the file attributes under Win32
  #define ISDIRATTR(a) ((a & FILE_ATTRIBUTE_DIRECTORY) != 0)
  #define ISFILEATTR(a) ((a & FILE_ATTRIBUTE_NORMAL) != 0)


/**********************************************************************/
/* prototype of static windows function...                            */
/**********************************************************************/

  #include "errno.h"                   // error number defines
  #include "eqfstart.mri"              // hard coded fatal-msg.
  #undef MAX_PATH

extern ERRDATA ErrData[];              // error structure

#define COPYFDATE( to, from ) \
  memcpy( (PVOID)&(to), (PVOID)&(from), sizeof(FDATE) )

#define COPYFTIME( to, from ) \
  memcpy( (PVOID)&(to), (PVOID)&(from), sizeof(FTIME) )

CHAR UtlGetDriveFromHandle( HFILE hf );

USHORT DosOpenLong
(
   PSZ      pszFname,                  // path name of file to be opened
   PHFILE   phfOpen,                   // file handle (returned )
   PUSHORT  pusAction,                 // action taken (returned)
   ULONG    ulFSize,                   // file's new size
   USHORT   usAttr,                    // file attribute
   USHORT   fsOpenFlags,               // action take if file exists
   USHORT   fsOpenMode                 // open mode
);

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlOpen                DosOpen: Open a file              |
//+----------------------------------------------------------------------------+
//|Function call:     UtlOpen( PSZ pszFname, PHFILE phfOpen, PUSHORT pusAction,|
//|                            ULONG ulFSize, USHORT usAttr, USHORT fsOpenFlags|
//|                            USHORT fsOpenMode, ULONG ulReserved, BOOL fMsg);|
//+----------------------------------------------------------------------------+
//|Description:       Interface function to UtlOpenHwnd                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFname       path name of file to open        |
//|                   ULONG    ulFSize        file's new size                  |
//|                   USHORT   usAttr         file attribute                   |
//|                   USHORT   fsOpenFlags    action take if file exists       |
//|                   USHORT   fsOpenMode     open mode                        |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PHFILE   phfOpen        file handle (returned )          |
//|                   PUSHORT  pusAction      action taken (returned)          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosOpen                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlOpenHwnd                                         |
//+----------------------------------------------------------------------------+
USHORT UtlOpen                                                            /*@3BA*/
(                                                                         /*@3BA*/
   PSZ      pszFname,                  // path name of file to be opened  /*@3BA*/
   PHFILE   phfOpen,                   // file handle (returned )         /*@3BA*/
   PUSHORT  pusAction,                 // action taken (returned)         /*@3BA*/
   ULONG    ulFSize,                   // file's new size                 /*@3BA*/
   USHORT   usAttr,                    // file attribute                  /*@3BA*/
   USHORT   fsOpenFlags,               // action take if file exists      /*@3BA*/
   USHORT   fsOpenMode,                // open mode                       /*@3BA*/
   ULONG    ulReserved,                // reserved (must be 0)            /*@3BA*/
   BOOL     fMsg                       // if TRUE handle errors in utility/*@3BA*/
)
{
   return( UtlOpenHwnd( pszFname,                                         /*@3BA*/
                        phfOpen,                                          /*@3BA*/
                        pusAction,                                        /*@3BA*/
                        ulFSize,                                          /*@3BA*/
                        usAttr,                                           /*@3BA*/
                        fsOpenFlags,                                      /*@3BA*/
                        fsOpenMode,                                       /*@3BA*/
                        ulReserved,                                       /*@3BA*/
                        fMsg,                                             /*@3BA*/
                        (HWND) NULL ) );                                         /*@3BA*/
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlOpenHwnd            DosOpen: Open a file              |
//+----------------------------------------------------------------------------+
//|Function call:     UtlOpenHwnd( PSZ pszFname, PHFILE phfOpen,               |
//|                                PUSHORT pusAction, ULONG ulFSize,           |                 |
//|                                USHORT usAttr, USHORT fsOpenFlags,          |
//|                                USHORT fsOpenMode, ULONG ulReserved,        |
//|                                BOOL fMsg, HWND hwndParent );               |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosOpen                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFname       path name of file to open        |
//|                   ULONG    ulFSize        file's new size                  |
//|                   USHORT   usAttr         file attribute                   |
//|                   USHORT   fsOpenFlags    action take if file exists       |
//|                   USHORT   fsOpenMode     open mode                        |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND     hwndParent     patent window handle             |
//+----------------------------------------------------------------------------+
//|Output parameter:  PHFILE   phfOpen        file handle (returned )          |
//|                   PUSHORT  pusAction      action taken (returned)          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosOpen                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosOpen                                           |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlOpenHwnd                                                        /*@3BC*/
(
   PSZ      pszFname,                  // path name of file to be opened
   PHFILE   phfOpen,                   // file handle (returned )
   PUSHORT  pusAction,                 // action taken (returned)
   ULONG    ulFSize,                   // file's new size
   USHORT   usAttr,                    // file attribute
   USHORT   fsOpenFlags,               // action take if file exists
   USHORT   fsOpenMode,                // open mode
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent

)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   BOOL   fTryCreate = TRUE;           // try-creation-of-file flag

   ulReserved;
   do {
      DosError(0);
      {
        DWORD dwAccess = 0;
        DWORD dwShareMode = 0;
        DWORD dwFlagsAndAttr = usAttr;

        // get open mode
        if ( fsOpenMode & OPEN_ACCESS_READONLY )
        {
            dwAccess |= GENERIC_READ;
        } /* endif */
        if ( fsOpenMode & OPEN_ACCESS_WRITEONLY )
        {
            dwAccess |= GENERIC_WRITE;
        } /* endif */
        if ( (fsOpenMode & OPEN_ACCESS_READWRITE) == OPEN_ACCESS_READWRITE)
        {
            dwAccess |= GENERIC_WRITE | GENERIC_READ;
        } /* endif */
        if ( dwAccess == 0 )
        {
            dwAccess = GENERIC_READ;   // use read access as default
        } /* endif */


        // share flags
        if ( !(fsOpenMode & OPEN_SHARE_DENYREAD) )
        {
          dwShareMode |= FILE_SHARE_READ;
        } /* endif */
        if ( !(fsOpenMode & OPEN_SHARE_DENYWRITE) )
        {
          dwShareMode |= FILE_SHARE_WRITE;
        } /* endif */

        // write through flag
        if ( fsOpenMode & OPEN_FLAGS_WRITE_THROUGH )
        {
          dwFlagsAndAttr |= FILE_FLAG_WRITE_THROUGH;
        }



        // open existing or create new file
        usRetCode = 1;                        // preset error condition
        if ( usRetCode && (fsOpenFlags & FILE_OPEN) )
        {
          *phfOpen = CreateFile( pszFname, dwAccess, dwShareMode, NULL,
                                 OPEN_EXISTING, dwFlagsAndAttr, NULL );
          usRetCode = (USHORT)((*phfOpen == INVALID_HANDLE_VALUE ) ?
                      (USHORT)GetLastError() : NO_ERROR);
          if ( (usRetCode == ERROR_ACCESS_DENIED)          ||
               (usRetCode == ERROR_SHARING_VIOLATION)      ||
               (usRetCode == ERROR_LOCK_VIOLATION)         ||
               (usRetCode == ERROR_SHARING_BUFFER_EXCEEDED) )
          {
            /************************************************************/
            /* File is currently in use, don't try to create it if      */
            /* FILE_CREATE has been specified                           */
            /************************************************************/
            fTryCreate = FALSE;
          }
          else
          {
            *pusAction = usRetCode ? 0 : FILE_EXISTED;
          } /* endif */
        } /* endif */

        if ( fTryCreate && usRetCode && (fsOpenFlags & FILE_TRUNCATE) )
        {
          *phfOpen = CreateFile( pszFname, dwAccess, dwShareMode, NULL,
                                 CREATE_ALWAYS, dwFlagsAndAttr, NULL );
          usRetCode = (USHORT)((*phfOpen == INVALID_HANDLE_VALUE ) ?
                      (USHORT)GetLastError() : NO_ERROR);
          *pusAction = usRetCode ? 0 : FILE_TRUNCATED;
        } /* endif */

        if ( fTryCreate && usRetCode && (usRetCode != ERROR_ACCESS_DENIED) && (usRetCode != ERROR_SHARING_VIOLATION) && (fsOpenFlags & FILE_CREATE) )
        {
          *phfOpen = CreateFile( pszFname, dwAccess, dwShareMode, NULL,
                                 CREATE_NEW, dwFlagsAndAttr, NULL );
          usRetCode = (USHORT)((*phfOpen == INVALID_HANDLE_VALUE ) ?
                      (USHORT)GetLastError() : NO_ERROR);
          /**************************************************************/
          /* if we are dealing with Os/2 server and DOS requester       */
          /* a file exist message is returning return code 2.           */
          /**************************************************************/
          if ( usRetCode == ERROR_FILE_NOT_FOUND  )
          {
            usRetCode = ERROR_FILE_EXISTS;
          } /* endif */

          *pusAction = usRetCode ? 0 : FILE_CREATED;
        } /* endif */

        // reset invalid handles to NULLHANDLE
        if ( *phfOpen == INVALID_HANDLE_VALUE )
        {
          *phfOpen = NULLHANDLE;
        } /* endif */

        /****************************************************************/
        /* check if we need to change the file size                     */
        /* In windows this has to be by hand - change filepointer, and  */
        /* force a write of (at least one byte) if we extend the file.  */
        /****************************************************************/
        if ( !usRetCode && ulFSize )
        {
          ULONG ulNOffset;
          ULONG  ulBytes;
          ULONG  ulWritten;

          usRetCode = UtlChgFilePtr( *phfOpen, 0, FILE_END, &ulNOffset, FALSE);
          if ( !usRetCode )
          {
            if ( ulNOffset < ulFSize )
            {
              ulFSize --;
              ulBytes = 1;
            }
            else
            {
              ulBytes = 0;
            } /* endif */
            usRetCode = UtlChgFilePtr( *phfOpen, ulFSize, FILE_BEGIN,
                                       &ulNOffset, FALSE);
            if ( !usRetCode )
            {
              usRetCode = UtlWriteL( *phfOpen, " ", ulBytes, &ulWritten, FALSE );
            } /* endif */

            if ( !usRetCode )
            {
              usRetCode = UtlChgFilePtr( *phfOpen, 0L, FILE_BEGIN,
                                         &ulNOffset, FALSE);
            } /* endif */
          } /* endif */
        } /* endif */
      }
      DosError(1);

      /**************************************************************/
      /* store drive for handle                                     */
      /**************************************************************/
      if ( !usRetCode )
      {
        // look for a free slot in our handle/drive array
        int i = 0;
        while ( i < MAX_OPEN_FILES )
        {
          if ( DriveHandles[i].chDrive == EOS )
          {
            // end of currently used entries detected
            // shift end to next entry and leave loop
            DriveHandles[i+1].chDrive = EOS;
            break;
          }
          else if ( DriveHandles[i].chDrive == BLANK )
          {
            // empty slot found, end loop
            break;
          }
          else
          {
            i++;
          } /* endif */
        } /* endwhile */
        if ( i < MAX_OPEN_FILES )
        {
          DriveHandles[i].hf = *phfOpen;
          DriveHandles[i].chDrive = *pszFname;
        } /* endif */
      } /* endif */
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszFname, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlClose               DosClose: Close a file            |
//+----------------------------------------------------------------------------+
//|Function call:     UtlClose( HFILE hf, BOOL fMsg );                         |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to UtlCloseHwnd                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosClose                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlCloseHwnd                                        |
//+----------------------------------------------------------------------------+
USHORT UtlClose
(
   HFILE    hf,                        // file handle
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlCloseHwnd( hf,
                         fMsg,
                         (HWND)NULL ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlCloseHwnd           DosClose: Close a file            |
//+----------------------------------------------------------------------------+
//|Function call:     UtlCloseHwnd (HFILE hf, BOOL fMsg, HWND hwndParent );    |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosClose                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosClose                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosClose                                          |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlCloseHwnd
(
   HFILE    hf,                        // file handle
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = 0;               // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   /*******************************************************************/
   /* close only if file has been opened                              */
   /*******************************************************************/
   if ( hf != (HFILE)NULL )
   {
     // remove handle from our handle/drive array
     int i = 0;
     while ( i < MAX_OPEN_FILES )
     {
       if ( DriveHandles[i].hf  == hf )
       {
         // mark as empty and leave loop
         DriveHandles[i].chDrive = BLANK;
         DriveHandles[i].hf = NULLHANDLE;
         break;
       }
       else
       {
         i++;
       } /* endif */
     } /* endwhile */

     do {
        DosError(0);

        if ( CloseHandle( hf ) == 0 )
        {
          usRetCode = (USHORT)GetLastError();
        } /* endif */

        DosError(1);
        if ( fMsg && usRetCode )
        {
           usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR,
                                    hwndParent);
        } /* endif */
     } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   } /* endif */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlRead            DosRead: Read data from a file        |
//+----------------------------------------------------------------------------+
//|Function call:     UtlRead( HFILE hf, PVOID pBuf, USHORT cbBuf,             |
//|                            PUSHORT pcbBytesRead, BOOL fMsg );              |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to UtlReadHwnd                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   PVOID    pBuf           input buffer                     |
//|                   USHORT   cbBuf          number of bytes to be read       |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pcbBytesRead   number of bytes read (returned)  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosRead                                   |
//+----------------------------------------------------------------------------+
//|Function flow:    call UtlReadHwnd                                          |
//+----------------------------------------------------------------------------+

USHORT UtlRead
(
   HFILE    hf,                        // file handle
   PVOID    pBuf,                      // input buffer
   USHORT   cbBuf,                     // number of bytes to be read
   PUSHORT  pcbBytesRead,              // number of bytes read (returned)
   BOOL     fMsg                    // if TRUE handle errors in utility
)
{
   ULONG  ulBuf = cbBuf;
   ULONG  ulBytesRead = *pcbBytesRead;
   USHORT  usRc = 0;

   usRc = UtlReadHwnd ( hf,
                         pBuf,
                         ulBuf,
                         &ulBytesRead,
                         fMsg,
                         (HWND)NULL ) ;
   *pcbBytesRead = (USHORT)ulBytesRead;
   return (usRc);
}

USHORT UtlReadL
(
   HFILE    hf,                        // file handle
   PVOID    pBuf,                      // input buffer
   ULONG    cbBuf,                     // number of bytes to be read
   PULONG   pcbBytesRead,              // number of bytes read (returned)
   BOOL     fMsg                    // if TRUE handle errors in utility
)
{
   return( UtlReadHwnd ( hf,
                         pBuf,
                         cbBuf,
                         pcbBytesRead,
                         fMsg,
                         (HWND)NULL ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlReadHwnd            DosRead: Read data from a file    |
//+----------------------------------------------------------------------------+
//|Function call:     UtlReadHwnd( HFILE hf, PVOID pBuf, USHORT cbBuf,         |
//|                                PUSHORT pcbBytesRead, BOOL fMsg,            |
//|                                HWND hwndParent );                          |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosRead                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   PVOID    pBuf           input buffer                     |
//|                   USHORT   cbBuf          number of bytes to be read       |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pcbBytesRead   number of bytes read (returned)  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosRead                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosRead                                           |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlReadHwnd
(
   HFILE    hf,                        // file handle
   PVOID    pBuf,                      // input buffer
   ULONG    cbBuf,                     // number of bytes to be read
   PULONG   pcbBytesRead,              // number of bytes read (returned)
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   do {
      DosError(0);

      {
        ULONG lRead;
        if ( ReadFile( hf, pBuf, cbBuf, &lRead, NULL ) == 0 )
        {
          usRetCode = (USHORT)GetLastError();
        }
        else
        {
          *pcbBytesRead = lRead;
        } /* endif */
      }

      DosError(1);
      if ( fMsg && usRetCode )
      {
        CHAR   szWork[2];

        szWork[0] = UtlGetDriveFromHandle( hf );
        if ( szWork[0] != EOS)
        {
          PSZ    pszDrive;
          szWork[1] = EOS;
          pszDrive = szWork;
          usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszDrive,
                                    DOS_ERROR, hwndParent );
        }
        else
        {
           usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL,
                                    DOS_ERROR, hwndParent );
        } /* endif */
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlWrite           DosWrite: Write data to a file        |
//+----------------------------------------------------------------------------+
//|Function call:     UtlWrite( HFILE hf, PVOID pBuf, USHORT cbBuf,            |
//|                             PUSHORT pcbBytesWritten, BOOL fMsg );          |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosWrite                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   PVOID    pBuf           output buffer                    |
//|                   USHORT   cbBuf          number of bytes to be written    |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pcbBytesWritten number of bytes written         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosWrite                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlWriteHwnd                                        |
//+----------------------------------------------------------------------------+

USHORT UtlWrite
(
   HFILE    hf,                        // file handle
   PVOID    pBuf,                      // output buffer
   USHORT    cbBuf,                     // number of bytes to be written
   PUSHORT   pcbBytesWritten,           // number of bytes written (returned)
   BOOL     fMsg                    // if TRUE handle errors in utility
)
{
   ULONG  ulBuf = cbBuf;
   ULONG  ulBytesWritten = *pcbBytesWritten;
   USHORT usRC = 0;

   usRC =  UtlWriteHwnd ( hf,
                         pBuf,
                         ulBuf,
                         &ulBytesWritten,
                         fMsg,
                         (HWND)NULL ) ;

   *pcbBytesWritten = (USHORT)ulBytesWritten;

  return (usRC);
}

USHORT UtlWriteL
(
   HFILE    hf,                        // file handle
   PVOID    pBuf,                      // output buffer
   ULONG    cbBuf,                     // number of bytes to be written
   PULONG   pcbBytesWritten,           // number of bytes written (returned)
   BOOL     fMsg                    // if TRUE handle errors in utility
)
{
   return( UtlWriteHwnd ( hf,
                         pBuf,
                         cbBuf,
                         pcbBytesWritten,
                         fMsg,
                         (HWND)NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlWriteHwnd           DosWrite: Write data to a file    |
//+----------------------------------------------------------------------------+
//|Function call:     UtlWriteHwnd( HFILE hf, PVOID pBuf, USHORT cbBuf,        |
//|                                 PUSHORT pcbBytesWritten, BOOL fMsg,        |
//|                                 HWND hwndParent );                         |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosWrite                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   PVOID    pBuf           output buffer                    |
//|                   USHORT   cbBuf          number of bytes to be written    |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pcbBytesWritten number of bytes written         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosWrite                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosWrite                                          |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlWriteHwnd
(
   HFILE    hf,                        // file handle
   PVOID    pBuf,                      // output buffer
   ULONG    cbBuf,                     // number of bytes to be written
   PULONG   pcbBytesWritten,           // number of bytes written (returned)
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   PSZ    pszDrive;
   CHAR   szWork[2];

   ULONG ulBytesWritten = 0;

   ULONG  ulTotBytesWritten = 0;

   do {
      DosError(0);
      if ( WriteFile( hf, pBuf, cbBuf, &ulBytesWritten, NULL ) == 0 )
      {
        usRetCode = (USHORT)GetLastError();
      } /* endif */

      DosError(1);

      if ( !usRetCode && ( cbBuf != ulBytesWritten ) )
          usRetCode = ERROR_DISK_FULL;

      /**************************************************************/
      /* adjust file pointer/number bytes written to allow retry in */
      /* case of disk full..                                        */
      /**************************************************************/
      if ( ulBytesWritten )
      {
        ulTotBytesWritten += ulBytesWritten;
        pBuf = (PBYTE)pBuf + ulBytesWritten;
        cbBuf -= ulBytesWritten;
      } /* endif */

      if ( fMsg && usRetCode )
      {
        szWork[0] = UtlGetDriveFromHandle( hf );
        if ( szWork[0] != EOS)
        {
          szWork[1] = EOS;
          pszDrive = szWork;
          usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszDrive,
                                    DOS_ERROR, hwndParent );
        }
        else
        {
           usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL,
                                    DOS_ERROR, hwndParent );
        } /* endif */
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */

   *pcbBytesWritten = ulTotBytesWritten;
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlWriteWoCheckHwnd    DosWrite: Write data to a file    |
//|                                          without check of *pcbBytesWritten |
//+----------------------------------------------------------------------------+
//|Function call:     UtlWriteWoCheckHwnd( HFILE hf, PVOID pBuf, USHORT cbBuf, |
//|                                 PUSHORT pcbBytesWritten, BOOL fMsg,        |
//|                                 HWND hwndParent );                         |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosWrite                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   PVOID    pBuf           output buffer                    |
//|                   USHORT   cbBuf          number of bytes to be written    |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pcbBytesWritten number of bytes written         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosWrite                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosWrite                                          |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlWriteWoCheckHwnd
(
   HFILE    hf,                        // file handle
   PVOID    pBuf,                      // output buffer
   ULONG    cbBuf,                     // number of bytes to be written
   PULONG   pcbBytesWritten,           // number of bytes written (returned)
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   do {
      DosError(0);

      {
        ULONG lWritten;
        if ( WriteFile( hf, pBuf, cbBuf, &lWritten, NULL ) == 0 )
        {
          usRetCode = (USHORT)GetLastError();
          if ( usRetCode == ERROR_DISK_FULL )
          {
            // reset error code and leave it to caller to detect and
            // handle a disk full condition
            usRetCode = NO_ERROR;
            *pcbBytesWritten = lWritten;
          } /* endif */
        }
        else
        {
          *pcbBytesWritten = lWritten;
        } /* endif */
      }

      DosError(1);
      if ( fMsg && usRetCode )
      {
        CHAR   szWork[2];
        szWork[0] = UtlGetDriveFromHandle( hf );
        if ( szWork[0] != EOS)
        {
          PSZ    pszDrive;

          szWork[1] = EOS;
          pszDrive = szWork;
          usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszDrive,
                                    DOS_ERROR, hwndParent );
        }
        else
        {
           usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL,
                                    DOS_ERROR, hwndParent );
        } /* endif */
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlChgFilePtr          DosChgFilePtr: Change file pointer|
//+----------------------------------------------------------------------------+
//|Function call:     UtlChgFilePtr( HFILE hf, LONG lOffset, USHORT fsMethod,  |
//|                                  PULONG pulNewOffset, BOOL fMsg);          |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosChgFilePtrHwnd                  |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   LONG     lOffset        distance to move in bytes        |
//|                   USHORT   fsMethod       method of moving                 |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PULONG   pulNewOffset   new pointer location             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosChgFilePtr                             |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlChgFilePtrHwnd                                   |
//+----------------------------------------------------------------------------+
USHORT UtlChgFilePtr
(
   HFILE    hf,                        // file handle
   LONG     lOffset,                   // distance to move in bytes
   USHORT   fsMethod,                  // method of moving
   PULONG   pulNewOffset,              // new pointer location (returned)
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlChgFilePtrHwnd( hf,
                              lOffset,
                              fsMethod,
                              pulNewOffset,
                              fMsg,
                              (HWND)NULL ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlChgFilePtrHwnd      DosChgFilePtr: Change file pointer|
//+----------------------------------------------------------------------------+
//|Function call:     UtlChgFilePtrHwnd( HFILE hf, LONG lOffset,               |
//|                                      USHORT fsMethod, PULONG pulNewOffset, |
//|                                      BOOL fMsg, HWND hwndParent);          |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosChgFilePtr                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   LONG     lOffset        distance to move in bytes        |
//|                   USHORT   fsMethod       method of moving                 |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PULONG   pulNewOffset   new pointer location             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosChgFilePtr                             |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosChgFilePtr                                     |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlChgFilePtrHwnd
(
   HFILE    hf,                        // file handle
   LONG     lOffset,                   // distance to move in bytes
   USHORT   fsMethod,                  // method of moving
   PULONG   pulNewOffset,              // new pointer location (returned)
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   LARGE_INTEGER liOffset;
   LARGE_INTEGER liNewOffset;
   BOOL  fOK = TRUE;

   liOffset.LowPart = lOffset;
   liOffset.HighPart = 0;
   liNewOffset.LowPart = 0;
   liNewOffset.HighPart = 0;


   do {
      DosError(0);

      fOK = SetFilePointerEx( hf, liOffset, &liNewOffset, fsMethod );

      *pulNewOffset = liNewOffset.LowPart;

      if ( !fOK )
      {
        usRetCode = (USHORT)GetLastError();
        *pulNewOffset = 0L;
      } /* endif */

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlDelete              DosDelete: Delete a file          |
//+----------------------------------------------------------------------------+
//|Function call:     UtlDelete( PSZ pszFName, ULONG ulReserved, BOOL fMsg );  |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to UtlDelete                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFName       name of file to be deleted       |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosDelete                                 |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlDeleteHwnd                                       |
//+----------------------------------------------------------------------------+

USHORT UtlDelete
(
   PSZ      pszFName,                  // name of file to be deleted
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlDeleteHwnd( pszFName,
                          ulReserved,
                          fMsg,
                          (HWND)NULL ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlDeleteHwnd          DosDelete: Delete a file          |
//+----------------------------------------------------------------------------+
//|Function call:     UtlDeleteHwnd( PSZ pszFName, ULONG ulReserved,           |
//|                                  BOOL fMsg, Hwnd hwndParent);              |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosDelete                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFName       name of file to be deleted       |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosDelete                                 |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     call UtlSetFileMode                                    |
//|                     disable OS/2 error handling                            |
//|                     call DosDelete                                         |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+

USHORT UtlDeleteHwnd
(
   PSZ      pszFName,                  // name of file to be deleted
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

  	 HANDLE hMutexSem = NULL;
     ulReserved;

	 // keep other process from doing property related stuff..
	 GETMUTEX(hMutexSem);

     UtlSetFileMode(pszFName, FILE_NORMAL, 0L, FALSE);
	 {

	   do {
		  DosError(0);

		  if ( DeleteFile( pszFName ) == 0 )
		  {
			usRetCode = (USHORT)GetLastError();
		  } /* endif */

		  DosError(1);
		  if ( fMsg && usRetCode )
		  {
			 usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszFName, DOS_ERROR, hwndParent );
		  } /* endif */
	   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */

      // release Mutex
      RELEASEMUTEX(hMutexSem);
    }
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlMkDir               DosMkDir: Create a directory      |
//+----------------------------------------------------------------------------+
//|Function call:     UtlMkDir( PSZ pszDirName, ULONG ulReserved, BOOL fMsg ); |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosMkDir                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszDirName     new directory name               |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosMkDir                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlMkDirHwnd                                        |
//+----------------------------------------------------------------------------+

USHORT UtlMkDir
(
   PSZ      pszDirName,                // new directory name
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlMkDirHwnd( pszDirName,
                         ulReserved,
                         fMsg,
                         (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlMkDirHwnd           DosMkDir: Create a directory      |
//+----------------------------------------------------------------------------+
//|Function call:     UtlMkDirHwnd( PSZ pszDirName, ULONG ulReserved,          |
//|                                 BOOL fMsg, HWND hwndParent);               |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosMkDir                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszDirName     new directory name               |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosMkDir                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosMkDir                                          |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlMkDirHwnd
(
   PSZ      pszDirName,                // new directory name
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   ulReserved;
   if ( !UtlDirExist( pszDirName  ) )
   {
     do {
        DosError(0);

        if ( CreateDirectory( pszDirName, NULL ) == 0 )
        {
          usRetCode = (USHORT)GetLastError();
        } /* endif */

        DosError(1);
        if ( fMsg && usRetCode )
        {
           usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszDirName, DOS_ERROR, hwndParent );
        } /* endif */
     } while ( fMsg &&
               usRetCode &&
               (usMBCode == MBID_RETRY) ); /* enddo */
   } /* endif */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlRmDir               DosRmDir: Remove a directory      |
//+----------------------------------------------------------------------------+
//|Function call:     UtlRmDir( PSZ pszDir, ULONG ulReserved, BOOL fMsg );     |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosRmDir                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszDir         directory name                   |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosRmDir                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     call of UtlRmDirHwnd                                     |
//+----------------------------------------------------------------------------+

USHORT UtlRmDir
(
   PSZ      pszDir,                    // new directory name
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlRmDirHwnd( pszDir,
                         ulReserved,
                         fMsg,
                         (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlRmDirHwnd           DosRmDir: Remove a directory      |
//+----------------------------------------------------------------------------+
//|Function call:     UtlRmDirHwnd( PSZ pszDir, ULONG ulReserved,              |
//|                                 BOOL fMsg, HWND hwndParent);               |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosRmDir                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszDir         directory name                   |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosRmDir                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosRmDir                                          |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlRmDirHwnd
(
   PSZ      pszDir,                    // directory name
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   ulReserved;
   do {
      DosError(0);

      if ( RemoveDirectory( pszDir ) == 0 )
      {
        usRetCode = (USHORT)GetLastError();
      } /* endif */

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszDir, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlCopy                DosCopy: Copy a file              |
//+----------------------------------------------------------------------------+
//|Function call:     UtlCopy( PSZ pszSrc, PSZ pszDst, USHORT usOpt,           |
//|                            ULONG ulReserved, BOOL fMsg );                  |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosCopy                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszSrc         old path name                    |
//|                   PSZ      pszDst         new path name                    |
//|                   USHORT   usOpt          operation mode                   |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosCopy                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     call of UtlCopyHwnd                                      |
//+----------------------------------------------------------------------------+

USHORT UtlCopy
(
   PSZ      pszSrc,                    // old path name
   PSZ      pszDst,                    // new path name
   USHORT   usOpt,                     // operation mode
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlCopyHwnd2( pszSrc,
                         pszDst,
                         usOpt,
                         ulReserved,
                         fMsg,
                         NULL,
                         (HWND)NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlCopyHwnd            DosCopy: Copy a file              |
//+----------------------------------------------------------------------------+
//|Function call:     UtlCopyHwnd( PSZ pszSrc, PSZ pszDst, USHORT usOpt,       |
//|                                ULONG ulReserved, BOOL fMsg,                |
//|                                HWND hwndParent );                          |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosCopy                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszSrc         old path name                    |
//|                   PSZ      pszDst         new path name                    |
//|                   USHORT   usOpt          operation mode                   |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosCopy                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlCopyHwnd2                                        |
//+----------------------------------------------------------------------------+
USHORT UtlCopyHwnd
(
   PSZ      pszSrc,                    // old path name
   PSZ      pszDst,                    // new path name
   USHORT   usOpt,                     // operation mode
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   return( UtlCopyHwnd2( pszSrc,
                         pszDst,
                         usOpt,
                         ulReserved,
                         fMsg,
                         NULL,
                         hwndParent ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlCopyHwnd2           DosCopy: Copy a file              |
//+----------------------------------------------------------------------------+
//|Function call:     UtlCopyHwnd2( PSZ pszSrc, PSZ pszDst, USHORT usOpt,      |
//|                                 ULONG ulReserved, BOOL fMsg,               |
//|                                 PUSHORT pusRCList,                         |
//|                                 HWND hwndParent );                         |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosCopy                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszSrc         old path name                    |
//|                   PSZ      pszDst         new path name                    |
//|                   USHORT   usOpt          operation mode                   |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   PUSHORT  pusRCList      list of return codes which       |
//|                                           should be returned to the caller |
//|                                           without popping-up a message box |
//|                                           the last entry in the list must  |
//|                                           be 0                             |
//|                                           If not used this parameter should|
//|                                           be NULL                          |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosCopy                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosCopy                                           |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured and error not in requested|
//|                        return code list                                    |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlCopyHwnd2
(
   PSZ      pszSrc,                    // old path name
   PSZ      pszDst,                    // new path name
   USHORT   usOpt,                     // operation mode
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg,                      // if TRUE handle errors in utility
   PUSHORT  pusRCList,                 // list of return codes or NULL
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   HDIR   hDirHandle ;                 // DosFind routine handle
   FILEFINDBUF ResultBuf;              // DOS file find struct
   USHORT  usCount = 1;
   USHORT  usRcCode;                   // second

   do {
      DosError(0);

      usRetCode = DosCopy( pszSrc, pszDst, usOpt, ulReserved );
      DosError(1);
      if ( fMsg && usRetCode )
      {
        // check return codes handled by calling function
        usMBCode = MBID_YES;
        if ( pusRCList != NULL )
        {
          PUSHORT pusRC = pusRCList;
          while ( *pusRC && (usMBCode == MBID_YES) )
          {
            if ( *pusRC == usRetCode )
            {
              usMBCode = MBID_CANCEL;  //return to calling function
            }
            else
            {
              pusRC++;                 // try next entry in list
            } /* endif */
          } /* endwhile */
        } /* endif */

        if ( usMBCode != MBID_CANCEL )
        {
          HPOINTER hPtr = GETCURSOR();

          if ( usRetCode == ERROR_NOT_READY     ||
               usRetCode == ERROR_WRITE_PROTECT ||
               usRetCode == ERROR_DISK_CHANGE   ||
               usRetCode == ERROR_DISK_FULL     ||
               usRetCode == ERROR_FILE_EXISTS   ||
               usRetCode == ERROR_ACCESS_DENIED ||
               usRetCode == ERROR_ACCESS_DENIED ||
               usRetCode == ERROR_PATH_NOT_FOUND ||
               usRetCode == ERROR_DIRECTORY ||
               usRetCode == ERROR_INVALID_DRIVE          )
          {
            /************************************************************/
            /* error occured in target drive                            */
            /************************************************************/
            if ( usRetCode == ERROR_NOT_READY   ||
                 usRetCode == ERROR_PATH_NOT_FOUND ||
                 usRetCode == ERROR_ACCESS_DENIED    )           /*(PTM KIT0840)*/
            {
              hDirHandle = HDIR_CREATE;
              usRcCode = UtlFindFirstHwnd( pszSrc, &hDirHandle, FILE_NORMAL,
                                           &ResultBuf, sizeof( ResultBuf),
                                           &usCount, 0L, FALSE, hwndParent);
              UtlFindCloseHwnd( hDirHandle, FALSE, hwndParent );
              // check if error occured in source or target drive
              if ( usRcCode == ERROR_NOT_READY      ||
                   usRcCode == ERROR_PATH_NOT_FOUND ||
                   usRcCode == ERROR_ACCESS_DENIED    )
              {
                usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszSrc, DOS_ERROR, hwndParent );
              }
              else
              {
                usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszDst, DOS_ERROR, hwndParent );
              } /* endif */
            }
            else
            {
              usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszDst, DOS_ERROR, hwndParent );
              if ( usMBCode == MBID_RETRY && usRetCode == ERROR_DISK_CHANGE)
              {
                  UtlSetDrive( *pszDst);
              } /* endif */
            } /* endif */
          } /* endif */
          else
          {
            usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszSrc, DOS_ERROR, hwndParent );
          } /* endif */
          SETCURSORFROMHANDLE(hPtr);
        } /* endif */
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlMove                DosMove: Move a file              |
//+----------------------------------------------------------------------------+
//|Function call:     UtlMove( PSZ pszSrc, PSZ pszDst, ULONG ulReserved,       |
//|                            BOOL fMsg );                                    |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosMove                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszSrc         old path name                    |
//|                   PSZ      pszDst         new path name                    |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosMove                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     call of UtlMoveHwnd                                      |
//+----------------------------------------------------------------------------+

USHORT UtlMove
(
   PSZ      pszSrc,                    // old path name
   PSZ      pszDst,                    // new path name
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlMoveHwnd( pszSrc,
                        pszDst,
                        ulReserved,
                        fMsg,
                        (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlMoveHwnd            DosMove: Move a file              |
//+----------------------------------------------------------------------------+
//|Function call:     UtlMoveHwnd( PSZ pszSrc, PSZ pszDst, ULONG ulReserved,   |
//|                                BOOL fMsg, HWND hwndParent );               |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosMove                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszSrc         old path name                    |
//|                   PSZ      pszDst         new path name                    |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosMove                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     call UtlSetFileMode                                    |
//|                     disable OS/2 error handling                            |
//|                     call DosMove                                           |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlMoveHwnd
(
   PSZ      pszSrc,                    // old path name
   PSZ      pszDst,                    // new path name
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   ulReserved;
   UtlSetFileMode(pszSrc, FILE_NORMAL, 0L, FALSE);

   do {
      DosError(0);

      if ( MoveFile( pszSrc, pszDst ) == 0 )
      {
        usRetCode = (USHORT)GetLastError();
      } /* endif */

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszSrc, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFindFirst       DosFindFirst: Find file(s)            |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFindFirst( PSZ pszFSpec, PHDIR phdir, USHORT usAttr,  |
//|                                 PFILEFINDBUF pffb, USHORT cbBuf,           |
//|                                 PUSHORT pcSearch, ULONG ulReserved,        |
//|                                 BOOL fMsg );                               |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFindFirst                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFSpec       path name of files to be found   |
//|                   PHDIR    phdir          directory handle                 |
//|                   USHORT   usAttr         attribute used to search for file|
//|                   USHORT   cbBuf          length of result buffer          |
//|                   PUSHORT  pcSearch       number of entries to find        |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PFILEFINDBUF pffb       result buffer                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFindFirst                              |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirstHwnd                                    |
//+----------------------------------------------------------------------------+


USHORT UtlFindFirst
(
   PSZ      pszFSpec,                  // path name of files to be found
   PHDIR    phdir,                     // directory handle
   USHORT   usAttr,                    // attribute used to search for the files
   PFILEFINDBUF pffb,                  // result buffer
   USHORT   cbBuf,                     // length of result buffer
   PUSHORT  pcSearch,             // number of matching entries in result buffer
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlFindFirstHwnd( pszFSpec,
                             phdir,
                             usAttr,
                             pffb,
                             cbBuf,
                             pcSearch,
                             ulReserved,
                             fMsg,
                             (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFindFirstHwnd       DosFindFirst: Find file(s)        |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFindFirstHwnd( PSZ pszFSpec, PHDIR phdir,             |
//|                                     USHORT usAttr, PFILEFINDBUF pffb,      |
//|                                     USHORT cbBuf, PUSHORT pcSearch,        |
//|                                     ULONG ulReserved, BOOL fMsg,           |
//|                                     HWND hwndParent );                     |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFindFirst                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFSpec       path name of files to be found   |
//|                   PHDIR    phdir          directory handle                 |
//|                   USHORT   usAttr         attribute used to search for file|
//|                   USHORT   cbBuf          length of result buffer          |
//|                   PUSHORT  pcSearch       number of entries to find        |
//|                   ULONG    ulReserved     reserved (must be 0)             |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PFILEFINDBUF pffb       result buffer                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFindFirst                              |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosFindFirst                                      |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlFindFirstHwnd
(
   PSZ      pszFSpec,                  // path name of files to be found
   PHDIR    phdir,                     // directory handle
   USHORT   usAttr,                    // attribute used to search for the files
   PFILEFINDBUF pffb,                  // result buffer
   USHORT   cbBuf,                     // length of result buffer
   PUSHORT  pcSearch,             // number of matching entries in result buffer
   ULONG    ulReserved,                // reserved (must be 0)
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   USHORT usSearch = 0;                    // number of entries searched
   HDIR   hdirNew;                     // new directory handle
   PSZ    pszFSpecCompl;
   CHAR   szFSpec[MAX_LONGPATH];

   cbBuf; ulReserved;
   if (pszFSpec)
   {
      strcpy(szFSpec, pszFSpec);

      // add '*.*' if path ends with '\' (normally root) e.g. "C:\"
      // otherwise root directory isn't recognised

      if (szFSpec[strlen(szFSpec)-1] == '\\')
         strcat(szFSpec, "*.*");

      pszFSpecCompl = szFSpec;
   }
   else
   {
      //pszFSpecCompl = pszFSpec;
      return (ERROR_INVALID_DATA);
   }

   do {
      DosError(0);

       if ( usAttr == 0 )
       {
         usAttr = FILE_ATTRIBUTE_NORMAL; // use normal file as default
       } /* endif */
       hdirNew = FindFirstFile( pszFSpecCompl, pffb );
       if ( hdirNew == INVALID_HANDLE_VALUE )
       {
         usRetCode = (USHORT)GetLastError();
         if ( usRetCode == ERROR_FILE_NOT_FOUND )
              usRetCode = ERROR_NO_MORE_FILES;
         usSearch = 0;
       }
       else
       {
         // look for files until the correct file type has been found
         // or no more files are available
         // caution: FILE_NORMAL is not contained in the returned attributes
         BOOL fFound = FALSE;

         fFound = (ISDIRATTR(usAttr) & ISDIRATTR(pffb->dwFileAttributes)) ||
                  (ISFILEATTR(usAttr) & !ISDIRATTR(pffb->dwFileAttributes));
         if ( fFound)
         {
           // check if 8.3 name is available
           if ( pffb->cAlternateFileName[0] == EOS )
           {
             // exactly check for TRUE as UtlIsLongName returns 2 for
             // short but mixed case file names
             if ( (UtlIsLongFileName( pffb->cFileName ) == TRUE) &&
                  (strcmp(pffb->cFileName,"..") != 0) )
             {
               fFound = FALSE; // ignore file with long name
             } /* endif */
           } /* endif */
         } /* endif */
         while ( (usRetCode == NO_ERROR) && !fFound )
         {
            if ( FindNextFile( hdirNew, pffb ) == 0 )
            {
              usRetCode = (USHORT)GetLastError();
              if ( usRetCode == ERROR_FILE_NOT_FOUND )
                usRetCode = ERROR_NO_MORE_FILES;
            }
            else
            {
              fFound = (ISDIRATTR(usAttr) & ISDIRATTR(pffb->dwFileAttributes)) ||
                        (ISFILEATTR(usAttr) & !ISDIRATTR(pffb->dwFileAttributes));
              if ( fFound)
              {
               // check if 8.3 name is available
                if ( pffb->cAlternateFileName[0] == EOS )
                {
                  // exactly check for TRUE as UtlIsLongName returns 2 for
                  // short but mixed case file names
                  if ( (UtlIsLongFileName( pffb->cFileName ) == TRUE) &&
                       (strcmp(pffb->cFileName,"..") != 0) )
                  {
                   fFound = FALSE; // ignore file with long name
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */
         } /* endwhile */

         if ( usRetCode == NO_ERROR )
         {
           usSearch = 1;
           // use normal file name if no alternate file name (8.3) is given
           if ( pffb->cAlternateFileName[0] == EOS )
           {
              strcpy( pffb->cAlternateFileName, pffb->cFileName );
           } /* endif */

           // add file search handle and attributes in our list of open
           // file search handles
           UtlAddSearchHandle( hdirNew, usAttr );
         }
         else
         {
           // close file handle as it will not be used anymore
            FindClose( hdirNew );
            hdirNew = HDIR_CREATE;
         } /* endif */
       } /* endif */

      DosError(1);
      if ( fMsg && usRetCode && (usRetCode != ERROR_NO_MORE_FILES) )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszFSpecCompl, DOS_ERROR, hwndParent );
         /*************************************************************/
         /* change the drive if necessary ...                         */
         /*************************************************************/
         if ( usMBCode == MBID_RETRY && usRetCode == ERROR_DISK_CHANGE)
         {
           UtlSetDrive( *pszFSpecCompl);
         } /* endif */
      } /* endif */
   } while ( fMsg &&
             usRetCode &&
             (usRetCode != ERROR_NO_MORE_FILES) &&
             (usMBCode == MBID_RETRY) ); /* enddo */
   *phdir = hdirNew;
   *pcSearch = (usRetCode == NO_ERROR ) ? usSearch : 0;
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFindNext            DosFindNext: Find next file(s)    |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFindNext( HDIR hdir, PFILEFINDBUF pffb, USHORT cbBuf, |
//|                                PUSHORT pcSearch, BOOL fMsg );              |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFindNext                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   HDIR     hdir           directory handle                 |
//|                   USHORT   cbBuf          length of result buffer          |
//|                   PUSHORT  pcSearch       number of entries to find        |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PFILEFINDBUF pffb       result buffer                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFindNext                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call of UtlFindNextHwnd                                  |
//+----------------------------------------------------------------------------+

USHORT UtlFindNext
(
   HDIR     hdir,                      // directory handle
   PFILEFINDBUF pffb,                  // result buffer
   USHORT   cbBuf,                     // length of result buffer
   PUSHORT  pcSearch,                  // number of entries to find
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlFindNextHwnd( hdir,
                            pffb,
                            cbBuf,
                            pcSearch,
                            fMsg,
                            (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFindNextHwnd        DosFindNext: Find next file(s)    |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFindNextHwnd( HDIR hdir, PFILEFINDBUF pffb,           |
//|                                    USHORT cbBuf, PUSHORT pcSearch,         |
//|                                    BOOL fMsg, HWND hwndParent );           |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFindNext                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   HDIR     hdir           directory handle                 |
//|                   USHORT   cbBuf          length of result buffer          |
//|                   PUSHORT  pcSearch       number of entries to find        |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PFILEFINDBUF pffb       result buffer                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFindNext                               |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosFindNext                                       |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlFindNextHwnd
(
   HDIR     hdir,                      // directory handle
   PFILEFINDBUF pffb,                  // result buffer
   USHORT   cbBuf,                     // length of result buffer
   PUSHORT  pcSearch,                  // number of entries to find
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   USHORT usSearch;                    // number of entries searched
   USHORT usAttr;                      // file attribute for this file search
   BOOL   fFound = FALSE;              // matching file was found flag

   cbBuf;
   do {
      DosError(0);

      // get file attribute flags active for this search handle
      usAttr = UtlAttrOfSearchHandle( hdir );

      // look for files until the correct file type has been found
      // or no more files are available
      do
      {
        if ( FindNextFile( hdir, pffb ) == 0 )
        {
          usRetCode = (USHORT)GetLastError();
          if ( usRetCode == ERROR_FILE_NOT_FOUND ) usRetCode = ERROR_NO_MORE_FILES;
        }
        else
        {
         fFound = (ISDIRATTR(usAttr) & ISDIRATTR(pffb->dwFileAttributes)) ||
                  (ISFILEATTR(usAttr) & !ISDIRATTR(pffb->dwFileAttributes));
         if ( fFound)
         {
           // check if 8.3 name is available
           if ( pffb->cAlternateFileName[0] == EOS )
           {
             // exactly check for TRUE as UtlIsLongName returns 2 for
             // short but mixed case file names
             if ( (UtlIsLongFileName( pffb->cFileName ) == TRUE) &&
                  (strcmp(pffb->cFileName,"..") != 0) )
             {
               fFound = FALSE; // ignore file with long name
             } /* endif */
           } /* endif */
         } /* endif */
        } /* endif */
      }
      while ( (usRetCode == NO_ERROR) && !fFound );

      if ( usRetCode == NO_ERROR )
      {
         // use normal file name if no alternate file name (8.3) is given
         if ( pffb->cAlternateFileName[0] == EOS )
         {
            strcpy( pffb->cAlternateFileName, pffb->cFileName );
         } /* endif */
      } /* endif */
      usSearch = ( usRetCode ) ? 0 : 1;
      DosError(1);
      if ( fMsg && usRetCode && (usRetCode != ERROR_NO_MORE_FILES) )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg &&
             usRetCode &&
             (usRetCode != ERROR_NO_MORE_FILES) &&
             (usMBCode == MBID_RETRY) ); /* enddo */
   *pcSearch = (usRetCode == NO_ERROR ) ? usSearch : 0;
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFindClose           DosFindClose: Close a find handle |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFindClose( HDIR hdir, BOOL fMsg );                    |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFindClose                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HDIR     hdir           directory handle                 |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFindClose                              |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindCloseHwnd                                    |
//+----------------------------------------------------------------------------+

USHORT UtlFindClose
(
   HDIR     hdir,                      // directory handle
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlFindCloseHwnd( hdir,
                             fMsg,
                             (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFindCloseHwnd       DosFindClose: Close a find handle |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFindCloseHwnd( HDIR hdir, BOOL fMsg,HWND hwndParent); |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFindClose                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HDIR     hdir           directory handle                 |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFindClose                              |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosFindClose                                      |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlFindCloseHwnd
(
   HDIR     hdir,                      // directory handle
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code

   fMsg; hwndParent;
   if ( FindClose( hdir ) == 0 )
   {
     usRetCode = (USHORT)GetLastError();
   } /* endif */

   // remove file search handle from our list of open
   // file search handles
   UtlRemoveSearchHandle( hdir );

   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlQFileInfo           DosQFileInfo: Query file info     |
//+----------------------------------------------------------------------------+
//|Function call:     UtlQFileInfo( HFILE hf, USHORT usInfoLevel,              |
//|                                 PBYTE pInfoBuf, USHORT cbInfoBuf,          |
//|                                 BOOL fMsg);                                |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosQFileInfo                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   USHORT   usInfoLevel    level of file information        |
//|                   USHORT   cbInfoBuf      length of storage area           |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PBYTE    pInfoBuf       storage area for information     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosQFileInfo                              |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlQfileInfoHwnd                                    |
//+----------------------------------------------------------------------------+
USHORT UtlQFileInfo
(
   HFILE    hf,                        // file handle
   USHORT   usInfoLevel,               // level of file information required
   PBYTE    pInfoBuf,                  // storage area for returned information
   USHORT   cbInfoBuf,                 // length of storage area
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlQFileInfoHwnd( hf,
                             usInfoLevel,
                             pInfoBuf,
                             cbInfoBuf,
                             fMsg,
                             (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlQFileInfoHwnd       DosQFileInfo: Query file info     |
//+----------------------------------------------------------------------------+
//|Function call:     UtlQFileInfoHwnd( HFILE hf, USHORT usInfoLevel,          |
//|                                     PBYTE pInfoBuf, USHORT cbInfoBuf,      |
//|                                     BOOL fMsg, HWND hwndParent );          |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosQFileInfo                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   USHORT   usInfoLevel    level of file information        |
//|                   USHORT   cbInfoBuf      length of storage area           |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PBYTE    pInfoBuf       storage area for information     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosQFileInfo                              |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosQFileInfo                                      |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlQFileInfoHwnd
(
   HFILE    hf,                        // file handle
   USHORT   usInfoLevel,               // level of file information required
   PBYTE    pInfoBuf,                  // storage area for returned information
   USHORT   cbInfoBuf,                 // length of storage area
   BOOL     fMsg,                       // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   PFILESTATUS pstFileSt;              // file status buffer

   do {
      DosError(0);

      /****************************************************************/
      /* currently ONLY usInfoLevel 1 function (file length) supported*/
      /****************************************************************/
      memset( pInfoBuf, 0, cbInfoBuf );
      if ( usInfoLevel != 1 )
      {
        usRetCode = ERROR_API_NOT_SUPPORTED;
      }
      else
      {
        BY_HANDLE_FILE_INFORMATION FileInfo;
        pstFileSt = (PFILESTATUS) pInfoBuf;
        if ( GetFileInformationByHandle( hf, &FileInfo ) != 0 )
        {
          FileTimeToDosDateTime( &FileInfo.ftLastAccessTime,
                                 (LPWORD)&pstFileSt->fdateLastAccess,
                                 (LPWORD)&pstFileSt->ftimeLastAccess );
          FileTimeToDosDateTime( &FileInfo.ftCreationTime,
                                 (LPWORD)&pstFileSt->fdateCreation,
                                 (LPWORD)&pstFileSt->ftimeCreation );
          FileTimeToDosDateTime( &FileInfo.ftLastWriteTime,
                                 (LPWORD)&pstFileSt->fdateLastWrite,
                                 (LPWORD)&pstFileSt->ftimeLastWrite );
          pstFileSt->cbFile      = FileInfo.nFileSizeLow;
          pstFileSt->cbFileAlloc = FileInfo.nFileSizeLow;
          pstFileSt->attrFile    = (USHORT)FileInfo.dwFileAttributes;
        }
        else
        {
            usRetCode = (USHORT)GetLastError();
        } /* endif */

      } /* endif */

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlQFSInfo             DosQFSInfo: Query file system info|
//+----------------------------------------------------------------------------+
//|Function call:     UtlQFSInfo( USHORT usDriveNo, USHORT usInfoLevel,        |
//|                               PBYTE pInfoBuf, USHORT cbInfoBuf,            |
//|                               BOOL fMsg );                                 |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosQFSInfo                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   USHORT   usDriveNo      drive number                     |
//|                   USHORT   usInfoLevel    level of filesystem info         |
//|                   USHORT   cbInfoBuf      length of storage area           |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PBYTE    pInfoBuf       storage area for information     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosQFSInfo                                |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlQFSInfoHwnd                                      |
//+----------------------------------------------------------------------------+

USHORT UtlQFSInfo
(
   USHORT   usDriveNo,                 // drive number
   USHORT   usInfoLevel,               // level of filesystem info required
   PBYTE    pInfoBuf,                  // storage area for returned information
   USHORT   cbInfoBuf,                 // length of storage area
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlQFSInfoHwnd( usDriveNo,
                           usInfoLevel,
                           pInfoBuf,
                           cbInfoBuf,
                           fMsg,
                           (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlQFSInfoHwnd         DosQFSInfo: Query file system info|
//+----------------------------------------------------------------------------+
//|Function call:     UtlQFSInfoHwnd( USHORT usDriveNo, USHORT usInfoLevel,    |
//|                                   PBYTE pInfoBuf, USHORT cbInfoBuf,        |
//|                                   BOOL fMsg , HWND hwndParent);            |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosQFSInfo                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   USHORT   usDriveNo      drive number                     |
//|                   USHORT   usInfoLevel    level of filesystem info         |
//|                   USHORT   cbInfoBuf      length of storage area           |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PBYTE    pInfoBuf       storage area for information     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosQFSInfo                                |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosQFSInfo                                        |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+

USHORT UtlQFSInfoHwnd
(
   USHORT   usDriveNo,                 // drive number
   USHORT   usInfoLevel,               // level of filesystem info required
   PBYTE    pInfoBuf,                  // storage area for returned information
   USHORT   cbInfoBuf,                 // length of storage area
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   PSZ    pszDriveNo = NULL;                  // name of drive to be checked
   CHAR   szDrive[2];
   PFSALLOCATE  pfsAlloc = (PFSALLOCATE) pInfoBuf;

   cbInfoBuf;

   do {
      DosError(0);

      /****************************************************************/
      /* only level 1 is supported (disk allocation units ...)        */
      /****************************************************************/
      if ( usInfoLevel == 1 )
      {
        DWORD dwSectorsPerCluster, dwBytesPerSector, dwFree, dwTotal;
        CHAR szRoot[4] = "A:\\";
        szRoot[0] = 'A' + (CHAR)(usDriveNo - 1); // 1 = A:, 2 = B:, ...
        if ( GetDiskFreeSpace( szRoot, &dwSectorsPerCluster,
                               &dwBytesPerSector,  &dwFree, &dwTotal ) != 0 )
        {
          pfsAlloc->cSectorUnit = dwSectorsPerCluster;
          pfsAlloc->cUnit       = dwTotal;
          pfsAlloc->cUnitAvail  = dwFree;
          pfsAlloc->cbSector    = (USHORT)dwBytesPerSector;
        }
        else
        {
          usRetCode = (USHORT)GetLastError();
        } /* endif */
      }
      else
      {
        usRetCode = ERROR_API_NOT_SUPPORTED;
      } /* endif */

      DosError(1);
      if ( fMsg && usRetCode )
      {
          szDrive[0] =  (CHAR) (usDriveNo + 'A' - 1);
          szDrive[1]= EOS;
         pszDriveNo = szDrive;
         usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszDriveNo, DOS_ERROR, hwndParent );
      } /* endif */
      if ( usMBCode == MBID_RETRY && usRetCode == ERROR_DISK_CHANGE)
      {
        UtlSetDrive( *pszDriveNo);
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlSetFileMode         DosSetFileMode: Change file mode  |
//+----------------------------------------------------------------------------+
//|Function call:     UtlSetFileMode( PSZ pszFile, USHORT usMode,              |
//|                                   ULONG ulReserved, BOOL fMsg )            |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosSetFileMode                     |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFile        fully qualified file name        |
//|                   USHORT   usMode         new file mode                    |
//|                   ULONG    ulReserved     reserved value must be 0L        |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosSetFileMode                            |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlSetFileModeHwnd                                  |
//+----------------------------------------------------------------------------+
USHORT UtlSetFileMode
(
   PSZ      pszFile,                   // fully qualified file name
   USHORT   usMode,                    // new file mode
   ULONG    ulReserved,                // reserved value must be 0L
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlSetFileModeHwnd( pszFile,
                               usMode,
                               ulReserved,
                               fMsg,
                               (HWND) NULL ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlSetFileModeHwnd     DosSetFileMode: Change file mode  |
//+----------------------------------------------------------------------------+
//|Function call:     UtlSetFileModeHwnd( PSZ pszFile, USHORT usMode,          |
//|                                       ULONG ulReserved, BOOL fMsg,         |
//|                                       HWND hwndParent );                   |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosSetFileMode                     |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFile        fully qualified file name        |
//|                   USHORT   usMode         new file mode                    |
//|                   ULONG    ulReserved     reserved value must be 0L        |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosSetFileMode                            |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosSetFileMode                                    |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlSetFileModeHwnd
(
   PSZ      pszFile,                   // fully qualified file name
   USHORT   usMode,                    // new file mode
   ULONG    ulReserved,                // reserved value must be 0L
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   ulReserved;
   do {
      DosError(0);

      if ( SetFileAttributes( pszFile, (DWORD)usMode ) == 0 )
      {
        usRetCode = (USHORT)GetLastError();
      } /* endif */

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlQFileMode           DosQFileMode: Query a file mode   |
//+----------------------------------------------------------------------------+
//|Function call:     UtlQFileMode( PSZ pszFileName, PUSHORT pusAttribute,     |
//|                                 ULONG ulReserved, BOOL fMsg );             |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosQFileMode                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFileName    ptr to fully qualified file name |
//|                   ULONG    ulReserved     reserved must be 0L              |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pusAttribute   attribute of file                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosQFileMode                              |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlQFileModeHwnd                                    |
//+----------------------------------------------------------------------------+
USHORT UtlQFileMode
(
   PSZ      pszFileName,               // pointer to fully qualified file name
   PUSHORT  pusAttribute,              // attribute of file
   ULONG    ulReserved,                // reserved must be 0L
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlQFileModeHwnd( pszFileName,
                             pusAttribute,
                             ulReserved,
                             fMsg,
                             (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlQFileModeHwnd       DosQFileMode: Query a file mode   |
//+----------------------------------------------------------------------------+
//|Function call:     UtlQFileModeHwnd( PSZ pszFileName, PUSHORT pusAttribute, |
//|                                     ULONG ulReserved, BOOL fMsg,           |
//|                                     HWND hwndParent );                     |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosQFileMode                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszFileName    ptr to fully qualified file name |
//|                   ULONG    ulReserved     reserved must be 0L              |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pusAttribute   attribute of file                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosQFileMode                              |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlQFileModeHwnd                                    |
//+----------------------------------------------------------------------------+
USHORT UtlQFileModeHwnd
(
   PSZ      pszFileName,               // pointer to fully qualified file name
   PUSHORT  pusAttribute,              // attribute of file
   ULONG    ulReserved,                // reserved must be 0L
   BOOL     fMsg,                   // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   ulReserved;
   do {
      DosError(0);

      {
        DWORD dwAttr = GetFileAttributes( pszFileName );
        if ( dwAttr != 0xFFFFFFFF )
        {
          *pusAttribute = (USHORT)dwAttr;
        }
        else
        {
          usRetCode = (USHORT)GetLastError();
        } /* endif */
      }

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlSetFHandState/...Hwnd DosSetFHandState: set file state|
//+----------------------------------------------------------------------------+
//|Function call:     UtlSetFHandState( HFILE hf, USHORT fsState, BOOL fMsg ); |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosSetFHandState                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             handle of an openend file        |
//|                   USHORT   fsState        new state for file               |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                 ( HWND     hwnd )                                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosSetFHandState                          |
//+----------------------------------------------------------------------------+
//|Function flow:     call DosSetFHandState                                    |
//+----------------------------------------------------------------------------+
USHORT UtlSetFHandState( HFILE hf, USHORT fsState, BOOL fMsg )
{
  return UtlSetFHandStateHwnd( hf, fsState, fMsg, (HWND)NULL );
}
USHORT UtlSetFHandStateHwnd( HFILE hf, USHORT fsState, BOOL fMsg, HWND hwnd )
{
   USHORT usRetCode = 0;               // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   hf; fsState;
   do {
      DosError(0);
      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwnd );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlBufReset/...Hwnd DosBufReset: flush buffer contents   |
//+----------------------------------------------------------------------------+
//|Function call:     UtlBufReset( HFILE hf, BOOL fMsg );                      |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosBufReset                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             handle of an openend file        |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                 ( HWND     hwnd )                                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosBufReset                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call DosBufReset                                         |
//+----------------------------------------------------------------------------+
USHORT UtlBufReset( HFILE hf, BOOL fMsg )
{
  return UtlBufResetHwnd( hf, fMsg, (HWND)NULL );
}

USHORT UtlBufResetHwnd( HFILE hf, BOOL fMsg, HWND hwnd )
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   do {
      DosError(0);

      if ( FlushFileBuffers( hf ) == 0 )
      {
        usRetCode = (USHORT)GetLastError();
      } /* endif */

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwnd );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}




//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlSetDrive                                              |
//+----------------------------------------------------------------------------+
//|Function call:     UtlSetDrive ( UCHAR NewDrive)                            |
//+----------------------------------------------------------------------------+
//|Description:       Set drive letter ...                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:   UCHAR    NewDrive                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: TRUE or FALSE                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//+----------------------------------------------------------------------------+
BOOL UtlSetDrive
(
  CHAR   szNewDrive                        // driveletter of new drive
)
{
  BOOL    fFlag=TRUE;

  CHAR szRootDir[5] = "A:\\";

  szRootDir[0] = szNewDrive;
  if ( SetCurrentDirectory( szRootDir ) == 0 )
  {
    fFlag = FALSE;
  } /* endif */

  return(fFlag);
}



/**********************************************************************/
/* the following section contains DOS-API functions which are not     */
/* available under Windows.                                           */
/* They are all emulated in a way, that the OS/2 source code could    */
/* stay the same....                                                  */
/**********************************************************************/

 /***   DosLoadModule - Load dynamic link module
  *
  *     Loads a dynamic link module and returns a handle for the module.
  *
  */

 USHORT APIENTRY DosLoadModule
 (
    PSZ pszFailName,                    // object name buffer
    USHORT cbFileName,                  // length of object name buffer
    PSZ pszModName,                     // dynamic link module name string
    PHMODULE phMod                      // module handle (returned)
 )
 {
   USHORT usRc = 0;                     // success indicator
   HINSTANCE hmod;

   pszFailName; cbFileName;
   DosError(0);                         // avoid error popup...
   hmod = LoadLibrary(pszModName );
   DosError(1);
   if ( hmod == NULL)
   {
     usRc = (USHORT)GetLastError();
     *phMod = (HMODULE) NULL;
   }
   else
   {
     *phMod = hmod;
   } /* endif */
   return usRc;
 }


 /***   DosGetProcAddr - Get dynamic link procedure address
  *
  *     Returns a far address to a desired procedure within a dynamic link
  *     module.
  *
  */

 USHORT APIENTRY DosGetProcAddr
 (
   HMODULE hMod,                        // handle of module where proc is located
   PSZ pszProcName,                     // name of module where proc is located
   PFN FAR * ppfnProcAddr               // procedure address (returned)
 )
 {
   USHORT usRc = 0;

   *ppfnProcAddr = GetProcAddress( hMod, pszProcName );
   if (!*ppfnProcAddr )
   {
//#ifdef _DEBUG
//     char szFilename[2048];
//	 FILE *pFile = fopen("missfunc.out", "a");
//	 if (pFile)
//	 {
//		GetModuleFileName(hMod, szFilename, sizeof(szFilename) - 1);
//		fprintf(pFile, "%s not found in %s\n", pszProcName, szFilename);
//		fclose(pFile);
//	 }
//#endif
     usRc = ERROR_PROC_NOT_FOUND;
   } /* endif */
   return usRc;
 }




 /***   DosGetModHandle - Get dynamic link module handle
  *
  *     Returns a handle to a previously loaded dynamic link module.
  *
  */

 USHORT APIENTRY DosGetModHandle
 (
   PSZ pszModName,                      // dynamic link module name
   PHMODULE phMod                       // module handle (returned)
 )
 {
   USHORT usRc = 0;                     // success indicator

   *phMod = GetModuleHandle( pszModName );
   if (!*phMod )
   {
     usRc = ERROR_MOD_NOT_FOUND ;
   } /* endif */

   return usRc;
 }

 /***   DosGetModName - Get dynamic link module name
  *
  *     Returns the fully qualified drive, path, filename, and extension
  *     associated with a referenced module handle.
  *
  */

 USHORT APIENTRY DosGetModName
 (
   HMODULE hMod,                        // module handle
   USHORT cbBuf,                        // max length of buffer
   PCHAR pchBuf                         // buffer (returned)
 )
 {
   USHORT  usRc = 0;                    // success indicator

   if ( !GetModuleFileName( hMod, pchBuf, cbBuf ) )
   {
     usRc = ERROR_INVALID_NAME;
   } /* endif */

   return usRc;
 }

 /***   DosFreeModule - Free dynamic link module
  *
  *     Frees the reference to a dynamic link module for a process. When the
  *     dynamic link module is no longer needed by any process, the module is
  *     freed from system memory.
  *
  */

 USHORT APIENTRY DosFreeModule
 (
   HMODULE hMod                         // module handle
 )
 {
   FreeModule( hMod );
   return 0;
 }

 /**********************************************************************/
 /* Dos Error handling  ...                                            */
 /**********************************************************************/
 /***   DosError - Enable hard error processing
  *     Allows a process to disable user notification (from OS/2) on hard errors
  *     and program exceptions.
  */

 USHORT APIENTRY DosError
 (
   USHORT fEnable                       // action flag (bit field)
 )
 {
   switch ( fEnable )
   {
     case 0:
       SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
       break;
     case 1:
       SetErrorMode(0);
       break;
   } /* endswitch */
   return 0;
 }

 /**********************************************************************/
 /* DosInsMessage  - copy message to a buffer, but substitute variables*/
 /*                  if necessary                                      */
 /**********************************************************************/
 USHORT APIENTRY DosInsMessage
 (
   PCHAR FAR * ppchVTable,              // table of variables to insert
   USHORT usVCount,                     // number of variables
   PSZ pszMsg,                          // address of input message
   ULONG cbMsg,                        // length of input message
   PCHAR pchBuf,                        // data area address to return message to
   ULONG  cbBuf,                        // length of data area (returned)
   PULONG pcbMsg                       // length of updated messsage
 )
 {
   ULONG   ulOutLen = 0;                // length of filled output
   USHORT  usRC = 0;                    // return code
   BYTE    c;                           // active byte
   USHORT  usIndex;                     // index into string table
   PSZ     pData;                       // pointer to data ...

   /********************************************************************/
   /* check for usVCount validity                                      */
   /********************************************************************/
   if ( usVCount > 9 )
   {
     usRC = ERROR_MR_INV_IVCOUNT;
   }
   else if ( usVCount == 0 )
   {
     if ( cbBuf >= cbMsg )
     {
       strcpy( pchBuf, pszMsg );
       *pcbMsg = cbMsg;
     }
     else
     {
       /****************************************************************/
       /* string too long -- truncate error message                    */
       /****************************************************************/
       strncpy( pchBuf, pszMsg, cbBuf );
       pchBuf[cbBuf-1] = EOS;
       *pcbMsg = cbBuf;

       usRC = ERROR_MR_MSG_TOO_LONG;
     } /* endif */
   }
   else
   {
     /****************************************************************/
     /* substitute any %i values through parameters used             */
     /****************************************************************/
     while ( ((c = *pszMsg++)!= NULC) && ( ulOutLen < cbBuf ))
     {
       if ( c == '%' )
       {
         c = *pszMsg++;
         if ( isdigit(c) )
         {
           if ( (c - '0') <= (SHORT)usVCount  )
           {
             usIndex = (USHORT) (c - '1');
             pData = (ppchVTable[ usIndex ]);
             if ( pData )
             {
               while ( ((c = *pData++)!= NULC) && ( ulOutLen < cbBuf ) )
               {
                 *pchBuf++ = c;
                 ulOutLen ++;
               } /* endwhile */
               // check if length problem
               if ( ulOutLen == cbBuf )
               {
                 usRC = ERROR_MR_MSG_TOO_LONG;
               } /* endif */
             }
             else
             {
               /********************************************************/
               /* treat not specified pointer to data as Empty string..*/
               /********************************************************/
             } /* endif */

           }
           else
           {
             /**********************************************************/
             /* passed digit not part of substitution - treat as normal*/
             /* character                                              */
             /**********************************************************/
             *pchBuf++ = c;
             ulOutLen ++;
           } /* endif */

         }
         else if ( c == EOS )
         {
           pszMsg--;                    // ensure that we leave while loop
         }
         else
         {
           /************************************************************/
           /* normal character ...                                     */
           /************************************************************/
           *pchBuf++ = c;
           ulOutLen ++;
         } /* endif */
       }
       else
       {
         *pchBuf++ = c;
         ulOutLen ++;
       } /* endif */
     } /* endwhile */
     *pchBuf = EOS;                     // add end indication
     *pcbMsg = ulOutLen;
   } /* endif */

   return usRC;
 }

 USHORT APIENTRY DosInsMessageW
 (
   PCHAR_W FAR * ppchVTable,              // table of variables to insert
   USHORT usVCount,                       // number of variables
   PSZ_W pszMsg,                          // address of input message
   ULONG cbMsg,                          // length of input message
   PCHAR_W pchBuf,                        // data area address to return message to
   ULONG   cbBuf,                          // length of data area (returned)
   PULONG  pcbMsg                         // length of updated messsage
 )
 {
   ULONG   ulOutLen = 0;                // length of filled output
   USHORT  usRC = 0;                    // return code
   CHAR_W    c;                           // active byte
   USHORT  usIndex;                     // index into string table
   PSZ_W     pData;                       // pointer to data ...

   /********************************************************************/
   /* check for usVCount validity                                      */
   /********************************************************************/
   if ( usVCount > 9 )
   {
     usRC = ERROR_MR_INV_IVCOUNT;
   }
   else if ( usVCount == 0 )
   {
     if ( cbBuf >= cbMsg )
     {
       UTF16strcpy( pchBuf, pszMsg );
       *pcbMsg = cbMsg;
     }
     else
     {
       /****************************************************************/
       /* string too long -- truncate error message                    */
       /****************************************************************/
       UTF16strncpy( pchBuf, pszMsg, (USHORT)cbBuf );
       pchBuf[cbBuf-1] = EOS;
       *pcbMsg = cbBuf;

       usRC = ERROR_MR_MSG_TOO_LONG;
     } /* endif */
   }
   else
   {
     /****************************************************************/
     /* substitute any %i values through parameters used             */
     /****************************************************************/
     while ( ((c = *pszMsg++)!= NULC) && ( ulOutLen < cbBuf ))
     {
       if ( c == '%' )
       {
         c = *pszMsg++;
         if ( iswdigit(c) )
         {
           if ( (c - '0') <= (SHORT)usVCount  )
           {
             usIndex = (USHORT) (c - '1');
             pData = (ppchVTable[ usIndex ]);
             if ( pData )
             {
               while ( ((c = *pData++)!= NULC) && ( ulOutLen < cbBuf ) )
               {
                 *pchBuf++ = c;
                 ulOutLen ++;
               } /* endwhile */
               // check if length problem
               if ( ulOutLen == cbBuf )
               {
                 usRC = ERROR_MR_MSG_TOO_LONG;
               } /* endif */
             }
             else
             {
               /********************************************************/
               /* treat not specified pointer to data as Empty string..*/
               /********************************************************/
             } /* endif */
           }
           else
           {
             /**********************************************************/
             /* passed digit not part of substitution - treat as normal*/
             /* character                                              */
             /**********************************************************/
             *pchBuf++ = c;
             ulOutLen ++;
           } /* endif */
         }
         else if ( c == EOS )
         {
           pszMsg--;                    // ensure that we leave while loop
         }
         else
         {
           /************************************************************/
           /* normal character ...                                     */
           /************************************************************/
           *pchBuf++ = c;
           ulOutLen ++;
         } /* endif */
       }
       else
       {
         *pchBuf++ = c;
         ulOutLen ++;
       } /* endif */
     } /* endwhile */
     *pchBuf = EOS;                     // add end indication
     *pcbMsg = ulOutLen;
   } /* endif */

   return usRC;
 }  /* end of DosInsMessageW */

 /**********************************************************************/
 /* DosGetMessage  - get an error message from an OS/2 message file    */
 /*  -- Attention: We have to deal with two different formats...       */
 /**********************************************************************/
 //+----------------------------------------------------------------------------+
 //|External function                                                           |
 //+----------------------------------------------------------------------------+
 //|Function name:     DosGetMessage                                            |
 //+----------------------------------------------------------------------------+
 //|Function call:     (same as under os/2)                                     |
 //+----------------------------------------------------------------------------+
 //|Description:       get an error message from an OS/2 message file           |
 //|                   This function emulates the access to external message    |
 //|                   files - it does NOT support internal (i.e. binded) msgs. |
 //|                   Attention: There are two different error formats avail.  |
 //|                              This is dependant on a flag in the header..   |
 //+----------------------------------------------------------------------------+
 //|Parameters:        PCHAR FAR * ppchVTable table of variables to insert      |
 //|                   USHORT  usVCount,      number of variables               |
 //|                   PCHAR pchBuf,          data area (where message is ret)  |
 //|                   USHORT cbBuf,          length of data area               |
 //|                   USHORT usMsgNum,       number of the message requested   |
 //|                   PSZ pszFileName,       message path and file name        |
 //|                   PUSHORT pcbMsg         length of message (returned)      |
 //+----------------------------------------------------------------------------+
 //|Returncode type:   USHORT                                                   |
 //+----------------------------------------------------------------------------+
 //|Returncodes:       ERROR_FILE_NOT_FOUND                                     |
 //|                   ERROR_MR_INV_MSGF_FORMAT                                 |
 //|                   ERROR_MR_MID_NOT_FOUND                                   |
 //|                   DOS Error codes                                          |
 //+----------------------------------------------------------------------------+
 //|Samples:           DosGetMessage( pParmTable, usNoOfParms,                  |
 //|                                  pBuffer, sizeof(chMsgBuffer), msgNo,      |
 //|                                  ErrData.chMsgFile, &usLength );           |
 //+----------------------------------------------------------------------------+
 //|Function flow:     - open message file                                      |
 //|                   - if error then                                          |
 //|                      prepare dummy message                                 |
 //|                     else                                                   |
 //|                      read and check header of file,                        |
 //|                     endif                                                  |
 //|                   - if okay read in message and substitute parameters      |
 //|                   - return success indicator                               |
 //+----------------------------------------------------------------------------+
 USHORT APIENTRY
 DosGetMessage
 (
   PCHAR FAR * ppchVTable ,             // table of variables to insert
   USHORT  usVCount,                    // number of variables
   PCHAR pchBuf,                        // data area (where message is returned)
   ULONG cbBuf,                        // length of data area
   USHORT usMsgNum,                     // number of the message requested
   PSZ pszFileName,                     // message path and file name
   PULONG pcbMsg                       // length of message (returned)
 )
 {
   USHORT   usRC = 0;                   // success indicator
   ULONG    ulMsgLen = 0;                   // length of message
   PCHAR    pMsgSubst[2];               // pointer to message subst. array
   CHAR     chNum[10];                  // itoa string buffer for message number
   PCHAR    * ppMsgSubst;

   // load the message string table if not done yet
   if ( !usRC )
   {
     if ( hMessagesMod == NULL )
	   {
       usRC = DosLoadModule( NULL, 0 , "MessagesE.DLL", &hMessagesMod );
     } /* endif */
   } /* endif */
   
   // get the message text from the messages string table
   if ( !usRC )
   {
     PSZ pszBuffer = chMsg + 9;
     ulMsgLen = sizeof(chMsg) - 10;
     ulMsgLen = (ULONG)LoadString( hMessagesMod, usMsgNum, pszBuffer, (int)ulMsgLen );
   } /* endif */
   
   /********************************************************************/
   /* in case of errors prepare dummy messages ....                    */
   /********************************************************************/
   if ( usRC )
   {
     /********************************************************************/
     /* prepare substitutions pointers                                   */
     /********************************************************************/
     pMsgSubst[0] = pszFileName;
     pMsgSubst[1] = itoa( usMsgNum, chNum, 10 );
     ppMsgSubst = &pMsgSubst[0];
     usVCount = 2;
     chMsg[8] = 'E';                   // it's an error message ....
     switch ( usRC )
     {
       case  ERROR_FILE_NOT_FOUND:
         strcpy( &chMsg[9], "Message file %1 cannot be found." );
         break;
       case  ERROR_MR_INV_MSGF_FORMAT:
         strcpy( &chMsg[9], "The system cannot read message file %1." );
         break;
       case  ERROR_MR_MID_NOT_FOUND:
         strcpy( &chMsg[9], "The system cannot find message %2 in message file %1." );
         break;
       default :
         strcpy( &chMsg[9], "The system cannot find message %2 in message file %1.");
         break;
     } /* endswitch */
     ulMsgLen = strlen( &chMsg[8] ) + 8;
   }
   else
   {
     ppMsgSubst = ppchVTable;
   } /* endif */

   /*************************************************************/
   /* first 9 characters are preset with error message and comp */
   /* identification                                            */
   /*************************************************************/
   {
     CHAR chMsgFileId[4];
     CHAR chMsgIdent[12];
     strcpy( chMsgFileId, "EQF" );
     sprintf( chMsgIdent, "%s%04d: ", chMsgFileId, usMsgNum );
     memcpy( chMsg, chMsgIdent, 9 );
   }

   /********************************************************************/
   /* call DosInsMessage to substitute data => they are all in ANSI    */
   /********************************************************************/
   DosInsMessage( ppMsgSubst, usVCount, &chMsg[0], ulMsgLen,
                  pchBuf, cbBuf, pcbMsg );

   /********************************************************************/
   /* close message file ( during stopping of workbench)               */
   /********************************************************************/

   return usRC;
 } /* end of function DosGetMessage */

 //+----------------------------------------------------------------------------+
 //|External function                                                           |
 //+----------------------------------------------------------------------------+
 //|Function name:     DosCopy                                                  |
 //+----------------------------------------------------------------------------+
 //|Function call:     _                                                        |
 //+----------------------------------------------------------------------------+
 //|Description:       copy one file into the other                             |
 //+----------------------------------------------------------------------------+
 //|Parameters:        PSZ     pszSrc,              source file name            |
 //|                   PSZ     pszTgt,              destination file name       |
 //|                   USHORT  usOpt,               options                     |
 //|                   ULONG   ulReserved                                       |
 //+----------------------------------------------------------------------------+
 //|Returncode type:   USHORT                                                   |
 //+----------------------------------------------------------------------------+
 //|Returncodes:       same as the OS/2 function                                |
 //+----------------------------------------------------------------------------+
 //|Function flow:     - allocate temp buffer                                   |
 //|                   - open source and create target file                     |
 //|                   - if error in target creation check if we are allowed to |
 //|                     overwrite the existing target file - if so try again   |
 //|                   - copy file and set target file date to source file date |
 //|                   - free resources                                         |
 //|                   - return success                                         |
 //+----------------------------------------------------------------------------+
 #define DOS_READ_BUF     8192         // buffer for dos file operations
 USHORT APIENTRY  DosCopy
 (
   PSZ     pszSrc,                      // source file name
   PSZ     pszTgt,                      // destination file name
   USHORT  usOpt,                       // options
   ULONG   ulReserved
 )
 {
   USHORT  usRC = 0;                    // return code
   ulReserved;

   if ( CopyFile( pszSrc, pszTgt, (usOpt & DCPY_EXISTING) ? FALSE : TRUE ) == 0 )
   {
     usRC = (USHORT)GetLastError();
   } /* endif */
   /********************************************************************/
   /* return success indication                                        */
   /********************************************************************/
   return usRC;
 } /* end of function DosCopy */


/***   DosNewSize - Change file size
 *
 *     Changes the size of a file.
 *     This implementation takes into
 *
 */

USHORT APIENTRY DosNewSize
(
        HFILE hf,                       /* file handle     */
        ULONG ulNewSize                 /* file's new size */
)
{

  USHORT usRetCode = 0;                // no error yet

  // position to new end of file
  if (SetFilePointer( hf, ulNewSize, NULL, FILE_BEGIN ) == 0xFFFFFFFF )
  {
    usRetCode = (USHORT)GetLastError();
  } /* endif */

  // make current position the end of the file
  if ( usRetCode == NO_ERROR )
  {
    if ( SetEndOfFile( hf ) == 0 )
    {
      usRetCode = (USHORT)GetLastError();
    } /* endif */
  } /* endif */
  return usRetCode;
}

/**********************************************************************/
/* get command line parameters                                        */
/**********************************************************************/
void UtlGetArgs
(
  HINSTANCE hInstance,                 // application instance
  PSZ pCmdLine,                        // passed command line
  int *argc,                           // return number of arguments
  char ** argv                         // return pointer array
)
{
  // BOOL fFlag;
  PSZ  pFileName;

  *argc = 0;
  /********************************************************************/
  /* allocate memory to hold application name....                     */
  /********************************************************************/
  if ( UtlAlloc((PVOID *) &pFileName, 0L, (LONG) MAX_LONGPATH, NOMSG ))
  {
    PSZ pszSource, pszTarget;

    GetModuleFileName( hInstance, pFileName, MAX_LONGPATH );
    argv[0] = pFileName;
    UtlStripBlanks( pCmdLine );

    *argc = 1;
    pszSource = pszTarget = pCmdLine;
    while ( *pszSource != EOS )
    {
      // skip leading blanks
      while ( *pszSource == BLANK )
      {
        pszSource++;
      } /* endwhile */

      // start new argument if not empty
      if ( *pszSource != EOS )
      {
        argv[*argc] = pszTarget;
        *argc += 1;
      } /* endif */

      // copy argument data and handle quotes
      while ( (*pszSource != EOS) && (*pszSource != BLANK) )
      {
        if ( *pszSource == DOUBLEQUOTE )
        {
          // handle string enclosed in double quotes; i.e. copy enclosed
          // string and do not care for blanks
          pszSource++;                 // skip starting quote
          while ( *pszSource != EOS)
          {
            if ( *pszSource == DOUBLEQUOTE )
            {
              // check for real (=doubled) quotes or string end delimiting quote
              pszSource++;
              if ( *pszSource == DOUBLEQUOTE )
              {
                // add this quote as quote to the string
                *pszTarget++ = *pszSource++;
              }
              else
              {
                // quote is string end delimiter, so leave loop
                break;
              } /* endif */
            }
            else
            {
              // copy character
              *pszTarget++ = *pszSource++;
            } /* endif */
          } /* endwhile */
        }
        else
        {
          // copy as-is
          *pszTarget++ = *pszSource++;
        } /* endif */
      } /* endwhile */
      if ( *pszSource != EOS )
      {
        pszSource++; // skip delimiter
      } /* endif */
      *pszTarget++ = EOS;
    } /* endwhile */

//  for information only: the old code of this function
//  for (*argc=1; *pCmdLine; ++*argc )
//  {
//
//    if ( !*pCmdLine )
//      break;
//
//    argv[*argc] = pCmdLine;
//    fFlag = FALSE;
//    do
//    {
//      if (!fFlag && ( *pCmdLine == BLANK ) )
//      {
//        *pCmdLine++ = EOS;  // start new string..
//        break;
//      }
//      if (!fFlag && ( *pCmdLine == '"' ) )
//      {
//        fFlag = (!fFlag);
//      } /* endif */
//      pCmdLine++;
//    } while ( *pCmdLine ); /* enddo */
//  } /* endfor */

    argv[*argc] = NULL;
  } /* endif */
  return;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlQPathInfo(Hwnd)     DosQPathInfo: Query file info     |
//+----------------------------------------------------------------------------+
//|Function call:     UtlQPathInfo( PSZ pszPath, USHORT usInfoLevel,           |
//|                                 PBYTE pInfoBuf, USHORT cbInfoBuf,          |
//|                                 ULONG ulReserved, BOOL fMsg);              |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosQPathInfo                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszPath        file name                        |
//|                   USHORT   usInfoLevel    level of file information        |
//|                   USHORT   cbInfoBuf      length of storage area           |
//|                   ULONG    ulReserved     reserved value must be 0L        |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   (HWND)                                                   |
//+----------------------------------------------------------------------------+
//|Output parameter:  PBYTE    pInfoBuf       storage area for information     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosQPathInfo                              |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosQFileInfo                                      |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlQPathInfo
(
   PSZ      pszPath,                   // Path string
   USHORT   usInfoLevel,               // level of file information required
   PBYTE    pInfoBuf,                  // storage area for returned information
   USHORT   cbInfoBuf,                 // length of storage area
   ULONG ulReserved,                   // Reserved (must be zero)
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlQPathInfoHwnd( pszPath, usInfoLevel, pInfoBuf, cbInfoBuf,
                             ulReserved, fMsg, (HWND) NULL ) );
}

USHORT UtlQPathInfoHwnd
(
   PSZ      pszPath,                   // Path string
   USHORT   usInfoLevel,               // level of file information required
   PBYTE    pInfoBuf,                  // storage area for returned information
   USHORT   cbInfoBuf,                 // length of storage area
   ULONG    ulReserved,                   // Reserved (must be zero)
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   PFILESTATUS pstFileSt;              // file status buffer

   ulReserved;
   do {
      DosError(0);
      /****************************************************************/
      /* currently ONLY usInfoLevel 1 function (file info) supported*/
      /****************************************************************/
      memset( pInfoBuf, 0, cbInfoBuf );
      if ( usInfoLevel != 1 )
      {
        usRetCode = ERROR_API_NOT_SUPPORTED;
      }
      else
      {
        // there is no PathInfo call under Win32 so we use
        // FindFirstFile to get the file information without
        // opening the file
        static WIN32_FIND_DATA FileInfo;
        HANDLE hDir = INVALID_HANDLE_VALUE;

        hDir = FindFirstFile( pszPath, &FileInfo );
        if ( hDir != INVALID_HANDLE_VALUE )
        {
          // copy file information to target structure
          pstFileSt = (PFILESTATUS) pInfoBuf;
          FileTimeToDosDateTime( &FileInfo.ftLastAccessTime,
                                 (LPWORD)&pstFileSt->fdateLastAccess,
                                 (LPWORD)&pstFileSt->ftimeLastAccess );
          FileTimeToDosDateTime( &FileInfo.ftCreationTime,
                                 (LPWORD)&pstFileSt->fdateCreation,
                                 (LPWORD)&pstFileSt->ftimeCreation );
          FileTimeToDosDateTime( &FileInfo.ftLastWriteTime,
                                 (LPWORD)&pstFileSt->fdateLastWrite,
                                 (LPWORD)&pstFileSt->ftimeLastWrite );
          pstFileSt->cbFile      = FileInfo.nFileSizeLow;
          pstFileSt->cbFileAlloc = FileInfo.nFileSizeLow;
          pstFileSt->attrFile    = (USHORT)FileInfo.dwFileAttributes;

          // close file find handle
          FindClose( hDir );
        }
        else
        {
          usRetCode = (USHORT)GetLastError();
          if ( usRetCode == ERROR_NO_MORE_FILES )
          {
            usRetCode = ERROR_FILE_NOT_FOUND;
          } /* endif */
        } /* endif */
      } /* endif */

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFileLocks          DosFileLocks: Lock/Unlock a file   |    |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFileLocks( HFILE hf, PFILELOCK pUnlock,               |
//|                                  PFILELOCK pLock, BOOL fMsg);              |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFileLocks                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   PFILELOCK pUnlock       range to be unlocked             |
//|                   PFILELOCK pLock         range to be locked               |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFileLocks                              |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFileLocksHwnd                                    |
//+----------------------------------------------------------------------------+
USHORT UtlFileLocks
(
  HFILE     hf,                        // file handle
  PFILELOCK pUnlock,                   // range to be unlocked
  PFILELOCK pLock,                     // range to be locked
  BOOL     fMsg                        // if TRUE handle errors in utility
)
{
   return( UtlFileLocksHwnd( hf, pUnlock, pLock, fMsg, (HWND)NULL ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFileLocksHwd       DosFileLocks: Lock/Unlock a file   |    |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFileLocksHwnd( HFILE hf, PFILELOCK pUnlock,           |
//|                                     PFILELOCK pLock, BOOL fMsg, HWND hwnd);|
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFileLocks                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             file handle                      |
//|                   PFILELOCK pUnlock       range to be unlocked             |
//|                   PFILELOCK pLock         range to be locked               |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFileLocks                              |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosFileLocks                                      |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlFileLocksHwnd
(
  HFILE     hf,                        // file handle
  PFILELOCK pUnlock,                   // range to be unlocked
  PFILELOCK pLock,                     // range to be locked
  BOOL     fMsg,                       // if TRUE handle errors in utility
  HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   do {
      DosError(0);

      if ( pUnlock != NULL )
      {
        if ( UnlockFile( hf, pUnlock->lOffset, 0L, pUnlock->lRange, 0L  ) == 0 )
        {
          usRetCode = (USHORT)GetLastError();
        } /* endif */
      } /* endif */
      if ( (usRetCode == NO_ERROR) && (pLock != NULL) )
      {
        if ( LockFile( hf, pLock->lOffset, 0L, pLock->lRange, 0L ) == 0 )
        {
          usRetCode = (USHORT)GetLastError();
        } /* endif */
      } /* endif */
      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlWait       DosSleep(OS/2) or UtlDispatch(Win)         |
//+----------------------------------------------------------------------------+
//|Function call:     UtlWait( SHORT sWaitTime );                              |
//+----------------------------------------------------------------------------+
//|Description:       Waits a given time before returning                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   SHORT    sWaitTime     wait time in miliseconds          |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     for OS/2:                                                |
//|                     DosSleep the given time                                |
//|                   for Win:                                                 |
//|                     get current time                                       |
//|                     do                                                     |
//|                       call UtlDispatch                                     |
//|                       get new time                                         |
//|                     until given wait time has been exceeded                |
//+----------------------------------------------------------------------------+
VOID UtlWait
(
  SHORT     sWaitTime                  // wait time in miliseconds
)
{
  Sleep( (LONG)sWaitTime );
} /* end of function UtlWait */

/**********************************************************************/
/* Find First File with long file name support                        */
/**********************************************************************/
USHORT UtlFindFirstLong
(
   PSZ      pszFSpec,                  // path name of files to be found
   PHDIR    phdir,                     // directory handle
   USHORT   usAttr,                    // attribute used to search for the files
   PLONGFILEFIND pffb,                 // result buffer
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlFindFirstLongHwnd( pszFSpec, phdir, usAttr, pffb, fMsg, (HWND)NULL ) );
}

USHORT UtlFindFirstLongHwnd
(
   PSZ      pszFSpec,                  // path name of files to be found
   PHDIR    phdir,                     // directory handle
   USHORT   usAttr,                    // attribute used to search for the files
   PLONGFILEFIND pffb,                 // result buffer
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   HDIR   hdirNew;                     // new directory handle
   static WIN32_FIND_DATA FindData;    // find data structure
   memset( pffb, 0, sizeof(LONGFILEFIND) );
   do
   {
      DosError(0);
       if ( !(usAttr & FILE_ATTRIBUTE_DIRECTORY) )
       {
         usAttr |= FILE_ATTRIBUTE_NORMAL;
       } /* endif */
       hdirNew = FindFirstFile( pszFSpec, &FindData );
       if ( hdirNew == INVALID_HANDLE_VALUE )
       {
         usRetCode = (USHORT)GetLastError();
       }
       else
       {
          // look for files until the correct file type has been found
         // or no more files are available
         if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
         {
           FindData.dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
         } /* endif */
         while ( (usRetCode == NO_ERROR) &&
                 !(FindData.dwFileAttributes & usAttr) )
         {
            if ( FindNextFile( hdirNew, &FindData ) == 0 )
            {
              usRetCode = (USHORT)GetLastError();
              if ( usRetCode == ERROR_FILE_NOT_FOUND )
                usRetCode = ERROR_NO_MORE_FILES;
            }
            else if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            {
              FindData.dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
            } /* endif */
         } /* endwhile */

         if ( usRetCode == NO_ERROR )
         {
           FileTimeToDosDateTime( &FindData.ftLastAccessTime,
                                  (LPWORD)&pffb->fdateLastAccess,
                                  (LPWORD)&pffb->ftimeLastAccess );
           FileTimeToDosDateTime( &FindData.ftCreationTime,
                                  (LPWORD)&pffb->fdateCreation,
                                  (LPWORD)&pffb->ftimeCreation );
           FileTimeToDosDateTime( &FindData.ftLastWriteTime,
                                  (LPWORD)&pffb->fdateLastWrite,
                                  (LPWORD)&pffb->ftimeLastWrite );
           pffb->cbFile = FindData.nFileSizeLow;
           pffb->attrFile = FindData.dwFileAttributes;
           strcpy( pffb->achName, FindData.cFileName );

           // add file search handle and attributes in our list of open
           // file search handles
           UtlAddSearchHandle( hdirNew, usAttr );
         }
         else
         {
           // close file handle as it will not be used anymore
            FindClose( hdirNew );
            hdirNew = HDIR_CREATE;   // init again

         } /* endif */
       } /* endif */

      DosError(1);
      if ( fMsg && usRetCode && (usRetCode != ERROR_NO_MORE_FILES) &&
           (usRetCode != ERROR_FILE_NOT_FOUND))
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 1, &pszFSpec, DOS_ERROR, hwndParent );
         /*************************************************************/
         /* change the drive if necessary ...                         */
         /*************************************************************/
         if ( usMBCode == MBID_RETRY && usRetCode == ERROR_DISK_CHANGE)
         {
           UtlSetDrive( *pszFSpec);
         } /* endif */
      } /* endif */
   } while ( fMsg &&
             usRetCode &&
             (usRetCode != ERROR_NO_MORE_FILES) &&
             (usRetCode != ERROR_FILE_NOT_FOUND) &&
             (usMBCode == MBID_RETRY) ); /* enddo */
   *phdir = hdirNew;
   return( usRetCode );
}

USHORT UtlFindNextLong
(
   HDIR     hdir,                      // directory handle
   PLONGFILEFIND pffb,                 // result buffer
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlFindNextLongHwnd( hdir, pffb, fMsg, (HWND) NULL ) );
}
USHORT UtlFindNextLongHwnd
(
   HDIR     hdir,                      // directory handle
   PLONGFILEFIND pffb,                 // result buffer
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   USHORT usAttr;                      // file attributes for search
   static WIN32_FIND_DATA FindData;    // find data structure

   do {
      DosError(0);

      // get file attribute flags active for this search handle
      usAttr = UtlAttrOfSearchHandle( hdir );

      // look for files until the correct file type has been found
      // or no more files are available
      do
      {
        if ( FindNextFile( hdir, &FindData  ) == 0 )
        {
          usRetCode = (USHORT)GetLastError();
          if ( usRetCode == ERROR_FILE_NOT_FOUND ) usRetCode = ERROR_NO_MORE_FILES;
        }
        else if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
          FindData.dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
        } /* endif */
      }
      while ( (usRetCode == NO_ERROR) &&
              !(FindData.dwFileAttributes & usAttr) );

      if ( usRetCode == NO_ERROR )
      {
        FileTimeToDosDateTime( &FindData.ftLastAccessTime,
                               (LPWORD)&pffb->fdateLastAccess,
                               (LPWORD)&pffb->ftimeLastAccess );
        FileTimeToDosDateTime( &FindData.ftCreationTime,
                               (LPWORD)&pffb->fdateCreation,
                                (LPWORD)&pffb->ftimeCreation );
        FileTimeToDosDateTime( &FindData.ftLastWriteTime,
                               (LPWORD)&pffb->fdateLastWrite,
                               (LPWORD)&pffb->ftimeLastWrite );
        pffb->cbFile = FindData.nFileSizeLow;
        pffb->attrFile = FindData.dwFileAttributes;
        strcpy( pffb->achName, FindData.cFileName );
      } /* endif */
      DosError(1);
      if ( fMsg && usRetCode && (usRetCode != ERROR_NO_MORE_FILES) )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg &&
             usRetCode &&
             (usRetCode != ERROR_NO_MORE_FILES) &&
             (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFindCloseLong       DosFindClose: Close a find handle |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFindClose( HDIR hdir, BOOL fMsg );                    |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFindClose                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HDIR     hdir           directory handle                 |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFindClose                              |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindCloseHwnd                                    |
//+----------------------------------------------------------------------------+

USHORT UtlFindCloseLong
(
   HDIR     hdir,                      // directory handle
   BOOL     fMsg                       // if TRUE handle errors in utility
)
{
   return( UtlFindCloseLongHwnd( hdir, fMsg, (HWND) NULL ) );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlFindCloseLongHwnd   DosFindClose: Close a find handle |
//+----------------------------------------------------------------------------+
//|Function call:     UtlFindCloseLongHwnd( HDIR hdir, BOOL fMsg,HWND hwndParent); |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosFindClose                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HDIR     hdir           directory handle                 |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosFindClose                              |
//+----------------------------------------------------------------------------+
//|Function flow:     do                                                       |
//|                     disable OS/2 error handling                            |
//|                     call DosFindClose                                      |
//|                     enable OS/2 error handling                             |
//|                     if fMsg and an error occured then                      |
//|                       call UtlErrorHwnd                                    |
//|                     endif                                                  |
//|                   while fMsg and an error occured and MB code is RETRY     |
//+----------------------------------------------------------------------------+
USHORT UtlFindCloseLongHwnd
(
   HDIR     hdir,                      // directory handle
   BOOL     fMsg,                      // if TRUE handle errors in utility
   HWND     hwndParent
)
{
   USHORT usRetCode = NO_ERROR;        // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code
   do {
      DosError(0);

      if ( FindClose( hdir ) == 0 )
      {
        usRetCode = (USHORT)GetLastError();
      } /* endif */
      // remove file search handle from our list of open
      // file search handles
      UtlRemoveSearchHandle( hdir );

      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = UtlErrorHwnd( usRetCode, 0, 0, NULL, DOS_ERROR, hwndParent );
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}

USHORT DosOpenLong
(
   PSZ      pszFname,                  // path name of file to be opened
   PHFILE   phfOpen,                   // file handle (returned )
   PUSHORT  pusAction,                 // action taken (returned)
   ULONG    ulFSize,                   // file's new size
   USHORT   usAttr,                    // file attribute
   USHORT   fsOpenFlags,               // action take if file exists
   USHORT   fsOpenMode                 // open mode
)
{
    return( UtlOpen(  pszFname, phfOpen, pusAction, ulFSize, usAttr,
                    fsOpenFlags, fsOpenMode, 0L, FALSE ) );
}

/**********************************************************************/
/* Get the drive letter /2 DosCopy as this function fails when copying   */
/* files across FAT/HPFS network boundaries                           */
/**********************************************************************/
CHAR UtlGetDriveFromHandle( HFILE hf )
{
  CHAR chDrive = EOS;
  // look for handle in our handle/drive array
  int i = 0;
  while ( i < MAX_OPEN_FILES )
  {
    if ( DriveHandles[i].hf  == hf )
    {
        // use drive letter and leave loop
        chDrive = DriveHandles[i].chDrive;
        break;
    }
    else
    {
      i++;
    } /* endif */
  } /* endwhile */

  return( chDrive );
} /* end of function UtlGetDriveFromHandle */


// add file search handle and attributes in our list of open
// file search handles
USHORT UtlAddSearchHandle
(
  HDIR        hdirNew,                 // file search handle being added
  USHORT      usAttr                   // file attribute used for search
)
{
  int i;                    // loop index
  int iOldest;              // index of oldest entry
  time_t timeOldest;        // time of oldest entry


  //  get free slot in our table
  timeOldest = OpenFileFinds[0].time;
  iOldest = 0;

  for ( i = 0; i < MAX_FILEFINDHANDLES; i++ )
  {
   if ( OpenFileFinds[i].hdir == NULLHANDLE )
   {
      break;
    }
    else  if ( timeOldest > OpenFileFinds[i].time )
    {
       iOldest = i;
      timeOldest = OpenFileFinds[i].time;
     } /* endif */
  } /* endfor */

  //  use oldest slot if no empty slot is available
  if ( i >= MAX_FILEFINDHANDLES )
  {
    i = iOldest;
  } /* endif */

  //  add new file search handle to table
  time( &(OpenFileFinds[i].time) );
  OpenFileFinds[i].usAttr = usAttr;
  OpenFileFinds[i].hdir = hdirNew;

  return( NO_ERROR );
} /* end of function UtlAddSearchHandle */

// get file attribute flags active for a specific search handle
USHORT UtlAttrOfSearchHandle
(
  HDIR        hdir                     // file search handle
)
{
  USHORT usAttr = 0;
  int i;

  for ( i = 0; i < MAX_FILEFINDHANDLES; i++ )
  {
   if ( OpenFileFinds[i].hdir == hdir )
   {
      usAttr = OpenFileFinds[i].usAttr;
      break;
   } /* endif */
  }  /* endfor */

  return( usAttr );
} /* end of function UtlAttrOfSearchHandle */

// remove file search handle from our list of open
// file search handles
USHORT UtlRemoveSearchHandle
(
  HDIR        hdir                     // file search handle
)
{
  int i;                    // loop index

  //  look for handle in our table
  for ( i = 0; i < MAX_FILEFINDHANDLES; i++ )
  {
   if ( OpenFileFinds[i].hdir == hdir )
    {
      OpenFileFinds[i].hdir = NULLHANDLE;
      break;
     } /* endif */
  } /* endfor */
  return( NO_ERROR );
} /* end of function UtlRemoveSearchHandle */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlGetFileSize       Query file size                     |
//|                   UtlGetFileSizeHwnd                                       |
//+----------------------------------------------------------------------------+
//|Function call:     UtlGetFileSize( HFILE hf, PULONG pulSize, BOOL fMsg );   |
//|                   UtlGetFileSizeHwnd( HFILE hf, PULONG pulSize, BOOL fMsg, |
//|                                       HWND hwnd );                         |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosQFileInfo and GetFileSize       |
//+----------------------------------------------------------------------------+
//|Parameter:         HFILE    hf             file handle                      |
//|                   PULONG   pulSize        pointer to buffer for file size  |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                   HWND                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of called file system functions              |
//+----------------------------------------------------------------------------+
USHORT UtlGetFileSize
(
   HFILE    hf,                        // file handle
   PULONG   pulSize,                   // points to buffer for file size
   BOOL     fMsg                        // if TRUE handle errors in utility
)
{
  return( UtlGetFileSizeHwnd( hf, pulSize, fMsg, NULLHANDLE ) );
} /* end of function UtlgetFileSize */

USHORT UtlGetFileSizeHwnd
(
   HFILE    hf,                        // file handle
   PULONG   pulSize,                   // points to buffer for file size
   BOOL     fMsg,                       // if TRUE handle errors in utility
   HWND     hwndParent
)
{
  USHORT    usRC = NO_ERROR;           // function return code
  DWORD     dwSize;

  fMsg; hwndParent;
  DosError(0);
  dwSize = GetFileSize( hf, NULL );
  DosError(1);
  if ( dwSize == 0xFFFFFFFF )
  {
    usRC = (USHORT)GetLastError();
    *pulSize = 0L;
  }
  else
  {
    *pulSize = dwSize;
  } /* endif */
   return( usRC );
} /* end of function UtlgetFileSizeHwnd */


