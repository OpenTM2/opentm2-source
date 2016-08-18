/*! \brief TransportThread.H - Include file for the memory proposal transport thread
	Copyright (c) 1999-2016, International Business Machines Corporation and others. All rights reserved.
	Description: This file contains the class definition for the JSONize helper class
*/

#ifndef _TRANSPORTTHREAD_H_
#define _TRANSPORTTHREAD_H_

#include <windows.h>
#include <process.h> 
#include <string>
#include <vector>
/*! \brief Starts the memory proposal transport thread
  \param pszPropertyPath fully qualified path name of OpenTM2 property diretory
  \returns TRUE when successful 
*/
BOOL StartTransportThread( const char *pszPropertyPath );

/*! \brief Stops the memory proposal transport thread
  \returns TRUE when successful 
*/
BOOL StopTransportThread();

/*! \brief Supplies some statistics
  \param piUploaded pointer to a variable receiving the number of uploaded proposals
  \param piDownloaded pointer to a variable receiving the number of downloaded proposals
  \param piNumOfMemories pointer to a variable receiving the number of processed memories
  \returns TRUE when successful 
*/
BOOL GetTransportStatistics( int *piUploaded, int *piDownloaded, int *piNumOfMemories );

void  initCriticalSection();
void  deleteCrititalSection();
void  getSelectedMemories(std::vector<std::string>& selMemories);
void  setSelectedMemories(std::vector<std::string>& selMemories);
int   getReplicatorInterval();
void  setReplicatorInterval(int rit);

#endif // ifndef _TRANSPORTTHREAD_H_
