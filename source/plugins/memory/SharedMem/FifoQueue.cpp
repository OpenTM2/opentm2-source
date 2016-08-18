/*! \brief FifoQueue.CPP - Simple First-In-First-Out queue implementation
	Copyright (c) 1999-2012, International Business Machines Corporation and others. All rights reserved.
	Description: This module contains functions to wrap parameters in the JSON format and to retrieve parameters from JSON strings
*/

#include "FifoQueue.h"
#include "string"
#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <stdio.h>
#include <direct.h>
#include <stdlib.h>


// prototype for helper functions
int MkMultDirHwnd( char *pszPath );


/*! \brief Private data area for fifo queues
*/
typedef struct _FIFODATA
{
  std::string strQueueName;            // name of the queue
  std::string strQueuePath;            // path name of the queue directory
  std::string strDataPath;             // data path to be used for queue data
  std::string strLastError;            // error message for last error condition
  int  iLastError;                     // error code of last error
  FifoQueue::OPENMODE mode;            // open mode 
} FIFODATA, *PFIFODATA;

// defines used by the code

// path within application data for queue data
#define APPLDATA_PATH "\\OpenTM2\\QueueData\\"


/*! \brief Constructor
*/
FifoQueue::FifoQueue()
{
  this->pvData = new(FIFODATA);
  if ( this->pvData != NULL )
  {
    PFIFODATA pData = (PFIFODATA) this->pvData;
    pData->mode = OM_UNDEFINED;
  } /* end */     

}


/*! \brief destructor
*/
FifoQueue::~FifoQueue()
{
  if ( this->pvData != NULL )
  {
    delete( this->pvData );
  } /* endif */     
}

/*! \brief Opens a fifo queue

  This opens a fifo queue for reading or writing. When
  the queue does not exist it is created

  \param pszQueueName Name of the queue
  \param OpenMode mode for access to queue: OM_READ or OM_WRITE
  \returns 0 or error code in case of errors
*/
int FifoQueue::open
(
  char *pszQueueName,
  OPENMODE OpenMode
)
{
  int iRC = 0;
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  // check open mode
  if ( (OpenMode != OM_READ) && (OpenMode != OM_WRITE) )
  {
    pData->iLastError = ERROR_INVALIDMODE;
    pData->strLastError = "Invalid queue open mode";
    return( pData->iLastError );
  }

  // get queue directory name
  pData->strQueueName = pszQueueName;
  iRC = this->getQueuePath();
  if ( iRC != 0 ) return( iRC );

  // create queue if queue (= directory) does not exist
  if ( _access( pData->strQueuePath.c_str(), 0 ) == -1 )
  {
    iRC = MkMultDirHwnd( (char *)pData->strQueuePath.c_str() );

    this->writeHead( 1 );
    this->writeTail( 1 );
  } /* end */     

  pData->mode = OpenMode;

  return( iRC );
}  

/*! \brief Close a fifo queue

  This method closed a previously opened fifo queue 

  \param pszQueueName Name of the queue

  \returns 0 or error code in case of errors
*/
int FifoQueue::close()
{
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  // nothing to do here, all our files are always closed

  pData->mode = OM_UNDEFINED;

  return( 0 );
}

/*! \brief Writes a data record to the fifo queue

  This method writes a data record to the queue

  \param pvRecord pointer to the data record
  \param iRecordSize size of data record in number of bytes

  \returns 0 or error code in case of errors
*/
int FifoQueue::writeRecord
(
  void *pvRecord,
  int  iRecSize
)
{
  int iRC = 0;
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  if ( pData->mode != OM_WRITE )
  {
    pData->iLastError = ERROR_INVALIDMODE;
    pData->strLastError = "writeRecord called for a queue opened in read mode";
    return( pData->iLastError );
  }

  unsigned long ulTail = this->getTail();
  unsigned long ulHead = this->getHead();

  if ( (ulHead + 1) == ulTail )
  {
    pData->iLastError = ERROR_QUEUEISFULL;
    pData->strLastError = "Queue is full";
    return( pData->iLastError );
  }

  char szID[20];
  sprintf( szID, "%lu", ulHead );

  iRC = this->writeRecord( szID, pvRecord, iRecSize );
  if ( iRC == 0 )
  {
    this->writeHead( ulHead + 1 );
  } /* end */  

  return( iRC );
}

/*! \brief Reads the next record from the queue

  This method reads the next data record from the queue 

  \param pvRecord pointer to the the buffer for the data record
  \param iBufSize size of buffer in number of bytes

  \returns 0 or error code in case of errors
*/
int FifoQueue::readRecord
(
  void *pvRecord,
  int  iBufSize
)
{
  int iRC = 0;
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  if ( pData->mode != OM_READ )
  {
    pData->iLastError = ERROR_INVALIDMODE;
    pData->strLastError = "readRecord called for a queue opened in write mode";
    return( pData->iLastError );
  }

  unsigned long ulTail = this->getTail();
  unsigned long ulHead = this->getHead();

  if ( ulHead == ulTail )
  {
    pData->iLastError = ERROR_QUEUEISEMPTY;
    pData->strLastError = "Queue is empty";
    return( pData->iLastError );
  }

  char szID[20];
  sprintf( szID, "%lu", ulTail );

  int iRecSize = 0;
  iRC = this->readRecord( szID, pvRecord, iBufSize, &iRecSize );
  if ( (iRC == 0) && (iRecSize > iBufSize) )
  {
    pData->iLastError = ERROR_BUFFERTOOSMALL;
    pData->strLastError = "supplied buffer is too small";
    return( pData->iLastError );
  } /* end */  

  if ( iRC == 0 )
  {
    this->deleteRecord( szID );
    this->writeTail( ulTail + 1 );
  } /* end */  

  return( iRC );
}


/*! \brief Gets the size of the next record in the queue

  This method returns the size of the the next data record in the queue 

  \param pvRecord pointer to the the buffer for the data record
  \param iBufSize size of buffer in number of bytes

  \returns size if record or -1 in case of errors
*/
int FifoQueue::getRecordSize
(
)
{
  int iRC = 0;
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( -1 );

  if ( pData->mode != OM_READ )
  {
    pData->iLastError = ERROR_INVALIDMODE;
    pData->strLastError = "readRecord called for a queue opened in write mode";
    return( -1 );
  }

  unsigned long ulTail = this->getTail();
  unsigned long ulHead = this->getHead();

  if ( ulHead == ulTail )
  {
    pData->iLastError = ERROR_QUEUEISEMPTY;
    pData->strLastError = "Queue is empty";
    return( -1 );
  }

  char szID[20];
  sprintf( szID, "%lu", ulTail );

  int iRecSize = 0;
  iRC = this->readRecord( szID, NULL, 0, &iRecSize );
  if ( iRC != 0 ) return( -1 ); 

  return( iRecSize );
}


/*! \brief Checks if the queue is empty

  \returns true if the queue is empty otherwise false
*/
bool FifoQueue::isEmpty()
{
  int iRC = 0;
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( true );

  if ( pData->mode != OM_READ ) return( true );

  unsigned long ulTail = this->getTail();
  unsigned long ulHead = this->getHead();

  return ( ulHead == ulTail );
}

/*! \brief Checks if the queue is open

  \returns true if the queue has been opened
*/
bool FifoQueue::isOpen()
{
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( false );
  return( pData->mode != OM_UNDEFINED );
}

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
int FifoQueue::getLastError
(
  std::string &strError
)
{
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  strError = pData->strLastError;
  return( pData->iLastError );
}


/*! \brief Get queue head
  \returns ID of queue head
*/
unsigned long FifoQueue::getHead()
{
  unsigned long ulHeadID = 0;
  this->readRecord( "QueueHead", (void *)&ulHeadID, sizeof(ulHeadID) );
  return( ulHeadID );
}


/*! \brief Get queue tail
  \returns ID of queue tail
*/
unsigned long FifoQueue::getTail()
{
  unsigned long ulTailID = 0;
  this->readRecord( "QueueTail", (void *)&ulTailID, sizeof(unsigned long) );
  return( ulTailID );
}

/*! \brief Write tail record
  \param ulID ID to store in tail record
  \returns 0 or error code
*/
int FifoQueue::writeTail( unsigned long ulID )
{
  return( this->writeRecord( "QueueTail", (void *)&ulID, sizeof(unsigned long) ) );
}

/*! \brief Write head record
  \param ulID ID to store in head record
  \returns 0 or error code
*/
int FifoQueue::writeHead( unsigned long ulID )
{
  return( this->writeRecord( "QueueHead", (void *)&ulID, sizeof(unsigned long) ) );
}

/*! \brief Write a record
  \param pszRecordName pointer to name of record
  \param pvRecordData pointer to record data
  \param iRecordSize size of record data
  \returns 0 or error code
*/
int FifoQueue::writeRecord( char *pszRecordName, void *pvRecordData, int iRecordSize )
{
  int iRC = 0;
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  std::string strFullName = pData->strQueuePath + "\\" + pszRecordName + ".DAT";

  FILE *hfRecord = fopen( strFullName.c_str(), "wb" );
  if ( hfRecord == NULL )
  {
    pData->iLastError = ERROR_FILEWRITEERROR;
    pData->strLastError = "Error: open of file " + strFullName + " failed";
    return( pData->iLastError );
  } /* end */     

  if ( fwrite( pvRecordData, iRecordSize, 1, hfRecord ) != 1 )
  {
    pData->iLastError = ERROR_FILEWRITEERROR;
    pData->strLastError = "Error: write to file " + strFullName + " failed";
    return( pData->iLastError );
  } /* end */     

  fclose( hfRecord );

  return( 0 );
}

/*! \brief Read a record
  \param pszRecordName pointer to name of record
  \param pvRecordData pointer to buffer for record data
  \param iRecordSize size of buffer
  \param piRecordSize pointer to buffer for record size
  \returns 0 or error code
*/
int FifoQueue::readRecord( char *pszRecordName, void *pvRecordBuffer, int iBufferSize, int *piRecordSize )
{
  int iRC = 0;
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  std::string strFullName = pData->strQueuePath + "\\" + pszRecordName + ".DAT";

  FILE *hfRecord = fopen( strFullName.c_str(), "rb" );
  if ( hfRecord == NULL )
  {
    pData->iLastError = ERROR_FILEREADERROR;
    pData->strLastError = "Error: open of file " + strFullName + " failed";
    return( pData->iLastError );
  } /* end */     

  if ( piRecordSize != NULL )
  {
    *piRecordSize = _filelength( _fileno( hfRecord ) );
  } /* endif */     

  if ( iBufferSize != 0 )
  {
    if ( fread( pvRecordBuffer, iBufferSize, 1, hfRecord ) != 1 )
    {
      pData->iLastError = ERROR_FILEREADERROR;
      pData->strLastError = "Error: read from file " + strFullName + " failed";
      return( pData->iLastError );
    } /* end */     
  } /* end */     

  fclose( hfRecord );

  return( 0 );
}


/*! \brief Get path name for the queue
  \returns 0 when successful or error code
*/
int FifoQueue::getQueuePath()
{
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  // use user application data folder when no data path has been specified
  if ( pData->strDataPath.empty() )
  {
    char path[ MAX_PATH ];
    if ( SHGetFolderPathA( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path ) == S_OK )
    {
      pData->strDataPath = path;
      pData->strDataPath.append( APPLDATA_PATH );
    }
    else
    {
      pData->iLastError = this->ERROR_NODATAPATH;
      pData->strLastError = "Failed to set up data path for queue data";
      return( pData->iLastError );
    }
  } /* endif */     

  pData->strQueuePath = pData->strDataPath;
  if ( pData->strQueuePath.at( pData->strQueuePath.length() - 1 ) != '\\' )
  {
    pData->strQueuePath.append( "\\" );
  } /* end */     
  pData->strQueuePath.append( pData->strQueueName );

  return( 0 );
}

/*! \brief Delete a record
  \param pszRecordName pointer to name of record
  \returns 0 or error code
*/
int FifoQueue::deleteRecord( char *pszRecordName )
{
  PFIFODATA pData = (PFIFODATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  std::string strFullName = pData->strQueuePath + "\\" + pszRecordName + ".DAT";

  if ( DeleteFile( strFullName.c_str() ) == 0 )
  {
    pData->iLastError = GetLastError();
    pData->strLastError = "Delete of record " + strFullName + " failed";
    return( pData->iLastError );
  } /* end */     

  return( 0 );
}

// helper function: create multiple directories
int MkMultDirHwnd( char *pszPath )
{
   PSZ   pszPathEnd;                   // ptr to current end of path
   int iRC = 0;                        // function return value
   char szPath[512];

   strcpy( szPath, pszPath );
   pszPathEnd = strchr( szPath, '\\' );
   while ( pszPathEnd && !iRC )
   {
      if ( *(pszPathEnd+1) == 0  )     // at terminating backslash ...
      {
         pszPathEnd = NULL;            // ... force end of loop
      }
      else                             // else create directory
      {
         pszPathEnd = strchr( pszPathEnd+1, '\\' );
         if ( pszPathEnd ) *pszPathEnd = 0;
          
         _mkdir( szPath );

         if ( pszPathEnd ) *pszPathEnd = '\\';
      } /* endif */
   } /* endwhile */
   return( iRC );
} /* MkMultDirHwnd */
