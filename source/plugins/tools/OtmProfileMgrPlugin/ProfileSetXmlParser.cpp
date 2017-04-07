//+----------------------------------------------------------------------------+
//|OtmProfileSetXmlParser.cpp     Parse profile set xml file                   |
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
//|                    during pasrsing profile setting xml file                |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
#include "ProfileSetXmlParser.h"
int CProfileSetXmlParser::DoProfileExport(POPTIONSET pOptionSet)
{
    int nRC = NO_ERROR;

    try
    {
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to create xml %s.", pOptionSet->strTarFile);
        // create the XML
        LocalFileFormatTarget * pMyFormTarget = new LocalFileFormatTarget(XMLString::transcode(pOptionSet->strTarFile));
        if (NULL == pMyFormTarget)
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "LocalFileFormatTarget failed.");
            nRC = ERROR_OTM_XERCESC_CREATE_A;
            return nRC;
        }

        // create a implementation
        DOMImplementation* pDomImpl = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("LS"));
        if (NULL == pDomImpl)
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "getDOMImplementation failed.");
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        // create out and set target
        DOMLSOutput * pLSOutput = pDomImpl->createLSOutput();
        if (NULL == pLSOutput)
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "createLSOutput failed.");
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLSOutput->setByteStream(pMyFormTarget);

        xercesc::DOMDocument* pXmlDoc = pDomImpl->createDocument(0, XMLString::transcode(KEY_PROFILE_SET), 0);
        if (NULL == pXmlDoc)
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "createDocument %s failed.", KEY_PROFILE_SET);
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        if (pOptionSet->bChkAll || pOptionSet->bChkTransEditor)
        {
            // export translation editor
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to export translation editor.");
            nRC = DoTransEditorExport(pXmlDoc);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export translation editor(%d).", nRC);
            if (nRC)
            {
                pXmlDoc->release();
                pLSOutput->release();
                delete pMyFormTarget;
                return nRC;
            }
        }

        // export workbench
        if (pOptionSet->bChkAll || pOptionSet->bChkWorkbench)
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to export workbench.");
            nRC = DoWorkbenchExport(pXmlDoc);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export workbench(%d).", nRC);
            if (nRC)
            {
                pXmlDoc->release();
                pLSOutput->release();
                delete pMyFormTarget;
                return nRC;
            }
        }

        // export folder list
        if (pOptionSet->bChkAll || pOptionSet->bChkFldList)
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to export folder list.");
            nRC = DoFolderListExport(pXmlDoc);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export folder list(%d).", nRC);
            if (nRC)
            {
                pXmlDoc->release();
                pLSOutput->release();
                delete pMyFormTarget;
                return nRC;
            }
        }

        // export last used values
        if (pOptionSet->bChkAll || pOptionSet->bChkLastVal)
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to export last used value.");
            nRC = DoLastUsedValuesExport(pXmlDoc);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export last used value(%d).", nRC);
            if (nRC)
            {
                pXmlDoc->release();
                pLSOutput->release();
                delete pMyFormTarget;
                return nRC;
            }
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

int CProfileSetXmlParser::DoTransEditorExport(xercesc::DOMDocument* pExportXmlDoc)
{
    int nRC = NO_ERROR;

    try
    {
        DOMElement* elementRoot = pExportXmlDoc->getDocumentElement();
        if (NULL == elementRoot)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        // translation editor node
        DOMElement*  pEditorEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_TRANS_EDITOR));
        if (NULL == pEditorEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pEditorEle);

        // load from property file
        EQFBReadProfile();

        // export font info
        nRC = FontInfoExport(pExportXmlDoc, pEditorEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export font info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }

        // export color info
        nRC = ColorInfoExport(pExportXmlDoc, pEditorEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export color info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }

        // key info
        nRC = KeyInfoExport(pExportXmlDoc, pEditorEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export key info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }
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

int CProfileSetXmlParser::ColorInfoExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement*  pEditorEle)
{
    int nRC = NO_ERROR;
    char strValue[MAX_BUF_SIZE];

    try
    {
        // font size node
        DOMElement*  pColorsEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_COLORS));
        if (NULL == pColorsEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pEditorEle->appendChild(pColorsEle);

        PTEXTTYPETABLE pTextTable = get_TextTypeTable();
        for (int iInx = 0; iInx < MAXCOLOUR; iInx++)
        {
            // load names for text types
            DOMElement*  pTextTypeEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FONT_TYPE));
            if (NULL == pTextTypeEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pColorsEle->appendChild(pTextTypeEle);

            // set text type name
            char strFontType[MAX_BUF_SIZE];
            memset(strFontType, 0x00, sizeof(strFontType));
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            LOADSTRING(hab, hResMod, IDS_TB_COL_EXIT+iInx, strFontType);

            pTextTypeEle->setAttribute(XMLString::transcode(KEY_NAME), XMLString::transcode(strFontType));

            // set foreground color node
            DOMElement*  pForegroundEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FOREGROUND_COLOR));
            if (NULL == pForegroundEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pTextTypeEle->appendChild(pForegroundEle);

            // set foreground color attribute index
            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", pTextTable[iInx].sFGColor);
            pForegroundEle->setAttribute(XMLString::transcode(KEY_INDEX), XMLString::transcode(strValue));

            // set foreground color attribute name
            memset(strValue, 0x00, sizeof(strValue));
            LOADSTRING(hab, hResMod, IDS_TB_COL_BLACK+pTextTable[iInx].sFGColor, strValue);
            pForegroundEle->setAttribute(XMLString::transcode(KEY_NAME), XMLString::transcode(strValue));

            // set background node
            DOMElement*  pBackgroundEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_BACKGROUND_COLOR));
            if (NULL == pBackgroundEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pTextTypeEle->appendChild(pBackgroundEle);

            // set background color attribute index
            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", pTextTable[iInx].sBGColor);
            pBackgroundEle->setAttribute(XMLString::transcode(KEY_INDEX), XMLString::transcode(strValue));

            // set foreground color attribute name
            memset(strValue, 0x00, sizeof(strValue));
            LOADSTRING(hab, hResMod, IDS_TB_COL_BLACK+pTextTable[iInx].sBGColor, strValue);
            pBackgroundEle->setAttribute(XMLString::transcode(KEY_NAME), XMLString::transcode(strValue));

            // set font symbolic name node
            DOMElement*  pFontSymbNameEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FONT_SYMB_NAME));
            if (NULL == pFontSymbNameEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pTextTypeEle->appendChild(pFontSymbNameEle);

            // set font symbolic name value
            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", pTextTable[iInx].usFont);
            pFontSymbNameEle->setTextContent(XMLString::transcode(strValue));

            // set underscore flag
            DOMElement*  pUnderscoreEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_UNDERSCORE_FLAG));
            if (NULL == pUnderscoreEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pTextTypeEle->appendChild(pUnderscoreEle);

            // set underscore value
            if (pTextTable[iInx].fUnderscore)
            {
                pUnderscoreEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
            }
            else
            {
                pUnderscoreEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
            }

            // set reserve flag node
            DOMElement*  pReserveEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_REVERSE_FLAG));
            if (NULL == pReserveEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pTextTypeEle->appendChild(pReserveEle);

            // set reverse flag
            if (pTextTable[iInx].fReverse)
            {
                pReserveEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
            }
            else
            {
                pReserveEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
            }
        }
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

int CProfileSetXmlParser::FontInfoExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement*  pEditorEle)
{
    int nRC = NO_ERROR;
    char strValue[MAX_BUF_SIZE];

    try
    {
        // font size node
        DOMElement*  pFontsEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FONTS));
        if (NULL == pFontsEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pEditorEle->appendChild(pFontsEle);

        PVIOFONTCELLSIZE pVioFont = get_vioFontSize();
        CHAR* paszFontFacesGlobal = get_aszFontFacesGlobal();
        for (int iInx = 0; iInx < MAX_DIF_DOC; iInx++)
        {
            DOMElement*  pWindowEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_WINDOW));
            if (NULL == pWindowEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pFontsEle->appendChild(pWindowEle);

            // set attribute window name
            char strWindowsName[MAX_BUF_SIZE];
            memset(strWindowsName, 0x00, sizeof(strWindowsName));
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            LOADSTRING(hab, hResMod, IDS_TB_FONT_OTHER+iInx, strWindowsName);
            pWindowEle->setAttribute(XMLString::transcode(KEY_CAPTION), XMLString::transcode(strWindowsName));

            // create note font name
            pWindowEle->setAttribute(XMLString::transcode(KEY_FONT), XMLString::transcode(paszFontFacesGlobal));

            // set attribute X
            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", (pVioFont + iInx)->cx);
            pWindowEle->setAttribute(XMLString::transcode(KEY_X), XMLString::transcode(strValue));

            // set attribute Y
            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", (pVioFont + iInx)->cy);
            pWindowEle->setAttribute(XMLString::transcode(KEY_Y), XMLString::transcode(strValue));

            paszFontFacesGlobal += LF_FACESIZE;
        }
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

int CProfileSetXmlParser::KeyInfoExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement*  pEditorEle)
{
    int nRC = NO_ERROR;
    char strValue[MAX_BUF_SIZE];

    try
    {
        // font size node
        DOMElement*  pKeysEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_KEYS));
        if (NULL == pKeysEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pEditorEle->appendChild(pKeysEle);

        KEYPROFTABLE* pKeyTable = get_KeyTable();
        while (pKeyTable->Function != LAST_FUNC)
        {
          if ( pKeyTable->bEditor != 0 ) // GQ 2016/04/26: suppress not-customizable keys 
          {
            // set function node
            DOMElement*  pFunctionEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FUNCTION));
            if (NULL == pFunctionEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pKeysEle->appendChild(pFunctionEle);

            char strFuncName[MAX_BUF_SIZE];
            memset(strFuncName, 0x00, sizeof(strFuncName));
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            LOADSTRING(hab, hResMod, ID_TB_KEYS_DLG + pKeyTable->Function, strFuncName);

            // set function id attribution
            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", pKeyTable->Function);
            pFunctionEle->setAttribute(XMLString::transcode(KEY_FUNC_ID), XMLString::transcode(strValue));

            pFunctionEle->setAttribute(XMLString::transcode(KEY_FUNC_NAME), XMLString::transcode(strFuncName));

            // set key node
            char strKeyName[MAX_BUF_SIZE];
            memset(strKeyName, 0x00, sizeof(strKeyName));
            EQFBKeyName(strKeyName, pKeyTable->ucCode, pKeyTable->ucState);
            pFunctionEle->setAttribute(XMLString::transcode(KEY_KEY), XMLString::transcode(strKeyName));

            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", pKeyTable->ucCode);
            pFunctionEle->setAttribute(XMLString::transcode(KEY_UCODE), XMLString::transcode(strValue));

            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", pKeyTable->ucState);
            pFunctionEle->setAttribute(XMLString::transcode(KEY_USTATE), XMLString::transcode(strValue));

            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", pKeyTable->bEditor);
            pFunctionEle->setAttribute(XMLString::transcode(KEY_EDITOR), XMLString::transcode(strValue));

          }
          pKeyTable++;
        }

        // export speical char
        SPECCHARKEYVEC* pSCKeyTable = GetSpecCharKeyVec();
        for (size_t iInx = 0; iInx < (*pSCKeyTable).size(); iInx++)
        {
            DOMElement*  pSpecCharEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_SPEC_CHAR));
            if (NULL == pSpecCharEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pKeysEle->appendChild(pSpecCharEle);

            // set name attributioin
            pSpecCharEle->setAttribute(XMLString::transcode(KEY_DISP_STR), (*pSCKeyTable)[iInx].wstrDispChar);

            // set key node
            char strKeyName[MAX_BUF_SIZE];
            memset(strKeyName, 0x00, sizeof(strKeyName));
            EQFBKeyName(strKeyName, (*pSCKeyTable)[iInx].ucCode, (*pSCKeyTable)[iInx].ucState);
            pSpecCharEle->setAttribute(XMLString::transcode(KEY_KEY), XMLString::transcode(strKeyName));

            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", (*pSCKeyTable)[iInx].ucCode);
            pSpecCharEle->setAttribute(XMLString::transcode(KEY_UCODE), XMLString::transcode(strValue));

            memset(strValue, 0x00, sizeof(strValue));
            sprintf(strValue, "%d", (*pSCKeyTable)[iInx].ucState);
            pSpecCharEle->setAttribute(XMLString::transcode(KEY_USTATE), XMLString::transcode(strValue));
        }
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

int CProfileSetXmlParser::DoWorkbenchExport(xercesc::DOMDocument* pExportXmlDoc)
{
    int nRC = NO_ERROR;

    // get system property from file first
    PPROPSYSTEM  pSystemProp = NULL;
    nRC = OtmGetSysProp(pSystemProp);
    if (nRC)
    {
        return nRC;
    }

    try
    {
        DOMElement* elementRoot = pExportXmlDoc->getDocumentElement();
        if (NULL == elementRoot)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        // workbench node
        DOMElement*  pWorkbenchEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_WORKBENCH));
        if (NULL == pWorkbenchEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pWorkbenchEle);

        // general tab export
        nRC = WorkbenchGeneralExport(pExportXmlDoc, pWorkbenchEle, pSystemProp);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export general tab info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }

        // markup tab export
        nRC = WorkbenchMarkupExport(pExportXmlDoc, pWorkbenchEle, pSystemProp);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export markup tab info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }
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

int CProfileSetXmlParser::WorkbenchGeneralExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pWorkbenchEle, PPROPSYSTEM & pSystemProp)
{
    int nRC = NO_ERROR;
    char strValue[MAX_BUF_SIZE];

    try
    {
        DOMElement*  pGeneralEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_GENERAL));
        if (NULL == pGeneralEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pWorkbenchEle->appendChild(pGeneralEle);

        // if replace geenric inline node
        DOMElement*  pIfGenericEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_REPLACE_GENERIC_INLINE));
        if (NULL == pIfGenericEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        if (pSystemProp->fNoGenericMarkup)
        {
            pIfGenericEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfGenericEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }
        pGeneralEle->appendChild(pIfGenericEle);

        // logo time node
        SHORT nLogoTime = (SHORT)GetIntFromRegistry(APPL_Name, KEY_FIRSTTIME, 9876);
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", nLogoTime);
        DOMElement*  pLogoTimeEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_LOGO_TIME));
        if (NULL == pLogoTimeEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLogoTimeEle->setTextContent(XMLString::transcode(strValue));
        pGeneralEle->appendChild(pLogoTimeEle);

        // web browser node
        DOMElement*  pWebBrowserEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_WEB_BROWSER));
        if (NULL == pWebBrowserEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pWebBrowserEle->setTextContent(XMLString::transcode(pSystemProp->szWebBrowser));
        pGeneralEle->appendChild(pWebBrowserEle);

        // if use ie like tree view node
        DOMElement*  pIfUseIETreeEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_USE_IELIKE_TREE_VIEW));
        if (NULL == pIfUseIETreeEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        if (pSystemProp->fUseIELikeListWindows)
        {
            pIfUseIETreeEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }
        else
        {
            pIfUseIETreeEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        pGeneralEle->appendChild(pIfUseIETreeEle);

        // small lookup fuzzy level node
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSystemProp->lSmallLkupFuzzLevel);
        DOMElement*  pSmallLkupFuzzLevelEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_SMALL_LKUP_FUZZ_LEVEL));
        if (NULL == pSmallLkupFuzzLevelEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pSmallLkupFuzzLevelEle->setTextContent(XMLString::transcode(strValue));
        pGeneralEle->appendChild(pSmallLkupFuzzLevelEle);

        // medium lookup fuzzy level node
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSystemProp->lMediumLkupFuzzLevel);
        DOMElement*  pMediumLkupFuzzLevelEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_MEDIUM_LKUP_FUZZ_LEVEL));
        if (NULL == pMediumLkupFuzzLevelEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pMediumLkupFuzzLevelEle->setTextContent(XMLString::transcode(strValue));
        pGeneralEle->appendChild(pMediumLkupFuzzLevelEle);

        // large lookup fuzzy level node
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSystemProp->lLargeLkupFuzzLevel);
        DOMElement*  pLargeLkupFuzzLevelEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_LARGE_LKUP_FUZZ_LEVEL));
        if (NULL == pLargeLkupFuzzLevelEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLargeLkupFuzzLevelEle->setTextContent(XMLString::transcode(strValue));
        pGeneralEle->appendChild(pLargeLkupFuzzLevelEle);

        // small fuzzy level node
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSystemProp->lSmallFuzzLevel);
        DOMElement*  pSmallFuzzLevelEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_SMALL_FUZZ_LEVEL));
        if (NULL == pSmallFuzzLevelEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pSmallFuzzLevelEle->setTextContent(XMLString::transcode(strValue));
        pGeneralEle->appendChild(pSmallFuzzLevelEle);

        // medium fuzzy level node
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSystemProp->lMediumFuzzLevel);
        DOMElement*  pMediumFuzzLevelEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_MEDIUM_FUZZ_LEVEL));
        if (NULL == pMediumFuzzLevelEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pMediumFuzzLevelEle->setTextContent(XMLString::transcode(strValue));
        pGeneralEle->appendChild(pMediumFuzzLevelEle);

        // large fuzzy level node
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSystemProp->lLargeFuzzLevel);
        DOMElement*  pLargeFuzzLevelEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_LARGE_FUZZ_LEVEL));
        if (NULL == pLargeFuzzLevelEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLargeFuzzLevelEle->setTextContent(XMLString::transcode(strValue));
        pGeneralEle->appendChild(pLargeFuzzLevelEle);

        // default target lanuage
        DOMElement*  pDefTarLangEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DEF_TAR_LANG));
        if (NULL == pDefTarLangEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pDefTarLangEle->setTextContent(XMLString::transcode(pSystemProp->szSystemPrefLang));
        pGeneralEle->appendChild(pDefTarLangEle);

        // TWB window position
        DOMElement*  pOTMWindowPosEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_OTM_WINDOW_POS));
        if (NULL == pOTMWindowPosEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGeneralEle->appendChild(pOTMWindowPosEle);
        nRC = WindowPosExport(pOTMWindowPosEle, &pSystemProp->Swp);
        if (nRC)
        {
            return nRC;
        }
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

int CProfileSetXmlParser::WorkbenchMarkupExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pWorkbenchEle, PPROPSYSTEM & pSystemProp)
{
    int nRC = NO_ERROR;
    char strValue[MAX_BUF_SIZE];

    try
    {
        DOMElement*  pMarkupEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_MARKUP));
        if (NULL == pMarkupEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pWorkbenchEle->appendChild(pMarkupEle);

        // enable processing IDDOC node
        DOMElement*  pEnableProcessingIDDOCEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ENABLE_PROCESSING_IDDOC));
        if (NULL == pEnableProcessingIDDOCEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        if (!pSystemProp->fNoSgmlDitaProcessing)
        {
            pEnableProcessingIDDOCEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pEnableProcessingIDDOCEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }
        pMarkupEle->appendChild(pEnableProcessingIDDOCEle);

        // display entities value
        DOMElement*  pDisplayEntitiesValueEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DISPLAY_ENTITIES_VALUE));
        if (NULL == pDisplayEntitiesValueEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        if (pSystemProp->fEntityProcessing)
        {
            pDisplayEntitiesValueEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pDisplayEntitiesValueEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }
        pMarkupEle->appendChild(pDisplayEntitiesValueEle);

        // Action for unknown markup: 0=Abort, 1=Skip, 2=Use default value
        DOMElement*  pActionForUnknowMarkupEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ACTION_FOR_UNKNOW_MARKUP));
        if (NULL == pActionForUnknowMarkupEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        pMarkupEle->appendChild(pActionForUnknowMarkupEle);

        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSystemProp->usMemImpMrkupAction);
        pActionForUnknowMarkupEle->setAttribute(XMLString::transcode(KEY_INDEX), XMLString::transcode(strValue));

        switch(pSystemProp->usMemImpMrkupAction)
        {
        case 0:
            pActionForUnknowMarkupEle->setAttribute(XMLString::transcode(KEY_VALUE), XMLString::transcode(VALUE_ABORT));
            break;
        case 1:
            pActionForUnknowMarkupEle->setAttribute(XMLString::transcode(KEY_VALUE), XMLString::transcode(VALUE_SKIP));
            break;
        case 2:
            pActionForUnknowMarkupEle->setAttribute(XMLString::transcode(KEY_VALUE), XMLString::transcode(VALUE_USE));
            break;
        default:
            nRC = ERROR_READ_WORKBENCH_INFO_A;
            break;
        }
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

int CProfileSetXmlParser::DoLastUsedValuesExport(xercesc::DOMDocument* pExportXmlDoc)
{
    int nRC = NO_ERROR;

    try
    {
        DOMElement* elementRoot = pExportXmlDoc->getDocumentElement();
        if (NULL == elementRoot)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        // translation editor node
        DOMElement*  pLastUsedValEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_LAST_USED_VALUES));
        if (NULL == pLastUsedValEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pLastUsedValEle);

        // export global find last used values
        nRC = GlobFindLastValExp(pExportXmlDoc, pLastUsedValEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export global find last used value info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }

        // temporarily delete for the encrypt problem start
        /*
        // export EqfSharedMemoryAccess.LastUsed
        nRC = SharedMemAccessLastValExp(pExportXmlDoc, pLastUsedValEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export share memory access last used info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }

        // export EqfSharedMemoryCreate.LastUsed
        nRC = SharedMemCreateLastValExp(pExportXmlDoc, pLastUsedValEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export share memory create last used info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }
        */
        // temporarily delete end

        // export BatchList.LastUsed
        nRC = BatchListLastValExp(pExportXmlDoc, pLastUsedValEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export batch list last used info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }

        // export EQFNFLUENT.TRG
        nRC = NFluentTrgExp(pExportXmlDoc, pLastUsedValEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export EQFNFLUENT.TRG last used info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }

        // vi. Content of all .LUV files in the \OTM\LIST\ directory
        nRC = ListLastValExp(pExportXmlDoc, pLastUsedValEle);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export list last used info end. (%d)", nRC);
        if (nRC)
        {
            return nRC;
        }
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

int CProfileSetXmlParser::GlobFindLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle)
{
    int nRC = NO_ERROR;
    char strValue[MAX_BUF_SIZE];

    try
    {
        DOMElement*  pGlobFindEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_GLOB_FIND_LAST_VAL));
        if (NULL == pGlobFindEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLastUsedValEle->appendChild(pGlobFindEle);

        PFOLFINDDATA pFolFindData;                    // ptr to IDA of dialog
        if (!UtlAlloc((PVOID *)&pFolFindData, 0L, (LONG)sizeof(FOLFINDDATA), ERROR_STORAGE))
        {
            nRC = ERROR_OTM_NO_MORE_MEMORY_A;
            return nRC;
        }

        if (!GFR_GetLastUsedValues(pFolFindData))
        {
            nRC = ERROR_READ_GLOBAL_FIND_INFO_A;
            return nRC;
        }

        // Find String node
        DOMElement*  pFindStrEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FIND_STR));
        if (NULL == pFindStrEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pFindStrEle);
        pFindStrEle->setTextContent(pFolFindData->pLastUsed->szFolFind);

        // replace string node
        DOMElement*  pReplaceStrEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_REPLACE_STR));
        if (NULL == pReplaceStrEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pReplaceStrEle);
        pReplaceStrEle->setTextContent(pFolFindData->pLastUsed->szFolChangeTo);

        // If find string in source
        DOMElement*  pIfFindStrInSrcEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_FIND_STR_IN_SRC));
        if (NULL == pIfFindStrInSrcEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfFindStrInSrcEle);
        if (pFolFindData->pLastUsed->fAndFindInSource)
        {
            pIfFindStrInSrcEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfFindStrInSrcEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // find string in source
        DOMElement*  pFindStrInSrcEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FIND_STR_IN_SRC));
        if (NULL == pFindStrInSrcEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pFindStrInSrcEle);
        pFindStrInSrcEle->setTextContent(pFolFindData->pLastUsed->szAndFindInSource);

        // If apply batch list node
        DOMElement*  pIfApplyBatLstEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_APPLY_BAT_LST));
        if (NULL == pIfApplyBatLstEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfApplyBatLstEle);
        if (pFolFindData->pLastUsed->fApplyBatchList)
        {
            pIfApplyBatLstEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfApplyBatLstEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        wchar_t wstrValue[MAX_BUF_SIZE];
        // Wildcard for single char
        DOMElement*  pWildcharForSingleEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_WILDCHAR_FOR_SINGLE));
        if (NULL == pWildcharForSingleEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pWildcharForSingleEle);
        memset(wstrValue, 0x00, sizeof(wstrValue));
        swprintf(wstrValue, L"%c", pFolFindData->pLastUsed->chWildCardSingleChar);
        pWildcharForSingleEle->setTextContent(wstrValue);

        // Wildcard for multiple char
        DOMElement*  pWildcharForMultiEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_WILDCHAR_FOR_MULTI));
        if (NULL == pWildcharForMultiEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pWildcharForMultiEle);
        memset(wstrValue, 0x00, sizeof(wstrValue));
        swprintf(wstrValue, L"%c", pFolFindData->pLastUsed->chWildCardMultChar);
        pWildcharForMultiEle->setTextContent(wstrValue);

        // if update translation memory node
        DOMElement*  pIfUpdateTMEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_UPDATE_TM));
        if (NULL == pIfUpdateTMEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfUpdateTMEle);
        if (pFolFindData->pLastUsed->fFolFindUpdateTM)
        {
            pIfUpdateTMEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfUpdateTMEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // if confirm on replace mode
        DOMElement*  pIfComfirmOnReplaceEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_CONFIRM_ON_REPLACE));
        if (NULL == pIfComfirmOnReplaceEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfComfirmOnReplaceEle);
        if (pFolFindData->pLastUsed->fFolFindConfirm)
        {
            pIfComfirmOnReplaceEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfComfirmOnReplaceEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // if find in translatable text only
        DOMElement*  pIfFindTransableOnlyEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_FIND_TRANSABLE_ONLY));
        if (NULL == pIfFindTransableOnlyEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfFindTransableOnlyEle);
        if (pFolFindData->pLastUsed->fFolFindTranslTextOnly)
        {
            pIfFindTransableOnlyEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfFindTransableOnlyEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // if whole words only node
        DOMElement*  pIfWholeWordsOnlyEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_WHOLE_WORDS_ONLY));
        if (NULL == pIfWholeWordsOnlyEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfWholeWordsOnlyEle);
        if (pFolFindData->pLastUsed->fFolFindWholeWordsOnly)
        {
            pIfWholeWordsOnlyEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfWholeWordsOnlyEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // if Case Respect
        DOMElement*  pIfCaseRespectEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_CASE_RESPECT));
        if (NULL == pIfCaseRespectEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfCaseRespectEle);
        if (pFolFindData->pLastUsed->fFolFindCaseRespect)
        {
            pIfCaseRespectEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfCaseRespectEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // "search text in" identifier (0 = target, 1 = source, 2 = both) node
        DOMElement*  pSearchInEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_SEARCH_IN));
        if (NULL == pSearchInEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pSearchInEle);
        switch (pFolFindData->pLastUsed->sSearchIn)
        {
        case 0:
            pSearchInEle->setTextContent(XMLString::transcode(VAL_TARGET));
            break;
        case 1:
            pSearchInEle->setTextContent(XMLString::transcode(VAL_SOURCE));
            break;
        case 2:
            pSearchInEle->setTextContent(XMLString::transcode(VAL_BOTH));
            break;
        default:
            pSearchInEle->setTextContent(XMLString::transcode(EMPTY_STR));
            break;
        }

        // if show source node
        DOMElement*  pIfShowSrcEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_SHOW_SRC));
        if (NULL == pIfShowSrcEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfShowSrcEle);
        if (pFolFindData->pLastUsed->fShowSource)
        {
            pIfShowSrcEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfShowSrcEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // if show target node
        DOMElement*  pIfShowTarEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_SHOW_TAR));
        if (NULL == pIfShowTarEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfShowTarEle);
        if (pFolFindData->pLastUsed->fShowTarget)
        {
            pIfShowTarEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfShowTarEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // if respect linebreaks
        DOMElement*  pIfRespectLBEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IF_RESPECT_LB));
        if (NULL == pIfRespectLBEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pIfRespectLBEle);
        if (pFolFindData->pLastUsed->fRespectLineFeeds)
        {
            pIfRespectLBEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIfRespectLBEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // show before and after node
        DOMElement*  pShowBefAftEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_SHOW_BEFORE_AFTER));
        if (NULL == pShowBefAftEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pFolFindData->pLastUsed->sShowBeforeAfter);
        pShowBefAftEle->setTextContent(XMLString::transcode(strValue));
        pGlobFindEle->appendChild(pShowBefAftEle);

        // dialog size and position node
        DOMElement*  pGFWinPosEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_GF_WIN_POS));
        if (NULL == pGFWinPosEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pGlobFindEle->appendChild(pGFWinPosEle);
        nRC = WindowPosExport(pGFWinPosEle, &pFolFindData->pLastUsed->swpFolFindSizePos);
        if (nRC)
        {
            return nRC;
        }
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

int CProfileSetXmlParser::SharedMemAccessLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle)
{
    int nRC = NO_ERROR;

    try
    {
        DOMElement*  pSharedMemAccesssEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_SHARE_MEM_ACCESS_LAST_VAL));
        if (NULL == pSharedMemAccesssEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLastUsedValEle->appendChild(pSharedMemAccesssEle);

        PACCESSOPTIONS pOptions = NULL;
        loadPropFile(SHARE_MEM_ACCESS_LST_USED_FLE, ACCESSPASSWORD, (void **)&pOptions, sizeof(ACCESSOPTIONS));

        char * strValue = pOptions->szWebServiceURL;
        if ((NULL == pOptions) || ((NULL == strValue) || (strlen(strValue) == 0)))
        {
            return nRC;
        }

        // web service url
        DOMElement*  pWebSevUrlEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_WEB_SEV_URL));
        if (NULL == pWebSevUrlEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pWebSevUrlEle->setTextContent(XMLString::transcode(pOptions->szWebServiceURL));
        pSharedMemAccesssEle->appendChild(pWebSevUrlEle);

        // user id
        DOMElement*  pUserIdEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_USER_ID));
        if (NULL == pUserIdEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pUserIdEle->setTextContent(XMLString::transcode(pOptions->szUserID));
        pSharedMemAccesssEle->appendChild(pUserIdEle);

        // password
        DOMElement*  pPasswordEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_PASSWORD));
        if (NULL == pUserIdEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pPasswordEle->setTextContent(XMLString::transcode(pOptions->szPassword));
        pSharedMemAccesssEle->appendChild(pPasswordEle);

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

int CProfileSetXmlParser::SharedMemCreateLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle)
{
    int nRC = NO_ERROR;

    try
    {
        DOMElement*  pSharedMemCreateEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_SHARE_MEM_CREATE_LAST_VAL));
        if (NULL == pSharedMemCreateEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLastUsedValEle->appendChild(pSharedMemCreateEle);

        PCREATEOPTIONS pOptions = NULL;
        loadPropFile(SHARE_MEM_CREATE_LST_USED_FLE, OPTIONSPASSWORD, (void **)&pOptions, sizeof(CREATEOPTIONS));

        char * strValue = pOptions->szWebServiceURL;
        if ((NULL == pOptions) || ((NULL == strValue) || (strlen(strValue) == 0)))
        {
            return nRC;
        }

        strValue = (char *) malloc (MAX_BUF_SIZE);
        // web service url
        DOMElement*  pWebSevUrlEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_WEB_SEV_URL));
        if (NULL == pWebSevUrlEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pWebSevUrlEle->setTextContent(XMLString::transcode(pOptions->szWebServiceURL));
        pSharedMemCreateEle->appendChild(pWebSevUrlEle);

        // user id
        DOMElement*  pUserIdEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_USER_ID));
        if (NULL == pUserIdEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pUserIdEle->setTextContent(XMLString::transcode(pOptions->szUserID));
        pSharedMemCreateEle->appendChild(pUserIdEle);

        // password
        DOMElement*  pPasswordEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_PASSWORD));
        if (NULL == pPasswordEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pPasswordEle->setTextContent(XMLString::transcode(pOptions->szPassword));
        pSharedMemCreateEle->appendChild(pPasswordEle);

        // DSGenericType
        DOMElement*  pDsGenTypeEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DS_GENERIC_TYPE));
        if (NULL == pDsGenTypeEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pDsGenTypeEle->setTextContent(XMLString::transcode(pOptions->szDSGenericType));
        pSharedMemCreateEle->appendChild(pDsGenTypeEle);

        // DSType
        DOMElement*  pDsTypeEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DS_TYPE));
        if (NULL == pDsTypeEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pDsTypeEle->setTextContent(XMLString::transcode(pOptions->szDSType));
        pSharedMemCreateEle->appendChild(pDsTypeEle);

        // DSServer
        DOMElement*  pDsServerEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DS_SERVER));
        if (NULL == pDsServerEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pDsServerEle->setTextContent(XMLString::transcode(pOptions->szDSServer));
        pSharedMemCreateEle->appendChild(pDsServerEle);

        // DSPort
        DOMElement*  pDsPortEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DS_PORT));
        if (NULL == pDsPortEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pDsPortEle->setTextContent(XMLString::transcode(pOptions->szDSPort));
        pSharedMemCreateEle->appendChild(pDsPortEle);

        // DSUser
        DOMElement*  pDsUserEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DS_USER));
        if (NULL == pDsUserEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pDsUserEle->setTextContent(XMLString::transcode(pOptions->szDSUser));
        pSharedMemCreateEle->appendChild(pDsUserEle);

        // DSPassword
        DOMElement*  pDsPwdEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DS_PWD));
        if (NULL == pDsPwdEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pDsPwdEle->setTextContent(XMLString::transcode(pOptions->szDSPassword));
        pSharedMemCreateEle->appendChild(pDsPwdEle);

        // UserList
        DOMElement*  pUserListEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_DS_USER_LIST));
        if (NULL == pUserListEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pUserListEle->setTextContent(XMLString::transcode(pOptions->szUserList));
        pSharedMemCreateEle->appendChild(pUserListEle);

        free(strValue);
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

int CProfileSetXmlParser::BatchListLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle)
{
    int nRC = NO_ERROR;

    try
    {
        DOMElement*  pBatLstLastValEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_BATCH_LIST_LAST_VAL));
        if (NULL == pBatLstLastValEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLastUsedValEle->appendChild(pBatLstLastValEle);

        PFOLFINDDATA pFolFindData;
        BOOL fOK = UtlAlloc( (PVOID *)&pFolFindData, 0L, (LONG)sizeof(FOLFINDDATA), ERROR_STORAGE);
        if (!fOK)
        {
            nRC = ERROR_READ_BATCH_LIST_INFO_A;
            return nRC;
        }

        UtlMakeEQFPath(pFolFindData->szNameBuffer, NULC, PROPERTY_PATH, NULL);
        strcat(pFolFindData->szNameBuffer, BACKSLASH_STR);
        strcat(pFolFindData->szNameBuffer, LASTUSEDBATCHLIST);
        if (!UtlFileExist(pFolFindData->szNameBuffer))
        {
            // if file does not exist, just return
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Batch list last used info does not exist.");
            return nRC;
        }

        if (!GFR_ImportBatchList(pFolFindData, pFolFindData->szNameBuffer))
        {
            nRC = ERROR_READ_BATCH_LIST_INFO_A;
            return nRC;
        }

        // export batch list
        for (int iInx = 0; iInx < pFolFindData->iBatchListUsed; iInx++)
        {
            // Find String node
            DOMElement*  pBatchItmEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_BATCH_ITM));
            if (NULL == pBatchItmEle)
            {
                nRC = ERROR_OTM_XERCESC_EXPORT_A;
                return nRC;
            }
            pBatLstLastValEle->appendChild(pBatchItmEle);

            // size attribute
            PFOLFINDBATCHLISTENTRY pBatchListEntry = pFolFindData->ppBatchList[iInx];
            pBatchItmEle->setAttribute(XMLString::transcode(KEY_FIND_IN_TARGET), 
                                       (PSZ_W)(((PBYTE)pBatchListEntry) + pBatchListEntry->iTargetFindOffs));
            pBatchItmEle->setAttribute(XMLString::transcode(KEY_FIND_IN_SOURCE), 
                                       (PSZ_W)(((PBYTE)pBatchListEntry) + pBatchListEntry->iSourceFindOffs));
            pBatchItmEle->setAttribute(XMLString::transcode(KEY_REPLACE_IN_TARGET), 
                                       (PSZ_W)(((PBYTE)pBatchListEntry) + pBatchListEntry->iTargetChangeOffs));

        }
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

int CProfileSetXmlParser::NFluentTrgExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle)
{
    int nRC = NO_ERROR;

    try
    {
        DOMElement*  pNFluentEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EQF_NFLUENT_TRG));
        if (NULL == pNFluentEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLastUsedValEle->appendChild(pNFluentEle);

        // read the value from the file
        PNFLUENTDATA pTGData = NULL;
        if (!UtlAlloc((PVOID *)&pTGData, 0L, sizeof(NFLUENTDATA), ERROR_STORAGE))
        {
            nRC = ERROR_OTM_NO_MORE_MEMORY_A;
            return nRC;
        }

        if (!GetTriggerFileSettings(pTGData))
        {
            nRC = ERROR_READ_NFLUENT_INFO_A;
            return nRC;
        }

        // MTLOGGING node
        DOMElement* pMTLogEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_MTLOGGING));
        if (NULL == pMTLogEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pMTLogEle);

        if (pTGData->bMTLogging)
        {
            pMTLogEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pMTLogEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOMATCH node
        DOMElement* pNomatchEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_NOMATCH));
        if (NULL == pNomatchEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pNomatchEle);

        if (pTGData->bNoMatch)
        {
            pNomatchEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pNomatchEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // INCLUDEWORDCOUNT node
        DOMElement* pIncWrdCntEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_INCLUDEWORDCOUNT));
        if (NULL == pIncWrdCntEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pIncWrdCntEle);

        if (pTGData->bIncWrdCnt)
        {
            pIncWrdCntEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pIncWrdCntEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // ALLSEGS node
        DOMElement* pAllSegsEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ALLSEGS));
        if (NULL == pAllSegsEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pAllSegsEle);

        if (pTGData->bAllSegs)
        {
            pAllSegsEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pAllSegsEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // ALLWMATCH node
        DOMElement* pAllWatchEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ALLWMATCH));
        if (NULL == pAllWatchEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pAllWatchEle);

        if (pTGData->bAllWMatch)
        {
            pAllWatchEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pAllWatchEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOMATCHEXP node
        DOMElement* pNomatchExpEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_NOMATCHEXP));
        if (NULL == pNomatchExpEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pNomatchExpEle);

        if (pTGData->bNoMatchExp)
        {
            pNomatchExpEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pNomatchExpEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // ALLSEGSEXP node
        DOMElement* pAllSegsExpEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ALLSEGSEXP));
        if (NULL == pAllSegsExpEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pAllSegsExpEle);

        if (pTGData->bAllSegsExp)
        {
            pAllSegsExpEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pAllSegsExpEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // ALLWMATCHSOURCE node
        DOMElement* pAllWatchSrcEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ALLWMATCHSOURCE));
        if (NULL == pAllWatchSrcEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pAllWatchSrcEle);

        if (pTGData->bAllWMatchSrc)
        {
            pAllWatchSrcEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pAllWatchSrcEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOPROPOSAL node
        DOMElement* pNoProposalEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_NOPROPOSAL));
        if (NULL == pNoProposalEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pNoProposalEle);

        if (pTGData->bNoProposal)
        {
            pNoProposalEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pNoProposalEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOPROPOSALEXP node
        DOMElement* pNoProposalExpEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_NOPROPOSALEXP));
        if (NULL == pNoProposalExpEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pNoProposalExpEle);

        if (pTGData->bNoMatchExp)
        {
            pNoProposalExpEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pNoProposalExpEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOPROPOSALEXP node
        DOMElement* pXLIFFEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_XLIFF));
        if (NULL == pXLIFFEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pXLIFFEle);

        if (pTGData->bXliff)
        {
            pXLIFFEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pXLIFFEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOMATCH_NODUPLICATE node
        DOMElement* pNoMatchNoDuplicEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_NOMATCH_NODUPLICATE));
        if (NULL == pNoMatchNoDuplicEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pNoMatchNoDuplicEle);

        if (pTGData->bNoMatchNoDuplic)
        {
            pNoMatchNoDuplicEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pNoMatchNoDuplicEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOMATCHEXP_NODUPLICATE node
        DOMElement* pNoMatchExpNoDuplicEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_NOMATCHEXP_NODUPLICATE));
        if (NULL == pNoMatchExpNoDuplicEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pNoMatchExpNoDuplicEle);

        if (pTGData->bNoMatchExpNoDuplic)
        {
            pNoMatchExpNoDuplicEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pNoMatchExpNoDuplicEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // ALLSEGS_NODUPLICATE node
        DOMElement* pAllSegsNoDuplicEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ALLSEGS_NODUPLICATE));
        if (NULL == pAllSegsNoDuplicEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pAllSegsNoDuplicEle);

        if (pTGData->bAllSegsNoDuplic)
        {
            pAllSegsNoDuplicEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pAllSegsNoDuplicEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // ALLSEGSEXP_NODUPLICATE node
        DOMElement* pAllSegsExpNoDuplicEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ALLSEGSEXP_NODUPLICATE));
        if (NULL == pAllSegsExpNoDuplicEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pAllSegsExpNoDuplicEle);

        if (pTGData->bAllSegsExpNoDuplic)
        {
            pAllSegsExpNoDuplicEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pAllSegsExpNoDuplicEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // ALLWMATCH_NODUPLICATE node
        DOMElement* pAllWMatchNoDuplicEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ALLWMATCH_NODUPLICATE));
        if (NULL == pAllWMatchNoDuplicEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pAllWMatchNoDuplicEle);

        if (pTGData->bAllWMatchNoDuplic)
        {
            pAllWMatchNoDuplicEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pAllWMatchNoDuplicEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // ALLWMATCHSOURCE_NODUPLICATE node
        DOMElement* pAllWMatchSrcNoDuplicEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_ALLWMATCHSOURCE_NODUPLICATE));
        if (NULL == pAllWMatchSrcNoDuplicEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pAllWMatchSrcNoDuplicEle);

        if (pTGData->bAllWMatchSrcNoDuplic)
        {
            pAllWMatchSrcNoDuplicEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pAllWMatchSrcNoDuplicEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOPROPOSAL_NODUPLICATE node
        DOMElement* pNoProposalNoDuplicEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_NOPROPOSAL_NODUPLICATE));
        if (NULL == pNoProposalNoDuplicEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pNoProposalNoDuplicEle);

        if (pTGData->bNoProposalNoDuplic)
        {
            pNoProposalNoDuplicEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pNoProposalNoDuplicEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // NOPROPOSALEXP_NODUPLICATE node
        DOMElement* pNoProposalExpNoDuplicEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_NOPROPOSALEXP_NODUPLICATE));
        if (NULL == pNoProposalExpNoDuplicEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pNFluentEle->appendChild(pNoProposalExpNoDuplicEle);

        if (pTGData->bNoProposalExpNoDuplic)
        {
            pNoProposalExpNoDuplicEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pNoProposalExpNoDuplicEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }
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

int CProfileSetXmlParser::ListLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle)
{
    int nRC = NO_ERROR;

    try
    {
        DOMElement*  pLstLastValEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EQF_LIST_LAST_VAL));
        if (NULL == pLstLastValEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pLastUsedValEle->appendChild(pLstLastValEle);

        SubListLastValExp(pExportXmlDoc, pLstLastValEle, FOLIMPEXPLASTUSEDDIR);
        SubListLastValExp(pExportXmlDoc, pLstLastValEle, DOCEXPVALLASTUSED);
        SubListLastValExp(pExportXmlDoc, pLstLastValEle, DOCEXPXMLLASTUSED);
        SubListLastValExp(pExportXmlDoc, pLstLastValEle, DOCEXPINTLASTUSED);
        SubListLastValExp(pExportXmlDoc, pLstLastValEle, DOCEXPEXTLASTUSED);
        SubListLastValExp(pExportXmlDoc, pLstLastValEle, DOCEXPSOURCELASTUSED);
        SubListLastValExp(pExportXmlDoc, pLstLastValEle, DOCEXPSNOMATCHLASTUSED);
        SubListLastValExp(pExportXmlDoc, pLstLastValEle, DOCIMPSTARTPATH);
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

void CProfileSetXmlParser::SubListLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLstLastValEle, const char * strLstName)
{
    FILE * hfList     = NULL;

    // setup file name and the open file for input
    char strListPath[MAX_BUF_SIZE];
    memset(strListPath, 0x00, sizeof(strListPath));
    UtlMakeLUVFileName(strLstName, strListPath);
    hfList = fopen(strListPath, "rb");
    if (hfList == NULL)
    {
        return;
    }

    // skip any unicode prefix
    int iMaxStringLen = MAX_BUF_SIZE;
    PSZ_W pszBuffer   = NULL;
    pszBuffer = (PSZ_W)malloc(iMaxStringLen * sizeof(CHAR_W));
    if (pszBuffer == NULL ) return;

    memset(pszBuffer, 0, iMaxStringLen * sizeof(CHAR_W));
    fread(pszBuffer, 1, 2, hfList);
    if (memcmp( pszBuffer, UNICODEFILEPREFIX, 2) != 0)
    {
        // undo read
        fseek(hfList, 0, SEEK_SET);
    }

    DOMElement*  pSubLstLastValEle = pExportXmlDoc->createElement(XMLString::transcode(strLstName));
    if (NULL == pLstLastValEle)
    {
        return;
    }
    pLstLastValEle->appendChild(pSubLstLastValEle);

    while (!feof(hfList))
    {
        memset(pszBuffer, 0, iMaxStringLen * sizeof(CHAR_W));
        fgetws(pszBuffer, iMaxStringLen, hfList);

        if (*pszBuffer != 0)
        {
            // remove any trailing LF and CR
            PSZ_W pszEndPos = pszBuffer + (wcslen(pszBuffer) - 1);
            if ((pszEndPos >= pszBuffer) && (*pszEndPos == L'\n'))  *pszEndPos-- = 0;
            if ((pszEndPos >= pszBuffer) && (*pszEndPos == L'\r'))  *pszEndPos-- = 0;

            // add non-empty lines to export file
            if (*pszBuffer != 0)
            {
                DOMElement*  pValueEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_VALUE));
                if (NULL == pValueEle)
                {
                    return;
                }
                pSubLstLastValEle->appendChild(pValueEle);
                pValueEle->setTextContent(pszBuffer);
            }
        }
    }

    if (hfList != NULL) fclose (hfList);
    free(pszBuffer);
}

int CProfileSetXmlParser::DoProfileImport(POPTIONSET pOptionSet)
{
    int nRC = NO_ERROR;

    // check whether the xml exists or not
    if (OTM_NOT_FOUND == access(pOptionSet->strTarFile, 0))
    {
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Error: not find the imported file %s.", pOptionSet->strTarFile);
        nRC = ERROR_OTM_FILE_NOT_FIND_A;
        return nRC;
    }

    // backup all the profiles first
    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to backup profiles.");
    nRC = BackupProfiles();
    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Backup end (%d)", nRC);
    if (nRC)
    {
        return nRC;
    }

    try
    {
        XercesDOMParser * pOtmParser = new XercesDOMParser;
        pOtmParser->setValidationScheme(XercesDOMParser::Val_Auto);
        pOtmParser->setDoNamespaces(false);
        pOtmParser->setDoSchema(false);
        pOtmParser->setLoadExternalDTD(false);
        pOtmParser->parse(pOptionSet->strTarFile);

        xercesc::DOMDocument* xmlDoc = pOtmParser->getDocument();
        xercesc::DOMElement* elementRoot = xmlDoc->getDocumentElement();

        if (NULL != elementRoot)
        {
            nRC = DoImportFromRoot(elementRoot, pOptionSet);
        }
    }
    catch (const XMLException& xmlExp)
    {
        char* strMsg = XMLString::transcode(xmlExp.getMessage());
        memset(m_strErrMsg, 0x00, sizeof(m_strErrMsg));
        strcpy(m_strErrMsg, strMsg);
        XMLString::release(&strMsg);
        nRC = ERROR_OTM_XERCESC_DOM_A;
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

int CProfileSetXmlParser::DoImportFromRoot(DOMNode* elementRoot, POPTIONSET pOptionSet)
{
    int nRC = NO_ERROR;
    xercesc::DOMNodeList* children = elementRoot->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * nodeName = XMLString::transcode(currentNode->getNodeName());
        if (!stricmp(KEY_TRANS_EDITOR, nodeName) && (pOptionSet->bChkAll || pOptionSet->bChkTransEditor))
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to import translation editor.");
            nRC = DoTransEditorImport(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import translation editor end. (%d)", nRC);
        }
        else if (!stricmp(KEY_WORKBENCH, nodeName) && (pOptionSet->bChkAll || pOptionSet->bChkWorkbench))
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to import workbench.");
            nRC = DoWorkbenchImport(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import workbench end. (%d)", nRC);
        }
        else if (!stricmp(KEY_FOLDER_LIST, nodeName) && (pOptionSet->bChkAll || pOptionSet->bChkFldList))
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to import folder list.");
            nRC = DoFolderListImport(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import folder list end. (%d)", nRC);
        }
        else if (!stricmp(KEY_LAST_USED_VALUES, nodeName) && (pOptionSet->bChkAll || pOptionSet->bChkLastVal))
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to last used value list.");
            nRC = DoLastUsedValuesImport(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import last used value end. (%d)", nRC);
        }

        if (nRC)
        {
            break;
        }
    }

    return nRC;
}

int CProfileSetXmlParser::DoTransEditorImport(DOMNode* pTransEditorEle)
{
    int nRC = NO_ERROR;

    // load from property file
    EQFBReadProfile();

    PTBDOCUMENT pDoc = NULL;

    xercesc::DOMNodeList* children = pTransEditorEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = XMLString::transcode(currentNode->getNodeName());

        if (!stricmp(KEY_FONTS, strNodeName))
        {
            nRC = FontInfoImport(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import font info end. (%d)", nRC);
        }
        else if (!stricmp(KEY_COLORS, strNodeName))
        {
            nRC = ColorsInfoImport(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import color info end. (%d)", nRC);
        }
        else if (!stricmp(KEY_KEYS, strNodeName))
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import key info end. (%d)", nRC);
        {
            nRC = KeyInfoImport(currentNode);
        }

        if (nRC)
        {
            break;
        }
    }

    // Save the translation editor property only success
    if (!nRC)
    {
        EQFBWriteProfile(pDoc);
    }

    return nRC;
}

int CProfileSetXmlParser::DoWorkbenchImport(DOMNode* pWorkbenchEle)
{
    int nRC = NO_ERROR;
    // get system property from file first
    PPROPSYSTEM  pSystemProp = NULL;
    nRC = OtmGetSysProp(pSystemProp);
    if (nRC)
    {
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Error: failed to get system property info end. (%d)", nRC);
        return nRC;
    }

    xercesc::DOMNodeList* children = pWorkbenchEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = XMLString::transcode(currentNode->getNodeName());

        if (!stricmp(KEY_GENERAL, strNodeName))
        {
            nRC = WorkbenchGeneralImport(currentNode, pSystemProp);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import general (%d).", nRC);
        }
        else if (!stricmp(KEY_MARKUP, strNodeName))
        {
            nRC = WorkbenchMarkupImport(currentNode, pSystemProp);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import markup (%d).", nRC);
        }

        if (nRC)
        {
            break;
        }
    }

    // Save the system property
    if (!nRC)
    {
        nRC = OtmSaveSysProp(pSystemProp);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Save property (%d).", nRC);
    }

    return nRC;
}

int CProfileSetXmlParser::FontInfoImport(DOMNode* pFontInfoEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pFontInfoEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    CHAR * paszFontFacesGlobal;
    PVIOFONTCELLSIZE pVioFont = get_vioFontSize();
    paszFontFacesGlobal = get_aszFontFacesGlobal();

    int nDocInx = -1;

    for (XMLSize_t iInx = 0; iInx < nodeCount; iInx++)
    {
        if (nRC)
        {
            break;
        }

        xercesc::DOMNode* currentNode = children->item(iInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (stricmp(KEY_WINDOW, strNodeName))
        {
            continue;
        }

        if (!currentNode->hasAttributes())
        {
            continue;
        }

        XMLSize_t nAttriCnt = currentNode->getAttributes()->getLength();

        for (XMLSize_t jInx = 0; jInx < nAttriCnt; jInx++)
        {
            char * strName = XMLString::transcode(currentNode->getAttributes()->item(jInx)->getNodeName());
            char * strValue = XMLString::transcode(currentNode->getAttributes()->item(jInx)->getNodeValue());

            if (!stricmp(KEY_CAPTION, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
            {
                char strWindowsName[MAX_BUF_SIZE];
                memset(strWindowsName, 0x00, sizeof(strWindowsName));
                HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                LOADSTRING(hab, hResMod, IDS_TB_FONT_OTHER + nDocInx + 1, strWindowsName);

                if (stricmp(strValue, strWindowsName))
                {
                    nRC = ERROR_READ_FONT_INFO_A;
                    break;
                }
                nDocInx++;
            }
            else if (!stricmp(KEY_FONT, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
            {
                strcpy(paszFontFacesGlobal, strValue);
                paszFontFacesGlobal += LF_FACESIZE;
            }
            else if (!stricmp(KEY_X, strName))
            {
                pVioFont[nDocInx].cx = atoi(strValue);
            }
            else if (!stricmp(KEY_Y, strName))
            {
                pVioFont[nDocInx].cy = atoi(strValue);
            }
        } // end for

    } // end for

    return nRC;
}

int CProfileSetXmlParser::ColorsInfoImport(DOMNode* pColorsInfoEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pColorsInfoEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    // get color info pointer
    PTEXTTYPETABLE pTextTable = get_TextTypeTable();

    int nColorInx = 0;

    for (XMLSize_t iInx = 0; iInx < nodeCount; iInx++)
    {
        if (nRC || (nColorInx >= MAXCOLOUR))
        {
            break;
        }

        xercesc::DOMNode* currentNode = children->item(iInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (!stricmp(KEY_FONT_TYPE, strNodeName))
        {
            // import one color
            nRC = ColorInfoImport(currentNode, pTextTable, nColorInx);
            nColorInx++;
        }

    } // end for

    if (nColorInx != MAXCOLOUR)
    {
        nRC = WARN_DATA_NOT_CONSISTENT;
    }

    return nRC;
}

int CProfileSetXmlParser::ColorInfoImport(DOMNode* pColorInfoEle, PTEXTTYPETABLE pTextTable, int nColorInx)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pColorInfoEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t iInx = 0; iInx < nodeCount; iInx++)
    {
        if (nRC)
        {
            break;
        }

        xercesc::DOMNode* currentNode = children->item(iInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName, * strNodeValue = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (NULL != currentNode->getFirstChild())
        {
            strNodeValue = XMLString::transcode(currentNode->getFirstChild()->getNodeValue());
        }

        if (!stricmp(KEY_FOREGROUND_COLOR, strNodeName))
        {
            pTextTable[nColorInx].sFGColor = (SHORT) atoi(GetColorIndex(currentNode));
        }
        else if (!stricmp(KEY_BACKGROUND_COLOR, strNodeName))
        {
            pTextTable[nColorInx].sBGColor = (SHORT) atoi(GetColorIndex(currentNode));
        }
        else if (!stricmp(KEY_UNDERSCORE_FLAG, strNodeName))
        {
            // set underscore flag
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTextTable[nColorInx].fUnderscore = true;
            }
            else
            {
                pTextTable[nColorInx].fUnderscore = false;
            }
        }
        else if (!stricmp(KEY_REVERSE_FLAG, strNodeName))
        {
            // set reserve flag
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTextTable[nColorInx].fReverse = true;
            }
            else
            {
                pTextTable[nColorInx].fReverse = false;
            }
        }
        else if (!stricmp(KEY_FONT_SYMB_NAME, strNodeName))
        {
            // set reserve flag
            if (NULL != strNodeValue)
            {
                pTextTable[nColorInx].usFont = (USHORT) atoi(strNodeValue);
            }
        }
    }

    return nRC;
}

int CProfileSetXmlParser::KeyInfoImport(DOMNode* pKeyInfoEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pKeyInfoEle->getChildNodes();
    XMLSize_t nodeCount = children->getLength();

    BOOL bSpecChar = FALSE;

    KEYPROFTABLE* pKeyTable = get_KeyTable();

    SPECCHARKEYVEC* pSCKeyTable = GetSpecCharKeyVec();
    (*pSCKeyTable).clear();
    int iSpecCharInx = 0;

    for (XMLSize_t iInx = 0; iInx < nodeCount; iInx++)
    {
        if (nRC)
        {
            break;
        }

        xercesc::DOMNode* currentNode = children->item(iInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        // if no attribute, just skip
        if (!currentNode->hasAttributes())
        {
            continue;
        }

        // check the key of the element
        XMLSize_t nAttriCnt = currentNode->getAttributes()->getLength();

        // initalize bEditor field of current entry
        if (!stricmp(KEY_FUNCTION, strNodeName))
        {
          pKeyTable->bEditor = 0;
        }

        SPECCHARKEY keySpecKey;
        // set one key
        for (XMLSize_t jInx = 0; jInx < nAttriCnt; jInx++)
        {
            char * strName = XMLString::transcode(currentNode->getAttributes()->item(jInx)->getNodeName());
            char * strValue = XMLString::transcode(currentNode->getAttributes()->item(jInx)->getNodeValue());

            if (!stricmp(KEY_FUNCTION, strNodeName))
            {
                if (!stricmp(KEY_FUNC_ID, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
                {
                    pKeyTable->Function = (USHORT) atoi(strValue);
                }
                else if (!stricmp(KEY_UCODE, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
                {
                    pKeyTable->ucCode = (UCHAR) atoi(strValue);
                }
                else if (!stricmp(KEY_USTATE, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
                {
                    pKeyTable->ucState = (UCHAR) atoi(strValue);
                }
                else if (!stricmp(KEY_EDITOR, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
                {
                    pKeyTable->bEditor = (BYTE) atoi(strValue);
                }
            }
            else if (!stricmp(KEY_SPEC_CHAR, strNodeName))
            {
                bSpecChar = TRUE;
                if (!stricmp(KEY_DISP_STR, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
                {
                    wchar_t * wstrValue = (wchar_t *) currentNode->getAttributes()->item(jInx)->getNodeValue();
                    wcscpy(keySpecKey.wstrDispChar, wstrValue);
                }
                else if (!stricmp(KEY_UCODE, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
                {
                    keySpecKey.ucCode = (UCHAR) atoi(strValue);
                }
                else if (!stricmp(KEY_USTATE, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
                {
                    keySpecKey.ucState = (UCHAR) atoi(strValue);
                }
            }
        }

        if (!bSpecChar)
        {
          // GQ: special handling for entries with missing editor flags: use editor flags from default table
          if ( pKeyTable->bEditor == 0 )
          {
            // search entry for this function in the default table
            KEYPROFTABLE* pDefKeyTable = get_DefKeyTable();
            while ( (pDefKeyTable->Function != LAST_FUNC) && (pDefKeyTable->Function != pKeyTable->Function) )
            {
              pDefKeyTable++;
            }

            // use bEditor flags from default table when found
            if ( pDefKeyTable->Function == pKeyTable->Function )
            {
              pKeyTable->bEditor = pDefKeyTable->bEditor;
            }
          }

          // go to next entry
          pKeyTable++;
        }
        else
        {
            (*pSCKeyTable).push_back(keySpecKey);
            iSpecCharInx++;
        }
    }

    return nRC;
}

int CProfileSetXmlParser::WorkbenchGeneralImport(DOMNode* pGeneralEle, PPROPSYSTEM  & pSystemProp)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pGeneralEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName, * strNodeValue = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (NULL != currentNode->getFirstChild())
        {
            strNodeValue = XMLString::transcode(currentNode->getFirstChild()->getNodeValue());
        }

        if ((NULL == strNodeValue) || (strlen(strNodeValue) == 0))
        {
            if (stricmp(KEY_OTM_WINDOW_POS, strNodeName))
            {
                continue;
            }
        }

        if (!stricmp(KEY_IF_REPLACE_GENERIC_INLINE, strNodeName))
        {
            // if replace generic inline
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pSystemProp->fNoGenericMarkup = false;
            }
            else
            {
                pSystemProp->fNoGenericMarkup = true;
            }
        }
        else if (!stricmp(KEY_LOGO_TIME, strNodeName))
        {
            // logo display time
            WriteIntToRegistry(APPL_Name, KEY_FIRSTTIME, atoi(strNodeValue));
        }
        else if (!stricmp(KEY_WEB_BROWSER, strNodeName))
        {
            // web browser
            strcpy(pSystemProp->szWebBrowser, strNodeValue);
        }
        else if (!stricmp(KEY_IF_USE_IELIKE_TREE_VIEW, strNodeName))
        {
            // if use IE like tree view
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pSystemProp->fUseIELikeListWindows = true;
            }
            else
            {
                pSystemProp->fNoGenericMarkup = false;
            }
        }
        else if (!stricmp(KEY_SMALL_LKUP_FUZZ_LEVEL, strNodeName))
        {
            // fuzzy match lookup small
            pSystemProp->lSmallLkupFuzzLevel = atoi(strNodeValue);
        }
        else if (!stricmp(KEY_MEDIUM_LKUP_FUZZ_LEVEL, strNodeName))
        {
            // fuzzy match lookup medium
            pSystemProp->lMediumLkupFuzzLevel = atoi(strNodeValue);
        }
        else if (!stricmp(KEY_LARGE_LKUP_FUZZ_LEVEL, strNodeName))
        {
            // fuzzy match lookup large
            pSystemProp->lLargeLkupFuzzLevel = atoi(strNodeValue);
        }
        else if (!stricmp(KEY_SMALL_FUZZ_LEVEL, strNodeName))
        {
            // fuzzy match disply small
            pSystemProp->lSmallFuzzLevel = atoi(strNodeValue);
        }
        else if (!stricmp(KEY_MEDIUM_FUZZ_LEVEL, strNodeName))
        {
            // fuzzy match disply medium
            pSystemProp->lMediumFuzzLevel = atoi(strNodeValue);
        }
        else if (!stricmp(KEY_LARGE_FUZZ_LEVEL, strNodeName))
        {
            // fuzzy match disply large
            pSystemProp->lLargeFuzzLevel = atoi(strNodeValue);
        }
        else if (!stricmp(KEY_DEF_TAR_LANG, strNodeName))
        {
            // default target language
            strcpy(pSystemProp->szSystemPrefLang, strNodeValue);
        }
        else if (!stricmp(KEY_OTM_WINDOW_POS, strNodeName))
        {
            WindowPosImport(currentNode, &pSystemProp->Swp);
        }
    }

    return nRC;
}

int CProfileSetXmlParser::WorkbenchMarkupImport(DOMNode* pMarkupEle, PPROPSYSTEM  & pSystemProp)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pMarkupEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName, * strNodeValue = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (NULL != currentNode->getFirstChild())
        {
            strNodeValue = XMLString::transcode(currentNode->getFirstChild()->getNodeValue());
        }

        if (!stricmp(KEY_ENABLE_PROCESSING_IDDOC, strNodeName))
        {
            // enable processing IDDOC node
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pSystemProp->fNoSgmlDitaProcessing = FALSE;
            }
            else
            {
                pSystemProp->fNoSgmlDitaProcessing = TRUE;
            }
        }
        else if (!stricmp(KEY_DISPLAY_ENTITIES_VALUE, strNodeName))
        {
            // display entities value
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pSystemProp->fEntityProcessing = TRUE;
            }
            else
            {
                pSystemProp->fEntityProcessing = FALSE;
            }
        }
        else if (!stricmp(KEY_ACTION_FOR_UNKNOW_MARKUP, strNodeName))
        {
            // Action for unknown markup: 0=Abort, 1=Skip, 2=Use default value
            if (!currentNode->hasAttributes())
            {
                continue;
            }

            XMLSize_t nAttriCnt = currentNode->getAttributes()->getLength();

            for (XMLSize_t jInx = 0; jInx < nAttriCnt; jInx++)
            {
                char * strName = XMLString::transcode(currentNode->getAttributes()->item(jInx)->getNodeName());
                char * strValue = XMLString::transcode(currentNode->getAttributes()->item(jInx)->getNodeValue());

                if (!stricmp(KEY_INDEX, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
                {
                    pSystemProp->usMemImpMrkupAction = (USHORT) atoi(strValue);
                }
            }
        }
    }

    return nRC;
}

int CProfileSetXmlParser::DoFolderListExport(xercesc::DOMDocument* pExportXmlDoc)
{
    int nRC = NO_ERROR;

    // get folder list property
    HPROP           hFLLProp;   // folder list properties handler
    PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
    EQFINFO         ErrorInfo;  // error returned by property handler

    char strPropPath[MAX_PATH];
    memset(strPropPath, 0x00, sizeof(strPropPath));

    UtlMakeEQFPath(strPropPath, NULC, SYSTEM_PATH, NULL);
    strcat(strPropPath, BACKSLASH_STR);
    strcat(strPropPath, DEFAULT_FOLDERLIST_NAME);

    hFLLProp = OpenProperties(strPropPath, NULL, PROP_ACCESS_READ, &ErrorInfo);
    if (NULL == hFLLProp)
    {
        return nRC;
    }

    pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd(hFLLProp);

    if (NULL == pFLLProp)
    {
        CloseProperties(hFLLProp, PROP_QUIT, &ErrorInfo);
        return nRC;
    }

    try
    {
        DOMElement* elementRoot = pExportXmlDoc->getDocumentElement();
        if (NULL == elementRoot)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }

        // folder list node
        DOMElement*  pFolderListEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FOLDER_LIST));
        if (NULL == pFolderListEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        elementRoot->appendChild(pFolderListEle);

        // export path option
        DOMElement*  pExportPathEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXPORT_PATH));
        if (NULL == pExportPathEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExportPathEle);
        pExportPathEle->setTextContent(XMLString::transcode(pFLLProp->szExportPath));

        // import path option
        DOMElement*  pImportPathEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IMPORT_PATH));
        if (NULL == pImportPathEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pImportPathEle);
        pImportPathEle->setTextContent(XMLString::transcode(pFLLProp->szImportPath));

        // word count data only fFolExpHistLog
        DOMElement*  pExportDataOnlyEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXP_DATA_ONLY));
        if (NULL == pExportDataOnlyEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExportDataOnlyEle);
        if (pFLLProp->fFolExpHistLog)
        {
            pExportDataOnlyEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pExportDataOnlyEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // with dictionaries fFolExpWithDict
        DOMElement*  pExportWithDictEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXP_WITH_DICT));
        if (NULL == pExportWithDictEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExportWithDictEle);
        if (pFLLProp->fFolExpWithDict)
        {
            pExportWithDictEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pExportWithDictEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // with translation memory fFolExpWithMem
        DOMElement*  pExportWithMemEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXP_WITH_MEM));
        if (NULL == pExportWithMemEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExportWithMemEle);
        if (pFLLProp->fFolExpWithMem)
        {
            pExportWithMemEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pExportWithMemEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // with search memory database fFolExpWithROMem
        DOMElement*  pExportWithMemDatabaseEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXP_WITH_MEM_DATABASE));
        if (NULL == pExportWithMemDatabaseEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExportWithMemDatabaseEle);
        if (pFLLProp->fFolExpWithROMem)
        {
            pExportWithMemDatabaseEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pExportWithMemDatabaseEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // Add notes fFolExpAddNote
        DOMElement*  pExportAddNoteEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXP_ADD_NOTE));
        if (NULL == pExportAddNoteEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExportAddNoteEle);
        if (pFLLProp->fFolExpAddNote)
        {
            pExportAddNoteEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pExportAddNoteEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // delete folder after export fFolExpDelFolder
        DOMElement*  pExportDelFolderEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXP_DEL_FOLDER));
        if (NULL == pExportDelFolderEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExportDelFolderEle);
        if (pFLLProp->fFolExpDelFolder)
        {
            pExportDelFolderEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pExportDelFolderEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // szExpOriginator option
        DOMElement*  pExpOriginatorEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXP_ORGININATOR));
        if (NULL == pExpOriginatorEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExpOriginatorEle);
        pExpOriginatorEle->setTextContent(XMLString::transcode(pFLLProp->szExpOriginator));

        // szExpEMail option
        DOMElement*  pExpEmailEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_EXP_EMAIL));
        if (NULL == pExpEmailEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pExpEmailEle);
        pExpEmailEle->setTextContent(XMLString::transcode(pFLLProp->szExpEMail));

        // folder list position
        DOMElement*  pFolderListPosEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_FOLDER_LIST_POS));
        if (NULL == pFolderListPosEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pFolderListPosEle);
        nRC = WindowPosExport(pFolderListPosEle, &pFLLProp->Swp);
        if (nRC)
        {
            return nRC;
        }

        // chFolImpToDrive option
        DOMElement*  pImpDriveEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IMP_DRIVE));
        if (NULL == pImpDriveEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pImpDriveEle);
        char strDrive[_MAX_DRIVE];
        memset(strDrive, 0x00, sizeof(strDrive));
        sprintf(strDrive, "%c", pFLLProp->chFolExpDrive);
        pImpDriveEle->setTextContent(XMLString::transcode(strDrive));

        // BOOL fFolImpWithDict option
        DOMElement*  pImportWihtDictEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IMP_WITH_DICT));
        if (NULL == pImportWihtDictEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pImportWihtDictEle);
        if (pFLLProp->fFolExpDelFolder)
        {
            pImportWihtDictEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pImportWihtDictEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }

        // BOOL fFolImpWithMem option
        DOMElement*  pImpWihtMemEle = pExportXmlDoc->createElement(XMLString::transcode(KEY_IMP_WITH_MEM));
        if (NULL == pImpWihtMemEle)
        {
            nRC = ERROR_OTM_XERCESC_EXPORT_A;
            return nRC;
        }
        pFolderListEle->appendChild(pImpWihtMemEle);
        if (pFLLProp->fFolExpDelFolder)
        {
            pImpWihtMemEle->setTextContent(XMLString::transcode(TRUE_VALUE_LOWER));
        }
        else
        {
            pImpWihtMemEle->setTextContent(XMLString::transcode(FALSE_VALUE_LOWER));
        }
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

    CloseProperties(hFLLProp, PROP_QUIT, &ErrorInfo);

    return nRC;
}

int CProfileSetXmlParser::DoFolderListImport(DOMNode* pFolderListEle)
{
    int nRC = NO_ERROR;

    char * strNodeValue = NULL;

    // get folder list property
    HPROP           hFLLProp;   // folder list properties handler
    PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
    EQFINFO         ErrorInfo;  // error returned by property handler

    char strPropPath[MAX_PATH];
    memset(strPropPath, 0x00, sizeof(strPropPath));

    UtlMakeEQFPath(strPropPath, NULC, SYSTEM_PATH, NULL);
    strcat(strPropPath, BACKSLASH_STR);
    strcat(strPropPath, DEFAULT_FOLDERLIST_NAME);

    hFLLProp = OpenProperties(strPropPath, NULL, PROP_ACCESS_READ, &ErrorInfo);
    if (NULL == hFLLProp)
    {
        return nRC;
    }

    SetPropAccess( hFLLProp, PROP_ACCESS_WRITE);
    pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd(hFLLProp);

    if (NULL == pFLLProp)
    {
        CloseProperties(hFLLProp, PROP_QUIT, &ErrorInfo);
        return nRC;
    }

    // load from property file
    xercesc::DOMNodeList* children = pFolderListEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = XMLString::transcode(currentNode->getNodeName());

        if (NULL != currentNode->getFirstChild())
        {
            strNodeValue = XMLString::transcode(currentNode->getFirstChild()->getNodeValue());
        }

        if ((NULL == strNodeValue) || (strlen(strNodeValue) == 0))
        {
            continue;
        }

        if (!stricmp(KEY_EXPORT_PATH, strNodeName))
        {
            // export path option
            if (NULL != currentNode->getFirstChild())
            {
                strcpy(pFLLProp->szExportPath, strNodeValue);
            }
        }
        else if (!stricmp(KEY_IMPORT_PATH, strNodeName))
        {
            // import path option
            if (NULL != currentNode->getFirstChild())
            {
                strcpy(pFLLProp->szImportPath, strNodeValue);
            }
        }
        else if (!stricmp(KEY_EXP_DATA_ONLY, strNodeName))
        {
            // word count data only fFolExpHistLog
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolExpHistLog = true;
            }
            else
            {
                pFLLProp->fFolExpHistLog = false;
            }
        }
        else if (!stricmp(KEY_EXP_WITH_DICT, strNodeName))
        {
            // with dictionaries fFolExpWithDict
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolExpWithDict = true;
            }
            else
            {
                pFLLProp->fFolExpWithDict = false;
            }
        }
        else if (!stricmp(KEY_EXP_WITH_MEM, strNodeName))
        {
            // with translation memory fFolExpWithMem
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolExpWithMem = true;
            }
            else
            {
                pFLLProp->fFolExpWithMem = false;
            }
        }
        else if (!stricmp(KEY_EXP_WITH_MEM_DATABASE, strNodeName))
        {
            // with search memory database fFolExpWithROMem
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolExpWithROMem = true;
            }
            else
            {
                pFLLProp->fFolExpWithROMem = false;
            }
        }
        else if (!stricmp(KEY_EXP_ADD_NOTE, strNodeName))
        {
            // Add notes fFolExpAddNote
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolExpAddNote = true;
            }
            else
            {
                pFLLProp->fFolExpAddNote = false;
            }
        }
        else if (!stricmp(KEY_EXP_DEL_FOLDER, strNodeName))
        {
            // delete folder after export fFolExpDelFolder
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolExpDelFolder = true;
            }
            else
            {
                pFLLProp->fFolExpDelFolder = false;
            }
        }
        else if (!stricmp(KEY_IMP_WITH_MEM, strNodeName))
        {
            // szExpOriginator option
            strcpy(pFLLProp->szExpOriginator, strNodeValue);
        }
        else if (!stricmp(KEY_EXP_EMAIL, strNodeName))
        {
            // szExpEMail option
            strcpy(pFLLProp->szExpEMail, strNodeValue);
        }
        else if (!stricmp(KEY_FOLDER_LIST_POS, strNodeName))
        {
            WindowPosImport(currentNode, &pFLLProp->Swp);
        }
        else if (!stricmp(KEY_IMP_WITH_DICT, strNodeName))
        {
            // BOOL fFolImpWithDict option
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolImpWithDict = true;
            }
            else
            {
                pFLLProp->fFolImpWithDict = false;
            }
        }
        else if (!stricmp(KEY_IMP_WITH_DICT, strNodeName))
        {
            // BOOL fFolImpWithDict option
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolImpWithDict = true;
            }
            else
            {
                pFLLProp->fFolImpWithDict = false;
            }
        }
        else if (!stricmp(KEY_IMP_WITH_MEM, strNodeName))
        {
            // BOOL fFolImpWithMem option
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFLLProp->fFolImpWithMem = true;
            }
            else
            {
                pFLLProp->fFolImpWithMem = false;
            }
        }
        else if (!stricmp(KEY_IMP_DRIVE, strNodeName))
        {
            // chFolExpDrive option
            pFLLProp->chFolExpDrive = strNodeValue[0];
        }
    }

    // save property
    SaveProperties(hFLLProp, &ErrorInfo);
    CloseProperties(hFLLProp, PROP_QUIT, &ErrorInfo);

    return nRC;
}

int CProfileSetXmlParser::DoLastUsedValuesImport(DOMNode* pLastValEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pLastValEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = XMLString::transcode(currentNode->getNodeName());

        if (!stricmp(KEY_GLOB_FIND_LAST_VAL, strNodeName))
        {
            nRC =GlobFindLastValImp(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import global find last used value (%d).", nRC);
        }
        // temporarily delete for the encrypt problem start
        /*
        else if (!stricmp(KEY_SHARE_MEM_ACCESS_LAST_VAL, strNodeName))
        {
            nRC =SharedMemAccessLastValImp(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import shared memeory access last used value (%d).", nRC);
        }
        else if (!stricmp(KEY_SHARE_MEM_CREATE_LAST_VAL, strNodeName))
        {
            nRC =SharedMemCreateLastValImp(currentNode);
        }
        */
        // temporarily delete end
        else if (!stricmp(KEY_BATCH_LIST_LAST_VAL, strNodeName))
        {
            nRC =BatchListLastValImp(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import shared memeory create last used value (%d).", nRC);
        }
        else if (!stricmp(KEY_EQF_NFLUENT_TRG, strNodeName))
        {
            nRC = NFluentTrgImp(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import EQFNFLUENT.TRG last used value (%d).", nRC);
        }
        else if (!stricmp(KEY_EQF_LIST_LAST_VAL, strNodeName))
        {
            nRC = ListLastValImp(currentNode);
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import list last used value (%d).", nRC);
        }

        if (nRC)
        {
            break;
        }
    }

    return nRC;
}

int CProfileSetXmlParser::GlobFindLastValImp(DOMNode* pGlobFindEle)
{
    int nRC = NO_ERROR;

    // Get the current value first
    PFOLFINDDATA pFolFindData;                    // ptr to IDA of dialog
    if (!UtlAlloc((PVOID *)&pFolFindData, 0L, (LONG)sizeof(FOLFINDDATA), ERROR_STORAGE))
    {
        nRC = ERROR_OTM_NO_MORE_MEMORY_A;
        return nRC;
    }

    if (!GFR_GetLastUsedValues(pFolFindData))
    {
        nRC = ERROR_READ_GLOBAL_FIND_INFO_A;
        return nRC;
    }

    xercesc::DOMNodeList* children = pGlobFindEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName, * strNodeValue = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (NULL != currentNode->getFirstChild())
        {
            strNodeValue = XMLString::transcode(currentNode->getFirstChild()->getNodeValue());
        }

        if (!stricmp(KEY_FIND_STR, strNodeName))
        {
            // Find String node
            if ((NULL == strNodeValue) || (strlen(strNodeValue) == 0))
            {
                pFolFindData->pLastUsed->szFolFind[0] = L'\0';
            }
            else
            {
                wchar_t * wstrNodeValue = (wchar_t *) currentNode->getFirstChild()->getNodeValue();
                wcscpy(pFolFindData->pLastUsed->szFolFind, wstrNodeValue);
            }
        }
        else if (!stricmp(KEY_REPLACE_STR, strNodeName))
        {
            // replace string node
            if ((NULL == strNodeValue) || (strlen(strNodeValue) == 0))
            {
                pFolFindData->pLastUsed->szFolChangeTo[0] = L'\0';
            }
            else
            {
                wchar_t * wstrNodeValue = (wchar_t *) currentNode->getFirstChild()->getNodeValue();
                wcscpy(pFolFindData->pLastUsed->szFolChangeTo, wstrNodeValue);
            }
        }
        else if (!stricmp(KEY_IF_FIND_STR_IN_SRC, strNodeName))
        {
            // If find string in source
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fAndFindInSource = true;
            }
            else
            {
                pFolFindData->pLastUsed->fAndFindInSource = false;
            }
        }
        else if (!stricmp(KEY_FIND_STR_IN_SRC, strNodeName))
        {
            // find string in source
            if ((NULL == strNodeValue) || (strlen(strNodeValue) == 0))
            {
                pFolFindData->pLastUsed->szAndFindInSource[0] = L'\0';
            }
            else
            {
                wchar_t * wstrNodeValue = (wchar_t *) currentNode->getFirstChild()->getNodeValue();
                wcscpy(pFolFindData->pLastUsed->szAndFindInSource, wstrNodeValue);
            }
        }
        else if (!stricmp(KEY_IF_APPLY_BAT_LST, strNodeName))
        {
            // If apply batch list node
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fApplyBatchList = true;
            }
            else
            {
                pFolFindData->pLastUsed->fApplyBatchList = false;
            }
        }
        else if (!stricmp(KEY_WILDCHAR_FOR_SINGLE, strNodeName))
        {
            // Wildcard for single char
            if ((NULL == strNodeValue) || (strlen(strNodeValue) == 0))
            {
                pFolFindData->pLastUsed->chWildCardSingleChar = L'\0';
            }
            else
            {
                wchar_t * wstrNodeValue = (wchar_t *) currentNode->getFirstChild()->getNodeValue();
                pFolFindData->pLastUsed->chWildCardSingleChar = wstrNodeValue[0];
            }
        }
        else if (!stricmp(KEY_WILDCHAR_FOR_MULTI, strNodeName))
        {
            // Wildcard for multiple char
            if ((NULL == strNodeValue) || (strlen(strNodeValue) == 0))
            {
                pFolFindData->pLastUsed->chWildCardMultChar = L'\0';
            }
            else
            {
                wchar_t * wstrNodeValue = (wchar_t *) currentNode->getFirstChild()->getNodeValue();
                pFolFindData->pLastUsed->chWildCardMultChar = wstrNodeValue[0];
            }
        }
        else if (!stricmp(KEY_IF_UPDATE_TM, strNodeName))
        {
            // if update translation memory node
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fFolFindUpdateTM = true;
            }
            else
            {
                pFolFindData->pLastUsed->fFolFindUpdateTM = false;
            }
        }
        else if (!stricmp(KEY_IF_CONFIRM_ON_REPLACE, strNodeName))
        {
            // if confirm on replace mode
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fFolFindConfirm = true;
            }
            else
            {
                pFolFindData->pLastUsed->fFolFindConfirm = false;
            }
        }
        else if (!stricmp(KEY_IF_FIND_TRANSABLE_ONLY, strNodeName))
        {
            // if find in translatable text only
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fFolFindTranslTextOnly = true;
            }
            else
            {
                pFolFindData->pLastUsed->fFolFindTranslTextOnly = false;
            }
        }
        else if (!stricmp(KEY_IF_WHOLE_WORDS_ONLY, strNodeName))
        {
            // if whole words only node
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fFolFindWholeWordsOnly = true;
            }
            else
            {
                pFolFindData->pLastUsed->fFolFindWholeWordsOnly = false;
            }
        }
        else if (!stricmp(KEY_IF_CASE_RESPECT, strNodeName))
        {
            // if Case Respect
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fFolFindCaseRespect = true;
            }
            else
            {
                pFolFindData->pLastUsed->fFolFindCaseRespect = false;
            }
        }
        else if (!stricmp(KEY_SEARCH_IN, strNodeName))
        {
            // "search text in" identifier (0 = target, 1 = source, 2 = both) node
            if (!stricmp(VAL_TARGET, strNodeValue))
            {
                pFolFindData->pLastUsed->sSearchIn = 0;
            }
            else if (!stricmp(VAL_SOURCE, strNodeValue))
            {
                pFolFindData->pLastUsed->sSearchIn = 1;
            }
            else if (!stricmp(VAL_BOTH, strNodeValue))
            {
                pFolFindData->pLastUsed->sSearchIn = 2;
            }   
        }
        else if (!stricmp(KEY_IF_SHOW_SRC, strNodeName))
        {
            // if show source node
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fShowSource = true;
            }
            else
            {
                pFolFindData->pLastUsed->fShowSource = false;
            }
        }
        else if (!stricmp(KEY_IF_SHOW_TAR, strNodeName))
        {
            // if show target node
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fShowTarget = true;
            }
            else
            {
                pFolFindData->pLastUsed->fShowTarget = false;
            }
        }
        else if (!stricmp(KEY_IF_RESPECT_LB, strNodeName))
        {
            // if respect linebreaks
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pFolFindData->pLastUsed->fRespectLineFeeds = true;
            }
            else
            {
                pFolFindData->pLastUsed->fRespectLineFeeds = false;
            }
        }
        else if (!stricmp(KEY_SHOW_BEFORE_AFTER, strNodeName))
        {
            // show before and after node
            pFolFindData->pLastUsed->sShowBeforeAfter = (SHORT) atoi(strNodeValue);
        }
        else if (!stricmp(KEY_OTM_WINDOW_POS, strNodeName))
        {
            WindowPosImport(currentNode, &pFolFindData->pLastUsed->swpFolFindSizePos);
        }
    }

    // save the value to file
    if (!GFR_SaveLastUsedValues(pFolFindData))
    {
        nRC = ERROR_SAVE_GLOBAL_FIND_INFO_A;
    }

    return nRC;
}

int CProfileSetXmlParser::SharedMemAccessLastValImp(DOMNode* pSharedMemAccesssEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pSharedMemAccesssEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    PACCESSOPTIONS pOptions = NULL;
    pOptions = (PACCESSOPTIONS) malloc (sizeof(ACCESSOPTIONS));
    memset(pOptions, 0x00, sizeof(ACCESSOPTIONS));

    BOOL bWrite = FALSE;

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        char strNodeValue[MAX_BUF_SIZE];
        memset(strNodeValue, 0x00, sizeof(strNodeValue));
        if (NULL != currentNode->getFirstChild())
        {
            strcpy(strNodeValue, XMLString::transcode(currentNode->getFirstChild()->getNodeValue()));
        }

        if (!stricmp(KEY_WEB_SEV_URL, strNodeName))
        {
            bWrite = TRUE;
            if ((NULL != strNodeValue) && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szWebServiceURL, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_USER_ID, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szUserID, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_PASSWORD, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szPassword, strNodeValue, strlen(strNodeValue));
            }
        }
    }

    if (bWrite)
    {
        writePropFile(SHARE_MEM_ACCESS_LST_USED_FLE, ACCESSPASSWORD, (void *)pOptions, sizeof(ACCESSOPTIONS));
    }

    return nRC;
}

int CProfileSetXmlParser::SharedMemCreateLastValImp(DOMNode* pSharedMemCreateEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pSharedMemCreateEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    PCREATEOPTIONS pOptions = NULL;
    pOptions = (PCREATEOPTIONS) malloc (sizeof(CREATEOPTIONS));
    memset(pOptions, 0x00, sizeof(CREATEOPTIONS));

    BOOL bWrite = FALSE;

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        char strNodeValue[MAX_BUF_SIZE];
        memset(strNodeValue, 0x00, sizeof(strNodeValue));
        if (NULL != currentNode->getFirstChild())
        {
            strcpy(strNodeValue, XMLString::transcode(currentNode->getFirstChild()->getNodeValue()));
        }

        if (!stricmp(KEY_WEB_SEV_URL, strNodeName))
        {
            bWrite = TRUE;
            if ((NULL != strNodeValue) && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szWebServiceURL, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_USER_ID, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szUserID, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_PASSWORD, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szPassword, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_DS_GENERIC_TYPE, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szDSGenericType, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_DS_TYPE, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szDSType, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_DS_SERVER, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szDSServer, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_DS_PORT, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szDSPort, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_DS_USER, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szDSUser, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_DS_PWD, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szDSPassword, strNodeValue, strlen(strNodeValue));
            }
        }
        else if (!stricmp(KEY_DS_USER_LIST, strNodeName))
        {
            if (NULL != strNodeValue && (strlen(strNodeValue) != 0))
            {
                strncpy(pOptions->szUserList, strNodeValue, strlen(strNodeValue));
            }
        }
    }

    if (bWrite)
    {
        writePropFile(SHARE_MEM_CREATE_LST_USED_FLE, OPTIONSPASSWORD, (void *)pOptions, sizeof(CREATEOPTIONS));
    }

    return nRC;
}

int CProfileSetXmlParser::BatchListLastValImp(DOMNode* pBatLstLastValEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pBatLstLastValEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    PFOLFINDDATA pFolFindData;
    BOOL fOK = UtlAlloc( (PVOID *)&pFolFindData, 0L, (LONG)sizeof(FOLFINDDATA), ERROR_STORAGE);
    if (!fOK)
    {
        nRC = ERROR_READ_BATCH_LIST_INFO_A;
        return nRC;
    }

    int nBatchListInx = 0;
    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (!stricmp(KEY_BATCH_ITM, strNodeName))
        {
            if (!currentNode->hasAttributes())
            {
                continue;
            }

            XMLSize_t nAttriCnt = currentNode->getAttributes()->getLength();

            memset(&(pFolFindData->BatchEntryData), 0, sizeof( pFolFindData->BatchEntryData));

            for (XMLSize_t jInx = 0; jInx < nAttriCnt; jInx++)
            {
                char * strName = XMLString::transcode(currentNode->getAttributes()->item(jInx)->getNodeName());
                wchar_t * wstrValue = (wchar_t *) currentNode->getAttributes()->item(jInx)->getNodeValue();

                if (!stricmp(strName, KEY_FIND_IN_TARGET))
                {
                    wcscpy(pFolFindData->BatchEntryData.szTargetFind, wstrValue);
                }
                else if (!stricmp(strName, KEY_FIND_IN_SOURCE))
                {
                    wcscpy(pFolFindData->BatchEntryData.szSourceFind, wstrValue);
                }
                else if (!stricmp(strName, KEY_REPLACE_IN_TARGET))
                {
                    wcscpy(pFolFindData->BatchEntryData.szTargetChange, wstrValue);
                }
            }

            // Add the entry to list
            PFOLFINDBATCHLISTENTRY pFolFindBLEntry = GFR_CreateBatchListEntry(&pFolFindData->BatchEntryData);
            if (pFolFindBLEntry == NULL)
            {
                nRC = ERROR_SAVE_BATCH_LIST_INFO_A;
                return nRC;
            }

            // insert entry into our internal table
            if (!GFR_AddBatchListEntry(pFolFindData, pFolFindBLEntry, nBatchListInx))
            {
                nRC = ERROR_SAVE_BATCH_LIST_INFO_A;
                return nRC;
            }
            nBatchListInx++;
        }
    }

    if (nBatchListInx > 0)
    {
        // set the profile name again
        UtlMakeEQFPath(pFolFindData->szNameBuffer, NULC, PROPERTY_PATH, NULL);
        strcat(pFolFindData->szNameBuffer, BACKSLASH_STR);
        strcat(pFolFindData->szNameBuffer, LASTUSEDBATCHLIST);

        // save the value
        if (!GFR_ExportBatchList(pFolFindData, pFolFindData->szNameBuffer))
        {
            nRC = ERROR_SAVE_BATCH_LIST_INFO_A;
        }
    }

    return nRC;
}

int CProfileSetXmlParser::NFluentTrgImp(DOMNode* pNFluentEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pNFluentEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    PNFLUENTDATA pTGData = NULL;
    if (!UtlAlloc((PVOID *)&pTGData, 0L, sizeof(NFLUENTDATA), ERROR_STORAGE))
    {
        nRC = ERROR_OTM_NO_MORE_MEMORY_A;
        return nRC;
    }

    if (!GetTriggerFileSettings(pTGData))
    {
        nRC = ERROR_READ_NFLUENT_INFO_A;
        return nRC;
    }

    for (XMLSize_t nInx = 0; nInx < nodeCount; nInx++)
    {
        xercesc::DOMNode* currentNode = children->item(nInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName, * strNodeValue = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (NULL != currentNode->getFirstChild())
        {
            strNodeValue = XMLString::transcode(currentNode->getFirstChild()->getNodeValue());
        }

        if ((NULL == strNodeValue) || (strlen(strNodeValue) == 0))
        {
            continue;
        }

        if (!stricmp(KEY_NOMATCH, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bNoMatch = true;
            }
            else
            {
                pTGData->bNoMatch = false;
            }
        }
        else if (!stricmp(KEY_ALLSEGS, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bAllSegs = true;
            }
            else
            {
                pTGData->bAllSegs = false;
            }
        }
        else if (!stricmp(KEY_ALLWMATCH, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bAllWMatch = true;
            }
            else
            {
                pTGData->bAllWMatch = false;
            }
        }
        else if (!stricmp(KEY_NOMATCHEXP, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bNoMatchExp = true;
            }
            else
            {
                pTGData->bNoMatchExp = false;
            }
        }
        else if (!stricmp(KEY_ALLSEGSEXP, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bAllSegsExp = true;
            }
            else
            {
                pTGData->bAllSegsExp = false;
            }
        }
        else if (!stricmp(KEY_ALLWMATCHSOURCE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bAllWMatchSrc = true;
            }
            else
            {
                pTGData->bAllWMatchSrc = false;
            }
        }
        else if (!stricmp(KEY_NOPROPOSAL, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bNoProposal = true;
            }
            else
            {
                pTGData->bNoProposal = false;
            }
        }
        else if (!stricmp(KEY_NOPROPOSALEXP, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bNoProposalExp = true;
            }
            else
            {
                pTGData->bNoProposalExp = false;
            }
        }
        else if (!stricmp(KEY_XLIFF, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bXliff = true;
            }
            else
            {
                pTGData->bXliff = false;
            }
        }
        else if (!stricmp(KEY_INCLUDEWORDCOUNT, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bIncWrdCnt = true;
            }
            else
            {
                pTGData->bIncWrdCnt = false;
            }
        }
        else if (!stricmp(KEY_NOMATCH_NODUPLICATE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bNoMatchNoDuplic = true;
            }
            else
            {
                pTGData->bNoMatchNoDuplic = false;
            }
        }
        else if (!stricmp(KEY_NOMATCHEXP_NODUPLICATE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bNoMatchExpNoDuplic = true;
            }
            else
            {
                pTGData->bNoMatchExpNoDuplic = false;
            }
        }
        else if (!stricmp(KEY_ALLSEGS_NODUPLICATE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bAllSegsNoDuplic = true;
            }
            else
            {
                pTGData->bAllSegsNoDuplic = false;
            }
        }
        else if (!stricmp(KEY_ALLSEGSEXP_NODUPLICATE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bAllSegsExpNoDuplic = true;
            }
            else
            {
                pTGData->bAllSegsExpNoDuplic = false;
            }
        }
        else if (!stricmp(KEY_ALLWMATCH_NODUPLICATE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bAllWMatchNoDuplic = true;
            }
            else
            {
                pTGData->bAllWMatchNoDuplic = false;
            }
        }
        else if (!stricmp(KEY_ALLWMATCHSOURCE_NODUPLICATE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bAllWMatchSrcNoDuplic = true;
            }
            else
            {
                pTGData->bAllWMatchSrcNoDuplic = false;
            }
        }
        else if (!stricmp(KEY_NOPROPOSAL_NODUPLICATE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bNoProposalNoDuplic = true;
            }
            else
            {
                pTGData->bNoProposalNoDuplic = false;
            }
        }
        else if (!stricmp(KEY_NOPROPOSALEXP_NODUPLICATE, strNodeName))
        {
            if (!stricmp(TRUE_VALUE_LOWER, strNodeValue))
            {
                pTGData->bNoProposalExpNoDuplic = true;
            }
            else
            {
                pTGData->bNoProposalExpNoDuplic = false;
            }
        }
    }

    // save the date
    if (!SaveTriggerFileSettings(pTGData))
    {
        nRC = ERROR_SAVE_NFLUENT_INFO_A;
    }

    return nRC;
}

int CProfileSetXmlParser::ListLastValImp(DOMNode* pLstLastValEle)
{
    int nRC = NO_ERROR;

    xercesc::DOMNodeList* children = pLstLastValEle->getChildNodes();
    if (NULL == children)
    {
        return nRC;
    }
    XMLSize_t nodeCount = children->getLength();

    for (XMLSize_t iInx = 0; iInx < nodeCount; iInx++)
    {
        xercesc::DOMNode* currentNode = children->item(iInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        if (!stricmp(FOLIMPEXPLASTUSEDDIR, strNodeName))
        {
            SubListLastValImp(currentNode, FOLIMPEXPLASTUSEDDIR);
        }
        else if (!stricmp(DOCEXPVALLASTUSED, strNodeName))
        {
            SubListLastValImp(currentNode, DOCEXPVALLASTUSED);
        }
        else if (!stricmp(DOCEXPXMLLASTUSED, strNodeName))
        {
            SubListLastValImp(currentNode, DOCEXPXMLLASTUSED);
        }
        else if (!stricmp(DOCEXPINTLASTUSED, strNodeName))
        {
            SubListLastValImp(currentNode, DOCEXPINTLASTUSED);
        }
        else if (!stricmp(DOCEXPEXTLASTUSED, strNodeName))
        {
            SubListLastValImp(currentNode, DOCEXPEXTLASTUSED);
        }
        else if (!stricmp(DOCEXPSOURCELASTUSED, strNodeName))
        {
            SubListLastValImp(currentNode, DOCEXPSOURCELASTUSED);
        }
        else if (!stricmp(DOCEXPSNOMATCHLASTUSED, strNodeName))
        {
            SubListLastValImp(currentNode, DOCEXPSNOMATCHLASTUSED);
        }
        else if (!stricmp(DOCIMPSTARTPATH, strNodeName))
        {
            SubListLastValImp(currentNode, DOCIMPSTARTPATH);
        }
    }

    return nRC;
}

void CProfileSetXmlParser::SubListLastValImp(DOMNode* pSubLstLastValEle, const char * strLstName)
{
    xercesc::DOMNodeList* children = pSubLstLastValEle->getChildNodes();
    if (NULL == children)
    {
        return;
    }

    XMLSize_t nodeCount = children->getLength();
    if (nodeCount <= 0)
    {
        return;
    }

    WSTRLIST wlstValues;

    for (XMLSize_t iInx = 0; iInx < nodeCount; iInx++)
    {
        xercesc::DOMNode* currentNode = children->item(iInx);
        if (!currentNode->getNodeType() || currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
        {
            continue;
        }

        char * strNodeName = NULL;
        strNodeName  = XMLString::transcode(currentNode->getNodeName());

        wchar_t * wstrNodeValue = NULL;
        if (NULL != currentNode->getFirstChild())
        {
            wstrNodeValue = (wchar_t *) currentNode->getFirstChild()->getNodeValue();
        }

        if ((NULL == wstrNodeValue) || (wcslen(wstrNodeValue) == 0))
        {
            continue;
        }

        if (!stricmp(KEY_VALUE, strNodeName))
        {
            // Add only not duplicated
            BOOL bDuplicated = FALSE;
            if (!wlstValues.empty())
            {
                for (size_t jInx = 0; jInx < wlstValues.size(); jInx++)
                {
                    if (!wcsicmp(wstrNodeValue, wlstValues[jInx].c_str()))
                    {
                        bDuplicated = TRUE;
                        break;
                    }
                }
            }

            if (!bDuplicated)
            {
                wlstValues.push_back(wstrNodeValue);
            }
        }
    }

    if (wlstValues.empty() || (0 == wlstValues.size()))
    {
        return;
    }

    FILE * hfList     = NULL;

    // setup file name and the open file for input
    char strListPath[MAX_BUF_SIZE];
    memset(strListPath, 0x00, sizeof(strListPath));
    UtlMakeLUVFileName(strLstName, strListPath);
    hfList = fopen(strListPath, "wb");
    if (hfList == NULL)
    {
        return;
    }

    fwrite((void *)UNICODEFILEPREFIX, 1, 2, hfList);

    for (size_t jInx = 0; jInx < wlstValues.size(); jInx++)
    {
        fputws(wlstValues[jInx].c_str(), hfList);
        fputws( L"\r\n", hfList);
    }

    fclose(hfList);
}

int CProfileSetXmlParser::WindowPosExport(DOMElement* pElement, EQF_SWP * pSwp)
{
    int nRC = NO_ERROR;
    char strValue[MAX_BUF_SIZE];

    try
    {
        // cx
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSwp->cx);
        pElement->setAttribute(XMLString::transcode(KEY_POS_CX), XMLString::transcode(strValue));

        // cy
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSwp->cy);
        pElement->setAttribute(XMLString::transcode(KEY_POS_CY), XMLString::transcode(strValue));

        // x
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSwp->x);
        pElement->setAttribute(XMLString::transcode(KEY_POS_X), XMLString::transcode(strValue));

        // y
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSwp->y);
        pElement->setAttribute(XMLString::transcode(KEY_POS_Y), XMLString::transcode(strValue));

        // fs
        memset(strValue, 0x00, sizeof(strValue));
        sprintf(strValue, "%d", pSwp->fs);
        pElement->setAttribute(XMLString::transcode(KEY_POS_FS), XMLString::transcode(strValue));
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

void CProfileSetXmlParser::WindowPosImport(xercesc::DOMNode* pDomNode, EQF_SWP * pSwp)
{
    if (!pDomNode->hasAttributes())
    {
        return;
    }

    XMLSize_t nAttriCnt = pDomNode->getAttributes()->getLength();

    for (XMLSize_t jInx = 0; jInx < nAttriCnt; jInx++)
    {
        char * strName = XMLString::transcode(pDomNode->getAttributes()->item(jInx)->getNodeName());
        char * strValue = XMLString::transcode(pDomNode->getAttributes()->item(jInx)->getNodeValue());

        if (!stricmp(strName, KEY_POS_CX))
        {
            // cx
            pSwp->cx = (SHORT) atoi(strValue);
        }
        else if (!stricmp(strName, KEY_POS_CY))
        {
            // cx
            pSwp->cy = (SHORT) atoi(strValue);
        }
        else if (!stricmp(strName, KEY_POS_X))
        {
            // x
            pSwp->x = (SHORT) atoi(strValue);
        }
        else if (!stricmp(strName, KEY_POS_Y))
        {
            // y
            pSwp->y = (SHORT) atoi(strValue);
        }
        else if (!stricmp(strName, KEY_POS_FS))
        {
            // fs
            pSwp->fs = (SHORT) atoi(strValue);
        }
    }
}

int CProfileSetXmlParser::BackupProfiles()
{
    int nRC = NO_ERROR;

    char strToPath[MAX_EQF_PATH];
    memset(strToPath, 0x00, sizeof(strToPath));
    UtlMakeEQFPath(strToPath, NULC, PLUGIN_PATH, NULL);
    strcat(strToPath, BACKSLASH_STR);
    strcat(strToPath, BAK_FLD_NAME);

    // create the folder if not existed
    if (!CreateDirectory(strToPath, NULL))
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            nRC = ERROR_OTM_CREATE_FOLDER_A;
            return nRC;
        }
    }

    char strZipProfile[MAX_PATH];
    memset(strZipProfile, 0x00, sizeof(strZipProfile));
    sprintf(strZipProfile, "%s\\%s_%s.zip", strToPath, PROFILE_BAK_NAME,GetTimeStampStr());

    zipFile zipProfile = zipOpen(strZipProfile, APPEND_STATUS_CREATE);
    if (NULL == zipProfile)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    char strFromPath[MAX_EQF_PATH];

    // zip EqfProW.prp
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, TPRO_PROFILE);
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // zip EQFSYSW.PRP
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, SYSTEM_PROPERTIES_NAME);
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // zip FLIST.L00
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, DEFAULT_FOLDERLIST_NAME);
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // zip GlobFindLastUsedValues.dat
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, "GlobFindLastUsedValues.dat");
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // Temporarily delete start
    /*
    // zip EqfSharedMemoryAccess.LastUsed
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, "EqfSharedMemoryAccess.LastUsed");
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // zip EqfSharedMemoryCreate.LastUsed
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, "EqfSharedMemoryCreate.LastUsed");
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }
    */
    // Temporarily delete end

    // zip BatchList.LastUsed
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, "BatchList.LastUsed");
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // zip EQFNFLUENT.TRG
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, "EQFNFLUENT.TRG");
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // zip OtmSpecCharKey.PRP
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeEQFPath(strFromPath, NULC, PROPERTY_PATH, NULL);
    strcat(strFromPath, BACKSLASH_STR);
    strcat(strFromPath, SPEC_CHAR_KEY_PROFILE);
    nRC = CompressOneFile(zipProfile, strFromPath, "PROPERTY");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // backup all .LUV files in the \OTM\LIST\ directory
    // backup folder export
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeLUVFileName(FOLIMPEXPLASTUSEDDIR, strFromPath);
    nRC = CompressOneFile(zipProfile, strFromPath, "LIST");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // backup document export
    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeLUVFileName(DOCEXPVALLASTUSED, strFromPath);
    nRC = CompressOneFile(zipProfile, strFromPath, "LIST");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeLUVFileName(DOCEXPXMLLASTUSED, strFromPath);
    nRC = CompressOneFile(zipProfile, strFromPath, "LIST");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeLUVFileName(DOCEXPINTLASTUSED, strFromPath);
    nRC = CompressOneFile(zipProfile, strFromPath, "LIST");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeLUVFileName(DOCEXPEXTLASTUSED, strFromPath);
    nRC = CompressOneFile(zipProfile, strFromPath, "LIST");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeLUVFileName(DOCEXPSOURCELASTUSED, strFromPath);
    nRC = CompressOneFile(zipProfile, strFromPath, "LIST");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeLUVFileName(DOCEXPSNOMATCHLASTUSED, strFromPath);
    nRC = CompressOneFile(zipProfile, strFromPath, "LIST");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    memset(strFromPath, 0x00, sizeof(strFromPath));
    UtlMakeLUVFileName(DOCIMPSTARTPATH, strFromPath);
    nRC = CompressOneFile(zipProfile, strFromPath, "LIST");
    if (nRC)
    {
        nRC = ERROR_BACKUP_PROFILE_A;
        return nRC;
    }

    // close the zip file
    zipClose(zipProfile, NULL);

    return nRC;
}

const char * CProfileSetXmlParser::GetColorIndex(DOMNode* pColorInxEle)
{
    if (!pColorInxEle->hasAttributes())
    {
        return EMPTY_STR;
    }

    XMLSize_t nAttriCnt = pColorInxEle->getAttributes()->getLength();

    for (XMLSize_t iInx = 0; iInx < nAttriCnt; iInx++)
    {
        const char * strName = XMLString::transcode(pColorInxEle->getAttributes()->item(iInx)->getNodeName());
        const char * strValue = XMLString::transcode(pColorInxEle->getAttributes()->item(iInx)->getNodeValue());

        if (!stricmp(KEY_INDEX, strName) && ((NULL != strValue) && (strlen(strValue) != 0)))
        {
            return strValue;
        }
    }

    return EMPTY_STR;
}

const char * CProfileSetXmlParser::GetParserErrMsg()
{
    return m_strErrMsg;
}

CProfileSetXmlParser::CProfileSetXmlParser(void)
{
    memset(m_strErrMsg, 0x00, sizeof(m_strErrMsg));
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

CProfileSetXmlParser::~CProfileSetXmlParser(void)
{
    XMLPlatformUtils::Terminate();
}

