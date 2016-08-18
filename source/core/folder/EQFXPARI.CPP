//+----------------------------------------------------------------------------+
//| EQFXPARI.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Parser for Idiom XLIFF Files                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define _CRT_NON_CONFORMING_SWPRINTFS

#pragma pack( push, TM2StructPacking, 1 )

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include "eqf.h"                  // General .H for EQF
#include "EQFMEMIE.H"
#include "EQFXLIFF.H"
#include "EQFMETADATA.H"

#pragma pack( pop, TM2StructPacking )

#include "core\pluginmanager\OtmMemory.h"
#include "core\pluginmanager\OtmMemoryPlugin.h"
#include "core\memory\MemoryFactory.h"

#include "cxmlwriter.H"

// the Win32 Xerces build requires the default structure packing...
#pragma pack( push )
#pragma pack(8)

#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#pragma pack( pop )


XERCES_CPP_NAMESPACE_USE


#include "eqfxlifi.H"

typedef struct _XLIFFNAMETOID
{
  CHAR_W   szName[40];                 // name of element
  ELEMENTID ID;                        // ID of element 
} XLIFFNAMETOID, *PXLIFFNAMETOID;


// the following table must be sorted!
XLIFFNAMETOID XLIFFNameToID[] =
{ 
  { L"alt-trans",                      ALT_TRANS_ELEMENT }, 
  { L"body",                           BODY_ELEMENT }, 
  { L"file",                           FILE_ELEMENT }, 
  { L"header",                         HEADER_ELEMENT }, 
  { L"iws:attribute",                  IWS_ATTRIBUTE_ELEMENT }, 
  { L"iws:boundary-seg",               IWS_BOUNDARY_SEG_ELEMENT }, 
  { L"iws:dwb-project-info",           IWS_DWB_PROJECT_INFO_ELEMENT }, 
  { L"iws:extension-level",            IWS_EXTENSION_LEVEL_ELEMENT }, 
  { L"iws:markup-seg",                 IWS_MARKUP_SEG_ELEMENT }, 
  { L"note",                           NOTE_ELEMENT }, 
  { L"iws:project-attribute-metadata", IWS_PROJECT_ATTRIBUTE_METADATA_ELEMENT }, 
  { L"iws:project-attributes",         IWS_PROJECT_ATTRIBUTES }, 
  { L"iws:scoping-configuration",      IWS_SCOPING_CONFIGURATION_ELEMENT }, 
  { L"iws:segment-metadata",           IWS_SEGMENT_METADATA_ELEMENT }, 
  { L"iws:status",                     IWS_STATUS_ELEMENT }, 
  { L"iws:tm-attribute-metadata",      IWS_TM_ATTRIBUTE_METADATA_ELEMENT }, 
  { L"ph",                             PH_ELEMENT }, 
  { L"source",                         SOURCE_ELEMENT }, 
  { L"target",                         TARGET_ELEMENT }, 
  { L"trans-unit",                     TRANS_UNIT_ELEMENT }, 
  { L"xliff",                          XLIFF_ELEMENT },
  { L"",                               UNKNOWN_ELEMENT } };


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Implementation of XLIFF SAX parser                                                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IdiomXLIFFParseHandler::IdiomXLIFFParseHandler()
{
#ifdef _DEBUG
  hfLog = fopen( "C:\\SAXTEST.LOG", "a" );
  if ( hfLog )
  {
    fprintf( hfLog, "**** SAXTEST ****\n" );
  } /* endif */
#else
  hfLog = NULLHANDLE;
#endif 

  // get XLIFF utlities instance
  m_Util = new CXLIFFUtil();

  // allocate buffer areas
  pBuf = (PBUFFERAREAS)malloc( sizeof(BUFFERAREAS) );
  if ( pBuf ) memset( pBuf, 0, sizeof(BUFFERAREAS) );

  // initialize element stack
  iStackSize = 0;
  iCurElement = 0;
  pStack = NULL;

  pfnInsertSegment = NULL;
  lMemHandle = 0;
  fHeaderDone = FALSE;

  m_ulSegNum = 1;

  m_pTagging = new CTagList();

  memset( &CurElement, 0, sizeof(CurElement) );

  ProcessingMode = IMPORT_MODE;

  fPassThru = FALSE;
  
  pxw = NULL; 

  ulSegNum = 1;
  lTranslatedSegs = 0;
  lTotalSegs = 0;
}

IdiomXLIFFParseHandler::~IdiomXLIFFParseHandler()
{
  if ( hfLog )  fclose( hfLog );
  if ( pStack ) free( pStack );
  if ( pBuf )   free( pBuf );

  delete m_pTagging;
}

void IdiomXLIFFParseHandler::startElement
(
  const XMLCh* const name,
  AttributeList& attributes
)
{
  PSZ_W pszName = (PSZ_W)name;
  int iAttribs = attributes.getLength(); 

  if ( hfLog ) fprintf( hfLog, "StartElement: %S\n", pszName );

  Push( &CurElement );

  CurElement.ID = GetElementID( pszName );
  CurElement.AttrID = UNKNOWN_ATTR;            // reset attribute ID of current element 

  if ( ProcessingMode == EXPORT_MODE )
  {
    // copy to output file if passthru is active
    if ( pxw && fPassThru )
    {
      pxw->WriteStartElement( pszName );

      for( int i = 0; i < iAttribs; i++ )
      {
        PSZ_W pszName = (PSZ_W)attributes.getName( i );
        PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

        pxw->WriteAttributeString( pszName, pszValue );
      } /* endfor */
    } /* endif */

    // handling for current element
    switch ( CurElement.ID )
    {
      case FILE_ELEMENT:
        // reset file info
        memset( &(pBuf->FileInfo), 0, sizeof(pBuf->FileInfo) );

        // get file attributes
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

          if ( pszName && pszValue )
          {
            if ( wcsicmp( pszName, L"original" ) == 0 )
            {
              wcsncpy( pBuf->FileInfo.szFile, pszValue, (sizeof(pBuf->FileInfo.szFile)/sizeof(CHAR_W)) - 1 ); 
            } /* endif */
          } /* endif */
        } /* endfor */
        break;

      case TRANS_UNIT_ELEMENT:
        // reset passthru mode for translation units..
        fPassThru = FALSE;
        m_pTagging->clear();

        memset( &(pBuf->TranslUnit), 0, sizeof(pBuf->TranslUnit) );

        // get unit number from attributes
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( wcsicmp( pszName, L"id" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            pBuf->TranslUnit.ulNum = _wtol( pszValue );
          } /* endif */
        } /* endfor */

        // fill translation unit with data from segmented target file
        WriteTransUnitFromSegmentData( pBuf->FileInfo.szFile, pBuf->TranslUnit.ulNum );

        break;

      case IWS_BOUNDARY_SEG_ELEMENT:
      case IWS_MARKUP_SEG_ELEMENT:
        // write this elements to the ouput file even if not in passthru mode
        if ( !this->fPassThru )
        {
          pxw->WriteStartElement( pszName );

          for( int i = 0; i < iAttribs; i++ )
          {
            PSZ_W pszName = (PSZ_W)attributes.getName( i );
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

            pxw->WriteAttributeString( pszName, pszValue );
          } /* endfor */
        } /* endif */
        break;

      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */
  }
  else
  {
    switch ( CurElement.ID )
    {
      case FILE_ELEMENT:
        // reset file info
        memset( &(pBuf->FileInfo), 0, sizeof(pBuf->FileInfo) );

        // get file attributes
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

          if ( pszName && pszValue )
          {
            if ( wcsicmp( pszName, L"source-language" ) == 0 )
            {
              CHAR szTemp[20];
              wcsncpy( pBuf->FileInfo.szSourceLanguage, pszValue, (sizeof(pBuf->FileInfo.szSourceLanguage)/sizeof(CHAR_W)) - 1 ); 
              this->pszSourceLanguage[0] = '\0';  
              WideCharToMultiByte( CP_OEMCP, 0, pszValue, -1, szTemp, sizeof(szTemp)-1, NULL, NULL );
              m_Util->TMXLanguage2TMLanguage( "", szTemp, this->pszSourceLanguage );
            } 
            else if ( wcsicmp( pszName, L"content" ) == 0 )
            {
              wcsncpy( pBuf->FileInfo.szContent, pszValue, (sizeof(pBuf->FileInfo.szContent)/sizeof(CHAR_W)) - 1 ); 
            } 
            else if ( wcsicmp( pszName, L"datatype" ) == 0 )
            {
              wcsncpy( pBuf->FileInfo.szDataType, pszValue, (sizeof(pBuf->FileInfo.szDataType)/sizeof(CHAR_W)) - 1 ); 
            } 
            else if ( wcsicmp( pszName, L"original" ) == 0 )
            {
              wcsncpy( pBuf->FileInfo.szFile, pszValue, (sizeof(pBuf->FileInfo.szFile)/sizeof(CHAR_W)) - 1 ); 
            } 
            else if ( wcsicmp( pszName, L"target-language" ) == 0 )
            {
              CHAR szTemp[20];
              wcsncpy( pBuf->FileInfo.szTargetLanguage, pszValue, (sizeof(pBuf->FileInfo.szTargetLanguage)/sizeof(CHAR_W)) - 1 ); 
              this->pszTargetLanguage[0] = '\0';  
              WideCharToMultiByte( CP_OEMCP, 0, pszValue, -1, szTemp, sizeof(szTemp)-1, NULL, NULL );
              m_Util->TMXLanguage2TMLanguage( "", szTemp, this->pszTargetLanguage );
            } 
            else if ( wcsicmp( pszName, L"tool" ) == 0 )
            {
              wcsncpy( pBuf->FileInfo.szTool, pszValue, (sizeof(pBuf->FileInfo.szTool)/sizeof(CHAR_W)) - 1 ); 
            } 
            else if ( wcsicmp( pszName, L"build-num" ) == 0 )
            {
              wcsncpy( pBuf->FileInfo.szBuild, pszValue, (sizeof(pBuf->FileInfo.szBuild)/sizeof(CHAR_W)) - 1 ); 
            } /* endif */
          } /* endif */
        } /* endfor */
        break;

      case TRANS_UNIT_ELEMENT:
        // reset translation unit info
        memset( &(pBuf->TranslUnit), 0, sizeof(pBuf->TranslUnit) );
        m_pTagging->clear();

        // get unit number and approved flag from attributes
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( wcsicmp( pszName, L"id" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            pBuf->TranslUnit.ulNum = _wtol( pszValue );
          }
          else if ( wcsicmp( pszName, L"approved" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            pBuf->TranslUnit.fApproved = (wcsicmp( pszValue, L"yes" ) == 0);
          } /* endif */
        } /* endfor */
        break;
      case ALT_TRANS_ELEMENT:
        // reset proposal info
        memset( &(pBuf->Proposal), 0, sizeof(pBuf->Proposal) );
        CurElement.fProposal = TRUE;

        // get quality
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( wcsicmp( pszName, L"match-quality" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            double dbValue = _wtof( pszValue );           
            pBuf->Proposal.sQuality = (SHORT)(dbValue * 100);
          } /* endif */
        } /* endfor */
        
        break;

      case NOTE_ELEMENT:
        pBuf->szNoteAuthor[0] = 0;

        // get note author
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( wcsicmp( pszName, L"from" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcsncpy( pBuf->szNoteAuthor, pszValue, (sizeof(pBuf->szNoteAuthor)/sizeof(CHAR_W)) - 1 );
          } /* endif */
        } /* endfor */
        fCatchData = TRUE;
        break;

      case SOURCE_ELEMENT:
        pBuf->szData[0] = 0;
        fCatchData = TRUE;
        break;

      case TARGET_ELEMENT:
        pBuf->szData[0] = 0;
        fCatchData = TRUE;
        // get state value from attributes
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( wcsicmp( pszName, L"state" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcsncpy( pBuf->TranslUnit.szState, pszValue, (sizeof(pBuf->TranslUnit.szState)/sizeof(CHAR_W)) - 1 );
          } /* endif */
        } /* endfor */
        break;

      case PH_ELEMENT:
        {
          // switch to tagging mode
          CurElement.fInlineTagging = TRUE;

          // get ID and tagging (if any)
          int id = -1;
          pBuf->szTagging[0] = 0;
          for( int i = 0; i < iAttribs; i++ )
          {
            PSZ_W pszName = (PSZ_W)attributes.getName( i );
            if ( wcsicmp( pszName, L"id" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
              id = _wtol( pszValue );
            } 
            else if ( wcsicmp( pszName, L"x" ) == 0 )
            {
              // add actual iline tagging to temp area
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
              wcscpy( pBuf->szTagging, pszValue );
            } /* endif */
          } /* endfor */

          // handle tagging
          if ( id != -1 )
          {
            PSZ_W pszTagging = NULL;
            if ( pBuf->szTagging[0] != 0 )
            {
              m_pTagging->add( id, pBuf->szTagging );
              pszTagging = pBuf->szTagging;
            }
            else
            {
              pszTagging = m_pTagging->get( id );
            } /* endif */

            // add tagging to current text buffer
            if ( pszTagging )
            {
              swprintf( pBuf->szData + wcslen(pBuf->szData), L"<ph id=%ld>%s</ph>", id, pszTagging );
            } /* endif */
          } /* endif */
        }
        break;
      case IWS_ATTRIBUTE_ELEMENT:
        // check if these are proposal attributes
        if ( CurElement.fProposal )
        {
          CurElement.AttrID = UNKNOWN_ATTR;
          for( int i = 0; i < iAttribs; i++ )
          {
            PSZ_W pszName = (PSZ_W)attributes.getName( i );
            if ( wcsicmp( pszName, L"name" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
              if ( pszValue != NULL )
              {
                if ( wcsicmp( pszValue, L"Modified On" ) == 0)
                {
                  CurElement.AttrID  = MODIFIED_ON_ATTR;
                }
                else if ( wcsicmp( pszValue, L"Created On" ) == 0)
                {
                  CurElement.AttrID  = CREATED_ON_ATTR;
                }
                else if ( wcsicmp( pszValue, L"Created By" ) == 0)
                {
                  CurElement.AttrID  = CREATED_BY_ATTR;
                }
                else if ( wcsicmp( pszValue, L"Modified By" ) == 0)
                {
                  CurElement.AttrID  = MODIFIED_BY_ATTR;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endfor */
        } /* endif */
        break;

      case IWS_STATUS_ELEMENT:
        // currently we are interested only in the "target_content" attribute
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( wcsicmp( pszName, L"target_content" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            if ( pszValue != NULL )
            {
              if ( wcsicmp( pszValue, L"placeholders_only" ) == 0)
              {
                pBuf->TranslUnit.fPlaceHoldersOnly = TRUE;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endfor */
        break;

      case IWS_SEGMENT_METADATA_ELEMENT:
        // get wordcount from attributes
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( wcsicmp( pszName, L"ws_word_count" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            if ( pszValue != NULL )
            {
              if ( wcsicmp( pszValue, L"placeholders_only" ) == 0)
              {
                pBuf->TranslUnit.lSourceWords = _wtol( pszValue );
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endfor */
        break;

      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */
  } /* endif */

  if ( hfLog )
  {
    for( int i = 0; i < iAttribs; i++ )
    {
      PSZ_W pszName = (PSZ_W)attributes.getName( i );
      PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
      fprintf( hfLog, "Attribute %ld: Name=%S, Value=%S\n", i, pszName, pszValue );
    } /* endfor */
  } /* endif */
}

void IdiomXLIFFParseHandler::endElement(const XMLCh* const name )
{
  PSZ_W pszName = (PSZ_W)name;
  if ( hfLog ) fprintf( hfLog, "EndElement: %S\n", pszName );

  ELEMENTID CurrentID = CurElement.ID;
  ATTRIBUTEID CurrentAttr = CurElement.AttrID;


  if ( ProcessingMode == EXPORT_MODE )
  {
    // in passthru mode, pass everything to output file
    if ( pxw && fPassThru )
    {
      pxw->WriteEndElement();
    } /* endif */

    // do any handling required for this element
    switch ( CurrentID )
    {
      case TRANS_UNIT_ELEMENT:
        // write end element of translation unit
        pxw->WriteEndElement();

        // resume passthru mode
        fPassThru = TRUE;
        break;

    case IWS_BOUNDARY_SEG_ELEMENT:
    case IWS_MARKUP_SEG_ELEMENT:
      // write this elements to the ouput file even if not in passthru mode
      if ( !this->fPassThru )
      {
        pxw->WriteEndElement();
      } /* endif */
      break;

      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */
  }
  else
  {
    switch ( CurrentID )
    {
      case SOURCE_ELEMENT:
        if ( CurElement.fProposal )
        {
          wcsncpy( pBuf->Proposal.szSource, pBuf->szData, MAX_SEGMENT_SIZE - 1);
          pBuf->Proposal.szSource[MAX_SEGMENT_SIZE-1] = 0;
        }
        else
        {
          wcsncpy( pBuf->TranslUnit.szSource, pBuf->szData, MAX_SEGMENT_SIZE - 1);
          pBuf->TranslUnit.szSource[MAX_SEGMENT_SIZE-1] = 0;
        } /* endif */
        fCatchData = FALSE;
        break;
      case TARGET_ELEMENT:
        if ( CurElement.fProposal )
        {
          wcsncpy( pBuf->Proposal.szTarget, pBuf->szData, MAX_SEGMENT_SIZE - 1);
          pBuf->Proposal.szTarget[MAX_SEGMENT_SIZE-1] = 0;
        }
        else
        {
          wcsncpy( pBuf->TranslUnit.szTarget, pBuf->szData, MAX_SEGMENT_SIZE - 1);
          pBuf->TranslUnit.szTarget[MAX_SEGMENT_SIZE-1] = 0;
        } /* endif */
        fCatchData = FALSE;

        // create a proposal from the source and target data if not in poposal mode
        if ( !CurElement.fProposal && pBuf->TranslUnit.szTarget[0] && pBuf->TranslUnit.szSource[0] )
        {
          PMD_PROPOSAL_DATA pPropData = (PMD_PROPOSAL_DATA)pBuf->PropMetaData;
          PSZ_W pszBuffer = (PSZ_W)&(pPropData->Buffer);
          LONG lCurPos = 0;
          
          memset( pBuf->PropMetaData, 0, sizeof(pBuf->PropMetaData) );

          pPropData->Hdr.lSize = sizeof(MD_PROPOSAL_DATA);
          pPropData->Hdr.lSize += (wcslen(pBuf->TranslUnit.szSource) + 1) * sizeof(CHAR_W); 
          pPropData->Hdr.lSize += (wcslen(pBuf->TranslUnit.szTarget) + 1) * sizeof(CHAR_W); 
          pPropData->Hdr.lSize += (1) * sizeof(CHAR_W); 
          pPropData->Hdr.lSize += (1) * sizeof(CHAR_W); 
          pPropData->Hdr.lSize += (wcslen(this->szSourceFile) + 1) * sizeof(CHAR_W); 
   
          pPropData->Hdr.Type = MD_PROPOSAL_TYPE;

          pPropData->sQuality = 102;
          pPropData->lCreationDate = 0;
          pPropData->lModificationDate = 0;

          pPropData->lSourcePos = lCurPos;
          wcscpy( pszBuffer + lCurPos, pBuf->TranslUnit.szSource );
          lCurPos += wcslen( pBuf->TranslUnit.szSource ) + 1;

          pPropData->lTargetPos = lCurPos;
          wcscpy( pszBuffer + lCurPos, pBuf->TranslUnit.szTarget );
          lCurPos += wcslen( pBuf->TranslUnit.szTarget ) + 1;

          pPropData->lAuthorPos = lCurPos;
          wcscpy( pszBuffer + lCurPos, L"" ); // pBuf->Proposal.szAuthor );
          lCurPos += 1;

          pPropData->lModifierPos = lCurPos;
          wcscpy( pszBuffer + lCurPos, L"" ); // pBuf->Proposal.szModifier );
          lCurPos +=  1;

          pPropData->lDocNamePos = lCurPos;
          wcscpy( pszBuffer + lCurPos, this->szSourceFile );
          lCurPos += wcslen( this->szSourceFile ) + 1;

          MDAddElement( &(pBuf->TranslUnit.pMetaData), (PMD_ELEMENT_HDR)pPropData );
        } /* endif */
        break;

      case NOTE_ELEMENT:
        {
          // create a new notelist area if none yet
          if ( pBuf->TranslUnit.pNoteList == NULL )
          {
            if ( UtlAlloc( (PVOID *)&(pBuf->TranslUnit.pNoteList), 0, sizeof(MD_NOTELIST_DATA), ERROR_STORAGE ) )
            {
              pBuf->TranslUnit.pNoteList->Hdr.lSize = sizeof(MD_NOTELIST_DATA);
              pBuf->TranslUnit.pNoteList->Hdr.Type = MD_NOTELIST_TYPE;
              pBuf->TranslUnit.pNoteList->lNumberOfNotes = 0;
            } /* endif */
          } /* endif */

          // enlarge notelist area and add new note 
          if ( pBuf->TranslUnit.pNoteList )
          {
            int iTextLen = wcslen( pBuf->szData );
            int iNoteSize = sizeof(MD_NOTE_DATA) + ((iTextLen + 1) * sizeof(CHAR_W));

            if ( UtlAlloc( (PVOID *)&(pBuf->TranslUnit.pNoteList), pBuf->TranslUnit.pNoteList->Hdr.lSize, 
                           pBuf->TranslUnit.pNoteList->Hdr.lSize + iNoteSize, ERROR_STORAGE ) )
            {
              PMD_NOTE_DATA pNote = (PMD_NOTE_DATA)(((PBYTE)pBuf->TranslUnit.pNoteList) + pBuf->TranslUnit.pNoteList->Hdr.lSize);
              pNote->lSize = iNoteSize;
              wcsncpy( pNote->szAuthor, pBuf->szNoteAuthor, (sizeof(pNote->szAuthor)/sizeof(CHAR_W)) - 1 );
              wcscpy( pNote->szNote, pBuf->szData );
              pBuf->TranslUnit.pNoteList->lNumberOfNotes += 1;
              pBuf->TranslUnit.pNoteList->Hdr.lSize      += iNoteSize;
            } /* endif */
          } /* endif */

          fCatchData = FALSE;

        }
        break;

      case TRANS_UNIT_ELEMENT:
        // setup translation unit text
        swprintf( pBuf->SegmentData.szSource, L"<TranslationUnit id=\"%lu\"", pBuf->TranslUnit.ulNum );
        if ( pBuf->FileInfo.szFile[0] ) swprintf( pBuf->SegmentData.szSource + wcslen(pBuf->SegmentData.szSource), L" doc=\"%s\"", pBuf->FileInfo.szFile );
        wcscat( pBuf->SegmentData.szSource, L">" );

        // write translation unit to our output files
        fwprintf( hfSource, L"%s\r\n", pBuf->SegmentData.szSource );
        fwprintf( hfSource, L"%s\r\n", pBuf->TranslUnit.szSource );

        // nop segment containing translation unit number
        if ( m_ulSegNum != 1 )
        {
          fwprintf( hfSegSource, L":qfn n=%lu.\r\n%s\r\n:eqfn.", m_ulSegNum, pBuf->SegmentData.szSource );
          fwprintf( hfSegTarget, L":qfn n=%lu.\r\n%s\r\n:eqfn.", m_ulSegNum, pBuf->SegmentData.szSource );
        }
        else
        {
          fwprintf( hfSegSource, L":qfn n=%lu.%s\r\n:eqfn.", m_ulSegNum, pBuf->SegmentData.szSource );
          fwprintf( hfSegTarget, L":qfn n=%lu.%s\r\n:eqfn.", m_ulSegNum, pBuf->SegmentData.szSource );
        } /* endif */

        m_ulSegNum++;

        // translatable segment containing the actual data
        fwprintf( hfSegSource, L":qff n=%lu.%s:eqff.", m_ulSegNum, pBuf->TranslUnit.szSource );
        if ( pBuf->TranslUnit.szTarget[0] && !pBuf->TranslUnit.fPlaceHoldersOnly )
        {
          CHAR_W szXAttr[40];                    // buffer for X= attribute
          COUNTFLAG CountFlag;
          USHORT usSrcWords = (USHORT)pBuf->TranslUnit.lSourceWords;
          USHORT usTgtWords = 0;
          USHORT usModWords = 0;

          memset( &CountFlag, 0, sizeof(CountFlag) );
          CountFlag.AnalAutoSubst = TRUE;

          swprintf( szXAttr, L"%4.4X%4.4X%4.4X%4.4X", *((PUSHORT)(&CountFlag)), usSrcWords, usTgtWords, usModWords );

          fwprintf( hfSegTarget, L":qfx n=%lu x=%s.%s:eqfx.", m_ulSegNum, szXAttr, pBuf->TranslUnit.szTarget );

          this->lTranslatedSegs++;
        }
        else
        {
          fwprintf( hfSegTarget, L":qff n=%lu.%s:eqff.", m_ulSegNum, pBuf->TranslUnit.szSource );
        } /* endif */
        this->lTotalSegs++;

        // segment status
        if ( pBuf->TranslUnit.fApproved || pBuf->TranslUnit.szState[0] )
        {
          MD_STATUS_DATA Status;

          memset( &Status, 0, sizeof(Status) );

          Status.Hdr.lSize = sizeof(Status);
          Status.Hdr.Type = MD_STATUS_TYPE;
          Status.fApproved = pBuf->TranslUnit.fApproved;
          wcsncpy( Status.szStatus, pBuf->TranslUnit.szState, sizeof(pBuf->TranslUnit.szState)/sizeof(CHAR_W) - 1 );
          MDAddElement( &(pBuf->TranslUnit.pMetaData), (PMD_ELEMENT_HDR)&Status );
        } /* endif */

        // notes
        if ( pBuf->TranslUnit.pNoteList )
        {
          MDAddElement( &(pBuf->TranslUnit.pMetaData), (PMD_ELEMENT_HDR)pBuf->TranslUnit.pNoteList );
          UtlAlloc( (PVOID *)&(pBuf->TranslUnit.pNoteList), 0, 0, NOMSG );
        } /* endif */

        // segment metadata
        if ( pBuf->TranslUnit.pMetaData )
        {
          pBuf->TranslUnit.pMetaData->ulSegNum = m_ulSegNum;
          fwrite( pBuf->TranslUnit.pMetaData, 1, pBuf->TranslUnit.pMetaData->lSize, hfMetaData );
          MDFreeMetaData( &(pBuf->TranslUnit.pMetaData) );
          pBuf->TranslUnit.pMetaData = NULL;
        } /* endif */

        m_ulSegNum++;

        break;

      case ALT_TRANS_ELEMENT:
        CurElement.fProposal = FALSE;

        // add proposal metadata to metadata list for this translation unit
        if ( pBuf->Proposal.szTarget[0] )
        {
          PMD_PROPOSAL_DATA pPropData = (PMD_PROPOSAL_DATA)pBuf->PropMetaData;
          PSZ_W pszBuffer = (PSZ_W)&(pPropData->Buffer);
          LONG lCurPos = 0;
          
          memset( pBuf->PropMetaData, 0, sizeof(pBuf->PropMetaData) );

          pPropData->Hdr.lSize = sizeof(MD_PROPOSAL_DATA);
          pPropData->Hdr.lSize += (wcslen(pBuf->Proposal.szSource) + 1) * sizeof(CHAR_W); 
          pPropData->Hdr.lSize += (wcslen(pBuf->Proposal.szTarget) + 1) * sizeof(CHAR_W); 
          pPropData->Hdr.lSize += (wcslen(pBuf->Proposal.szAuthor) + 1) * sizeof(CHAR_W); 
          pPropData->Hdr.lSize += (wcslen(pBuf->Proposal.szModifier) + 1) * sizeof(CHAR_W); 
          pPropData->Hdr.lSize += (wcslen(pBuf->Proposal.szDocName) + 1) * sizeof(CHAR_W); 
   
          pPropData->Hdr.Type = MD_PROPOSAL_TYPE;

          pPropData->sQuality = pBuf->Proposal.sQuality;
          pPropData->lCreationDate = pBuf->Proposal.lCreationDate;
          pPropData->lModificationDate = pBuf->Proposal.lModificationDate;

          pPropData->lSourcePos = lCurPos;
          wcscpy( pszBuffer + lCurPos, pBuf->Proposal.szSource );
          lCurPos += wcslen( pBuf->Proposal.szSource ) + 1;

          pPropData->lTargetPos = lCurPos;
          wcscpy( pszBuffer + lCurPos, pBuf->Proposal.szTarget );
          lCurPos += wcslen( pBuf->Proposal.szTarget ) + 1;

          pPropData->lAuthorPos = lCurPos;
          wcscpy( pszBuffer + lCurPos, pBuf->Proposal.szAuthor );
          lCurPos += wcslen( pBuf->Proposal.szAuthor ) + 1;

          pPropData->lModifierPos = lCurPos;
          wcscpy( pszBuffer + lCurPos, pBuf->Proposal.szModifier );
          lCurPos += wcslen( pBuf->Proposal.szModifier ) + 1;

          pPropData->lDocNamePos = lCurPos;
          wcscpy( pszBuffer + lCurPos, pBuf->Proposal.szDocName );
          lCurPos += wcslen( pBuf->Proposal.szDocName ) + 1;

          MDAddElement( &(pBuf->TranslUnit.pMetaData), (PMD_ELEMENT_HDR)pPropData );

        } /* endif */
        break;

      case PH_ELEMENT:
        CurElement.fInlineTagging = FALSE;
        break; 

      case IWS_ATTRIBUTE_ELEMENT:
        switch ( CurElement.AttrID )
        {
          case MODIFIED_ON_ATTR :
          case CREATED_ON_ATTR :
            {
              // assumption: the date/time is in long form plus 3 trailing digits (msecs???)
              // s we cut-off the last three digits and use the remaining value as long time value
              int iLen = wcslen( pBuf->szProp );
              if ( iLen > 3 )
              {
                pBuf->szProp[iLen-3] = 0; 
                if ( CurElement.AttrID == MODIFIED_ON_ATTR )
                {
                  pBuf->Proposal.lModificationDate = _wtol( pBuf->szProp );
                }
                else if ( CurElement.AttrID == CREATED_ON_ATTR )
                {
                  pBuf->Proposal.lCreationDate = _wtol( pBuf->szProp );
                } /* endif */
              } /* endif */
            }
            break;
          case MODIFIED_BY_ATTR :
            pBuf->szProp[sizeof(pBuf->Proposal.szModifier) - 1] = NULC;
            wcscpy( pBuf->Proposal.szModifier, pBuf->szProp );
            break;
          case CREATED_BY_ATTR :
            pBuf->szProp[sizeof(pBuf->Proposal.szAuthor) - 1] = NULC;
            wcscpy( pBuf->Proposal.szAuthor, pBuf->szProp );
            break;
          default:
            break;
        } /*endswitch */
        CurElement.AttrID = UNKNOWN_ATTR;
        break; 

      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */
  } /* endif */

  Pop( &CurElement );
}

void IdiomXLIFFParseHandler::characters(const XMLCh* const chars, const XMLSize_t /*unsigned int*/ length)
{
  PSZ_W pszChars = (PSZ_W)chars;

  if ( hfLog ) fprintf( hfLog, "%ld Characters; \"%S\"\n", length, pszChars );

  if ( ProcessingMode == EXPORT_MODE )
  {
    // in passthru mode, pass everything to output file
    if ( pxw && fPassThru )
    {
      pxw->WriteString( pszChars );
    }
    else
    {
      // some string of other elements are also written when not in passthru mode
      switch ( CurElement.ID )
      {
        case IWS_BOUNDARY_SEG_ELEMENT:
        case IWS_MARKUP_SEG_ELEMENT:
          pxw->WriteString( pszChars );
        default:
            break;
      } /*endswitch */
    } /* endif */
  }
  else
  {
    if ( (CurElement.ID == IWS_ATTRIBUTE_ELEMENT) && (CurElement.AttrID != UNKNOWN_ATTR) )
    {
      // store data in attribute value buffer
      wcsncpy( pBuf->szProp, pszChars, (sizeof(pBuf->szProp)/sizeof(CHAR_W)) - 1 ) ;
      pBuf->szProp[sizeof(pBuf->szProp) - 1] = 0;
    }
    else if ( fCatchData && !CurElement.fInlineTagging )
    {
      // add data to current data buffer 
      int iCurLen = wcslen( pBuf->szData );
      if ( (iCurLen + length + 1) < DATABUFFERSIZE)
      {
        wcsncpy( pBuf->szData + iCurLen, pszChars, length );
        pBuf->szData[iCurLen+length] = 0;
      } /* endif */
    } /* endif */
  } /* endif */
}

void IdiomXLIFFParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    if ( hfLog ) fprintf( hfLog, "Fatal Error: %s\n", message );
    XMLString::release( &message );
}

void IdiomXLIFFParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    if ( hfLog ) fprintf( hfLog, "Error: %s\n", message );
    XMLString::release( &message );
}

void IdiomXLIFFParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    if ( hfLog ) fprintf( hfLog, "Warning: %s\n", message );
    XMLString::release( &message );
}


// get the ID for a TMX element
ELEMENTID IdiomXLIFFParseHandler::GetElementID( PSZ_W pszName )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;

  while ( (IDFound == UNKNOWN_ELEMENT) && (XLIFFNameToID[i].szName[0] != 0) )
  {
    if ( wcsicmp( pszName, XLIFFNameToID[i].szName ) == 0 )
    {
      IDFound = XLIFFNameToID[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method IdiomXLIFFParseHandler::GetElementID */

void IdiomXLIFFParseHandler::Push( PXLIFFELEMENT pElement )
{
  // enlarge stack if necessary
  if ( iCurElement >= iStackSize )
  {
    pStack = (PXLIFFELEMENT)realloc( pStack, (iStackSize + 5) * sizeof(XLIFFELEMENT) );
    iStackSize += 5;
  } /* endif */

  // add element to stack
  if ( pStack )
  {
    memcpy( pStack + iCurElement, pElement, sizeof(XLIFFELEMENT) );
    iCurElement++;
  } /* endif */

  return;
} /* end of method IdiomXLIFFParseHandler::Push */

void IdiomXLIFFParseHandler::Pop( PXLIFFELEMENT pElement )
{
  if ( pStack && iCurElement )
  {
    iCurElement--;
    memcpy( pElement, pStack + iCurElement, sizeof(XLIFFELEMENT) );
  } /* endif */
  return;
} /* end of method IdiomXLIFFParseHandler::Pop */

void IdiomXLIFFParseHandler::SetNameLists( CNameList *pLangList, CNameList *pTypeList )
{
  m_Util->SetNameLists( pLangList, pTypeList );
} /* end of method IdiomXLIFFParseHandler::SetNameLists */

//
// set a file handle
//
void IdiomXLIFFParseHandler::SetFile( USHORT usFileID, FILE *hf )
{
  switch ( usFileID )
  {
    case DIRSOURCEDOC_PATH    : hfSource = hf; break;
    case DIRSEGSOURCEDOC_PATH : hfSegSource = hf; break;
    case DIRSEGTARGETDOC_PATH : hfSegTarget = hf; break;
    case METADATA_PATH        : hfMetaData = hf; break;
    default: break;
  } /*endswitch */
}

//
// set the XML writer to be used for XML output
//
void IdiomXLIFFParseHandler::SetXMLWriter( CXmlWriter *pInXW )
{
  pxw = pInXW;
}

void IdiomXLIFFParseHandler::SetMode( XLIFFPROCESSMODE mode )
{
  ProcessingMode = mode;
  if ( mode == EXPORT_MODE)
  {
    fPassThru = TRUE;
  } /* endif */
}

void IdiomXLIFFParseHandler::SetLoadedDocs( PTBDOCUMENT pInTargetDoc, PTBDOCUMENT pInSourceDoc )
{
  pSourceDoc = pInSourceDoc;
  pTargetDoc = pInTargetDoc;
  ulSegNum = 1;
}

void IdiomXLIFFParseHandler::SetLanguagePointer( PSZ pszInSourceLanguage, PSZ pszInTargetLanguage )
{
  this->pszSourceLanguage = pszInSourceLanguage;
  this->pszTargetLanguage = pszInTargetLanguage;
}

void IdiomXLIFFParseHandler::SetSourceFileName( PSZ pszFileName )
{
  MultiByteToWideChar( CP_OEMCP, 0, pszFileName, -1, this->szSourceFile, sizeof(this->szSourceFile)/sizeof(CHAR_W) );
}

// extract a numeric value from a string
BOOL IdiomXLIFFParseHandler::GetValue( PSZ pszString, int iLen, int *piResult )
{
  BOOL fOK = TRUE;
  char szNumber[10];
  char *pszNumber = szNumber;

  *piResult = 0;

  while ( iLen && fOK )
  {
    if ( isdigit(*pszString) )
    {
      *pszNumber++ = *pszString++;
      iLen--;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /*endwhile */

  if ( fOK )
  {
    *pszNumber = '\0';
    *piResult = atoi( szNumber );
  } /* endif */

  return( fOK );
} /* end of method IdiomXLIFFParseHandler::GeValue */

void IdiomXLIFFParseHandler::SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsert, LONG lHandle )
{
  pfnInsertSegment = pfnInsert;
  lMemHandle = lHandle;
} /* end of method IdiomXLIFFParseHandler::SetMemInterface */

void IdiomXLIFFParseHandler::GetDescription( char *pszDescription, int iBufSize )
{
  *pszDescription = 0;

  if ( pBuf && pBuf->szDescription[0] )
  {
    strncpy( pszDescription, pBuf->szDescription, iBufSize );
    pszDescription[iBufSize-1] = 0;
  } /* endif */
} /* end of method IdiomXLIFFParseHandler::GetDescription */

void IdiomXLIFFParseHandler::SetSourceLanguage( char *pszSourceLang)
{
  if ( pBuf )
  {
    strcpy( pBuf->szMemSourceLang, pszSourceLang);
  } /* endif */
} /* end of method IdiomXLIFFParseHandler::SetSourceLanguage */

void IdiomXLIFFParseHandler::GetSourceLanguage( char *pszSourceLang, int iBufSize )
{
  *pszSourceLang = 0;

  if ( pBuf && pBuf->szLang[0] )
  {
    strncpy( pszSourceLang, pBuf->szLang, iBufSize );
    pszSourceLang[iBufSize-1] = 0;
  } /* endif */
} /* end of method IdiomXLIFFParseHandler::GetSourceLanguage */

BOOL IdiomXLIFFParseHandler::IsHeaderDone()
{
 return( fHeaderDone );
}


//
// Write a translation unit from the data in the segmented TM file
//
int IdiomXLIFFParseHandler::WriteTransUnitFromSegmentData
( 
  PSZ_W       pszFileName,
  LONG        lTranslUnit
)
{
  int iRC = 0;
  BOOL fFound = FALSE;

  // find segment belonging to this translation unit
  PTBSEGMENT pSeg = EQFBGetSeg( pTargetDoc, ulSegNum );
  while ( pSeg && !fFound )
  {
    if ( (pSeg->pDataW != NULL) && (pSeg->qStatus == QF_NOP) )
    {
      PSZ_W pszStart = pSeg->pDataW;

      while ( (*pszStart == L'\r') || (*pszStart == L'\n') ) pszStart++;

      if ( wcsnicmp( pszStart, L"<TranslationUnit", 16 ) == 0 )
      {
        fFound = TRUE;

        // compare file (=document) name
        if ( pBuf->FileInfo.szFile[0] )
        {
          this->GetAttrValue( pszStart, L"doc", pBuf->szData );
          if ( wcscmp( pBuf->FileInfo.szFile, pBuf->szData ) != 0 )
          {
            fFound = FALSE;
          } /* endif */
        } /* endif */

        // compare translation unit ID
        if ( fFound )
        {
          this->GetAttrValue( pszStart, L"id", pBuf->szData );
          ULONG ulID = _wtol( pBuf->szData );
          if ( ulID != pBuf->TranslUnit.ulNum )
          {
            fFound = FALSE;
          } /* endif */
        } /* endif */
      } /* endif */

      // position to next segment if correct unit has been found
      if ( fFound )
      {
        ulSegNum++;
        pSeg = EQFBGetSeg( pTargetDoc, ulSegNum );
      } /* endif */
    } /* endif */

    if ( !fFound )
    {
      ulSegNum++;
      pSeg = EQFBGetSeg( pTargetDoc, ulSegNum );
    } /* endif */
  } /*endwhile */

  // write any translation unit attribues from metadata first
  if ( fFound && pSeg && pSeg->pvMetadata )
  {
    PMD_ELEMENT_HDR pElement = MDGetFirstElement( (PMD_METADATA)pSeg->pvMetadata );
    while ( pElement->Type != MD_ENDOFLIST_TYPE )
    {
      // we are only interested in status information
      if ( pElement->Type == MD_STATUS_TYPE )
      {
        PMD_STATUS_DATA pStatus = (PMD_STATUS_DATA)pElement; 
        if ( pStatus->fApproved )
        {
          pxw->WriteAttributeString( L"approved", L"yes" );
        } /* endif */
        break;
      } /*endif */
      pElement = MDGetNextElement( pElement );
    } /*endwhile */
  } /* endif */

  // write source and any segmented target to output file
  if ( fFound && pSeg )
  {
    PTBSEGMENT pSourceSeg = EQFBGetSeg( pSourceDoc, ulSegNum );
    if ( pSourceSeg )
    {
      pxw->WriteStartElement( L"source" );
      this->WriteSegmentText( pSourceSeg->pDataW );
      pxw->WriteEndElement();

      pxw->WriteStartElement( L"target" );
      if ( pSeg->qStatus == QF_XLATED )
      {
        this->WriteSegmentText( pSeg->pDataW );
      } /* endif */
      pxw->WriteEndElement();
    } /* endif */

    // write segment metadata
    if ( pSeg->pvMetadata )
    {
      PMD_ELEMENT_HDR pElement = MDGetFirstElement( (PMD_METADATA)pSeg->pvMetadata );
      while ( pElement->Type != MD_ENDOFLIST_TYPE )
      {
        switch ( pElement->Type )
        {
          case MD_NOTELIST_TYPE:
            {
              PMD_NOTELIST_DATA pNoteList = (PMD_NOTELIST_DATA)pElement; 
              PMD_NOTE_DATA pNote = (PMD_NOTE_DATA)(((PBYTE)(pNoteList)) + sizeof(MD_NOTELIST_DATA));
              int i = 0;
              while ( i < pNoteList->lNumberOfNotes )
              {
                // write current note to output file
                pxw->WriteStartElement( L"note" );
                if ( pNote->szAuthor[0] )
                {
                  pxw->WriteAttributeString( L"from", pNote->szAuthor );
                } /* endif */
                pxw->WriteString( pNote->szNote );
                pxw->WriteEndElement();

                // continue with next note
                pNote = (PMD_NOTE_DATA)( ((PBYTE)pNote) + pNote->lSize);
                i++;
              } /*endwhile */
            }
            break;

          case MD_HISTORY_TYPE:
            {
              PMD_HIST_DATA pHistory = (PMD_HIST_DATA)pElement; 

              pxw->WriteStartElement( L"iws:attribute" );
              pxw->WriteAttributeString( L"name", L"Modified On" );
              this->WriteDateAsNumber( pHistory->lModificationDate  );
              pxw->WriteInt( pHistory->lModificationDate );
              pxw->WriteEndElement();

              pxw->WriteStartElement( L"iws:attribute" );
              pxw->WriteAttributeString( L"name", L"Created On" );
              this->WriteDateAsNumber( pHistory->lCreationDate );
              pxw->WriteEndElement();

              pxw->WriteStartElement( L"iws:attribute" );
              pxw->WriteAttributeString( L"name", L"Created By" );
              pxw->WriteString( pHistory->szCreator );
              pxw->WriteEndElement();

              pxw->WriteStartElement( L"iws:attribute" );
              pxw->WriteAttributeString( L"name", L"Modified By" );
              pxw->WriteString( pHistory->szModifier );
              pxw->WriteEndElement();
            }
            break;

          case MD_STATUS_TYPE:
            {
              // has already bee processed..
            }
            break;

          case MD_PROPOSAL_TYPE:
            {
              PMD_PROPOSAL_DATA pProposal = (PMD_PROPOSAL_DATA)pElement;
              if ( pProposal->sQuality <= 100 )  // do not write the exact-exact proposals created during import        
              {
                pxw->WriteStartElement( L"alt-trans" );
                if ( pProposal->sQuality == 100 )
                {
                  pxw->WriteAttributeString( L"match-quality", L"1.00" );
                }
                else
                {
                  CHAR_W szTemp[10];
                  swprintf( szTemp, L"0.%2.2d", pProposal->sQuality );
                  pxw->WriteAttributeString( L"match-quality", szTemp );
                } /* endif */

                pxw->WriteStartElement( L"source" );
                this->WriteSegmentText( ((PSZ_W)pProposal->Buffer) + pProposal->lSourcePos );
                pxw->WriteEndElement();

                pxw->WriteStartElement( L"target" );
                this->WriteSegmentText( ((PSZ_W)pProposal->Buffer) + pProposal->lTargetPos );
                pxw->WriteEndElement();

                pxw->WriteStartElement( L"iws:attribute" );
                pxw->WriteAttributeString( L"name", L"Modified On" );
                this->WriteDateAsNumber( pProposal->lModificationDate  );
                pxw->WriteEndElement();

                pxw->WriteStartElement( L"iws:attribute" );
                pxw->WriteAttributeString( L"name", L"Created On" );
                this->WriteDateAsNumber( pProposal->lCreationDate  );
                pxw->WriteEndElement();

                pxw->WriteStartElement( L"iws:attribute" );
                pxw->WriteAttributeString( L"name", L"Created By" );
                pxw->WriteString( ((PSZ_W)pProposal->Buffer) + pProposal->lAuthorPos );
                pxw->WriteEndElement();

                pxw->WriteStartElement( L"iws:attribute" );
                pxw->WriteAttributeString( L"name", L"Modified By" );
                pxw->WriteString( ((PSZ_W)pProposal->Buffer) + pProposal->lModifierPos );
                pxw->WriteEndElement();
                pxw->WriteEndElement();
              } /* endif */
            }
            break;

          default:
            break;
        } /*endswitch */
        pElement = MDGetNextElement( pElement );
      } /*endwhile */

    } /* endif */
  } /* endif */

  // next time restart at first segment if segment not found
  if ( !pSeg )
  {
    ulSegNum = 1;
  } /* endif */

  return( iRC );
}

//
// Write a source or target text to output file and handle inline tags
//
// caution: uses pBuf->szData for string processing
//
int IdiomXLIFFParseHandler::WriteSegmentText( PSZ_W pszText )
{
  int iRC = 0;

  // replace inline tagging withing segment with Idiom like inline tagging
  {
    BOOL fTagging = FALSE;
    pBuf->szData[0] = 0;

    // process segment text
    while ( *pszText )
    {
      PSZ_W pszTagStart = pszText;
      PSZ_W pszPHStart = NULL;
      PSZ_W pszPHEnd = NULL;
      PSZ_W pszEPHStart = NULL;
      PSZ_W pszEPHEnd = NULL;
      fTagging = FALSE;

      // look for tag start end end sequences
      pszPHStart = FindTag( pszTagStart, L"<PH" );
      if ( pszPHStart )    pszPHEnd = wcschr( pszPHStart, L'>' );
      if ( pszPHEnd )      pszEPHStart = FindTag( pszPHEnd + 1, L"</PH" );  
      if ( pszEPHStart )   pszEPHEnd = wcschr( pszEPHStart, L'>' );  

      // process tagging or add rest of segment text to segment buffer
      if ( pszEPHEnd )
      {
        CHAR_W szPlaceHolder[10];

        // get tagging ID
        int id = 0;
        PSZ_W pszID;
        
        *pszPHEnd = 0;
        pszID = wcsstr( pszPHStart, L"id=" );
        if ( pszID ) id = _wtoi( pszID + 3 );
        *pszPHEnd = L'>';

        // write any text before the tagging to the output file
        if ( pszPHStart != pszText )
        {
          *pszPHStart = 0;
          pxw->WriteString( pszText );
          *pszPHStart = L'<';
        } /* endif */

        // check if tagging is already in our tagging list
        PSZ_W pszTagging = m_pTagging->get( id );

        pxw->Formatting = CXmlWriter::None;
        pxw->WriteStartElement( "ph" );
        pxw->WriteStartAttribute( "id" );
        pxw->WriteInt( id );
        pxw->WriteEndAttribute();
        if ( pszTagging == NULL )
        {
          *pszEPHStart = 0;
          m_pTagging->add( id, pszPHEnd + 1 );
          pxw->WriteAttributeString( L"x", pszPHEnd + 1 );
          *pszEPHStart = L'<';
        } /* endif */

        swprintf( szPlaceHolder, L"{%ld}", id );
        pxw->WriteString( szPlaceHolder );

        pxw->WriteEndElement();

        pxw->Formatting = CXmlWriter::Indented;

        pszText = pszEPHEnd + 1;
      }
      else
      {
        wcscpy( pBuf->szData, pszText );
        pszText += wcslen(pszText);

        // truncate any traling CRLF
        {
          int iLen = wcslen( pBuf->szData );
          if ( iLen )
          {
            if ( pBuf->szData[iLen-1] == L'\n' )
            {
              iLen--;
              pBuf->szData[iLen] = 0;
            } /* endif */
          } /* endif */
          if ( iLen )
          {
            if ( pBuf->szData[iLen-1] == L'\r' )
            {
              iLen--;
              pBuf->szData[iLen] = 0;
            } /* endif */
          } /* endif */
        }

        // write to output file
        pxw->WriteString( pBuf->szData );
      } /* endif */
    } /*endwhile */
  }


  return( iRC );
}

//
// find the given tag
//
PSZ_W IdiomXLIFFParseHandler::FindTag( PSZ_W pszText, PSZ_W pszTag )
{
  int iLen = wcslen(pszTag) ;

  while( *pszText )
  {
    if ( (*pszText == *pszTag) && (wcsnicmp( pszText, pszTag, iLen ) == 0) )
    {
      return( pszText );
    } /* endif */
    pszText++;
  } /*endwhile */
  return( NULL );
}

// Write a long date as integer value to output file
//
int IdiomXLIFFParseHandler::WriteDateAsNumber( LONG lDate )
{
  CHAR_W szDate[20];
  int iRC = 0;

  _ltow( lDate, szDate, 10 );
  wcscat( szDate, L"000" );
  this->pxw->WriteString( szDate );

  return( iRC );
}

BOOL IdiomXLIFFParseHandler::GetAttrValue( PSZ_W pszString, PSZ_W pszAttrName, PSZ_W pszBuffer )
{
  BOOL fFound = FALSE;
  PSZ_W pszPos = pszString;

  // preset target buffer
  *pszBuffer = 0;

  // search attribute name in string
  int iLen = wcslen( pszAttrName );

  // loop over string and examine attributes
  while ( *pszPos && !fFound )
  {
    // look for next blank (which is the attributes delimiter)
    while ( *pszPos && (*pszPos != L' ') ) pszPos++;

    // test attribute name 
    if ( *pszPos )
    {
      pszPos++;
      if ( wcsnicmp( pszPos, pszAttrName, iLen ) == 0 )
      {
        if ( pszPos[iLen] == L'=' )
        {
          pszPos += iLen + 1;
          if ( *pszPos == L'\"' )
          {
            pszPos++;
            while ( *pszPos && (*pszPos != L'\"'))
            {
              *pszBuffer++ = *pszPos++;
            } /*endwhile */
            *pszBuffer = 0;
            fFound = TRUE;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /*endwhile */

  return( fFound );
}
