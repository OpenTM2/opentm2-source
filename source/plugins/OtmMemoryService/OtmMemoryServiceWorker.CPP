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
//| Description: Processes the requests from the web service frame work        |
//+----------------------------------------------------------------------------+
//

#include <time.h>
#include <process.h>
#include <stdarg.h>
#include <windows.h>
#include <wincrypt.h>
#include <restbed>
#include "core\PluginManager\OtmMemory.h"
#include "OTMMSJSONFactory.h"
#include "OtmMemoryServiceWorker.h"
#include <eqfmsg.h>

// import memory process
void importMemoryProcess( void *pvData );


/** Initialize the static instance variable */
OtmMemoryServiceWorker* OtmMemoryServiceWorker::instance = 0;

/*! \brief This static method returns a pointer to the OtmMemoryService object.
	The first call of the method creates the OtmMemoryService instance.
*/
OtmMemoryServiceWorker* OtmMemoryServiceWorker::getInstance()
{
	if (instance == 0)
	{
		instance = new OtmMemoryServiceWorker();
    instance->hSession = 0;
  }
	return instance;
}

/*! \brief OtmMemoryServiceWorker constructor
*/
OtmMemoryServiceWorker::OtmMemoryServiceWorker()
{
  hfLog = NULL;
}


/*! \brief Data area for the processing of the deleteMemory function
*/
typedef struct _DELETEMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} DELETEMEMORYDATA, *PDELETEMEMORYDATA;



/*! \brief Data area for the processing of the createMemory function
*/
typedef struct _CREATEMEMORYDATA
{
  char szMemory[260];
  wchar_t szMemoryW[260]; // memory name in UTF-16 encoding
  char szIsoSourceLang[40];
  char szOtmSourceLang[40];
  wchar_t szError[512];
} CREATEMEMORYDATA, *PCREATEMEMORYDATA;


/*! \brief Data area for the processing of the importMemory function
*/
typedef struct _IMPORTMEMORYDATA
{
  HSESSION hSession;
  OtmMemoryServiceWorker *pMemoryServiceWorker;
  char szMemory[260];
  char szInFile[260];
  char szError[512];
} IMPORTMEMORYDATA, *PIMPORTMEMORYDATA;




/*! \brief Data area for the processing of the openMemory function
*/
typedef struct _OPENMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} OPENMEMORYDATA, *POPENMEMORYDATA;

/*! \brief Data area for the processing of the createMemory function
*/
typedef struct _CLOSEMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} CLOSEMEMORYDATA, *PCLOSEMEMORYDATA;


/*! \brief Data area for the processing of the lookupInMemory function (also used by searchMemory and updateMemory)
*/
typedef struct _LOOKUPINMEMORYDATA
{
  char szMemory[260];
  wchar_t szSource[2050];
  wchar_t szTarget[2050];
  char szIsoSourceLang[40];
  char szIsoTargetLang[40];
  char szOtmSourceLang[40];
  char szOtmTargetLang[40];
  int lSegmentNum;
  char szDocName[260];
  char szMarkup[128];
  wchar_t szContext[2050];
  wchar_t szAddInfo[2050];
  wchar_t szError[512];
  char szType[256];
  char szAuthor[80];
  char szDateTime[40];
  char szSearchMode[40];
  char szSearchPos[80];
  int iNumOfProposals;
  int iSearchTime;
  wchar_t szSearchString[2050];
} LOOKUPINMEMORYDATA, *PLOOKUPINMEMORYDATA;




  /*! \brief Verify OpenTM2 API session
    \returns 0 if successful or an error code in case of failures
  */
int OtmMemoryServiceWorker::verifyAPISession
(
)
{
  if ( this->hSession != 0 ) return( 0 );

  this->iLastRC = EqfStartSession( &(this->hSession) );
  if ( this->iLastRC != 0 ) swprintf( this->szLastError, L"OpenTM2 API session could not be started, the return code is %ld", this->iLastRC );
  return( this->iLastRC );
}

/*! \brief build return JSON string in case of errors
  \param iRC error return code
  \param pszErrorMessage error message text
  \param strError JSON string receiving the error information
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryServiceWorker::buildErrorReturn
(
  int iRC,
  wchar_t *pszErrorMsg,
  std::wstring &strErrorReturn
)
{
  JSONFactory *factory = JSONFactory::getInstance();
  int i = strErrorReturn.size();
  factory->startJSONW( strErrorReturn );
  i = strErrorReturn.size();
  factory->addParmToJSONW( strErrorReturn, L"ReturnValue", iRC );
  i = strErrorReturn.size();
  factory->addParmToJSONW( strErrorReturn, L"ErrorMsg", pszErrorMsg );
  i = strErrorReturn.size();
  factory->terminateJSONW( strErrorReturn );
  i = strErrorReturn.size();
  return( 0 );
}

int OtmMemoryServiceWorker::buildErrorReturn
(
	int iRC,
	wchar_t *pszErrorMsg,
	std::string &strErrorReturn
)
{
	std::wstring strUTF16;
	buildErrorReturn( iRC, pszErrorMsg, strUTF16 );
  strErrorReturn = convertToUTF8( strUTF16 );

	return(0);
}

int OtmMemoryServiceWorker::buildErrorReturn
(
  int iRC,
  char *pszErrorMsg,
  std::string &strErrorReturn
)
{
  JSONFactory *factory = JSONFactory::getInstance();
  int i = strErrorReturn.size();
  factory->startJSON( strErrorReturn );
  i = strErrorReturn.size();
  factory->addParmToJSON( strErrorReturn, "ReturnValue", iRC );
  i = strErrorReturn.size();
  factory->addParmToJSON( strErrorReturn, "ErrorMsg", pszErrorMsg );
  i = strErrorReturn.size();
  factory->terminateJSON( strErrorReturn );
  i = strErrorReturn.size();
  return( 0 );
}




/*! \brief find a memory in our list of active memories
  \param pszMemory name of the memory
  \returns index in the memory table if successful, -1 if memory is not contained in the list
*/
int OtmMemoryServiceWorker::findMemoryInList( const char *pszMemory )
{
  // for debugging only: cause an exception when the special memory name "XX_CauseAnException_XX" is neing used
#ifdef _DEBUG
  if ( strcmp( pszMemory, "XX_CauseAnException_XX" ) == 0 )
  {
    int *pi = NULL;
    return( *pi );
  } /* endif */
#endif 

  // 
  for( int i = 0; i < (int)this->vMemoryList.size(); i++ )
  {
    if ( stricmp( this->vMemoryList[i].szName, pszMemory ) == 0 )
    {
      return( i );
    } /* endif */
  } /* endfor */
  return( -1 );
}

/*! \brief find a free slot in our list of active memories, add a new entry if none found
  \returns index of the next free slot in the memory table or -1 if there is no free slot and the max number of entries has been reached
*/
int OtmMemoryServiceWorker::getFreeSlot()
{
  // first look for a free slot in the existing list
  for( int i = 0; i < (int)this->vMemoryList.size(); i++ )
  {
    if ( this->vMemoryList[i].szName[0] == 0 )
    {
      return( i );
    } /* endif */
  } /* endfor */

  // add a new entry when the maximum list size has not been reached yet
  if ( this->vMemoryList.size() < OTMMEMSERVICE_MAX_NUMBER_OF_OPEN_MEMORIES )
  {
    OtmMemoryServiceWorker::OPENEDMEMORY NewEntry;
    memset( &NewEntry, 0, sizeof(NewEntry) );
    this->vMemoryList.push_back( NewEntry );
    return( this->vMemoryList.size() - 1 );
  } /* endif */

  return( -1 );
}



/*! \brief close any memories which haven't been used for a long time
  \returns 0
*/
int OtmMemoryServiceWorker::cleanupMemoryList()
{
  time_t curTime;

  time( &curTime );

  // close any memory in the list which hasn't been used for a long time
  for( int i = 0; i < (int)this->vMemoryList.size(); i++ )
  {
    if ( this->vMemoryList[i].szName[0] == 0 )
    {
      if ( (curTime - this->vMemoryList[i].tLastAccessTime) > OTMMEMSERVICE_MEMORY_TIMEOUT )
      {
        removeFromMemoryList( i );
      }
    } /* endif */
  } /* endfor */

  return( 0 );
}

/*! \brief get the handle for a memory, if the memory is not opened yet it will be openend
\param pszMemory name of the memory
\param plHandle buffer for the memory handle
\param pszError buffer for an error message text in case of failures
\param iErrorBufSize size of the error text buffer in number of characters
\param piReturnCode pointer to a buffer for the OpenTM2 error code
\returns OK if successful or an HTTP error code in case of failures
*/
int OtmMemoryServiceWorker::getMemoryHandle( char *pszMemory, PLONG plHandle, wchar_t *pszError, int iErrorBufSize, int *piErrorCode )
{
  *piErrorCode = 0;
  int iIndex = findMemoryInList( pszMemory );
  if ( iIndex != -1 )
  {
    switch ( this->vMemoryList[iIndex].eStatus )
    {
      case OPEN_STATUS:
        *plHandle = this->vMemoryList[iIndex].lHandle;
        time( &( this->vMemoryList[iIndex].tLastAccessTime ) );
        return( restbed::OK );
        break;
      case IMPORT_RUNNING_STATUS:
        //wcsncpy( pszError, L"TM is busy, import is running", iErrorBufSize );
        //*piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        //return( restbed::PROCESSING );
        //break;
      case IMPORT_FAILED_STATUS:
        //wcsncpy( pszError, L"TM import failed, memory may be not usable", iErrorBufSize );
        //*piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        //return( restbed::INTERNAL_SERVER_ERROR );
        //break;
      case AVAILABLE_STATUS:
        {
          // open the memory
          LONG lHandle = 0;
          *piErrorCode = EqfOpenMem( this->hSession, pszMemory, &lHandle, 0 );
          if ( *piErrorCode != 0 )
          {
            unsigned short usRC = 0;
            EqfGetLastErrorW( this->hSession, &usRC, pszError, (unsigned short)iErrorBufSize );
            return( restbed::INTERNAL_SERVER_ERROR );
          } /* endif */
          this->vMemoryList[iIndex].lHandle = lHandle;
          strcpy( this->vMemoryList[iIndex].szName, pszMemory );
          time( &( this->vMemoryList[iIndex].tLastAccessTime ) );
          this->vMemoryList[iIndex].eStatus = OPEN_STATUS;
          //if ( this->vMemoryList[iIndex].pszError != NULL ) free( this->vMemoryList[iIndex].pszError );
          //this->vMemoryList[iIndex].pszError = NULL;
          *plHandle = lHandle;
          return( restbed::OK );
        }
        break;
      default:
        wcsncpy( pszError, L"TM is in undefined state", iErrorBufSize );
        *piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        return( restbed::INTERNAL_SERVER_ERROR );
        break;
    }
  } /* endif */


  // cleanup the memory list (close memories not used for a longer time)
  cleanupMemoryList();

  // find a free slot in the memory list
  iIndex = getFreeSlot();

  // handle "too many open memories" condition
  if ( iIndex == -1 )
  {
    wcscpy( pszError, L"Error: too many open translation memory databases" );
    return( restbed::INTERNAL_SERVER_ERROR );
  } /* endif */

  // open the memory
  LONG lHandle = 0;
  *piErrorCode = EqfOpenMem( this->hSession, pszMemory, &lHandle, 0 );
  if ( *piErrorCode != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pszError, (unsigned short)iErrorBufSize );
    return( restbed::INTERNAL_SERVER_ERROR );
  } /* endif */

  // add opened memory to the memory list
  this->vMemoryList[iIndex].lHandle = lHandle;
  strcpy( this->vMemoryList[iIndex].szName, pszMemory );
  time( &(this->vMemoryList[iIndex].tLastAccessTime) );
  this->vMemoryList[iIndex].eStatus = OPEN_STATUS;
  this->vMemoryList[iIndex].eImportStatus = OPEN_STATUS;
  this->vMemoryList[iIndex].pszError = NULL;
  *plHandle = lHandle;

  return( restbed::OK );
}

/*! \brief convert a long time value into the UTC date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converted date and time
  \returns 0 
*/
int OtmMemoryServiceWorker::convertTimeToUTC( LONG lTime, char *pszDateTime )
{
  struct tm   *pTimeDate;            // time/date structure

  if ( lTime != 0L ) lTime += 10800L;// correction: + 3 hours

  pTimeDate = gmtime( (time_t *)&lTime );
  if ( pTimeDate->tm_isdst == 1 )
  {
    // correct summertime offset
    lTime -= 3600; 
    pTimeDate = gmtime( (time_t *)&lTime );
    
  }
  if ( (lTime != 0L) && pTimeDate )   // if gmtime was successful ...
  {
    sprintf( pszDateTime, "%4.4d%2.2d%2.2dT%2.2d%2.2d%2.2dZ", 
             pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1, pTimeDate->tm_mday,
             pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec );
  }
  else
  {
    *pszDateTime = 0;
  } /* endif */

  return( 0 );
}

/*! \brief convert a long time value to a date time string in the form YYYY-MM-DD HH:MM:SS
\param lTime long time value
\param pszDateTime buffer receiving the converted date and time
\returns 0
*/
int OtmMemoryServiceWorker::convertTimeToDateTimeString( LONG lTime, char *pszDateTime )
{
  struct tm   *pTimeDate;            // time/date structure

  if ( lTime != 0L ) lTime += 10800L;// correction: + 3 hours

  pTimeDate = gmtime( (time_t *)&lTime );
  if ( ( lTime != 0L ) && pTimeDate )   // if gmtime was successful ...
  {
    sprintf( pszDateTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
      pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1, pTimeDate->tm_mday,
      pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec );
  }
  else
  {
    *pszDateTime = 0;
  } /* endif */

  return( 0 );
}


/*! \brief convert a UTC time value to a OpenTM2 long time value 
    \param pszDateTime buffer containing the UTC date and time
    \param plTime pointer to a long buffer receiving the converted time 
  \returns 0 
*/
int OtmMemoryServiceWorker::convertUTCTimeToLong( char *pszDateTime, PLONG plTime )
{
  *plTime = 0;

  // we currently support only dates in the form UTC format YYYYMMDDThhmmssZ
  if ( (pszDateTime != NULL) && (strlen(pszDateTime) == 16) )
  {
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

    // split string into date/time parts
    BOOL fOK = getValue( pszDateTime, 4, &iYear );
    if ( fOK ) fOK = getValue( pszDateTime + 4, 2, &iMonth );
    if ( fOK ) fOK = getValue( pszDateTime + 6, 2, &iDay );
    if ( fOK ) fOK = getValue( pszDateTime + 9, 2, &iHour );
    if ( fOK ) fOK = getValue( pszDateTime + 11, 2, &iMin );
    if ( fOK ) fOK = getValue( pszDateTime + 13, 2, &iSec );

    if ( fOK )
    {
      struct tm timeStruct;
      memset( &timeStruct, 0, sizeof(timeStruct) );
      timeStruct.tm_hour = iHour;
      timeStruct.tm_min = iMin;
      timeStruct.tm_sec = iSec;
      timeStruct.tm_mday = iDay;
      timeStruct.tm_mon = iMonth - 1;
      timeStruct.tm_year = iYear - 1900;

      *plTime = mktime( &timeStruct );

      // as mktime is always using the local time zone, we have to adjust the time to UTC time
      time_t lLocalTime = 0;
      time_t lGMTime = 0;
      time( &lGMTime );
      lLocalTime = mktime( localtime( &lGMTime ) );
      lGMTime = mktime( gmtime( &lGMTime ) );
      LONG lDiff = lLocalTime - lGMTime;
      *plTime += lDiff;

      *plTime -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)

    } /* endif */
  } /* endif */
  return( 0 );
}

/*! \brief convert a date and time in the form YYYY-MM-DD HH:MM:SS to a OpenTM2 long time value
\param pszDateTime buffer containing the TMX date and time
\param plTime pointer to a long buffer receiving the converted time
\returns 0
*/
int OtmMemoryServiceWorker::convertDateTimeStringToLong( char *pszDateTime, PLONG plTime )
{
  *plTime = 0;

  // we currently support only dates in the form YYYYMMDDThhmmssZ
  if ( ( pszDateTime != NULL ) && ( strlen( pszDateTime ) == 16 ) )
  {
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

    // split string into date/time parts
    BOOL fOK = getValue( pszDateTime, 4, &iYear );
    if ( fOK ) fOK = getValue( pszDateTime + 5, 2, &iMonth );
    if ( fOK ) fOK = getValue( pszDateTime + 8, 2, &iDay );
    if ( fOK ) fOK = getValue( pszDateTime + 11, 2, &iHour );
    if ( fOK ) fOK = getValue( pszDateTime + 14, 2, &iMin );
    if ( fOK ) fOK = getValue( pszDateTime + 17, 2, &iSec );

    if ( fOK )
    {
      struct tm timeStruct;
      memset( &timeStruct, 0, sizeof( timeStruct ) );
      timeStruct.tm_hour = iHour;
      timeStruct.tm_min = iMin;
      timeStruct.tm_sec = iSec;
      timeStruct.tm_mday = iDay;
      timeStruct.tm_mon = iMonth - 1;
      timeStruct.tm_year = iYear - 1900;

      *plTime = mktime( &timeStruct );

      *plTime -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)

    } /* endif */
  } /* endif */
  return( 0 );
}

/*! \brief extract a numeric value from a string
    \param pszString string containing the numeric value
    \param iLen number of digits to be processed
    \param piResult pointer to a int value receiving the extracted value
  \returns TRUE if successful and FALSE in case of errors
*/
BOOL OtmMemoryServiceWorker::getValue( PSZ pszString, int iLen, int *piResult )
{
  BOOL fOK = TRUE;
  char szNumber[10];
  char *pszNumber = szNumber;

  *piResult = 0;

  while ( iLen && fOK )
  {
    if ( isdigit(*pszString) )
    {
      *pszNumber++ = *pszString++;
      iLen--;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /*endwhile */

  if ( fOK )
  {
    *pszNumber = '\0';
    *piResult = atoi( szNumber );
  } /* endif */

  return( fOK );
} 

/*! \brief close a memory and remove it from the open list
    \param iIndex index of memory in the open list
  \returns 0 
*/
int OtmMemoryServiceWorker::removeFromMemoryList( int iIndex )
{
  LONG lHandle = this->vMemoryList[iIndex].lHandle;

  // remove the memory from the list
  this->vMemoryList[iIndex].lHandle = 0;
  this->vMemoryList[iIndex].tLastAccessTime = 0;
  this->vMemoryList[iIndex].szName[0] = 0;
  if ( this->vMemoryList[iIndex].pszError ) free( this->vMemoryList[iIndex].pszError );
  this->vMemoryList[iIndex].pszError = NULL;

  // close the memory
  EqfCloseMem( this->hSession, lHandle, 0 );

  return( 0 );
} 

/*! \brief convert a UTF8 std::string to a UTF16 std::wstring
  \param strUTF8String string in UTF8 encoding
  \returns string converted to UTF16
*/
std::wstring OtmMemoryServiceWorker::convertToUTF16( const std::string& strUTF8String )
{
	int iUTF16Len;
	int iUTF8Len = (int)strUTF8String.length() + 1;
	iUTF16Len = MultiByteToWideChar( CP_UTF8, 0, strUTF8String.c_str(), iUTF8Len, 0, 0 );
	std::wstring strUTF16( iUTF16Len, L'\0');
	MultiByteToWideChar( CP_UTF8, 0, strUTF8String.c_str(), iUTF8Len, &strUTF16[0], iUTF16Len );
	return strUTF16;
}

/*! \brief convert a UTF8 std::string to a UTF16 std::wstring
\param strUTF8String string in UTF8 encoding
\returns string converted to UTF16
*/
std::string OtmMemoryServiceWorker::convertToUTF8(const std::wstring& strUTF16String)
{
	int iUTF8Len;
	int iUTF16Len = (int)strUTF16String.length() + 1;
	iUTF8Len = WideCharToMultiByte( CP_UTF8, 0, strUTF16String.c_str(), iUTF16Len, 0, 0, 0, 0 );
	std::string strUTF8( iUTF8Len, '\0');
	WideCharToMultiByte( CP_UTF8, 0, strUTF16String.c_str(), iUTF16Len, &strUTF8[0], iUTF8Len, 0, 0 );
	return strUTF8;
}

/*! \brief convert a UTF8 std::string to a ASCII std::string (on spot conversion)
\param strText on input string in UTF8 encoding, on output string in ASCII encoding
\returns string converted to UTF16
*/
void OtmMemoryServiceWorker::convertUTF8ToASCII( std::string& strText )
{
  int iUTF16Len;
  int iASCIILen;
  int iUTF8Len = (int)strText.length() + 1;
  char *pszNewData = NULL;
  iUTF16Len = MultiByteToWideChar( CP_UTF8, 0, strText.c_str(), iUTF8Len, 0, 0 );
  std::wstring strUTF16( iUTF16Len, L'\0' );
  MultiByteToWideChar( CP_UTF8, 0, strText.c_str(), iUTF8Len, &strUTF16[0], iUTF16Len );
  iASCIILen = WideCharToMultiByte( CP_UTF8, 0, strUTF16.c_str(), iUTF16Len, 0, 0, 0, 0 );
  pszNewData = (char *)malloc( iASCIILen + 1 );
  WideCharToMultiByte( CP_OEMCP, 0, strUTF16.c_str(), iUTF16Len, pszNewData, iASCIILen, 0, 0 );
  strText = pszNewData;
  free( pszNewData );
}

/*! \brief convert a ASCII std::string to UTF8 std::string (on spot conversion)
\param strText on input string in ASCII encoding, on output string in UTF8 encoding
*/
void OtmMemoryServiceWorker::convertASCIIToUTF8( std::string& strText )
{
  int iUTF16Len;
  int iUTF8Len;
  int iASCIILen = (int)strText.length() + 1;
  char *pszNewData = NULL;
  iUTF16Len = MultiByteToWideChar( CP_OEMCP, 0, strText.c_str(), iASCIILen, 0, 0 );
  std::wstring strUTF16( iUTF16Len, L'\0' );
  MultiByteToWideChar( CP_OEMCP, 0, strText.c_str(), iASCIILen, &strUTF16[0], iUTF16Len );
  iUTF8Len = WideCharToMultiByte( CP_UTF8, 0, strUTF16.c_str(), iUTF16Len, 0, 0, 0, 0 );
  pszNewData = (char *)malloc( iUTF8Len + 1);
  WideCharToMultiByte( CP_UTF8, 0, strUTF16.c_str(), iUTF16Len, pszNewData, iUTF8Len, 0, 0 );
  strText = pszNewData;
  free( pszNewData );
}


/*! \brief Import a memory from a TMX file
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns 0 if successful or an error code in case of failures
*/
int OtmMemoryServiceWorker::import
(
  std::string  strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  convertUTF8ToASCII( strMemory );
  if ( hfLog ) fprintf( hfLog, "+POST %s/import\n", strMemory.c_str() );
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    buildErrorReturn( iRC, L"Missing name of memory", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  // check if memory exists
  if ( EqfMemoryExists( this->hSession, (PSZ)strMemory.c_str() ) != 0 )
  {
    return( restbed::NOT_FOUND );
  }

  // extract TMX data
  std::string strTmxData;
  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( strInputParms, &iRC );
  if ( parseHandle == NULL )
  {
    buildErrorReturn( iRC, L"Missing or incorrect JSON data in request body", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */

  while ( iRC == 0 )
  {
    std::string name;
    std::string value;
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      if ( stricmp( name.c_str(), "tmxData" ) == 0 )
      {
        strTmxData = value;
        break;
      }
    }
  } /* endwhile */
  factory->parseJSONStop( parseHandle );
  if ( strTmxData.empty() )
  {
    buildErrorReturn( iRC, L"Missing TMX data", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  // setup temp file name for TMX file 
  char szTempFile[MAX_PATH];
  iRC = buildTempFileName( szTempFile );
  if ( iRC != 0  )
  {
    buildErrorReturn( -1, L"Could not create file name for temporary data", strOutputParms );
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  if ( hfLog ) fprintf( hfLog, "+   Temp TMX File is %s\n", szTempFile );

  // decode TMX data and write it to temp file
  std::string strError;
  iRC = decodeBase64ToFile( strTmxData.c_str(), szTempFile, strError ) ;
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, (char *)strError.c_str(), strOutputParms );
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  // find the memory to our memory list
  int iIndex = this->findMemoryInList( strMemory.c_str() );
  if ( iIndex != -1 )
  {
    // close the memory - when open
    if ( this->vMemoryList[iIndex].eStatus == OPEN_STATUS )
    {
      LONG lHandle = this->vMemoryList[iIndex].lHandle;
      this->vMemoryList[iIndex].lHandle = 0;
      this->vMemoryList[iIndex].eStatus = AVAILABLE_STATUS;
      this->vMemoryList[iIndex].eImportStatus = IMPORT_RUNNING_STATUS;
      EqfCloseMem( this->hSession, lHandle, 0 );
    }
  }
  else
  {
    // cleanup the memory list (close memories not used for a longer time)
    cleanupMemoryList();

    // find a free slot in the memory list
    iIndex = getFreeSlot();

    // handle "too many open memories" condition
    if ( iIndex == -1 )
    {
      buildErrorReturn( iRC, L"Error: too many open translation memory databases", strOutputParms );
      return( restbed::INTERNAL_SERVER_ERROR );
    } /* endif */

    this->vMemoryList[iIndex].lHandle = 0;
    this->vMemoryList[iIndex].eStatus = AVAILABLE_STATUS;
    this->vMemoryList[iIndex].eImportStatus = IMPORT_RUNNING_STATUS;
    strcpy( this->vMemoryList[iIndex].szName, strMemory.c_str() );
  }

  // start the import background process
  PIMPORTMEMORYDATA pData = new( IMPORTMEMORYDATA );
  strcpy( pData->szInFile, szTempFile );
  strcpy( pData->szMemory, strMemory.c_str() );
  pData->hSession = hSession;
  pData->pMemoryServiceWorker = this;
  _beginthread( &importMemoryProcess, 0, (void *)pData );

  return( restbed::CREATED );
}

/*! \brief Create a new memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns 0 if successful or an error code in case of failures
*/
int OtmMemoryServiceWorker::createMemory
(
  std::string strInputParms,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  std::string strData;
  std::string strName;
  std::string strSourceLang;
  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( strInputParms, &iRC );
  if ( parseHandle == NULL )
  {
    buildErrorReturn( iRC, L"Missing or incorrect JSON data in request body", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */

  std::string name;
  std::string value;
  while ( iRC == 0 )
  {
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      if ( stricmp( name.c_str(), "data" ) == 0 )
      {
        strData = value;
      }
      else if ( stricmp( name.c_str(), "name" ) == 0 )
      {
        strName = value;
      }
      else if ( stricmp( name.c_str(), "sourceLang" ) == 0 )
      {
        strSourceLang = value;
      }
    }
  } /* endwhile */
  factory->parseJSONStop( parseHandle );

  //std::wstring strInputParmsW = convertToUTF16( strInputParms );

  //PCREATEMEMORYDATA pData = new( CREATEMEMORYDATA );
  //memset( pData, 0, sizeof( CREATEMEMORYDATA ) );
  //JSONFactory *factory = JSONFactory::getInstance();
  //JSONFactory::JSONPARSECONTROL parseControl[] = { { L"name", JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szMemory ), sizeof( pData->szMemory ) },
  //                                                 { L"sourceLang",  JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoSourceLang ), sizeof( pData->szIsoSourceLang ) },
  //                                                 { L"data",  JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoSourceLang ), sizeof( pData->szIsoSourceLang ) },
  //                                                 { L"",      JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  if ( strName.empty() )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory name input parameter", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( strSourceLang.empty() )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source language input parameter", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */

  // convert the ISO source language to a OpenTM2 source language name
  char szOtmSourceLang[40];
  EqfGetOpenTM2Lang( this->hSession, (PSZ)strSourceLang.c_str(), szOtmSourceLang );
  if ( szOtmSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    swprintf( this->szLastError, L"Error: Could not convert language %S to OpenTM2 language name", strSourceLang.c_str() );
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */

  // either create an empty memory or build the memory using binary input data
  convertUTF8ToASCII( strName );
  if ( strData.empty() )
  {
    iRC = (int)EqfCreateMem( this->hSession, (PSZ)strName.c_str(), "", 0, szOtmSourceLang, 0 );
  }
  else
  {
    // setup temp file name for ZIP package file 
    char szTempFile[MAX_PATH];
    iRC = buildTempFileName( szTempFile );
    if ( iRC != 0 )
    {
      buildErrorReturn( -1, L"Could not create file name for temporary data", strOutputParms );
      return( restbed::INTERNAL_SERVER_ERROR );
    }

    if ( hfLog ) fprintf( hfLog, "+   Temp binary file is %s\n", szTempFile );

    // decode binary data and write it to temp file
    std::string strError;
    iRC = decodeBase64ToFile( strData.c_str(), szTempFile, strError );
    if ( iRC != 0 )
    {
      buildErrorReturn( iRC, (char *)strError.c_str(), strOutputParms );
      return( restbed::INTERNAL_SERVER_ERROR );
    }

    iRC = (int)EqfImportMemInInternalFormat( this->hSession, (PSZ)strName.c_str(), szTempFile, 0 );

    DeleteFile( szTempFile );
  }

  if ( iRC != 0 )
  {
    char szError[MAX_PATH];
    unsigned short usRC = 0;
    EqfGetLastError( this->hSession, &usRC, szError, sizeof( szError ) );
    buildErrorReturn( iRC, szError, strOutputParms );
    switch ( usRC )
    {
      case ERROR_MEM_NAME_INVALID:
        iRC = restbed::CONFLICT;
        break;
      case TMT_MANDCMDLINE:
      case ERROR_NO_SOURCELANG:
      case ERROR_PROPERTY_LANG_DATA:
        iRC = restbed::BAD_REQUEST;
        break;
      default:
        iRC = restbed::INTERNAL_SERVER_ERROR;
    }
    return( iRC );
  }

  factory->startJSON( strOutputParms );

  convertASCIIToUTF8( strName );

  factory->addParmToJSON( strOutputParms, "name", strName );
  factory->terminateJSON( strOutputParms );
  return( restbed::OK );
}

/* write a single proposal to a JSON string
\param strJSON JSON stirng receiving the proposal data
\param pProp pointer to a MEMPROPOSAL containing the proposal
\param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
\returns 0 is successful
*/
int OtmMemoryServiceWorker::addProposalToJSONString
(
  std::wstring &strJSON,
  PMEMPROPOSAL pProp,
  void *pvData
)
{
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  PLOOKUPINMEMORYDATA pData = (PLOOKUPINMEMORYDATA)pvData;

  pJsonFactory->addElementStartToJSONW( strJSON );

  pJsonFactory->addParmToJSONW( strJSON, L"source", pProp->szSource );
  pJsonFactory->addParmToJSONW( strJSON, L"target", pProp->szTarget );
  pJsonFactory->addParmToJSONW( strJSON, L"segmentNumber", pProp->lSegmentNum );
  MultiByteToWideChar( CP_OEMCP, 0, pProp->szId, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"id", pData->lSegmentNum );
  MultiByteToWideChar( CP_OEMCP, 0, pProp->szDocName, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"documentName", pData->szSource );
  MultiByteToWideChar( CP_OEMCP, 0, pProp->szDocShortName, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"documentShortName", pData->szSource );
  EqfGetIsoLang( this->hSession, pProp->szSourceLanguage, pData->szIsoSourceLang );
  MultiByteToWideChar( CP_OEMCP, 0, pData->szIsoSourceLang, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"sourceLang", pData->szSource );
  EqfGetIsoLang( this->hSession, pProp->szTargetLanguage, pData->szIsoSourceLang );
  MultiByteToWideChar( CP_OEMCP, 0, pData->szIsoSourceLang, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"targetLang", pData->szSource );

  switch ( pProp->eType )
  {
    case GLOBMEMORY_PROPTYPE: wcscpy( pData->szSource, L"GlobalMemory" ); break;
    case GLOBMEMORYSTAR_PROPTYPE: wcscpy( pData->szSource, L"GlobalMemoryStar" ); break;
    case MACHINE_PROPTYPE: wcscpy( pData->szSource, L"MachineTranslation" ); break;
    case MANUAL_PROPTYPE: wcscpy( pData->szSource, L"Manual" ); break;
    default: wcscpy( pData->szSource, L"undefined" ); break;
  }
  pJsonFactory->addParmToJSONW( strJSON, L"type", pData->szSource );

  switch ( pProp->eMatch )
  {
    case EXACT_MATCHTYPE: wcscpy( pData->szSource, L"Exact" ); break;
    case EXACTEXACT_MATCHTYPE: wcscpy( pData->szSource, L"ExactExact" ); break;
    case EXACTSAMEDOC_MATCHTYPE: wcscpy( pData->szSource, L"ExactSameDoc" ); break;
    case FUZZY_MATCHTYPE: wcscpy( pData->szSource, L"Fuzzy" ); break;
    case REPLACE_MATCHTYPE: wcscpy( pData->szSource, L"Replace" ); break;
    default: wcscpy( pData->szSource, L"undefined" ); break;
  }
  pJsonFactory->addParmToJSONW( strJSON, L"matchType", pData->szSource );

  MultiByteToWideChar( CP_OEMCP, 0, pProp->szTargetAuthor, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"author", pData->szSource );

  convertTimeToUTC( pProp->lTargetTime, pData->szDocName );
  MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"timestamp", pData->szSource );

  pJsonFactory->addParmToJSONW( strJSON, L"matchRate", pProp->iFuzziness );

  MultiByteToWideChar( CP_OEMCP, 0, pProp->szMarkup, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"markupTable", pData->szSource );

  pJsonFactory->addParmToJSONW( strJSON, L"context", pProp->szContext );

  pJsonFactory->addParmToJSONW( strJSON, L"additionalInfo", pProp->szAddInfo );

  pJsonFactory->addElementEndToJSONW( strJSON );

  return( 0 );
}


/* write proposals to a JSON string 
\param strJSON JSON stirng receiving the proposal data
\param pProposals pointer to a MEMPROPOSAL array containing the proposals
\param iNumOfProposals number of proposals to write to JSON string
\param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
\returns 0 is successful
*/
int OtmMemoryServiceWorker::addProposalsToJSONString
(
  std::wstring &strJSON,
  PMEMPROPOSAL pProposals,
  int iNumOfProposals,
  void *pvData
)
{
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  PLOOKUPINMEMORYDATA pData = (PLOOKUPINMEMORYDATA)pvData;
  
  pJsonFactory->addArrayStartToJSONW( strJSON );
  for ( int i = 0; i < iNumOfProposals; i++ )
  {
    PMEMPROPOSAL pProp = pProposals + i;
    addProposalToJSONString( strJSON, pProp, pvData );
  } /* endfor */
  pJsonFactory->addArrayEndToJSONW( strJSON  );
  
  return( 0 );
}

/*! \brief Search for matching proposals
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::search
(
  std::string  strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    buildErrorReturn( iRC, L"Missing name of memory", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

    // parse input parameters
  std::wstring strInputParmsW = convertToUTF16( strInputParms );
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof( LOOKUPINMEMORYDATA ) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szSource ), sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) },
                                                   { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( pData->lSegmentNum ), 0 },
                                                   { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szDocName ), sizeof( pData->szDocName ) },
                                                   { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoSourceLang ), sizeof( pData->szIsoSourceLang ) },
                                                   { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoTargetLang ), sizeof( pData->szIsoTargetLang ) },
                                                   { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szMarkup ), sizeof( pData->szMarkup ) },
                                                   { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szContext ), sizeof( pData->szContext ) / sizeof( pData->szContext[0] ) },
                                                   { L"numOfProposals", JSONFactory::INT_PARM_TYPE,          &( pData->iNumOfProposals ), 0 },
                                                   { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParmsW, parseControl );
  if ( iRC != 0 )
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */

  if ( pData->szSource[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source text input parameter", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source language input parameter", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoTargetLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing target language input parameter", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szMarkup[0] == 0 )
  {
    // use default markup table if none given
    strcpy( pData->szMarkup, "OTMANSI" );
  } /* end */
  if ( pData->iNumOfProposals > 20 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Too many proposals requested, the maximum value is 20", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->iNumOfProposals == 0 )
  {
    pData->iNumOfProposals = 5;
  }

  // get the handle of the memory 
  long lHandle = 0;
  int httpRC = this->getMemoryHandle( (char *)strMemory.c_str(), &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != restbed::OK )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( httpRC );
  } /* endif */

  // prepare the memory lookup
  PMEMPROPOSAL pSearchKey = new (MEMPROPOSAL);
  PMEMPROPOSAL pFoundProposals = new( MEMPROPOSAL[pData->iNumOfProposals] );
  memset( pSearchKey, 0, sizeof( *pSearchKey ) );
  memset( pFoundProposals, 0, sizeof( MEMPROPOSAL ) * pData->iNumOfProposals );
  wcscpy( pSearchKey->szSource, pData->szSource );
  strcpy( pSearchKey->szDocName, pData->szDocName );
  pSearchKey->lSegmentNum = pData->lSegmentNum;
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoSourceLang, pSearchKey->szSourceLanguage );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoTargetLang, pSearchKey->szTargetLanguage );
  strcpy( pSearchKey->szMarkup, pData->szMarkup );
  wcscpy( pSearchKey->szContext, pData->szContext );

  // do the lookup and handle the results
  int iNumOfProposals = pData->iNumOfProposals;
  iRC = EqfQueryMem( this->hSession, lHandle, pSearchKey, &iNumOfProposals, pFoundProposals, 0 );
  if ( iRC == 0 )
  {
    std::wstring strOutputParmsW;
    factory->startJSONW( strOutputParmsW );
    factory->addParmToJSONW( strOutputParmsW, L"ReturnValue", iRC );
    factory->addParmToJSONW( strOutputParmsW, L"ErrorMsg", L"" );
    factory->addParmToJSONW( strOutputParmsW, L"NumOfFoundProposals", iNumOfProposals );
    if ( iNumOfProposals > 0 )
    {
      factory->addNameToJSONW( strOutputParmsW, L"results" );
      addProposalsToJSONString( strOutputParmsW, pFoundProposals, iNumOfProposals, (void *)pData );
    } /* endif */

    factory->terminateJSONW( strOutputParmsW );
    strOutputParms = convertToUTF8( strOutputParmsW );
    iRC = restbed::OK;
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( usRC, pData->szError, strOutputParms );
    iRC = restbed::INTERNAL_SERVER_ERROR;
  } /* endif */

  if ( pSearchKey ) delete pSearchKey;
  if ( pFoundProposals ) delete pFoundProposals;
  delete pData;

  return( iRC );
}


/*! \brief Do a concordance search for the given search string
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::concordanceSearch
(
  std::string  strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    buildErrorReturn( iRC, L"Missing name of memory", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  // parse input parameters
  std::wstring strInputParmsW = convertToUTF16( strInputParms );
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof( LOOKUPINMEMORYDATA ) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"searchString",   JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szSearchString ), sizeof( pData->szSearchString ) / sizeof( pData->szSearchString[0] ) },
  { L"searchType",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szSearchMode ), sizeof( pData->szSearchMode ) },
  { L"searchPosition", JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szSearchPos ), sizeof( pData->szSearchPos ) },
  { L"numResults",     JSONFactory::INT_PARM_TYPE,          &( pData->iNumOfProposals ), 0 },
  { L"numOfProposals", JSONFactory::INT_PARM_TYPE,          &( pData->iNumOfProposals ), 0 },
  { L"msSearchAfterNumResults", JSONFactory::INT_PARM_TYPE,          &( pData->iSearchTime ), 0 },
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParmsW, parseControl );
  if ( iRC != 0 )
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */

  if ( pData->szSearchString[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing search string", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->iNumOfProposals > 20 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Too many proposals requested, the maximum value is 20", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->iNumOfProposals == 0 )
  {
    pData->iNumOfProposals = 5;
  }

    // get the handle of the memory 
  long lHandle = 0;
  int httpRC = this->getMemoryHandle( (char *)strMemory.c_str(), &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != restbed::OK )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( httpRC );
  } /* endif */

  // do the search and handle the results
  LONG lOptions = 0;
  if ( stricmp( pData->szSearchMode, "Source" ) == 0 )
  {
    lOptions |= SEARCH_IN_SOURCE_OPT;
  }
  else if ( stricmp( pData->szSearchMode, "Target" ) == 0 )
  {
    lOptions |= SEARCH_IN_TARGET_OPT;
  }
  else if ( stricmp( pData->szSearchMode, "SourceAndTarget" ) == 0 )
  {
    lOptions |= SEARCH_IN_SOURCE_OPT | SEARCH_IN_TARGET_OPT;
  } /* endif */
  lOptions |= SEARCH_CASEINSENSITIVE_OPT;

  PMEMPROPOSAL pProposal = new( MEMPROPOSAL );
  std::wstring strProposals;

  // loop until end reached or enough proposals have been found
  int iFoundProposals = 0;
  int iActualSearchTime = 0; // for the first call run until end of TM or one proposal has been found
  do
  {
    memset( pProposal, 0, sizeof( *pProposal ) );
    iRC = EqfSearchMem( this->hSession, lHandle, pData->szSearchString, pData->szSearchPos, pProposal, iActualSearchTime, lOptions );
    iActualSearchTime = pData->iSearchTime;
    if ( iRC == 0 )
    {
      addProposalToJSONString( strProposals, pProposal, (void *)pData );
      iFoundProposals++;
    }
  } while ( ( iRC == 0 ) && ( iFoundProposals < pData->iNumOfProposals ) );

  if ( iFoundProposals || (iRC == ENDREACHED_RC) || (iRC == TIMEOUT_RC) )
  {
    std::wstring strOutputParmsW;
    factory->startJSONW( strOutputParmsW );
//    factory->addParmToJSONW( strOutputParmsW, L"ReturnValue", iRC );
    if ( iRC == ENDREACHED_RC )
    {
      factory->addParmToJSONW( strOutputParmsW, L"NewSearchPosition" );
    }
    else
    {
      factory->addParmToJSONW( strOutputParmsW, L"NewSearchPosition", pData->szSearchPos );
    }
    if ( iFoundProposals > 0 )
    {
      factory->addNameToJSONW( strOutputParmsW, L"results" );
      factory->addArrayStartToJSONW( strOutputParmsW );
      strOutputParmsW.append( strProposals );
      factory->addArrayEndToJSONW( strOutputParmsW );
    } /* endif */

    factory->terminateJSONW( strOutputParmsW );
    strOutputParms = convertToUTF8( strOutputParmsW );
    iRC = restbed::OK;
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( usRC, pData->szError, strOutputParms );
    iRC = restbed::INTERNAL_SERVER_ERROR;
  } /* endif */

  //if ( pFoundProposals ) delete pFoundProposals;
  if ( pProposal ) delete pProposal;
  delete pData;

  return( iRC );
}


/*! \brief Close all open memories
\returns http return code0 if successful or an error code in case of failures
*/
int OtmMemoryServiceWorker::closeAll
(
)
{
  for ( int i = 0; i < ( int )this->vMemoryList.size(); i++ )
  {
    if ( this->vMemoryList[i].szName[0] != 0 )
    {
      removeFromMemoryList( i );
    }
  } /* endfor */
  return( 0 );
}

/*! \brief Set the log file handle
\param hfLog log file handle or NULL to stop logging
*/
void OtmMemoryServiceWorker::setLogFile
(
  FILE *hfLogIn
)
{
  this->hfLog = hfLogIn;
}

/*! \brief List all available TMs
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::list
(
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  // get buffer size required for the list of TMs
  LONG lBufferSize = 0;
  iRC = EqfListMem( this->hSession, NULL, &lBufferSize );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    buildErrorReturn( usRC, this->szLastError, strOutputParms );
    return( restbed::INTERNAL_SERVER_ERROR );
  } /* endif */

  // get the list of TMs
  PSZ pszBuffer = new( char[lBufferSize] );
  iRC = EqfListMem( this->hSession, pszBuffer, &lBufferSize );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    buildErrorReturn( usRC, this->szLastError, strOutputParms );
    delete pszBuffer;
    return( restbed::INTERNAL_SERVER_ERROR );
  } /* endif */

  // add all TMs to reponse area
  std::wstring strOutputParmsW;
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  pJsonFactory->startJSONW( strOutputParmsW );
  pJsonFactory->addArrayStartToJSONW( strOutputParmsW );
  PSZ pszCurTM = pszBuffer;
  while ( *pszCurTM != 0 )
  {
    wchar_t szTM[MAX_PATH];

    // isolate TM name and convert it to UTF16
    PSZ pszEnd = strchr( pszCurTM, ',' );
    if ( pszEnd ) *pszEnd = 0;
    MultiByteToWideChar( CP_OEMCP, 0, pszCurTM, -1, szTM, sizeof( szTM ) / sizeof( szTM[0] ) );

    // add name to response area
    pJsonFactory->addElementStartToJSONW( strOutputParmsW );
    pJsonFactory->addParmToJSONW( strOutputParmsW, L"name", szTM );
    pJsonFactory->addElementEndToJSONW( strOutputParmsW );

    // go to next TM
    if ( pszEnd )
    {
      pszCurTM = pszEnd + 1;
    }
    else
    {
      pszCurTM += strlen( pszCurTM );
    }
  } /* endwhile */
  pJsonFactory->addArrayEndToJSONW( strOutputParmsW );
  pJsonFactory->terminateJSONW( strOutputParmsW );
  strOutputParms = convertToUTF8( strOutputParmsW );
  iRC = restbed::OK;
}


/*! \brief Update an entry of the memory
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::updateEntry
(
  std::string strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    buildErrorReturn( iRC, L"Missing name of memory", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  convertUTF8ToASCII( strMemory );

    // parse input parameters
  std::wstring strInputParmsW = convertToUTF16( strInputParms );
  // parse input parameters
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof( LOOKUPINMEMORYDATA ) );
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szSource ), sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) },
  { L"target",         JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szTarget ), sizeof( pData->szTarget ) / sizeof( pData->szTarget[0] ) },
  { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( pData->lSegmentNum ), 0 },
  { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szDocName ), sizeof( pData->szDocName ) },
  { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoSourceLang ), sizeof( pData->szIsoSourceLang ) },
  { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoTargetLang ), sizeof( pData->szIsoTargetLang ) },
  { L"type",           JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szType ), sizeof( pData->szType ) },
  { L"author",         JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szAuthor ), sizeof( pData->szAuthor ) },
  { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szMarkup ), sizeof( pData->szMarkup ) },
  { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szContext ), sizeof( pData->szContext ) / sizeof( pData->szContext[0] ) },
  { L"timeStamp",      JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szDateTime ), sizeof( pData->szDateTime ) },
  { L"addInfo",        JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szAddInfo ), sizeof( pData->szAddInfo ) / sizeof( pData->szAddInfo[0] ) },
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = pJsonFactory->parseJSON( strInputParmsW, parseControl );
  if ( iRC != 0 )
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */

  if ( pData->szSource[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source text", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szTarget[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing target text", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source language", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoTargetLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing target language", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szMarkup[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing markup table", strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */

    // get the handle of the memory 
  long lHandle = 0;
  int httpRC = this->getMemoryHandle( (char *)strMemory.c_str(), &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != restbed::OK )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( httpRC );
  } /* endif */

    // prepare the proposal data
  PMEMPROPOSAL pProp = new ( MEMPROPOSAL );
  memset( pProp, 0, sizeof( *pProp ) );
  wcscpy( pProp->szSource, pData->szSource );
  wcscpy( pProp->szTarget, pData->szTarget );
  pProp->lSegmentNum = pData->lSegmentNum;
  strcpy( pProp->szDocName, pData->szDocName );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoSourceLang, pProp->szSourceLanguage );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoTargetLang, pProp->szTargetLanguage );
  pProp->eType = this->getMemProposalType( pData->szType );
  strcpy( pProp->szTargetAuthor, pData->szAuthor );
  if ( pData->szMarkup[0] == 0 )
  {
    // use default
    strcpy( pProp->szMarkup, "OTMANSI" );
  }
  else if ( stricmp( pData->szMarkup, "translate5" ) == 0 )
  {
    strcpy( pProp->szMarkup, "OTMXUXLF" );
  }
  else
  {
    strcpy( pProp->szMarkup, pData->szMarkup );
  }
  wcscpy( pProp->szContext, pData->szContext );
  LONG lTime = 0;
  if ( pData->szDateTime[0] != 0 )
  {
    // use provided time stamp
    convertUTCTimeToLong( pData->szDateTime, &(pProp->lTargetTime) );
  }
  else
  {
    // a lTime value of zero automatically sets the update time
    // so refresh the time stamp (using OpenTM2 very special time logic...)
    // and convert the time to a date time string
    LONG            lTimeStamp;             // buffer for current time
    time( (time_t*)&lTimeStamp );
    lTimeStamp -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
    this->convertTimeToUTC( lTimeStamp, pData->szDateTime );
  }
  wcscpy( pProp->szAddInfo, pData->szAddInfo );

  // update the memory
  iRC = EqfUpdateMem( this->hSession, lHandle, pProp, 0 );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( restbed::INTERNAL_SERVER_ERROR );
  } /* endif */

  // return the entry data
  std::wstring strOutputParmsW;
  pJsonFactory->startJSONW( strOutputParmsW );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"sourceLang", pData->szIsoSourceLang );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"targetLang", pData->szIsoTargetLang );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"source", pData->szSource );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"target", pData->szTarget );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"documentName", pData->szDocName );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"segmentNumber", pData->lSegmentNum );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"markupTable", pData->szMarkup );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"timeStamp", pData->szDateTime );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"author", pData->szAuthor );
  pJsonFactory->terminateJSONW( strOutputParmsW );
  strOutputParms = convertToUTF8( strOutputParmsW );

  iRC = restbed::OK;
  delete pData;

  return( iRC );
}


/*! \brief delete a memory
\param strMemory name of memory
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::deleteMem
(
  std::string strMemory,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    buildErrorReturn( iRC, L"Missing name of memory", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  JSONFactory *pJsonFactory = JSONFactory::getInstance();

  // close memory if it is open
  int iIndex = this->findMemoryInList( (char *)strMemory.c_str() );
  if ( iIndex != -1 )
  {
    // close the memory and remove it from our list
    removeFromMemoryList( iIndex );
  } /* endif */

  // delete the memory
  iRC = EqfDeleteMem( this->hSession, (PSZ)strMemory.c_str() );
  if ( iRC == ERROR_MEMORY_NOTFOUND )
  {
    return( iRC = restbed::NOT_FOUND );
  }
  else if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  iRC = restbed::OK;


  return( iRC );
}

/*! \brief get the data of a memory in binary form or TMX form
\param strMemory name of memory
\param strType type of the requested memory data
\param vMemData vector<Byte> receiving the memory data
\returns http return code
*/
int OtmMemoryServiceWorker::getMem
(
  std::string strMemory,
  std::string strType,
  restbed::Bytes &vMemData
)
{
  if ( hfLog ) fprintf( hfLog, "=== getMem request, memory = %s, format = %s\n", strMemory.c_str(), strType.c_str() );
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    if ( hfLog ) fprintf( hfLog, "    Error: no valid API session\n" );
    return( restbed::BAD_REQUEST );
  } /* endif */

  convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    if ( hfLog ) fprintf( hfLog, "    Error: no memory name specified\n" );
    return( restbed::BAD_REQUEST );
  } /* endif */

  // close memory if it is open
  int iIndex = this->findMemoryInList( (char *)strMemory.c_str() );
  if ( iIndex != -1 )
  {
    // close the memory and remove it from our list
    removeFromMemoryList( iIndex );
  } /* endif */

  // check if memory exists
  if ( EqfMemoryExists( this->hSession, (PSZ)strMemory.c_str() ) != 0 )
  {
    if ( hfLog ) fprintf( hfLog, "    Error: memory %s does not exist\n", strMemory.c_str() );
    return( restbed::NOT_FOUND );
  }

  // get a temporary file name for the memory package file or TMX file
  char szTempFile[MAX_PATH];
  iRC = buildTempFileName( szTempFile );
  if ( iRC != 0 )
  {
    if ( hfLog ) fputs( "    Error: creation of temporary file for memory data failed\n", hfLog );
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  // export the memory in internal format
  if ( strType.compare( "application/xml" ) == 0 )
  {
    iRC = EqfExportMem( this->hSession, (PSZ)strMemory.c_str(), szTempFile, TMX_UTF8_OPT | OVERWRITE_OPT | COMPLETE_IN_ONE_CALL_OPT );
    if ( iRC != 0 )
    {
      unsigned short usRC = 0;
      EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      if ( hfLog ) fprintf( hfLog, "    Error: EqfExportMem failed with rc=%ld, error message is %ws\n", iRC, this->szLastError );
      return( restbed::INTERNAL_SERVER_ERROR );
    }
  }
  else if ( strType.compare( "application/zip" ) == 0 )
  {
    iRC = EqfExportMemInInternalFormat( this->hSession, (PSZ)strMemory.c_str(), szTempFile, 0 );
    if ( iRC != 0 )
    {
      unsigned short usRC = 0;
      EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      if ( hfLog ) fprintf( hfLog, "    Error: EqfExportMemInInternalFormat failed with rc=%ld, error message is %ws\n", iRC, this->szLastError );
      return( restbed::INTERNAL_SERVER_ERROR );
    }
  }
  else
  {
    if ( hfLog ) fprintf( hfLog, "    Error: the type %s is not supported\n", strType.c_str() );
    return( restbed::NOT_ACCEPTABLE );
  }

  // fill the vMemData vector with the content of zTempFile
  iRC = this->loadFileIntoByteVector( szTempFile, vMemData );
  if ( iRC != 0 )
  {
    if ( hfLog ) fprintf( hfLog, "    Error: failed to load the temporary file\n" );
    DeleteFile( szTempFile );
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  // cleanup
  DeleteFile( szTempFile );

  return( restbed::OK );
}




/*! \brief get the status of a memory
\param strMemory name of memory
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::getStatus
(
  std::string strMemory,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    buildErrorReturn( iRC, L"Missing name of memory", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  JSONFactory *factory = JSONFactory::getInstance();

  // check if memory is contained in our list
  int iIndex = this->findMemoryInList( (char *)strMemory.c_str() );
  if ( iIndex != -1 )
  {
    // set status value
    char *pszStatus = "";
    switch ( this->vMemoryList[iIndex].eImportStatus )
    {
      case IMPORT_RUNNING_STATUS: pszStatus = "import"; break;
      case IMPORT_FAILED_STATUS: pszStatus = "failed"; break;
      default: pszStatus = "available"; break;
    }
    // return the status of the memory
    factory->startJSON( strOutputParms );
    factory->addParmToJSON( strOutputParms, "tmxImportStatus", pszStatus );
    if ( ( this->vMemoryList[iIndex].eImportStatus == IMPORT_FAILED_STATUS ) && ( this->vMemoryList[iIndex].pszError != NULL ) )
    {
      factory->addParmToJSON( strOutputParms, "ErrorMsg", this->vMemoryList[iIndex].pszError );
    }
    factory->terminateJSON( strOutputParms );
    return( restbed::OK );
  } /* endif */

  // check if memory exists
  //if ( EqfMemoryExists( this->hSession, (char *)strMemory.c_str() ) != 0 )
  //{
    return( restbed::NOT_FOUND );
  //}

  //factory->startJSON( strOutputParms );
  //factory->addParmToJSON( strOutputParms, "tmxImportStatus", "available" );
  //factory->terminateJSON( strOutputParms );
  //return( restbed::OK );
}

MemProposalType OtmMemoryServiceWorker::getMemProposalType( char *pszType )
{
  if ( stricmp( pszType, "GlobalMemory" ) == 0 )
  {
    return( GLOBMEMORY_PROPTYPE );
  }
  else if ( stricmp( pszType, "GlobalMemoryStar" ) == 0 )
  {
    return( GLOBMEMORYSTAR_PROPTYPE );
  }
  else if ( stricmp( pszType, "MachineTranslation" ) == 0 )
  {
    return( MACHINE_PROPTYPE );
  }
  else if ( stricmp( pszType, "Manual" ) == 0 )
  {
    return( MANUAL_PROPTYPE );
  } /* endif */
  return( UNDEFINED_PROPTYPE );
}

// update memory status
void OtmMemoryServiceWorker::importDone( char *pszMemory, int iRC, char *pszError )
{
  int iIndex = this->findMemoryInList( pszMemory );
  if ( iIndex != -1 )
  {
    if ( iRC == 0 )
    {
      vMemoryList[iIndex].eImportStatus = AVAILABLE_STATUS;
    }
    else
    {
      vMemoryList[iIndex].eImportStatus = IMPORT_FAILED_STATUS;
      vMemoryList[iIndex].pszError = new char[strlen( pszError ) + 1];
      strcpy( vMemoryList[iIndex].pszError, pszError );
    }
  }
}

/*! \brief build a unique name for of a temporary file
\param pszTempFile buffer reiceiving the temporary file name
\returns 0 is sucessfull or a return code
*/
int OtmMemoryServiceWorker::buildTempFileName( char *pszTempFile )
{ 
  int iRC = 0;

  // setup temp file name for TMX file 
  char szTempPath[MAX_PATH];
  iRC = GetTempPath( MAX_PATH, szTempPath );
  if ( iRC > MAX_PATH || ( iRC == 0 ) )
  {
    iRC = -1;
  }
  else
  {
    iRC = GetTempFileName( szTempPath, "OTM", 0, pszTempFile );
    if ( iRC != 0 ) iRC = 0;
  }
  return( iRC );
}

/*! \brief read a binary file and encode it using BASE64 encoding
\param pszFile fully qualified name of file being encoded
\param ppStringData adress of a pointer which will receive the (per malloc allocated) pointer to the area for the encoded string
it is up to the caller to free this area using free()
\param strError string receiving any error message text
\returns 0 is sucessfull or a return code
*/
int OtmMemoryServiceWorker::encodeFileInBase64( char *pszFile, char **ppStringData, std::string &strError )
{
  int iRC = 0;

  // open file
  HANDLE hFile = CreateFile( pszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile == NULL )
  {
    iRC = GetLastError();
    strError = "open of package file "; 
    strError.append( pszFile ); 
    strError.append( " failed" );
    return( iRC );
  }

  // get the size of file
  DWORD dwFileSize = 0;
  DWORD dwFileSizeHigh = 0;
  dwFileSize = GetFileSize( hFile, &dwFileSizeHigh );

  // allocate byte array for file content
  BYTE *pByteArray = (BYTE *)malloc( dwFileSize );
  if ( pByteArray == NULL )
  {
    CloseFile( &hFile );
    strError = "Insufficient memory";
    return( iRC );
  }

  // read file into byte array
  DWORD dwBytesRead = 0;
  if ( !ReadFile( hFile, pByteArray, dwFileSize, &dwBytesRead, NULL ) )
  {
    CloseFile( &hFile );
    free( pByteArray );
    iRC = GetLastError();
    strError = "read of package ";
    strError.append( pszFile );
    strError.append( " failed" );
    return( iRC );
  }

  // close file
  CloseFile( &hFile );

  // get encoded length of data
  DWORD dwEncodedLength = 0;
  if ( !CryptBinaryToString( pByteArray, dwBytesRead, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &dwEncodedLength ) )
  {
    CloseFile( &hFile );
    free( pByteArray );
    iRC = GetLastError();
    strError = "encoding of binary data failed";
    return( iRC );
  }

  // get area for encoded string
  char *pStringData = (char *)malloc( dwEncodedLength );
  if ( pStringData == NULL )
  {
    free( pByteArray );
    strError = "Insufficient memory";
    return( iRC );
  }

  // encode data
  if ( !CryptBinaryToString( pByteArray, dwBytesRead, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, pStringData, &dwEncodedLength ) )
  {
    free( pByteArray );
    free( pStringData );
    iRC = GetLastError();
    strError = "encoding of binary data failed";
    return( iRC );
  }

  *ppStringData = pStringData;

  // cleanup
  free( pByteArray );

  return( iRC );
}

/*! \brief convert a BASE64 encoded string to a binary file
\param pStringData pointer to the BASE64 data
\param pszFile fully qualified name of file being written
\param strError string receiving any error message text
\returns 0 is sucessfull or a return code
*/
int OtmMemoryServiceWorker::decodeBase64ToFile( const char *pStringData, const char *pszFile, std::string &strError )
{
  int iRC = 0;

  // get decoded length of data
  DWORD dwDecodedLength = 0;
  if ( !CryptStringToBinary( pStringData, 0, CRYPT_STRING_BASE64, NULL, &dwDecodedLength, NULL, NULL ) )
  {
    iRC = GetLastError();
    strError = "decoding of BASE64 data failed";
    return( iRC );
  }

  // get byte array for decoded data
  BYTE *pData = (BYTE *)malloc( dwDecodedLength );
  if ( pData == NULL )
  {
    strError = "Insufficient memory";
    return( iRC );
  }

  // decode data
  if ( !CryptStringToBinary( pStringData, 0, CRYPT_STRING_BASE64, pData, &dwDecodedLength, NULL, NULL ) )
  {
    iRC = GetLastError();
    free( pData );
    strError = "decoding of BASE64 data failed";
    return( iRC );
  }

  // open output file
  HANDLE hFile = CreateFile( pszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile == INVALID_HANDLE_VALUE )
  {
    iRC = GetLastError();
    free( pData );
    strError = "creation of output file ";
    strError.append( pszFile );
    strError.append( " failed" );
    return( iRC );
  }

  // write byte array to the file
  DWORD dwBytesWritten = 0;
  if ( !WriteFile( hFile, pData, dwDecodedLength, &dwBytesWritten, NULL ) )
  {
    iRC = GetLastError();
    free( pData );
    CloseFile( &hFile );
    strError = "write to output file  ";
    strError.append( pszFile );
    strError.append( " failed" );
    return( iRC );
  }

  // close file
  CloseFile( &hFile );

  // cleanup
  free( pData );

  return( iRC );
}

/*! \brief read a binary file into a Byte vector
\param pszFile fully qualified name of file being read
\param vFileData vector receiving the file data
it is up to the caller to free this area using free()
\param strError string receiving any error message text
\returns 0 is sucessfull or a return code
*/
int OtmMemoryServiceWorker::loadFileIntoByteVector( char *pszFile, restbed::Bytes &vFileData )
{
  int iRC = 0;

  // open file
  HANDLE hFile = CreateFile( pszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile == NULL )
  {
    iRC = GetLastError();
    if ( hfLog ) fprintf( hfLog, "   Error: open of file %s failed with rc=%ld\n", pszFile, iRC );
    return( iRC );
  }

  // get the size of file
  DWORD dwFileSize = 0;
  DWORD dwFileSizeHigh = 0;
  dwFileSize = GetFileSize( hFile, &dwFileSizeHigh );

  // adjust size of Byte vector
  vFileData.resize( dwFileSize );

  // read file into byte vector
  DWORD dwBytesRead = 0;
  if ( !ReadFile( hFile, &vFileData[0], dwFileSize, &dwBytesRead, NULL ) )
  {
    CloseFile( &hFile );
    iRC = GetLastError();
    if ( hfLog ) fprintf( hfLog, "   Error: reading of %ld bytes from file %s failed with rc=%ld\n", dwFileSize, pszFile, iRC );
    return( iRC );
  }

  // close file
  CloseFile( &hFile );

  return( iRC );
}




void importMemoryProcess( void *pvData )
{
  PIMPORTMEMORYDATA pData = (PIMPORTMEMORYDATA)pvData;

  // call the OpenTM2 API function
  pData->szError[0] = 0;
  int iRC = (int)EqfImportMem( pData->hSession, pData->szMemory, pData->szInFile, TMX_OPT | COMPLETE_IN_ONE_CALL_OPT );

  // handle any error
  char *pszError = NULL;
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastError( pData->hSession, &usRC, pData->szError, sizeof( pData->szError ) );
    pszError = new char[strlen( pData->szError ) + 1];
    strcpy( pszError, pData->szError );
  }

  // update memory status
  pData->pMemoryServiceWorker->importDone( pData->szMemory, iRC, pszError );

  // cleanup
  DeleteFile( pData->szInFile );
  delete( pData );

  return;
}

