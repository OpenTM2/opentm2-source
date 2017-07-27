/*! \file
	Description: Defines the OtmProofReadEntry class 

	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#include <vector>

#ifndef _OtmProofReadEntry_H_
#define _OtmProofReadEntry_H_

class OtmProofReadEntry
/*! \brief This class implements a proof table read entry
*/

{
public:
/*! \brief Constructor
*/
  OtmProofReadEntry() { m_pvTargetChangeList = NULL; m_fSelected = FALSE; m_fProcessed = FALSE; m_iDocNumber = 0; m_iFolNumber = 0; m_pvModTargetChangeList = NULL; };

/*! \brief Destructor
*/
  ~OtmProofReadEntry() {};

/*! \brief Gets the source string from a proof read entry
  \returns pointer to source of proof read entry
*/
  const wchar_t *getSource() { return( m_strSource.c_str() ); };

/*! \brief Sets the source string of a proof read entry
  \param pszSource pointer to the source string
*/
  void setSource( const wchar_t *pszSource ) { m_strSource = pszSource; };

/*! \brief Gets the target string from a proof read entry
  \returns pointer to target of proof read entry or NULL in case of errors
*/
	const wchar_t *getTarget(){ return( m_strTarget.c_str() ); };

/*! \brief Sets the target string of a proof read entry
  \param pszTarget pointer to the target string
*/
	void setTarget( const wchar_t *pszTarget ) { m_strTarget = pszTarget; };

/*! \brief Gets the modified target string from a proof read entry
  \returns pointer to modified target of proof read entry or NULL in case of errors
*/
	const wchar_t *getModTarget(){ return( m_strModTarget.c_str() ); };

/*! \brief Sets the modified target string of a proof read entry
  \param pszModTarget pointer to the modified target string
*/
	void setModTarget( const wchar_t *pszModTarget ) { m_strModTarget = pszModTarget; };

/*! \brief Gets the new target string from a proof read entry
  \returns pointer to new target of proof read entry or NULL in case of errors
*/
	const wchar_t *getNewTarget(){ return( m_strNewTarget.c_str() ); };

/*! \brief Sets the new target string of a proof read entry
  \param pszTarget pointer to the new target string
*/
	void setNewTarget( const wchar_t *pszNewTarget ) { m_strNewTarget = pszNewTarget; };

/*! \brief Gets the comment of a proof read entry
  \returns pointer to proof read entry comment
*/
	const wchar_t *getComment(){ return( m_strComment.c_str() ); };

/*! \brief Sets comment of a proof read entry
  \param pszComment pointer to the new target string
*/
	void setComment( const wchar_t *pszComment ) { m_strComment = pszComment; };

/*! \brief Gets the segment number from a proof read entry
  \returns segment number 
*/
	unsigned long getSegmentNumber(){ return( m_ulSegmentNumber ); };

/*! \brief Sets the segment number  of a proof read entry
  \param ulSegmentNumber segment number 
*/
	void setSegmentNumber( unsigned long ulSegmentNumber ) { m_ulSegmentNumber = ulSegmentNumber; };

/*! \brief Gets the selected flag from a proof read entry
  \returns selected flag
*/
	BOOL getSelected(){ return( m_fSelected ); };

/*! \brief Sets the selected flag of a proof read entry
  \param fSelected selected flag
*/
	void setSelected( BOOL fSelected ) { m_fSelected = fSelected; };

/*! \brief Gets the processed  flag from a proof read entry
  \returns processed  flag
*/
	BOOL getProcessed(){ return( m_fProcessed ); };

/*! \brief Sets the processed flag of a proof read entry
  \param fProcessed processed  flag
*/
	void setProcessed( BOOL fProcessed ) { m_fProcessed = fProcessed; };

/*! \brief Gets the document number of a proof read entry
  \returns document number
*/
	int getDocumentNumber(){ return( m_iDocNumber ); };

/*! \brief Sets the document number of a proof read entry
  \param iDocNum document number 
*/
	void setDocumentNumber( int iDocNum ) { m_iDocNumber = iDocNum; };

/*! \brief Gets the folder number of a proof read entry
  \returns folder number
*/
	int getFolderNumber(){ return( m_iFolNumber ); };

/*! \brief Sets the folder number of a proof read entry
  \param iFolNum document number 
*/
	void setFolderNumber( int iFolNum ) { m_iFolNumber = iFolNum; };
  
  /*! \brief Gets the target change list of a proof read entry
  \returns pointer to target change list
*/
  PVOID getTargetChangeList(){ return( m_pvTargetChangeList ); };

/*! \brief Sets the target change list of a proof read entry
  \param pvChangeList pointer to target change list
*/
	void setTargetChangeList( PVOID pvChangeList ) { m_pvTargetChangeList = pvChangeList; };

  /*! \brief Gets the modified target change list of a proof read entry
  \returns pointer to modified target change list
*/
  PVOID getModTargetChangeList(){ return( m_pvModTargetChangeList ); };

/*! \brief Sets the mdoified target change list of a proof read entry
  \param pvChangeList pointer to the modified target change list
*/
	void setModTargetChangeList( PVOID pvChangeList ) { m_pvModTargetChangeList = pvChangeList; };


  /*! \brief Checks if a proof read entry is an unchanged (by the proof reader) entry
  \returns TRUE when entry is unchanged
*/
  BOOL isUnChanged(){ return( m_strModTarget.empty() || (m_strModTarget.compare( m_strTarget ) == 0) ); };

 
private:
	std::wstring m_strSource;
	std::wstring m_strTarget;
	std::wstring m_strModTarget;
	std::wstring m_strNewTarget;
  std::wstring m_strComment;
  unsigned long m_ulSegmentNumber;
  BOOL m_fSelected;
  BOOL m_fProcessed;
  int m_iDocNumber;
  int m_iFolNumber;
  PVOID m_pvTargetChangeList;
  PVOID m_pvModTargetChangeList;
};


#endif