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
#ifndef _OtmMemoryService_H_
#define _OtmMemoryService_H_

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

class __declspec(dllexport) OtmMemoryService
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

/*! \brief This static method returns a pointer to the OtmMemoryService object.
	  The first call of the method creates the OtmMemoryService instance.
  */
	static OtmMemoryService* getInstance();

  /*! \brief Import a memory using the internal memory files
    \param strInputParms input parameters in JSON format
       Sample: { "Memory":"MyTestMemory", "Files":"C:/FileArea/MyTestMemory.ZIP" } 
    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"" } 
    \returns 0 if successful or an error code in case of failures
  */
  int importMemoryFromPackage
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );

  /*! \brief Delete a memory (the memory is closed automatically before the delete)
    \param strInputParms input parameters in JSON format
       Sample: { "Memory":"MyTestMemory" } 
    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"" } 
    \returns 0 if successful or an error code in case of failures
  */
  int deleteMemory
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );


  /*! \brief Create a new memory
    \param strInputParms input parameters in JSON format
       Sample: { "Memory":"MyTestMemory", "SourceLanguage":"en-US" } 
    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"" } 
    \returns 0 if successful or an error code in case of failures
  */
  int createMemory
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );

  /*! \brief Import a memory from a TMX file
    \param strInputParms input parameters in JSON format
       Sample: { "Memory":"MyTestMemory", "TMXFile":"C:/FileArea/MyTstMemory.TMX" } 
    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"" } 
    \returns 0 if successful or an error code in case of failures
  */
  int importMemory
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );


  /*! \brief Open a memory 
    \param strInputParms input parameters in JSON format
       Sample: { "Memory":"MyTestMemory" } 
    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"" } 
    \returns 0 if successful or an error code in case of failures
  */
  int openMemory
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );


  /*! \brief Close a memory 
    \param strInputParms input parameters in JSON format
       Sample: { "Memory":"MyTestMemory" } 
    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"" } 
    \returns 0 if successful or an error code in case of failures
  */
  int closeMemory
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );

  /*! \brief Get matches from a TM
    \param strInputParms input parameters in JSON format
       Sample: { "Memory":"MyTestMemory", "SearchCriteria":{ "Source": "This is the source text", "Segment": 231, "DocumentName":"Anothertest.txt", 
                 "SourceLanguage":"en-US", "TargetLanguage":"de-de", "Markup":"EQFHTML3", "Context":null } } 
    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"", "NumOfFoundProposals": 2, "FoundProposals":                  [ { "Source": "This is the source text", "Target": "This is the translated text", "Segment": 231, "ID":"identifier", "DocumentName":"Anothertest.txt",
                     "DocumentShortName":"ANOTHERT.001", "SourceLanguage":"en-US", "TargetLanguage":"de-de", "Type":"Manual", "Match":"Exact", "Author":"A.Nonymous",
                     "DateTime":"20161013T152948Z", "Fuzziness":100, "Markup":"EQFHTML3", "Context":null, "AddInfo":null },                   { "Source": "This is the source text", "Target": "This is another translated text", "Segment": 15, "ID":"identifier", "DocumentName":"OtherDoc.txt",
                     "DocumentShortName":"OTHERD.001", "SourceLanguage":"en-US", "TargetLanguage":"de-de", "Type":"Manual", "Match":"Exact", "Author":"",
                     "DateTime":"20161004T110214Z", "Fuzziness":100, "Markup":"EQFHTML3", "Context":null, "AddInfo":null } ] }     \returns 0 if successful or an error code in case of failures
  */
  int lookupInMemory
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );

  /*! \brief Search in a TM (Concordance search)
    \param strInputParms input parameters in JSON format
       Sample: { "Memory": "TestMemory", "SearchString": "this is the search string", "Search":"Source", "SearchPosition":"" }
    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"", "NewSearchPosition":"254:43", "FoundProposal":                  { "Source": "This is the source text", "Target": "This is the translated text", "Segment": 231, "ID":"identifier", "DocumentName":"Anothertest.txt",
                   "DocumentShortName":"ANOTHERT.001", "SourceLanguage":"en-US", "TargetLanguage":"de-de", "Type":"Manual", "Match":"Exact", "Author":"A.Nonymous",
                   "DateTime":"20161013T152948Z", "Fuzziness":100, "Markup":"EQFHTML3", "Context":null, "AddInfo":null } }    \returns 0 if successful or an error code in case of failures
  */
  int searchMemory
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );

  /*! \brief Write a memory proposal to a TM 
    \param strInputParms input parameters in JSON format
       Sample: { "Memory": "TestMemory", "Proposal":                  { "Source": "This is the source text", "Target": "This is the translated text", "Segment": 231, "DocumentName":"Anothertest.txt",
                   "SourceLanguage":"en-US", "TargetLanguage":"de-de", "Type":"Manual", "Author":"A.Nonymous",
                   "DateTime":"20161013T152948Z", "Markup":"EQFHTML3", "Context":"", "AddInfo":"" } }    \param strOutParms on return filled with the output parameters in JSON format 
       Sample: { "ReturnValue":0, "ErrorMsg":"" }    \returns 0 if successful or an error code in case of failures
  */
  int updateMemory
  (
    std::wstring strInputParms,
    std::wstring &strOutputParms
  );

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

  /*! \brief get the handle for a memory, if the memory is not opened yet it will be openend
    \param pszMemory name of the memory
    \param plHandle buffer for the memory handle
    \param pszError buffer for an error message text in case of failures
    \param oErrorBufSize size of the error text buffer in number of characters
    \returns 0 if successful or an error code in case of failures
  */
  int getMemoryHandle( char *pszMemory, PLONG plHandle, wchar_t *pszError, int iErrorBufSize );

  /*! \brief find a memory in our list of active memories
    \param pszMemory name of the memory
    \returns index in the memory table if successful, -1 if memory is not contained in the list
  */
  int findMemoryInList( char *pszMemory );

  /*! \brief find a free slot in our list of active memories, add a new entry if none found
    \returns index of the next free slot in the memory table or -1 if there is no free slot and the max number of entries has been reached
  */
  int getFreeSlot();

  /*! \brief close any memories which haven't been used for a long time
    \returns 0
  */
  int cleanupMemoryList();

  /*! \brief convert a long time value into the TMX date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converterd date time
    \returns 0 
  */
  int convertTimeToTMX( LONG lTime, char *pszDateTime );

  /*! \brief convert a TMX time value to a OpenTM2 long time value 
      \param pszDateTime buffer containing the TMX date and time
      \param plTime pointer to a long buffer receiving the converted time 
    \returns 0 
  */
  int convertTMXTimeToLong( char *pszDateTime, PLONG plTime );

  /*! \brief extract a numeric value from a string
      \param pszString string containing the numeric value
      \param iLen number of digits to be processed
      \param piResult pointer to a int value receiving the extracted value
    \returns TRUE if successful and FALSE in case of errors
  */
  BOOL getValue( PSZ pszString, int iLen, int *piResult );

  /*! \brief close a memory and remove it from the open list
      \param iIndex index of memory in the open list
      \returns 0 
  */
  int removeFromMemoryList( int iIndex );

  /*! \brief Info structure for an open translation memory
  */
	typedef struct _OPENEDMEMORY
  {
    char szName[260];               // name of the memory
    time_t tLastAccessTime;         // last time memory has been used
    long lHandle;                   // memory handle          
  } OPENEDMEMORY, *POPENEDMEMORY;

/*! \brief Pointer to the list of opened memories
*/
  std::vector<OPENEDMEMORY> vMemoryList;

/*! \brief Pointer to the instance of the OtmMemoryService object (singleton).
*/
	static OtmMemoryService* instance;

/*! \brief OpenTM2 API session handle
*/
	HSESSION hSession;

/*! \brief Last return code
*/
  int iLastRC;
  
/*! \brief Buffer for last error message
*/
  wchar_t szLastError[4000];



};


#endif