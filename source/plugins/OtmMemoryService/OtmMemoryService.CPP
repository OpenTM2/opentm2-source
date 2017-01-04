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
#include <stdarg.h>
#include <windows.h>
#include "core\utilities\LanguageFactory.h"
#include "OtmProposal.h"
#include "JSONFactory.h"
#include "OtmMemoryService.h"


/** Initialize the static instance variable */
OtmMemoryService* OtmMemoryService::instance = 0;

/*! \brief This static method returns a pointer to the OtmMemoryService object.
	The first call of the method creates the OtmMemoryService instance.
*/
OtmMemoryService* OtmMemoryService::getInstance()
{
	if (instance == 0)
	{
		instance = new OtmMemoryService();
    instance->hSession = 0;
  }
	return instance;
}

/*! \brief Data area for the processing of the importMemoryFromPackage function
*/
typedef struct _IMPORTMEMORYFROMPACKAGEDATA
{
  char szMemory[260];
  char szFiles[260];
  wchar_t szError[512];
} IMPORTMEMORYFROMPACKAGEDATA, *PIMPORTMEMORYFROMPACKAGEDATA;

/*! \brief Import a memory using the internal memory files
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::importMemoryFromPackage
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  PIMPORTMEMORYFROMPACKAGEDATA pData = new( IMPORTMEMORYFROMPACKAGEDATA );
  memset( pData, 0, sizeof(IMPORTMEMORYFROMPACKAGEDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"Files", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szFiles), sizeof(pData->szFiles) },
                                                   { L"",      JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szFiles[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing files input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // call the OpenTM2 API function
  iRC = (int) EqfImportMemInInternalFormat( this->hSession, pData->szMemory, pData->szFiles, 0 );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof(pData->szError)/sizeof(wchar_t) );
  }
  else
  {
    pData->szError[0] = 0;
  }
  buildErrorReturn( iRC, pData->szError, strOutputParms );

  delete pData;

  return( iRC );
}

/*! \brief Data area for the processing of the deleteMemory function
*/
typedef struct _DELETEMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} DELETEMEMORYDATA, *PDELETEMEMORYDATA;


/*! \brief Delete a memory (the memory is closed automatically before the delete)
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::deleteMemory
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  PDELETEMEMORYDATA pData = new( DELETEMEMORYDATA );
  memset( pData, 0, sizeof(DELETEMEMORYDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"",      JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // close the memory if it is open
  int iIndex = this->findMemoryInList( pData->szMemory );
  if ( iIndex != -1 )
  {
    // close the memory and remove it from our list
    removeFromMemoryList( iIndex );
  } /* endif */

  // call the OpenTM2 API function
  iRC = (int) EqfDeleteMem( this->hSession, pData->szMemory );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof(pData->szError)/sizeof(wchar_t) );
  }
  else
  {
    pData->szError[0] = 0;
  }
  buildErrorReturn( iRC, pData->szError, strOutputParms );

  delete pData;

  return( iRC );
}

/*! \brief Data area for the processing of the createMemory function
*/
typedef struct _CREATEMEMORYDATA
{
  char szMemory[260];
  char szIsoSourceLang[40];
  char szOtmSourceLang[40];
  wchar_t szError[512];
} CREATEMEMORYDATA, *PCREATEMEMORYDATA;

/*! \brief Create a new memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::createMemory
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  PCREATEMEMORYDATA pData = new( CREATEMEMORYDATA );
  memset( pData, 0, sizeof(CREATEMEMORYDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"SourceLanguage",  JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szIsoSourceLang), sizeof(pData->szIsoSourceLang) },
                                                   { L"",      JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szIsoSourceLang[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source language input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // convert the ISO source language to a OpenTM2 source language name
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();
  pLangFactory->getOpenTM2NameFromISO( pData->szIsoSourceLang, pData->szOtmSourceLang );

  // call the OpenTM2 API function
  iRC = (int) EqfCreateMem( this->hSession, pData->szMemory, "", 0, pData->szOtmSourceLang, 0 );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof(pData->szError)/sizeof(wchar_t) );
  }
  else
  {
    pData->szError[0] = 0;
  }
  buildErrorReturn( iRC, pData->szError, strOutputParms );

  delete pData;

  return( iRC );
}

/*! \brief Data area for the processing of the importMemory function
*/
typedef struct _IMPORTMEMORYDATA
{
  char szMemory[260];
  char szInFile[260];
  wchar_t szError[512];
} IMPORTMEMORYDATA, *PIMPORTMEMORYDATA;


/*! \brief Import a memory from a TMX file
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::importMemory
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  PIMPORTMEMORYDATA pData = new( IMPORTMEMORYDATA );
  memset( pData, 0, sizeof(IMPORTMEMORYDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"TMXFile",JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szInFile), sizeof(pData->szInFile) },
                                                   { L"",       JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szInFile[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing TMX file name input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // call the OpenTM2 API function
  iRC = (int) EqfImportMem( this->hSession, pData->szMemory, pData->szInFile, TMX_OPT );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof(pData->szError)/sizeof(wchar_t) );
  }
  else
  {
    pData->szError[0] = 0;
  }
  buildErrorReturn( iRC, pData->szError, strOutputParms );

  delete pData;

  return( iRC );
}


/*! \brief Data area for the processing of the openMemory function
*/
typedef struct _OPENMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} OPENMEMORYDATA, *POPENMEMORYDATA;


/*! \brief Open a memory 
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::openMemory
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  POPENMEMORYDATA pData = new( OPENMEMORYDATA );
  memset( pData, 0, sizeof(OPENMEMORYDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"",       JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // get handle for the memory thus implicitly opening it
  LONG lHandle = 0;
  pData->szError[0] = 0;
  iRC = getMemoryHandle( pData->szMemory, &lHandle, pData->szError, sizeof(pData->szError)/sizeof(wchar_t) );
  buildErrorReturn( iRC, pData->szError, strOutputParms );

  delete pData;

  return( iRC );
}

/*! \brief Data area for the processing of the createMemory function
*/
typedef struct _CLOSEMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} CLOSEMEMORYDATA, *PCLOSEMEMORYDATA;

/*! \brief Close a memory 
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::closeMemory
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  PCLOSEMEMORYDATA pData = new( CLOSEMEMORYDATA );
  memset( pData, 0, sizeof(CLOSEMEMORYDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"",       JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // get the index of the memory in the memory list
  int iIndex = this->findMemoryInList( pData->szMemory );
  if ( iIndex == -1 )
  {
    iRC = ERROR_MEMORY_NOT_FOUND;
    wcscpy( pData->szError, L"Error: The memory is not opened");
  }
  else
  {
    // close the memory and remove it from our list
    removeFromMemoryList( iIndex );
    pData->szError[0] = 0;
  } /* endif */
  buildErrorReturn( iRC, pData->szError, strOutputParms );

  delete pData;

  return( iRC );
}

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
} LOOKUPINMEMORYDATA, *PLOOKUPINMEMORYDATA;


/*! \brief Get matches from a TM
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::lookupInMemory
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof(LOOKUPINMEMORYDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory",         JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"Source",         JSONFactory::UTF16_STRING_PARM_TYPE, &(pData->szSource), sizeof(pData->szSource)/sizeof(pData->szSource[0]) },
                                                   { L"Segment",        JSONFactory::INT_PARM_TYPE,          &(pData->lSegmentNum), 0 },
                                                   { L"DocumentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szDocName), sizeof(pData->szDocName) },
                                                   { L"SourceLanguage", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szIsoSourceLang), sizeof(pData->szIsoSourceLang) },
                                                   { L"TargetLanguage", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szIsoTargetLang), sizeof(pData->szIsoTargetLang) },
                                                   { L"Markup",         JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMarkup), sizeof(pData->szMarkup) },
                                                   { L"Context",        JSONFactory::UTF16_STRING_PARM_TYPE, &(pData->szContext), sizeof(pData->szContext)/sizeof(pData->szContext[0]) },
                                                   { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szSource[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source text input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szIsoSourceLang[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source language input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szIsoTargetLang[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing target language input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szMarkup[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing markup table input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // get the handle of the memory 
  long lHandle = 0;
  iRC = this->getMemoryHandle( pData->szMemory, &lHandle, pData->szError, sizeof(pData->szError)/sizeof(pData->szError[0]) );
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( iRC );
  } /* endif */

  // prepare the memory lookup
  OtmProposal SearchKey;
  std::vector<OtmProposal *> FoundProposals;
  for( int i = 0; i < 3; i++ )
  {
    FoundProposals.push_back( new OtmProposal );
  } /* endfor */
  SearchKey.setSource( pData->szSource );
  SearchKey.setDocName( pData->szDocName );
  SearchKey.setSegmentNum( pData->lSegmentNum );
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();
  pLangFactory->getOpenTM2NameFromISO( pData->szIsoSourceLang, pData->szOtmSourceLang );
  pLangFactory->getOpenTM2NameFromISO( pData->szIsoTargetLang, pData->szOtmTargetLang );
  SearchKey.setSourceLanguage( pData->szOtmSourceLang );
  SearchKey.setTargetLanguage( pData->szOtmTargetLang );
  SearchKey.setMarkup( pData->szMarkup );
  SearchKey.setContext( pData->szContext );

  // do the lookup and handle the results
  iRC = EqfQueryMem( this->hSession, lHandle, (PVOID)&SearchKey, (PVOID) &FoundProposals, 0 );
  if ( iRC == 0 )
  {
    int iNumOfProposals = OtmProposal::getNumOfProposals( FoundProposals );
    JSONFactory *factory = JSONFactory::getInstance();
    factory->startJSONW( strOutputParms );
    factory->addParmToJSONW( strOutputParms , L"ReturnValue", iRC );
    factory->addParmToJSONW( strOutputParms , L"ErrorMsg", L"" );
    factory->addParmToJSONW( strOutputParms , L"NumOfFoundProposals", iNumOfProposals );
    if ( iNumOfProposals > 0 )
    {
      factory->addNameToJSONW( strOutputParms , L"FoundProposals" );
      factory->addArrayStartToJSONW( strOutputParms );
      for( int i = 0; i < iNumOfProposals; i++ )
      {
        OtmProposal *pProp = FoundProposals[i];
        factory->addElementStartToJSONW( strOutputParms );

        pProp->getSource( pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"Source", pData->szSource );

        pProp->getTarget( pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"Target", pData->szSource );

        pData->lSegmentNum = pProp->getSegmentNum();
        factory->addParmToJSONW( strOutputParms , L"Segment", pData->lSegmentNum );

        pProp->getID( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
        MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"ID", pData->lSegmentNum );

        pProp->getDocName( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
        MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"DocumentName", pData->szSource );

        pProp->getDocShortName( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
        MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"DocumentShortName", pData->szSource );

        pProp->getSourceLanguage( pData->szOtmSourceLang, sizeof(pData->szOtmSourceLang)/sizeof(pData->szOtmSourceLang[0]) ); 
        pLangFactory->getISOName( pData->szOtmSourceLang, pData->szIsoSourceLang );
        MultiByteToWideChar( CP_OEMCP, 0, pData->szOtmSourceLang, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"SourceLanguage", pData->szSource );

        pProp->getTargetLanguage( pData->szOtmSourceLang, sizeof(pData->szOtmSourceLang)/sizeof(pData->szOtmSourceLang[0]) ); 
        pLangFactory->getISOName( pData->szOtmSourceLang, pData->szIsoSourceLang );
        MultiByteToWideChar( CP_OEMCP, 0, pData->szOtmSourceLang, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"TargetLanguage", pData->szSource );

        OtmProposal::eProposalType eType = pProp->getType();
        switch( eType )
        {
          case OtmProposal::eptGlobalMemory: wcscpy( pData->szSource, L"GlobalMemory" ); break;
          case OtmProposal::eptGlobalMemoryStar:wcscpy( pData->szSource, L"GlobalMemoryStar" ); break;
          case OtmProposal::eptMachine: wcscpy( pData->szSource, L"MachineTranslation" ); break;
          case OtmProposal::eptManual: wcscpy( pData->szSource, L"Manual" ); break;
          default: wcscpy( pData->szSource, L"undefined" ); break;
        }
        factory->addParmToJSONW( strOutputParms , L"Type", pData->szSource );

        OtmProposal::eMatchType eMatchType = pProp->getMatchType();
        switch( eMatchType )
        {
          case OtmProposal::emtExact: wcscpy( pData->szSource, L"Exact" ); break;
          case OtmProposal::emtExactExact:wcscpy( pData->szSource, L"ExactExact" ); break;
          case OtmProposal::emtExactSameDoc: wcscpy( pData->szSource, L"ExactSameDoc" ); break;
          case OtmProposal::emtFuzzy: wcscpy( pData->szSource, L"Fuzzy" ); break;
          case OtmProposal::emtReplace: wcscpy( pData->szSource, L"Replace" ); break;
          default: wcscpy( pData->szSource, L"undefined" ); break;
        }
        factory->addParmToJSONW( strOutputParms , L"Match", pData->szSource );

        pProp->getAuthor( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
        MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"Author", pData->szSource );

        long lTime = pProp->getUpdateTime();
        convertTimeToTMX( lTime, pData->szDocName );
        MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"DateTime", pData->szSource );

        int iFuzzyness = pProp->getFuzziness();
        factory->addParmToJSONW( strOutputParms , L"Fuzzyness", iFuzzyness);

        pProp->getMarkup( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
        MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
        factory->addParmToJSONW( strOutputParms , L"Markup", pData->szSource );

        pProp->getContext( pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) ); 
        factory->addParmToJSONW( strOutputParms , L"Context", pData->szSource );

        pProp->getAddInfo( pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) ); 
        factory->addParmToJSONW( strOutputParms , L"AddInfo", pData->szSource );

        factory->addElementEndToJSONW( strOutputParms );
      } /* endfor */
      factory->addArrayEndToJSONW( strOutputParms );
    } /* endif */

    factory->terminateJSONW( strOutputParms );
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof(pData->szError)/sizeof(pData->szError[0]) );
    buildErrorReturn( iRC, pData->szError, strOutputParms );
  } /* endif */

  delete pData;

  return( iRC );
}

/*! \brief Search in a TM (Concordance search)
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::searchMemory
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof(LOOKUPINMEMORYDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory",         JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"SearchString",   JSONFactory::UTF16_STRING_PARM_TYPE, &(pData->szSource), sizeof(pData->szSource)/sizeof(pData->szSource[0]) },
                                                   { L"Search",         JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szSearchMode), sizeof(pData->szSearchMode) },
                                                   { L"SearchPosition", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szSearchPos), sizeof(pData->szSearchPos) },
                                                   { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szSource[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing search string input parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // get the handle of the memory 
  long lHandle = 0;
  iRC = this->getMemoryHandle( pData->szMemory, &lHandle, pData->szError, sizeof(pData->szError)/sizeof(pData->szError[0]) );
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( iRC );
  } /* endif */

  // do the search and handle the results
  OtmProposal Result;
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
  iRC = EqfSearchMem( this->hSession, lHandle, pData->szSource, pData->szSearchPos, (PVOID)&Result, lOptions );
  if ( iRC == 0 )
  {
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    JSONFactory *factory = JSONFactory::getInstance();
    factory->startJSONW( strOutputParms );
    factory->addParmToJSONW( strOutputParms , L"ReturnValue", iRC );
    factory->addParmToJSONW( strOutputParms , L"ErrorMsg", L"" );

    MultiByteToWideChar( CP_OEMCP, 0, pData->szSearchPos, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"NextSearchPosition", pData->szSource );

    factory->addNameToJSONW( strOutputParms , L"FoundProposal" );

    factory->addElementStartToJSONW( strOutputParms );

    Result.getSource( pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"Source", pData->szSource );

    Result.getTarget( pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"Target", pData->szSource );

    pData->lSegmentNum = Result.getSegmentNum();
    factory->addParmToJSONW( strOutputParms , L"Segment", pData->lSegmentNum );

    Result.getID( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
    MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"ID", pData->lSegmentNum );

    Result.getDocName( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
    MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"DocumentName", pData->szSource );

    Result.getDocShortName( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
    MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"DocumentShortName", pData->szSource );

    Result.getSourceLanguage( pData->szOtmSourceLang, sizeof(pData->szOtmSourceLang)/sizeof(pData->szOtmSourceLang[0]) ); 
    pLangFactory->getISOName( pData->szOtmSourceLang, pData->szIsoSourceLang );
    MultiByteToWideChar( CP_OEMCP, 0, pData->szOtmSourceLang, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"SourceLanguage", pData->szSource );

    Result.getTargetLanguage( pData->szOtmSourceLang, sizeof(pData->szOtmSourceLang)/sizeof(pData->szOtmSourceLang[0]) ); 
    pLangFactory->getISOName( pData->szOtmSourceLang, pData->szIsoSourceLang );
    MultiByteToWideChar( CP_OEMCP, 0, pData->szOtmSourceLang, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"TargetLanguage", pData->szSource );

    OtmProposal::eProposalType eType = Result.getType();
    switch( eType )
    {
      case OtmProposal::eptGlobalMemory: wcscpy( pData->szSource, L"GlobalMemory" ); break;
      case OtmProposal::eptGlobalMemoryStar:wcscpy( pData->szSource, L"GlobalMemoryStar" ); break;
      case OtmProposal::eptMachine: wcscpy( pData->szSource, L"MachineTranslation" ); break;
      case OtmProposal::eptManual: wcscpy( pData->szSource, L"Manual" ); break;
      default: wcscpy( pData->szSource, L"undefined" ); break;
    }
    factory->addParmToJSONW( strOutputParms , L"Type", pData->szSource );

    OtmProposal::eMatchType eMatchType = Result.getMatchType();
    switch( eMatchType )
    {
      case OtmProposal::emtExact: wcscpy( pData->szSource, L"Exact" ); break;
      case OtmProposal::emtExactExact:wcscpy( pData->szSource, L"ExactExact" ); break;
      case OtmProposal::emtExactSameDoc: wcscpy( pData->szSource, L"ExactSameDoc" ); break;
      case OtmProposal::emtFuzzy: wcscpy( pData->szSource, L"Fuzzy" ); break;
      case OtmProposal::emtReplace: wcscpy( pData->szSource, L"Replace" ); break;
      default: wcscpy( pData->szSource, L"undefined" ); break;
    }
    factory->addParmToJSONW( strOutputParms , L"Match", pData->szSource );

    Result.getAuthor( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
    MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"Author", pData->szSource );

    long lTime = Result.getUpdateTime();
    convertTimeToTMX( lTime, pData->szDocName );
    MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"DateTime", pData->szSource );

    int iFuzzyness = Result.getFuzziness();
    factory->addParmToJSONW( strOutputParms , L"Fuzzyness", iFuzzyness);

    Result.getMarkup( pData->szDocName, sizeof(pData->szDocName)/sizeof(pData->szDocName[0]) ); 
    MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) );
    factory->addParmToJSONW( strOutputParms , L"Markup", pData->szSource );

    Result.getContext( pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) ); 
    factory->addParmToJSONW( strOutputParms , L"Context", pData->szSource );

    Result.getAddInfo( pData->szSource, sizeof(pData->szSource)/sizeof(pData->szSource[0]) ); 
    factory->addParmToJSONW( strOutputParms , L"AddInfo", pData->szSource );

    factory->addElementEndToJSONW( strOutputParms );

    factory->terminateJSONW( strOutputParms );
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof(pData->szError)/sizeof(pData->szError[0]) );
    buildErrorReturn( iRC, pData->szError, strOutputParms );
  } /* endif */

  delete pData;

  return( iRC );
}

/*! \brief Write a memory proposal to a TM 
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format 
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::updateMemory
(
  std::wstring strInputParms,
  std::wstring &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof(LOOKUPINMEMORYDATA) );
  JSONFactory *factory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"Memory",         JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMemory), sizeof(pData->szMemory) },
                                                   { L"Source",         JSONFactory::UTF16_STRING_PARM_TYPE, &(pData->szSource), sizeof(pData->szSource)/sizeof(pData->szSource[0]) },
                                                   { L"Target",         JSONFactory::UTF16_STRING_PARM_TYPE, &(pData->szTarget), sizeof(pData->szTarget)/sizeof(pData->szTarget[0]) },
                                                   { L"Segment",        JSONFactory::INT_PARM_TYPE,          &(pData->lSegmentNum), 0 },
                                                   { L"DocumentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szDocName), sizeof(pData->szDocName) },
                                                   { L"SourceLanguage", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szIsoSourceLang), sizeof(pData->szIsoSourceLang) },
                                                   { L"TargetLanguage", JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szIsoTargetLang), sizeof(pData->szIsoTargetLang) },
                                                   { L"Type",           JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szType), sizeof(pData->szType) },
                                                   { L"Author",         JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szAuthor), sizeof(pData->szAuthor) },
                                                   { L"Markup",         JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szMarkup), sizeof(pData->szMarkup) },
                                                   { L"Context",        JSONFactory::UTF16_STRING_PARM_TYPE, &(pData->szContext), sizeof(pData->szContext)/sizeof(pData->szContext[0]) },
                                                   { L"DateTime",       JSONFactory::ASCII_STRING_PARM_TYPE, &(pData->szDateTime), sizeof(pData->szDateTime) },
                                                   { L"AddInfo",        JSONFactory::UTF16_STRING_PARM_TYPE, &(pData->szAddInfo), sizeof(pData->szAddInfo)/sizeof(pData->szAddInfo[0]) },
                                                   { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParms, parseControl );
  if ( iRC != 0 ) 
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( iRC, L"Error: Parsing of input parameters failed", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  if ( pData->szMemory[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szSource[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing proposal source text parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szTarget[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing proposal target text parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szIsoSourceLang[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing proposal source language parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szIsoTargetLang[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing proposal target language parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       
  if ( pData->szMarkup[0] == 0  )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing proposal markup parameter", strOutputParms );
    delete pData;
    return( iRC );
  } /* end */       

  // get the handle of the memory 
  long lHandle = 0;
  iRC = this->getMemoryHandle( pData->szMemory, &lHandle, pData->szError, sizeof(pData->szError)/sizeof(pData->szError[0]) );
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( iRC );
  } /* endif */

  // prepare the proposal data
  OtmProposal Proposal;
  Proposal.setSource( pData->szSource );
  Proposal.setTarget( pData->szTarget );
  Proposal.setSegmentNum( pData->lSegmentNum );
  Proposal.setDocName( pData->szDocName );
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();
  pLangFactory->getOpenTM2NameFromISO( pData->szIsoSourceLang, pData->szOtmSourceLang );
  pLangFactory->getOpenTM2NameFromISO( pData->szIsoTargetLang, pData->szOtmTargetLang );
  Proposal.setSourceLanguage( pData->szOtmSourceLang );
  Proposal.setTargetLanguage( pData->szOtmTargetLang );
  if ( stricmp( pData->szType, "GlobalMemory" ) == 0 )
  {
    Proposal.setType( OtmProposal::eptGlobalMemory );
  }
  else if ( stricmp( pData->szType, "GlobalMemoryStar" ) == 0 )
  {
    Proposal.setType( OtmProposal::eptGlobalMemoryStar );
  }
  else if ( stricmp( pData->szType, "MachineTranslation" ) == 0 )
  {
    Proposal.setType( OtmProposal::eptMachine );
  }
  else if ( stricmp( pData->szType, "Manual" ) == 0 )
  {
    Proposal.setType( OtmProposal::eptManual );
  } /* endif */
  Proposal.setAuthor( pData->szAuthor);
  Proposal.setMarkup( pData->szMarkup );
  Proposal.setContext( pData->szContext );
  LONG lTime = 0;
  convertTMXTimeToLong( pData->szDateTime, &lTime );
  Proposal.setUpdateTime( lTime );
  Proposal.setAddInfo( pData->szAddInfo );

  // update the memory
  iRC = EqfUpdateMem( this->hSession, lHandle, (void *)&Proposal, 0);
  if ( iRC == 0 )
  {
    pData->szError[0] = 0;
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof(pData->szError)/sizeof(pData->szError[0]) );
  } /* endif */
  buildErrorReturn( iRC, pData->szError, strOutputParms );

  delete pData;

  return( iRC );
}



  /*! \brief Verify OpenTM2 API session
    \returns 0 if successful or an error code in case of failures
  */
int OtmMemoryService::verifyAPISession
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
int OtmMemoryService::buildErrorReturn
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

/*! \brief find a memory in our list of active memories
  \param pszMemory name of the memory
  \returns index in the memory table if successful, -1 if memory is not contained in the list
*/
int OtmMemoryService::findMemoryInList( char *pszMemory )
{
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
int OtmMemoryService::getFreeSlot()
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
    OtmMemoryService::OPENEDMEMORY NewEntry;
    memset( &NewEntry, 0, sizeof(NewEntry) );
    this->vMemoryList.push_back( NewEntry );
    return( this->vMemoryList.size() - 1 );
  } /* endif */

  return( -1 );
}



/*! \brief close any memories which haven't been used for a long time
  \returns 0
*/
int OtmMemoryService::cleanupMemoryList()
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


/*! \brief get the handle for a memory, if the memory is not opened yet it will be opened
  \param pszMemory name of the memory
  \param plHandle buffer for the memory handle
  \param pszError buffer for an error message text in case of failures
  \returns 0 if successful or an error code in case of failures
*/
int OtmMemoryService::getMemoryHandle( char *pszMemory, PLONG plHandle, wchar_t *pszError, int iErrorBufSize )
{
  int iIndex = findMemoryInList( pszMemory );
  if ( iIndex != -1 )
  {
    *plHandle = this->vMemoryList[iIndex].lHandle;
    time( &(this->vMemoryList[iIndex].tLastAccessTime) );
    return( 0 );
  } /* endif */

  // cleanup the memory list (close memories not used for a longer time)
  cleanupMemoryList();

  // find a free slot in the memory list
  iIndex = getFreeSlot();

  // handle "too many open memories" condition
  if ( iIndex == -1 )
  {
    wcscpy( pszError, L"Error: too many open translation memory databases" );
    return( ERROR_TOO_MANY_OPEN_MEMORIES );
  } /* endif */

  // open the memory
  LONG lHandle = 0;
  int iRC = EqfOpenMem( this->hSession, pszMemory, &lHandle, 0 );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pszError, (unsigned short)iErrorBufSize );
    return( iRC );
  } /* endif */

  // add opened memory to the memory list
  this->vMemoryList[iIndex].lHandle = lHandle;
  strcpy( this->vMemoryList[iIndex].szName, pszMemory );
  time( &(this->vMemoryList[iIndex].tLastAccessTime) );
  *plHandle = lHandle;

  return( 0 );
}

/*! \brief convert a long time value into the TMX date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converterd date time
  \returns 0 
*/
int OtmMemoryService::convertTimeToTMX( LONG lTime, char *pszDateTime )
{
  struct tm   *pTimeDate;            // time/date structure

  if ( lTime != 0L ) lTime += 10800L;// correction: + 3 hours

  pTimeDate = gmtime( (time_t *)&lTime );
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

/*! \brief convert a TMX time value to a OpenTM2 long time value 
    \param pszDateTime buffer containing the TMX date and time
    \param plTime pointer to a long buffer receiving the converted time 
  \returns 0 
*/
int OtmMemoryService::convertTMXTimeToLong( char *pszDateTime, PLONG plTime )
{
  *plTime = 0;

  // we currently support only dates in the form YYYYMMDDThhmmssZ
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
BOOL OtmMemoryService::getValue( PSZ pszString, int iLen, int *piResult )
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
int OtmMemoryService::removeFromMemoryList( int iIndex )
{
  LONG lHandle = this->vMemoryList[iIndex].lHandle;

  // remove the memory from the list
  this->vMemoryList[iIndex].lHandle = 0;
  this->vMemoryList[iIndex].tLastAccessTime = 0;
  this->vMemoryList[iIndex].szName[0] = 0;

  // close the memory
  EqfCloseMem( this->hSession, lHandle, 0 );

  return( 0 );
} 
