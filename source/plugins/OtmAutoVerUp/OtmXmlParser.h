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

#include "core\pluginmanager\OtmHttp.h"
#include "core\pluginmanager\OtmSftp.h"
#include "core\pluginmanager\OtmHttps.h"
#include "core\pluginmanager\OtmSftpConfig.h"
#include "OtmAutoVerUpComm.h"

XERCES_CPP_NAMESPACE_USE;

#define LOG_AUTO_VER_UP_PARSER_NAME                   "AutoVerUpXmlParser"

// error code define
#define PLUGIN_PARSER_ERR                             10

// len define
#define COMP_SHORT_DSCP_LEN                           256
#define COMP_LONG_DSCP_LEN                            20000
#define COMP_SEV_MAX_LEN                              10
#define COMP_IMP_MAX_LEN                              256
#define COMP_AFTIST_MAX_LEN                           256

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
    char strVersion[MAX_VER_LEN];
    char strDate[MAX_DATE_LEN];
    char strShortDscp[COMP_SHORT_DSCP_LEN];
    char strLongDscp[COMP_LONG_DSCP_LEN];
    char strSeverity[COMP_SEV_MAX_LEN];
    char strImpact[COMP_IMP_MAX_LEN];
    char strAfterAction[COMP_AFTIST_MAX_LEN];
    COMPONENT component;
    vector <FIXPACK> fixPacks;
} OTMCOMPONENT, *POTMCOMPONENT;

class COtmXmlParser
{
public:
    COtmXmlParser(void);
    ~COtmXmlParser(void);
    int XmlParser(char * strConfigPath, char * strXml, BOOL bDownload);
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
    void ClearAllComponents();
    int TestConnection(const char * strConfigPath);

private:
    LogWriter m_logAutoVerUpParser;
    BOOL m_bLogOpened;

private:
    void parseFromRoot(DOMNode* elementRoot);
    void parseFromNode(DOMNode* elementNode, POTMCOMPONENT pOneComponent);
    void parseFixpacks(DOMNode* elementNode, POTMCOMPONENT pOneComponent);
    int DownloadXml(char * strConfigPath, char * strXml);
    bool CurlInit(CURL *&curl, const char* url,string &content);
    bool GetURLDataBycurl(const char* URL,  string &content);
    void InitComponent(POTMCOMPONENT pOneComponent);

    vector <OTMCOMPONENT> m_gComponents;
};
