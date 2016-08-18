/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#include "..\PluginManager\PluginManager.h"
#include "OtmProposal.h"

/*! \brief Data class for the transport of memory proposals
 *
 * 
 */

/*! \brief maximum size of segment data */
#define OTMPROPOSAL_MAXSEGLEN 2048

/*! \brief maximum size of names */
#define OTMPROPOSAL_MAXNAMELEN 256

/*! \brief private proposal data area */
typedef struct _OTMPROPOSALDATA
{
  /*! \brief internal key of this proposal */
	char szInternalKey[OTMPROPOSAL_MAXNAMELEN];

  /*! \brief ID of this proposal */
	char szId[OTMPROPOSAL_MAXNAMELEN];

  /*! \brief Source string of memory proposal  (UTF-16) */
  wchar_t szSource[OTMPROPOSAL_MAXSEGLEN+1];
	//std::wstring strSource;

	/*! \brief Target string of memory proposal (UTF-16). */
	//std::wstring strTarget;
  wchar_t szTarget[OTMPROPOSAL_MAXSEGLEN+1];

	/*! \brief Name of document from which the proposal comes from. */
	//std::string strDocName;
	char szDocName[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief Short (8.3) name of the document from which the proposal comes from. */
	//std::string strDocShortName;
	char szDocShortName[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief Segment number within the document from which the proposal comes from. */
  long lSegmentNum;                  

	/*! \brief source language. */
  //std::string strSourceLanguage;
	char szSourceLanguage[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief target language. */
  //std::string strTargetLanguage;
  char szTargetLanguage[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief origin or type of the proposal. */
  OtmProposal::eProposalType eType;

	/*! \brief match type of the proposal. */
  OtmProposal::eMatchType eMatch;

	/*! \brief Author of the proposal. */
  // std::string strTargetAuthor;   
  char szTargetAuthor[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief Update time stamp of the proposal. */
  long    lTargetTime;

	/*! \brief Fuzziness of the proposal. */
  int iFuzziness;                 

	/*! \brief Markup table (format) of the proposal. */
  //std::string strMarkup;
  char szMarkup[OTMPROPOSAL_MAXNAMELEN];

  /*! \brief Context information of the proposal */
  //std::wstring strContext;  
  wchar_t szContext[OTMPROPOSAL_MAXSEGLEN+1];

  /*! \brief Additional information of the proposal */
  //std::wstring strAddInfo; 
  wchar_t szAddInfo[OTMPROPOSAL_MAXSEGLEN+1];

  /*! \brief Proposal data has been filled flag */
  bool fFilled; 

  /*! \brief Index of memory when looking up in a list of memories */
  int iMemoryIndex; 

  /*! \brief ranking of the context information (0..100) */
  int iContextRanking; 

  /*! \brief list of replacement values */
  long pvReplacementList;

} OTMPROPOSALDATA, *POTMPROPOSALDATA;

/*! \brief Prototypes of helper functions */
//int CopyToBufferW( const std::wstring &strSource, wchar_t *pszBuffer, int iBufSize );
//int CopyToBuffer( const std::string &strSource, char *pszBuffer, int iBufSize );
int CopyToBufferW( wchar_t *pszSource, wchar_t *pszBuffer, int iBufSize );
int CopyToBuffer( char *pszSource, char *pszBuffer, int iBufSize );

/*! \brief Constructors */
OtmProposal::OtmProposal() 
{
  POTMPROPOSALDATA pData = new OTMPROPOSALDATA;
  this->pvProposalData = (void *)pData;
  this->clear();
  pData->fFilled = 0;
};

/*! \brief Destructor */
OtmProposal::~OtmProposal() 
{
  if ( this->pvProposalData != NULL )
  {
    POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
    delete pData;  
  } /* end */     
};

/* operations */

void OtmProposal::clear()
{
  if ( this->pvProposalData != NULL )
  {
    POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
    memset( pData, 0, sizeof(OTMPROPOSALDATA) );
    pData->eType = OtmProposal::eptUndefined;
    pData->eMatch = OtmProposal::emtUndefined;
  } /* end */     

}

/* setters and getters */

/* \brief get the internal key of the proposal
   The internal key is the memory internal identifier of the proposal
   \param pszBuffer Pointer to buffer receiving the proposal key
   \param iBufSize Size of the buffer in number of characters
	 \returns Number of characters copied to pszBuffer including the terminating null character
*/
int OtmProposal::getInternalKey( char *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBuffer( pData->szInternalKey, pszBuffer, iBufSize ) );
}

  	
  /* \brief set the internal proposal key
     \param pszBuffer Pointer to buffer containing the proposal key
   */
void OtmProposal::setInternalKey( char *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  strcpy( pData->szInternalKey, pszBuffer );
  pData->fFilled = 1;
}

/* \brief get length of proposal source text 
  	\returns Number of characters in proposal source text
  */
int OtmProposal::getSourceLen()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( wcslen( pData->szSource ) );
}

/* \brief get proposal source text 
    \param pszBuffer Pointer to buffer receiving the proposal source text
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getSource( wchar_t *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBufferW( pData->szSource, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal source text
    \param pszBuffer Pointer to buffer containing the proposal source text
  */
void OtmProposal::setSource( wchar_t *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  wcsncpy( pData->szSource, pszBuffer, OTMPROPOSAL_MAXSEGLEN );
  pData->szSource[OTMPROPOSAL_MAXSEGLEN] = 0;
  pData->fFilled = 1;
}
  	
/* \brief get length of proposal target text 
  	\returns Number of characters in proposal target text
  */
int OtmProposal::getTargetLen()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( wcslen( pData->szTarget ) );
}

/* \brief get proposal target text   
    \param pszBuffer Pointer to buffer receiving the proposal target text
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getTarget( wchar_t *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBufferW( pData->szTarget, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal target text
    \param pszBuffer Pointer to buffer containing the proposal target text
  */
void OtmProposal::setTarget( wchar_t *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  wcsncpy( pData->szTarget, pszBuffer, OTMPROPOSAL_MAXSEGLEN );
  pData->szTarget[OTMPROPOSAL_MAXSEGLEN] = 0;
  pData->fFilled = 1;
}
  	
/* \brief get proposal ID
    \param pszBuffer Pointer to buffer receiving the proposal ID
    \param iBufSize Size of the buffer in number of characters
    \returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getID( char *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBuffer( pData->szId, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal ID 
    \param pszBuffer Pointer to buffer containing the proposal ID
  */
void OtmProposal::setID( char *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  strncpy( pData->szId, pszBuffer, sizeof(pData->szId)-1 );
  pData->fFilled = 1;
}
  	

/* \brief get proposal document name

  \param pszBuffer Pointer to buffer receiving the document name
  \param iBufSize Size of the buffer in number of characters
  \returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getDocName( char *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBuffer( pData->szDocName, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal document short name
    \param pszBuffer Pointer to buffer containing the document name
  */
void OtmProposal::setDocName( char *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  strncpy( pData->szDocName, pszBuffer, sizeof(pData->szDocName)-1 );
  pData->fFilled = 1;
}
  	
/* \brief get proposal document short name
    \param pszBuffer Pointer to buffer receiving the document short name
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getDocShortName( char *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBuffer( pData->szDocShortName, pszBuffer, iBufSize ) );
}

  	
/* \brief set the proposal document short name
    \param pszBuffer Pointer to buffer containing the document short short name
  */
void OtmProposal::setDocShortName( char *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  strncpy( pData->szDocShortName, pszBuffer, sizeof(pData->szDocShortName)-1 );
  pData->fFilled = 1;
}


/* \brief get proposal segment number
  \returns proposal segment number
  */
long OtmProposal::getSegmentNum()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( pData->lSegmentNum );
}
  	
/* \brief set the proposal segment number
    \param lSegmentNum new segment number of proposal
  */
void OtmProposal::setSegmentNum( long lSegmentNumIn )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  pData->lSegmentNum = lSegmentNumIn;
  pData->fFilled = 1;
}

  	
/* \brief get proposal source language
    \param pszBuffer Pointer to buffer receiving the proposal source language
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getSourceLanguage( char *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBuffer( pData->szSourceLanguage, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal source language
    \param pszBuffer Pointer to buffer containing the proposal source language
  */
void OtmProposal::setSourceLanguage( char *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  strncpy( pData->szSourceLanguage, pszBuffer, sizeof(pData->szSourceLanguage)-1 );
  pData->fFilled = 1;
}


/* \brief get proposal target language
    \param pszBuffer Pointer to buffer receiving the proposal target language
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getTargetLanguage( char *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBuffer( pData->szTargetLanguage, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal target language
    \param pszBuffer Pointer to buffer containing the proposal target language
  */
void OtmProposal::setTargetLanguage( char *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  strncpy( pData->szTargetLanguage, pszBuffer, sizeof(pData->szTargetLanguage)-1 );
  pData->fFilled = 1;
}


/* \brief get proposal type
    \returns proposal type
  */
OtmProposal::eProposalType OtmProposal::getType()
{
  if ( this->pvProposalData == NULL ) return( OtmProposal::eptUndefined );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( pData->eType );
}
  	
/* \brief set the proposal type
    \param eType new type of the proposal
  */
void OtmProposal::setType( eProposalType eTypeIn )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  pData->eType = eTypeIn;
  pData->fFilled = 1;
}

/* \brief get match type
    \returns proposal type
  */
OtmProposal::eMatchType OtmProposal::getMatchType()
{
  if ( this->pvProposalData == NULL ) return( OtmProposal::emtUndefined );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( pData->eMatch );
}
  	
/* \brief set the match type
    \param eType new type of the proposal
  */
void OtmProposal::setMatchType( OtmProposal::eMatchType eMatchTypeIn )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  pData->eMatch = eMatchTypeIn;
  pData->fFilled = 1;
}


/* \brief get name of proposal author
    \param pszBuffer Pointer to buffer receiving the name of the proposal author
    \param iBufSize Size of the buffer in number of characters
    \returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getAuthor( char *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBuffer( pData->szTargetAuthor, pszBuffer, iBufSize ) );
}
  	
/* \brief set the name of the proposal author
    \param pszBuffer Pointer to buffer containing the name of the proposal author
  */
void OtmProposal::setAuthor( char *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  strncpy( pData->szTargetAuthor, pszBuffer, sizeof(pData->szTargetAuthor)-1 );
  pData->fFilled = 1;
}


/* \brief get proposal time stamp
    \returns proposal segment number
  */
long OtmProposal::getUpdateTime()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( pData->lTargetTime );
}
  	
/* \brief set the proposal time stamp
    \param lTime new time stamp of proposal
  */
void OtmProposal::setUpdateTime( long lTime )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  pData->lTargetTime = lTime;
  pData->fFilled = 1;
}

/* \brief get proposal fuzziness
    \returns proposal fuzziness
  */
int OtmProposal::getFuzziness()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( pData->iFuzziness );
}

  	
/* \brief set the proposal fuzziness
    \param lFuzzinessTime new fuzziness of proposal
  */
void OtmProposal::setFuzziness( long iFuzzinessIn )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  pData->iFuzziness = iFuzzinessIn;
  pData->fFilled = 1;
}


/* \brief get markup table name (format)
    \param pszBuffer Pointer to buffer receiving the name of the markup table name
    \param iBufSize Size of the buffer in number of characters
    \returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getMarkup( char *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBuffer( pData->szMarkup, pszBuffer, iBufSize ) );
}
  	
/* \brief set markup table name (format)
    \param pszBuffer Pointer to buffer containing the markup table name
  */
void OtmProposal::setMarkup( char *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  strncpy( pData->szMarkup, pszBuffer, sizeof(pData->szMarkup)-1 );
  pData->fFilled = 1;
}

/* \brief get length of proposal context 
  	\returns Number of characters in proposal context
  */
int OtmProposal::getContextLen()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( wcslen( pData->szContext ) );
}

/* \brief get proposal Context 
    \param pszBuffer Pointer to buffer receiving the proposal context
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getContext( wchar_t *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBufferW( pData->szContext, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal context
    \param pszBuffer Pointer to buffer containing the proposal context
  */
void OtmProposal::setContext( wchar_t *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  wcsncpy( pData->szContext, pszBuffer, OTMPROPOSAL_MAXSEGLEN );
  pData->fFilled = 1;
}

/* \brief get proposal context ranking
  	\returns proposal context ranking
  */
int OtmProposal::getContextRanking()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( pData->iContextRanking );
}
  	
/* \brief set the proposal context ranking
     \param iContextRanking context ranking for the proposal (0..100)
  */
void OtmProposal::setContextRanking( int iContextRankingIn )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  pData->iContextRanking = iContextRankingIn;
  pData->fFilled = 1;
}

/* \brief get length of proposal AddInfo text 
  	\returns Number of characters in proposal AddInfo text
  */
int OtmProposal::getAddInfoLen()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( wcslen( pData->szAddInfo ) );
}

/* \brief get additional info
    \param pszBuffer Pointer to buffer receiving the additional info
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getAddInfo( wchar_t *pszBuffer, int iBufSize )
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( CopyToBufferW( pData->szAddInfo, pszBuffer, iBufSize ) );
}


/* \brief set the proposal additional information
    \param pszBuffer Pointer to buffer containing the additional info
  */
void OtmProposal::setAddInfo( wchar_t *pszBuffer )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  wcsncpy( pData->szAddInfo, pszBuffer, OTMPROPOSAL_MAXSEGLEN );
  pData->fFilled = 1;
}


/* \brief set the proposal memory index
    \param iIndex new value for the memory index
  */
void OtmProposal::setMemoryIndex( int iIndex )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  pData->iMemoryIndex = iIndex;
}

/* \brief get memory index
  	\returns memory index of this proposal
  */
int OtmProposal::getMemoryIndex()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( pData->iMemoryIndex );
}


/* \brief set the replacement list
    \param pList new value for the replacement list
  */
void OtmProposal::setReplacementList( long pList )
{
  if ( this->pvProposalData == NULL ) return;
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  pData->pvReplacementList = pList;
}

/* \brief get replacement list
  	\returns pointer to replacement list
  */
long OtmProposal::getReplacementList()
{
  if ( this->pvProposalData == NULL ) return( 0 );
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( (long)pData->pvReplacementList );
}


/* \brief check if proposal match type is one of the exact match types
  */
bool OtmProposal::isExactMatch()
{
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( (pData->eType == eptManual) && ((pData->eMatch == emtExact) || (pData->eMatch == emtExactContext) || (pData->eMatch == emtExactExact)) );
}

/* \brief check if proposal is empty (i.e. has not been used)
  */
bool OtmProposal::isEmpty()
{
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( !pData->fFilled );
}

/* \brief check if source and target of proposal is equal
  */
bool OtmProposal::isSourceAndTargetEqual()
{
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  return( wcscmp( pData->szSource, pData->szTarget ) == 0 );
}

/* \brief check if target strings of two proposals are identical
   \param otherProposal pointer to second proposal 
  */
bool OtmProposal::isSameTarget( OtmProposal *otherProposal )
{
  POTMPROPOSALDATA pData = (POTMPROPOSALDATA)this->pvProposalData;
  POTMPROPOSALDATA pOtherData = (POTMPROPOSALDATA)otherProposal->pvProposalData;
  return( wcscmp( pData->szTarget, pOtherData->szTarget ) == 0 );
}

/*! \brief Clear the data of proposals stored in a vector
  \param Proposals reference to a vector containing the proposals
*/
void OtmProposal::clearAllProposals(
  std::vector<OtmProposal *> &Proposals
)
{
  for ( int i = 0; i < (int)Proposals.size(); i++ )
  {
    Proposals[i]->clear();
  } /* endfor */     
}

/*! \brief Get the number of filled proposals in a proposal list
  \param Proposals reference to a vector containing the proposals
*/
int OtmProposal::getNumOfProposals(
  std::vector<OtmProposal *> &Proposals
)
{
  int iFilled = 0;
  for ( int i = 0; i < (int)Proposals.size(); i++ )
  {
    if ( !Proposals[i]->isEmpty() ) iFilled++;
  } /* endfor */  
  return( iFilled );
}




  /* \brief assignment operator to copy the datafields from one
    OtmProposal to another one
     \param copyme reference to OtmProposal being copied
   */
OtmProposal &OtmProposal::operator=( const OtmProposal &copyme )
{
  if (this != &copyme ) 
  {
    POTMPROPOSALDATA pSource = (POTMPROPOSALDATA)copyme.pvProposalData;
    POTMPROPOSALDATA pTarget = (POTMPROPOSALDATA)this->pvProposalData;

    if ( (pSource != NULL) && (pTarget != NULL) && (pSource != pTarget) )
    {
      memcpy( pTarget, pSource, sizeof(OTMPROPOSALDATA) );
    }
  }
  return *this; 
}




/*! \brief Copies a string to the user supplied buffer area*/
int CopyToBufferW( wchar_t *pszSource, wchar_t *pszBuffer, int iBufSize )
{
  int iCopied = wcslen( pszSource );
  if ( iCopied >= iBufSize ) iCopied = iBufSize - 1;
  memcpy( pszBuffer, pszSource, iCopied * sizeof(wchar_t) );
  pszBuffer[iCopied] = 0;
  return( iCopied );
}

int CopyToBuffer( char *pszSource, char *pszBuffer, int iBufSize )
{
  int iCopied = strlen( pszSource );
  if ( iCopied >= iBufSize ) iCopied = iBufSize - 1;
  memcpy( pszBuffer, pszSource, iCopied );
  pszBuffer[iCopied] = 0;
  return( iCopied );
}
 
