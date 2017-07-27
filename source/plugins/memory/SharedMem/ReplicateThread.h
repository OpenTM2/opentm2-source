/*
	Copyright (c) 1999-2017, International Business Machines Corporation and others. All rights reserved.
	Description: This file contains the class definition for the JSONize helper class
*/

#ifndef _TRANSPORTTHREAD_H_
#define _TRANSPORTTHREAD_H_
#include <string>
#include <vector>
#include <set>
#include <map>
#include "EqfSharedMemoryPlugin.h"
#include "core\PluginManager\PluginManager.h"
#include "..\EqfMemoryPlugin\EqfmemoryPlugin.h"
#include "..\EqfMemoryPlugin\EqfMemory.h"
#include "EqfSharedMemory.h"
#include "FifoQueue.h"

void startReplicateThread();
void stopReplicateThread();


struct CMemoryUpdateCounters
{

    CMemoryUpdateCounters() : memName(""),upCounterPath("") {}

    CMemoryUpdateCounters(std::string &propPath,std::string &mem) : memName(mem),upCounterPath(propPath + memName + ".UDC"),upcounter("0") {}

    void loadUpCounterProperty();
    void writeUpCounterProperty();
    void deleteUpCounterProperty();

	std::string memName;
	std::string upCounterPath;

    std::string upcounter;
    std::set<std::string> ownUpCounterSet;
};


struct ReplicateHelper
{
    void init();
    void listShareMemories(std::vector<std::string>&,std::string postfix=".SHP");
    void fixInconsistence();
    bool loadProperty(std::string &memName, std::shared_ptr<EqfSharedMemoryPlugin::SHAREDMEMPROP> &propPtr);
    OtmMemory *openMemory(PSZ name);
    void putProposal(OtmMemory *pMemory, std::string &strTmx);

    void upload(PSZ name,
		        CMemoryUpdateCounters &upMgr,
		        std::shared_ptr<EqfSharedMemoryPlugin::SHAREDMEMPROP> &);

    void download(PSZ name,
				  CMemoryUpdateCounters &upMgr,
				  std::shared_ptr<EqfSharedMemoryPlugin::SHAREDMEMPROP> &);

    //
    EqfSharedMemoryPlugin                     *sharedPlugin;
    EqfMemoryPlugin                           *plugin;
    
    std::shared_ptr<MemoryWebServiceClient>   pWebClient;
    std::map<std::string, std::string>        cacheInMem;
    std::string                               propPath;
    std::string                               header;
};

#endif
