/*! \brief LoadBalancerList.CPP
	Copyright (c) 2018, International Business Machines Corporation and others. All rights reserved.
	Description: Load Balancer List related functions
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_EDITORAPI        // editor API (required for EQFTPI.H)

#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF

#include "OtmProposal.h"
#include "AnalysisMemLookupData.h"
#include "core\memory\MemoryFactory.h"

  #include "eqftai.h"
  #include "eqftpi.h"
  #include "ValDocExp.h"
  #include "eqfserno.h"
  #include <time.h>
#pragma pack( pop, TM2StructPacking )


#include "cxmlwriter.h"

//
// load balance list creation
//

typedef struct _TABALANCELISTDATA
{
  CHAR        szFolObjName[MAX_EQF_PATH];        // folder object name
  CHAR        szFolPropName[MAX_EQF_PATH];       // folder properties name
  CHAR        szFolderLongName[MAX_LONGFILESPEC];// folder long name
  CHAR        szFolderShortName[MAX_FILESPEC];    // folder short name
  CHAR        szSubFolObjName[MAX_EQF_PATH];     // folder object name
  CHAR        szMarkup[MAX_FILESPEC];            // markup of folder
  CHAR        szMemory[MAX_LONGFILESPEC];        // folder's memory db
  CHAR        szSourceLang[60];                  // folder source language
  CHAR        szTargetLang[60];                  // folder target language
  CHAR        szBalanceFile[MAX_LONGFILESPEC];   // fully qualified balance file name
  int         iNumOfDocs;                        // number of documents in the folder
  ULONG       ulTotalWords;                      // total number of words in documents
  ULONG       ulTotalUntranslated;               // total number of untranslated words
  PVOID       pvXW;                              // active XmlWriter object
  PSZ         pszDocList;                        // ptr to list of documents
  CHAR        szBuffer[4096];                    // general use working area
} TABALANCELISTDATA, *PTABALANCELISTDATA;


// write folder info part to balance file
int TABalanceWriteFolderInfo
(
  PTABALANCELISTDATA pData
)
{
  int iRC = 0;
  PPROPFOLDER pProp = NULL;
  CXmlWriter *xw = (CXmlWriter *)pData->pvXW;

  // setup property file name
  UtlMakeEQFPath( pData->szFolPropName, NULC, PROPERTY_PATH, NULL );
  strcat( pData->szFolPropName, BACKSLASH_STR );
  strcat( pData->szFolPropName, UtlGetFnameFromPath( pData->szFolObjName ) );

  // load properties into memory
  {
    ULONG ulPropLen = 0;

    // load document property file
    if ( !UtlLoadFileL( pData->szFolPropName, (PVOID *)&pProp, &ulPropLen, FALSE, FALSE ) )
    {
      iRC = ERROR_PROPERTY_ACCESS;
    }
  }

  // write folder info
  if ( pProp )
  {
    xw->WriteAttributeString( "Type", "Original" );

    xw->WriteElementString( "Name", pData->szFolderLongName );

    sprintf( pData->szBuffer, "%ld", pData->iNumOfDocs );
    xw->WriteElementString( "TotalDocuments", pData->szBuffer );

    sprintf( pData->szBuffer, "%lu", pData->ulTotalWords );
    xw->WriteElementString( "TotalWords", pData->szBuffer );

    sprintf( pData->szBuffer, "%lu", pData->ulTotalUntranslated );
    xw->WriteElementString( "UntranslatedWords", pData->szBuffer );

    xw->WriteElementString( "Markup", pData->szMarkup );

    xw->WriteElementString( "Memory", pData->szMemory );

    sprintf( pData->szBuffer, "%c", pProp->chDrive );
    xw->WriteElementString( "Drive", pData->szBuffer );

    xw->WriteElementString( "Editor", pProp->szEditor );

    strcpy( pData->szSourceLang, pProp->szSourceLang );
    xw->WriteElementString( "SourceLanguage", pProp->szSourceLang );

    strcpy( pData->szTargetLang, pProp->szTargetLang );
    xw->WriteElementString( "TargetLanguage", pProp->szTargetLang );

    if ( pProp->aLongDicTbl[0][0] != EOS )
    {
      xw->WriteStartElement( "DictionaryList" );

      int i = 0;
      while ( (i < MAX_NUM_OF_FOLDER_DICS) && (pProp->aLongDicTbl[i][0] != EOS) )
      {
        xw->WriteElementString( "Name", pProp->aLongDicTbl[i] );
        i++;
      }     /*end while */
      xw->WriteEndElement();   // DictionaryList
    }
    else if (pProp->DicTbl[0] != EOS)
    {
      PSZ pszTemp = pProp->DicTbl;
      PSZ pszEnd = NULL;

      xw->WriteStartElement( "DictionaryList" );

      do
      {
        pszEnd = strchr( pszTemp, X15 );
        if ( pszEnd ) *pszEnd = EOS;
        xw->WriteElementString( "Name", pszTemp );
        if ( pszEnd ) *pszEnd = X15;
        if ( pszEnd ) pszTemp = pszEnd + 1;
      } while ( pszEnd );

      xw->WriteEndElement();   // DictionaryList
    } /* endif */


    if ( pProp->aLongMemTbl[0][0] != EOS )
    {
      int i = 0;
      xw->WriteStartElement( "SearchMemoryList" );
      while ( (i < MAX_NUM_OF_READONLY_MDB) && (pProp->aLongMemTbl[i][0] != EOS) )
      {
        xw->WriteElementString( "Name", pProp->aLongMemTbl[i] );
        i++;
      } /* endwhile */
      xw->WriteEndElement();                  // SearchMemoryList
    }
    else if ( pProp->MemTbl[0] != EOS )
    {
      PSZ pszTemp = pProp->MemTbl;
      PSZ pszEnd = NULL;

      xw->WriteStartElement( "SearchMemoryList" );

      do
      {
        pszEnd = strchr( pszTemp, X15 );
        if ( pszEnd ) *pszEnd = EOS;
        xw->WriteElementString( "Name", pszTemp );
        if ( pszEnd ) *pszEnd = X15;
        if ( pszEnd ) pszTemp = pszEnd + 1;
      } while ( pszEnd );
      xw->WriteEndElement();                  // SearchMemoryList
    } /* endif */
  } /* endif */

  // cleanup
  if ( pProp )  UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

  return( iRC );
} /* end of function TABalanceWriteFolderInfo */


// write document info part to balance file
int TABalanceWriteDocInfo
(
  PTABALANCELISTDATA pData,                      // our data area
  PPROPDOCUMENT      pProp                       // document properties
)
{
  int iRC = 0;
  CXmlWriter *xw = (CXmlWriter *)pData->pvXW;

  xw->WriteStartElement( "Document" );

  // write document path, name and extension
  {
    PSZ pszName = ( pProp->szLongName[0] ) ? pProp->szLongName : pProp->PropHead.szName;
    PSZ pszPath = NULL;
    strcpy( pData->szBuffer, pszName );
    pszName = pData->szBuffer;

    xw->WriteElementString( "FullName", pszName );

    PSZ pszNamePos = strrchr( pszName,  BACKSLASH );
    if ( pszNamePos )
    {
      pszPath = pszName;
      pszName = pszNamePos + 1;
      *pszNamePos = EOS;
    } /* endif */
    PSZ pszExtension = strrchr( pszName, DOT );

    if ( pszPath ) xw->WriteElementString( "Path", pszPath );
    xw->WriteElementString( "Name", pszName );
    if ( pszExtension ) xw->WriteElementString( "Extension", pszExtension + 1 );
  }

  // show any subfolder name for document
  if ( pProp->ulParentFolder )
  {
    SubFolIdToObjectName( pProp->PropHead.szPath, pProp->ulParentFolder, pData->szSubFolObjName );
    SubFolObjectNameToName( pData->szSubFolObjName, pData->szBuffer );
    xw->WriteElementString( "SubFolder", pData->szBuffer );
  } /* endif */

  sprintf( pData->szBuffer, "%lu", pProp->ulTotal );
  xw->WriteElementString( "TotalWords", pData->szBuffer );

  sprintf( pData->szBuffer, "%lu", pProp->ulNotTranslated );
  xw->WriteElementString( "UntranslatedWords", pData->szBuffer );

  sprintf( pData->szBuffer, "%u", pProp->usComplete );
  xw->WriteElementString( "CompletionRate", pData->szBuffer );

  if ( pProp->szVendor[0] ) xw->WriteElementString( "Translator", pProp->szVendor );

  xw->WriteElementString( "Markup", pProp->szFormat[0] ? pProp->szFormat : pData->szMarkup );

  // document size
  {
    char        szBuffer[MAX_EQF_PATH];
    FILEFINDBUF stFile;
    USHORT      usCount = 1;
    HDIR        hSearch = HDIR_CREATE;

    strcpy( szBuffer, pProp->PropHead.szPath );
    strcat( szBuffer, BACKSLASH_STR );
    UtlQueryString( QST_SOURCEDIR, szBuffer + strlen(szBuffer), MAX_FILESPEC );
    strcat( szBuffer, BACKSLASH_STR );
    strcat( szBuffer, pProp->PropHead.szName );

    memset( &stFile, 0, sizeof(stFile) );
    UtlFindFirst( szBuffer, &hSearch, 0, &stFile, sizeof(stFile), &usCount, 0L, FALSE );
    UtlFindClose( hSearch, FALSE );

    sprintf( pData->szBuffer, "%ld", stFile.nFileSizeLow );
    xw->WriteElementString( "Size", pData->szBuffer );
  }

  xw->WriteEndElement();                         // Document

  return( iRC );
} /* end of function TABalanceWriteDocInfo */



// main function for balance list creation
int TABalanceList
(
  PSZ         pszFolObjName,                     // folder object name
  PSZ         pszDocs,                           // ptr to list of documents
  USHORT      usFiles                            // number of source files
)
{
  int         iRC = 0;                           // function return code
  PTABALANCELISTDATA pData = NULL;               // ptr to our data area
  PSZ         pDocNameBuffer = NULL;             // document name buffer
  CXmlWriter *xw = NULL;                         // ptr to XmlWriter object

  // check input parameter
  if ( pszFolObjName == NULL )
  {
    iRC = ERROR_INVALID_DATA;
  } /* endif */

  // allocated data area
  if ( !iRC )
  {
    if ( !UtlAlloc( (PVOID *)&pData, 0L, sizeof(TABALANCELISTDATA ), NOMSG ) )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // get folder data, setup file names
  if ( !iRC )
  {
    pData->iNumOfDocs = (int)usFiles;
    pData->pszDocList = pszDocs;

    strcpy( pData->szFolObjName, pszFolObjName );
    SubFolObjectNameToName( pData->szFolObjName, pData->szFolderLongName );
    strcpy( pData->szFolderShortName, UtlGetFnameFromPath( pData->szFolObjName ) );

    UtlMakeEQFPath( pData->szBalanceFile, NULC, SYSTEM_PATH, NULL );
    strcat( pData->szBalanceFile, "\\LOADBALANCE\\" );
    strcat( pData->szBalanceFile, pData->szFolderLongName );
    strcat( pData->szBalanceFile, ".XML" );

    FolQueryInfo2( pData->szFolObjName, pData->szMemory, pData->szMarkup, NULL, NULL, NULL, FALSE );
  } /* endif */

  // start XML output
  if ( !iRC )
  {
    xw = new CXmlWriter( pData->szBalanceFile );
    xw->Formatting = CXmlWriter::Indented;
    xw->Encoding = CXmlWriter::UTF8;
    xw->Indention = 2;

    xw->WriteStartDocument();

    //xw->WriteStylesheet( "matchlist.xsl" );
    xw->WriteStartElement( "LoadBalance" );

    pData->pvXW = (PVOID)xw;
  } /* endif */

  // writer header info
  if ( !iRC )
  {
    LONG lTime = 0;

    xw->WriteStartElement( "Header" );

    xw->WriteElementString( "Generator", "OpenTM2" );

    sprintf( pData->szBuffer, "%d.%d.%d", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE );
    xw->WriteElementString( "GeneratorVersion", pData->szBuffer );

    time( &lTime );
    UtlLongToDateString( lTime, pData->szBuffer, sizeof(pData->szBuffer) );
    xw->WriteElementString( "GeneratedDate", pData->szBuffer );

    UtlLongToTimeString( lTime, pData->szBuffer, sizeof(pData->szBuffer) );
    xw->WriteElementString( "GeneratedTime", pData->szBuffer );

    xw->WriteEndElement();                       // Header

  } /* endif */

  // loop documents to compute total number of untranslated words
  if ( !iRC )
  {
    PSZ pszCurDoc = pData->pszDocList;
    int iDocItem = 0;
    while ( !iRC && (iDocItem < pData->iNumOfDocs) )
    {
      CHAR szDocProp[MAX_EQF_PATH];
      PPROPDOCUMENT pDocProp = NULL;
      ULONG ulPropLen = 0L;                    // size of loaded doc property file

      // setup document property file name
      UtlMakeEQFPath( szDocProp, pData->szFolObjName[0], PROPERTY_PATH, pData->szFolderShortName );
      strcat( szDocProp, BACKSLASH_STR );
      strcat( szDocProp, pszCurDoc );

      // load document property file
      if ( UtlLoadFileL( szDocProp, (PVOID *)&pDocProp, &ulPropLen, FALSE, FALSE ) )
      {
        pData->ulTotalWords        += pDocProp->ulTotal;
        pData->ulTotalUntranslated += pDocProp->ulNotTranslated;
        UtlAlloc( (PVOID *)&pDocProp, 0L, 0L, NOMSG );
      }
      else
      {
        // TODO: error handling: load of document property file failed
      } /* endif */

      // next document
      pszCurDoc += strlen(pszCurDoc) + 1;
      iDocItem++;
    } /*endwhile */
  } /* endif */

  // writer folder info
  if ( !iRC )
  {
    xw->WriteStartElement( "Folder" );
    iRC = TABalanceWriteFolderInfo( pData );
  } /* endif */

  // write document info(s)
  if ( !iRC )
  {
    PSZ pszCurDoc = pData->pszDocList;
    int iDocItem = 0;
    xw->WriteStartElement( "DocumentList" );
    while ( !iRC && (iDocItem < pData->iNumOfDocs) )
    {
      CHAR szDocProp[MAX_EQF_PATH];
      PPROPDOCUMENT pDocProp = NULL;
      ULONG ulPropLen = 0L;                    // size of loaded doc property file

      // setup document property file name
      UtlMakeEQFPath( szDocProp, pData->szFolObjName[0], PROPERTY_PATH, pData->szFolderShortName );
      strcat( szDocProp, BACKSLASH_STR );
      strcat( szDocProp, pszCurDoc );

      // load document property file
      if ( UtlLoadFileL( szDocProp, (PVOID *)&pDocProp, &ulPropLen, FALSE, FALSE ) )
      {
        iRC = TABalanceWriteDocInfo( pData, pDocProp );
        UtlAlloc( (PVOID *)&pDocProp, 0L, 0L, NOMSG );
      }
      else
      {
        // TODO: error handling: load of document property file failed
      } /* endif */

      // next document
      pszCurDoc += strlen(pszCurDoc) + 1;
      iDocItem++;
    } /*endwhile */
    xw->WriteEndElement();                         // DocumentList
    xw->WriteEndElement();                         // Folder
  } /* endif */

  // terminate XML output
  if ( xw != NULL )
  {
    xw->WriteEndElement(); // LoadBalance
    xw->WriteEndDocument();
    xw->Close();
    delete xw;
  } /* endif */

  // cleanup
  if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
  if ( pDocNameBuffer ) UtlAlloc( (PVOID *)&pDocNameBuffer, 0L, 0L, NOMSG );

  return( 0 );
} /* end of function TABalanceList */
