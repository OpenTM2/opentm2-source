//+----------------------------------------------------------------------------+
//| EQFSEGMD.C                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Combines all functions and definitions related to segment     |
//|              metadata                                                      |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_MORPH            // morph settings for Arabic/Hebrew detection
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // public EDITOR API functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFBDLG.ID"             // IDs of dialog controls
#include "EQFMETADATA.H"          // Metadata defines
#include "eqfutmdi.h"             // MDI utilities


// list of translation unit states
PSZ pszTranslStates[] = { "Needs translation", "Needs review translation", "New", "Signed-off", "Translated", "Final", 
                           "Needs adaption", "Needs review", NULL };

// list of comment styles
PSZ pszStyles[] = { "", "Source bug", "Others", "S-T Inconsistency", "Q&A", "Global Memory", NULL };

// GQ: Allowed functions when focus is in the MLE or combobox
static USHORT usAllowedFunctions[] = { ACTPROP_FUNC, DICTDOWN_FUNC, DICTUP_FUNC, ESCAPE_FUNC, FILE_FUNC, NEXTDOC_FUNC, PREVDOC_FUNC, QUIT_FUNC,
                                SAVE_FUNC, GOTOSEG_FUNC, POSTEDIT_FUNC, AUTOTRANS_FUNC, FIND_FUNC, OPEN_FUNC, KEYS_FUNC, ADDFUZZY_FUNC, 
                                DISPORG_FUNC, PRINT_FUNC, FONTSIZE_FUNC, MARGINACT_FUNC, SPELLSEG_FUNC, SPELLFILE_FUNC, COMMAND_FUNC,  SETTINGS_FUNC, 
                                GOTO_FUNC, ACTTRANS_FUNC, SRCPROP_FUNC, SAVEAS_FUNC, REIMPORT_FUNC, EDITADD_FUNC, SHOWTRANS_FUNC, TOCGOTO_FUNC, 
                                SPELLAUTO_FUNC, GOTOSEGMENT_FUNC, CFIND_FUNC, SEGPROP_FUNC, TSEGNEXT_EXACT_FUNC, TSEGNEXT_FUZZY_FUNC, TSEGNEXT_NONE_FUNC, 
                                TSEGNEXT_MT_FUNC, TSEGNEXT_GLOBAL_FUNC, LAST_FUNC };


//
// get the first proposal from the supplied metadata
//
BOOL MDGetFirstProposal( PVOID pvMetaData, PMD_PROPOSAL pProposal )
{
  BOOL fAvailable = FALSE;

  if ( pvMetaData )
  {
    PMD_METADATA pData = (PMD_METADATA)pvMetaData;
    pProposal->pvNext = MDGetFirstElement( pData );
    fAvailable = MDGetNextProposal( pProposal );
  } /* endif */

  return( fAvailable );
} /* end of MDGetFirstProposal */

//
// get the next proposal from the supplied metadata
//
BOOL MDGetNextProposal( PMD_PROPOSAL pProposal )
{
  BOOL fAvailable = FALSE;

  if ( pProposal && pProposal->pvNext )
  {
    PMD_ELEMENT_HDR pElement = (PMD_ELEMENT_HDR)pProposal->pvNext;
    memset( pProposal, 0, sizeof(MD_PROPOSAL) );
    while ( (pElement->Type != MD_PROPOSAL_TYPE) && (pElement->Type != MD_ENDOFLIST_TYPE) )
    {
      pElement = MDGetNextElement( pElement );
    } /*endwhile */
    if ( pElement->Type == MD_PROPOSAL_TYPE )
    {
      PSZ_W pszTemp;
      int iBytes = 0;

      PMD_PROPOSAL_DATA pMDProp = (PMD_PROPOSAL_DATA)pElement;
  
      pProposal->sQuality = pMDProp->sQuality;
      pProposal->ulSegmentId = pMDProp->ulSegmentId;
      pProposal->lDate = pMDProp->lModificationDate;
      
      pszTemp = (PSZ_W)&(pMDProp->Buffer);
      wcscpy( pProposal->szSource, pszTemp + pMDProp->lSourcePos );
      wcscpy( pProposal->szTarget, pszTemp + pMDProp->lTargetPos );
      iBytes = WideCharToMultiByte( CP_OEMCP, 0, pszTemp + pMDProp->lAuthorPos, -1, pProposal->szAuthorName, sizeof(pProposal->szAuthorName), NULL, NULL );
      pProposal->szAuthorName[iBytes] = NULC;
      if ( *(pszTemp + pMDProp->lDocNamePos) != 0 )
      {
        iBytes = WideCharToMultiByte( CP_OEMCP, 0, pszTemp + pMDProp->lDocNamePos, -1, pProposal->szDocName, sizeof(pProposal->szDocName), NULL, NULL );
        pProposal->szDocName[iBytes] = NULC;
      }
      else
      {
        strcpy( pProposal->szDocName, "XLIFF" );
      } /* endif */
      fAvailable = TRUE;

      pProposal->pvNext = MDGetNextElement( pElement );
    } /* endif */
  } /* endif */
  return( fAvailable );
}

//
// Write metadata to file
//
USHORT MDWriteMetaData( PTBDOCUMENT pDoc )
{
  USHORT     usRC = 0;
  HANDLE     hfOut = NULLHANDLE;

  // setup output file name and open output file
  {
    CHAR      szMetadataFile[MAX_EQF_PATH];      // buffer for output file name
    CHAR      szFolder[MAX_FILESPEC];            // buffer for folder name
    CHAR      szDocument[MAX_FILESPEC];          // buffer for document name
    CHAR      chDrive;                           // buffer for drice letter
    PSZ       pszTemp;  

    strcpy( szMetadataFile, pDoc->szDocName );
    chDrive = szMetadataFile[0];
    pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) strcpy( szDocument, pszTemp );
    if ( pszTemp ) pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) strcpy( szFolder, pszTemp ); 
    if ( pszTemp )
    {
      USHORT    usAction = 0;                      // action performed by UtlOpen

      UtlMakeEQFPath( szMetadataFile, chDrive, METADATA_PATH, szFolder );
      UtlMkDir( szMetadataFile, 0, FALSE );
      strcat( szMetadataFile, BACKSLASH_STR );
      strcat( szMetadataFile, szDocument );
      usRC = UtlOpen( szMetadataFile, &hfOut, &usAction, 0L, FILE_NORMAL, FILE_CREATE | FILE_TRUNCATE, 
                      OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, 0L, FALSE );
    }
    else
    {
      usRC = ERROR_INVALID_DATA;
    } /* endif */
  }


  // write metadata of all segments to ouput file
  if ( !usRC )
  {
    PTBSEGMENT pSeg = NULL;                        // ptr to segment
    ULONG      ulSegNum = 1;
    ULONG      ulAddSegNum = 1;
    ULONG      ulActiveTable = STANDARDTABLE;

    pSeg = EQFBGetFromBothTables( pDoc, &ulSegNum, &ulAddSegNum, &ulActiveTable );
    while ( !usRC && pSeg )
    {
      if ( pSeg->pvMetadata )
      {
        ULONG ulWritten = 0;

        PMD_METADATA pMetadata = (PMD_METADATA)pSeg->pvMetadata;
        pMetadata->ulSegNum = pSeg->ulSegNum;
        usRC = UtlWriteL( hfOut, pMetadata, pMetadata->lSize, &ulWritten, TRUE );
      } /* endif */
      pSeg = EQFBGetFromBothTables( pDoc, &ulSegNum, &ulAddSegNum, &ulActiveTable );
    } /*endwhile */
  } /* endif */

  // write empty metadata as terminator
  if ( !usRC )
  {
    ULONG ulWritten = 0;
    MD_METADATA Metadata;
  
    memset( &Metadata, 0, sizeof(Metadata) );
    usRC = UtlWriteL( hfOut, &Metadata, sizeof(Metadata), &ulWritten, TRUE );
  } /* endif */

  // close output file
  if ( hfOut ) UtlClose( hfOut, FALSE );

  return( usRC );
} /* end of MDWriteMetaData */

//
// Load metadata from file
//
USHORT MDLoadMetaData( PTBDOCUMENT pDoc )
{
  USHORT     usRC = 0;
  HANDLE     hfIn = NULLHANDLE;

  // setup input file name and open input file
  {
    CHAR      szMetadataFile[MAX_EQF_PATH];      // buffer for output file name
    CHAR      szFolder[MAX_FILESPEC];            // buffer for folder name
    CHAR      szDocument[MAX_FILESPEC];          // buffer for document name
    CHAR      chDrive;                           // buffer for drice letter
    PSZ       pszTemp;  

    strcpy( szMetadataFile, pDoc->szDocName );
    chDrive = szMetadataFile[0];
    pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) strcpy( szDocument, pszTemp );
    if ( pszTemp ) pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) strcpy( szFolder, pszTemp ); 
    if ( pszTemp )
    {
      USHORT    usAction = 0;                      // action performed by UtlOpen

      UtlMakeEQFPath( szMetadataFile, chDrive, METADATA_PATH, szFolder );
      UtlMkDir( szMetadataFile, 0, FALSE );
      strcat( szMetadataFile, BACKSLASH_STR );
      strcat( szMetadataFile, szDocument );
      usRC = UtlOpen( szMetadataFile, &hfIn, &usAction, 0L, FILE_NORMAL, FILE_OPEN, 
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, FALSE );
    }
    else
    {
      usRC = ERROR_INVALID_DATA;
    } /* endif */
  }


  // read metadata and add it to approbriate segment
  if ( !usRC )
  {
    ULONG      ulRead = 0;
    MD_METADATA Metadata;

    // get metadata area header
    usRC = UtlReadL( hfIn, &Metadata, sizeof(Metadata), &ulRead, TRUE );
    if ( !usRC && (ulRead != sizeof(Metadata)))  usRC = ERROR_READ_FAULT;

    // while there are metadata areas...
    while ( !usRC && (Metadata.lSize != 0) )
    {
      PBYTE pbMetadata = NULL;

      // allocate buffer for metadata
      if ( !UtlAlloc( (PVOID *)&pbMetadata, 0, Metadata.lSize, ERROR_STORAGE ) )
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */

      // read rest of metadata
      if ( !usRC )
      {
        memcpy( pbMetadata, &Metadata, sizeof(MD_METADATA) );
        usRC = UtlReadL( hfIn, pbMetadata + sizeof(MD_METADATA), Metadata.lSize - sizeof(Metadata), &ulRead, TRUE );
        if ( !usRC && (ulRead != (Metadata.lSize - sizeof(Metadata))))  usRC = ERROR_READ_FAULT;
      } /* endif */

      // add metadata to segment
      if ( !usRC )
      {
        PTBSEGMENT pSeg = EQFBGetSegW( pDoc, Metadata.ulSegNum );
        if ( pSeg )
        {
          pSeg->pvMetadata = pbMetadata;
        }
        else
        {
          UtlAlloc( (PVOID *)&pbMetadata, 0, 0, NOMSG );
        } /* endif */
      } /* endif */

      // get header of next metadata area
      if ( !usRC )
      {
        usRC = UtlReadL( hfIn, &Metadata, sizeof(Metadata), &ulRead, TRUE );
        if ( !usRC && (ulRead != sizeof(Metadata)))  usRC = ERROR_READ_FAULT;
      } /* endif */
    } /*endwhile */
  } /* endif */


  // close input file
  if ( hfIn ) UtlClose( hfIn, FALSE );


  return( usRC );
} /* end of MDLoadMetaData */

//
// Convert the binary segment meta data format to the XML based segment meta data format
//
BOOL MDConvertToMemMetadata( PVOID pvSegMetaData, PSZ_W pszMemMetaData )
{
  BOOL             fOK = TRUE;
  PMD_METADATA     pSegMetaData = (PMD_METADATA)pvSegMetaData;

  if ( pSegMetaData )
  {
    PMD_ELEMENT_HDR pCurElement = MDGetFirstElement( pSegMetaData );
    while ( pCurElement && (pCurElement->Type != MD_ENDOFLIST_TYPE) )
    {
      switch ( pCurElement->Type )
      {
        case MD_COMMENT_TYPE:
          {
            PSZ_W pszSource, pszTarget;
            PMD_COMMENT_DATA pMDComment = (PMD_COMMENT_DATA)pCurElement;
            swprintf( pszMemMetaData, L"<Note style=\"%s\">", pMDComment->szStyle );

            // copy note text to buffer and escape special characters
            pszSource = pMDComment->szComment;
            pszTarget = pszMemMetaData + wcslen(pszMemMetaData);
            while ( *pszSource != 0 )
            {
              if ( *pszSource == L'<' )
              {
                wcscpy( pszTarget, L"&lt;" );
                pszTarget += 4;
                pszSource++;
              }
              else if ( *pszSource == L'>' )
              {
                wcscpy( pszTarget, L"&gt;" );
                pszTarget += 4;
                pszSource++;
              }
              else if ( *pszSource == L'&' )
              {
                wcscpy( pszTarget, L"&amp;" );
                pszTarget += 5;
                pszSource++;
              }
              else
              {
                *pszTarget++ = *pszSource++;
              } /* endif */                 
            } /* endwhile */               
            wcscpy( pszTarget, L"</Note>" );
            pszMemMetaData = pszTarget + wcslen(pszTarget);
          }
          break;
        case MD_NOTELIST_TYPE:
        case MD_HISTORY_TYPE:
        case MD_STATUS_TYPE:
        default:
          // ignore these elements
          break;
      } /*endswitch */
      pCurElement = MDGetNextElement( pCurElement );
    } /*endwhile */
  } /* endif */

  // terminate TM meta data buffer
  *pszMemMetaData = 0;

  return( fOK );
} /* end of MDConvertToMemMetadata */

//
// Get first element of a metadata area
//
PMD_ELEMENT_HDR MDGetFirstElement( PMD_METADATA pData )
{
  PMD_ELEMENT_HDR pElement = NULL;

  if ( pData )
  {
    pElement = (PMD_ELEMENT_HDR)(((PBYTE)pData) + sizeof(MD_METADATA) );
  } /* endif */
  return( pElement );
} /* end of MDGetFirstElement */

//
// Get next element of a metadata area
//
PMD_ELEMENT_HDR MDGetNextElement( PMD_ELEMENT_HDR pElement )
{
  if ( pElement )
  {
    pElement = (PMD_ELEMENT_HDR)(((PBYTE)pElement) + pElement->lSize);
  } /* endif */
  return( pElement );
} /* end of MDGetNextElement */

//
// Create an empty metadata area
//
BOOL MDCreateMetaData( PMD_METADATA *ppData )
{
  BOOL             fOK = TRUE;
  int              iSize = 0;

  iSize = sizeof(MD_METADATA) + sizeof(MD_ELEMENT_HDR);

  fOK = UtlAlloc( (PVOID *)ppData, 0, iSize, ERROR_STORAGE );

  if ( fOK )
  {
    PMD_ELEMENT_HDR pElement = MDGetFirstElement( *ppData );
    PMD_METADATA pData = *ppData;
    pData->lSize = iSize;
    pElement->lSize = sizeof(MD_ELEMENT_HDR);
    pElement->Type  = MD_ENDOFLIST_TYPE;
  } /* endif */
  return( fOK );
} /* end of MDCreateArea */

//
// Release a metadata area
//
BOOL MDFreeMetaData( PMD_METADATA *ppData )
{
  BOOL             fOK = TRUE;

  if ( ppData && *ppData )
  {
    UtlAlloc( (PVOID *)ppData, 0, 0, NOMSG );
    *ppData = NULL;
  } /* endif */
  return( fOK );
} /* end of MDFreeMetaData */


//
// Find first metadata element of the given type
//
PMD_ELEMENT_HDR MDFindElementByType( PMD_METADATA pData, MD_TYPE Type )
{
  PMD_ELEMENT_HDR pElement = MDGetFirstElement( pData );

  while ( pElement && (pElement->Type != MD_ENDOFLIST_TYPE) && (pElement->Type != Type) )
  {
    pElement = MDGetNextElement( pElement );
  } /*endwhile */
  if ( pElement->Type != Type )
  {
    pElement = NULL;
  } /* endif */
  return( pElement );
} /* end of MDFindElementByType */

//
// Add a metadata element to a metadata area
//
BOOL MDAddElement( PMD_METADATA *ppData, PMD_ELEMENT_HDR pElement )
{
  BOOL             fOK = TRUE;
  int              iNewSize = 0;

  // create new metadata area if there is none yet
  if ( *ppData == NULL )
  {
    fOK = MDCreateMetaData( ppData );
  } /* endif */

  // compute new size of metadata area
  iNewSize = (*ppData)->lSize + pElement->lSize;

  // reallocate data area
  fOK = UtlAlloc( (PVOID *)ppData, (*ppData)->lSize, iNewSize, ERROR_STORAGE );

  // add new element right before end-of-list element
  if ( fOK )
  {
    // find end-of-list element
    PMD_ELEMENT_HDR pEndOfList = MDFindElementByType( *ppData, MD_ENDOFLIST_TYPE );

    if ( pEndOfList )
    {
      // make room for new element
      memmove( ((PBYTE)pEndOfList) + pElement->lSize, pEndOfList, sizeof(MD_ELEMENT_HDR) );

      // insert new element
      memcpy( pEndOfList, pElement, pElement->lSize );

      // adjust metadata area size
      (*ppData)->lSize = iNewSize;

    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of MDAddElement */

//
// Add comment metadata to metadata area
//
void MDAddCommentData( PVOID *ppvData, PSZ_W pszComment, PSZ_W pszStyle )
{
  PMD_COMMENT_DATA pMDComment = NULL;
  PMD_METADATA *ppData = (PMD_METADATA *)ppvData;

  int iSize = sizeof(MD_COMMENT_DATA) + ((wcslen(pszComment) + 1) * sizeof(CHAR_W));

  UtlAlloc( (PVOID *)&pMDComment, 0, iSize, ERROR_STORAGE );

  if ( pMDComment )
  {
    pMDComment->Hdr.lSize = iSize;
    pMDComment->Hdr.Type  = MD_COMMENT_TYPE;
    wcscpy( pMDComment->szStyle, pszStyle );
    wcscpy( pMDComment->szComment, pszComment );

    MDAddElement( ppData, (PMD_ELEMENT_HDR)pMDComment );

    UtlAlloc( (PVOID *)&pMDComment, 0, 0, NOMSG );
  } /* endif */  
} /* end of MDAddCommentData */



