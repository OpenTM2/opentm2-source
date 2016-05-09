//+----------------------------------------------------------------------------+
//|ProfileConfXmlParser.cpp     Parse profile set xml file                     |
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
//|                    during parse profile config xml file                    |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+


#include "ProfileConfXmlParser.h"

int CProfileConfXmlParser::LoadProfileSetConfig(POPTIONSET pSetOption)
{
    int nRC = NO_ERROR;

    // check whether the xml exists or not
    if (OTM_NOT_FOUND == access(m_strConfPath, 0))
    {
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Not find the config file %s.", m_strConfPath);
        nRC = ERROR_OTM_FILE_NOT_FIND_A;
        return nRC;
    }

    XercesDOMParser* otmParser;
    otmParser = new XercesDOMParser;
    otmParser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    otmParser->setDoNamespaces(false);
    otmParser->setDoSchema(false);
    otmParser->setLoadExternalDTD(false);
    otmParser->parse(m_strConfPath);

    xercesc::DOMDocument* docProfileSetConfXml = otmParser->getDocument();
    DOMElement* elementProfileSetConfRoot = docProfileSetConfXml->getDocumentElement();

    if (NULL != elementProfileSetConfRoot)
    {
        nRC = OtmParseFromRoot(elementProfileSetConfRoot, pSetOption);
    }

    // if no error, union the selection
    if (!nRC)
    {
        if (pSetOption->bChkAll)
        {
            pSetOption->bChkTransEditor = TRUE;
            pSetOption->bChkWorkbench = TRUE;
            pSetOption->bChkFldList = TRUE;
            pSetOption->bChkLastVal = TRUE;
        }
        else if (pSetOption->bChkTransEditor && pSetOption->bChkWorkbench && pSetOption->bChkFldList && pSetOption->bChkLastVal)
        {
            pSetOption->bChkAll = TRUE;
        }
    }

    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Parse the config file end (%d).", nRC);

    return nRC;
}

int CProfileConfXmlParser::OtmParseFromRoot(DOMNode* elementConfRoot, POPTIONSET pSetOption)
{
    int nRC = NO_ERROR;

    // initialize
    DOMNodeList* childNodeList = elementConfRoot->getChildNodes();
    XMLSize_t nodeChildCnt = childNodeList->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeChildCnt; nInx++)
    {
        DOMNode* currentNode = childNodeList->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strName = XMLString::transcode(currentNode->getNodeName());
        char * strValue = NULL;
        if (NULL != currentNode->getFirstChild())
        {
            strValue = XMLString::transcode(currentNode->getFirstChild()->getNodeValue());
        }

        if ((NULL == strValue) || strlen(strValue) == 0)
        {
            continue;
        }

        if (!stricmp(KEY_MODE, strName))
        {
            if (!stricmp(strValue, MODE_EXPORT))
            {
                pSetOption->bExport = TRUE;
            }
            else
            {
                pSetOption->bExport = FALSE;
            }
        }
        else if (!stricmp(KEY_DIRS, strName))
        {
            xercesc::DOMNodeList* curNodeChildList = currentNode->getChildNodes();
            XMLSize_t nSubNodeCnt = curNodeChildList->getLength();

            for (XMLSize_t jInx = 0; jInx < nSubNodeCnt; jInx++)
            {
                DOMNode* subNode = curNodeChildList->item(jInx);
                if (!subNode->getNodeType() || subNode->getNodeType() != DOMNode::ELEMENT_NODE)
                {
                    continue;
                }

                char * strSubName = XMLString::transcode(subNode->getNodeName());
                char * strSubValue = NULL;
                if (NULL != subNode->getFirstChild())
                {
                    strSubValue = XMLString::transcode(subNode->getFirstChild()->getNodeValue());
                }

                if ((NULL == strSubValue) || strlen(strSubValue) == 0)
                {
                    continue;
                }

                if (!stricmp(KEY_DIR, strSubName))
                {
                    pSetOption->lstStrDirs.push_back(strSubValue);
                }
            }
        }
        else if (!stricmp(KEY_TAR_FILE, strName))
        {
            strcpy(pSetOption->strTarFile, strValue);
        }
        else if (!stricmp(KEY_NAMES, strName))
        {
            xercesc::DOMNodeList* curNodeChildList = currentNode->getChildNodes();
            XMLSize_t nSubNodeCnt = curNodeChildList->getLength();

            for (XMLSize_t jInx = 0; jInx < nSubNodeCnt; jInx++)
            {
                DOMNode* subNode = curNodeChildList->item(jInx);
                if (!subNode->getNodeType() || subNode->getNodeType() != DOMNode::ELEMENT_NODE)
                {
                    continue;
                }

                char * strSubName = XMLString::transcode(subNode->getNodeName());
                char * strSubValue = NULL;
                if (NULL != subNode->getFirstChild())
                {
                    strSubValue = XMLString::transcode(subNode->getFirstChild()->getNodeValue());
                }

                if ((NULL == strSubValue) || strlen(strSubValue) == 0)
                {
                    continue;
                }

                if (!stricmp(KEY_NAME, strSubName))
                {
                    pSetOption->lstStrNames.push_back(strSubValue);
                }
            }
        }
        else if (!stricmp(KEY_MAX_HIST_CNT, strName))
        {
            pSetOption->nMaxHistCnt = atoi(strValue);
        }
        else if (!stricmp(KEY_CHK_ALL, strName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strValue))
            {
                pSetOption->bChkAll = TRUE;
            }
            else
            {
                pSetOption->bChkAll = FALSE;
            }
        }
        else if (!stricmp(KEY_CHK_TRANS_EDITOR, strName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strValue))
            {
                pSetOption->bChkTransEditor = TRUE;
            }
            else
            {
                pSetOption->bChkTransEditor = FALSE;
            }
        }
        else if (!stricmp(KEY_CHK_WORKBENCH, strName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strValue))
            {
                pSetOption->bChkWorkbench = TRUE;
            }
            else
            {
                pSetOption->bChkWorkbench = FALSE;
            }
        }
        else if (!stricmp(KEY_CHK_FOLDER_LIST, strName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strValue))
            {
                pSetOption->bChkFldList = TRUE;
            }
            else
            {
                pSetOption->bChkFldList = FALSE;
            }
        }
        else if (!stricmp(KEY_CHK_LAST_USED_VAL, strName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strValue))
            {
                pSetOption->bChkLastVal = TRUE;
            }
            else
            {
                pSetOption->bChkLastVal = FALSE;
            }
        }
        else if (!stricmp(KEY_IF_ENCRYPT, strName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strValue))
            {
                pSetOption->bEncrypt = TRUE;
            }
            else
            {
                pSetOption->bEncrypt = FALSE;
            }
        }
        else if (!stricmp(KEY_IF_KEEP_ORI_FILE, strName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strValue))
            {
                pSetOption->bKeepOriFile = TRUE;
            }
            else
            {
                pSetOption->bKeepOriFile = FALSE;
            }
        }
    }

    return nRC;
}

int CProfileConfXmlParser::SaveProfileSetConfig(POPTIONSET pSetOption)
{
    int nRC = NO_ERROR;

    try
    {
        // create the XML
        LocalFileFormatTarget * pMyFormTarget = new LocalFileFormatTarget(XMLString::transcode(m_strConfPath));
        if (NULL == pMyFormTarget)
        {
            nRC = ERROR_OTM_XERCESC_CREATE_A;
            return nRC;
        }

        // create a implementation
        DOMImplementation* pDomImpl = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("LS"));
        if (NULL == pDomImpl)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        // create out and set target
        DOMLSOutput * pLSOutput = pDomImpl->createLSOutput();
        if (NULL == pLSOutput)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLSOutput->setByteStream(pMyFormTarget);

        xercesc::DOMDocument* pXmlDoc = pDomImpl->createDocument(0, XMLString::transcode(KEY_PROFILE_CONFIG), 0);
        if (NULL == pXmlDoc)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        DOMElement* elementRoot = pXmlDoc->getDocumentElement();
        if (NULL == elementRoot)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        // mode node
        DOMElement*  pModeEle = pXmlDoc->createElement(XMLString::transcode(KEY_MODE));
        if (NULL == pModeEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pModeEle);

        if (pSetOption->bExport)
        {
            pModeEle->setTextContent(XMLString::transcode(MODE_EXPORT));
        }
        else
        {
            pModeEle->setTextContent(XMLString::transcode(MODE_IMPORT));
        }

        // target file node
        DOMElement*  pTarFileEle = pXmlDoc->createElement(XMLString::transcode(KEY_TAR_FILE));
        if (NULL == pTarFileEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pTarFileEle);
        pTarFileEle->setTextContent(XMLString::transcode(pSetOption->strTarFile));

        // dirs node
        DOMElement*  pDirsEle = pXmlDoc->createElement(XMLString::transcode(KEY_DIRS));
        if (NULL == pDirsEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pDirsEle);

        for (size_t iInx = 0; iInx < pSetOption->lstStrDirs.size(); iInx++)
        {
            DOMElement*  pDirEle = pXmlDoc->createElement(XMLString::transcode(KEY_DIR));
            if (NULL == pDirEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pDirsEle->appendChild(pDirEle);
            pDirEle->setTextContent(XMLString::transcode(pSetOption->lstStrDirs[iInx].c_str()));
        }

        // names node
        DOMElement*  pNamesEle = pXmlDoc->createElement(XMLString::transcode(KEY_NAMES));
        if (NULL == pNamesEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pNamesEle);

        for (size_t iInx = 0; iInx < pSetOption->lstStrNames.size(); iInx++)
        {
            DOMElement*  pNameEle = pXmlDoc->createElement(XMLString::transcode(KEY_NAME));
            if (NULL == pNameEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pNamesEle->appendChild(pNameEle);
            pNameEle->setTextContent(XMLString::transcode(pSetOption->lstStrNames[iInx].c_str()));
        }

        char strValue[MAX_BUF_SIZE];
        // current selection for dirs
        DOMElement*  pMaxHistCntEle = pXmlDoc->createElement(XMLString::transcode(KEY_MAX_HIST_CNT));
        if (NULL == pMaxHistCntEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pMaxHistCntEle);
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSetOption->nMaxHistCnt);
        pMaxHistCntEle->setTextContent(XMLString::transcode(strValue));

        // check all node
        DOMElement*  pChkAllEle = pXmlDoc->createElement(XMLString::transcode(KEY_CHK_ALL));
        if (NULL == pChkAllEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pChkAllEle);
        if (pSetOption->bChkAll)
        {
            pChkAllEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pChkAllEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // check translation editor node
        DOMElement*  pChkTransEditorEle = pXmlDoc->createElement(XMLString::transcode(KEY_CHK_TRANS_EDITOR));
        if (NULL == pChkTransEditorEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pChkTransEditorEle);
        if (pSetOption->bChkTransEditor)
        {
            pChkTransEditorEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pChkTransEditorEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // check workbench node
        DOMElement*  pChkWorkbenchEle = pXmlDoc->createElement(XMLString::transcode(KEY_CHK_WORKBENCH));
        if (NULL == pChkWorkbenchEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pChkWorkbenchEle);
        if (pSetOption->bChkWorkbench)
        {
            pChkWorkbenchEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pChkWorkbenchEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // check folder list node
        DOMElement*  pChkFldListEle = pXmlDoc->createElement(XMLString::transcode(KEY_CHK_FOLDER_LIST));
        if (NULL == pChkFldListEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pChkFldListEle);
        if (pSetOption->bChkFldList)
        {
            pChkFldListEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pChkFldListEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // check last used value node
        DOMElement*  pChkLstUsedValEle = pXmlDoc->createElement(XMLString::transcode(KEY_CHK_LAST_USED_VAL));
        if (NULL == pChkLstUsedValEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pChkLstUsedValEle);
        if (pSetOption->bChkLastVal)
        {
            pChkLstUsedValEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pChkLstUsedValEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }


        // set output format
        DOMLSSerializer * pLSSerializer = pDomImpl->createLSSerializer();
        if (pLSSerializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
            pLSSerializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
        if (pLSSerializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTWhitespaceInElementContent, true))
            pLSSerializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTWhitespaceInElementContent, true);
        if (pLSSerializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTXercesPrettyPrint, false))
            pLSSerializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTXercesPrettyPrint, false);

        // write to file
        pLSSerializer->write(pXmlDoc, pLSOutput);

        // release temp value
        pXmlDoc->release();
        pLSOutput->release();
        pLSSerializer->release();

        delete pMyFormTarget;
    }
    catch (const XMLException& xmlExp)
    {
        char* strMsg = XMLString::transcode(xmlExp.getMessage());
        memset(m_strErrMsg, 0x00, sizeof(m_strErrMsg));
        strcpy(m_strErrMsg, strMsg);
        XMLString::release(&strMsg);
        nRC = ERROR_OTM_XERCESC_DOM_A;
    }
    catch (const OutOfMemoryException& memExp)
    {
        char* strMsg = XMLString::transcode(memExp.getMessage());
        memset(m_strErrMsg, 0x00, sizeof(m_strErrMsg));
        strcpy(m_strErrMsg, strMsg);
        XMLString::release(&strMsg);
        nRC = ERROR_OTM_XERCESC_MEM_A;
    }
    catch (const DOMException& domExp)
    {
        char* strMsg = XMLString::transcode(domExp.getMessage());
        memset(m_strErrMsg, 0x00, sizeof(m_strErrMsg));
        strcpy(m_strErrMsg, strMsg);
        XMLString::release(&strMsg);
        nRC = ERROR_OTM_XERCESC_DOM_A;
    }
    catch (...)
    {
        nRC = ERROR_OTM_XERCESC_UNKNOW_A;
    }
    return nRC;
}

CProfileConfXmlParser::CProfileConfXmlParser(const char * strStartDir)
{
    memset(m_strErrMsg,   0x00, sizeof(m_strErrMsg));
    memset(m_strConfPath, 0x00, sizeof(m_strConfPath));

    if ((NULL != strStartDir) && (strlen(strStartDir) != 0))
    {
        sprintf(m_strConfPath, "%s\\%s", strStartDir, CONF_FILE_NAME);
    }
    else
    {
        strcpy(m_strConfPath, CONF_FILE_NAME);
    }

    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& xmlExp)
    {
        char* strMsg = XMLString::transcode(xmlExp.getMessage());
        memset(m_strErrMsg, 0x00, sizeof(m_strErrMsg));
        strcpy(m_strErrMsg, strMsg);
        XMLString::release(&strMsg);
    }
}


CProfileConfXmlParser::~CProfileConfXmlParser(void)
{
    XMLPlatformUtils::Terminate();
}
