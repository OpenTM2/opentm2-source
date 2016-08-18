//+----------------------------------------------------------------------------+
//|OtmXmlParser.h     OTM Auto Version Up function                             |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Flora Lee                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during auto version up                                  |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

// the Win32 Xerces build requires the default structure packing
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include "OtmLogWriterLoc.h"

XERCES_CPP_NAMESPACE_USE;

#define LOG_AUTO_VER_UP_PARSER_NAME                   "AutoVerUpXmlParser"

// keyword of xml
#define KEY_COMPONENT                                 "component"
#define KEY_ATTRI_NAME                                "name"
#define KEY_VERSION                                   "version"
#define KEY_DATE                                      "availabledate"
#define KEY_LONG_DSCP                                 "longdescription"
#define KEY_SHORT_DSCP                                "description"
#define KEY_SEVERITY                                  "Severity"
#define KEY_IMPACT                                    "Impact"
#define KEY_AFTERACTION                               "AfterAction"
#define KEY_INSTALL                                   "install"
#define KEY_COPY                                      "copy"
#define KEY_ATTRI_FROM                                "from"
#define KEY_ATTRI_TO                                  "to"
#define KEY_DOWNLOAD                                  "download"
#define KEY_ATTRI_DLTYPE                              "type"
#define KEY_ATTRI_METHOD                              "method"
#define KEY_ATTRI_RESTART                             "restart"
#define KEY_ATTRI_NEED_WAIT                           "needwait"
#define KEY_ARRTI_TRUE_STR                            "true"
#define KEY_FIXPACKS                                  "Fixpacks"
#define KEY_FIXPACK                                   "Fixpack"
#define KEY_ATTRI_ID                                  "id"

// error code define
#define PLUGIN_PARSER_ERR                             10

// len define
#define COMP_SHORT_DSCP_LEN                           256
#define COMP_LONG_DSCP_LEN                            20000
#define COMP_SEV_MAX_LEN                              10
#define COMP_IMP_MAX_LEN                              256
#define COMP_AFTIST_MAX_LEN                           256
#define COMP_MAX_VER_LEN                              50

typedef struct _COMPONENT
{
    char strId[MAX_LEN];
    char strDLUrl[MAX_BUF_SIZE];
    char strDLType[MAX_DL_TYPE_LEN];
    char strMethod[MAX_METHOD_LEN];
    char strFrom[MAX_PATH];
    char strTo[MAX_PATH];
    BOOL bRestart;
    BOOL bNeedWait;
} COMPONENT, * PCOMPONENT;

typedef struct _OTMCOMPONENT
{
    char strName[MAX_LEN];
    char strVersion[COMP_MAX_VER_LEN];
    char strDate[MAX_DATE_LEN];
    char strShortDscp[COMP_SHORT_DSCP_LEN];
    char strLongDscp[COMP_LONG_DSCP_LEN];
    char strSeverity[COMP_SEV_MAX_LEN];
    char strImpact[COMP_IMP_MAX_LEN];
    char strAfterAction[COMP_AFTIST_MAX_LEN];
    COMPONENT component;
    vector <FIXPACK> fixPacks;
} OTMCOMPONENT, *POTMCOMPONENT;

class COtmXmlLocParser
{
public:
    COtmXmlLocParser(void);
    ~COtmXmlLocParser(void);
    int XmlParser(const char * strXml);
    int GetComponentsCnt();
    int GetComponentsTotalCnt();
    int GetComponentCntByName(char * strName);
    char * GetComponentName(int nInx);
    char * GetComponentShortDscp(int nInx);
    char * GetComponentLongDscp(int nInx);
    char * GetComponentSeverity(int nInx);
    char * GetComponentImpact(int nInx);
    char * GetComponentAfterAction(int nInx);
    char * GetComponentVersion(int nInx);
    char * GetComponentDate(int nInx);
    char * GetComponentDLUrl(int nInx);
    char * GetComponentDLType(int nInx);
    char * GetComponentMethod(int nInx);
    BOOL GetComponentRestart(int nInx);
    BOOL GetComponentNeedWait(int nInx);
    int GetCompFixpacksCnt(int nInx);
    char * GetCompFixpackId(int iInx, int jInx);
    char * GetCompFixpackDLUrl(int iInx, int jInx);
    char * GetCompFixpackDLType(int iInx, int jInx);
    char * GetCompFixpackMethod(int iInx, int jInx);
    char * GetCompFixpackDate(int iInx, int jInx);
    char * GetCompFixpackShortDscp(int iInx, int jInx);
    char * GetCompFixpackLongDscp(int iInx, int jInx);
    BOOL GetCompFixpackRestart(int iInx, int jInx);
    BOOL GetCompFixpackNeedWait(int iInx, int jInx);
    int GetCompCopiesByName(const char * strComp, const char * strVer, PCOTMCOPIES pCompCopies);
    int GetComponentPos(PMAINTHREADINFO pMainTdInfo);
    void ClearAllComponents();

private:
    LogWriter m_logAutoVerUpParser;
    BOOL m_bLogOpened;

private:
    void parseFromRoot(DOMNode* elementRoot);
    void parseFromNode(DOMNode* elementNode, POTMCOMPONENT pOneComponent);
    void parseFixpacks(DOMNode* elementNode, POTMCOMPONENT pOneComponent);
    bool GetURLDataBycurl(const char* URL,  string &content);
    void InitComponent(POTMCOMPONENT pOneComponent);

    vector <OTMCOMPONENT> m_gComponents;
};
