/*! \file
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMSHAREDMEMORY_H_
#define _OTMSHAREDMEMORY_H_

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
#include "OtmMemory.h"

/*! \brief Abstract base-class for translation memory objects */
class __declspec(dllexport) OtmSharedMemory : public OtmMemory
{

public:

/*! \brief Constructors */
	OtmSharedMemory() {};
	
 
/*! \brief Destructor */
	virtual ~OtmSharedMemory() {};

/*! \brief Get plugin responsible for the local copy of this memory
  	\returns pointer to memory plugin object
*/
virtual void *getLocalPlugin() = 0;

/*! \brief Get local memory object of this memory
  	\returns pointer to memory object
*/
virtual OtmMemory *getLocalMemory() = 0;

private:

};

#endif // #ifndef _OTMSHAREDMEMORY_H_
