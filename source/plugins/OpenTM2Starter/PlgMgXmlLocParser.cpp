//+----------------------------------------------------------------------------+
//|EQFPLGMG.CPP     OTM  Plugin Manager Parser function                        |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
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

#include "PlgMgXmlLocParser.h"

int CPlgMgXmlLocParser::XmlParser(const char * strXml)
{
    int nRC = NO_ERROR;
#ifdef _DEBUG
    m_logPlgMgXmlParser.writef(LOG_PLUGIN_MGR_LOC_XML_NAME, "Parse start %s", strXml);
#endif

    // check whether the xml exists or not
    if (OTM_NOT_FOUND == access(strXml, 0))
    {
#ifdef _DEBUG
        m_logPlgMgXmlParser.writef(LOG_PLUGIN_MGR_LOC_XML_NAME, "Error: not find the xml file %s.", strXml);
#endif
        nRC = ERROR_OTM_FILE_NOT_FIND_A;
        return nRC;
    }

    ClearAllPlugins();

    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch)
    {
#ifdef _DEBUG
        m_logPlgMgXmlParser.writef(LOG_PLUGIN_MGR_LOC_XML_NAME, "Error=%d, %s", ERROR_OTM_XERCESC_INITIAL_A, toCatch.getMessage());
#endif
        return ERROR_OTM_XERCESC_INITIAL_A;
    }

    XercesDOMParser* otmParser;
    otmParser = new XercesDOMParser;
    otmParser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    otmParser->setDoNamespaces(false);
    otmParser->setDoSchema(false);
    otmParser->setLoadExternalDTD(false);
    otmParser->parse(strXml);

    xercesc::DOMDocument* docOtmPlugin = otmParser->getDocument();
    DOMElement* elementOtmPluginRoot = docOtmPlugin->getDocumentElement();

    OtmParseFromRoot(elementOtmPluginRoot);

    XMLPlatformUtils::Terminate();

#ifdef _DEBUG
    m_logPlgMgXmlParser.writef(LOG_PLUGIN_MGR_LOC_XML_NAME, "Parse end");
#endif
    return nRC;
}

void CPlgMgXmlLocParser::OtmParseFromRoot(DOMNode* elementOtmRoot)
{
    DOMNodeList* childNodeList = elementOtmRoot->getChildNodes();
    XMLSize_t nodeChildCnt = childNodeList->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeChildCnt; nInx++)
    {
        CPLUGIN onePlugin;
        InitPlugin(&onePlugin);
        DOMNode* childNode = childNodeList->item(nInx);
        if (childNode->getNodeType() && (childNode->getNodeType() == DOMNode::ELEMENT_NODE))
        {
            OtmParseFromNode(childNode, &onePlugin);
            m_gPlugins.push_back(onePlugin);
        }
    }
#ifdef _DEBUG
    m_logPlgMgXmlParser.writef(LOG_PLUGIN_MGR_LOC_XML_NAME, "Parse from root end.");
#endif
}

void CPlgMgXmlLocParser::OtmParseFromNode(DOMNode* elementNode, PCPLUGIN pOnePlugin)
{
    char * strValue, * strName;
    DOMNode* childNode = NULL;

    char * nodeName = XMLString::transcode(elementNode->getNodeName());

    if (!stricmp(KEY_PLUGIN, nodeName))
    {
        if (!elementNode->hasAttributes())
        {
            return;
        }

        for (int iInx = 0; iInx < (int) elementNode->getAttributes()->getLength(); iInx++)
        {
            strName = XMLString::transcode(elementNode->getAttributes()->item(iInx)->getNodeName());
            strValue = XMLString::transcode(elementNode->getAttributes()->item(iInx)->getNodeValue());
            if (!stricmp(KEY_ATTRI_NAME, strName))
            {
                int nLen = min(sizeof(pOnePlugin->strName), strlen(strValue));
                strncpy(pOnePlugin->strName, strValue, nLen);
                pOnePlugin->strName[nLen] = EOS;
            }
        }
    }
    else if (!stricmp(KEY_VERSION, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOnePlugin->strVersion), strlen(strValue));
        strncpy(pOnePlugin->strVersion, strValue, nLen);
        pOnePlugin->strVersion[nLen] = EOS;
    }
    else if (!stricmp(KEY_LONG_DSCP, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        BOOL bStart = TRUE;

        char strTempValue[PLUGIN_LNG_DSCP_MAX_LEN];
        memset(strTempValue, 0x00, sizeof(strTempValue));

        DOMNode * childNode = elementNode->getFirstChild();
        while(childNode != 0)
        {
            strValue = XMLString::transcode(childNode->getNodeValue());
            // work with child
            if (bStart)
            {
                int nLen = min(sizeof(strTempValue), strlen(strValue));
                strncpy(strTempValue, strValue, nLen);
                bStart = FALSE;
            }
            else
            {
                if (NULL == strValue)
                {
                    int nLen = (int) strlen(strTempValue);
                    strTempValue[nLen] = '\r';
                    strTempValue[nLen+1] = '\n';
                }
                else
                {
                    sprintf(strTempValue, "%s%s", strTempValue, strValue);
                }
            }
            //pickup next child
            childNode   = childNode->getNextSibling();
        }

        int nLen = min(sizeof(pOnePlugin->strLongDscp), strlen(strTempValue));
        strncpy(pOnePlugin->strLongDscp, strTempValue, nLen);
        pOnePlugin->strLongDscp[nLen] = EOS;
    }
    else if (!stricmp(KEY_SHORT_DSCP, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOnePlugin->strShortDscp), strlen(strValue));
        strncpy(pOnePlugin->strShortDscp, strValue, nLen);
        pOnePlugin->strShortDscp[nLen] = EOS;
    }
    else if (!stricmp(KEY_TYPE, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOnePlugin->strType), strlen(strValue));
        strncpy(pOnePlugin->strType, strValue, nLen);
        pOnePlugin->strType[nLen] = EOS;
    }
    else if (!stricmp(KEY_SEVERITY, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOnePlugin->strSeverity), strlen(strValue));
        strncpy(pOnePlugin->strSeverity, strValue, nLen);
        pOnePlugin->strSeverity[nLen] = EOS;
    }
    else if (!stricmp(KEY_IMPACT, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOnePlugin->strImpact), strlen(strValue));
        strncpy(pOnePlugin->strImpact, strValue, nLen);
        pOnePlugin->strImpact[nLen] = EOS;
    }
    else if (!stricmp(KEY_AFTERACTION, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOnePlugin->strAfterAction), strlen(strValue));
        strncpy(pOnePlugin->strAfterAction, strValue, nLen);
        pOnePlugin->strAfterAction[nLen] = EOS;
    }
    else if (!stricmp(KEY_MAINPART, nodeName))
    {
        // set the version info of plugin if it have
        CPLUGINMAIN onePluginMain;
        InitPluginMain(&onePluginMain);

        if (elementNode->hasAttributes())
        {
            // process attributes
            for (int iInx = 0; iInx < (int) elementNode->getAttributes()->getLength(); iInx++)
            {
                strName = XMLString::transcode(elementNode->getAttributes()->item(iInx)->getNodeName());
                strValue = XMLString::transcode(elementNode->getAttributes()->item(iInx)->getNodeValue());
                if (!stricmp(KEY_ATTRI_VER, strName))
                {
                    int nLen = min(sizeof(onePluginMain.strVersion), strlen(strValue));
                    strncpy(onePluginMain.strVersion, strValue, nLen);
                    onePluginMain.strVersion[nLen] = EOS;
                }
            }
        }
        else
        {
            // if no attribute of version, read from summary version
            int nLen = min(sizeof(onePluginMain.strVersion), strlen(pOnePlugin->strVersion));
            strncpy(onePluginMain.strVersion, pOnePlugin->strVersion, nLen);
            onePluginMain.strVersion[nLen] = EOS;
        }

        DOMNodeList* childList = elementNode->getChildNodes();
        XMLSize_t nChildCnt = childList->getLength();

        for (XMLSize_t nInx = 0; nInx < nChildCnt; nInx++)
        {
            childNode = childList->item(nInx);
            nodeName = XMLString::transcode(childNode->getNodeName());

            // set copy attribute
            if (!stricmp(KEY_COPY, nodeName))
            {
                // Initialize
                COTMCOPY pluginCopy;
                InitOtmCopy(&pluginCopy);

                if (!childNode->hasAttributes())
                {
                    continue;
                }

                for (int iInx = 0; iInx < (int) childNode->getAttributes()->getLength(); iInx++)
                {
                    strName = XMLString::transcode(childNode->getAttributes()->item(iInx)->getNodeName());
                    strValue = XMLString::transcode(childNode->getAttributes()->item(iInx)->getNodeValue());
                    if (!stricmp(KEY_ATTRI_FROM, strName))
                    {
                        int nLen = min(sizeof(pluginCopy.strFrom), strlen(strValue));
                        strncpy(pluginCopy.strFrom, strValue, nLen);
                        pluginCopy.strFrom[nLen] = EOS;
                    }
                    else if (!stricmp(KEY_ATTRI_TO, strName))
                    {
                        int nLen = min(sizeof(pluginCopy.strTo), strlen(strValue));
                        strncpy(pluginCopy.strTo, strValue, nLen);
                        pluginCopy.strTo[nLen] = EOS;
                    }
                }
                onePluginMain.pluginCopies.push_back(pluginCopy);
            }
            else if (!stricmp(KEY_DOWNLOAD, nodeName))
            {
                strValue = XMLString::transcode(childNode->getFirstChild()->getNodeValue());
                int nLen = min(sizeof(onePluginMain.strDLUrl), strlen(strValue));
                strncpy(onePluginMain.strDLUrl, strValue, nLen);
                onePluginMain.strDLUrl[nLen] = EOS;

                if (!childNode->hasAttributes())
                {
                    continue;
                }

                for (int iInx = 0; iInx < (int) childNode->getAttributes()->getLength(); iInx++)
                {
                    strName   = XMLString::transcode(childNode->getAttributes()->item(iInx)->getNodeName());
                    strValue  = XMLString::transcode(childNode->getAttributes()->item(iInx)->getNodeValue());
                    if (!stricmp(KEY_ATTRI_DLTYPE, strName))
                    {
                        int nLen = min(sizeof(onePluginMain.strDLType), strlen(strValue));
                        strncpy(onePluginMain.strDLType, strValue, nLen);
                        onePluginMain.strDLType[nLen] = EOS;
                    }
                    else if (!stricmp(KEY_ATTRI_METHOD, strName))
                    {
                        int nLen = min(sizeof(onePluginMain.strMethod), strlen(strValue));
                        strncpy(onePluginMain.strMethod, strValue, nLen);
                        onePluginMain.strMethod[nLen] = EOS;
                    }
                    else if (!stricmp(KEY_ATTRI_RESTART, strName))
                    {
                        if (!stricmp(KEY_ARRTI_TRUE_STR, strValue))
                        {
                            onePluginMain.bRestart = TRUE;
                        }
                        else
                        {
                            onePluginMain.bRestart = FALSE;
                        }
                    }
                }
            }
            else if (!stricmp(KEY_DATE, nodeName))
            {
                strValue = XMLString::transcode(childNode->getFirstChild()->getNodeValue());
                int nLen = min(sizeof(onePluginMain.strDate), strlen(strValue));
                strncpy(onePluginMain.strDate, strValue, nLen);
                onePluginMain.strDate[nLen] = EOS;
            }
            else if (!stricmp(KEY_FIXPACKS, nodeName))
            {
                ParseFixpacks(childNode, &onePluginMain);
            }
        }

        pOnePlugin->pluginMains.push_back(onePluginMain);
        elementNode = childNode;
    }
    else if (!stricmp(KEY_DEPENDENCIES, nodeName))
    {
        DOMNodeList* childList = elementNode->getChildNodes();
        XMLSize_t nChildCnt = childList->getLength();

        for (XMLSize_t nInx = 0; nInx < nChildCnt; nInx++)
        {
            childNode = childList->item(nInx);
            nodeName = XMLString::transcode(childNode->getNodeName());

            if (!stricmp(KEY_DEPENDENCY, nodeName))
            {
                CPLUGINDEPN pluginDepn;
                InitPluginDepn(&pluginDepn);

                // process the attributes of the dependency
                if (!childNode->hasAttributes())
                {
                    continue;
                }

                for (int iInx = 0; iInx < (int) childNode->getAttributes()->getLength(); iInx++)
                {
                    strName = XMLString::transcode(childNode->getAttributes()->item(iInx)->getNodeName());
                    strValue = XMLString::transcode(childNode->getAttributes()->item(iInx)->getNodeValue());
                    if (!stricmp(KEY_ATTRI_NAME, strName))
                    {
                        int nLen = min(sizeof(pluginDepn.strName), strlen(strValue));
                        strncpy(pluginDepn.strName, strValue, nLen);
                        pluginDepn.strName[nLen] = EOS;
                    }
                }

                DOMNodeList* subChildList = childNode->getChildNodes();
                XMLSize_t nSubChildCnt = subChildList->getLength();

                // process the child of the dependency
                for (XMLSize_t nInx = 0; nInx < nSubChildCnt; nInx++)
                {
                    DOMNode* subChildNode = subChildList->item(nInx);
                    nodeName = XMLString::transcode(subChildNode->getNodeName());

                    if (!stricmp(KEY_DOWNLOAD, nodeName))
                    {
                        strValue = XMLString::transcode(subChildNode->getFirstChild()->getNodeValue());
                        int nLen = min(sizeof(pluginDepn.strDLUrl), strlen(strValue));
                        strncpy(pluginDepn.strDLUrl, strValue, nLen);
                        pluginDepn.strDLUrl[nLen] = EOS;
                    }
                    else if (!stricmp(KEY_COPY, nodeName))
                    {
                        if (!childNode->hasAttributes())
                        {
                            continue;
                        }

                        for (int iInx = 0; iInx < (int) childNode->getAttributes()->getLength(); iInx++)
                        {
                            strName = XMLString::transcode(subChildNode->getAttributes()->item(iInx)->getNodeName());
                            strValue = XMLString::transcode(subChildNode->getAttributes()->item(iInx)->getNodeValue());
                            if (!stricmp(KEY_ATTRI_FROM, strName))
                            {
                                int nLen = min(sizeof(pluginDepn.strFrom), strlen(strValue));
                                strncpy(pluginDepn.strFrom, strValue, nLen);
                                pluginDepn.strFrom[nLen] = EOS;
                            }
                            else if (!stricmp(KEY_ATTRI_TO, strName))
                            {
                                int nLen = min(sizeof(pluginDepn.strTo), strlen(strValue));
                                strncpy(pluginDepn.strTo, strValue, nLen);
                                pluginDepn.strTo[nLen] = EOS;
                            }
                        }
                    }
                }
                // add one depedency
                pOnePlugin->pluginDepends.push_back(pluginDepn);
            }
            elementNode = childNode;
        }
    }

    DOMNodeList* children = elementNode->getChildNodes();
    XMLSize_t nChildCnt = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nChildCnt; nInx++)
    {
        DOMNode* currentNode = children->item(nInx);
        OtmParseFromNode(currentNode, pOnePlugin);
    }
}

int CPlgMgXmlLocParser::GetPluginsCnt()
{
    return m_gPlugins.size();
}

int CPlgMgXmlLocParser::GetPluginsTotalCnt()
{
    int nTotal = 0;
    for (int iInx = 0; m_gPlugins.size(); iInx++)
    {
        nTotal++;
        for (int jInx = 0; m_gPlugins[iInx].pluginDepends.size(); jInx++)
        {
            nTotal++;
        }
    }
    return nTotal;
}


int CPlgMgXmlLocParser::GetPluginCopies(PMAINTHREADINFO pMainTdInfo, PCOTMCOPIES pPluginCopies)
{
    int nRC = NO_ERROR;

    int iInx = pMainTdInfo->iInx;
    int jInx = pMainTdInfo->jInx;

    if (!pMainTdInfo->bFixpack)
    {
        for (int xInx = 0; xInx < (int) m_gPlugins[iInx].pluginMains[jInx].pluginCopies.size(); xInx++)
        {
            COTMCOPY pluginCopy;
            InitOtmCopy(&pluginCopy);

            strcpy(pluginCopy.strFrom,      m_gPlugins[iInx].pluginMains[jInx].pluginCopies[xInx].strFrom);
            strcpy(pluginCopy.strTo,        m_gPlugins[iInx].pluginMains[jInx].pluginCopies[xInx].strTo);

            pPluginCopies->push_back(pluginCopy);
        }
    }
    else
    {
        int kInx = pMainTdInfo->kInx;
        for (int xInx = 0; xInx < (int) m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].fixpackCopies.size(); xInx++)
        {
            COTMCOPY compCopy;
            InitOtmCopy(&compCopy);

            strcpy(compCopy.strFrom,      m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].fixpackCopies[xInx].strFrom);
            strcpy(compCopy.strTo,        m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].fixpackCopies[xInx].strTo);

            pPluginCopies->push_back(compCopy);
        }
    }

    return nRC;
}

char * CPlgMgXmlLocParser::GetPluginName(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strName;
}

char * CPlgMgXmlLocParser::GetPluginType(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strType;
}

char * CPlgMgXmlLocParser::GetPluginDscp(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strShortDscp;
}

char * CPlgMgXmlLocParser::GetPluginLongDscp(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strLongDscp;
}

char * CPlgMgXmlLocParser::GetPluginSeverity(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strSeverity;
}

char * CPlgMgXmlLocParser::GetPluginImpact(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strImpact;
}

char * CPlgMgXmlLocParser::GetPluginAfterAction(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strAfterAction;
}

int CPlgMgXmlLocParser::GetPluginMainCnt(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }
    return m_gPlugins[nInx].pluginMains.size();
}

char * CPlgMgXmlLocParser::GetMainVersion(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].strVersion;
}

char * CPlgMgXmlLocParser::GetMainDate(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].strDate;
}

char * CPlgMgXmlLocParser::GetMainDLUrl(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].strDLUrl;
}

char * CPlgMgXmlLocParser::GetMainDLType(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].strDLType;
}

char * CPlgMgXmlLocParser::GetMainMethod(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].strMethod;
}

BOOL CPlgMgXmlLocParser::GetMainNeedRestart(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].bRestart;
}

int CPlgMgXmlLocParser::GetPluginDepnsCnt(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return 0;
    }
    return m_gPlugins[nInx].pluginDepends.size();
}

char * CPlgMgXmlLocParser::GetPluginDepnName(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginDepnsCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginDepends[jInx].strName;
}

char * CPlgMgXmlLocParser::GetPluginDepnDL(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginDepnsCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginDepends[jInx].strDLUrl;
}

void CPlgMgXmlLocParser::ClearAllPlugins()
{
    m_gPlugins.clear();
}

void CPlgMgXmlLocParser::InitPlugin(PCPLUGIN pOnePlugin)
{
    // do some initilize
    memset(pOnePlugin->strName,                 0x00, sizeof(pOnePlugin->strName));
    memset(pOnePlugin->strType,                 0x00, sizeof(pOnePlugin->strType));
    memset(pOnePlugin->strLongDscp,             0x00, sizeof(pOnePlugin->strLongDscp));
    memset(pOnePlugin->strShortDscp,            0x00, sizeof(pOnePlugin->strShortDscp));
    memset(pOnePlugin->strVersion,              0x00, sizeof(pOnePlugin->strVersion));
    memset(pOnePlugin->strSeverity,             0x00, sizeof(pOnePlugin->strSeverity));
    memset(pOnePlugin->strImpact,               0x00, sizeof(pOnePlugin->strImpact));
    memset(pOnePlugin->strAfterAction,          0x00, sizeof(pOnePlugin->strAfterAction));

    pOnePlugin->pluginMains.clear();
    pOnePlugin->pluginDepends.clear();
}

void CPlgMgXmlLocParser::InitPluginMain(PCPLUGINMAIN pOnePluginMain)
{
    // do some initilize
    pOnePluginMain->bRestart = FALSE;
    memset(pOnePluginMain->strVersion,  0x00, sizeof(pOnePluginMain->strVersion));
    memset(pOnePluginMain->strDLType,   0x00, sizeof(pOnePluginMain->strDLType));
    memset(pOnePluginMain->strDLUrl,    0x00, sizeof(pOnePluginMain->strDLUrl));
    memset(pOnePluginMain->strMethod,   0x00, sizeof(pOnePluginMain->strMethod));
    memset(pOnePluginMain->strDate,     0x00, sizeof(pOnePluginMain->strDate));

    pOnePluginMain->pluginCopies.clear();
    pOnePluginMain->fixPacks.clear();
}

void CPlgMgXmlLocParser::InitPluginDepn(PCPLUGINDEPN pOnePluginDepn)
{
    // do some initilize
    memset(pOnePluginDepn->strName,   0x00, sizeof(pOnePluginDepn->strName));
    memset(pOnePluginDepn->strDLType, 0x00, sizeof(pOnePluginDepn->strDLType));
    memset(pOnePluginDepn->strDLUrl,  0x00, sizeof(pOnePluginDepn->strDLUrl));
    memset(pOnePluginDepn->strFrom,   0x00, sizeof(pOnePluginDepn->strFrom));
    memset(pOnePluginDepn->strTo,     0x00, sizeof(pOnePluginDepn->strTo));
}

void CPlgMgXmlLocParser::ParseFixpacks(DOMNode* elementNode, PCPLUGINMAIN pOnePluginMain)
{
    char * strValue, * strName;

    DOMNodeList* childNode = elementNode->getChildNodes();
    XMLSize_t nodeCount = childNode->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        DOMNode* currentNode = childNode->item(nInx);
        char * nodeName = XMLString::transcode(currentNode->getNodeName());

        // one fix pack
        if (!stricmp(KEY_FIXPACK, nodeName))
        {
            // check whether has attributes first
            if (!currentNode->hasAttributes())
            {
                continue;
            }

            FIXPACK fixPack;
            // Do initialize
            InitFixpack(&fixPack);

            // get attribute
            BOOL bNullId = FALSE;
            for (XMLSize_t iInx = 0; iInx < currentNode->getAttributes()->getLength(); iInx++)
            {
                strName = XMLString::transcode(currentNode->getAttributes()->item(iInx)->getNodeName());
                strValue = XMLString::transcode(currentNode->getAttributes()->item(iInx)->getNodeValue());

                if (!stricmp(KEY_ATTRI_ID, strName))
                {
                    if (strlen(strValue) == 0)
                    {
                        bNullId = TRUE;
                        break;
                    }
                    int nLen = min(sizeof(fixPack.strId), strlen(strValue));
                    strncpy(fixPack.strId, strValue, nLen);
                    fixPack.strId[nLen] = EOS;
                }
            }

            // if id value is null, not include this fixpack
            if (bNullId)
            {
                continue;
            }

            // get sub children
            DOMNodeList* subChildNode = currentNode->getChildNodes();
            XMLSize_t nodeSubCount = subChildNode->getLength();
            for (XMLSize_t iInx = 0; iInx < nodeSubCount; iInx++)
            {
                DOMNode* currentSubNode = subChildNode->item(iInx);
                nodeName = XMLString::transcode(currentSubNode->getNodeName());

                // get copies
                if (!stricmp(KEY_DOWNLOAD, nodeName))
                {
                    // check whether has attributes first
                    if (!currentSubNode->hasAttributes())
                    {
                        continue;
                    }
                    for (XMLSize_t iSubInx = 0; iSubInx < currentSubNode->getAttributes()->getLength(); iSubInx++)
                    {
                        strName = XMLString::transcode(currentSubNode->getAttributes()->item(iSubInx)->getNodeName());
                        strValue = XMLString::transcode(currentSubNode->getAttributes()->item(iSubInx)->getNodeValue());
                        if (!stricmp(KEY_ATTRI_DLTYPE, strName))
                        {
                            int nLen = min(sizeof(fixPack.strDLType), strlen(strValue));
                            strncpy(fixPack.strDLType, strValue, nLen);
                            fixPack.strDLType[nLen] = EOS;
                        }
                        else if (!stricmp(KEY_ATTRI_METHOD, strName))
                        {
                            int nLen = min(sizeof(fixPack.strMethod), strlen(strValue));
                            strncpy(fixPack.strMethod, strValue, nLen);
                            fixPack.strMethod[nLen] = EOS;
                        }
                        else if (!stricmp(KEY_ATTRI_RESTART, strName))
                        {
                            if (!stricmp(KEY_ARRTI_TRUE_STR, strValue))
                            {
                                fixPack.bRestart = TRUE;
                            }
                            else
                            {
                                fixPack.bRestart = FALSE;
                            }
                        }
                    }

                    strValue = XMLString::transcode(currentSubNode->getFirstChild()->getNodeValue());
                    int nLen = min(sizeof(fixPack.strDLUrl), strlen(strValue));
                    strncpy(fixPack.strDLUrl, strValue, nLen);
                    fixPack.strDLUrl[nLen] = EOS;
                }
                else if (!stricmp(KEY_DATE, nodeName))
                {
                    strValue = XMLString::transcode(currentSubNode->getFirstChild()->getNodeValue());
                    int nLen = min(sizeof(fixPack.strDate), strlen(strValue));
                    strncpy(fixPack.strDate, strValue, nLen);
                    fixPack.strDate[nLen] = EOS;
                }
                else if (!stricmp(KEY_SHORT_DSCP, nodeName))
                {
                    strValue = XMLString::transcode(currentSubNode->getFirstChild()->getNodeValue());
                    int nLen = min(sizeof(fixPack.strShortDscp), strlen(strValue));
                    strncpy(fixPack.strShortDscp, strValue, nLen);
                    fixPack.strShortDscp[nLen] = EOS;
                }
                else if (!stricmp(KEY_LONG_DSCP, nodeName))
                {
                    BOOL bStart = TRUE;

                    char strTempValue[MAX_LONG_DSCP_LEN];
                    memset(strTempValue, 0x00, sizeof(strTempValue));

                    DOMNode * childCurSubNode = currentSubNode->getFirstChild();
                    while(childCurSubNode != 0)
                    {
                        strValue = XMLString::transcode(childCurSubNode->getNodeValue());
                        // work with child
                        if (bStart)
                        {
                            int nLen = min(sizeof(strTempValue), strlen(strValue));
                            strncpy(strTempValue, strValue, nLen);
                            bStart = FALSE;
                        }
                        else
                        {
                            if (NULL == strValue)
                            {
                                int nLen = (int) strlen(strTempValue);
                                strTempValue[nLen] = '\r';
                                strTempValue[nLen+1] = '\n';
                            }
                            else
                            {
                                sprintf(strTempValue, "%s%s", strTempValue, strValue);
                            }
                        }
                        //pickup next child
                        childCurSubNode   = childCurSubNode->getNextSibling();
                    }

                    int nLen = min(sizeof(fixPack.strLongDscp), strlen(strTempValue));
                    strncpy(fixPack.strLongDscp, strTempValue, nLen);
                    fixPack.strLongDscp[nLen] = EOS;
                }
                else if (!stricmp(KEY_COPY, nodeName))
                {
                    // check whether has attribute
                    if (!currentSubNode->hasAttributes())
                    {
                        continue;
                    }

                    // Initialize
                    COTMCOPY fixpackCopy;
                    InitOtmCopy(&fixpackCopy);

                    for (XMLSize_t iSubInx = 0; iSubInx < currentSubNode->getAttributes()->getLength(); iSubInx++)
                    {
                        strName = XMLString::transcode(currentSubNode->getAttributes()->item(iSubInx)->getNodeName());
                        strValue = XMLString::transcode(currentSubNode->getAttributes()->item(iSubInx)->getNodeValue());
                        if (!stricmp(KEY_ATTRI_FROM, strName))
                        {
                            int nLen = min(sizeof(fixpackCopy.strFrom), strlen(strValue));
                            strncpy(fixpackCopy.strFrom, strValue, nLen);
                            fixpackCopy.strFrom[nLen] = EOS;
                        }
                        else if (!stricmp(KEY_ATTRI_TO, strName))
                        {
                            int nLen = min(sizeof(fixpackCopy.strTo), strlen(strValue));
                            strncpy(fixpackCopy.strTo, strValue, nLen);
                            fixpackCopy.strTo[nLen] = EOS;
                        }
                    }
                    fixPack.fixpackCopies.push_back(fixpackCopy);
                }
            }

            // Add the fixpack info to vector
            pOnePluginMain->fixPacks.push_back(fixPack);
        }
    }
}

int CPlgMgXmlLocParser::GetMainFixpacksCnt(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return 0;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return 0;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks.size();
}

char * CPlgMgXmlLocParser::GetMainFixpackId(int iInx, int jInx, int kInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    if ((kInx >= GetMainFixpacksCnt(iInx, jInx)) || (kInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].strId;
}

char * CPlgMgXmlLocParser::GetMainFixpackDLUrl(int iInx, int jInx, int kInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    if ((kInx >= GetMainFixpacksCnt(iInx, jInx)) || (kInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].strDLUrl;
}

char * CPlgMgXmlLocParser::GetMainFixpackDLType(int iInx, int jInx, int kInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    if ((kInx >= GetMainFixpacksCnt(iInx, jInx)) || (kInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].strDLType;
}

char * CPlgMgXmlLocParser::GetMainFixpackMethod(int iInx, int jInx, int kInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    if ((kInx >= GetMainFixpacksCnt(iInx, jInx)) || (kInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].strMethod;
}

char * CPlgMgXmlLocParser::GetMainFixpackShortDscp(int iInx, int jInx, int kInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    if ((kInx >= GetMainFixpacksCnt(iInx, jInx)) || (kInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].strShortDscp;
}

BOOL CPlgMgXmlLocParser::GetMainFixpackNeedRestart(int iInx, int jInx, int kInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    if ((kInx >= GetMainFixpacksCnt(iInx, jInx)) || (kInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].bRestart;
}

char * CPlgMgXmlLocParser::GetMainFixpackDate(int iInx, int jInx, int kInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    if ((kInx >= GetMainFixpacksCnt(iInx, jInx)) || (kInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].strDate;
}

char * CPlgMgXmlLocParser::GetMainFixpackLongDscp(int iInx, int jInx, int kInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    if ((kInx >= GetMainFixpacksCnt(iInx, jInx)) || (kInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].fixPacks[kInx].strLongDscp;
}

int CPlgMgXmlLocParser::GetPluginPos(PMAINTHREADINFO pMainTdInfo)
{
    int nRC = ERROR_PLUGIN_NOT_FOUND_A;

    for (int iInx = 0; iInx < GetPluginsCnt(); iInx++)
    {
        // find the same plugin name
        if (stricmp(pMainTdInfo->strName, GetPluginName(iInx)))
        {
            continue;
        }

        for (int jInx = 0; jInx < GetPluginMainCnt(iInx); jInx++)
        {
            // then find the same version
            if (stricmp(pMainTdInfo->strVer, GetMainVersion(iInx, jInx)))
            {
                continue;
            }

            // set iInx and jInx
            pMainTdInfo->iInx = iInx;
            pMainTdInfo->jInx = jInx;

            // if not fixpack, all value has been set, just return
            if (!pMainTdInfo->bFixpack)
            {
                nRC = NO_ERROR;
                return nRC;
            }

            // if fixpack, set kInx
            for (int kInx = 0; kInx < GetMainFixpacksCnt(iInx, jInx); kInx++)
            {
                // find the same id info
                if (stricmp(pMainTdInfo->strFixpkId, GetMainFixpackId(iInx, jInx, kInx)))
                {
                    continue;
                }

                // if find set the kInx and just return
                pMainTdInfo->kInx = kInx;
                nRC = NO_ERROR;
                return nRC;
            }
        }
    }

    return nRC;
}

CPlgMgXmlLocParser::CPlgMgXmlLocParser(void)
{
}

CPlgMgXmlLocParser::~CPlgMgXmlLocParser(void)
{
}
