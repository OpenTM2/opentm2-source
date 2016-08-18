/*! \file
	Description: Tag table related text analysis functions.

	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#include "eqf.h"                  // General Translation Manager include file
#include "eqftag00.h"

#include "eqftai.h"               // Private include file for Text Analysis

/**********************************************************************/
/* Static array of loaded tag tables                                  */
/**********************************************************************/
#define MAX_NUM_OF_TABLES 120          // max. number of loaded tag tables
static LOADEDTABLE TALoadedTables[ MAX_TASK + 1 ][MAX_NUM_OF_TABLES];
static SHORT sTablesInUse[ MAX_TASK + 1 ] = {0};         // number of tag tables currently in use

USHORT TALoadTagTable
(
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  PLOADEDTABLE     *ppTagTable,        // address of caller's tag table pointer
  BOOL             fInternal,          // internal-table flag
  BOOL             fMsg                // show-error-messages flag
)
{
  return( TALoadTagTableHwnd( pszTableName, ppTagTable, fInternal, fMsg, NULLHANDLE ) );
}

USHORT TALoadTagTableHwnd
(
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  PLOADEDTABLE     *ppTagTable,        // address of caller's tag table pointer
  BOOL             fInternal,          // internal-table flag
  BOOL             fMsg,               // show-error-messages flag
  HWND             hwnd                // owner handle for error messages
)
{
  hwnd;
  return( TALoadTagTableExHwnd( pszTableName, ppTagTable, fInternal,
                                0L, fMsg, NULLHANDLE ) );
}

USHORT TALoadTagTableExHwnd            // loads/accesses a tag table
(
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  PLOADEDTABLE     *ppTagTable,        // address of caller's tag table pointer
  BOOL             fInternal,          // internal-table flag
  ULONG            ulFlags,            // option flags
  BOOL             fMsg,               // show-error-messages flag
  HWND             hwnd                // owner handle for error messages
)
{
  CHAR             szTagTableFileName[MAX_EQF_PATH]; // buffer for file name
  PLOADEDTABLE     pTable = NULL;            // ptr for processing of loaded tables
  SHORT            i;                 // loop counter
  USHORT           usRC = NO_ERROR;   // return value of function
  ULONG            ulBytesRead;       // # of bytes read from file
  PTAGTABLE        pTagTable = NULL;  // ptr to in-memory copy of tag table
  USHORT           usTaskId = UtlGetTask();
  /********************************************************************/
  /* check if caller really looks for a tagtable - or only tries to   */
  /* check on us....                                                  */
  /********************************************************************/
  if ( !pszTableName || ! *pszTableName  )
  {
    if ( fMsg )
    {
      UtlErrorHwnd( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR, hwnd );
    } /* endif */
    usRC = ERROR_INVALID_PARAMETER;
  } /* endif */

  /********************************************************************/
  /* Resolve dependencies in specified flags                          */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    // set user exit load flag if one of the exit functions is to be loaded
    if ( ulFlags & TALOADPROTTABLEFUNC )
    {
      ulFlags |= TALOADUSEREXIT;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Look for tag table in our list of loaded tag tables              */
  /********************************************************************/
  if (usRC == NO_ERROR )
  {
    i = 0;
    pTable = TALoadedTables[ usTaskId ];
    while ( (i < sTablesInUse[ usTaskId ]) &&
            (stricmp( pTable->szName, pszTableName ) != 0) )
    {
      i++;
      pTable++;
    } /* endwhile */

    /********************************************************************/
    /* Handle tables not contained in our tag table list                */
    /********************************************************************/
    if ( i >= sTablesInUse[ usTaskId ] )
    {
      /******************************************************************/
      /* Search a free slot in the used tables area                     */
      /******************************************************************/
      pTable = TALoadedTables[ usTaskId ];
      for ( i = 0; i < sTablesInUse[ usTaskId ]; i++, pTable++ )
      {
        /****************************************************************/
        /* break out of loop if a unused slot is found                  */
        /****************************************************************/
        if ( (pTable->szName[0] == EOS) && (pTable->sUseCount <= 0) )
        {
          break;
        } /* endif */

      } /* endfor */

      /******************************************************************/
      /* If no free slot found use a new slot at the end of the table   */
      /******************************************************************/
      if ( i >= MAX_NUM_OF_TABLES )
      {
        // we've reached the end of our static table...
        // ... look for tables currently not in use
        PLOADEDTABLE pFreeTable = NULL;
        pTable = TALoadedTables[ usTaskId ];
        for ( i = 0; i < sTablesInUse[ usTaskId ]; i++, pTable++ )
        {
          if ( pTable->sUseCount <= 0 )
          {
            // if we found another free table space compare time stamps
            if ( pFreeTable != NULL )
            {
              if ( pFreeTable->ulTimeStamp > pTable->ulTimeStamp )
              {
                // timestamp of current table is older, so use this one
                pFreeTable = pTable;
              } /* endif */
            }
            else
            {
              // no free table yet so use this one
              pFreeTable = pTable;
            } /* endif */
          } /* endif */
        } /* endfor */

        if ( pFreeTable )
        {
          // free table data before re-using this entry
          TAFreeTableSpace( pFreeTable );
          memset( pFreeTable, 0, sizeof(LOADEDTABLE) );
          pTable = pFreeTable;
        }
        else
        {
          // no more table space, all tables in use
          return( ERROR_OUT_OF_STRUCTURES );
        } /* endif */
      }
      else if ( i >= sTablesInUse[ usTaskId ] )
      {
        /****************************************************************/
        /* Add a new slot in the table free area                        */
        /****************************************************************/
        pTable = TALoadedTables[ usTaskId ] + sTablesInUse[ usTaskId ];
        memset(pTable, 0, sizeof(LOADEDTABLE));
        sTablesInUse[ usTaskId ]++;
      } /* endif */

      /******************************************************************/
      /* Build fully qualified tag table name for internal tables       */
      /******************************************************************/
      if ( fInternal )
      {
        UtlMakeEQFPath( szTagTableFileName, NULC, TABLE_PATH, NULL );
        strcat( szTagTableFileName, BACKSLASH_STR );
        strcat( szTagTableFileName, pszTableName );
        strcat( szTagTableFileName, EXT_OF_INTTABLE );
      }
      else
      {
        MUGetMarkupTableFileName( pszTableName, NULL, szTagTableFileName, sizeof(szTagTableFileName) );
      } /* endif */

      /******************************************************************/
      /* Load tag table file into memory                                */
      /******************************************************************/
      if ( !UtlLoadFileHwnd( szTagTableFileName, (PVOID *)&pTagTable,
                        &ulBytesRead, FALSE, fMsg, hwnd ) )
      {
         usRC = ERROR_READ_FAULT;        // indicate tag table read error
      }
      else
      {
        /****************************************************************/
        /* Fill/Initialize data area for loaded tag table               */
        /****************************************************************/
        memset( pTable, 0, sizeof(LOADEDTABLE) );
        strcpy( pTable->szName, pszTableName );
        strupr( pTable->szName );
        pTable->pTagTable = pTagTable;
        pTable->sUseCount = 0;

        TATagsToUnicode(pTable, (USHORT)ulBytesRead, 0L);

        // fill version 3 data fields in  LOADEDTABLE structure
        if ( pTable->pTagTable->usVersion >= TAGTABLE_VERSION3 )
        {
          if ( pTable->pTagTable->chMultSubst )
          {
            pTable->chMultSubst    = pTable->pTagTable->chMultSubst;
          }
          else
          {
            pTable->chMultSubst    = CHAR_MULT_SUBST;
          } /* endif */
          if ( pTable->pTagTable->chSingleSubst )
          {
            pTable->chSingleSubst  = pTable->pTagTable->chSingleSubst;
          }
          else
          {
            pTable->chSingleSubst  = CHAR_SNGL_SUBST;
          } /* endif */
          pTable->fUseUnicodeForSegFile  = pTable->pTagTable->fUseUnicodeForSegFile;
          strcpy( pTable->szDescription, pTable->pTagTable->szDescription );
          strcpy( pTable->szDescriptiveName, pTable->pTagTable->szDescriptiveName );
          pTable->usCharacterSet  = pTable->pTagTable->usCharacterSet;
          pTable->fReflow = pTable->pTagTable->fReflow;

        }
        else
        {
          pTable->chMultSubst    = CHAR_MULT_SUBST;
          pTable->chSingleSubst  = CHAR_SNGL_SUBST;
          pTable->fUseUnicodeForSegFile  = FALSE;
          strcpy( pTable->szDescription, EMPTY_STRING );
          strcpy( pTable->szDescriptiveName, EMPTY_STRING );
          pTable->usCharacterSet  = TAGCHARSET_ASCII;
          pTable->fReflow = TAGREFLOW_NOTSPECIFIED;
        } /* endif */
        // store single/multiple substitutions in Unicode as well
        {
		  CHAR chTemp[2];
		  CHAR_W chTempW[2];
          chTemp[1] = EOS;
	      chTemp[0] = pTable->chMultSubst;
		  ASCII2Unicode( &chTemp[0], &chTempW[0], 0L );
		  pTable->chMultSubstW = chTempW[0];
		  chTemp[0] = pTable->chSingleSubst;
		  ASCII2Unicode( &chTemp[0], &chTempW[0], 0L );
		  pTable->chSingleSubstW = chTempW[0];
	    }
	    // store the TRNotesSettings for later use
        ASCII2Unicode( pTable->pTagTable->chTrnote1Text, pTable->chTrnote1TextW, 0L);
        ASCII2Unicode( pTable->pTagTable->chTrnote2Text, pTable->chTrnote2TextW, 0L);
        pTable->ulTRNote1Len = UTF16strlenBYTE(pTable->chTrnote1TextW);
        pTable->ulTRNote2Len = UTF16strlenBYTE(pTable->chTrnote2TextW);
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Load user exit if required and not done yet                      */
  /********************************************************************/
  if ( (usRC == NO_ERROR) &&                     // no error yet and
       (ulFlags & TALOADUSEREXIT) )             // user exit requested
  {
    TALoadExitIfAvailable( pTable );
  } /* endif */

  /********************************************************************/
  /* Get protect table user exit if required and not done yet         */
  /********************************************************************/
  if ( (usRC == NO_ERROR) &&                         // no error yet and
       (ulFlags & TALOADPROTTABLEFUNC) )            // function requested
  {
    TALoadProtectFunctionIfAvailable( pTable );
  } /* endif */

  /********************************************************************/
  /* Get compare context user exit if required and not done yet       */
  /********************************************************************/
  if ( (usRC == NO_ERROR) &&                         // no error yet and
       (ulFlags & TALOADCOMPCONTEXTFUNC) &&          // function requested and
       (pTable->hmodUserExit != NULL) &&             // user exit loaded and
       (pTable->pfnCompareContext == NULL) &&        // function not loaded yet and
       !pTable->fCompareContextLoadFailed )          // function address did not fail yet
  {
    pTable->pfnCompareContext = NULL;

    usRC = DosGetProcAddr( pTable->hmodUserExit,
                           EQFCOMPARECONTEXT_EXIT,
                           (PFN*)&(pTable->pfnCompareContext) );
    if ( usRC != NO_ERROR )
    {
      pTable->fCompareContextLoadFailed = TRUE;
    } /* endif */
    usRC = NO_ERROR;  // reset error condition to allow continuation
  } /* endif */

  /********************************************************************/
  /* Get compare context user exit if required and not done yet       */
  /********************************************************************/
  if ( (usRC == NO_ERROR) &&                         // no error yet and
       (ulFlags & TALOADCOMPCONTEXTFUNC) &&          // function requested and
       (pTable->hmodUserExit != NULL) &&             // user exit loaded and
       (pTable->pfnCompareContext == NULL) &&        // function not loaded yet and
       !pTable->fCompareContextLoadFailed )          // function address did not fail yet
  {
    pTable->pfnCompareContext = NULL;

    usRC = DosGetProcAddr( pTable->hmodUserExit,
                           EQFCOMPARECONTEXT_EXIT,
                           (PFN*)&(pTable->pfnCompareContext) );
    if ( usRC != NO_ERROR )
    {
      pTable->fCompareContextLoadFailed = TRUE;
    } /* endif */
    usRC = NO_ERROR;  // reset error condition to allow continuation
  } /* endif */

  /********************************************************************/
  /* Get get seg context user exit if required and not done yet       */
  /********************************************************************/
  if ( (usRC == NO_ERROR) &&                         // no error yet and
       (ulFlags & TALOADGETSEGCONTEXTFUNC) &&        // function requested and
       (pTable->hmodUserExit != NULL) &&             // user exit loaded and
       (pTable->pfnGetSegContext == NULL) &&        // function not loaded yet and
       !pTable->fGetSegContextLoadFailed )          // function address did not fail yet
  {
    pTable->pfnGetSegContext = NULL;

    usRC = DosGetProcAddr( pTable->hmodUserExit,
                           EQFGETSEGCONTEXT_EXIT,
                           (PFN*)&(pTable->pfnGetSegContext) );
    if ( usRC != NO_ERROR )
    {
      pTable->fGetSegContextLoadFailed = TRUE;
    } /* endif */
    usRC = NO_ERROR;  // reset error condition to allow continuation
  } /* endif */


  /********************************************************************/
  /* Get query exit info function address if required and not done yet*/
  /********************************************************************/
  if ( (usRC == NO_ERROR) &&                         // no error yet and
       (ulFlags & TALOADQUERYEXITINFOFUNC) &&        // function requested and
       (pTable->hmodUserExit != NULL) &&             // user exit loaded and
       (pTable->pfnQueryExitInfo == NULL) &&         // function not loaded yet and
       !pTable->fQueryExitInfoLoadFailed )           // function address did not fail yet
  {
    pTable->pfnQueryExitInfo = NULL;

    usRC = DosGetProcAddr( pTable->hmodUserExit,
                           EQFQUERYEXITINFO_EXIT,
                           (PFN*)&(pTable->pfnQueryExitInfo) );
    if ( usRC != NO_ERROR )
    {
      pTable->fQueryExitInfoLoadFailed = TRUE;
    } /* endif */
    usRC = NO_ERROR;  // reset error condition to allow continuation
  } /* endif */


  // Increment tag table use count and update time stamp
  if ( usRC == NO_ERROR )
  {
    pTable->sUseCount++;
    UtlTime( (PLONG) &(pTable->ulTimeStamp) );
  } /* endif */

  /********************************************************************/
  /* Set caller's tag table pointer                                   */
  /********************************************************************/
  *ppTagTable = ( usRC == NO_ERROR ) ? pTable : NULL;


  return ( usRC );
} /* end of TALoadTagTable(Hwnd) */

USHORT TALoadExitIfAvailable( PLOADEDTABLE pTable )
{
  USHORT usRC = NO_ERROR;

  if ( (pTable->pTagTable->szSegmentExit[0]) &&  // tag table has a user exit and
       (pTable->hmodUserExit == NULL) &&         // user exit not loaded yet and
       !pTable->fUserExitLoadFailed )            // user exit did not fail yet
  {
    CHAR  szExit[MAX_LONGFILESPEC];              // buffer for user exit DLL name

    if ( MUGetUserExitFileName( pTable->szName, NULL, szExit, sizeof(szExit) ) )
    {
      usRC = DosLoadModule( NULL, 0, szExit, &(pTable->hmodUserExit) );

      if ( usRC != NO_ERROR )
      {
        pTable->hmodUserExit = NULL;
        pTable->fUserExitLoadFailed = TRUE;
      } /* endif */
    }
    else
    {
      pTable->hmodUserExit = NULL;
      pTable->fUserExitLoadFailed = TRUE;
    } /* endif */

  } /* endif */
  return ( usRC );
} /* end of function TALoadExitIfAvailable */


USHORT TALoadProtectFunctionIfAvailable( PLOADEDTABLE pTable )
{
  USHORT usRC = 0;
  USHORT usRCW = 0;

  if ( (pTable->hmodUserExit != NULL) &&             // user exit loaded and
       (pTable->pfnProtTable == NULL) &&             // function not loaded yet and
       (pTable->pfnProtTableW == NULL) &&            // function not loaded yet and
       !pTable->fProtTableLoadFailed   )             // function address did not fail yet
  {
    pTable->pfnProtTable = NULL;
    pTable->pfnProtTableW = NULL;

    usRCW = DosGetProcAddr( pTable->hmodUserExit,
                            EQFPROTTABLEW_EXIT,
                            (PFN*)&(pTable->pfnProtTableW) );
    usRC = DosGetProcAddr( pTable->hmodUserExit,
                            EQFPROTTABLE_EXIT,
                            (PFN*)&(pTable->pfnProtTable) );
    if ( (usRC != NO_ERROR) && (usRCW != NO_ERROR) )
    {
      pTable->fProtTableLoadFailed = TRUE;
    } /* endif */
  } /* endif */
  return( usRC ? usRC : usRCW );
} /* end of function TALoadProtectFunctionIfAvailable */


USHORT TAFreeTagTable
(
  PLOADEDTABLE     pTagTable          // caller's tag table pointer
)
{
  USHORT           usRC = NO_ERROR;   // return value of function
  USHORT           usTaskId = UtlGetTask();

  /********************************************************************/
  /* Check if supplied tag table pointer points into our static       */
  /* tag table array                                                  */
  /********************************************************************/
  if ( (pTagTable < TALoadedTables[ usTaskId ] ) ||
       (pTagTable >= (TALoadedTables[ usTaskId ] + sTablesInUse[ usTaskId ])) )
  {
    usRC = ERROR_INVALID_DATA;
  } /* endif */

  /********************************************************************/
  /* Decrement tag table use count                                    */
  /********************************************************************/
  if ( (usRC == NO_ERROR) && (pTagTable->sUseCount > 0) )
  {
    if ( pTagTable->sUseCount != 0 ) pTagTable->sUseCount--;
//  donot free tagtables due to very bad performance impact in memory organize
//  if ( pTagTable->sUseCount == 0 )
//  {
//    TAFreeTableSpace( pTagTable );
//    memset( pTagTable, 0, sizeof(LOADEDTABLE) );
//  } /* endif */
  } /* endif */

  return ( usRC );

} /* end of TAFreeTagTable */


USHORT TAInvalidateTagTable
(
  PSZ  pszTableFileName                // fully qualified tag table name
)
{
  CHAR             szTableName[MAX_FNAME];       // buffer for tag table name
  PLOADEDTABLE     pTable;            // ptr for processing of loaded tables
  SHORT            i;                 // loop counter
  USHORT           usTaskId = UtlGetTask();

  /********************************************************************/
  /* Isolate tag table name                                           */
  /********************************************************************/
  Utlstrccpy( szTableName, UtlGetFnameFromPath( pszTableFileName ), DOT );

  /********************************************************************/
  /* Search tag table in our array of loaded tag tables               */
  /********************************************************************/
  i = 0;
  pTable = TALoadedTables[ usTaskId ];
  while ( (i < sTablesInUse[ usTaskId ]) &&
          (stricmp( pTable->szName, szTableName ) != 0) )
  {
    i++;
    pTable++;
  } /* endwhile */

  /********************************************************************/
  /* Invalidate found table, free all memory of table if use count    */
  /* is zero                                                          */
  /********************************************************************/
  if ( i < sTablesInUse[ usTaskId ] )
  {
    /******************************************************************/
    /* Invalidate tag table by clearing the tag table name            */
    /******************************************************************/
    pTable->szName[0] = EOS;

    /******************************************************************/
    /* Free all memory occupied by tag table if table is not in use   */
    /* anymore                                                        */
    /******************************************************************/
    if ( pTable->sUseCount <= 0 )
    {
      TAFreeTableSpace( pTable );
    } /* endif */
  } /* endif */

  return( NO_ERROR );
} /* end of TAInvalidateTagTable */


/**********************************************************************/
/* Free space occupied by a loaded tag table                          */
/**********************************************************************/
USHORT TAFreeTableSpace
(
  PVOID            pvTable             // ptr to loaded tag table
)
{
  PNODEAREA        pArea, pRoot;       // ptr for node area processing
  PLOADEDTABLE     pTable = (PLOADEDTABLE)pvTable;

  /********************************************************************/
  /* Free space of in-memory copy of tag table                        */
  /********************************************************************/
  if ( pTable->pTagTable != NULL )
  {
    UtlAlloc( (PVOID *)&pTable->pTagTable, 0L, 0L, NOMSG );
  } /* endif */
  if (pTable->pTagTableW != NULL)
  {
    UtlAlloc( (PVOID *)&pTable->pTagTableW, 0L, 0L, NOMSG);
  }
  /********************************************************************/
  /* Free space of EQFFastTokenize tree areas                         */
  /********************************************************************/
  if ( pTable->pTagTree != NULL )
  {
    UtlAlloc( (PVOID *)&pTable->pTagTree, 0L, 0L, NOMSG );
  } /* endif */
  if ( pTable->pAttrTree != NULL )
  {
    UtlAlloc( (PVOID *) &pTable->pAttrTree, 0L, 0L, NOMSG );
  } /* endif */

  /********************************************************************/
  /* Free space of TATagTokenize node areas                           */
  /********************************************************************/
  pRoot = pTable->pNodeArea;
  if ( pRoot )
  {
     do
     {
       pArea = pRoot;
       pRoot = pArea->pNext;
       UtlAlloc( (PVOID *)&pArea, 0L, 0L, NOMSG );
     } while ( pRoot ); /* enddo */
     pTable->pNodeArea = NULL;
  } /* endif */

  pRoot = pTable->pAttrNodeArea;
  if ( pRoot )
  {
     do
     {
       pArea = pRoot;
       pRoot = pArea->pNext;
       UtlAlloc( (PVOID *)&pArea, 0L, 0L, NOMSG );
     } while ( pRoot ); /* enddo */
     pTable->pAttrNodeArea = NULL;
  } /* endif */

  /********************************************************************/
  /* Free any loaded module                                           */
  /********************************************************************/
  if ( pTable->hmodUserExit != NULL )
  {
    DosFreeModule( pTable->hmodUserExit );
    pTable->hmodUserExit = NULL;
  } /* endif */
  return( NO_ERROR );
} /* end of TAFreeTableSpace */

/**********************************************************************/
/* Free all memory occupied by tag tables (don't care for use count!) */
/**********************************************************************/
USHORT TAFreeAllTagTableSpace()
{
  USHORT           usRC = NO_ERROR;   // return value of function
  PLOADEDTABLE     pTable;             // ptr to loaded tag table
  int              i;                  // loop counter
  USHORT           usTaskId = UtlGetTask();

  i = 0;
  pTable = TALoadedTables[ usTaskId ];
  while ( i < sTablesInUse[ usTaskId ] )
  {
    TAFreeTableSpace( pTable );
    memset( pTable, 0, sizeof(LOADEDTABLE) );
    i++;
    pTable++;
  } /* endwhile */
  sTablesInUse[ usTaskId ] = 0;

  return ( usRC );

} /* end of TAFreeTagTable */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TAGetTagNameByID
//------------------------------------------------------------------------------
// Description:       function will return the name for the specified tag id
//                    in the provided buffer.
//                    If FALSE is returned, no tag with this ID exist.
//------------------------------------------------------------------------------
// Parameters:
//                    PVOID         pVoidTable,       pointer to tag table
//                    SHORT         sTagID            tag id
//                    PSZ           pTagName          ptr to buffer for tagname
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   tag name found and buffer filled with it
//                    FALSE  no tag found
//------------------------------------------------------------------------------
// Function flow:     check input parameters
//                    if okay,
//                      get tagname for specified id and fill it in buffer
//                    return success indicator
//------------------------------------------------------------------------------

BOOL TAGetTagNameByID
(
  PVOID         pVoidTable,            // pointer to tag table
  SHORT         sTagID,                // tag id
  PSZ           pszName                // buffer for tag name
)
{
  BOOL         fOK = TRUE;             // success indicator
  PLOADEDTABLE pTable;                 // pointer to tag table

  pTable = (PLOADEDTABLE) pVoidTable;

  /********************************************************************/
  /* check input parameters                                           */
  /********************************************************************/

  if ( !pTable || !pTable->pTagTable )
  {
    /******************************************************************/
    /* to be on the secure side                                       */
    /******************************************************************/
    fOK = FALSE;
  } /* endif */

  /********************************************************************/
  /* tokenize passed data and check if first token is a tag which     */
  /* is column based                                                  */
  /********************************************************************/
  if ( fOK )
  {
    PBYTE pByte    = (PBYTE)pTable->pTagTable;  // used in OFFSETTOPOINTER...
    SHORT sNumTags = (SHORT) pTable->pTagTable->uNumTags;
    PTAG  pTag = OFFSETTOPOINTER(PTAG, pTable->pTagTable->stFixTag.uOffset);
    PSZ   pTagNames = OFFSETTOPOINTER(PSZ, pTable->pTagTable->uTagNames);


    if ( (sTagID < sNumTags) && (sTagID >= 0))
    {
      strcpy( pszName, pTag[sTagID].uTagnameOffs + pTagNames );
    } /* endif */
  } /* endif */

  return fOK;
} /* end of function TAGetTagNameByID */

BOOL TAGetTagNameByIDW
(
  PVOID         pVoidTable,            // pointer to tag table
  SHORT         sTagID,                // tag id
  PSZ_W         pszNameW               // buffer for tag name
)
{

  BOOL         fOK = TRUE;             // success indicator
  PLOADEDTABLE pTable;                 // pointer to tag table

  pTable = (PLOADEDTABLE) pVoidTable;

  /********************************************************************/
  /* check input parameters                                           */
  /********************************************************************/

  if ( !pTable || !pTable->pTagTable )
  {
    fOK = FALSE;
  } /* endif */

  *pszNameW = EOS;
  /********************************************************************/
  /* tokenize passed data and check if first token is a tag which     */
  /* is column based                                                  */
  /********************************************************************/
  if ( fOK )
  {
    PBYTE pByte    = (PBYTE)pTable->pTagTable;  // used in OFFSETTOPOINTER...
    SHORT sNumTags = (SHORT) pTable->pTagTable->uNumTags;
    PTAG  pTag = OFFSETTOPOINTER(PTAG, pTable->pTagTable->stFixTag.uOffset);

    if ( (sTagID < sNumTags) && (sTagID >= 0))
    {
      UTF16strcpy( pszNameW, pTag[sTagID].uTagnameOffs + pTable->pTagNamesW );
    } /* endif */
  } /* endif */

    return fOK;
}

BOOL  TATagsToUnicode
(
    PVOID         pvLoadedTable,
    USHORT        usSize,
    ULONG         ulCP
)
{
    PLOADEDTABLE  pTable = (PLOADEDTABLE)pvLoadedTable;
    BOOL       fOK = TRUE;
    USHORT     usTagCount = 0;
    USHORT     usAttrCount = 0;
    PTAGTABLE  pTagTable = NULL;
    USHORT     i,j = 0;

    PSZ     pCurName;
    PSZ_W   pTmpW;
    PBYTE   pByte;
    PTAG    pTag;
    PSZ     pTagNames;
    PATTRIBUTE  pAttr;


    pTagTable = pTable->pTagTable;
    pByte = (PBYTE) pTagTable;
    pTag = (PTAG) (pByte + pTagTable->stFixTag.uOffset);
    pTagNames = (PSZ) (pByte + pTagTable->uTagNames);
    usTagCount = pTagTable->uNumTags;
    usAttrCount = pTagTable->stAttribute.uNumber;
    pAttr = (PATTRIBUTE) (pByte + pTagTable->stAttribute.uOffset);

    if ( UtlAlloc( (PVOID *)&pTmpW, 0L, usSize * sizeof(CHAR_W), ERROR_STORAGE ) )
    {
        pTable->pTagTableW = pTmpW;
        for ( i = 0; i < usTagCount; i++ )
        {
          pCurName = pTag[i].uTagnameOffs + pTagNames;
          if (pCurName)
          {
            ASCII2Unicode( pCurName, pTmpW + pTag[i].uTagnameOffs, ulCP );
          }
        } /* endfor */
        for ( i= 0; i < usTagCount; i++ )
        {
          pCurName = pTag[i].uEndDelimOffs + pTagNames;
          if (pCurName)
          {
            ASCII2Unicode( pCurName, pTmpW + pTag[i].uEndDelimOffs, ulCP );
          }
        } /* endfor */

         for ( i = 0; i < usAttrCount; i++ )
         {
           pCurName = pTagNames + pAttr[i].uStringOffs;
           if (pCurName)
           {
              ASCII2Unicode(pCurName, pTmpW + pAttr[i].uStringOffs, ulCP );
           }
         } /* endfor */
         for ( i= 0; i < usAttrCount; i++ )
         {
             pCurName = pAttr[i].uEndDelimOffs + pTagNames;
             if (pCurName)
             {
                ASCII2Unicode( pCurName, pTmpW + pAttr[i].uEndDelimOffs, ulCP );
             }
         } /* endfor */
         for ( i = 0; i < 27; i++ )
         {
            pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttributeIndex[i].uOffset);
            for ( j = 0; j < pTagTable->stAttributeIndex[i].uNumber; j++ )
            {
               pCurName = pTagNames + pAttr->uStringOffs;
               if ( pCurName)
               {
                 ASCII2Unicode( pCurName, pTmpW + pAttr->uStringOffs, ulCP);
               }
               pCurName = pTagNames + pAttr->uEndDelimOffs;
               if ( pCurName)
               {
                   ASCII2Unicode( pCurName, pTmpW + pAttr->uEndDelimOffs, ulCP);
               }

               pAttr++;
            } /* endfor */
         } /* endfor */
         pTable->pTagNamesW = pTmpW;
    }
    else
    {
      fOK = FALSE;
    }


    return fOK;
  }  /* end of TATagsToUnicode */

