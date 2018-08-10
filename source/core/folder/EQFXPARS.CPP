//+----------------------------------------------------------------------------+
//| EQFXPARS.CPP                                                               |
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
//| Description: Parser for XLIFF Folder Files                                 |
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
XLIFFNAMETOID FolXliffNameToID[] =
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
  { L"iws:project-attribute-metadata", IWS_PROJECT_ATTRIBUTE_METADATA_ELEMENT },
  { L"iws:project-attributes",         IWS_PROJECT_ATTRIBUTES },
  { L"iws:scoping-configuration",      IWS_SCOPING_CONFIGURATION_ELEMENT },
  { L"iws:segment-metadata",           IWS_SEGMENT_METADATA_ELEMENT },
  { L"iws:status",                     IWS_STATUS_ELEMENT },
  { L"iws:tm-attribute-metadata",      IWS_TM_ATTRIBUTE_METADATA_ELEMENT },
  { L"note",                           NOTE_ELEMENT },
  { L"ph",                             PH_ELEMENT },
  { L"source",                         SOURCE_ELEMENT },
  { L"sub",                            SUB_ELEMENT },
  { L"target",                         TARGET_ELEMENT },
  { L"tmgr:alias",                     TM_ALIAS_ELEMENT },
  { L"tmgr:conversion",                TM_CONVERSION_ELEMENT },
  { L"tmgr:coordinator",               TM_COORDINATOR_ELEMENT },
  { L"tmgr:coordinatoremail",          TM_COORDINATOEMAIL_ELEMENT },
  { L"tmgr:description",               TM_DESCRIPTION_ELEMENT },
  { L"tmgr:dict",                      TM_DICT_ELEMENT },
  { L"tmgr:editor",                    TM_EDITOR_ELEMENT },
  { L"tmgr:expemail",                  TM_EXPEMAIL_ELEMENT },
  { L"tmgr:exporigionator",            TM_EXPORIGIONATOR_ELEMENT },
  { L"tmgr:folder",                    TM_FOLDER_ELEMENT },
  { L"tmgr:longname",                  TM_LONGNAME_ELEMENT },
  { L"tmgr:markup",                    TM_MARKUP_ELEMENT },
  { L"tmgr:memory",                    TM_MEMORY_ELEMENT },
  { L"tmgr:orglongname",               TM_ORGLONGNAME_ELEMENT },
  { L"tmgr:orgname",                   TM_ORGNAME_ELEMENT },
  { L"tmgr:parent",                    TM_PARENT_ELEMENT },
  { L"tmgr:parentemail",               TM_PARENTEMAIL_ELEMENT },
  { L"tmgr:prevversion",               TM_PREVVERSION_ELEMENT },
  { L"tmgr:product",                   TM_PRODUCT_ELEMENT },
  { L"tmgr:productfamily",             TM_PRODUCTFAMILY_ELEMENT },
  { L"tmgr:properties",                TM_PROPERTIES_ELEMENT },
  { L"tmgr:romem",                     TM_ROMEM_ELEMENT },
  { L"tmgr:segstatus",                 TM_SEGSTATUS_ELEMENT },
  { L"tmgr:shipment",                  TM_SHIPMENT_ELEMENT },
  { L"tmgr:shortname",                 TM_SHORTNAME_ELEMENT },
  { L"tmgr:similarproduct",            TM_SIMILARPRODUCT_ELEMENT },
  { L"tmgr:sourcelang",                TM_SOURCELANG_ELEMENT },
  { L"tmgr:subjectdict",               TM_SUBJECTDICT_ELEMENT },
  { L"tmgr:subjectmem",                TM_SUBJECTMEM_ELEMENT },
  { L"tmgr:targetlang",                TM_TARGETLANG_ELEMENT },
  { L"tmgr:vendor",                    TM_VENDOR_ELEMENT },
  { L"tmgr:vendoremail",               TM_VENDOREMAIL_ELEMENT },
  { L"tmgr:version",                   TM_VERSION_ELEMENT },
  { L"trans-unit",                     TRANS_UNIT_ELEMENT },
  { L"xliff",                          XLIFF_ELEMENT },
  { L"",                               UNKNOWN_ELEMENT } };





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Implementation of XLIFF SAX parser                                                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FolXliffParseHandler::FolXliffParseHandler()
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

  fPassThru = FALSE;
  
  pxw = NULL; 

  ulSegNum = 1;
  lTranslatedSegs = 0;
  lTotalSegs = 0;

  fFolderCreated = FALSE;           
  fDocumentCreated = FALSE;         
  fError = FALSE;
  m_fDocChecked = FALSE;
  m_fSkipDoc = FALSE;

  pTargetDoc = NULL;
}

FolXliffParseHandler::~FolXliffParseHandler()
{
  if ( hfLog )  fclose( hfLog );
  if ( pStack ) free( pStack );
  if ( pBuf )   free( pBuf );

  delete m_pTagging;
}

void FolXliffParseHandler::startElement
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

  switch ( CurElement.ID )
  {
    case FILE_ELEMENT:
      // reset file info
      memset( &(pBuf->FileInfo), 0, sizeof(pBuf->FileInfo) );
      m_fDocChecked = FALSE;

      // get file attributes
      for( int i = 0; i < iAttribs; i++ )
      {
        PSZ_W pszName = (PSZ_W)attributes.getName( i );
        PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

        if ( pszName && pszValue )
        {
          if ( _wcsicmp( pszName, L"source-language" ) == 0 )
          {
            CHAR szTemp[20];
            wcsncpy( pBuf->FileInfo.szSourceLanguage, pszValue, (sizeof(pBuf->FileInfo.szSourceLanguage)/sizeof(CHAR_W)) - 1 ); 
            if ( pBuf->FileInfo.szTMSourceLanguage[0] == '\0' )
            {
              pBuf->FileInfo.szTMSourceLanguage[0] = '\0';  
              WideCharToMultiByte( CP_OEMCP, 0, pszValue, -1, szTemp, sizeof(szTemp)-1, NULL, NULL );
              m_Util->TMXLanguage2TMLanguage( "", szTemp, pBuf->FileInfo.szTMSourceLanguage );
            } /* endif */
          } 
          else if ( _wcsicmp( pszName, L"content" ) == 0 )
          {
            wcsncpy( pBuf->FileInfo.szContent, pszValue, (sizeof(pBuf->FileInfo.szContent)/sizeof(CHAR_W)) - 1 ); 
          } 
          else if ( _wcsicmp( pszName, L"datatype" ) == 0 )
          {
            wcsncpy( pBuf->FileInfo.szDataType, pszValue, (sizeof(pBuf->FileInfo.szDataType)/sizeof(CHAR_W)) - 1 ); 
          } 
          else if ( _wcsicmp( pszName, L"original" ) == 0 )
          {
            WideCharToMultiByte( CP_ACP, 0, pszValue, -1, pBuf->FileInfo.szFile, sizeof(pBuf->FileInfo.szFile)-1, NULL, NULL );
          } 
          else if ( _wcsicmp( pszName, L"target-language" ) == 0 )
          {
            CHAR szTemp[20];
            wcsncpy( pBuf->FileInfo.szTargetLanguage, pszValue, (sizeof(pBuf->FileInfo.szTargetLanguage)/sizeof(CHAR_W)) - 1 ); 
            if ( pBuf->FileInfo.szTMTargetLanguage[0] == '\0' )
            {
              pBuf->FileInfo.szTMTargetLanguage[0] = '\0';  
              WideCharToMultiByte( CP_OEMCP, 0, pszValue, -1, szTemp, sizeof(szTemp)-1, NULL, NULL );
              m_Util->TMXLanguage2TMLanguage( "", szTemp, pBuf->FileInfo.szTMTargetLanguage );
            } /* endif */
          } 
          else if ( _wcsicmp( pszName, L"tool" ) == 0 )
          {
            wcsncpy( pBuf->FileInfo.szTool, pszValue, (sizeof(pBuf->FileInfo.szTool)/sizeof(CHAR_W)) - 1 ); 
          } 
          else if ( _wcsicmp( pszName, L"build-num" ) == 0 )
          {
            wcsncpy( pBuf->FileInfo.szBuild, pszValue, (sizeof(pBuf->FileInfo.szBuild)/sizeof(CHAR_W)) - 1 ); 
          } /* endif */
        } /* endif */
      } /* endfor */
      break;

    case TRANS_UNIT_ELEMENT:
      // reset translation unit info
      memset( &(pBuf->TranslUnit), 0, sizeof(pBuf->TranslUnit) );
      pBuf->TranslUnit.qStatus = QF_TOBE;
      m_pTagging->clear();

      // check if folder and document info is available
      if ( m_fSkipDoc )
      {
        // ignore translation unit
      }
      else
      {
        if ( !m_fDocChecked )
        {
          CheckAndPrepareDocument();
        } /* endif */

        // get unit number and approved flag from attributes
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"id" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            pBuf->TranslUnit.ulNum = _wtol( pszValue );
          }
          else if ( _wcsicmp( pszName, L"approved" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            pBuf->TranslUnit.fApproved = (_wcsicmp( pszValue, L"yes" ) == 0);
          }
          else if ( _wcsicmp( pszName, TM_SEGSTATUS_L ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            if ( _wcsicmp( pszValue, L"NOP" ) == 0 )
            {
              pBuf->TranslUnit.qStatus = QF_NOP;
            }
            else if ( _wcsicmp( pszValue, L"XLATED" ) == 0 )
            {
              pBuf->TranslUnit.qStatus = QF_XLATED;
            }
            else if ( _wcsicmp( pszValue, L"TOBE" ) == 0 )
            {
              pBuf->TranslUnit.qStatus = QF_TOBE;
            }
            else if ( _wcsicmp( pszValue, L"ATTR" ) == 0 )
            {
              pBuf->TranslUnit.qStatus = QF_ATTR;
            }
            else if ( _wcsicmp( pszValue, L"CURRENT" ) == 0 )
            {
              pBuf->TranslUnit.qStatus = QF_CURRENT;
            }
            else
            {
            } /* endif */
          } /* endif */
        } /* endfor */
      } /* endif */
      break;
    case ALT_TRANS_ELEMENT:
      // reset proposal info
      memset( &(pBuf->Proposal), 0, sizeof(pBuf->Proposal) );
      CurElement.fProposal = TRUE;

      // get quality
      for( int i = 0; i < iAttribs; i++ )
      {
        PSZ_W pszName = (PSZ_W)attributes.getName( i );
        if ( _wcsicmp( pszName, L"match-quality" ) == 0 )
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
        if ( _wcsicmp( pszName, L"from" ) == 0 )
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
        if ( _wcsicmp( pszName, L"state" ) == 0 )
        {
          PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
          wcsncpy( pBuf->TranslUnit.szState, pszValue, (sizeof(pBuf->TranslUnit.szState)/sizeof(CHAR_W)) - 1 );
        } /* endif */
      } /* endfor */
      break;

    case SUB_ELEMENT:
    case PH_ELEMENT:
      break;
#ifdef USE_XLIFF_INLINTAGGING
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
#endif
    case IWS_ATTRIBUTE_ELEMENT:
      // check if these are proposal attributes
      if ( CurElement.fProposal )
      {
        CurElement.AttrID = UNKNOWN_ATTR;
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"name" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            if ( pszValue != NULL )
            {
              if ( _wcsicmp( pszValue, L"Modified On" ) == 0)
              {
                CurElement.AttrID  = MODIFIED_ON_ATTR;
              }
              else if ( _wcsicmp( pszValue, L"Created On" ) == 0)
              {
                CurElement.AttrID  = CREATED_ON_ATTR;
              }
              else if ( _wcsicmp( pszValue, L"Created By" ) == 0)
              {
                CurElement.AttrID  = CREATED_BY_ATTR;
              }
              else if ( _wcsicmp( pszValue, L"Modified By" ) == 0)
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
        if ( _wcsicmp( pszName, L"target_content" ) == 0 )
        {
          PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
          if ( pszValue != NULL )
          {
            if ( _wcsicmp( pszValue, L"placeholders_only" ) == 0)
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
        if ( _wcsicmp( pszName, L"ws_word_count" ) == 0 )
        {
          PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
          if ( pszValue != NULL )
          {
            if ( _wcsicmp( pszValue, L"placeholders_only" ) == 0)
            {
              pBuf->TranslUnit.lSourceWords = _wtol( pszValue );
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endfor */
      break;

    case TM_PROPERTIES_ELEMENT:
      break;

    case TM_FOLDER_ELEMENT:
    case TM_MARKUP_ELEMENT:
    case TM_MEMORY_ELEMENT:
    case TM_SOURCELANG_ELEMENT:
    case TM_TARGETLANG_ELEMENT:
    case TM_SHORTNAME_ELEMENT:
    case TM_ALIAS_ELEMENT:
    case TM_EDITOR_ELEMENT:
    case TM_CONVERSION_ELEMENT:
    case TM_VENDOR_ELEMENT:
    case TM_VENDOREMAIL_ELEMENT:
    case TM_SHIPMENT_ELEMENT:
    case TM_DESCRIPTION_ELEMENT:
    case TM_ORGNAME_ELEMENT:
    case TM_ORGLONGNAME_ELEMENT:
    case TM_LONGNAME_ELEMENT:
    case TM_COORDINATOR_ELEMENT:
    case TM_COORDINATOEMAIL_ELEMENT:
    case TM_PARENT_ELEMENT:
    case TM_PARENTEMAIL_ELEMENT:
    case TM_EXPORIGIONATOR_ELEMENT:
    case TM_EXPEMAIL_ELEMENT:
    case TM_PRODUCT_ELEMENT:
    case TM_PRODUCTFAMILY_ELEMENT:
    case TM_SIMILARPRODUCT_ELEMENT:
    case TM_SUBJECTDICT_ELEMENT:
    case TM_SUBJECTMEM_ELEMENT:
    case TM_PREVVERSION_ELEMENT:
    case TM_VERSION_ELEMENT:
    case TM_ROMEM_ELEMENT:
    case TM_DICT_ELEMENT:
    case TM_SEGSTATUS_ELEMENT:
    case UNKNOWN_ELEMENT:
      pBuf->szData[0] = 0;
      fCatchData = TRUE;
      break;

    default:
      break;
  } /*endswitch */

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

void FolXliffParseHandler::endElement(const XMLCh* const name )
{
  ELEMENTID idPrev;
  PSZ_W pszName = (PSZ_W)name;

  if ( hfLog ) fprintf( hfLog, "EndElement: %S\n", pszName );

  ELEMENTID CurrentID = CurElement.ID;

  switch ( CurrentID )
  {
    case XLIFF_ELEMENT:
      break;

    case FILE_ELEMENT:
      if ( !m_fSkipDoc )
      {
        if ( pTargetDoc )
        {
          // update document properies
          m_Util->SetDocCompletion( pBuf->FileInfo.szFolObjName, pBuf->FileInfo.szShortName, pTargetDoc );

          // write segmented target file
          m_Util->SaveSegmentedFile( pBuf->szSegTargetFile, pTargetDoc ); 
        } /* endif */

        // close any open memory
        CloseMem();

        // free document structure
        if ( pTargetDoc )
        {
          m_Util->FreeSegmentedFile( &pTargetDoc ); 
        } /* endif */


      } /* endif */

      m_fDocChecked = FALSE;
      break;

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
      if ( !m_fSkipDoc )
      {
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
      } /* endif */
      fCatchData = FALSE;
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
      if ( !m_fSkipDoc )
      {
        BOOL fTranslated = FALSE;

        // get translation state
        if ( pBuf->TranslUnit.szState[0] == 0 )
        {
          // no state given, assume unit has been translated
          fTranslated = TRUE;
        }
        else if ( (_wcsicmp( pBuf->TranslUnit.szState, L"translated" ) == 0 ) ||
                  (_wcsicmp( pBuf->TranslUnit.szState, L"final" ) == 0 ) ||
                  (_wcsicmp( pBuf->TranslUnit.szState, L"signed-off" ) == 0 ) )
        {
          fTranslated = TRUE;
        } /* endif */

        // update segment data with new value
        if ( fTranslated && !CurElement.fProposal && pBuf->TranslUnit.szTarget[0] && pBuf->TranslUnit.szSource[0] )
        {
          m_Util->UpdateSegment( pTargetDoc, pBuf->TranslUnit.ulNum, pBuf->TranslUnit.szTarget );   

          // add match to memory
          if ( pImpExpData->fWithFolderMem )
          {
            UpdateMemProposal( pBuf->TranslUnit.ulNum, pBuf->TranslUnit.szSource, pBuf->TranslUnit.szTarget,
                               pBuf->FileInfo.szMarkup, pBuf->FileInfo.szTMSourceLanguage, pBuf->FileInfo.szTMTargetLanguage, 
                               pBuf->FileInfo.szShortName, pBuf->FileInfo.szFile, pBuf->TranslUnit.lDate );
          } /* endif */
        } /* endif */
      } /* endif */
      break;

    case ALT_TRANS_ELEMENT:
      if ( !m_fSkipDoc )
      {
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
      } /* endif */
      break;

    case SUB_ELEMENT:
    case PH_ELEMENT:
//      CurElement.fInlineTagging = FALSE;
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

    case TM_MARKUP_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szMarkup, sizeof(pBuf->FileInfo.szMarkup)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_FOLDER_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szFolLongName, sizeof(pBuf->FileInfo.szFolLongName)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_MEMORY_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szMemory, sizeof(pBuf->FileInfo.szMemory)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_SOURCELANG_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szTMSourceLanguage, sizeof(pBuf->FileInfo.szTMSourceLanguage)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_TARGETLANG_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szTMTargetLanguage, sizeof(pBuf->FileInfo.szTMTargetLanguage)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_SHORTNAME_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szShortName, sizeof(pBuf->FileInfo.szShortName)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_ALIAS_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szTMTargetLanguage, sizeof(pBuf->FileInfo.szTMTargetLanguage)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_EDITOR_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szEditor, sizeof(pBuf->FileInfo.szEditor)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_CONVERSION_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szEditor, sizeof(pBuf->FileInfo.szEditor)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_VENDOR_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szVendor, sizeof(pBuf->FileInfo.szVendor)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_VENDOREMAIL_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szVendorEMail, sizeof(pBuf->FileInfo.szVendorEMail)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_SHIPMENT_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev  == TM_PROPERTIES_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FileInfo.szShipment, sizeof(pBuf->FileInfo.szShipment)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_DESCRIPTION_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szDescription, sizeof(pBuf->FolderInfo.szDescription)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_ORGNAME_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szOrgName, sizeof(pBuf->FolderInfo.szOrgName)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_ORGLONGNAME_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szOrgLongName, sizeof(pBuf->FolderInfo.szOrgLongName)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_LONGNAME_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szLongName, sizeof(pBuf->FolderInfo.szLongName)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_COORDINATOR_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szCoordinator, sizeof(pBuf->FolderInfo.szCoordinator)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_COORDINATOEMAIL_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szCoordinatorEMail, sizeof(pBuf->FolderInfo.szCoordinatorEMail)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_PARENT_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szParent, sizeof(pBuf->FolderInfo.szParent)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_PARENTEMAIL_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szParentEMail, sizeof(pBuf->FolderInfo.szParentEMail)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_EXPORIGIONATOR_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szExpOriginator, sizeof(pBuf->FolderInfo.szExpOriginator)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_EXPEMAIL_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szExpEMail, sizeof(pBuf->FolderInfo.szExpEMail)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_PRODUCT_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szProduct, sizeof(pBuf->FolderInfo.szProduct)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_PRODUCTFAMILY_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szProductFamily, sizeof(pBuf->FolderInfo.szProductFamily)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_SIMILARPRODUCT_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szSimilarProduct, sizeof(pBuf->FolderInfo.szSimilarProduct)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_SUBJECTDICT_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szSubjectDict, sizeof(pBuf->FolderInfo.szSubjectDict)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_SUBJECTMEM_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szSubjectMem, sizeof(pBuf->FolderInfo.szSubjectMem)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_PREVVERSION_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szPrevVersion, sizeof(pBuf->FolderInfo.szPrevVersion)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_VERSION_ELEMENT:
      idPrev = GetParentID();
      if ( idPrev == TM_FOLDER_ELEMENT )
      {
        WideCharToMultiByte( CP_OEMCP, 0, pBuf->szData, -1, pBuf->FolderInfo.szVersion, sizeof(pBuf->FolderInfo.szVersion)-1, NULL, NULL );
      } /* endif */
      fCatchData = FALSE;
      break;

    case TM_ROMEM_ELEMENT:
      idPrev = GetParentID();
      fCatchData = FALSE;
      break;

    case TM_DICT_ELEMENT:
      idPrev = GetParentID();
      fCatchData = FALSE;
      break;


    case UNKNOWN_ELEMENT:
    default:
      break;
  } /*endswitch */

  Pop( &CurElement );
}

void FolXliffParseHandler::characters(const XMLCh* const chars, const XMLSize_t /*unsigned int*/ length)
{
  PSZ_W pszChars = (PSZ_W)chars;

  if ( hfLog ) fprintf( hfLog, "%ld Characters; \"%S\"\n", length, pszChars );

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
}

void FolXliffParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if ( hfLog ) fprintf( hfLog, "Fatal Error: %s at column %ld in line %ld\n", message, col, line );
    this->fError = TRUE;
    sprintf( this->pBuf->szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void FolXliffParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if ( hfLog ) fprintf( hfLog, "Error: %s at column %ld in line %ld\n", message, col, line  );
    this->fError = TRUE;
    sprintf( this->pBuf->szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void FolXliffParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    if ( hfLog ) fprintf( hfLog, "Warning: %s\n", message );
    XMLString::release( &message );
}

// get the ID for a TMX element
ELEMENTID FolXliffParseHandler::GetElementID( PSZ_W pszName )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;

  if ( hfLog ) fprintf( hfLog, "GetElementID:   \"%S\"\n",pszName );

  while ( (IDFound == UNKNOWN_ELEMENT) && (FolXliffNameToID[i].szName[0] != 0) )
  {
    if ( _wcsicmp( pszName, FolXliffNameToID[i].szName ) == 0 )
    {
      IDFound = FolXliffNameToID[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method FolXliffParseHandler::GetElementID */

void FolXliffParseHandler::Push( PXLIFFELEMENT pElement )
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
} /* end of method FolXliffParseHandler::Push */

void FolXliffParseHandler::Pop( PXLIFFELEMENT pElement )
{
  if ( pStack && iCurElement )
  {
    iCurElement--;
    memcpy( pElement, pStack + iCurElement, sizeof(XLIFFELEMENT) );
  } /* endif */
  return;
} /* end of method FolXliffParseHandler::Pop */

ELEMENTID FolXliffParseHandler::GetParentID()
{
  if ( pStack && iCurElement )
  {
    PXLIFFELEMENT pPrevElement = pStack + (iCurElement - 1);
    return( pPrevElement->ID );
  } /* endif */
  return ( UNKNOWN_ELEMENT) ;
} /* end of method FolXliffParseHandler::Pop */

void FolXliffParseHandler::SetNameLists( CNameList *pLangList, CNameList *pTypeList )
{
  m_Util->SetNameLists( pLangList, pTypeList );
} /* end of method FolXliffParseHandler::SetNameLists */

//
// set a file handle
//
void FolXliffParseHandler::SetFile( USHORT usFileID, FILE *hf )
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
void FolXliffParseHandler::SetXMLWriter( CXmlWriter *pInXW )
{
  pxw = pInXW;
}

void FolXliffParseHandler::SetSourceFileName( PSZ pszFileName )
{
  MultiByteToWideChar( CP_OEMCP, 0, pszFileName, -1, this->szSourceFile, sizeof(this->szSourceFile)/sizeof(CHAR_W) );
}

// extract a numeric value from a string
BOOL FolXliffParseHandler::GetValue( PSZ pszString, int iLen, int *piResult )
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
} /* end of method FolXliffParseHandler::GeValue */

void FolXliffParseHandler::SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsert, LONG lHandle )
{
  pfnInsertSegment = pfnInsert;
  lMemHandle = lHandle;
} /* end of method FolXliffParseHandler::SetMemInterface */

void FolXliffParseHandler::GetDescription( char *pszDescription, int iBufSize )
{
  *pszDescription = 0;

  if ( pBuf && pBuf->szDescription[0] )
  {
    strncpy( pszDescription, pBuf->szDescription, iBufSize );
    pszDescription[iBufSize-1] = 0;
  } /* endif */
} /* end of method FolXliffParseHandler::GetDescription */

void FolXliffParseHandler::SetSourceLanguage( char *pszSourceLang)
{
  if ( pBuf )
  {
    strcpy( pBuf->szMemSourceLang, pszSourceLang);
  } /* endif */
} /* end of method FolXliffParseHandler::SetSourceLanguage */

void FolXliffParseHandler::GetSourceLanguage( char *pszSourceLang, int iBufSize )
{
  *pszSourceLang = 0;

  if ( pBuf && pBuf->szLang[0] )
  {
    strncpy( pszSourceLang, pBuf->szLang, iBufSize );
    pszSourceLang[iBufSize-1] = 0;
  } /* endif */
} /* end of method FolXliffParseHandler::GetSourceLanguage */

BOOL FolXliffParseHandler::IsHeaderDone()
{
 return( fHeaderDone );
}



//
// Write a source or target text to output file and handle inline tags
//
// caution: uses pBuf->szData for string processing
//
int FolXliffParseHandler::WriteSegmentText( PSZ_W pszText )
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
PSZ_W FolXliffParseHandler::FindTag( PSZ_W pszText, PSZ_W pszTag )
{
  int iLen = wcslen(pszTag) ;

  if ( hfLog ) fprintf( hfLog, "FindTag:   \"%S\"\n",pszText );

  while( *pszText )
  {
    if ( (*pszText == *pszTag) && (_wcsnicmp( pszText, pszTag, iLen ) == 0) )
    {
      return( pszText );
    } /* endif */
    pszText++;
  } /*endwhile */
  return( NULL );
}

// Write a long date as integer value to output file
//
int FolXliffParseHandler::WriteDateAsNumber( LONG lDate )
{
  CHAR_W szDate[20];
  int iRC = 0;

  _ltow( lDate, szDate, 10 );
  wcscat( szDate, L"000" );
  this->pxw->WriteString( szDate );

  return( iRC );
}

BOOL FolXliffParseHandler::GetAttrValue( PSZ_W pszString, PSZ_W pszAttrName, PSZ_W pszBuffer )
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
      if ( _wcsnicmp( pszPos, pszAttrName, iLen ) == 0 )
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

BOOL FolXliffParseHandler::ErrorOccured( void )
{
  return( this->fError );
}

void FolXliffParseHandler::GetErrorText( char *pszTextBuffer, int iBufSize )
{
  *pszTextBuffer = '\0';

  if ( this->pBuf != NULL )
  {
    if ( this->pBuf->szErrorMessage[0] != '\0' )
    {
      strncpy( pszTextBuffer, this->pBuf->szErrorMessage, iBufSize );
      pszTextBuffer[iBufSize-1] = '\0';
    } /* endif */
  } /* endif */
}

void FolXliffParseHandler::SetImpExpData( PFOLIMPEXPXLIFF pData )
{
  this->pImpExpData = pData;
}

void FolXliffParseHandler::CopyWhenFilled( PSZ pszFrom, PSZ pszTo )
{
  if ( *pszFrom )
  {
    strcpy( pszTo, pszFrom );
  } /* endif */
}

// check if folder exist
int FolXliffParseHandler::CheckFolder()
{
  int iRC = 0;

  // remove any CR LFs contained in folder name and strip leading and trailing blanks
  {
    PSZ pszSource, pszTarget;

    pszSource = pszTarget = pBuf->FileInfo.szFolLongName;
    while ( *pszSource )
    {
      if ( (*pszSource != CR) && (*pszSource != LF) )
      {
        *pszTarget++ = *pszSource++;
      } /* endif */
    } /*endwhile */
    *pszTarget = EOS;
    UtlStripBlanks( pBuf->FileInfo.szFolLongName );
  }

  // check if a folder name was contained in XLIFF file 
  if ( pBuf->FileInfo.szFolLongName[0] == EOS )
  {
    iRC = XLIFF_NO_FOLDER_NAME;
    UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 0, NULL, EQF_ERROR, pImpExpData->hwndErrMsg );
    m_fSkipDoc = TRUE;
  } /* endif */

  // check if folder exists and set folder short name and folder object name
  if ( !iRC )
  {
    BOOL fFolExists = SubFolNameToObjectName( pBuf->FileInfo.szFolLongName, pBuf->FileInfo.szFolObjName );
    if ( fFolExists )
    {
      Utlstrccpy( pBuf->FileInfo.szFolShortName, UtlGetFnameFromPath( pBuf->FileInfo.szFolObjName ), DOT );
    }
    else
    {
      PSZ pszParm = pBuf->FileInfo.szFolLongName;
      iRC = XLIFF_FOLDER_DOES_NOT_EXIST;
      UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pImpExpData->hwndErrMsg );
      m_fSkipDoc = TRUE;
    } /* endif */
  } /* endif */

  // check if a folder name was contained in XLIFF file 
  if ( pBuf->FileInfo.szFolLongName[0] == EOS )
  {
    iRC = XLIFF_NO_FOLDER_NAME;
    UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 0, NULL, EQF_ERROR, pImpExpData->hwndErrMsg );
    m_fSkipDoc = TRUE;
  } /* endif */

  // check if folder matches the selected target folder
  if ( !iRC )
  {
    if ( pImpExpData->szTargetFolder[0] == EOS  )
    {
      strcpy( pImpExpData->szTargetFolder, pBuf->FileInfo.szFolLongName );
      strcpy( pImpExpData->szTargetFolObj, pBuf->FileInfo.szFolObjName );
    }
    else if ( strcmp( pBuf->FileInfo.szFolLongName, pImpExpData->szTargetFolder ) != 0 )
    {
      PSZ pszParm = pBuf->FileInfo.szFolLongName;
      iRC = XLIFF_FOLDER_DOES_NOT_EXIST;
      UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pImpExpData->hwndErrMsg );
      m_fSkipDoc = TRUE;
    } /* endif */
  } /* endif */

  // get folder memory
  if ( !iRC )
  {
    FolQueryInfo2( pImpExpData->szTargetFolObj, pBuf->FolderInfo.szLongMemory, NULL, NULL, NULL, NULL, FALSE );
  } /* endif */
  return( iRC );
}


// check if document exists and prepare import of translation units
int FolXliffParseHandler::CheckAndPrepareDocument()
{
  int iRC = 0;
  CHAR szDocShortName[MAX_FILESPEC];
  BOOL fIsNew = TRUE;

  // check folder
  iRC = CheckFolder();

  // check if document exists in TM
  if ( !iRC )
  {
    // check if this is a new document
    FolLongToShortDocName( pBuf->FileInfo.szFolObjName, pBuf->FileInfo.szFile, szDocShortName, &fIsNew );

    if ( fIsNew )
    {
      PSZ pszParms[2];
      pszParms[0] = pBuf->FileInfo.szFile;
      pszParms[1] = pBuf->FileInfo.szFolLongName;
      iRC = XLIFF_DOCUMENT_DOES_NOT_EXIST;
      UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 2, pszParms, EQF_ERROR, pImpExpData->hwndErrMsg );
    }
    else
    {
      // setup document object name
      strcpy( pBuf->FileInfo.szDocObjName, pBuf->FileInfo.szFolObjName );
      strcat( pBuf->FileInfo.szDocObjName, BACKSLASH_STR );
      strcat( pBuf->FileInfo.szDocObjName, szDocShortName );
    } /* endif */
  } /* endif */

  // get memory to be used for this document
  if ( !iRC )
  {
    DocQueryInfo( pBuf->FileInfo.szDocObjName, pBuf->FileInfo.szMemory, NULL, NULL, NULL, FALSE );
  } /* endif */

  // load segmented target document
  if ( !iRC && !m_fSkipDoc )
  {
    m_Util->BuildTMFileName( pBuf->szSegTargetFile, pImpExpData->szTargetFolObj[0], DIRSEGTARGETDOC_PATH, 
                             UtlGetFnameFromPath(pImpExpData->szTargetFolObj), szDocShortName ); 
    iRC = m_Util->LoadSegmentedFile( pBuf->szSegTargetFile, &pTargetDoc, TRUE );
  } /* endif */

  // open memory if memory handling has been requested
  if ( !iRC && pImpExpData->fWithFolderMem )
  {
    OpenMem( pBuf->FileInfo.szMemory );
  } /* endif */

  if ( iRC )
  {
    m_fSkipDoc = TRUE;
  }
  else
  {
    m_fDocChecked = TRUE;
  } /* endif */

  return( iRC );
}

int FolXliffParseHandler::OpenMem
( 
  PSZ              pszMemory           // name of memory being opened
)
{
  int              iRC = 0;

  if ( strcmp( pBuf->szCurMemory, pszMemory ) != 0 )
  {
    // close any open document memory database                     
    CloseMem();

    // open memory 
    {
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      pBuf->pMem = pFactory->openMemory( NULL, pszMemory, NONEXCLUSIVE, &iRC );

      if ( !iRC )
      {
        strcpy( pBuf->szCurMemory, pszMemory );
      }
      else
      {
        // memory not accessible or not found
        UtlErrorHwnd( EQFS_TM_NOT_FOUND, MB_CANCEL, 1, &pszMemory, EQF_ERROR, pImpExpData->hwndErrMsg );
        iRC = EQFS_TM_NOT_FOUND;
      } /* endif */
    }
  } /* endif */

  return( iRC );
}

int FolXliffParseHandler::CloseMem()
{
  int              iRC = 0;

  if ( pBuf->pMem != NULL )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory( pBuf->pMem );
    pBuf->pMem = NULL;
    pBuf->szCurMemory[0] = '\0';
  } /* endif */
  return( iRC );
}

int FolXliffParseHandler::UpdateMemProposal
(
  ULONG        ulSegNum,                         // segment number
  PSZ_W        pszSource,                        // source string
  PSZ_W        pszTarget,                        // target string
  PSZ          pszMarkup,                        // markup
  PSZ          pszSourceLang,                    // source language
  PSZ          pszTargetLang,                    // target language
  PSZ          pszShortName,                     // document short name
  PSZ          pszLongName,                      // document long name
  LONG         lTime                             // proposal time
)
{
  int              iRC = 0;

  if ( pBuf->pMem == NULL )
  {
    return( 0 );
  } /* endif */

  OtmProposal Prop;

  Prop.setSource( pszSource );
  Prop.setTarget( pszTarget );
  Prop.setDocShortName( pszShortName );
  Prop.setDocName( pszLongName );
  if ( lTime == 0)
  {
	  UtlTime( &lTime );
  } /* endif */
  Prop.setUpdateTime( lTime );
  Prop.setType( OtmProposal::eptManual );
  Prop.setSegmentNum( ulSegNum );
  Prop.setMarkup( pszMarkup );
  Prop.setSourceLanguage( pszSourceLang );
  Prop.setTargetLanguage( pszTargetLang );
  iRC = pBuf->pMem->putProposal( Prop );

  return( iRC );
}
