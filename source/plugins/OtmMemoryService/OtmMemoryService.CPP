//+----------------------------------------------------------------------------+
//| OtmMemoryService.CPP                                                       |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (c) 2016, QSoft GmbH. All rights reserved.                  |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Web Server providing memory services using OpenTM2            |
//+----------------------------------------------------------------------------+
//

#include <time.h>
#include <stdarg.h>
#include <windows.h>
#include <process.h>  
#include <memory>
#include <thread>
#include <cstdlib>
#include <restbed>
#include <sstream>
#include "core\utilities\LanguageFactory.h"
#include "OtmProposal.h"
#include "OTMMSJSONFactory.h"
#include "OtmMemoryServiceWorker.h"
#include "eqfserno.h"

using namespace std;
using namespace restbed;


const char szVersionID[] = "X-OpenTM2-Version";

static Service service;
static std::shared_ptr<restbed::Settings> pSettings = NULL;
static OtmMemoryServiceWorker *pMemService = NULL;
static char szLogFile[MAX_PATH];
static FILE *hfLog = NULL;
static HWND hwndLogButton = NULL;

void restoreBlanks( std::string &strInOut );
void WriteCrashLog( char *pszLogDir );

// number of entries in the transaction log
#define NUM_OF_TRANSACTENTRIES 50

//  transaction IDs
typedef enum _TRANSACTID 
{
  UNUSED_TRANSACTID = 0, GET_LISTMEM_TRANSACTID, POST_CREATEMEM_TRANSACTID, POST_IMPORT_TRANSACTID,
  POST_FUZZYSEARCH_TRANSACTID, POST_CONCORDANCE_TRANSACTID, POST_UPDATEENTRY_TRANSACTID, DELETE_MEMORY_TRANSACTID, GET_MEMORY_TRANSACTID,
  GET_STATUS_TRANSACTID
} TRANSACTID;

// transaction log
typedef struct _TRANSACTLOG
{
  time_t lStartTimeStamp;
  time_t lStopTimeStamp;
  TRANSACTID Id;
  char szMemory[MAX_PATH];
} TRANSACTLOG;

static TRANSACTLOG *pTransActLog = NULL;

int AddToTransActLog( TRANSACTID Id, const char *pszMemory );
int TransActDone( int iTransActIndex );

void ServiceThread
(
	void   *pvSettings                       // pointer to thread data
)
{
	pvSettings;

  // allocate array for transaction logging
  pTransActLog = new( TRANSACTLOG[NUM_OF_TRANSACTENTRIES] );
  if ( pTransActLog ) memset( pTransActLog, 0, NUM_OF_TRANSACTENTRIES * sizeof( TRANSACTLOG ) );

  // start the service
	service.start(pSettings);
}

void delete_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );
  string strTM = request->get_path_parameter( "id", "" );
  restoreBlanks( strTM );
  int iTransActIndex = AddToTransActLog( DELETE_MEMORY_TRANSACTID, strTM.c_str() );

  if ( hfLog != NULL ) fprintf( hfLog, "==== processing DELETE request, content type=\"%s\", content length=%ld, mem=\"%s\"====\n", strType.c_str(), content_length,
    strTM.c_str() );

  string strResponseBody;
  int rc = pMemService->deleteMem( strTM, strResponseBody );
  session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, STR_DRIVER_LEVEL_NUMBER } } );

  TransActDone( iTransActIndex );

  if ( hfLog != NULL ) fprintf( hfLog, "...Done, RC=%ld\nOutput:\n-----\n%s\n----\n====\n", rc, strResponseBody.c_str() );
}

void get_memory_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  std::string strAccept = request->get_header( "Accept", "" );
  string strTM = request->get_path_parameter( "id", "" );
  restoreBlanks( strTM );
  int iTransActIndex = AddToTransActLog( GET_MEMORY_TRANSACTID, strTM.c_str() );

  if ( hfLog != NULL ) fprintf( hfLog, "==== processing GET memory request, Accept=\"%s\", mem=\"%s\"====\n", strAccept.c_str(), strTM.c_str() );

  restbed::Bytes vMemData;
  int rc = pMemService->getMem( strTM, strAccept, vMemData );
  session->close( rc, vMemData, { { "Content-Length", ::to_string( vMemData.size() ) },{ "Content-Type", strAccept.c_str() },{ szVersionID, STR_DRIVER_LEVEL_NUMBER } } );

  TransActDone( iTransActIndex );

  if ( hfLog != NULL ) fprintf( hfLog, "...Done, RC=%ld\n", rc );
}

void get_method_handler(const shared_ptr< Session > session)
{
	const auto request = session->get_request();

	size_t content_length = request->get_header("Content-Length", 0);
	std::string strType = request->get_header("Content-Type", "");
  int iTransActIndex = AddToTransActLog( GET_LISTMEM_TRANSACTID, "" );

  if ( hfLog != NULL ) fprintf( hfLog, "==== processing GET request, content type=\"%s\", content length=%ld====\n", strType.c_str(), content_length );

  string strResponseBody;
  pMemService->list( strResponseBody );
  session->close( OK, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, STR_DRIVER_LEVEL_NUMBER } } );
  TransActDone( iTransActIndex );
}

void getStatus_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  string strTM = request->get_path_parameter( "id", "" );
  restoreBlanks( strTM );
  int iTransActIndex = AddToTransActLog( GET_STATUS_TRANSACTID, strTM.c_str() );

  if ( hfLog != NULL ) fprintf( hfLog, "==== processing GET Status request mem=\"%s\"====\n", strTM.c_str() );


  string strResponseBody;
  int iRC = pMemService->getStatus( strTM, strResponseBody );
  session->close( iRC, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" }, { szVersionID, STR_DRIVER_LEVEL_NUMBER } } );
  TransActDone( iTransActIndex );
}

// replace plus signs in string with blanks
void restoreBlanks( std::string &strInOut )
{
  int iLen = strInOut.length();
  for ( int i = 0; i < iLen; i++ )
  {
    if ( strInOut[i] == '+' )
    {
      strInOut[i] = ' ';
    }
  }
}

void postImport_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  if ( hfLog != NULL ) fprintf( hfLog, "==== processing POST mem/import request, content type=\"%s\", content length=%ld====\n", strType.c_str(), content_length );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );
    int iTransActIndex = AddToTransActLog( POST_IMPORT_TRANSACTID, strTM.c_str() );
    string strInData = string( body.begin(), body.end() );
    string strResponseBody;
    if ( hfLog != NULL ) fprintf( hfLog, "Memory name=\"%s\"\n", strTM.c_str() );
    if ( hfLog != NULL ) fprintf( hfLog, "Input:\n-----\n%s\n----\n", strInData.c_str() );
    int rc = pMemService->import( strTM, strInData, strResponseBody );
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, STR_DRIVER_LEVEL_NUMBER } } );
    TransActDone( iTransActIndex );
    if ( hfLog != NULL ) fprintf( hfLog, "...Done, RC=%ld\nOutput:\n-----\n%s\n----\n====\n", rc, strResponseBody.c_str() );
  } );
}

void post_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );
  if ( hfLog != NULL ) fprintf( hfLog, "==== processing POST request, content type=\"%s\", content length=%ld====\n", strType.c_str(), content_length );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    int iTransActIndex = AddToTransActLog( POST_CREATEMEM_TRANSACTID, "" );
    string strInData = string( body.begin(), body.end() );
    if ( hfLog != NULL ) fprintf( hfLog, "Input:\n-----\n%s\n----\n", strInData.c_str() );
    string strResponseBody;
    int rc = pMemService->createMemory( strInData, strResponseBody );
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, STR_DRIVER_LEVEL_NUMBER } } );
    TransActDone( iTransActIndex );
    if ( hfLog != NULL ) fprintf( hfLog, "...Done, RC=%ld\nOutput:\n-----\n%s\n----\n====\n", rc, strResponseBody.c_str() );
  } );
}


void postFuzzySearch_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  if ( hfLog != NULL ) fprintf( hfLog, "==== processing POST mem/fuzzysearch request, content type=\"%s\", content length=%ld====\n", strType.c_str(), content_length );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );
    int iTransActIndex = AddToTransActLog( POST_FUZZYSEARCH_TRANSACTID, strTM.c_str() );

    string strInData = string( body.begin(), body.end() );
    string strResponseBody;
    if ( hfLog != NULL ) fprintf( hfLog, "Memory name=\"%s\"\n", strTM.c_str() );
    if ( hfLog != NULL ) fprintf( hfLog, "Input:\n-----\n%s\n----\n", strInData.c_str() );
    int rc = pMemService->search( strTM, strInData, strResponseBody );
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, STR_DRIVER_LEVEL_NUMBER } } );
    TransActDone( iTransActIndex );
    if ( hfLog != NULL ) fprintf( hfLog, "...Done, RC=%ld\nOutput:\n-----\n%s\n----\n====\n", rc, strResponseBody.c_str() );
  } );
}

void postConcordanceSearch_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  if ( hfLog != NULL ) fprintf( hfLog, "==== processing POST mem/concordancesearch request, content type=\"%s\", content length=%ld====\n", strType.c_str(), content_length );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );
    int iTransActIndex = AddToTransActLog( POST_CONCORDANCE_TRANSACTID, strTM.c_str() );

    string strInData = string( body.begin(), body.end() );
    string strResponseBody;
    if ( hfLog != NULL ) fprintf( hfLog, "Memory name=\"%s\"\n", strTM.c_str() );
    if ( hfLog != NULL ) fprintf( hfLog, "Input:\n-----\n%s\n----\n", strInData.c_str() );
    int rc = pMemService->concordanceSearch( strTM, strInData, strResponseBody );
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, STR_DRIVER_LEVEL_NUMBER } } );
    TransActDone( iTransActIndex );
    if ( hfLog != NULL ) fprintf( hfLog, "...Done, RC=%ld\nOutput:\n-----\n%s\n----\n====\n", rc, strResponseBody.c_str() );
  } );
}

void postEntry_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  if ( hfLog != NULL ) fprintf( hfLog, "==== processing POST mem/entry request, content type=\"%s\", content length=%ld====\n", strType.c_str(), content_length );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );
    int iTransActIndex = AddToTransActLog( POST_UPDATEENTRY_TRANSACTID, strTM.c_str() );

    string strInData = string( body.begin(), body.end() );
    string strResponseBody;
    if ( hfLog != NULL ) fprintf( hfLog, "Memory name=\"%s\"\n", strTM.c_str() );
    if ( hfLog != NULL ) fprintf( hfLog, "Input:\n-----\n%s\n----\n", strInData.c_str() );
    int rc = pMemService->updateEntry( strTM, strInData, strResponseBody );
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, STR_DRIVER_LEVEL_NUMBER } } );
    TransActDone( iTransActIndex );
    if ( hfLog != NULL ) fprintf( hfLog, "...Done, RC=%ld\nOutput:\n-----\n%s\n----\n====\n", rc, strResponseBody.c_str() );
  } );
}



BOOL PrepareOtmMemoryService( char *pszService, unsigned *puiPort )
{
  pMemService = OtmMemoryServiceWorker::getInstance();
  if ( pMemService == NULL )
  {
    return FALSE;
  } /* endif */

  {
    unsigned int uiPort = 8080;
    char szServiceName[100] = "";
    unsigned int uiWorkerThreads = 10;
    unsigned int uiTimeOut = 3600;
    char szValue[100];

    // get configuration settings
    {
      char szConfFileName[MAX_PATH];

      GetModuleFileName( NULL, szConfFileName, sizeof( szConfFileName ) );
      char *pszExt = strrchr( szConfFileName, '.' );
      if ( pszExt != NULL )strcpy( pszExt, ".conf" );
      GetPrivateProfileString( "Settings", "TimeOut", "3600", szValue, sizeof( szValue ), szConfFileName );
      uiTimeOut = atoi( szValue );
      GetPrivateProfileString( "Settings", "Port", "8080", szValue, sizeof( szValue ), szConfFileName );
      uiPort = atoi( szValue );
      GetPrivateProfileString( "Settings", "Threads", "10", szValue, sizeof( szValue ), szConfFileName );
      uiWorkerThreads = atoi( szValue );
      GetPrivateProfileString( "Settings", "ServiceName", "otmmemoryservice", szServiceName, sizeof( szServiceName ), szConfFileName );

      // set caller's service name and port fields
      strcpy( pszService, szServiceName );
      *puiPort = uiPort;
    }

    // handler for resource URL w/o memory name
    auto resource = make_shared< Resource >();
    sprintf( szValue, "/%s", szServiceName );
    resource->set_path( szValue );
    resource->set_method_handler( "GET", get_method_handler );
    resource->set_method_handler( "POST", post_method_handler );

    // handler for resource URL w memory name/import
    auto memImport = make_shared< Resource >();
    sprintf( szValue, "/%s/{id: .+}/import", szServiceName );
    memImport->set_path( szValue );
    memImport->set_method_handler( "POST", postImport_method_handler );

    // handler for resource URL w memory name/fuzzysearch
    auto fuzzysearch = make_shared< Resource >();
    sprintf( szValue, "/%s/{id: .+}/fuzzysearch", szServiceName );
    fuzzysearch->set_path( szValue );
    fuzzysearch->set_method_handler( "POST", postFuzzySearch_method_handler );

    // handler for resource URL w memory name/concordancesearch
    auto concordancesearch = make_shared< Resource >();
    sprintf( szValue, "/%s/{id: .+}/concordancesearch", szServiceName );
    concordancesearch->set_path( szValue );
    concordancesearch->set_method_handler( "POST", postConcordanceSearch_method_handler );

    // handler for resource URL w memory name/entry
    auto postEntry = make_shared< Resource >();
    sprintf( szValue, "/%s/{id: .+}/entry", szServiceName );
    postEntry->set_path( szValue );
    postEntry->set_method_handler( "POST", postEntry_method_handler );

    // handler for resource URL w memory name
    auto memname = make_shared< Resource >();
    sprintf( szValue, "/%s/{id: .+}", szServiceName );
    memname->set_path( szValue );
    memname->set_method_handler( "DELETE", delete_method_handler );
    memname->set_method_handler( "GET", get_memory_method_handler );

    // handler for resource URL w memory name/status
    auto getStatus = make_shared< Resource >();
    sprintf( szValue, "/%s/{id: .+}/status", szServiceName );
    getStatus->set_path( szValue );
    getStatus->set_method_handler( "GET", getStatus_method_handler );


    pSettings = make_shared< Settings >();
    pSettings->set_port( (uint16_t)uiPort );
    pSettings->set_worker_limit( uiWorkerThreads );
    pSettings->set_default_header( "Connection", "close" );
    pSettings->set_connection_timeout( std::chrono::seconds( uiTimeOut ) );


    service.publish( resource );
    service.publish( memImport );
    service.publish( fuzzysearch );
    service.publish( concordancesearch );
    service.publish( memname );
    service.publish( postEntry );
    service.publish( getStatus );
  }

  // prepare log file
  GetModuleFileName( NULL, szLogFile, MAX_PATH );
  strcat( szLogFile, ".log" );

  return( TRUE );
}



BOOL StartOtmMemoryService()
{
  // start web service
  _beginthread( ServiceThread, 0, NULL );

  return( TRUE );
}

void StopOtmMemoryService()
{
  service.stop();

  pMemService->closeAll();

  if ( hfLog ) fclose( hfLog );

	return;
}

void SetLogFile( FILE *hfLog )
{
  if ( pMemService != NULL ) pMemService->setLogFile( hfLog );
}

// add a transaction to our transaction log
int AddToTransActLog( TRANSACTID Id, const char *pszMemory )
{
  // return when no transaction log array exists
  if ( pTransActLog == NULL ) return( -1 );

  // find a free slot in the transaction log array
  int i = 0;
  int iOldestEntry = -1;
  while ( (i < NUM_OF_TRANSACTENTRIES) && (pTransActLog[i].Id != UNUSED_TRANSACTID) ) i++;

  // if none found look for oldest completed transaction
  if ( i >= NUM_OF_TRANSACTENTRIES )
  {
    i = 0;
    time_t lOldestTime = 0;
    while ( i < NUM_OF_TRANSACTENTRIES ) 
    {
      if ( pTransActLog[i].lStopTimeStamp != 0 ) // transaction completed?
      {
        if ( ( lOldestTime == 0 ) || ( lOldestTime > pTransActLog[i].lStopTimeStamp) )
        {
          lOldestTime = pTransActLog[i].lStopTimeStamp;
          iOldestEntry = i;
        }
      }
      i++;
    }
  }

  // return if no entry in transaction log is available
  if ( ( i >= NUM_OF_TRANSACTENTRIES ) && ( iOldestEntry == -1 ) ) return( -1 );

  // fill fields of found entry
  TRANSACTLOG *pEntry = pTransActLog + i;
  memset( pEntry, 0, sizeof( TRANSACTLOG ) );
  pEntry->Id = Id;
  time( &( pEntry->lStartTimeStamp ) );
  strcpy( pEntry->szMemory, pszMemory );

  return( i );
}

// mark a transaction as completed
int TransActDone( int iTransActIndex )
{
  // return when no transaction log array exists
  if ( pTransActLog == NULL ) return( 0 );

  // return when no valid index is given
  if ( ( iTransActIndex < 0 ) || ( iTransActIndex >= NUM_OF_TRANSACTENTRIES ) ) return( 0 );

  // set transaction stop time
  pTransActLog[iTransActIndex].lStopTimeStamp = 0;
  time( &(pTransActLog[iTransActIndex].lStopTimeStamp) );

  return( 0 );
}


// convert time to a string
void MakeDateTimeString( time_t lTime, char chDateDelim, char chDateTimeDelim, char chTimeDelim, char *pszDateTime )
{
  char szBuffer[10];

  tm *pTime = localtime( &lTime );
  ltoa( pTime->tm_year + 1900, szBuffer, 10 );
  memcpy( pszDateTime, szBuffer, 4 );
  pszDateTime += 4;
  if ( chDateDelim != 0 ) *pszDateTime++ = chDateDelim;
  ltoa( pTime->tm_mon + 100, szBuffer, 10 );
  memcpy( pszDateTime, szBuffer + 1, 2 );
  pszDateTime += 2;
  if ( chDateDelim != 0 ) *pszDateTime++ = chDateDelim;
  ltoa( pTime->tm_mday + 100, szBuffer, 10 );
  memcpy( pszDateTime, szBuffer + 1, 2 );
  pszDateTime += 2;
  if ( chDateTimeDelim != 0 ) *pszDateTime++ = chDateTimeDelim;
  ltoa( pTime->tm_hour + 100, szBuffer, 10 );
  memcpy( pszDateTime, szBuffer + 1, 2 );
  pszDateTime += 2;
  if ( chTimeDelim != 0 ) *pszDateTime++ = chTimeDelim;
  ltoa( pTime->tm_min + 100, szBuffer, 10 );
  memcpy( pszDateTime, szBuffer + 1, 2 );
  pszDateTime += 2;
  if ( chTimeDelim != 0 ) *pszDateTime++ = chTimeDelim;
  ltoa( pTime->tm_sec + 100, szBuffer, 10 );
  memcpy( pszDateTime, szBuffer + 1, 2 );
  pszDateTime += 2;
  *pszDateTime = 0;
}

// write a timestamp to the log file
void WriteTimeStamp( const char *pszPrefix, time_t lTime, FILE *hfLog )
{
  char szDateTime[20];
  MakeDateTimeString( lTime, '/', ' ', ':', szDateTime );
  fwrite( pszPrefix, 1, strlen( pszPrefix ), hfLog );
  fwrite( szDateTime, 1, strlen( szDateTime ), hfLog );
}

// write a transaction log entry to a log file
void WriteTransActLogEntry( TRANSACTLOG *pEntry, FILE *hfLog )
{
  char *pszFunction = "UNKNOWN";
  switch ( pEntry->Id )
  {
    case UNUSED_TRANSACTID: return; break;
    case GET_LISTMEM_TRANSACTID: pszFunction = "GET-LISTMEM"; break;
    case POST_CREATEMEM_TRANSACTID: pszFunction = "POST-CREATEMEM"; break;
    case POST_IMPORT_TRANSACTID: pszFunction = "POST-IMPORTMEM"; break;
    case POST_FUZZYSEARCH_TRANSACTID: pszFunction = "POST-FUZZYSEARCH"; break;
    case POST_CONCORDANCE_TRANSACTID: pszFunction = "POST-CONCORDANCE"; break;
    case POST_UPDATEENTRY_TRANSACTID: pszFunction = "POST-UPDATEENTRY"; break;
    case DELETE_MEMORY_TRANSACTID: pszFunction = "DELETE-MEM"; break;
    case GET_MEMORY_TRANSACTID: pszFunction = "GET-MEMORY"; break;
    case GET_STATUS_TRANSACTID: pszFunction = "GET-STATUS"; break;
    default: break;
  }

  fprintf( hfLog, "%s(", pszFunction );
  WriteTimeStamp( "Start=", pEntry->lStartTimeStamp, hfLog );

  if ( pEntry->lStopTimeStamp != 0 )
  {
    fwrite( " ", 1, 1, hfLog );
    WriteTimeStamp( "Stop=", pEntry->lStopTimeStamp, hfLog );
  }

  if ( pEntry->szMemory[0] != 0 )
  {
    fprintf( hfLog, " Mem=%s", pEntry->szMemory );
  }

  fputs( ")\n", hfLog );

}

// compare time stamps of two transaction log entries
int TransActLogCompare( const void * pElem1, const void * pElem2 )
{
  int iIndex1 = *( (int *)pElem1 );
  int iIndex2 = *( (int *)pElem2 );
  return( pTransActLog[iIndex1].lStartTimeStamp - pTransActLog[iIndex2].lStartTimeStamp );
}

// write transaction log to a file
void WriteTransActLog( FILE *hfLog )
{
  // make sorted array for the access to the transaction log 
  int aiSortedArray[NUM_OF_TRANSACTENTRIES];
  for ( int i = 0; i < NUM_OF_TRANSACTENTRIES; i++ ) aiSortedArray[i] = i;
  qsort( (void *)aiSortedArray, (size_t)NUM_OF_TRANSACTENTRIES, (size_t)sizeof(int), TransActLogCompare );

  // list all incomplete transactions
  fputs( "Incomplete transactions:\n", hfLog );
  for ( int i = 0; i < NUM_OF_TRANSACTENTRIES; i++ )
  {
    TRANSACTLOG *pEntry = pTransActLog + aiSortedArray[i];
    if ( (pEntry->Id != UNUSED_TRANSACTID) && (pEntry->lStopTimeStamp == 0) )
    {
      WriteTransActLogEntry( pEntry, hfLog );
    }
  }

  // list all completed transactions
  fputs( "\nRecently completed transactions:\n", hfLog );
  for ( int i = 0; i < NUM_OF_TRANSACTENTRIES; i++ )
  {
    TRANSACTLOG *pEntry = pTransActLog + aiSortedArray[i];
    if ( ( pEntry->Id != UNUSED_TRANSACTID ) && ( pEntry->lStopTimeStamp != 0 ) )
    {
      WriteTransActLogEntry( pEntry, hfLog );
    }
  }
}

// write crash log
void WriteCrashLog( char *pszLogDir )
{
  static char szLogFile[MAX_PATH];
  time_t lTime = 0;
  time( &lTime );
  strcpy( szLogFile, pszLogDir );
  strcat( szLogFile, "\\OtmMemoryService-Crash-" );
  MakeDateTimeString( lTime, '\0', '-', '\0', szLogFile + strlen( szLogFile ) );
  strcat( szLogFile, ".log" );
  FILE *hfLog = fopen( szLogFile, "w" );
  if ( hfLog )
  {
    fprintf( hfLog, "OtmMemoryService terminated abnormally (unhandled exception)\n\n" );
    WriteTransActLog( hfLog );
    fclose( hfLog );
  }
}