/*! \file

Description: Implementation of the abstract OtmMemory class


Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TAGTABLE         // tag table and format functions
#define INCL_EQF_TP
#define INCL_EQF_TM
#define INCL_EQF_DAM
//#define INCL_EQF_ANALYSIS         // analysis functions
//#include <eqf.h>                  // General Translation Manager include file

#include "core\PluginManager\PluginManager.h"
#include "OtmProposal.h"
#include "EqfSharedMemory.h"
#include "JSONFactory.h"
#include "TMXFactory.h"

EqfSharedMemory::EqfSharedMemory()
{
  this->pLocalMemory = NULL;
  this->pMemoryPlugin = NULL;
  this->pProperties = NULL;
   this->strMemoryName = "";
}


EqfSharedMemory::~EqfSharedMemory()
{
}


/*! \brief Get number of markups used for the proposals in this memory
  	\returns number of markups used by the memory proposals or 0 if no markup information can be provided
*/
int EqfSharedMemory::getNumOfMarkupNames()
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getNumOfMarkupNames() );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get markup name at position n [n = 0.. GetNumOfMarkupNames()-1]
    \param iPos position of markup table name
    \param pszBuffer pointer to a buffer for the markup table name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
int EqfSharedMemory::getMarkupName
(
  int iPos,
  char *pszBuffer,
  int iSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getMarkupName( iPos, pszBuffer, iSize) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get source language of the memory
    \param pszBuffer pointer to a buffer for the source language name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
int EqfSharedMemory::getSourceLanguage
(
  char *pszBuffer,
  int iSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getSourceLanguage( pszBuffer, iSize ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get description of the memory
    \param pszBuffer pointer to a buffer for the description
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
int EqfSharedMemory::getDescription
(
  char *pszBuffer,
  int iSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getDescription( pszBuffer, iSize ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Set the description of the memory
  \param pszBuffer pointer to the description text
*/
void EqfSharedMemory::setDescription
(
  char *pszBuffer
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->setDescription( pszBuffer ) );
  }
  else
  {
    return;
  } /* end */     
}





/*! \brief Get the name of the memory
    \param pszBuffer pointer to a buffer for the name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
int EqfSharedMemory::getName
(
  char *pszBuffer,
  int iSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getName( pszBuffer, iSize ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get the name of the memory
    \param strName reference of a string receiving the memory name
*/
int EqfSharedMemory::getName
(
  std::string &strName
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getName( strName ) );
  }
  else
  {
    strName = "";
    return( -1 );
  } /* endif */     
}


/*! \brief Store the supplied proposal in the memory
    When the proposal aready exists it will be overwritten with the supplied data

    \param pProposal pointer to a OtmProposal object

  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::putProposal
(
  OtmProposal &Proposal
)
{
  int iRC = 0;
  if ( this->pLocalMemory != NULL )
  {
    // if proposal is empty, we only let it trigger to read input queue
    if(Proposal.getSourceLen()==0 && Proposal.getTargetLen()==0)
    {
      this->readProposalsFromInQueue();
      return iRC;
    }

    iRC = this->pLocalMemory->putProposal( Proposal );
    if ( iRC == 0 )
    {
       // send proposal to shared memory and check for updates 
      iRC = this->sendProposalToOutQueue( this->ePut, Proposal );
      // as now when download success, it will notify to read proposal
      // So should we still keep the trigger?
      //this->readProposalsFromInQueue();
    } /* end */       
  } /* end */     
  return( iRC );
}


/*! \brief Get the the first proposal from the memory and prepare sequential access
    \param pProposal pointer to a OtmProposal object which will be filled with the proposal data
  	\returns 0 or error code in case of errors
*/
  int EqfSharedMemory::getFirstProposal
  (
    OtmProposal &Proposal
  )
  {
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getFirstProposal( Proposal ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

  /*! \brief Get the the first proposal from the memory and prepare sequential access
    \param pProposal pointer to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::getFirstProposal
(
  OtmProposal &Proposal,
  int *piProgress
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getFirstProposal( Proposal, piProgress ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get the next proposal from the memory 
    \param lHandle the hande returned by GetFirstProposal
    \param pProposal pointer to a OtmProposal object which will be filled with the proposal data
  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::getNextProposal
(
  OtmProposal &Proposal
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getNextProposal( Proposal ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}


/*! \brief Get the next proposal from the memory 
    \param lHandle the hande returned by GetFirstProposal
    \param pProposal pointer to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::getNextProposal
(
  OtmProposal &Proposal,
  int *piProgress
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getNextProposal( Proposal, piProgress ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

  /*! \brief Get the current sequential access key (the key for the next proposal in the memory) 
  \param pszKeyBuffer pointer to the buffer to store the sequential access key
  \param iKeyBufferSize size of the key buffer in number of characters
  \returns 0 or error code in case of errors
*/
int EqfSharedMemory::getSequentialAccessKey
(
  char *pszKeyBuffer,
  int  iKeyBufferSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getSequentialAccessKey( pszKeyBuffer, iKeyBufferSize ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

    
/*! \brief Set the current sequential access key to resume the sequential access at the given position
  \param pszKey a sequential access key previously returned by getSequentialAccessKey
  \returns 0 or error code in case of errors
*/
int EqfSharedMemory::setSequentialAccessKey
(
  char *pszKey
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->setSequentialAccessKey( pszKey ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}


/*! \brief Get the the proposal having the supplied key (InternalKey from the OtmProposal)
    \param pszKey internal key of the proposal
    \param Proposal buffer for the returned proposal data
  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::getProposal
(
  char *pszKey,
  OtmProposal &Proposal
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getProposal( pszKey, Proposal ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}


/*! \brief Provides a part of the memory in binary format

     The binary format is used by the folder export to add the memory
     in the internal format to the exported folder.

    \param pMemoryPartData pointer to the data area for the extraxt om binary format

  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::getMemoryPart
(
  PMEMORYPARTDATA pData           // points to data area supplied by the caller
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getMemoryPart( pData ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

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
int EqfSharedMemory::searchProposal
(
  OtmProposal &SearchKey,
  std::vector<OtmProposal *> &FoundProposals,
  unsigned long ulOptions
)
{
  if ( this->pLocalMemory != NULL )
  {
    int iRC = this->pLocalMemory->searchProposal( SearchKey, FoundProposals, ulOptions );

    //this->readProposalsFromInQueue();

    return( iRC );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Set or clear the pointer to a loaded global memory option file

    This method sets a pointer to a loaded global memory option file.
    When set the option file will be used to decide how global memory proposals will be processed.

    \param pvGlobalMemoryOptions pointer to a loaded global memory option file or NULL to clear the current option file pointer

  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::setGlobalMemoryOptions
(
  void *pvGlobalMemoryOptions
)
{
  if ( this->pLocalMemory != NULL )
  {
    int iRC = this->pLocalMemory->setGlobalMemoryOptions( pvGlobalMemoryOptions );

    return( iRC );
  }
  else
  {
    return( 0 );
  } /* end */     
}



/*! \brief Updates some fields of a specific proposal in the memory

    \param Proposal reference to a OtmProposal object containing the data being changed
    \param usUpdateFlags Flags selecting the update fields

  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::updateProposal
(
  OtmProposal &Proposal,
  USHORT      usUpdateFlags
)
{
  int iRC = 0;

  if ( this->pLocalMemory != NULL )
  {
    iRC = this->pLocalMemory->updateProposal( Proposal, usUpdateFlags );
    if ( iRC == 0 )
    {
       // send proposal to shared memory and check for updates 
      this->sendProposalToOutQueue( this->eUpdate, Proposal );

      //this->readProposalsFromInQueue();
    } /* end */       
    return( iRC );
  }
  else
  {
    return( 0 );
  } /* end */     
}



/*! \brief Delete a specific proposal from the memory

    \param Proposal reference to a OtmProposal object

  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::deleteProposal
(
  OtmProposal &Proposal
)  
{
  int iRC = 0;

  if ( this->pLocalMemory != NULL )
  {
    iRC = this->pLocalMemory->deleteProposal( Proposal );
    if ( iRC == 0 )
    {
       // send proposal to shared memory and check for updates 
      this->sendProposalToOutQueue( this->eDelete, Proposal );

      //this->readProposalsFromInQueue();
    } /* end */       
    return( iRC );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Rebuild internal index after mass updates
  This method is called after mass updates (e.g. memory import) has beebn performed.
  The memory can rebuild or optimize its internal index when necessary.

  \returns 0 or error code in case of errors
*/
int EqfSharedMemory::rebuildIndex
(
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->rebuildIndex() );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get plugin responsible for this memory
  	\returns pointer to memory plugin object
*/
void *EqfSharedMemory::getPlugin()
{
  return( this->pMemoryPlugin );
}

/*! \brief Initialize the memory object and prepares sharing of proposals

    \param pMemoryPlugin pointer to shared memory plugin
    \param pszMemoryName name of the memory
    \param pLocalMem pointer to local version of the memory

  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::initialize( EqfSharedMemoryPlugin *pMemoryPlugin, char *pszMemoryName, OtmMemory *pLocalMem )
{
  int iRC = 0;

  this->pLocalMemory = pLocalMem;
  this->pMemoryPlugin = pMemoryPlugin;
  this->strMemoryName = pszMemoryName;

  // load memory properties
  iRC = pMemoryPlugin->loadProperties( pszMemoryName, &(this->pProperties) );
  if ( iRC != 0 )
  {
    this->iLastError = ERROR_PROPERTYLOADFAILED;
    this->strLastError = "Could not load properties of shared memory " + this->strMemoryName;
  } /* end */     
  
  // create input and output queue
  if ( iRC == 0 )
  {
    iRC = this->inQueue.open( this->pProperties->szInQueueName, FifoQueue::OM_READ );
    if ( iRC != 0 )
    {
      this->iLastError = this->inQueue.getLastError( this->strLastError );
    } /* end */     
  } /* endif */     
  if ( iRC == 0 )
  {
    iRC = this->outQueue.open( this->pProperties->szOutQueueName, FifoQueue::OM_WRITE );
    if ( iRC != 0 )
    {
      this->iLastError = this->outQueue.getLastError( this->strLastError );
    } /* end */     
  } /* endif */     

  // cleanup in case of errors
  if ( iRC != 0 )
  {
    if ( this->pProperties != NULL ) UtlAlloc( (void **)&(this->pProperties), 0, 0, NOMSG );
    if ( this->inQueue.isOpen() ) this->inQueue.close();
    if ( this->outQueue.isOpen() ) this->outQueue.close();
  } /* endif */     

  return( iRC );
}

/*! \brief Terminates sharing of proposals and release all resources

  	\returns 0 or error code in case of errors
*/
int EqfSharedMemory::terminate()
{
  if ( this->pProperties != NULL ) UtlAlloc( (void **)&(this->pProperties), 0, 0, NOMSG );
  if ( this->inQueue.isOpen() ) this->inQueue.close();
  if ( this->outQueue.isOpen() ) this->outQueue.close();
  return( 0 );
}




/*! \brief Get plugin responsible for the local copy of this memory
  	\returns pointer to memory plugin object
*/
void *EqfSharedMemory::getLocalPlugin()
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getPlugin() );
  }
  else
  {
    return( NULL );
  } /* end */     
}

/*! \brief Get local memory object of this memory
  	\returns pointer to memory object
*/
OtmMemory *EqfSharedMemory::getLocalMemory()
{
  return( this->pLocalMemory );
}


/*! \brief Get number of proposals in thismemory
  	\returns number of proposals
*/
unsigned long EqfSharedMemory::getProposalNum()
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getProposalNum() );
  }
  else
  {
    return( 0 );
  } /* end */     
}

  /*! \brief Get overall file size of this memory
  	\returns size of memory in number of bytes
*/
unsigned long EqfSharedMemory::getFileSize()
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getFileSize() );
  }
  else
  {
    return( 0 );
  } /* end */     
}


/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
int EqfSharedMemory::getLastError
(
  std::string &strError
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getLastError( strError ) );
  }
  else
  {
    strError = "";
    return( 0 );
  } /* end */     
}


/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
int EqfSharedMemory::getLastError
(
  char *pszError,
  int iBufSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getLastError( pszError, iBufSize ) );
  }
  else
  {
    *pszError = 0;
    return( 0 );
  } /* end */     
}

/*! \brief Get number of different document names used in the memory
  	\returns number of markups used by the memory proposals or 0 if no document name information can be provided
*/
int EqfSharedMemory::getNumOfDocumentNames()
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getNumOfDocumentNames() );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get document name at position n [n = 0.. GetNumOfDocumentNames()-1]
    \param iPos position of document name
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
int EqfSharedMemory::getDocumentName
(
  int iPos,
  char *pszBuffer,
  int iSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getDocumentName( iPos, pszBuffer, iSize ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

  /*! \brief Get number of different document short names used in the memory
  	\returns number of document short names used by the memory proposals or 0 if no document short name information can be provided
*/
int EqfSharedMemory::getNumOfDocumentShortNames()
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getNumOfDocumentShortNames() );
  }
  else
  {
    return( 0 );
  } /* end */     
}


/*! \brief Get document name at position n [n = 0.. GetNumOfDocumentNames()-1]
    \param iPos position of document name
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
int EqfSharedMemory::getDocumentShortName
(
  int iPos,
  char *pszBuffer,
  int iSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getDocumentShortName( iPos, pszBuffer, iSize ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}


/*! \brief Get number of different languages used in the memory
  	\returns number of languages used by the memory proposals or 0 if no language information can be provided
*/
int EqfSharedMemory::getNumOfLanguages()
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getNumOfLanguages() );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get language at position n [n = 0.. GetNumOfLanguages()-1]
    \param iPos position of language
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
int EqfSharedMemory::getLanguage
(
  int iPos,
  char *pszBuffer,
  int iSize
)
{
  if ( this->pLocalMemory != NULL )
  {
    return( this->pLocalMemory->getLanguage( iPos, pszBuffer, iSize) );
  }
  else
  {
    return( 0 );
  } /* end */     
}



/*! \brief Send proposal to the output queue
  	\returns 0 when successful or error code 
*/
int EqfSharedMemory::sendProposalToOutQueue
( 
  EqfSharedMemory::eClientTask Task,
  OtmProposal &Proposal
)
{
  int iRC = 0;
  TMXFactory *pTMXFactory = TMXFactory::getInstance();
  std::string strTMXData;

  pTMXFactory->ProposalToTMX( Proposal, strTMXData );

  // write proposal to out queue
  iRC = this->outQueue.writeRecord( (void *)strTMXData.c_str(), strTMXData.length() + 1 );
  if ( iRC != 0 )
  {
    this->iLastError = this->outQueue.getLastError( this->strLastError );
  } /* end */     

  return( iRC );
}


/*! \brief read new proposal from in queue and store them in the local memory
  	\returns 0 when successful or error code 
*/
int EqfSharedMemory::readProposalsFromInQueue
( 
)
{
  int iRC = 0;

  TMXFactory *pTMXFactory = TMXFactory::getInstance();
  std::string strTMXData;
  //OtmProposal Proposal;
  std::vector<OtmProposal*> proposals;

  int iProposalCount = 3;
  while ( (iRC == 0) && iProposalCount && !this->inQueue.isEmpty() )
  {
    int iSize = this->inQueue.getRecordSize();
    char *pszRecordData = new( char[iSize] );
    iRC = this->inQueue.readRecord( (void *)pszRecordData, iSize );
    if ( iRC != 0 )
    {
      this->iLastError = this->inQueue.getLastError( this->strLastError );
    }
    else
    {
      strTMXData = pszRecordData;

      // must filter the first '\n', else the saxparser can't run
      std::size_t idx = strTMXData.find("\\n");
      if(idx != std::string::npos)
        strTMXData.erase(idx,2);

      iRC = pTMXFactory->TMX2Proposal( strTMXData, proposals );

      if ( iRC != 0) 
      {
        this->iLastError = pTMXFactory->getLastError( this->strLastError );
      }
      else if ( !proposals.empty())
      {
        // add propopsals to memory
        for(std::vector<OtmProposal*>::iterator iter=proposals.begin();
              iter != proposals.end();
              iter++)
        { 
           OtmProposal  *tempProposal = *iter;
           if(!tempProposal->isEmpty())
           {
             this->pLocalMemory->putProposal( *tempProposal );
           }
        }
       
      } /* endif */
    } /* endif */       
    iProposalCount--;
    delete []pszRecordData;
  } /* endwhile */     

  return( iRC );
}


