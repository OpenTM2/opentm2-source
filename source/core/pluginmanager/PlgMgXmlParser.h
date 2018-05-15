//+----------------------------------------------------------------------------+
//|EQFPLGMG.CPP     OTM  Plugin Manager Parser function                        |
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
//|                    during plugin manager parser                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#pragma once
//// the Win32 Xerces build requires the default structure packing...
//#pragma pack(push, TM2StructPacking, 1)
//#pragma comment(linker,"\"/manifestdependency:type='win32' \
//name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
//processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/TransService.hpp"
#include "xercesc/dom/DOM.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/framework/LocalFileFormatTarget.hpp"

#include "core\utilities\LogWriter.h"
#include "core\PluginManager\PluginManager.h"

#include "core\PluginManager\OtmHttp.h"
#include "core\PluginManager\OtmHttps.h"
#include "core\PluginManager\OtmSftp.h"
#include "core\PluginManager\OtmSftpConfig.h"
#include "core\PluginManager\OtmPlgMgGUIComm.h"

XERCES_CPP_NAMESPACE_USE
#define DOT_STR                                       "."

#define LOG_PLUGIN_MGR_PARSER_NAME                    "PluginMgrXmlParser"

// keyword of xml
#define KEY_PLUGINS                                   "plugins"
#define KEY_PLUGIN                                    "plugin"
#define KEY_ATTRI_NAME                                "name"
#define KEY_VERSION                                   "version"
#define KEY_LONG_DSCP                                 "longdescription"
#define KEY_SHORT_DSCP                                "description"
#define KEY_DATE                                      "availabledate"
#define KEY_INSTALL                                   "install"
#define KEY_TYPE                                      "type"
#define KEY_SEVERITY                                  "Severity"
#define KEY_IMPACT                                    "Impact"
#define KEY_AFTERACTION                               "AfterAction"
#define KEY_MAINPART                                  "mainpart"
#define KEY_COPY                                      "copy"
#define KEY_ATTRI_FROM                                "from"
#define KEY_ATTRI_TO                                  "to"
#define KEY_DOWNLOAD                                  "download"
#define KEY_ATTRI_DLTYPE                              "type"
#define KEY_ATTRI_VER                                 "ver"
#define KEY_ATTRI_METHOD                              "method"
#define KEY_ATTRI_RESTART                             "restart"
#define KEY_ATTRI_NEED_WAIT                           "needwait"
#define KEY_ARRTI_TRUE_STR                            "true"
#define KEY_OTM_VER                                   "OtmVersion"
#define KEY_DEPENDENCIES                              "dependencies"
#define KEY_DEPENDENCY                                "dependency"

#define PLUGIN_PARSER_ERR                             10
#define PLUGIN_NAME_MAX_LEN                           100
#define PLUGIN_VER_MAX_LEN                            10
#define PLUGIN_TYP_MAX_LEN                            30
#define PLUGIN_SEV_MAX_LEN                            10
#define PLUGIN_IMP_MAX_LEN                            256
#define PLUGIN_AFTIST_MAX_LEN                         256
#define PLUGIN_DL_TYPE_MAX_LEN                        10
#define PLUGIN_DL_METHOD_MAX_LEN                      20
#define PLUGIN_SHT_DSCP_MAX_LEN                       256
#define PLUGIN_LNG_DSCP_MAX_LEN                       20000
#define MAX_PROXY_ADDRESS_LEN                         50
#define MAX_PROXY_PORT_LEN                            10
#define MIN_PLUGIN_CNT_DEF                            0
#define OTM_VER_LEN                                   80

typedef struct _CPLUGINMAIN
{
    char strVersion[PLUGIN_VER_MAX_LEN];
    char strDLUrl[MAX_PATH];
    char strDLType[PLUGIN_DL_TYPE_MAX_LEN];
    char strMethod[PLUGIN_DL_METHOD_MAX_LEN];
    char strDate[MAX_DATE_LEN];
    BOOL bRestart;
    BOOL bNeedWait;
    COTMCOPIES pluginCopies;
    vector <FIXPACK> fixPacks;

} CPLUGINMAIN, * PCPLUGINMAIN;

typedef struct _CPLUGINDEPN
{
    char strName[PLUGIN_NAME_MAX_LEN];
    char strDLUrl[MAX_PATH];
    char strDLType[PLUGIN_DL_TYPE_MAX_LEN];
    char strFrom[MAX_PATH];
    char strTo[MAX_PATH];

} CPLUGINDEPN, * PCPLUGINDEPN;

typedef struct _CPLUGIN
{
    char strName[PLUGIN_NAME_MAX_LEN];
    char strVersion[PLUGIN_VER_MAX_LEN];
    char strType[PLUGIN_TYP_MAX_LEN];
    char strShortDscp[PLUGIN_SHT_DSCP_MAX_LEN];
    char strLongDscp[PLUGIN_LNG_DSCP_MAX_LEN];
    char strSeverity[PLUGIN_SEV_MAX_LEN];
    char strImpact[PLUGIN_IMP_MAX_LEN];
    char strAfterAction[PLUGIN_AFTIST_MAX_LEN];

	// WLP P403853
	string strOtmVersion;

    vector <CPLUGINMAIN> pluginMains;
    vector <CPLUGINDEPN> pluginDepends;

} CPLUGIN, *PCPLUGIN;

class __declspec(dllexport) CPlgMgXmlParser
{
public:
    CPlgMgXmlParser(void);
    ~CPlgMgXmlParser(void);

    int TestConnection(const char * strConfigPath);
    int XmlParser(char * strConfigPath, char * strXml, BOOL bDownload);

    int GetPluginsCnt();
    int GetPluginsTotalCnt();
    int GetPluginCntByName(char * strName);
    char * GetPluginName(int nInx);
    char * GetPluginType(int nInx);
    char * GetPluginDscp(int nInx);
    char * GetPluginLongDscp(int nInx);
    char * GetPluginSeverity(int nInx);
    char * GetPluginImpact(int nInx);
    char * GetPluginAfterAction(int nInx);
    int GetPluginMainCnt(int nInx);
    char * GetMainVersion(int iInx, int jInx);
    
	//WLP P403853
	//string&  GetOtmVersion(int iInx);
	const char* CPlgMgXmlParser::GetOtmVersionByName(const char* pPlugginName);

    char * GetMainDLUrl(int iInx, int jInx);
    char * GetMainDLType(int iInx, int jInx);
    char * GetMainMethod(int iInx, int jInx);
    char * GetMainDate(int iInx, int jInx);
    BOOL GetMainNeedRestart(int iInx, int jInx);
    BOOL GetMainNeedWait(int iInx, int jInx);
    int GetMainFixpacksCnt(int iInx, int jInx);
    char * GetMainFixpackId(int iInx, int jInx, int kInx);
    char * GetMainFixpackDLUrl(int iInx, int jInx, int kInx);
    char * GetMainFixpackDLType(int iInx, int jInx, int kInx);
    char * GetMainFixpackMethod(int iInx, int jInx, int kInx);
    char * GetMainFixpackDate(int iInx, int jInx, int kInx);
    char * GetMainFixpackShortDscp(int iInx, int jInx, int kInx);
    char * GetMainFixpackLongDscp(int iInx, int jInx, int kInx);
    BOOL GetMainFixpackNeedRestart(int iInx, int jInx, int kInx);
    BOOL GetMainFixpackNeedWait(int iInx, int jInx, int kInx);

    int GetPluginDepnsCnt(int nInx);
    char * GetPluginDepnName(int iInx, int jInx);
    char * GetPluginDepnDL(int iInx, int jInx);

    int GetPluginPos(PMAINTHREADINFO pMainTdInfo);
    int GetPluginCopies(PMAINTHREADINFO pMainTdInfo, PCOTMCOPIES pPluginCopies);
    void ClearAllPlugins();

    int GetPluginPaths(const char * strXml, const char * strPluginName, PCOTMCOPIES pDefPathCopies, POTMGRPSTING pgrpPluginPath);

    int WriteToLocalXml(char * strXml, char * strPluginName, POTMGRPSTING pgrpPluginPath);

    // not use
    void ReleasePlugin(int nInx);

private:
    LogWriter m_logPlgMgXmlParser;
    BOOL m_bLogOpened;

    void OtmParseFromRoot(DOMNode* nodeOtmRoot);
    void OtmParseFromNode(DOMNode* elementNode, PCPLUGIN pOnePlugin);
    void ParseFixpacks(DOMNode* elementNode, PCPLUGINMAIN pOnePluginMain);
    int DownloadXml(char * strConfigPath, char * strXml);
    bool CurlInit(CURL *&curl, const char* url,string &content);
    bool GetURLDataBycurl(const char* URL,  string &content);
    void InitPlugin(PCPLUGIN pOnePlugin);
    void InitPluginMain(PCPLUGINMAIN pOnePluginMain);
    void InitPluginDepn(PCPLUGINDEPN pOnePluginDepn);

    // functions for get plugin path
    int OtmCompareModuleName(char * strModule1, char * strModule2);
    int SearchPluginPaths(XercesDOMParser* pParser, const char * strPluginName, PCOTMCOPIES pDefPathCopies, POTMGRPSTING pgrpPluginPath);
    int SearchCopyToValues(DOMNode* startNode, const char * strPluginName, PCOTMCOPIES pDefPathCopies, POTMGRPSTING pgrpPluginPath);
    int GetCopyAttriValues(DOMNode* strartNode, const char * strPluginName, PCOTMCOPIES pDefPathCopies, POTMGRPSTING pgrpValues);

    // not use
    int OtmAppendPluginNodeChild(XercesDOMParser* pOtmDomParser, char * strPluginName, POTMGRPSTING pgrpPluginPath);
    int OtmAppendMPNodeChild(XercesDOMParser* pOtmDomParser, DOMNode * otmNode, POTMGRPSTING pgrpPluginPath);
    int OtmAppendModuleNodeChild(XercesDOMParser* pOtmDomParser, DOMNode * otmNode, POTMGRPSTING pgrpPluginPath);

    vector <CPLUGIN> m_gPlugins;
};
