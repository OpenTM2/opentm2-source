/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _EqfMemory_h_
#define _EqfMemory_h_

#include "core\utilities\LogWriter.h"
#include <string>
#include "core\PluginManager\OtmMemory.h"
#include "OtmProposal.h"
#include "EqfMemoryPlugin.h"

class EqfMemory: public OtmMemory
/*! \brief This class implements the standard translation memory (EQF) for OpenTM2.
*/

{
public:
/*! \brief Constructors
*/
	EqfMemory() {};

	EqfMemory( EqfMemoryPlugin *pMemoryPlugin, HTM htm, char *pszName );

/*! \brief Destructor
*/
	~EqfMemory();

/*! \brief Store the supplied proposal in the memory
    When the proposal aready exists it will be overwritten with the supplied data

    \param pProposal pointer to a OtmProposal object

  	\returns 0 or error code in case of errors
*/
  int putProposal
  (
    OtmProposal &Proposal
  ); 

  /*! \brief Get the the first proposal from the memory and prepare sequential access
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
  	\returns handle for usage with GetNextProposal or 0 in case of errors
*/
  int getFirstProposal
  (
    OtmProposal &Proposal
  ); 

  /*! \brief Get the the first proposal from the memory and prepare sequential access
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns handle for usage with GetNextProposal or 0 in case of errors
*/
  int getFirstProposal
  (
    OtmProposal &Proposal,
    int *piProgress
  ); 

/*! \brief Get the next proposal from the memory 
    \param lHandle the hande returned by GetFirstProposal
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
  	\returns 0 or error code in case of errors
*/
  int getNextProposal
  (
    OtmProposal &Proposal
  ); 

/*! \brief Get the next proposal from the memory (with progress info) 
    \param lHandle the hande returned by GetFirstProposal
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns 0 or error code in case of errors
*/
  int getNextProposal
  (
    OtmProposal &Proposal,
    int *piProgress
  ); 

    /*! \brief Get the current sequential access key (the key for the next proposal in the memory) 
    \param pszKeyBuffer pointer to the buffer to store the sequential access key
    \param iKeyBufferSize size of the key buffer in number of characters
  	\returns 0 or error code in case of errors
  */
  int getSequentialAccessKey
  (
    char *pszKeyBuffer,
    int  iKeyBufferSize
  ); 

    
  /*! \brief Set the current sequential access key to resume the sequential access at the given position
    \param pszKey a sequential access key previously returned by getSequentialAccessKey
  	\returns 0 or error code in case of errors
  */
  int setSequentialAccessKey
  (
    char *pszKey
  ); 


/*! \brief Get the the proposal having the supplied key (InternalKey from the OtmProposal)
    \param pszKey internal key of the proposal
    \param Proposal buffer for the returned proposal data
  	\returns 0 or error code in case of errors
*/
  int getProposal
  (
    char *pszKey,
    OtmProposal &Proposal
  ); 

/*! \brief Provides a part of the memory in binary format

     The binary format is used by the folder export to add the memory
     in the internal format to the exported folder.

    \param pMemoryPartData pointer to the data area for the extraxt om binary format

  	\returns 0 or error code in case of errors
*/
  int getMemoryPart
  (
    PMEMORYPARTDATA pData           // points to data area supplied by the caller
  );

/*! \brief Get a list of memory proposals matching the given search key

    This method uses the search data contained in the search key to find one or more
    matching proposals in the memory. At least the szSource and the szTargetLang members of the
    search key have to be filled by the caller.
    The caller provides a list of OtmProposals which will be filled with the data of the matching 
    proposals. The number of requested proposals is determined by the number
    of proposals in the list.

    \param SearchKey proposal containing search string and meta data
    \param FoundProposals refernce to vector with OtmProposal objects
    \param ulOptions options for the lookup

  	\returns 0 or error code in case of errors
*/
  int searchProposal
  (
    OtmProposal &SearchKey,
    std::vector<OtmProposal *> &FoundProposals,
    unsigned long ulOptions

  ); 

/*! \brief Updates some fields of a specific proposal in the memory

    \param Proposal reference to a OtmProposal object containing the data being changed
    \param usUpdateFlags Flags selecting the update fields

  	\returns 0 or error code in case of errors
*/
  int updateProposal
  (
    OtmProposal &Proposal,
    USHORT      usUpdateFlags
  ); 

/*! \brief Delete a specific proposal from the memory

    \param Proposal reference to a OtmProposal object

  	\returns 0 or error code in case of errors
*/
  int deleteProposal
  (
    OtmProposal &Proposal
  ); 

  /*! \brief Rebuild internal index after mass updates
    This method is called after mass updates (e.g. memory import) has beebn performed.
    The memory can rebuild or optimize its internal index when necessary.

  	\returns 0 or error code in case of errors
*/
  int rebuildIndex
  (
  ); 


/*! \brief Get number of markups used for the proposals in this memory
  	\returns number of markups used by the memory proposals or 0 if no markup information can be provided
*/
  int getNumOfMarkupNames();

/*! \brief Get markup name at position n [n = 0.. GetNumOfMarkupNames()-1]
    \param iPos position of markup table name
    \param pszBuffer pointer to a buffer for the markup table name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  int getMarkupName
  (
    int iPos,
    char *pszBuffer,
    int iSize
  );

/*! \brief Get number of different document names used in the memory
  	\returns number of markups used by the memory proposals or 0 if no document name information can be provided
*/
  int getNumOfDocumentNames();

/*! \brief Get document name at position n [n = 0.. GetNumOfDocumentNames()-1]
    \param iPos position of document name
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  int getDocumentName
  (
    int iPos,
    char *pszBuffer,
    int iSize
  );

  /*! \brief Get number of different document short names used in the memory
  	\returns number of document short names used by the memory proposals or 0 if no document short name information can be provided
*/
  	int getNumOfDocumentShortNames();

/*! \brief Get document name at position n [n = 0.. GetNumOfDocumentNames()-1]
    \param iPos position of document name
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  	int getDocumentShortName
    (
      int iPos,
      char *pszBuffer,
      int iSize
    );

/*! \brief Get number of different languages used in the memory
  	\returns number of languages used by the memory proposals or 0 if no language information can be provided
*/
  int getNumOfLanguages();

/*! \brief Get language at position n [n = 0.. GetNumOfLanguages()-1]
    \param iPos position of language
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  int getLanguage
  (
    int iPos,
    char *pszBuffer,
    int iSize
  );



/*! \brief Get source language of the memory
    \param pszBuffer pointer to a buffer for the source language name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  int getSourceLanguage
  (
    char *pszBuffer,
    int iSize
  );

/*! \brief Get the name of the memory
    \param pszBuffer pointer to a buffer for the name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  int getName
  (
    char *pszBuffer,
    int iSize
  );

  /*! \brief Get the name of the memory
    \param strName reference of a string receiving the memory name
*/
  int getName
  (
    std::string &strName
  );


/*! \brief Get description of the memory
    \param pszBuffer pointer to a buffer for the description
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  int getDescription
  (
    char *pszBuffer,
    int iSize
  );

  /*! \brief Set the description of the memory
    \param pszBuffer pointer to the description text
  */
  void setDescription
  (
    char *pszBuffer
  );

/*! \brief Get plugin responsible for this memory
  	\returns pointer to memory plugin object
*/
  void *getPlugin();

/*! \brief Provide the internal memory handle
  	\returns memory handle
*/
  HTM getHTM();
  
/*! \brief Get number of proposals in this memory
  	\returns number of proposals
*/
  unsigned long getProposalNum();

/*! \brief Get overall file size of this memory
  	\returns size of memory in number of bytes
*/
  unsigned long getFileSize();

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
	int getLastError
  (
    std::string &strError
  ); 
/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
	int getLastError
  (
    char *pszError,
    int iBufSize
  ); 
  
/*! \brief Set or clear the pointer to a loaded global memory option file

    This method sets a pointer to a loaded global memory option file.
    When set the option file will be used to decide how global memory proposals will be processed.

    \param pvGlobalMemoryOptions pointer to a loaded global memory option file or NULL to clear the current option file pointer

  	\returns 0 or error code in case of errors
*/
  int setGlobalMemoryOptions
  (
    void *pvGlobalMemoryOptions
  ); 


private:
  HTM htm;                                       // old fashioned memory handle for this memory
  PTMX_CLB pTmClb;                               // ptr to ctl block struct
  PTMX_EXT_IN_W  pTmExtIn;                       // ptr to extract input struct
  PTMX_EXT_OUT_W pTmExtOut;                      // ptr to extract output struct
  PTMX_PUT_IN_W  pTmPutIn;                       // ptr to TMX_PUT_IN_W structure
  PTMX_PUT_OUT_W pTmPutOut;                      // ptr to TMX_PUT_OUT_W structure
  PTMX_GET_IN_W  pTmGetIn;                       // ptr to TMX_PUT_IN_W structure
  PTMX_GET_OUT_W pTmGetOut;                      // ptr to TMX_PUT_OUT_W structure
  unsigned long ulNextKey;                       // next TM key for GetFirstProposal/GetNextProposal
  unsigned short usNextTarget;                   // next TM target for GetFirstProposal/GetNextProposal
  EqfMemoryPlugin *pMemoryPlugin;                // memory plugin for this memory
  char szName[MAX_LONGFILESPEC];                 // memory name
	std::string strLastError;
	int iLastError;
  LogWriter Log;                                 // log object (only used when logging is active)
  void *pvGlobalMemoryOptions;                   // pointert to global memory options to be used for global memory proposals

/*! \brief Fill OtmProposal from TMX_GET_OUT_W structure
    \param ulKey key of record containing the proposal
    \param usTargetNum number of target within record 
    \param Proposal reference to the OtmProposal being filled
  	\returns 0 or error code in case of errors
*/
int SetProposalKey
(
  ULONG   ulKey,
  USHORT  usTargetNum,
  OtmProposal *pProposal
);

/*! \brief Split an internal key into record number and target number
    \param Proposal reference to the OtmProposal 
    \param pulKey pointer to record number buffer
    \param pusTargetNum pointer to buffer for number of target within record 
  	\returns 0 or error code in case of errors
*/
int SplitProposalKeyIntoRecordAndTarget
(
  OtmProposal &Proposal,
  ULONG   *pulKey,
  USHORT  *pusTargetNum
);

/*! \brief Split an internal key into record number and target number
    \param pszKey pointer to the internal key of the OtmProposal 
    \param pulKey pointer to record number buffer
    \param pusTargetNum pointer to buffer for number of target within record 
  	\returns 0 or error code in case of errors
*/
int SplitProposalKeyIntoRecordAndTarget
(
  char    *pszKey,
  ULONG   *pulKey,
  USHORT  *pusTargetNum
);


/*! \brief Fill OtmProposal from TMX_GET_OUT_W structure
    \param pExtOut pointer to the TMX_GET_OUT_W structure
    \param Proposal reference to the OtmProposal being filled
  	\returns 0 or error code in case of errors
*/
int ExtOutToOtmProposal
(
  PTMX_EXT_OUT_W pExtOut,
  OtmProposal &Proposal
);

/*! \brief Fill OtmProposal from TMX_MATCH_TABLE_W structure
    \param pMatch pointer to the TMX_MATCH_TABLE_W structure
    \param Proposal reference to the OtmProposal being filled
  	\returns 0 or error code in case of errors
*/
int EqfMemory::MatchToOtmProposal
(
  PTMX_MATCH_TABLE_W pMatch,
  OtmProposal *pProposal
);

/*! \brief Fill TMX_PUT_IN_W structure with OtmProposal data
    \param Proposal reference to the OtmProposal containing the data
    \param pPutIn pointer to the TMX_PUT_IN_W structure
  	\returns 0 or error code in case of errors
*/
int EqfMemory::OtmProposalToPutIn
(
  OtmProposal &Proposal,
  PTMX_PUT_IN_W pPutIn
);

/*! \brief Fill TMX_GET_IN_W structure with OtmProposal data
    \param Proposal reference to the OtmProposal containing the data
    \param pGetIn pointer to the TMX_GET_IN_W structure
  	\returns 0 or error code in case of errors
*/
int EqfMemory::OtmProposalToGetIn
(
  OtmProposal &Proposal,
  PTMX_GET_IN_W pGetIn
);

/*! \brief Handle a return code from the memory functions and create 
    the approbriate error message text for it
    \param iRC return code from memory function
    \param pszMemName long memory name
    \param pszMarkup markup table name
  	\returns original or modified error return code
*/
int handleError( int iRC, char *pszMemName, char *pszMarkup );

};

#endif // #ifndef _EqfMemory_h_