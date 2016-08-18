//+----------------------------------------------------------------------------+
//| EQFENTITY.C                                                                |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|   Entity processing functions for IDDOC and DITA entities                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//
// PVCS Section
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
// $Revision: 1.2 $ ----------- 3 Apr 2009
// GQ: - some corrections in entity processing
// 
//
// $Revision: 1.1 $ ----------- 1 Apr 2009
// GQ: - initial put
// 

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqfentity.h>            // Entity handling API

/**********************************************************************************/
/* Internal structures                                                            */
/**********************************************************************************/

typedef struct _ENTITY
{
  int         iIDOffset;
  int         iValueOffs;
  ULONG       ulSegNum;
} ENTITY, *PENTITY;

typedef struct _ENTITYFILE
{
  CHAR        szName[MAX_LONGFILESPEC];          // document name
  ULONG       ulDataLen;                         // length of data area
  PSZ_W       pszData;                           // data area
  int         iNumOfEntities;                    // number of entities
  int         iEntityTableSize;                  // size of entity name table (number of entries) 
  PENTITY     pEntities;                         // ptr to entity table
} ENTITYFILE, *PENTITYFILE;

typedef struct _LOADEDENTITIES
{
  CHAR        szFolObjName;                      // object name of folder
  int         iNumOfFileEntries;                 // number of entity files
  int         iFileTableSize;                    // size of file table (number of entries) 
  PENTITYFILE pFiles;                            // ptr to entity file array
} LOADEDENTITIES, *PLOADEDENTITIES;

// data area used by ScanForEntities
typedef struct _SCANDATA
{
  CHAR_W      szLine[8096];                      // buffer for output line
  CHAR        szOutFile[MAX_LONGFILESPEC];       // name of output file
  CHAR        szBuffer[MAX_LONGFILESPEC];        // general buffer area
} SCANDATA, *PSCANDATA;

/**********************************************************************************/
/* Local variables                                                                */
/**********************************************************************************/

// list of markup tables for entity processing
char szEntityMarkups[][40] =  { "IBMIDDOC", "IBMDITA", "" };

/**********************************************************************************/
/* Local functions                                                                */
/**********************************************************************************/
BOOL LoadEntityFile( PLOADEDENTITIES pEntities, PSZ pszBuffer ); 
int FindDoc( PLOADEDENTITIES pEntities, PSZ pszDocName );
int FindEntity( PENTITYFILE pFile, PSZ_W pszID, ULONG ulSeg );

/**********************************************************************************/
/* API calls                                                                      */
/**********************************************************************************/

BOOL ScanForEntities( PTBDOCUMENT pDoc )
{
  BOOL        fOK = TRUE;
  ULONG       ulSegNum = 1;
  ULONG       ulAddSegNum = 1;
  ULONG       ulTable = STANDARDTABLE;
  FILE        *hOutFile = NULL;
  PSCANDATA   pData = NULL;

  // allocate our data area
  fOK = UtlAlloc( (PVOID *)&pData, 0, sizeof(SCANDATA), ERROR_STORAGE );

  // setup entity file name (we use the RTF subdir for the file)
  if ( fOK )
  {
    strcpy( pData->szOutFile, pDoc->szDocName );
    strcpy( pData->szBuffer, UtlSplitFnameFromPath( pData->szOutFile ) );
    UtlSplitFnameFromPath( pData->szOutFile );
    strcat( pData->szOutFile, BACKSLASH_STR );
    UtlQueryString( QST_ENTITY, pData->szOutFile + strlen(pData->szOutFile), sizeof(pData->szOutFile) );
    UtlMkMultDir( pData->szOutFile, FALSE );
    strcat( pData->szOutFile, BACKSLASH_STR );
    strcat( pData->szOutFile, pData->szBuffer );
  } /* endif */


  // delete any existing entity file
  UtlDelete( pData->szOutFile, 0, FALSE );

  while ( fOK && (ulSegNum <= pDoc->ulMaxSeg) )
  {
    PTBSEGMENT pSeg = EQFBGetFromBothTables( pDoc, &ulSegNum, &ulAddSegNum, &ulTable );

    // scan segment data
    if ( pSeg != NULL )
    {
      PSZ_W pszData = pSeg->pDataW;
      PSZ_W pszOut = pData->szLine;

      while ( *pszData )
      {
        if ( *pszData == L'<' )
        {
          if ( wcsnicmp( pszData, L"<!ENTITY", 8 ) == 0 )
          {
            int iIdLen = 0;

            pszOut = pData->szLine;


            // skip tag
            pszData += 8;

            // find begin of ID
            while ( iswspace( *pszData ) ) pszData++;

            // write segnum to out buffer
            swprintf( pszOut, L"<%lu><", ulSegNum );
            pszOut += wcslen(pszOut);

            // copy ID to outbuffer
            while ( iswalnum( *pszData ) ) 
            {
              *pszOut++ = *pszData++;
              iIdLen++;
            } /*endwhile */

            // add value delimiter
            *pszOut++ = L'>';
            *pszOut++ = L'<';

            // skip any whitespace
            while ( *pszData && (*pszData == L' ') ) pszData++;

            // check for empty ID or system flag (we ignore this entities)
            if ( (iIdLen == 0) || (wcsnicmp( pszData, L"SYSTEM", 6 ) == 0) )
            {
              // ignore entry
            }
            else
            {
              // find value
              while ( *pszData && (*pszData != L'>') && (*pszData != L'\"') ) pszData++;

              // copy value
              if ( *pszData == L'\"' )
              {
                // copy value
                *pszData++;
                while ( *pszData && (*pszData != L'\"') ) *pszOut++ = *pszData++;

                // add value delimiter and linefeed and write line to output file
                *pszOut++ = L'>';
                *pszOut++ = L'\r';
                *pszOut++ = L'\n';
                *pszOut++ = 0;

                // open output file if not done yet
                if ( hOutFile == NULL )
                {
                  hOutFile = fopen( pData->szOutFile, "wb" );
                  if ( hOutFile )
                  {
                    fwrite( UNICODEFILEPREFIX, 1, 2, hOutFile );
                  }
                  else
                  {
                    fOK = FALSE;
                  } /* endif */
                } /* endif */

                // write line
                if ( fOK )
                {
                  fwrite( pData->szLine, wcslen(pData->szLine), sizeof(CHAR_W), hOutFile );
                } /* endif */
              }
              else
              {
                // no value found ignore this entity
              } /* endif */
            } /* endif */
          }
          else
          {
            pszData++;
          } /* endif */
        }
        else
        {
          pszData++;
        } /* endif */
      } /*endwhile */
    } /* endif */
  } /*endwhile */

  if ( hOutFile )
  {
    fclose( hOutFile );
  }

  if ( pData ) UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );

  return( fOK );
} /* end of function ScanForEntities*/


// data area used by LoadEntities
typedef struct _LOADDATA
{
  CHAR        szSearchPath[MAX_EQF_PATH];
  CHAR        szEntityPath[MAX_EQF_PATH];
  CHAR        szBuffer[MAX_LONGFILESPEC];
  WIN32_FIND_DATA FindData;
} LOADDATA, *PLOADDATA;

LONG LoadEntities( PSZ pszFolObject )
{
  BOOL fOK = TRUE;
  PLOADEDENTITIES pEntities = NULL;
  PLOADDATA pData = NULL;
  HANDLE hDir;
  BOOL fMoreFiles = FALSE;

  // allocate load data area
  fOK = UtlAlloc( (PVOID *)&pData, 0, sizeof(LOADDATA), ERROR_STORAGE );

  // allocate entity data area
  if ( fOK ) fOK = UtlAlloc( (PVOID *)&pEntities, 0, sizeof(LOADEDENTITIES), ERROR_STORAGE );

  // setup search path
  if ( fOK )
  {
    UtlMakeEQFPath( pData->szSearchPath, pszFolObject[0], ENTITY_PATH, UtlGetFnameFromPath(pszFolObject) ); 
    strcpy( pData->szEntityPath, pData->szSearchPath );
    strcat( pData->szSearchPath, "\\*.*" );
  } /* endif */

  // loop over all entity files of folder
  if ( fOK )
  {
    hDir = FindFirstFile( pData->szSearchPath, &pData->FindData );
    if ( hDir != INVALID_HANDLE_VALUE )
    {
      do
      {
        if ( (strcmp( pData->FindData.cFileName, ".." ) != 0) &&
             (strcmp( pData->FindData.cFileName, "." ) != 0) )
        {
          strcpy( pData->szBuffer, pData->szEntityPath );
          strcat( pData->szBuffer, BACKSLASH_STR );
          strcat( pData->szBuffer, pData->FindData.cFileName );

          fOK = LoadEntityFile( pEntities, pData->szBuffer ); 
        } /* endif */

        fMoreFiles = FindNextFile( hDir, &pData->FindData );
      } while ( fOK && fMoreFiles );
      FindClose( hDir );
    } /* endif */
  } /* endif */


  // free load data area
  UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );

  return ( fOK ? ((LONG)pEntities) : 0 );
} /* end of function LoadEntities*/

BOOL isEntityMarkup( PSZ pszMarkup )
{
  BOOL fFound = FALSE;
  int i = 0;
  while ( !fFound && (szEntityMarkups[i][0] != EOS) )
  {
    if ( stricmp( szEntityMarkups[i], pszMarkup) ==  0 )
    {
      fFound = TRUE;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( fFound );
} /* end of function isEntityMarkup*/

BOOL SetEntity( LONG hEntity, PSZ_W pszID, ULONG ulSeg, PSZ pszDocument, PSZ_W pszValue )
{
  BOOL fOK = TRUE;

  hEntity; pszID; ulSeg; pszDocument; pszValue;

  return( fOK );
} /* end of function SetEntity */

BOOL isEntity( PSZ_W pszWord )
{
  BOOL fEntity = FALSE;

  int iLen = wcslen( pszWord );

  if ( (iLen > 2) && (pszWord[0] == L'&') && (pszWord[iLen-1] == ';')  )
  {
    fEntity = TRUE;
  } /* endif */
  return( fEntity );
} /* end of function isEntity */

BOOL GetEntityValue( LONG hEntity, PSZ_W pszID, ULONG ulSeg, PSZ pszDocument, PSZ_W pszValue )
{
  BOOL fFound = FALSE;
  PLOADEDENTITIES pEntities = NULL;
  int iDocIndex = -1;
  int iEntityIndex = -1;

  if ( hEntity == 0 )
  {
    return( FALSE );
  } /* endif */

  pEntities = (PLOADEDENTITIES)hEntity;
  
  // search for document in document table
  iDocIndex = FindDoc( pEntities, pszDocument );

  // search in found document first
  if ( iDocIndex != -1 )
  {
    iEntityIndex = FindEntity( pEntities->pFiles + iDocIndex, pszID, ulSeg );
  } /* endif */

  // use found entity or search remaining documents
  if ( iEntityIndex != -1 )
  {
    fFound = TRUE;
    wcscpy( pszValue, pEntities->pFiles[iDocIndex].pszData + pEntities->pFiles[iDocIndex].pEntities[iEntityIndex].iValueOffs );
  }
  else
  {
    int iDoc = 0;
    while ( !fFound && (iDoc < pEntities->iNumOfFileEntries) )
    {
      if ( iDoc != iDocIndex )
      {
        iEntityIndex = FindEntity( pEntities->pFiles + iDoc, pszID, 0xFFFFFFFF );
        if ( iEntityIndex != -1 )
        {
          fFound = TRUE;
          wcsncpy( pszValue, pEntities->pFiles[iDoc].pszData + pEntities->pFiles[iDoc].pEntities[iEntityIndex].iValueOffs, 255 );
          pszValue[255] = 0;
        }
        else
        {
          iDoc++;
        } /* endif */
      }
      else
      {
        iDoc++;
      } /* endif */
    } /*endwhile */
  } /* endif */

  return( fFound );
} /* end of function GetEntityValue */

BOOL SaveEntitites( LONG hEntities, PSZ pszFolObject )
{
  BOOL fOK = TRUE;


  hEntities; pszFolObject;

  return( fOK );
} /* end of function SaveEntitites */

BOOL FreeEntitites( LONG hEntity )
{
  BOOL fOK = TRUE;
  PLOADEDENTITIES pEntities = NULL;
  PENTITYFILE pFile = NULL;

  if ( hEntity == 0 )
  {
    return( TRUE );
  } /* endif */

  pEntities = (PLOADEDENTITIES)hEntity;
  
  pFile = pEntities->pFiles;

  while ( pEntities->iNumOfFileEntries )
  {
    UtlAlloc( (PVOID *)&(pFile->pEntities), 0, 0, NOMSG );
    UtlAlloc( (PVOID *)&(pFile->pszData), 0, 0, NOMSG );
    pEntities->iNumOfFileEntries--;
    pFile++;
  } /*endwhile */

  UtlAlloc( (PVOID *)&pEntities, 0, 0, NOMSG );

  return( fOK );
} /* end of function FreeEntitites*/

/**********************************************************************************/
/* Internal functions                                                             */
/**********************************************************************************/

BOOL LoadEntityFile( PLOADEDENTITIES pEntities, PSZ pszFile )
{
  BOOL fOK = TRUE;

  PENTITYFILE pEntityFile = NULL;                // current entity file area
  int         iEntries = 0;                      // number of entities in entity file area 

  // enlarge entity file area
  if ( pEntities->iNumOfFileEntries >= pEntities->iFileTableSize )
  {
    int iOldSize = pEntities->iNumOfFileEntries * sizeof(ENTITYFILE); 
    int iNewSize = (pEntities->iNumOfFileEntries + 5) * sizeof(ENTITYFILE); 
      
    fOK = UtlAlloc( (PVOID *)&(pEntities->pFiles), iOldSize, iNewSize, ERROR_STORAGE );

    pEntities->iFileTableSize += 5;
  } /* endif */

  // use next free entry in table
  if ( fOK )
  {
    pEntityFile = pEntities->pFiles + pEntities->iNumOfFileEntries;
    pEntities->iNumOfFileEntries++;

    strcpy( pEntityFile->szName, UtlGetFnameFromPath( pszFile ) );
  } /* endif */

  // load entity data into memory
  if ( fOK )
  {
    ULONG ulLen = 0;
    fOK = UtlLoadFileL( pszFile, (PVOID *)&(pEntityFile->pszData), &ulLen, FALSE, FALSE );
    pEntityFile->ulDataLen = ulLen / sizeof(CHAR_W);
  } /* endif */

  // count number of entities in data
  if ( fOK )
  { 
    // the number of entries is the number of lines in the data 
    int iLines = 1;
    ULONG ulPos = 0;
    PSZ_W pszTest = pEntityFile->pszData;

    while ( ulPos < pEntityFile->ulDataLen )
    {
      if ( *pszTest == L'\n' )
      {
        iLines++;
      } /* endif */
      ulPos++;
      pszTest++;
    } /*endwhile */
    iEntries = iLines;
  } /* endif */

  // allocate entity array
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&(pEntityFile->pEntities), 0, iEntries * sizeof(ENTITY), ERROR_STORAGE );
    if ( fOK )
    {
      pEntityFile->iNumOfEntities = 0;
      pEntityFile->iEntityTableSize = iEntries;
    } /* endif */
  } /* endif */

  // fill entity area
  if ( fOK )
  {
    ULONG ulPos = 0;
    PSZ_W pszData = pEntityFile->pszData;
    PENTITY pEntity = pEntityFile->pEntities;

    // skip any BOM
    if ( memcmp( pszData, UNICODEFILEPREFIX, 2 ) == 0 )
    {
      ulPos += 1;
    } /* endif */

    // process data
    while ( ulPos < pEntityFile->ulDataLen )
    {
      int iSegNumOffs = 0;
      int iIDOffs = 0;
      int iValueOffs = 0;

      // remember start of segment number
      if ( (pszData[ulPos] == L'<') ) iSegNumOffs = ulPos + 1;

      // find end of segment number
      while ( (ulPos < pEntityFile->ulDataLen) && (pszData[ulPos] != L'>') && (pszData[ulPos] != L'\n') ) ulPos++;

      // terminate segment number 
      if ( (pszData[ulPos] == L'>') ) pszData[ulPos] = 0;

      // find begin of ID
      while ( (ulPos < pEntityFile->ulDataLen) && (pszData[ulPos] != L'<') && (pszData[ulPos] != L'\n') ) ulPos++;

      // remember start of ID
      if ( (pszData[ulPos] == L'<') ) iIDOffs = ulPos + 1;

      // find end of ID
      while ( (ulPos < pEntityFile->ulDataLen) && (pszData[ulPos] != L'>') && (pszData[ulPos] != L'\n') ) ulPos++;

      // terminate ID 
      if ( pszData[ulPos] == L'>' ) pszData[ulPos] = 0;

      // find start of value
      while ( (ulPos < pEntityFile->ulDataLen) && (pszData[ulPos] != L'<') && (pszData[ulPos] != L'\n') ) ulPos++;

      // remember start of Value
      if ( (pszData[ulPos] == L'<') ) iValueOffs = ulPos + 1;

      // find end of value
      while ( (ulPos < pEntityFile->ulDataLen) && (pszData[ulPos] != L'>') && (pszData[ulPos] != L'\n') ) ulPos++;

      // terminate value
      if ( pszData[ulPos] == L'>' ) pszData[ulPos] = 0;

      // add found entity
      if ( (iSegNumOffs != 0) && (iIDOffs != 0) && (iValueOffs != 0) ) 
      {
        pEntity->ulSegNum = _wtol( pszData + iSegNumOffs );
        pEntity->iIDOffset = iIDOffs;
        pEntity->iValueOffs = iValueOffs;
        pEntity++;
        pEntityFile->iNumOfEntities++;
      } /* endif */

      // skip rest of line
      while ( (ulPos < pEntityFile->ulDataLen) && (pszData[ulPos] != L'\n') ) ulPos++;
      if ( pszData[ulPos] == L'\n' ) ulPos++;

    } /*endwhile */
  } /* endif */

  return( fOK );
} /* end of function LoadEntityFile */

// find the document in our document table
int FindDoc( PLOADEDENTITIES pEntities, PSZ pszDocName )
{
  int iDocIndex = 0;
  BOOL fFound = FALSE;

  while ( !fFound && (iDocIndex < pEntities->iNumOfFileEntries) )
  {
    if ( strcmp( pszDocName, pEntities->pFiles[iDocIndex].szName ) == 0)
    {
      fFound = TRUE;
    }
    else
    {
      iDocIndex++;
    } /* endif */
  } /*endwhile */

  return( fFound ? iDocIndex : -1 );
} /* end of function FindDoc */

int FindEntity( PENTITYFILE pFile, PSZ_W pszID, ULONG ulSeg )
{
  int iEntityIndex = 0;
  BOOL fFound = FALSE;
  PENTITY pEntity = pFile->pEntities;

  while ( !fFound && (iEntityIndex < pFile->iNumOfEntities) && (pEntity->ulSegNum <= ulSeg) )
  {
    PSZ_W pszEntity = pFile->pszData + pEntity->iIDOffset;
    if ( wcsicmp( pszEntity, pszID ) == 0 )
    {
      fFound = TRUE;
    }
    else
    {
      pEntity++;
      iEntityIndex++;
    } /* endif */
  } /*endwhile */

  return( fFound ? iEntityIndex : -1 );
} /* end of function FindEntity */

