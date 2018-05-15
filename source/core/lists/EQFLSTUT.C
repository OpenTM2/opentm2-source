/******************************************************************************/
//+----------------------------------------------------------------------------+
//|EQFLSTUT.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2013, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author: Gerhard Queck (QSoft)                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Utilities and other subroutines of the list handler                       |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|  LstReadNoiseExclList   Read an exclusion/noise list                       |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
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
// $Revision: 1.2 $ ----------- 15 Jan 2004
// --RJ: UtlBufWriteConv: add ulAnsiCP param
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
// $Revision: 1.3 $ ----------- 16 Sep 2002
// --RJ: P015855: use Oem-CP for Unicode2Ansi too
//
//
// $Revision: 1.2 $ ----------- 29 Jul 2002
// --RJ: R07197: add cp for conversion functions
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.6 $ ----------- 17 Dec 2001
// RJ: fix Unicode problem in listprocessing
//
//
// $Revision: 1.5 $ ----------- 11 Dec 2001
// --RJ: use EQFGetCPOem() instead of CP_OEMCP
//
//
// $Revision: 1.4 $ ----------- 3 Dec 2001
// RJ: fix Unicode problem in list handling
//
//
// $Revision: 1.3 $ ----------- 25 Sep 2001
// -- RJ: Move function LstBufWrite to EQFUTFIL.c
//
//
// $Revision: 1.2 $ ----------- 18 Sep 2001
// --RJ: unicode enabling
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.5 $ ----------- 10 Apr 2001
// GQ: Adjusted to new type of size parameter of term lists
//
//
// $Revision: 1.4 $ ----------- 14 Feb 2001
// - added handling for different character code pages
//
//
// $Revision: 1.3 $ ----------- 25 Sep 2000
// -- add support for more than 64k segments
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFLSTUT.CV_   1.9   15 Mar 1999 12:17:36   BUILD  $
 *
 * $Log:   K:\DATA\EQFLSTUT.CV_  $
 *
 *    Rev 1.9   15 Mar 1999 12:17:36   BUILD
 * - use UtlgetFileSize instead of UtlQFileInfo
 *
 *    Rev 1.8   20 Apr 1998 11:53:42   BUILD
 * - supply additional parameter of UtlPrintOpen
 *
 *    Rev 1.7   14 Jan 1998 14:30:40   BUILD
 * - changed TEXT to TEXT_TOKEN
 * - enabled compile for Windows 32bit environment
 *
 *    Rev 1.6   24 Nov 1997 11:03:28   BUILD
 * - fixed PTM KBT0129: Not possile to open a NTL or FTL if <TERM> is contained
 *   in the context string
 *
 *    Rev 1.5   26 Feb 1997 17:18:50   BUILD
 * -- Compiler defines for _POE22, _TKT21, and NEWTCSTUFF eliminated
 *
 *    Rev 1.4   15 Apr 1996 10:16:08   BUILD
 * - fixed PTM KWT0515: Exclusion list from additional drives displayed too
 *
 *    Rev 1.3   18 Mar 1996 16:56:26   BUILD
 * - fixed PTM KWT0456: Printing - document names are not consistent
 *
 *    Rev 1.1   10 Jan 1996 18:23:16   BUILD
 * - added handling for addedna term lists
 *
 *    Rev 1.0   09 Jan 1996 09:09:10   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_LIST             // terminology list functions
#define INCL_EQF_PRINT            // print functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file

#include "eqflist.id"             // List Handler IDs
#include "eqflp.h"                // Defines for generic list handlers
#include "eqflisti.h"             // Private List Handler include file

/**********************************************************************/
/* Array with corresponding end tags, index is the start tag          */
/**********************************************************************/
   LISTTAGS LstEndTag[] =
   {
     (LISTTAGS)0,                                // end tag for ECONREF_TAG
     (LISTTAGS)0,                                // end tag for ECONTEXT_TAG
     (LISTTAGS)0,                                // end tag for ECREATEDATE_TAG
     (LISTTAGS)0,                                // end tag for ECREATETIME_TAG
     (LISTTAGS)0,                                // end tag for EDEST_TAG
     (LISTTAGS)0,                                // end tag for EDICTNAME_TAG
     (LISTTAGS)0,                                // end tag for EEXCLDICT_TAG
     (LISTTAGS)0,                                // end tag for EEXCLLISTNAME_TAG
     (LISTTAGS)0,                                // end tag for EFREQUENCY_TAG
     (LISTTAGS)0,                                // end tag for EFTLIST_TAG
     (LISTTAGS)0,                                // end tag for EHEADER_TAG
     (LISTTAGS)0,                                // end tag for ELEMMA_TAG
     (LISTTAGS)0,                                // end tag for EMARK_TAG
     (LISTTAGS)0,                                // end tag for ENTLIST_TAG
     (LISTTAGS)0,                                // end tag for EOUTDICT_TAG
     (LISTTAGS)0,                                // end tag for ETERM_TAG
     (LISTTAGS)0,                                // end tag for ETEXTNAME_TAG
     (LISTTAGS)0,                                // end tag for ETRANSLATION_TAG
     ECONREF_TAG,                      // end tag for CONREF_TAG
     ECONTEXT_TAG,                     // end tag for CONTEXT_TAG
     ECREATEDATE_TAG,                  // end tag for CREATEDATE_TAG
     ECREATETIME_TAG,                  // end tag for CREATETIME_TAG
     EDEST_TAG,                        // end tag for DEST_TAG
     EDICTNAME_TAG,                    // end tag for DICTNAME_TAG
     EEXCLDICT_TAG,                    // end tag for EXCLDICT_TAG
     EEXCLLISTNAME_TAG,                // end tag for EXCLLISTNAME_TAG
     EFREQUENCY_TAG,                   // end tag for FREQUENCY_TAG
     EFTLIST_TAG,                      // end tag for FTLIST_TAG
     EHEADER_TAG,                      // end tag for HEADER_TAG
     ELEMMA_TAG,                       // end tag for LEMMA_TAG
     EMARK_TAG,                        // end tag for MARK_TAG
     ENTLIST_TAG,                      // end tag for NTLIST_TAG
     EOUTDICT_TAG,                     // end tag for OUTDICT_TAG
     ETERM_TAG,                        // end tag for TERM_TAG
     ETEXTNAME_TAG,                    // end tag for TEXTNAME_TAG
     ETRANSLATION_TAG,                 // end tag for TRANSLATION_TAG
     (LISTTAGS)0                                 // end tag for ID_ATTR
   };

#define NO_CONTEXT_ID   0xFFFF         // symbolic value for not set context ID

USHORT LstInsertListNames
(
   LISTTYPES  Type,                    // type of list being listed
   BOOL       fDetailed,               // TRUE = detailed list
   HWND       hwndLB,                  // handle of listbox being filled
   PSZ        pszBuffer,               // pointer to buffer for listbox item text
   BOOL       fDisableUpdate           // disable-listbox-update flag
)
{
  HDIR          hDir = HDIR_CREATE;
  FILEFINDBUF   stResultBuf;
  USHORT        usCount;
  CHAR          szDriveList[MAX_DRIVELIST]; // buffer for MAT drive list
  PSZ           pTemp;                 // helper pointer
  OBJNAME       szObjName;             // buffer for list object names
  BOOL          fCombo;                // TRUE = target is a combobox
  USHORT        usPathID = 0;          // path ID for listtype
  PSZ           pszExtension = NULL;   // ptr to list file extension
  BOOL          fOK = TRUE;            // internal O.K. flag
  USHORT        usItems;               // number of items in listbox

  ISCOMBOBOX( hwndLB, fCombo );
  if ( fDisableUpdate )
  {
    ENABLEUPDATEHWND_FALSE( hwndLB );
  } /* endif */
  if ( fCombo )
  {
    CBDELETEALLHWND( hwndLB );
  }
  else
  {
    DELETEALLHWND( hwndLB );
  } /* endif */

  if ( (Type == ABR_TYPE) || (Type == ADD_TYPE) )
  {
    UtlFillTableLB( hwndLB, SOURCE_LANGUAGES );
  }
  else
  {
    /****************************************************************/
    /* Build list containing only EQF primary drive (to scan all    */
    /* EQF drives use UtlQueryString with QST_VALIDEQFDRIVES to fill*/
    /* szDriveList with the list of all valid EQF drives)           */
    /****************************************************************/
    UtlQueryString( QST_PRIMARYDRIVE, szDriveList, sizeof(szDriveList) );
    szDriveList[1] = EOS;              // leave only drive letter in string

    /****************************************************************/
    /* Loop through all drives in drive lists and add found lists   */
    /* to listbox                                                   */
    /****************************************************************/
    pTemp = szDriveList;             // start with first drive
    while ( fOK && (*pTemp != EOS) )
    {
      /**************************************************************/
      /* Build search path for lists                                     */
      /**************************************************************/
      switch ( Type )
      {
        case NTL_TYPE :
          usPathID = LIST_PATH;
          pszExtension = EXT_OF_NEWTERMS_LIST;
          break;

        case FTL_TYPE :
          usPathID = LIST_PATH;
          pszExtension = EXT_OF_FOUNDTERMS_LIST;
          break;

        case NOISE_TYPE :
          usPathID = TABLE_PATH;
          pszExtension = EXT_OF_EXCLUSION;
          break;

        case EXCL_TYPE :
          usPathID = LIST_PATH;
          pszExtension = EXT_OF_EXCLUSION;
          break;
        default :
          fOK = FALSE;
          break;
      } /* endswitch */

      if ( fOK )
      {
        UtlMakeEQFPath( pszBuffer, *pTemp, usPathID, NULL );
        strcat( pszBuffer, BACKSLASH_STR );
        strcat( pszBuffer, DEFAULT_PATTERN_NAME );
        strcat( pszBuffer, pszExtension );

        /**************************************************************/
        /* Search for lists on a specific drive                       */
        /**************************************************************/
        usCount = 1;
        hDir = HDIR_CREATE;
        if ( UtlFindFirst( pszBuffer, &hDir, FILE_NORMAL,
                      &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0) )
        {
          usCount = 0;  // no files as return code is set
        } /* endif */
        while ( usCount )
        {
          /************************************************************/
          /* Build listbox item string                                */
          /************************************************************/
          if ( !fDetailed )
          {
            Utlstrccpy( pszBuffer, RESBUFNAME(stResultBuf), DOT );
          }
          else
          {
            UtlMakeEQFPath( szObjName, *pTemp, usPathID, NULL );
            strcat( szObjName, BACKSLASH_STR );
            strcat( szObjName, RESBUFNAME(stResultBuf) );
            {
              FDATE fDate;
              FTIME fTime;
              FileTimeToDosDateTime( &stResultBuf.ftLastWriteTime,
                                     (LPWORD)(PVOID)&fDate, (LPWORD)(PVOID)&fTime );
              LstBuildListboxItem( pszBuffer, szObjName, &fDate,
                                   &fTime, RESBUFSIZE(stResultBuf) );
            }
          } /* endif */

          /************************************************************/
          /* Add list to listbox                                      */
          /************************************************************/
          if ( fCombo )
          {
            CBINSERTITEMHWND( hwndLB, pszBuffer );
          }
          else
          {
            INSERTITEMHWND( hwndLB, pszBuffer );
          } /* endif */

          /************************************************************/
          /* Find next list                                           */
          /************************************************************/
          UtlFindNext( hDir, &stResultBuf, sizeof( stResultBuf), &usCount, 0);
        } /* endwhile */

        //  close file search handle
        if ( hDir != HDIR_CREATE ) UtlFindClose( hDir, FALSE );

      } /* endif */

      /**************************************************************/
      /* Skip to next drive in drive list                           */
      /**************************************************************/
      pTemp++;
    } /* endwhile */
  } /* endif */

  if ( fCombo )
  {
    usItems = CBQUERYITEMCOUNTHWND( hwndLB );
  }
  else
  {
    usItems = QUERYITEMCOUNTHWND( hwndLB );
  } /* endif */

  if ( fDisableUpdate )
  {
    ENABLEUPDATEHWND_TRUE( hwndLB );
  } /* endif */

  return( usItems );
} /* end of function LstInsertListNames */



/**********************************************************************/
/* Global variables                                                   */
/**********************************************************************/
static PSZ pszGlobTermBuf;             // global term buffer pointer (is
                                       // required for qsort in
                                       // LstExclListWrite)

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstBuildListboxItem   build an item for the list listbox |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Combine all passed data and form the text if a list box  |
//|                   item for list handler lists list box.                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ        pszItem        ptr to buffer for item text    |
//|                   PSZ        pszListObjName pointer to list object name    |
//|                   PFDATE     pstDate        file date of list file         |
//|                   PFTIME     pstTime        file time of list file         |
//|                   ULONG      ulListSize     size of list file              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       length of created item text                              |
//+----------------------------------------------------------------------------+
//|Function flow:     fill drive, name and date/time values                    |
//|                   setup list box item string                               |
//|                   return size fo created string                            |
//+----------------------------------------------------------------------------+
USHORT LstBuildListboxItem
(
   PSZ        pszItem,                 // pointer to buffer for listbox item text
   PSZ        pszListObjName,          // pointer to list object name
   PFDATE     pstDate,                 // file date of list file
   PFTIME     pstTime,                 // file time of list file
   ULONG      ulListSize               // size of list file
)
{
  CHAR        szName[MAX_FNAME];       // buffer for name of list
  CHAR        szDrive[MAX_DRIVE];      // drive letter of list

  /********************************************************************/
  /* Fill drive, name and date/time values                            */
  /********************************************************************/
  strncpy( szDrive, pszListObjName, 2 );
  szDrive[2] = EOS;
  Utlstrccpy( szName, UtlGetFnameFromPath( pszListObjName ), DOT );

  /********************************************************************/
  /* Build listbox item string                                        */
  /********************************************************************/
  sprintf( pszItem,
           "%s\x15%s\x15%s\x15%ld\x15%u\x15%u.%u",
           pszListObjName,
           szName,
           szDrive,
           ulListSize,
           *((PUSHORT)pstDate),
           *((PUSHORT)pstDate),
           *((PUSHORT)pstTime) );

  /********************************************************************/
  /* Return size of string to calling function                        */
  /********************************************************************/
  return( (USHORT)strlen( pszItem ) );

} /* end of function LstBuildListboxItem */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstReadSGMLList                                          |
//+----------------------------------------------------------------------------+
//|Function call:     LstReadSGMLList( PSZ pszListName,                        |
//|                                    PLISTHEADER pListHeader,                |
//|                                    PTERMTABLE *ppTermTable,                |
//|                                    PCONTEXTTABLE *ppContextTable,          |
//|                                    PPOOL *ppPool,                          |
//|                                    BOOL fExternal,                         |
//|                                    LISTTYPES Type );                       |
//+----------------------------------------------------------------------------+
//|Description:       Read a list in the new-terms-list / found-terms-list     |
//|                   SGML format (internal or external) and tokenize the list.|
//|                   Create Term and context table(s) for the tokenized data. |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ          pszListName       fully qualified list name |
//|                   PLISTHEADER  pListHeader       ptr to list header        |
//|                   PTERMTABLE   *ppTermTable      address of term table ptr |
//|                   PCONTEXTTABLE *ppContextTable  address of context table p|
//|                   PPOOL        *ppPool           address of string pool ptr|
//|                   BOOL         fExternal         TRUE = read ext. format   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                        function completed successfully |
//|                   ERROR_NOT_ENOUGH_STORAGE memory allocation failed        |
//|                   ERROR_BAD_FORMAT         SGML format is incorrect        |
//+----------------------------------------------------------------------------+
//|Side effects:      - Term table is created and filled with term data        |
//|                   - Context table and pool is created and filled with      |
//|                     the contexts in the list                               |
//|                   - Term list header data is filled                        |
//+----------------------------------------------------------------------------+
//|Function flow:     Initialize all variables                                 |
//|                   Set start processing state                               |
//|                   Create the string pool                                   |
//|                   Open the list                                            |
//|                   Get the list size                                        |
//|                   Allocate input and token buffer                          |
//|                   Load list format tag table                               |
//|                   while ok and not end of list                             |
//|                      Fill input buffer                                     |
//|                      Tokenize data in input buffer                         |
//|                      convert tokens to entries in header or term table     |
//|                   endwhile                                                 |
//|                   Cleanup                                                  |
//+----------------------------------------------------------------------------+
USHORT LstReadSGMLList
(
  PSZ          pszListName,            // fully qualified name of list
  PLISTHEADER  pListHeader,            // ptr to list header data structure
  PTERMTABLE   *ppTermTable,           // address of term table pointer
  PCONTEXTTABLE *ppContextTable,       // address of context table pointer
  PPOOL        *ppPool,                // address of string pool pointer
  BOOL         fExternal,              // TRUE = read external format list
  LISTTYPES    Type,                   // type of list expected
  USHORT       usFormat                // (character) format of data (ANSI,ASCII,...)
)
{
#define MAX_CONTEXT 0xF000
  USHORT       usOpenAction;           // action performed by DosOpen
  USHORT       usDosRC;                // return code of DosXXX calls
  ULONG        ulBytesToRead;          // # of bytes to read from file
  ULONG        ulBytesRead = 0;        // # of bytes read from file
  HFILE        hList = NULLHANDLE;     // handle of list file
  ULONG        ulBytesInBuffer = 0;    // # of bytes in input buffer
  ULONG        ulRemaining = 0;        // # of bytes in document not processed
  PSZ_W        pRest = NULL;           // ptr to start of not-processed bytes
  USHORT       usColPos = 0;           // column pos used by EQFTagTokenize
  PTOKENENTRY  pTok;                   // ptr for token table processing
  USHORT       usRC = 0;               // function return code
  enum
  {
    LISTPREFIX_STATE,                  // nothing in list processed yet
    BEFOREHEADER_STATE,                // waiting for header state
    LISTHEADER_STATE,                  // processing list header
    TERMDATA_STATE,                    // processing term data
    INBETWEEN_STATE,                   // no structure is currently active
    ENDOFLIST_STATE                    // list end has been encountered
  } CurrentState;                      // current tokenization state
  SHORT        sEndTag = 0;                // value of expected end tag
  PLOADEDTABLE pTagTable = NULL;       // pointer to list format tag table
  PBYTE        pInBuf    = NULL;       // pointer to input buffer
  PBYTE        pConvBuf    = NULL;     // pointer to conversion buffer
  PBYTE        pTokBuf   = NULL;       // pointer to token buffer
  PSZ_W        pContextBuf   = NULL;   // pointer to buffer for context
  BOOL         fTagOpen      = FALSE;  // TRUE if we are waiting for an end tag
  PSZ          pError;                 // ptr to parameter for UtlError call
  PSZ_W        pErrorW;
  PSZ_W        pszSource;              // ptr for buffer processing
  USHORT       usI;                    // general loop index
  SHORT        sEndListTag = 0;        // expected end list tag
  CHAR_W       chTemp;                 // temporary buffer for single characters
  TERM         TermData;               // data of current term
  ULONG        ulDataLength = 0L;      // for test purposes: overall data length
  USHORT       usContextID = 0;        // ID of a context string
  USHORT       usGeneratedID = 1;      // ID for generated context IDs
  BOOL         fContextMode = FALSE;   // TRUE = process text of context
  CHAR         szTemp[MAX_EQF_PATH];
  PSZ          pszTemp= NULL;
  ULONG        ulOemCP = 0L;

   /*******************************************************************/
   /* Initialize all variables                                        */
   /*******************************************************************/
   *ppPool = NULL;
   memset( pListHeader, 0, sizeof(*pListHeader) );
   *ppContextTable = NULL;
   *ppTermTable    = NULL;

   /*******************************************************************/
   /* Set start processing state                                      */
   /*******************************************************************/
   CurrentState = LISTPREFIX_STATE;

   /*******************************************************************/
   /* Create the string pool                                          */
   /*******************************************************************/
   *ppPool = PoolCreate( 32000 );
   if ( !*ppPool )
   {
     UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR);
     usRC = ERROR_NOT_ENOUGH_MEMORY;
   } /* endif */

   /*******************************************************************/
   /* Open the list                                                   */
   /*******************************************************************/
   if ( !usRC )
   {
      usRC = UtlOpen( pszListName,
                      &hList,
                      &usOpenAction, 0L,
                      FILE_NORMAL,
                      FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                      0L,
                      TRUE);
   } /* endif */

   /*******************************************************************/
   /* Get the list size                                               */
   /*******************************************************************/
   if ( !usRC )
   {
     usRC = UtlGetFileSize( hList, &ulRemaining, TRUE );
     if ( !ulRemaining )
     {
       pError = UtlGetFnameFromPath( pszListName );
       UtlError( NO_VALID_FORMAT, MB_CANCEL, 1, &pError, EQF_ERROR );
       usRC = ERROR_BAD_FORMAT;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Allocate input buffer, token buffer, and context buffer         */
   /*******************************************************************/
   if ( !usRC )
   {
     UtlAlloc( (PVOID *) &pInBuf, 0L, (LONG) IO_BUFFER_SIZE*sizeof(CHAR_W), ERROR_STORAGE );

     if ( pInBuf )
     {
       UtlAlloc( (PVOID *) &pConvBuf, 0L, (LONG) IO_BUFFER_SIZE*sizeof(CHAR_W), ERROR_STORAGE );
     } /* endif */

     if ( pConvBuf )
     {
       UtlAlloc( (PVOID *) &pTokBuf, 0L, (LONG) TOK_BUFFER_SIZE, ERROR_STORAGE );
     } /* endif */

     if ( pInBuf && pTokBuf )
     {
       UtlAlloc( (PVOID *)&pContextBuf, 0L, (LONG)MAX_CONTEXT * sizeof(CHAR_W), ERROR_STORAGE );
     } /* endif */

     usRC = (USHORT)(( pInBuf && pConvBuf && pTokBuf && pContextBuf ) ? 0 : ERROR_NOT_ENOUGH_MEMORY);
   } /* endif */

   /*******************************************************************/
   /* Load list format tag table                                      */
   /*******************************************************************/
   if ( !usRC )
   {
     ulOemCP = GetLangOEMCP(NULL);
     usRC = TALoadTagTable( LISTFORMATTABLE, &pTagTable, TRUE, TRUE );
   } /* endif */

   if (!usRC && (usFormat == SGMLFORMAT_UNICODE))
   {
     PSZ   pData = (PSZ)pInBuf;
     int   iLen = strlen(UNICODEFILEPREFIX);
     ULONG ulTemp;

      UtlReadL(hList, pData, 8, &ulBytesRead, FALSE );
      if (memcmp( pData, UNICODEFILEPREFIX, iLen) == 0 )
      {
        //position right behind prefix
        UtlChgFilePtr( hList, iLen, FILE_BEGIN, &ulTemp, FALSE );
        // skip prefix
        ulRemaining -= iLen;
      }
      else
      {
        if (fExternal)
        {
           usRC = UtlError( NO_VALID_UNICODEFORMAT, MB_YESNO, 1, &pszListName,
                                     EQF_WARNING );

           if ( usRC == MBID_NO )
           {
              usRC = ERROR_INVALID_DATA;
           } /* endif */
           else
           {
             //position at begin again and read from begin
             UtlChgFilePtr( hList, 0L, FILE_BEGIN, &ulTemp, FALSE);
           }
         }
         else
         {
           // FTL/NTL list is in ASCII - migrate to TM/Unicode by read in as ASCII!
           usFormat = SGMLFORMAT_ASCII;
           //position at begin again and read from begin
           UtlChgFilePtr( hList, 0L, FILE_BEGIN, &ulTemp, FALSE);
         }
      }

   }
   /*******************************************************************/
   /* Read/tokenize input until complete                              */
   /*******************************************************************/
   while ( !usRC && (ulRemaining  || ulBytesInBuffer ))
   {
      /****************************************************************/
      /* Fill input buffer                                            */
      /****************************************************************/
      ulBytesToRead =  min( ulRemaining,
                         (ULONG) (IO_BUFFER_SIZE - ulBytesInBuffer - sizeof(CHAR_W)));
      if ( ulBytesToRead )
      {
        if (usFormat == SGMLFORMAT_UNICODE)
        {
         usDosRC = UtlReadL( hList,
                            pInBuf + ulBytesInBuffer,
                            ulBytesToRead,
                            &ulBytesRead,
                            TRUE );
         ulRemaining -=  ulBytesRead;
        }
        else                          //read ANSI/ASCII and convert them to Unicode
        {
          //USHORT usConvCP = EQFGetCPOem(); /*CP_OEMCP; */    //for SGMLFORMAT_ASCII and default
          // now use system preferences language!!

          ULONG ulBytes;
          ULONG ulAnsiBytesToRead;
          ULONG ulAnsiBytesRead;
          ULONG  ulAnsiCP = 0L;
          USHORT usConvCP = 0;

           ulAnsiCP = GetLangAnsiCP(NULL);
           if (usFormat == SGMLFORMAT_ANSI )
           {
              usConvCP = (USHORT)ulAnsiCP;
           }
           else
           {
             usConvCP = (USHORT)ulOemCP;
           }

           // re-compute bytes to read
           ulAnsiBytesToRead = min( ulRemaining,
                                    (ULONG)((IO_BUFFER_SIZE - ulBytesInBuffer - 1)/sizeof(CHAR_W)) );

           // read data into conversion buffer
           usDosRC = UtlReadL( hList, pConvBuf, ulAnsiBytesToRead,
                              &ulAnsiBytesRead, TRUE );
           ulRemaining -=  ulAnsiBytesRead;
           ulBytesToRead = ulAnsiBytesToRead;
           // convert data to Unicode
           if ( usDosRC == NO_ERROR )
           {

             ulBytes = MultiByteToWideChar( usConvCP, 0,
                                            (LPCSTR)pConvBuf, ulAnsiBytesRead,
                                            (LPWSTR)(pInBuf + ulBytesInBuffer),
                                            ulAnsiBytesRead*sizeof(CHAR_W) );
             ulBytesRead = ulBytes*sizeof(CHAR_W);
           } /* endif */
         }  /* endif */
         ulBytesInBuffer += ulBytesRead;
      } /* endif */

      *((PSZ_W)(pInBuf+ulBytesInBuffer)) = '\0';  // set end of data indicator

      /****************************************************************/
      /* Tokenize data in input buffer                                */
      /****************************************************************/
      if ( !usRC )
      {
         /*************************************************************/
         /* At end of file, remove EOF and CRLF from data             */
         /*************************************************************/
         if ( ulRemaining == 0L )
         {
            pszSource = (PSZ_W)(pInBuf + ulBytesInBuffer);
            pszSource--;            // point to last character
            usI = 3;                // remove not more than 3 chars
            while ( ulBytesInBuffer && usI &&
                    UTF16strchr( L"\r\n\x1A", *pszSource ) )
            {
               *pszSource-- = EOS;
               usI--;
               ulBytesInBuffer -= sizeof(CHAR_W );
            } /* endwhile */
         } /* endif */

         TATagTokenizeW( (PSZ_W)pInBuf,
                        pTagTable,
                        (ulRemaining == 0L),
                        &pRest,
                        &usColPos,
                        (PTOKENENTRY) pTokBuf,
                        TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
      } /* endif */

      if ( !usRC )
      {
         /**********************************************************/
         /* Convert tokens to entries in header or term table      */
         /**********************************************************/
         pTok = (PTOKENENTRY) pTokBuf;
         while ( !usRC && (pTok->sTokenid != ENDOFLIST) )
         {
           if ( fContextMode )
           {
             // context mode: all all strings of all tokens to
             // context buffer until end of context tag is found or
             // an overflow of the context buffer occurs
             if ( pTok->sTokenid == ECONTEXT_TAG)
             {
               // context data has been completed, add context data to
               // string pool and add pointer to context data

               if ( fExternal )
               {
                 // As external files have no context ID, the
                 // context ID has to be generated here
                 usContextID = usGeneratedID++;
               } /* endif */

               if ( usContextID == NO_CONTEXT_ID )
               {
                 usRC = ERROR_BAD_FORMAT; // ID attribute missing
               }
               else
               {
                 pszSource = PoolAddStringW( *ppPool, pContextBuf );
                 if ( pszSource )
                 {
                   usRC = LstAddContext( ppContextTable, usContextID,
                                         pszSource );
                 }
                 else
                 {
                   usRC = ERROR_NOT_ENOUGH_MEMORY;
                 } /* endif */
                 if ( !usRC )
                 {
                   usRC = LstAddContextIDToList( &(TermData.pContextList),
                                                 usContextID );
                 } /* endif */
               } /* endif */
               fContextMode = FALSE;   // terminate context mode
               fTagOpen = FALSE;       // data tag has been closed
             }
             else
             {
               // zero-terminate the data
               chTemp = pTok->pDataStringW[pTok->usLength];
               pTok->pDataStringW[pTok->usLength] = EOS;

               if ( (UTF16strlenCHAR(pContextBuf) + pTok->usLength + 1) < MAX_CONTEXT )
               {
                 UTF16strcat( pContextBuf, pTok->pDataStringW );
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // context too long
               } /* endif */
               pTok->pDataStringW[pTok->usLength] = chTemp;
             } /* endif */
           }
           else switch ( pTok->sTokenid )
           {
             case FTLIST_TAG:
               if ( Type != FTL_TYPE )
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong type of list
               }
               else if ( CurrentState == LISTPREFIX_STATE )
               {
                 /***************************************************/
                 /* List start encountered, switch to before header */
                 /* mode                                            */
                 /***************************************************/
                 CurrentState = ( fExternal ) ? INBETWEEN_STATE :
                                                BEFOREHEADER_STATE;
                 sEndListTag   = EFTLIST_TAG;
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case NTLIST_TAG:
               if ( Type != NTL_TYPE )
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong type of list
               }
               else if ( CurrentState == LISTPREFIX_STATE )
               {
                 /***************************************************/
                 /* List start encountered, switch to before header */
                 /* mode                                            */
                 /***************************************************/
                 CurrentState = ( fExternal ) ? INBETWEEN_STATE :
                                                BEFOREHEADER_STATE;
                 sEndListTag   = ENTLIST_TAG;
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case HEADER_TAG:
               if ( ( CurrentState == BEFOREHEADER_STATE ) && !fExternal )
               {
                 /***************************************************/
                 /* Header start encountered, switch to list header */
                 /* mode                                            */
                 /***************************************************/
                 CurrentState = LISTHEADER_STATE;
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case EHEADER_TAG:
               if ( ( CurrentState == LISTHEADER_STATE ) && !fTagOpen
                      && !fExternal )
               {
                 /*************************************************/
                 /* Header end encountered, switch to in-between  */
                 /* mode                                          */
                 /*************************************************/
                 CurrentState = INBETWEEN_STATE;
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case TEXTNAME_TAG:
             case OUTDICT_TAG:
             case EXCLLISTNAME_TAG:
             case EXCLDICT_TAG:
             case CREATEDATE_TAG:
             case CREATETIME_TAG:
               if ( ( CurrentState == LISTHEADER_STATE ) && !fTagOpen )
               {
                 /*************************************************/
                 /* Start tag for list header data encountered    */
                 /*************************************************/
                 fTagOpen = TRUE;      // data tag is open
                 sEndTag = (SHORT)LstEndTag[pTok->sTokenid];
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case DICTNAME_TAG:
               if ( ( ( CurrentState == LISTHEADER_STATE ) ||
                      ( CurrentState == TERMDATA_STATE ) ) && !fTagOpen )
               {
                 /*************************************************/
                 /* Start tag for dictionary names encountered    */
                 /*************************************************/
                 fTagOpen = TRUE;      // data tag is open
                 sEndTag = (SHORT)LstEndTag[pTok->sTokenid];
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case ETEXTNAME_TAG:
             case EOUTDICT_TAG:
             case EEXCLLISTNAME_TAG:
             case EEXCLDICT_TAG:
             case ECREATEDATE_TAG:
             case ECREATETIME_TAG:
               if ( ( CurrentState == LISTHEADER_STATE ) && fTagOpen &&
                    ( pTok->sTokenid == sEndTag ) )
               {
                 /*************************************************/
                 /* End tag for list header data encountered      */
                 /*************************************************/
                 fTagOpen = FALSE;     // data tag has been closed
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case EDICTNAME_TAG:
               if ( ( ( CurrentState == LISTHEADER_STATE ) ||
                      ( CurrentState == TERMDATA_STATE ) ) && fTagOpen &&
                    ( pTok->sTokenid == sEndTag ) )
               {
                 /*************************************************/
                 /* End tag for dictionary names encountered      */
                 /*************************************************/
                 fTagOpen = FALSE;     // data tag has been closed
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case ETERM_TAG:
               if ( ( CurrentState == TERMDATA_STATE ) && !fTagOpen )
               {
                 /*****************************************************/
                 /* Add data of current term to term table            */
                 /*****************************************************/
                 usRC = LstAddTerm( ppTermTable, &TermData );

                 /*************************************************/
                 /* term end encountered, switch to in-between    */
                 /* mode                                          */
                 /*************************************************/
                 CurrentState = INBETWEEN_STATE;
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case CONREF_TAG:
             case CONTEXT_TAG:
             case FREQUENCY_TAG:
             case LEMMA_TAG:
             case TRANSLATION_TAG:
             case MARK_TAG:
             case DEST_TAG:
               if ( fExternal &&
                    ( (pTok->sTokenid == CONREF_TAG) ||
                      (pTok->sTokenid == FREQUENCY_TAG) ||
                      (pTok->sTokenid == MARK_TAG) ||
                      (pTok->sTokenid == DEST_TAG) ) )
               {
                 usRC = ERROR_BAD_FORMAT;        // tags not allowed in external
                                                 // format
               }
               else if ( ( CurrentState == TERMDATA_STATE ) && !fTagOpen )
               {
                 /*****************************************************/
                 /* Start tag for term data encountered               */
                 /*****************************************************/
                 fTagOpen = TRUE;      // data tag is open
                 sEndTag = (SHORT)LstEndTag[pTok->sTokenid];
                 if ( pTok->sTokenid == CONTEXT_TAG )
                 {
                   /***************************************************/
                   /* invalidate context id                           */
                   /***************************************************/
                   usContextID = NO_CONTEXT_ID;

                   /***************************************************/
                   /* For external lists only: start context mode     */
                   /* (in internal lists we have to process the       */
                   /* ID= attribute first)                            */
                   /***************************************************/
                   if ( fExternal )
                   {
                     fContextMode = TRUE;        // start context mode
                     pContextBuf[0] = EOS;       // clear context buffer
                   } /* endif */
                 } /* endif */
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case ECONREF_TAG:
             case ECONTEXT_TAG:
             case EFREQUENCY_TAG:
             case ELEMMA_TAG:
             case ETRANSLATION_TAG:
             case EMARK_TAG:
             case EDEST_TAG:
               if ( ( CurrentState == TERMDATA_STATE ) && fTagOpen &&
                    ( pTok->sTokenid == sEndTag ) )
               {
                 fTagOpen = FALSE;     // data tag has been closed
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case TERM_TAG:
               if ( CurrentState == INBETWEEN_STATE )
               {
                 /*****************************************************/
                 /* Reset term data                                   */
                 /*****************************************************/
                 memset( &TermData, 0, sizeof(TERM) );

                 /*****************************************************/
                 /* Term start encountered, switch to term data mode  */
                 /*****************************************************/
                 CurrentState = TERMDATA_STATE;
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case ENTLIST_TAG:
             case EFTLIST_TAG:
               if ( ( CurrentState == INBETWEEN_STATE ) &&
                    ( sEndListTag   == pTok->sTokenid ) )
               {
                 /*****************************************************/
                 /* End of list encountered, switch to end list mode  */
                 /*****************************************************/
                 CurrentState = ENDOFLIST_STATE;
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // wrong position for tag
               } /* endif */
               break;

             case ID_ATTR:
               pszSource = UTF16strchr( pTok->pDataStringW, ATTR_VALUE_DELIMITER );
               if ( fExternal )
               {
                 usRC = ERROR_BAD_FORMAT;        // tag not allowed in external
                                                 // format
               }
               else if ( pszSource )
               {
                 usContextID = (USHORT)(_wtoi( pszSource + 1 ));
                 /*****************************************************/
                 /* Start context mode (for external list this has    */
                 /* been done during processing of the CONTEXT tag)   */
                 /*****************************************************/
                 fContextMode = TRUE;            // start context mode
                 pContextBuf[0] = EOS;           // clear context buffer
               }
               else
               {
                 usRC = ERROR_BAD_FORMAT;      // ID attribute corrupted
               } /* endif */
               break;

             case TEXT_TOKEN:
               /*******************************************************/
               /* If a data tag is open, store data as-is else ignore */
               /* data if it contains only whitespace characters or   */
               /* set error condition                                 */
               /*******************************************************/
               if ( fTagOpen )
               {
                 /*****************************************************/
                 /* Remember data length for statistical purposes     */
                 /*****************************************************/
                 ulDataLength += (LONG)pTok->usLength;

                 /*****************************************************/
                 /* Zero-terminate the data and add it to the string  */
                 /* pool                                              */
                 /*****************************************************/
                 chTemp = pTok->pDataStringW[pTok->usLength];
                 pTok->pDataStringW[pTok->usLength] = EOS;

                 /*****************************************************/
                 /* Process data for the currently active tag         */
                 /*****************************************************/
                 switch ( sEndTag )
                 {
                   case ETEXTNAME_TAG:
                     Unicode2ASCII (pTok->pDataStringW, szTemp, ulOemCP );
                     pszTemp = PoolAddString( *ppPool, szTemp );
                     if ( pszTemp )
                     {
                       usRC = LstAddPtrToList( &pListHeader->DocumentList,
                                               pszTemp);
                     }
                     else
                     {
                       usRC = ERROR_NOT_ENOUGH_MEMORY;
                     } /* endif */
                     break;
                   case EDICTNAME_TAG:
                     if ( CurrentState == LISTHEADER_STATE )
                     {
                       Unicode2ASCII( pTok->pDataStringW, szTemp, ulOemCP );
                       pszTemp = PoolAddString( *ppPool, szTemp);
                       if ( pszTemp )
                       {
                         usRC = LstAddPtrToList( &pListHeader->DictionaryList,
                                                 pszTemp);
                       }
                       else
                       {
                         usRC = ERROR_NOT_ENOUGH_MEMORY;
                       } /* endif */
                     }
                     else
                     {
                     } /* endif */
                     break;
                   case EOUTDICT_TAG:
                     Unicode2ASCII( pTok->pDataStringW, szTemp, ulOemCP );
                     pszTemp = PoolAddString( *ppPool, szTemp );
                     if ( pszTemp )
                     {
                       usRC = LstAddPtrToList( &pListHeader->OutDictionaryList,
                                               pszTemp );
                     }
                     else
                     {
                       usRC = ERROR_NOT_ENOUGH_MEMORY;
                     } /* endif */
                     break;
                   case EEXCLLISTNAME_TAG:
                     Unicode2ASCII( pTok->pDataStringW, szTemp, ulOemCP );
                     pszTemp = PoolAddString( *ppPool, szTemp );
                     if ( pszTemp )
                     {
                       usRC = LstAddPtrToList( &pListHeader->ExclListList,
                                               pszTemp );
                     }
                     else
                     {
                       usRC = ERROR_NOT_ENOUGH_MEMORY;
                     } /* endif */
                     break;
                   case EEXCLDICT_TAG:
                     Unicode2ASCII( pTok->pDataStringW, szTemp, ulOemCP );
                     pszTemp = PoolAddString( *ppPool, szTemp );
                     if ( pszTemp )
                     {
                       usRC = LstAddPtrToList( &pListHeader->ExclDictList,
                                               pszTemp );
                     }
                     else
                     {
                       usRC = ERROR_NOT_ENOUGH_MEMORY;
                     } /* endif */
                     break;
                   case ECREATEDATE_TAG:
                     pszSource = PoolAddStringW( *ppPool, pTok->pDataStringW );
                     if ( pszSource )
                     {
                       pListHeader->pszCreateDate = pszSource;
                     }
                     else
                     {
                       usRC = ERROR_NOT_ENOUGH_MEMORY;
                     } /* endif */
                     break;
                   case ECREATETIME_TAG:
                     pszSource = PoolAddStringW( *ppPool, pTok->pDataStringW );
                     if ( pszSource )
                     {
                       pListHeader->pszCreateTime = pszSource;
                     }
                     else
                     {
                       usRC = ERROR_NOT_ENOUGH_MEMORY;
                     } /* endif */
                     break;
                   case ECONREF_TAG:
                     usContextID = (USHORT)(_wtoi( pTok->pDataStringW ));
                     usRC = LstAddContextIDToList( &(TermData.pContextList),
                                                     usContextID );
                     break;
                   case ECONTEXT_TAG:
                     if ( fExternal )
                     {
                       /*********************************************/
                       /* As external files have no context ID, the */
                       /* context ID has to be generated here       */
                       /*********************************************/
                       usContextID = usGeneratedID++;
                     } /* endif */

                     if ( usContextID == NO_CONTEXT_ID )
                     {
                       usRC = ERROR_BAD_FORMAT; // ID attribute missing
                     }
                     else
                     {
                       pszSource = PoolAddStringW( *ppPool, pTok->pDataStringW );
                       if ( pszSource )
                       {
                         usRC = LstAddContext( ppContextTable, usContextID,
                                              pszSource );
                       }
                       else
                       {
                         usRC = ERROR_NOT_ENOUGH_MEMORY;
                       } /* endif */
                       if ( !usRC )
                       {
                         usRC = LstAddContextIDToList( &(TermData.pContextList),
                                                       usContextID );
                       } /* endif */
                     } /* endif */
                     break;
                   case EFREQUENCY_TAG:
                     TermData.ulFrequency = _wtol(pTok->pDataStringW);
                     break;
                   case ELEMMA_TAG:
                     TermData.pszName = PoolAddStringW( *ppPool,
                                                      pTok->pDataStringW );
                     if ( !TermData.pszName )
                     {
                       usRC = ERROR_NOT_ENOUGH_MEMORY;
                     } /* endif */
                     break;
                   case ETRANSLATION_TAG:
                     TermData.pszTranslation = PoolAddStringW( *ppPool,
                                                            pTok->pDataStringW );
                     if ( !TermData.pszTranslation )
                     {
                       usRC = ERROR_NOT_ENOUGH_MEMORY;
                     } /* endif */
                     break;
                   case EMARK_TAG:
                     pszSource = pTok->pDataStringW;
                     while ( *pszSource )
                     {
                       switch ( *pszSource )
                       {
                         case DELETE_MARK :
                           TermData.Flags.fMark = DELMARK_FLAG;
                           break;
                         case EXCL_MARK   :
                           TermData.Flags.fMark = EXCLMARK_FLAG;
                           break;
                         case DICT_MARK   :
                           TermData.Flags.fMark = DICTMARK_FLAG;
                           break;
                         case SELECT_MARK :
                           TermData.Flags.fSelected = TRUE;
                           break;
                         case TOP_MARK    :
                           TermData.Flags.fTop = TRUE;
                           break;
                         case CONTEXT_MARK    :
                           TermData.Flags.fContext = TRUE;
                           break;
                         default :
                           /*******************************************/
                           /* Ignore unknown mark flags               */
                           /*******************************************/
                           break;
                       } /* endswitch */
                       pszSource++;
                     } /* endwhile */
                     break;
                   case EDEST_TAG:
                     {
                       CHAR       szTempDest[MAX_FNAME]; // exclusion list or dictionary name

                       Unicode2ASCII( pTok->pDataStringW, szTempDest, ulOemCP );

                       TermData.pszDestination = PoolAddString( *ppPool, szTempDest );

                       if ( !TermData.pszDestination )
                       {
                         usRC = ERROR_NOT_ENOUGH_MEMORY;
                       } /* endif */
                     }
                     break;
                   default :
                     break;
                 } /* endswitch */

                 /*****************************************************/
                 /* Restore previously saved string end character     */
                 /*****************************************************/
                 pTok->pDataStringW[pTok->usLength] = chTemp;
               }
               else
               {
                 pErrorW = pTok->pDataStringW;
                 for ( usI = 0; usI < pTok->usLength; usI++ )
                 {
                   if ( (pTok->pDataStringW[usI] != BLANK) &&
                        (pTok->pDataStringW[usI] != CR)    &&
                        (pTok->pDataStringW[usI] != LF) )
                   {
                     usRC = ERROR_BAD_FORMAT;    // unknown data in file
                     break;                      // leave for loop
                   } /* endif */
                 } /* endfor */
               } /* endif */
               break;

             default :
               usRC = ERROR_BAD_FORMAT;      // invalid/unknown tag
               break;
           } /* endswitch */

           pTok++;
         } /* endwhile */

         /**********************************************************/
         /* is there still data in input buffer and something read */
         /* in the last time ??                                    */
         /* if not we are done - may be a problem with EOF symbol  */
         /**********************************************************/
         if ( pRest && ulBytesToRead )
         {
            // calculate number of not-processed bytes
            ulBytesInBuffer -= ((PBYTE)pRest - (PBYTE)pInBuf);

            // shift not-processed bytes to start of buffer
            if ( ulBytesInBuffer )
            {
               memmove( pInBuf, pRest, ulBytesInBuffer );
            } /* endif */
         }
         else
         {
            ulBytesInBuffer = 0;       // all data in buffer has been processed
         } /* endif */
      } /* endif */


      /****************************************************************/
      /* Report any error condition                                   */
      /****************************************************************/
      if ( usRC )
      {
        switch ( usRC )
        {
          case  ERROR_NOT_ENOUGH_MEMORY:
            UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
            break;
          case ERROR_BAD_FORMAT:
          default :
            pError = UtlGetFnameFromPath( pszListName );
            UtlError( NO_VALID_FORMAT, MB_CANCEL, 1, &pError, EQF_ERROR );
            break;
        } /* endswitch */
      } /* endif */
    } /* endwhile */


   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( hList )           UtlClose( hList, FALSE );
   if ( pTagTable )       TAFreeTagTable( pTagTable );
   if ( pInBuf )          UtlAlloc( (PVOID *) &pInBuf, 0L, 0L, NOMSG );
   if ( pConvBuf )        UtlAlloc( (PVOID *) &pInBuf, 0L, 0L, NOMSG );
   if ( pTokBuf )         UtlAlloc( (PVOID *) &pTokBuf, 0L, 0L, NOMSG );
   if ( pContextBuf )     UtlAlloc( (PVOID *) &pContextBuf, 0L, 0L, NOMSG );

   return( usRC );

} /* end of function LstReadSGMLList */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstAddContext                                            |
//+----------------------------------------------------------------------------+
//|Function call:     LstAddContext( PCONTEXTTABLE *pppContextTable,           |
//|                                  USHORT usID, PSZ pszContext );            |
//+----------------------------------------------------------------------------+
//|Description:       Adds a new context to the context table under the given  |
//|                   ID. If the context table does not exist, it is created.  |
//|                   Only the pointer to the context is stored, not the       |
//|                   context string itself.                                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCONTEXTTABLE *ppContextTable   pointer to context       |
//|                                                   table pointer            |
//|                   USHORT        usID              ID (number) of the       |
//|                                                   context to add           |
//|                   PSZ           pszContext        pointer to context text  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      = Context has been added                          |
//|                   ERROR_NOT_ENOUGH_MEMORY  = Could not add context due to  |
//|                                   memory shortage                          |
//+----------------------------------------------------------------------------+
//|Function flow:     if context pointer is not set then                       |
//|                     create initial context table                           |
//|                   endif                                                    |
//|                   while no error and context ID not in current table       |
//|                     skip to next table, create it if required              |
//|                   endwhile                                                 |
//|                   if no error then                                         |
//|                     add ptr to current context table                       |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
USHORT LstAddContext
(
  PCONTEXTTABLE *ppContextTable,       // pointer to context table pointer
  USHORT        usID,                  // ID (number) of the context to add
  PSZ_W         pszContext             // pointer to context text
)
{
  USHORT        usRC = 0;              // function return code
  PCONTEXTTABLE pTable = NULL;         // pointer to current context table
  PCONTEXTTABLE pNewTable;             // pointer to new context table

  /********************************************************************/
  /* If no context table exists, create the initial one               */
  /********************************************************************/
  if ( *ppContextTable == NULL )
  {
    UtlAlloc( (PVOID *) &pNewTable, 0L, (LONG)CONTEXT_TABLE_SIZE, NOMSG );
    if ( pNewTable )
    {
      *ppContextTable = pNewTable;
      pNewTable->usSize  = CONTEXT_TABLE_SIZE;
      pNewTable->usFirst = 0;
      pNewTable->usLast  = ((CONTEXT_TABLE_SIZE - sizeof(CONTEXTTABLE)) /
                           sizeof(PSZ_W)) - 1;
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* while no error and context ID is not in current table skip       */
  /* to next table and create if required                             */
  /********************************************************************/
  if ( !usRC  )
  {
    pTable = *ppContextTable;
    while ( !usRC && (usID > pTable->usLast) )
    {
      if ( !pTable->pNextTable )
      {
        /**************************************************************/
        /* create the next table                                      */
        /**************************************************************/
        UtlAlloc( (PVOID *) &pNewTable, 0L, (LONG)CONTEXT_TABLE_SIZE, NOMSG );
        if ( pNewTable )
        {
          pTable->pNextTable = pNewTable;
          pNewTable->usSize  = CONTEXT_TABLE_SIZE;
          pNewTable->usFirst = pTable->usLast + 1;
          pNewTable->usLast  = pNewTable->usFirst +
                               ((CONTEXT_TABLE_SIZE - sizeof(CONTEXTTABLE)) /
                               sizeof(PSZ_W)) - 1;
        }
        else
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
      }
      else
      {
        pTable = pTable->pNextTable;
      } /* endif */
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* If no error occured, add the context to the current context table*/
  /********************************************************************/
  if ( !usRC  )
  {
    if ( (usID >= pTable->usFirst) && (usID <= pTable->usLast) )
    {
      ((PSZ_W *)(pTable+1))[usID - pTable->usFirst] = pszContext;
    }
    else
    {
      /****************************************************************/
      /* This code should never become active ...                     */
      /****************************************************************/
      usRC = 999;
      UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return return code to calling function                           */
  /********************************************************************/
  return( usRC );

} /* end of function LstAddContext */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstGetContext                                            |
//+----------------------------------------------------------------------------+
//|Function call:     LstGetContext( PCONTEXTTABLE pContextTable, USHORT usID )|
//+----------------------------------------------------------------------------+
//|Description:       Get the pointer to the context with the given ID.        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCONTEXTTABLE pContextTable  pointer to first context    |
//|                                                table                       |
//|                   USHORT        usID           ID (number) of the requested|
//|                                                context                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL    if no context withthe given ID is in the table(s)|
//|                   other   pointer to context string                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pContextTable must have been created and filled using    |
//|                   LstAddContext                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     while context ID not in current table                    |
//|                     skip to next table                                     |
//|                   endwhile                                                 |
//|                   return pointer of context or NULL if not found           |
//+----------------------------------------------------------------------------+
PSZ_W LstGetContext
(
  PCONTEXTTABLE pContextTable,         // pointer to first context table
  USHORT        usID                   // ID (number) of the requested context
)
{
  PCONTEXTTABLE pTable;                // pointer to current context table
  PSZ_W         pszContext = NULL;     // pointer to context text

  /********************************************************************/
  /* While context ID not in current table skip to next one           */
  /********************************************************************/
  pTable = pContextTable;
  while ( pTable && (usID > pTable->usLast) )
  {
    pTable = pTable->pNextTable;
  } /* endwhile */


  /********************************************************************/
  /* Return pointer to context or NULL if none found                  */
  /********************************************************************/
  if ( pTable )
  {
    if ( (usID >= pTable->usFirst) && (usID <= pTable->usLast) )
    {
      pszContext = ((PSZ_W *)(pTable+1))[usID - pTable->usFirst];
    }
    else
    {
      /****************************************************************/
      /* This code should never become active ...                     */
      /****************************************************************/
      pszContext = NULL;
      UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
    } /* endif */

  }
  else
  {
    pszContext = NULL;                 // no context with the given ID
  } /* endif */
  return( pszContext );

} /* end of function LstGetContext */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstSetContext                                            |
//+----------------------------------------------------------------------------+
//|Function call:     LstSetContext( PCONTEXTTABLE pContextTable, USHORT usID, |
//|                                  PSZ pszNewContext );                      |
//+----------------------------------------------------------------------------+
//|Description:       Replace the pointer of the context with the given ID     |
//|                   with the given value                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCONTEXTTABLE pContextTable  pointer to first context    |
//|                                                table                       |
//|                   USHORT        usID           ID (number) of the requested|
//|                                                context                     |
//|                   PSZ           pszNewContext  pointer to new context      |
//|                                                string or NULL              |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE   if no context with the given ID is in the table  |
//|                   TRUE    pointer of context has been replaced             |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pContextTable must have been created and filled using    |
//|                   LstAddContext                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     while context ID not in current table                    |
//|                     skip to next table                                     |
//|                   endwhile                                                 |
//|                   if found replace pointer of context with the given one   |
//|                   return TRUE if successful or FALSE in case of errors     |
//+----------------------------------------------------------------------------+
BOOL LstSetContext
(
  PCONTEXTTABLE pContextTable,         // pointer to first context table
  USHORT        usID,                  // ID (number) of the requested context
  PSZ_W         pszNewContext          // pointer to new context text or NULL
)
{
  PCONTEXTTABLE pTable;                // pointer to current context table
  BOOL          fOK = TRUE;            // OK flag, is returned to caller

  /********************************************************************/
  /* While context ID not in current table skip to next one           */
  /********************************************************************/
  pTable = pContextTable;
  while ( pTable && (usID > pTable->usLast) )
  {
    pTable = pTable->pNextTable;
  } /* endwhile */


  /********************************************************************/
  /* Replace pointer of context with the given one                    */
  /********************************************************************/
  if ( pTable )
  {
    if ( (usID >= pTable->usFirst) && (usID <= pTable->usLast) )
    {
      ((PSZ_W *)(pTable+1))[usID - pTable->usFirst] = pszNewContext;
    }
    else
    {
      /****************************************************************/
      /* This code should never become active ...                     */
      /****************************************************************/
      fOK = FALSE;
      UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
    } /* endif */
  }
  else
  {
    fOK = FALSE;                       // no context with the given ID
  } /* endif */

  return( fOK );

} /* end of function LstSetContext */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstAddContextIDToList                                    |
//+----------------------------------------------------------------------------+
//|Function call:     LstAddContextIDToList( PCONTEXTLIST *ppContextList,      |
//|                                          USHORT usID );                    |
//+----------------------------------------------------------------------------+
//|Description:       Adds the given context ID to the context list of a term. |
//|                   If the context list does not exist, it is created.       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCONTEXTLIST  *ppContextList    address of context       |
//|                                                   list pointer             |
//|                   USHORT        usID              ID (number) of the       |
//|                                                   context to add           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      = Context ID has been successfully added          |
//|                   ERROR_NOT_ENOUGH_MEMORY  = Could not add context ID      |
//|                            due to memory shortage                          |
//+----------------------------------------------------------------------------+
//|Function flow:     if context list pointer is not set then                  |
//|                     create initial context list                            |
//|                   endif                                                    |
//|                   if no error and current context list is full then        |
//|                     enlarge context list                                   |
//|                   endif                                                    |
//|                   if no error then                                         |
//|                     add context ID to context list                         |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
USHORT LstAddContextIDToList
(
  PCONTEXTLIST  *ppContextList,        // address of context list pointer
  USHORT        usID                   // ID (number) of the context to add
)
{
  USHORT        usRC = 0;              // function return code
  PCONTEXTLIST  pContextList;          // pointer to context list
  USHORT        usSize;                // overall context list size

  /********************************************************************/
  /* If no context list exists, create one                            */
  /********************************************************************/
  if ( *ppContextList == NULL )
  {
    usSize = sizeof(CONTEXTLIST) + (CONTEXT_LIST_SIZE * sizeof(USHORT));

    UtlAlloc( (PVOID *) &pContextList, 0L, (LONG)usSize, NOMSG );
    if ( pContextList )
    {
      *ppContextList  = pContextList;
      pContextList->usSize  = CONTEXT_LIST_SIZE;
      pContextList->usUsed  = 0;
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* If context list is full, enlarge context list                    */
  /********************************************************************/
  if ( !usRC )
  {
    pContextList = *ppContextList;
    if ( pContextList->usUsed >= pContextList->usSize )
    {
      usSize = sizeof(CONTEXTLIST) + (pContextList->usSize * sizeof(USHORT));
      UtlAlloc( (PVOID *) &pContextList, (LONG)usSize,
                (LONG)(usSize + (CONTEXT_LIST_SIZE * sizeof(USHORT))),
                NOMSG );
      if ( pContextList )
      {
        *ppContextList  = pContextList;
        pContextList->usSize  += CONTEXT_LIST_SIZE;
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* If no error occured, add context ID to context list              */
  /********************************************************************/
  if ( !usRC  )
  {
    pContextList = *ppContextList;
    pContextList->ausContextID[pContextList->usUsed++] = usID;
  } /* endif */

  /********************************************************************/
  /* Return return code to calling function                           */
  /********************************************************************/
  return( usRC );

} /* end of function LstAddContextIDToList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstDestroyContextTable                                   |
//+----------------------------------------------------------------------------+
//|Function call:     LstDestroyContextTable( PCONTEXTTABLE pContextTable );   |
//+----------------------------------------------------------------------------+
//|Description:       Free all memory allocated for the context table.         |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCONTEXTTABLE pContextTable  pointer to first context    |
//|                                                table                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pContextTable must have been created and filled using    |
//|                   LstAddContext                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     while there are context tables                           |
//|                     free memory of table                                   |
//|                     skip to next table                                     |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
VOID LstDestroyContextTable
(
  PCONTEXTTABLE pContextTable          // pointer to first context table
)
{
  PCONTEXTTABLE pTable;                // pointer to current context table

  /********************************************************************/
  /* While there are tables, free memory of table and skip to next    */
  /* table                                                            */
  /********************************************************************/
  while ( pContextTable )
  {
    pTable = pContextTable->pNextTable;          // remember next table
    UtlAlloc( (PVOID *) &pContextTable, 0L, 0L, NOMSG );   // free current one
    pContextTable = pTable;                      // make next table the current
  } /* endwhile */

} /* end of function LstDestroyContextTable */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstLastContextID                                         |
//+----------------------------------------------------------------------------+
//|Function call:     LstLastContextID( PCONTEXTTABLE pContextTable );         |
//+----------------------------------------------------------------------------+
//|Description:       Get the last used context ID in the supplied context     |
//|                   table.                                                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCONTEXTTABLE pContextTable  pointer to first context    |
//|                                                table                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       all     ID of last context in context table              |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pContextTable must have been created and filled using    |
//|                   LstAddContext                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     while not in last context table                          |
//|                     skip to next table                                     |
//|                   endwhile                                                 |
//|                   find the last context in table                           |
//|                   return ID of context found                               |
//+----------------------------------------------------------------------------+
USHORT LstLastContextID
(
  PCONTEXTTABLE pContextTable          // pointer to first context table
)
{
  PCONTEXTTABLE pTable = NULL;         // pointer to current context table
  PSZ_W         *ppContext;            // pointer into context pointer array
  USHORT        usContextID = 0;       // ID of last context in table

  /********************************************************************/
  /* Go to last context table                                         */
  /********************************************************************/
  if ( pContextTable )
  {
    pTable = pContextTable;
    while ( pTable->pNextTable )
    {
      pTable = pTable->pNextTable;
    } /* endwhile */
  } /* endif */


  /********************************************************************/
  /* Find last context in table                                       */
  /********************************************************************/
  if ( pTable )
  {
    usContextID = pTable->usLast;
    ppContext = (PSZ_W *)(pTable+1) + (pTable->usLast - pTable->usFirst);
    while ( (usContextID > pTable->usFirst) && !*ppContext )
    {
      ppContext--;
      usContextID--;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Return found context ID                                          */
  /********************************************************************/
  return( usContextID );

} /* end of function LstLastContextID */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstAddTerm                                               |
//+----------------------------------------------------------------------------+
//|Function call:     LstAddTerm( PTERMTABLE *ppTermTable, PTERM pTerm );      |
//+----------------------------------------------------------------------------+
//|Description:       Adds a new Term structure to the end of the term table   |
//|                   If the Term table does not exist, it is created.         |
//+----------------------------------------------------------------------------+
//|Input parameter:   PTERMTABLE *ppTermTable   pointer to term table pointer  |
//|                   PTERM      pTerm          pointer to term data           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      = Term has been added                             |
//|                   ERROR_NOT_ENOUGH_MEMORY  = Could not add term due to     |
//|                                   memory shortage                          |
//+----------------------------------------------------------------------------+
//|Function flow:     if Term pointer is not set then                          |
//|                     create initial Term table                              |
//|                   endif                                                    |
//|                   position to end of term table and add a new term table   |
//|                     if last term table is full                             |
//|                   if no error then                                         |
//|                     add term data at end of term table                     |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
USHORT LstAddTerm
(
  PTERMTABLE    *ppTermTable,          // pointer to term table pointer
  PTERM         pTerm                  // pointer to term data structure
)
{
  USHORT        usRC = 0;              // return code of function
  PTERMTABLE    pTable = NULL;         // pointer to current term table
  PTERMTABLE    pNewTable;             // pointer to new term table
  PTERM         pTermData;             // position for term data in term table

  /********************************************************************/
  /* If no term table exists, create the initial one                  */
  /********************************************************************/
  if ( *ppTermTable == NULL )
  {
    UtlAlloc( (PVOID *) &pNewTable, 0L, (LONG)TERM_TABLE_SIZE, NOMSG );
    if ( pNewTable )
    {
      *ppTermTable = pNewTable;
      pNewTable->usSize  = TERM_TABLE_SIZE;
      pNewTable->usUsedEntries = 0;
      pNewTable->usMaxEntries  = ((TERM_TABLE_SIZE - sizeof(TERMTABLE)) /
                                 sizeof(TERM));
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* position to last term table                                      */
  /********************************************************************/
  if ( !usRC )
  {
    pTable = *ppTermTable;
    while ( pTable->pNextTable )
    {
      pTable = pTable->pNextTable;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Add new table if last table is full                              */
  /********************************************************************/
  if ( !usRC  )
  {
    if ( pTable->usUsedEntries >= pTable->usMaxEntries )
    {
      UtlAlloc( (PVOID *) &pNewTable, 0L, (LONG)TERM_TABLE_SIZE, NOMSG );
      if ( pNewTable )
      {
        pNewTable->usSize        = TERM_TABLE_SIZE;
        pNewTable->usUsedEntries = 0;
        pNewTable->usMaxEntries  = ((TERM_TABLE_SIZE - sizeof(TERMTABLE)) /
                                   sizeof(TERM));
        pTable->pNextTable = pNewTable;
        pTable = pNewTable;
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* If ok add the term data to the end of the current term table     */
  /********************************************************************/
  if ( !usRC  )
  {
    pTermData = (PTERM)(pTable + 1);
    pTermData = pTermData + pTable->usUsedEntries;
    memcpy( pTermData, pTerm, sizeof(TERM) );
    pTable->usUsedEntries++;
  } /* endif */

  /********************************************************************/
  /* Return return code to calling functionn                          */
  /********************************************************************/
  return( usRC );

} /* end of function LstAddTerm */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstDestroyTermTable                                      |
//+----------------------------------------------------------------------------+
//|Function call:     LstDestroyTermTable( PTERMTABLE pTermTable );            |
//+----------------------------------------------------------------------------+
//|Description:       Free all memory allocated for the term tables.           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PTERMTABLE pTermTable  pointer to first term table       |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pTermTable must have been created and filled using       |
//|                   LstAddTerm                                               |
//+----------------------------------------------------------------------------+
//|Function flow:     while there are term tables                              |
//|                     free memory of table                                   |
//|                     skip to next table                                     |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
VOID LstDestroyTermTable
(
  PTERMTABLE pTermTable          // pointer to first Term table
)
{
  PTERMTABLE pTable;                   // pointer to current Term table
  PTERM      pTerm;                    // pointer to current term
  USHORT     usI;                      // loop index

  /********************************************************************/
  /* While there are tables, free memory of table and skip to next    */
  /* table                                                            */
  /********************************************************************/
  while ( pTermTable )
  {
    pTable = pTermTable->pNextTable;             // remember next table

    pTerm = (PTERM)(pTermTable+1);
    for ( usI = 0; usI < pTermTable->usUsedEntries; usI++, pTerm++ )
    {
      if ( pTerm->pContextList )
      {
        UtlAlloc( (PVOID *) &pTerm->pContextList, 0L, 0L, NOMSG );
      } /* endif */
    } /* endfor */
    UtlAlloc( (PVOID *) &pTermTable, 0L, 0L, NOMSG );   // free current one
    pTermTable = pTable;                      // make next table the current
  } /* endwhile */

} /* end of function LstDestroyTermTable */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstDeleteList                                            |
//+----------------------------------------------------------------------------+
//|Function call:     LstDeleteList( PSZ pszList );                            |
//+----------------------------------------------------------------------------+
//|Description:       Checks if the list is locked, ask for user confirmation  |
//|                   and deletes the list.                                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ   pszList     fully qualified list name              |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   =  list has been deleted                          |
//|                   FALSE  =  list not deleted                               |
//+----------------------------------------------------------------------------+
//|Samples:           LstDeleteList( "D:\EQF\LIST\OLD.NTL" );                  |
//+----------------------------------------------------------------------------+
//|Function flow:     if a symbol for the list exists then                     |
//|                     display 'list in use' message                          |
//|                   endif                                                    |
//|                   if ok then                                               |
//|                     if a handler responds with TRUE to about-to-delete msg |
//|                       display 'list in use' message                        |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   if ok get user confirmation for delete                   |
//|                   if ok delete list                                        |
//|                   return OK flag to calling function                       |
//+----------------------------------------------------------------------------+
BOOL LstDeleteList
(
  PSZ   pszList                        // fully qualified list name
)
{
  USHORT        usRC;                  // message box and UtlDelete return code
  BOOL          fOK = TRUE;            // internal OK flag
  CHAR          szListName[MAX_FNAME]; // buffer for list name
  PSZ           pszParm;               // ptr to error parameter

  /****************************************************************/
  /* Isolate list name                                            */
  /****************************************************************/
  Utlstrccpy( szListName, UtlGetFnameFromPath( pszList ), DOT );
  pszParm = szListName;

  /****************************************************************/
  /* Check if a symbol for this list exists, indicating that the  */
  /* list is currently in use                                     */
  /****************************************************************/
  if ( QUERYSYMBOL( pszList ) != -1 )
  {
    UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszParm, EQF_INFO );
    fOK = FALSE;
  } /* endif */

  /****************************************************************/
  /* Ask all handlers if list is currently in use                 */
  /****************************************************************/
  if ( fOK )
  {
    if( EqfSend2AllHandlers( WM_EQF_ABOUTTODELETE,
                             MP1FROMSHORT( clsLIST ),
                             MP2FROMP(pszList) ))
    {
      UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszParm, EQF_INFO );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /****************************************************************/
  /* Ask for user confirmation                                    */
  /****************************************************************/
  if ( fOK  )
  {
    usRC = UtlError( WARNING_LST_DELETE_LIST,
                     MB_YESNO | MB_QUERY | MB_DEFBUTTON2,
                     1, &pszParm, EQF_QUERY );
    if ( usRC != MBID_YES )
    {
      fOK = FALSE;                 // user does not want to delete list
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Delete the list                                                  */
  /********************************************************************/
  if ( fOK )
  {
    usRC = UtlDelete( pszList, 0L, TRUE );
    if ( usRC )
    {
      UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszParm, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Broadcast delete confirmation                                    */
  /********************************************************************/
  if ( fOK )
  {
    EqfSend2AllHandlers( WM_EQFN_DELETED,
                         MP1FROMSHORT( clsLIST ),
                         MP2FROMP(pszList) );
  } /* endif */

  /********************************************************************/
  /* Return OK flag to calling function                               */
  /********************************************************************/
  return( fOK );

} /* end of function LstDeleteList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstWriteSGMLList       Write a SGML list                 |
//+----------------------------------------------------------------------------+
//|Function call:     LstWriteSGMLList( PSZ pszListName,                       |
//|                                     PLISTHEADER pListHeader,               |
//|                                     PTERMTABLE pTermTable,                 |
//|                                     PCONTEXTTABLE pContextTable,           |
//|                                     BOOL fExternal,                        |
//|                                     LISTTYPES Type );                      |
//+----------------------------------------------------------------------------+
//|Description:       Write a list in the new-terms-list / found-terms-list    |
//|                   SGML format (internal or external)                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ           pszListName   ptr to fully qualified list  |
//|                                               name                         |
//|                   PLISTHEADER   pListHeader   ptr to list header structure |
//|                   PTERMTABLE    pTermTable    ptr to list term table       |
//|                   PCONTEXTTABLE pContextTable ptr to list context table    |
//|                   BOOL          fExternal     TRUE = write list in external|
//|                                               format                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       function completed successfully                  |
//|                   ERROR_NOT_ENOUGH_MEMORY  allocation of memory failed     |
//|                   other   return codes of Dosxxxx calls                    |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate buffers                                         |
//|                   create context reference atom table                      |
//|                   load list format table                                   |
//|                   open the output file                                     |
//|                   write list start tag                                     |
//|                   if not external format then                              |
//|                     write header tags and data                             |
//|                   endif                                                    |
//|                   while not end of term tables                             |
//|                     loop through all term in term table                    |
//|                       if terms is not deleted then                         |
//|                         write term tags                                    |
//|                       endif                                                |
//|                     endloop                                                |
//|                     skip to next term table                                |
//|                   endwhile                                                 |
//|                   write list end tag                                       |
//|                   close output file                                        |
//|                   delete old list file and rename output file to list file |
//|                   cleanup                                                  |
//+----------------------------------------------------------------------------+
USHORT LstWriteSGMLList
(
   PSZ           pszListName,          // ptr to fully qualified list name
   PLISTHEADER   pListHeader,          // ptr to list header structure
   PTERMTABLE    pTermTable,           // ptr to list term table
   PCONTEXTTABLE pContextTable,        // ptr to list context table
   BOOL          fExternal,            // TRUE = write list in external format
   LISTTYPES     Type,                 // type of list written
   USHORT        usFormat              // (character) format of data (ANSI,ASCII,...)

)
{
   CHAR        szTempName[CCHMAXPATH]; // name of temporary file
   CHAR_W      chName[MAX_LONGFILESPEC];// temp buffer for filenames
   PBUFCB      pBufCB = NULL;          // ptr to CB for buffered IO
   USHORT      usI;                    // general loop index
   BOOL        fExist = FALSE;         // file exists flag
   USHORT      usRC = 0;               // return code of function
   PSZ_W       pszSource;              // source pointer for segment data
   PSZ_W       pszStart;               // pointer to start of data
   PSZ_W       pszTag1, pszTag2;       // pointer to tag names
   PTAG        pTag = NULL;            // pointer to structure of active tag
   //PSZ         pTagNames;              // pointer to start of tagnames
   PBYTE       pByte;                  // helper pointer
   PLOADEDTABLE pLoadedTable = NULL;   // pointer to active QF tag table
   PTAGTABLE   pTagTable = NULL;       // pointer to active QF tag table
   CHAR_W      szIDAttr[40];           // buffer for preprocessed ID= attr
   PATTRIBUTE  pAttr;                  // ptr to start of attributes in tagtable
   PSZ_W       pBuf;                   // ptr to general string buffer
   PTERM       pTerm;                  // ptr to term entries
   CHAR_W      szMarkFlags[10];        // buffer for mark flags
   PSZ_W       pszTemp;                // helper pointer

   // Variables for context processing
   HATOMTBL    hatomConRef = NULLHANDLE;// atom table for context numbers written
                                       // to disk already
   USHORT      usContext;              // index for context lists
   USHORT      usContextID;            // ID for user supplied contexts
   CHAR_W      szIDAttrValue[40];      // value for ID= attr of context
   CHAR_W      szContextID[10];        // buffer for string representation of
                                       // context ID
   PSZ_W       pszContext;             // pointer to context string
   PSZ_W       pTagNamesW = NULL;
   ULONG       ulOemCP = 0L;
   ULONG       ulFormatCP = 0L;        // acc. to format ASCII or ANSICP
   ULONG       ulAnsiCP = 0L;

  /********************************************************************/
  /* Allocate buffers                                                 */
  /********************************************************************/
  UtlAlloc( (PVOID *) &pBuf, 0L, (LONG)IO_BUFFER_SIZE, ERROR_STORAGE );
  if ( !pBuf )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  /********************************************************************/
  /* Create context reference atom table                              */
  /********************************************************************/
  if ( !usRC )
  {
    hatomConRef = WinCreateAtomTable( 0, 0 );
  } /* endif */

  /********************************************************************/
  /* Load list format table                                           */
  /********************************************************************/
  if ( !usRC )
  {
    ulOemCP = GetLangOEMCP(NULL);  // get system pref. lang
    if (usFormat == SGMLFORMAT_ASCII)
    {
      ulFormatCP = ulOemCP;
    }
    if (usFormat == SGMLFORMAT_ANSI)
    {
      ulFormatCP = ulOemCP;
      ulAnsiCP = GetLangAnsiCP(NULL);
    }

    usRC = TALoadTagTable( LISTFORMATTABLE, &pLoadedTable, TRUE, TRUE );
    if ( usRC == NO_ERROR )
    {
      pTagTable = pLoadedTable->pTagTable;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Address tag names in tag table and preprocess attribute strings  */
  /********************************************************************/
  if ( !usRC )
  {
    pByte     = (PBYTE) pTagTable;
    pTag      = (PTAG) (pByte + pTagTable->stFixTag.uOffset);
    //pTagNames = (PSZ) (pByte +  pTagTable-> uTagNames);
    pTagNamesW = pLoadedTable->pTagNamesW;

    pAttr = (PATTRIBUTE) (pByte + pTagTable->stAttribute.uOffset);

    pszSource = pTagNamesW + pAttr[ID_ATTR - pTagTable->uNumTags].uStringOffs;
    pszStart  = szIDAttr;
    while ( (*pszStart = *pszSource++) != NULC)
    {
      if ( *pszStart == CHAR_MULT_SUBST )
      {
        *pszStart++ = '%';
        *pszStart++ = 'd';
      }
      else
      {
        pszStart++;
      } /* endif */
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Create temporary file name                                       */
  /********************************************************************/
  if ( !usRC )
  {
    if ( fExternal )
    {
      strcpy( szTempName, pszListName );
    }
    else
    {
      usI = 1;
      do
      {
         sprintf( szTempName, "%c:\\LP%6.6d.$$$", *pszListName, usI );
         fExist = UtlFileExist( szTempName );
         usI++;
      } while ( usI && fExist ); /* enddo */
      usRC = ( fExist ) ? ERROR_FILE_NOT_FOUND : usRC;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Open the temporary list file                                     */
  /********************************************************************/
  if ( !usRC )
  {
     usRC = UtlBufOpen( &pBufCB, szTempName, IO_BUFFER_SIZE, FILE_CREATE, TRUE );
  } /* endif */

  /********************************************************************/
  /* Get last context ID in use to have a base for the IDs required   */
  /* for user supplied context strings                                */
  /********************************************************************/
  usContextID = LstLastContextID( pContextTable );
  //if usFormat is Unicode, write UNICODEFILEPREFIX first
  if (usFormat == SGMLFORMAT_UNICODE )
  {
     usRC = UtlBufWrite( pBufCB, UNICODEFILEPREFIX,
                               (SHORT)strlen(UNICODEFILEPREFIX), TRUE );
  }

  /********************************************************************/
  /* Write list start tag                                             */
  /********************************************************************/
  if ( !usRC )
  {
    if ( Type == NTL_TYPE )
    {
      pszTag1 = pTag[NTLIST_TAG].uTagnameOffs + pTagNamesW;
    }
    else
    {
      pszTag1 = pTag[FTLIST_TAG].uTagnameOffs + pTagNamesW;
    } /* endif */
    swprintf( pBuf, L"%s%c%c%c", pszTag1, END_TAG_CHAR, CR, LF );
    usRC = UtlBufWriteConv( pBufCB, pBuf,
                       (SHORT)UTF16strlenBYTE(pBuf), TRUE, usFormat,
                       ulFormatCP, ulAnsiCP );
  } /* endif */

  /********************************************************************/
  /* Write list header                                                */
  /********************************************************************/
  if ( !usRC && !fExternal )
  {

    /**************************************************************/
    /* Header start tag                                           */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[HEADER_TAG].uTagnameOffs + pTagNamesW;
      swprintf( pBuf, L" %s%c%c%c", pszTag1, END_TAG_CHAR, CR, LF );
      usRC = UtlBufWriteConv( pBufCB, pBuf,
                         (SHORT)UTF16strlenBYTE(pBuf), TRUE, usFormat,
                         ulFormatCP, ulAnsiCP );
    } /* endif */

    /**************************************************************/
    /* Document names                                             */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[TEXTNAME_TAG].uTagnameOffs + pTagNamesW;
      pszTag2 = pTag[ETEXTNAME_TAG].uTagnameOffs + pTagNamesW;

      usI = 0;
      while ( !usRC &&
              pListHeader->DocumentList &&
              pListHeader->DocumentList->Ptr[usI] )
      {
        ASCII2Unicode( (PSZ)pListHeader->DocumentList->Ptr[usI], chName, ulOemCP );
        swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                 END_TAG_CHAR,
                 chName,
                 pszTag2,
                 END_TAG_CHAR, CR, LF );
        usRC = UtlBufWriteConv( pBufCB, pBuf,
                           (SHORT)(UTF16strlenCHAR(pBuf) * sizeof(CHAR_W)), TRUE, usFormat,
                           ulFormatCP, ulAnsiCP );
        usI++;
      } /* endwhile */
    } /* endif */

    /**************************************************************/
    /* Dictionary names                                           */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[DICTNAME_TAG].uTagnameOffs + pTagNamesW;
      pszTag2 = pTag[EDICTNAME_TAG].uTagnameOffs + pTagNamesW;

      usI = 0;
      while ( !usRC &&
              pListHeader->DictionaryList &&
              pListHeader->DictionaryList->Ptr[usI] )
      {
        ASCII2Unicode((PSZ)pListHeader->DictionaryList->Ptr[usI], chName, ulOemCP);

        swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                 END_TAG_CHAR, chName,
                 pszTag2,
                 END_TAG_CHAR, CR, LF );
        usRC = UtlBufWriteConv( pBufCB, pBuf,
                           (SHORT)(UTF16strlenCHAR(pBuf) * sizeof(CHAR_W)),
                             TRUE, usFormat, ulFormatCP, ulAnsiCP );
        usI++;
      } /* endwhile */
    } /* endif */

    /**************************************************************/
    /* Output dictionaries                                        */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[OUTDICT_TAG].uTagnameOffs + pTagNamesW;
      pszTag2 = pTag[EOUTDICT_TAG].uTagnameOffs + pTagNamesW;

      usI = 0;
      while ( !usRC &&
              pListHeader->OutDictionaryList &&
              pListHeader->OutDictionaryList->Ptr[usI] )
      {
        ASCII2Unicode((PSZ)pListHeader->OutDictionaryList->Ptr[usI], chName, ulOemCP);

        swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                 END_TAG_CHAR,
                 chName,
                 pszTag2,
                 END_TAG_CHAR, CR, LF );
        usRC = UtlBufWriteConv( pBufCB, pBuf,
                           (SHORT)(UTF16strlenCHAR(pBuf) * sizeof(CHAR_W)),
                           TRUE, usFormat, ulFormatCP, ulAnsiCP );
        usI++;
      } /* endwhile */
    } /* endif */

    /**************************************************************/
    /* Exclusion lists                                            */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[EXCLLISTNAME_TAG].uTagnameOffs + pTagNamesW;
      pszTag2 = pTag[EEXCLLISTNAME_TAG].uTagnameOffs + pTagNamesW;

      usI = 0;
      while ( !usRC &&
              pListHeader->ExclListList &&
              pListHeader->ExclListList->Ptr[usI] )
      {
        ASCII2Unicode((PSZ)pListHeader->ExclListList->Ptr[usI], chName, ulOemCP);

        swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                 END_TAG_CHAR,
                 chName,
                 pszTag2,
                 END_TAG_CHAR, CR, LF );
       usRC = UtlBufWriteConv( pBufCB, pBuf,
                         (SHORT)(UTF16strlenCHAR(pBuf) * sizeof(CHAR_W)),
                              TRUE, usFormat, ulFormatCP, ulAnsiCP );
       usI++;
      } /* endwhile */
    } /* endif */

    /**************************************************************/
    /* Exclusion dictionaries                                     */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[EXCLDICT_TAG].uTagnameOffs + pTagNamesW;
      pszTag2 = pTag[EEXCLDICT_TAG].uTagnameOffs + pTagNamesW;

      usI = 0;
      while ( !usRC &&
              pListHeader->ExclDictList &&
              pListHeader->ExclDictList->Ptr[usI] )
      {
        ASCII2Unicode((PSZ)pListHeader->ExclDictList->Ptr[usI], chName, ulOemCP);

        swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                 END_TAG_CHAR,
                 chName,
                 pszTag2,
                 END_TAG_CHAR, CR, LF );
        usRC = UtlBufWriteConv( pBufCB, pBuf,
                          (SHORT)(UTF16strlenCHAR(pBuf) * sizeof(CHAR_W)),
                              TRUE, usFormat, ulFormatCP, ulAnsiCP );
        usI++;
      } /* endwhile */
    } /* endif */

    /**************************************************************/
    /* Create date                                                */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[CREATEDATE_TAG].uTagnameOffs + pTagNamesW;
      pszTag2 = pTag[ECREATEDATE_TAG].uTagnameOffs + pTagNamesW;
      swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                 END_TAG_CHAR, pListHeader->pszCreateDate, pszTag2,
                 END_TAG_CHAR, CR, LF );
      usRC = UtlBufWriteConv( pBufCB, pBuf,
                         (SHORT)UTF16strlenBYTE(pBuf) , TRUE, usFormat,
                         ulFormatCP, ulAnsiCP );
    } /* endif */

    /**************************************************************/
    /* Create time                                                */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[CREATETIME_TAG].uTagnameOffs + pTagNamesW;
      pszTag2 = pTag[ECREATETIME_TAG].uTagnameOffs + pTagNamesW;
      swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                 END_TAG_CHAR, pListHeader->pszCreateTime, pszTag2,
                 END_TAG_CHAR, CR, LF );
      usRC = UtlBufWriteConv( pBufCB, pBuf,
                        (SHORT)UTF16strlenBYTE(pBuf), TRUE, usFormat,
                        ulFormatCP, ulAnsiCP );
    } /* endif */

    /**************************************************************/
    /* Header end tag                                             */
    /**************************************************************/
    if ( !usRC )
    {
      pszTag1 = pTag[EHEADER_TAG].uTagnameOffs + pTagNamesW;
      swprintf( pBuf, L" %s%c%c%c", pszTag1, END_TAG_CHAR, CR, LF );
      usRC = UtlBufWriteConv( pBufCB, pBuf,
                          (SHORT)(UTF16strlenCHAR(pBuf) * sizeof(CHAR_W)),
                          TRUE, usFormat, ulFormatCP, ulAnsiCP );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Write term data                                                  */
  /********************************************************************/
  if ( !usRC )
  {
    while ( !usRC && pTermTable )
    {
      pTerm = (PTERM)(pTermTable+1);
      for ( usI = 0; usI < pTermTable->usUsedEntries; usI++, pTerm++ )
      {
        if ( !pTerm->Flags.fDeleted )
        {
          /**************************************************************/
          /* Term data start tag                                        */
          /**************************************************************/
          pszTag1 = pTag[TERM_TAG].uTagnameOffs + pTagNamesW;
          swprintf( pBuf, L" %s%c%c%c", pszTag1, END_TAG_CHAR, CR, LF );
          usRC = UtlBufWriteConv( pBufCB, pBuf,
                              (SHORT)(UTF16strlenCHAR(pBuf) * sizeof(CHAR_W)),
                              TRUE, usFormat, ulFormatCP, ulAnsiCP );

          /**************************************************************/
          /* Lemma                                                      */
          /**************************************************************/
          if ( !usRC )
          {
            pszTag1 = pTag[LEMMA_TAG].uTagnameOffs + pTagNamesW;
            pszTag2 = pTag[ELEMMA_TAG].uTagnameOffs + pTagNamesW;
            swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                     END_TAG_CHAR, pTerm->pszName, pszTag2,
                     END_TAG_CHAR, CR, LF );
            usRC = UtlBufWriteConv( pBufCB, pBuf,
                              (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                                    ulFormatCP, ulAnsiCP );
          } /* endif */

          /**************************************************************/
          /* Frequency                                                  */
          /**************************************************************/
          if ( !usRC && !fExternal )
          {
            pszTag1 = pTag[FREQUENCY_TAG].uTagnameOffs + pTagNamesW;
            pszTag2 = pTag[EFREQUENCY_TAG].uTagnameOffs + pTagNamesW;
            swprintf( pBuf, L"  %s%c%ld%s%c%c%c", pszTag1,
                     END_TAG_CHAR, pTerm->ulFrequency, pszTag2,
                     END_TAG_CHAR, CR, LF );
            usRC = UtlBufWriteConv( pBufCB, pBuf,
                              (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                              ulFormatCP, ulAnsiCP );
          } /* endif */

          /**************************************************************/
          /* Marks                                                      */
          /**************************************************************/
          if ( !usRC && !fExternal )
          {
            /************************************************************/
            /* Preprocess mark string                                   */
            /************************************************************/
            pszTemp = szMarkFlags;

            if ( pTerm->Flags.fMark == DELMARK_FLAG )
            {
              *pszTemp++ = DELETE_MARK;
            }
            else if ( pTerm->Flags.fMark == EXCLMARK_FLAG )
            {
              *pszTemp++ = EXCL_MARK;
            }
            else if ( pTerm->Flags.fMark == DICTMARK_FLAG )
            {
              *pszTemp++ = DICT_MARK;
            } /* endif */

            if ( pTerm->Flags.fSelected )
            {
              *pszTemp++ = SELECT_MARK;
            } /* endif */

            if ( pTerm->Flags.fTop )
            {
              *pszTemp++ = TOP_MARK;
            } /* endif */

            if ( pTerm->Flags.fContext )
            {
              *pszTemp++ = CONTEXT_MARK;
            } /* endif */

            *pszTemp = EOS;

            /************************************************************/
            /* Write mark tag to list                                   */
            /************************************************************/
            if ( szMarkFlags[0] )
            {
              pszTag1 = pTag[MARK_TAG].uTagnameOffs + pTagNamesW;
              pszTag2 = pTag[EMARK_TAG].uTagnameOffs + pTagNamesW;
              swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                       END_TAG_CHAR, szMarkFlags, pszTag2,
                       END_TAG_CHAR, CR, LF );
              usRC = UtlBufWriteConv( pBufCB, pBuf,
                                (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                                ulFormatCP, ulAnsiCP );
            } /* endif */
          } /* endif */

          /**************************************************************/
          /* Destination                                                */
          /**************************************************************/
          if ( !usRC && pTerm->pszDestination && !fExternal &&
               ( (pTerm->Flags.fMark == EXCLMARK_FLAG) ||
                 (pTerm->Flags.fMark == DICTMARK_FLAG) ) )
          {
            ASCII2Unicode(pTerm->pszDestination, chName, ulOemCP);
            pszTag1 = pTag[DEST_TAG].uTagnameOffs + pTagNamesW;
            pszTag2 = pTag[EDEST_TAG].uTagnameOffs + pTagNamesW;
            swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                     END_TAG_CHAR, chName, pszTag2,
                     END_TAG_CHAR, CR, LF );
            usRC = UtlBufWriteConv( pBufCB, pBuf,
                               (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                               ulFormatCP, ulAnsiCP );
          } /* endif */

          /**************************************************************/
          /* Translation                                                */
          /**************************************************************/
          if ( !usRC && pTerm->pszTranslation)
          {
            pszTag1 = pTag[TRANSLATION_TAG].uTagnameOffs + pTagNamesW;
            pszTag2 = pTag[ETRANSLATION_TAG].uTagnameOffs + pTagNamesW;
            swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                     END_TAG_CHAR, pTerm->pszTranslation, pszTag2,
                     END_TAG_CHAR, CR, LF );
            usRC = UtlBufWriteConv( pBufCB, pBuf,
                               (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                               ulFormatCP, ulAnsiCP );
          } /* endif */

          /**************************************************************/
          /* Context                                                    */
          /**************************************************************/
          if ( !usRC )
          {

            if ( pTerm->pszContext )
            {
              /********************************************************/
              /* User supplied context (via mark dialog). As user     */
              /* supplied contexts do not have an ID, we have to      */
              /* create an ID.                                        */
              /********************************************************/

              /********************************************************/
              /* Use next free context ID                             */
              /********************************************************/
              usContextID++;

              /********************************************************/
              /* Write the context to the list                        */
              /********************************************************/
              pszTag1 = pTag[CONTEXT_TAG].uTagnameOffs + pTagNamesW;
              pszTag2 = pTag[ECONTEXT_TAG].uTagnameOffs + pTagNamesW;
              if ( fExternal )
              {
                swprintf( pBuf, L"  %s%c", pszTag1, END_TAG_CHAR );
              }
              else
              {
                swprintf( szIDAttrValue, szIDAttr, usContextID );
                swprintf( pBuf, L"  %s %s%c", pszTag1, szIDAttrValue, END_TAG_CHAR );
              } /* endif */
              usRC = UtlBufWriteConv( pBufCB, pBuf,
                                 (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                                 ulFormatCP, ulAnsiCP );

              if ( !usRC  )
              {
                usRC = UtlBufWriteConv( pBufCB, pTerm->pszContext,
                                    (SHORT)(UTF16strlenBYTE(pTerm->pszContext)), TRUE, usFormat,
                                    ulFormatCP, ulAnsiCP );
              } /* endif */


              if ( !usRC )
              {
                swprintf( pBuf, L"%s%c%c%c", pszTag2, END_TAG_CHAR, CR, LF );
                usRC = UtlBufWriteConv( pBufCB, pBuf,
                                   (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                                   ulFormatCP, ulAnsiCP );
              } /* endif */
            }
            else if ( pTerm->pContextList )
            {
              /********************************************************/
              /* User context created in list processing of analysis. */
              /********************************************************/

              usContext = 0;               // index in context list
              while ( !usRC && (usContext < pTerm->pContextList->usUsed ) )
              {
                /**********************************************************/
                /* Check if context has already been written to list or   */
                /* list is written in external format                     */
                /**********************************************************/
                swprintf( szContextID, L"%d",
                         pTerm->pContextList->ausContextID[usContext] );
                if ( fExternal || !WinFindAtomW( hatomConRef, szContextID, ulOemCP ) )
                {
                  /********************************************************/
                  /* Write actual context to list                         */
                  /********************************************************/
                  pszTag1 = pTag[CONTEXT_TAG].uTagnameOffs + pTagNamesW;
                  pszTag2 = pTag[ECONTEXT_TAG].uTagnameOffs + pTagNamesW;
                  if ( fExternal )
                  {
                    swprintf( pBuf, L"  %s%c", pszTag1, END_TAG_CHAR );
                  }
                  else
                  {
                    swprintf( szIDAttrValue, szIDAttr,
                             pTerm->pContextList->ausContextID[usContext] );
                    swprintf( pBuf, L"  %s %s%c", pszTag1, szIDAttrValue, END_TAG_CHAR );
                  } /* endif */
                  usRC = UtlBufWriteConv( pBufCB, pBuf,
                                    (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                                    ulFormatCP, ulAnsiCP );

                  if ( !usRC  )
                  {
                    pszContext = LstGetContext( pContextTable,
                                    pTerm->pContextList->ausContextID[usContext] );
                    usRC = UtlBufWriteConv( pBufCB, pszContext,
                                     (SHORT)(UTF16strlenBYTE(pszContext)), TRUE, usFormat,
                                     ulFormatCP, ulAnsiCP );
                  } /* endif */

                  if ( !usRC )
                  {
                    swprintf( pBuf, L"%s%c%c%c", pszTag2, END_TAG_CHAR, CR, LF );
                    usRC = UtlBufWriteConv( pBufCB, pBuf,
                                       (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                                       ulFormatCP, ulAnsiCP );
                  } /* endif */

                  /********************************************************/
                  /* Add context ID to context reference atom table       */
                  /********************************************************/
                  if ( !fExternal )
                  {
                    WinAddAtomW( hatomConRef, szContextID, ulOemCP );
                  } /* endif */
                }
                else
                {
                  /********************************************************/
                  /* Write context reference to list                      */
                  /********************************************************/
                  pszTag1 = pTag[CONREF_TAG].uTagnameOffs + pTagNamesW;
                  pszTag2 = pTag[ECONREF_TAG].uTagnameOffs + pTagNamesW;
                  swprintf( pBuf, L"  %s%c%s%s%c%c%c", pszTag1,
                           END_TAG_CHAR, szContextID, pszTag2,
                           END_TAG_CHAR, CR, LF );
                  usRC = UtlBufWriteConv( pBufCB, pBuf,
                                    (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                                    ulFormatCP, ulAnsiCP );
                } /* endif */

                usContext++;               // continue with next context

              } /* endwhile */
            } /* endif */
          } /* endif */

          /**************************************************************/
          /* Term data end tag                                          */
          /**************************************************************/
          if ( !usRC )
          {
            pszTag1 = pTag[ETERM_TAG].uTagnameOffs + pTagNamesW;
            swprintf( pBuf, L" %s%c%c%c", pszTag1, END_TAG_CHAR, CR, LF );
            usRC = UtlBufWriteConv( pBufCB, pBuf,
                               (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                               ulFormatCP, ulAnsiCP );
          } /* endif */

          if ( usRC )
          {
            break;                       // leave for loop
          } /* endif */
        } /* endif */

      } /* endfor */
      pTermTable = pTermTable->pNextTable;
    } /* endwhile */
  } /* endif */

  /**************************************************************/
  /* List end tag                                               */
  /**************************************************************/
  if ( !usRC )
  {
    if ( Type == NTL_TYPE )
    {
      pszTag1 = pTag[ENTLIST_TAG].uTagnameOffs + pTagNamesW;
    }
    else
    {
      pszTag1 = pTag[EFTLIST_TAG].uTagnameOffs + pTagNamesW;
    } /* endif */
    swprintf( pBuf, L"%s%c%c%c", pszTag1, END_TAG_CHAR, CR, LF );
    usRC = UtlBufWriteConv( pBufCB, pBuf,
                       (SHORT)(UTF16strlenBYTE(pBuf)), TRUE, usFormat,
                       ulFormatCP, ulAnsiCP );
  } /* endif */

  /********************************************************************/
  /* Close temporary list, erase original list and rename new one     */
  /********************************************************************/
  if ( !usRC )
  {
    usRC = UtlBufClose( pBufCB, TRUE );

    if ( !fExternal )                                           /* 2@KIT1291A */
    {
      fExist = UtlFileExist( pszListName );

      if ( !usRC && fExist )
      {
        usRC = UtlDelete( pszListName, 0L, TRUE );
      } /* endif */
      if ( !usRC )
      {
        usRC = UtlMove( szTempName, pszListName, 0L, TRUE );
      } /* endif */
    } /* endif */                                               /* 1@KIT1291A */

    /******************************************************************/
    /* Communicate updated or created internal lists                  */
    /******************************************************************/
    if ( !usRC && !fExternal )
    {
      if ( fExist )
      {
        EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                             MP1FROMSHORT( PROP_CLASS_LIST ),
                             MP2FROMP(pszListName) );
      }
      else
      {
        EqfSend2AllHandlers( WM_EQFN_CREATED,
                             MP1FROMSHORT( clsLIST ),
                             MP2FROMP(pszListName) );
      } /* endif */

    } /* endif */
  }
  else
  {
     if ( pBufCB )
     {
       UtlBufClose( pBufCB, FALSE );
     } /* endif */
     UtlDelete( szTempName, 0L, FALSE );
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pBuf )           UtlAlloc( (PVOID *) &pBuf, 0L, 0L, NOMSG );
  if ( pLoadedTable )   TAFreeTagTable( pLoadedTable );
  if ( hatomConRef )    WinDestroyAtomTable( hatomConRef );

  return( usRC );
} /* end of function LstWriteSGMLList */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstAddPtrToList                                          |
//+----------------------------------------------------------------------------+
//|Function call:     LstAddPtrToList( PPTRLIST *ppPtrList, PVOID Ptr );       |
//+----------------------------------------------------------------------------+
//|Description:       Adds the given ptr to the pointer list. The pointer list |
//|                   is created if it does not exist. The end of the pointer  |
//|                   list is indicated using a NULL pointer.                  |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPTRLIST   *ppPtrList     address of pointer list pointer|
//|                   PVOID      Ptr            pointer being added to pointer |
//|                                             list                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      = Ptr has been added                              |
//|                   ERROR_NOT_ENOUGH_MEMORY  = Could not add Ptr due to      |
//|                                   memory shortage                          |
//+----------------------------------------------------------------------------+
//|Function flow:     if pointer list pointer is NULL then                     |
//|                     create initial pointer list                            |
//|                   endif                                                    |
//|                   if pointer list is full then                             |
//|                     enlarge pointer list                                   |
//|                   endif                                                    |
//|                   if ok then                                               |
//|                     add pointer to pointer list                            |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
USHORT LstAddPtrToList
(
  PPTRLIST   *ppPtrList,               // address of pointer list pointer
  PVOID      Ptr                       // pointer being added to pointer list
)
{
  USHORT        usRC = 0;              // return code of function
  PPTRLIST      pPtrList;              // pointer list pointer
  ULONG         ulSize;                // overall size of pointer list

  /********************************************************************/
  /* If no pointer list exists, create one                            */
  /********************************************************************/
  pPtrList = *ppPtrList;
  if ( pPtrList == NULL )
  {
    ulSize = (LONG) (sizeof(PTRLIST) + (PTR_LIST_ENTRIES * sizeof(PVOID)));
    UtlAlloc( (PVOID *) &pPtrList, 0L, ulSize, NOMSG );
    if ( pPtrList )
    {
      pPtrList->usMaxEntries  = PTR_LIST_ENTRIES;
      pPtrList->usEntries     = 1;     // one ptr -> NULL element
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if pointer list if full, enlarge pointer list                    */
  /********************************************************************/
  if ( !usRC )
  {
    if ( pPtrList->usEntries == pPtrList->usMaxEntries )
    {
      ulSize = (LONG)(sizeof(PTRLIST) +
                      (pPtrList->usMaxEntries * sizeof(PVOID)));
      UtlAlloc( (PVOID *) &pPtrList, ulSize,
                      ulSize + (PTR_LIST_ENTRIES * sizeof(PVOID)), NOMSG );
      if ( pPtrList )
      {
        pPtrList->usMaxEntries += PTR_LIST_ENTRIES;
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Add pointer to pointer list                                      */
  /********************************************************************/
  if ( !usRC  )
  {
    pPtrList->Ptr[pPtrList->usEntries-1] = Ptr;
    pPtrList->Ptr[pPtrList->usEntries]   = NULL;
    pPtrList->usEntries++;
  } /* endif */

  /********************************************************************/
  /* Return return code to calling function                           */
  /********************************************************************/
  *ppPtrList = pPtrList;
  return( usRC );

} /* end of function LstAddPtrToList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstGetListTypeFromName                                   |
//+----------------------------------------------------------------------------+
//|Function call:     LstGetListTypeFromName( PSZ pszListName );               |
//+----------------------------------------------------------------------------+
//|Description:       Evaluate the type of list from the fully qualified list  |
//|                   name. Only the built-in list types are supported.        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ     pszListName    fully qualified list name         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       UNKNOWN_TYPE     type of list could not be evaluated     |
//|                   NTL_TYPE         list is a new terms list                |
//|                   FTL_TYPE         list is a found terms list              |
//|                   EXCL_TYPE        list is an exclusion list               |
//|                   NOISE_TYPE       list is a noise list                    |
//+----------------------------------------------------------------------------+
//|Samples:           usType = LstGetListTypeFromName( "D:\\EQF\\LIST\\NEW.LST |
//+----------------------------------------------------------------------------+
//|Function flow:     get pointer to begin of file extension                   |
//|                   if file extension found then                             |
//|                     if extension is found terms list extension then        |
//|                       set return code to FTL_TYPE                          |
//|                     elseif extension is new terms list extension then      |
//|                       set return code to NTL_TYPE                          |
//|                     elseif extension is noise/exclusion list extension then|
//|                       if list path is noise list path then                 |
//|                         set return code to NOISE_TYPE                      |
//|                       elseif list path is exclusion list path then         |
//|                         set return code to EXCL_TYPE                       |
//|                       else                                                 |
//|                         set return code to UNKNOWN_TYPE                    |
//|                       endif                                                |
//|                     else                                                   |
//|                       set return code to UNKNOWN_TYPE                      |
//|                     endif                                                  |
//|                   else                                                     |
//|                     set return code to UNKNOWN_TYPE                        |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
USHORT LstGetListTypeFromName
(
  PSZ    pszListName                   // fully qualified list name
)
{
  PSZ      pszExtension;               // position of extension in list name
  USHORT   usListType = UNKNOWN_TYPE;  // type of list
  CHAR     szPath[MAX_EQF_PATH];       // buffer for list paths

  /********************************************************************/
  /* Get position of extension in list name                           */
  /********************************************************************/
  pszExtension = strchr( UtlGetFnameFromPath( pszListName ), DOT );

  /********************************************************************/
  /* Evaluate list type                                               */
  /********************************************************************/
  if ( pszExtension )
  {
    if ( stricmp( pszExtension, EXT_OF_NEWTERMS_LIST ) == 0 )
    {
      usListType = NTL_TYPE;
    }
    else if ( stricmp( pszExtension, EXT_OF_FOUNDTERMS_LIST ) == 0 )
    {
      usListType = FTL_TYPE;
    }
    else if ( stricmp( pszExtension, EXT_OF_EXCLUSION ) == 0 )
    {
      /****************************************************************/
      /* Check for table path = NOISE lists path                      */
      /* ignoring the first path character (= drive letter)           */
      /****************************************************************/
      UtlMakeEQFPath( szPath, NULC, TABLE_PATH, NULL );
      if ( memicmp( szPath + 1, pszListName + 1, strlen(szPath) - 1) == 0)
      {
        usListType = NOISE_TYPE;
      }
      else
      {
        /**************************************************************/
        /* Check for list path = exclusion lists path                 */
        /* ignoring the first path character (= drive letter)         */
        /**************************************************************/
        UtlMakeEQFPath( szPath, NULC, LIST_PATH, NULL );
        if ( memicmp( szPath + 1, pszListName + 1, strlen(szPath) - 1) == 0)
        {
          usListType = EXCL_TYPE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return evaluated list type to calling function                   */
  /********************************************************************/
  return( usListType );

} /* end of function LstGetListTypeFromName */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstRefreshListsLB                                        |
//+----------------------------------------------------------------------------+
//|Function call:     LstRefreshListsLB( PLISTIDA pIda, BOOL fSaveSelection ); |
//+----------------------------------------------------------------------------+
//|Description:       Refreshes the lists in the lists list box of the list    |
//|                   instance window by calling the active list processor     |
//|                   to fill the list box. If fSaveSelection flag is set,     |
//|                   the function trys to keep the selected list and the      |
//|                   first visible list in the list box constant.             |
//+----------------------------------------------------------------------------+
//|Input parameter:   PLISTIDA   pIda           pointer to list window IDA     |
//|                   BOOL       fSaveSelection save current selection flag    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       LST_OK_RC  if function completed successfully            |
//|                   other      return code of called list processor          |
//+----------------------------------------------------------------------------+
//|Function flow:     disable list box window update                           |
//|                   if fSaveSelected is set then                             |
//|                     get currently selected item                            |
//|                     get current top item                                   |
//|                   endif                                                    |
//|                   delete all list box items                                |
//|                   call active list processor to fill list box              |
//|                   if ok and fSaveSelected then                             |
//|                     if saved top item is in list then                      |
//|                       set item as top item                                 |
//|                     endif                                                  |
//|                     if saved selected item is in list then                 |
//|                       select items                                         |
//|                     else                                                   |
//|                       select first item                                    |
//|                     endif                                                  |
//|                   end                                                      |
//|                   enable list box window update                            |
//|                   return list processor return code                        |
//+----------------------------------------------------------------------------+
USHORT LstRefreshListsLB
(
  PLSTIDA    pIda,                     // pointer to list window IDA
  BOOL       fSaveSelection            // save current selection flag
)
{
  USHORT      usRC = LST_OK_RC;        // list processor return code
  SHORT       sItem;                   // item index
  CHAR        szTopItem[MAX_EQF_PATH]; // name of first visible list
  CHAR        szSelItem[MAX_EQF_PATH]; // name of selected list

  /********************************************************************/
  /* Disable list box window update                                   */
  /********************************************************************/
  ENABLEUPDATEHWND_FALSE( pIda->hLBox );

  /********************************************************************/
  /* if fSaveSelected save current top item and current selection     */
  /********************************************************************/
  if ( fSaveSelection )
  {
    sItem = (SHORT) WinSendMsg( pIda->hLBox, LM_QUERYTOPINDEX, 0L, 0L );
    if ( sItem != LIT_NONE )
    {
      QUERYITEMTEXTHWND( pIda->hLBox, sItem, pIda->szBuffer );
      strcpy( szTopItem, UtlParseX15( pIda->szBuffer, LST_OBJECT_IND ) );
    }
    else
    {
      szTopItem[0] = EOS;
    } /* endif */

    sItem = QUERYSELECTIONHWND( pIda->hLBox );
    if ( sItem != LIT_NONE )
    {
      QUERYITEMTEXTHWND( pIda->hLBox, sItem, pIda->szBuffer );
      strcpy( szSelItem, UtlParseX15( pIda->szBuffer, LST_OBJECT_IND ) );
    }
    else
    {
      szSelItem[0] = EOS;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Fill listbox                                                     */
  /********************************************************************/
  LstInsertListNames( pIda->Type, TRUE, pIda->hLBox, pIda->szBuffer, TRUE );

  /********************************************************************/
  /* If ok and fSaveSelected is set try to select previously selected */
  /* list and try to set first visible list                           */
  /********************************************************************/
  if ( (usRC == LST_OK_RC) && fSaveSelection )
  {
    sItem = SEARCHITEMHWND( pIda->hLBox, szTopItem );
    if ( sItem != LIT_NONE )
    {
      SETTOPINDEXHWND( pIda->hLBox, sItem );
    } /* endif */

    sItem = SEARCHITEMHWND( pIda->hLBox, szSelItem );
    if ( sItem != LIT_NONE )
    {
      SELECTITEMHWND( pIda->hLBox, sItem );
    }
    else
    {
      SELECTITEMHWND( pIda->hLBox, 0 );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Enable list box window update                                    */
  /********************************************************************/
  ENABLEUPDATEHWND_TRUE( pIda->hLBox  );

  /********************************************************************/
  /* return list processor return code                                */
  /********************************************************************/
  return( usRC );

} /* end of function LstRefreshListsLB */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstCompare                                               |
//+----------------------------------------------------------------------------+
//|Function call:     LstCompare( PUSHORT pusIndex1, PUSHORT pusIndex2 );      |
//+----------------------------------------------------------------------------+
//|Description:       Compare two terms (used by qsort in LstWriteExclList)    |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUSHORT     pusIndex1    pointer to index of first term  |
//|                   PUSHORT     pusIndex2    pointer to index of second term |
//+----------------------------------------------------------------------------+
//|Returncode type:   int                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       -1          pusIndex1 term is smaller than pusIndex2 term|
//|                    0          pusIndex1 term is equal to pusIndex2 term    |
//|                    1          pusIndex1 term is greater than pusIndex2 term|
//+----------------------------------------------------------------------------+
//|Prerequesits:      global variable pszGlobTermBuf must point to the begin of|
//|                   the term buffer                                          |
//+----------------------------------------------------------------------------+
//|Function flow:     return strcmp result                                     |
//+----------------------------------------------------------------------------+
int LstCompare
(
  const void *parg1,                   // pointer to index of first term
  const void *parg2                    // pointer to index of second term
)
{
  PSZ   *ppszTerm1 = (PSZ *)parg1;
  PSZ   *ppszTerm2 = (PSZ *)parg2;

  return( stricmp( *ppszTerm1, *ppszTerm2 ) );
} /* end of function LstCompare */

int LstCompareW
(
  const void *parg1,                   // pointer to index of first term
  const void *parg2                    // pointer to index of second term
)
{
  PSZ_W   *ppszTerm1 = (PSZ_W *)parg1;
  PSZ_W   *ppszTerm2 = (PSZ_W *)parg2;

  return( _wcsicmp( *ppszTerm1, *ppszTerm2 ) );
} /* end of function LstCompare */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstWriteExclList                                         |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Write an exclusion list or a noise list in the MAT       |
//|                   internal format. Errors are handled within this          |
//|                   function.                                                |
//|                   The input term buffer contains the terms as zero-        |
//|                   terminated strings. The end of the buffer is indicated   |
//|                   by an empty string.                                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ     pszListName    name of out list (fully qualified)|
//|                   PSZ     pszTermBuf     pointer to term buffer            |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    List has been written to disk                    |
//|                   FALSE   Errors occured                                   |
//+----------------------------------------------------------------------------+
//|Samples:           fOK = LstWriteExclList( "D:\\EQF\\TABLE\\TEST.LST",      |
//|                                           "term1\0term2\0term3\0\0" );     |
//+----------------------------------------------------------------------------+
//|Function flow:     get number of terms in term buffer, compute sizes,       |
//|                    fill-in header data, prepare file names                 |
//|                   check if max exclusion list size is exceeded             |
//|                   allocate term index array                                |
//|                   fill term index array                                    |
//|                   sort term index array                                    |
//|                   check if target disk has enough space left for list      |
//|                   open output file                                         |
//|                   write list to disk                                       |
//|                   close output file                                        |
//|                   delete old list file (if any) and rename temp file       |
//|                   cleanup                                                  |
//+----------------------------------------------------------------------------+
BOOL LstWriteExclList
(
  PSZ     pszListName,                 // name of out list (fully qualified)
  PSZ_W   pszTermBuf                   // pointer to term buffer
)
{
  BOOL    fOK = TRUE;                  // our OK flag; is returned to caller
  USHORT  usTerms;                     // number of terms in list
  ULONG   ulTermBufSize;               // size of term buffer
  PSZ_W   pszTerm;                     // pointer to current term
  EXCLUSIONLIST ListHeader;            // header for exclusion list
  PUSHORT pusTermIndex = NULL;         // pointer to term index array
  USHORT  usI;                         // generally use loop index
  CHAR    szTempName[MAX_EQF_PATH];    // buffer for name of temporary file
  ULONG   ulListSize;                  // overall size of list
  CHAR    szListName[MAX_FNAME];       // buffer for list name
  CHAR    szListDrive[MAX_DRIVE];      // buffer for drive name
  PSZ     apszErrParm[10];             // error parameter array
  USHORT  usDosRC = 0;                 // Return code from Dos operations
  HFILE   hOutFile = NULLHANDLE;       // File handle for input file
  USHORT  usAction;                    // file action performed by DosOpen
  ULONG   ulBytesWritten = 0;          // number of bytes written to file
  PSZ_W   *pszTerms = NULL;            // array of term pointers

  /********************************************************************/
  /* Get number of terms in term buffer, compute sizes and            */
  /* prepare file names                                               */
  /********************************************************************/
  usTerms = 0;
  pszTerm = pszTermBuf;
  while ( *pszTerm )
  {
    usTerms++;
    pszTerm += UTF16strlenCHAR(pszTerm) + 1;
  } /* endwhile */
  ulTermBufSize = (PBYTE)pszTerm - (PBYTE)pszTermBuf;
  ulListSize = strlen(UNICODEFILEPREFIX) +
               sizeof(EXCLUSIONLIST) + (usTerms * sizeof(USHORT)) +
               ulTermBufSize;

  Utlstrccpy( szListName, UtlGetFnameFromPath( pszListName ), DOT );

  /********************************************************************/
  /* Check if max exclusion list size is exceeded                     */
  /********************************************************************/
  if ( fOK )
  {
    if ( ulListSize >= MAX_EXCLLIST_SIZE )
    {
      apszErrParm[0] = szListName;
      UtlError( ERROR_LST_TOO_LARGE, MB_CANCEL, 1, apszErrParm, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Allocate term pointer array                                      */
  /********************************************************************/
  if ( fOK  )
  {
    fOK = UtlAlloc( (PVOID *) (PVOID *)&pszTerms, 0L,
                    (LONG) max( MIN_ALLOC, usTerms * sizeof(PSZ_W)),
                    ERROR_STORAGE );
  } /* endif */

  /************************************************************/
  /* Fill term pointer array                                  */
  /************************************************************/
  if ( fOK  )
  {
    pszTerm = pszTermBuf;
    for ( usI = 0; usI < usTerms; usI++ )
    {
      pszTerms[usI] = pszTerm;
      pszTerm += UTF16strlenCHAR(pszTerm) + 1;
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* Sort term pointer array                                          */
  /********************************************************************/
  if ( fOK )
  {
    qsort( (PVOID)pszTerms,
           (size_t)usTerms,
           (size_t)sizeof(PSZ_W),
           LstCompareW );
  } /* endif */

  /********************************************************************/
  /* Remove duplicate terms from list                                 */
  /********************************************************************/
  if ( fOK )
  {
    SHORT  sCurTerm = 0;
    while ( sCurTerm < ((SHORT)usTerms - 1) ) // do up to n-1 term
    {
      if ( UTF16strcmp( pszTerms[sCurTerm], pszTerms[sCurTerm+1] ) == 0 )
      {
        /**************************************************************/
        /* remove duplicate                                           */
        /**************************************************************/
        memmove( &(pszTerms[sCurTerm]), &(pszTerms[sCurTerm+1]),
                 (usTerms - sCurTerm - 1) * sizeof(PSZ_W) );
        usTerms--;
      }
      else
      {
        sCurTerm++;
      } /* endif */
    } /* endwhile */
  } /* endif */


  /********************************************************************/
  /* Allocate term index array                                        */
  /********************************************************************/
  if ( fOK  )
  {
    fOK = UtlAlloc( (PVOID *) &pusTermIndex, 0L,
                    (LONG) max( MIN_ALLOC, usTerms * sizeof(USHORT)),
                    ERROR_STORAGE );
  } /* endif */

  /************************************************************/
  /* Fill term index array                                    */
  /************************************************************/
  if ( fOK  )
  {
    for ( usI = 0; usI < usTerms; usI++ )
    {
      pusTermIndex[usI] = (USHORT)(pszTerms[usI] - pszTermBuf);
    } /* endfor */
  } /* endif */

  /************************************************************/
  /* Check if target disk has enough space free to  receive   */
  /* the file (even if an existing file is overwritten, the   */
  /* complete size is required as the file is written under   */
  /* a temp name and renamed after successful operation to    */
  /* the actual name)                                         */
  /************************************************************/
  if ( fOK  )
  {
    if ( UtlQueryFreeSpace( pszListName[0], TRUE ) <
         (ULONG64)( strlen(UNICODEFILEPREFIX) +
                 sizeof(EXCLUSIONLIST) + ulTermBufSize +
                 (usTerms * sizeof(USHORT) ) ) )
    {
      sprintf( szListDrive, "%c%c", pszListName[0], COLON );
      apszErrParm[0] = szListName;
      apszErrParm[1] = szListDrive;
      UtlError( ERROR_LST_NOSPACE, MB_CANCEL, 2, apszErrParm, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Setup list header                                                */
  /********************************************************************/
  if ( fOK )
  {
    ulListSize = strlen(UNICODEFILEPREFIX) +
                 sizeof(EXCLUSIONLIST) + (usTerms * sizeof(USHORT)) +
                 ulTermBufSize;
    ListHeader.uLength      = (SHORT)ulListSize;
    ListHeader.usNumEntries = usTerms;
    ListHeader.uFirstEntry  = sizeof(ListHeader);
    ListHeader.uStrings     = sizeof(ListHeader) + (usTerms * sizeof(USHORT));
  } /* endif */

  /************************************************************/
  /* Open output file                                         */
  /************************************************************/
  if ( fOK  )
  {
    strcpy( szTempName, pszListName );
    UtlSplitFnameFromPath( szTempName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, LIST_TEMP_NAME );
    usDosRC = UtlOpen( szTempName, &hOutFile, &usAction,
                       0L,
                       FILE_NORMAL,
                       FILE_CREATE | FILE_TRUNCATE,
                       OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
                       0L,
                       TRUE );
    if ( usDosRC != NO_ERROR )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Write list to disk                                               */
  /********************************************************************/
  if ( fOK  )
  {
    if ( !usDosRC )
    {
      usDosRC = UtlWriteL( hOutFile, UNICODEFILEPREFIX, strlen(UNICODEFILEPREFIX),
                &ulBytesWritten, TRUE );
    } /* endif */
    if (usDosRC != NO_ERROR )
    {
        fOK = FALSE;
    }
    else
    {
      usDosRC = UtlWriteL( hOutFile, &ListHeader, sizeof(ListHeader),
              &ulBytesWritten, TRUE );
    } /* endif */

    if ( usDosRC != NO_ERROR )
    {
      fOK = FALSE;
    }
    else if ( sizeof(ListHeader) != ulBytesWritten )
    {
      usDosRC = ERROR_DISK_FULL;
      UtlError( usDosRC, MB_CANCEL, 1, &pszListName, DOS_ERROR );
      fOK = FALSE;
    } /* endif */

    if ( fOK )
    {
      usDosRC = UtlWriteL( hOutFile, pusTermIndex, (usTerms * sizeof(USHORT)),
                &ulBytesWritten, TRUE );
      if ( usDosRC != NO_ERROR )
      {
        fOK = FALSE;
      }
      else if ( (usTerms * sizeof(USHORT)) != ulBytesWritten )
      {
        usDosRC = ERROR_DISK_FULL;
        UtlError( usDosRC, MB_CANCEL, 1, &pszListName, DOS_ERROR );
        fOK = FALSE;
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      usDosRC = UtlWriteL( hOutFile, pszTermBuf, ulTermBufSize,
                &ulBytesWritten, TRUE );
      if ( usDosRC != NO_ERROR )
      {
        fOK = FALSE;
      }
      else if ( ulTermBufSize != ulBytesWritten )
      {
        usDosRC = ERROR_DISK_FULL;
        UtlError( usDosRC, MB_CANCEL, 1, &pszListName, DOS_ERROR );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  /************************************************************/
  /* Close output file                                        */
  /************************************************************/
  if ( hOutFile  )
  {
    usDosRC = UtlClose( hOutFile, TRUE );
    if ( usDosRC != NO_ERROR )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /************************************************************/
  /* Delete old list file and rename temp name                */
  /************************************************************/
  if ( fOK  )
  {
    usDosRC = UtlDelete( pszListName, 0L, FALSE );
    usDosRC = UtlMove( szTempName, pszListName, 0L, TRUE );
    if ( usDosRC != NO_ERROR )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pusTermIndex )   UtlAlloc( (PVOID *) &pusTermIndex, 0L, 0L, NOMSG );
  if ( pszTerms )   UtlAlloc( (PVOID *) (PVOID *)&pszTerms, 0L, 0L, NOMSG );

  return( fOK );

} /* end of function LstWriteExclList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstPrintList                                             |
//+----------------------------------------------------------------------------+
//|Function call:     LstPrintList( PSZ pszListPath, USHORT usListType );      |
//+----------------------------------------------------------------------------+
//|Description:       Prints a list. Supported list types are the built-in     |
//|                   list types of the list processor.                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ     pszListPath     Fully qualified list name        |
//|                   USHORT  usListType      Type of list (see LISTTYPES enum)|
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    list has been printed                            |
//|                   FALSE   errors occured, list has not or only partially   |
//|                           been printed                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
BOOL LstPrintList
(
  PSZ     pszListPath,                 // Fully qualified list name
  USHORT  usListType                   // Type of list (see LISTTYPES enum)
)
{
  BOOL          fOK = TRUE;            // internal OK flag
  PEXCLUSIONLIST pExclList = NULL;     // pointer to loaded exclusion lists
  PSZ           pszLineBuffer = NULL;  // ptr to line buffer
  CHAR          szListName[MAX_FNAME]; // buffer for name of list
  PSZ           pszParm;               // ptr to error message parameters
  USHORT        usI;                   // general loop index or length buffer
  HPRINT        hPrint = NULLHANDLE;   // print handle
  PSZ_W         pszTerm;               // pointer to current term
  PUSHORT       pusTermInd;            // pointer to term index
  LISTHEADER    ListHead;              // header for NTL+FTL lists
  PCONTEXTTABLE pContextTable= NULL;   // context table for NTL+FTL lists
  PTERMTABLE    pTermTable   = NULL;   // term table for NTL+FTL lists
  PPOOL         pPool        = NULL;   // string pool for NTL+FTL lists
  PSZ_W         pszContext;            // pointer to current context
  USHORT        usContext;             // index for context list processing
  USHORT        usRC;                  // return code of called functions
  PTERMTABLE    pTable;                // ptr to current term table
  PTERM         pTerm;                 // ptr to current term

  /********************************************************************/
  /* Isolate list name                                                */
  /********************************************************************/
  Utlstrccpy( szListName, UtlGetFnameFromPath( pszListPath ), DOT );

  /********************************************************************/
  /* Allocate line buffer                                             */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &pszLineBuffer, 0L, (LONG)(2 * MAX_PRINT_LINE),
                    ERROR_STORAGE );
  } /* endif */

  /****************************************************************/
  /* Read list                                                    */
  /****************************************************************/
  if ( fOK )
  {
    switch ( usListType )
    {
      case NTL_TYPE :
      case FTL_TYPE :
        if ( fOK )
        {
          memset( &ListHead, 0, sizeof(ListHead) );
          usRC = LstReadSGMLList( pszListPath, &ListHead,
                                  &pTermTable, &pContextTable,
                                  &pPool, FALSE, (LISTTYPES)usListType, SGMLFORMAT_UNICODE);
          fOK = ( usRC == NO_ERROR );
        } /* endif */
        break;
      case NOISE_TYPE :
      case EXCL_TYPE :
        {  ULONG  ulLength = 0L;
          /****************************************************************/
          /* Load noise / exclusion list                                  */
          /****************************************************************/
          pExclList = NULL;
          usRC = LstReadNoiseExclList( pszListPath, &ulLength, &pExclList );
          fOK = ( usRC == NO_ERROR);

     //     fOK = UtlLoadFile( pszListPath, (PVOID *)&pExclList, &usI, FALSE, TRUE );
        }
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* Setup list title                                                 */
  /********************************************************************/
  if ( fOK )
  {
    SHORT   sTitleStringID = 0;

    /******************************************************************/
    /* Get string ID for list title                                   */
    /******************************************************************/
    switch ( usListType )
    {
      case NTL_TYPE :   sTitleStringID = SID_LISTPRINT_NTL_TITLE; break;
      case FTL_TYPE :   sTitleStringID = SID_LISTPRINT_FTL_TITLE; break;
      case NOISE_TYPE : sTitleStringID = SID_LISTPRINT_NOISE_TITLE; break;
      case EXCL_TYPE :  sTitleStringID = SID_LISTPRINT_EXCL_TITLE; break;
      default :
        /****************************************************************/
        /* Unknown list type: seems to be an internal error ...         */
        /****************************************************************/
        UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
        fOK = FALSE;
        break;
    } /* endswitch */

    /**************************************************************/
    /* Load title text into first part of line buffer             */
    /**************************************************************/
    if ( fOK )
    {
      HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      WinLoadString( NULLHANDLE, hResMod, sTitleStringID, MAX_PRINT_LINE,
                     pszLineBuffer );
      ANSITOOEM( pszLineBuffer );
    } /* endif */

    /**************************************************************/
    /* Insert list name parameter, target is second half of       */
    /* line buffer                                                */
    /**************************************************************/
    if ( fOK )
    {
      ULONG MsgLen;

      pszParm = szListName;
      DosInsMessage( &pszParm, 1, pszLineBuffer,
                   (strlen( pszLineBuffer ) + 1),
                   pszLineBuffer + MAX_PRINT_LINE,
                   MAX_PRINT_LINE, &MsgLen );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Open print handle                                                */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlPrintOpen( &hPrint, pszLineBuffer + MAX_PRINT_LINE, NULLHANDLE  );
    if ( !fOK )
    {
      pszParm = szListName;
      UtlError( ERROR_LST_PRINT, MB_CANCEL, 1, &pszParm, EQF_ERROR );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Print list                                                       */
  /********************************************************************/
  if ( fOK )
  {
    switch ( usListType )
    {
      case NTL_TYPE :
      case FTL_TYPE :
        if ( fOK )
        {
          /**************************************************************/
          /* Print generated title line                                 */
          /**************************************************************/
          fOK = UtlPrintLine( hPrint, pszLineBuffer + MAX_PRINT_LINE );
          if ( fOK )
          {
            fOK = UtlPrintLine( hPrint, EMPTY_STRING );
          } /* endif */
          if ( !fOK )
          {
            pszParm = szListName;
            UtlError( ERROR_LST_PRINT, MB_CANCEL, 1, &pszParm, EQF_ERROR );
          } /* endif */
        } /* endif */

        if ( fOK )
        {
          /**********************************************************/
          /* Print all terms of the list                            */
          /**********************************************************/
          if ( fOK )
          {
            pTable = pTermTable;
            while ( fOK && pTable )
            {
              pTerm = (PTERM)(pTable+1);
              for ( usI = 0;
                    (usI < pTable->usUsedEntries) && fOK;
                    usI++, pTerm++ )
              {
                /********************************************************/
                /* Print name of term                                   */
                /********************************************************/
                fOK = UtlPrintLineW( hPrint, pTerm->pszName );
                if ( !fOK )
                {
                  pszParm = szListName;
                  UtlError( ERROR_LST_PRINT, MB_CANCEL, 1, &pszParm, EQF_ERROR );
                } /* endif */

                /****************************************************/
                /* Print context string                             */
                /****************************************************/
                if ( pTerm->pszContext )
                {
                  fOK = UtlPrintLineW( hPrint, pTerm->pszContext );
                }
                else if ( pTerm->pContextList )
                {
                  usContext = 0;
                  while ( fOK && (usContext < pTerm->pContextList->usUsed) )
                  {
                    pszContext = LstGetContext( pContextTable,
                                   pTerm->pContextList->ausContextID[usContext] );
                    fOK = UtlPrintLineW( hPrint, pszContext );
                    usContext++;
                  } /* endwhile */
                } /* endif */

              } /* endfor */
              pTable = pTable->pNextTable;
            } /* endwhile */
          } /* endif */
        } /* endif */
        break;

      case NOISE_TYPE :
      case EXCL_TYPE :
        /****************************************************************/
        /* Print list title                                             */
        /****************************************************************/
        if ( fOK )
        {
          /**************************************************************/
          /* Print generated title line                                 */
          /**************************************************************/
          fOK = UtlPrintLine( hPrint, pszLineBuffer + MAX_PRINT_LINE );
          if ( fOK )
          {
            fOK = UtlPrintLine( hPrint, EMPTY_STRING );
          } /* endif */
          if ( !fOK )
          {
            pszParm = szListName;
            UtlError( ERROR_LST_PRINT, MB_CANCEL, 1, &pszParm, EQF_ERROR );
          } /* endif */
        } /* endif */

        /****************************************************************/
        /* Print terms in list                                          */
        /****************************************************************/
        if ( fOK )
        {
          /**************************************************************/
          /* Position to term offset area in noise / exclusion list     */
          /**************************************************************/
          pusTermInd = (PUSHORT) (((PSZ)pExclList) + pExclList->uFirstEntry);

          /**************************************************************/
          /* Print the terms                                            */
          /**************************************************************/
          for ( usI = 0; (usI < pExclList->usNumEntries) && fOK; usI++ )
          {
            pszTerm = (PSZ_W)((PBYTE)pExclList + pExclList->uStrings) + *pusTermInd;
            fOK = UtlPrintLineW( hPrint, pszTerm );
             if ( !fOK )
             {
               pszParm = szListName;
               UtlError( ERROR_LST_PRINT, MB_CANCEL, 1, &pszParm, EQF_ERROR );
             } /* endif */
             pusTermInd++;
          } /* endfor */
        } /* endif */

        break;

      default :
        /****************************************************************/
        /* Unknown list type: seems to be an internal error ...         */
        /****************************************************************/
        UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
        fOK = FALSE;
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pExclList )      UtlAlloc( (PVOID *) &pExclList, 0L, 0L, NOMSG );
  if ( pszLineBuffer )  UtlAlloc( (PVOID *) &pszLineBuffer, 0L, 0L, NOMSG );
  if ( hPrint )         UtlPrintClose( hPrint );
  if ( pTermTable )     LstDestroyTermTable( pTermTable );
  if ( pContextTable )  LstDestroyContextTable( pContextTable );
  if ( pPool )          PoolDestroy( pPool );

  return( fOK );
} /* end of function LstPrintList */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstReadNoiseExclList   Read an exclusion/noise list      |
//+----------------------------------------------------------------------------+
//|Function call:     LstReadNoiseExclList( pszListName, pusListSize,          |
//|                                         ppExcList );                       |
//+----------------------------------------------------------------------------+
//|Description:       Read an internal format noise/exclusion list into        |
//|                   memory and do a consistency check. Errors are handled    |
//|                   within this procedure.                                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ             pszListName  name of list to read        |
//|                   PUSHORT         pusListSize  address of list size var    |
//|                   PEXCLUSIONLIST  *ppExclList  address of excl. list ptr   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR          function completed OK                  |
//|                   other             DOS error codes                        |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlLoadFile to read list into memory                |
//|                   if ok                                                    |
//|                     check list header data                                 |
//|                     if not ok                                              |
//|                       issue error message                                  |
//|                       free list memory                                     |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   set caller's exclusion list pointer                      |
//|                   return return code                                       |
//+----------------------------------------------------------------------------+
USHORT LstReadNoiseExclList
(
   PSZ             pszListName,        // name of list to read
   PULONG          pulListSize,        // address of list size variable
   PEXCLUSIONLIST  *ppExclList         // address oif exclusion list pointer
)
{
  PEXCLUSIONLIST pExclList = NULL;     // pointer to loaded exclusion lists
  USHORT         usRC = NO_ERROR;      // function return code
  PSZ            pError;               // pointer to erraneous parameter
  ULONG          ulListSize = *pulListSize;
  ULONG          ulOemCP = 0L;

  ulOemCP = GetLangOEMCP(NULL);

  /********************************************************************/
  /* load exclusion list into memory                                  */
  /********************************************************************/
  if ( !UtlLoadFileL( pszListName, (PVOID *)&pExclList, &ulListSize, FALSE, TRUE ) )
  {
    usRC = ERROR_READ_FAULT;
    UtlAlloc( (PVOID *)&pExclList, 0L, 0L, NOMSG );
    pExclList = NULL;
  } /* endif */

  /********************************************************************/
  /* do a consistency check on the list                               */
  /********************************************************************/
  if ( !usRC )
  {
    PBYTE p;
    // check whether start is UNICODEFILEPREFIX!
    ULONG ulPrefLen = strlen(UNICODEFILEPREFIX);
    if ( memcmp( (PBYTE) pExclList, UNICODEFILEPREFIX, ulPrefLen) == 0)
    {
      // it is Unicode -- get rid of Unicode prefix
      memmove( (PBYTE)pExclList, (PBYTE)pExclList + ulPrefLen, ulListSize - ulPrefLen );
      ulListSize -= ulPrefLen;
      *pulListSize = ulListSize;
      if (  (pExclList->uLength != (*pulListSize) + ulPrefLen) ||
            (pExclList->usNumEntries >= (ulListSize / 2)) ||
            (pExclList->uFirstEntry > ulListSize) ||
            (pExclList->uStrings > ulListSize) )
      {
        usRC = ERROR_BAD_FORMAT;
        pError = UtlGetFnameFromPath( pszListName );
        UtlError( NO_VALID_FORMAT, MB_CANCEL, 1, &pError, EQF_ERROR );
        UtlAlloc( (PVOID *) &pExclList, 0L, 0L, NOMSG );
        pExclList = NULL;
      } /* endif */
      p = (PBYTE) pExclList;
      *(p + ulListSize ) = EOS;
      *(p + ulListSize+1) = EOS;
      *ppExclList = pExclList;
    }
    else
    {
      // it is our Old ASCII format - convert to Unicode
      PBYTE pExclListW = NULL;
      UtlAlloc( (PVOID *)&pExclListW, 0L, ulListSize * sizeof(CHAR_W), NOMSG );
      if (!pExclListW)
      {
        usRC = ERROR_READ_FAULT;
        *ppExclList = NULL;
      }
      else
      {
         /**************************************************************/
         /* Position to term offset area in noise / exclusion list     */
         /**************************************************************/
         USHORT usI;
         PSZ_W pszTarget = (PSZ_W)pExclListW;
         PSZ   pszTemp;
         PUSHORT pusTermInd = (PUSHORT) (((PSZ)pExclList) + pExclList->uFirstEntry);

         /**************************************************************/
         /* Loop through terms and add the terms to the term buffer.   */
         /* The terms are seperated using \0                           */
         /**************************************************************/
         for ( usI = 0; usI < pExclList->usNumEntries; usI++ )
         {
           pszTemp = (PSZ)pExclList + pExclList->uStrings + *pusTermInd;
           if ( *pszTemp )
           {
             ASCII2Unicode( pszTemp, pszTarget, ulOemCP);    // convert to Unicode
             pszTarget   += UTF16strlenCHAR(pszTarget) + 1;
           } /* endif */
           pusTermInd++;
         } /* endfor */
         *pszTarget = EOS;

         if ( LstWriteExclList( pszListName, (PSZ_W)pExclListW ) )
         {
           UtlAlloc( (PVOID *)&pExclListW, 0L, 0L, NOMSG );
           usRC = LstReadNoiseExclList( pszListName, pulListSize, ppExclList );
         }
         else
         {
           UtlAlloc( (PVOID *)&pExclListW, 0L, 0L, NOMSG );
           usRC = ERROR_WRITE_FAULT;
         }
      }
    }
  } /* endif */

  return( usRC );
} /* end of function LstReadNoiseExclList */


//+----------------------------------------------------------------------------+
//|                          End of EQFLSTUT.C                                 |
//+----------------------------------------------------------------------------+
