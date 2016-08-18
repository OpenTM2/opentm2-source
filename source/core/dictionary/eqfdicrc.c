//+----------------------------------------------------------------------------+
//| EQFDICRC.C                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| Description: Dictionary error code handling                                |
//+----------------------------------------------------------------------------+
//
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_LDB              // dictionary data encoding functions
#include "eqf.h"                  // General Translation Manager include file
#include "eqfdtag.h"              // include tag definitions
#include "eqfdde.h"
#include "eqfdicti.h"             // Private include file of dictionary handler
#include "eqfrdics.h"             // remote include file

USHORT DictRcHandling
(
USHORT       usRc,          // Return code
PSZ          pszDictPath,   // Pointer to full dict path
HTM          htm,           // TM handle (may be undefined or NULLHANDLE)
PSZ          pszServer      // Pointer to servername
)
{
  CHAR szName[MAX_FILESPEC];
  if ( pszDictPath != NULL )
  {
    if ( strchr( pszDictPath,'.' ) && strchr( pszDictPath,'\\' ))
    {
      // Copy dict name from the full path
      Utlstrccpy( szName, UtlGetFnameFromPath( pszDictPath ), DOT );
    }
    else
    {
      // It is not a full path string but a user error/server name
      // szName is set to an empty string in order to avoid Trap D's
      strcpy( szName, "" );
    } /* endif */
  } /* endif */
  else
  {
    // if the name is NULL copy an empty string to the name
    strcpy( szName, "" );
  } /* endelse */
  return( DictRcHandling2( usRc, pszDictPath, htm, pszServer, szName ) );
} /* end of function DictRcHandling2 */

USHORT DictRcHandling2
(
USHORT       usRc,          // Return code
PSZ          pszDictPath,   // Pointer to full dict path
HTM          htm,           // TM handle (may be undefined or NULLHANDLE)
PSZ          pszServer,     // Pointer to servername
PSZ          pszName        // pointer to dictionary name (long or short)
)
{
  CHAR         chString[16];             // Work string
  PSZ          pReplAddr[2];             // Arrey of pointers to replacement strings

  pszServer;
  pReplAddr[0] = pszName;
  htm = htm;

  switch ( usRc )
  {
  //---------------------------------------------------------------------
  case ERROR_PATH_NOT_FOUND:
  case ERROR_INVALID_DRIVE:
    if ( pszDictPath != NULL )
    {
      // Issue the message: drive %1 is not accessible.
      memcpy( chString, pszDictPath, 2 );
      chString[2] = NULC;
      pReplAddr[0] = chString;
      UtlError( ERROR_EQF_DRIVE_NOT_ACCESSIBLE, MB_CANCEL, 1,
                &pReplAddr[0], EQF_ERROR );
    } /* endif */
    break;

    //---------------------------------------------------------------------
  case ERROR_NO_MORE_FILES:
    //issue message that file couldn't be opened
    UtlError( EQFS_FILE_OPEN_FAILED, MB_CANCEL, 1,
              &pReplAddr[0], EQF_ERROR );
    break;

    //---------------------------------------------------------------------
  case ERROR_NOT_ENOUGH_MEMORY:
    // Issue the message: Short on system storage.
    UtlError( ERROR_STORAGE, MB_CANCEL, 0,
              NULL, EQF_ERROR );
    break;

    //---------------------------------------------------------------------
  case DISK_FULL:
    memcpy( chString, pszDictPath, 2 );
    chString[2] = NULC;
    pReplAddr[0] = chString;
    UtlError( ERROR_DISK_IS_FULL, MB_CANCEL, 1,
              &pReplAddr[0], EQF_ERROR );
    break;
    //---------------------------------------------------------------------
  case TMERR_NO_MORE_MEMORY_AVAILABLE:
    UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
    break;
    //---------------------------------------------------------------------
  case TMERR_PROP_NOT_FOUND:
    // If the path name is there issue an error message else
    // issue undefined error message
    if ( pszDictPath != NULL )
    {
      UtlError( ERROR_DICT_NOT_FOUND, MB_CANCEL, 1,
                &pReplAddr[0], EQF_ERROR );
    } /* endif */
    break;
    //---------------------------------------------------------------------
  case BTREE_OPEN_FAILED:
  case BTREE_ACCESS_ERROR:
    UtlError( usRc, MB_CANCEL, 1, pReplAddr, QDAM_ERROR );
    break;
    //---------------------------------------------------------------------
  default:
    UtlError( usRc, MB_CANCEL, 0, NULL, EQF_ERROR );
    break;
  } /* endswitch */

  return usRc;
} /* end of function DictRcHandling2  */


