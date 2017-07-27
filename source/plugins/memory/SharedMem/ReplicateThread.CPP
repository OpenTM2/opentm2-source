/*
	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#include "ReplicateThread.h"

#include "TmxFactory.h"
#include <memory>
#include <fstream>
#include <process.h>

static volatile bool            gStop;
static ReplicateHelper          gReplicateHelper;
static void ReplicateProc(void *pData);

void startReplicateThread()
{
	gStop = false;
    _beginthread(ReplicateProc, 0, NULL);
}

void stopReplicateThread()
{
    gStop = true;
	gReplicateHelper.fixInconsistence();
}

void ReplicateProc(void *pData)
{
	gReplicateHelper.init();
    //
    Sleep(30000);

	// begin loop
    while (!gStop)
    {
		// important check
		if(gReplicateHelper.sharedPlugin==NULL || gReplicateHelper.plugin==NULL)
		    break;

        std::vector<std::string> mems;
	    gReplicateHelper.listShareMemories(mems);

		if (mems.empty())
		{
			Sleep(30000);
			continue;
		}
        
		//TODO: schedule memories with priority
		for (auto iter = mems.begin(); iter != mems.end(); iter++)
		{
			// load counter property
			CMemoryUpdateCounters upMgr(gReplicateHelper.propPath,*iter);
			upMgr.loadUpCounterProperty();

			// property load
			std::shared_ptr<EqfSharedMemoryPlugin::SHAREDMEMPROP> prop;
			bool isOK = gReplicateHelper.loadProperty(*iter, prop);
            if(!isOK)
			    continue;

			//uploadg
			gReplicateHelper.upload(const_cast<char *>(iter->c_str()), upMgr, prop);
			Sleep(5000);

			//download
			gReplicateHelper.download(const_cast<char *>(iter->c_str()), upMgr, prop);
			Sleep(5000);
		}
        
		gReplicateHelper.fixInconsistence();
		//
		Sleep(5000);
    }

    gReplicateHelper.fixInconsistence();
	// stop runing
    _endthread();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//              ReplicateHelper                                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ReplicateHelper::init()
{
	sharedPlugin = dynamic_cast<EqfSharedMemoryPlugin *>(PluginManager::getInstance()->getPlugin("EqfSharedMemoryPlugin"));
	plugin = dynamic_cast<EqfMemoryPlugin *>(PluginManager::getInstance()->getPlugin("EqfMemoryPlugin"));

	propPath.resize(512);
	propPath = UtlMakeEQFPath(&(propPath[0]), NULC, PROPERTY_PATH, NULL);
	propPath += "\\";

	pWebClient = std::make_shared<MemoryWebServiceClient>("WebServiceInReplicators");

	header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><tmx version=\"1.4\"><header srclang=\"en-US\" datatype=\"plaintext\"></header><body>";
}

void ReplicateHelper::listShareMemories(std::vector<std::string> &shmms, std::string postfix)
{
    shmms.clear();
	WIN32_FIND_DATA FindData;
	HANDLE hDir = FindFirstFile((propPath+"*"+postfix).c_str(), &FindData);
	if (hDir != INVALID_HANDLE_VALUE)
	{
		BOOL fMoreFiles = TRUE;
		do
		{
			std::string temp(FindData.cFileName);
			std::size_t ix = temp.find(postfix);
			if (ix != std::string::npos)
				shmms.push_back(temp.substr(0, ix));
			fMoreFiles = FindNextFile(hDir, &FindData);
		} while (fMoreFiles);

		FindClose(hDir);
	}
}

void ReplicateHelper::fixInconsistence()
{
	std::vector<std::string> shps;
	listShareMemories(shps,".SHP");
    std::set<std::string> shpset(shps.begin(),shps.end());

	std::vector<std::string> udcs;
	listShareMemories(udcs,".UDC");
	for(auto iter=udcs.begin();iter!=udcs.end();iter++)
	{
		if(shpset.find(*iter) == shpset.end())
		{
			// no shp have udc, illegal
			std::string memname( iter->substr(0,iter->find(".UDC")) );
			CMemoryUpdateCounters temp(propPath,memname);
			temp.deleteUpCounterProperty();
		}
	}
    
}

bool  ReplicateHelper::loadProperty(std::string &memName, std::shared_ptr<EqfSharedMemoryPlugin::SHAREDMEMPROP> &propPtr)
{
	if (sharedPlugin == NULL)
		return false;

	EqfSharedMemoryPlugin::SHAREDMEMPROP prop;
	EqfSharedMemoryPlugin::PSHAREDMEMPROP pProp = &prop;
	int iRC = sharedPlugin->loadProperties(const_cast<char *>(memName.c_str()), &pProp);
	if(iRC)
	    return false;
	propPtr = std::make_shared<EqfSharedMemoryPlugin::SHAREDMEMPROP>(*pProp);
	return true;
}

OtmMemory *ReplicateHelper::openMemory(PSZ name)
{
	if (plugin == NULL)
		return NULL;

	// 0 means exclusive
	OtmMemory *pMemory = NULL;
	pMemory = plugin->openMemory(name, FALSE, NULL, 0);
	return pMemory;
}

void ReplicateHelper::putProposal(OtmMemory *pMemory, std::string &strTmx)
{
	if (pMemory == NULL)
		return;
	std::vector<OtmProposal *> proposals;
	TMXFactory::getInstance()->TMX2Proposal(strTmx, proposals);
	for (auto iter = proposals.begin(); iter != proposals.end(); iter++)
	{
		pMemory->putProposal(**iter);
	}
}

void ReplicateHelper::upload(PSZ name,
							CMemoryUpdateCounters &upMgr,
							std::shared_ptr<EqfSharedMemoryPlugin::SHAREDMEMPROP> &prop)
{
	if (pWebClient == NULL || sharedPlugin==NULL)
		return;
	//CReadFromSharedBuffer rb(name);
	CSharedBuffer4Thread *rb = sharedPlugin->getSyncBuffer(std::string(name));
	std::string strTmx;
	if(rb != NULL)
	    rb->read(strTmx);

	if (!strTmx.empty())
	{
		pWebClient->setEndpointUrl(prop->szWebServiceURL);
		std::string uploadCounter;
		int iRC = pWebClient->uploadProposal(name, prop->szUserID, prop->szPassword, header + strTmx + "</body></tmx>", uploadCounter);

		if (!uploadCounter.empty() && upMgr.ownUpCounterSet.find(uploadCounter) == upMgr.ownUpCounterSet.end())
		{
			upMgr.ownUpCounterSet.insert(uploadCounter);
			upMgr.writeUpCounterProperty();
		}
	}
}

void ReplicateHelper::download(PSZ name,
				               CMemoryUpdateCounters &upMgr,
							   std::shared_ptr<EqfSharedMemoryPlugin::SHAREDMEMPROP> &prop)
{
	if (pWebClient == NULL)
		return;

	std::string quename(name);
	CInQueue que(quename);
	bool bQueEmpty = que.isEmpty();

	std::string strTMXProposal = "";
	std::string ownUploadedCounter;
	pWebClient->setEndpointUrl(prop->szWebServiceURL);
	std::string newCounter = upMgr.upcounter;
	int iRC = pWebClient->downloadProposal(name,
							prop->szUserID,
							prop->szPassword,
							strTMXProposal,
							newCounter,
							upMgr.ownUpCounterSet);

	if (iRC == 0 && strTMXProposal.find("<tu ") != std::string::npos)
	{
		upMgr.upcounter = newCounter;
		cacheInMem[name] += strTMXProposal;
		upMgr.writeUpCounterProperty();
	}

	// no need to continue
	if (cacheInMem[name].empty() && bQueEmpty)
		return;

	OtmMemory *pMemory = openMemory(name);
	if (pMemory == NULL || !bQueEmpty)
	{
		if (cacheInMem[name].size() > 1024 * 1024)
		{
			que.write(cacheInMem[name]);
			cacheInMem[name].clear();
		}
	}

	if (pMemory != NULL)
	{
		if (bQueEmpty)
		{
			putProposal(pMemory, cacheInMem[name]);
			cacheInMem[name].clear();
		}
		else
		{
			std::string tmx;
			que.read(tmx);
			if (!tmx.empty())
				putProposal(pMemory, cacheInMem[name]);
		}

		plugin->closeMemory(pMemory);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//              CMemoryUpdateCounters                                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemoryUpdateCounters::loadUpCounterProperty()
{
    std::ifstream fin(upCounterPath.c_str());
	if (fin)
	{
	    // update counter
	    std::string ucLine;
	    if (getline(fin, ucLine))
	    {
		    upcounter = ucLine;
	    }

		// ownupgraded counter
		std::string ocLine;
		if (getline(fin, ocLine))
		{
			std::string strOwnUploaded = ocLine;
			// add counter into vector returned back
			std::string::size_type startOff = 0;
			std::string::size_type ix = std::string::npos;
			do
			{
				ix = strOwnUploaded.find_first_of(",", startOff);
				std::string temp = strOwnUploaded.substr(startOff, ix - startOff);

				if (atoi(temp.c_str()) > atoi(upcounter.c_str()))
					ownUpCounterSet.insert(temp);

				if (ix + 1 > strOwnUploaded.size())
					break;

				startOff = ix + 1;

			} while (ix != std::string::npos);
		}
		fin.close();
	}	
}

void CMemoryUpdateCounters::writeUpCounterProperty()
{
	//try best to keep right
	std::string shpname(upCounterPath);
	std::ifstream fin(shpname.replace(shpname.find(".UDC"),4,".SHP"));
	if(!fin)
	{
		deleteUpCounterProperty();
		return;
	}
    fin.close();

	// write
    std::ofstream fout(upCounterPath.c_str());
    if(fout)
    {
		fout<<upcounter<<std::endl;

		std::string strTemp;
		for(auto iter=ownUpCounterSet.begin();
			iter!=ownUpCounterSet.end();
			iter++)
		{
			strTemp += *iter;
			strTemp += ",";
		}

		if(strTemp.size()>0)
		{
			strTemp.erase(strTemp.size()-1);
		}
	  
		fout<<strTemp<<std::endl;
		fout.close();
    }
}

void CMemoryUpdateCounters::deleteUpCounterProperty()
{
	std::ifstream fin(upCounterPath.c_str());
	if (fin)
	{
		fin.close();
        remove(upCounterPath.c_str());
	}
}
