//+----------------------------------------------------------------------------+
//|OtmXmlParser.cpp     Parse AVU xml function                                 |
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
//|                    during pasrsing auto version up xml file                |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
#include "OtmXmlLocParser.h"

int COtmXmlLocParser::XmlParser(const char * strXml)
{
    int nRC = NO_ERROR;

    // check whether the xml exists or not
    if (OTM_NOT_FOUND == access(strXml, 0))
    {
        m_logAutoVerUpParser.writef("Error: not find the xml file %s.", strXml);
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
        return ERROR_OTM_XERCESC_INITIAL_A;
    }

    XercesDOMParser* myParser;
    myParser = new XercesDOMParser;
    myParser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    myParser->setDoNamespaces(false);
    myParser->setDoSchema(false);
    myParser->setLoadExternalDTD(false);
    myParser->parse(strXml);

    xercesc::DOMDocument* xmlDoc = myParser->getDocument();
    xercesc::DOMElement* elementRoot = xmlDoc->getDocumentElement();

    if (NULL != elementRoot)
    {
        m_gComponents.clear();
        parseFromRoot(elementRoot);
    }

    XMLPlatformUtils::Terminate();

    m_logAutoVerUpParser.writef("Parese end (%d)", nRC);
    return nRC;
}

void COtmXmlLocParser::parseFromRoot(DOMNode* elementRoot)
{
    OTMCOMPONENT oneComponent;
    InitComponent(&oneComponent);
    xercesc::DOMNodeList* children = elementRoot->getChildNodes();
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        DOMNode* currentNode = children->item(nInx);
        if (currentNode->getNodeType() && currentNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {
            parseFromNode(currentNode, &oneComponent);
            m_gComponents.push_back(oneComponent);
        }
    }
}

void COtmXmlLocParser::parseFromNode(DOMNode* elementNode, POTMCOMPONENT pOneComponent)
{
    char * strValue, * strName;
    DOMNode* currentNode = NULL;

    char * nodeName = XMLString::transcode(elementNode->getNodeName());

    if (!stricmp(KEY_COMPONENT, nodeName))
    {
        // check whether has attributes first
        if (!elementNode->hasAttributes())
        {
            return;
        }

        InitComponent(pOneComponent);

        for (int iInx = 0; iInx < (int) elementNode->getAttributes()->getLength(); iInx++)
        {
            strName = XMLString::transcode(elementNode->getAttributes()->item(iInx)->getNodeName());
            strValue = XMLString::transcode(elementNode->getAttributes()->item(iInx)->getNodeValue());
            if (!stricmp(KEY_ATTRI_NAME, strName))
            {
                int nLen = min(sizeof(pOneComponent->strName), strlen(strValue));
                strncpy(pOneComponent->strName, strValue, nLen);
                pOneComponent->strName[nLen] = EOS;
            }
        }
    }
    else if (!stricmp(KEY_VERSION, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOneComponent->strVersion), strlen(strValue));
        strncpy(pOneComponent->strVersion, strValue, nLen);
        pOneComponent->strVersion[nLen] = EOS;
    }
    else if (!stricmp(KEY_DATE, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOneComponent->strDate), strlen(strValue));
        strncpy(pOneComponent->strDate, strValue, nLen);
        pOneComponent->strDate[nLen] = EOS;
    }
    else if (!stricmp(KEY_SHORT_DSCP, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOneComponent->strShortDscp), strlen(strValue));
        strncpy(pOneComponent->strShortDscp, strValue, nLen);
        pOneComponent->strShortDscp[nLen] = EOS;
    }
    else if (!stricmp(KEY_LONG_DSCP, nodeName))
    {
        BOOL bStart = TRUE;

        char strTempValue[COMP_LONG_DSCP_LEN];
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

        int nLen = min(sizeof(pOneComponent->strLongDscp), strlen(strTempValue));
        strncpy(pOneComponent->strLongDscp, strTempValue, nLen);
        pOneComponent->strLongDscp[nLen] = EOS;
    }
    else if (!stricmp(KEY_SEVERITY, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOneComponent->strSeverity), strlen(strValue));
        strncpy(pOneComponent->strSeverity, strValue, nLen);
        pOneComponent->strSeverity[nLen] = EOS;
    }
    else if (!stricmp(KEY_IMPACT, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOneComponent->strImpact), strlen(strValue));
        strncpy(pOneComponent->strImpact, strValue, nLen);
        pOneComponent->strImpact[nLen] = EOS;
    }
    else if (!stricmp(KEY_AFTERACTION, nodeName))
    {
        strValue = XMLString::transcode(elementNode->getFirstChild()->getNodeValue());
        int nLen = min(sizeof(pOneComponent->strAfterAction), strlen(strValue));
        strncpy(pOneComponent->strAfterAction, strValue, nLen);
        pOneComponent->strAfterAction[nLen] = EOS;
    }
    else if (!stricmp(KEY_INSTALL, nodeName))
    {
        DOMNodeList* children = elementNode->getChildNodes();
        XMLSize_t nodeCount = children->getLength();

        for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
        {
            currentNode = children->item(nInx);
            nodeName = XMLString::transcode(currentNode->getNodeName());

            if (!stricmp(KEY_COPY, nodeName))
            {
                if (!currentNode->hasAttributes())
                {
                    continue;
                }

                for (int iInx = 0; iInx < (int) currentNode->getAttributes()->getLength(); iInx++)
                {
                    strName = XMLString::transcode(currentNode->getAttributes()->item(iInx)->getNodeName());
                    strValue = XMLString::transcode(currentNode->getAttributes()->item(iInx)->getNodeValue());
                    if (!stricmp(KEY_ATTRI_FROM, strName))
                    {
                        int nLen = min(sizeof(pOneComponent->component.strFrom), strlen(strValue));
                        strncpy(pOneComponent->component.strFrom, strValue, nLen);
                        pOneComponent->component.strFrom[nLen] = EOS;
                    }
                    else if (!stricmp(KEY_ATTRI_TO, strName))
                    {
                        int nLen = min(sizeof(pOneComponent->component.strTo), strlen(strValue));
                        strncpy(pOneComponent->component.strTo, strValue, nLen);
                        pOneComponent->component.strTo[nLen] = EOS;
                    }
                }
            }
            else if (!stricmp(KEY_DOWNLOAD, nodeName))
            {
                if (!currentNode->hasAttributes())
                {
                    continue;
                }
                for (int iInx = 0; iInx < (int) currentNode->getAttributes()->getLength(); iInx++)
                {
                    strName = XMLString::transcode(currentNode->getAttributes()->item(iInx)->getNodeName());
                    strValue = XMLString::transcode(currentNode->getAttributes()->item(iInx)->getNodeValue());
                    if (!stricmp(KEY_ATTRI_DLTYPE, strName))
                    {
                        int nLen = min(sizeof(pOneComponent->component.strDLType), strlen(strValue));
                        strncpy(pOneComponent->component.strDLType, strValue, nLen);
                        pOneComponent->component.strDLType[nLen] = EOS;
                    }
                    else if (!stricmp(KEY_ATTRI_METHOD, strName))
                    {
                        int nLen = min(sizeof(pOneComponent->component.strMethod), strlen(strValue));
                        strncpy(pOneComponent->component.strMethod, strValue, nLen);
                        pOneComponent->component.strMethod[nLen] = EOS;
                    }
                    else if (!stricmp(KEY_ATTRI_RESTART, strName))
                    {
                        if (!stricmp(KEY_ARRTI_TRUE_STR, strValue))
                        {
                            pOneComponent->component.bRestart = TRUE;
                        }
                        else
                        {
                            pOneComponent->component.bRestart = FALSE;
                        }
                    }
                    else if (!stricmp(KEY_ATTRI_NEED_WAIT, strName))
                    {
                        if (!stricmp(KEY_ARRTI_TRUE_STR, strValue))
                        {
                            pOneComponent->component.bNeedWait = TRUE;
                        }
                        else
                        {
                            pOneComponent->component.bNeedWait = FALSE;
                        }
                    }
                }

                strValue = XMLString::transcode(currentNode->getFirstChild()->getNodeValue());
                int nLen = min(sizeof(pOneComponent->component.strDLUrl), strlen(strValue));
                strncpy(pOneComponent->component.strDLUrl, strValue, nLen);
                pOneComponent->component.strDLUrl[nLen] = EOS;
            }
            else if (!stricmp(KEY_FIXPACKS, nodeName))
            {
                parseFixpacks(currentNode, pOneComponent);
            }
        }

        elementNode = currentNode;
    }

    xercesc::DOMNodeList* children = elementNode->getChildNodes();
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        parseFromNode(currentNode, pOneComponent);
    }
}

void COtmXmlLocParser::parseFixpacks(DOMNode* elementNode, POTMCOMPONENT pOneComponent)
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
                        else if (!stricmp(KEY_ATTRI_NEED_WAIT, strName))
                        {
                            if (!stricmp(KEY_ARRTI_TRUE_STR, strValue))
                            {
                                fixPack.bNeedWait = TRUE;
                            }
                            else
                            {
                                fixPack.bNeedWait = FALSE;
                            }
                        }
                    }

                    strValue = XMLString::transcode(currentSubNode->getFirstChild()->getNodeValue());
                    int nLen = min(sizeof(fixPack.strDLUrl), strlen(strValue));
                    strncpy(fixPack.strDLUrl, strValue, nLen);
                    fixPack.strDLUrl[nLen] = EOS;
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
                else if (!stricmp(KEY_DATE, nodeName))
                {
                    strValue = XMLString::transcode(currentSubNode->getFirstChild()->getNodeValue());
                    int nLen = min(sizeof(fixPack.strDate), strlen(strValue));
                    strncpy(fixPack.strDate, strValue, nLen);
                    fixPack.strDate[nLen] = EOS;
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
            pOneComponent->fixPacks.push_back(fixPack);
        }
    }
}

// only get the count of how many components
int COtmXmlLocParser::GetComponentsCnt()
{
    return m_gComponents.size();
}

// get the total count, inlcude fixpagecks' number of each component
int COtmXmlLocParser::GetComponentsTotalCnt()
{
    int nTotal = 0;
    for (int iInx = 0; iInx < (int) m_gComponents.size(); iInx++)
    {
        nTotal++;
        for (int jInx =0; jInx < (int) m_gComponents[iInx].fixPacks.size(); jInx++)
        {
            nTotal++;
        }
    }
    return nTotal;
}

int COtmXmlLocParser::GetComponentCntByName(char * strName)
{
    int nCnt = 0;

    // skip when name is null
    if (NULL == strName)
    {
        return nCnt;
    }

    // fine the component
    for (int iInx = 0; iInx < (int) m_gComponents.size(); iInx++)
    {
        if (!IsFixpack(strName))
        {
            if (!stricmp(strName, m_gComponents[iInx].strName))
            {
                nCnt++;
            }
        }
        else
        {
            for (int jInx =0; jInx < (int) m_gComponents[iInx].fixPacks.size(); jInx++)
            {
                char strId[MAX_LEN];
                char strCompName[MAX_LEN];
                char strCompVer[COMP_MAX_VER_LEN];

                memset(strId,        0x00, sizeof(strId));
                memset(strCompName,  0x00, sizeof(strCompName));
                memset(strCompVer,   0x00, sizeof(strCompVer));

                SplitFixpackName(strName, strCompName, strCompVer, strId);

                if (!stricmp(strCompName, m_gComponents[iInx].strName) && 
                    !stricmp(strCompVer,  m_gComponents[iInx].strVersion) &&
                    !stricmp(strId,       m_gComponents[iInx].fixPacks[jInx].strId))
                {
                    nCnt++;
                }
            }
        }
    }
    return nCnt;
}

char * COtmXmlLocParser::GetComponentName(int nInx)
{
    if ((nInx >= GetComponentsCnt()) && (nInx < 0))
    {
        return NULL;
    }

    return m_gComponents[nInx].strName;
}

char * COtmXmlLocParser::GetComponentDate(int nInx)
{
    if ((nInx >= GetComponentsCnt()) && (nInx < 0))
    {
        return NULL;
    }

    return m_gComponents[nInx].strDate;
}

char * COtmXmlLocParser::GetComponentShortDscp(int nInx)
{
    if ((nInx >= GetComponentsCnt()) && (nInx < 0))
    {
        return NULL;
    }

    return m_gComponents[nInx].strShortDscp;
}

char * COtmXmlLocParser::GetComponentLongDscp(int nInx)
{
    if ((nInx >= GetComponentsCnt()) && (nInx < 0))
    {
        return NULL;
    }

    return m_gComponents[nInx].strLongDscp;
}

char * COtmXmlLocParser::GetComponentVersion(int nInx)
{
    if ((nInx >= GetComponentsCnt()) && (nInx < 0))
    {
        return NULL;
    }

    return m_gComponents[nInx].strVersion;
}

char * COtmXmlLocParser::GetComponentSeverity(int nInx)
{
    if ((nInx >= GetComponentsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gComponents[nInx].strSeverity;
}

char * COtmXmlLocParser::GetComponentImpact(int nInx)
{
    if ((nInx >= GetComponentsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gComponents[nInx].strImpact;
}

char * COtmXmlLocParser::GetComponentAfterAction(int nInx)
{
    if ((nInx >= GetComponentsCnt()) || (nInx < 0))
    {
        return NULL;
    }

    return m_gComponents[nInx].strAfterAction;
}

char * COtmXmlLocParser::GetComponentDLUrl(int nInx)
{
    if (nInx >= GetComponentsCnt())
    {
        return NULL;
    }
    return m_gComponents[nInx].component.strDLUrl;
}

char * COtmXmlLocParser::GetComponentDLType(int nInx)
{
    if (nInx >= GetComponentsCnt())
    {
        return NULL;
    }
    return m_gComponents[nInx].component.strDLType;
}

char * COtmXmlLocParser::GetComponentMethod(int nInx)
{
    if (nInx >= GetComponentsCnt())
    {
        return NULL;
    }
    return m_gComponents[nInx].component.strMethod;
}

BOOL COtmXmlLocParser::GetComponentRestart(int nInx)
{
    if (nInx >= GetComponentsCnt())
    {
        return NULL;
    }
    return m_gComponents[nInx].component.bRestart;
}

BOOL COtmXmlLocParser::GetComponentNeedWait(int nInx)
{
    if (nInx >= GetComponentsCnt())
    {
        return NULL;
    }
    return m_gComponents[nInx].component.bNeedWait;
}

int COtmXmlLocParser::GetCompFixpacksCnt(int nInx)
{
    if (nInx >= GetComponentsCnt())
    {
        return 0;
    }
    return m_gComponents[nInx].fixPacks.size();
}

char * COtmXmlLocParser::GetCompFixpackId(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].strId;
}

char * COtmXmlLocParser::GetCompFixpackDLUrl(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].strDLUrl;
}

char * COtmXmlLocParser::GetCompFixpackDLType(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].strDLType;
}

char * COtmXmlLocParser::GetCompFixpackMethod(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].strMethod;
}

char * COtmXmlLocParser::GetCompFixpackDate(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].strDate;
}

char * COtmXmlLocParser::GetCompFixpackShortDscp(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].strShortDscp;
}

char * COtmXmlLocParser::GetCompFixpackLongDscp(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].strLongDscp;
}

BOOL COtmXmlLocParser::GetCompFixpackRestart(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].bRestart;
}

BOOL COtmXmlLocParser::GetCompFixpackNeedWait(int iInx, int jInx)
{
    if ((iInx >= GetComponentsCnt()) || (iInx < 0))
    {
        return NULL;
    }

    if ((jInx >= GetCompFixpacksCnt(iInx)) || (jInx < 0))
    {
        return NULL;
    }

    return m_gComponents[iInx].fixPacks[jInx].bNeedWait;
}

int COtmXmlLocParser::GetCompCopiesByName(const char * strComp, const char * strVer, PCOTMCOPIES pCompCopies)
{
    int nRC = NO_ERROR;

    // skip when name is null
    if ((NULL == strComp) || (strlen(strComp) == 0))
    {
        nRC = ERROR_COMP_NAME_A;
        return nRC;
    }

    // find the plugin
    for (int iInx = 0; iInx < (int) m_gComponents.size(); iInx++)
    {
        if (IsFixpack(strComp))
        {
            char strId[MAX_LEN];
            char strCompName[MAX_LEN];
            char strCompVer[COMP_MAX_VER_LEN];

            memset(strId,        0x00, sizeof(strId));
            memset(strCompName,  0x00, sizeof(strCompName));
            memset(strCompVer,   0x00, sizeof(strCompVer));

            BOOL bFound = FALSE;
            SplitFixpackName(strComp, strCompName, strCompVer, strId);

            for (int jInx = 0; jInx < (int) m_gComponents[iInx].fixPacks.size(); jInx++)
            {
                if (!stricmp(m_gComponents[iInx].strName,    strCompName) && 
                    !stricmp(m_gComponents[iInx].strVersion, strCompVer) && 
                    !stricmp(m_gComponents[iInx].fixPacks[jInx].strId, strId))
                {
                    for (int kInx = 0; kInx < (int) m_gComponents[iInx].fixPacks[jInx].fixpackCopies.size(); kInx++)
                    {
                        COTMCOPY compCopy;
                        InitOtmCopy(&compCopy);

                        strcpy(compCopy.strFrom,      m_gComponents[iInx].fixPacks[jInx].fixpackCopies[kInx].strFrom);
                        strcpy(compCopy.strTo,        m_gComponents[iInx].fixPacks[jInx].fixpackCopies[kInx].strTo);

                        pCompCopies->push_back(compCopy);
                    }

                    bFound = TRUE;
                    break;
                }
            }

            if (bFound)
            {
                break;
            }
        }
        else
        {
            // not fixpack
            if (!stricmp(strComp, m_gComponents[iInx].strName) &&
                !stricmp(strVer,  m_gComponents[iInx].strVersion))
            {
                COTMCOPY compCopy;
                InitOtmCopy(&compCopy);

                strcpy(compCopy.strFrom,      m_gComponents[iInx].component.strFrom);
                strcpy(compCopy.strTo,        m_gComponents[iInx].component.strTo);

                pCompCopies->push_back(compCopy);
            }
        }
    }
    return nRC;
}

void COtmXmlLocParser::ClearAllComponents()
{
    m_gComponents.clear();
}

void COtmXmlLocParser::InitComponent(POTMCOMPONENT pOneComponent)
{
    pOneComponent->component.bRestart  = FALSE;
    pOneComponent->component.bNeedWait = FALSE;
    memset(pOneComponent->component.strDLType,     0x00, sizeof(pOneComponent->component.strDLType));
    memset(pOneComponent->component.strMethod,     0x00, sizeof(pOneComponent->component.strMethod));
    memset(pOneComponent->component.strDLUrl,      0x00, sizeof(pOneComponent->component.strDLUrl));
    memset(pOneComponent->component.strFrom,       0x00, sizeof(pOneComponent->component.strFrom));
    memset(pOneComponent->component.strTo,         0x00, sizeof(pOneComponent->component.strTo));
    memset(pOneComponent->component.strId,         0x00, sizeof(pOneComponent->component.strId));

    memset(pOneComponent->strDate,                 0x00, sizeof(pOneComponent->strDate));
    memset(pOneComponent->strLongDscp,             0x00, sizeof(pOneComponent->strLongDscp));
    memset(pOneComponent->strName,                 0x00, sizeof(pOneComponent->strName));
    memset(pOneComponent->strShortDscp,            0x00, sizeof(pOneComponent->strShortDscp));
    memset(pOneComponent->strVersion,              0x00, sizeof(pOneComponent->strVersion));

    pOneComponent->fixPacks.clear();
}

int COtmXmlLocParser::GetComponentPos(PMAINTHREADINFO pMainTdInfo)
{
    int nRC = ERROR_PLUGIN_NOT_FOUND_A;

    for (int iInx = 0; iInx < GetComponentsCnt(); iInx++)
    {
        // find the same plugin name
        char * strTemp = GetComponentName(iInx);
        char * strTemp1 = GetComponentVersion(iInx);
        if (!stricmp(pMainTdInfo->strName, GetComponentName(iInx)) && 
            !stricmp(pMainTdInfo->strVer,  GetComponentVersion(iInx)))
        {
            // set iInx and jInx
            pMainTdInfo->iInx = iInx;

            // if not fixpack, all value has been set, just return
            if (!pMainTdInfo->bFixpack)
            {
                nRC = NO_ERROR;
                return nRC;
            }

            // if fixpack, set kInx
            for (int jInx = 0; jInx < GetCompFixpacksCnt(iInx); jInx++)
            {
                // find the same id info
                if (stricmp(pMainTdInfo->strFixpkId, GetCompFixpackId(iInx, jInx)))
                {
                    continue;
                }

                // if find set the kInx and just return
                pMainTdInfo->jInx = jInx;
                nRC = NO_ERROR;
                return nRC;
            }
        }
    }

    return nRC;
}

COtmXmlLocParser::COtmXmlLocParser(void)
{
}

COtmXmlLocParser::~COtmXmlLocParser(void)
{
}

