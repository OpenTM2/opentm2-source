//+----------------------------------------------------------------------------+
//| EQFXUTIL.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2017, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: XLIFF related utilities                                       |
//+----------------------------------------------------------------------------+
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
#include "EQFHLOG.H"

#pragma pack( pop, TM2StructPacking )

#include "core\pluginmanager\OtmMemory.h"
#include "core\pluginmanager\OtmMemoryPlugin.h"
#include "core\memory\MemoryFactory.h"
#include "core\utilities\LanguageFactory.h"


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

//
// Implementation of CXLIFFUtil class
// 

CXLIFFUtil::CXLIFFUtil()
{
}

CXLIFFUtil::~CXLIFFUtil()
{
}

// convert a TMX language name to an TM language name
BOOL CXLIFFUtil::TMXLanguage2TMLanguage( PSZ pszTMLanguage, PSZ pszTMXLanguage, PSZ pszResultingLanguage )
{
  BOOL fOK = TRUE;

  strcpy( pszResultingLanguage, pszTMLanguage );
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();
  pLangFactory->getOpenTM2NameFromISO( pszTMXLanguage, pszResultingLanguage );

  return( fOK );
} /* end of method XLIFFParseHandler::GeValue */

void CXLIFFUtil::SetNameLists( CNameList *pLangList, CNameList *pTypeList )
{
  pLangTmx2Tmgr = pLangList;
  pType2Markup = pTypeList;
} /* end of method XLIFFParseHandler::SetNameLists */

// help functions for date conversion
BOOL CXLIFFUtil::IsLeapYear( const int iYear )
{
  if ((iYear % 400) == 0)
    return true;
  else if ((iYear % 100) == 0)
    return false;
  else if ((iYear % 4) == 0)
    return true;
  return false;
}                   

int CXLIFFUtil::GetDaysOfMonth( const int iMonth, const int iYear )
{
  int aiDaysOfMonth[13] = {  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  if (iMonth == 2)
  {
    if ( IsLeapYear( iYear ) )
      return 29;
    else
      return 28;
  }
  if (( iMonth >= 1) && (iMonth <= 12))
    return aiDaysOfMonth[iMonth];
  else
  {
    return 0;
  }
}                   

int CXLIFFUtil::GetDaysOfYear( const int iYear )
{
  if ( IsLeapYear( iYear ) )
  {
    return 366;
  }
  else
  {
    return 365;
  } /* endif */
}                   

int CXLIFFUtil::GetYearDay( const int iDay, const int iMonth, const int iYear )
{
  int iLocalDay = iDay;
  int iLocalMonth = iMonth;

  while ( iLocalMonth > 1)
  {
    iLocalMonth--;
    iLocalDay += GetDaysOfMonth( iLocalMonth, iYear );
  }
  return iLocalDay ;
}                   

//
// build the name of a TM file
//
USHORT CXLIFFUtil::BuildTMFileName
(
  PSZ         pszFileName,                       // pointer to file name buffer
  CHAR        chDrive,                           // drive letter
  USHORT      usPathID,                          // path ID
  PSZ         pszFolder,                         // folder containing the document (short name)
  PSZ         pszDocument                        // document short name
)
{
  UtlMakeEQFPath( pszFileName, chDrive, usPathID, pszFolder );
  strcat( pszFileName, BACKSLASH_STR );
  strcat( pszFileName, pszDocument );

  return( 0 );
} /* end of */

//
//
//
int CXLIFFUtil::OpenOutputFile( PSZ pszFile, FILE **phfFile )
{
  int iRC = 0;
  FILE *hfFile;

  hfFile = fopen( pszFile, "wb" );

  if ( hfFile )
  {
    *phfFile = hfFile;
  }
  else
  {
    iRC = ERROR_OPEN_FAILED;
  } /* endif */

  return( iRC );
}

int CXLIFFUtil::CloseFile( FILE *hfFile )
{
  int iRC = 0;

  if ( hfFile ) fclose( hfFile );

  return( iRC );
}

int CXLIFFUtil::WriteBOM( FILE *hfFile )
{
  int iRC = 0;

  fwrite( UNICODEFILEPREFIX, 1, 2, hfFile );

  return( iRC );
}

//
// load segmented target file into memory
//
int CXLIFFUtil::LoadSegmentedFile
( 
  PSZ          pszFileName,
  PTBDOCUMENT *ppDoc,
  BOOL         fTargetDoc
)
{
  int iRC = 0;
  PTBDOCUMENT pDoc = NULL;

  // allocate document structure
  if ( !UtlAlloc( (PVOID *)ppDoc, 0L, (LONG) sizeof(TBDOCUMENT), ERROR_STORAGE ) )
  {
    iRC = ERR_NOMEMORY;
  } /* endif */

  if ( !iRC )
  {
    pDoc = *ppDoc;
    iRC = (SHORT)TALoadTagTable( DEFAULT_QFTAG_TABLE, (PLOADEDTABLE *) &pDoc->pQFTagTable, TRUE, FALSE );
  } /* endif */

  if ( !iRC )
  {
    // set document short name in our document structures
    strcpy(  pDoc->szDocName, pszFileName );

    if ( fTargetDoc ) pDoc->docType = STARGET_DOC;

    iRC = EQFBFileReadExW( pszFileName, pDoc, FILEREAD_SINGLETABLE );
  } /* endif */

  return( iRC );
}

//
// Update segment data and state with new data
//
int CXLIFFUtil::UpdateSegment
(
  PTBDOCUMENT  pDoc,
  ULONG        ulSeg,
  PSZ_W        pszData
)
{
  int iRC = 0;

  PTBSEGMENT pSeg = EQFBGetSegW( pDoc, ulSeg );
  if ( pSeg )
  {
    // get length of new value
    int iNewLen = wcslen(pszData) + 1;

    // re-alloc segment data buffer
    if ( iNewLen > pSeg->usLength )
    {
      // get new segment data area
      UtlAlloc( (PVOID *)&(pSeg->pDataW), 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *)&(pSeg->pDataW), 0L, iNewLen * sizeof(CHAR_W), ERROR_STORAGE );
      pSeg->usLength = (USHORT)iNewLen;
    } /* endif */

    wcscpy( pSeg->pDataW, pszData );
    pSeg->qStatus = QF_XLATED;
    pSeg->SegFlags.Typed      = FALSE;
    pSeg->usTgtWords = 0;
    pSeg->usSrcWords = 0;
  } /* endif */
  return( iRC );
}

//
// Save segment target file to disk
//
int CXLIFFUtil::SaveSegmentedFile
(
  PSZ          pszFile,
  PTBDOCUMENT  pDoc
)
{
  return( EQFBFileWriteEx( pszFile, pDoc, DOCIMPNEWTARGET_LOGTASK3, SGMLFORMAT_UNICODE ) );
}

//
// free memory allocated for a segmented file
//
int CXLIFFUtil::FreeSegmentedFile
( 
  PTBDOCUMENT *ppDoc
)
{
  PTBDOCUMENT pDoc = *ppDoc;

  TAFreeTagTable( (PLOADEDTABLE)pDoc->pQFTagTable );

  SegFileFreeDoc( (PVOID *)ppDoc );

  return( 0 );
} /* end of FreeSegmentedFile */ 

//
// Compute document completion ratio and update document properties
//
int CXLIFFUtil::SetDocCompletion
(
  PSZ          pszFolObjName,
  PSZ          pszDocShortName,
  PTBDOCUMENT  pDoc
)
{
  HPROP           hProp;               // handle to document properties
  PPROPDOCUMENT   pProp = NULL;        // pointer to document properties
  ULONG           ulErrorInfo;         // error indicator from PRHA
  ULONG           ulTrans = 0;
  ULONG           ulNotTrans = 0;
  int             iRC = 0;

  // count tanslated/untranslated segments
  {
    ULONG ulSegNum = 1;
    PTBSEGMENT pSeg = EQFBGetSegW( pDoc, ulSegNum );
    while ( pSeg )
    {

      if ( !pSeg->SegFlags.Joined || pSeg->SegFlags.JoinStart )
      {
         if ( pSeg->qStatus == QF_XLATED ) 
         {
           ulTrans++;
         }
         else if ( pSeg->qStatus != QF_NOP ) 
         {
           ulNotTrans++;
         } /* endif */
      } else {
        // ignore joined segments
      }
      ulSegNum++;
      pSeg = EQFBGetSegW( pDoc, ulSegNum );
    } /*endwhile */
  }

  // update properties
  hProp = OpenProperties( pszDocShortName, pszFolObjName, PROP_ACCESS_READ, &ulErrorInfo );
  if( hProp != NULL )  
  {
    pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );
    if ( SetPropAccess( hProp, PROP_ACCESS_WRITE) )
    {
      // reset some dates
//    pProp->ulTouched = 0L;
      pProp->ulXLated = 0L;

      // reset memory match counts (not valid anymore)
      pProp->ulNoProps       = 0;
      pProp->ulFuzzy         = 0;
      pProp->ulExactExact    = 0;
      pProp->ulExactOne      = 0;
      pProp->ulExactMore     = 0;
      pProp->ulTotal         = 0;
      pProp->ulRepl          = 0;
      pProp->ulFuzzyRepl     = 0;
      pProp->ulSegNoProps    = 0;
      pProp->ulSegFuzzy      = 0;
      pProp->ulSegExactExact = 0;
      pProp->ulSegExactOne   = 0;
      pProp->ulSegExactMore  = 0;
      pProp->ulSegTotal      = 0;
      pProp->ulSegRepl       = 0;
      pProp->ulSegFuzzyRepl  = 0;
      pProp->ulSegMachineMatch = 0;
      pProp->ulMachineMatch  = 0;
      memset( &pProp->Total,      0,    sizeof(COUNTSUMS) );
      memset( &pProp->ExactExact, 0, sizeof(COUNTSUMS) );
      memset( &pProp->ExactOne,   0,    sizeof(COUNTSUMS) );
      memset( &pProp->Fuzzy1,     0,   sizeof(COUNTSUMS) );
      memset( &pProp->Fuzzy2,     0,   sizeof(COUNTSUMS) );
      memset( &pProp->Fuzzy3,     0,   sizeof(COUNTSUMS) );
      memset( &pProp->NoProps,    0,  sizeof(COUNTSUMS) );
      memset( &pProp->MTProps,    0,  sizeof(COUNTSUMS) );
      memset( &pProp->Repl,       0,     sizeof(COUNTSUMS) );
      memset( &pProp->ExactMore,  0, sizeof(COUNTSUMS) );

      // set completion rate
      if ( ulTrans == 0 )
      {
        pProp->usComplete = 0;
      }
      else if ( ulNotTrans == 0 )
      {
        pProp->usComplete = 100;
        UtlTime( (PLONG)&(pProp->ulXLated) );
      }
      else
      {
        pProp->usComplete = (USHORT)( (ulTrans * 100L) / (ulTrans + ulNotTrans) );
        if ( !pProp->usComplete && ulTrans )
        {
          pProp->usComplete = 1;
        } /* endif */
      } /* endif */

      // set import time
      UtlTime( (PLONG)&(pProp->ulImp) );
      UtlTime( (PLONG)&(pProp->ulTouched) );


      SaveProperties( hProp, &ulErrorInfo );
      ResetPropAccess( hProp, PROP_ACCESS_WRITE );
    } /* endif */
    CloseProperties( hProp, PROP_FILE, &ulErrorInfo);
  } /* endif */

  return ( iRC );
} /* end of SetSegDate */

//+----------------------------------------------------------------------------+
//| Implementation of class for tagging lists                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
CTagList::CTagList()
{
  m_iTaggingSize = 0;
  m_iTaggingUsed = 0;
  m_pTagging = NULL;
  m_iBufSize = 0;
  m_iBufUsed = 0;
  m_pBuffer = NULL;
}

CTagList::~CTagList()
{
  if ( m_pTagging ) free ( m_pTagging );
  if ( m_pBuffer )  free( m_pBuffer );
}

void CTagList::clear()
{
  m_iTaggingUsed = 0;
  m_iBufUsed = 0;
}

int CTagList::add( int id, PSZ_W pszTagging )
{
  int iRC = 0;

  int iNewLen = wcslen( pszTagging) + 1;

  // enlarge table if required
  if ( m_iTaggingSize <= m_iTaggingUsed )
  {
    int iNewSize = m_iTaggingSize + 5;
    m_pTagging = (struct _TAGGING *)realloc( m_pTagging, iNewSize * sizeof(TAGGING) );
    m_iTaggingSize = iNewSize;
  } /* endif */

  // enlarge buffer if required
  if ( (m_iBufUsed + iNewLen) >= m_iBufSize )
  {
    int iNewSize = m_iBufSize + max( iNewLen, 512 );
    m_pBuffer = (PSZ_W)realloc( m_pBuffer, iNewSize * sizeof(CHAR_W) );
    m_iBufSize = iNewSize;
  } /* endif */
 
  // add new element
  if ( m_pBuffer && m_pTagging )
  {
    m_pTagging[m_iTaggingUsed].id = id;
    m_pTagging[m_iTaggingUsed].iOffset = m_iBufUsed;
    m_iTaggingUsed += 1;

    wcscpy( m_pBuffer + m_iBufUsed, pszTagging );
    m_iBufUsed += iNewLen;
  }
  else
  {
    iRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  return( iRC );
}

PSZ_W CTagList::get( int id )
{
  PSZ_W pszTags = NULL;

  for( int i = 0; i < m_iTaggingUsed; i++ )
  {
    if ( m_pTagging[i].id == id )
    {
      pszTags = m_pBuffer + m_pTagging[i].iOffset;
      break;
    } /* endif */
  } /* endfor */

  return( pszTags );
}

