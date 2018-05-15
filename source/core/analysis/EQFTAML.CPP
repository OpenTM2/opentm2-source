/*! \brief EQFTAML.CPP
	Copyright (c) 1990-2017, International Business Machines Corporation and others. All rights reserved.
	Description: XML Output related functions of TranslationManager
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_EDITORAPI        // editor API (required for EQFTPI.H)

#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF

#include "OtmProposal.h"
#include "AnalysisMemLookupData.h"
#include "core\memory\MemoryFactory.h"

  #include "eqftai.h"
  #include "eqftpi.h"
  #include "eqftaml.h"
  #include "eqfserno.h"
  #include <time.h>
#pragma pack( pop, TM2StructPacking )


#include "cxmlwriter.h"

__declspec(dllexport)
  int TAMatchListInit( PTAINPUT pTAIn );
__declspec(dllexport)
  int TAMatchListTerminate( PTAINPUT pTAIn );
__declspec(dllexport)
  int TAMatchListProcess( PTAINPUT pTAIn, PSZ_W pszSegment, PSZ_W pszProposal, MATCHTYPE MatchType, USHORT usFuzzyness );
__declspec(dllexport)
  int XSLTConversion( PSZ pszXmlFile, PSZ pszHtmlFile, PSZ pszStyleSheet, BOOL fMsg, HWND hwndErrMsg );

// includes for Xalan XSLT

// the Win32 Xerces/Xalan build requires the default structure packing...
#pragma pack( push )
#pragma pack(8)

#include <iostream>
#ifdef XALANCODEDOESWORK
  #include <xercesc/util/PlatformUtils.hpp>
  #include <xercesc/util/PlatformUtils.hpp>
  #include <xalanc/XalanTransformer/XalanTransformer.hpp>
#endif
#pragma pack( pop )

// includes for OLE interface
#include <ole2.h>
#include <oleauto.h>


// data area for match list processing
typedef struct _TAMATCHLISTDATA
{
  PVOID          pvXW;
  FILE           *hfMemory;                      // memory output file handle
  CHAR_W         szLine[4096];                   // buffer for output lines
  LONG           lNum;                           // number of currently processed segment
  CHAR_W         szBuffer[4096];                 // general working area and buffer
  CHAR_W         szSource[4096];                 // buffer for segment source
  CHAR_W         szTarget[4096];                 // buffer for segment target
  CHAR_W         szSegment[4096];                // buffer for original segment
} TAMATCHLISTDATA, *PTAMATCHLISTDATA;

// write a UTF16 string to a C stream
int TAMatchWriteToFile ( FILE *hf, PSZ_W pszData )
{
  if ( hf )
  {
    int iDataLen = UTF16strlenBYTE( pszData );
    fwrite( pszData, 1, iDataLen, hf);
  } /* endif */
  return( 0 );
}

// write a UTF16 string to a C stream, convert LFs to CRLF
int TAMatchWriteToFileWithLFHandling ( FILE *hf, PSZ_W pszData )
{
  if ( hf )
  {
    PSZ_W pszLF = pszData;
    PSZ_W pszStart = pszData;

    do
    {
      while ( *pszLF && (*pszLF != L'\n') ) pszLF++;

      if ( *pszLF )
      {
        if ( pszLF != pszStart )
        {
          *pszLF = 0;
          TAMatchWriteToFile ( hf, pszStart );
          *pszLF = '\n';
        } /* endif */
        TAMatchWriteToFile ( hf, L"\r\n" );
        pszLF++;
        pszStart = pszLF;
      }
      else
      {
        TAMatchWriteToFile ( hf, pszStart );
      } /* endif */
    } while ( *pszLF );
  } /* endif */
  return( 0 );
}

int TAMatchListInit
(
  PTAINPUT    pTAIn
)
{
  int iRC = 0;
  PTAMATCHLISTDATA pData = NULL;

  if ( pTAIn == NULL )
  {
    iRC = ERROR_INVALID_DATA;
  } /* endif */

  // allocated data area
  if ( !iRC )
  {
    if ( pTAIn->pvMatchListData ) UtlAlloc( &(pTAIn->pvMatchListData), 0L, 0L, NOMSG );

    if ( !UtlAlloc( &(pTAIn->pvMatchListData), 0L, sizeof(TAMATCHLISTDATA), NOMSG ) )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
      pData = (PTAMATCHLISTDATA)pTAIn->pvMatchListData;
    } /* endif */
  } /* endif */

  // start XML and memory output
  if ( !iRC )
  {
    CHAR szMatchList[MAX_LONGFILESPEC];

    strcpy( szMatchList, pTAIn->szFolder );
    UtlSplitFnameFromPath( szMatchList );
    strcat( szMatchList, "\\MATCHLIST" );
    UtlMkDir( szMatchList, 0, FALSE );
    strcat( szMatchList, "\\" );
    SubFolObjectNameToName( pTAIn->szFolder , szMatchList + strlen(szMatchList) );
    strcat( szMatchList, ".XML" );

    CXmlWriter *xw = new CXmlWriter( szMatchList );
    xw->Formatting = CXmlWriter::Indented;
    xw->Encoding = CXmlWriter::UTF8;
    xw->Indention = 2;

    xw->WriteStartDocument();

    xw->WriteStylesheet( "matchlist.xsl" );
    xw->WriteStartElement( "entrylist" );

    pData->pvXW = (PVOID)xw;

    PSZ pszEnd = strrchr( szMatchList, '.' );
    if ( pszEnd ) *pszEnd = EOS;
    strcat( szMatchList, ".EXP" );

    pData->hfMemory = fopen( szMatchList, "wb" );
    pData->lNum = 1;

    if ( pData->hfMemory ) fwrite( UNICODEFILEPREFIX, 1, 2, pData->hfMemory );
    TAMatchWriteToFile ( pData->hfMemory , L"<NTMMemoryDb>\r\n" );
    TAMatchWriteToFile ( pData->hfMemory , L"<Description>\r\n" );
    TAMatchWriteToFile ( pData->hfMemory , L"\r\n" );
    TAMatchWriteToFile ( pData->hfMemory , L"</Description>\r\n" );
  } /* endif */

  return( iRC );
} /* end of function TAMatchListInit */

int TAMatchListTerminate
(
  PTAINPUT    pTAIn
)
{
  int iRC = 0;

  if ( pTAIn == NULL )
  {
    iRC = ERROR_INVALID_DATA;
  } /* endif */

  // cleanup
  if ( !iRC )
  {
    if ( pTAIn->pvMatchListData )
    {
      PTAMATCHLISTDATA pData = (PTAMATCHLISTDATA)pTAIn->pvMatchListData;

      // terminate XML output
      CXmlWriter *xw = (CXmlWriter *)pData->pvXW;

      xw->WriteEndElement(); // entrylist
      xw->WriteEndDocument();
      xw->Close();

      delete xw;

      // terminate our memory file
      TAMatchWriteToFile ( pData->hfMemory , L"</NTMMemoryDb>\r\n" );

      if ( pData->hfMemory ) fclose( pData->hfMemory );

      UtlAlloc( &(pTAIn->pvMatchListData), 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( iRC );
} /* end of function TAMatchListTerminate */

int TAMatchNormalizeString
(
  PSZ_W      pszText
)
{
  PSZ_W pszSource = pszText;
  PSZ_W pszTarget = pszText;
  CHAR_W chLast = 0;

  while ( *pszSource )
  {
    if ( (*pszSource == L' ') || (*pszSource == L'\r') ||(*pszSource == L'\n') )
    {
      if ( chLast != L' ' )
      {
        chLast = L' ';
        *pszTarget++ = chLast;
      } /* endif */
      pszSource++;
    }
    else
    {
      chLast = *pszSource++;
      *pszTarget++ = chLast;
    } /* endif */
  } /*endwhile */
  *pszTarget = 0;

  return( 0 );
}

// write a single change element
int TAMatchWriteSingleChange
(
  CXmlWriter *xw,
  PSZ        pszType,
  PSZ_W      pszText,
  int        iLen
)
{
  CHAR_W chTemp;

  xw->WriteStartElement( "change" );
  xw->WriteAttributeString( "type", pszType );
  chTemp = *(pszText + iLen);
  *(pszText + iLen) = 0;
  xw->WriteString( pszText );
  *(pszText + iLen) = chTemp;
  xw->WriteEndElement(); // "change"
  return( 0 );
}

int TAMatchWriteDiff
(
  PTAMATCHLISTDATA pData,
  CXmlWriter *xw,
  PFUZZYTOK  pFuzzyTok,
  PFUZZYTOK  pTargetTok,
  PSZ_W      pszText
)
{
  int iRc = 0;
  PFUZZYTOK pToken = pFuzzyTok;
  PFUZZYTOK pTargetToken = pTargetTok;
  SHORT    sPrev = MARK_EQUAL;

  pszText;

  xw->WriteStartElement( "changelist" );

  while ( pToken->ulHash )
  {
    PSZ_W pszStart = pToken->pData;
    PSZ_W pszTargetStart = pTargetToken->pData;
    int iLen = 0;

    switch ( pToken->sType )
    {
      case MARK_DELETED:
        while ( pToken->sType == MARK_DELETED )
        {
          iLen += pToken->usStop - pToken->usStart + 1;
          pToken++;
        } /* endwhile */
        pToken--;        // undo last increment

        TAMatchWriteSingleChange( xw, "delete", pszStart, iLen );
        break;

      case MARK_INSERTED:
        {
          BOOL fProcessed = FALSE;
          int iTokLen = pToken->usStop - pToken->usStart + 1;

          // test for trademark token and to specific handling...
          if ( (iTokLen >= 4) && (wcsnicmp( pszStart, L"<tm", 3 ) == 0) )
          {
            BOOL fFound = FALSE;
            PFUZZYTOK pTokenTest = pToken;

            // find trademark end token
            while ( !fFound && pTokenTest->ulHash &&
                    ((pTokenTest->sType == MARK_INSERTED) || (pTokenTest->sType == MARK_EQUAL) ) )
            {
              if ( pTokenTest->sType == MARK_INSERTED )
              {
                if ( wcsnicmp( pTokenTest->pData, L"</tm>", 5 ) == 0 )
                {
                  fFound = TRUE;
                }
                else
                {
                  pTokenTest++;
                } /* endif */
              }
              else
              {
                pTokenTest++;
              } /* endif */
            } /*endwhile */

            // handle trademark
            if ( fFound )
            {
              int iEqualTokens = 0;

              // add all "equal" tokens as "change from"
              PFUZZYTOK pTokenProcess = pToken;
              PSZ_W pszChangeString = pData->szBuffer;
              while ( pTokenProcess != pTokenTest )
              {
                if ( pTokenProcess->sType == MARK_EQUAL )
                {
                  int l = pTokenProcess->usStop - pTokenProcess->usStart + 1;
                  wcsncpy( pszChangeString, pTokenProcess->pData, l );
                  iEqualTokens++;
                  pszChangeString += l;
                } /* endif */
                pTokenProcess++;
              } /*endwhile */
              *pszChangeString = 0;

              if ( iEqualTokens )
              {
                TAMatchWriteSingleChange( xw, "modify", pData->szBuffer, wcslen(pData->szBuffer) );

                // add all tokens as "change to"
                while ( pToken <= pTokenTest )
                {
                  iLen += pToken->usStop - pToken->usStart + 1;
                  pToken++;
                } /* endwhile */
                pToken--;
                TAMatchWriteSingleChange( xw, "changeto", pszStart, iLen );

                fProcessed = TRUE;
              } /* endif */
            } /* endif */
          } /* endif */

          // use standard processing if not processed yet
          if ( !fProcessed )
          {
            while ( pToken->sType == MARK_INSERTED )
            {
              iLen += pToken->usStop - pToken->usStart + 1;
              pToken++;
            } /* endwhile */
            pToken--;        // undo last increment

            TAMatchWriteSingleChange( xw, "insert", pszStart, iLen );
          } /* endif */
        }
        break;

      case MARK_MODIFIED:
        while ( pToken->sType == MARK_MODIFIED )
        {
          iLen += pToken->usStop - pToken->usStart + 1;
          pToken++;
        } /* endwhile */
        pToken--;        // undo last increment

        // search modified tokens in target tokens
        while ( pTargetToken->ulHash && (pTargetToken->sType != MARK_MODIFIED) )
        {
          pTargetToken++;
        } /*endwhile */

        if ( pTargetToken->sType == MARK_MODIFIED )
        {
          int iTargetLen = 0;
          pszTargetStart = pTargetToken->pData;
          while ( pTargetToken->sType == MARK_MODIFIED )
          {
            iTargetLen  += pTargetToken->usStop - pTargetToken->usStart + 1;
            pTargetToken++;
          } /* endwhile */

          TAMatchWriteSingleChange( xw, "modify", pszTargetStart, iTargetLen );
          TAMatchWriteSingleChange( xw, "changeto", pszStart, iLen );
        } /* endif */
        break;

      case MARK_EQUAL:
        while ( pToken->sType == MARK_EQUAL )
        {
          iLen += pToken->usStop - pToken->usStart + 1;
          pToken++;
        } /* endwhile */
        pToken--;        // undo last increment

        TAMatchWriteSingleChange( xw, "equal", pszStart, iLen );
        break;

      default :
        break;
    } /* endswitch */
    sPrev = pToken->sType;
    pToken++;
  } /* endwhile */

  xw->WriteEndElement(); // "changelist"

  return iRc;
}



int TAMatchListProcess
(
  PTAINPUT    pTAIn,
  PSZ_W       pszSegment,
  PSZ_W       pszProposal,
  MATCHTYPE   MatchType,
  USHORT      usFuzzyness
)
{
  int iRC = 0;
  PTAMATCHLISTDATA pData = NULL;

  usFuzzyness; pszProposal;

  if ( (pTAIn == NULL) || (pTAIn->pvMatchListData == NULL) )
  {
    iRC = ERROR_INVALID_DATA;
  } /* endif */

  if ( !iRC )
  {
    pData = (PTAMATCHLISTDATA)pTAIn->pvMatchListData;
    PTAINSTDATA  pInD = pTAIn->pInD;     // ta instance data
    UTF16strcpy( pData->szSegment, pszSegment );
    ULONG ulSrcOemCP = GetLangOEMCP( pInD->szDocSourceLang);


    CXmlWriter *xw = (CXmlWriter *)pData->pvXW;

    if ( (MatchType == FUZZY_MATCH) || (MatchType == REPL_MATCH) || (MatchType == FUZZYREPL_MATCH)  )
    {
      USHORT i = 0;
      PTAINSTDATA  pInD = pTAIn->pInD;     // ta instance data

      // get match from memory including inline tagging
      int iRc = 0;         // return from tm functions

      PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;

      pLookupData->SearchKey.clear();
      pLookupData->SearchKey.setDocShortName( pInD->pszCurSourceFile);
      pLookupData->SearchKey.setDocName( pTAIn->apszLongNames[pInD->usCurNumSourceFile-1] );

      pLookupData->SearchKey.setSegmentNum( pInD->ulSegNum );
      pLookupData->SearchKey.setSource( pData->szSegment );
      int iOptions = GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS;
      pLookupData->SearchKey.setMarkup( pInD->szDocFormat );
      pLookupData->SearchKey.setSourceLanguage( pInD->szDocSourceLang );
      pLookupData->SearchKey.setTargetLanguage( pInD->szDocTargetLang );
      pLookupData->SearchKey.setContext( pInD->szContext );

      OtmProposal::clearAllProposals( pLookupData->BestProposals );
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      i = 0;


      while ( !iRc && (i < pLookupData->SearchMemoryDBs.size()) )
      {
        OtmProposal::clearAllProposals( pLookupData->FoundProposals );
        iRc = pLookupData->SearchMemoryDBs[i]->searchProposal( pLookupData->SearchKey, pLookupData->FoundProposals, iOptions );
        if ( !iRc )
        {
          int iNumOfProposals = OtmProposal::getNumOfProposals( pLookupData->FoundProposals );
          for ( int iProposal = 0; iProposal < iNumOfProposals; iProposal++ ) pLookupData->FoundProposals[iProposal]->setMemoryIndex( i ); 
          pFactory->copyBestMatches( pLookupData->FoundProposals, pLookupData->BestProposals, 3 );
          i++;
        } /* endif */
      } /* endwhile */

      USHORT usNumMatchesFound = (USHORT)OtmProposal::getNumOfProposals( pLookupData->BestProposals );
      // normalize segment and match
      if ( !iRc && (usNumMatchesFound != 0) )
      {
        TAMatchNormalizeString( pData->szSegment );
        pLookupData->BestProposals[0]->getSource( pData->szSource, sizeof(pData->szSource) / sizeof(CHAR_W) );
        TAMatchNormalizeString( pData->szSource );
        pLookupData->BestProposals[0]->getTarget( pData->szTarget, sizeof(pData->szTarget) / sizeof(CHAR_W) );
        TAMatchNormalizeString( pData->szTarget );
      } /* endif */

      // find difference between segment and proposal
      if ( !iRc && usNumMatchesFound )
      {
        BOOL         fOK = TRUE;
        PFUZZYTOK    pFuzzyTgt = NULL;
        PFUZZYTOK    pFuzzyTok = NULL;       // returned token list
        PSZ          pInBuf = NULL;          // buffer for EQFBFindDiffEx function
        PSZ          pTokBuf = NULL;         // buffer for EQFBFindDiffEx function
        SHORT        sLangID = 0;

        // allocate required buffers
        fOK = UtlAlloc((PVOID *)&pInBuf, 0L, 50000L, NOMSG );
        if ( fOK ) fOK = UtlAlloc(( PVOID *)&pTokBuf, 0L, 10000, NOMSG );

        MorphGetLanguageID( pInD->szDocSourceLang , &sLangID );

        // call function to evaluate the differences
        if ( fOK )
        {
          fOK = EQFBFindDiffEx( pInD->pLoadedTable, (PBYTE)pInBuf, (PBYTE)pTokBuf, pData->szSegment,
                                pData->szSource, sLangID, (PVOID *)&pFuzzyTok,
                                (PVOID *)&pFuzzyTgt, ulSrcOemCP );
        } /* endif */

        // get proposal target text for the following compares
        pLookupData->BestProposals[0]->getTarget( pLookupData->szSegmentBuffer, sizeof(pLookupData->szSegmentBuffer) / sizeof(CHAR_W) );

        // check if all strings to be deleted or are contained in the target match
        if ( fOK )
        {
          PFUZZYTOK pToken = pFuzzyTgt;
          while ( fOK && pToken->ulHash )
          {
            CHAR_W chTemp;
            PSZ_W pszStart = pToken->pData;
            int iLen = 0;

            switch ( pToken->sType )
            {
              case MARK_DELETED:
                while ( pToken->sType == MARK_DELETED )
                {
                  iLen += pToken->usStop - pToken->usStart + 1;
                  pToken++;
                } /* endwhile */
                pToken--;        // undo last increment

                chTemp = *(pszStart + iLen);
                *(pszStart + iLen) = 0;
                fOK = wcsstr( pLookupData->szSegmentBuffer, pszStart ) != NULL;
                *(pszStart + iLen) = chTemp;
                break;
              default :
                break;
            } /* endswitch */
            pToken++;
          } /* endwhile */
        } /* endif */


        // check if all strings to be modified or are contained in the target match
        if ( fOK )
        {
          PFUZZYTOK pToken = pFuzzyTok;

          while ( fOK && pToken->ulHash )
          {
            CHAR_W chTemp;
            PSZ_W pszStart = pToken->pData;
            int iLen = 0;

            switch ( pToken->sType )
            {
              case MARK_MODIFIED:
                while ( pToken->sType == MARK_MODIFIED )
                {
                  iLen += pToken->usStop - pToken->usStart + 1;
                  pToken++;
                } /* endwhile */
                pToken--;        // undo last increment

                chTemp = *(pszStart + iLen);
                *(pszStart + iLen) = 0;
                fOK = wcsstr( pLookupData->szSegmentBuffer, pszStart ) != NULL;
                *(pszStart + iLen) = chTemp;
                break;

              default :
                break;
            } /* endswitch */
            pToken++;
          } /* endwhile */
        } /* endif */

        // write found match to output memory
        if ( fOK )
        {
          // buffer for proposal data
          char szTargetLanguage[MAX_LANG_LENGTH];
          char szAuthor[MAX_USERID];
          char szShortName[MAX_FILESPEC];
          char szMarkup[MAX_FNAME];

          // fill buffers with proposal data
          pLookupData->BestProposals[0]->getTargetLanguage( szTargetLanguage, sizeof(szTargetLanguage) );
          pLookupData->BestProposals[0]->getAuthor( szAuthor, sizeof(szAuthor) );
          pLookupData->BestProposals[0]->getDocShortName( szShortName, sizeof(szShortName) );
          pLookupData->BestProposals[0]->getMarkup( szMarkup, sizeof(szMarkup) );
          pLookupData->BestProposals[0]->getDocName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );

          swprintf( pData->szLine, L"<Segment>%10.10l\r\n", pData->lNum++ );
          TAMatchWriteToFile ( pData->hfMemory , pData->szLine );
          swprintf( pData->szLine,
                    L"%s%06ld%s%s%s%016lu%s%S%s%S%s%S%s%S%s%S%s%S%s",
                    L"<Control>\r\n",
                    pLookupData->BestProposals[0]->getSegmentNum(),
                    X15_STRW,
                    (pLookupData->BestProposals[0]->getType() == OtmProposal::eptMachine) ?  L"1" : L"0",
                    X15_STRW,
                    pLookupData->BestProposals[0]->getUpdateTime(),
                    X15_STRW,
                    pInD->szDocSourceLang,
                    X15_STRW,
                    szTargetLanguage,
                    X15_STRW,
                    szAuthor,
                    X15_STRW,
                    szMarkup,
                    X15_STRW,
                    szShortName,
                    X15_STRW,
                    pLookupData->szNameBuffer,
                    L"\r\n</Control>\r\n");
          TAMatchWriteToFile ( pData->hfMemory , pData->szLine );
          TAMatchWriteToFile ( pData->hfMemory , L"<Source>" );
          pLookupData->BestProposals[0]->getSource( pLookupData->szSegmentBuffer, sizeof(pLookupData->szSegmentBuffer) / sizeof(CHAR_W) );
          TAMatchWriteToFile( pData->hfMemory , pLookupData->szSegmentBuffer );
          TAMatchWriteToFile ( pData->hfMemory , L"</Source>\r\n" );
          TAMatchWriteToFile ( pData->hfMemory , L"<Target>" );
          pLookupData->BestProposals[0]->getTarget( pLookupData->szSegmentBuffer, sizeof(pLookupData->szSegmentBuffer) / sizeof(CHAR_W) );
          TAMatchWriteToFile( pData->hfMemory , pLookupData->szSegmentBuffer );
          TAMatchWriteToFile ( pData->hfMemory , L"</Target>\r\n" );
          TAMatchWriteToFile ( pData->hfMemory , L"</Segment>\r\n" );
        } /* endif */

        // write entry
        if ( fOK )
        {
          xw->WriteStartElement( "entry" );

          xw->WriteStartElement( "segment" );
          xw->WriteStartAttribute( "document" );
          pLookupData->SearchKey.getDocName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
          xw->WriteString( pLookupData->szNameBuffer );
          xw->WriteEndAttribute();
          xw->WriteStartAttribute( "segnumber" );
          xw->WriteInt( (int)pInD->ulSegNum );
          xw->WriteEndAttribute();
          xw->WriteStartAttribute( "markup" );
          xw->WriteString( pInD->szDocFormat );
          xw->WriteEndAttribute();
          xw->WriteString( pData->szSegment );
          xw->WriteEndElement(); // "segment"

          xw->WriteStartElement( "match" );
          xw->WriteStartAttribute( "fuzziness" );
          xw->WriteInt( pLookupData->BestProposals[0]->getFuzziness() );
          xw->WriteEndAttribute(); // "fuzziness"
          xw->WriteStartAttribute( "document" );
          pLookupData->BestProposals[0]->getDocName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
          xw->WriteString( pLookupData->szNameBuffer );
          xw->WriteEndAttribute();
          xw->WriteStartAttribute( "segnumber" );
          xw->WriteInt( pLookupData->BestProposals[0]->getSegmentNum() );
          xw->WriteEndAttribute();
          xw->WriteStartAttribute( "markup" );
          pLookupData->BestProposals[0]->getMarkup( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
          xw->WriteString( pLookupData->szNameBuffer );
          xw->WriteEndAttribute();
          xw->WriteStartAttribute( "memory" );
          pLookupData->SearchMemoryDBs[pLookupData->BestProposals[0]->getMemoryIndex()]->getName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
          xw->WriteString( pLookupData->szNameBuffer );
          xw->WriteEndAttribute();

          xw->WriteStartElement( "source" );
          xw->WriteString( pData->szSource );
          xw->WriteEndElement(); // "source"

          xw->WriteStartElement( "target" );
          xw->WriteString( pData->szTarget );
          xw->WriteEndElement(); // "target"

          xw->WriteEndElement(); // "match"

          TAMatchWriteDiff( pData, xw, pFuzzyTgt, pFuzzyTok, pData->szSource );


          xw->WriteEndElement(); // "entry"
        } /* endif */

        // free allocated buffers and lists
        if ( pFuzzyTgt ) UtlAlloc( (PVOID *)&pFuzzyTgt, 0L, 0L, NOMSG );
        if ( pFuzzyTok ) UtlAlloc( (PVOID *)&pFuzzyTok, 0L, 0L, NOMSG );
        if ( pInBuf )    UtlAlloc( (PVOID *)&pInBuf, 0L, 0L, NOMSG );
        if ( pTokBuf )   UtlAlloc( (PVOID *)&pTokBuf, 0L, 0L, NOMSG );


      } /* endif */

    } /* endif */
  } /* endif */




  return( iRC );
}
