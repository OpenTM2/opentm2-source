/*! \file
	Description: Defines a list of OtmProofReadEntries class 

	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#include <vector>
#include "OtmProofReadEntry.h"

#ifndef _OtmProofReadList_H_
#define _OtmProofReadList_H_

class OtmProofReadList
/*! \brief This class implements a proof read entry list
*/

{
public:
/*! \brief Constructor
*/
	OtmProofReadList();

/*! \brief Destructor
*/
	~OtmProofReadList();

/*! \brief Gets the number of entries in the list
  \returns number of entries in the list
*/
	int size()
  {
    return( m_vList.size() );
  }

/*! \brief Gets the number of folders in the folder list
  \returns number of folders in the list
*/
	int getNumOfFolders()
  {
    return( m_vFolderList.size() );
  }

/*! \brief Gets the number of documents in the document list
  \returns number of documents in the list
*/
	int getNumOfDocuments()
  {
    return( m_vDocumentList.size() );
  }

/*! \brief Access entry at the given position
  \param index index of the requested entry
  \returns entry at given poistopm
*/
  (OtmProofReadEntry *)& operator[] (int index) 
  {
    return m_vList[index];
  }

/*! \brief Add a new entry
  \param pEntry pointer to the new OtmReadEntry
*/
  void addEntry( OtmProofReadEntry *pEntry ) 
  {
    m_vList.push_back( pEntry );
  }

/*! \brief Get folder entry at the given position
  \param index index of the requested entry
  \returns entry at given position
*/
  std::string& getFolder( int index) 
  {
    return m_vFolderList[index];
  }


  /*! \brief Get document entry at the given position
  \param index index of the requested entry
  \returns document name as std::string
*/
  std::string& getDocument( int index) 
  {
    return m_vDocumentList[index];
  }

  /*! \brief Get document markup for the specified document 
  \param index index of the document
  \returns markup name as std::string
*/
  std::string& getDocMarkup( int index) 
  {
    return m_vDocmarkupList[index];
  }

  /*! \brief Get the target language for the specified document 
  \param index index of the document
  \returns target language as std::string
*/
  std::string& getDocTargetLang( int index) 
  {
    return m_vDocTargetLangList[index];
  }

/*! \brief Set list creation date
  \param pszCreationDate date as string
*/
  void setCreationDate( char *pszCreationDate ) 
  {
    m_strCreationDate = pszCreationDate;
  }

/*! \brief Set list proof read date
  \param pszProofReadDate date as string
*/
  void setProofReadDate( char *pszProofReadDate ) 
  {
    m_strProofReadDate = pszProofReadDate;
  }

/*! \brief Set translator name
  \param pszTranslator name of translator
*/
  void setTranslator( char *pszTranslator ) 
  {
    m_strTranslator = pszTranslator;
  }

/*! \brief Set name of proof reader
  \param pszProofReader name of proof reader
*/
  void setProofReader( char *pszProofReader ) 
  {
    m_strProofReader = pszProofReader;
  }

/*! \brief Add a document to our document list
  \param strName name of the document
  \param strSourceLang source language of the document
  \param strTargetLang target language of the document
  \param strMarkup markup of the document
  \returns index of the added document
*/
  int  addDocument( std::string strName, std::string strSourceLang, std::string strTargetLang, std::string strMarkup );

/*! \brief Add a folder to our folder list
  \param strName name of the folder
  \returns index of the added folder
*/
  int  addFolder( std::string strName );


/*! \brief Loads the proof read document entry list from the specified file
  \param pszInFile fully qualified name of the XML file containing the proof read entries being loaded
  \param hwndError handle of parent window for error messages
  \returns 0 if successful or an error code
*/
	int load( const char *pszInFile, HWND hwndError );

/*! \brief Resets the list content
*/
	void clear();

/*! \brief Saves the proof read document entry list to the specified file
  \param pszOutFile fully qualified name of the XML file receiving the proof read entries
  \param hwndError handle of parent window for error messages
  \returns 0 if successful, an error code in case of errors
*/
	int save( char *pszOutFile, HWND hwndEror );

/*! \brief get the number of selected entries
  \returns number of selected entries
*/
	int getNumOfSelected();

 
private:
	std::vector<OtmProofReadEntry *>m_vList;
	std::vector<std::string>m_vDocumentList;
	std::vector<std::string>m_vFolderList;
  std::string m_strCreationDate;
  std::string m_strProofReadDate;
  std::string m_strTranslator;
  std::string m_strProofReader;
  std::vector<std::string> m_vDocSourceLangList;
  std::vector<std::string> m_vDocTargetLangList;
  std::vector<std::string> m_vDocmarkupList;


};


#endif