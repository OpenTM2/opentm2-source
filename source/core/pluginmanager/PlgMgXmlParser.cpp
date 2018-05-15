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

#include "core\PluginManager\PlgMgXmlParser.h"

#include <memory>

int CPlgMgXmlParser::TestConnection(const char * strConfigPath)
{
    int nRC = NO_ERROR;
    char strUrl[MAX_BUF_SIZE];
    memset(strUrl, 0x00, sizeof(strUrl));
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_URL, EMPTY_STR, strUrl, sizeof(strUrl), strConfigPath);

    sprintf(strUrl, "%s/%s", strUrl, PLUGIN_MGR_XML);

    NETWORKPARAM networkParam;
    InitNetworkParam(&networkParam);

    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_ADDRESS, EMPTY_STR, networkParam.strProxyAddress, sizeof(networkParam.strProxyAddress), strConfigPath);
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_PORT, EMPTY_STR, networkParam.strProxyPort, sizeof(networkParam.strProxyPort), strConfigPath);
    networkParam.nTimeout = GetPrivateProfileInt(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_TIMEOUT, DEF_CONNECT_TIMEOUT, strConfigPath);
	
	// Deleted and replace it with smart pointer
    //COtmHttps * otmHttps = new COtmHttps();
	std::unique_ptr<COtmHttps> otmHttps( new COtmHttps() );

    char strSFTPConfig[MAX_PATH];
    memset(strSFTPConfig, 0x00, sizeof(strSFTPConfig));

    char strDrive[_MAX_DRIVE];
    char strDir[_MAX_DIR];
    char strFname[_MAX_FNAME];
    char strExt[_MAX_EXT];
    _splitpath(strConfigPath, strDrive, strDir, strFname, strExt);

    sprintf(strSFTPConfig, "%s%s%s", strDrive, strDir, PLUGIN_MGR_SFTP_INFO_CONF);

    m_logPlgMgXmlParser.writef("Start to test connection for HTTPs.");
    nRC = otmHttps->TestConnection(strUrl, &networkParam);
    m_logPlgMgXmlParser.writef("End test %d, %s", nRC, networkParam.strError);

    //delete otmHttps;
    if (nRC)
    {
        return nRC;
    }


    return nRC;
}

int CPlgMgXmlParser::XmlParser(char * strConfigPath, char * strXml, BOOL bDownload)
{
    int nRC = NO_ERROR;

    ClearAllPlugins();

    if (bDownload)
    {
        // download the xml config file first
        m_logPlgMgXmlParser.writef("Start download %s", strXml);
        nRC = DownloadXml(strConfigPath, strXml);
        m_logPlgMgXmlParser.writef("End download %s(%d)", strXml, nRC);

        if (nRC)
        {
            m_logPlgMgXmlParser.writef("Error=%d", nRC);
            return nRC;
        }
    }

    // check whether the xml exists or not
    if (OTM_NOT_FOUND == access(strXml, 0))
    {
        m_logPlgMgXmlParser.writef("Error: not find the xml file %s.", strXml);
        nRC = ERROR_OTM_FILE_NOT_FIND_A;
        return nRC;
    }

    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch)
    {
        char* message = XMLString::transcode(toCatch.getMessage());
        XMLString::release(&message);
        m_logPlgMgXmlParser.writef("Error=%d, %s", ERROR_OTM_XERCESC_INITIAL_A, toCatch.getMessage());
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

    if (NULL != elementOtmPluginRoot)
    {
        OtmParseFromRoot(elementOtmPluginRoot);
    }

    XMLPlatformUtils::Terminate();
    m_logPlgMgXmlParser.writef("Parese end (%d)", nRC);
    return nRC;
}

void CPlgMgXmlParser::OtmParseFromRoot(DOMNode* elementOtmRoot)
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
}

void CPlgMgXmlParser::ReleasePlugin(int nInx)
{
    OtmPlugin*         otmPlugin;
    PluginManager* thePluginManager = PluginManager::getInstance();
    otmPlugin = thePluginManager->getPlugin(nInx);

//    thePluginManager->deregisterPlugin((OtmPlugin*) otmPlugin);
}

void CPlgMgXmlParser::OtmParseFromNode(DOMNode* elementNode, PCPLUGIN pOnePlugin)
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
	else if (!stricmp(KEY_OTM_VER,nodeName))
	{
		// WLP P403853
		strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        pOnePlugin->strOtmVersion = strValue;
	}
    else if (!stricmp(KEY_LONG_DSCP, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        // Not to write to the config file now
        /*char strConfigPath[MAX_PATH];
        char strPluginPath[MAX_PATH];
        memset(strConfigPath, 0x00, sizeof(strConfigPath));
        memset(strPluginPath, 0x00, sizeof(strPluginPath));

        // get local path of plugin
        UtlQueryString(QST_PLUGINPATH, strPluginPath, sizeof(strPluginPath));
        UtlMakeEQFPath(strPluginPath, NULC, PLUGIN_PATH, NULL);
        if (NULL == strPluginPath)
        {
            return;
        }

        sprintf(strConfigPath, "%s\\%s", strPluginPath, PLUGIN_MGR_CONFIG);
        ::WritePrivateProfileString(pOnePlugin->strName, KEY_PLUGIN_LONG_DSCP, strValue, strConfigPath);*/
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
                    else if (!stricmp(KEY_ATTRI_NEED_WAIT, strName))
                    {
                        if (!stricmp(KEY_ARRTI_TRUE_STR, strValue))
                        {
                            onePluginMain.bNeedWait = TRUE;
                        }
                        else
                        {
                            onePluginMain.bNeedWait = FALSE;
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

int CPlgMgXmlParser::DownloadXml(char * strConfigPath, char * strXml)
{
    int nRC = NO_ERROR;
    char strUrl[MAX_BUF_SIZE];
    memset(strUrl, 0x00, sizeof(strUrl));
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_URL, EMPTY_STR, strUrl, sizeof(strUrl), strConfigPath);

    sprintf(strUrl, "%s/%s", strUrl, PLUGIN_MGR_XML);

    NETWORKPARAM networkParam;
    InitNetworkParam(&networkParam);

    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_ADDRESS, EMPTY_STR, networkParam.strProxyAddress, sizeof(networkParam.strProxyAddress), strConfigPath);
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_PORT, EMPTY_STR, networkParam.strProxyPort, sizeof(networkParam.strProxyPort), strConfigPath);
    networkParam.nTimeout = GetPrivateProfileInt(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_TIMEOUT, DEF_CONNECT_TIMEOUT, strConfigPath);
    memset(networkParam.strError, 0x00, sizeof(networkParam.strError));

    /*COtmHttp * otmHttp = new COtmHttp();
    nRC = otmHttp->DownloadFile(strUrl, strXml, strProxyAddress, strProxyPort);*/
    COtmHttps * otmHttps = new COtmHttps();


    m_logPlgMgXmlParser.writef("Start download URL");
    nRC = otmHttps->DownloadFile(strUrl, strXml, &networkParam);
    m_logPlgMgXmlParser.writef("End download %d, %s", nRC, networkParam.strError);

    delete otmHttps;
    if (nRC)
    {
        return nRC;
    }


    return nRC;
}

int CPlgMgXmlParser::GetPluginsCnt()
{
    return m_gPlugins.size();
}

int CPlgMgXmlParser::GetPluginsTotalCnt()
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

int CPlgMgXmlParser::GetPluginCntByName(char * strName)
{
    int nCnt = 0;

    // skip when name is null
    if (NULL == strName)
    {
        return nCnt;
    }

    // find the plugin
    for (int iInx = 0; m_gPlugins.size(); iInx++)
    {
        if (!stricmp(strName, m_gPlugins[iInx].strName))
        {
            nCnt++;
            /*for (int jInx =0; jInx < (int) m_gPlugins[iInx].pluginDepends.size(); jInx++)
            {
                nCnt++;
            }*/
            break;
        }
    }
    return nCnt;
}

int CPlgMgXmlParser::GetPluginCopies(PMAINTHREADINFO pMainTdInfo, PCOTMCOPIES pPluginCopies)
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

char * CPlgMgXmlParser::GetPluginName(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strName;
}

char * CPlgMgXmlParser::GetPluginType(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strType;
}

char * CPlgMgXmlParser::GetPluginDscp(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strShortDscp;
}

char * CPlgMgXmlParser::GetPluginLongDscp(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strLongDscp;
}

char * CPlgMgXmlParser::GetPluginSeverity(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strSeverity;
}

char * CPlgMgXmlParser::GetPluginImpact(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strImpact;
}

char * CPlgMgXmlParser::GetPluginAfterAction(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[nInx].strAfterAction;
}

int CPlgMgXmlParser::GetPluginMainCnt(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return NULL;
    }
    return m_gPlugins[nInx].pluginMains.size();
}

char * CPlgMgXmlParser::GetMainVersion(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].strVersion;
}

const char* CPlgMgXmlParser::GetOtmVersionByName(const char* pPlugginName)
{
	if (pPlugginName == NULL)
    {
        return NULL;
    }

	for(auto iter=m_gPlugins.begin(); iter!=m_gPlugins.end(); iter++)
	{
		if(stricmp(pPlugginName,iter->strName) == 0)
			return iter->strOtmVersion.c_str();
	}
	
	return NULL;
}

char * CPlgMgXmlParser::GetMainDate(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].strDate;
}

char * CPlgMgXmlParser::GetMainDLUrl(int iInx, int jInx)
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

char * CPlgMgXmlParser::GetMainDLType(int iInx, int jInx)
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

char * CPlgMgXmlParser::GetMainMethod(int iInx, int jInx)
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

BOOL CPlgMgXmlParser::GetMainNeedRestart(int iInx, int jInx)
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

BOOL CPlgMgXmlParser::GetMainNeedWait(int iInx, int jInx)
{
    if ((iInx >= GetPluginsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetPluginMainCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gPlugins[iInx].pluginMains[jInx].bNeedWait;
}

int CPlgMgXmlParser::GetPluginDepnsCnt(int nInx)
{
    if ((nInx >= GetPluginsCnt()) || (nInx < 0))
    {
        return 0;
    }
    return m_gPlugins[nInx].pluginDepends.size();
}

char * CPlgMgXmlParser::GetPluginDepnName(int iInx, int jInx)
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

char * CPlgMgXmlParser::GetPluginDepnDL(int iInx, int jInx)
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

void CPlgMgXmlParser::ClearAllPlugins()
{
    m_gPlugins.clear();
}

void CPlgMgXmlParser::InitPlugin(PCPLUGIN pOnePlugin)
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

void CPlgMgXmlParser::InitPluginMain(PCPLUGINMAIN pOnePluginMain)
{
    // do some initilize
    pOnePluginMain->bRestart  = FALSE;
    pOnePluginMain->bNeedWait = TRUE;
    memset(pOnePluginMain->strVersion,  0x00, sizeof(pOnePluginMain->strVersion));
    memset(pOnePluginMain->strDLType,   0x00, sizeof(pOnePluginMain->strDLType));
    memset(pOnePluginMain->strDLUrl,    0x00, sizeof(pOnePluginMain->strDLUrl));
    memset(pOnePluginMain->strMethod,   0x00, sizeof(pOnePluginMain->strMethod));
    memset(pOnePluginMain->strDate,     0x00, sizeof(pOnePluginMain->strDate));

    pOnePluginMain->pluginCopies.clear();
    pOnePluginMain->fixPacks.clear();
}

void CPlgMgXmlParser::InitPluginDepn(PCPLUGINDEPN pOnePluginDepn)
{
    // do some initilize
    memset(pOnePluginDepn->strName,   0x00, sizeof(pOnePluginDepn->strName));
    memset(pOnePluginDepn->strDLType, 0x00, sizeof(pOnePluginDepn->strDLType));
    memset(pOnePluginDepn->strDLUrl,  0x00, sizeof(pOnePluginDepn->strDLUrl));
    memset(pOnePluginDepn->strFrom,   0x00, sizeof(pOnePluginDepn->strFrom));
    memset(pOnePluginDepn->strTo,     0x00, sizeof(pOnePluginDepn->strTo));
}

int CPlgMgXmlParser::GetPluginPaths(const char * strXml, const char * strPluginName, PCOTMCOPIES pDefPathCopies, POTMGRPSTING pgrpPluginPath)
{
    int nRC = NO_ERROR;

    // judge whether the xlm exists or not
    if (-1 == access(strXml, 0))
    {
        nRC = ERROR_OTM_FILE_NOT_FIND_A;
        return nRC;
    }

    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch)
    {
        char* message = XMLString::transcode(toCatch.getMessage());
        m_logPlgMgXmlParser.writef("Error=%d, %s", ERROR_OTM_XERCESC_INITIAL_A, toCatch.getMessage());
        XMLString::release(&message);
        return ERROR_OTM_XERCESC_INITIAL_A;
    }

    XercesDOMParser* pOtmParserLocal;
    pOtmParserLocal = new XercesDOMParser;
    pOtmParserLocal->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    pOtmParserLocal->setDoNamespaces(false);
    pOtmParserLocal->setDoSchema(false);
    pOtmParserLocal->setLoadExternalDTD(false);
    pOtmParserLocal->parse(strXml);

    nRC = SearchPluginPaths(pOtmParserLocal, strPluginName, pDefPathCopies, pgrpPluginPath);

    XMLPlatformUtils::Terminate();
    return nRC;
}

int CPlgMgXmlParser::SearchPluginPaths(XercesDOMParser* pParser, const char * strPluginName, PCOTMCOPIES pDefPathCopies, POTMGRPSTING pgrpPluginPath)
{
    m_logPlgMgXmlParser.writef("Start to search plugin path %s", strPluginName);

    int nRC = NO_ERROR;
    xercesc::DOMDocument* docPluginXml = pParser->getDocument();
    DOMElement* elementPluginRoot = docPluginXml->getDocumentElement();
	if (NULL == elementPluginRoot)
    {
        m_logPlgMgXmlParser.writef("Error: empty xml.");
        nRC = ERROR_EMPTY_FILE_A;
        return nRC;
    }

    DOMNodeList * nodeListPlugin;
    try
    {
        nodeListPlugin = elementPluginRoot->getElementsByTagName(XMLString::transcode(KEY_PLUGIN));
    }
    catch (const XMLException& toCatch)
    {
        char* message = XMLString::transcode(toCatch.getMessage());
        m_logPlgMgXmlParser.writef("Error=%d, %s", ERROR_OTM_XERCESC_INITIAL_A, toCatch.getMessage());
        XMLString::release(&message);
        return ERROR_OTM_XERCESC_INITIAL_A;
    }

    XMLSize_t nodeListCnt = nodeListPlugin->getLength();
    if (0 == nodeListCnt)
    {
        m_logPlgMgXmlParser.writef("Error: empty xml.");
        nRC = ERROR_EMPTY_FILE_A;
        return nRC;
    }

    nRC = ERROR_CANNOT_FIND_KEY_C;
    for (XMLSize_t nInx = 0; nInx < nodeListCnt; nInx++)
    {
        DOMNode* childNode = nodeListPlugin->item(nInx);
        if (childNode->getNodeType() && (childNode->getNodeType() == DOMNode::ELEMENT_NODE))
        {
            if (!childNode->hasAttributes())
            {
                continue;
            }

            for (XMLSize_t nAttriInx = 0; nAttriInx < childNode->getAttributes()->getLength(); nAttriInx++)
            {
                char * strAttriName = XMLString::transcode(childNode->getAttributes()->item(nAttriInx)->getNodeName());
                if ((NULL == strAttriName) || stricmp(KEY_ATTRI_NAME, strAttriName))
                {
                    continue;
                }

                char * strAttriValue = XMLString::transcode(childNode->getAttributes()->item(nAttriInx)->getNodeValue());
                if ((NULL == strAttriName) || stricmp(strPluginName, strAttriValue))
                {
                    continue;
                }
                // check the name of plugin, which is the attribute
                nRC = SearchCopyToValues(childNode, strPluginName, pDefPathCopies, pgrpPluginPath);
                break;
            }
        }
    }

    return nRC;
}

int CPlgMgXmlParser::SearchCopyToValues(DOMNode* startNode, const char * strPluginName, PCOTMCOPIES pDefPathCopies, POTMGRPSTING pgrpPluginPath)
{
    int nRC = ERROR_CANNOT_FIND_KEY_C;

    DOMNodeList* childrenNodes = startNode->getChildNodes();
    XMLSize_t nodeCount = childrenNodes->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        DOMNode* childNode = childrenNodes->item(nInx);
        if (childNode->getNodeType() && (childNode->getNodeType() == DOMNode::ELEMENT_NODE))
        {
            char * strNodeName = XMLString::transcode(childNode->getNodeName());
            if (NULL == strNodeName)
            {
                continue;
            }
            else if (!stricmp(KEY_INSTALL, strNodeName))
            {
                nRC = SearchCopyToValues(childNode, strPluginName, pDefPathCopies, pgrpPluginPath);
            }
            else if (!stricmp(KEY_MAINPART, strNodeName))
            {
                nRC = SearchCopyToValues(childNode, strPluginName, pDefPathCopies, pgrpPluginPath);
            }
            else if (!stricmp(KEY_COPY, strNodeName))
            {
                nRC = GetCopyAttriValues(childNode, strPluginName, pDefPathCopies, pgrpPluginPath);
            }
            else
            {
                continue;
            }
        }
    }

    return nRC;
}

int CPlgMgXmlParser::GetCopyAttriValues(DOMNode* strartNode, const char * strPluginName, PCOTMCOPIES pDefPathCopies, POTMGRPSTING pgrpValues)
{
    int nRC = ERROR_CANNOT_FIND_KEY_C;

    if (!strartNode->hasAttributes())
    {
        m_logPlgMgXmlParser.writef("Copy has no attribute value");
        return nRC;
    }

    char strFilePath[MAX_PATH];
    char strFileName[MAX_PATH];

    memset(strFilePath, 0x00, sizeof(strFilePath));
    memset(strFileName, 0x00, sizeof(strFileName));

    DOMNamedNodeMap * childAttriNodeMap = strartNode->getAttributes();
    for (int iAttriInx = 0; iAttriInx < (int) childAttriNodeMap->getLength(); iAttriInx++)
    {
        char * strAttriName = XMLString::transcode(childAttriNodeMap->item(iAttriInx)->getNodeName());
        char * strAttriValue = XMLString::transcode(childAttriNodeMap->item(iAttriInx)->getNodeValue());
        if ((NULL != strAttriName) && !stricmp(KEY_ATTRI_TO, strAttriName))
        {
            if ((NULL != strAttriValue) && (strlen(strAttriValue) != 0))
            {
                nRC = NO_ERROR;
                char strNewTo[MAX_PATH];
                char strNewTo2[MAX_PATH];
                char strPath[MAX_PATH];
                memset(strNewTo, 0x00, sizeof(strNewTo));
                memset(strNewTo2, 0x00, sizeof(strNewTo2));
                memset(strPath,  0x00, sizeof(strPath));

                // 1. replace the default path
                for (int jInx = 0; (pDefPathCopies != NULL) && (jInx < (int) pDefPathCopies->size()); jInx++)
                {
                    char strTemp[MAX_PATH];
                    memset(strTemp, 0x00, sizeof(strTemp));

                    if (0 == jInx)
                    {
                        sprintf(strTemp, "%s", strAttriValue);
                    }
                    else
                    {
                        sprintf(strTemp, "%s", strNewTo);
                    }
                    ReplaceDefaultDir(strTemp, (*pDefPathCopies)[jInx].strFrom, (*pDefPathCopies)[jInx].strTo, strNewTo);
                }
                m_logPlgMgXmlParser.writef("Replace default=%s,%s", strAttriValue, strNewTo);
                ReplaceOriginalDir(strNewTo, strFilePath, strNewTo2);
                m_logPlgMgXmlParser.writef("Replace original=%s,%s,%s", strNewTo, strFilePath, strNewTo2);
                OtmJointPath(strPath, strNewTo2, strFileName);
                m_logPlgMgXmlParser.writef("The path of the plugin is %s", strPath);
                pgrpValues->push_back(strPath);
            }
        }
        else if ((NULL != strAttriName) && !stricmp(KEY_ATTRI_FROM, strAttriName))
        {
            if ((NULL != strAttriValue) && (strlen(strAttriValue) != 0))
            {
                OtmSplitPath(strAttriValue, strFilePath, strFileName);
            }
            else
            {
                strcpy(strFilePath, strPluginName);
            }
        }
    }

    return nRC;
}

int CPlgMgXmlParser::WriteToLocalXml(char * strXml, char * strPluginName, POTMGRPSTING pgrpPluginPath)
{
    int nRC = NO_ERROR;

    // judge whether the xlm exists or not
    if (-1 == access(strXml, 0))
    {
        nRC = ERROR_OTM_FILE_NOT_FIND_A;
        return nRC;
    }

    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch)
    {
        char* message = XMLString::transcode(toCatch.getMessage());
        m_logPlgMgXmlParser.writef("Error=%d, %s", ERROR_OTM_XERCESC_INITIAL_A, toCatch.getMessage());
        XMLString::release(&message);
        return ERROR_OTM_XERCESC_INITIAL_A;
    }

    XMLCh strValue[MAX_BUF_SIZE];
    memset(strValue, 0x00, sizeof(strValue));

    XercesDOMParser* pOtmParserLocal;
    pOtmParserLocal = new XercesDOMParser;
    pOtmParserLocal->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    pOtmParserLocal->setDoNamespaces(false);
    pOtmParserLocal->setDoSchema(false);
    pOtmParserLocal->setLoadExternalDTD(false);
    pOtmParserLocal->parse(strXml);

    nRC = OtmAppendPluginNodeChild(pOtmParserLocal, strPluginName, pgrpPluginPath);

    DOMImplementation * otmImpl   = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("core"));
    if (nRC)
    {
        XMLPlatformUtils::Terminate();
        return nRC;
    }

    DOMLSOutput       * otmOutput = ((DOMImplementationLS*)otmImpl)->createLSOutput();
    DOMLSSerializer   * otmSerial = ((DOMImplementationLS*)otmImpl)->createLSSerializer();

    if (otmSerial->getDomConfig()->canSetParameter(XMLString::transcode("format-pretty-print"), true))
    {
        otmSerial->getDomConfig()->setParameter(XMLString::transcode("format-pretty-print"), true);
    }

    XMLFormatTarget*             m_pFormatTarget;
    // construct the LocalFileFormatTarget
    m_pFormatTarget = new LocalFileFormatTarget(strXml);
    otmOutput->setByteStream(m_pFormatTarget);  // output to xml file
    xercesc::DOMDocument* docPluginLocalXml = pOtmParserLocal->getDocument();
    otmSerial->write(docPluginLocalXml, otmOutput);

    docPluginLocalXml->release();
    otmSerial->release();
    otmOutput->release();

    XMLPlatformUtils::Terminate();
    return nRC;
}

int CPlgMgXmlParser::OtmAppendPluginNodeChild(XercesDOMParser* pOtmDomParser, char * strPluginName, POTMGRPSTING pgrpPluginPath)
{
    int nRC = NO_ERROR;
    BOOL bFound = FALSE;

    xercesc::DOMDocument  * docPluginLocalXml = pOtmDomParser->getDocument();
    DOMElement* pRootElement = docPluginLocalXml->getDocumentElement();
    DOMNodeList           * otmNodeList       = docPluginLocalXml->getElementsByTagName(XMLString::transcode(KEY_PLUGIN));

    for (int iLstInx = 0; iLstInx < (int) otmNodeList->getLength(); iLstInx++)
    {
        if (!otmNodeList->item(iLstInx)->hasAttributes())
        {
            continue;
        }

        for (int iAttriInx = 0; iAttriInx < (int) otmNodeList->item(iLstInx)->getAttributes()->getLength(); iAttriInx++)
        {
            char * strName   = XMLString::transcode(otmNodeList->item(iLstInx)->getAttributes()->item(iAttriInx)->getNodeName());
            char * strValue  = XMLString::transcode(otmNodeList->item(iLstInx)->getAttributes()->item(iAttriInx)->getNodeValue());
            if (!stricmp(KEY_ATTRI_NAME, strName))
            {
                if (stricmp(strValue, strPluginName))
                {
                    continue;
                }

                bFound = TRUE;
                nRC = OtmAppendMPNodeChild(pOtmDomParser, otmNodeList->item(iLstInx), pgrpPluginPath);
            }
        }
    }

    if (!bFound)
    {
        // add a new plugin element
        DOMElement  * newDomElement;

        newDomElement = docPluginLocalXml->createElement(XMLString::transcode(KEY_PLUGIN));
        newDomElement->setAttribute(XMLString::transcode(KEY_ATTRI_NAME), XMLString::transcode(strPluginName));

        for (int iInx = 0; iInx < (int) pgrpPluginPath->size(); iInx++)
        {
            DOMElement  * newChildElement    = docPluginLocalXml->createElement(XMLString::transcode(KEY_MAINPART));
            DOMElement  * newChildSubElement = docPluginLocalXml->createElement(XMLString::transcode(KEY_COPY));
            // following has problem, not txtnode, should be attribute
            DOMText     * newDomText         = docPluginLocalXml->createTextNode(XMLString::transcode((*pgrpPluginPath)[iInx].c_str()));
            newChildSubElement->appendChild(newDomText);
            newChildElement->appendChild(newChildSubElement);
            newDomElement->appendChild(newChildElement);
        }
        pRootElement->appendChild(newDomElement);
    }
    return nRC;
}

int CPlgMgXmlParser::OtmAppendMPNodeChild(XercesDOMParser* pOtmDomParser, DOMNode * otmNode, POTMGRPSTING pgrpPluginPath)
{
    int nRC = NO_ERROR;

    DOMNodeList*  nodeChildLst = otmNode->getChildNodes();
    XMLSize_t nChildCnt = nodeChildLst->getLength();

    for (XMLSize_t nInx = 0; nInx < nChildCnt; nInx++)
    {
        DOMNode* childNode = nodeChildLst->item(nInx);
        if (childNode->getNodeType() && (childNode->getNodeType() == DOMNode::ELEMENT_NODE))
        {
            // check node name
            char * strNodeName = XMLString::transcode(childNode->getNodeName());
            if ((NULL == strNodeName) || stricmp(KEY_MAINPART, strNodeName))
            {
                continue;
            }

            OtmAppendModuleNodeChild(pOtmDomParser, childNode, pgrpPluginPath);
        }
    }
    return nRC;
}

int CPlgMgXmlParser::OtmAppendModuleNodeChild(XercesDOMParser* pOtmDomParser, DOMNode * otmNode, POTMGRPSTING pgrpPluginPath)
{
    int nRC = NO_ERROR;
    DOMNodeList*  nodeChildLst = otmNode->getChildNodes();
    XMLSize_t nChildCnt = nodeChildLst->getLength();

    for (XMLSize_t nInx = 0; nInx < nChildCnt; nInx++)
    {
        DOMNode* childNode = nodeChildLst->item(nInx);
        if (childNode->getNodeType() && (childNode->getNodeType() == DOMNode::ELEMENT_NODE))
        {
            // check node name
            char * strNodeName = XMLString::transcode(childNode->getNodeName());
            if ((NULL == strNodeName) || stricmp(KEY_COPY, strNodeName))
            {
                continue;
            }

            // check node value
            char * strNodeValue = XMLString::transcode(childNode->getFirstChild()->getNodeValue());
            for (int nInx = 0; nInx < (int) pgrpPluginPath->size(); nInx++)
            {
                nRC = OtmCompareModuleName(strNodeValue, (char *)(*pgrpPluginPath)[nInx].c_str());
                if (nRC)
                {
                    xercesc::DOMDocument* otmDocument = pOtmDomParser->getDocument();
                    DOMElement * newDomElement = otmDocument->createElement(XMLString::transcode(KEY_COPY));
                    DOMText    * newDomText    = otmDocument->createTextNode(XMLString::transcode((*pgrpPluginPath)[nInx].c_str()));
                    newDomElement->appendChild(newDomText);
                    childNode->appendChild(newDomElement);
                }
            }
        }
    }
    return nRC;
}

int CPlgMgXmlParser::OtmCompareModuleName(char * strModule1, char * strModule2)
{
    int nRC = ERROR_OTM_XML_PARSER_A;

    if ((NULL == strModule1) || (NULL == strModule2))
    {
        return nRC;
    }

    string stdModule1(strModule1);
    string stdModule2(strModule2);
    string stdModuleName1, stdModuleName2;

    size_t nPos = stdModule1.rfind(BACKSLASH_STR);
    if (string::npos != nPos)
    {
        stdModuleName1 = stdModule1.substr(nPos+1);
    }

    nPos = stdModule2.rfind(BACKSLASH_STR);
    if (string::npos != nPos)
    {
        stdModuleName2 = stdModule2.substr(nPos+1);
    }

    nRC = stricmp(stdModuleName1.c_str(), stdModuleName2.c_str());

    return nRC;
}

void CPlgMgXmlParser::ParseFixpacks(DOMNode* elementNode, PCPLUGINMAIN pOnePluginMain)
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

int CPlgMgXmlParser::GetMainFixpacksCnt(int iInx, int jInx)
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

char * CPlgMgXmlParser::GetMainFixpackId(int iInx, int jInx, int kInx)
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

char * CPlgMgXmlParser::GetMainFixpackDLUrl(int iInx, int jInx, int kInx)
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

char * CPlgMgXmlParser::GetMainFixpackDLType(int iInx, int jInx, int kInx)
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

char * CPlgMgXmlParser::GetMainFixpackMethod(int iInx, int jInx, int kInx)
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

char * CPlgMgXmlParser::GetMainFixpackShortDscp(int iInx, int jInx, int kInx)
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

BOOL CPlgMgXmlParser::GetMainFixpackNeedRestart(int iInx, int jInx, int kInx)
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

char * CPlgMgXmlParser::GetMainFixpackDate(int iInx, int jInx, int kInx)
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

char * CPlgMgXmlParser::GetMainFixpackLongDscp(int iInx, int jInx, int kInx)
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

int CPlgMgXmlParser::GetPluginPos(PMAINTHREADINFO pMainTdInfo)
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

CPlgMgXmlParser::CPlgMgXmlParser(void)
{
    // setting for log
    this->m_bLogOpened = FALSE;
#ifdef _DEBUG
    if (!this->m_logPlgMgXmlParser.isOpen())
    {
        this->m_logPlgMgXmlParser.open(LOG_PLUGIN_MGR_PARSER_NAME);
        m_bLogOpened = TRUE;
    }
#endif
}

CPlgMgXmlParser::~CPlgMgXmlParser(void)
{
    if (this->m_bLogOpened)
    {
        this->m_logPlgMgXmlParser.close();
    }
}
