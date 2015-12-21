/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMMEMORY_H_
#define _OTMMEMORY_H_

#ifndef CPPTEST
extern "C" {
#endif
#define INCL_EQF_TAGTABLE         // tag table and format functions
#define INCL_EQF_TP
#define INCL_EQF_TM
#define INCL_EQF_DAM
#include "eqf.h"
//#include "eqftmi.h"
//#include "eqftmm.h"
//#include "eqfqdami.h"
//#include "eqftmrem.h"
#ifndef CPPTEST
}
#endif

#include "vector"
#include "OtmProposal.h"

/*! \brief Abstract base-class for translation memory objects */
class __declspec(dllexport) OtmMemory
{

public:

/*! \brief Constructors */
	OtmMemory() {};
	
 
/*! \brief Destructor */
	virtual ~OtmMemory() {};

/*! \brief Error code definition
*/
  static const int ERROR_NOSHAREDMEMORYPLUGIN = 8001;
  static const int ERROR_PROPERTYLOADFAILED   = 8002;
  static const int ERROR_MEMORYOBJECTISNULL   = 8003;
  static const int ERROR_BUFFERTOOSMALL       = 8004;
  static const int ERROR_INVALIDOBJNAME       = 8005;
  static const int ERROR_MEMORYEXISTS         = 8006;
  static const int ERROR_INVALIDREQUEST       = 8007;
  static const int INFO_ENDREACHED            = 8008;
  static const int ERROR_ENTRYISCORRUPTED     = 8009;

/*! \brief Provide a list of memory proposals matching the given search key

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
  virtual int searchProposal
  (
    OtmProposal &SearchKey,
    std::vector<OtmProposal *> &FoundProposals,
    unsigned long ulOptions

  ) = 0; 

/*! \brief Get the the first proposal from the memory and prepare sequential access
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
  	\returns 0 or error code in case of errors
*/
  virtual int getFirstProposal
  (
    OtmProposal &Proposal
  ) = 0; 

/*! \brief Get the the first proposal from the memory and prepare sequential access
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns 0 or error code in case of errors
*/
  virtual int getFirstProposal
  (
    OtmProposal &Proposal,
    int *piProgress
  ) = 0; 

/*! \brief Get the the proposal having the supplied key (InternalKey from the OtmProposal)
    \param pszKey internal key of the proposal
    \param Proposal buffer for the returned proposal data
  	\returns 0 or error code in case of errors
*/
  virtual int getProposal
  (
    char *pszKey,
    OtmProposal &Proposal
  ) = 0; 

/*! \brief Get the next proposal from the memory 
    \param lHandle the hande returned by GetFirstProposal
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
  	\returns 0 or error code in case of errors
*/
  virtual int getNextProposal
  (
    OtmProposal &Proposal
  ) = 0; 

/*! \brief Get the next proposal from the memory 
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns 0 if a proposal was returned, ENDREACHED when the end of the memory has been reached or an error code in case of errors
*/
  virtual int getNextProposal
  (
    OtmProposal &Proposal,
    int *piProgress
  ) = 0; 

  /*! \brief Get the current sequential access key (the key for the next proposal in the memory) 
    \param pszKeyBuffer pointer to the buffer to store the sequential access key
    \param iKeyBufferSize size of the key buffer in number of characters
  	\returns 0 or error code in case of errors
  */
  virtual int getSequentialAccessKey
  (
    char *pszKeyBuffer,
    int  iKeyBufferSize
  ) = 0; 

    
  /*! \brief Set the current sequential access key to resume the sequential access at the given position
    \param pszKey a sequential access key previously returned by getSequentialAccessKey
  	\returns 0 or error code in case of errors
  */
  virtual int setSequentialAccessKey
  (
    char *pszKey
  ) = 0; 

/*! \brief Store the supplied proposal in the memory
    When the proposal aready exists it will be overwritten with the supplied data

    \param pProposal pointer to a OtmProposal object

  	\returns 0 or error code in case of errors
*/
  virtual int putProposal
  (
    OtmProposal &Proposal
  ) = 0; 


  /*! \brief Rebuild internal index after mass updates
    This method is called after mass updates (e.g. memory import) has beebn performed.
    The memory can rebuild or optimize its internal index when necessary.

  	\returns 0 or error code in case of errors
*/
  virtual int rebuildIndex
  (
  ) = 0; 

/*! \brief Flags for the update of proposals */

static const int UPDATE_MARKUP   = 0x01;           // update markup/tag table
static const int UPDATE_MTFLAG   = 0x02;           // update machine translation flag
static const int UPDATE_TARGLANG = 0x04;           // update target language
static const int UPDATE_DATE     = 0x08;           // update proposal update time

/*! \brief Updates some fields of a specific proposal in the memory

    \param Proposal reference to a OtmProposal object containing the data being changed
    \param usUpdateFlags Flags selecting the update fields

  	\returns 0 or error code in case of errors
*/
  virtual int updateProposal
  (
    OtmProposal &Proposal,
    USHORT      usUpdateFlags
  ) = 0; 

/*! \brief Delete a specific proposal from the memory

    \param Proposal reference to a OtmProposal object

  	\returns 0 or error code in case of errors
*/
  virtual int deleteProposal
  (
    OtmProposal &Proposal
  ) = 0; 


/*! \brief Data area for the getMemoryPart method */
typedef struct _MEMORYPARTDATA
{
  // fields provided and filled by the caller of the method
  BOOL   fFirstCall;                   // TRUE = this is the initial call for this memory file, FALSE = a subsequent call
  char   szFileName[MAX_LONGFILESPEC]; // name of file being processed currently (the file name is from the list of memory data files
                                       // provided using the getMemoryFiles method
  PBYTE  pbBuffer;                     // points to the buffer for the provided data
  ULONG  ulBytesToRead;                // number of bytes to be copied to buffer

  // data fields filled by the getMemoryPart method
  BOOL   fFileIsComplete;              // TRUE = the data of the current file has been stored in the buffer completely, 
                                       // FALSE = there is more data coming
  ULONG  ulBytesRead;                  // length of data stored in the buffer in number of bytes

  // private data fields for the getMemoryPart method (this data is not used by the calling function in any way)
  ULONG  ulFilePos;                    // could be used for the read position in current file
  ULONG  ulRemaining;                  // could be used for the number of bytes remaining
  ULONG  ulTotalSize;                  // could be used for the total size of the current file
  ULONG  ulState;                      // could be used for current processing state
  void   *pPrivatData;                 // could be used as anchor for private data areas
  char   chPrivateBuffer[1024];        // could be used as private buffer area
} MEMORYPARTDATA, *PMEMORYPARTDATA;

/*! \brief Provides part of a memory data file in binary format

     The binary format is used by the folder export to add the memory
     in the internal format to the exported folder.

    \param pMemoryPartData pointer to the data area for the extraxt om binary format

  	\returns 0 or error code in case of errors
*/
  virtual int getMemoryPart
  (
    PMEMORYPARTDATA pData           // points to data area supplied by the caller
  ) = 0;

/*! \brief Get the name of the memory
    \param pszBuffer pointer to a buffer for the name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  virtual int getName
  (
    char *pszBuffer,
    int iSize
  ) = 0;

/*! \brief Get the name of the memory
    \param strName reference of a string receiving the memory name
*/
  virtual int getName
  (
    std::string &strName
  ) = 0;



/*! \brief Get number of markups used for the proposals in this mmoryProvides a part of the memory in binary format
  	\returns number of markups used by the memory proposals or 0 if no markup information can be provided
*/
  virtual int getNumOfMarkupNames() = 0;

/*! \brief Get markup name at position n [n = 0.. GetNumOfMarkupNames()-1]
    \param iPos position of markup table name
    \param pszBuffer pointer to a buffer for the markup table name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  virtual int getMarkupName
  (
    int iPos,
    char *pszBuffer,
    int iSize
  ) = 0;

/*! \brief Get source language of the memory
    \param pszBuffer pointer to a buffer for the source language name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  virtual int getSourceLanguage
  (
    char *pszBuffer,
    int iSize
  ) = 0;

/*! \brief Get description of the memory
    \param pszBuffer pointer to a buffer for the description
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  virtual int getDescription
  (
    char *pszBuffer,
    int iSize
  ) = 0;

/*! \brief Set the description of the memory
    \param pszBuffer pointer to the description text
*/
  virtual void setDescription
  (
    char *pszBuffer
  ) = 0;


/*! \brief Get plugin responsible for this memory
  	\returns pointer to memory plugin object
*/
  virtual void *getPlugin() = 0;

/*! \brief Get number of proposals in thismemory
  	\returns number of proposals
*/
  virtual unsigned long getProposalNum() = 0;

/*! \brief Get overall file size of this memory
  	\returns size of memory in number of bytes
*/
  virtual unsigned long getFileSize() = 0;



/*! \brief OtmMemory related return codes

*/
  static const int ERROR_INTERNALKEY_MISSING = 1001;

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
	virtual int getLastError
  (
    std::string &strError
  ) = 0; 
/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
	virtual int getLastError
  (
    char *pszError,
    int iBufSize
  ) = 0; 

/*! \brief Get number of different document names used in the memory
  	\returns number of document used by the memory proposals or 0 if no document name information can be provided
*/
  	virtual int getNumOfDocumentNames() = 0;

/*! \brief Get document name at position n [n = 0.. GetNumOfDocumentNames()-1]
    \param iPos position of document name
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  	virtual int getDocumentName
    (
      int iPos,
      char *pszBuffer,
      int iSize
    ) = 0;

/*! \brief Get number of different document short names used in the memory
  	\returns number of document short names used by the memory proposals or 0 if no document short name information can be provided
*/
  	virtual int getNumOfDocumentShortNames() = 0;

/*! \brief Get document name at position n [n = 0.. GetNumOfDocumentNames()-1]
    \param iPos position of document name
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  	virtual int getDocumentShortName
    (
      int iPos,
      char *pszBuffer,
      int iSize
    ) = 0;

/*! \brief Get number of different languages used in the memory
  	\returns number of languages used by the memory proposals or 0 if no language information can be provided
*/
  	virtual int getNumOfLanguages() = 0;

/*! \brief Get language at position n [n = 0.. GetNumOfLanguages()-1]
    \param iPos position of language
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  	virtual int getLanguage
    (
      int iPos,
      char *pszBuffer,
      int iSize
    ) = 0;
	
/*! \brief Set or clear the pointer to a loaded global memory option file

    This method sets a pointer to a loaded global memory option file.
    When set the option file will be used to decide how global memory proposals will be processed.

    \param pvGlobalMemoryOptions pointer to a loaded global memory option file or NULL to clear the current option file pointer

  	\returns 0 or error code in case of errors
*/
  virtual int setGlobalMemoryOptions
  (
    void *pvGlobalMemoryOptions
  ) = 0; 
  

private:

};

#endif // #ifndef _OTMMEMORY_H_
  