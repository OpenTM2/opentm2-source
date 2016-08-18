//+----------------------------------------------------------------------------+
//| EQFXNAME.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: XLIFF CNameListClass                                          |
//|                                                                            |
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

//
// implementation of name list class
//

// construct an empty name list
CNameList::CNameList()
{
  m_pList = NULL;
  m_iListSize = 0;
}

// destructor
CNameList::~CNameList() 
{
  if ( m_pList ) free( m_pList );
}

// add the given name and value to the list
BOOL CNameList::AddName( const char *pszName, const char *pszValue )
{
  BOOL fOK = TRUE;

  int iNameLen = strlen(pszName) + 1;
  int iValueLen = strlen(pszValue) + 1;  

  m_pList = (PSZ)realloc( m_pList, m_iListSize + iNameLen + iValueLen );

  if ( m_pList )
  {
    PSZ pszListEnd = m_pList + m_iListSize;
    strcpy( pszListEnd, pszName );
    pszListEnd  += iNameLen;
    strcpy( pszListEnd, pszValue );
    m_iListSize += iNameLen + iValueLen;
  }
  else
  {
    fOK = FALSE;
    m_pList = NULL;
    m_iListSize = 0;
  } /* endif */
  
  return( fOK );
}

// find the given name and return its value in the supplied buffer
BOOL CNameList::FindName( const char *pszName, char *pszValue, int iBufSize )
{
  BOOL fFound = FALSE;
  PSZ  pszEntry = m_pList;
  PSZ  pszListEnd = m_pList + m_iListSize;

  while ( !fFound && (pszEntry < pszListEnd) )
  {
    if ( _stricmp( pszEntry, pszName ) == 0 )
    {
      // entry found
      while ( *pszEntry ) pszEntry++; // skip name
      pszEntry++;
      memset( pszValue, 0, iBufSize );
      strncpy( pszValue, pszEntry, iBufSize - 1 );
      fFound = TRUE;
    }
    else
    {
      // try next entry
      while ( *pszEntry ) pszEntry++; // skip name
      pszEntry++;
      while ( *pszEntry ) pszEntry++; // skip value
      pszEntry++;
    } /* endif */
  } /*endwhile */

  return( fFound );
}
