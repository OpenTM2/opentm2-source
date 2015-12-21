/*! \brief FifoQueue.H - Include file for the FifoQueue class
	Copyright (c) 1999-2012, International Business Machines Corporation and others. All rights reserved.
	Description: This file contains the class definition for the JSONize helper class
*/

#ifndef _FIFOQUEUE_H_
#define _FIFOQUEUE_H_

#include "string"

/*! \brief class for a primitive FirstIn-FirstOut queue 
 
  This class is a singleton and provides functions for the processing
  of JSON formatted strings

*/
class FifoQueue
{

public:

  /*! \brief return codes resturned by FifoClass
  */
  static const int ERROR_NODATAPATH                 = 4001;
  static const int ERROR_FILEWRITEERROR             = 4002;
  static const int ERROR_FILEREADERROR              = 4003;
  static const int ERROR_INSUFFICIENTMEMORY         = 4004;
  static const int ERROR_INVALIDMODE                = 4005;
  static const int ERROR_QUEUEISFULL                = 4006;
  static const int ERROR_QUEUEISEMPTY               = 4007;
  static const int ERROR_BUFFERTOOSMALL             = 4008;

  /*! \brief queue open modes
  */
  typedef enum _OPENMODE { OM_READ, OM_WRITE, OM_UNDEFINED } OPENMODE;

  /*! \brief Constructor
  */
  FifoQueue::FifoQueue();


  /*! \brief destructor
  */
  FifoQueue::~FifoQueue();

  /*! \brief Sets the path to be used for queue data

    This sets the path there any queue data is to be stored
    when no path is set, the OpenTM2\QueueData directory in the user's
    APPDATA directory is being used
    \param pszQueuePath Name of the queue
  	\returns 0 or error code in case of errors
  */
  int setDataPath
  (
    char *pszDataPath
  );

  /*! \brief Opens a fifo queue

    This opens a fifo queue for reading or writing. When
    the queue does not exist it is created

    \param pszQueueName Name of the queue
    \param OpenMode mode for access to queue: OM_READ or OM_WRITE
  	\returns 0 or error code in case of errors
  */
  int open
  (
    char *pszQueueName,
    OPENMODE OpenMode
  );

  /*! \brief Close a fifo queue

    This method closed a previously opened fifo queue 

    \param pszQueueName Name of the queue

  	\returns 0 or error code in case of errors
  */
  int close
  (
  );

  /*! \brief Writes a data record to the fifo queue

    This method writes a data record to the queue

    \param pvRecord pointer to the data record
    \param iRecordSize size of data record in number of bytes

  	\returns 0 or error code in case of errors
  */
  int writeRecord
  (
    void *pvRecord,
    int  iRecSize
  );

  /*! \brief Reads the next record from the queue

    This method reads the next data record from the queue 

    \param pvRecord pointer to the the buffer for the data record
    \param iBufSize size of buffer in number of bytes

  	\returns 0 or error code in case of errors
  */
  int readRecord
  (
    void *pvRecord,
    int  iBufSize
  );

  /*! \brief Gets the size of the next record in the queue

    This method returns the size of the the next data record in the queue 

    \param pvRecord pointer to the the buffer for the data record
    \param iBufSize size of buffer in number of bytes

    \returns size if record or -1 in case of errors
*/
  int getRecordSize
  (
  );

  /*! \brief Checks if the queue is empty

  	\returns true if the queue is empty otherwise false
  */
  bool isEmpty();

  /*! \brief Checks if the queue is open

  	\returns true if the queue has been opened
  */
  bool isOpen();

  /*! \brief Get the error message for the last error occured

      \param strError reference to a string receiving the error mesage text
  	  \returns last error code
  */
  int getLastError
  (
    std::string &strError
  );


private:
  // private queue data area
  void *pvData; 

  /*! \brief Get queue head
  	\returns ID of queue head
  */
  unsigned long getHead();

  /*! \brief Get queue tail
  	\returns ID of queue tail
  */
  unsigned long getTail();

  /*! \brief Write tail record
    \param ulID ID to store in tail record
    \returns 0 or error code
  */

  /*! \brief Read a record
    \param pszRecordName pointer to name of record
    \param pvRecordData pointer to buffer for record data
    \param iRecordSize size of buffer
    \returns 0 or error code
  */
  int readRecord( char *pszRecordName, void *pvRecordBuffer, int iBufferSize, int *piRecordSize = NULL );

  int writeTail( unsigned long ulID );

  /*! \brief Write head record
    \param ulID ID to store in head record
    \returns 0 or error code
  */
  int writeHead( unsigned long ulID );

  /*! \brief Write a record
    \param pszRecordName pointer to name of record
    \param pvRecordData pointer to record data
    \param iRecordSize size of record data
    \returns 0 or error code
  */
  int writeRecord( char *pszRecordName, void *pvRecordData, int iRecordSize );

  /*! \brief Delete a record
    \param pszRecordName pointer to name of record
    \returns 0 or error code
  */
  int deleteRecord( char *pszRecordName );


  /*! \brief Get path name for the queue
    \returns 0 when successful or error code
  */
  int FifoQueue::getQueuePath();

};

#endif // ifndef _FIFOQUEUE_H_
