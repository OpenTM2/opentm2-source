//+----------------------------------------------------------------------------+
//| EQFTADIT.CPP                                                            |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//| 	Copyright (C) 1990-2012, International Business Machines                 |
//| 	Corporation and others. All rights reserved                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: SGML-DITA special processing functions                        |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

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

#ifndef CPPTEST
extern "C"
{
#endif
  #pragma pack( push, TM2StructPacking, 1 )

  #define INCL_EQF_TP               // public translation processor functions
  #define INCL_EQF_EDITORAPI        // editor API
  #define INCL_EQF_TAGTABLE         // tagtable defines and functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
  #define INCL_EQF_DLGUTILS         // dialog utilities
  #define INCL_EQF_MORPH
  #define INCL_EQF_DAM
  #include "eqf.h"                  // General .H for EQF
  #include <EQFTADIT.H>             // IBMIDDOC->IBMDITA special processing functions
  #include "EQFMEMIE.H"

  #pragma pack( pop, TM2StructPacking )
#ifndef CPPTEST
}
#endif

#ifdef _DEBUG
  #define SGMLDITA_LOGGING
#endif

// max number of changes in a single chage entry
#define MAX_CHANGES_PER_ENTRY 10

typedef struct _TMO_REPLACE_ENTRY
{
  int         iEntries;                          // number of changes in this entry
  int         iFromLength;                       // overall length of change-from strings (used for sorting)
  int         aiOffsFrom[MAX_CHANGES_PER_ENTRY]; // offset array for change-from strings
  int         aiOffsTo[MAX_CHANGES_PER_ENTRY];   // offset array for change-to strings
} TMO_REPLACE_ENTRY, *PTMO_REPLACE_ENTRY;

typedef struct _TMO_REPLACE_LIST
{
  int         iEntriesUsed;                      // number of used change entries in list
  int         iEntriesSize;                      // number of entries in change entries in list
  PTMO_REPLACE_ENTRY pEntries;                   // ptr to array of change entries
  int         iBufferSize;                       // size of change string buffer in number of characters
  int         iBufferUsed;                       // number of characters used in string buffer
  BOOL        fErrorInList;                      // TRUE = errors occurend while loading this list
  CHAR        szErrorText[2048];                 // buffer for error messages
  PSZ_W       pszStringBuffer;                   // ptr to change string buffer
} TMO_REPLACE_LIST, *PTMO_REPLACE_LIST;


BOOL DITARemoveTagging( PSZ_W pszSourceIn, PSZ_W pszSourceOut, PSZ_W pszTargetIn, PSZ_W pszTargetOut, PSZ pszMarkup, ULONG ulSrcCP );
BOOL DITAReplaceIDDOCVars( PSZ_W pszData );
BOOL DITARemoveAdditionalWhitepace( PSZ_W pszSource, PSZ_W pszProposal );
BOOL DITAConvertEntities( PSZ_W pszSource, PSZ_W pszProposal );
BOOL DITAApplyReplaceList( PTMO_REPLACE_LIST pList, PSZ_W pszSource, PSZ_W pTarget );
int  DITAApplySingleReplacement( PTMO_REPLACE_LIST pList, int iEntry, PSZ_W pszInString, PSZ_W pszOutString );
int DITAReplaceString( PSZ_W pszString, PSZ_W pszFrom, PSZ_W pszTo );

//
// class for our SAX handler
//

#define MAX_CAPTURE_LEN 8096


class ReplListParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  ReplListParseHandler( PTMO_REPLACE_LIST pInList );
  virtual ~ReplListParseHandler();

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t length);

  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);

private:
  int ReplListParseHandler::AddStringToBuffer( PSZ_W pszText );
  BOOL ReplListParseHandler::AddChangeEntry( PTMO_REPLACE_ENTRY pEntry );

  enum { FROM_ELEMENT, TO_ELEMENT } m_CurrentElement;
  BOOL        m_fCaptureText;                    // TRUE = capture text to our text buffer
  CHAR_W      m_szTextBuffer[MAX_CAPTURE_LEN];   // text capture buffer
  TMO_REPLACE_ENTRY m_CurEntry;                  // currently processed replace entry 
  PTMO_REPLACE_LIST m_pList;                     // ptr to current replace list
};


#ifdef SGMLDITA_LOGGING
          FILE *hfLog = NULL;
          CHAR szLogFile[MAX_EQF_PATH];
          static CHAR_W szSegBuf[4096];
          static CHAR_W szLineBuf[4096];
#endif

BOOL DITASpecialProcessingRequired
(
  PSZ         pszDocMarkup,                      // markup of document
  PSZ         pszMemMarkup,                      // markup of memory proposal
  DITAPROCESSINGMODES mode                       // type of special processing
)
{
  BOOL fDitaProcessing = ( (stricmp( pszMemMarkup , "IBMIDDOC" ) == 0) &&
                           (stricmp( pszDocMarkup, "IBMDITA" ) == 0) );

  mode;                                          // currently not used

  return( fDitaProcessing );
} /* end of DITASpecialProcessingRequired */

// return value 2 = TMO replacement performed
BOOL DITAPreprocessProposals
(
  PSZ_W       pszSource,                         // source of memory proposal
  PSZ_W       pszTarget,                         // target of memory proposal
  PVOID       pvReplaceList                      // ptr to replace list or NULL
)
{
  BOOL fReplacedListApplied = FALSE;
  BOOL fReplaced1 = FALSE;
  BOOL fReplaced2 = FALSE;
          
  // apply change list on proposal
  if ( pvReplaceList )
  {
    PTMO_REPLACE_LIST pList = (PTMO_REPLACE_LIST)pvReplaceList;

    fReplacedListApplied = DITAApplyReplaceList( pList, pszSource, pszTarget );
  } /* endif */

  fReplaced1 = DITAReplaceIDDOCVars( pszSource );
  fReplaced2 = DITAReplaceIDDOCVars( pszTarget );

  if ( fReplacedListApplied )
  {
    return( 2 );
  }
  else
  {
    return( fReplaced1 || fReplaced2 );
  } /* endif */
  
} /* end of function DITAPreprocessProposals */

BOOL DITAProcessProposal
( 
  PSZ_W       pszDocSource,                      // source of document segment
  PSZ_W       pszSource,                         // source of memory proposal
  PSZ_W       pszTarget,                         // target of memory proposal
  PSZ         pszMarkup,                         // markup of memory proposal
  ULONG       ulOemCP                            // OEM CP for proposal source 
)
{
  // remove remaining inline tagging
  BOOL fTagsRemoved = DITARemoveTagging( pszSource, pszSource, pszTarget, pszTarget, pszMarkup, ulOemCP );

  // adjust leadig and trailing whitespace 
  BOOL fSourceWSChanged = DITARemoveAdditionalWhitepace( pszDocSource, pszSource );
  BOOL fTargetWSChanged = DITARemoveAdditionalWhitepace( pszDocSource, pszTarget );

  // convert entities to keyword/ph tag sequences
  BOOL fSourceEntChanged = DITAConvertEntities( pszDocSource, pszSource );
  BOOL fTargetEntChanged = DITAConvertEntities( pszDocSource, pszTarget );

  return( fTagsRemoved || fSourceWSChanged || fTargetWSChanged || fSourceEntChanged || fTargetEntChanged);
} /* end of function DITAProcessProposal */


BOOL DITALoadReplacementList
( 
  PSZ         pszReplaceList,                    // fully qualified replacement list name 
  PVOID       *ppvReplaceList                    // ptr to callers replacement list pointer
)
{
  BOOL fOK = TRUE;
  PTMO_REPLACE_LIST pList = NULL;

  fOK = UtlAlloc( (PVOID *)&pList, 0, sizeof(TMO_REPLACE_LIST), ERROR_STORAGE );

  if ( fOK )
  {
    try 
    {
      XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch) 
    {
      toCatch;

      return( FALSE );
    }
  } /* endif */

  if ( fOK )
  {
    SAXParser *parser = new SAXParser();      // Create a SAX parser object
    ReplListParseHandler *handler = new ReplListParseHandler( pList );

    //  install our SAX handler as the document and error handler.
    parser->setDocumentHandler( handler );
    parser->setErrorHandler( handler );
    parser->setValidationSchemaFullChecking( FALSE );
    parser->setDoSchema( FALSE );
    //parser->setDoValidation( FALSE );
    parser->setLoadExternalDTD( FALSE );
    parser->setValidationScheme( SAXParser::Val_Never );

    try
    {
      parser->parse( pszReplaceList );
    }
    catch (const OutOfMemoryException& )
    {
      pList->fErrorInList = TRUE;  
      strcpy( pList->szErrorText, "Memory allocation failed" );
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

      pList->fErrorInList = TRUE;  
      strcpy( pList->szErrorText, "Error reading replacement list" );
    }
  } /* endif */
  
  // handle errors found while parsing the list
  if ( pList )
  {
    if ( pList->fErrorInList )
    {
      PSZ pszParms[2];

      pszParms[0] = pszReplaceList;
      pszParms[1] = pList->szErrorText;

      UtlError( ERROR_IN_REPLLIST, MB_CANCEL, 2, pszParms, EQF_ERROR ); 

      DITAFreeReplacementList ( (PVOID *)&pList );

      fOK = FALSE;
    } /* endif */
  } /* endif */

  *ppvReplaceList = pList;

  return( fOK  );
} /* end of function DITALoadReplacementList */

BOOL DITAFreeReplacementList
( 
  PVOID       *ppvReplaceList                    // ptr to callers replacement list pointer
)
{
  PTMO_REPLACE_LIST pList = (PTMO_REPLACE_LIST)*ppvReplaceList;

  if ( pList != NULL )
  {
    if ( pList->pEntries ) UtlAlloc( (PVOID *)&(pList->pEntries), 0, 0, NOMSG );
    if ( pList->pszStringBuffer ) UtlAlloc( (PVOID *)&(pList->pszStringBuffer), 0, 0, NOMSG );
    UtlAlloc( (PVOID *)ppvReplaceList, 0, 0, NOMSG );
  } /* endif */

  return( TRUE );
} /* end of function DITAFreeReplacementList */


// helper functions for DITARemoveTagging 
BOOL DITASkipTags( PSTARTSTOP pStartStop, PSZ_W pszSource, PSZ_W pszTarget )
{
  BOOL fTagsRemoved = FALSE;
  BOOL fBold = FALSE;
  PSTARTSTOP pEntry = pStartStop;
  while ( (pEntry->usStart != 0) || (pEntry->usStop != 0)  || (pEntry->usType != 0) )
  {
    switch ( pEntry->usType )
    {
      case UNPROTECTED_CHAR :
        {
          USHORT usPos = pEntry->usStart;
          while ( usPos <= pEntry->usStop ) *pszTarget++ = pszSource[usPos++];
        }
        break;
      default :
        {
          PSZ_W pszTag = pszSource + pEntry->usStart;
          // ignore protected data (if it is not a variable)
          if ( pszTag[0] == L'&' )
          {
            USHORT usPos = pEntry->usStart;
            while ( usPos <= pEntry->usStop ) *pszTarget++ = pszSource[usPos++];
          }
          else if ( wcsnicmp( pszTag, L"<ph", 3 ) == 0 ) 
          {
            pszTag += 3;
            while ( iswspace( *pszTag ) ) pszTag++;

            if ( wcsnicmp( pszTag, L"style=\"bold\"", 12 ) == 0 ) 
            {
              fBold = TRUE;
              wcscpy( pszTarget, L"<b>" );
              pszTarget += 3;
            } /* endif */
          }
          else if ( fBold && (wcsnicmp( pszTag, L"</ph>", 5 ) == 0) ) 
          {
            fBold = FALSE;
            wcscpy( pszTarget, L"</b>" );
            pszTarget += 4;
          }
          else
          {
            fTagsRemoved = TRUE;
          } /* endif */
        }
        break;
    } /* endswitch */
    pEntry++;
  } /* endwhile */
  *pszTarget = 0;
  return( fTagsRemoved );
} /* end of function DITASkipTags */

USHORT DITAMakeStartStopTable( PSZ_W pszSource, PLOADEDTABLE pTable, PSTARTSTOP *ppStartStop, PVOID *ppTokenList, PLONG plSize, ULONG ulSrcCP ) 
{
  LONG       lOldSize = *plSize;
  LONG       lNewSize = 0;
  USHORT     usRC = NO_ERROR;
  int        iIterations = 0;

  if ( *ppTokenList == NULL )
  {
    *plSize = lOldSize = sizeof(TOKENENTRY) * 1000;
    if ( !UtlAlloc( ppTokenList, 0L, lOldSize, NOMSG ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  if ( !usRC )
  {
    usRC = TACreateProtectTableW( pszSource, pTable, 0, (PTOKENENTRY)*ppTokenList,
                                  (USHORT)lOldSize, ppStartStop,
                                  pTable->pfnProtTable,
                                  pTable->pfnProtTableW, ulSrcCP );

    while ((iIterations < 10) && (usRC == EQFRS_AREA_TOO_SMALL))
    {
      // (re)allocate token buffer
      lNewSize = lOldSize + (sizeof(TOKENENTRY) * 128);
      if (UtlAlloc( ppTokenList, lOldSize, lNewSize, NOMSG) )
      {
        lOldSize = lNewSize;
        iIterations++;
      }
      else
      {
        iIterations = 10;    // force end of loop
      } /* endif */
      // retry tokenization
      if (iIterations < 10 )
      {
        usRC = TACreateProtectTableW( pszSource, pTable, 0, (PTOKENENTRY)*ppTokenList, (USHORT)lOldSize, ppStartStop,
                                      pTable->pfnProtTable, pTable->pfnProtTableW, ulSrcCP );
      } /* endif */
    } /* endwhile */
  } /* endif */
  *plSize = lOldSize;
  return( usRC );
} /* end of function DITAMakeStartStopTable */

BOOL DITACompareTags( PSZ_W pszSource, PSTARTSTOP pStartStopSource, PSZ_W pszTarget, PSTARTSTOP pStartStopTarget ) 
{
  BOOL fTagsEqual = TRUE;
  PSTARTSTOP pEntry = pStartStopSource;

  while ( (pEntry->usStart != 0) || (pEntry->usStop != 0)  || (pEntry->usType != 0) )
  {
    if ( (pEntry->usType == UNPROTECTED_CHAR) || (pszSource[pEntry->usStart] == L'&')  )
    {
      // ignore translatable data 
    }
    else
    {
      // look for this tag in target table
      PSTARTSTOP pTargetEntry = pStartStopTarget;
      PSZ_W pszSourceTag = pszSource + pEntry->usStart;
      int  iSourceLen = pEntry->usStop - pEntry->usStart + 1;
      BOOL fFound = FALSE;

      while ( !fFound && ((pTargetEntry->usStart != 0) || (pTargetEntry->usStop != 0)  || (pTargetEntry->usType != 0)) )
      {
        if ( pTargetEntry->usType & 0x8000 )
        {
          // this entry has been used already
        }
        else if ( (pTargetEntry->usType == UNPROTECTED_CHAR) || (pszTarget[pTargetEntry->usStart] == L'&')  )
        {
          // ignore translatable data 
        }
        else
        {
          PSZ_W pszTargetTag = pszTarget + pTargetEntry->usStart;
          int  iTargetLen = pTargetEntry->usStop - pTargetEntry->usStart + 1;

          if ( iSourceLen == iTargetLen )
          {
            if ( wcsncmp( pszSourceTag, pszTargetTag, iSourceLen ) == 0 )
            {
              fFound = TRUE;
            } /* endif */
          } /* endif */
        } /* endif */
        if ( !fFound )
        {
          pTargetEntry++;
        } /* endif */
      } /* endwhile */

      if ( fFound )
      {
        // mark tag as processed
        pTargetEntry->usType |= 0x8000;
      }
      else
      {
        fTagsEqual = FALSE;
      } /* endif */
    } /* endif */
    pEntry++;
  } /* endwhile */

  // remove our 'tag has been used flag' and look for unused tags
  {
    PSTARTSTOP pTargetEntry = pStartStopTarget;

    while ( (pTargetEntry->usStart != 0) || (pTargetEntry->usStop != 0)  || (pTargetEntry->usType != 0) )
    {
      if ( (pTargetEntry->usType == UNPROTECTED_CHAR) || (pszTarget[pTargetEntry->usStart] == L'&')  )
      {
        // leave text as-is
      }
      else if ( pTargetEntry->usType & 0x8000 )
      {
        pTargetEntry->usType &= ~0x8000;
      }
      else
      {
        fTagsEqual = FALSE;
      } /* endif */
      pTargetEntry++;
    } /* endwhile */
  }
  return( fTagsEqual );
} /* end of function DITAMakeStartStopTable */

BOOL DITARemoveTagging( PSZ_W pszSourceIn, PSZ_W pszSourceOut, PSZ_W pszTargetIn, PSZ_W pszTargetOut, PSZ pszMarkup, ULONG ulSrcCP )
{
  PVOID     pTokenList = NULL;         // ptr to token table
  PLOADEDTABLE pTable = NULL;          // pointer to tagtable
  USHORT    usRC = NO_ERROR;           // returned value
  PSTARTSTOP pStartStopSource = NULL;        // ptr to start/stop table
  PSTARTSTOP pStartStopTarget = NULL;        // ptr to start/stop table
  LONG       lSize = 0;                // current size of oken list buffer
  BOOL       fTagsRemoved = FALSE;
  BOOL       fTagsMatching = FALSE;

  if ( !usRC )
  {
    //load tag table for tokenize function
    usRC = TALoadTagTableExHwnd( pszMarkup, &pTable, FALSE, TALOADUSEREXIT | TALOADPROTTABLEFUNC | TALOADCOMPCONTEXTFUNC,
                                 FALSE, NULLHANDLE );
    if ( usRC )
    {
      usRC = ERROR_TA_ACC_TAGTABLE;
    } /* endif */
  } /* endif */

  // build protect start/stop table for tag recognition in source and target
  if ( !usRC )
  {
    usRC = DITAMakeStartStopTable( pszSourceIn, pTable, &pStartStopSource, &pTokenList, &lSize, ulSrcCP );
    if ( !usRC )
    {
      usRC = DITAMakeStartStopTable( pszTargetIn, pTable, &pStartStopTarget, &pTokenList, &lSize, ulSrcCP );
    } /* endif */
  } /* endif */

  //// check for matching tags
  //if ( !usRC )
  //{
  //  fTagsMatching = DITACompareTags( pszSourceIn, pStartStopSource, pszTargetIn, pStartStopTarget );
  //} /* endif */

  // skip tags but leave/convert bold tags
  if ( !usRC )
  {
    fTagsRemoved = DITASkipTags( pStartStopSource, pszSourceIn, pszSourceOut );
    fTagsRemoved |= DITASkipTags( pStartStopTarget, pszTargetIn, pszTargetOut );
  } /* endif */

  // cleanup
  if ( pStartStopSource ) UtlAlloc( (PVOID *) &pStartStopSource, 0L, 0L, NOMSG );
  if ( pStartStopTarget ) UtlAlloc( (PVOID *) &pStartStopTarget, 0L, 0L, NOMSG );
  if ( pTokenList ) UtlAlloc( (PVOID *) &pTokenList, 0L, 0L, NOMSG );
  if ( pTable != NULL ) TAFreeTagTable( pTable );

  return( fTagsRemoved );
} /* end of function DITARemoveTagging */

typedef struct _IBMIDDOCVAR
{
  CHAR_W   szVarName[20];                        // name of variable  
  CHAR_W   chValue;                              // character value for variable 
} IBMIDDOCVAR, *PIBMIDDOCVAR;

// table of IBMIDDOC character entities and the corresponding Unicode character
// Note: this table must be sorted alphabetically as we use bsearch 
static IBMIDDOCVAR IDDOCVars[] = 
{
  { L"&AElig",       0x00C6 }, // AE ligature
  { L"&Aa",          0x00C1 }, // A acute
  { L"&Ac",          0x00C2 }, // A circumflex
  { L"&Ad",          0x2193 }, // arrow down
  { L"&Ae",          0x00C4 }, // A umlaut
  { L"&Ag",          0x00C0 }, // A grave
  { L"&Al",          0x2190 }, // arrow left
  { L"&Alpha",       0x0391 }, // Alpha
  { L"&Ao",          0x00C5 }, // A overcircle
  { L"&Ar",          0x2192 }, // arrow right
  { L"&At",          0x00C3 }, // A tilde
  { L"&Au",          0x2191 }, // arrow up
  { L"&BOX",         0x25A0 }, // solid box
  { L"&BOXBOT",      0x2584 }, // solid box bottom half
  { L"&BOXLEFT",     0x258C }, // solid box left half
  { L"&BOXRIGHT",    0x2590 }, // solid box right half
  { L"&BOXTOP",      0x2580 }, // solid box top half
  { L"&Beta",        0x0392 }, // Beta
  { L"&CLUB",        0x2663 }, // club solid
  { L"&Cc",          0x00C7 }, // C cedilla
  { L"&Chi",         0x03A7 }, // Chi
  { L"&DIAMOND",     0x2666 }, // diamond solid
  { L"&Delta",       0x0394 }, // Delta
  { L"&Dstroke",     0x0110 }, // Eth or D stroke
  { L"&Ea",          0x00C9 }, // E acute
  { L"&Eb",          0x2502 }, // end of line, bottom
  { L"&Ec",          0x00CA }, // E circumflex
  { L"&Ee",          0x00CB }, // E umlaut
  { L"&Eg",          0x00C8 }, // E grave
  { L"&El",          0x2500 }, // end of line, left
  { L"&Epsilon",     0x0395 }, // Epsilon
  { L"&Er",          0x2500 }, // end of line, right
  { L"&Et",          0x2502 }, // end of line, top
  { L"&Eta",         0x0397 }, // Eta
  { L"&Eth",         0x00D0 }, // eth, Icelandic capital
  { L"&FACE",        0x263B }, // face solid
  { L"&Gamma",       0x0393 }, // Gamma
  { L"&HEART",       0x2665 }, // heart solid
  { L"&Ia",          0x00CD }, // I acute
  { L"&Ic",          0x00CE }, // I circumflex
  { L"&Icap",        0x0049 }, // I capital character
  { L"&Ie",          0x00CF }, // I umlaut
  { L"&Ig",          0x00CC }, // I grave
  { L"&Iota",        0x0399 }, // Iota
  { L"&Ju",          0x253C }, // line junction
  { L"&Kappa",       0x039A }, // Kappa
  { L"&Lambda",      0x039B }, // Lambda
  { L"&Lh",          0x2500 }, // line horizontal
  { L"&Ll",          0x2514 }, // lower left corner
  { L"&Lr",          0x2518 }, // lower right corner
  { L"&Lsterling",   0x00A3 }, // pound sterling
  { L"&Lv",          0x2502 }, // line vertical
  { L"&Mu",          0x039C }, // Mu
  { L"&Nt",          0x00D1 }, // N tilde
  { L"&Nu",          0x039D }, // Nu
  { L"&OElig",       0x0152 }, // OE ligature
  { L"&Oa",          0x00D3 }, // O acute
  { L"&Oc",          0x00D4 }, // O circumflex
  { L"&Oe",          0x00D6 }, // O umlaut
  { L"&Og",          0x00D2 }, // O grave
  { L"&Omega",       0x03A9 }, // Omega
  { L"&Omicron",     0x039F }, // Omicron
  { L"&Os",          0x00D8 }, // O slash
  { L"&Ot",          0x00D5 }, // O tilde
  { L"&Phi",         0x03A6 }, // Phi
  { L"&Pi",          0x03A0 }, // Pi
  { L"&Psi",         0x03A8 }, // Psi
  { L"&Rho",         0x03A1 }, // Rho
  { L"&Rx",          0x211E }, // physician Rx
  { L"&SPADE",       0x2660 }, // spade solid
  { L"&STAR",        0x22C6 }, // star solid
  { L"&Sigma",       0x03A3 }, // Sigma
  { L"&TELEPHONE",   0x260E }, // telephone solid
  { L"&TRIANGLE",    0x25B2 }, // triangle solid
  { L"&Tau",         0x03A4 }, // Tau
  { L"&Td",          0x252C }, // T char, bar down
  { L"&Theta",       0x0398 }, // Theta
  { L"&Thorn",       0x00DE }, // Thorn, Icelandic capital
  { L"&Tl",          0x2524 }, // T char, bar left
  { L"&Tr",          0x251C }, // T char, bar right
  { L"&Tu",          0x2534 }, // T char, bar up
  { L"&Ua",          0x00DA }, // U acute
  { L"&Uc",          0x00DB }, // U circumflex
  { L"&Ue",          0x00DC }, // U umlaut
  { L"&Ug",          0x00D9 }, // U grave
  { L"&Ul",          0x250C }, // upper left corner
  { L"&Upsilon",     0x03A5 }, // Upsilon
  { L"&Ur",          0x2510 }, // upper right corner
  { L"&Xi",          0x039E }, // Xi
  { L"&Ya",          0x00DD }, // Y acute
  { L"&Ye",          0x0178 }, // Y umlaut
  { L"&Zeta",        0x0396 }, // Zeta
  { L"&aa",          0x00E1 }, // a acute
  { L"&ac",          0x00E2 }, // a circumflex
  { L"&acute",       0x00B4 }, // accent acute
  { L"&ae",          0x00E4 }, // a umlaut
  { L"&aelig",       0x00E6 }, // ae ligature
  { L"&ag",          0x00E0 }, // a grave
  { L"&aleph",       0x05D0 }, // aleph
  { L"&all",         0x2200 }, // all
  { L"&alpha",       0x03B1 }, // alpha
  { L"&and",         0x2227 }, // and symbol
  { L"&angle",       0x2220 }, // angle
  { L"&angstrom",    0x00C5 }, // angstrom
  { L"&ao",          0x00E5 }, // a overcircle
  { L"&apos",        0x0027 }, // apostrophe
  { L"&app",         0x2248 }, // approximately
  { L"&approx",      0x2248 }, // approximately
  { L"&approxid",    0x224B }, // approximately identical
  { L"&arc",         0x2222 }, // arc
  { L"&asterisk",    0x002A }, // asterisk
  { L"&at",          0x00E3 }, // a tilde
  { L"&atsign",      0x0040 }, // at sign
  { L"&aus",         0x00AA }, // underscored a
  { L"&ballot",      0x2610 }, // ballot box
  { L"&because",     0x2235 }, // because
  { L"&beta",        0x03B2 }, // beta
  { L"&blank",       0x2422 }, // blank (b with slash)
  { L"&box",         0x2610 }, // ballot box
  { L"&box12",       0x2592 }, // shaded box 1/2 dots
  { L"&box14",       0x2591 }, // shaded box 1/4 dots
  { L"&box34",       0x2593 }, // shaded box 3/4 dots
  { L"&bs",          0x2408 }, // backspace
  { L"&bsl",         0x005C }, // back slash
  { L"&bslash",      0x005C }, // back slash
  { L"&bul",         0x2022 }, // bullet
  { L"&bullet",      0x2022 }, // bullet
  { L"&bx0012",      0x2555 }, // ASCII code 184
  { L"&bx0021",      0x2556 }, // ASCII code 183
  { L"&bx0022",      0x2557 }, // ASCII code 187
  { L"&bx0120",      0x2553 }, // ASCII code 214
  { L"&bx0121",      0x2565 }, // ASCII code 210
  { L"&bx0202",      0x2550 }, // ASCII code 205
  { L"&bx0210",      0x2552 }, // ASCII code 213
  { L"&bx0212",      0x2564 }, // ASCII code 209
  { L"&bx0220",      0x2554 }, // ASCII code 201
  { L"&bx0222",      0x2566 }, // ASCII code 203
  { L"&bx1002",      0x255B }, // ASCII code 190
  { L"&bx1012",      0x2561 }, // ASCII code 181
  { L"&bx1200",      0x2558 }, // ASCII code 212
  { L"&bx1202",      0x2567 }, // ASCII code 207
  { L"&bx1210",      0x255E }, // ASCII code 198
  { L"&bx1212",      0x256A }, // ASCII code 216
  { L"&bx2001",      0x255C }, // ASCII code 189
  { L"&bx2002",      0x255D }, // ASCII code 188
  { L"&bx2020",      0x2551 }, // ASCII code 186
  { L"&bx2021",      0x2562 }, // ASCII code 182
  { L"&bx2022",      0x2563 }, // ASCII code 185
  { L"&bx2100",      0x2559 }, // ASCII code 211
  { L"&bx2101",      0x2568 }, // ASCII code 208
  { L"&bx2120",      0x255F }, // ASCII code 199
  { L"&bx2121",      0x256B }, // ASCII code 215
  { L"&bx2200",      0x255A }, // ASCII code 200
  { L"&bx2202",      0x2569 }, // ASCII code 202
  { L"&bx2220",      0x2560 }, // ASCII code 204
  { L"&bx2222",      0x256C }, // ASCII code 206
  { L"&bxas",        0x2534 }, // box ascender
  { L"&bxbj",        0x2534 }, // box ascender
  { L"&bxcj",        0x253C }, // box cross
  { L"&bxcr",        0x253C }, // box cross
  { L"&bxde",        0x252C }, // box descender
  { L"&bxh",         0x2500 }, // box horizontal
  { L"&bxle",        0x251C }, // box left junction
  { L"&bxlj",        0x251C }, // box left junction
  { L"&bxll",        0x2514 }, // box lower-left
  { L"&bxlr",        0x2518 }, // box lower-right
  { L"&bxri",        0x2524 }, // box right junction
  { L"&bxrj",        0x2524 }, // box right junction
  { L"&bxtj",        0x252C }, // box descender
  { L"&bxul",        0x250C }, // box upper-left
  { L"&bxur",        0x2510 }, // box upper-right
  { L"&bxv",         0x2502 }, // box vertical
  { L"&caret",       0x2038 }, // caret
  { L"&cc",          0x00E7 }, // c cedilla
  { L"&cdot",        0x2299 }, // circled dot
  { L"&cdq",         0x201D }, // close double quote
  { L"&cdqf",        0x00BB }, // French close double quote
  { L"&cdqg",        0x201D }, // German close double quote
  { L"&cedilla",     0x00B8 }, // cedilla
  { L"&cent",        0x00A2 }, // cent
  { L"&cequal",      0x229C }, // circled equals
  { L"&char",        0x2032 }, // character
  { L"&check",       0x2713 }, // checkmark
  { L"&chi",         0x03C7 }, // chi
  { L"&circ",        0x25CB }, // circle
  { L"&circle",      0x25CB }, // circle
  { L"&cminus",      0x2296 }, // circled minus
  { L"&colon",       0x003A }, // colon
  { L"&comma",       0x002C }, // comma
  { L"&concat",      0x2225 }, // concatenate
  { L"&congruent",   0x2245 }, // congruent
  { L"&contains",    0x2283 }, // contains as a subset
  { L"&copr",        0x00A9 }, // copyright
  { L"&copyr",       0x00A9 }, // copyright
  { L"&cplus",       0x2295 }, // circled plus
  { L"&csq",         0x2019 }, // close single quote
  { L"&csqg",        0x2019 }, // German close single quote
  { L"&ctimes",      0x2297 }, // circled times
  { L"&ctq",         0x201D }, // close typographic quote
  { L"&currency",    0x00A4 }, // currency international
  { L"&cursor",      0x2501 }, // fat cursor
  { L"&dagger",      0x2020 }, // dagger
  { L"&dahead",      0x25BC }, // down arrowhead
  { L"&darrow",      0x2193 }, // down arrow
  { L"&dbldag",      0x2021 }, // double dagger
  { L"&dbls",        0x00A7 }, // double S
  { L"&dblus",       0x2017 }, // underscore double
  { L"&dblxclam",    0x203C }, // double exclamation point
  { L"&dblxclm",     0x203C }, // double exclamation point
  { L"&decrease",    0x2198 }, // decrease
  { L"&deg",         0x00B0 }, // degree
  { L"&degree",      0x00B0 }, // degree
  { L"&del",         0x2207 }, // del
  { L"&delta",       0x03B4 }, // delta
  { L"&determines",  0x21A6 }, // determines
  { L"&diam",        0x25C7 }, // diamond wide
  { L"&diamond",     0x25C7 }, // diamond
  { L"&div",         0x00F7 }, // divide
  { L"&divide",      0x00F7 }, // divide
  { L"&divslash",    0x002F }, // division slash
  { L"&dollar",      0x0024 }, // dollar
  { L"&dot",         0x002E }, // dot
  { L"&dotdot",      0x2025 }, // double dot
  { L"&doubleC",     0x2102 }, // double C
  { L"&doubleN",     0x2115 }, // double N
  { L"&doubleP",     0x2119 }, // double P
  { L"&doubleQ",     0x211A }, // double Q
  { L"&doubleR",     0x211D }, // double R
  { L"&doubleZ",     0x2124 }, // double Z
  { L"&dsc",         0x005D }, // declaration subset close
  { L"&dso",         0x005B }, // declaration subset open
  { L"&ea",          0x00E9 }, // e acute
  { L"&ebin",        0x0027 }, // binary end
  { L"&ec",          0x00EA }, // e circumflex
  { L"&echar",       0x0027 }, // character end
  { L"&ee",          0x00EB }, // e umlaut
  { L"&eg",          0x00E8 }, // egrave
  { L"&egml",        0x002E }, // gml end tag delimiter
  { L"&ehex",        0x0027 }, // end quoted hex string
  { L"&ellip",       0x2026 }, // ellipsis
  { L"&ellipsis",    0x2026 }, // ellipsis
  { L"&emdash",      0x2014 }, // em dash
  { L"&endash",      0x2013 }, // en dash, dash
  { L"&epsilon",     0x03B5 }, // epsilon
  { L"&eq",          0x003D }, // equals
  { L"&eqsym",       0x003D }, // equals
  { L"&equals",      0x003D }, // equals
  { L"&eqv",         0x007E }, // equivalent
  { L"&erc",         0x003B }, // entity reference close
  { L"&eta",         0x03B7 }, // eta
  { L"&eth",         0x00F0 }, // eth, Icelandic small
  { L"&euler",       0x2107 }, // Eulers
  { L"&euro",        0x20AC }, // Either Euro glyph or EUR
  { L"&eurochar",    0x20AC }, // Either Euro glyph or E
  { L"&exists",      0x2203 }, // exists
  { L"&face",        0x263A }, // face
  { L"&factorial",   0x0021 }, // factorial
  { L"&female",      0x2640 }, // female symbol
  { L"&ff",          0xFB00 }, // ff ligature
  { L"&ffi",         0xFB03 }, // ffi ligature
  { L"&ffl",         0xFB04 }, // ffl ligature
  { L"&fi",          0xFB01 }, // fi ligature
  { L"&fl",          0xFB02 }, // fl ligature
  { L"&florin",      0x0192 }, // florin
  { L"&fnof",        0x0192 }, // function of
  { L"&frac12",      0x00BD }, // one half
  { L"&frac14",      0x00BC }, // one quarter
  { L"&frac18",      0x215B }, // one eighth
  { L"&frac34",      0x00BE }, // three fourths
  { L"&frac38",      0x215C }, // three eighths
  { L"&frac58",      0x215D }, // five eighths
  { L"&frac78",      0x215E }, // seven eighths
  { L"&gamma",       0x03B3 }, // gamma
  { L"&ge",          0x2265 }, // greater than or equal to
  { L"&gerank",      0x227D }, // greater than or equal rank
  { L"&gesym",       0x2265 }, // greater than or equal to
  { L"&gml",         0x003A }, // gml delimiter
  { L"&grave",       0x0060 }, // accent grave
  { L"&gtequiv",     0x2273 }, // greater than or equivilent
  { L"&gtgt",        0x226B }, // much greater than
  { L"&gtlt",        0x2277 }, // greater than or less than
  { L"&gtrank",      0x227B }, // greater than rank
  { L"&hamilton",    0x210B }, // hamiltonian
  { L"&hat",         0x005E }, // hat
  { L"&hbar",        0x0127 }, // h bar
  { L"&house",       0x2302 }, // house
  { L"&hyphen",      0x002D }, // hyphen
  { L"&ia",          0x00ED }, // i acute
  { L"&ic",          0x00EE }, // i circumflex
  { L"&identical",   0x2261 }, // identical
  { L"&idotless",    0x0131 }, // i dotless
  { L"&ie",          0x00EF }, // i umlaut
  { L"&iff",         0x21D4 }, // if and only if
  { L"&ig",          0x00EC }, // i grave
  { L"&ij",          0x0133 }, // ij ligature
  { L"&increase",    0x2197 }, // increase
  { L"&infinity",    0x221E }, // infinity
  { L"&intbot",      0x2321 }, // integral bottom half
  { L"&integral",    0x222B }, // integral
  { L"&intersect",   0x2229 }, // intersection of sets
  { L"&inttop",      0x2320 }, // integral top half
  { L"&inve",        0x00A1 }, // inverted exclamation
  { L"&invellip",    0x22EE }, // indented vertical ellipsis
  { L"&invq",        0x00BF }, // inverted question mark
  { L"&iota",        0x03B9 }, // iota
  { L"&isubset",     0x2286 }, // improper subset
  { L"&isuperset",   0x2287 }, // improper superset
  { L"&join",        0x222A }, // join
  { L"&kappa",       0x03BA }, // kappa
  { L"&lahead",      0x25C4 }, // left arrowhead
  { L"&lambda",      0x03BB }, // lambda
  { L"&larrow",      0x2190 }, // left arrow
  { L"&lbarb",       0x21BC }, // left barb
  { L"&lbrace",      0x007B }, // left brace
  { L"&lbracket",    0x005B }, // left bracket
  { L"&lbrc",        0x007B }, // left brace
  { L"&lbrk",        0x005B }, // left bracket
  { L"&lbullet",     0x25CF }, // large bullet
  { L"&ldarrow",     0x21D0 }, // left double arrow
  { L"&le",          0x2264 }, // less than or equal to
  { L"&lerank",      0x227C }, // less than or equal rank
  { L"&less",        0x2272 }, // less than or equivilent
  { L"&lesym",       0x2264 }, // less than or equal to
  { L"&lita",        0x0027 }, // alternate literal delimiter
  { L"&liter",       0x2113 }, // liter
  { L"&lmultdot",    0x2022 }, // multiply dot large
  { L"&lnot",        0x00AC }, // logical not
  { L"&lnotrev",     0x2310 }, // backward logical not
  { L"&lnotusd",     0x2319 }, // upside down not
  { L"&lor",         0x007C }, // logical or
  { L"&loz",         0x25CA }, // lozenge
  { L"&lozenge",     0x25CA }, // lozenge
  { L"&lpar",        0x0028 }, // left parenthesis
  { L"&lparen",      0x0028 }, // left parenthesis
  { L"&lrarrow",     0x2194 }, // left-right arrow
  { L"&ltequiv",     0x2272 }, // ltequiv
  { L"&ltlt",        0x226A }, // much less than
  { L"&ltrank",      0x227A }, // less than rank
  { L"&male",        0x2642 }, // male symbol
  { L"&mathast",     0x002A }, // mathematics asterisk
  { L"&mdash",       0x2014 }, // em dash
  { L"&meet",        0x2229 }, // meet
  { L"&memberof",    0x2208 }, // member of
  { L"&minus",       0x002D }, // minus operation
  { L"&minusop",     0x002D }, // minus operation
  { L"&mp",          0x2213 }, // minus-plus
  { L"&mu",          0x03BC }, // mu
  { L"&mult",        0x00D7 }, // multiply
  { L"&ndash",       0x2013 }, // en dash, dash
  { L"&ne",          0x2260 }, // not equal to
  { L"&nearly",      0x2248 }, // nearly equal
  { L"&nesym",       0x2260 }, // not equal to
  { L"&nexists",     0x2204 }, // not existant
  { L"&nidentical",  0x2262 }, // not identical
  { L"&nisubset",    0x2288 }, // not improper subset
  { L"&nisuperset",  0x2289 }, // not improper superset
  { L"&nlerank",     0x22E0 }, // not less or equal rank
  { L"&nltrank",     0x2280 }, // not less than rank
  { L"&nmemberof",   0x2209 }, // not a member of
  { L"&nnearly",     0x2249 }, // not nearly equal
  { L"&note1616",    0x266C }, // pair of 16th notes
  { L"&note18",      0x266A }, // eighth note
  { L"&notsym",      0x00AC }, // not symbol
  { L"&nsubset",     0x2284 }, // not a subset
  { L"&nsuperset",   0x2285 }, // not a superset
  { L"&nt",          0x00F1 }, // n tilde
  { L"&nu",          0x03BD }, // nu
  { L"&numsign",     0x0023 }, // number sign
  { L"&oa",          0x00F3 }, // o acute
  { L"&oc",          0x00F4 }, // o circumflex
  { L"&odq",         0x201C }, // open double quote
  { L"&odqf",        0x00AB }, // French open double quote
  { L"&odqg",        0x201E }, // German open double quote
  { L"&oe",          0x00F6 }, // o umlaut
  { L"&oelig",       0x0153 }, // oe ligature
  { L"&og",          0x00F2 }, // o grave
  { L"&omega",       0x03C9 }, // omega
  { L"&omicron",     0x03BF }, // omicron
  { L"&or",          0x2228 }, // or symbol
  { L"&os",          0x00F8 }, // o slash
  { L"&osq",         0x2018 }, // open single quote
  { L"&osqg",        0x201A }, // German open single quote
  { L"&ot",          0x00F5 }, // o tilde
  { L"&otq",         0x201C }, // open typographic quote
  { L"&ous",         0x00BA }, // underscored o
  { L"&overline",    0x203E }, // overline
  { L"&par",         0x00B6 }, // paragraph
  { L"&parallel",    0x2225 }, // parallel
  { L"&partial",     0x2202 }, // partial
  { L"&per",         0x002E }, // period (starter set)
  { L"&percent",     0x0025 }, // percent
  { L"&period",      0x002E }, // period
  { L"&pero",        0x0025 }, // parm entity ref; open
  { L"&perpend",     0x2534 }, // perpendicular
  { L"&peseta",      0x20A7 }, // peseta
  { L"&phi",         0x03C6 }, // phi
  { L"&pi",          0x03C0 }, // pi
  { L"&planck",      0x0127 }, // h bar
  { L"&plus",        0x002B }, // plus
  { L"&plusend",     0x002B }, // plus at end of line
  { L"&plusmin",     0x00B1 }, // plus-minus
  { L"&plusop",      0x002B }, // plus operation
  { L"&pm",          0x00B1 }, // plus-minus
  { L"&prime",       0x2032 }, // prime
  { L"&product",     0x220F }, // product
  { L"&proportion",  0x221D }, // proportion
  { L"&psi",         0x03C8 }, // psi
  { L"&quest",       0x003F }, // question mark
  { L"&rahead",      0x25BA }, // right arrowhead
  { L"&rarrow",      0x2192 }, // right arrow
  { L"&ratio",       0x003A }, // ratio
  { L"&rbarb",       0x21C0 }, // right barb
  { L"&rbl",         0x00A0 }, // required blank
  { L"&rbrace",      0x007D }, // right brace
  { L"&rbracket",    0x005D }, // right bracket
  { L"&rbrc",        0x007D }, // right brace
  { L"&rbrk",        0x005D }, // right bracket
  { L"&rdarrow",     0x21D2 }, // right double arrow
  { L"&regtm",       0x00AE }, // registered trademark
  { L"&revbul",      0x25D8 }, // reverse bullet
  { L"&revcir",      0x25D9 }, // reverse circle
  { L"&rho",         0x03C1 }, // rho
  { L"&riemann",     0x211B }, // riemann integral
  { L"&rpar",        0x0029 }, // right parenthesis
  { L"&rparen",      0x0029 }, // right parenthesis
  { L"&rprime",      0x0060 }, // right prime
  { L"&scriptI",     0x2110 }, // script I
  { L"&scriptl",     0x2113 }, // liter
  { L"&sect",        0x00A7 }, // double S
  { L"&section",     0x00A7 }, // double S
  { L"&semi",        0x003B }, // semicolon
  { L"&shiftin",     0x240F }, // double byte shift in
  { L"&shiftout",    0x240E }, // double byte shift out
  { L"&sigma",       0x03C3 }, // sigma
  { L"&similar",     0x007E }, // similar
  { L"&slash",       0x2044 }, // slash right
  { L"&slr",         0x2044 }, // slash right
  { L"&smultdot",    0x22C5 }, // muliply dot small
  { L"&splitvbar",   0x00A6 }, // split veritical bar
  { L"&sqbul",       0x25AA }, // square bullet
  { L"&sqbullet",    0x25AA }, // square bullet
  { L"&sqrt",        0x221A }, // square root
  { L"&ss",          0x00DF }, // German es-zet
  { L"&ssbl",        0x00A0 }, // syntax significant blank
  { L"&ssq",         0x0027 }, // straight single quote
  { L"&sub0",        0x2080 }, // subscript 0
  { L"&sub1",        0x2081 }, // subscript 1
  { L"&sub2",        0x2082 }, // subscript 2
  { L"&sub3",        0x2083 }, // subscript 3
  { L"&sub4",        0x2084 }, // subscript 4
  { L"&sub5",        0x2085 }, // subscript 5
  { L"&sub6",        0x2086 }, // subscript 6
  { L"&sub7",        0x2087 }, // subscript 7
  { L"&sub8",        0x2088 }, // subscript 8
  { L"&sub9",        0x2089 }, // subscript 9
  { L"&sublpar",     0x208D }, // subscript left parenthesis
  { L"&subminus",    0x208B }, // subscript minus
  { L"&subplus",     0x208A }, // subscript plus
  { L"&subrpar",     0x208E }, // subscript right parenthesis
  { L"&subset",      0x2282 }, // subset of, included in
  { L"&suchthat",    0x220B }, // such that
  { L"&sum",         0x2211 }, // sum
  { L"&sun",         0x263C }, // sun
  { L"&sup0",        0x2070 }, // superscript 0
  { L"&sup1",        0x00B9 }, // superscript 1
  { L"&sup2",        0x00B2 }, // superscript 2
  { L"&sup3",        0x00B3 }, // superscript 3
  { L"&sup4",        0x2074 }, // superscript 4
  { L"&sup5",        0x2075 }, // superscript 5
  { L"&sup6",        0x2076 }, // superscript 6
  { L"&sup7",        0x2077 }, // superscript 7
  { L"&sup8",        0x2078 }, // superscript 8
  { L"&sup9",        0x2079 }, // superscript 9
  { L"&superset",    0x2283 }, // superset
  { L"&suplpar",     0x0028 }, // superscript left parenthesis
  { L"&supminus",    0x002D }, // superscript minus
  { L"&supn",        0x006E }, // superscript n
  { L"&supplus",     0x002B }, // superscript plus
  { L"&suprpar",     0x0029 }, // superscript right parenthesis
  { L"&tau",         0x03C4 }, // tau
  { L"&telephone",   0x260F }, // telephone
  { L"&therefore",   0x2234 }, // therefore
  { L"&theta",       0x03B8 }, // theta
  { L"&thorn",       0x00FE }, // thorn, Icelandic small
  { L"&tilde",       0x007E }, // tilde
  { L"&times",       0x00D7 }, // multiply
  { L"&tm",          0x2122 }, // trademark
  { L"&triangle",    0x25B3 }, // triangle
  { L"&ua",          0x00FA }, // u acute
  { L"&uahead",      0x25B2 }, // up arrowhead
  { L"&uarrow",      0x2191 }, // up arrow
  { L"&uc",          0x00FB }, // u circumflex
  { L"&udarrow",     0x2195 }, // up-down arrow
  { L"&udarrowus",   0x21A8 }, // up/down arrow/underscore
  { L"&ue",          0x00FC }, // u umlaut
  { L"&ug",          0x00F9 }, // u grave
  { L"&ulbarb",      0x21BF }, // up lef barb
  { L"&umlaut",      0x00A8 }, // umlaut
  { L"&union",       0x222A }, // union of 2 sets
  { L"&upsilon",     0x03C5 }, // upsilon
  { L"&urbarb",      0x21BE }, // up right barb
  { L"&us",          0x005F }, // underscore
  { L"&usec",        0x00B5 }, // micro second
  { L"&vardelta",    0x018D }, // delta (variation)
  { L"&varphi",      0x03D5 }, // phi (variation)
  { L"&varsigma",    0x03C2 }, // sigma (variation)
  { L"&vartheta",    0x03D1 }, // theta (variation)
  { L"&vbar",        0x007C }, // vertical bar
  { L"&vector",      0x21C0 }, // vector
  { L"&vellip",      0x22EE }, // vertical ellipsis
  { L"&weierstr",    0x2118 }, // weierstrass elliptic
  { L"&won",         0x20A9 }, // won (Korean currency)
  { L"&xclam",       0x0021 }, // exclamation point
  { L"&xclm",        0x0021 }, // exclamation point
  { L"&xi",          0x03BE }, // xi
  { L"&ya",          0x00FD }, // y acute
  { L"&ye",          0x00FF }, // y umlaut
  { L"&yen",         0x00A5 }, // yen
  { L"&zero",        0x00D8 }, // zero slashed
  { L"&zeta",        0x03B6 }, // zeta
  { L"",                 0 }
};

// the following value is computed automatically
static int numOfIDDOCVars = 0;

int DITACompareEntries( const void *p1, const void *p2 )
{
 PIBMIDDOCVAR pVar1 = (PIBMIDDOCVAR)p1;
 PIBMIDDOCVAR pVar2 = (PIBMIDDOCVAR)p2;
 return( wcscmp( pVar1->szVarName, pVar2->szVarName ) );
}

// replace &...; variables with their character representation
BOOL DITAReplaceIDDOCVars( PSZ_W pszData )
{
  PSZ_W       pszTarget = pszData;
  BOOL        fReplaced = FALSE;

  while ( *pszData )
  {
    if ( *pszData == L'&' )
    {
      BOOL fFound = FALSE;
      PIBMIDDOCVAR pVar = IDDOCVars;

      PSZ_W pszTemp = pszData + 1;
      while ( iswalpha( *pszTemp ) ) pszTemp++; 

      if ( *pszTemp == L';')
      {
        // count number of IDDOCVars if not done yet
        if ( numOfIDDOCVars == 0)
        {
          pVar = IDDOCVars;
          while ( pVar->szVarName[0] )
          {
            numOfIDDOCVars++;
            pVar++;
          } /*endwhile */
        } /* endif */

        // isolate character entity name
        *pszTemp = 0;

        // search name in table
        if ( wcslen( pszData ) < (sizeof(IDDOCVars[0].szVarName) / 2 ) )
        {
          IBMIDDOCVAR Key;
          wcscpy( Key.szVarName, pszData );
          pVar = (PIBMIDDOCVAR)bsearch( &Key, IDDOCVars, numOfIDDOCVars, sizeof(IBMIDDOCVAR), DITACompareEntries );  
        }
        else
        {
          pVar = NULL;
        } /* endif */
        *pszTemp = L';';

        // if found replace entity with its character value
        if ( pVar )
        {
          pszData = pszTemp + 1;
          *pszTarget++ = pVar->chValue;
          fFound = TRUE;
          fReplaced = TRUE;
        } /* endif */
      } /* endif */

      if ( !fFound ) 
      {
        *pszTarget++ = *pszData++;
      } /* endif */
    }
    else
    {
      *pszTarget++ = *pszData++;
    } /* endif */
  } /*endwhile */
  *pszTarget = 0;

  return( fReplaced );
} /* end of function DITAReplaceIDDOCVars */

// removed superfluos whitespace at the beggining or end of the proposal
BOOL DITARemoveAdditionalWhitepace
(
  PSZ_W         pszSource, 
  PSZ_W         pszProposal 
)
{
  int           iLen;
  BOOL          fChanged = FALSE;

  // only remove leading whitespace if source has none
  if ( (*pszSource != L'\n') && (*pszSource != L'\r') && (*pszSource != L' ') )
  {
    PSZ_W pszProp = pszProposal;
    PSZ_W pszTemp = pszProposal;

    while ( (*pszTemp == L'\n') || (*pszTemp == L'\r') || (*pszTemp == L' ') )
    {
      pszTemp++;
    } /*endwhile */

    if ( pszTemp != pszProp )
    {
      while ( *pszTemp )
      {
        *pszProp++ = *pszTemp++;
      } /*endwhile */
      fChanged = TRUE;
      *pszProp = 0;
    } /* endif */
  } /* endif */

  // check trailing whitespace
  iLen = wcslen( pszSource );
  if ( iLen )
  {
    iLen--;
    if  ( (pszSource[iLen] != L'\n') && (pszSource[iLen] != L'\r') && (pszSource[iLen] != L' ') )
    {
      iLen = wcslen( pszProposal );
      while ( iLen &&  ( (pszProposal[iLen-1] == L'\n') || (pszProposal[iLen-1] == L'\r') || (pszProposal[iLen-1] == L' ') ) )
      {
        iLen--;
        fChanged = TRUE;
        pszProposal[iLen] = 0;
      } /*endwhile */
    } /* endif */
  } /* endif */
  return( fChanged );
} /* end of function DITARemoveAdditionalWhitepace */

// table with DITA phrases for IDDOC entitites
typedef struct _DITAENTITY
{
  CHAR_W      szStartTag[40];
  int         iStartTagLen;
  CHAR_W      szEndTag[40];
  int         iEndTagLen;
  CHAR_W      szRefAttr[40];
  int         iRefAttrLen;
} DITAENTITY, *PDITAENTITY;

DITAENTITY DitaEntity[] =
{
  { L"<keyword",  8,   L"</keyword>", 10, L"conref=", 7  },
  { L"<ph",       3,   L"</ph>",       5, L"conref=", 7  },
  { L"",          0,   L"",            0, L"",        0  }
};

// find <keyword> or <ph> phrase for the given entity
BOOL DITAFindEntity
( 
  PSZ_W       pszSource,                         // source segment 
  PSZ_W       pszEntity,                         // entity used in proposal             
  int         *piLength,                         // ptr to caller's length field
  PSZ_W       *ppszPhrase                        // ptr to caller's phrase field 
)
{
  PSZ_W pszSearch = pszSource;
  BOOL  fFound = FALSE;

  while ( *pszSearch && !fFound )
  {
    if ( *pszSearch == L'<' )
    {
      BOOL    fTagMatch = FALSE;                 // TRUE = matching tag found
      PDITAENTITY pEnt = DitaEntity;             // ptr to table entry 
      PSZ_W   pszTagEnd = NULL;                  // ptr to end character of start tag
      int     iPhraseLen = 0;                    // length of complete phrase (including end tag)

      while ( !fTagMatch && pEnt->iStartTagLen )
      {
        if ( wcsncmp( pszSearch, pEnt->szStartTag, pEnt->iStartTagLen ) == 0 )
        {
          fTagMatch = TRUE;
        }
        else
        {
          pEnt++;
        } /* endif */
      } /*endwhile */

      // find end character of tag
      if ( fTagMatch )
      {
        pszTagEnd = wcschr( pszSearch, L'>' );
        if ( !pszTagEnd )
        {
          fTagMatch = FALSE;
        } /* endif */
      } /* endif */

      // check if corresponding end tag is directly following this tag
      if ( fTagMatch)
      {
        if ( wcsncmp( pszTagEnd + 1, pEnt->szEndTag, pEnt->iEndTagLen ) != 0 )
        {
          fTagMatch = FALSE;
        }
        else
        {
          iPhraseLen = (pszTagEnd - pszSearch) + 1 + pEnt->iEndTagLen;
        } /* endif */
      } /* endif */

      // find reference attribute and check reference value
      if ( fTagMatch )
      {
        PSZ_W pszReference = NULL;
         
        *pszTagEnd = 0;                          // set temporary string end at end of tag

        pszReference = wcsstr( pszSearch, pEnt->szRefAttr );
        if ( pszReference )
        {
          if ( pszReference[pEnt->iRefAttrLen] == L'\"' )
          {
            PSZ_W pszEndOfValue = wcschr( pszReference + pEnt->iRefAttrLen + 1, L'\"' );
            if ( pszEndOfValue )
            {
              PSZ_W pszLastSlash;

              *pszEndOfValue = 0;
              pszLastSlash = wcschr( pszReference + pEnt->iRefAttrLen, L'/' );
              if ( pszLastSlash )
              {
                if ( wcscmp( pszLastSlash + 1, pszEntity ) == 0 )
                {
                  fFound = TRUE; 
                  *piLength = iPhraseLen;
                  *ppszPhrase = pszSearch; 
                }
                else
                {
                  fTagMatch = FALSE;             // entity does not match 
                } /* endif */
              }
              else
              {
                fTagMatch = FALSE;               // no slash in reference of value found 
              } /* endif */

              *pszEndOfValue = L'\"';
            }
            else
            {
              fTagMatch = FALSE;                 // no end of value found 
            } /* endif */
          }
          else
          {
            fTagMatch = FALSE;                   // no value following reference attribute
          } /* endif */
        }
        else
        {
          fTagMatch = FALSE;                     // no reference attribute found
        } /* endif */

        *pszTagEnd = L'>';
      } /* endif */

      if ( !fFound )
      {
        if ( iPhraseLen )
        {
          pszSearch += iPhraseLen;
        }
        else
        {
          pszSearch += 1;
        } /* endif */
      } /* endif */
    }
    else
    {
      pszSearch++;
    } /* endif */
  } /*endwhile */

  return( fFound );
} /* end of function DITAFindEntity */

// convert entities to <keyword..> or <ph> sequences
BOOL DITAConvertEntities
( 
  PSZ_W pszSource, 
  PSZ_W pszProposal 
)
{
  static CHAR_W szNewProposal[8096];             // buffer for new proposal
  PSZ_W   pszOrg = pszProposal;                  // work for original proposal
  PSZ_W   pszNew = szNewProposal;                // work pointer for new proposal  
  BOOL    fModified = FALSE;                     // proposal-has-been-modified flag 

  while ( *pszOrg )
  {
    if ( *pszOrg == L'&' )
    {
      PSZ_W pszEnd = pszOrg + 1;
      while ( iswalpha(*pszEnd) ) pszEnd++;
      if ( *pszEnd == L';' )
      {
        BOOL fFound = FALSE;
        int  iLength = 0;                        // length of found phrase
        PSZ_W  pszPos = NULL;                    // position of found phrase

        // look for a keyword or ph phrase for this entity
        *pszEnd = 0;
        fFound = DITAFindEntity( pszSource, pszOrg + 1, &iLength, &pszPos );
        *pszEnd = L';';

        if ( fFound)
        {
          wcsncpy( pszNew, pszPos, iLength );
          pszNew += iLength;
          pszOrg = pszEnd + 1;
          fModified = TRUE;
        }
        else
        {
          *pszNew++ = *pszOrg++;                  // copy to new posposal as-is
        } /* endif */
      }
      else
      {
        // no entity found, 
        *pszNew++ = *pszOrg++;                    // copy to new posposal as-is
      } /* endif */
    }
    else
    {
      *pszNew++ = *pszOrg++;                     // copy to new posposal as-is
    } /* endif */
  } /*endwhile */
  *pszNew = 0;                                   // terminate new proposal

  // use new proposal if modified and modified segment does not excceed max segment length
  if ( fModified )
  {
    if ( wcslen( szNewProposal ) >= MAX_SEGMENT_SIZE )
    {
      fModified = FALSE;                         // new proposal is too large, ignore this change
    }
    else
    {
      wcscpy( pszProposal, szNewProposal );      // replace original proposal with modified one 
    } /* endif */
  } /* endif */

  return( fModified );
} /* end of function DITAConvertEntities */


//
// Implementation of Repl SAX parser
//
ReplListParseHandler::ReplListParseHandler( PTMO_REPLACE_LIST pInList )
{
  m_pList = pInList;
}

ReplListParseHandler::~ReplListParseHandler()
{
}

void ReplListParseHandler::startElement
(
  const XMLCh* const name, 
  AttributeList& attributes
)
{
  PSZ_W pszName = (PSZ_W)name;

  attributes;                                    // currently not used

  m_szTextBuffer[0] = EOS;                       // clear text buffer

  if ( wcsicmp( pszName, L"ChangeEntry" ) == 0 )
  {
    // start a new change entry
    memset( &m_CurEntry, 0, sizeof(m_CurEntry) );
    for( int i = 0; i < MAX_CHANGES_PER_ENTRY; i++ )
    {
      m_CurEntry.aiOffsFrom[i] = -1;
      m_CurEntry.aiOffsTo[i] = -1;
    } /* endfor */
  } 
  else  if ( wcsicmp( pszName, L"From" ) == 0 )
  {
    // start text capturing
    m_fCaptureText = TRUE; 
  } 
  else  if ( wcsicmp( pszName, L"To" ) == 0 )
  {
    // start text capturing
    m_fCaptureText = TRUE; 
  } /* endif */

} /* end of method ReplListParseHandler::startElement */

void ReplListParseHandler::endElement(const XMLCh* const name )
{
  PSZ_W pszName = (PSZ_W)name;

  m_fCaptureText = FALSE;                        // stop text capturing

  if ( wcsicmp( pszName, L"ChangeEntry" ) == 0 )
  {
    // check for single replace entries without <change> tags around them
    if ( !m_CurEntry.iEntries )
    {
      if ( m_CurEntry.aiOffsFrom[0] != -1 )
      {
        m_CurEntry.iEntries = 1;
      }
      else
      {
        if ( !m_pList->fErrorInList )
        {
          m_pList->fErrorInList = TRUE;            // TRUE = errors occurend while loading this list
          strcpy( m_pList->szErrorText, "Missing <From> in <ChangeEntry>" );
        } /* endif */
      } /* endif */
    } /* endif */

    // add entry to the replacement list
    if ( m_CurEntry.iEntries )
    {
      AddChangeEntry( &m_CurEntry );
    } /* endif */
  } 
  else  if ( wcsicmp( pszName, L"Change" ) == 0 )
  {
    if ( m_CurEntry.iEntries > MAX_CHANGES_PER_ENTRY )
    {
      if ( !m_pList->fErrorInList )
      {
        m_pList->fErrorInList = TRUE;            
        strcpy( m_pList->szErrorText, "More than 10 changes in a <ChangeEntry>" );
      } /* endif */
    }  
    else if (m_CurEntry.aiOffsFrom[m_CurEntry.iEntries] == -1 )
    {
      m_pList->fErrorInList = TRUE;            
      strcpy( m_pList->szErrorText, "Missing <From> in <Change> element" );
    }
    else
    {
      m_CurEntry.iEntries += 1;
    } /* endif */
  } 
  else  if ( wcsicmp( pszName, L"From" ) == 0 )
  {
    m_CurEntry.aiOffsFrom[m_CurEntry.iEntries] = AddStringToBuffer( m_szTextBuffer );
  } 
  else  if ( wcsicmp( pszName, L"To" ) == 0 )
  {
    m_CurEntry.aiOffsTo[m_CurEntry.iEntries] = AddStringToBuffer( m_szTextBuffer );
  } /* endif */
} /* end of method ReplListParseHandler::endElement */

void ReplListParseHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
  PSZ_W pszChars = (PSZ_W)chars;

  // store characters in our buffer
  if ( m_fCaptureText )
  {
    int iCurLen = wcslen( m_szTextBuffer );
    if ( iCurLen + length >= MAX_CAPTURE_LEN )
    {
      if ( !m_pList->fErrorInList )
      {
        m_pList->fErrorInList = TRUE;            
        strcpy( m_pList->szErrorText, "A string in the list is too large" );
      } /* endif */
    }
    else
    {
      wcsncpy( m_szTextBuffer + iCurLen, pszChars, length );
      m_szTextBuffer[iCurLen + length] = 0;
    } /* endif */
  } /* endif */
} /* end of method ReplListParseHandler::characters */

int ReplListParseHandler::AddStringToBuffer( PSZ_W pszText )
{
  int iOffset = 0;

  int iTextLen = wcslen( pszText ) + 1;

  // enlarge buffer area if required
  if ( (m_pList->iBufferSize - m_pList->iBufferUsed) < iTextLen )
  {
    int iNewSize = m_pList->iBufferSize + max ( 4096, iTextLen );
    if ( !UtlAlloc( (PVOID *)&(m_pList->pszStringBuffer), m_pList->iBufferSize * sizeof(CHAR_W), 
         iNewSize * sizeof(CHAR_W), NOMSG ) )
    {
      if ( !m_pList->fErrorInList )
      {
        m_pList->fErrorInList = TRUE;            
        strcpy( m_pList->szErrorText, "Memory allocation failed" );
      } /* endif */
      return( -1 );
    } /* endif */
    m_pList->iBufferSize = iNewSize;
  } /* endif */

  // add string to buffer
  iOffset = m_pList->iBufferUsed;
  wcscpy( m_pList->pszStringBuffer + iOffset, pszText );
  m_pList->iBufferUsed += iTextLen;

  return( iOffset );
} /* end of method ReplListParseHandler::AddStringToBuffer */

BOOL ReplListParseHandler::AddChangeEntry( PTMO_REPLACE_ENTRY pEntry )
{
  // enlarge change entry list if required
  if ( m_pList->iEntriesSize == m_pList->iEntriesUsed )
  {
    int iNewSize = m_pList->iEntriesSize + 10;
    if ( !UtlAlloc( (PVOID *)&(m_pList->pEntries), m_pList->iEntriesSize * sizeof(TMO_REPLACE_ENTRY), 
                    iNewSize * sizeof(TMO_REPLACE_ENTRY), NOMSG ) )
    {
      if ( !m_pList->fErrorInList )
      {
        m_pList->fErrorInList = TRUE;
        strcpy( m_pList->szErrorText, "Memory allocation failed" );
      } /* endif */
      return( FALSE );
    } /* endif */
    m_pList->iEntriesSize = iNewSize;
  } /* endif */

  // compute overall length of change-from strings in new entry
  pEntry->iFromLength = 0;
  for( int i = 0; i < pEntry->iEntries; i++ )
  {
    pEntry->iFromLength += wcslen( m_pList->pszStringBuffer + pEntry->aiOffsFrom[i] );
  } /* endfor */

  // look for correct place to insert new entry (based on change from length)
  int iPos = 0;
  while ( (iPos < m_pList->iEntriesUsed) && (m_pList->pEntries[iPos].iFromLength > pEntry->iFromLength) )
  {
    iPos++;
  } /*endwhile */

  // make room for new entry
  if ( iPos < m_pList->iEntriesUsed )
  {
    memmove( m_pList->pEntries + iPos + 1, m_pList->pEntries + iPos, 
             (m_pList->iEntriesUsed - iPos) * sizeof(TMO_REPLACE_ENTRY) ); 
  } /* endif */

  // add new entry to list
  memcpy( m_pList->pEntries + iPos, pEntry, sizeof(TMO_REPLACE_ENTRY) ); 
  m_pList->iEntriesUsed += 1;

  return( TRUE );
} /* end of method ReplListParseHandler::AddStringToBuffer */

void ReplListParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    m_pList->fErrorInList = TRUE;
    strcpy( m_pList->szErrorText, message );
    XMLString::release( &message );
}

void ReplListParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    m_pList->fErrorInList = TRUE;
    strcpy( m_pList->szErrorText, message );
    XMLString::release( &message );
}

void ReplListParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    XMLString::release( &message );
}

// apply a replacement list on source and target of the proposal
BOOL DITAApplyReplaceList
( 
  PTMO_REPLACE_LIST pList,                       // replacement list to be applied
  PSZ_W       pszSource,                         // proposal source 
  PSZ_W       pszTarget                          // proposal target  
)
{
  static CHAR_W szNewSource[8096];               // buffer for new proposal text 
  static CHAR_W szNewTarget[8096];               // buffer for new target text
  BOOL        fModified = FALSE;                 // TRUE = source and proposal have been modified

  for( int i = 0; i < pList->iEntriesUsed; i++ )
  {
    // apply single replace entry
    int iTargetChanges = 0;
    int iSourceChanges = DITAApplySingleReplacement( pList, i, pszSource, szNewSource );

    // only try to change target string if source has been changed
    if ( iSourceChanges )
    {
      iTargetChanges = DITAApplySingleReplacement( pList, i, pszTarget, szNewTarget );
    } /* endif */

    // only use replacement if source and target change numbers are the same 
    if ( iSourceChanges == iTargetChanges )
    {
      if ( iSourceChanges )
      {
        fModified = TRUE;
        wcscpy( pszSource, szNewSource );
        wcscpy( pszTarget, szNewTarget );
      } /* endif */
    } /* endif */
  } /* endfor */
  return( fModified );
} /* end of function DITAApplyReplaceList */

// apply a replacment entry on a string and return number of changes performed
int  DITAApplySingleReplacement
( 
  PTMO_REPLACE_LIST pList,                       // replacement list            
  int         iEntry,                            // entry in list to be applied on string
  PSZ_W       pszInString,                       // input string
  PSZ_W       pszOutString                       // string with performed changes
)
{
  int iNumOfChanges = 0;
  PTMO_REPLACE_ENTRY pEntry = pList->pEntries + iEntry;
  BOOL fFound = TRUE;                            // from strings found in string 

  // for multiple changes: check if all from strings are contained in string
  if ( pEntry->iEntries > 1 )
  {
    int i = 0;
    while ( fFound && (i < pEntry->iEntries) )
    {
      PSZ_W pszFrom = pList->pszStringBuffer + pEntry->aiOffsFrom[i];
      if ( wcsstr( pszInString, pszFrom ) == NULL )
      {
        fFound = FALSE;
      } /* endif */
      i++;
    } /*endwhile */
  } /* endif */

  // apply all replacements
  if ( fFound )
  {
    wcscpy( pszOutString, pszInString );
    for( int i = 0; i < pEntry->iEntries; i++ )
    {
      PSZ_W pszFrom = pList->pszStringBuffer + pEntry->aiOffsFrom[i];
      PSZ_W pszTo   = (pEntry->aiOffsFrom[i] == -1 ) ? NULL : pList->pszStringBuffer + pEntry->aiOffsTo[i];
      iNumOfChanges += DITAReplaceString( pszOutString, pszFrom, pszTo );
    } /* endfor */
  } /* endif */

  return( iNumOfChanges );
} /* end of function DITAApplySingleReplacement */

// change all occurences of pszFrom in string pszString to pszTo and return number of changes
int DITAReplaceString
( 
  PSZ_W       pszString,                         // string being processed
  PSZ_W       pszFrom,                           // change from 
  PSZ_W       pszTo                              // change to  
)
{
  static CHAR_W szBuffer[8096];                  // buffer for modified string
  PSZ_W       pszSource = pszString;             // current input position
  PSZ_W       pszTarget = szBuffer;              // current output position
  int         iChanges = 0;                      // number of changes
  PSZ_W       pszFound = NULL;                   // position of found pszFrom string
  int         iFromLen = wcslen(pszFrom);        // length of pszFrom string
  int         iToLen = (pszTo != NULL) ? wcslen(pszTo) : 0; // length of pszTo string

  do
  {
    pszFound = wcsstr( pszSource, pszFrom );
    if ( pszFound != NULL )
    {
      // copy data in front of found string to pszTarget
      if ( pszSource != pszFound )
      {
        int iLen = pszFound - pszSource;
        wcsncpy( pszTarget, pszSource, iLen );
        pszTarget += iLen;
      } /* endif */

      // replace found string
      if ( iToLen )
      {
        wcsncpy( pszTarget, pszTo, iToLen );
        pszTarget += iToLen;
      } /* endif */

      // prepare for next search
      iChanges++;
      pszSource = pszFound + iFromLen;
    } /* endif */
  } while ( pszFound );

  // copy rest of data include string end delimiter
  wcscpy( pszTarget, pszSource );

  // update string if changes were applied
  if ( iChanges )
  {
    wcscpy( pszString, szBuffer );
  } /* endif */

  return( iChanges );
} /* end of function DITAReplaceString */
