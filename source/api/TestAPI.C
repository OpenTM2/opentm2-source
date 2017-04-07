//+----------------------------------------------------------------------------+
//| TestAPI.C                                                        |
//|   Program to run tests of the OppenZM2 API layer for non-Scripter API calls|
//+----------------------------------------------------------------------------+
// Copyright (c) 2017, International Business Machines
// Corporation and others.  All rights reserved.
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "OtmFunc.h" 

int TestMemoryCalls( HSESSION hSession );
int TestLangCalls( HSESSION hSession );

int main
(
  int  argc,
  char *argv[],
  char *envp[]
)
{
  int iRC = 0;

  envp; 

  argc--; argv++;     //skip program name


  HSESSION hSession = NULL;
  USHORT usRC = EqfStartSession( &hSession );
  if ( hSession == NULL )
  {
    printf( "Could not start OpenTM2 API session, rc=%u\n", usRC );
    return( usRC );
  }

  TestMemoryCalls( hSession );

  TestLangCalls( hSession );

  if ( hSession != NULL ) EqfEndSession( hSession );
  
  return( iRC );
} /* end of main */


// list memory proposals
void ShowProposals( int iNumProposals, PMEMPROPOSAL pProposal )
{
  for( int i = 0; i < iNumProposals; i++, pProposal++ )
  {
    printf( "  Proposal %ld:\n", i );
    
    printf( "    Segment      = %ld\n", pProposal->lSegmentNum);
    printf( "    ID           = %s\n", pProposal->szId );
    printf( "    Source       = %S\n", pProposal->szSource );
    printf( "    Target       = %S\n", pProposal->szTarget );
    printf( "    DocName      = %s\n", pProposal->szDocName );
    printf( "    DocShortName = %s\n", pProposal->szDocShortName );
    printf( "    SourceLang   = %s\n", pProposal->szSourceLanguage );
    printf( "    TargetLang   = %s\n", pProposal->szTargetLanguage );
    char *pszType = "";
    switch( pProposal->eMatch )
    {
      case UNDEFINED_MATCHTYPE: pszType = "Undefined"; break;
	    case EXACT_MATCHTYPE: pszType = "Exact"; break;
	    case EXACTEXACT_MATCHTYPE: pszType = "ExactExact"; break;
	    case EXACTSAMEDOC_MATCHTYPE: pszType = "ExactSameDoc"; break;
	    case FUZZY_MATCHTYPE: pszType = "Fuzzy"; break;
	    case REPLACE_MATCHTYPE: pszType = "Replace"; break;
    }
    printf( "    MatchType    = %s\n", pszType );

    pszType = "";
    switch( pProposal->eType )
    {
      case UNDEFINED_PROPTYPE: pszType = "Undefined"; break;
      case MANUAL_PROPTYPE: pszType = "Manual"; break;
      case MACHINE_PROPTYPE: pszType = "Machine"; break;
      case GLOBMEMORY_PROPTYPE: pszType = "GlobalMemory"; break;
      case GLOBMEMORYSTAR_PROPTYPE: pszType = "GlobalMemoryStar"; break;
    }
    printf( "    ProposalType = %s\n", pszType );
    printf( "    Author       = %s\n", pProposal->szTargetAuthor );
    printf( "    TimeStamp    = %ld\n", pProposal->lTargetTime );
    printf( "    Fuzzyness    = %ld\n", pProposal->iFuzziness );
    printf( "    Markup       = %s\n", pProposal->szMarkup );
    printf( "    Context      = %S\n", pProposal->szContext );
    printf( "    AddInfo      = %S\n\n", pProposal->szAddInfo );
  }
}

// test the memory related API calls
int TestMemoryCalls( HSESSION hSession )
{
  USHORT usRC = 0;

  printf( "Testing memory related API calls\n" );

  printf( "Test MEMAPI01: Call EqfListMem to get size of buffer required for the list of memory names.\n" );

  LONG lBufSize = 0;
  printf( "  call: usRC = EqfListMem( hSession, NULL, &lBufSize );\n" );
  usRC = EqfListMem( hSession, NULL, &lBufSize );
  printf( "  RC = %u\n  lBufSize = %ld\n\n", usRC, lBufSize );

  if ( lBufSize != 0 )
  {
    PSZ pszBuffer = new char[lBufSize];
    printf( "Test MEMAPI02: Call EqfListMem to get the list of memory names.\n" );
    printf( "  call: usRC = EqfListMem( hSession, pszBuffer, &lBufSize );\n" );
    usRC = EqfListMem( hSession, pszBuffer, &lBufSize );
    printf( "  RC = %u\n  memory list = %s\n\n", usRC, pszBuffer );
    delete pszBuffer;
  }

  printf( "Test MEMAPI03: Call EqfExportMemInInternalFormat to export a memory as a ZIP package.\n" );

  printf( "  Call: usRC = EqfExportMemInInternalFormat( hSession, \"ShowMeMemHtml\", \"\\OTM\\ShowMeMemHtml.ZIP\", 0 );\n" );
  usRC = EqfExportMemInInternalFormat( hSession, "ShowMeMemHtml", "\\OTM\\ShowMeMemHtml.ZIP", 0 );
  printf( "  RC = %u\n\n", usRC );

  // ensure that this memory does not exist
  EqfDeleteMem( hSession, "KopieOfShowMeMemHtml" );

  printf( "Test MEMAPI04: Call EqfImportMemInInternalFormat to import a memory from a ZIP package.\n" );
  printf( "  Call: usRC = EqfImportMemInInternalFormat( hSession, \"KopieOfShowMeMemHtml\", \"\\OTM\\ShowMeMemHtml.ZIP\", 0 );\n" );
  usRC = EqfImportMemInInternalFormat( hSession, "KopieOfShowMeMemHtml", "\\OTM\\ShowMeMemHtml.ZIP", 0 );
  printf( "  RC = %u\n\n", usRC );
  DeleteFile( "\\OTM\\ShowMeMemHtml.ZIP" );

  printf( "Test MEMAPI05: Call EqfOpenMem to open a not existing memory.\n" );
  LONG lMemHandle = -1;
  printf( "  Call; usRC = EqfOpenMem( hSession, \"NotExistingMemory\", &lMemHandle, 0 );\n" );
  usRC = EqfOpenMem( hSession, "NotExistingMemory", &lMemHandle, 0 );
  printf( "  RC = %u\n\n", usRC );

  printf( "Test MEMAPI06: Call EqfOpenMem to open a existing memory.\n" );
  printf( "  Call; usRC = EqfOpenMem( hSession, \"KopieOfShowMeMemHtml\", &lMemHandle, 0 );\n" );
  usRC = EqfOpenMem( hSession, "KopieOfShowMeMemHtml", &lMemHandle, 0 );
  printf( "  RC = %u\n\n", usRC );

  {
    printf( "Test MEMAPI07: Call EqfUpdateMem to add a propoposal to a memory.\n" );
  
    // prepare a proposal
    PMEMPROPOSAL pProposal = new ( MEMPROPOSAL );
    memset( pProposal, 0, sizeof(MEMPROPOSAL) );
    pProposal->eType = MANUAL_PROPTYPE;
    pProposal->lSegmentNum = 123;
    strcpy( pProposal->szDocName, "AnotherTestDocument.txt" );
    strcpy( pProposal->szMarkup, "OTMANSI" );
    wcscpy( pProposal->szSource, L"A rose is a rose." );
    wcscpy( pProposal->szTarget, L"A rose is a rose." );
    strcpy( pProposal->szSourceLanguage, "English(U.S.)" );
    strcpy( pProposal->szTargetLanguage, "German(Reform)" );
  
    printf( "  Call; usRC = EqfUpdateMem( hSession, lMemHandle, pProposal, 0 );\n" );
    usRC = EqfUpdateMem( hSession, lMemHandle, pProposal, 0 );
    printf( "  RC = %u\n\n", usRC );

    delete pProposal;
  }

  {
    printf( "Test MEMAPI08: Call EqfQueryMem to retrieve the proposal added in test MEMAPI07.\n" );

    // prepare search key
    PMEMPROPOSAL pSearchKey = new ( MEMPROPOSAL );
    memset( pSearchKey, 0, sizeof(MEMPROPOSAL) );
    pSearchKey->eType = MANUAL_PROPTYPE;
    pSearchKey->lSegmentNum = 123;
    strcpy( pSearchKey->szDocName, "AnotherTestDocument.txt" );
    strcpy( pSearchKey->szMarkup, "OTMANSI" );
    wcscpy( pSearchKey->szSource, L"A rose is a rose." );
    strcpy( pSearchKey->szSourceLanguage, "English(U.S.)" );
    strcpy( pSearchKey->szTargetLanguage, "German(reform)" );

    // prepare array for returned proposals
    int iNumProposals = 2;
    PMEMPROPOSAL pProposals = new ( MEMPROPOSAL[iNumProposals] );
    memset( pProposals, 0, iNumProposals * sizeof(MEMPROPOSAL) );
    
    printf( "  Call: usRC = EqfQueryMem( hSession, lMemHandle, pSearchKey, &iNumProposals, pProposals, 0 );\n" );
    usRC = EqfQueryMem( hSession, lMemHandle, pSearchKey, &iNumProposals, pProposals, 0 );
    printf( "  RC = %u\n  NumProposals = %ld\n", usRC, iNumProposals );
    ShowProposals( iNumProposals, pProposals );

    delete pSearchKey;
    delete pProposals;
  }

  {
    printf( "Test MEMAPI09: Call EqfSearchMem to do a concordance search\n" );


    // prepare buffer for returned proposal
    PMEMPROPOSAL pProposal = new ( MEMPROPOSAL );
    memset( pProposal, 0, sizeof(MEMPROPOSAL) );
    char szStartPos[20];
    memset( szStartPos, 0, sizeof(szStartPos) );

    printf( "  Call: usRC = EqfQueryMem( hSession, lMemHandle, L\"rose\", szStartPos, pProposal, SEARCH_IN_SOURCE_OPT );\n" );
    usRC = EqfSearchMem( hSession, lMemHandle, L"rose", szStartPos, pProposal, 0, SEARCH_IN_SOURCE_OPT );
    printf( "  RC = %u\n", usRC );
    if ( usRC == 0 ) ShowProposals( 1, pProposal );

    delete pProposal;
  }

  printf( "Test MEMAPI10: Call EqfCloseMem to close the previously opened memory\n" );

  printf( "  Call: usRC = EqfCloseMem( hSession, lMemHandle, 0 );\n" );
  usRC = EqfCloseMem( hSession, lMemHandle, 0 );
  printf( "  RC = %u\n", usRC );

  // cleanup
  EqfDeleteMem( hSession, "KopieOfShowMeMemHtml" );

  return( 0 );
}

// test the language name related API calls
int TestLangCalls( HSESSION hSession )
{
  USHORT usRC = 0;
  char szLang[80];

  printf( "\nTesting language name related API calls\n\n" );

  printf( "Test LANGAPI01: Call EqfGetOpenTM2Lang to convert ISO language IDs to OpenTM2 language names.\n" );

  printf( "  call: usRC = EqfGetOpenTM2Lang( hSession, \"en\", szLang );\n" );
  szLang[0] = '\0';
  usRC = EqfGetOpenTM2Lang( hSession, "en", szLang );
  printf( "  RC = %u\n  OpenTM2Lang=%s\n\n", usRC, szLang );

  printf( "  call: usRC = EqfGetOpenTM2Lang( hSession, \"en-US\", szLang );\n" );
  szLang[0] = '\0';
  usRC = EqfGetOpenTM2Lang( hSession, "en-US", szLang );
  printf( "  RC = %u\n  OpenTM2Lang=%s\n\n", usRC, szLang );

  printf( "  call: usRC = EqfGetOpenTM2Lang( hSession, \"en-GB\", szLang );\n" );
  szLang[0] = '\0';
  usRC = EqfGetOpenTM2Lang( hSession, "en-GB", szLang );
  printf( "  RC = %u\n  OpenTM2Lang=%s\n\n", usRC, szLang );

  printf( "Test LANGAPI02: Call EqfGetIsoLang to convert OpenTM2 language names to ISO language IDs.\n" );

  printf( "  call: usRC = EqfGetIsoLang( hSession, \"German(reform)\", szLang );\n" );
  szLang[0] = '\0';
  usRC = EqfGetIsoLang( hSession, "German(reform)", szLang );
  printf( "  RC = %u\n  ISOLang=%s\n\n", usRC, szLang );

  printf( "  call: usRC = EqfGetIsoLang( hSession, \"English(U.S.)\", szLang );\n" );
  szLang[0] = '\0';
  usRC = EqfGetIsoLang( hSession, "English(U.S.)", szLang );
  printf( "  RC = %u\n  ISOLang=%s\n\n", usRC, szLang );

  return( 0 );
}
