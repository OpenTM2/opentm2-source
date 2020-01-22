//+----------------------------------------------------------------------------+
//| OtmMemoryTool.h                                                            |
//+----------------------------------------------------------------------------+
//| Copyright (c) 2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: liping                                                             |
//|(based on OtmCmm,OtmDmm, OtmTmCl, OtmRecM, OtmTmT by G.Queck)               |
//+----------------------------------------------------------------------------+
//|  This include file                                                         |
//+----------------------------------------------------------------------------+
#ifndef _OTMMEMORYTOOL_H_
#define _OTMMEMORYTOOL_H_

#include <string>
#include <vector>
#include <memory>
#include <wtypes.h>

#include <eqf.h>
#include <eqfdde.h>
#include "OtmProposal.h"
#include "core\pluginmanager\OtmMemory.h"

#define TASK_NUM 5
#define OTMMEMORYTOOL_CLASS     "OtmMemoryToolClass"       // class name
#define OTMMEMORYTOOL_SPACESTRING "                                     "
#define OTMMEMORYTOOL_XPOS      20               // position on the screen
#define OTMMEMORYTOOL_YPOS      50               // position on the screen
#define OTMMEMORYTOOL_YDELTA    30               // difference in line position

#define OTMMEMORYTOOL_PROGRESS_LEN   75          // number of dots...
#define OTMMEMORYTOOL_PROGRESS_STR   "."         // progress string...

// status
#define OTMMEMORYTOOL_STAT_WORKING   0
#define OTMMEMORYTOOL_STAT_CLEANUP   1



const std::string Tasks[TASK_NUM] = {
    "DeleteMtProposal",
    "ReverseMemory",
    "DeleteIdentical",
    "ChgProposalMeta"
};

class OtmMemoryTool
{
public:
    // Constructor
    OtmMemoryTool():m_bFirstCall(false),m_bNoConfirm(false),m_ulSegNo(0)
    {
    }

    // members are non-virtual, it's better to keep them from overloading
    int open();
    bool close();
    bool executeTask();
    bool checkCommandLine(std::vector<std::string>& commandVec);
    BATCHCMD validateToken(const std::string& token, std::string& cmdParam);

    // virtual members
    virtual ~OtmMemoryTool()
    {
    }

    virtual int openTask( OtmMemory * )
    { 
        return 0;
    }

    virtual bool closeTask()
    {
        return true;
    }

    // pure virtual
    virtual bool doExecute(OtmProposal& proposal) = 0;
    virtual void getInfoDisplay(std::vector<std::string> & vec) = 0;
    virtual bool checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd)=0;
    virtual PCMDLIST getCommandList() = 0;

public:
    void setConfirmFlag(bool confirmFlag)
    {
        m_bNoConfirm = confirmFlag;
    }

    bool getConfirmFlag() const
    {
        return m_bNoConfirm;
    }

    unsigned long getSegNo() const
    {
        return m_ulSegNo;
    }

    std::string getLastErrorMsg() const 
    {
        return m_strLastError;
    }

public:
    static const int  MAX_RC_STRING = 256;

protected:
    std::shared_ptr<OtmMemory>    m_pMem;
    std::string                   m_strMemName;
    std::string                   m_strLastError;

private:
    bool                          m_bFirstCall;
    bool                          m_bNoConfirm;
    unsigned long                 m_ulSegNo;

    CMDLIST                       m_pCmdList[3];
    
};


// begin define sub-class 

/*******************************************************************************************************/
/*                                          ReverseMemory                                              */
/*******************************************************************************************************/
class ReverseMemory : public OtmMemoryTool
{
public:
    ReverseMemory():m_ulSuccSegNo(0),m_bFlag(false)
    {
        initCommandList();
    }

    int  openTask( OtmMemory * );
    bool closeTask();
    bool doExecute(OtmProposal& proposal);
    bool checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd);
    void getInfoDisplay(std::vector<std::string> & vec);

    PCMDLIST getCommandList()
    {
        return m_pCmdList;
    }
    

private:
    void initCommandList();
private:
    std::string                   m_strTgtMemName;
    std::shared_ptr<OtmMemory>    m_pTgtMem;
    unsigned long                 m_ulSuccSegNo;
    bool                          m_bFlag;
    CMDLIST                       m_pCmdList[4];

};


/*******************************************************************************************************/
/*                                                 DeleteMtProposal                                    */
/*******************************************************************************************************/

class DeleteMtProposal : public OtmMemoryTool
{
public:
    DeleteMtProposal():m_ulDelSegNo(0)
    {
        initCommandList();
    }

    // reimplement
    bool doExecute(OtmProposal& proposal);
    bool checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd);
    void getInfoDisplay(std::vector<std::string> & vec);

    PCMDLIST getCommandList()
    {
        return m_pCmdList;
    }

private:
    void initCommandList();
private:
    CMDLIST                       m_pCmdList[3];
    unsigned long                 m_ulDelSegNo;
};


/*******************************************************************************************************/
/*                                        DeleteIdenticalProposal                                      */
/*******************************************************************************************************/
class DeleteIdenticalProposal : public OtmMemoryTool
{
public:
    DeleteIdenticalProposal():m_ulSuccSegNo(0),m_ulSkippedSegNo(0)
    {
        initCommandList();
    }

    int  openTask( OtmMemory * );
    bool closeTask();
    bool doExecute(OtmProposal& proposal);
    bool checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd);
    void getInfoDisplay(std::vector<std::string> & vec);

    PCMDLIST getCommandList()
    {
        return m_pCmdList;
    }
    

private:
    void initCommandList();
    void TmClNormalize(wchar_t*,wchar_t*);
private:
    std::string                   m_strTgtMemName;
    std::shared_ptr<OtmMemory>    m_pTgtMem;
    unsigned long                 m_ulSuccSegNo;
    unsigned long                 m_ulSkippedSegNo;
    CMDLIST                       m_pCmdList[4];

};

/*******************************************************************************************************/
/*                                        ChgProposalMeta                                              */
/*******************************************************************************************************/
class ChgProposalMeta : public OtmMemoryTool
{
public:
    ChgProposalMeta():m_lNewDate(0),m_ulMarkupSegs(0),m_ulLangSegs(0),m_ulSetSegNo(0),m_ulClearSegNo(0),
                      m_ulDateSegs(0),m_bSetMTFlag(false),m_bClearMTFlag(false)
    {
        initCommandList();
    }

    bool closeTask();
    bool doExecute(OtmProposal& proposal);
    bool checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd);
    void getInfoDisplay(std::vector<std::string> & vec);

    PCMDLIST getCommandList()
    {
        return m_pCmdList;
    }
    
private:
    void initCommandList();

private:
    std::string                   m_strFromMarkup;
    std::string                   m_strToMarkup;
    std::string                   m_strFromLang;
    std::string                   m_strToLang;
    std::string                   m_strDate;
    std::string                   m_strDoc;

    bool                          m_bSetMTFlag;
    bool                          m_bClearMTFlag;

    long                          m_lNewDate;
    unsigned long                 m_ulMarkupSegs;
    unsigned long                 m_ulLangSegs;
    unsigned long                 m_ulDateSegs;
    unsigned long                 m_ulSetSegNo;
    unsigned long                 m_ulClearSegNo;

    CMDLIST                       m_pCmdList[10];

};
#endif
