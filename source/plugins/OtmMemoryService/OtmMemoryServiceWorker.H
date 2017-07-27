/**
* \file OtmMemoryService.H
*
* \brief Header file for OtmMemoryService class
*
*	Copyright Notice:
*
*	Copyright (C) 2016, QSoft GmbH. All rights reserved
*	
**/
#ifndef _OtmMemoryServiceWorker_H_
#define _OtmMemoryServiceWorker_H_

#include <locale>
#include <codecvt>
#include <string>
#include <vector>
#include "OTMFUNC.H"

/*! \brief constant defining the maximum number of opened memories
*/
#define OTMMEMSERVICE_MAX_NUMBER_OF_OPEN_MEMORIES 40

/*! \brief constant defining the timeout for memories (in number of seconds)
  When a memory has not been used for the given time it is automatically closed
*/
#define OTMMEMSERVICE_MEMORY_TIMEOUT 7200

class __declspec(dllexport) OtmMemoryServiceWorker
/*! \brief   This class provides translation memory related services

*/

{
public:


/*! \brief Error return codes
  */
#define ERROR_INTERNALFUNCTION_FAILED  14001
#define ERROR_INPUT_PARMS_INVALID      14002
#define ERROR_TOO_MANY_OPEN_MEMORIES   14003
#define ERROR_MEMORY_NOT_FOUND         14004

  /*! \brief OtmMemoryServiceWorker constructor
  */
  OtmMemoryServiceWorker();


/*! \brief This static method returns a pointer to the OtmMemoryService object.
	  The first call of the method creates the OtmMemoryService instance.
  */
	static OtmMemoryServiceWorker* getInstance();

	
  /*! \brief Import a memory from a TMX file
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int import
  (
    std::string  strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  /*! \brief Search for matching proposals 
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int search
  (
    std::string  strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  /*! \brief Do a concordance search for the given search string
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int concordanceSearch
  (
    std::string  strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  /*! \brief Create a new memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int createMemory
  (
    std::string strInputParms,
    std::string &strOutputParms
  );

  /*! \brief Close all open memories
  \returns http return code0 if successful or an error code in case of failures
  */
  int closeAll
  (
  );

  /*! \brief Set the log file handle
  \param hfLog log file handle or NULL to stop logging
  */
  void setLogFile
  (
    FILE *hfLog
  );

  /*! \brief List all available TMs
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int list
  (
    std::string &strOutputParms
  );

  /*! \brief Update an entry of the memory
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int updateEntry
  (
    std::string strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  /*! \brief delete a memory
  \param strMemory name of memory
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int deleteMem
  (
    std::string strMemory,
    std::string &strOutputParms
  );

  /*! \brief get the data of a memory in binary form or TMX form
  \param strMemory name of memory
  \param strType type of the requested memory data
  \param vMemData vector<Byte> receiving the memory data
  \returns http return code
  */
  int getMem
  (
    std::string strMemory,
    std::string strType,
    restbed::Bytes &vMemData
  );

  /*! \brief get the status of a memory
  \param strMemory name of memory
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int getStatus
  ( 
    std::string strMemory, 
    std::string &strOutputParms 
  );

  /* change the status of a memory in our memory list at the end of an import
  \param pszMemory name of the memory
  \param
  */
  void importDone( char *pszMemory, int iRC, char *pszError );


private:
  
  /*! \brief Verify OpenTM2 API session
    \returns 0 if successful or an error code in case of failures
  */
  int verifyAPISession
  (
  );

  /*! \brief build return JSON string in case of errors
    \param iRC error return code
    \param pszErrorMsg error message text
    \param strError JSON string receiving the error information
    \returns 0 if successful or an error code in case of failures
  */
  int buildErrorReturn
  (
    int iRC,
    wchar_t *pszErrorMsg,
    std::wstring &strErrorReturn
  );

  int buildErrorReturn
  (
    int iRC,
    wchar_t *pszErrorMsg,
    std::string &strErrorReturn
  );

  int buildErrorReturn
  (
    int iRC,
    char *pszErrorMsg,
    std::string &strErrorReturn
  );


  /*! \brief get the handle for a memory, if the memory is not opened yet it will be openend
    \param pszMemory name of the memory
    \param plHandle buffer for the memory handle
    \param pszError buffer for an error message text in case of failures
    \param iErrorBufSize size of the error text buffer in number of characters
    \param piReturnCode pointer to a buffer for the OpenTM2 error code
    \returns OK if successful or an HTTP error code in case of failures
  */
  int getMemoryHandle( char *pszMemory, PLONG plHandle, wchar_t *pszError, int iErrorBufSize, int *piErrorCode );

  /*! \brief find a memory in our list of active memories
    \param pszMemory name of the memory
    \returns index in the memory table if successful, -1 if memory is not contained in the list
  */
  int findMemoryInList( const char *pszMemory );

  /*! \brief find a free slot in our list of active memories, add a new entry if none found
    \returns index of the next free slot in the memory table or -1 if there is no free slot and the max number of entries has been reached
  */
  int getFreeSlot();

  /*! \brief close any memories which haven't been used for a long time
    \returns 0
  */
  int cleanupMemoryList();

  /*! \brief convert a long time value into the UTC date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converted date time
    \returns 0 
  */
  int convertTimeToUTC( LONG lTime, char *pszDateTime );

  /*! \brief convert a UTC time value to a OpenTM2 long time value 
      \param pszDateTime buffer containing the UTC date and time
      \param plTime pointer to a long buffer receiving the converted time 
    \returns 0 
  */
  int convertUTCTimeToLong( char *pszDateTime, PLONG plTime );

  /*! \brief extract a numeric value from a string
      \param pszString string containing the numeric value
      \param iLen number of digits to be processed
      \param piResult pointer to a int value receiving the extracted value
    \returns TRUE if successful and FALSE in case of errors
  */
  BOOL getValue( PSZ pszString, int iLen, int *piResult );

  /*! \brief convert a long time value to a date time string in the form YYYY-MM-DD HH:MM:SS
  \param lTime long time value
  \param pszDateTime buffer receiving the convererd date and time
  \returns 0
  */
  int convertTimeToDateTimeString( LONG lTime, char *pszDateTime );

  /*! \brief convert a date and time in the form YYYY-MM-DD HH:MM:SS to a OpenTM2 long time value
  \param pszDateTime buffer containing the TMX date and time
  \param plTime pointer to a long buffer receiving the converted time
  \returns 0
  */
  int convertDateTimeStringToLong( char *pszDateTime, PLONG plTime );
  
  /*! \brief close a memory and remove it from the open list
      \param iIndex index of memory in the open list
      \returns 0 
  */
  int removeFromMemoryList( int iIndex );

  /*! \brief convert a UTF8 std::string to a UTF16 std::wstring
  \param strUTF8String string in UTF8 encoding
  \returns string converted to UTF16
  */
  std::wstring convertToUTF16(const std::string& strUTF8String);

  /*! \brief convert a UTF8 std::string to a UTF16 std::wstring
  \param strUTF8String string in UTF8 encoding
  \returns string converted to UTF16
  */
  std::string convertToUTF8( const std::wstring& strUTF16String );

  /*! \brief convert a UTF8 std::string to a ASCII std::string (on spot conversion)
  \param strText on input string in UTF8 encoding, on output string in ASCII encoding
  */
  void convertUTF8ToASCII( std::string& strText );

  /*! \brief convert a ASCII std::string to UTF8 std::string (on spot conversion)
  \param strText on input string in ASCII encoding, on output string in UTF8 encoding
  */
  void convertASCIIToUTF8( std::string& strText );

  /*! \brief build a unique name for of a temporary file 
  \param pszTempFile buffer reiceiving the temporary file name
  \returns 0 is sucessfull or a return code
  */
  int buildTempFileName( char *pszTempFile );


  /* write proposals to a JSON string
  \param strJSON JSON stirng receiving the proposal data
  \param pProposals pointer to a MEMPROPOSAL array containing the proposals
  \param iNumOfProposals number of proposals to write to JSON string
  \param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
  \returns 0 is successful
  */
  int addProposalsToJSONString
  (
    std::wstring &strJSON,
    PMEMPROPOSAL pProposals,
    int iNumOfProposals,
    void *pvData
  );

  /* write a single proposal to a JSON string
  \param strJSON JSON stirng receiving the proposal data
  \param pProp pointer to a MEMPROPOSAL containing the proposal
  \param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
  \returns 0 is successful
  */
  int addProposalToJSONString
  (
    std::wstring &strJSON,
    PMEMPROPOSAL pProp,
    void *pvData
  );

  /* convert string proposal type to proposal type enum value
  \param pszType proposal type as string
  \returns proposal type as enum value
  */
  MemProposalType getMemProposalType( char *pszType );

  /*! \brief read a binary file and encode it using BASE64 encoding
  \param pszFile fully qualified name of file being encoded
  \param ppStringData adress of a pointer which will receive the (per malloc allocated) pointer to the area for the encoded string
  \param strError string receiving any error message text
  \returns 0 is sucessfull or a return code
  */
  int encodeFileInBase64( char *pszFile, char **ppStringData, std::string &strError );

  /*! \brief convert a BASE64 encoded string to a binary file
  \param pStringData pointer to the BASE64 data
  \param pszFile fully qualified name of file being written
  \param strError string receiving any error message text
  \returns 0 is sucessfull or a return code
  */
  int decodeBase64ToFile( const char *pStringData, const char *pszFile, std::string &strError );

  /*! \brief read a binary file into a Byte vector
  \param pszFile fully qualified name of file being read
  \param vFileData vector receiving the file data
  it is up to the caller to free this area using free()
  \param strError string receiving any error message text
  \returns 0 is sucessfull or a return code
  */
  int loadFileIntoByteVector( char *pszFile, restbed::Bytes &vFileData );

  /*! \brief States of a memory
  */
  typedef enum 
  {
    OPEN_STATUS,            // memory is available and open
    AVAILABLE_STATUS,       // memory has been imported but is not open yet
    IMPORT_RUNNING_STATUS,  // memory import is running
    IMPORT_FAILED_STATUS    // memory import failed
  } MEMORY_STATUS;

  /*! \brief Info structure for an open translation memory
  */
	typedef struct _OPENEDMEMORY
  {
    char szName[260];               // name of the memory
    time_t tLastAccessTime;         // last time memory has been used
    long lHandle;                   // memory handle     
    MEMORY_STATUS eStatus;          // status of the memory
    MEMORY_STATUS eImportStatus;    // status of the current/last memory import
    char *pszError;                 // pointer to an error message (only when eStatus is IMPORT_FAILED_STATUS)
  } OPENEDMEMORY, *POPENEDMEMORY;

/*! \brief Pointer to the list of opened memories
*/
  std::vector<OPENEDMEMORY> vMemoryList;

/*! \brief Pointer to the instance of the OtmMemoryService object (singleton).
*/
	static OtmMemoryServiceWorker* instance;

/*! \brief OpenTM2 API session handle
*/
	HSESSION hSession;

/*! \brief Last return code
*/
  int iLastRC;
  
/*! \brief Buffer for last error message
*/
  wchar_t szLastError[4000];

  /*! \brief log file handle 
  */
  FILE *hfLog;



};


#endif