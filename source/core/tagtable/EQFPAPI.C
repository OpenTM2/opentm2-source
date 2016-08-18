/*                                                                            */
/*  Parser API functions                                                      */
/*                                                                            */
/*     Copyright (C) 1990-2014, International Business Machines               */
/*     Corporation and others. All rights reserved                            */
/*                                                                            */
/*============================================================================*/
//
//
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_TAGTABLE         // tag table defines
#define INCL_EQF_TP               // editor defines
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file
#include <OTMFUNC.H>              // nonDDE funtions (for error codes) and EQFPAPI.H

#define PARSE_ID_LEN    5
#define PARSEAPI_ID     "PAPI"
#define PARSESEGFILE_ID "PSGF"
#define PARSEMARKUP_ID  "PMUP"

// internal structure for parser API environment
typedef struct _PARSERAPI
{
  CHAR        szID[PARSE_ID_LEN];      // ID fo structure
  PVOID       pvFirstSegFile;          // ptr to loaded segmented files chain
  // for SGML statement creation
  PLOADEDTABLE pQFTagTable;            // loaded QF tag table
  char        szMarkAttr[50];          // buffer for preprocessed attribute
  char        szNoCountAttr[50];       // buffer for preprocessed attribute
  char        szCurrentAttr[50];       // buffer for preprocessed attribute
  char        szJoinAttr[50];          // buffer for preprocessed attribute
  char        szNAttr[50];             // buffer for preprocessed attribute
  char        szStatusAttr[50];        // buffer for preprocessed attribute
  char        szCountAttr[50];         // buffer for preprocessed attribute
  char        szSGMLSegment[5000];     // buffer for complete SGML segment statement
  PTAG        pTag;                    // ptr to tag info within tag table
  char       *pTagNames;               // ptr to tag names with tag table
  BOOL       fQFTagInfoLoaded;         // TRUE = tag info fields have been prepared
  char       szSegFileName[MAX_LONGPATH]; // buffer for segment file name
  char       szDocObjName[MAX_LONGPATH]; // buffer for document object name
  char       szSourceLang[MAX_LANG_LENGTH]; // buffer for document source language
  BOOL       fMorphActive;             // TRUE = morph functions have been activated
  SHORT      sLangID;                  // morph language ID
  PSZ        pTermList;                // pointer to term list
  USHORT     usTermListSize;           // current term list size
  WCHAR      szMarkAttrW[50];          // buffer for preprocessed attribute
  WCHAR      szNoCountAttrW[50];       // buffer for preprocessed attribute
  WCHAR      szCurrentAttrW[50];       // buffer for preprocessed attribute
  WCHAR      szJoinAttrW[50];          // buffer for preprocessed attribute
  WCHAR      szNAttrW[50];             // buffer for preprocessed attribute
  WCHAR      szStatusAttrW[50];        // buffer for preprocessed attribute
  WCHAR      szCountAttrW[50];         // buffer for preprocessed attribute
  WCHAR      szSGMLSegmentW[5000];     // buffer for complete SGML segment statement
  BOOL       fNonDDEMode;              // TRUE = api is used in nonDDE mode
  ULONG      hSession;                 // nonDDE session handle (if any)
} PARSERAPI, *PPARSERAPI;

// internal structure containing segmented file
typedef struct _PARSSEGFILE
{
  CHAR        szID[PARSE_ID_LEN];      // ID fo structure
  TBDOCUMENT  Document;                // loaded document
  PVOID       pvNextSegFile;           // ptr to next loaded segmented file in chain
  PPARSERAPI  pParserAPI;              // ptr to parser API structure
} PARSSEGFILE, *PPARSSEGFILE;

// internal structure containing markup table info
typedef struct _PARSERMARKUP
{
  CHAR             szID[PARSE_ID_LEN]; // ID of structure
  PPARSERAPI       pParsAPI;           // anchor of parser API structure
  PLOADEDTABLE     pTable;             // loaded markup table
  BOOL             fTokenAvailable;    // TRUE = tokens are available
  BOOL             fEndOfTokenList;    // TRUE = we are at the end of the token list
  USHORT           usTokBufferEntries; // current size of token buffer in number of entries
  PTOKENENTRY      pTokBuffer;         // ptr to token buffer
  PTOKENENTRY      pCurrentToken;      // ptr to current token
  PSZ              pData;              // ptr to data being tokenized
} PARSERMARKUP, *PPARSERMARKUP;


int ParsIntCheckSegFileHandle( HPARSSEGFILE hSegFile );
int ParsIntCheckAPIHandle( HPARSER hParser );
int ParsIntCheckMarkupHandle( HPARSMARKUP hMarkup );
int ParseUpdateLineCol( PSZ_W pszData, PLONG plLine, PLONG plColumn, PBOOL pfTrailingLF );



int  ParsLoadSegFile
(
  HPARSER     hParser,                 // parser API handle
  PSZ         pszFileName,             // fully qualified document filename
  HPARSSEGFILE *phSegFile              // buffer for returned segmented file handle
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;
  BOOL      fFolderLocked = FALSE;
  BOOL      fDocLocked = FALSE;
  OBJNAME   szFolObjName;
  OBJNAME   szDocObjName;

  // check parser API handle
  iRC = ParsIntCheckAPIHandle( hParser );

  // check if folder or document is locked and lock them
  if ( !iRC )
  {
    // get folder object name (get primary drive, copy segmented file name wo drive, cut off doc name)
    UtlQueryString( QST_PRIMARYDRIVE, szFolObjName, sizeof(szFolObjName) );
    strcpy( szFolObjName + 1, pszFileName + 1 );
    UtlSplitFnameFromPath( szFolObjName );          // cut off document name
    UtlSplitFnameFromPath( szFolObjName );          // cut off segdir

    // check if folder is locked, if not lock folder
    if ( QUERYSYMBOL( szFolObjName ) != -1 )
    {
      iRC = ERROR_FOLDER_LOCKED;
    }
    else
    {
      SETSYMBOL( szFolObjName );
      fFolderLocked = TRUE;
    } /* endif */

    // check if document is locked if not lock document
    if ( !iRC )
    {
      strcpy( szDocObjName, szFolObjName );
      szDocObjName[0] = pszFileName[0];
      strcat( szDocObjName, BACKSLASH_STR );
      strcat( szDocObjName, UtlGetFnameFromPath( pszFileName ) );
      if ( QUERYSYMBOL( szDocObjName ) != -1 )
      {
        iRC = ERROR_DOC_LOCKED;
      }
      else
      {
        SETSYMBOL( szDocObjName );
        fDocLocked = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  // allocate segmented file data area
  if ( !iRC )
  {
    if ( !UtlAlloc( (PVOID *)&pSegFile, 0L, (LONG) sizeof(PARSSEGFILE), NOMSG ) )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // load internal QF tag table
  if ( !iRC )
  {
    iRC = TALoadTagTable( DEFAULT_QFTAG_TABLE,
                          (PLOADEDTABLE *)&pSegFile->Document.pQFTagTable,
                          TRUE, FALSE );
  } /* endif */

  // load segmented document
  if ( !iRC )
  {
    iRC = EQFBFileReadExW( pszFileName, &pSegFile->Document, 0L );
  } /* endif */


  // cleanup in case of errors pr anchor our data area
  if ( iRC )
  {
    if ( pSegFile )
    {
      if ( pSegFile->Document.pQFTagTable )
      {
        TAFreeTagTable( (PLOADEDTABLE) pSegFile->Document.pQFTagTable );
      } /* endif */
      UtlAlloc( (PVOID *)&pSegFile, 0L, 0L, NOMSG );
      pSegFile = NULL;
    } /* endif */
  }
  else
  {
    PPARSERAPI pParsAPI = (PPARSERAPI)hParser;

    // set segmented file data area ID and anchor parser API area
    memcpy( pSegFile->szID, PARSESEGFILE_ID, PARSE_ID_LEN );
    pSegFile->pParserAPI = pParsAPI;

    // anchor our segmented file in parser API data area
    {
      pSegFile->pvNextSegFile = pParsAPI->pvFirstSegFile;
      pParsAPI->pvFirstSegFile = (PVOID)pSegFile;
    }
  } /* endif */

  // unlock folder and document
  if ( fDocLocked ) REMOVESYMBOL( szDocObjName );
  if ( fFolderLocked ) REMOVESYMBOL( szFolObjName );

  // return to caller
  *phSegFile = (HPARSSEGFILE)pSegFile;
  return( iRC );
} /* end of function ParsLoadSegFile */

// free a segmented file loaded into memory
int  ParsFreeSegFile
(
  HPARSSEGFILE hSegFile                // handle of loaded segmented file
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;
  PTBDOCUMENT     pDoc = NULL;
  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // free all memory associated with segmented file
  if ( !iRC )
  {
	 pDoc = &(pSegFile->Document);
	 if (pDoc) EQFBFreeDoc( &pDoc, EQFBFREEDOC_NODOCIDAFREE );
  } /* endif */

  // remove document from loaded documents chain
  if ( !iRC )
  {
    // remove our segfile from loaded files chain
    PPARSERAPI pParsAPI = pSegFile->pParserAPI;

    if ( pParsAPI->pvFirstSegFile == NULL )
    {
      // no document in chain ...
    }
    else if ( pParsAPI->pvFirstSegFile == (PVOID)pSegFile )
    {
      // our segfile is the first in the chain...
      pParsAPI->pvFirstSegFile = pSegFile->pvNextSegFile ;
    }
    else
    {
      // look for our segfile in the chain
      PPARSSEGFILE pTempSegFile = (PPARSSEGFILE)pParsAPI->pvFirstSegFile;

      while ( (pTempSegFile->pvNextSegFile != NULL) &&
              (pTempSegFile->pvNextSegFile != (PVOID)pSegFile) )
      {
        pTempSegFile = (PPARSSEGFILE)pTempSegFile->pvNextSegFile;
      } /* endwhile */
      if ( pTempSegFile->pvNextSegFile == (PVOID)pSegFile )
      {
        pTempSegFile->pvNextSegFile = pSegFile->pvNextSegFile;
      } /* endif */
    } /* endif */
  } /* endif */

  // free segmented file data area
  if ( !iRC )
  {
	UtlAlloc( (PVOID *)&pSegFile, 0L, 0L, NOMSG );
  } /* endif */

  return( iRC );
} /* end of function ParsFreeSegFile */

// write a segmented file loaded into memory to an external file
int  ParsWriteSegFile
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  char       *pszFileName              // fully qualified document filename
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;

  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // write segmented file
  if ( !iRC )
  {
    iRC = EQFBFileWrite( pszFileName, &pSegFile->Document );
  } /* endif */

  return( iRC );
} /* end of function ParsWriteSegFile */


// get the data of a specific segment
int  ParsGetSeg
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  LONG         lSegNum,                // number of segment to retrieve
  PPARSSEGMENT pSeg                    // ptr to buffer for segment data
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;

  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // check segment number
  if ( !iRC )
  {
    if ( (lSegNum < 0) || (lSegNum >= (LONG)pSegFile->Document.ulMaxSeg) )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // access segment and copy segment data
  if ( !iRC )
  {
    PTBSEGMENT pTBSeg = EQFBGetSeg( &pSegFile->Document, lSegNum );
    if ( pTBSeg != NULL )
    {
      switch ( pTBSeg->qStatus )
      {
        case QF_XLATED:  pSeg->Status = PARSSEG_XLATED; break;
        case QF_TOBE:    pSeg->Status = PARSSEG_TOBE;   break;
        case QF_NOP:     pSeg->Status = PARSSEG_NOP;    break;
        case QF_ATTR:    pSeg->Status = PARSSEG_ATTR;   break;
        default:         pSeg->Status = PARSSEG_NOP;    break;
      } /* endswitch */

      pSeg->SegFlags.Joined       = pTBSeg->SegFlags.Joined;
      pSeg->SegFlags.Marked       = pTBSeg->SegFlags.Marked;
      pSeg->SegFlags.Current      = pTBSeg->SegFlags.Current;
      pSeg->SegFlags.UnTrans      = pTBSeg->SegFlags.UnTrans;
      pSeg->SegFlags.Typed        = pTBSeg->SegFlags.Typed;
      pSeg->SegFlags.Copied       = pTBSeg->SegFlags.Copied;
      pSeg->SegFlags.Expanded     = pTBSeg->SegFlags.Expanded;
      pSeg->SegFlags.NoCount      = pTBSeg->SegFlags.NoCount;
      pSeg->SegFlags.InsertAdd    = pTBSeg->SegFlags.InsertAdd;
      pSeg->SegFlags.NoWrite      = pTBSeg->SegFlags.NoWrite;
      pSeg->SegFlags.Spellchecked = pTBSeg->SegFlags.Spellchecked;
      pSeg->SegFlags.Free1        = pTBSeg->SegFlags.NoReorder;
      pSeg->SegFlags.Free2        = pTBSeg->SegFlags.Dummy5;
      pSeg->SegFlags.Free3        = pTBSeg->SegFlags.NoContextAvailable;
      pSeg->SegFlags.Free4        = pTBSeg->SegFlags.Changed;


      *((PUSHORT)&(pSeg->CountFlags)) = *((PUSHORT)&(pTBSeg->CountFlag));

      pSeg->usSrcWords              = pTBSeg->usSrcWords;
      pSeg->usTgtWords              = pTBSeg->usTgtWords;
      pSeg->usModWords              = pTBSeg->usModWords;

      strcpy( pSeg->szData,           pTBSeg->pData );
      pSeg->usLength                = pTBSeg->usLength;
      pSeg->lSegNum                 = pTBSeg->ulSegNum;
    }
    else
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParseGetSeg */

// get the data of a specific segment (UTF16 version)
int  ParsGetSegW
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  LONG         lSegNum,                // number of segment to retrieve
  PPARSSEGMENTW pSeg                    // ptr to buffer for segment data
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;

  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // check segment number
  if ( !iRC )
  {
    if ( (lSegNum < 0) || (lSegNum >= (LONG)pSegFile->Document.ulMaxSeg) )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // access segment and copy segment data
  if ( !iRC )
  {
    PTBSEGMENT pTBSeg = EQFBGetSegW( &pSegFile->Document, lSegNum );
    memset( pSeg, 0, sizeof(PARSSEGMENTW) );
    if ( pTBSeg != NULL )
    {
      switch ( pTBSeg->qStatus )
      {
        case QF_XLATED:  pSeg->Status = PARSSEG_XLATED; break;
        case QF_TOBE:    pSeg->Status = PARSSEG_TOBE;   break;
        case QF_NOP:     pSeg->Status = PARSSEG_NOP;    break;
        case QF_ATTR:    pSeg->Status = PARSSEG_ATTR;   break;
        default:         pSeg->Status = PARSSEG_NOP;    break;
      } /* endswitch */

      pSeg->SegFlags.Joined       = pTBSeg->SegFlags.Joined;
      pSeg->SegFlags.Marked       = pTBSeg->SegFlags.Marked;
      pSeg->SegFlags.Current      = pTBSeg->SegFlags.Current;
      pSeg->SegFlags.UnTrans      = pTBSeg->SegFlags.UnTrans;
      pSeg->SegFlags.Typed        = pTBSeg->SegFlags.Typed;
      pSeg->SegFlags.Copied       = pTBSeg->SegFlags.Copied;
      pSeg->SegFlags.Expanded     = pTBSeg->SegFlags.Expanded;
      pSeg->SegFlags.NoCount      = pTBSeg->SegFlags.NoCount;
      pSeg->SegFlags.InsertAdd    = pTBSeg->SegFlags.InsertAdd;
      pSeg->SegFlags.NoWrite      = pTBSeg->SegFlags.NoWrite;
      pSeg->SegFlags.Spellchecked = pTBSeg->SegFlags.Spellchecked;
      pSeg->SegFlags.Free1        = pTBSeg->SegFlags.NoReorder;
      pSeg->SegFlags.Free2        = pTBSeg->SegFlags.Dummy5;
      pSeg->SegFlags.Free3        = pTBSeg->SegFlags.NoContextAvailable;
      pSeg->SegFlags.Free4        = pTBSeg->SegFlags.Changed;


      *((PUSHORT)&(pSeg->CountFlags)) = *((PUSHORT)&(pTBSeg->CountFlag));

      pSeg->usSrcWords              = pTBSeg->usSrcWords;
      pSeg->usTgtWords              = pTBSeg->usTgtWords;
      pSeg->usModWords              = pTBSeg->usModWords;

      UTF16strcpy( pSeg->szData,      pTBSeg->pDataW );
      pSeg->usLength                = pTBSeg->usLength;
      pSeg->lSegNum                 = pTBSeg->ulSegNum;
    }
    else
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParseGetSegW */


// update a segment of a segmented file loaded into memory
int  ParsUpdateSeg
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  long         lSegNum,                // number of segment to update
  PPARSSEGMENT pSeg                    // ptr new segment data
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;

  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // check segment number
  if ( !iRC )
  {
    if ( (lSegNum < 0) || (lSegNum >= (LONG)pSegFile->Document.ulMaxSeg) )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // access segment and update segment data
  if ( !iRC )
  {
    PTBSEGMENT pTBSeg = EQFBGetSeg( &pSegFile->Document, lSegNum );
    if ( pTBSeg != NULL )
    {
      switch ( pSeg->Status )
      {
        case PARSSEG_XLATED:  pTBSeg->qStatus = QF_XLATED; break;
        case PARSSEG_TOBE:    pTBSeg->qStatus = QF_TOBE;   break;
        case PARSSEG_NOP:     pTBSeg->qStatus = QF_NOP;    break;
        case PARSSEG_ATTR:    pTBSeg->qStatus = QF_ATTR;   break;
        default:              pTBSeg->qStatus = QF_NOP;    break;
      } /* endswitch */

      pTBSeg->SegFlags.Joined       = pSeg->SegFlags.Joined;
      pTBSeg->SegFlags.Marked       = pSeg->SegFlags.Marked;
      pTBSeg->SegFlags.Current      = pSeg->SegFlags.Current;
      pTBSeg->SegFlags.UnTrans      = pSeg->SegFlags.UnTrans;
      pTBSeg->SegFlags.Typed        = pSeg->SegFlags.Typed;
      pTBSeg->SegFlags.Copied       = pSeg->SegFlags.Copied;
      pTBSeg->SegFlags.Expanded     = pSeg->SegFlags.Expanded;
      pTBSeg->SegFlags.NoCount      = pSeg->SegFlags.NoCount;
      pTBSeg->SegFlags.InsertAdd    = pSeg->SegFlags.InsertAdd;
      pTBSeg->SegFlags.NoWrite      = pSeg->SegFlags.NoWrite;
      pTBSeg->SegFlags.Spellchecked = pSeg->SegFlags.Spellchecked;
      pTBSeg->SegFlags.NoReorder    = pSeg->SegFlags.Free1;
      pTBSeg->SegFlags.Dummy5       = pSeg->SegFlags.Free2;
      pTBSeg->SegFlags.NoContextAvailable = pSeg->SegFlags.Free3;
      pTBSeg->SegFlags.Changed      = pSeg->SegFlags.Free4;


      *((PUSHORT)&(pTBSeg->CountFlag)) = *((PUSHORT)&(pSeg->CountFlags));

      pTBSeg->usSrcWords              = pSeg->usSrcWords;
      pTBSeg->usTgtWords              = pSeg->usTgtWords;
      pTBSeg->usModWords              = pSeg->usModWords;

      // replace ASCII segment data with new value
      pSeg->usLength = (USHORT)strlen( pSeg->szData );
      if ( pTBSeg->pData ) UtlAlloc( (PVOID *)&(pTBSeg->pData), 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *)&(pTBSeg->pData), 0L, max( MIN_ALLOC, pSeg->usLength), NOMSG );
      strcpy( pTBSeg->pData,            pSeg->szData );
      pTBSeg->usLength                = pSeg->usLength;

      // replace UTF16 segment data with new value
      if ( pTBSeg->pDataW ) UtlAlloc( (PVOID *)&(pTBSeg->pDataW), 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *)&(pTBSeg->pDataW), 0L, max( MIN_ALLOC, (2 * pSeg->usLength + 10)), NOMSG );
      ASCII2Unicode( pSeg->szData, pTBSeg->pDataW, pSegFile->Document.ulOemCodePage );

      pTBSeg->ulSegNum                = pSeg->lSegNum;
    }
    else
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParseUpdateSeg */

// update a segment of a segmented file loaded into memory (UTF16 version)
int  ParsUpdateSegW
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  long         lSegNum,                // number of segment to update
  PPARSSEGMENTW pSeg                   // ptr new segment data
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;

  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // check segment number
  if ( !iRC )
  {
    if ( (lSegNum < 0) || (lSegNum >= (LONG)pSegFile->Document.ulMaxSeg) )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // access segment and update segment data
  if ( !iRC )
  {
    PTBSEGMENT pTBSeg = EQFBGetSegW( &pSegFile->Document, lSegNum );
    if ( pTBSeg != NULL )
    {
      switch ( pSeg->Status )
      {
        case PARSSEG_XLATED:  pTBSeg->qStatus = QF_XLATED; break;
        case PARSSEG_TOBE:    pTBSeg->qStatus = QF_TOBE;   break;
        case PARSSEG_NOP:     pTBSeg->qStatus = QF_NOP;    break;
        case PARSSEG_ATTR:    pTBSeg->qStatus = QF_ATTR;   break;
        default:              pTBSeg->qStatus = QF_NOP;    break;
      } /* endswitch */

      pSeg->usLength = (USHORT)UTF16strlenCHAR( pSeg->szData );

      pTBSeg->SegFlags.Joined       = pSeg->SegFlags.Joined;
      pTBSeg->SegFlags.Marked       = pSeg->SegFlags.Marked;
      pTBSeg->SegFlags.Current      = pSeg->SegFlags.Current;
      pTBSeg->SegFlags.UnTrans      = pSeg->SegFlags.UnTrans;
      pTBSeg->SegFlags.Typed        = pSeg->SegFlags.Typed;
      pTBSeg->SegFlags.Copied       = pSeg->SegFlags.Copied;
      pTBSeg->SegFlags.Expanded     = pSeg->SegFlags.Expanded;
      pTBSeg->SegFlags.NoCount      = pSeg->SegFlags.NoCount;
      pTBSeg->SegFlags.InsertAdd    = pSeg->SegFlags.InsertAdd;
      pTBSeg->SegFlags.NoWrite      = pSeg->SegFlags.NoWrite;
      pTBSeg->SegFlags.Spellchecked = pSeg->SegFlags.Spellchecked;
      pTBSeg->SegFlags.NoReorder    = pSeg->SegFlags.Free1;
      pTBSeg->SegFlags.Dummy5       = pSeg->SegFlags.Free2;
      pTBSeg->SegFlags.NoContextAvailable = pSeg->SegFlags.Free3;
      pTBSeg->SegFlags.Changed      = pSeg->SegFlags.Free4;


      *((PUSHORT)&(pTBSeg->CountFlag)) = *((PUSHORT)&(pSeg->CountFlags));

      pTBSeg->usSrcWords              = pSeg->usSrcWords;
      pTBSeg->usTgtWords              = pSeg->usTgtWords;
      pTBSeg->usModWords              = pSeg->usModWords;

      UtlAlloc( (PVOID *)&(pTBSeg->pDataW ), 0L, 0L, NOMSG );

      UtlAlloc( (PVOID *)&(pTBSeg->pDataW), 0L, max( MIN_ALLOC, (pSeg->usLength*2+10) ), NOMSG );

      UTF16strcpy( pTBSeg->pDataW,      pSeg->szData );

      pTBSeg->usLength                = pSeg->usLength;
      pTBSeg->ulSegNum                = pSeg->lSegNum;
    }
    else
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParseUpdateSegW */




// terminate parser API environment
int ParsTerminate
(
  HPARSER hParser                      // parser API handle returned by ParsInitialize
)
{
  int iRC = NO_ERROR;
  PPARSERAPI pParsAPI = (PPARSERAPI)hParser;

  iRC = ParsIntCheckAPIHandle( hParser );

  // free resources used
  if ( !iRC )
  {
    if ( pParsAPI->pQFTagTable )
    {
      TAFreeTagTable( pParsAPI->pQFTagTable );
    } /* endif */

    while ( pParsAPI->pvFirstSegFile != NULL)
    {
      ParsFreeSegFile( (HPARSSEGFILE)pParsAPI->pvFirstSegFile );
    } /* endwhile */

    if ( pParsAPI->fMorphActive )
    {
      MorphFreeLanguageID( pParsAPI->sLangID );
    } /* endif */

    if ( pParsAPI->pTermList )
    {
      UtlAlloc( (PVOID *) &pParsAPI->pTermList, 0L, 0L, NOMSG );
    } /* endif */

  } /* endif */

  // free parser API area
  if ( !iRC )
  {
    UtlAlloc( (PVOID *)&pParsAPI, 0L, 0L, NOMSG );
  } /* endif */

  return( iRC );
} /* end of function ParsTerminate */

// initialize parser API environment
int ParsInitialize
(
  HPARSER *phParser,                   // ptr to buffer for parser API handle
  char    *pszDocPath                  // path to document source or target seg file
)
{
  int iRC = NO_ERROR;
  PPARSERAPI pParsAPI = NULL;

  if ( !UtlAlloc( (PVOID *)&pParsAPI, 0L, sizeof(PARSERAPI), NOMSG ) )
  {
    iRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // check document path name
  if ( !iRC )
  {
    if ( pszDocPath == NULL )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // fill our fields
  if ( !iRC )
  {
    // store ID
    memcpy( pParsAPI->szID, PARSEAPI_ID, PARSE_ID_LEN );

    if ( strcmp( pszDocPath, "<NONDDE>") == 0 )
    {
      pParsAPI->fNonDDEMode = TRUE;
    }
    else
    {
      // store original segment file name

      strcpy( pParsAPI->szSegFileName, pszDocPath );

      // build document object path by removing the directory name
      strcpy( pParsAPI->szDocObjName, pszDocPath );
      UtlSplitFnameFromPath( pParsAPI->szDocObjName );
      UtlSplitFnameFromPath( pParsAPI->szDocObjName );
      strcat( pParsAPI->szDocObjName, BACKSLASH_STR );
      strcat( pParsAPI->szDocObjName, UtlGetFnameFromPath( pszDocPath ) );
    } /* endif */
  } /* endif */

  *phParser = (HPARSER)pParsAPI;
  return( iRC );
} /* end of function ParsInitialize*/


// make a SGML tagged segment as used in the segmented files
int ParsMakeSGMLSegment
(
  HPARSER     hParser,                 // parser API handle
  PPARSSEGMENT pSeg,                   // ptr to segment data
  char        *pszBuffer,              // buffer for SGML tagged segment
                                       // (should be at least twice the max segment size)
  int         iBufferSize,             // size of buffer for SGML tagged segment
  BOOL        fSourceFile              // TRUE = create SGML for a segmented source file
                                       // FALSE = create SGML for a segmented target file
)
{
  int iRC = NO_ERROR;
  PPARSERAPI pParsAPI = (PPARSERAPI)hParser;

  iRC = ParsIntCheckAPIHandle( hParser );

  // load QF tag information if required
  if ( !iRC && !pParsAPI->fQFTagInfoLoaded )
  {
    iRC = TALoadTagTable( DEFAULT_QFTAG_TABLE, &pParsAPI->pQFTagTable, TRUE, FALSE );
    if ( !iRC )
    {
      PBYTE pByte;                     // helper pointer
      PTAGTABLE pTagTable;             // pointer to active QF tag table

      EQFBFillWriteAttr( (PVOID)pParsAPI->pQFTagTable, pParsAPI->szMarkAttr,
                         pParsAPI->szNoCountAttr, pParsAPI->szCurrentAttr,
                         pParsAPI->szJoinAttr, pParsAPI->szNAttr, pParsAPI->szStatusAttr,
                         pParsAPI->szCountAttr );

       pTagTable = pParsAPI->pQFTagTable->pTagTable;
       pByte = (PBYTE) (PVOID)pTagTable;
       pParsAPI->pTag = (PTAG)(pByte + pTagTable->stFixTag.uOffset);
       pParsAPI->pTagNames = (PSZ)(pByte + pTagTable->uTagNames);
       pParsAPI->fQFTagInfoLoaded = TRUE;
    } /* endif */
  } /* endif */

  // create SGML segment statement
  if ( !iRC )
  {
    SHORT sToken;
    PSZ   pszTag;
    PSZ   pszPos = pParsAPI->szSGMLSegment;

    switch ( pSeg->Status )
    {
      case PARSSEG_TOBE:    sToken = QFF_TAG;  break;
      case PARSSEG_NOP:     sToken = QFN_TAG;  break;
      case PARSSEG_XLATED:  sToken = QFX_TAG;  break;
      case PARSSEG_ATTR:    sToken = QFA_TAG;  break;
      default:              sToken = QFN_TAG;  break;
    } /* endswitch */
    pszTag = pParsAPI->pTag[sToken].uTagnameOffs + pParsAPI->pTagNames;

    // add start tag
    strcpy( pszPos, pszTag );
    pszPos += strlen( pszPos );

    // add segment number attribute
    sprintf( pszPos, pParsAPI->szNAttr, pSeg->lSegNum );
    pszPos += strlen( pszPos );

    // add join state attribute
    if ( (pSeg->SegFlags.JoinStart || pSeg->SegFlags.Joined) )
    {
      *pszPos++ = ' ';
      sprintf( pszPos, pParsAPI->szJoinAttr, (pSeg->SegFlags.JoinStart) ? 1 : 2 );
      pszPos += strlen( pszPos );
    } /* endif */

    // add mark attribute
    if ( pSeg->SegFlags.Marked )
    {
      *pszPos++ = ' ';
      sprintf( pszPos, pParsAPI->szMarkAttr );
      pszPos += strlen( pszPos );
    } /* endif */

    // add current attribute
    if ( pSeg->SegFlags.Current )
    {
      *pszPos++ = ' ';
      sprintf( pszPos, pParsAPI->szCurrentAttr );
      pszPos += strlen( pszPos );
    } /* endif */

    // add mark attribute
    if ( pSeg->SegFlags.NoCount )
    {
      *pszPos++ = ' ';
      sprintf( pszPos, pParsAPI->szNoCountAttr );
      pszPos += strlen( pszPos );
    } /* endif */

    // add Status state attribute
    /**********************************************************/
    /* if Typed = TRUE and COpied = TRUE -> szStatusAttr =2  */
    /* if Typed = TRUE and Copied =FALSE -> szStatusAttr =1  */
    /* if Typed =FALSE and Copied = TRUE -> szStatusAttr =3  */
    /**********************************************************/
    if ( pSeg->SegFlags.Typed || pSeg->SegFlags.Copied  )
    {
      *pszPos++ = ' ';
      if ( pSeg->SegFlags.Typed )
      {
        if ( pSeg->SegFlags.Copied )
        {
          sprintf( pszPos, pParsAPI->szStatusAttr, 2 );
        }
        else
        {
          sprintf( pszPos, pParsAPI->szStatusAttr, 1 );
        } /* endif */
      }
      else
      {
        sprintf( pszPos, pParsAPI->szStatusAttr, 3 );
      } /* endif */
      pszPos += strlen( pszPos );
    } /* endif */

    // add count attribute
    if ( !fSourceFile )
    {
       if ( (*((PUSHORT)(&pSeg->CountFlags)) == 0) &&
            (pSeg->usSrcWords == 0) &&
            (pSeg->usTgtWords == 0) &&
            (pSeg->usModWords == 0) )
       {
         // nothing to do, count data is empty
       }
       else
       {
         USHORT usCheckSum = EQFBBuildCountCheckSum( *((PUSHORT)(&pSeg->CountFlags)),
           pSeg->usSrcWords, pSeg->usTgtWords, pSeg->usModWords );

         *pszPos++ = ' ';
         sprintf( pszPos, pParsAPI->szCountAttr,
                  *((PUSHORT)(&pSeg->CountFlags)),
                  pSeg->usSrcWords,
                  pSeg->usTgtWords,
                  pSeg->usModWords,
                  usCheckSum );
         pszPos += strlen( pszPos );
       } /* endif */
    } /* endif */

    // add tag end character
    *pszPos++ = '.';

    // add segment data
    strcpy( pszPos, pSeg->szData );
    pszPos += strlen( pszPos );

    // write end tag to file
    switch ( pSeg->Status )
    {
      case PARSSEG_TOBE:    sToken = EQFF_TAG;  break;
      case PARSSEG_NOP:     sToken = EQFN_TAG;  break;
      case PARSSEG_XLATED:  sToken = EQFX_TAG;  break;
      case PARSSEG_ATTR:    sToken = EQFA_TAG;  break;
      default:              sToken = EQFN_TAG;  break;
    } /* endswitch */
    pszTag = pParsAPI->pTag[sToken].uTagnameOffs + pParsAPI->pTagNames;
    sprintf( pszPos, "%s.", pszTag );
    pszPos += strlen( pszPos );
  } /* endif */

  // check if callers buffer is large enough
  if ( !iRC )
  {
    if ( iBufferSize > (int)strlen(pParsAPI->szSGMLSegment) )
    {
      strcpy( pszBuffer, pParsAPI->szSGMLSegment );
    }
    else
    {
      iRC = ERROR_BAD_LENGTH;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParsMakeSGMLSegment */


int ParsMakeSGMLSegmentW
(
  HPARSER     hParser,                 // parser API handle
  PPARSSEGMENTW pSeg,                   // ptr to segment data
  WCHAR       *pszBuffer,              // buffer for SGML tagged segment
                                       // (should be at least twice the max segment size)
  int         iBufferSize,             // size of buffer for SGML tagged segment
  BOOL        fSourceFile              // TRUE = create SGML for a segmented source file
                                       // FALSE = create SGML for a segmented target file
)
{
  int iRC = NO_ERROR;
  PPARSERAPI pParsAPI = (PPARSERAPI)hParser;

  iRC = ParsIntCheckAPIHandle( hParser );

  // load QF tag information if required
  if ( !iRC && !pParsAPI->fQFTagInfoLoaded )
  {
    iRC = TALoadTagTable( DEFAULT_QFTAG_TABLE, &pParsAPI->pQFTagTable, TRUE, FALSE );
    if ( !iRC )
    {
      PBYTE pByte;                     // helper pointer
      PTAGTABLE pTagTable;             // pointer to active QF tag table

      EQFBFillWriteAttrW( (PVOID)pParsAPI->pQFTagTable, pParsAPI->szMarkAttrW,
                         pParsAPI->szNoCountAttrW, pParsAPI->szCurrentAttrW,
                         pParsAPI->szJoinAttrW, pParsAPI->szNAttrW, pParsAPI->szStatusAttrW,
                         pParsAPI->szCountAttrW );

       pTagTable = pParsAPI->pQFTagTable->pTagTable;
       pByte = (PBYTE) (PVOID)pTagTable;
       pParsAPI->pTag = (PTAG)(pByte + pTagTable->stFixTag.uOffset);
       pParsAPI->pTagNames = (PSZ)(pByte + pTagTable->uTagNames);
       pParsAPI->fQFTagInfoLoaded = TRUE;
    } /* endif */
  } /* endif */

  // create SGML segment statement
  if ( !iRC )
  {
    SHORT sToken;
    PSZ   pszTag;
    WCHAR *pszPos = pParsAPI->szSGMLSegmentW;

    switch ( pSeg->Status )
    {
      case PARSSEG_TOBE:    sToken = QFF_TAG;  break;
      case PARSSEG_NOP:     sToken = QFN_TAG;  break;
      case PARSSEG_XLATED:  sToken = QFX_TAG;  break;
      case PARSSEG_ATTR:    sToken = QFA_TAG;  break;
      default:              sToken = QFN_TAG;  break;
    } /* endswitch */
    pszTag = pParsAPI->pTag[sToken].uTagnameOffs + pParsAPI->pTagNames;

    // add start tag
    ASCII2Unicode( pszTag, pszPos, 0L );
    pszPos += UTF16strlenCHAR( pszPos );


    // add segment number attribute
    *pszPos++ = BLANK;
    swprintf( pszPos, pParsAPI->szNAttrW, pSeg->lSegNum );
    pszPos += UTF16strlenCHAR( pszPos );

    // add join state attribute
    if ( (pSeg->SegFlags.JoinStart || pSeg->SegFlags.Joined) )
    {
      *pszPos++ = BLANK;
      swprintf( pszPos, pParsAPI->szJoinAttrW, (pSeg->SegFlags.JoinStart) ? 1 : 2 );
      pszPos += UTF16strlenCHAR( pszPos );
    } /* endif */

    // add mark attribute
    if ( pSeg->SegFlags.Marked )
    {
      *pszPos++ = BLANK;
      swprintf( pszPos, pParsAPI->szMarkAttrW, 0 );
      pszPos += UTF16strlenCHAR( pszPos );
    } /* endif */

    // add current attribute
    if ( pSeg->SegFlags.Current )
    {
      *pszPos++ = BLANK;
      swprintf( pszPos, pParsAPI->szCurrentAttrW, 0 );
      pszPos += UTF16strlenCHAR( pszPos );
    } /* endif */

    // add mark attribute
    if ( pSeg->SegFlags.NoCount )
    {
      *pszPos++ = BLANK;
      swprintf( pszPos, pParsAPI->szNoCountAttrW, 0 );
      pszPos += UTF16strlenCHAR( pszPos );
    } /* endif */

    // add Status state attribute
    /**********************************************************/
    /* if Typed = TRUE and COpied = TRUE -> szStatusAttr =2  */
    /* if Typed = TRUE and Copied =FALSE -> szStatusAttr =1  */
    /* if Typed =FALSE and Copied = TRUE -> szStatusAttr =3  */
    /**********************************************************/
    if ( pSeg->SegFlags.Typed || pSeg->SegFlags.Copied  )
    {
      *pszPos++ = BLANK;
      if ( pSeg->SegFlags.Typed )
      {
        if ( pSeg->SegFlags.Copied )
        {
          swprintf( pszPos, pParsAPI->szStatusAttrW, 2 );
        }
        else
        {
          swprintf( pszPos, pParsAPI->szStatusAttrW, 1 );
        } /* endif */
      }
      else
      {
        swprintf( pszPos, pParsAPI->szStatusAttrW, 3 );
      } /* endif */
      pszPos += UTF16strlenCHAR( pszPos );
    } /* endif */

    // add count attribute
    if ( !fSourceFile )
    {
       if ( (*((PUSHORT)(&pSeg->CountFlags)) == 0) &&
            (pSeg->usSrcWords == 0) &&
            (pSeg->usTgtWords == 0) &&
            (pSeg->usModWords == 0) )
       {
         // nothing to do, count data is empty
       }
       else
       {
         USHORT usCheckSum = EQFBBuildCountCheckSum( *((PUSHORT)(&pSeg->CountFlags)),
           pSeg->usSrcWords, pSeg->usTgtWords, pSeg->usModWords );
        *pszPos++ = BLANK;
         swprintf( pszPos, pParsAPI->szCountAttrW,
                  *((PUSHORT)(&pSeg->CountFlags)),
                  pSeg->usSrcWords,
                  pSeg->usTgtWords,
                  pSeg->usModWords,
                  usCheckSum );
        pszPos += UTF16strlenCHAR( pszPos );
       } /* endif */
    } /* endif */

    // add tag end character
    UTF16strcpy( pszPos, L"." );
    pszPos += UTF16strlenCHAR( pszPos );

    // add segment data
    UTF16strcpy( pszPos, pSeg->szData );
    pszPos += UTF16strlenCHAR( pszPos );

    // write end tag to file
    switch ( pSeg->Status )
    {
      case PARSSEG_TOBE:    sToken = EQFF_TAG;  break;
      case PARSSEG_NOP:     sToken = EQFN_TAG;  break;
      case PARSSEG_XLATED:  sToken = EQFX_TAG;  break;
      case PARSSEG_ATTR:    sToken = EQFA_TAG;  break;
      default:              sToken = EQFN_TAG;  break;
    } /* endswitch */
    ASCII2Unicode( (PSZ)(pParsAPI->pTag[sToken].uTagnameOffs + pParsAPI->pTagNames),
                  pszPos, 0L );
    pszPos += UTF16strlenCHAR( pszPos );
    UTF16strcpy( pszPos, L"." );
    pszPos += UTF16strlenCHAR( pszPos );
  } /* endif */

  // check if callers buffer is large enough
  if ( !iRC )
  {
    if ( iBufferSize > (int)UTF16strlenBYTE(pParsAPI->szSGMLSegmentW) )
    {
      UTF16strcpy( pszBuffer, pParsAPI->szSGMLSegmentW );
    }
    else
    {
      iRC = ERROR_BAD_LENGTH;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParsMakeSGMLSegmentW */

// inplace conversion of data
int ParsConvert
(
  HPARSER     hParser,                 // parser API handle
  PARSCONVERSION Conversion,           // conversion mode
  char       *pszData,                 // ptr to data eing converted
  unsigned short usLen                 // length of data being converted
)
{
  int iRC = NO_ERROR;
  // get system preferences language and call ParsConvert2 with it ( simply use NULL as lang.)
    iRC = ParsConvert2(hParser, Conversion, pszData, usLen, NULL);
    return( iRC );
} /* end of function ParsConvert */


// inplace conversion of data
int ParsConvert2
(
  HPARSER     hParser,                 // parser API handle
  PARSCONVERSION Conversion,           // conversion mode
  char       *pszData,                 // ptr to data eing converted
  unsigned short usLen,                 // length of data being converted
  char       *pszLanguage               // language of input data
)
{
  int iRC = NO_ERROR;
  PUCHAR  pConvTable = NULL;
  ULONG   ulOemCP = 0L;
  ULONG   ulAnsiCP = 0L;

  ulOemCP = GetLangOEMCP(pszLanguage);
  ulAnsiCP = GetLangAnsiCP(pszLanguage);

  iRC = ParsIntCheckAPIHandle( hParser );

  // get conversion table
  if ( !iRC )
  {
    switch ( Conversion )
    {
      case ANSItoUTF16:
      case ASCIItoUTF16:
      case UTF16toASCII:
      case UTF16toANSI:
        {
          PSZ pBuffer = NULL;

          // make copy of input data
          if ( UtlAlloc( (PVOID *)&pBuffer, 0L, (usLen + 20), NOMSG ) )
          {
            memcpy( pBuffer, pszData, usLen );
          }
          else
          {
            iRC = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */

          // convert data
          switch ( Conversion )
          {
            case ASCIItoUTF16:
              ASCII2UnicodeBuf( pBuffer, (PSZ_W) pszData, usLen, ulOemCP); /*, usLen + 10 );*/

              break;
            case ANSItoUTF16:
             {
				LONG lBytesLeft = 0L;
				LONG lRc = NO_ERROR;
//              Ansi2UnicodeBuf( pBuffer, (PUSHORT) pszData, usLen, ulOemCP); /*, usLen + 10 );*/
                UtlDirectAnsi2UnicodeBuf( pBuffer, (PSZ_W)pszData,
                                                usLen, ulAnsiCP, FALSE, &lRc,
                                                &lBytesLeft );

		 	 }
              break;
            case UTF16toASCII:
              Unicode2ASCIIBuf( (PSZ_W)pBuffer, pszData, (USHORT)( usLen / 2), (LONG)usLen, ulOemCP );

              break;
            case UTF16toANSI:
              { ULONG ulLen = 0L;
                LONG  lRc = NO_ERROR;
                ulLen = UtlDirectUnicode2AnsiBuf( (PSZ_W)pBuffer, pszData, (USHORT) (usLen / 2),
                                               (LONG)usLen, ulAnsiCP,
                                               FALSE, &lRc );
                iRC = (int) lRc;
		      }

              break;
          } /* endswitch */

          // cleanup
          if ( pBuffer ) UtlAlloc( (PVOID *)&pBuffer, 0L, 0L, NOMSG );

        }
        break;

      case ASCIItoANSI:
        UtlQueryCharTableLang( ASCII_TO_ANSI_TABLE, &pConvTable, pszLanguage );
        if ( pConvTable )
        {
          int i;
          unsigned char *pData = (PUCHAR)pszData;
          for ( i = 0; i < usLen; i++ )
          {
            *pData = pConvTable[*pData];
            pData++;
          } /* endfor */
        } /* endif */
        break;

      case ANSItoASCII:
        UtlQueryCharTableLang( ANSI_TO_ASCII_TABLE, &pConvTable, pszLanguage );
        if ( pConvTable )
        {
          int i;
          unsigned char *pData = (PUCHAR)pszData;
          for ( i = 0; i < usLen; i++ )
          {
            *pData = pConvTable[*pData];
            pData++;
          } /* endfor */
        } /* endif */
        break;

      default:          iRC = ERROR_BAD_FORMAT; break;
    } /* endswitch */
  } /* endif */

  return( iRC );
} /* end of function ParsConvert */


int ParsBuildTempName( char *pszSourceName, char *pszTempName )
{
  int iRC = NO_ERROR;

  PSZ pszName, pszExt;

  // copy source name to temp name and look for extention
  strcpy( pszTempName, pszSourceName );
  pszName = strrchr( pszTempName, '\\' );
  pszName = (pszName != NULL) ? (pszName + 1) : pszTempName;
  pszExt = strrchr( pszName, '.' );
  pszExt  = (pszExt != NULL) ? (pszExt) : (pszName + strlen(pszName));

  // append maybe extension
  if ( *pszExt == '.' )
  {
    // set pointer to next character following period
    pszExt++;
  }
  else
  {
    // no extension so far, add a period and set pointer right behind it
    *pszExt++ = '.';
    *pszExt = '\0';
  } /* endif */

  if ( strcmp( pszExt, "$$$" ) == 0 )
  {
    strcpy( pszExt, "___" );
  }
  else
  {
    strcpy( pszExt, "$$$" );
  } /* endif */

  return( iRC );
} /* end of function ParsBuildTempName */

// split data into segments using the morphologic functions
// The functions looks for segment breaks in the supplied data applying the
// morphology for the document source language. The segment breaks are
// returned in form of an segment break list. The segment break list is the
// list of the offsets of segbreaks within the data. The last element of the
// list is zero. If the buffer for the list is too small the return code
// ERROR_BAD_LENGTH is returned and the the first element of the segbreak list
// contains the required size of the list (in number of elements)
int ParsSplitSeg
(
  HPARSER     hParser,                 // parser API handle
  char       *pszData,                 // text data being splitted into segments
  unsigned short usDataLength,         // length of text data
  unsigned short *pusSegBreaks,        // ptr to buffer which receives the segbreaks
  unsigned short usElements            // size of segbreak buffer in number of elements
)
{
  // function is not supported anymore
  return( ERROR_INVALID_DATA );
} /* end of function ParsSplitSeg */

int ParsSplitSegW
(
  HPARSER     hParser,                 // parser API handle
  WCHAR       *pszData,                // text data being splitted into segments
  unsigned short usDataLength,         // length of text data
  unsigned short *pusSegBreaks,        // ptr to buffer which receives the segbreaks
  unsigned short usElements            // size of segbreak buffer in number of elements
)
{
  ULONG   ulOemCP = 0L;
  int iRC = NO_ERROR;
  PPARSERAPI pParsAPI = (PPARSERAPI)hParser;
  WCHAR *pszBuffer = NULL;

  iRC = ParsIntCheckAPIHandle( hParser );

  // check parameters
  if ( !iRC )
  {
    if ( (pszData == NULL) || (pusSegBreaks == NULL) || (usElements == 0) )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // immediate return if no data to tokenize
  if ( !iRC )
  {
    if ( usDataLength == 0 )
    {
      *pusSegBreaks = 0;
      return( NO_ERROR );
    } /* endif */
  } /* endif */

  // activate morphologic functions (if not done yet)
  if ( !iRC && !pParsAPI->fMorphActive )
  {
    // get document source language
    pParsAPI->szSourceLang[0] = EOS;
    iRC = (int)DocQueryInfo( pParsAPI->szDocObjName, NULL, NULL,
                             pParsAPI->szSourceLang, NULL, FALSE );

    // activate morphologic functions
    if ( iRC == NO_ERROR )
    {
      iRC = MorphGetLanguageID( pParsAPI->szSourceLang, &pParsAPI->sLangID );
      ulOemCP = GetLangOEMCP( pParsAPI->szSourceLang);
    } /* endif */

    if ( iRC == NO_ERROR )
    {
      pParsAPI->fMorphActive = TRUE;
    } /* endif */
  } /* endif */

  // copy data to temporary buffer
  if ( !iRC )
  {
    LONG lAllocLength = max( (LONG)(2 * usDataLength+1), MIN_ALLOC );

    if ( UtlAlloc( (PVOID *)&pszBuffer, 0L, lAllocLength, NOMSG ) )
    {
      memcpy( pszBuffer, pszData, usDataLength );
      pszBuffer[usDataLength] = 0;
    }
    else
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // split data into segments
  if ( !iRC )
  {
    iRC = (int)MorphTokenizeW( pParsAPI->sLangID, pszBuffer,
                               &(pParsAPI->usTermListSize),
                               (PVOID *)&(pParsAPI->pTermList),
                               MORPH_FLAG_OFFSLIST, ulOemCP );

    // count number of entries in returned list and check size of target buffer
    if ( iRC == NO_ERROR )
    {
      USHORT usBreaks = 0;
      PFLAGOFFSLIST pTerm = (PFLAGOFFSLIST)pParsAPI->pTermList;

      while ( (pTerm->usOffs != 0) || (pTerm->usLen  != 0) || (pTerm->lFlags != 0L ) )
      {
        if ( (pTerm->lFlags & TF_NEWSENTENCE) && (pTerm->usOffs != 0) )
        {
          usBreaks++;
        } /* endif */
        pTerm++;
      } /* endwhile */

      if ( (usBreaks + 1) > usElements )
      {
        // set error condition and number of required entries
        iRC = ERROR_BAD_LENGTH;
        *pusSegBreaks = usBreaks + 1;
      } /* endif */
    } /* endif */

    // fill caller's segment break table
    if ( iRC == NO_ERROR )
    {
      PFLAGOFFSLIST pTerm = (PFLAGOFFSLIST)pParsAPI->pTermList;

      while ( (pTerm->usOffs != 0) || (pTerm->usLen  != 0) || (pTerm->lFlags != 0L ) )
      {
        if ( (pTerm->lFlags & TF_NEWSENTENCE) && (pTerm->usOffs != 0) )
        {
          *pusSegBreaks++ = pTerm->usOffs;
        } /* endif */
        pTerm++;
      } /* endwhile */
      *pusSegBreaks = 0;
    } /* endif */
  } /* endif */

  if ( pszBuffer ) UtlAlloc( (PVOID *)&pszBuffer, 0L, 0L, NOMSG );

  return( iRC );
} /* end of function ParsSplitSegW */



// get long document name for current document
int ParsGetDocName
(
  HPARSER     hParser,                 // parser API handle
  char       *pszDocName               // ptr to buffer for document name (should
                                       // be 256 bytes in size)
)
{
  int iRC = NO_ERROR;
  PPARSERAPI pParsAPI = (PPARSERAPI)hParser;

  iRC = ParsIntCheckAPIHandle( hParser );

  if ( !iRC )
  {
    if ( pszDocName == NULL )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  if ( !iRC )
  {
    // get document long name (if any)
    *pszDocName = EOS;
    DocQueryInfo2( pParsAPI->szDocObjName, NULL, NULL, NULL, NULL,
                   pszDocName, NULL, NULL, FALSE );

    // use document short name if no long name is available
    if ( *pszDocName == EOS )
    {
      strcpy( pszDocName, UtlGetFnameFromPath( pParsAPI->szSegFileName ) );
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParsGetDocName */

// get language settings for current document
int ParsGetDocLang
(
  HPARSER     hParser,                 // parser API handle
  char       *pszSourceLang,           // ptr to buffer for source language or NULL
  char       *pszTargetLang            // ptr to buffer for target language or NULL
                                       // (size should be at least 40 bytes)
)
{
  int iRC = NO_ERROR;
  PPARSERAPI pParsAPI = (PPARSERAPI)hParser;

  iRC = ParsIntCheckAPIHandle( hParser );

  // get document languages
  if ( !iRC && (pszSourceLang || pszTargetLang) )
  {
    if ( pszSourceLang ) pszSourceLang[0] = EOS;
    if ( pszTargetLang ) pszTargetLang[0] = EOS;
    DocQueryInfo( pParsAPI->szDocObjName, NULL, NULL, pszSourceLang,
                  pszTargetLang, FALSE );
  } /* endif */

  return( iRC );
} /* end of function ParsGetDocLang */


// get number of segments in loaded segmented file
int ParsGetSegNum
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  long         *plSegCount             // ptr to buffer for returned segment number
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;

  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // get segment number
  if ( !iRC )
  {
    if ( plSegCount != NULL )
    {
      *plSegCount = (long)pSegFile->Document.ulMaxSeg - 1;  // ignore dummy segment 0 !!!
    }
    else
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParseGetSegNum */


// Load a markup table into memory for usage with function ParsTokenize
int ParsLoadMarkup
(
  HPARSER     hParser,                 // parser API handle
  HPARSMARKUP *phMarkup,               // ptr to buffer receiving the markup handle
  char        *pszMarkup               // name of the markup table
)
{
  int iRC = NO_ERROR;
  PLOADEDTABLE pTable = NULL;
  PPARSERAPI pParsAPI = (PPARSERAPI)hParser;
  PPARSERMARKUP pParsMarkup = NULL;

  phMarkup;

  iRC = ParsIntCheckAPIHandle( hParser );

  // check if markup name has been specified
  if ( !iRC )
  {
    if ( (pszMarkup == NULL) || (*pszMarkup == EOS) )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // load markup table
  if ( !iRC )
  {
    iRC = TALoadTagTable( pszMarkup, &pTable, FALSE, FALSE );
  } /* endif */

  // allocate markup control area
  if ( !iRC )
  {
    if ( !UtlAlloc( (PVOID *)&pParsMarkup, 0L, sizeof(PARSERMARKUP), NOMSG ) )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // anchor parser handle and tag table
  if ( !iRC )
  {
    memcpy( pParsMarkup->szID, PARSEMARKUP_ID, PARSE_ID_LEN );
    pParsMarkup->pTable = pTable;
    pParsMarkup->pParsAPI = pParsAPI;
  } /* endif */

  // cleanup in case of errors
  if ( iRC )
  {
    if ( pParsMarkup ) UtlAlloc( (PVOID *)&pParsMarkup, 0L, 0L, NOMSG );
    if ( pTable )  TAFreeTagTable( pTable );
  } /* endif */

  return( iRC );
}

// Free a markup table loaded with ParsLoadMarkup
int ParsFreeMarkup
(
  HPARSMARKUP  hMarkup                  // handle of markup as created by ParsloadMarkup
)
{
  int iRC = NO_ERROR;
  PPARSERMARKUP pParsMarkup = (PPARSERMARKUP)hMarkup;

  iRC = ParsIntCheckMarkupHandle( hMarkup );

  // free all data
  if ( !iRC )
  {
    if ( pParsMarkup->pTable )  TAFreeTagTable( pParsMarkup->pTable );
    if ( pParsMarkup->pTokBuffer ) UtlAlloc( (PVOID *)&pParsMarkup->pTokBuffer, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *)&pParsMarkup, 0L, 0L, NOMSG );
  } /* endif */

  return( iRC );
}

// Look for tags in the given buffer, the buffer must be zero terminated
int ParsTokenize
(
  HPARSMARKUP  hMarkup,                 // handle of markup as created by ParsloadMarkup
  char         *pszData                 // pointer to data being tokenized

)
{
  int iRC = NO_ERROR;
  PPARSERMARKUP pParsMarkup = (PPARSERMARKUP)hMarkup;

  iRC = ParsIntCheckMarkupHandle( hMarkup );

  // check if pszData is set
  if ( !iRC )
  {
    if ( pszData == NULL )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // tokenize the data
  if ( !iRC )
  {
    PSZ pszRest = NULL;
    USHORT usColPos = 0;

    // allocate initial token buffer
    if ( !pParsMarkup->pTokBuffer )
    {
      LONG lNewSize;
      pParsMarkup->usTokBufferEntries = 200;
      lNewSize = (LONG)pParsMarkup->usTokBufferEntries * sizeof(TOKENENTRY);
      if ( !UtlAlloc( (PVOID *)&(pParsMarkup->pTokBuffer), 0L, lNewSize, NOMSG ) )
      {
        iRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */

    if ( !iRC )
    {
      do
      {
        // tokenize
        if ( !TATagTokenize( pszData, pParsMarkup->pTable, TRUE, &pszRest,
                             &usColPos, pParsMarkup->pTokBuffer,
                             pParsMarkup->usTokBufferEntries , 0L) )
        {
          iRC = ERROR_GEN_FAILURE;
        } /* endif */

        // realloc token buffer if not large enough small
        if ( !iRC && pszRest )
        {
          LONG lNewSize;
          LONG lOldSize = (LONG)pParsMarkup->usTokBufferEntries * sizeof(TOKENENTRY);
          pParsMarkup->usTokBufferEntries += 200;
          lNewSize = (LONG)pParsMarkup->usTokBufferEntries * sizeof(TOKENENTRY);
          if ( !UtlAlloc( (PVOID *)&(pParsMarkup->pTokBuffer), lOldSize, lNewSize, NOMSG ) )
          {
            iRC = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
        } /* endif */
      } while ( !iRC && pszRest ); // while OK and buffer to small
    } /* endif */

    if ( !iRC )
    {
      pParsMarkup->fTokenAvailable = TRUE;
      pParsMarkup->fEndOfTokenList = FALSE;
      pParsMarkup->pCurrentToken = pParsMarkup->pTokBuffer;
      pParsMarkup->pData = pszData;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParsTokenize */

// Look for tags in the given buffer, the buffer must be zero terminated (UTF16 version)
int ParsTokenizeW
(
  HPARSMARKUP  hMarkup,                 // handle of markup as created by ParsloadMarkup
  WCHAR        *pszData                 // pointer to data being tokenized

)
{
  int iRC = NO_ERROR;
  PPARSERMARKUP pParsMarkup = (PPARSERMARKUP)hMarkup;

  iRC = ParsIntCheckMarkupHandle( hMarkup );

  // check if pszData is set
  if ( !iRC )
  {
    if ( pszData == NULL )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // tokenize the data
  if ( !iRC )
  {
    WCHAR *pszRest = NULL;
    USHORT usColPos = 0;
      // allocate initial token buffer
    if ( !pParsMarkup->pTokBuffer )
    {
      LONG lNewSize;
      pParsMarkup->usTokBufferEntries = 200;
      lNewSize = (LONG)pParsMarkup->usTokBufferEntries * sizeof(TOKENENTRY);
      if ( !UtlAlloc( (PVOID *)&(pParsMarkup->pTokBuffer), 0L, lNewSize, NOMSG ) )
      {
        iRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */

    if ( !iRC )
    {
      do
      {
        // tokenize
        if ( !TATagTokenizeW( pszData, pParsMarkup->pTable, TRUE, &pszRest,
                              &usColPos, pParsMarkup->pTokBuffer,
                              pParsMarkup->usTokBufferEntries ) )
        {
          iRC = ERROR_GEN_FAILURE;
        } /* endif */

        // realloc token buffer if not large enough small
        if ( !iRC && pszRest )
        {
          LONG lNewSize;
          LONG lOldSize = (LONG)pParsMarkup->usTokBufferEntries * sizeof(TOKENENTRY);
          pParsMarkup->usTokBufferEntries += 200;
          lNewSize = (LONG)pParsMarkup->usTokBufferEntries * sizeof(TOKENENTRY);
          if ( !UtlAlloc( (PVOID *)&(pParsMarkup->pTokBuffer), lOldSize, lNewSize, NOMSG ) )
          {
            iRC = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
        } /* endif */
      } while ( !iRC && pszRest ); // while OK and buffer to small
    } /* endif */

    if ( !iRC )
    {
      pParsMarkup->fTokenAvailable = TRUE;
      pParsMarkup->fEndOfTokenList = FALSE;
      pParsMarkup->pCurrentToken = pParsMarkup->pTokBuffer;
      pParsMarkup->pData = (PSZ)pszData;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParsTokenizeW */


// return the next token from the token list produced with ParsTokenize
// At the end of the token list a token with an ID of PARSTOKEN_ENDOFLIST
// is returned
int ParsGetNextToken
(
  HPARSMARKUP hMarkup,                 // handle of markup as created by ParsloadMarkup
  PPARSTOKEN  pToken                   // ptr to a PARSTOKEN structure receiving the
                                       // data of the token
)
{
  int iRC = NO_ERROR;
  PPARSERMARKUP pParsMarkup = (PPARSERMARKUP)hMarkup;

  iRC = ParsIntCheckMarkupHandle( hMarkup );

  // check if pszToken is set
  if ( !iRC )
  {
    if ( pToken == NULL )
    {
      iRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // check if tokens are available
  if ( !iRC && !pParsMarkup->fTokenAvailable)
  {
    iRC = ERROR_NO_ITEMS;
  } /* endif */

  // get current token
  if ( !iRC )
  {
    memset( pToken, 0, sizeof(PARSTOKEN) );

    if ( !pParsMarkup->fEndOfTokenList )
    {
      if ( pParsMarkup->pCurrentToken->sTokenid == ENDOFLIST )
      {
        pParsMarkup->fEndOfTokenList = TRUE;
        pToken->iTokenID = PARSTOKEN_ENDOFLIST;
      }
      else
      {
        pToken->iLength   = pParsMarkup->pCurrentToken->usLength;
        pToken->iStart    = pParsMarkup->pCurrentToken->pDataString - pParsMarkup->pData;
        pToken->iTokenID  = pParsMarkup->pCurrentToken->sTokenid;
        pToken->usAddInfo = pParsMarkup->pCurrentToken->sAddInfo;
        pToken->usClassID = pParsMarkup->pCurrentToken->ClassId;
        pToken->usFixedID = pParsMarkup->pCurrentToken->usOrgId;
        pParsMarkup->pCurrentToken++;
      } /* endif */
    }
    else
    {
      pToken->iTokenID = PARSTOKEN_ENDOFLIST;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParsGetNextToken */



// get the line number for the given segment
int  ParsGetSourceLine
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  LONG         lSegNum,                // number of segment to check
  PLONG        plStartLine,            // ptr to buffer for start line
  PLONG        plEndLine               // ptr to buffer for end line
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;

  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );
  if ( iRC ) iRC = INVALIDFILEHANDLE_RC;

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // check segment number
  if ( !iRC )
  {
    if ( (lSegNum < 0) || (lSegNum >= (LONG)pSegFile->Document.ulMaxSeg) )
    {
      iRC = INVALIDSEGMENT_RC;
    } /* endif */
  } /* endif */

  // access segment, check if segment is joined
  if ( !iRC )
  {
    PTBSEGMENT pTBSeg = EQFBGetSegW( &pSegFile->Document, lSegNum );
    if ( pTBSeg->SegFlags.Joined )
    {
      iRC = SEGMENTISJOINED_RC;
    } /* endif */
  } /* endif */

  // loop through segments and count line numbers
  if ( !iRC )
  {
    BOOL fTrailingLF = FALSE;
    LONG lCurSeg = 1;
    PTBSEGMENT pTBSeg = EQFBGetSegW( &pSegFile->Document, lCurSeg );
    LONG lLine = 1;
    LONG lColumn = 1;
    while ( (lCurSeg < lSegNum) && (pTBSeg != NULL) )
    {
      if ( !pTBSeg->SegFlags.Joined )
      {
        ParseUpdateLineCol( pTBSeg->pDataW, &lLine, &lColumn, &fTrailingLF );
        if ( fTrailingLF )
        {
          lLine++;
          lColumn = 1;
        } /* endif */
      } /* endif */
      lCurSeg++;
      pTBSeg = EQFBGetSegW( &pSegFile->Document, lCurSeg );
    } /* endwhile */

    if ( pTBSeg != NULL )
    {
      PTBSEGMENT pTBSeg = EQFBGetSegW( &pSegFile->Document, lSegNum );
      *plStartLine = lLine;
      ParseUpdateLineCol( pTBSeg->pDataW, &lLine, &lColumn, &fTrailingLF );
      *plEndLine = lLine;
    }
    else
    {
      // segment not found or end of file reached
      iRC = INVALIDSEGMENT_RC;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParsGetSourceLine */

// get the line number for the given segment
int  ParsGetSegmentNumber
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  LONG         lSearchLine,            // line number of segment
  LONG         lSearchColumn,          // column position of segment
  PLONG        plSegNum                // ptr to buffer for segment number
)
{
  PPARSSEGFILE pSegFile = NULL;
  int iRC = NO_ERROR;

  // check segmented file handle
  iRC = ParsIntCheckSegFileHandle( hSegFile );
  if ( iRC ) iRC = INVALIDFILEHANDLE_RC;

  // set segmented file pointer
  if ( !iRC )
  {
    pSegFile = (PPARSSEGFILE)hSegFile;
  } /* endif */

  // loop through segments and count line numbers
  if ( !iRC )
  {
    BOOL fFound = FALSE;
    BOOL fTrailingLF = FALSE;
    LONG lCurSeg = 1;
    LONG lLine = 1;
    LONG lColumn = 1;

    while ( !iRC && !fFound  )
    {
      LONG lSegStartLine = lLine;
      LONG lSegStartColumn = lColumn;
      PTBSEGMENT pTBSeg = EQFBGetSegW( &pSegFile->Document, lCurSeg );
      if ( !pTBSeg->SegFlags.Joined )
      {
        ParseUpdateLineCol( pTBSeg->pDataW, &lLine, &lColumn, &fTrailingLF );

        lColumn--;   // we at first column of next segment!!!

        if ( (lSearchLine > lLine ) ||
            ((lSearchLine == lLine ) && (lSearchColumn > lColumn)) )
        {
          // contínue search, we are still not at our segment
          lColumn++;                            // set column to start of next segment
          if ( fTrailingLF )
          {
            lLine++;
            lColumn = 1;
          } /* endif */
        }
        else if ( (lSearchLine > lSegStartLine ) ||
                  ((lSearchLine == lSegStartLine ) && (lSearchColumn >= lSegStartColumn)) )
        {
          // search position is within current segment
          fFound = TRUE;
        }
        else
        {
          // segment position not found
          iRC = NOMATCHINGSEGMENT_RC;
        } /* endif */

      } /* endif */

      if ( !iRC && !fFound ) lCurSeg++;
    } /* endwhile */

    if ( fFound )
    {
      *plSegNum = lCurSeg;
    }
    else
    {
      // segment not found or end of file reached
      iRC = NOMATCHINGSEGMENT_RC;
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function ParsGetSegmentNumber */




//
// Internal functions
//

// Update line and column info using supplied segment data
int ParseUpdateLineCol( PSZ_W pszData, PLONG plLine, PLONG plColumn, PBOOL pfTrailingLF )
{
  *pfTrailingLF = FALSE;
  while ( *pszData )
  {
    switch ( *pszData )
    {
      case  LF:
        if ( pszData[1] )
        {
          *plLine += 1;
          *plColumn = 1;
        }
        else
        {
          *pfTrailingLF = TRUE;
        } /* endif */
        break;
      default :
        *plColumn += 1;
        break;
    } /* endswitch */
    pszData++;
  } /* endwhile */
  return( 0 );
} /* end of function ParseUpdateLineCol */

// check parser API handle
int ParsIntCheckAPIHandle( HPARSER hParser )
{
  int iRC = NO_ERROR;
  PPARSERAPI pParsAPI = (PPARSERAPI)hParser;

  if ( pParsAPI == NULL )
  {
    iRC = ERROR_INVALID_PARAMETER;
  }
  else if ( memcmp( pParsAPI->szID, PARSEAPI_ID, PARSE_ID_LEN ) != 0 )
  {
    iRC = ERROR_INVALID_PARAMETER;
  } /* endif */
  return( iRC );
} /* end of function ParsIntCheckAPIHandle */

// check segmented file handle
int ParsIntCheckSegFileHandle( HPARSSEGFILE hSegFile )
{
  int iRC = NO_ERROR;
  PPARSSEGFILE pSegFile = (PPARSSEGFILE)hSegFile;

  if ( pSegFile == NULL )
  {
    iRC = ERROR_INVALID_PARAMETER;
  }
  else if ( memcmp( pSegFile->szID, PARSESEGFILE_ID, PARSE_ID_LEN ) != 0 )
  {
    iRC = ERROR_INVALID_PARAMETER;
  } /* endif */

  return( iRC );
} /* end of function ParsIntCheckSegFileHandle */

// check markup handle
int ParsIntCheckMarkupHandle( HPARSMARKUP hMarkup )
{
  int iRC = NO_ERROR;
  PPARSERMARKUP pParsMarkup = (PPARSERMARKUP)hMarkup;

  if ( pParsMarkup == NULL )
  {
    iRC = ERROR_INVALID_PARAMETER;
  }
  else if ( memcmp( pParsMarkup->szID, PARSEMARKUP_ID, PARSE_ID_LEN ) != 0 )
  {
    iRC = ERROR_INVALID_PARAMETER;
  } /* endif */

  return( iRC );
} /* end of function ParsIntCheckMarkupHandle */
