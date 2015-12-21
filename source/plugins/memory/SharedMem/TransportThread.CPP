/*! \file
	Description: EQF Shared Translation Memory Component Transport Thread

	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

// use only if we are dealing with MT
#include "string"
#include "vector"
#include "core\PluginManager\PluginManager.h"
#include "EqfSharedMemoryPlugin.h"
#include "EqfSharedMemory.h"
#include "FifoQueue.h"
#include "MemoryWebServiceClient.h"
#include "TransportThread.h"
#include <process.h>              /* _beginthread, _endthread */
#include <time.h>
#include "core\utilities\LogWriter.h"

#define THREADWAITTIME  50

// encryption password for properties data
#define PROPERTIESPASSWORD "pasdf09n456)dv!lxseq3=.Ö@"

typedef enum THREADSTATE
{
  ThreadState_NotStarted,
  ThreadState_Stopped,
  ThreadState_Ready,
  ThreadState_Busy,
  ThreadState_Done
} THREADSTATE;

typedef enum THREADTASK
{
  ThreadTask_None,
  ThreadTask_ListPropFiles,
  ThreadTask_AddNewMemories,
  ThreadTask_RemoveDeletedMemories,
  ThreadTask_ProcessMemory
} THREADTASK;

// structure containing information on a shared memory
typedef struct _SHAREDMEMDATA
{
  std::string strPropFileName;                       // property file name
  BOOL        fPropertiesLoaded;                     // TRUE = properties have been loaded and queues have been opened
  BOOL        fMarkedForDelete;                      // TRUE = entry has been marked for deletion
  BOOL        fPropertiesLoadFailed;                 // TREU = load of properties failed 
  EqfSharedMemoryPlugin::SHAREDMEMPROP Properties;   // copy of shared memory properties
  FifoQueue   inQueue;                               // input queue = output queue of the property file
  FifoQueue   outQueue;                              // output queue = input queue of the property file
  std::string strLastUpdateID;                       // update ID received on last download 
} SHAREDMEMDATA, *PSHAREDMEMDATA;

typedef struct _TRANSPORTTHREADDATA
{
  std::vector<std::string> ExistingPropFiles;    // list of existing shared memory property files 
  std::string strPropertyPath;                   // path to property directory
  std::vector<PSHAREDMEMDATA> MemoryList;        // list of shared memories being processed
  int iCurEntry;                                 // currently active entry in MemList
  THREADTASK  Task;                              // current task to be processed by thread
  THREADSTATE State;                             // current state of thread
  BOOL        fKill;                             // thread kill flag
  BOOL        fOK;                               // OK flag of performed task
  MemoryWebServiceClient *pWebClient;            // pointer to web service client object
  int iUploaded;                                 // number of uploaded proposals
  int iDownloaded;                               // number of downloaded proposals 
  LogWriter   *pLog; 

} TRANSPORTTHREADDATA, *PTRANSPORTTHREADDATA;


static PTRANSPORTTHREADDATA pThreadData = NULL;

// make update counter file name using properties file name
void MakeUpdateCounterFileName( std::string &strPropPath, std::string &strPropFile, std::string &strUpdateCounterFileName )
{
  strUpdateCounterFileName = strPropPath + "\\" + strPropFile;
  strUpdateCounterFileName.erase( strUpdateCounterFileName.length() - 4  );
  strUpdateCounterFileName.append( ".UDC");
}

// function borrowed from EqfSharedMemoryPlugin
/*! \brief Simple data decrypter/encrypter
  \param pbData pointer to data area being encrypter/decrypted
  \param iSize number of bytes in data area
  \param pszPassword password to be used for decryption/encryption
  \param fEncrypt true = encrypt, false = decrypt
	\returns 0
*/
int Encrypt( PBYTE pbData, int iSize, PSZ pszPassword, BOOL fEncrypt )
{
  if ( fEncrypt )
  {
    BYTE bLastValue = 0;
    int iLen = strlen(pszPassword);
    int iPW = 0;
    for ( int i = 0; i < iSize; i++ )
    {
      BYTE bValue = (BYTE)(pbData[i] + bLastValue);
      bValue = (BYTE)(bValue + (pszPassword[iPW] * (iPW+1) * 3));
      bLastValue = bValue;
      pbData[i] = bValue;
      iPW++;
      if ( iPW == iLen ) iPW = 0;
    } /* endfor */
  }
  else
  {
    BYTE bLastValue = 0;
    int iLen = strlen(pszPassword);
    int iPW = 0;
    for ( int i = 0; i < iSize; i++ )
    {
      BYTE bValue = pbData[i];
      bValue = (BYTE)(bValue - (pszPassword[iPW] * (iPW+1) * 3));
      bValue = (BYTE)(bValue - bLastValue);
      bLastValue = pbData[i];
      pbData[i] = bValue;
      iPW++;
      if ( iPW == iLen ) iPW = 0;
    } /*endfor */
  } /* endif */
  return( 0 );
}

// function 'borrowed' from EqfSharedMemoryPlugin
int LoadPropertyFile( const char *pszPropFile, EqfSharedMemoryPlugin::PSHAREDMEMPROP pProps )
{
  int iRC = 0;
  char szPath[512];
  unsigned short usSize = 0;

  memset( pProps, 0, sizeof(EqfSharedMemoryPlugin::SHAREDMEMPROP) );

  FILE *hfProp = fopen( pszPropFile, "rb" );
  if ( hfProp == NULL ) return( GetLastError() );

  if ( fread( pProps, sizeof(EqfSharedMemoryPlugin::SHAREDMEMPROP), 1, hfProp ) != 1 )
  {
    int iRC = GetLastError();  
    fclose( hfProp );
    return( iRC );
  } /* endif */     
  fclose( hfProp );

  // decrypt the data
  Encrypt( (PBYTE)pProps, sizeof(EqfSharedMemoryPlugin::SHAREDMEMPROP), PROPERTIESPASSWORD, FALSE );

  return( 0 );
}


// load current update counter value
void LoadUpdateCounter( std::string &strPropPath, std::string &strPropFileName, std::string &strUpdateCounter )
{
  strUpdateCounter = "0"; // set default

  std::string strUpdateCounterFileName;
  MakeUpdateCounterFileName( strPropPath, strPropFileName, strUpdateCounterFileName );

  FILE *hf = fopen( strUpdateCounterFileName.c_str(), "rb" );
  if ( hf == NULL ) return;

  char szBuffer[256];
  memset( szBuffer, 0, sizeof(szBuffer) );
  fread( szBuffer, sizeof(szBuffer), 1, hf );
  fclose( hf );

  strUpdateCounter = szBuffer;
  return;
}

// write new update counter value
void WriteUpdateCounter( std::string &strPropPath, std::string &strPropFileName, std::string &strUpdateCounter )
{
  std::string strUpdateCounterFileName;
  MakeUpdateCounterFileName( strPropPath, strPropFileName, strUpdateCounterFileName );

  FILE *hf = fopen( strUpdateCounterFileName.c_str(), "wb" );
  if ( hf == NULL ) return;

  fwrite( strUpdateCounter.c_str(), strUpdateCounter.length() + 1, 1, hf );
  fclose( hf );

  return;
}


// create list of existing shared memories (= check for property files)
void ListMemoryProperties( PTRANSPORTTHREADDATA pData )
{
  pData->ExistingPropFiles.clear();
  WIN32_FIND_DATA FindData;
  std::string strSearchPattern = pData->strPropertyPath + "\\*.SHP";
  HDIR hDir = FindFirstFile( strSearchPattern.c_str(), &FindData );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    BOOL fMoreFiles = TRUE;
    do
    {
      pData->ExistingPropFiles.push_back( FindData.cFileName );
      fMoreFiles = FindNextFile( hDir, &FindData );
    } while ( fMoreFiles );
    FindClose( hDir );
  } /* endif */
}

// add memories not contained in our list of memory property files
void AddNewMemories( PTRANSPORTTHREADDATA pData )
{
  std::vector<std::string>::iterator itProfileName;
  for ( itProfileName = pData->ExistingPropFiles.begin() ; itProfileName < pData->ExistingPropFiles.end(); itProfileName++ )
  {
    // search property file name in our list of processed memories
    std::vector<PSHAREDMEMDATA>::iterator itpMemoryData;
    BOOL fFound = FALSE;
    for ( itpMemoryData = pData->MemoryList.begin() ; itpMemoryData < pData->MemoryList.end(); itpMemoryData++ )
    {
      if ( (*itpMemoryData)->strPropFileName.compare( *itProfileName ) == 0 )
      {
        fFound = TRUE;
        break;
      } /* end */         
    } /* endfor */
    if ( !fFound )
    {
      PSHAREDMEMDATA pNewEntry = new(SHAREDMEMDATA);
      pNewEntry->fMarkedForDelete = FALSE;
      pNewEntry->strLastUpdateID = "0";
      pNewEntry->fPropertiesLoaded = FALSE;
      pNewEntry->fPropertiesLoadFailed = FALSE;
      pNewEntry->fMarkedForDelete = FALSE;
      pNewEntry->strPropFileName = *itProfileName;
      memset( &(pNewEntry->Properties), 0, sizeof(pNewEntry->Properties) );

      pData->MemoryList.push_back( pNewEntry );

      pThreadData->pLog->writef( "Added new shared memory %s to list of replicated memories", pNewEntry->strPropFileName.c_str() );

    } /* end */       
  } /* endfor */
}

// remove memories which do not have a property file anymore
void RemoveDeletedMemories( PTRANSPORTTHREADDATA pData )
{
  std::vector<PSHAREDMEMDATA>::iterator itpMemoryData;

  // first run: mark entries without property file for deletion
  for ( itpMemoryData = pData->MemoryList.begin() ; itpMemoryData < pData->MemoryList.end(); itpMemoryData++ )
  {
    BOOL fFound = FALSE;
    std::vector<std::string>::iterator itProfileName;
    for ( itProfileName = pData->ExistingPropFiles.begin() ; itProfileName < pData->ExistingPropFiles.end(); itProfileName++ )
    {
      if ( (*itpMemoryData)->strPropFileName.compare( *itProfileName ) == 0 )
      {
        fFound = TRUE;
        break;
      } /* end */         
    } /* endfor */
    if ( !fFound )
    {
      (*itpMemoryData)->fMarkedForDelete = TRUE;
    } /* end */       
  } /* endfor */

  // second run: remove marked entries
  std::size_t i = 0;
  while( i < pData->MemoryList.size() )
  {
    if ( pData->MemoryList[i]->fMarkedForDelete )
    {
      PSHAREDMEMDATA pEntry = pData->MemoryList[i];
      if ( pEntry->inQueue.isOpen() ) pEntry->inQueue.close();
      if ( pEntry->outQueue.isOpen() ) pEntry->outQueue.close();
      pData->MemoryList.erase( pData->MemoryList.begin() + i );
      pThreadData->pLog->writef( "Removed deleted shared memory %s from list of replicated memories", pEntry->strPropFileName.c_str() );
      delete( pEntry );
    }
    else
    {
      i++;
    } /* endif */       
  } /* endwhile */
}

// send/receive entries for current memory
void ProcessCurrentMemory( PTRANSPORTTHREADDATA pData )
{
  if ( pData->iCurEntry >= pData->MemoryList.size() ) return;

  PSHAREDMEMDATA pEntry = pData->MemoryList[pData->iCurEntry];

  // load properties and open queues if not done yet
  if ( !pData->fKill && !pEntry->fPropertiesLoaded && !pEntry->fPropertiesLoadFailed )
  {
    std::string strPropFile = pData->strPropertyPath + "\\" + pEntry->strPropFileName;
    int iRC = LoadPropertyFile( strPropFile.c_str(), &(pEntry->Properties) );
    if ( iRC == 0 )
    {
      pEntry->fPropertiesLoaded = TRUE;
      pEntry->inQueue.open( pEntry->Properties.szOutQueueName, FifoQueue::OM_READ );
      pEntry->outQueue.open( pEntry->Properties.szInQueueName, FifoQueue::OM_WRITE );
    } 
    else
    {
      pEntry->fPropertiesLoadFailed = TRUE;
    } /* end */       
  } /* end */     


  // upload entries from in queue
  if ( !pData->fKill && pEntry->fPropertiesLoaded )
  {
    int iRC = 0;
    int iCounter = 5;
    pData->pWebClient->setEndpointUrl( pEntry->Properties.szWebServiceURL );
    while ( !pData->fKill &&  !pEntry->inQueue.isEmpty() && (iRC == 0) && (iCounter != 0) )
    {
      int iRecSize = pEntry->inQueue.getRecordSize();
      if ( iRecSize != 0 )
      {
        char *pszRecord = new( char[iRecSize+1] );
        iRC = pEntry->inQueue.readRecord( (void *)pszRecord, iRecSize );
        pszRecord[iRecSize] = 0;
        if ( iRC == 0 )
        {
          std::string strTMXProposal = pszRecord;

          // to delete all line breaks in the proposal
          // otherwise OpenTMS will report error
          // just now use '#', in future we should determin which character matbe more reasonable
          std::size_t idx = strTMXProposal.find('\n');
          while(idx != std::string::npos && (idx+2)<strTMXProposal.length())
          {
            strTMXProposal.replace(idx,1,1,'#');
            idx = strTMXProposal.find('\n');
          }
              

          iRC = pData->pWebClient->uploadProposal( pEntry->Properties.szName, pEntry->Properties.szUserID, pEntry->Properties.szPassword,strTMXProposal );

          pThreadData->pLog->writef( "Sending proposal %s to web service %s...", strTMXProposal.c_str(), pEntry->Properties.szWebServiceURL );
          if ( iRC == 0 )
          {
            pData->iUploaded++;
          }
          else
          {
            std::string strError;
            pData->pWebClient->getLastError( strError );
            pThreadData->pLog->writef( "... send failed with rc=%ld, message is %s", iRC, strError.c_str() );
          } /* endif */
        } /* endif */  
        // release the resource allocate by new
        delete []pszRecord;
        		
      } /* endif */         
      iCounter--;
    } /* endwhile */       
  } /* end */     

  // download entries and write to out queue
  if ( !pData->fKill && pEntry->fPropertiesLoaded )
  {
    int iRC = 0;
    int iCounter = 5;
    pData->pWebClient->setEndpointUrl( pEntry->Properties.szWebServiceURL );
    while ( !pData->fKill && (iRC == 0) && (iCounter != 0) )
    {
      std::string strTMXProposal = "";
      std::string strUpdateCounter;
      LoadUpdateCounter( pData->strPropertyPath, pEntry->strPropFileName, strUpdateCounter );

      // keep the old update conuter when download proposal
      std::string strNewUpdateCounter = strUpdateCounter;
      iRC = pData->pWebClient->downloadProposal( pEntry->Properties.szName, pEntry->Properties.szUserID, pEntry->Properties.szPassword, strTMXProposal, strNewUpdateCounter );
      if ( iRC == 0 )
      {
        // only change the update counter when really changed
        //WriteUpdateCounter( pData->strPropertyPath, pEntry->strPropFileName, strUpdateCounter );

        // check if TMX proposals contains at least one translation unit
        if ( strTMXProposal.find( "<tu " ) != std::string::npos )
        {
          pThreadData->pLog->writef( "... Received proposal %s,mem=%s", strTMXProposal.c_str(),pEntry->Properties.szName);
          // only change the update counter when really changed
          strUpdateCounter = strNewUpdateCounter;
          WriteUpdateCounter( pData->strPropertyPath, pEntry->strPropFileName, strUpdateCounter );		 
          
          pData->iDownloaded++;

          iRC = pEntry->outQueue.writeRecord( (void *)strTMXProposal.c_str(), strTMXProposal.length() + 1 );
        }
        else
        {
          // force end of loop for this memory
          iRC = -1;
        } /* end */           

        // trigger the main process to read input queue of the memory
        // use WM_COPYDATA for process communication
        HWND hwnd = FindWindow( "OpenTM2" ,NULL);
        if(hwnd != NULL)
        {
          COPYDATASTRUCT cds;
          cds.dwData = 0;
          cds.cbData = sizeof(pEntry->Properties.szName);
          cds.lpData = pEntry->Properties.szName;
          SendMessage(hwnd,WM_COPYDATA, 0,(LPARAM)(LPVOID)&cds);
          Sleep(50);
        }

      }/* end if ( iRC == 0 ) */
      else
      {
        std::string strError;
        pData->pWebClient->getLastError( strError );
        pThreadData->pLog->writef( "... Receive of new proposals failed with rc=%ld, message is %s", iRC, strError.c_str() );
      } /* end */         
      iCounter--;
    } /* endwhile */       
  } /* end */     


}

void ProcessTransportThread( void *pvData );


BOOL StartTransportThread( const char *pszPropertyPath )
{

  pThreadData = new(TRANSPORTTHREADDATA);
  pThreadData->Task  = ThreadTask_None;
  pThreadData->State = ThreadState_NotStarted;
  pThreadData->strPropertyPath = pszPropertyPath;
  pThreadData->iUploaded = 0;
  pThreadData->iDownloaded = 0;
  
  _beginthread( ProcessTransportThread, 0, (void *)pThreadData );
  return( TRUE );
} /* end of function MTStartThread */


BOOL StopTransportThread()
{
  // wait until transport thread closes down
  if ( pThreadData->State != ThreadState_NotStarted )
  {
    int iWaits = 100;
    pThreadData->fKill = TRUE;
    while ( iWaits && (pThreadData->State != ThreadState_Stopped) )
    {
      Sleep( 500 );
      iWaits--;
    } /* endwhile */
    if ( pThreadData->State == ThreadState_Stopped )
    {
      delete( pThreadData );
      pThreadData = NULL;
    } /* endif */
  } /* endif */
  return( TRUE );
}

/*! \brief Supplies some statistics
  \param piUploaded pointer to a variable receiving the number of uploaded proposals
  \param piDownloaded pointer to a variable receiving the number of downloaded proposals
  \param piNumOfMemories pointer to a variable receiving the number of processed memories
  \returns TRUE when successful 
*/
BOOL GetTransportStatistics( int *piUploaded, int *piDownloaded, int *piNumOfMemories )
{
  *piUploaded = pThreadData->iUploaded;
  *piDownloaded = pThreadData->iDownloaded;
  *piNumOfMemories = (int)pThreadData->MemoryList.size();
  return( TRUE );
}

void ProcessTransportThread
(
  void   *pvData                       // pointer to thread data
)
{
  PTRANSPORTTHREADDATA pData = (PTRANSPORTTHREADDATA )pvData;

  pData->Task = ThreadTask_ListPropFiles;
  pData->State = ThreadState_Busy;
  pData->fKill = FALSE;
  pThreadData->pLog = new LogWriter();
  pThreadData->pLog->open( "TransportThread" );
  pThreadData->pLog->write( "Starting transport thread..." );
  pThreadData->pWebClient = new  MemoryWebServiceClient( "WebServiceInReplicator" );
  if ( pThreadData->pWebClient == NULL )
  {
    pThreadData->pLog->write( "Error: could not create web client object" );
  } /* end */     


  do
  {
    switch( pData->Task )
    {
      case ThreadTask_ListPropFiles:
        ListMemoryProperties( pData );
        pData->Task = ThreadTask_RemoveDeletedMemories;
        break;
      case ThreadTask_RemoveDeletedMemories:
        RemoveDeletedMemories( pData );
        pData->Task = ThreadTask_AddNewMemories;
        break;
      case ThreadTask_AddNewMemories:
        AddNewMemories( pData );
        pData->Task = ThreadTask_ProcessMemory;
        pData->iCurEntry = 0;
        break;
      case ThreadTask_ProcessMemory:
        ProcessCurrentMemory( pData );
        pData->iCurEntry++;
        if ( pData->iCurEntry > pData->MemoryList.size() )
        {
          pData->iCurEntry = 0;
          pData->Task = ThreadTask_ListPropFiles;
        }
        else
        {
          pData->Task = ThreadTask_ProcessMemory;
        } /* end */             
        break;
      default:
        // do nothing
        break;
    } /* endswitch */
    Sleep( THREADWAITTIME );
  } while( !pData->fKill );


  if ( pData->pLog != NULL ) 
  {
    pData->pLog->write( "Transport thread has been stopped" );
    pData->pLog->close();
    delete( pData->pLog );
  } /* endif */     
  delete( pData->pWebClient );
  pData->State = ThreadState_Stopped;

  _endthread();
} /* end of function ProcessTransportThread */

