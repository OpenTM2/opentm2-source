/*! \file
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMPROPOSAL_H_
#define _OTMPROPOSAL_H_

#include <string>
#include <vector>

/*! \brief Data class for the transport of memory proposals
 *
 * 
 */
class __declspec(dllexport) OtmProposal
{

public:

/*! \brief Constructors */
	OtmProposal();

/*! \brief Destructor */
	~OtmProposal();

  /*! \enum eProposalType
	Proposal types.
*/
	enum eProposalType
	{
		eptUndefined,			/*!< proposal type has not been set or is not available */
		eptManual,	  		/*!< proposal was produced by manual translation */
		eptMachine,	    	/*!< proposal was produced by automatic (machine) translation */
		eptGlobalMemory, 	/*!< proposal is from global memory  */
    eptGlobalMemoryStar /*!< proposal is from global memory, to be marked with an asterisk  */
	};

  /*! \enum eMatchType
	Match types.
*/
	enum eMatchType
	{
		emtUndefined,			/*!< match type has not been set or is not available */
		emtExact,	    		/*!< proposal is an exact match */
		emtExactExact,		/*!< proposal is an exact match, document name and segment number are exact also */
		emtExactContext,	/*!< proposal is an exact match from the same document */
		emtFuzzy,	    		/*!< proposal is a fuzzy match */
		emtReplace 	    	/*!< proposal is a replace match */
	};

  /* operations */

  /* \brief clear all proposal fields 
   */
  void clear();

  /* setters and getters */

   /* \brief get the key of the proposal
     The internal key is the memory internal identifier of the proposal
     \param pszBuffer Pointer to buffer receiving the proposal key
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getInternalKey( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal key
     \param pszBuffer Pointer to buffer containing the proposal key
   */
  void setInternalKey( char *pszBuffer );
  	

  /* \brief get length of proposal source text 
  	 \returns Number of characters in proposal source text
   */
  int getSourceLen();

  /* \brief get proposal source text 
     \param pszBuffer Pointer to buffer receiving the proposal source text
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getSource( wchar_t *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal source text
     \param pszBuffer Pointer to buffer containing the proposal source text
   */
  void setSource( wchar_t *pszBuffer );

  /* \brief get length of proposal target text 
  	 \returns Number of characters in proposal target text
   */
  int getTargetLen();

  /* \brief get proposal target text   
     \param pszBuffer Pointer to buffer receiving the proposal target text
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getTarget( wchar_t *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal target text
     \param pszBuffer Pointer to buffer containing the proposal target text
   */
  void setTarget( wchar_t *pszBuffer );
  	
  /* \brief get proposal ID
     \param pszBuffer Pointer to buffer receiving the proposal ID
     \param iBufSize Size of the buffer in number of characters
     \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getID( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal ID 
     \param pszBuffer Pointer to buffer containing the proposal ID
   */
  void setID( char *pszBuffer );
  	

  /* \brief get proposal document name

    \param pszBuffer Pointer to buffer receiving the document name
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getDocName( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal document short name
     \param pszBuffer Pointer to buffer containing the document name
   */
  void setDocName( char *pszBuffer );
  	
  /* \brief get proposal document short name
     \param pszBuffer Pointer to buffer receiving the document short name
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getDocShortName( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal document short name
     \param pszBuffer Pointer to buffer containing the document short short name
   */
  void setDocShortName( char *pszBuffer );

  /* \brief get proposal segment number
   	\returns proposal segment number
   */
  long getSegmentNum();
  	
  /* \brief set the proposal segment number
     \param lSegmentNum new segment number of proposal
   */
  void setSegmentNum( long lSegmentNum );
  	
  /* \brief get proposal source language
     \param pszBuffer Pointer to buffer receiving the proposal source language
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getSourceLanguage( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal source language
     \param pszBuffer Pointer to buffer containing the proposal source language
   */
  void setSourceLanguage( char *pszBuffer );

  /* \brief get proposal target language
     \param pszBuffer Pointer to buffer receiving the proposal target language
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getTargetLanguage( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal target language
     \param pszBuffer Pointer to buffer containing the proposal target language
   */
  void setTargetLanguage( char *pszBuffer );

  /* \brief get proposal type
     \returns proposal type
   */
  eProposalType getType();
  	
  /* \brief set the proposal type
     \param eType new type of the proposal
   */
  void setType( eProposalType eType );

  /* \brief get match type
     \returns proposal type
   */
  eMatchType getMatchType();
  	
  /* \brief set the match type
     \param eType new type of the proposal
   */
  void setMatchType( eMatchType eType );

  /* \brief get name of proposal author
     \param pszBuffer Pointer to buffer receiving the name of the proposal author
     \param iBufSize Size of the buffer in number of characters
     \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getAuthor( char *pszBuffer, int iBufSize );
  	
  /* \brief set the name of the proposal author
     \param pszBuffer Pointer to buffer containing the name of the proposal author
   */
  void setAuthor( char *pszBuffer );

  /* \brief get proposal time stamp
     \returns proposal segment number
   */
  long getUpdateTime();
  	
  /* \brief set the proposal time stamp
     \param lTime new time stamp of proposal
   */
  void setUpdateTime( long lSegmentNum );

  /* \brief get proposal fuzziness
     \returns proposal fuzziness
   */
  int getFuzziness();
  	
  /* \brief set the proposal fuzziness
     \param lFuzzinessTime new fuzziness of proposal
   */
  void setFuzziness( long iFuzziness );

  /* \brief get markup table name (format)
     \param pszBuffer Pointer to buffer receiving the name of the markup table name
     \param iBufSize Size of the buffer in number of characters
     \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getMarkup( char *pszBuffer, int iBufSize );
  	
  /* \brief set markup table name (format)
     \param pszBuffer Pointer to buffer containing the markup table name
   */
  void setMarkup( char *pszBuffer );

  /* \brief get length of proposal context
  	 \returns Number of characters in proposal context
   */
  int getContextLen();

  /* \brief get proposal Context 
     \param pszBuffer Pointer to buffer receiving the proposal context
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getContext( wchar_t *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal context
     \param pszBuffer Pointer to buffer containing the proposal context
   */
  void setContext( wchar_t *pszBuffer );

  /* \brief get proposal context ranking
  	\returns proposal context ranking
   */
  int getContextRanking();
  	
  /* \brief set the proposal context ranking
     \param iContextRanking context ranking for the proposal (0..100)
   */
  void setContextRanking( int iContextRanking );

  /* \brief get length of proposal AddInfo text 
  	 \returns Number of characters in proposal AddInfo text
   */
  int getAddInfoLen();

  /* \brief get additional info
     \param pszBuffer Pointer to buffer receiving the additional info
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getAddInfo( wchar_t *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal additional information
     \param pszBuffer Pointer to buffer containing the additional info
   */
  void setAddInfo( wchar_t *pszBuffer );

  /* \brief set the proposal memory index
     \param iIndex new value for the memory index
  */
  void setMemoryIndex( int iIndex );

  /* \brief get memory index
     \returns memory index of this proposal
  */
  int getMemoryIndex();

  /* \brief set the DITA replacement list
     \param pList new value for the DITA replacement list
  */
  void setReplacementList( long pList );

  /* \brief get DITA replacement list
     \returns pointer to DITA replacement list
  */
  long getReplacementList();

  /* \brief check if proposal match type is one of the exact match types
   */
  bool isExactMatch();

  /* \brief check if proposal is empty (i.e. has not been used)
   */
  bool isEmpty();

  /* \brief check if source and target of proposal is equal
   */
  bool isSourceAndTargetEqual();

  /* \brief check if target strings are identical
     \param otherProposal pointer to second proposal 
  */
  bool isSameTarget( OtmProposal *otherProposal );

  /*! \brief Clear the data of proposals stored in a vector
      \param Proposals reference to a vector containing the proposals
  */
  void static clearAllProposals(
    std::vector<OtmProposal *> &Proposals
  );

  /*! \brief Get the number of filled proposals in a proposal list
      \param Proposals reference to a vector containing the proposals
  */
  int static getNumOfProposals(
    std::vector<OtmProposal *> &Proposals
  );


  /* \brief assignment operator to copy the datafields from one
    OtmProposal to another one
     \param copyme reference to OtmProposal being copied
   */
  OtmProposal &operator=( const OtmProposal &copyme );


private:

  /* \brief private proposal data */
	void *pvProposalData; 
};


#endif // #ifndef _OTMPROPOSAL_H_
 